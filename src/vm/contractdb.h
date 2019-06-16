// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef CONTRACT_DB_H
#define CONTRACT_DB_H

#include "transaction/txdb.h"
#include "vm/contract.h"

class DBContractData
{
public:
    MCAmount coins;
    std::string data;
    int txIndex;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(coins);
        READWRITE(data);
        READWRITE(txIndex);
    }
};

// 合约某高度存盘数据
class DBContractContextByHeight
{
public:
    bool dirty = false;
    int32_t blockHeight;
    std::vector<uint256> vecBlockHash;
    std::vector<DBContractData> vecBlockContractData;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(blockHeight);
    }
};

// 区块关联的智能合约存盘数据
class DBContractContext
{
public:
    std::string code;
    std::list<DBContractContextByHeight> items;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(code);
        READWRITE(items);
    }
};

class ContractDataDB
{
private:
    MCDBWrapper db;
    MCDBBatch removeBatch;
    std::vector<uint160> removes;
    std::map<MCContractID, DBContractContext> contractData;

public:
    ContractDataDB() = delete;
    ContractDataDB(const ContractDataDB&) = delete;
    ContractDataDB& operator=(const ContractDataDB&) = delete;
    ContractDataDB(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe);

    int GetContractContext(const MCContractID& contractId, ContractContext& context, const MCBlockIndex* currentPrevBlockIndex);

    bool WriteBatch(MCDBBatch& batch);
    bool WriteBlockContractContextToDisk(const MCBlockIndex* pBlockIndex, const MapContractContext& contractContext);
    bool UpdateBlockContractToDisk(const MCBlockIndex* pBlockIndex);
    void PruneContractContext();
};
extern ContractDataDB* mpContractDb;

#endif
