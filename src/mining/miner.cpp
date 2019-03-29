// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mining/miner.h"

#include "misc/amount.h"
#include "chain/chain.h"
#include "chain/chainparams.h"
#include "transaction/coins.h"
#include "consensus/consensus.h"
#include "consensus/merkle.h"
#include "consensus/tx_verify.h"
#include "consensus/validation.h"
#include "coding/hash.h"
#include "net/net.h"
#include "policy/feerate.h"
#include "policy/policy.h"
#include "misc/pow.h"
#include "primitives/transaction.h"
#include "script/standard.h"
#include "misc/timedata.h"
#include "transaction/txmempool.h"
#include "utils/util.h"
#include "utils/utilmoneystr.h"
#include "validation/validation.h"
#include "validation/validationinterface.h"
#include "wallet/wallet.h"
#include "coding/base58.h"
#include "validation/checkpoints.h"
#include "key/key.h"
#include "key/keystore.h"
#include "policy/policy.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "script/sign.h"
#include "ui/ui_interface.h"
#include "wallet/coincontrol.h"
#include "net/compat.h"
#include "init.h"
#include "mining/mining.h"
#include "transaction/txdb.h"
#include "rpc/server.h"
#include "policy/rbf.h"

#include <assert.h>
#include <algorithm>
#include <queue>
#include <utility>
#include "chain/branchchain.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include "chain/branchdb.h"
#include "rpc/branchchainrpc.h"
typedef base_uint<512> uint512;

//////////////////////////////////////////////////////////////////////////////
//
// MagnachainMiner
//

// Define reserve size use in addPackageTxs, to avoid bad-blk-length failure.
// Because some tx will add more data to transaction after add to block(after addPackageTxs),
// that will increase block final size.
uint64_t ReservePubContractBlockDataSize = 100;
uint64_t ReserveCallContractBlockDataSize = 1000;
uint64_t ReserveBranchTxBlockDataSize = 100;

//
// Unconfirmed transactions in the memory pool often depend on other
// transactions in the memory pool. When we select transactions from the
// pool, we select by highest fee rate of a transaction combined with all
// its ancestors.

uint64_t nLastBlockTx = 0;
uint64_t nLastBlockWeight = 0;

int64_t UpdateTime(MCBlockHeader* pblock, const Consensus::Params& consensusParams, const MCBlockIndex* pindexPrev)
{
    int64_t nOldTime = pblock->nTime;
    int64_t nNewTime = std::max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());

    if (nOldTime < nNewTime)
        pblock->nTime = nNewTime;

    // Updating time can change work required on testnet:
    if (consensusParams.fPowAllowMinDifficultyBlocks)
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock, consensusParams);

    return nNewTime - nOldTime;
}

BlockAssembler::Options::Options() {
    blockMinFeeRate = MCFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);
    nBlockMaxWeight = DEFAULT_BLOCK_MAX_WEIGHT;
}

BlockAssembler::BlockAssembler(const MCChainParams& params, const Options& options) : chainparams(params)
{
    blockMinFeeRate = options.blockMinFeeRate;
    // Limit weight to between 4K and MAX_BLOCK_WEIGHT-4K for sanity:
    nBlockMaxWeight = std::max<size_t>(4000, std::min<size_t>(MAX_BLOCK_WEIGHT - 4000, options.nBlockMaxWeight));
	outpoint = options.outpoint;
}

BlockAssembler::Options BlockAssembler::DefaultOptions(const MCChainParams& params)
{
    // Block resource limits
    // If neither -blockmaxsize or -blockmaxweight is given, limit to DEFAULT_BLOCK_MAX_*
    // If only one is given, only restrict the specified resource.
    // If both are given, restrict both.
    BlockAssembler::Options options;
    options.nBlockMaxWeight = gArgs.GetArg("-blockmaxweight", DEFAULT_BLOCK_MAX_WEIGHT);
    if (gArgs.IsArgSet("-blockmintxfee")) {
        MCAmount n = 0;
        ParseMoney(gArgs.GetArg("-blockmintxfee", ""), n);
        options.blockMinFeeRate = MCFeeRate(n);
    } else {
        options.blockMinFeeRate = MCFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);
    }
    return options;
}

BlockAssembler::BlockAssembler(const MCChainParams& params) : BlockAssembler(params, DefaultOptions(params)) {}

void BlockAssembler::resetBlock()
{
    inBlock.clear();

    // Reserve space for coinbase tx
    nBlockWeight = 4000;
    nBlockSigOpsCost = 400;
    fIncludeWitness = false;

    // These counters do not include coinbase tx
    nBlockTx = 0;
    nFees = 0;
}

void BlockAssembler::onlyUnconfirmed(MCTxMemPool::setEntries& testSet)
{
    for (MCTxMemPool::setEntries::iterator iit = testSet.begin(); iit != testSet.end();) {
        // Only test txs not already in the block
        if (inBlock.count(*iit)) {
            testSet.erase(iit++);
        } else {
            iit++;
        }
    }
}

uint64_t BlockAssembler::GetTxReserveBlockSize(const MCTransaction& tx)
{
    if (tx.nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION) {
        return ReservePubContractBlockDataSize;
    }
    else if (tx.nVersion == MCTransaction::CALL_CONTRACT_VERSION) {
        return ReserveCallContractBlockDataSize;
    }
    else if (tx.IsBranchChainTransStep2()) {
        if (chainparams.IsMainChain()) {
            return ReserveBranchTxBlockDataSize;
        }
    }
    return 0;
}

bool BlockAssembler::TestPackage(MCTxMemPool::txiter iter, uint64_t packageSize, int64_t packageSigOpsCost)
{
    packageSize += GetTxReserveBlockSize(iter->GetTx()) * std::max<uint64_t>( 1, iter->GetCountWithAncestors());

    // TODO: switch to weight-based accounting for packages instead of vsize-based accounting.
    if (nBlockWeight + WITNESS_SCALE_FACTOR * packageSize >= nBlockMaxWeight)
        return false;
    if (nBlockSigOpsCost + packageSigOpsCost >= MAX_BLOCK_SIGOPS_COST)
        return false;
    return true;
}

// Perform transaction-level checks before adding to block:
// - transaction finality (locktime)
// - premature witness (in case segwit transactions are added to mempool before
//   segwit activation)
bool BlockAssembler::TestPackageTransactions(const MCTxMemPool::setEntries& package)
{
    for (const MCTxMemPool::txiter it : package) {
        if (!IsFinalTx(it->GetTx(), nHeight, nLockTimeCutoff))
            return false;
        if (!fIncludeWitness && it->GetTx().HasWitness())
            return false;
    }
    return true;
}

void BlockAssembler::AddToBlock(MCTxMemPool::txiter iter, MakeBranchTxUTXO& utxoMaker)
{
    MCTransactionRef tx = iter->GetSharedTx();
    if ((chainparams.IsMainChain() && tx->IsBranchChainTransStep2()) || (tx->IsSmartContract() && tx->pContractData->contractCoinsOut.size() > 0)) {
        if (utxoMaker.mapCache.count(tx->GetHash()) == 0)
            throw std::runtime_error("utxo make did not make target transaction");
        pblock->vtx.emplace_back(utxoMaker.mapCache[tx->GetHash()]);
    }
    else
        pblock->vtx.emplace_back(tx);

    pblocktemplate->vTxFees.push_back(iter->GetFee());
    pblocktemplate->vTxSigOpsCost.push_back(iter->GetSigOpCost());
    nBlockWeight += iter->GetTxWeight() + GetTxReserveBlockSize(iter->GetTx()); // append reserve block size
    ++nBlockTx;
    nBlockSigOpsCost += iter->GetSigOpCost();
    nFees += iter->GetFee();
    inBlock.insert(iter);

    bool fPrintPriority = gArgs.GetBoolArg("-printpriority", DEFAULT_PRINTPRIORITY);
    if (fPrintPriority) {
        LogPrintf("fee %s txid %s\n",
            MCFeeRate(iter->GetModifiedFee(), iter->GetTxSize()).ToString(),
                  iter->GetTx().GetHash().ToString());
    }
}

int BlockAssembler::UpdatePackagesForAdded(const MCTxMemPool::setEntries& alreadyAdded,
    indexed_modified_transaction_set& mapModifiedTx)
{
    int nDescendantsUpdated = 0;
    for (const MCTxMemPool::txiter it : alreadyAdded) {
        MCTxMemPool::setEntries descendants;
        mempool.CalculateDescendants(it, descendants, true);
        // Insert all descendants (not yet in block) into the modified set
        for (MCTxMemPool::txiter desc : descendants) {
            if (alreadyAdded.count(desc))
                continue;
            ++nDescendantsUpdated;
            modtxiter mit = mapModifiedTx.find(desc);
            if (mit == mapModifiedTx.end()) {
                MCTxMemPoolModifiedEntry modEntry(desc);
                modEntry.nSizeWithAncestors -= it->GetTxSize();
                modEntry.nModFeesWithAncestors -= it->GetModifiedFee();
                modEntry.nSigOpCostWithAncestors -= it->GetSigOpCost();
                mapModifiedTx.insert(modEntry);
            } else {
                mapModifiedTx.modify(mit, update_for_parent_inclusion(it));
            }
        }
    }
    return nDescendantsUpdated;
}

// Skip entries in mapTx that are already in a block or are present
// in mapModifiedTx (which implies that the mapTx ancestor state is
// stale due to ancestor inclusion in the block)
// Also skip transactions that we've already failed to add. This can happen if
// we consider a transaction in mapModifiedTx and it fails: we can then
// potentially consider it again while walking mapTx.  It's currently
// guaranteed to fail again, but as a belt-and-suspenders check we put it in
// failedTx and avoid re-evaluation, since the re-evaluation would be using
// cached size/sigops/fee values that are not actually correct.
bool BlockAssembler::SkipMapTxEntry(MCTxMemPool::txiter it, indexed_modified_transaction_set& mapModifiedTx, MCTxMemPool::setEntries& failedTx)
{
    assert(it != mempool.mapTx.end());
    return mapModifiedTx.count(it) || inBlock.count(it) || failedTx.count(it);
}

void BlockAssembler::SortForBlock(const MCTxMemPool::setEntries& package, MCTxMemPool::txiter entry, std::vector<MCTxMemPool::txiter>& sortedEntries)
{
    // Sort package by ancestor count
    // If a transaction A depends on transaction B, then A's ancestor count
    // must be greater than B's.  So this is sufficient to validly order the
    // transactions for block inclusion.
    sortedEntries.clear();
    sortedEntries.insert(sortedEntries.begin(), package.begin(), package.end());
    std::sort(sortedEntries.begin(), sortedEntries.end(), CompareTxIterByAncestorCount());
}

