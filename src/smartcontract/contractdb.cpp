// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "smartcontract/contractdb.h"
#include "coding/base58.h"
#include "univalue.h"
#include "transaction/txmempool.h"
#include "validation/validation.h"
#include "smartcontract/smartcontract.h"
#include "consensus/consensus.h"

ContractDataDB* mpContractDb = nullptr;

MCAmount GetTxContractOut(const MCTransaction& tx)
{
    MCAmount out = 0;
    for (auto kOut : tx.vout) {
        opcodetype opcode;
        std::vector<unsigned char> vch;
        MCScript::const_iterator pc1 = kOut.scriptPubKey.begin();
        kOut.scriptPubKey.GetOp(pc1, opcode, vch);
        if (opcode == OP_CONTRACT) {
            out += kOut.nValue;
            break;
        }
    }
    return out;
}

struct CmpByBlockHeight {
    bool operator()(const uint256& bh1, const uint256& bh2) const
    {
        return (mapBlockIndex.count(bh1) > 0 && mapBlockIndex.count(bh2) > 0) ?
            (mapBlockIndex[bh1]->nHeight < mapBlockIndex[bh2]->nHeight) :
            (mapBlockIndex.count(bh1) <= 0 ? false : true);
    }
};

void ContractContext::SetCache(const MCContractID& contractId, ContractInfo& contractInfo)
{
    cache[contractId] = std::move(contractInfo);
}

void ContractContext::SetData(const MCContractID& contractId, ContractInfo& contractInfo)
{
    data[contractId] = std::move(contractInfo);
}

bool ContractContext::GetData(const MCContractID& contractId, ContractInfo& contractInfo)
{
    if (cache.size() > 0) {
        auto it = cache.find(contractId);
        if (it != cache.end()) {
            contractInfo.code = it->second.code;
            contractInfo.data = it->second.data;
            return true;
        }
    }

    auto it = data.find(contractId);
    if (it != data.end()) {
        contractInfo.code = it->second.code;
        contractInfo.data = it->second.data;
        return true;
    }

    return false;
}

void ContractContext::Commit()
{
    for (auto it : cache)
        data[it.first] = std::move(it.second);
    ClearCache();
}

void ContractContext::ClearCache()
{
    cache.clear();
}

void ContractContext::ClearData()
{
    data.clear();
}

void ContractContext::ClearAll()
{
    ClearCache();
    ClearData();
}

ContractDataDB::ContractDataDB(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe)
    : db(path, nCacheSize, fMemory, fWipe, true), writeBatch(db), removeBatch(db), threadPool(boost::thread::hardware_concurrency())
{
    for (int i = 0; i < threadPool.size(); ++i)
        threadPool.schedule(boost::bind(InitializeThread, this));
}

void ContractDataDB::InitializeThread(ContractDataDB* contractDB)
{
    {
        LOCK(contractDB->cs_cache);
        contractDB->threadId2SmartLuaState.insert(std::make_pair(boost::this_thread::get_id(), new SmartLuaState()));
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));
}

bool interrupt;

void ContractDataDB::ExecutiveTransactionContractThread(ContractDataDB* contractDB, MCBlock* pBlock, SmartContractThreadData* threadData)
{
    auto it = contractDB->threadId2SmartLuaState.find(boost::this_thread::get_id());
    if (it == contractDB->threadId2SmartLuaState.end())
        throw std::runtime_error(strprintf("%s sls == nullptr\n", __FUNCTION__));

    if (contractDB != nullptr)
        contractDB->ExecutiveTransactionContract(it->second, pBlock, threadData);
}

