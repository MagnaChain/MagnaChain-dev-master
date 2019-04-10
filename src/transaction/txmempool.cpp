// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "transaction/txmempool.h"

#include "consensus/consensus.h"
#include "consensus/tx_verify.h"
#include "consensus/validation.h"
#include "validation/validation.h"
#include "policy/policy.h"
#include "policy/fees.h"
#include "misc/reverse_iterator.h"
#include "io/streams.h"
#include "misc/timedata.h"
#include "utils/util.h"
#include "utils/utilmoneystr.h"
#include "utils/utiltime.h"
#include "rpc/branchchainrpc.h"
#include "chain/branchdb.h"

MCTxMemPoolEntry::MCTxMemPoolEntry(const MCTransactionRef& _tx, const MCAmount& _nFee,
    int64_t _nTime, unsigned int _entryHeight,
    bool _spendsCoinbase, int64_t _sigOpsCost, LockPoints lp, uint64_t order) :
    tx(_tx), nFee(_nFee), nTime(_nTime), entryHeight(_entryHeight),
    spendsCoinbase(_spendsCoinbase), sigOpCost(_sigOpsCost), lockPoints(lp)
{
    nTxWeight = GetTransactionWeight(*tx);
    nUsageSize = RecursiveDynamicUsage(tx);

    nCountWithDescendants = 1;
    nSizeWithDescendants = GetTxSize();
    nModFeesWithDescendants = nFee;

    feeDelta = 0;

    nCountWithAncestors = 1;
    nSizeWithAncestors = GetTxSize();
    nModFeesWithAncestors = nFee;
    nSigOpCostWithAncestors = sigOpCost;
    entryOrder = (order > 0 ? order : mempool.GenerateNextOrder());
}

MCTxMemPoolEntry::MCTxMemPoolEntry(const MCTxMemPoolEntry& other)
{
    *this = other;
}

void MCTxMemPoolEntry::UpdateFeeDelta(int64_t newFeeDelta)
{
    nModFeesWithDescendants += newFeeDelta - feeDelta;
    nModFeesWithAncestors += newFeeDelta - feeDelta;
    feeDelta = newFeeDelta;
}

void MCTxMemPoolEntry::UpdateLockPoints(const LockPoints& lp)
{
    lockPoints = lp;
}

void MCTxMemPoolEntry::UpdateContract(SmartLuaState* sls)
{
    // 该接口仅在第一次加到txmempool中调用
    if (contractData == nullptr) {
        contractData.reset(new MCTxMemPoolEntryContractData);
    }

    contractData->contractAddrs.insert(sls->contractIds.begin(), sls->contractIds.end());
    contractData->runningTimes = sls->runningTimes;
    contractData->deltaDataLen = sls->deltaDataLen;
    nSizeWithDescendants = GetTxSize();
    nSizeWithAncestors = nSizeWithDescendants;
}

size_t MCTxMemPoolEntry::GetTxSize() const
{
    int factor = 1;
    if (tx->IsPregnantTx() || tx->IsBranchCreate() || tx->IsProve() || tx->IsReport())
        factor = 10;
    if (tx->IsBranchChainTransStep2())
        factor = 20;

    int32_t runningTimes = 0;
    int32_t deltaDataLen = 0;
    if (contractData != nullptr) {
        runningTimes = contractData->runningTimes;
        deltaDataLen = contractData->deltaDataLen;
    }

    return GetVirtualTransactionSize(nTxWeight, sigOpCost, runningTimes, deltaDataLen, factor);
}

// Update the given tx for any in-mempool descendants.
// Assumes that setMemPoolChildren is correct for the given tx and all
// descendants.
void MCTxMemPool::UpdateForDescendants(txiter updateIt, cacheMap &cachedDescendants, const std::map<uint256, int> &setExclude)
{
    setEntries stageEntries, setAllDescendants;
    stageEntries = GetMemPoolChildren(updateIt);

    while (!stageEntries.empty()) {
        const txiter cit = *stageEntries.begin();
        setAllDescendants.insert(cit);
        stageEntries.erase(cit);
        const setEntries &setChildren = GetMemPoolChildren(cit);
        for (const txiter childEntry : setChildren) {
            cacheMap::iterator cacheIt = cachedDescendants.find(childEntry);
            if (cacheIt != cachedDescendants.end()) {
                // We've already calculated this one, just add the entries for this set
                // but don't traverse again.
                for (const txiter cacheEntry : cacheIt->second) {
                    setAllDescendants.insert(cacheEntry);
                }
            }
            else if (!setAllDescendants.count(childEntry)) {
                // Schedule for later processing
                stageEntries.insert(childEntry);
            }
        }
    }
    // setAllDescendants now contains all in-mempool descendants of updateIt.
    // Update and add to cached descendant map
    int64_t modifySize = 0;
    MCAmount modifyFee = 0;
    int64_t modifyCount = 0;
    int index = setExclude.find(updateIt->GetTx().GetHash())->second;
    for (txiter cit : setAllDescendants) {
        bool modify = true;
        auto iter = setExclude.find(cit->GetTx().GetHash());
        if (iter != setExclude.end()) {
            if (!cit->GetTx().IsSmartContract() || iter->second > index) {
                modify = false;
            }
        }

        if (modify) {
            modifySize += cit->GetTxSize();
            modifyFee += cit->GetModifiedFee();
            modifyCount++;
            cachedDescendants[updateIt].insert(cit);
            // Update ancestor state for each descendant
            mapTx.modify(cit, update_ancestor_state(updateIt->GetTxSize(), updateIt->GetModifiedFee(), 1, updateIt->GetSigOpCost()));
        }
    }
    mapTx.modify(updateIt, update_descendant_state(modifySize, modifyFee, modifyCount));
}

// vHashesToUpdate is the set of transaction hashes from a disconnected block
// which has been re-added to the mempool.
// for each entry, look for descendants that are outside vHashesToUpdate, and
// add fee/size information for such descendants to the parent.
// for each such descendant, also update the ancestor state to include the parent.
void MCTxMemPool::UpdateTransactionsFromBlock(const std::vector<uint256> &vHashesToUpdate)
{
    LOCK(cs);
    // For each entry in vHashesToUpdate, store the set of in-mempool, but not
    // in-vHashesToUpdate transactions, so that we don't have to recalculate
    // descendants when we come across a previously seen entry.
    cacheMap mapMemPoolDescendantsToUpdate;

    // Use a set for lookups into vHashesToUpdate (these entries are already
    // accounted for in the state of their ancestors)
    std::map<uint256, int> setAlreadyIncluded;
    for (int i = 0; i < vHashesToUpdate.size(); ++i) {
        setAlreadyIncluded[vHashesToUpdate[i]] = i;
    }

    // Iterate in reverse, so that whenever we are looking at a transaction
    // we are sure that all in-mempool descendants have already been processed.
    // This maximizes the benefit of the descendant cache and guarantees that
    // setMemPoolChildren will be updated, an assumption made in
    // UpdateForDescendants.
    for (const uint256 &hash : reverse_iterate(vHashesToUpdate)) {
        // we cache the in-mempool children to avoid duplicate updates
        setEntries setChildren;
        // calculate children from mapNextTx
        txiter it = mapTx.find(hash);
        if (it == mapTx.end()) {
            continue;
        }
        auto iter = mapNextTx.lower_bound(MCOutPoint(hash, 0));
        // First calculate the children, and update setMemPoolChildren to
        // include them, and update their setMemPoolParents to include this tx.
        for (; iter != mapNextTx.end() && iter->first->hash == hash; ++iter) {
            const uint256 &childHash = iter->second->GetHash();
            txiter childIter = mapTx.find(childHash);
            assert(childIter != mapTx.end());
            // We can skip updating entries we've encountered before or that
            // are in the block (which are already accounted for).
            if (setChildren.insert(childIter).second && !setAlreadyIncluded.count(childIter->GetTx().GetHash())) {
                UpdateChild(it, childIter, true);
                UpdateParent(childIter, it, true);
            }
        }
        UpdateForDescendants(it, mapMemPoolDescendantsToUpdate, setAlreadyIncluded);
    }
}

bool MCTxMemPool::CalculateMemPoolAncestorsRecursive(const MCTxMemPoolEntry &entry, setEntries& parentHashes, setEntries &setAncestors, uint64_t limitAncestorCount, uint64_t limitAncestorSize, uint64_t limitDescendantCount, uint64_t limitDescendantSize, std::string &errString) const
{
    LOCK(cs);

    size_t totalSizeWithAncestors = entry.GetTxSize();
    while (!parentHashes.empty()) {
        txiter stageit = *parentHashes.begin();

        setAncestors.insert(stageit);
        parentHashes.erase(stageit);
        totalSizeWithAncestors += stageit->GetTxSize();

        if (stageit->GetSizeWithDescendants() + entry.GetTxSize() > limitDescendantSize) {
            errString = strprintf("exceeds descendant size limit for tx %s [limit: %u]", stageit->GetTx().GetHash().ToString(), limitDescendantSize);
            return false;
        }
        else if (stageit->GetCountWithDescendants() + 1 > limitDescendantCount) {
            errString = strprintf("too many descendants for tx %s [limit: %u]", stageit->GetTx().GetHash().ToString(), limitDescendantCount);
            return false;
        }
        else if (totalSizeWithAncestors > limitAncestorSize) {
            errString = strprintf("exceeds ancestor size limit [limit: %u]", limitAncestorSize);
            return false;
        }

        const setEntries & setMemPoolParents = GetMemPoolParents(stageit);
        for (const txiter &phash : setMemPoolParents) {
            // If this is a new ancestor, add it.
            if (setAncestors.count(phash) == 0) {
                parentHashes.insert(phash);
            }
            if (parentHashes.size() + setAncestors.size() + 1 > limitAncestorCount) {
                errString = strprintf("too many unconfirmed ancestors [limit: %u]", limitAncestorCount);
                return false;
            }
        }
    }

    return true;
}