//
bool GroupTransactionComparer(const std::pair<uint256, int>& v1, const std::pair<uint256, int>& v2)
{
    if (v1.second < v2.second)
        return true;
    else if (v1.second > v2.second)
        return false;
    return v1.first < v2.first;
}

// 按照输入关联及关联合约地址的分组调用智能合约
void BlockAssembler::GroupingTransaction(int offset, std::vector<const MCTxMemPoolEntry*>& blockTxEntries)
{
    typedef std::map<int, std::vector<std::pair<uint256, int>>> MAPGROUP;

    MAPGROUP group2trans;
    std::set<int> mergeGroups;
    std::map<uint256, int> trans2group;
    std::map<MCContractID, int> contract2group;

    {
        int groupId = 0;
        auto& temp = group2trans[groupId];
        for (int i = 0; i < offset; ++i) {
            uint256 hash;
            if (blockTxEntries[i] != nullptr) {// the offset has init with nullptr, because coinbase tx and stake tx no relative mempool entry
                hash = blockTxEntries[i]->GetTx().GetHash();
            }
            else if (pblock->vtx[i] != nullptr){// coinbase is not init.
                hash = pblock->vtx[i]->GetHash();
            }
            temp.emplace_back(std::make_pair(hash, i));
            trans2group[hash] = groupId;
        }
    }

    int nextGroupId = 1;
    for (int i = offset; i < pblock->vtx.size(); ++i) {
        mergeGroups.clear();
        int groupId = nextGroupId;
        const MCTransactionRef ptx = pblock->vtx[i];

        if (trans2group.find(ptx->GetHash()) != trans2group.end()) {
            groupId = trans2group[ptx->GetHash()];
        }

        std::set<uint256> parentHash;
        for (int j = 0; j < ptx->vin.size(); ++j) {
            const MCOutPoint& preOutPoint = ptx->vin[j].prevout;
            if (!preOutPoint.hash.IsNull()) {
                parentHash.insert(ptx->vin[j].prevout.hash);
            }
        }

        if (ptx->IsSyncBranchInfo()) {
            const uint256& hash = g_pBranchDataMemCache->GetParent(*ptx);
            parentHash.insert(hash);
        }

        int lastGroupId = -1;
        for (const uint256& hash : parentHash) {
            if (trans2group.find(hash) != trans2group.end()) {
                int preTransGroupId = trans2group[hash];
                groupId = std::min(preTransGroupId, groupId);
                if (lastGroupId != -1) {
                    groupId = std::min(groupId, lastGroupId);
                }
                if (groupId != preTransGroupId) {
                    mergeGroups.insert(preTransGroupId);
                }
                if (lastGroupId != -1 && groupId != lastGroupId) {
                    mergeGroups.insert(lastGroupId);
                }
            }
            else if (lastGroupId == -1 || lastGroupId == groupId) {
                // 输入不在区块中，标记该交易与当前groupid相同
                trans2group[hash] = groupId;
                auto& temp = group2trans[groupId];
                temp.emplace_back(std::make_pair(hash, std::numeric_limits<int>::max()));
            }
            else {
                groupId = std::min(lastGroupId, groupId);
                if (groupId != lastGroupId) {
                    mergeGroups.insert(lastGroupId);
                }
            }
            lastGroupId = groupId;
        }

        if (ptx->IsSmartContract()) {
            const MCTxMemPoolEntry* entry = blockTxEntries[i];
            int finalGroupId = groupId;
            for (auto& contractAddr : entry->contractData->contractAddrs) {
                if (contract2group.find(contractAddr) != contract2group.end()) {
                    int contractGroupId = contract2group[contractAddr];
                    finalGroupId = std::min(contractGroupId, finalGroupId);
                }
                else
                    contract2group[contractAddr] = groupId;
            }

            if (finalGroupId != groupId) {
                mergeGroups.insert(groupId);
                groupId = finalGroupId;
            }

            for (auto& contractAddr : entry->contractData->contractAddrs) {
                if (contract2group.find(contractAddr) != contract2group.end()) {
                    int contractGroupId = contract2group[contractAddr];
                    if (finalGroupId != contractGroupId) {
                        contract2group[contractAddr] = finalGroupId;
                        mergeGroups.insert(contractGroupId);
                    }
                }
            }
        }

        auto& des = group2trans[groupId];
        for (auto item : mergeGroups) {
            auto& src = group2trans[item];
            for (auto& it : src) {
                des.emplace_back(it);
                trans2group[it.first] = groupId;
            }
            group2trans.erase(item);
        }

        des.emplace_back(std::make_pair(ptx->GetHash(), i));
        trans2group[ptx->GetHash()] = groupId;
        if (ptx->IsSmartContract()) {
            if (!ptx->pContractData->address.IsNull()) {
                contract2group[ptx->pContractData->address] = groupId;
            }
        }

        if (groupId == nextGroupId) {
            nextGroupId++;
        }
    }

    std::vector<MCTransactionRef> vtx(pblock->vtx);
    pblock->vtx.clear();
    std::vector<MCAmount> vTxFees(pblocktemplate->vTxFees);
    pblocktemplate->vTxFees.clear();
    std::vector<MCAmount> vTxSigOpsCost(pblocktemplate->vTxSigOpsCost);
    pblocktemplate->vTxSigOpsCost.clear();

    // 限制了分组的数量
    std::vector<MAPGROUP::iterator> finalGroup;
    MAPGROUP::iterator iter = group2trans.begin();
    for (; iter != group2trans.end(); ++iter) {
        for (int i = iter->second.size() - 1; i >= 0; --i) {
            if (iter->second[i].second == std::numeric_limits<int>::max()) {
                iter->second.erase(iter->second.begin() + i);
            }
        }
        
        assert(iter->second.size() > 0);
        finalGroup.emplace_back(iter);
        if (finalGroup.size() >= MAX_GROUP_NUM) {
            ++iter;
            break;
        }
    }

    if (iter != group2trans.end()) {
        // 将超出最大分组数量的分组交易合并到现有的分组中
        int minGroupIndex = 0;
        int minGroupSize = iter->second.size();
        while (iter != group2trans.end()) {
            for (int i = iter->second.size() - 1; i >= 0; --i) {
                if (iter->second[i].second == std::numeric_limits<int>::max()) {
                    iter->second.erase(iter->second.begin() + i);
                }
            }

            assert(iter->second.size() > 0);
            if (minGroupSize + iter->second.size() > std::numeric_limits<uint16_t>::max()) {
                return;
            }

            finalGroup[minGroupIndex]->second.insert(finalGroup[minGroupIndex]->second.end(),
                iter->second.begin(), iter->second.end());
            iter = group2trans.erase(iter);

            minGroupIndex = 0;
            minGroupSize = finalGroup[0]->second.size();
            for (int i = 1; i < finalGroup.size(); ++i) {
                size_t sz = finalGroup[i]->second.size();
                if (sz < minGroupSize) {
                    minGroupSize = sz;
                    minGroupIndex = i;
                }
            }

            if (minGroupIndex == -1 || minGroupSize <= 0 || minGroupSize > std::numeric_limits<uint16_t>::max()) {
                return;
            }
        }
    }

    // 将分组好的交易重新打入包中
    int total = 0;
    pblock->groupSize.clear();
    for (int i = 0; i < finalGroup.size(); ++i) {
        assert(finalGroup[i]->second.size() > 0);
        total += finalGroup[i]->second.size();
        std::sort(finalGroup[i]->second.begin(), finalGroup[i]->second.end(), GroupTransactionComparer);
        for (int j = 0; j < finalGroup[i]->second.size(); ++j) {
            std::pair<uint256, int>& item = finalGroup[i]->second[j];
            assert(item.second < std::numeric_limits<int>::max());
            pblock->vtx.emplace_back(vtx[item.second]);
            pblocktemplate->vTxFees.emplace_back(vTxFees[item.second]);
            pblocktemplate->vTxSigOpsCost.emplace_back(vTxSigOpsCost[item.second]);
        }
        pblock->groupSize.emplace_back(finalGroup[i]->second.size());
    }
    LogPrint(BCLog::MINING, "%s:%d %d:%d\n", __FUNCTION__, __LINE__, total, vtx.size());
    assert(total == vtx.size());
}

MCAmount MakeBranchTxUTXO::UseUTXO(const uint160& key, MCAmount nAmount, std::vector<MCOutPoint>& vInOutPoints)
{
    if (mapBranchCoins.count(key) == 0) {
        CoinListPtr pcoinlist = pcoinListDb->GetList(key);

        BranchUTXOCache cache;
        if (pcoinlist != nullptr)
            cache.coinlist = *pcoinlist;// copy
        mapBranchCoins.insert(std::make_pair(key, cache));
    }
    BranchUTXOCache& utxoCache = mapBranchCoins[key];

    MCAmount nValue = 0;
    std::vector<int> usedIndex;//用来删除已使用的币
    //first get from db list
    //优先使用较老的币
    for (int i = 0; i < utxoCache.coinlist.coins.size(); i++) {
        const MCOutPoint& outpoint = utxoCache.coinlist.coins[i];
        const Coin& coin = pcoinsTip->AccessCoin(outpoint);// MCCoinsViewCache
        if (coin.IsSpent())
            continue;
        if (coin.IsCoinBase() && chainActive.Height() - coin.nHeight < COINBASE_MATURITY)
            continue;

        nValue += coin.out.nValue;
        vInOutPoints.push_back(outpoint);
        usedIndex.push_back(i);
        if (nValue >= nAmount)
            break;
    }

    // 2nd from cache output
    if (nValue < nAmount) {
        for (BranchUTXOCache::MAP_CACHE_COIN::iterator mit = utxoCache.mapCacheCoin.begin();
            mit != utxoCache.mapCacheCoin.end(); mit++) {
            const MCOutPoint& outpoint = mit->first;
            const MCTxOut& out = mit->second;

            nValue += out.nValue;
            vInOutPoints.push_back(outpoint);
            if (nValue >= nAmount)
                break;
        }
    }

    if (nValue < nAmount) {
        vInOutPoints.clear();
        return 0;
    }

    //从后面往前删除
    for (std::vector<int>::reverse_iterator rit = usedIndex.rbegin(); rit != usedIndex.rend(); rit++)
    {
        utxoCache.coinlist.coins.erase(utxoCache.coinlist.coins.begin() + *rit);
    }

    // from back to front, erase from vector(utxoCahce.coinlist.coins)
    for (auto rit = vInOutPoints.rbegin(); rit != vInOutPoints.rend(); rit++) {
        const MCOutPoint& outpoint = *rit;

        //spend utxoCahce
        //for (std::vector<MCOutPoint>::reverse_iterator ritc = utxoCache.coinlist.coins.rbegin();// for back end to remove fast.
        //    ritc != utxoCache.coinlist.coins.rend(); ritc++) {
        //    if (*ritc == outpoint)
        //    {
        //        utxoCache.coinlist.coins.erase(std::next(ritc).base());
        //        break;
        //    }
        //}

        utxoCache.mapCacheCoin.erase(outpoint);
    }

    return nValue;
}

