// Copyright (c) 2016-2018 The CellLink developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "branchdb.h"

#include "primitives/block.h"
#include "chainparams.h"
#include "coding/hash.h"
#include "init.h"
#include "misc/pow.h"
#include "misc/random.h"
#include "ui/ui_interface.h"
#include "coding/uint256.h"
#include "utils/util.h"
#include "validation/validation.h"
#include <stdint.h>

#include "primitives/block.h"
#include <boost/thread.hpp>
#include "chain/chain.h"

//static const std::string DB_MAP_TOP_HASH_DATA = "db_map_top_hash_data";
static const std::string DB_MAP_REPORT_PROVE_DATA = "db_mao_report_prove_data";

BranchDb* pBranchDb = nullptr;

BranchCache branchDataMemCache;

void BranchData::InitBranchGenesisBlockData(const uint256 &branchid)
{
    const CellChainParams& bparams = BranchParams(branchid);
    const CellBlock& genesisblock = bparams.GenesisBlock();
    if (mapHeads.count(genesisblock.GetHash()))
        return;

    BranchBlockData& blockdata = mapHeads[genesisblock.GetHash()];
    blockdata.header = genesisblock.GetBlockHeader();
    blockdata.nHeight = 0;
    blockdata.pStakeTx = MakeTransactionRef();
    blockdata.nChainWork = GetBlockProof(genesisblock.nBits);

    vecChainActive.push_back(blockdata.header.GetHash());
}

uint256 BranchData::TipHash(void)
{
    if (vecChainActive.size() == 0)
    {
        return uint256();
    }
    return vecChainActive.back();
}

uint32_t BranchData::Height(void)
{
    return vecChainActive.size() - 1;
}

void BranchData::BuildBestChain(BranchBlockData& blockdata)
{
    const uint256 newTipHash = blockdata.header.GetHash();

    mapHeads[newTipHash] = blockdata;

    if (vecChainActive.back() == blockdata.header.hashPrevBlock)
    {
        vecChainActive.push_back(newTipHash);
    }
    else
    {
        const BranchBlockData& tipBlock = mapHeads[vecChainActive.back()];
        if (blockdata.nChainWork > tipBlock.nChainWork)
        {
            std::vector<uint256> forkChain;
            forkChain.push_back(newTipHash);
            uint256 forkHash = mapHeads[newTipHash].header.hashPrevBlock;
            while (vecChainActive[mapHeads[forkHash].nHeight] != forkHash)
            {
                forkChain.push_back(forkHash);
                forkHash = mapHeads[forkHash].header.hashPrevBlock;
                vecChainActive.pop_back();
            }
            vecChainActive.insert(vecChainActive.end(), forkChain.rbegin(), forkChain.rend());
        }
        //else
    }
}

void BranchData::RemoveBlock(const uint256& blockhash)
{
    mapHeads.erase(blockhash);

    if (vecChainActive.back() == blockhash)
    {
        vecChainActive.pop_back();
    }
}

bool BranchCache::HasInCache(const CellTransaction& tx)
{
    BranchBlockData blockData;
    blockData.InitDataFromTx(tx);

    uint256 blockHash = blockData.header.GetHash();
    uint256 branchHash = tx.pBranchBlockData->branchID;

    if (mapBranchCache.count(branchHash)){
        if (mapBranchCache[branchHash].mapHeads.count(blockHash)){
            if (mapBranchCache[branchHash].mapHeads[blockHash].flags == BranchBlockData::eADD){
                return true;
            }
        }
    }
    return false;
}

void BranchCache::AddToCache(const CellTransaction& tx)
{
    BranchBlockData blockData;
    blockData.flags = BranchBlockData::eADD;
    blockData.InitDataFromTx(tx);

    blockData.txHash = tx.GetHash();

    uint256 blockHash = blockData.header.GetHash();
    uint256 branchHash = tx.pBranchBlockData->branchID;

    BranchData& bData = mapBranchCache[branchHash];
    bData.mapHeads[blockHash] = blockData;
}

// Use in follow situation. 1.After finish build block
// keep cache 
void BranchCache::RemoveFromCache(const CellTransaction& tx)
{
    BranchBlockData blockData;
    blockData.flags = BranchBlockData::eDELETE;
    blockData.InitDataFromTx(tx);

    blockData.txHash = tx.GetHash();

    uint256 blockHash = blockData.header.GetHash();
    uint256 branchHash = tx.pBranchBlockData->branchID;

    BranchData& bData = mapBranchCache[branchHash];
    if (bData.mapHeads.count(blockHash))//update map data
        bData.mapHeads[blockHash].flags = BranchBlockData::eDELETE;
    else                                // set map data
        bData.mapHeads[blockHash] = blockData;
}

void BranchCache::RemoveFromBlock(const std::vector<CellTransactionRef>& vtx)
{
    for (const auto& tx : vtx)
    {
        if (tx->IsSyncBranchInfo()){
            RemoveFromCache(*tx);
        }
    }
}

