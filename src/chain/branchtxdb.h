// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BRANCH_TXDB_H
#define BRANCH_TXDB_H

#include "transaction/txdb.h"

//跨链交易接收方交易的key 
static const char DB_BRANCH_CHAIN_TX_DATA = 'b';
static const char DB_BRANCH_CHAIN_RECV_TX_DATA = 'r';
static const std::string DB_BRANCH_CHAIN_LIST = "chainlist";
static const char DB_MINE_COIN_LOCK = 'c';

class BranchChainTxEntry {
public:
    char key;
    uint256 txhash;

    BranchChainTxEntry(const uint256& hash, char dkey) : txhash(hash), key(dkey) {}

    template<typename Stream>
    void Serialize(Stream &s) const {
        s << key;
        s << txhash;
    }

    template<typename Stream>
    void Unserialize(Stream& s) {
        s >> key;
        s >> txhash;
    }

    friend inline bool operator==(const BranchChainTxEntry& a, const BranchChainTxEntry& b) { return a.txhash == b.txhash; }
    friend inline bool operator!=(const BranchChainTxEntry& a, const BranchChainTxEntry& b) { return !(a == b); }
    friend inline bool operator<(const BranchChainTxEntry& a, const BranchChainTxEntry& b)
    {
        return a.txhash < b.txhash;
    }
};

class DbDataFlag
{
public:
    DbDataFlag() {}
    DbDataFlag(unsigned char f):flags(f){}
    enum {
        eADD,
        eDELETE,
    };
    unsigned char flags;
};

class MCCreateBranchChainInfo
{
public:
    uint256 txid;
    uint256 blockhash;//in which block
    std::string branchVSeeds;
    std::string branchSeedSpec6;

    MCCreateBranchChainInfo()
    {}
    MCCreateBranchChainInfo(const MCCreateBranchChainInfo& from)
        :txid(from.txid), blockhash(from.blockhash), branchVSeeds(from.branchVSeeds), branchSeedSpec6(from.branchSeedSpec6)
    {
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(txid);
        READWRITE(blockhash);
        READWRITE(branchVSeeds);
        READWRITE(branchSeedSpec6);
    }

    friend inline bool operator==(const MCCreateBranchChainInfo& a, const MCCreateBranchChainInfo& b)
    {
        return a.txid == b.txid && a.blockhash == b.blockhash && a.branchVSeeds == b.branchVSeeds && a.branchSeedSpec6 == b.branchSeedSpec6;
    }
};

class BranchChainTxInfo :public DbDataFlag
{
public:
    BranchChainTxInfo() :txindex(0) { blockhash.SetNull(); }
    BranchChainTxInfo(const BranchChainTxInfo& from) :blockhash(from.blockhash), txindex(from.txindex), txnVersion(from.txnVersion) {}

    uint256 blockhash;
    uint32_t txindex;
    int32_t txnVersion;
    MCCreateBranchChainInfo createchaininfo; // temp data, not serialize

    bool IsInit() { return !blockhash.IsNull() && txindex != 0; }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(blockhash);
        READWRITE(txindex);
        READWRITE(txnVersion);
    }

    friend inline bool operator==(const BranchChainTxInfo& a, const BranchChainTxInfo& b) { return a.blockhash == b.blockhash && a.txindex == b.txindex && a.txnVersion == b.txnVersion; }
    friend inline bool operator!=(const BranchChainTxInfo& a, const BranchChainTxInfo& b) { return !(a == b); }
};

class BranchChainTxRecvInfo :public DbDataFlag
{
public:
    uint256 blockhash;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(blockhash);
    }
};

class CoinReportInfo : public DbDataFlag
{
public:
    CoinReportInfo() {}
    CoinReportInfo(uint256 txid, unsigned char flag) :reporttxid(txid),DbDataFlag(flag) {}

    uint256 reporttxid;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(reporttxid);
    }
};

typedef std::map<BranchChainTxEntry, BranchChainTxInfo> BRANCH_CHAIN_INFO_MAP;// [tx hash, sendinfo]
typedef std::map<BranchChainTxEntry, BranchChainTxRecvInfo> BRANCH_CHAIN_RECV_MAP; //[key, recvinfo]
typedef std::map<uint256, std::vector<CoinReportInfo>> COIN_BE_REPORT;// [coinpreouthash,vector<un_prove_reporttxid>] 

// 在connectblock 或者disconnectblock 时,需要数据库操作先写到cache里面,
// 待整个过程没错后才把cache的数据写到数据库
class BranchChainTxRecordsCache
{
private:
    //已创建的支链和发起跨链交易
    void AddBranchChainTxRecord(const MCTransactionRef& tx, const uint256& blockhash, uint32_t txindex);
    void DelBranchChainTxRecord(const MCTransactionRef& tx);

    //已接收的跨链交易
    void AddBranchChainRecvTxRecord(const MCTransactionRef& tx, const uint256& blockhash);
    void DelBranchChainRecvTxRecord(const MCTransactionRef& tx);
public:
    void AddToCache(const MCTransactionRef& ptx, const uint256& blockhash, int blocktxindex);
    void RemoveFromCache(const MCTransactionRef& ptx);

    bool HasInCache(const MCTransaction& tx);
    void RemoveFromBlock(const std::vector<MCTransactionRef>& vtx);

    //锁币解锁
    /**
     * fBlockConnect is in block connect or block disconnect
     */
    void UpdateLockMineCoin(const MCTransactionRef& ptx, bool fBlockConnect);

    BRANCH_CHAIN_INFO_MAP m_mapChainTxInfos;
    BRANCH_CHAIN_RECV_MAP m_mapRecvRecord;//temp record.
    COIN_BE_REPORT m_mapCoinBeReport;
};

//跨链交易的相关记录 
class BranchChainTxRecordsDb
{
public:
    BranchChainTxRecordsDb() = delete;
    BranchChainTxRecordsDb(const BranchChainTxRecordsDb&) = delete;

    BranchChainTxRecordsDb(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe);

    BranchChainTxInfo GetBranchChainTxInfo(const uint256& txid);

    bool IsTxRecvRepeat(const MCTransaction& tx, const MCBlock* pBlock = nullptr);

    void Flush(BranchChainTxRecordsCache &cache);

    typedef std::vector<MCCreateBranchChainInfo> CREATE_BRANCH_TX_CONTAINER;

    size_t GetCreateBranchSize() { return m_vCreatedBranchTxs.size(); }
    const CREATE_BRANCH_TX_CONTAINER& GetCreateBranchTxsInfo() { return m_vCreatedBranchTxs; }
    bool IsBranchCreated(const uint256 &branchid) const;

    bool IsMineCoinLock(const uint256& coinhash) const;
private:
    MCDBWrapper m_db;
    CREATE_BRANCH_TX_CONTAINER m_vCreatedBranchTxs;
};
extern BranchChainTxRecordsDb* g_pBranchChainTxRecordsDb;

extern BranchChainTxRecordsCache* g_pBranchTxRecordCache;
#endif