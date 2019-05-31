// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef CONTRACT_DB_H
#define CONTRACT_DB_H

#include "transaction/txdb.h"

// 合约某高度存盘数据
class DBContractInfoByHeight
{
public:
    bool dirty = false;
    int32_t blockHeight;
    std::vector<uint256> vecBlockHash;
    std::vector<std::pair<MCAmount, std::string>> vecBlockContractData;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(blockHeight);
    }
};

// 区块关联的智能合约存盘数据
class DBContractInfo
{
public:
    std::string code;
    std::list<DBContractInfoByHeight> items;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(code);
        READWRITE(items);
    }
};

class MCContractID;

class ContractDataDB
{
private:
    MCDBWrapper db;
    MCDBBatch removeBatch;
    std::vector<uint160> removes;
    std::map<MCContractID, DBContractInfo> contractData;

public:
    ContractDataDB() = delete;
    ContractDataDB(const ContractDataDB&) = delete;
    ContractDataDB& operator=(const ContractDataDB&) = delete;
    ContractDataDB(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe);

    int GetContractInfo(const MCContractID& contractId, ContractInfo& contractInfo, const MCBlockIndex* currentPrevBlockIndex);

    bool WriteBatch(MCDBBatch& batch);
    bool WriteBlockContractInfoToDisk(const MCBlockIndex* pBlockIndex, const CONTRACT_DATA& contractContext);
    bool UpdateBlockContractToDisk(const MCBlockIndex* pBlockIndex);
    void PruneContractInfo();
};
extern ContractDataDB* mpContractDb;

#endif
