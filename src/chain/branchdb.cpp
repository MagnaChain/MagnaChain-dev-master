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

void BranchData::SnapshotBlockTip(const uint256& mainBlockHash)
{
    mapSnapshotBlockTip[mainBlockHash] = vecChainActive.back();

    //prune snapshot data
    if (mapSnapshotBlockTip.size() > 200)
    {
        MAP_MAINBLOCK_BRANCHTIP::iterator oldest = mapSnapshotBlockTip.begin();
        for (MAP_MAINBLOCK_BRANCHTIP::iterator mit = mapSnapshotBlockTip.begin();
            mit != mapSnapshotBlockTip.end(); mit++)
        {
            if (mapBlockIndex[oldest->first]->nHeight > mapBlockIndex[mit->first]->nHeight)
            {
                oldest = mit;
            }
        }
        mapSnapshotBlockTip.erase(oldest);
    }
}

void BranchData::RecoverTip(const uint256& mainBlockHash)
{
    uint256 preblockhash = mainBlockHash;
    while (mapSnapshotBlockTip.count(preblockhash) == 0)// find last best tip
    {
        if (mapBlockIndex.count(preblockhash) == 0 || preblockhash.IsNull()){
            //assert error
            break;
        }
        preblockhash = mapBlockIndex[preblockhash]->GetBlockHeader().hashPrevBlock;
    }
    // 
    if (mapSnapshotBlockTip.count(preblockhash))
    {
        const uint256 bestTip = mapSnapshotBlockTip[preblockhash];
        ActivateBestChain(bestTip);
    }
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

bool BranchData::IsBlockInBestChain(const uint256& blockhash)
{
    if (!mapHeads.count(blockhash)){
        return false;
    }
    uint32_t height = mapHeads[blockhash].nHeight;
    if (height >= vecChainActive.size()){
        return false;
    }
    return vecChainActive[height] == blockhash;
}

int BranchData::GetBlockMinedHeight(const uint256& blockhash)
{
    if (!IsBlockInBestChain(blockhash)){
        return 0;
    }
    uint32_t height = mapHeads[blockhash].nHeight;
    return Height() - height;
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
            ActivateBestChain(newTipHash);
        }
        //else
    }
}

void BranchData::ActivateBestChain(const uint256 &bestTipHash)
{
    if (vecChainActive.back() == bestTipHash){
        return;
    }

    std::vector<uint256> forkChain;
    forkChain.push_back(bestTipHash);
    int64_t besttipeheight = mapHeads[bestTipHash].nHeight;// for debug easy
    while (besttipeheight < vecChainActive.size())
    {
        vecChainActive.pop_back();
    }
    uint256 forkHash = mapHeads[bestTipHash].header.hashPrevBlock;
    while (mapHeads[forkHash].nHeight >= vecChainActive.size())
    {
        assert(mapHeads[forkHash].nHeight > mapHeads[mapHeads[forkHash].header.hashPrevBlock].nHeight);
        forkChain.push_back(forkHash);
        forkHash = mapHeads[forkHash].header.hashPrevBlock;
    }

    while (vecChainActive[mapHeads[forkHash].nHeight] != forkHash)
    {
        forkChain.push_back(forkHash);
        forkHash = mapHeads[forkHash].header.hashPrevBlock;
        vecChainActive.pop_back();
    }
    vecChainActive.insert(vecChainActive.end(), forkChain.rbegin(), forkChain.rend());
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
    uint256 mReportKey = Hash(DB_MAP_REPORT_PROVE_DATA.begin(), DB_MAP_REPORT_PROVE_DATA.end());
    db.Write(mReportKey, mReortTxFlag);
}

