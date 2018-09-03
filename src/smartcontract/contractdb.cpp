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
        if (opcode == OP_PUB_CONTRACT || opcode == OP_TRANS_CONTRACT) {
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

static std::map<CellKeyID, ContractInfo> cache;    // ���ݻ��棬���ڻع�

void ContractContext:: SetCache(const CellKeyID& key, ContractInfo& contractInfo)
{
    cache[key] = std::move(contractInfo);
}

void ContractContext::SetData(const CellKeyID& key, ContractInfo& contractInfo)
{
    _data[key] = std::move(contractInfo);
}

bool ContractContext::GetData(const CellKeyID& key, ContractInfo& contractInfo)
{
    if (cache.size() > 0) {
        auto it = cache.find(key);
        if (it == cache.end()) {
            contractInfo.code = it->second.code;
            contractInfo.data = it->second.data;
            return true;
        }
    }

    auto it = _data.find(key);
    if (it != _data.end()) {
        contractInfo.code = it->second.code;
        contractInfo.data = it->second.data;
        return true;
    }

    return false;
}

void ContractContext::Commit()
{
    for (auto it : cache)
        _data[it.first] = std::move(it.second);
    ClearCache();
}

void ContractContext::ClearCache()
{
    cache.clear();
}

void ContractContext::ClearData()
{
    _data.clear();
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
    contractDB->threadId2SmartLuaState.insert(std::make_pair(boost::this_thread::get_id(), new SmartLuaState()));
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));
}

void ContractDataDB::ExecutiveTransactionContractThread(ContractDataDB* contractDB, const std::shared_ptr<const CellBlock>& pblock,
    int offset, int size, int blockHeight, ContractContext* pContractContext, CellBlockIndex* pPrefBlockIndex, bool* interrupt)
{
    auto it = contractDB->threadId2SmartLuaState.find(boost::this_thread::get_id());
    if (it == contractDB->threadId2SmartLuaState.end()) {
        LogPrintf("%s sls == nullptr\n", __FUNCTION__);
        return;
    }

    if (contractDB != nullptr)
        contractDB->ExecutiveTransactionContract(it->second, pblock, offset, size, blockHeight, pContractContext, pPrefBlockIndex, interrupt);
}

void ContractDataDB::ExecutiveTransactionContract(SmartLuaState* sls, const std::shared_ptr<const CellBlock>& pBlock,
    int offset, int size, int blockHeight, ContractContext* pContractContext, CellBlockIndex* pPrefBlockIndex, bool* interrupt)
{
    for (int i = offset; i < offset + size; ++i) {
        if (*interrupt)
            break;

        const CellTransactionRef& tx = pBlock->vtx[i];
        if (tx->IsNull() || !tx->IsSmartContract())
            continue;

        CellKeyID contractKey = tx->contractAddrs[0];
        CellLinkAddress contractAddrs(contractKey);
        CellLinkAddress senderAddr(tx->contractSender.GetID());
        CellAmount amount = GetTxContractOut(*tx);

        if (tx->nVersion == CellTransaction::PUBLISH_CONTRACT_VERSION) {
            std::string code;
            std::string rawCode = tx->contractCode;
            SmartContractRet scr;
            sls->Initialize(pBlock->GetBlockTime(), blockHeight, pContractContext, pPrefBlockIndex, SmartLuaState::SAVE_TYPE_DATA);
            int result = PublishContract(sls, amount, contractAddrs, senderAddr, rawCode, code, scr);
            if (result != 0)
                *interrupt = true;
        }
        else if (tx->nVersion == CellTransaction::CALL_CONTRACT_VERSION) {
            std::string strFuncName = tx->contractFun;
            UniValue args;
            args.read(tx->contractParams);

            SmartContractRet scr;
            long callNum = MAX_CONTRACT_CALL;
            sls->Initialize(pBlock->GetBlockTime(), blockHeight, pContractContext, pPrefBlockIndex, SmartLuaState::SAVE_TYPE_DATA);
            int result = CallContract(sls, callNum, amount, contractAddrs, senderAddr, strFuncName, args, scr);
            if (result != 0)
                *interrupt = true;
        }
    }
}

bool ContractDataDB::RunBlockContract(const std::shared_ptr<const CellBlock> pblock, ContractContext* pContractContext)
{
    auto it = mapBlockIndex.find(pblock->hashPrevBlock);
    if (it == mapBlockIndex.end())
        return false;
    
    CellBlockIndex* prevBlockInex = it->second;
    int blockHeight = prevBlockInex->nHeight + 1;

    int offset = 0;
    bool interrupt = false;
    printf("Generate block vtx size:%d, group:%d\n", pblock->vtx.size(), pblock->groupSize.size());
    std::vector<std::set<uint256>> groupTransaction;
    for (int i = 0; i < pblock->groupSize.size(); ++i) {
        threadPool.schedule(boost::bind(ExecutiveTransactionContractThread, this, pblock, offset, pblock->groupSize[i], blockHeight, pContractContext, prevBlockInex, &interrupt));
        offset += pblock->groupSize[i];
    }
    threadPool.wait();

    if (interrupt)
        return false;

    std::set<uint256> checker;
    for (int i = 0; i < groupTransaction.size(); ++i) {
        for (auto item : groupTransaction[i]) {
            if (checker.find(item) == checker.end())
                checker.insert(item);
            else
                return false;
        }
    }

    return true;
}