std::vector<uint256> BranchCache::GetAncestorsBlocksHash(const CellTransaction& tx)
{
    std::vector<uint256> vecRet;

    BranchBlockData blockData;
    blockData.InitDataFromTx(tx);

    //uint256 blockHash = blockData.header.GetHash();
    uint256 branchHash = tx.pBranchBlockData->branchID;

    if (mapBranchCache.count(branchHash))
    {
        BranchData& branchdata = mapBranchCache[branchHash];
        uint256& preblockhash = blockData.header.hashPrevBlock;
        while (branchdata.mapHeads.count(preblockhash)){
            BranchBlockData& preblockdata = branchdata.mapHeads[preblockhash];
            vecRet.push_back(preblockdata.txHash);
            preblockhash = preblockdata.header.hashPrevBlock;
        }
    }

    return vecRet;
}

const BranchBlockData* BranchCache::GetBranchBlockData(const uint256 &branchhash, const uint256 &blockhash)
{
    if (mapBranchCache.count(branchhash)){
        if (mapBranchCache[branchhash].mapHeads.count(blockhash)){
            if (mapBranchCache[branchhash].mapHeads[blockhash].flags == BranchBlockData::eADD)
            {
                return &mapBranchCache[branchhash].mapHeads[blockhash];
            }
        }
    }
    return nullptr;
}

BranchDb::BranchDb(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe)
    : db(path, nCacheSize, fMemory, fWipe, true)
{
}

void BranchBlockData::InitDataFromTx(const CellTransaction& tx)
{
    BranchBlockData& bBlockData = *this;
    //set block header
    tx.pBranchBlockData->GetBlockHeader(bBlockData.header);

    bBlockData.nHeight = tx.pBranchBlockData->blockHeight;

    CellDataStream cds(tx.pBranchBlockData->vchStakeTxData, SER_NETWORK, INIT_PROTO_VERSION);
    cds >> (bBlockData.pStakeTx);

    bBlockData.txHash = tx.GetHash();
}

void BranchDb::Flush(const std::shared_ptr<const CellBlock>& pblock, bool fConnect)
{
    if (!Params().IsMainChain()) {
        return;
    }

    if (fConnect){
        OnConnectBlock(pblock);
    }
    else {
        OnDisconnectBlock(pblock);
    }
//    uint256 mtopKey = Hash(DB_MAP_TOP_HASH_DATA.begin(), DB_MAP_TOP_HASH_DATA.end());
//    db.Write(mtopKey, mTopHashDatas);
    uint256 mReportKey = Hash(DB_MAP_REPORT_PROVE_DATA.begin(), DB_MAP_REPORT_PROVE_DATA.end());
    db.Write(mReportKey, mReortTxFlag);
}

void BranchDb::OnConnectBlock(const std::shared_ptr<const CellBlock>& pblock)
{
    const CellBlock& block = *pblock;
    const std::vector<CellTransactionRef>& trans = block.vtx;
    const uint256 blockHash = block.GetHash();
    for (size_t i = 0; i < trans.size(); ++i) {
        CellTransactionRef transaction = trans[i];
        if (transaction->IsSyncBranchInfo()) {
            AddBlockInfoTxData(transaction, blockHash, i);
        }
    }
}

void BranchDb::OnDisconnectBlock(const std::shared_ptr<const CellBlock>& pblock)
{
    const CellBlock& block = *pblock;
    const std::vector<CellTransactionRef>& trans = block.vtx;
    const uint256 blockHash = block.GetHash();
    for (int i = trans.size() - 1; i >= 0; i--) {
        CellTransactionRef transaction = trans[i];
        if (transaction->IsSyncBranchInfo()) {
            DelBlockInfoTxData(transaction, blockHash, i);
        }
    }
}

//interface for test easy
void BranchDb::AddBlockInfoTxData(CellTransactionRef &transaction, const uint256 &blockHash, const size_t iTxVtxIndex)
{
    BranchBlockData bBlockData;
    bBlockData.InitDataFromTx(*transaction);

    //uint256 bBlockHash = bBlockData.header.GetHash();
    uint256 branchHash = transaction->pBranchBlockData->branchID;
    BranchData& bData = mapBranchsData[branchHash];
    bData.InitBranchGenesisBlockData(branchHash);

    const uint256& hashPrevBlock = bBlockData.header.hashPrevBlock;
    //tx from blockhash and vtx index
    bBlockData.mBlockHash = blockHash;
    bBlockData.txIndex = iTxVtxIndex;
    bBlockData.nChainWork = (bData.mapHeads.count(hashPrevBlock) ? bData.mapHeads[hashPrevBlock].nChainWork : 0)
        + GetBlockProof(bBlockData.header.nBits);

    bData.BuildBestChain(bBlockData);

    db.Write(branchHash, bData);

    //SetTopHash(branchHash, bBlockData.header);
}