bool MakeBranchTxUTXO::MakeTxUTXO(MCMutableTransaction& tx, const uint160& key, MCAmount nAmount, MCScript& scriptSig, MCScript& changeScriptPubKey)
{
    std::vector<MCOutPoint> vInOutPoints;
    MCAmount nValue = UseUTXO(key, nAmount, vInOutPoints);
    if (nValue < nAmount)
        return false;

    MCCoinControl coin_control;
    const uint32_t nSequence = coin_control.signalRbf ? MAX_BIP125_RBF_SEQUENCE : (MCTxIn::SEQUENCE_FINAL - 1);

    for (std::vector<MCOutPoint>::iterator it = vInOutPoints.begin();// from back to front, erase from vector(utxoCahce.coinlist.coins)
        it != vInOutPoints.end(); it++) {
        const MCOutPoint& outpoint = *it;

        //add to vin
        tx.vin.push_back(MCTxIn(outpoint, scriptSig, nSequence));
    }

    //recharge
    if (nValue > nAmount)
    {
        MCTxOut tmpOut;
        tmpOut.scriptPubKey = changeScriptPubKey;
        tmpOut.nValue = nValue - nAmount;
        tx.vout.push_back(tmpOut);
    }

    return true;
}

bool BlockAssembler::UpdateIncompleteTx(MCTxMemPool::txiter iter, MakeBranchTxUTXO& utxoMaker)
{
    MCMutableTransaction newTx(*iter->GetSharedTx());
    uint256 oldHash = newTx.GetHash();

    bool success = true;
    int vOutSize = newTx.vout.size();
    std::vector<uint160> keys;
    if (chainparams.IsMainChain() && newTx.IsBranchChainTransStep2()) {
        const std::string strFromChain = iter->GetSharedTx()->fromBranchId;
        uint256 branchhash;
        branchhash.SetHex(strFromChain);
        uint160 branchcoinaddress = Hash160(branchhash.begin(), branchhash.end());

        MCScript scriptPubKey;
        scriptPubKey << OP_TRANS_BRANCH << ToByteVector(branchhash);

        MCScript scriptSig = MCScript();
        newTx.vin.clear();
        success = utxoMaker.MakeTxUTXO(newTx, branchcoinaddress, newTx.inAmount, scriptSig, scriptPubKey);
        keys.push_back(branchcoinaddress);
    }
    if (newTx.IsSmartContract() && newTx.pContractData->contractCoinsOut.size() > 0) {
        for (auto it : newTx.pContractData->contractCoinsOut) {
            const MCContractID& contractId = it.first;
            MCScript contractScript = GetScriptForDestination(contractId);
            MCScript contractChangeScript = MCScript() << OP_CONTRACT_CHANGE << ToByteVector(contractId);
            success = utxoMaker.MakeTxUTXO(newTx, contractId, it.second, contractScript, contractChangeScript);
            if (!success) {
                break;
            }
            keys.push_back(it.first);
        }
    }

    if (success) {
        uint256 newHash = newTx.GetHash();
        for (int i = vOutSize; i < newTx.vout.size(); ++i) {
            BranchUTXOCache& utxoCache = utxoMaker.mapBranchCoins[keys[i - vOutSize]];
            utxoCache.mapCacheCoin.insert(std::make_pair(MCOutPoint(newHash, i), newTx.vout[i]));
        }
        utxoMaker.mapCache.insert(std::make_pair(oldHash, MakeTransactionRef(newTx)));
    }

    return success;
}

// This transaction selection algorithm orders the mempool based
// on feerate of a transaction including all unconfirmed ancestors.
// Since we don't remove transactions from the mempool as we select them
// for block inclusion, we need an alternate method of updating the feerate
// of a transaction with its not-yet-selected ancestors as we go.
// This is accomplished by walking the in-mempool descendants of selected
// transactions and storing a temporary modified state in mapModifiedTxs.
// Each time through the loop, we compare the best transaction in
// mapModifiedTxs with the next transaction in the mempool to decide what
// transaction package to work on next.
void BlockAssembler::addPackageTxs(int& nPackagesSelected, int& nDescendantsUpdated)
{
    //is in generateforbigboom block,need to pre-add coinbase tx weight first, some test will make block out of size.
    if (nHeight <= chainparams.GetConsensus().BigBoomHeight)
    {
        nBlockWeight += 37000;// TODO: coinbase is not init, cannot calc size here. this value get from a true bigboom coinbase tx.
    }

    int offset = pblock->vtx.size();
    // mapModifiedTx will store sorted packages after they are modified
    // because some of their txs are already in the block
    indexed_modified_transaction_set mapModifiedTx;
    // Keep track of entries that failed inclusion, to avoid duplicate work
    MCTxMemPool::setEntries failedTx;

    // Start by adding all descendants of previously added txs to mapModifiedTx
    // and modifying them for their already included ancestors
    UpdatePackagesForAdded(inBlock, mapModifiedTx);

    MakeBranchTxUTXO makeBTxHelper;

    MCTxMemPool::indexed_transaction_set::index<ancestor_score>::type::iterator mi = mempool.mapTx.get<ancestor_score>().begin();
    MCTxMemPool::txiter iter;

    // Limit the number of attempts to add transactions to the block when it is
    // close to full; this is just a simple heuristic to finish quickly if the
    // mempool has a lot of entries.
    const int64_t MAX_CONSECUTIVE_FAILURES = 1000;
    int64_t nConsecutiveFailed = 0;

    std::vector<const MCTxMemPoolEntry*> blockTxEntries;
    blockTxEntries.insert(blockTxEntries.end(), offset, nullptr);
    while (mi != mempool.mapTx.get<ancestor_score>().end() || !mapModifiedTx.empty()) {
        // First try to find a new transaction in mapTx to evaluate.
        if (mi != mempool.mapTx.get<ancestor_score>().end() &&
                SkipMapTxEntry(mempool.mapTx.project<0>(mi), mapModifiedTx, failedTx)) {
            ++mi;
            continue;
        }

        // Now that mi is not stale, determine which transaction to evaluate:
        // the next entry from mapTx, or the best from mapModifiedTx?
        bool fUsingModified = false;
        modtxscoreiter modit = mapModifiedTx.get<ancestor_score>().begin();
        if (mi == mempool.mapTx.get<ancestor_score>().end()) {
            // We're out of entries in mapTx; use the entry from mapModifiedTx
            iter = modit->iter;
            fUsingModified = true;
        }
        else {
            // Try to compare the mapTx entry to the mapModifiedTx entry
            iter = mempool.mapTx.project<0>(mi);
            if (modit != mapModifiedTx.get<ancestor_score>().end() &&
                CompareModifiedEntry()(*modit, MCTxMemPoolModifiedEntry(iter))) {
                // The best entry in mapModifiedTx has higher score
                // than the one from mapTx
                // Switch which transaction (package) to consider
                iter = modit->iter;
                fUsingModified = true;
            }
            else {
                // Either no entry in mapModifiedTx, or it's worse than mapTx.
                // Increment mi for the next loop iteration.
                ++mi;
            }
        }

        // the SyncBranchInfo tx has two type ancestors and descendants: 1 coins relation. 2 branch block connect relation.
        // OP: can we move to ancestors?
        const MCTransactionRef& iterTx = iter->GetSharedTx();
        if (iterTx->IsSyncBranchInfo()) {//one branch's header transactions should come to block vtx in order
            std::vector<uint256> ancestors = g_pBranchDataMemCache->GetAncestorsBlocksHash(*iterTx);
            bool hasAncestorFailed = false;
            for (std::vector<uint256>::reverse_iterator rit = ancestors.rbegin(); rit != ancestors.rend(); ++rit) {
                MCTxMemPool::txiter it = mempool.mapTx.find(*rit);
                if (it == mempool.mapTx.end())
                    continue;//error
                if (inBlock.count(it)) {// don't skip mapModifiedTx //original code SkipMapTxEntry(it, mapModifiedTx, failedTx)
                    continue;
                }
                if (failedTx.count(it)){// ancestors is fail, so cur
                    hasAncestorFailed = true;
                    break;// fail
                }
                if (!fUsingModified)
                    --mi;
                fUsingModified = mapModifiedTx.count(it);
                iter = it;
                break;// ok
            }
            if (hasAncestorFailed){
                if (fUsingModified){
                    mapModifiedTx.get<ancestor_score>().erase(modit);
                    failedTx.insert(iter);
                }
                continue;
            }
        }

        // We skip mapTx entries that are inBlock, and mapModifiedTx shouldn't
        // contain anything that is inBlock.
        assert(!inBlock.count(iter));

        uint64_t packageSize = iter->GetSizeWithAncestors();
        MCAmount packageFees = iter->GetModFeesWithAncestors();
        int64_t packageSigOpsCost = iter->GetSigOpCostWithAncestors();
        if (fUsingModified) {
            packageSize = modit->nSizeWithAncestors;
            packageFees = modit->nModFeesWithAncestors;
            packageSigOpsCost = modit->nSigOpCostWithAncestors;
        }
        if (packageFees < blockMinFeeRate.GetFee(packageSize)) {
            LogPrintf("%s:%d %d < %d\n", __FUNCTION__, __LINE__, packageFees, blockMinFeeRate.GetFee(packageSize));
			// Everything else we might consider has a lower fee rate
			break;
		}

		if (!TestPackage(iter, packageSize, packageSigOpsCost)) {
			if (fUsingModified) {
				// Since we always look at the best entry in mapModifiedTx,
				// we must erase failed entries so that we can consider the
				// next best entry on the next loop iteration
				mapModifiedTx.get<ancestor_score>().erase(modit);
				failedTx.insert(iter);
			}

			++nConsecutiveFailed;

			if (nConsecutiveFailed > MAX_CONSECUTIVE_FAILURES && nBlockWeight > nBlockMaxWeight - 4000) {
				// Give up if we're close to full and haven't succeeded in a while
				break;
			}
			continue;
		}

        MCTxMemPool::setEntries ancestors;
		uint64_t nNoLimit = std::numeric_limits<uint64_t>::max();
		std::string dummy;
		mempool.CalculateMemPoolAncestors(*iter, nullptr, ancestors, nNoLimit, nNoLimit, nNoLimit, nNoLimit, dummy, false);

		onlyUnconfirmed(ancestors);
		ancestors.insert(iter);

		// Test if all tx's are Final
		if (!TestPackageTransactions(ancestors)) {
			if (fUsingModified) {
				mapModifiedTx.get<ancestor_score>().erase(modit);
				failedTx.insert(iter);
			}
			continue;
		}

		// This transaction will make it in; reset the failed counter.
		nConsecutiveFailed = 0;

		// Package can be added. Sort the entries in a valid order.
        std::vector<MCTxMemPool::txiter> sortedEntries;
		SortForBlock(ancestors, iter, sortedEntries);

        bool fail = false;
        for (size_t i = 0; i < sortedEntries.size(); ++i) {
            MCTxMemPool::txiter entry = sortedEntries[i];
            const MCTransactionRef& entryTx = entry->GetSharedTx();

            if ((chainparams.IsMainChain() && entryTx->IsBranchChainTransStep2()) ||
                (entryTx->IsSmartContract() && entryTx->pContractData->contractCoinsOut.size() > 0)) {
                if (!UpdateIncompleteTx(entry, makeBTxHelper)) {
                    //++mi;
                    if (fUsingModified) {
                        mapModifiedTx.get<ancestor_score>().erase(modit);
                        failedTx.insert(entry);
                    }
                    fail = true;
                    break;//next
                }
            }
        }

        if (fail)
            continue;

        for (size_t i = 0; i < sortedEntries.size(); ++i) {
			AddToBlock(sortedEntries[i], makeBTxHelper);
			// Erase from the modified set, if present
			mapModifiedTx.erase(sortedEntries[i]);
            blockTxEntries.emplace_back(&*sortedEntries[i]);
		}

		++nPackagesSelected;

		// Update transactions that depend on each of these
		nDescendantsUpdated += UpdatePackagesForAdded(ancestors, mapModifiedTx);

        if (pblock->vtx.size() >= gArgs.GetArg("-maxtxnuminblock", std::numeric_limits<int64_t>::max())) {
            break;
        }
    }

    // 默认使用分片重新排列交易
    bool grouping = gArgs.GetBoolArg("-grouping", true);
    if (grouping) {
        GroupingTransaction(offset, blockTxEntries);
    }
    else {
        pblock->groupSize.emplace_back(pblock->vtx.size());
    }
}