void ContractDataDB::ExecutiveTransactionContract(SmartLuaState* sls, MCBlock* pBlock, SmartContractThreadData* threadData)
{
#ifndef _DEBUG
    try {
#endif
        std::map<MCContractID, uint256> contract2txid;
        threadData->contractContext.txFinalData.data.resize(threadData->groupSize);
        for (int i = threadData->offset; i < threadData->offset + threadData->groupSize; ++i) {
            if (interrupt)
                return;

            const MCTransactionRef tx = pBlock->vtx[i];
            if (tx->IsNull()) {
                interrupt = true;
                return;
            }

            assert(!tx->GetHash().IsNull());
            threadData->associationTransactions.insert(tx->GetHash());
            //printf("%s group offset %d, tx hash %s\n", __FUNCTION__, threadData->offset, tx->GetHash().ToString().c_str());
            for (int j = 0; j < tx->vin.size(); ++j) {
                if (!tx->vin[j].prevout.hash.IsNull()) {
                    //printf("%s group offset %d, vin hash %s\n", __FUNCTION__, threadData->offset, tx->vin[j].prevout.hash.ToString().c_str());
                    threadData->associationTransactions.insert(tx->vin[j].prevout.hash);
                }
            }

            if (!tx->IsSmartContract())
                continue;

            MCContractID contractId = tx->pContractData->address;
            MagnaChainAddress contractAddr(contractId);
            MagnaChainAddress senderAddr(tx->pContractData->sender.GetID());
            MCAmount amount = GetTxContractOut(*tx);

            UniValue ret(UniValue::VARR);
            if (tx->nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION) {
                std::string rawCode = tx->pContractData->codeOrFunc;
                sls->Initialize(pBlock->GetBlockTime(), threadData->blockHeight, i, senderAddr, &threadData->contractContext, threadData->pPrevBlockIndex, SmartLuaState::SAVE_TYPE_CACHE, nullptr);
                if (!PublishContract(sls, contractAddr, rawCode, ret)) {
                    interrupt = true;
                    return;
                }
            }
            else if (tx->nVersion == MCTransaction::CALL_CONTRACT_VERSION) {
                std::string strFuncName = tx->pContractData->codeOrFunc;
                UniValue args;
                args.read(tx->pContractData->args);

                long maxCallNum = MAX_CONTRACT_CALL;
                sls->Initialize(pBlock->GetBlockTime(), threadData->blockHeight, i, senderAddr, &threadData->contractContext, threadData->pPrevBlockIndex, SmartLuaState::SAVE_TYPE_CACHE, threadData->pCoinAmountCache);
                if (!CallContract(sls, contractAddr, amount, strFuncName, args, maxCallNum, ret) || tx->pContractData->amountOut != sls->contractOut) {
                    interrupt = true;
                    return;
                }

                if (tx->pContractData->amountOut > 0 && sls->recipients.size() == 0) {
                    interrupt = true;
                    return;
                }

                MCAmount total = 0;
                for (int j = 0; j < sls->recipients.size(); ++j) {
                    if (!tx->IsExistVout(sls->recipients[j])) {
                        interrupt = true;
                        return;
                    }
                    total += sls->recipients[j].nValue;
                }

                if (total != tx->pContractData->amountOut) {
                    interrupt = true;
                    return;
                }

                if (tx->pContractData->amountOut > 0) {
                    if (sls->pCoinAmountCache->HasKeyInCache(contractId)) {
                        MCAmount amount = sls->pCoinAmountCache->GetAmount(contractId);
                        threadData->contractContext.txFinalData.coins = amount + tx->pContractData->amountOut;
                    }
                }
            }

            for (auto it : sls->contractDataFrom) {
                pBlock->prevContractData[i].items[it.first].blockHash = it.second.blockHash;
                pBlock->prevContractData[i].items[it.first].txIndex = it.second.txIndex;
            }
            threadData->contractContext.txFinalData.data[i - threadData->offset] = threadData->contractContext.cache;
            threadData->contractContext.Commit();
            sls->contractDataFrom.clear();
        }
#ifndef _DEBUG
    }
    catch (...) {
        interrupt = true;
    }
#endif
}

