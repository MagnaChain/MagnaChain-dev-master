// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef CONTRACT_DB_H
#define CONTRACT_DB_H

#include "transaction/txdb.h"
#include <boost/threadpool.hpp>

// 智能合约的存盘数据
class DBContractInfo
{
public:
    ContractDataFrom from;
    uint32_t blockHeight;
    std::string data;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(from);
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
        READWRITE(data);
    }
};

class MCContractID;
typedef std::map<MCContractID, DBBlockContractInfo> DBContractMap;
typedef std::map<MCContractID, ContractInfo> CONTRACT_DATA;

class ContractTxFinalData
{
public:
    MCAmount coins;
    std::vector<CONTRACT_DATA> data;
};

class ContractContext
{
    friend class ContractDataDB;

public:
    CONTRACT_DATA cache;    // 数据缓存，用于回滚
    CONTRACT_DATA data;
    ContractTxFinalData txFinalData;
    CONTRACT_DATA prevData;

public:
    void SetCache(const MCContractID& contractId, ContractInfo& contractInfo);
    void SetData(const MCContractID& contractId, ContractInfo& contractInfo);
    bool GetData(const MCContractID& contractId, ContractInfo& contractInfo);
    void Commit();
    void ClearCache();
    void ClearData();
    void ClearAll();
};

class SmartLuaState;
class MagnaChainAddress;

struct SmartContractThreadData
{
    int offset;
    uint16_t groupSize;
    int blockHeight;
    ContractContext contractContext;
    MCBlockIndex* pPrevBlockIndex;
    CoinAmountCache* pCoinAmountCache;
    std::set<uint256> associationTransactions;
};

typedef std::map<uint256, std::vector<std::map<MCContractID, ContractInfo>>> BLOCK_CONTRACT_DATA;
class ContractDataDB
{
private:
    MCDBWrapper db;
    boost::threadpool::pool threadPool;
    std::map<boost::thread::id, SmartLuaState*> threadId2SmartLuaState;
    mutable MCCriticalSection cs_cache;

    // 合约缓存，同时包含多个合约对应的多个块合约数据快照
    DBContractMap contractData;
    BLOCK_CONTRACT_DATA blockContractData;
    std::map<int, std::vector<std::pair<uint256, bool>>> mapHeightHash;

public:
    ContractContext contractContext;

public:
    ContractDataDB() = delete;
    ContractDataDB(const ContractDataDB&) = delete;
    ContractDataDB& operator=(const ContractDataDB&) = delete;
    ContractDataDB(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe);
    static void InitializeThread(ContractDataDB* contractDB);

    int GetContractInfo(const MCContractID& contractId, ContractInfo& contractInfo, MCBlockIndex* currentPrevBlockIndex);

    bool RunBlockContract(MCBlock* pBlock, ContractContext* pContractContext, CoinAmountCache* pCoinAmountCache);
    static void ExecutiveTransactionContractThread(ContractDataDB* contractDB, MCBlock* pBlock, SmartContractThreadData* threadData);
    void ExecutiveTransactionContract(SmartLuaState* sls, MCBlock* pBlock, SmartContractThreadData* threadData);

    void UpdateBlockContractInfo(MCBlockIndex* pBlockIndex, ContractContext* contractContext);
    void Flush();
};
extern ContractDataDB* mpContractDb;

extern MCAmount GetTxContractOut(const MCTransaction& tx);

#endif