void IncrementExtraNonce(MCBlock* pblock, const MCBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{
    // Update nExtraNonce
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock) {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;
    //unsigned int nHeight = pindexPrev->nHeight+1; // Height first in coinbase required for block.version=2
    //MCMutableTransaction txCoinbase(*pblock->vtx[0]);
    //txCoinbase.vin[0].scriptSig = (MCScript() << nHeight << CScriptNum(nExtraNonce)) + COINBASE_FLAGS;
    //assert(txCoinbase.vin[0].scriptSig.size() <= 100);

    //pblock->vtx[0] = MakeTransactionRef(std::move(txCoinbase));
    pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
}

//////////////////////////////////// modified for pcoin/////////////////////////////////


void static GenerateSleep()
{
	boost::this_thread::interruption_point();
}

void static MagnaChainMiner(const MCChainParams& chainparams)
{
    LogPrintf("MagnaChainMiner started\n");
	RenameThread("magnachain-miner");

	unsigned int nExtraNonce = 0;

    MCWallet* const pwallet = ::vpwallets[0];

	while (!ShutdownRequested()) {
		try {
            std::set<MCTxDestination> setAddress;
            std::vector<MCOutput> vecOutputs;
			{
				assert(pwallet != nullptr);

				LOCK2(cs_main, pwallet->cs_wallet);

                if (Params().IsMainChain())
                    pwallet->AvailableCoins(vecOutputs, nullptr, false);
                else
                    pwallet->AvailableMortgageCoins(vecOutputs, false);
				//for (const MCOutput& out : vecOutputs) {
				//	MCTxDestination address;
				//	const MCScript& scriptPubKey = out.tx->tx->vout[out.i].scriptPubKey;
				//	bool fValidAddress = ExtractDestination(scriptPubKey, address);

				//	if (setAddress.count(address))
				//		continue;

				//	if (fValidAddress) {
				//		if (!scriptPubKey.IsPayToScriptHash()) {
				//			setAddress.insert(address);
				//		}
				//	}
				//}

			}
            //std::vector< MCScript> vecScript;
            //BOOST_FOREACH(const MCTxDestination& addr, setAddress) {
			//	vecScript.push_back(GetScriptForDestination(addr));
			//}
			generateBlocks(pwallet, vecOutputs, vecOutputs.size(), vecOutputs.size(), true, GenerateSleep);

			// Check for stop or if block needs to be rebuilt
			boost::this_thread::interruption_point();

		}
		catch (const UniValue& objError) {
			UniValue err = find_value(objError, "message");
			LogPrintf(err.isStr() ? err.get_str().c_str() : "catch UniValue exception\n");
		}
		catch (const boost::thread_interrupted &e)
		{
			LogPrintf("MagnaChainMiner terminated for boost::thread_interrupted\n");
            //throw;
		}
		catch (const std::runtime_error &e)
		{
			LogPrintf("MagnaChainMiner runtime error: %s\n", e.what());
			//return;
		}
		catch (const std::exception& e)
		{
			LogPrintf("MagnaChainMiner std::exception error: %s\n", e.what());
			//return;
		}
	}
}

void GenerateMCs(bool fGenerate, int nThreads, const MCChainParams& chainparams)
{
	static boost::thread_group* minerThreads = NULL;

	if (nThreads < 0)
		nThreads = GetNumCores();

	if (minerThreads != NULL)
	{
		minerThreads->interrupt_all();
		delete minerThreads;
		minerThreads = NULL;
	}

	if (nThreads == 0 || !fGenerate)
		return;

	minerThreads = new boost::thread_group();
	for (int i = 0; i < nThreads; i++)
        minerThreads->create_thread(boost::bind(&MagnaChainMiner, boost::cref(chainparams)));
}


namespace BlockExplorer
{
    class Explorer
    {
    public:
        // not safe enough
        inline static bool FastCheckCoin(const Coin& coin, int iChainHeight, bool fNeedUnmature)
        {
            if (coin.IsCoinBase() && fNeedUnmature &&
                (iChainHeight - coin.nHeight) < COINBASE_MATURITY) {
                return false;
            }
            if (coin.nHeight > iChainHeight)
                return false;
            if (coin.IsSpent())
                return false;
            return true;
        }


        //static bool ScanBlocks(bool fNeedUnmatue, std::vector< const Coin*>& vecOutputs, std::string strAddr, bool fOnlyConfirmed = true)
        //{
        //	int iChainHeight = chainActive.Height();
        //	MCScript kScript = GetScriptForDestination(MagnaChainAddress(strAddr).Get());

        //	pcoinsTip->BatchLoad();
        //	MCCoinsMap& mapCoins = pcoinsTip->GetCacheCoins();
        //	for (MCCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end(); ++it ) {
        //		const Coin& coin = it->second.coin;
        //		if (FastCheckCoin(coin, iChainHeight, fNeedUnmatue)) {
        //			if( coin.out.scriptPubKey == kScript )
        //				vecOutputs.push_back( &coin);
        //		}
        //	}
        //	return true;
        //}

        //static void GetAvailableCoins(bool fNeedUnmature, std::vector<const MCOutPoint>& vecOutputs, std::string strAddr, bool fOnlyConfirmed = true)
        //{
        //	ScanBlocks(fNeedUnmature, vecOutputs, strAddr, fOnlyConfirmed);
        //}
        static inline CoinListPtr GetCoinList(const MCTxDestination& kDest)
        {
            const MCKeyID& kChild = boost::get<MCKeyID>(kDest);
            return pcoinListDb->GetList((const uint160&)kChild);
        }

        static inline CoinListPtr GetCoinList(const std::string& strAddr)
        {
            MagnaChainAddress kAddr(strAddr);
            MCTxDestination kDest = kAddr.Get();
            return GetCoinList(kDest);
        }

        static MCAmount CountAmount(CoinList& kList, bool fNeedUnmature)
        {
            MCAmount total = 0;
            int iChainHeight = chainActive.Height();

            BOOST_FOREACH (const MCOutPoint& op, kList.coins) {
                const Coin& coin = pcoinsTip->AccessCoin(op);
                if (FastCheckCoin(coin, iChainHeight, fNeedUnmature)) {
                    MCAmount v = coin.out.nValue;
                    total += v;
                }
            }
            return total;
        }

        static MCAmount GetUnspent(const uint160& kAddr)
        {
            CoinListPtr pList = pcoinListDb->GetList(kAddr);
            if (pList == nullptr)
                return 0;
            return CountAmount(*pList, true);
        }

        static MCAmount GetUnspent(const std::string& strAddr)
        {
            CoinListPtr pList = GetCoinList(strAddr);
            if (pList == nullptr)
                return 0;
            return CountAmount(*pList, true);
        }
    };
}

using namespace BlockExplorer;
const uint32_t iMaxWorkBits = 17760256;


MCAmount GetBlockSubsidy(int nHeight, const Consensus::Params& consensusParams)
{
	if (!Params().IsMainChain())
	{
		return 0;
	}

	int halvings = nHeight / consensusParams.nSubsidyHalvingInterval;
	// Force block reward to zero when right shift is undefined.
    if (halvings >= 64)
        return 0;

    //178*COIN * nSubsidyHalvingInterval = 1.495199999454e+17
    MCAmount nSubsidy = 85 * (COIN); // 50 * COIN;
    // Subsidy is cut in half every 210,000 blocks which will occur approximately every 4 years.
    nSubsidy >>= halvings;

    if (nHeight <= consensusParams.BigBoomHeight) {
        nSubsidy += consensusParams.BigBoomValue;
    }
    return nSubsidy;
}

/*
class TestMiner
{
public:
	TestMiner()
	{
		MCAmount total = 0;
		Consensus::Params consensus;
		consensus.nSubsidyHalvingInterval = 210000 * 5;
		consensus.BigBoomHeight = 1000;
		consensus.BigBoomValue = 20000 * COIN;

		for (int i = 0; i < consensus.nSubsidyHalvingInterval * 165; ++i)
		{
			total += GetBlockSubsidy( i, consensus);
		}
		uint64_t iCoin = total / COIN;

	}
}t;
*/

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
	return true;
	//bool fNegative;
	//bool fOverflow;
	//arith_uint256 bnTarget;

	//bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

	//// Check range
	//if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
	//	return false;

	//// Check proof of work matches claimed amount
	//if (UintToArith256(hash) > bnTarget)
	//	return false;

	//return true;
}

