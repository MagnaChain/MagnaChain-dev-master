// Copyright (c) 2016-2018 The CellLink developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_BRANCHDB_H
#define BITCOIN_BRANCHDB_H

#include "chain.h"
#include "io/dbwrapper.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

//the state of report and prove
const uint16_t RP_INVALID = 0;
const uint16_t RP_FLAG_REPORTED = 1;//被举报了
const uint16_t RP_FLAG_PROVED = 2;//已证明

class BranchBlockData
{
public:
    BranchBlockData();

    typedef std::vector<uint256> VEC_HASH;
    typedef std::map<uint256, uint16_t> MAP_REPORT_PROVE;
    enum {//dead status
        eLive = 0,
        eDeadSelf = 1<<0,
        eDeadInherit = 1<<1,
    };

    CellBlockHeader header; // 侧链spv
    int32_t nHeight;     // 侧链块高度
    CellTransactionRef pStakeTx;

    // mBlockHash 和 txIndex 为了查找到原来交易
    uint256 mBlockHash; // 主链打包块的hash
    uint256 txHash; // 侧链向主链发送侧链块spv的交易hash
    int txIndex;    // 交易在主链打包块的index

    arith_uint256 nChainWork;// 包含全部祖先和自己的work
    VEC_HASH vecSonHashs;// 子block hash,make a block tree chain
    unsigned char deadstatus;
    MAP_REPORT_PROVE mapReportStatus;// deadstatus需要知道当前block的举报状态，和BranchDb的mReortTxFlag差不多，或者需要改成多key容器
                                  // 而且放这里比mReortTxFlag更有优势吧，数据分散到每个block，而不是集中起来。
    bool IsDead();//是否在被举报状态

    void InitDataFromTx(const CellTransaction& tx);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(header);
        READWRITE(nHeight);
        READWRITE(pStakeTx);
        READWRITE(mBlockHash);
        READWRITE(txHash);
        READWRITE(txIndex);

        READWRITE(nChainWork);
        READWRITE(vecSonHashs);
        READWRITE(deadstatus);
        READWRITE(mapReportStatus);
    }

    enum {
        eADD,
        eDELETE,
        //eModify,
        //eOriginal,
    };
    unsigned char flags; // memory only
};

typedef std::vector<uint256> VBRANCH_CHAIN;
typedef std::map<uint256, BranchBlockData> MAPBRANCH_HEADERS;
typedef std::map<uint256, uint256> MAP_MAINBLOCK_BRANCHTIP;
//
class BranchData
{
public:
    MAPBRANCH_HEADERS mapHeads;
    VBRANCH_CHAIN vecChainActive;
    MAP_MAINBLOCK_BRANCHTIP mapSnapshotBlockTip; // record connected main block, each branch tip

    BranchBlockData* GetBranchBlockData(const uint256& blockHash);
    void AddNewBlockData(BranchBlockData& blockdata);
    void ActivateBestChain(const uint256 &bestTipHash);
    void RemoveBlock(const uint256& blockhash);

    const BranchBlockData* GetAncestor(BranchBlockData* pBlock, int height);

    //
    void UpdateDeadStatus(const uint256& blockId, bool &fStatusChange);
    void DeadTransmit(const uint256& blockId);
    //
    void UpdateRebornStatus(const uint256& blockId, bool &fStatusChange);
    void RebornTransmit(const uint256& blockId);
    // 
    uint256 FindBestTipBlock();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(mapHeads);
        READWRITE(vecChainActive);
        READWRITE(mapSnapshotBlockTip);
    }

    void InitBranchGenesisBlockData(const uint256 &branchid);
    void SnapshotBlockTip(const uint256& mainBlockHash);
    void RecoverTip(const uint256& mainBlockHash);

    uint256 TipHash(void);
    int32_t Height(void);
    bool IsBlockInBestChain(const uint256& blockhash);
    int GetBlockMinedHeight(const uint256& blockhash);

    enum {
        eADD,
        eDELETE,
        eModify,
        eOriginal,
    };
    unsigned char flags; // memory only, cache data use it
private:
    void FindBestBlock(const uint256& blockhash, uint256& mostworkblock);
};

typedef std::map<uint256, BranchData> MAPBRANCHS_DATA;

//参考 CellCoinsView CellCoinsViewBacked CellCoinsViewCache CellCoinsViewDB 的机构来重写这里的cache
// Interface
class BrandchDataView
{
public:
    virtual bool HasBranchData(const uint256& branchHash) const;
    virtual uint256 GetBranchTipHash(const uint256& branchid);
    virtual uint32_t GetBranchHeight(const uint256& branchid);
    virtual BranchData GetBranchData(const uint256& branchHash);
    virtual bool IsBlockInActiveChain(const uint256& branchHash, const uint256& blockHash);
    virtual int GetBranchBlockMinedHeight(const uint256& branchHash, const uint256& blockHash);
    virtual uint16_t GetTxReportState(const uint256& rpBranchId, const uint256& rpBlockId, const uint256& flagHash);
};

