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

//class CandidateHash
//{
//public:
//    uint256 preHash;
//    uint256 hash;
//    uint32_t height;
//
//
//    ADD_SERIALIZE_METHODS;
//
//    template <typename Stream, typename Operation>
//    inline void SerializationOp(Stream& s, Operation ser_action)
//    {
//        READWRITE(preHash);
//        READWRITE(hash);
//        READWRITE(height);
//    }
//};
//
//class TopHashData
//{
//public:
//    uint256 topHash;
//    uint32_t topHeight;
//    uint256 forkHash;
//    std::vector<CandidateHash> candidates;
//
//    ADD_SERIALIZE_METHODS;
//
//    template <typename Stream, typename Operation>
//    inline void SerializationOp(Stream& s, Operation ser_action)
//    {
//        READWRITE(topHash);
//        READWRITE(forkHash);
//        READWRITE(candidates);
//    }
//
//public:
//};

//typedef std::map<uint256, TopHashData> MAPTOPHASHDATAS;

class BranchBlockData
{
public:
    CellBlockHeader header; // 侧链spv
    int32_t nHeight;     // 侧链块高度
    CellTransactionRef pStakeTx;

    // mBlockHash 和 txIndex 为了查找到原来交易
    uint256 mBlockHash; // 主链打包块的hash
    uint256 txHash; // 侧链向主链发送侧链块spv的交易hash
    int txIndex;    // 交易在主链打包块的index

    arith_uint256 nChainWork;// 包含全部祖先和自己的work

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
    }

    enum {
        eADD,
        eDELETE,
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

    void BuildBestChain(BranchBlockData& blockdata);
    void ActivateBestChain(const uint256 &bestTipHash);
    void RemoveBlock(const uint256& blockhash);

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
    uint32_t Height(void);
};

typedef std::map<uint256, BranchData> MAPBRANCHS_DATA;

class BranchCache
{
public:
    MAPBRANCHS_DATA mapBranchCache;

    bool HasInCache(const CellTransaction& tx);
    void AddToCache(const CellTransaction& tx);
    void RemoveFromCache(const CellTransaction& tx);
    void RemoveFromBlock(const std::vector<CellTransactionRef>& vtx);

    std::vector<uint256> GetAncestorsBlocksHash(const CellTransaction& tx);

    const BranchBlockData* GetBranchBlockData(const uint256 &branchhash, const uint256 &blockhash);
};

const uint16_t FLAG_REPORTED = 1;
const uint16_t FLAG_PROVED = 2;

/*
 1、保证每个BranchData的mapHeads的BranchBlockData的preblock数据是存在的。
    也就是每个数据都有完整的到达创世块的链路径
 */

class BranchDb
{
public:
    BranchDb() = delete;
    BranchDb(const BranchDb&) = delete;
    BranchDb(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe);
    BranchDb& operator=(const BranchDb&) = delete;

    // flush data in connectblock and disconnnectblock, add or remove data.
    void Flush(const std::shared_ptr<const CellBlock>& pblock, bool fConnect);
    void OnConnectBlock(const std::shared_ptr<const CellBlock>& pblock);
    void OnDisconnectBlock(const std::shared_ptr<const CellBlock>& pblock);

    void AddBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch);
    void DelBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch);

    void LoadData();

    //void SetTopHash(const uint256& branchHash, const CellBlockHeader& bBlockHeader);

    uint256 GetBranchTipHash(const uint256& branchid);
    uint32_t GetBranchHeight(const uint256& branchid);
public:
 //   MAPTOPHASHDATAS mTopHashDatas;
    std::map<uint256, uint16_t> mReortTxFlag;

    bool HasBranchData(const uint256& branchHash) const
    {
        return mapBranchsData.count(branchHash) > 0;
    }
    BranchData GetBranchData(const uint256& branchHash)
    {
        return mapBranchsData[branchHash];
    }
private:
    bool WriteModifyToDB(const std::set<uint256>& modifyBranch);

    CellDBWrapper db;
    MAPBRANCHS_DATA mapBranchsData;
};

extern BranchDb* pBranchDb;

//branch mempool tx cache data
extern BranchCache branchDataMemCache;

#endif // BITCOIN_BRANCHDB_H