static uint256 guMaxWork = uint256S("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

// 如有修改,同时也需修改 GetBlockHeaderWork
uint32_t GetBlockWork(const MCBlock& block, const MCOutPoint& out, uint256& block_hash)
{
	block_hash  = guMaxWork;
	BlockMap::iterator mi = mapBlockIndex.find(block.hashPrevBlock);
	if (mi == mapBlockIndex.end())
	{
		return 0;
	}
    MCBlockIndex* pPreIndex = mi->second;
	const int iPrevHeight = pPreIndex->nHeight;
	//const int iTop = chainActive.Height();

	const bool isBigBoom = iPrevHeight < Params().GetConsensus().BigBoomHeight;

	const int iMatureDepth = COINBASE_MATURITY - 1;
    MCAmount total = 0;

	// 取得矿工当前可用的UTXO
    MCTxDestination kDest;
	ExtractDestination(block.vtx[0]->vout[0].scriptPubKey, kDest);

	if ( isBigBoom )
		total = 0;
    else if (!Params().IsMainChain() && iPrevHeight == 0)//侧链第2个块,即传世块后的第1个块
    {
        if (block.vtx.size() <= 2)
            return 0;
        const MCTransaction& tx = *block.vtx[1];
        std::vector<MCTransactionRef>::const_iterator itFound = std::find_if(block.vtx.begin(), block.vtx.end(), [&tx](const MCTransactionRef& ptx) { return ptx->GetHash() == tx.vin[0].prevout.hash; });
        if (itFound != block.vtx.end()){
            int iRun = 1;
            total = ((*itFound)->vout[0].nValue / COIN) * iRun;
        }
        else
            return 0;
    }
	else {
		Coin coin;
		if (pcoinsTip->GetCoin(out, coin)) {
            MCAmount v = coin.out.nValue;
			// 计算深度时从上一次挖矿的时候开始算，同时要减去一个成熟时间
			int iHeight = coin.nHeight;
			iHeight += iMatureDepth;
            if (!Params().IsMainChain() && iPrevHeight < 2*COINBASE_MATURITY) {// 侧链前 2 COINBASE_MATURITY 块的高度处理
                iHeight = -COINBASE_MATURITY;
                iHeight = std::min(iHeight, COINBASE_MATURITY);
            }
			int iRun = iPrevHeight - iHeight;
			if (iRun > 0) {
				total += (v / COIN) * iRun;
			}
		}
	}

	if (total >= std::numeric_limits<uint32_t>::max()) {
		total = std::numeric_limits<uint32_t>::max();
	}

	// 计算前100个区块的平均值
	{
		MCAmount iAvg = 0;
		MCBlockIndex *ptest = pPreIndex;
		int i = 0;
		for ( ;i < 100; ++i)
		{
			if (ptest == NULL)
			{
				break;
			}
			iAvg += ptest->nNonce;

			ptest = ptest->pprev;
		}
		iAvg /= i;
		if ( iAvg > 100 && total > iAvg + iAvg /20 ) {
			total = iAvg + iAvg / 20;
		}
	//	LogPrintf("%s: Amount before avg: %d after change: %d,  avg: %d \n", __func__, iBeforeAvg, total, iAvg );
	}

	// 计算HASH

	int nType = SER_GETHASH;
	int nVersion = PROTOCOL_VERSION;
    MCHashWriter sheader(nType, nVersion);
    MCHashWriter snum(nType, nVersion);

	int iCheck2 = 2;
	int iCheck3 = 3;
    MCBlockIndex* pNextIndex = pPreIndex;
	for (int i = 0; i < 1000; ++i)
	{
		if (pNextIndex == NULL)
		{
			break;
		}
		if (i == iCheck2)
		{
			iCheck2 *= 2;
			sheader << pNextIndex->GetBlockHash();
		}
		if (i == iCheck3)
		{
			iCheck3 *= 3;
			snum << pNextIndex->GetBlockHash();
		}

        pNextIndex = pNextIndex->pprev;
	}

    if (kDest.type() != typeid(MCKeyID))
    {
        LogPrintf("%s: Mine out key type invalid \n", __func__);
        return 0;
    }
    MCKeyID kKey = boost::get<MCKeyID>(kDest);
	sheader << (uint160)kKey;
	sheader << out.hash;
	sheader << out.n;

	snum << (uint160)kKey;
	block_hash = sheader.GetHash();
	uint256 num_hash = snum.GetHash();

	// percent 
	uint64_t iPercent = num_hash.GetCheapHash() % 100;
    MCAmount iMount = total * iPercent / 100;
	//LogPrintf("%s: before change work %s \n", __func__, block_hash.GetHex());

	arith_uint256 iTmp = UintToArith256(block_hash);

	// 计算经过币龄加权之后的HASH
	if (iMount > 1 && iMount <= total)
		iTmp /= iMount;
	if (isBigBoom) {
		iTmp /= 16;
	}
	else {
		if (iMount == 0) {
			block_hash = guMaxWork;
			LogPrintf("%s: block work To MAX WORK, Amount is 0, total %d \n", __func__, total);
			return 0;
		}
	}
//	LogPrintf("%s: block work %s , Amount:%d \n", __func__, iTmp.GetHex(), iMount);

	uint32_t iComp = iTmp.GetCompact();
	iTmp.SetCompact(iComp);
	block_hash = ArithToUint256(iTmp);

//	LogPrintf("%s: Compat block work %s \n", __func__, iTmp.GetHex());

	return total;
}

bool CheckBlockWork(const MCBlock& block, MCValidationState& state, const Consensus::Params& consensusParams)
{
	uint256 hash;
	uint32_t iBlockWork = GetBlockWork(block, block.prevoutStake, hash);

	// check
	bool fNegative;
	bool fOverflow;
	arith_uint256 bnTarget;

	bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);

	//// Check range
	if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256( consensusParams.powLimit))
		return state.DoS(0, false, REJECT_INVALID, "CheckBlockWork fail, bnTarget error");

    if (UintToArith256(hash) > bnTarget) {
        // only for -reindex, -reindexchainstate
        if (mapBlockIndex.size() == 1 && mapBlockIndex.count(block.GetHash()) > 0 && block.GetHash() == Params().GenesisBlock().GetHash()) {
            return true;
        }
        else {
            return state.DoS(0, false, REJECT_INVALID, "CheckBlockWork fail, UintToArith256(hash) > bnTarget");
        }
    }

    if (iBlockWork != block.nNonce) {
        return state.DoS(0, false, REJECT_INVALID, "CheckBlockWork fail, iBlockWork != block.nNonce");;
    }

	return true;
}

////---------------------------------------------------------
inline const BranchBlockData* GetBranchBlockData(BranchData& branchdata, const uint256 &blockhash, const uint256 &branchhash, BranchCache *pBranchCache){
    if (branchdata.mapHeads.count(blockhash))
        return &branchdata.mapHeads[blockhash];
    if (pBranchCache)// is get data from mempool
        return pBranchCache->GetBranchBlockData(branchhash, blockhash);
    return nullptr;
}

////---------------------------------------------------------
//主链获取侧链头工作量
//核心算法需要和 GetBlockWork 一致
uint32_t GetBlockHeaderWork(const MCBranchBlockInfo& block, uint256& block_hash, const MCChainParams &params, BranchData& branchdata, BranchCache *pBranchCache, MCCoinsViewCache* pCoins)
{
    ///// get and check data
    const MCOutPoint& out = block.prevoutStake;
    MCDataStream cds(block.vchStakeTxData, SER_NETWORK, INIT_PROTO_VERSION);
    MCTransactionRef ptx;
    cds >> ptx;
    MCKeyID kKey;
    uint256 fromTxHash;
    int64_t iPreCoinHeigh;
    if (!GetMortgageCoinData(ptx->vout[0].scriptPubKey, &fromTxHash, &kKey, &iPreCoinHeigh))
        return 0;
    const MCAmount coinValue = ptx->vout[0].nValue;

    //挖矿币-找出抵押币并作相应验证
    const Coin& fromCoin = pCoins->AccessCoin(MCOutPoint(fromTxHash, 0));
    {
        if (fromCoin.IsSpent() || coinValue != fromCoin.out.nValue) 
            return 0;

        MCKeyID kKeyId;
        uint256 branchid;
        int64_t presetHeight;
        if (!GetMortgageMineData(fromCoin.out.scriptPubKey, &branchid, &kKeyId, &presetHeight))
            return 0;
        if (params.GetBranchHash() != branchid)
            return 0;
        if (kKey != kKeyId)
            return 0;
    }

    ///// calc block work
    block_hash = guMaxWork;
    
    const BranchBlockData* pPreIndex = GetBranchBlockData(branchdata, block.hashPrevBlock, params.GetBranchHash(), pBranchCache);
    if (pPreIndex == nullptr)
        return 0;

    const int iPrevHeight = pPreIndex->nHeight;
    if (block.blockHeight != iPrevHeight + 1)
        return 0;

    const bool isBigBoom = iPrevHeight < params.GetConsensus().BigBoomHeight;

    const int iMatureDepth = COINBASE_MATURITY - 1;
    MCAmount total = 0;

    if (isBigBoom)
        total = 0;
    else if (!params.IsMainChain() && iPrevHeight == 0)//侧链第2个块,即传世块后的第1个块
    {
        int iRun = 1;
        total = (coinValue / COIN) * iRun;
    }
    else {
        MCAmount v = coinValue;
        // 计算深度时从上一次挖矿的时候开始算，同时要减去一个成熟时间
        int iHeight = iPreCoinHeigh;
        iHeight += iMatureDepth;
        if (!params.IsMainChain() && iPrevHeight < 2 * COINBASE_MATURITY) {// 侧链前 COINBASE_MATURITY 块的高度处理
            iHeight = -COINBASE_MATURITY;
        }
        int iRun = iPrevHeight - iHeight;
        if (iRun > 0) {
            total += (v / COIN) * iRun;
        }
    }

    if (total >= std::numeric_limits<uint32_t>::max()) {
        total = std::numeric_limits<uint32_t>::max();
    }

    // 计算前100个区块的平均值
    {
        MCAmount iAvg = 0;
        const BranchBlockData* ptest = pPreIndex;
        int i = 0;
        for (; i < 100; ++i)
        {
            if (ptest == NULL)
            {
                break;
            }
            iAvg += ptest->header.nNonce;

            ptest = GetBranchBlockData(branchdata, ptest->header.hashPrevBlock, params.GetBranchHash(), pBranchCache);
        }
        iAvg /= i;
        if (iAvg > 100 && total > iAvg + iAvg / 20) {
            total = iAvg + iAvg / 20;
        }
    }

    // 计算HASH
    int nType = SER_GETHASH;
    int nVersion = PROTOCOL_VERSION;
    MCHashWriter sheader(nType, nVersion);
    MCHashWriter snum(nType, nVersion);

    int iCheck2 = 2;
    int iCheck3 = 3;
    const BranchBlockData* pNextIndex = pPreIndex;
    for (int i = 0; i < 1000; ++i)
    {
        if (pNextIndex == NULL)
        {
            break;
        }
        if (i == iCheck2)
        {
            iCheck2 *= 2;
            sheader << pNextIndex->header.GetHash();
        }
        if (i == iCheck3)
        {
            iCheck3 *= 3;
            snum << pNextIndex->header.GetHash();
        }

        pNextIndex = GetBranchBlockData(branchdata, pNextIndex->header.hashPrevBlock, params.GetBranchHash(), pBranchCache);
    }

    sheader << (uint160)kKey;
    sheader << out.hash;
    sheader << out.n;

    snum << (uint160)kKey;
    block_hash = sheader.GetHash();
    uint256 num_hash = snum.GetHash();

    // percent 
    uint64_t iPercent = num_hash.GetCheapHash() % 100;
    MCAmount iMount = total * iPercent / 100;
    arith_uint256 iTmp = UintToArith256(block_hash);

    // 计算经过币龄加权之后的HASH
    if (iMount > 1 && iMount <= total)
        iTmp /= iMount;
    if (isBigBoom) {
        iTmp /= 16;
    }
    else {
        if (iMount == 0) {
            block_hash = guMaxWork;
            LogPrintf("%s: block work To MAX WORK, Amount is 0, total %d \n", __func__, total);
            return 0;
        }
    }

    uint32_t iComp = iTmp.GetCompact();
    iTmp.SetCompact(iComp);
    block_hash = ArithToUint256(iTmp);

    return total;
}

