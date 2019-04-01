// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "coding/base58.h"
#include "consensus/consensus.h"
#include "smartcontract/contractdb.h"
#include "smartcontract/smartcontract.h"
#include "transaction/txmempool.h"
#include "univalue.h"
#include "validation/validation.h"

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
    for (int i = 0; i < threadPool.size(); ++i) {
        threadPool.schedule(boost::bind(InitializeThread, this));
    }
}

void ContractDataDB::InitializeThread(ContractDataDB* contractDB)
{
    {
        LOCK(contractDB->cs_cache);
        contractDB->threadId2SmartLuaState.insert(std::make_pair(boost::this_thread::get_id(), new SmartLuaState()));
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));
}

void ContractDataDB::ExecutiveTransactionContract(MCBlock* pBlock, SmartContractThreadData* threadData)
{
    auto it = threadId2SmartLuaState.find(boost::this_thread::get_id());
    if (it == threadId2SmartLuaState.end()) {
        throw std::runtime_error(strprintf("%s:%d it == threadId2SmartLuaState.end()\n", __FUNCTION__, __LINE__));
    }

    SmartLuaState* sls = it->second;
    if (sls == nullptr) {
        throw std::runtime_error(strprintf("%s:%d sls == nullptr\n", __FUNCTION__, __LINE__));
    }

#ifndef _DEBUG
    try {
#endif
        bool mainChain = Params().IsMainChain();
        if (!mainChain) {
            threadData->contractContext.txFinalData.resize(threadData->groupSize);
        }

        std::map<MCContractID, uint256> contract2txid;
        for (int i = threadData->offset; i < threadData->offset + threadData->groupSize; ++i) {
            if (interrupt) {
                return;
            }

            const MCTransactionRef tx = pBlock->vtx[i];
            if (tx->IsNull()) {
                LogPrintf("%s:%d => tx is null\n", __FUNCTION__, __LINE__);
                interrupt = true;
                return;
            }

            threadData->associationTransactions.insert(tx->GetHash());
            for (int j = 0; j < tx->vin.size(); ++j) {
                if (!tx->vin[j].prevout.hash.IsNull() && !tx->IsStake()) { // branch first block's stake tx's input is from the same block(支链第一个块的stake交易的输入来自同一区块中的交易，其他情况下stake的输入不可能来自同一区块)
                    threadData->associationTransactions.insert(tx->vin[j].prevout.hash);
                }
            }

            if (!tx->IsSmartContract()) {
                continue;
            }

            MCContractID contractId = tx->pContractData->address;
            MagnaChainAddress contractAddr(contractId);
            MagnaChainAddress senderAddr(tx->pContractData->sender.GetID());
            MCAmount amount = GetTxContractOut(*tx);

            UniValue ret(UniValue::VARR);
            if (tx->nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION) {
                LogPrintf("%s:%d tx %s publishcontract %s\n", __FUNCTION__, __LINE__, tx->GetHash().ToString(), tx->pContractData->address.ToString());
                std::string rawCode = tx->pContractData->codeOrFunc;
                sls->Initialize(true, threadData->pPrevBlockIndex->GetBlockTime(), threadData->blockHeight, i, senderAddr,
                    &threadData->contractContext, threadData->pPrevBlockIndex, SmartLuaState::SAVE_TYPE_CACHE, nullptr);
                if (!PublishContract(sls, contractAddr, rawCode, ret, true) || tx->pContractData->contractCoinsOut.size() > 0 || tx->pContractData->contractCoinsOut != sls->contractCoinsOut) {
                    LogPrintf("%s:%d => publish contract fail\n", __FUNCTION__, __LINE__);
                    interrupt = true;
                    return;
                }
            } else if (tx->nVersion == MCTransaction::CALL_CONTRACT_VERSION) {
                LogPrintf("%s:%d tx %s callcontract %s\n", __FUNCTION__, __LINE__, tx->GetHash().ToString(), tx->pContractData->address.ToString());
                const std::string& strFuncName = tx->pContractData->codeOrFunc;
                UniValue args;
                args.read(tx->pContractData->args);

                sls->Initialize(false, threadData->pPrevBlockIndex->GetBlockTime(), threadData->blockHeight, i, senderAddr, &threadData->contractContext,
                    threadData->pPrevBlockIndex, SmartLuaState::SAVE_TYPE_CACHE, threadData->pCoinAmountCache);
                if (!CallContract(sls, contractAddr, amount, strFuncName, args, ret) || tx->pContractData->contractCoinsOut != sls->contractCoinsOut) {
                    LogPrintf("%s:%d => call contract fail\n", __FUNCTION__, __LINE__);
                    interrupt = true;
                    return;
                }

                if (tx->pContractData->contractCoinsOut.size() > 0 && sls->recipients.size() == 0) {
                    LogPrintf("%s:%d => tx->pContractData->amountOut > 0 && sls->recipients.size() == 0\n", __FUNCTION__, __LINE__);
                    interrupt = true;
                    return;
                }

                MCAmount totalRecv = 0;
                for (int j = 0; j < sls->recipients.size(); ++j) {
                    if (!tx->IsExistVout(sls->recipients[j])) {
                        LogPrintf("%s:%d => vout not exist\n", __FUNCTION__, __LINE__);
                        interrupt = true;
                        return;
                    }
                    totalRecv += sls->recipients[j].nValue;
                }

                MCAmount totalSend = 0;
                for (auto it : tx->pContractData->contractCoinsOut) {
                    totalSend += it.second;
                }

                if (totalRecv != totalSend) {
                    LogPrintf("%s:%d => amount not match\n", __FUNCTION__, __LINE__);
                    interrupt = true;
                    return;
                }

                if (!mainChain) {
                    for (auto it : tx->pContractData->contractCoinsOut) {
                        threadData->contractContext.txFinalData[i].contractCoins[it.first] = threadData->pCoinAmountCache->GetAmount(it.first);
                    }
                }

                if (tx->pContractData->contractCoinsOut.size() > 0) {
                    for (auto it : tx->pContractData->contractCoinsOut) {
                        threadData->pCoinAmountCache->DecAmount(it.first, it.second);
                    }
                }

                const std::vector<MCTxOut>& vout = tx->vout;
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
                        threadData->pCoinAmountCache->IncAmount(contractId, vout[i].nValue);
                    }
                }
            }

            if (!mainChain) {
                for (auto it : sls->contractDataFrom) {
                    pBlock->prevContractData[i].items[it.first].blockHash = it.second.blockHash;
                    pBlock->prevContractData[i].items[it.first].txIndex = it.second.txIndex;
                }
                threadData->contractContext.txFinalData[i - threadData->offset].data = threadData->contractContext.cache;
            }
            threadData->contractContext.Commit();
            sls->contractDataFrom.clear();
        }