void ContractDataDB::UpdateBlockContractInfo(CellBlockIndex* pBlockIndex, ContractContext* pContractContext)
{
    if (pBlockIndex == nullptr)
        return;

    int blockHeight = pBlockIndex->nHeight;
    int confirmBlockHeight = blockHeight - DEFAULT_CHECKBLOCKS;

    if (confirmBlockHeight > 0) {
        // ��������ĺ�Լ���ݣ�������ȷ����Ч�ֲ������
        CellBlockIndex* newConfirmBlock = pBlockIndex->GetAncestor(confirmBlockHeight);
        for (auto ci : _contractData) {
            DBBlockContractInfo& dbBlockContractInfo = ci.second;

            DBContractList::iterator saveIt = dbBlockContractInfo.data.end();
            for (DBContractList::iterator it = dbBlockContractInfo.data.begin(); it != dbBlockContractInfo.data.end();) {
                if (it->blockHeight <= confirmBlockHeight) {
                    BlockMap::iterator bi = mapBlockIndex.find(it->blockHash);
                    // ��С��ȷ���������µĲ����ڸ��������������Ƴ���
                    if (bi == mapBlockIndex.end() || newConfirmBlock->GetAncestor(it->blockHeight)->GetBlockHash() != it->blockHash) {
                        DBContractList::iterator temp = it++;
                        dbBlockContractInfo.data.erase(temp);
                        continue;
                    }
                    if (it->blockHeight < confirmBlockHeight) {
                        // ������ǰȷ�Ͽ����µ����һ�����ݣ���ֹ�����˳�ʱ������������Ϣ���������¼��ص���������
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

    // ����������Ӧ�Ľ����
    for (auto ci : pContractContext->_data) {
        DBContractInfo dbContractInfo;
        dbContractInfo.blockHash = pBlockIndex->GetBlockHash();
        dbContractInfo.blockHeight = pBlockIndex->nHeight;
        dbContractInfo.data = ci.second.data;

        // ���߶�Ѱ�Ҳ����
        DBBlockContractInfo& blockContractInfo = _contractData[ci.first];
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
    for (auto it : _contractData) {
        DBBlockContractInfo& dbBlockContractInfo = it.second;
        if (dbBlockContractInfo.data.size() == 0) {
            // �ú�Լû����Ч�����ݣ����ж�Ϊ�ֲ����Ч�����ݣ��Ƴ�֮
            batch.Erase(it.first);
            removes.emplace_back(it.first);
        }
        else
            batch.Write(it.first, it.second);

        // �������Ѳ�������ֻʣ��һ�����ݣ�����̺���ڴ��Ƴ�
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

    for (uint160& contractKey : removes)
        _contractData.erase(contractKey);
}

bool ContractDataDB::GetContractInfo(const CellKeyID& contractKey, ContractInfo& contractInfo, CellBlockIndex* currentPrevBlockIndex)
{
    LOCK(cs_cache);

    // ����Ƿ��ڻ����У����������Լ���
    DBContractMap::iterator di = _contractData.find(contractKey);
    if (di == _contractData.end()) {
        DBBlockContractInfo dbBlockContractInfo;
        if (!db.Read(contractKey, dbBlockContractInfo))
            return false;
        else {
            _contractData[contractKey] = dbBlockContractInfo;
            di = _contractData.find(contractKey);
        }
    }

    // ������ȡ��Ӧ�ڵ������
    CellBlockIndex* prevBlock = (currentPrevBlockIndex ? currentPrevBlockIndex : chainActive.Tip());
    int blockHeight = prevBlock->nHeight;
    // ������ĩβ�洢��ߵ�����
    for (DBContractList::reverse_iterator it = di->second.data.rbegin(); it != di->second.data.rend(); ++it) {
        if (it->blockHeight <= blockHeight) {
            BlockMap::iterator bi = mapBlockIndex.find(it->blockHash);
            assert(bi != mapBlockIndex.end());
            CellBlockIndex* checkBlockIndex = prevBlock->GetAncestor(it->blockHeight);
            // �ҵ�ͬһ�ֲ�ʱ
            if (checkBlockIndex == bi->second) {
                contractInfo.code = di->second.code;
                contractInfo.data = it->data;
                return true;
            }
        }
    }

    return false;
}