//interface for test easy
void BranchDb::DelBlockInfoTxData(CellTransactionRef &transaction, const uint256 &blockHash, const size_t iTxVtxIndex)
{
    BranchBlockData bBlockData;
    bBlockData.InitDataFromTx(*transaction);

    uint256 bBlockHash = bBlockData.header.GetHash();
    uint256 branchHash = transaction->pBranchBlockData->branchID;
    BranchData& bData = mapBranchsData[branchHash];

    bData.RemoveBlock(bBlockHash);

    db.Write(branchHash, bData);
}

uint256 BranchDb::GetBranchTipHash(const uint256& branchid)
{
    if (!HasBranchData(branchid))
    {
        return uint256();
    }
    BranchData& branchdata = mapBranchsData[branchid];
    return branchdata.TipHash();
}

uint32_t BranchDb::GetBranchHeight(const uint256& branchid)
{
    if (!HasBranchData(branchid))
    {
        return 0;
    }
    BranchData& branchdata = mapBranchsData[branchid];
    return branchdata.Height();
}

void BranchDb::LoadData()
{
    //LogPrintf("===== 1-branch db load data: %s \n", Params().GetBranchId()); 
    if (!Params().IsMainChain()) {
        //LogPrintf("===== 2-branch db load data: %s \n", Params().GetBranchId());  
        return;
    }
    //uint256 mtopKey = Hash(DB_MAP_TOP_HASH_DATA.begin(), DB_MAP_TOP_HASH_DATA.end()); 
    uint256 mReportKey = Hash(DB_MAP_REPORT_PROVE_DATA.begin(), DB_MAP_REPORT_PROVE_DATA.end()); 
    CellDBIterator* it = db.NewIterator();
    for (it->SeekToFirst(); it->Valid(); it->Next()){
        uint256 keyHash ;
        
        if (it->GetKey(keyHash))
        {
            /*if (keyHash == mtopKey)
            {
                MAPTOPHASHDATAS mtData;
                if (it->GetValue(mtData))
                {
                    mTopHashDatas = mtData;
                    continue;
                }
            }
            else */
            if (keyHash == mReportKey)
            {
                std::map<uint256, uint16_t> mrData;
                if (it->GetValue(mrData))
                {
                    mReortTxFlag = mrData;
                    continue;
                }
                 
            }
            BranchData data;
            if(it->GetValue(data))
            {
                mapBranchsData[keyHash] = data; 
                //LogPrintf("===== 3-branch db load data: %s \n", branchHash.GetHex());
            }
        }
    }
    /*
    MAPTOPHASHDATAS mtData;
    db.Read(mtopKey, mtData);
    mTopHashDatas = mtData;
    */
}


//void BranchDb::SetTopHash(const uint256& branchHash, const CellBlockHeader& bBlockHeader)
//{
//    if (!Params().IsMainChain()) {
//        return;
//    }
//    TopHashData& tHashData = mTopHashDatas[branchHash];
//    std::vector<CandidateHash>& candidates = tHashData.candidates;
//
//    uint256 preBlockHash = bBlockHeader.hashPrevBlock;
//    uint256 blockHash = bBlockHeader.GetHash();
//    if (tHashData.topHash.IsNull() && candidates.size() <= 0 && tHashData.forkHash.IsNull()) {
//        tHashData.topHash = blockHash;
//        tHashData.topHeight += 1;
//        return;
//    }
//
//    if (preBlockHash == tHashData.topHash) {
//        tHashData.forkHash = tHashData.topHash;
//        tHashData.topHash = blockHash;
//        tHashData.topHeight += 1;
//        return;
//    }
//
//    if (preBlockHash == tHashData.forkHash) {
//        CandidateHash tmp;
//        tmp.hash = blockHash;
//        tmp.height = 1;
//        tmp.preHash = preBlockHash;
//        candidates.emplace_back(tmp);
//    }
//
//    bool IsFork = false;
//    CandidateHash fHash;
//    for (size_t i = 0; i < candidates.size(); ++i) {
//        if (preBlockHash == candidates[i].hash) {
//            candidates[i].hash = blockHash;
//            candidates[i].preHash = preBlockHash;
//            candidates[i].height = candidates[i].height + 1;
//            uint32_t cHeight = candidates[i].height;
//            if (tHashData.topHeight == cHeight) {
//                tHashData.topHash = tHashData.forkHash;
//                tHashData.topHeight = tHashData.topHeight - cHeight;
//            } else {
//                tHashData.topHash = blockHash;
//                tHashData.topHeight = tHashData.topHeight + 1;
//            }
//            break;
//        } else if (preBlockHash == candidates[i].preHash) {
//            IsFork = true;
//            fHash = candidates[i];
//            break;
//        }
//    }
//
//    if (IsFork) {
//        candidates.clear();
//        candidates.emplace_back(fHash);
//        CandidateHash tmp;
//        tmp.hash = blockHash;
//        tmp.height = 1;
//        tmp.preHash = preBlockHash;
//        candidates.emplace_back(tmp);
//
//        tHashData.forkHash = preBlockHash;
//        tHashData.topHash = preBlockHash;
//        tHashData.topHeight = tHashData.topHeight - 1;
//    }
//}