bool ContractDataDB::RunBlockContract(MCBlock* pBlock, ContractContext* pContractContext, CoinAmountCache* pCoinAmountCache)
{
    auto it = mapBlockIndex.find(pBlock->hashPrevBlock);
    if (it == mapBlockIndex.end())
        return false;
    
    MCBlockIndex* pPrevBlockIndex = it->second;
    int blockHeight = pPrevBlockIndex->nHeight + 1;

    int offset = 0;
    interrupt = false;
    LogPrintf("Generate block vtx size:%d, group:%d\n", pBlock->vtx.size(), pBlock->groupSize.size());
    std::vector<SmartContractThreadData> threadData(pBlock->groupSize.size(), SmartContractThreadData());
    int size = pBlock->vtx.size();
    pBlock->prevContractData.resize(size);
    for (int i = 0; i < pBlock->groupSize.size(); ++i) {
        threadData[i].offset = offset;
        threadData[i].groupSize = pBlock->groupSize[i];
        threadData[i].blockHeight = blockHeight;
        threadData[i].pPrevBlockIndex = pPrevBlockIndex;
        threadData[i].pCoinAmountCache = pCoinAmountCache;
        threadPool.schedule(boost::bind(ExecutiveTransactionContractThread, this, pBlock, &threadData[i]));
        offset += pBlock->groupSize[i];
    }
    threadPool.wait();

    if (interrupt)
        return false;

    offset = 0;
    pContractContext->ClearCache();
    pContractContext->txFinalData.data.resize(pBlock->vtx.size());
    std::set<uint256> finalTransactions;
    for (int i = 0; i < threadData.size(); ++i) {
        for (int j = offset; j < offset + pBlock->groupSize[i]; ++j)
            pContractContext->txFinalData.data[j] = std::move(threadData[i].contractContext.txFinalData.data[j - offset]);
        pContractContext->txFinalData.coins = threadData[i].contractContext.txFinalData.coins;
        offset += pBlock->groupSize[i];

        for (auto item : threadData[i].associationTransactions) {
            if (finalTransactions.find(item) == finalTransactions.end())
                finalTransactions.insert(item);
            else
                return false;
        }

        for (auto item : threadData[i].contractContext.data) {
            if (pContractContext->cache.find(item.first) == pContractContext->cache.end())
                pContractContext->SetCache(item.first, item.second);
            else
                return false;
        }
    }
    pContractContext->Commit();

    return true;
}