bool MCTxMemPool::SearchForParents(const MCTxMemPoolEntry& entry, setEntries& parentHashes, std::set<MCContractID>* exclude, uint64_t limitAncestorCount, std::string& errString) const
{
    LOCK(cs);
    const MCTransaction &tx = entry.GetTx();

    // Get parents of this transaction that are in the mempool
    // GetMemPoolParents() is only valid for entries in the mempool, so we
    // iterate mapTx to find parents.
    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        txiter piter = mapTx.find(tx.vin[i].prevout.hash);
        if (piter != mapTx.end()) {
            parentHashes.insert(piter);
            if (parentHashes.size() + 1 > limitAncestorCount) {
                errString = strprintf("too many unconfirmed parents [limit: %u]", limitAncestorCount);
                return false;
            }
        }
    }

    // 获取内存池中与合约关联地址的交易
    if (entry.contractData != nullptr) {
        const uint256& txHash = tx.GetHash();
        for (const MCContractID& contractId : entry.contractData->contractAddrs) {
            if (exclude != nullptr) {
                if (exclude->count(contractId) > 0) {
                    continue;
                }
            }

            auto links = contractLinksMap.find(contractId);
            if (links == contractLinksMap.end()) {
                continue;
            }

            auto prev = links->second.end();
            for (auto liter = links->second.begin(); liter != links->second.end(); ++liter) {
                if ((*liter)->GetOrder() >= entry.GetOrder()) {
                    if ((*liter)->GetOrder() == entry.GetOrder()) {
                        assert((*liter)->GetTx().GetHash() == txHash);
                    }
                    break;
                }
                else {
                    prev = liter;
                }
            }

            if (prev != links->second.end() && entry.GetTx().GetHash() != (*prev)->GetTx().GetHash()) {
                parentHashes.insert(*prev);
                if (parentHashes.size() + 1 > limitAncestorCount) {
                    errString = strprintf("too many unconfirmed parents [limit: %u]", limitAncestorCount);
                    return false;
                }
            }
        }
    }

    //if (tx.IsSyncBranchInfo()){
    //    uint256 prebranchheadblocktxhash = g_pBranchDataMemCache->GetParent(tx);
    //    txiter piter = mapTx.find(prebranchheadblocktxhash);
    //    if (piter != mapTx.end()) {
    //        parentHashes.insert(piter);
    //        if (parentHashes.size() + 1 > limitAncestorCount) {
    //            errString = strprintf("too many unconfirmed parents 3 [limit: %u]", limitAncestorCount);
    //            return false;
    //        }
    //    }
    //}

    return true;
}

bool MCTxMemPool::CalculateMemPoolAncestors(const MCTxMemPoolEntry& entry, std::set<MCContractID>* exclude, setEntries& setAncestors, uint64_t limitAncestorCount, uint64_t limitAncestorSize, uint64_t limitDescendantCount, uint64_t limitDescendantSize, std::string& errString, bool fSearchForParents /* = true */) const
{
    LOCK(cs);

    setEntries parentHashes;

    if (fSearchForParents) {
        if (!SearchForParents(entry, parentHashes, exclude, limitAncestorCount, errString))
            return false;
    }
    else {
        // If we're not searching for parents, we require this to be an
        // entry in the mempool already.
        txiter it = mapTx.iterator_to(entry);
        parentHashes = GetMemPoolParents(it);
    }

    return CalculateMemPoolAncestorsRecursive(entry, parentHashes, setAncestors, limitAncestorCount, limitAncestorSize, limitDescendantCount, limitDescendantSize, errString);
}

void MCTxMemPool::UpdateAncestorsOf(bool add, txiter it, setEntries &setAncestors)
{
    setEntries parentIters = GetMemPoolParents(it);
    // add or remove this tx as a child of each parent
    for (txiter piter : parentIters) {
        UpdateChild(piter, it, add);
    }
    const int64_t updateCount = (add ? 1 : -1);
    const int64_t updateSize = updateCount * it->GetTxSize();
    const MCAmount updateFee = updateCount * it->GetModifiedFee();
    for (txiter ancestorIt : setAncestors) {
        mapTx.modify(ancestorIt, update_descendant_state(updateSize, updateFee, updateCount));
    }
}

void MCTxMemPool::UpdateEntryForAncestors(txiter it, const setEntries &setAncestors)
{
    int64_t updateCount = setAncestors.size();
    int64_t updateSize = 0;
    MCAmount updateFee = 0;
    int64_t updateSigOpsCost = 0;
    for (txiter ancestorIt : setAncestors) {
        updateSize += ancestorIt->GetTxSize();
        updateFee += ancestorIt->GetModifiedFee();
        updateSigOpsCost += ancestorIt->GetSigOpCost();
    }
    mapTx.modify(it, update_ancestor_state(updateSize, updateFee, updateCount, updateSigOpsCost));
}

void MCTxMemPool::UpdateChildrenForRemoval(txiter it)
{
    const setEntries &setMemPoolChildren = GetMemPoolChildren(it);
    for (txiter updateIt : setMemPoolChildren) {
        UpdateParent(updateIt, it, false);
    }
}

void MCTxMemPool::UpdateForRemoveFromMempool(const setEntries &entriesToRemove, bool updateDescendants)
{
    // For each entry, walk back all ancestors and decrement size associated with this
    // transaction
    const uint64_t nNoLimit = std::numeric_limits<uint64_t>::max();
    for (txiter removeIt : entriesToRemove) {
        setEntries setAncestors;
        const MCTxMemPoolEntry &entry = *removeIt;
        std::string dummy;
        // Since this is a tx that is already in the mempool, we can call CMPA
        // with fSearchForParents = false.  If the mempool is in a consistent
        // state, then using true or false should both be correct, though false
        // should be a bit faster.
        // However, if we happen to be in the middle of processing a reorg, then
        // the mempool can be in an inconsistent state.  In this case, the set
        // of ancestors reachable via mapLinks will be the same as the set of 
        // ancestors whose packages include this transaction, because when we
        // add a new transaction to the mempool in AddUnchecked(), we assume it
        // has no children, and in the case of a reorg where that assumption is
        // false, the in-mempool children aren't linked to the in-block tx's
        // until UpdateTransactionsFromBlock() is called.
        // So if we're being called during a reorg, ie before
        // UpdateTransactionsFromBlock() has been called, then mapLinks[] will
        // differ from the set of mempool parents we'd calculate by searching,
        // and it's important that we use the mapLinks[] notion of ancestor
        // transactions as the set of things to update for removal.
        CalculateMemPoolAncestors(entry, nullptr, setAncestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, false);
        // Note that UpdateAncestorsOf severs the child links that point to
        // removeIt in the entries for the parents of removeIt.
        UpdateAncestorsOf(false, removeIt, setAncestors);

        // updateDescendants should be true whenever we're not recursively
        // removing a tx and all its descendants, eg when a transaction is
        // confirmed in a block.
        // Here we only update statistics and not data in mapLinks (which
        // we need to preserve until we're finished with all operations that
        // need to traverse the mempool).
        setEntries setDescendants;
        if (updateDescendants) {
            CalculateDescendants(removeIt, setDescendants, true);
            setDescendants.erase(removeIt); // don't update state for self
            int64_t modifySize = -((int64_t)removeIt->GetTxSize());
            MCAmount modifyFee = -removeIt->GetModifiedFee();
            int modifySigOps = -removeIt->GetSigOpCost();
            for (txiter dit : setDescendants) {
                mapTx.modify(dit, update_ancestor_state(modifySize, modifyFee, -1, modifySigOps));
            }
        }

        // After updating all the ancestor sizes, we can now sever the link between each
        // transaction being removed and any mempool children (ie, update setMemPoolParents
        // for each direct child of a transaction being removed).
        UpdateChildrenForRemoval(removeIt);
        RemoveFromContractLinks(removeIt);

        for (auto piter : setAncestors) {
            setEntries setDescendants2;
            CalculateDescendants(piter, setDescendants2, true);

            for (auto citer : setDescendants) {
                if (!setDescendants2.count(citer)) {
                    mapTx.modify(citer, update_ancestor_state(-(int64_t)piter->GetTxSize(), -piter->GetModifiedFee(), -1, -piter->GetSigOpCost()));
                    mapTx.modify(piter, update_descendant_state(-(int64_t)citer->GetTxSize(), -citer->GetModifiedFee(), -1));
                }
            }
        }
    }
}

void MCTxMemPoolEntry::UpdateDescendantState(int64_t modifySize, MCAmount modifyFee, int64_t modifyCount)
{
    nSizeWithDescendants += modifySize;
    assert(int64_t(nSizeWithDescendants) > 0);
    nModFeesWithDescendants += modifyFee;
    nCountWithDescendants += modifyCount;
    assert(int64_t(nCountWithDescendants) > 0);
}

void MCTxMemPoolEntry::UpdateAncestorState(int64_t modifySize, MCAmount modifyFee, int64_t modifyCount, int modifySigOps)
{
    nSizeWithAncestors += modifySize;
    assert(int64_t(nSizeWithAncestors) > 0);
    nModFeesWithAncestors += modifyFee;
    nCountWithAncestors += modifyCount;
    assert(int64_t(nCountWithAncestors) > 0);
    nSigOpCostWithAncestors += modifySigOps;
    assert(int(nSigOpCostWithAncestors) >= 0);
}


MCTxMemPool::MCTxMemPool(MCBlockPolicyEstimator* estimator) :
    nTransactionsUpdated(0), minerPolicyEstimator(estimator), nCreateBranchTxCount(0), nextEntryOrder(1)
{
    DoClear(); //lock free clear

   // Sanity checks off by default for performance, because otherwise
   // accepting transactions becomes O(N^2) where N is the number
   // of transactions in the pool
    nCheckFrequency = 0;
}

bool MCTxMemPool::IsSpent(const MCOutPoint& outpoint) const
{
    LOCK(cs);
    return mapNextTx.count(outpoint);
}

unsigned int MCTxMemPool::GetTransactionsUpdated() const
{
    LOCK(cs);
    return nTransactionsUpdated;
}

void MCTxMemPool::AddTransactionsUpdated(unsigned int n)
{
    LOCK(cs);
    nTransactionsUpdated += n;
}