////---------------------------------------------------------
//主链检查侧链头工作量
//核心和 CheckBlockWork 相同
bool CheckBlockHeaderWork(const MCBranchBlockInfo& block, MCValidationState& state, const MCChainParams &params, BranchData& branchdata, BranchCache *pBranchCache, MCCoinsViewCache* pCoins)
{
    const Consensus::Params& consensusParams = params.GetConsensus();

    uint256 hash;
    uint32_t iAmount = GetBlockHeaderWork(block, hash, params, branchdata, pBranchCache, pCoins);

    // check
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);

    //// Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(consensusParams.powLimit))
        return state.DoS(100, false, REJECT_INVALID, "CheckBlockHeaderWork fail, range check fail");;

    if (UintToArith256(hash) > bnTarget)
        return state.DoS(100, false, REJECT_INVALID, "CheckBlockHeaderWork fail, target check fail");;

    if (iAmount != block.nNonce)
        return state.DoS(100, false, REJECT_INVALID, "CheckBlockHeaderWork fail, nonce check fail");;
    return true;
}

bool ContextualCheckBlockHeader(const MCBlockHeader& block, MCValidationState& state, const MCChainParams& params, const MCBlockIndex* pindexPrev, int64_t nAdjustedTime)
{
	assert(pindexPrev != nullptr);
	const int nHeight = pindexPrev->nHeight + 1;

	// Check proof of work
	const Consensus::Params& consensusParams = params.GetConsensus();
	// if (block.nBits != GetNextWorkRequired(pindexPrev, &block, consensusParams))
	// return state.DoS(100, false, REJECT_INVALID, "bad-diffbits", false, "incorrect proof of work");
	arith_uint256 nBlockWork;
	bool fNegative;
	bool fOverflow;
	nBlockWork.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || nBlockWork == 0)
		return state.DoS(100, false, REJECT_INVALID, "bad-diffbits of block work", false, "incorrect proof of work");

	arith_uint256 nWorkdRequired;
	nWorkdRequired.SetCompact(GetNextWorkRequired(pindexPrev, &block, consensusParams), &fNegative, &fOverflow);
	if (fNegative || fOverflow || nWorkdRequired == 0)
		return state.DoS(100, false, REJECT_INVALID, "bad-diffbits of required work", false, "incorrect proof of work");

    if (nBlockWork > nWorkdRequired) {
		return state.DoS(100, false, REJECT_INVALID, "bad-diffbits", false, "incorrect proof of work");
	}

	// Check against checkpoints
	if (fCheckpointsEnabled) {
		// Don't accept any forks from the main chain prior to last checkpoint.
		// GetLastCheckpoint finds the last checkpoint in MapCheckpoints that's in our
		// MapBlockIndex.
        MCBlockIndex* pcheckpoint = Checkpoints::GetLastCheckpoint(params.Checkpoints());
		if (pcheckpoint && nHeight < pcheckpoint->nHeight)
			return state.DoS(100, error("%s: forked chain older than last checkpoint (height %d)", __func__, nHeight), REJECT_CHECKPOINT, "bad-fork-prior-to-checkpoint");
        const uint256* pcheckhash = params.GetCheckpointHeightHash(nHeight);
        if (pcheckhash && block.GetHash() != *pcheckhash)
        {
            return state.DoS(100, error("%s: forked chain older than last checkpoint2 (height %d)", __func__, nHeight), REJECT_CHECKPOINT, "bad-fork-prior-to-checkpoint2");
        }
	}

	// Check timestamp against prev
	if (block.GetBlockTime() <= pindexPrev->GetMedianTimePast())
		return state.Invalid(false, REJECT_INVALID, "time-too-old", "block's timestamp is too early");

	// Check timestamp
	if (block.GetBlockTime() > nAdjustedTime + MAX_FUTURE_BLOCK_TIME)
		return state.Invalid(false, REJECT_INVALID, "time-too-new", "block timestamp too far in the future");

	// Reject outdated version blocks when 95% (75% on testnet) of the network has upgraded:
	// check for version 2, 3 and 4 upgrades
	if ((block.nVersion < 2 && nHeight >= consensusParams.BIP34Height) ||
		(block.nVersion < 3 && nHeight >= consensusParams.BIP66Height) ||
		(block.nVersion < 4 && nHeight >= consensusParams.BIP65Height))
		return state.Invalid(false, REJECT_OBSOLETE, strprintf("bad-version(0x%08x)", block.nVersion),
			strprintf("rejected nVersion=0x%08x block", block.nVersion));

	return true;
}

unsigned int GetNextWorkRequired(const MCBlockIndex* pindexLast, const MCBlockHeader* pblock, const Consensus::Params& params)
{
	unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

	// Genesis block
	if (pindexLast == NULL)
		return nProofOfWorkLimit;

	// time limit
	if (pblock->nTime < pindexLast->nTime)
		return iMaxWorkBits;
	if (pblock->nTime - pindexLast->nTime < params.nPowTargetSpacing)
		return iMaxWorkBits;

	uint32_t iRunTime = 1;
    if (pblock->nTime - pindexLast->nTime > params.nPowTargetSpacing * 2) {
        iRunTime = pblock->nTime - pindexLast->nTime - params.nPowTargetSpacing * 2;
        if (iRunTime > 60 * 60 * 24 * 10)
            iRunTime = 60 * 60 * 24 * 10;
        iRunTime /= 10;
        if (iRunTime < 1)
            iRunTime = 1;
    }


    bool fNegative;
    bool fOverflow;
    uint512 nMostWork;

    int iCount = 0;
    for (int i = 0; i < 100; ++i) {
        if (pindexLast == NULL)
            break;
        arith_uint256 nWork;
        nWork.SetCompact(pindexLast->nBits, &fNegative, &fOverflow);
        if (!fNegative && !fOverflow && nWork > 0) {
            uint512 tmp;
            tmp.SetHex(nWork.GetHex());
            nMostWork += tmp;
            iCount++;
        }
        pindexLast = pindexLast->pprev;
    }
    nMostWork /= iCount;

    //	LogPrintf("%s: Average Target %s , count :%d \n", __func__, nMostWork.GetHex(), iCount );

    nMostWork *= 2 * iRunTime;
    arith_uint256 nTarget;
    uint512 uPowLimit;
    uPowLimit.SetHex(params.powLimit.GetHex());
    if (nMostWork > uPowLimit) {
        nTarget = UintToArith256(params.powLimit);
    } else {
        nTarget.SetHex(nMostWork.GetHex());
    }

    //	LogPrintf("%s: target %s , RunTime:%d \n", __func__, nTarget.GetHex(), iRunTime);
    //	LogPrintf("%s: Compat target %s \n", __func__, arith_uint256().SetCompact(nTarget.GetCompact()).GetHex());

    return nTarget.GetCompact();
}
////---------------------------------------------------------
//主链上获取侧链的nextwork
unsigned int GetBranchNextWorkRequired(const BranchBlockData* pindexLast, const MCBlockHeader* pblock, const MCChainParams& params, BranchData &branchdata, BranchCache *pBranchCache)
{
    const Consensus::Params& consensusParams = params.GetConsensus();
    unsigned int nProofOfWorkLimit = UintToArith256(consensusParams.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // time limit
    if (pblock->nTime < pindexLast->header.nTime)
        return iMaxWorkBits;
    if (pblock->nTime - pindexLast->header.nTime < consensusParams.nPowTargetSpacing)
        return iMaxWorkBits;

    uint32_t iRunTime = 1;
    if (pblock->nTime - pindexLast->header.nTime > consensusParams.nPowTargetSpacing * 2) {
        iRunTime = pblock->nTime - pindexLast->header.nTime - consensusParams.nPowTargetSpacing * 2;
        if (iRunTime > 60 * 60 * 24 * 10)
            iRunTime = 60 * 60 * 24 * 10;
        iRunTime /= 10;
        if (iRunTime < 1)
            iRunTime = 1;
    }

    bool fNegative;
    bool fOverflow;
    uint512 nMostWork;

    int iCount = 0;
    for (int i = 0; i < 100; ++i) {
        if (pindexLast == NULL)
            break;
        arith_uint256 nWork;
        nWork.SetCompact(pindexLast->header.nBits, &fNegative, &fOverflow);
        if (!fNegative && !fOverflow && nWork > 0) {
            uint512 tmp;
            tmp.SetHex(nWork.GetHex());
            nMostWork += tmp;
            iCount++;
        }
        pindexLast = GetBranchBlockData(branchdata, pindexLast->header.hashPrevBlock, params.GetBranchHash(), pBranchCache);
    }
    nMostWork /= iCount;

    nMostWork *= 2 * iRunTime;
    arith_uint256 nTarget;
    uint512 uPowLimit;
    uPowLimit.SetHex(consensusParams.powLimit.GetHex());
    if (nMostWork > uPowLimit) {
        nTarget = UintToArith256(consensusParams.powLimit);
    }
    else {
        nTarget.SetHex(nMostWork.GetHex());
    }

    return nTarget.GetCompact();
}
////---------------------------------------------------------
//主链上验证侧链
bool BranchContextualCheckBlockHeader(const MCBlockHeader& block, MCValidationState& state, const MCChainParams& params, BranchData &branchdata, 
    int64_t nAdjustedTime, BranchCache *pBranchCache)
{
    const BranchBlockData* pindexPrev = GetBranchBlockData(branchdata, block.hashPrevBlock, params.GetBranchHash(), pBranchCache);
    if (pindexPrev == nullptr)
        return state.DoS(0, false, REJECT_INVALID, "bad-diffbits of block work, pindexPrev is null");// sync branch transaction maybe in disorder

    // Check proof of work
    arith_uint256 nBlockWork;
    bool fNegative;
    bool fOverflow;
    nBlockWork.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || nBlockWork == 0)
        return state.DoS(100, false, REJECT_INVALID, "bad-diffbits of block work, nBlockWork error!");

    arith_uint256 nWorkdRequired;
    nWorkdRequired.SetCompact(GetBranchNextWorkRequired(pindexPrev, &block, params, branchdata, pBranchCache), &fNegative, &fOverflow);
    if (fNegative || fOverflow || nWorkdRequired == 0)
        return state.DoS(100, false, REJECT_INVALID, "bad-diffbits of required work, nWorkedRequired error!");

    if (nBlockWork > nWorkdRequired) {
        return state.DoS(100, false, REJECT_INVALID, "bad-diffbits, incorrect proof of work");
    }

    // Check against checkpoints
    //...

    // Check timestamp against prev
    // GetMedianTimePast
    int64_t mediantime = std::numeric_limits<int64_t>::max();
    {
        const int nMedianTimeSpan = 11;
        int64_t pmedian[nMedianTimeSpan];
        int64_t* pbegin = &pmedian[nMedianTimeSpan];
        int64_t* pend = &pmedian[nMedianTimeSpan];

        const BranchBlockData* pindex = pindexPrev;
        for (int i = 0; i < nMedianTimeSpan && pindex; ){
            *(--pbegin) = (int64_t)pindex->header.nTime;

            pindex = GetBranchBlockData(branchdata, pindex->header.hashPrevBlock, params.GetBranchHash(), pBranchCache);
            i++;
        }
        std::sort(pbegin, pend);
        mediantime = pbegin[(pend - pbegin) / 2];
    }

    if (block.GetBlockTime() <= mediantime)
        return state.Invalid(false, REJECT_INVALID, "time-too-old", "block's timestamp is too early");

    // Check timestamp
    if (block.GetBlockTime() > nAdjustedTime + MAX_FUTURE_BLOCK_TIME)
        return state.Invalid(false, REJECT_INVALID, "time-too-new", "block timestamp too far in the future");

    return true;
}

