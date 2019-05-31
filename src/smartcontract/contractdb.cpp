// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "coding/base58.h"
#include "consensus/consensus.h"
#include "smartcontract/contractdb.h"
#include "smartcontract/smartcontract.h"
#include "validation/validation.h"

ContractDataDB* mpContractDb = nullptr;

ContractDataDB::ContractDataDB(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe)
    : db(path, nCacheSize, fMemory, fWipe, true), removeBatch(db)
{
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

bool ContractDataDB::WriteBlockContractInfoToDisk(const MCBlockIndex* pBlockIndex, const CONTRACT_DATA& data)
{
    assert(pBlockIndex != nullptr);

    MCDBBatch writeBatch(db);
    size_t maxBatchSize = (size_t)gArgs.GetArg("-dbbatchsize", nDefaultDbBatchSize);
    for (auto ci : data) {
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

        for (size_t i = 0; i < insertPoint->vecBlockHash.size(); ++i) {
            if (insertPoint->vecBlockHash[i] == ci.second.blockHash) {
                insertPoint->vecBlockHash.erase(insertPoint->vecBlockHash.begin() + i);
                insertPoint->vecBlockContractData.erase(insertPoint->vecBlockContractData.begin() + i);
                break;
            }
        }

        // 待存盘的合约数据
        insertPoint->dirty = true;
        insertPoint->vecBlockHash.push_back(ci.second.blockHash);
        insertPoint->vecBlockContractData.emplace_back(ci.second.coins, ci.second.data);

        // 数据存盘
        MCHashWriter keyHash(SER_GETHASH, 0);
        keyHash << ci.first << ci.second.blockHash;
        writeBatch.Write(keyHash.GetHash(), std::make_pair(ci.second.coins, ci.second.data));
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

bool ContractDataDB::UpdateBlockContractToDisk(const MCBlockIndex* pBlockIndex)
{
    assert(pBlockIndex != nullptr);

    int blockHeight = pBlockIndex->nHeight;
    int checkDepth = gArgs.GetArg("-checkblocks", DEFAULT_CHECKBLOCKS) / Params().GetConsensus().nPowTargetSpacing;
    int confirmBlockHeight = blockHeight - checkDepth;
    int removeBlockHeight = confirmBlockHeight - REDEEM_SAFE_HEIGHT - checkDepth;
    size_t maxBatchSize = (size_t)gArgs.GetArg("-dbbatchsize", nDefaultDbBatchSize);

    // 遍历缓存的合约数据，清理明确是无效分叉的数据
    removes.clear();
    removeBatch.Clear();
    MCDBBatch writeBatch(db);
    const MCBlockIndex* newConfirmBlock = pBlockIndex->GetAncestor(confirmBlockHeight);
    for (auto& ci : contractData) {
        auto& contractInfo = ci.second;
        auto saveIt = contractInfo.items.end();

        for (auto heightIt = contractInfo.items.begin(); heightIt != contractInfo.items.end();) {
            // 将小于确认区块以下的不属于该链的区块数据移除掉
            if (heightIt->blockHeight <= confirmBlockHeight) {
                if (heightIt->vecBlockHash.size() == 0) {
                    MCHashWriter keyHeightHash(SER_GETHASH, 0);
                    keyHeightHash << ci.first << heightIt->blockHeight;
                    db.Read(keyHeightHash.GetHash(), heightIt->vecBlockHash);
                }

                for (size_t i = 0; i < heightIt->vecBlockHash.size();) {
                    BlockMap::iterator bi = mapBlockIndex.find(heightIt->vecBlockHash[i]);
                    if (bi == mapBlockIndex.end() ||
                        newConfirmBlock->GetAncestor(heightIt->blockHeight)->GetBlockHash() != heightIt->vecBlockHash[i]) {
                        MCHashWriter keyHash(SER_GETHASH, 0);
                        keyHash << ci.first << heightIt->vecBlockHash[i];
                        removeBatch.Erase(keyHash.GetHash());

                        heightIt->dirty = true;
                        heightIt->vecBlockHash.erase(heightIt->vecBlockHash.begin() + i);
                        if (heightIt->vecBlockContractData.size() > 0) {
                            heightIt->vecBlockContractData.erase(heightIt->vecBlockContractData.begin() + i);
                        }
                        continue;
                    } else {
                        ++i;
                    }
                }

                if (heightIt->vecBlockHash.size() == 1 && heightIt->vecBlockContractData.size() == 0) {
                    heightIt->vecBlockContractData.resize(1);
                    MCHashWriter keyHash(SER_GETHASH, 0);
                    keyHash << ci.first << heightIt->vecBlockHash[0];
                    db.Read(keyHash.GetHash(), heightIt->vecBlockContractData[0]);
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

                if (contractInfo.items.size() == 1) {
                    removes.emplace_back(ci.first);
                }
            }

            if (heightIt->dirty) {
                heightIt->dirty = false;
                MCHashWriter keyHeightHash(SER_GETHASH, 0);
                keyHeightHash << ci.first << heightIt->blockHeight;
                writeBatch.Write(keyHeightHash.GetHash(), heightIt->vecBlockHash);
                if (writeBatch.SizeEstimate() > maxBatchSize) {
                    if (!WriteBatch(writeBatch)) {
                        return false;
                    }
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
    if (removeBatch.SizeEstimate() > 0)
        WriteBatch(removeBatch);

    for (uint160& contractId : removes)
        contractData.erase(contractId);
    removes.clear();
}

int ContractDataDB::GetContractInfo(const MCContractID& contractId, ContractInfo& contractInfo, const MCBlockIndex* prevBlockIndex)
{
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

    // 遍历获取相应节点的数据(链表最末尾存储最高的区块)
    for (auto it = di->second.items.rbegin(); it != di->second.items.rend(); ++it) {
        if (prevBlockIndex->nHeight >= it->blockHeight) {
            const MCBlockIndex* targetBlockIndex = prevBlockIndex->GetAncestor(it->blockHeight);
            // get all hash of blocks at the same height if not initialize
            if (it->vecBlockHash.size() == 0) {
                MCHashWriter keyHeightHash(SER_GETHASH, 0);
                keyHeightHash << contractId << it->blockHeight;
                db.Read(keyHeightHash.GetHash(), it->vecBlockHash);
                it->vecBlockContractData.resize(it->vecBlockHash.size());
            }
            // find the contract info from the target block
            for (size_t i = 0; i < it->vecBlockHash.size(); ++i) {
                if (it->vecBlockHash[i] == targetBlockIndex->GetBlockHash()) {
                    if (it->vecBlockContractData[i].second.empty()) {
                        MCHashWriter keyHash(SER_GETHASH, 0);
                        keyHash << contractId << it->vecBlockHash[i];
                        db.Read(keyHash.GetHash(), it->vecBlockContractData[i]);
                    }

                    contractInfo.txIndex = 0;
                    contractInfo.code = di->second.code;
                    contractInfo.blockHash = it->vecBlockHash[i];
                    contractInfo.coins = it->vecBlockContractData[i].first;
                    contractInfo.data = it->vecBlockContractData[i].second;
                    return it->blockHeight;
                }
            }
        }
    }

    return -1;
}