bool MCTxMemPool::AddUnchecked(const uint256& hash, const MCTxMemPoolEntry &entry, setEntries &setAncestors, bool validFeeEstimate)
{
    NotifyEntryAdded(entry.GetSharedTx());
    // Add to memory pool without checking anything.
    // Used by AcceptToMemoryPool(), which DOES do
    // all the appropriate checks.
    LOCK(cs);
    indexed_transaction_set::iterator newit = mapTx.insert(entry).first;
    mapLinks.insert(make_pair(newit, TxLinks()));

    // Update transaction for any feeDelta created by PrioritiseTransaction
    // TODO: refactor so that the fee delta is calculated before inserting
    // into mapTx.
    std::map<uint256, MCAmount>::const_iterator pos = mapDeltas.find(hash);
    if (pos != mapDeltas.end()) {
        const MCAmount &delta = pos->second;
        if (delta) {
            mapTx.modify(newit, update_fee_delta(delta));
        }
    }

    // Update cachedInnerUsage to include contained transaction's usage.
    // (When we update the entry for in-mempool parents, memory usage will be
    // further updated.)
    cachedInnerUsage += entry.DynamicMemoryUsage();

    const MCTransaction& tx = newit->GetTx();
    std::set<uint256> setParentTransactions;
    if (!tx.IsBranchChainTransStep2()){ 
        for (unsigned int i = 0; i < tx.vin.size(); i++) {
            mapNextTx.insert(std::make_pair(&tx.vin[i].prevout, &tx));
            setParentTransactions.insert(tx.vin[i].prevout.hash);
        }
        //if (tx.IsSyncBranchInfo()){
        //    uint256 prebranchheadblocktxhash = g_pBranchDataMemCache->GetParent(tx);
        //    // no value insert to mapNextTx
        //    if (!prebranchheadblocktxhash.IsNull()){
        //        setParentTransactions.insert(prebranchheadblocktxhash);
        //    }
        //}
    }
    // 设置内存池中与合约关联地址的交易
    if (entry.contractData != nullptr) {
        for (const MCContractID& contractId : entry.contractData->contractAddrs) {
            auto& links = contractLinksMap[contractId];
            auto next = links.end();
            for (auto liter = links.begin(); liter != links.end(); ++liter) {
                if ((*liter)->GetOrder() >= entry.GetOrder()) {
                    next = liter;
                    assert((*liter)->GetOrder() != entry.GetOrder());
                    break;
                }
                else {
                    for (int i = 0; i < (*liter)->GetTx().vin.size(); ++i) {
                        const uint256& lhash = (*liter)->GetTx().vin[i].prevout.hash;
                        txiter piter = mapTx.find(lhash);
                        if (piter != mapTx.end()) {
                            txlinksMap::const_iterator it = mapLinks.find(*liter);
                            if (!it->second.parents.count(piter)) {;
                                setAncestors.insert(piter);
                            }
                        }
                    }
                }
            }

            auto prev = next;
            if (prev != links.begin()) {
                prev--;
            }
            else {
                prev = links.end();
            }

            if (prev != links.end()) {
                setParentTransactions.insert((*prev)->GetTx().GetHash());

                if (next != links.end()) {
                    bool remove = true;
                    for (int i = 0; i < (*next)->GetTx().vin.size(); ++i) {
                        if ((*next)->GetTx().vin[i].prevout.hash == (*prev)->GetTx().GetHash()) {
                            remove = false;
                            break;
                        }
                    }
                    if (remove) {
                        for (const MCContractID nextCntractId : (*next)->contractData->contractAddrs) {
                            if (nextCntractId == contractId) {
                                continue;
                            }
                            auto& nextLinks = contractLinksMap[nextCntractId];
                            auto nextPrev = nextLinks.begin();
                            for (auto nextliter = nextLinks.begin(); nextliter != nextLinks.end(); ++nextliter) {
                                if ((*nextliter)->GetOrder() >= (*next)->GetOrder()) {
                                    break;
                                }
                                else {
                                    nextPrev = nextliter;
                                }
                            }
                            if (*nextPrev == *prev) {
                                remove = false;
                            }
                        }
                    }
                    if (remove) {
                        UpdateChild(*prev, *next, false);
                        UpdateParent(*next, *prev, false);
                    }
                }
            }
            auto target = links.insert(next, newit);
            next = target;
            next++;
            if (next != links.end()) {
                UpdateChild(*target, *next, true);
                UpdateParent(*next, *target, true);
            }
        }
    }
    // Don't bother worrying about child transactions of this one.
    // Normal case of a new transaction arriving is that there can't be any
    // children, because such children would be orphans.
    // An exception to that is if a transaction enters that used to be in a block.
    // In that case, our disconnect block logic will call UpdateTransactionsFromBlock
    // to clean up the mess we're leaving here.

    // Update ancestors with information about this tx
    for (const uint256 &phash : setParentTransactions) {
        txiter pit = mapTx.find(phash);
        if (pit != mapTx.end()) {
            UpdateParent(newit, pit, true);
        }
    }

    UpdateAncestorsOf(true, newit, setAncestors);
    UpdateEntryForAncestors(newit, setAncestors);

    // update contract amount
    if (entry.GetTx().pContractData != nullptr && entry.GetTx().pContractData->contractCoinsOut.size() > 0) {
        for (auto it : entry.GetTx().pContractData->contractCoinsOut) {
            pCoinAmountCache->DecAmount(it.first, it.second);
        }
    }

    const std::vector<MCTxOut>& vout = entry.GetTx().vout;
    for (int i = 0; i < vout.size(); ++i) {
        const MCScript& scriptPubKey = vout[i].scriptPubKey;
        if (scriptPubKey.IsContract()) {
            opcodetype opcode;
            std::vector<unsigned char> vch;
            MCScript::const_iterator pc = scriptPubKey.begin();
            MCScript::const_iterator end = scriptPubKey.end();
            scriptPubKey.GetOp(pc, opcode, vch);

            assert(opcode == OP_CONTRACT || opcode == OP_CONTRACT_CHANGE);
            vch.clear();
            vch.assign(pc + 1, end);
            uint160 key = uint160(vch);
            MCContractID contractId = MCContractID(key);
            pCoinAmountCache->IncAmount(contractId, vout[i].nValue);
        }
    }

    nTransactionsUpdated++;
    totalTxSize += entry.GetTxSize();
    if (minerPolicyEstimator) {
        minerPolicyEstimator->ProcessTransaction(entry, validFeeEstimate);
    }

    vTxHashes.emplace_back(tx.GetWitnessHash(), newit);
    newit->vTxHashesIdx = vTxHashes.size() - 1;

    if (tx.IsBranchCreate()) {
        nCreateBranchTxCount++;
    }
    return true;
}

void MCTxMemPool::RemoveFromContractLinks(txiter it)
{
    if (it->contractData == nullptr)
        return;

    for (const MCContractID& contractId : it->contractData->contractAddrs) {
        auto linkIter = contractLinksMap.find(contractId);
        assert(linkIter != contractLinksMap.end());
        auto& links = linkIter->second;
        for (auto liter = links.begin(); liter != links.end(); ++liter) {
            if (*liter != it) {
                continue;
            }

            auto citer = links.erase(liter);
            if (citer == links.end()) {
                break;
            }

            auto piter = links.end();
            if (citer != links.begin()) {
                piter = citer;
                --piter;
            }
            if (piter == links.end()) {
                break;
            }

            if (mapLinks[*piter].children.count(*citer) == 0) {
                assert(mapLinks[*citer].parents.count(*piter) == 0);
                UpdateParent(*citer, *piter, true);
                UpdateChild(*piter, *citer, true);
            }
            break;
        }

        if (links.size() == 0) {
            contractLinksMap.erase(contractId);
        }
    }
}

void MCTxMemPool::RemoveUnchecked(txiter it, MemPoolRemovalReason reason)
{
    NotifyEntryRemoved(it->GetSharedTx(), reason);
    const uint256 hash = it->GetTx().GetHash();
    for (const MCTxIn& txin : it->GetTx().vin)
        mapNextTx.erase(txin.prevout);

    if (vTxHashes.size() > 1) {
        vTxHashes[it->vTxHashesIdx] = std::move(vTxHashes.back());
        vTxHashes[it->vTxHashesIdx].second->vTxHashesIdx = it->vTxHashesIdx;
        vTxHashes.pop_back();
        if (vTxHashes.size() * 2 < vTxHashes.capacity())
            vTxHashes.shrink_to_fit();
    }
    else {
        vTxHashes.clear();
    }

    totalTxSize -= it->GetTxSize();
    cachedInnerUsage -= it->DynamicMemoryUsage();
    cachedInnerUsage -= memusage::DynamicUsage(mapLinks[it].parents) + memusage::DynamicUsage(mapLinks[it].children);

    if (it->GetTx().IsBranchCreate()) {
        nCreateBranchTxCount--;
    }

    // update contract amount
    const std::vector<MCTxOut>& vout = it->GetTx().vout;
    for (int i = 0; i < vout.size(); ++i) {
        const MCScript& scriptPubKey = vout[i].scriptPubKey;
        if (scriptPubKey.IsContract()) {
            opcodetype opcode;
            std::vector<unsigned char> vch;
            MCScript::const_iterator pc = scriptPubKey.begin();
            MCScript::const_iterator end = scriptPubKey.end();
            scriptPubKey.GetOp(pc, opcode, vch);

            if (opcode == OP_CONTRACT || opcode == OP_CONTRACT_CHANGE) {
                vch.clear();
                vch.assign(pc + 1, end);
                uint160 key = uint160(vch);
                MCContractID contractId = MCContractID(key);
                pCoinAmountCache->DecAmount(contractId, vout[i].nValue);
            }
        }
    }

    mapLinks.erase(it);
    mapTx.erase(it);
    nTransactionsUpdated++;
    if (minerPolicyEstimator) {
        minerPolicyEstimator->RemoveTx(hash, false);
    }
}