bool ContractDataDB::WriteBatch(MCDBBatch& batch) {
    if (!CheckDiskSpace(batch.SizeEstimate() - nMinDiskSpace))
        return false;
    LogPrint(BCLog::COINDB, "Writing partial batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
    db.WriteBatch(batch);
    batch.Clear();
    return true;
}

bool ContractDataDB::WriteBlockContractInfoToDisk(MCBlockIndex* pBlockIndex, ContractContext* pContractContext)
{
    assert(pBlockIndex != nullptr);
    LOCK(cs_cache);

    MCDBBatch writeBatch(db);
    size_t maxBatchSize = (size_t)gArgs.GetArg("-dbbatchsize", nDefaultDbBatchSize);
    for (auto ci : pContractContext->data) {
        DBContractInfo& contractInfo = contractData[ci.first];
        if (contractInfo.code.empty())
            contractInfo.code = ci.second.code;
        ci.second.blockHash = pBlockIndex->GetBlockHash();

        // 将区块插入对应的结点中
        auto insertPoint = contractInfo.items.end();
        for (auto it = contractInfo.items.begin(); it != contractInfo.items.end(); ++it) {
            if (pBlockIndex->nHeight < it->blockHeight) {
                insertPoint = contractInfo.items.insert(it, DBContractInfoByHeight());
                insertPoint->blockHeight = pBlockIndex->nHeight;
                break;
            }
            else if (pBlockIndex->nHeight == it->blockHeight) {
                insertPoint = it;
                break;
            }
        }

        if (insertPoint == contractInfo.items.end()) {
            insertPoint = contractInfo.items.insert(insertPoint, DBContractInfoByHeight());
            insertPoint->blockHeight = pBlockIndex->nHeight;
        }

        for (int i = 0; i < insertPoint->vecBlockHash.size(); ++i) {
            if (insertPoint->vecBlockHash[i] != ci.second.blockHash) {
                insertPoint->vecBlockHash.erase(insertPoint->vecBlockHash.begin() + i);
                insertPoint->vecBlockContractData.erase(insertPoint->vecBlockContractData.begin() + i);
                break;
            }
        }

        // 待存盘的合约数据
        insertPoint->dirty = true;
        insertPoint->vecBlockHash.emplace_back(ci.second.blockHash);
        insertPoint->vecBlockContractData.emplace_back(ci.second.data);

        // 数据存盘
        MCHashWriter keyHash(SER_GETHASH, 0);
        keyHash << ci.first << ci.second.blockHash;
        writeBatch.Write(keyHash.GetHash(), ci.second.data);
        if (writeBatch.SizeEstimate() > maxBatchSize) {
            if (!WriteBatch(writeBatch))
                return false;
        }
    }

    if (writeBatch.SizeEstimate() > 0) {
        if (!WriteBatch(writeBatch))
            return false;
    }

    return true;
}

bool ContractDataDB::UpdateBlockContractToDisk(MCBlockIndex* pBlockIndex)
{
    assert(pBlockIndex != nullptr);
    LOCK(cs_cache);

    int blockHeight = pBlockIndex->nHeight;
    int checkDepth = gArgs.GetArg("-checkblocks", DEFAULT_CHECKBLOCKS) / Params().GetConsensus().nPowTargetSpacing;
    int confirmBlockHeight = blockHeight - checkDepth;
    int removeBlockHeight = confirmBlockHeight - REDEEM_SAFE_HEIGHT - checkDepth;
    size_t maxBatchSize = (size_t)gArgs.GetArg("-dbbatchsize", nDefaultDbBatchSize);

    // 遍历缓存的合约数据，清理明确是无效分叉的数据
    removes.clear();
    removeBatch.Clear();
    MCDBBatch writeBatch(db);
    MCBlockIndex* newConfirmBlock = pBlockIndex->GetAncestor(confirmBlockHeight);
    for (auto& ci : contractData) {
        auto& contractInfo = ci.second;
        auto saveIt = contractInfo.items.end();

        for (auto heightIt = contractInfo.items.begin(); heightIt != contractInfo.items.end();) {
            // 将小于确认区块以下的不属于该链的区块数据移除掉
            if (heightIt->blockHeight <= confirmBlockHeight) {
                for (int i = 0; i < heightIt->vecBlockHash.size(); ++i) {
                    BlockMap::iterator bi = mapBlockIndex.find(heightIt->vecBlockHash[i]);
                    if (bi == mapBlockIndex.end() || 
                        newConfirmBlock->GetAncestor(heightIt->blockHeight)->GetBlockHash() != heightIt->vecBlockHash[i]) {
                        MCHashWriter keyHash(SER_GETHASH, 0);
                        keyHash << ci.first << heightIt->vecBlockHash[i];
                        removeBatch.Erase(keyHash.GetHash());

                        heightIt->dirty = true;
                        heightIt->vecBlockHash.erase(heightIt->vecBlockHash.begin() + i);
                        heightIt->vecBlockContractData.erase(heightIt->vecBlockContractData.begin() + i);
                        continue;
                    }
                }
                assert(heightIt->vecBlockHash.size() == 1 && heightIt->vecBlockContractData.size() == 1);
            }

            if (heightIt->blockHeight < removeBlockHeight) {
                // 保留当前即将移除块以下的最近一笔数据，防止程序退出时区块链保存信息不完整导致加载到错误数据
                if (saveIt != contractInfo.items.end()) {
                    // 移除存盘数据
                    assert(saveIt->vecBlockHash.size() == 1);
                    MCHashWriter keyBlockHash(SER_GETHASH, 0);
                    keyBlockHash << ci.first << *saveIt->vecBlockHash.begin();
                    removeBatch.Erase(keyBlockHash.GetHash());

                    MCHashWriter keyHeightHash(SER_GETHASH, 0);
                    keyHeightHash << ci.first << saveIt->blockHeight;
                    removeBatch.Erase(keyHeightHash.GetHash());
                    contractInfo.items.erase(saveIt);
                }
                saveIt = heightIt;

                if (contractInfo.items.size() == 1)
                    removes.emplace_back(ci.first);
            }

            if (heightIt->dirty) {
                heightIt->dirty = false;
                MCHashWriter keyHeightHash(SER_GETHASH, 0);
                keyHeightHash << ci.first << heightIt->blockHeight;
                writeBatch.Write(keyHeightHash.GetHash(), heightIt->vecBlockHash);
                if (writeBatch.SizeEstimate() > maxBatchSize) {
                    if (!WriteBatch(writeBatch))
                        return false;
                }
            }

            ++heightIt;
        }

        writeBatch.Write(ci.first, ci.second);
        if (writeBatch.SizeEstimate() > maxBatchSize) {
            if (!WriteBatch(writeBatch))
                return false;
        }
    }

    if (writeBatch.SizeEstimate() > 0) {
        if (!WriteBatch(writeBatch))
            return false;
    }

    return true;
}

void ContractDataDB::PruneContractInfo()
{
    LOCK(cs_cache);
    if (removeBatch.SizeEstimate() > 0)
        WriteBatch(removeBatch);

    for (uint160& contractId : removes)
        contractData.erase(contractId);
    removes.clear();
}

int ContractDataDB::GetContractInfo(const MCContractID& contractId, ContractInfo& contractInfo, MCBlockIndex* prevBlockIndex)
{
    LOCK(cs_cache);

    // 检查是否在缓存中，不存在则尝试加载
    auto di = contractData.find(contractId);
    if (di == contractData.end()) {
        DBContractInfo contractInfo;
        if (!db.Read(contractId, contractInfo))
            return -1;
        else {
            contractData[contractId] = contractInfo;
            di = contractData.find(contractId);
        }
    }

    // 遍历获取相应节点的数据
    MCBlockIndex* prevBlock = (prevBlockIndex ? prevBlockIndex : chainActive.Tip());
    // 链表最末尾存储最高的区块
    for (auto it = di->second.items.rbegin(); it != di->second.items.rend(); ++it) {
        if (prevBlock->nHeight >= it->blockHeight) {
            MCBlockIndex* targetBlockIndex = prevBlock->GetAncestor(it->blockHeight);
            if (it->vecBlockHash.size() == 0) {
                MCHashWriter keyHeightHash(SER_GETHASH, 0);
                keyHeightHash << contractId << it->blockHeight;
                db.Read(keyHeightHash.GetHash(), it->vecBlockHash);
                it->vecBlockContractData.resize(it->vecBlockHash.size());
            }
            for (int i = 0; i < it->vecBlockHash.size(); ++i) {
                if (it->vecBlockHash[i] == targetBlockIndex->GetBlockHash()) {
                    if (it->vecBlockContractData[i].empty()) {
                        MCHashWriter keyHash(SER_GETHASH, 0);
                        keyHash << contractId << it->vecBlockHash[i];
                        db.Read(keyHash.GetHash(), it->vecBlockContractData[i]);
                    }

                    contractInfo.txIndex = 0;
                    contractInfo.code = di->second.code;
                    contractInfo.blockHash = it->vecBlockHash[i];
                    contractInfo.data = it->vecBlockContractData[i];
                    return it->blockHeight;
                }
            }
        }
    }

    return -1;
}