////---------------------------------------------------------
bool SignatureCoinbaseTransaction(int nHeight, const MCKeyStore* keystoreIn, MCMutableTransaction& txNew, MCAmount nValue, const MCScript& scriptPubKey)
{
    if (keystoreIn == nullptr)
        return false;

    int nIn = 0;
    SignatureData sigdata;

    txNew.vin[0].scriptSig = MCScript() << nHeight << OP_0;

    MCTransaction txNewConst(txNew);
    if (!ProduceSignature(TransactionSignatureCreator(keystoreIn, &txNewConst, 0, nValue, SIGHASH_ALL), scriptPubKey, sigdata)) {
        return false;
    } else {
        txNew.vin[0].scriptSig = txNew.vin[0].scriptSig + sigdata.scriptSig;
        return true;
    }
}


bool CheckCoinbaseSignature(int nHeight, const MCTransaction& t)
{
    if (t.vout.size() == 0)
        return false;
    MCMutableTransaction tx(t);
    MCScript kScriptPubKey = tx.vout[0].scriptPubKey;

    MCAmount total = 0;
    for (size_t i = 0; i < tx.vout.size(); ++i) {
        total += tx.vout[i].nValue;
    }
    CScriptWitness kDummyScript;

    MCScript scriptSig = tx.vin[0].scriptSig;
    MCScript kScriptHead = MCScript() << nHeight << OP_0;
    scriptSig.FindAndDelete(kScriptHead);
    tx.vin[0].scriptSig = kScriptHead;
    MCTransaction newTx(tx);
    TransactionSignatureChecker kCheck(&newTx, 0, total);

    bool bRet = VerifyScript(scriptSig, kScriptPubKey, &kDummyScript, STANDARD_SCRIPT_VERIFY_FLAGS, kCheck);
    return bRet;
}

/*
static MagnaChainAddress kdevAddrs[] = {
    "XCNW2QxKePZieDEhC549sX819Tf3PXi54A",
    "XBuJzqnbNtispDyANSi6v6N7TD4RTk7W8F",
    "XCKSDkWueBHiREeu22v6Ct8e3qjFrtRc8u",
};
*/

bool MakeStakeTransaction(const MCKeyStore& keystore, MCMutableTransaction& mtx, MCOutPoint& preout, MCCoinsViewCache *pcoinsCache, const int blockheight)
{
    if (pcoinsCache == nullptr)
    {
        pcoinsCache = ::pcoinsTip;
    }

    Coin theCoin;
	if (!pcoinsCache->GetCoin(preout, theCoin)) {
		return false;
	}

	mtx.vin.resize(1);
	mtx.vin[0].prevout = preout;

	mtx.vout.resize(1);
	mtx.vout[0] = theCoin.out;

	int nHashType = SIGHASH_ALL;
	bool fHashSingle = ((nHashType & ~SIGHASH_ANYONECANPAY) == SIGHASH_SINGLE);

	// Sign what we can:
	for (unsigned int i = 0; i < mtx.vin.size(); i++) {
        MCTxIn& txin = mtx.vin[i];
		const Coin& coin = pcoinsCache->AccessCoin(txin.prevout);
		if (coin.IsSpent()) {
			return false;
		}

        const MCScript& prevPubKey = coin.out.scriptPubKey;
        if (!Params().IsMainChain())// 侧链使用挖矿币,挖矿币脚本记录挖矿币相关数据
        {
            uint256 fromtxid;
            MCKeyID keyid;
            int64_t precoinheight;
            if (!GetMortgageCoinData(prevPubKey, &fromtxid, &keyid, &precoinheight))
                return false;
            mtx.vout[0].scriptPubKey = MCScript() << OP_MINE_BRANCH_COIN << ToByteVector(fromtxid) << coin.nHeight << OP_2DROP << OP_DUP << OP_HASH160 << ToByteVector(keyid) << OP_EQUALVERIFY << OP_CHECKSIG;
        }
		const MCAmount& amount = coin.out.nValue;
        
        const MCTransaction txConst(mtx);

		SignatureData sigdata;
		// Only sign SIGHASH_SINGLE if there's a corresponding output:
		if (!fHashSingle || (i < mtx.vout.size()))
			ProduceSignature(MutableTransactionSignatureCreator(&keystore, &mtx, i, amount, nHashType), prevPubKey, sigdata);
		sigdata = CombineSignatures(prevPubKey, TransactionSignatureChecker(&txConst, i, amount), sigdata, DataFromTransaction(mtx, i));

		UpdateTransaction(mtx, i, sigdata);

		ScriptError serror = SCRIPT_ERR_OK;
		if (!VerifyScript(txin.scriptSig, prevPubKey, &txin.scriptWitness, STANDARD_SCRIPT_VERIFY_FLAGS, TransactionSignatureChecker(&txConst, i, amount), &serror)) {
			return false;
		}
	}

	return true;
}

MCAmount MakeCoinbaseTransaction(MCMutableTransaction& coinbaseTx, MCAmount nFees, MCBlockIndex* pindexPrev, const MCScript& scriptPubKeyIn, const MCChainParams& chainparams)
{
    int nHeight = pindexPrev->nHeight + 1;

    // calc mining reward
    MCTxDestination kMinerDest;
    ExtractDestination(scriptPubKeyIn, kMinerDest);
    std::string strMineAddr = MagnaChainAddress(kMinerDest).ToString();
    LogPrint(BCLog::MINING, "MakeCoinbaseTransaction: miner address : %s \n", strMineAddr);
    MCAmount kReward = GetBlockSubsidy(nHeight, chainparams.GetConsensus());
    MCAmount kMinReward = kReward;

    // Create coinbase transaction.
    coinbaseTx.vin.resize(1);
    coinbaseTx.vin[0].prevout.SetNull();
    coinbaseTx.vin[0].scriptSig = MCScript() << nHeight << OP_0;

    // split output when outcoin too large, this use for big boom stage
    // because big boom coin well make block work too big
    // 在gen big boom的时候，把大额的币分成小份
    if (chainparams.IsMainChain() && nHeight <= chainparams.GetConsensus().BigBoomHeight)
    {
        const MCAmount minAmount = (10000 * COIN);
        MCAmount total = nFees + kMinReward;
        int splitsize = total / minAmount;
        splitsize = std::max(splitsize, 1);
        coinbaseTx.vout.resize(splitsize);
        for (int i=0; i< splitsize; i++)
        {
            coinbaseTx.vout[i].scriptPubKey = scriptPubKeyIn;
            if (i == splitsize - 1){
                coinbaseTx.vout[i].nValue = total - minAmount * (splitsize - 1);
            }
            else {
                coinbaseTx.vout[i].nValue = minAmount;
            }
        }
    }
    else{
        coinbaseTx.vout.resize(1);
        coinbaseTx.vout[0].scriptPubKey = scriptPubKeyIn;
        coinbaseTx.vout[0].nValue = nFees + kMinReward; //nFees + GetBlockSubsidy(nHeight, chainparams.GetConsensus());
    }

    return kReward + nFees;
}


bool CheckCoinbaseTx(const MCBlock& block, MCBlockIndex* pindex, MCAmount nFees, const MCChainParams& chainparams)
{
    MCBlock kStub(block);
    const MCTransaction& tx = *kStub.vtx[0];
    if (tx.vout.size() == 0)
        return false;
    MCMutableTransaction coinbaseTx;
    MakeCoinbaseTransaction(coinbaseTx, nFees, pindex->pprev, tx.vout[0].scriptPubKey, chainparams);
    kStub.vtx[0] = MakeTransactionRef(std::move(coinbaseTx));
    GenerateCoinbaseCommitment(kStub, pindex->pprev, chainparams.GetConsensus());

    const MCTransaction& checkTx = *kStub.vtx[0];
    if (checkTx.vout.size() != tx.vout.size()) {
        LogPrintf("%s : chech vout size:%d tx vout size: %d", __func__, checkTx.vout.size(), tx.vout.size());
        return false;
    }
    for (size_t i = 0; i < tx.vout.size(); ++i) {
        if (checkTx.vout[i].scriptPubKey != tx.vout[i].scriptPubKey) {
            LogPrintf("%s : %d script pub check fail", __func__, i);
            return false;
        }
        if (checkTx.vout[i].nValue != tx.vout[i].nValue) {
            LogPrintf("%s : %d nValue check fail", __func__, i);
            return false;
        }
    }
    return true;
}