void MCTxMemPool::CheckContract(txiter titer, SmartLuaState* sls)
{
    assert(titer->contractData != nullptr);
    const uint256& txHash = titer->GetTx().GetHash();

    int64_t oldSize = titer->GetTxSize();
    int64_t oldModifiedFee = titer->GetModifiedFee();
    int64_t oldSigOpsCost = titer->GetSigOpCost();

    bool resize = false;
    if (sls->runningTimes != titer->contractData->runningTimes || sls->deltaDataLen != titer->contractData->deltaDataLen) {
        titer->contractData->runningTimes = sls->runningTimes;
        titer->contractData->deltaDataLen = sls->deltaDataLen;
        resize = true;
    }

    int64_t newSize = titer->GetTxSize();
    int64_t newModifiedFee = titer->GetModifiedFee();
    int64_t newSigOpsCost = titer->GetSigOpCost();

    std::string dummy;
    setEntries ancestors, descendants;
    setEntries otherAncestors, otherDescendants;
    setEntries tempAncestors, tempDescendants;
    uint64_t nNoLimit = std::numeric_limits<uint64_t>::max();
    std::set<MCContractID> exclude;

    // 重新计算旧的有依赖关系的相关父与子交易大小
    if (resize || newSize != oldSize || newModifiedFee != oldModifiedFee || newSigOpsCost != oldSigOpsCost)
    {
        int64_t deltaSize = newSize - oldSize;
        int64_t deltaModifiedFee = newModifiedFee - oldModifiedFee;
        int64_t deltaSigOpsCost = newSigOpsCost - oldSigOpsCost;

        ancestors.clear();
        CalculateMemPoolAncestors(*titer, nullptr, ancestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true);
        ancestors.insert(titer);
        for (const txiter& iter : ancestors) {
            mapTx.modify(iter, update_descendant_state(deltaSize, deltaModifiedFee, 0));
        }

        descendants.clear();
        CalculateDescendants(titer, descendants, true);
        for (const txiter& iter : descendants) {
            mapTx.modify(iter, update_ancestor_state(deltaSize, deltaModifiedFee, 0, deltaSigOpsCost));
        }

        totalTxSize += deltaSize;
    }

    // 移除旧的不再有依赖关系的合约依赖项
    bool contractChanged = (sls->contractIds != titer->contractData->contractAddrs);
    if (contractChanged) {
        // 移除已不存在的合约依赖
        for (const MCContractID& contractId : titer->contractData->contractAddrs) {
            if (sls->contractIds.count(contractId) > 0) {
                continue;
            }

            exclude.clear();
            exclude.insert(contractId);

            auto& links = contractLinksMap[contractId];
            for (auto liter = links.begin(); liter != links.end(); ++liter) {
                if (*liter != titer) {
                    continue;
                }

                std::list<txiter>::iterator piter = links.end();
                // 更新合约依赖链的父交易
                if (liter != links.begin()) {
                    piter = liter;
                    --piter;

                    // 如果存在合约依赖的同时还存在输入依赖，则不移除
                    ancestors.clear();
                    otherAncestors.clear();
                    assert(CalculateMemPoolAncestors(*titer, &exclude, ancestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true));
                    if (ancestors.count(*piter) == 0) {
                        CalculateMemPoolAncestors(**piter, &exclude, otherAncestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true);
                        otherAncestors.insert(*piter);
                    }

                    if (otherAncestors.size() > 0) {
                        // 断开合约依赖
                        UpdateChild(*piter, *liter, false);
                        UpdateParent(*liter, *piter, false);

                        for (const txiter aiter : ancestors) {
                            otherAncestors.erase(aiter);
                        }
                    }
                }

                // 更新合约依赖链的子交易
                std::list<txiter>::iterator citer = links.end();
                if (liter != links.end()) {
                    citer = liter;
                    ++citer;

                    // 更新链的下一个子交易，确定是否还存在非该合约的依赖
                    if (citer != links.end()) {
                        ancestors.clear();
                        otherDescendants.clear();
                        assert(CalculateMemPoolAncestors(**citer, &exclude, ancestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true));
                        if (ancestors.count(titer) == 0) {
                            CalculateDescendants(*citer, otherDescendants, true);
                        }
                    }

                    if (otherDescendants.size() > 0) {
                        // 先断开合约的依赖
                        UpdateParent(*citer, *liter, false);
                        UpdateChild(*liter, *citer, false);

                        // 重新计算断开依赖后的子交易
                        descendants.clear();
                        CalculateDescendants(titer, descendants, true);

                        // 从setUpdate移除myDescendants拥有的交易
                        for (const txiter& diter : descendants) {
                            otherDescendants.erase(diter);
                        }
                    }
                }

                // 连接新的父与子交易
                if (piter != links.end() && citer != links.end()) {
                    UpdateParent(*citer, *piter, true);
                    UpdateChild(*piter, *citer, true);
                }

                citer = links.erase(liter);
                if (links.size() == 0) {
                    contractLinksMap.erase(contractId);
                }
                else {
                    // 更新父交易
                    descendants.clear();
                    CalculateDescendants(titer, descendants, true);
                    for (const txiter aiter : otherAncestors) {
                        tempDescendants.clear();
                        CalculateDescendants(aiter, tempDescendants, true);
                        for (const txiter diter : descendants) {
                            if (tempDescendants.count(diter) == 0) {
                                mapTx.modify(aiter, update_descendant_state(-(int64_t)diter->GetTxSize(), -diter->GetModifiedFee(), -1));
                                mapTx.modify(diter, update_ancestor_state(-(int64_t)aiter->GetTxSize(), -aiter->GetModifiedFee(), -1, -aiter->GetSigOpCost()));
                            }
                        }
                    }

                    // 更新子交易
                    ancestors.clear();
                    CalculateMemPoolAncestors(*titer, nullptr, ancestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true);
                    ancestors.insert(titer);
                    for (const txiter diter : otherDescendants) {
                        tempAncestors.clear();
                        CalculateMemPoolAncestors(*diter, nullptr, tempAncestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true);
                        for (const txiter aiter : ancestors) {
                            if (tempAncestors.count(aiter) == 0) {
                                mapTx.modify(diter, update_ancestor_state(-(int64_t)aiter->GetTxSize(), -aiter->GetModifiedFee(), -1, -aiter->GetSigOpCost()));
                                mapTx.modify(aiter, update_descendant_state(-(int64_t)diter->GetTxSize(), -diter->GetModifiedFee(), -1));
                            }
                        }
                    }
                }

                break;
            }
        }
    }

    // 插入新的合约依赖项
    if (sls->contractIds != titer->contractData->contractAddrs) {
        for (const MCContractID& contractId : sls->contractIds) {
            if (titer->contractData->contractAddrs.count(contractId) > 0) {
                continue;
            }

            exclude.clear();
            exclude.insert(contractId);

            auto& links = contractLinksMap[contractId];
            auto citer = links.end();
            for (auto liter = links.begin(); liter != links.end(); ++liter) {
                if ((*liter)->GetOrder() >= titer->GetOrder()) {
                    assert((*liter)->GetOrder() != titer->GetOrder());
                    citer = liter;
                    break;
                }
            }

            std::list<txiter>::iterator piter = links.end();
            if (citer != links.begin()) {
                piter = citer;
                --piter;
            }

            // 处于链的中间位置
            if (citer != links.end() && piter != links.end()) {
                otherAncestors.clear();
                CalculateMemPoolAncestors(**citer, &exclude, otherAncestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true);
                if (otherAncestors.count(*piter) == 0) {
                    // 如果没有其它相关的父子关系则断开直接依赖
                    UpdateChild(*piter, *citer, false);
                    UpdateParent(*citer, *piter, false);
                }
            }

            // 连接父交易
            if (piter != links.end()) {
                if (ancestors.count(*piter) == 0) {
                    otherAncestors.clear();
                    CalculateMemPoolAncestors(**piter, nullptr, otherAncestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true);
                    otherAncestors.insert(*piter);
                    for (txiter aiter : otherAncestors) {
                        otherDescendants.clear();
                        CalculateDescendants(aiter, otherDescendants, true);
                        for (const txiter diter : descendants) {
                            if (otherDescendants.count(diter) == 0) {
                                mapTx.modify(aiter, update_descendant_state((int64_t)diter->GetTxSize(), diter->GetModifiedFee(), 1));
                                mapTx.modify(diter, update_ancestor_state((int64_t)aiter->GetTxSize(), aiter->GetModifiedFee(), 1, aiter->GetSigOpCost()));
                            }
                        }
                    }
                }

                UpdateChild(*piter, titer, true);
                UpdateParent(titer, *piter, true);
            }

            if (citer != links.end()) {
                if (descendants.count(*citer) == 0) {
                    otherDescendants.clear();
                    CalculateDescendants(*citer, otherDescendants, true);
                    for (txiter diter : otherDescendants) {
                        otherAncestors.clear();
                        CalculateMemPoolAncestors(*diter, nullptr, otherAncestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true);
                        for (const txiter aiter : ancestors) {
                            if (otherDescendants.count(aiter) == 0) {
                                mapTx.modify(diter, update_ancestor_state((int64_t)aiter->GetTxSize(), aiter->GetModifiedFee(), 1, aiter->GetSigOpCost()));
                                mapTx.modify(aiter, update_descendant_state((int64_t)diter->GetTxSize(), diter->GetModifiedFee(), 1));
                            }
                        }
                    }
                }

                UpdateChild(titer, *citer, true);
                UpdateParent(*citer, titer, true);
            }

            ancestors.clear();
            CalculateMemPoolAncestors(*titer, nullptr, ancestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, true);
            descendants.clear();
            CalculateDescendants(titer, descendants, true);

            links.insert(citer, titer);
        }
    }

    if (contractChanged) {
        titer->contractData->contractAddrs = std::move(sls->contractIds);
    }
}


