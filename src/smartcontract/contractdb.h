// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef CONTRACT_DB_H
#define CONTRACT_DB_H

#include "transaction/txdb.h"

#include <boost/threadpool.hpp>

// 执行智能合约时的上下文数据
struct ContractInfo
{
public:
    std::string code;
    std::string data;
};

// 智能合约的存盘数据
class DBContractInfo
{
public:
    uint256 blockHash;
    uint32_t blockHeight;
    std::string data;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(blockHash);
        READWRITE(blockHeight);
        READWRITE(data);
    }
};

// 区块关联的智能合约存盘数据
typedef std::list<DBContractInfo> DBContractList;
class DBBlockContractInfo
{
public:
    std::string code;
    DBContractList data;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(code);
        if (ser_action.ForRead()) {
            uint64_t size;
            READWRITE(size);
            for (int i = 0; i < size; ++i) {
                DBContractInfo dbContractInfo;
                READWRITE(dbContractInfo);
                data.emplace_back(dbContractInfo);
            }
        }
        else {
            uint64_t size = data.size();
            READWRITE(size);
            for (DBContractList::iterator it = data.begin(); it != data.end(); ++it) {
                READWRITE(*it);
            }
        }
        
    }
};

class CellContractID;
typedef std::map<CellContractID, DBBlockContractInfo> DBContractMap;

class ContractContext
{
    friend class ContractDataDB;

private:
    std::map<CellContractID, ContractInfo> _data;

public:
    void SetCache(const CellContractID& contractId, ContractInfo& contractInfo);
    void SetData(const CellContractID& contractId, ContractInfo& contractInfo);
    bool GetData(const CellContractID& contractId, ContractInfo& contractInfo);
    void Commit();
    void ClearCache();
    void ClearData();
    void ClearAll();
};

class SmartLuaState;
class CellLinkAddress;

struct SmartContractValidationData
{
    std::set<uint256> transactions;
    std::set<CellContractID> contracts;
};

class ContractDataDB
{
private:
    CellDBWrapper db;
    boost::threadpool::pool threadPool;
    std::map<boost::thread::id, SmartLuaState*> threadId2SmartLuaState;
    mutable CellCriticalSection cs_cache;

    // 合约缓存，同时包含多个合约对应的多个块合约数据快照
    DBContractMap _contractData;

public:
    ContractContext _contractContext;

public:
    ContractDataDB() = delete;
    ContractDataDB(const ContractDataDB&) = delete;
    ContractDataDB& operator=(const ContractDataDB&) = delete;
    ContractDataDB(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe);
    static void InitializeThread(ContractDataDB* contractDB);

    bool GetContractInfo(const CellContractID& contractId, ContractInfo& contractInfo, CellBlockIndex* currentPrevBlockIndex);

    bool RunBlockContract(const std::shared_ptr<const CellBlock> pblock, ContractContext* contractContext, CoinAmountCache* pCoinAmountCache);
    static void ExecutiveTransactionContractThread(ContractDataDB* contractDB, const std::shared_ptr<const CellBlock>& pblock,
        int offset, int size, int blockHeight, ContractContext* contractContext, CellBlockIndex* prefBlockIndex, SmartContractValidationData& validationData, CoinAmountCache* pCoinAmountCache);
    void ExecutiveTransactionContract(SmartLuaState* sls, const std::shared_ptr<const CellBlock>& pblock,
        int offset, int size, int blockHeight, ContractContext* contractContext, CellBlockIndex* prefBlockIndex, SmartContractValidationData& validationData, CoinAmountCache* pCoinAmountCache);

    void UpdateBlockContractInfo(CellBlockIndex* pBlockIndex, ContractContext* contractContext);
    void Flush();
};
extern ContractDataDB* mpContractDb;

extern CellAmount GetTxContractOut(const CellTransaction& tx);

#endif