std::unique_ptr<MCBlockTemplate> BlockAssembler::CreateNewBlock(const MCScript& scriptPubKeyIn, ContractContext* pContractContext, bool fMineWitnessTx, const MCKeyStore* keystoreIn, MCCoinsViewCache *pcoinsCache)
{
    if (pcoinsCache == nullptr)
    {
        pcoinsCache = ::pcoinsTip;
    }
	int64_t nTimeStart = GetTimeMicros();

	resetBlock();

	pblocktemplate.reset(new MCBlockTemplate());

	if (!pblocktemplate.get())
		return nullptr;
	pblock = &pblocktemplate->block; // pointer for convenience

	// Add dummy coinbase tx as first transaction
	pblock->vtx.emplace_back();
    pblocktemplate->vTxFees.push_back(-1);       // updated at end
    pblocktemplate->vTxSigOpsCost.push_back(-1); // updated at end

    LOCK2(cs_main, mempool.cs);

    MCBlockIndex* pindexPrev = chainActive.Tip();
    nHeight = pindexPrev->nHeight + 1;
    if (!this->outpoint.IsNull()) {
        MCMutableTransaction stakeTx;
        stakeTx.nVersion = MCTransaction::STAKE;
        if (!MakeStakeTransaction(*keystoreIn, stakeTx, this->outpoint, pcoinsCache, nHeight)) {
            return nullptr;
        }

        // set outpoint
        pblock->prevoutStake = outpoint;

        pblocktemplate->vTxFees.push_back(-1);       // updated at end
        pblocktemplate->vTxSigOpsCost.push_back(-1); // updated at end
        pblock->vtx.emplace_back(MakeTransactionRef(std::move(stakeTx)));
    }

	pblock->nVersion = ComputeBlockVersion(pindexPrev, chainparams.GetConsensus());
	// -regtest only: allow overriding block.nVersion with
	// -blockversion=N to test forking scenarios
	if (chainparams.MineBlocksOnDemand())
		pblock->nVersion = gArgs.GetArg("-blockversion", pblock->nVersion);

	pblock->nTime = GetAdjustedTime();
	const int64_t nMedianTimePast = pindexPrev->GetMedianTimePast();

	nLockTimeCutoff = (STANDARD_LOCKTIME_VERIFY_FLAGS & LOCKTIME_MEDIAN_TIME_PAST)
		? nMedianTimePast
		: pblock->GetBlockTime();

	// Decide whether to include witness transactions
	// This is only needed in case the witness softfork activation is reverted
	// (which would require a very deep reorganization) or when
	// -promiscuousmempoolflags is used.
	// TODO: replace this with a call to main to assess validity of a mempool
	// transaction (which in most cases can be a no-op).
	fIncludeWitness = IsWitnessEnabled(pindexPrev, chainparams.GetConsensus()) && fMineWitnessTx;

    this->addReportProofTxs(scriptPubKeyIn, pcoinsCache);

	int nPackagesSelected = 0;
	int nDescendantsUpdated = 0;
	addPackageTxs(nPackagesSelected, nDescendantsUpdated);

	int64_t nTime1 = GetTimeMicros();

	nLastBlockTx = nBlockTx;
	nLastBlockWeight = nBlockWeight;

	// make coin base
	MCMutableTransaction coinbaseTx;
	MCAmount nReward= MakeCoinbaseTransaction(coinbaseTx, nFees, pindexPrev, scriptPubKeyIn, chainparams);

	pblock->vtx[0] = MakeTransactionRef(std::move(coinbaseTx));
	pblocktemplate->vchCoinbaseCommitment = GenerateCoinbaseCommitment(*pblock, pindexPrev, chainparams.GetConsensus());
	pblocktemplate->vTxFees[0] = -nFees;

	LogPrint(BCLog::MINING,"CreateNewBlock(): block weight: %u txs: %u fees: %ld sigops %d\n", GetBlockWeight(*pblock), nBlockTx, nFees, nBlockSigOpsCost);

	// Fill in header
	pblock->hashPrevBlock = pindexPrev->GetBlockHash();
	UpdateTime(pblock, chainparams.GetConsensus(), pindexPrev);
	pblock->nBits = GetNextWorkRequired(pindexPrev, pblock, chainparams.GetConsensus());
	pblock->nNonce = 0;
	pblocktemplate->vTxSigOpsCost[0] = WITNESS_SCALE_FACTOR * GetLegacySigOpCount(*pblock->vtx[0]);


	// 如果block work大于要求，将NBITS设为实际值
	uint256 out_hash;
	pblock->nNonce = GetBlockWork(*pblock, outpoint, out_hash);
	arith_uint256 bTarget;
	bTarget.SetCompact(pblock->nBits);
	if (UintToArith256(out_hash) < bTarget) 
	{
		pblock->nBits = UintToArith256(out_hash).GetCompact();
		bTarget.SetCompact(pblock->nBits);
		LogPrint(BCLog::MINING, "CreateNewBlock(): new target %s \n", bTarget.GetHex());
	}

	// to verify is mining with the address owner
	MCMutableTransaction kSignTx(*pblock->vtx[0]);
	if (!SignatureCoinbaseTransaction(nHeight, keystoreIn, kSignTx, nReward, scriptPubKeyIn))
		throw std::runtime_error( "sign coin base transaction error");
	pblock->vtx[0] = MakeTransactionRef(std::move(kSignTx));

	MCValidationState state;
	if (!TestBlockValidity(state, chainparams, *pblock, pindexPrev, false, false)) {
		throw std::runtime_error(strprintf("%s: TestBlockValidity failed: %s", __func__, FormatStateMessage(state)));
	}

    CoinAmountDB coinAmountDB;
    CoinAmountCache coinAmountCache(&coinAmountDB);
    LogPrintf("%s:%d => vtx size:%d, group:%d\n", __FUNCTION__, __LINE__, pblock->vtx.size(), pblock->groupSize.size());
    if (!mpContractDb->RunBlockContract(pblock, pContractContext, &coinAmountCache)) {
        error("%s:%d RunBlockContract fail\n", __FUNCTION__, __LINE__);
        return nullptr;
    }

	int64_t nTime2 = GetTimeMicros();
	LogPrint(BCLog::MINING, "CreateNewBlock() packages: %.2fms (%d packages, %d updated descendants), validity: %.2fms (total %.2fms)\n", 0.001 * (nTime1 - nTimeStart), nPackagesSelected, nDescendantsUpdated, 0.001 * (nTime2 - nTime1), 0.001 * (nTime2 - nTimeStart));

	return std::move(pblocktemplate);
}
/** 只有举报在证明超时时才会调用
 * ptxReport 举报交易
 * minerpkey 矿工地址
 * pCoinsCache 一个block的cache或者是pcoinsTip
 */
void BlockAssembler::addReportProofTx(const MCTransactionRef &ptxReport, const MCScript &minerpkey, const MCCoinsViewCache* pCoinsCache)
{
    if (!chainparams.IsMainChain())
        return;

    if (!ptxReport->IsReport() || ptxReport->pReportData == nullptr)
        return;

    // 检查ptxReport是否满足高度 REPORT_OUTOF_HEIGHT

    //get data from ptxReport
    uint256 reportbranchid = ptxReport->pReportData->reportedBranchId;
    uint256 reportblockhash = ptxReport->pReportData->reportedBlockHash;
    if (!g_pBranchDb->HasBranchData(reportbranchid))
        return;

    BranchData branchdata = g_pBranchDb->GetBranchData(reportbranchid);
    if (!branchdata.mapHeads.count(reportblockhash))// best chain check?
        return;
    
    // 从stake交易取出prevout(抵押币)
    BranchBlockData blockdata = branchdata.mapHeads[reportblockhash];
    uint256 coinfromtxid;
    if (!GetMortgageCoinData(blockdata.pStakeTx->vout[0].scriptPubKey, &coinfromtxid))
        return;

    // 检查ptxReport有没有被证明
    uint256 reportFlagHash = GetReportTxHashKey(*ptxReport);
    if (blockdata.mapReportStatus.count(reportFlagHash) == 0 || blockdata.mapReportStatus[reportFlagHash] == RP_FLAG_PROVED){
        return;
    }

    MCOutPoint prevout(coinfromtxid, 0);// 抵押币放在vout[0]位
    const Coin& coin = pCoinsCache->AccessCoin(prevout);
    if (coin.IsSpent())
        return;
    
    const MCAmount& nValueIn = coin.out.nValue;   
    const MCScript& reporterAddress = ptxReport->vout[0].scriptPubKey;

    // 不用留手续费, 因为这个是矿工自己创建的交易
    MCMutableTransaction mtx;
    mtx.nVersion = MCTransaction::REPORT_REWARD;
    mtx.reporttxid = ptxReport->GetHash();
    
    mtx.vin.resize(1);
    mtx.vin[0].prevout = prevout;

    MCAmount nReporterValue = nValueIn / 2;
    MCAmount nMinerValue = nValueIn - nReporterValue;

    mtx.vout.resize(2);
    mtx.vout[0].nValue = nReporterValue;
    mtx.vout[0].scriptPubKey = reporterAddress;
    mtx.vout[1].nValue = nMinerValue;
    mtx.vout[1].scriptPubKey = minerpkey;

    // add to block
    pblock->vtx.emplace_back(MakeTransactionRef(mtx));
    pblocktemplate->vTxFees.push_back(-1);
    pblocktemplate->vTxSigOpsCost.push_back(-1);
}

// 监控举报交易,发现超过证明时间还没证明成功的,就把抵押币
// simple imp. miner can be as an report-outtime hunter check some miner miss transaction
// OP:对所有report记录下来，按块高度排序，对超时的进行处理
void BlockAssembler::addReportProofTxs(const MCScript& scriptPubKeyIn, MCCoinsViewCache *pcoinsCache)
{
    if (!chainparams.IsMainChain())
        return;

    uint32_t nOutOfHeight = REPORT_OUTOF_HEIGHT;
    MCBlockIndex *pbi = chainActive[chainActive.Tip()->nHeight - nOutOfHeight];// assume that create new block after active chain's tip
    if (pbi != nullptr)
    {
        std::shared_ptr<MCBlock> pblock = std::make_shared<MCBlock>();
        MCBlock& block = *pblock;
        if (ReadBlockFromDisk(block, pbi, chainparams.GetConsensus()))
        {
            for (int i = 1; i < block.vtx.size(); i++)
            {
                const MCTransactionRef& tx = block.vtx[i];
                if (tx->IsReport())
                {
                    this->addReportProofTx(tx, scriptPubKeyIn, pcoinsCache);
                }
            }
        }
    }
}