//Revert Transaction for follow situations:
// 1. Step2 transaction duplicate check, it should exclude pPMT data field to get the hash identify.
// 2. Re-add to mempool or remove from mempool should keep the pPMT data field.
MCMutableTransaction RevertTransaction(const MCTransaction& tx, const MCTransactionRef &pFromTx, bool fFromMempool)
{
    MCTransactionRef pfromtx = pFromTx;
    MCMutableTransaction mtx(tx);
 
    if (tx.IsBranchChainTransStep2() && tx.fromBranchId != MCBaseChainParams::MAIN) {
        //recover tx: remove UTXO
        //vin like func MakeBranchTransStep2Tx
        mtx.vin.clear();
        mtx.vin.resize(1);
        mtx.vin[0].prevout.hash.SetNull();
        mtx.vin[0].prevout.n = 0;
        mtx.vin[0].scriptSig.clear();
        //remove vout branch recharge
        for (int i = mtx.vout.size() - 1; i >= 0; i--) {
            const MCScript& scriptPubKey = mtx.vout[i].scriptPubKey;
            if (IsCoinBranchTranScript(scriptPubKey)) {
                mtx.vout.erase(mtx.vout.begin() + i);
            }
        }
        if(!fFromMempool){
            mtx.pPMT.reset(new MCSpvProof());// transaction may be mined in diff blocks, so remove spv info
        }
    }
    else if (tx.IsSmartContract()) {
        for (int i = mtx.vin.size() - 1; i >= 0; i--) {
            if (mtx.vin[i].scriptSig.IsContract()) {
                mtx.vin.erase(mtx.vin.begin() + i);
            }
        }
        for (int i = mtx.vout.size() - 1; i >= 0; i--) {
            const MCScript& scriptPubKey = mtx.vout[i].scriptPubKey;
            if (scriptPubKey.IsContractChange()) {
                mtx.vout.erase(mtx.vout.begin() + i);
            }
        }
    }

    return mtx;
}

// note that.
// IsBranchChainTransStep2 transaction will have different txid identify by fFromMempool.
//
uint256 MCTxMemPool::GetOriTxHash(const MCTransaction& tx, bool fFromMempool)
{
    uint256 txHash = tx.GetHash();
    if ((tx.IsBranchChainTransStep2() && tx.fromBranchId != MCBaseChainParams::MAIN && fFromMempool) ||
        (tx.IsSmartContract() && tx.pContractData->contractCoinsOut.size() > 0)) {
        auto it = mapFinalTx2OriTx.find(txHash);
        if (it == mapFinalTx2OriTx.end()) {
            uint256 oriTxHash = RevertTransaction(tx, nullptr, fFromMempool).GetHash();
            mapFinalTx2OriTx[txHash] = oriTxHash;
            return oriTxHash;
        }
        return it->second;
    }
    if (tx.IsBranchChainTransStep2() && tx.fromBranchId != MCBaseChainParams::MAIN && !fFromMempool){
        auto it = mapFinalTx2OriTx2.find(txHash);
        if (it == mapFinalTx2OriTx2.end()) {
            uint256 oriTxHash = RevertTransaction(tx, nullptr, fFromMempool).GetHash();
            mapFinalTx2OriTx2[txHash] = oriTxHash;
            return oriTxHash;
        }
        return it->second;
    }
    return tx.GetHash();
}

// Calculates descendants of entry that are not already in setDescendants, and adds to
// setDescendants. Assumes entryit is already a tx in the mempool and setMemPoolChildren
// is correct for tx and all descendants.
// Also assumes that if an entry is in setDescendants already, then all
// in-mempool descendants of it are already in setDescendants as well, so that we
// can save time by not iterating over those entries.
void MCTxMemPool::CalculateDescendants(txiter entryit, setEntries &setDescendants, bool allContractDependencies)
{
    setEntries stage;
    if (setDescendants.count(entryit) == 0) {
        stage.insert(entryit);
    }

    // Traverse down the children of entry, only adding children that are not
    // accounted for in setDescendants already (because those children have either
    // already been walked, or will be walked in this iteration).
    while (!stage.empty()) {
        txiter it = *stage.begin();
        setDescendants.insert(it);
        stage.erase(it);

        const uint256& txHash = it->GetSharedTx()->GetHash();
        const setEntries &setChildren = GetMemPoolChildren(it);
        for (const txiter &citer : setChildren) {
            if (!setDescendants.count(citer)) {
                bool add = allContractDependencies;
                if (!add) {
                    // 仅有合约依赖的不加入
                    if (citer->contractData != nullptr) {
                        MCTransactionRef ptx = citer->GetSharedTx();
                        for (int i = 0; i < ptx->vin.size(); ++i) {
                            if (ptx->vin[i].prevout.hash == txHash) {
                                add = true;
                                break;
                            }
                        }
                    }
                    else
                        add = true;
                }
                if (add) {
                    stage.insert(citer);
                }
            }
        }
    }
}

void MCTxMemPool::RemoveRecursive(const MCTransaction &origTx, MemPoolRemovalReason reason)
{
    // Remove transaction from memory pool
    {
        LOCK(cs);
        setEntries txToRemove;
        txiter origit = mapTx.find(origTx.GetHash());
        if (origit != mapTx.end()) {
            txToRemove.insert(origit);
        }
        else {
            // When recursively removing but origTx isn't in the mempool
            // be sure to remove any children that are in the pool. This can
            // happen during chain re-orgs if origTx isn't re-accepted into
            // the mempool for any reason.
            for (unsigned int i = 0; i < origTx.vout.size(); i++) {
                auto it = mapNextTx.find(MCOutPoint(origTx.GetHash(), i));
                if (it == mapNextTx.end())
                    continue;
                txiter nextit = mapTx.find(it->second->GetHash());
                assert(nextit != mapTx.end());
                txToRemove.insert(nextit);
            }
        }
        setEntries setAllRemoves;
        for (txiter it : txToRemove) {
            CalculateDescendants(it, setAllRemoves, false);
        }

        RemoveStaged(setAllRemoves, true, reason);
    }
}

void MCTxMemPool::RemoveForReorg(const MCCoinsViewCache *pcoins, unsigned int nMemPoolHeight, int flags)
{
    // Remove transactions spending a coinbase which are now immature and no-longer-final transactions
    LOCK(cs);
    setEntries txToRemove;
    for (indexed_transaction_set::const_iterator it = mapTx.begin(); it != mapTx.end(); it++) {
        const MCTransaction& tx = it->GetTx();
        LockPoints lp = it->GetLockPoints();
        bool validLP = TestLockPointValidity(&lp);
        if (!CheckFinalTx(tx, flags) || !CheckSequenceLocks(tx, flags, &lp, validLP)) {
            // Note if CheckSequenceLocks fails the LockPoints may still be invalid
            // So it's critical that we remove the tx and not depend on the LockPoints.
            txToRemove.insert(it);
        }
        else if (it->GetSpendsCoinbase()) {
            for (const MCTxIn& txin : tx.vin) {
                indexed_transaction_set::const_iterator it2 = mapTx.find(txin.prevout.hash);
                if (it2 != mapTx.end())
                    continue;
                const Coin &coin = pcoins->AccessCoin(txin.prevout);
                if (nCheckFrequency != 0) assert(!coin.IsSpent());
                if (coin.IsSpent() || (coin.IsCoinBase() && ((signed long)nMemPoolHeight) - coin.nHeight < COINBASE_MATURITY)) {
                    txToRemove.insert(it);
                    break;
                }
            }
        }
        if (!validLP) {
            mapTx.modify(it, update_lock_points(lp));
        }
    }
    setEntries setAllRemoves;
    for (txiter it : txToRemove) {
        CalculateDescendants(it, setAllRemoves, false);
    }
    RemoveStaged(setAllRemoves, true, MemPoolRemovalReason::REORG);
}

void MCTxMemPool::RemoveConflicts(const MCTransaction &tx)
{
    // Remove transactions which depend on inputs of tx, recursively
    LOCK(cs);
    for (const MCTxIn &txin : tx.vin) {
        auto it = mapNextTx.find(txin.prevout);
        if (it != mapNextTx.end()) {
            const MCTransaction &txConflict = *it->second;
            if (txConflict != tx)
            {
                ClearPrioritisation(txConflict.GetHash());
                RemoveRecursive(txConflict, MemPoolRemovalReason::CONFLICT);
            }
        }
    }
}

void MCTxMemPool::RemoveForVector(const std::vector<MCTransactionRef>& vtx, bool fFromMemPool)
{
    for (const auto& tx : vtx)
    {
        uint256 txid = GetOriTxHash(*tx, fFromMemPool);// remove Transaction that has be modified, as IsBranchChainTransStep2
        txiter it = mapTx.find(txid);
        if (it != mapTx.end()) {
            setEntries stage;
            stage.insert(it);
            RemoveStaged(stage, true, MemPoolRemovalReason::BLOCK);
        }
        RemoveConflicts(*tx);
        ClearPrioritisation(txid);
    }
}
/**
* Called when a block is connected. Removes from mempool and updates the miner fee estimator.
*/
void MCTxMemPool::RemoveForBlock(const std::vector<MCTransactionRef>& vtx, unsigned int nBlockHeight)
{
    LOCK(cs);
    std::vector<const MCTxMemPoolEntry*> entries;
    // remove all duplicate branch block header info tx.
    std::set<uint256> setHeaderMixHash;// collect be mined branch block hash
    bool bIsMainChain = Params().IsMainChain();

    for (const auto& tx : vtx)
    {
        uint256 hash = GetOriTxHash(*tx, true);

        indexed_transaction_set::iterator i = mapTx.find(hash);
        if (i != mapTx.end())
            entries.push_back(&*i);
        if (bIsMainChain && tx->IsSyncBranchInfo()){//record
            BranchBlockData blockData;
            blockData.InitDataFromTx(*tx);
            const uint256 blockHash = blockData.header.GetHash();
            const uint256& branchHash = tx->pBranchBlockData->branchID;
            uint256 mixhash = blockHash | branchHash;
            setHeaderMixHash.insert(mixhash);
        }
    }

    // Before the txs in the new block have been removed from the mempool, update policy estimates
    if (minerPolicyEstimator) { minerPolicyEstimator->ProcessBlock(nBlockHeight, entries); }
    RemoveForVector(vtx, true);

    // remove all duplicate branch block header info tx.
    if (bIsMainChain && !setHeaderMixHash.empty())
    {
        std::vector<MCTransactionRef> vRemove;
        for (const auto& it : mapTx) {
            const MCTransactionRef& ptx = it.GetPtrTx();
            if (ptx->IsSyncBranchInfo()){
                BranchBlockData blockData;
                blockData.InitDataFromTx(*ptx);
                const uint256 blockHash = blockData.header.GetHash();
                const uint256& branchHash = ptx->pBranchBlockData->branchID;
                uint256 mixhash = blockHash | branchHash;
                if (setHeaderMixHash.count(mixhash)) {
                    vRemove.push_back(ptx);

                    uint256 txid = ptx->GetHash();
                    LogPrintf("remove duplicate syncbranchheaderinfo tx from mempool %s\n", txid.GetHex());
                }
            }
        }
        RemoveForVector(vRemove, true);
    }

    lastRollingFeeUpdate = GetTime();
    blockSinceLastRollingFeeBump = true;
    mapFinalTx2OriTx.clear();
    mapFinalTx2OriTx2.clear();
}