// 数据操作(通用部分)
class BranchDataProcesser: public BrandchDataView
{
public:
    //std::map<uint256, uint16_t> mReortTxFlag;
    // <override
    uint256 GetBranchTipHash(const uint256& branchid) override;
    uint32_t GetBranchHeight(const uint256& branchid) override;
    bool HasBranchData(const uint256& branchHash) const override;
    BranchData GetBranchData(const uint256& branchHash) override;
    bool IsBlockInActiveChain(const uint256& branchHash, const uint256& blockHash) override;
    int GetBranchBlockMinedHeight(const uint256& branchHash, const uint256& blockHash) override;

    uint16_t GetTxReportState(const uint256& rpBranchId, const uint256& rpBlockId, const uint256& flagHash) override;
    //override>
    // flush data in connectblock and disconnnectblock, add or remove data.
    void Flush(const std::shared_ptr<const CellBlock>& pblock, bool fConnect);
protected:
    void OnConnectBlock(const std::shared_ptr<const CellBlock>& pblock);
    void OnDisconnectBlock(const std::shared_ptr<const CellBlock>& pblock);

    virtual void AddBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch);
    virtual void DelBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch);
    virtual bool AddReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch);
    virtual bool AddProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch);
    virtual bool DelReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch);
    virtual bool DelProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch);

    virtual bool WriteModifyToDB(const std::set<uint256>& modifyBranch);
protected:
    MAPBRANCHS_DATA mapBranchsData;
};

// 1、内存池的记录 2、verifydb时的那种临时db
// 缓存操作 临时操作 当失败时可以直接丢弃 而不污染数据源
// 优先从本地class取数据，在本地取不到的情况下再向readonly_db取
class BranchCache : public BranchDataProcesser
{
public:
    BranchCache() = delete;
    BranchCache(const BranchCache&) = delete;
    BranchCache(BrandchDataView *dataview) :readonly_db(dataview) {};
    BranchCache& operator=(const BranchCache&) = delete;

    BrandchDataView *readonly_db;// never modify db's data in this class. If data not in this class, fetch data from db.

    std::map<uint256, uint16_t> mReortTxFlagCache;

    bool HasInCache(const CellTransaction& tx);
    void AddToCache(const CellTransaction& tx);
    
    void RemoveFromBlock(const std::vector<CellTransactionRef>& vtx);

    //获取cache中的block的祖先
    std::vector<uint256> GetAncestorsBlocksHash(const CellTransaction& tx);

    const BranchBlockData* GetBranchBlockData(const uint256 &branchhash, const uint256 &blockhash);

    // <override
    uint256 GetBranchTipHash(const uint256& branchid) override;
    uint32_t GetBranchHeight(const uint256& branchid) override;
    bool HasBranchData(const uint256& branchHash) const override;
    BranchData GetBranchData(const uint256& branchHash) override;
    bool IsBlockInActiveChain(const uint256& branchHash, const uint256& blockHash) override;
    int GetBranchBlockMinedHeight(const uint256& branchHash, const uint256& blockHash) override;

    uint16_t GetTxReportState(const uint256& rpBranchId, const uint256& rpBlockId, const uint256& flagHash) override;
    //override>
    // overwrite, before do modify to data, load data from readonly_db if data not exist in local db.
    void AddBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch) override;
    void DelBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch) override;
    bool AddReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch) override;
    bool AddProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch) override;
    bool DelReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch) override;
    bool DelProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch) override;
    //
private:
    bool FetchDataFromSource(const uint256& branchId);
private:
    void RemoveFromCache(const CellTransaction& tx);
};

/*
 1、保证每个BranchData的mapHeads的BranchBlockData的preblock数据是存在的。
    也就是每个数据都有完整的到达创世块的链路径
 */
// 持久化
class BranchDb : public BranchDataProcesser
{
public:
    BranchDb() = delete;
    BranchDb(const BranchDb&) = delete;
    BranchDb(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe);
    BranchDb& operator=(const BranchDb&) = delete;
    
    void LoadData();
// <override
    //uint256 GetBranchTipHash(const uint256& branchid) override;
    //uint32_t GetBranchHeight(const uint256& branchid) override;
    //bool HasBranchData(const uint256& branchHash) const override;
    //BranchData GetBranchData(const uint256& branchHash) override;
    //bool IsBlockInActiveChain(const uint256& branchHash, const uint256& blockHash) override;
    //int GetBranchBlockMinedHeight(const uint256& branchHash, const uint256& blockHash) override;

    //uint16_t GetTxReportState(const uint256& rpBranchId, const uint256& rpBlockId, const uint256& flagHash) override;
//override>
protected:
    //void OnConnectBlock(const std::shared_ptr<const CellBlock>& pblock);
    //void OnDisconnectBlock(const std::shared_ptr<const CellBlock>& pblock);

    //void AddBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch);
    //void DelBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch);
    //bool AddReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch);
    //bool AddProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch);
    //bool DelReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch);
    //bool DelProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch);

    bool WriteModifyToDB(const std::set<uint256>& modifyBranch) override;
protected:
    CellDBWrapper db;
};

extern BranchDb* g_pBranchDb;

//branch mempool tx cache data
extern BranchCache* g_pBranchDataMemCache;

#endif // BITCOIN_BRANCHDB_H