#ifndef _DEBUG
    } catch (std::exception e) {
        LogPrintf("%s:%d => unknown exception %s\n", __FUNCTION__, __LINE__, e.what());
        interrupt = true;
    }
#endif
}

bool ContractDataDB::RunBlockContract(MCBlock* pBlock, ContractContext* pContractContext, CoinAmountCache* pCoinAmountCache)
{
    auto it = mapBlockIndex.find(pBlock->hashPrevBlock);
    if (it == mapBlockIndex.end()) {
        throw std::runtime_error(strprintf("%s:%d => it == mapBlockIndex.end()", __FUNCTION__, __LINE__));
    }

    if (pBlock->groupSize.size() > MAX_GROUP_NUM) {
        throw std::runtime_error(strprintf("%s:%d => pBlock->groupSize.size() > MAX_GROUP_NUM", __FUNCTION__, __LINE__));
    }

    int totalSize = 0;
    for (int i = 0; i < pBlock->groupSize.size(); ++i) {
        totalSize += pBlock->groupSize[i];
    }
    if (totalSize == 0 || totalSize != pBlock->vtx.size()) {
        throw std::runtime_error(strprintf("%s:%d => totalSize not match", __FUNCTION__, __LINE__));
    }

    MCBlockIndex* pPrevBlockIndex = it->second;
    int blockHeight = pPrevBlockIndex->nHeight + 1;

    int offset = 0;
    interrupt = false;
    std::vector<SmartContractThreadData> threadData(pBlock->groupSize.size(), SmartContractThreadData());
    int size = pBlock->vtx.size();
    bool mainChain = Params().IsMainChain();
    if (!mainChain) {
        pBlock->prevContractData.resize(size);
    }
    for (int i = 0; i < pBlock->groupSize.size(); ++i) {
        threadData[i].offset = offset;
        threadData[i].groupSize = pBlock->groupSize[i];
        threadData[i].blockHeight = blockHeight;
        threadData[i].pPrevBlockIndex = pPrevBlockIndex;
        threadData[i].pCoinAmountCache = pCoinAmountCache;
        threadPool.schedule(boost::bind(&ContractDataDB::ExecutiveTransactionContract, this, pBlock, &threadData[i]));
        offset += pBlock->groupSize[i];
    }
    threadPool.wait();

    if (interrupt) {
        throw std::runtime_error(strprintf("%s:%d => run contract interrupt", __FUNCTION__, __LINE__));
    }

    offset = 0;
    pContractContext->ClearCache();
    if (!mainChain) {
        pContractContext->txFinalData.resize(pBlock->vtx.size());
    }
    std::set<uint256> finalTransactions;
    for (int i = 0; i < threadData.size(); ++i) {
        // 检查是否有关联交易交叉
        for (auto item : threadData[i].associationTransactions) {
            if (finalTransactions.count(item) == 0) {
                finalTransactions.insert(item);
            } else {
                throw std::runtime_error(strprintf("%s:%d => association transactions have cross", __FUNCTION__, __LINE__));
            }
        }

        // 保存数据，同时检查是否有关联数据交叉
        for (auto item : threadData[i].contractContext.data) {
            if (pContractContext->cache.count(item.first) == 0) {
                pContractContext->SetCache(item.first, item.second);
            } else {
                throw std::runtime_error(strprintf("%s:%d => contract context have cross", __FUNCTION__, __LINE__));
            }
        }

        // 支链保存交易最后的数据以便在需要时提供证明使用
        if (!mainChain) {
            for (int j = offset; j < offset + pBlock->groupSize[i]; ++j) {
                pContractContext->txFinalData[j].contractCoins = std::move(threadData[i].contractContext.txFinalData[j - offset].contractCoins);
                pContractContext->txFinalData[j].data = std::move(threadData[i].contractContext.txFinalData[j - offset].data);
            }
        }
        offset += pBlock->groupSize[i];
    }
    pContractContext->Commit();

    return true;
}

bool ContractDataDB::WriteBatch(MCDBBatch& batch)
{
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
            } else if (pBlockIndex->nHeight == it->blockHeight) {
                insertPoint = it;
                break;
            }
        }

        if (insertPoint == contractInfo.items.end()) {
            insertPoint = contractInfo.items.insert(insertPoint, DBContractInfoByHeight());
            insertPoint->blockHeight = pBlockIndex->nHeight;
        }

        for (int i = 0; i < insertPoint->vecBlockHash.size(); ++i) {
            if (insertPoint->vecBlockHash[i] == ci.second.blockHash) {
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
                for (int i = 0; i < heightIt->vecBlockHash.size();) {
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
                    } else {
                        ++i;
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
        if (!db.Read(contractId, contractInfo)) {
            return -1;
        } else {
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