void MCTxMemPool::DoClear()
{
    mapLinks.clear();
    mapTx.clear();
    mapNextTx.clear();
    totalTxSize = 0;
    cachedInnerUsage = 0;
    lastRollingFeeUpdate = GetTime();
    blockSinceLastRollingFeeBump = false;
    rollingMinimumFeeRate = 0;
    ++nTransactionsUpdated;
}

void MCTxMemPool::Clear()
{
    LOCK(cs);
    DoClear();
}

void MCTxMemPool::Check(const MCCoinsViewCache *pcoins) const
{
    if (nCheckFrequency == 0)
        return;

    if (GetRand(std::numeric_limits<uint32_t>::max()) >= nCheckFrequency)
        return;


    LogPrint(BCLog::MEMPOOL, "Checking mempool with %u transactions and %u inputs\n", (unsigned int)mapTx.size(), (unsigned int)mapNextTx.size());

    uint64_t checkTotal = 0;
    uint64_t innerUsage = 0;

    LOCK(cs);
    MCCoinsViewCache mempoolDuplicate(const_cast<MCCoinsViewCache*>(pcoins));
    const int64_t nSpendHeight = GetSpendHeight(mempoolDuplicate);

    std::list<const MCTxMemPoolEntry*> waitingOnDependants;
    for (indexed_transaction_set::const_iterator it = mapTx.begin(); it != mapTx.end(); it++) {
        unsigned int i = 0;
        checkTotal += it->GetTxSize();
        innerUsage += it->DynamicMemoryUsage();
        const MCTransaction& tx = it->GetTx();
        uint256 txHash = it->GetTx().GetHash();
        txlinksMap::const_iterator linksiter = mapLinks.find(it);
        assert(linksiter != mapLinks.end());
        const TxLinks &links = linksiter->second;
        innerUsage += memusage::DynamicUsage(links.parents) + memusage::DynamicUsage(links.children);
        bool fDependsWait = false;

        setEntries setParentCheck;
        int64_t parentSizes = 0;
        int64_t parentSigOpCost = 0;
        for (const MCTxIn &txin : tx.vin) {
            // Check that every mempool transaction's inputs refer to available coins, or other mempool tx's.
            indexed_transaction_set::const_iterator it2 = mapTx.find(txin.prevout.hash);
            if (it2 != mapTx.end()) {
                const MCTransaction& tx2 = it2->GetTx();
                assert(tx2.vout.size() > txin.prevout.n && !tx2.vout[txin.prevout.n].IsNull());
                fDependsWait = true;
                if (setParentCheck.insert(it2).second) {
                    parentSizes += it2->GetTxSize();
                    parentSigOpCost += it2->GetSigOpCost();
                }
            }
            else {
                if (!(tx.IsBranchChainTransStep2())) {// mempool branchchain trans step2 tx no valid input 
                    bool havecoin = pcoins->HaveCoin(txin.prevout);
                    if (!havecoin) {
                        LogPrint(BCLog::MEMPOOL, "assert if have coin for input, fail to find input. txid %s, tx-version %d, %s\n", tx.GetHash().GetHex(), tx.nVersion);
                    }
                    assert(havecoin);
                }
            }
            // Check whether its inputs are marked in mapNextTx.
            auto it3 = mapNextTx.find(txin.prevout);
            if (!(tx.IsBranchChainTransStep2())) {
                assert(it3 != mapNextTx.end());
                assert(it3->first == &txin.prevout);
                assert(it3->second == &tx);
            }
            i++;
        }

        if (it->contractData != nullptr) {
            for (const MCContractID& contractId : it->contractData->contractAddrs) {
                auto links = contractLinksMap.find(contractId);
                if (links != contractLinksMap.end()) {
                    for (auto liter = links->second.begin(); liter != links->second.end(); ++liter) {
                        if (*liter == it) {
                            if (liter != links->second.begin()) {
                                auto piter = liter;
                                --piter;
                                if (setParentCheck.insert(*(piter)).second) {
                                    parentSizes += (*piter)->GetTxSize();
                                    parentSigOpCost += (*piter)->GetSigOpCost();
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        assert(setParentCheck == GetMemPoolParents(it));
        // Verify ancestor state is correct.
        setEntries setAncestors;
        uint64_t nNoLimit = std::numeric_limits<uint64_t>::max();
        std::string dummy;
        CalculateMemPoolAncestors(*it, nullptr, setAncestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy);
        uint64_t nCountCheck = setAncestors.size() + 1;
        uint64_t nSizeCheck = it->GetTxSize();
        MCAmount nFeesCheck = it->GetModifiedFee();
        int64_t nSigOpCheck = it->GetSigOpCost();

        for (txiter ancestorIt : setAncestors) {
            nSizeCheck += ancestorIt->GetTxSize();
            nFeesCheck += ancestorIt->GetModifiedFee();
            nSigOpCheck += ancestorIt->GetSigOpCost();
        }

        assert(it->GetCountWithAncestors() == nCountCheck);
        assert(it->GetSizeWithAncestors() == nSizeCheck);
        assert(it->GetSigOpCostWithAncestors() == nSigOpCheck);
        assert(it->GetModFeesWithAncestors() == nFeesCheck);

        // Check children against mapNextTx
        MCTxMemPool::setEntries setChildrenCheck;
        auto iter = mapNextTx.lower_bound(MCOutPoint(txHash, 0));
        int64_t childSizes = 0;
        for (; iter != mapNextTx.end() && iter->first->hash == txHash; ++iter) {
            txiter childit = mapTx.find(iter->second->GetHash());
            assert(childit != mapTx.end()); // mapNextTx points to in-mempool transactions
            if (setChildrenCheck.insert(childit).second) {
                childSizes += childit->GetTxSize();
            }
        }

        if (it->contractData != nullptr) {
            for (const MCContractID& contractId : it->contractData->contractAddrs) {
                auto links = contractLinksMap.find(contractId);
                assert(links != contractLinksMap.end());
                for (auto liter = links->second.begin(); liter != links->second.end(); ++liter) {
                    if (*liter == it) {
                        auto citer = liter;
                        ++citer;
                        if (citer != links->second.end()) {
                            if (setChildrenCheck.insert(*citer).second) {
                                childSizes += (*citer)->GetTxSize();
                            }
                        }
                        break;
                    }
                }
            }
        }

        assert(setChildrenCheck == GetMemPoolChildren(it));
        // Also check to make sure size is greater than sum with immediate children.
        // just a sanity check, not definitive that this calc is correct...
        assert(it->GetSizeWithDescendants() >= childSizes + it->GetTxSize());

        if (fDependsWait)
            waitingOnDependants.push_back(&(*it));
        else {
            MCValidationState state;
            bool fCheckResult = tx.IsCoinBase() || tx.IsBranchChainTransStep2() || // mempool branchchain trans step2 tx no valid input 
                Consensus::CheckTxInputs(tx, state, mempoolDuplicate, nSpendHeight);
            assert(fCheckResult);
            UpdateCoins(tx, mempoolDuplicate, 1000000, true);
        }
    }
    unsigned int stepsSinceLastRemove = 0;
    while (!waitingOnDependants.empty()) {
        const MCTxMemPoolEntry* entry = waitingOnDependants.front();
        waitingOnDependants.pop_front();
        MCValidationState state;
        if (!mempoolDuplicate.HaveInputs(entry->GetTx())) {
            waitingOnDependants.push_back(entry);
            stepsSinceLastRemove++;
            assert(stepsSinceLastRemove < waitingOnDependants.size());
        }
        else {
            bool fCheckResult = entry->GetTx().IsCoinBase() ||
                Consensus::CheckTxInputs(entry->GetTx(), state, mempoolDuplicate, nSpendHeight);
            assert(fCheckResult);
            UpdateCoins(entry->GetTx(), mempoolDuplicate, 1000000, true);
            stepsSinceLastRemove = 0;
        }
    }
    for (auto it = mapNextTx.cbegin(); it != mapNextTx.cend(); it++) {
        uint256 hash = it->second->GetHash();
        indexed_transaction_set::const_iterator it2 = mapTx.find(hash);
        const MCTransaction& tx = it2->GetTx();
        assert(it2 != mapTx.end());
        assert(&tx == it->second);
    }

    assert(totalTxSize == checkTotal);
    assert(innerUsage == cachedInnerUsage);
}

bool MCTxMemPool::CompareDepthAndScore(const uint256& hasha, const uint256& hashb)
{
    LOCK(cs);
    indexed_transaction_set::const_iterator i = mapTx.find(hasha);
    if (i == mapTx.end()) return false;
    indexed_transaction_set::const_iterator j = mapTx.find(hashb);
    if (j == mapTx.end()) return true;
    uint64_t counta = i->GetCountWithAncestors();
    uint64_t countb = j->GetCountWithAncestors();
    if (counta == countb) {
        return CompareTxMemPoolEntryByScore()(*i, *j);
    }
    return counta < countb;
}

namespace {
    class DepthAndScoreComparator
    {
    public:
        bool operator()(const MCTxMemPool::indexed_transaction_set::const_iterator& a, const MCTxMemPool::indexed_transaction_set::const_iterator& b)
        {
            uint64_t counta = a->GetCountWithAncestors();
            uint64_t countb = b->GetCountWithAncestors();
            if (counta == countb) {
                return CompareTxMemPoolEntryByScore()(*a, *b);
            }
            return counta < countb;
        }
    };
} // namespace

std::vector<MCTxMemPool::indexed_transaction_set::iterator> MCTxMemPool::GetSortedDepthAndScore()
{
    std::vector<indexed_transaction_set::iterator> iters;
    AssertLockHeld(cs);

    iters.reserve(mapTx.size());

    for (indexed_transaction_set::iterator mi = mapTx.begin(); mi != mapTx.end(); ++mi) {
        iters.push_back(mi);
    }
    std::sort(iters.begin(), iters.end(), DepthAndScoreComparator());
    return iters;
}

void MCTxMemPool::QueryHashes(std::vector<uint256>& vtxid)
{
    LOCK(cs);
    auto iters = GetSortedDepthAndScore();

    vtxid.clear();
    vtxid.reserve(mapTx.size());

    for (auto it : iters) {
        vtxid.push_back(it->GetTx().GetHash());
    }
}

static TxMempoolInfo GetInfo(MCTxMemPool::indexed_transaction_set::const_iterator it) {
    return TxMempoolInfo{ it->GetSharedTx(), it->GetTime(), MCFeeRate(it->GetFee(), it->GetTxSize()), it->GetModifiedFee() - it->GetFee() };
}

std::vector<TxMempoolInfo> MCTxMemPool::InfoAll()
{
    LOCK(cs);
    auto iters = GetSortedDepthAndScore();

    std::vector<TxMempoolInfo> ret;
    ret.reserve(mapTx.size());
    for (auto it : iters) {
        ret.push_back(GetInfo(it));
    }

    return ret;
}

MCTransactionRef MCTxMemPool::Get(const uint256& hash) const
{
    LOCK(cs);
    indexed_transaction_set::const_iterator i = mapTx.find(hash);
    if (i == mapTx.end())
        return nullptr;
    return i->GetSharedTx();
}

TxMempoolInfo MCTxMemPool::Info(const uint256& hash) const
{
    LOCK(cs);
    indexed_transaction_set::const_iterator i = mapTx.find(hash);
    if (i == mapTx.end())
        return TxMempoolInfo();
    return GetInfo(i);
}

void MCTxMemPool::PrioritiseTransaction(const uint256& hash, const MCAmount& nFeeDelta)
{
    {
        LOCK(cs);
        MCAmount &delta = mapDeltas[hash];
        delta += nFeeDelta;
        txiter it = mapTx.find(hash);
        if (it != mapTx.end()) {
            mapTx.modify(it, update_fee_delta(delta));
            // Now update all ancestors' modified fees with descendants
            setEntries setAncestors;
            uint64_t nNoLimit = std::numeric_limits<uint64_t>::max();
            std::string dummy;
            CalculateMemPoolAncestors(*it, nullptr, setAncestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, false);
            for (txiter ancestorIt : setAncestors) {
                mapTx.modify(ancestorIt, update_descendant_state(0, nFeeDelta, 0));
            }
            // Now update all descendants' modified fees with ancestors
            setEntries setDescendants;
            CalculateDescendants(it, setDescendants, true);
            setDescendants.erase(it);
            for (txiter descendantIt : setDescendants) {
                mapTx.modify(descendantIt, update_ancestor_state(0, nFeeDelta, 0, 0));
            }
            ++nTransactionsUpdated;
        }
    }
    LogPrintf("PrioritiseTransaction: %s feerate += %s\n", hash.ToString(), FormatMoney(nFeeDelta));
}

void MCTxMemPool::ApplyDelta(const uint256 hash, MCAmount &nFeeDelta) const
{
    LOCK(cs);
    std::map<uint256, MCAmount>::const_iterator pos = mapDeltas.find(hash);
    if (pos == mapDeltas.end())
        return;
    const MCAmount &delta = pos->second;
    nFeeDelta += delta;
}

void MCTxMemPool::ClearPrioritisation(const uint256 hash)
{
    LOCK(cs);
    mapDeltas.erase(hash);
}

bool MCTxMemPool::HasNoInputsOf(const MCTransaction &tx) const
{
    for (unsigned int i = 0; i < tx.vin.size(); i++)
        if (Exists(tx.vin[i].prevout.hash))
            return false;
    return true;
}

MCCoinsViewMemPool::MCCoinsViewMemPool(MCCoinsView* baseIn, const MCTxMemPool& mempoolIn) : MCCoinsViewBacked(baseIn), mempool(mempoolIn) { }

bool MCCoinsViewMemPool::GetCoin(const MCOutPoint &outpoint, Coin &coin) const {
    // If an entry in the mempool exists, always return that one, as it's guaranteed to never
    // conflict with the underlying cache, and it cannot have pruned entries (as it contains full)
    // transactions. First checking the underlying cache risks returning a pruned entry instead.
    MCTransactionRef ptx = mempool.Get(outpoint.hash);
    if (ptx) {
        if (outpoint.n < ptx->vout.size()) {
            if (ptx->IsSmartContract() && ptx->pContractData->contractCoinsOut.size() > 0) {
                return false;
            }
            coin = Coin(ptx->vout[outpoint.n], MEMPOOL_HEIGHT, false);
            return true;
        }
        else {
            return false;
        }
    }
    return base->GetCoin(outpoint, coin);
}

size_t MCTxMemPool::DynamicMemoryUsage() const {
    LOCK(cs);
    // Estimate the overhead of mapTx to be 15 pointers + an allocation, as no exact formula for boost::multi_index_contained is implemented.
    return memusage::MallocUsage(sizeof(MCTxMemPoolEntry) + 15 * sizeof(void*)) * mapTx.size() + memusage::DynamicUsage(mapNextTx) + memusage::DynamicUsage(mapDeltas) + memusage::DynamicUsage(mapLinks) + memusage::DynamicUsage(vTxHashes) + cachedInnerUsage;
}

void MCTxMemPool::RemoveStaged(setEntries &stage, bool updateDescendants, MemPoolRemovalReason reason) {
    AssertLockHeld(cs);
    UpdateForRemoveFromMempool(stage, updateDescendants);
    for (const txiter& it : stage) {
        RemoveUnchecked(it, reason);
    }
}

int MCTxMemPool::Expire(int64_t time) {
    LOCK(cs);
    indexed_transaction_set::index<entry_time>::type::iterator it = mapTx.get<entry_time>().begin();
    setEntries toremove;
    while (it != mapTx.get<entry_time>().end() && it->GetTime() < time) {
        toremove.insert(mapTx.project<0>(it));
        it++;
    }
    setEntries stage;
    for (txiter removeit : toremove) {
        CalculateDescendants(removeit, stage, false);
    }
    RemoveStaged(stage, true, MemPoolRemovalReason::EXPIRY);
    return stage.size();
}

void MCTxMemPool::ReacceptTransactions()
{
    LOCK(cs);
    std::vector<MCTransactionRef> vecRemoves;
    mpContractDb->contractContext.ClearAll();
    pCoinAmountCache->Clear();

    static SmartLuaState sls;
    auto entries = mempool.GetSortedDepthAndScore();
    for (int i = 0; i < entries.size(); ++i) {
        MCTransactionRef pTx = entries[i]->GetSharedTx();
        if (pTx->IsSmartContract()) {
            try {
                if (!CheckSmartContract(&sls, *entries[i], SmartLuaState::SAVE_TYPE_DATA, pCoinAmountCache)) {
                    vecRemoves.emplace_back(pTx);
                }
                else {
                    CheckContract(entries[i], &sls);
                    if (pTx->pContractData != nullptr && pTx->pContractData->contractCoinsOut.size() > 0) {
                        for (auto iter : pTx->pContractData->contractCoinsOut) {
                            pCoinAmountCache->DecAmount(iter.first, iter.second);
                        }
                    }

                    for (int j = 0; j < pTx->vout.size(); ++j) {
                        const MCScript& scriptPubKey = pTx->vout[j].scriptPubKey;
                        if (scriptPubKey.IsContract()) {
                            opcodetype opcode;
                            std::vector<unsigned char> vch;
                            MCScript::const_iterator pc = scriptPubKey.begin();
                            MCScript::const_iterator end = scriptPubKey.end();
                            scriptPubKey.GetOp(pc, opcode, vch);

                            assert(opcode == OP_CONTRACT || opcode == OP_CONTRACT_CHANGE);
                            vch.clear();
                            vch.assign(pc + 1, end);
                            uint160 key = uint160(vch);
                            MCContractID contractId = MCContractID(key);
                            pCoinAmountCache->IncAmount(contractId, pTx->vout[j].nValue);
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                LogPrintf("ReacceptTransactions contract tx exception %s\n", e.what());
                vecRemoves.emplace_back(pTx);
            }
            catch (...) {
                LogPrintf("ReacceptTransactions contract tx unknow exception\n");
                vecRemoves.emplace_back(pTx);
            }
        }
    }

    for (const MCTransactionRef pTx : vecRemoves) {
        RemoveRecursive(*pTx, MemPoolRemovalReason::BLOCK);
    }
}

bool MCTxMemPool::AddUnchecked(const uint256&hash, const MCTxMemPoolEntry &entry, bool validFeeEstimate)
{
    LOCK(cs);
    setEntries setAncestors;
    uint64_t nNoLimit = std::numeric_limits<uint64_t>::max();
    std::string dummy;
    CalculateMemPoolAncestors(entry, nullptr, setAncestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy);
    return AddUnchecked(hash, entry, setAncestors, validFeeEstimate);
}

void MCTxMemPool::PreCalculateTxOrder(const DisconnectedBlockTransactions& disconnectpool, std::map<uint256, uint64_t>& orders)
{
    // map of txs in block
    // value.first is base order, value.second is adjust order before tx
    std::map<uint256, std::pair<txiter, MCTransactionRef>> txOrders;
    auto it = disconnectpool.queuedTx.get<insertion_order>().rbegin();
    while (it != disconnectpool.queuedTx.get<insertion_order>().rend()) {
        MCTransactionRef ptx = *it++;
        const uint256& hash = ptx->GetHash();
        txiter before = mapTx.end();
        uint64_t order = std::numeric_limits<uint64_t>::max();

        // before any tx in mempool?
        auto iter = mapNextTx.lower_bound(MCOutPoint(hash, 0));
        for (; iter != mapNextTx.end() && iter->first->hash == hash; ++iter) {
            const uint256 &childHash = iter->second->GetHash();
            txiter childIter = mapTx.find(childHash);
            assert(childIter != mapTx.end());
            if (before == mapTx.end() || childIter->GetOrder() < before->GetOrder()) {
                before = childIter;
            }
        }

        // is publish contract?
        if (ptx->nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION) {
            auto iter = contractLinksMap.find(ptx->pContractData->address);
            if (iter != contractLinksMap.end() && iter->second.size() > 0) {
                auto begin = iter->second.begin();
                if (before == mapTx.end() || (*begin)->GetOrder() < before->GetOrder()) {
                    before = *begin;
                }
            }
        }

        // is vin's prevout in this block?
        for (int i = 0; i < ptx->vin.size(); ++i) {
            auto piter = txOrders.find(ptx->vin[i].prevout.hash);
            if (piter != txOrders.end()) {
                auto temp = piter;
                while (temp->second.second != nullptr) {
                    temp = txOrders.find(temp->second.second->GetHash());
                    assert(temp != txOrders.end());
                }
                // make prevout before this tx
                if (before != mapTx.end() && temp->second.first->GetOrder() > before->GetOrder()) {
                    txOrders[ptx->vin[i].prevout.hash] = std::make_pair(piter->second.first, ptx);
                }
            }
        }

        txOrders[hash] = std::make_pair(before, nullptr);
    }

    std::map<uint64_t, uint256> order2tx;
    it = disconnectpool.queuedTx.get<insertion_order>().rbegin();
    while (it != disconnectpool.queuedTx.get<insertion_order>().rend()) {
        MCTransactionRef ptx = *it++;
        auto item = txOrders.find(ptx->GetHash());
        if (orders.count(item->first) == 0) {
            int offset = 0;
            auto temp = item;
            while (temp->second.second != nullptr) {
                temp = txOrders.find(temp->second.second->GetHash());
                assert(temp != txOrders.end());
                offset++;
            }

            uint64_t minOrder = (temp->second.first == mapTx.end() ? GenerateNextOrder() : temp->second.first->GetOrder());
            uint64_t maxOrder = minOrder;

            temp = item;
            while (temp->second.second != nullptr) {
                orders[temp->first] = maxOrder++;
                temp = txOrders.find(temp->second.second->GetHash());
            }
            orders[temp->first] = maxOrder;
            assert(minOrder + offset == maxOrder);

            // calculate the number of item need to offset
            std::vector<uint256> modifyItems;
            auto oi = order2tx.begin();
            auto mi = mapTx.get<entry_time>().begin();
            while (oi != order2tx.end() || mi != mapTx.get<entry_time>().end()) {
                bool oivalid = false;
                while (oi != order2tx.end()) {
                    if (oi->first >= minOrder && oi->first <= maxOrder) {
                        oivalid = true;
                        break;
                    }
                    else if (oi->first > maxOrder) {
                        break;
                    }
                    oi++;
                }

                bool mivalid = false;
                while (mi != mapTx.get<entry_time>().end()) {
                    if (mi->GetOrder() >= minOrder && mi->GetOrder() <= maxOrder) {
                        mivalid = true;
                        break;
                    }
                    else if (mi->GetOrder() > maxOrder) {
                        break;
                    }
                    mi++;
                }

                if (!oivalid && !mivalid) {
                    break;
                }

                bool addoi = false;
                if (oivalid && mivalid) {
                    assert(oi->first != mi->GetOrder());
                    if (oi->first < mi->GetOrder()) {
                        addoi = true;
                    }
                }
                else if (oivalid) {
                    addoi = true;
                }
                else if (mivalid) {
                    addoi = false;
                }

                if (addoi) {
                    modifyItems.emplace_back(oi->second);
                    oi++;
                }
                else {
                    modifyItems.emplace_back(mi->GetTx().GetHash());
                    mi++;
                }
                maxOrder++;
            }

            nextEntryOrder = std::max(maxOrder + 1, nextEntryOrder);

            MCTxMemPoolEntry::ModifyOrder modifier;
            for (int i = modifyItems.size() - 1; i >= 0; --i) {
                if (orders.count(modifyItems[i])) {
                    order2tx[maxOrder] = modifyItems[i];
                    orders[modifyItems[i]] = maxOrder--;
                }
                else {
                    order2tx.erase(maxOrder);
                    txiter iter = mapTx.find(modifyItems[i]);
                    modifier.order = maxOrder--;
                    mapTx.modify(iter, modifier);
                }
            }

            temp = item;
            while (temp->second.second != nullptr) {
                order2tx[minOrder++] = temp->first;
                temp = txOrders.find(temp->second.second->GetHash());
            }
            order2tx[minOrder] = temp->first;
        }
    }
}

void MCTxMemPool::UpdateChild(txiter entry, txiter child, bool add)
{
    setEntries s;
    if (add && mapLinks[entry].children.insert(child).second) {
        cachedInnerUsage += memusage::IncrementalDynamicUsage(s);
    }
    else if (!add && mapLinks[entry].children.erase(child)) {
        cachedInnerUsage -= memusage::IncrementalDynamicUsage(s);
    }
}

void MCTxMemPool::UpdateParent(txiter entry, txiter parent, bool add)
{
    setEntries s;
    if (add && mapLinks[entry].parents.insert(parent).second) {
        cachedInnerUsage += memusage::IncrementalDynamicUsage(s);
    }
    else if (!add && mapLinks[entry].parents.erase(parent)) {
        cachedInnerUsage -= memusage::IncrementalDynamicUsage(s);
    }
}

const MCTxMemPool::setEntries & MCTxMemPool::GetMemPoolParents(txiter entry) const
{
    assert(entry != mapTx.end());
    txlinksMap::const_iterator it = mapLinks.find(entry);
    assert(it != mapLinks.end());
    return it->second.parents;
}

const MCTxMemPool::setEntries & MCTxMemPool::GetMemPoolChildren(txiter entry) const
{
    assert(entry != mapTx.end());
    txlinksMap::const_iterator it = mapLinks.find(entry);
    assert(it != mapLinks.end());
    return it->second.children;
}

MCFeeRate MCTxMemPool::GetMinFee(size_t sizelimit) const {
    LOCK(cs);
    if (!blockSinceLastRollingFeeBump || rollingMinimumFeeRate == 0)
        return MCFeeRate(rollingMinimumFeeRate);

    int64_t time = GetTime();
    if (time > lastRollingFeeUpdate + 10) {
        double halflife = ROLLING_FEE_HALFLIFE;
        if (DynamicMemoryUsage() < sizelimit / 4)
            halflife /= 4;
        else if (DynamicMemoryUsage() < sizelimit / 2)
            halflife /= 2;

        rollingMinimumFeeRate = rollingMinimumFeeRate / pow(2.0, (time - lastRollingFeeUpdate) / halflife);
        lastRollingFeeUpdate = time;

        if (rollingMinimumFeeRate < (double)incrementalRelayFee.GetFeePerK() / 2) {
            rollingMinimumFeeRate = 0;
            return MCFeeRate(0);
        }
    }
    return std::max(MCFeeRate(rollingMinimumFeeRate), incrementalRelayFee);
}

void MCTxMemPool::trackPackageRemoved(const MCFeeRate& rate) {
    AssertLockHeld(cs);
    if (rate.GetFeePerK() > rollingMinimumFeeRate) {
        rollingMinimumFeeRate = rate.GetFeePerK();
        blockSinceLastRollingFeeBump = false;
    }
}

void MCTxMemPool::TrimToSize(size_t sizelimit, std::vector<MCOutPoint>* pvNoSpendsRemaining) {
    LOCK(cs);

    unsigned nTxnRemoved = 0;
    MCFeeRate maxFeeRateRemoved(0);
    while (!mapTx.empty() && DynamicMemoryUsage() > sizelimit) {
        indexed_transaction_set::index<descendant_score>::type::iterator it = mapTx.get<descendant_score>().begin();

        // We set the new mempool min fee to the feerate of the removed set, plus the
        // "minimum reasonable fee rate" (ie some value under which we consider txn
        // to have 0 fee). This way, we don't allow txn to enter mempool with feerate
        // equal to txn which were removed with no block in between.
        MCFeeRate removed(it->GetModFeesWithDescendants(), it->GetSizeWithDescendants());
        removed += incrementalRelayFee;
        trackPackageRemoved(removed);
        maxFeeRateRemoved = std::max(maxFeeRateRemoved, removed);

        setEntries stage;
        CalculateDescendants(mapTx.project<0>(it), stage, false);
        nTxnRemoved += stage.size();

        std::vector<MCTransaction> txn;
        if (pvNoSpendsRemaining) {
            txn.reserve(stage.size());
            for (txiter iter : stage)
                txn.push_back(iter->GetTx());
        }
        RemoveStaged(stage, true, MemPoolRemovalReason::SIZELIMIT);
        if (pvNoSpendsRemaining) {
            for (const MCTransaction& tx : txn) {
                for (const MCTxIn& txin : tx.vin) {
                    if (Exists(txin.prevout.hash)) continue;
                    pvNoSpendsRemaining->push_back(txin.prevout);
                }
            }
        }
    }

    if (maxFeeRateRemoved > MCFeeRate(0)) {
        LogPrint(BCLog::MEMPOOL, "Removed %u txn, rolling minimum fee bumped to %s\n", nTxnRemoved, maxFeeRateRemoved.ToString());
    }
}

bool MCTxMemPool::TransactionWithinChainLimit(const uint256& txid, size_t chainLimit) const {
    LOCK(cs);
    auto it = mapTx.find(txid);
    return it == mapTx.end() || (it->GetCountWithAncestors() < chainLimit &&
        it->GetCountWithDescendants() < chainLimit);
}

SaltedTxidHasher::SaltedTxidHasher() : k0(GetRand(std::numeric_limits<uint64_t>::max())), k1(GetRand(std::numeric_limits<uint64_t>::max()))
{
}