void BranchDb::OnConnectBlock(const std::shared_ptr<const CellBlock>& pblock)
{
    std::set<uint256> modifyBranch;
    const CellBlock& block = *pblock;
    const std::vector<CellTransactionRef>& trans = block.vtx;
    const uint256 mainBlockHash = block.GetHash();
    for (size_t i = 0; i < trans.size(); ++i) {
        CellTransactionRef transaction = trans[i];
        if (transaction->IsSyncBranchInfo()) {
            AddBlockInfoTxData(transaction, mainBlockHash, i, modifyBranch);
        }
    }

    for (const uint256& branchHash : modifyBranch)
    {
        BranchData& bData = mapBranchsData[branchHash];
        bData.SnapshotBlockTip(mainBlockHash);
    }

    // after scan finish, save to db
    bool retdb = WriteModifyToDB(modifyBranch);
}

void BranchDb::OnDisconnectBlock(const std::shared_ptr<const CellBlock>& pblock)
{
    std::set<uint256> modifyBranch;
    const CellBlock& block = *pblock;
    const std::vector<CellTransactionRef>& trans = block.vtx;
    const uint256 mainBlockHash = block.GetHash();
    for (int i = trans.size() - 1; i >= 0; i--) {
        CellTransactionRef transaction = trans[i];
        if (transaction->IsSyncBranchInfo()) {
            DelBlockInfoTxData(transaction, mainBlockHash, i, modifyBranch);
        }
    }

    //recover branch best tip by snapshot
    for (const uint256& branchHash : modifyBranch)
    {
        BranchData& bData = mapBranchsData[branchHash];
        bData.RecoverTip(pblock->hashPrevBlock);// 恢复前一个块的
    }

    // after scan finish, save to db
    bool retdb = WriteModifyToDB(modifyBranch);
}

//interface for test easy
void BranchDb::AddBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch)
{
    BranchBlockData bBlockData;
    bBlockData.InitDataFromTx(*transaction);

    uint256 branchHash = transaction->pBranchBlockData->branchID;
    BranchData& bData = mapBranchsData[branchHash];
    bData.InitBranchGenesisBlockData(branchHash);

    const uint256& hashPrevBlock = bBlockData.header.hashPrevBlock;
    //tx from blockhash and vtx index
    bBlockData.mBlockHash = mainBlockHash;
    bBlockData.txIndex = iTxVtxIndex;
    bBlockData.nChainWork = (bData.mapHeads.count(hashPrevBlock) ? bData.mapHeads[hashPrevBlock].nChainWork : 0)
        + GetBlockProof(bBlockData.header.nBits);

    bData.BuildBestChain(bBlockData);

    modifyBranch.insert(branchHash);
}

//interface for test easy
void BranchDb::DelBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch)
{
    BranchBlockData bBlockData;
    bBlockData.InitDataFromTx(*transaction);

    uint256 bBlockHash = bBlockData.header.GetHash();
    uint256 branchHash = transaction->pBranchBlockData->branchID;
    BranchData& bData = mapBranchsData[branchHash];

    bData.RemoveBlock(bBlockHash);

    modifyBranch.insert(branchHash);
}

bool BranchDb::WriteModifyToDB(const std::set<uint256>& modifyBranch)
{
    CellDBBatch batch(db);
    for (const uint256& branchHash : modifyBranch)
    {
        BranchData& bData = mapBranchsData[branchHash];
        batch.Write(branchHash, bData);
    }
    bool retdb = db.WriteBatch(batch);
    return retdb;
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
}

BranchData BranchDb::GetBranchData(const uint256& branchHash)
{
    BranchData& branchdata = mapBranchsData[branchHash];
    branchdata.InitBranchGenesisBlockData(branchHash);
    return branchdata;
}

bool BranchDb::IsBlockInActiveChain(const uint256& branchHash, const uint256& blockHash)
{
    if (!HasBranchData(branchHash))
        return false;

    BranchData& branchdata = mapBranchsData[branchHash];
    return branchdata.IsBlockInBestChain(blockHash);
}

int BranchDb::GetBranchBlockMinedHeight(const uint256& branchHash, const uint256& blockHash)
{
    if (!HasBranchData(branchHash))
        return 0;

    BranchData& branchdata = mapBranchsData[branchHash];
    
    return 0;
}