// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef CONTRACT_DB_H
#define CONTRACT_DB_H

#include "transaction/txdb.h"

#include <boost/threadpool.hpp>

// ִ�����ܺ�Լʱ������������
struct ContractInfo
{
public:
    std::string code;
    std::string data;
};

// ���ܺ�Լ�Ĵ�������
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

// ������������ܺ�Լ��������
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
typedef std::map<uint160, DBBlockContractInfo> DBContractMap;

class ContractContext
{
    friend class ContractDataDB;

private:
    std::map<CellKeyID, ContractInfo> _data;

public:
    void SetCache(const CellKeyID& key, ContractInfo& contractInfo);
    void SetData(const CellKeyID& key, ContractInfo& contractInfo);
    bool GetData(const CellKeyID& key, ContractInfo& contractInfo);
    void Commit();
    void ClearCache();
    void ClearData();
    void ClearAll();
};

class SmartLuaState;

class ContractDataDB
{
private:
    CellDBWrapper db;
    boost::threadpool::pool threadPool;
    std::map<boost::thread::id, SmartLuaState*> threadId2SmartLuaState;
    mutable CellCriticalSection cs_cache;

    // ��Լ���棬ͬʱ���������Լ��Ӧ�Ķ�����Լ���ݿ���
    DBContractMap _contractData;

public:
    ContractContext _contractContext;

public:
    ContractDataDB(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe);
    static void InitializeThread(ContractDataDB* contractDB);

    bool GetContractInfo(const CellKeyID& contractKey, ContractInfo& contractInfo, CellBlockIndex* currentPrevBlockIndex);

    bool RunBlockContract(const std::shared_ptr<const CellBlock> pblock, ContractContext* contractContext);
    static void ExecutiveTransactionContractThread(ContractDataDB* contractDB, const std::shared_ptr<const CellBlock>& pblock,
        int offset, int size, int blockHeight, ContractContext* contractContext, CellBlockIndex* prefBlockIndex, bool* interrupt);
    void ExecutiveTransactionContract(SmartLuaState* sls, const std::shared_ptr<const CellBlock>& pblock,
        int offset, int size, int blockHeight, ContractContext* contractContext, CellBlockIndex* prefBlockIndex, bool* interrupt);

    void UpdateBlockContractInfo(CellBlockIndex* pBlockIndex, ContractContext* contractContext);
    void Flush();
};
extern ContractDataDB* mpContractDb;

extern CellAmount GetTxContractOut(const CellTransaction& tx);

#endif
