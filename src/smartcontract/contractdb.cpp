// Copyright (c) 2016-2018 The CellLink Core developers
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

ContractDataDB* mpContractDb = nullptr;

CellAmount GetTxContractOut(const CellTransaction& tx)
{
    CellAmount out = 0;
    for (auto kOut : tx.vout) {
        opcodetype opcode;
        std::vector<unsigned char> vch;
        CellScript::const_iterator pc1 = kOut.scriptPubKey.begin();
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

void ContractContext::SetCache(const CellContractID& contractId, ContractInfo& contractInfo)
{
    cache[contractId] = std::move(contractInfo);
}

void ContractContext::SetData(const CellContractID& contractId, ContractInfo& contractInfo)
{
    data[contractId] = std::move(contractInfo);
}

bool ContractContext::GetData(const CellContractID& contractId, ContractInfo& contractInfo)
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
    : db(path, nCacheSize, fMemory, fWipe, true), threadPool(boost::thread::hardware_concurrency())
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

void ContractDataDB::ExecutiveTransactionContractThread(ContractDataDB* contractDB, const std::shared_ptr<const CellBlock> pBlock, SmartContractThreadData* threadData)
{
    auto it = contractDB->threadId2SmartLuaState.find(boost::this_thread::get_id());
    if (it == contractDB->threadId2SmartLuaState.end())
        throw std::runtime_error(strprintf("%s sls == nullptr\n", __FUNCTION__));

    if (contractDB != nullptr)
        contractDB->ExecutiveTransactionContract(it->second, pBlock, threadData);
}

void ContractDataDB::ExecutiveTransactionContract(SmartLuaState* sls, const std::shared_ptr<const CellBlock> pBlock, SmartContractThreadData* threadData)
{
#ifndef _DEBUG
    try {
#endif
        std::map<CellContractID, uint256> contract2txid;
        for (int i = threadData->offset; i < threadData->offset + threadData->groupSize; ++i) {
            if (interrupt)
                return;

            const CellTransactionRef& tx = pBlock->vtx[i];
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

            CellContractID contractId = tx->contractAddr;
            CellLinkAddress contractAddr(contractId);
            CellLinkAddress senderAddr(tx->contractSender.GetID());
            CellAmount amount = GetTxContractOut(*tx);

            UniValue ret(UniValue::VARR);
            if (tx->nVersion == CellTransaction::PUBLISH_CONTRACT_VERSION) {
                std::string rawCode = tx->contractCode;
                sls->Initialize(pBlock->GetBlockTime(), threadData->blockHeight, senderAddr, &threadData->contractContext, threadData->pPrevBlockIndex, SmartLuaState::SAVE_TYPE_CACHE, threadData->pCoinAmountCache);
                if (!PublishContract(sls, contractAddr, rawCode, ret)) {
                    interrupt = true;
                    return;
                }
            }
            else if (tx->nVersion == CellTransaction::CALL_CONTRACT_VERSION) {
                std::string strFuncName = tx->contractFun;
                UniValue args;
                args.read(tx->contractParams);

                long maxCallNum = MAX_CONTRACT_CALL;
                sls->Initialize(pBlock->GetBlockTime(), threadData->blockHeight, senderAddr, &threadData->contractContext, threadData->pPrevBlockIndex, SmartLuaState::SAVE_TYPE_CACHE, threadData->pCoinAmountCache);
                if (!CallContract(sls, contractAddr, amount, strFuncName, args, maxCallNum, ret) || tx->contractOut != sls->contractOut) {
                    interrupt = true;
                    return;
                }

                if (tx->contractOut > 0 && sls->recipients.size() == 0) {
                    interrupt = true;
                    return;
                }

                CellAmount total = 0;
                for (int i = 0; i < sls->recipients.size(); ++i) {
                    if (!tx->IsExistVout(sls->recipients[i])) {
                        interrupt = true;
                        return;
                    }
                    total += sls->recipients[i].nValue;
                }

                if (total != tx->contractOut) {
                    interrupt = true;
                    return;
                }
            }

            threadData->contractContext.Commit();
        }
#ifndef _DEBUG
    }
    catch (...) {
        interrupt = true;
    }
#endif
}

bool ContractDataDB::RunBlockContract(const std::shared_ptr<const CellBlock> pBlock, ContractContext* pContractContext, CoinAmountCache* pCoinAmountCache)
{
    auto it = mapBlockIndex.find(pBlock->hashPrevBlock);
    if (it == mapBlockIndex.end())
        return false;
    
    CellBlockIndex* pPrevBlockIndex = it->second;
    int blockHeight = pPrevBlockIndex->nHeight + 1;

    int offset = 0;
    interrupt = false;
    printf("Generate block vtx size:%d, group:%d\n", pBlock->vtx.size(), pBlock->groupSize.size());
    std::vector<SmartContractThreadData> threadData(pBlock->groupSize.size(), SmartContractThreadData());
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

    pContractContext->ClearCache();
    std::set<uint256> finalTransactions;
    for (int i = 0; i < threadData.size(); ++i) {
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

void ContractDataDB::UpdateBlockContractInfo(CellBlockIndex* pBlockIndex, ContractContext* pContractContext)
{
    if (pBlockIndex == nullptr)
        return;

    int blockHeight = pBlockIndex->nHeight;
    int confirmBlockHeight = blockHeight - DEFAULT_CHECKBLOCKS;

    if (confirmBlockHeight > 0) {
        // 遍历缓存的合约数据，清理明确是无效分叉的数据
        CellBlockIndex* newConfirmBlock = pBlockIndex->GetAncestor(confirmBlockHeight);
        for (auto ci : contractData) {
            DBBlockContractInfo& dbBlockContractInfo = ci.second;

            DBContractList::iterator saveIt = dbBlockContractInfo.data.end();
            for (DBContractList::iterator it = dbBlockContractInfo.data.begin(); it != dbBlockContractInfo.data.end();) {
                if (it->blockHeight <= confirmBlockHeight) {
                    BlockMap::iterator bi = mapBlockIndex.find(it->blockHash);
                    // 将小于确认区块以下的不属于该链的区块数据移除掉
                    if (bi == mapBlockIndex.end() || newConfirmBlock->GetAncestor(it->blockHeight)->GetBlockHash() != it->blockHash) {
                        DBContractList::iterator temp = it++;
                        dbBlockContractInfo.data.erase(temp);
                        continue;
                    }
                    if (it->blockHeight < confirmBlockHeight) {
                        // 保留当前确认块以下的最近一笔数据，防止程序退出时区块链保存信息不完整导致加载到错误数据
                        if (saveIt != dbBlockContractInfo.data.end())
                            dbBlockContractInfo.data.erase(saveIt);
                        saveIt = it;
                    }
                    ++it;
                }
                else
                    break;
            }
        }
    }

    // 将区块插入对应的结点中
    for (auto ci : pContractContext->data) {
        DBContractInfo dbContractInfo;
        dbContractInfo.blockHash = pBlockIndex->GetBlockHash();
        dbContractInfo.blockHeight = pBlockIndex->nHeight;
        dbContractInfo.data = ci.second.data;

        // 按高度寻找插入点
        DBBlockContractInfo& blockContractInfo = contractData[ci.first];
        if (blockContractInfo.code.empty())
            blockContractInfo.code = ci.second.code;
        DBContractList::iterator insertPoint = blockContractInfo.data.end();
        for (DBContractList::iterator it = blockContractInfo.data.begin(); it != blockContractInfo.data.end(); ++it) {
            if (blockHeight < it->blockHeight) {
                insertPoint = it;
                break;
            }
        }
        blockContractInfo.data.insert(insertPoint, dbContractInfo);
    }
    pContractContext->ClearData();
}

void ContractDataDB::Flush()
{
    LOCK(cs_cache);
    LogPrint(BCLog::COINDB, "flush contract data to db");

    int confirmBlockHeight = chainActive.Height() - DEFAULT_CHECKBLOCKS;

    std::vector<uint160> removes;
    CellDBBatch batch(db);
    size_t batch_size = (size_t)gArgs.GetArg("-dbbatchsize", nDefaultDbBatchSize);
    for (auto it : contractData) {
        DBBlockContractInfo& dbBlockContractInfo = it.second;
        if (dbBlockContractInfo.data.size() == 0) {
            // 该合约没有有效的数据，则判定为分叉后无效的数据，移除之
            batch.Erase(it.first);
            removes.emplace_back(it.first);
        }
        else
            batch.Write(it.first, it.second);

        // 该区块已不可逆且只剩下一笔数据，则存盘后从内存移除
        if (dbBlockContractInfo.data.size() == 1) {
            BlockMap::iterator mi = mapBlockIndex.find(dbBlockContractInfo.data.begin()->blockHash);
            if (mi != mapBlockIndex.end() && mi->second->nHeight < confirmBlockHeight)
                removes.emplace_back(it.first);
        }

        if (batch.SizeEstimate() > batch_size) {
            LogPrint(BCLog::COINDB, "Writing partial batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
            db.WriteBatch(batch);
            batch.Clear();
        }
    }
    db.WriteBatch(batch);
    batch.Clear();

    for (uint160& contractId : removes)
        contractData.erase(contractId);
}

int ContractDataDB::GetContractInfo(const CellContractID& contractId, ContractInfo& contractInfo, CellBlockIndex* currentPrevBlockIndex)
{
    LOCK(cs_cache);

    // 检查是否在缓存中，不存在则尝试加载
    DBContractMap::iterator di = contractData.find(contractId);
    if (di == contractData.end()) {
        DBBlockContractInfo dbBlockContractInfo;
        if (!db.Read(contractId, dbBlockContractInfo))
            return -1;
        else {
            contractData[contractId] = dbBlockContractInfo;
            di = contractData.find(contractId);
        }
    }

    // 遍历获取相应节点的数据
    CellBlockIndex* prevBlock = (currentPrevBlockIndex ? currentPrevBlockIndex : chainActive.Tip());
    int blockHeight = prevBlock->nHeight;
    // 链表最末尾存储最高的区块
    for (DBContractList::reverse_iterator it = di->second.data.rbegin(); it != di->second.data.rend(); ++it) {
        if (it->blockHeight <= blockHeight) {
            BlockMap::iterator bi = mapBlockIndex.find(it->blockHash);
            assert(bi != mapBlockIndex.end());
            CellBlockIndex* checkBlockIndex = prevBlock->GetAncestor(it->blockHeight);
            // 找到同一分叉时
            if (checkBlockIndex == bi->second) {
                contractInfo.code = di->second.code;
                contractInfo.data = it->data;
                return it->blockHeight;
            }
        }
    }

    return -1;
}
