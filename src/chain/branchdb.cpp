// Copyright (c) 2016-2018 The MagnaChain developers
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
#include "chain/branchchain.h"

static const std::string DB_MAP_REPORT_PROVE_DATA = "db_map_report_prove_data";

BranchDb* g_pBranchDb = nullptr;

BranchCache* g_pBranchDataMemCache = nullptr;

BranchBlockData::BranchBlockData():nHeight(0), txIndex(0), deadstatus(eLive)
{

}

bool BranchBlockData::IsDead()
{
    bool isAllProve = true;
    for (const std::pair<uint256, uint16_t>& p : mapReportStatus) {
        if (p.second == RP_FLAG_REPORTED) {
            isAllProve = false;
            break;
        }
    }
    return !isAllProve;
}

void BranchBlockData::InitDataFromTx(const CellTransaction& tx)
{
    //set block header
    tx.pBranchBlockData->GetBlockHeader(this->header);

    this->nHeight = tx.pBranchBlockData->blockHeight;

    CellDataStream cds(tx.pBranchBlockData->vchStakeTxData, SER_NETWORK, INIT_PROTO_VERSION);
    cds >> (this->pStakeTx);

    this->txHash = tx.GetHash();
}

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
    //LogPrintf("bBlockData.deadstatus = %d InitBranchGenesisBlockData\n", blockdata.deadstatus);
    blockdata.deadstatus = BranchBlockData::eLive;

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

int32_t BranchData::Height(void)
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

BranchBlockData* BranchData::GetBranchBlockData(const uint256& blockHash)
{
    auto it = mapHeads.find(blockHash);
    if (it == mapHeads.end())
        return nullptr;
    return &(it->second);
}

void BranchData::AddNewBlockData(BranchBlockData& blockdata)
{
    const uint256 newTipHash = blockdata.header.GetHash();
    const uint256& hashPrevBlock = blockdata.header.hashPrevBlock;

    //update parent's son hashs
    if (mapHeads.count(hashPrevBlock)) {
        mapHeads[hashPrevBlock].vecSonHashs.push_back(newTipHash);

        //继承hashPrevBlock的死亡属性
        if (mapHeads[hashPrevBlock].deadstatus)
            blockdata.deadstatus = blockdata.deadstatus | BranchBlockData::eDeadInherit;
    }

    //add new block head data
    mapHeads[newTipHash] = blockdata;
    if (!blockdata.deadstatus)
    {
        if (vecChainActive.back() == blockdata.header.hashPrevBlock)
            vecChainActive.push_back(newTipHash);
        else
        {
            const BranchBlockData& tipBlock = mapHeads[vecChainActive.back()];
            if (blockdata.nChainWork > tipBlock.nChainWork)
                ActivateBestChain(newTipHash);
        }
    }
}

void BranchData::ActivateBestChain(const uint256 &bestTipHash)
{
    if (vecChainActive.back() == bestTipHash)
        return;

    std::vector<uint256> forkChain;
    forkChain.push_back(bestTipHash);
    int64_t besttipeheight = mapHeads[bestTipHash].nHeight;// for debug easy
    while (besttipeheight < vecChainActive.size())
        vecChainActive.pop_back();

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

const BranchBlockData* BranchData::GetAncestor(BranchBlockData* pBlock, int height)
{
    if (pBlock == nullptr)
        return nullptr;

    int delta = pBlock->nHeight - height;
    if (delta <= 0)
        return pBlock;

    BranchBlockData* cur = pBlock;
    for (int i = 0; i < delta; ++i) {
        auto it = mapHeads.find(cur->header.hashPrevBlock);
        if (it != mapHeads.end())
            cur = &(it->second);
        else
            return nullptr;
    }

    return cur;
}

void BranchData::UpdateDeadStatus(const uint256& blockId, bool &fStatusChange){
    if (!mapHeads.count(blockId)){
        return;//assert
    }
    if (mapHeads[blockId].deadstatus & BranchBlockData::eDeadSelf){
        return;//alread dead
    }
    mapHeads[blockId].deadstatus = mapHeads[blockId].deadstatus | BranchBlockData::eDeadSelf;
    fStatusChange = true;
    // dead transmit
    for (const uint256 sonhash : mapHeads[blockId].vecSonHashs){
        DeadTransmit(sonhash);
    }
}

void BranchData::DeadTransmit(const uint256& blockId)
{
    mapHeads[blockId].deadstatus = mapHeads[blockId].deadstatus | BranchBlockData::eDeadInherit;
    // 
    if (mapHeads[blockId].deadstatus & BranchBlockData::eDeadSelf){
        return;// 如何子block中有被举报死掉的块，它下面的应该不用递归了，没bug的话它的后代应该是dead的。
    }
    for (const uint256 sonhash : mapHeads[blockId].vecSonHashs) {
        DeadTransmit(sonhash);
    }
}

void BranchData::UpdateRebornStatus(const uint256& blockId, bool &fStatusChange)
{
    if (!mapHeads[blockId].IsDead())//all prove, is time to reborn
    {
        //移除dead self的状态
        mapHeads[blockId].deadstatus = mapHeads[blockId].deadstatus & (~BranchBlockData::eDeadSelf);
        fStatusChange = true;
    }
    if (!mapHeads[blockId].deadstatus){
        for (const uint256 sonhash : mapHeads[blockId].vecSonHashs) {
            RebornTransmit(sonhash);
        }
    }
}

void BranchData::RebornTransmit(const uint256& blockId)
{
    //移除dead inherit状态
    mapHeads[blockId].deadstatus = mapHeads[blockId].deadstatus & (~BranchBlockData::eDeadInherit);
    if (mapHeads[blockId].deadstatus) {
        return;
    }
    for (const uint256 sonhash : mapHeads[blockId].vecSonHashs) {
        RebornTransmit(sonhash);
    }
}

uint256 BranchData::FindBestTipBlock()
{
    if (vecChainActive.size() <= 0)
        return uint256();

    const uint256& genesisBlockId = vecChainActive[0];
    uint256 mostworkblock = genesisBlockId;
    FindBestBlock(genesisBlockId, mostworkblock);
    return mostworkblock;
}

void BranchData::FindBestBlock(const uint256& blockhash, uint256& mostworkblock)
{
    BranchBlockData& blockdata = mapHeads[blockhash];
    if (blockdata.deadstatus)
    {
        return;
    }
    if (blockdata.nChainWork > mapHeads[mostworkblock].nChainWork)
    {
        mostworkblock = blockhash;
    }
    for (const uint256& sonblockid : blockdata.vecSonHashs)
    {
        FindBestBlock(sonblockid, mostworkblock);
    }
}

bool BrandchDataView::HasBranchData(const uint256& branchHash) const
{
    return false;
}
uint256 BrandchDataView::GetBranchTipHash(const uint256& branchid)
{
    return uint256();
}
uint32_t BrandchDataView::GetBranchHeight(const uint256& branchid)
{
    return 0;
}
BranchData BrandchDataView::GetBranchData(const uint256& branchHash)
{
    return BranchData();
}
bool BrandchDataView::IsBlockInActiveChain(const uint256& branchHash, const uint256& blockHash)
{
    return false;
}
int BrandchDataView::GetBranchBlockMinedHeight(const uint256& branchHash, const uint256& blockHash)
{
    return -1;
}
uint16_t BrandchDataView::GetTxReportState(const uint256& rpBranchId, const uint256& rpBlockId, const uint256& flagHash)
{
    return RP_INVALID;
}

uint256 BranchDataProcesser::GetBranchTipHash(const uint256& branchid)
{
    if (!HasBranchData(branchid))
    {
        return uint256();
    }
    BranchData& branchdata = mapBranchsData[branchid];
    return branchdata.TipHash();
}

uint32_t BranchDataProcesser::GetBranchHeight(const uint256& branchid)
{
    if (!HasBranchData(branchid))
    {
        return 0;
    }
    BranchData& branchdata = mapBranchsData[branchid];
    return branchdata.Height();
}

bool BranchDataProcesser::HasBranchData(const uint256& branchHash) const
{
    return mapBranchsData.count(branchHash) > 0;
}

BranchData BranchDataProcesser::GetBranchData(const uint256& branchHash)
{
    BranchData& branchdata = mapBranchsData[branchHash];
    branchdata.InitBranchGenesisBlockData(branchHash);
    return branchdata;
}

bool BranchDataProcesser::IsBlockInActiveChain(const uint256& branchHash, const uint256& blockHash)
{
    if (!HasBranchData(branchHash))
        return false;

    BranchData& branchdata = mapBranchsData[branchHash];
    return branchdata.IsBlockInBestChain(blockHash);
}

int BranchDataProcesser::GetBranchBlockMinedHeight(const uint256& branchHash, const uint256& blockHash)
{
    if (!HasBranchData(branchHash))
        return 0;

    BranchData& branchdata = mapBranchsData[branchHash];
    return branchdata.GetBlockMinedHeight(blockHash);
}
uint16_t BranchDataProcesser::GetTxReportState(const uint256& rpBranchId, const uint256& rpBlockId, const uint256& flagHash)
{
    if (!HasBranchData(rpBranchId))
        return RP_INVALID;

    BranchData& branchdata = mapBranchsData[rpBranchId];
    if (branchdata.mapHeads.count(rpBlockId) == 0)
        return RP_INVALID;

    if (branchdata.mapHeads[rpBlockId].mapReportStatus.count(flagHash) == 0)
        return RP_INVALID;
    else
        return branchdata.mapHeads[rpBlockId].mapReportStatus[flagHash];
}

void BranchDataProcesser::Flush(const std::shared_ptr<const CellBlock>& pblock, bool fConnect)
{
    if (!Params().IsMainChain())
        return;

    if (fConnect)
        OnConnectBlock(pblock);
    else
        OnDisconnectBlock(pblock);
}

void BranchDataProcesser::OnConnectBlock(const std::shared_ptr<const CellBlock>& pblock)
{
    std::set<uint256> modifyBranch;// 修改过的branch data的branchid
    std::set<uint256> brokenChainBranch;
    const CellBlock& block = *pblock;
    const uint256 mainBlockHash = block.GetHash();
    for (size_t i = 0; i < block.vtx.size(); ++i) {
        CellTransactionRef tx = block.vtx[i];
        if (tx->IsSyncBranchInfo()) {
            AddBlockInfoTxData(tx, mainBlockHash, i, modifyBranch);
        }
        if (tx->IsReport()) {
            AddReportTxData(tx, brokenChainBranch);
        }
        if (tx->IsProve()) {
            AddProveTxData(tx, brokenChainBranch);
        }
    }

    // re find best tip block
    for (const uint256& branchHash : brokenChainBranch)
    {
        BranchData& bData = mapBranchsData[branchHash];
        uint256 besttipblock = bData.FindBestTipBlock();
        if (bData.vecChainActive.back() != besttipblock)
        {
            bData.ActivateBestChain(besttipblock);
            modifyBranch.insert(branchHash);
        }
    }

    //支链besttipblock的快照，用于在disconnected block时快速回滚
    for (const uint256& branchHash : modifyBranch)
    {
        BranchData& bData = mapBranchsData[branchHash];
        bData.SnapshotBlockTip(mainBlockHash);
    }

    // after scan finish, save to db
    bool retdb = WriteModifyToDB(modifyBranch);
}

void BranchDataProcesser::OnDisconnectBlock(const std::shared_ptr<const CellBlock>& pblock)
{
    std::set<uint256> modifyBranch;
    std::set<uint256> brokenChainBranch;// no use
    const CellBlock& block = *pblock;
    const uint256 mainBlockHash = block.GetHash();
    for (int i = block.vtx.size() - 1; i >= 0; i--) {
        CellTransactionRef tx = block.vtx[i];
        if (tx->IsSyncBranchInfo()) {
            DelBlockInfoTxData(tx, mainBlockHash, i, modifyBranch);
        }
        if (tx->IsReport()) {
            DelReportTxData(tx, brokenChainBranch, modifyBranch);
        }
        if (tx->IsProve()) {
            DelProveTxData(tx, brokenChainBranch, modifyBranch);
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
void BranchDataProcesser::AddBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch)
{
    BranchBlockData bBlockData;
    //LogPrintf("bBlockData.deadstatus = %d 111", bBlockData.deadstatus);
    bBlockData.InitDataFromTx(*transaction);
    //LogPrintf("bBlockData.deadstatus = %d 222", bBlockData.deadstatus);
    //bBlockData.deadstatus = BranchBlockData::eLive;

    uint256 branchHash = transaction->pBranchBlockData->branchID;
    BranchData& bData = mapBranchsData[branchHash];
    bData.InitBranchGenesisBlockData(branchHash);

    const uint256& hashPrevBlock = bBlockData.header.hashPrevBlock;
    //tx from blockhash and vtx index
    bBlockData.mBlockHash = mainBlockHash;
    bBlockData.txIndex = iTxVtxIndex;
    bBlockData.nChainWork = (bData.mapHeads.count(hashPrevBlock) ? bData.mapHeads[hashPrevBlock].nChainWork : 0)
        + GetBlockProof(bBlockData.header.nBits);

    bData.AddNewBlockData(bBlockData);

    modifyBranch.insert(branchHash);
}

//interface for test easy
void BranchDataProcesser::DelBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch)
{
    BranchBlockData bBlockData;
    bBlockData.InitDataFromTx(*transaction);

    uint256 bBlockHash = bBlockData.header.GetHash();
    uint256 branchHash = transaction->pBranchBlockData->branchID;
    BranchData& bData = mapBranchsData[branchHash];

    bData.RemoveBlock(bBlockHash);

    modifyBranch.insert(branchHash);
}


bool BranchDataProcesser::AddReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch)
{
    uint256 reportFlagHash = GetReportTxHashKey(*tx);
    //-----------
    const uint256& rpBranchId = tx->pReportData->reportedBranchId;
    const uint256& rpBlockId = tx->pReportData->reportedBlockHash;
    if (mapBranchsData.count(rpBranchId)) {// ok, we must assert(mapBranchsData.count(rpBranchId));
        BranchData& branchdata = mapBranchsData[rpBranchId];
        if (branchdata.mapHeads.count(rpBlockId)) {
            branchdata.mapHeads[rpBlockId].mapReportStatus[reportFlagHash] = RP_FLAG_REPORTED;
            //update dead status, dead transmit
            bool deadchanged = false;
            branchdata.UpdateDeadStatus(rpBlockId, deadchanged);
            if (deadchanged) {
                brokenChainBranch.insert(rpBranchId);
            }
        }
        else
            return error("Fatal error: report tx block data not exist! txid %s\n", tx->GetHash().ToString());
    }
    else
        return error("Fatal error: report tx branch data not exist! txid %s \n", tx->GetHash().ToString());
    return true;
}

bool BranchDataProcesser::AddProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch)
{
    uint256 proveFlagHash = GetProveTxHashKey(*tx);
    //-----------
    const uint256& rpBranchId = tx->pProveData->branchId;
    const uint256& rpBlockId = tx->pProveData->blockHash;
    if (mapBranchsData.count(rpBranchId)) {// ok, we must assert(mapBranchsData.count(rpBranchId));
        BranchData& branchdata = mapBranchsData[rpBranchId];
        if (branchdata.mapHeads.count(rpBlockId)) {
            //assert branchdata.mapHeads[rpBlockId].mapReportStatus[proveFlagHash] == RP_FLAG_REPORTED
            branchdata.mapHeads[rpBlockId].mapReportStatus[proveFlagHash] = RP_FLAG_PROVED;
            //update dead status, reborn transmit
            bool deadchanged = false;
            branchdata.UpdateRebornStatus(rpBlockId, deadchanged);
            if (deadchanged) {
                brokenChainBranch.insert(rpBranchId);
            }
        }
        else
            return error("Fatal error: prove tx block data not exist! %s\n", tx->GetHash().ToString());
    }
    else
        return error("Fatal error: prove tx branch data not exist! %s\n", tx->GetHash().ToString());
    return true;
}

bool BranchDataProcesser::DelReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch)
{
    uint256 reportFlagHash = GetReportTxHashKey(*tx);
    //-----------
    const uint256& rpBranchId = tx->pReportData->reportedBranchId;
    const uint256& rpBlockId = tx->pReportData->reportedBlockHash;
    if (mapBranchsData.count(rpBranchId)) {// ok, we must assert(mapBranchsData.count(rpBranchId));
        BranchData& branchdata = mapBranchsData[rpBranchId];
        if (branchdata.mapHeads.count(rpBlockId)) {
            branchdata.mapHeads[rpBlockId].mapReportStatus.erase(reportFlagHash);
            //update dead status, 检查是否可以移除死亡状态
            bool deadchanged = false;
            branchdata.UpdateRebornStatus(rpBlockId, deadchanged);
            if (deadchanged) {
                brokenChainBranch.insert(rpBranchId);
                modifyBranch.insert(rpBranchId);
            }
        }
        else {
            return error("Fatal error: disconnect report tx block data not exist! txid %s\n", tx->GetHash().ToString());
        }
    }
    else {
        return error("Fatal error: disconnect report tx branch data not exist! txid %s\n", tx->GetHash().ToString());
    }
    return true;
}

bool BranchDataProcesser::DelProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch)
{
    uint256 proveFlagHash = GetProveTxHashKey(*tx);
    //-----------
    const uint256& rpBranchId = tx->pProveData->branchId;
    const uint256& rpBlockId = tx->pProveData->blockHash;
    if (mapBranchsData.count(rpBranchId)) {// ok, we must assert(mapBranchsData.count(rpBranchId));
        BranchData& branchdata = mapBranchsData[rpBranchId];
        if (branchdata.mapHeads.count(rpBlockId)) {
            branchdata.mapHeads[rpBlockId].mapReportStatus[proveFlagHash] = RP_FLAG_REPORTED;
            //update dead status, 检查是否需要reborn to dead.
            bool deadchanged = false;
            branchdata.UpdateDeadStatus(rpBlockId, deadchanged);
            if (deadchanged) {
                brokenChainBranch.insert(rpBranchId);
                modifyBranch.insert(rpBranchId);
            }
        }
        else {
            return error("Fatal error: disconnect prove tx block data not exist! txid %s\n", tx->GetHash().ToString());
        }
    }
    else {
        return error("Fatal error: disconnect prove tx branch data not exist! txid %s\n", tx->GetHash().ToString());
    }
    return true;
}

bool BranchDataProcesser::WriteModifyToDB(const std::set<uint256>& modifyBranch)
{
    (void*)(&modifyBranch);
    return true;
}

bool BranchCache::HasInCache(const CellTransaction& tx)
{
    if (tx.IsSyncBranchInfo())
    {
        BranchBlockData blockData;
        blockData.InitDataFromTx(tx);

        uint256 blockHash = blockData.header.GetHash();
        uint256 branchHash = tx.pBranchBlockData->branchID;

        if (mapBranchsData.count(branchHash)){
            if (mapBranchsData[branchHash].mapHeads.count(blockHash)){
                if (mapBranchsData[branchHash].mapHeads[blockHash].flags == BranchBlockData::eADD){
                    return true;
                }
            }
        }
    }
    return false;
}

void BranchCache::AddToCache(const CellTransaction& tx)
{
    if (tx.IsSyncBranchInfo()) {
        BranchBlockData blockData;
        blockData.flags = BranchBlockData::eADD;
        blockData.InitDataFromTx(tx);

        blockData.txHash = tx.GetHash();

        uint256 blockHash = blockData.header.GetHash();
        uint256 branchHash = tx.pBranchBlockData->branchID;

        bool isNewBranch = this->mapBranchsData.count(branchHash) == 0;
        BranchData& bData = this->mapBranchsData[branchHash];
        if (isNewBranch)
            bData.flags = BranchData::eADD;
        bData.mapHeads[blockHash] = blockData;
    }
    if (tx.IsReport()){
        uint256 reportFlagHash = GetReportTxHashKey(tx);
        this->mReortTxFlagCache[reportFlagHash] = RP_FLAG_REPORTED;
    }
    if (tx.IsProve())
    {
        uint256 proveFlagHash = GetProveTxHashKey(tx);

        //check before
        //if (this->mReortTxFlagCache.count(proveFlagHash) == 0 || this->mReortTxFlagCache[proveFlagHash] != RP_FLAG_REPORTED)
        //{
        //    AbortNode;
        //}

        this->mReortTxFlagCache[proveFlagHash] = RP_FLAG_PROVED;
    }
}

uint256 BranchCache::GetBranchTipHash(const uint256& branchid)
{
    if (mapBranchsData.count(branchid)){
        return mapBranchsData[branchid].TipHash();
    }
    if (readonly_db){
        return readonly_db->GetBranchTipHash(branchid);
    }
    return uint256();
}

uint32_t BranchCache::GetBranchHeight(const uint256& branchid)
{
    if (mapBranchsData.count(branchid)){
        return mapBranchsData[branchid].Height();
    }
    if (readonly_db){
        return readonly_db->GetBranchHeight(branchid);
    }
    return 0;
}

bool BranchCache::HasBranchData(const uint256& branchHash) const
{
    return mapBranchsData.count(branchHash) 
        || (readonly_db && readonly_db->HasBranchData(branchHash));
}

BranchData BranchCache::GetBranchData(const uint256& branchHash)
{
    // try get local data
    if (mapBranchsData.count(branchHash))
    {
        return mapBranchsData[branchHash];
    }
    // try get db data
    if (readonly_db && readonly_db->HasBranchData(branchHash))
    {
        return readonly_db->GetBranchData(branchHash);
    }

    BranchData& branchdata = mapBranchsData[branchHash];
    branchdata.InitBranchGenesisBlockData(branchHash);
    return branchdata;
}

bool BranchCache::IsBlockInActiveChain(const uint256& branchHash, const uint256& blockHash)
{
    if (mapBranchsData.count(branchHash))
    {
        return mapBranchsData[branchHash].IsBlockInBestChain(blockHash);
    }
    if (readonly_db)
    {
        return readonly_db->IsBlockInActiveChain(branchHash, blockHash);
    }
    return false;
}
int BranchCache::GetBranchBlockMinedHeight(const uint256& branchHash, const uint256& blockHash)
{
    if (mapBranchsData.count(branchHash))
    {
        return mapBranchsData[branchHash].GetBlockMinedHeight(blockHash);
    }
    if (readonly_db)
    {
        return readonly_db->GetBranchBlockMinedHeight(branchHash, blockHash);
    }
    return 0;
}

uint16_t BranchCache::GetTxReportState(const uint256& rpBranchId, const uint256& rpBlockId, const uint256& flagHash)
{
    if (mapBranchsData.count(rpBranchId))
    {
        BranchData& branchdata = mapBranchsData[rpBranchId];
        if (branchdata.mapHeads.count(rpBlockId))
        {
            if (branchdata.mapHeads[rpBlockId].mapReportStatus.count(flagHash))
                return branchdata.mapHeads[rpBlockId].mapReportStatus[flagHash];
        }
        return RP_INVALID;
    }
    if (readonly_db)
    {
        return readonly_db->GetTxReportState(rpBranchId, rpBlockId, flagHash);
    }
    return RP_INVALID;
}

// Use in follow situation. 1.After finish build block
// keep cache 
void BranchCache::RemoveFromCache(const CellTransaction& tx)
{
    if (tx.IsSyncBranchInfo())
    {
        BranchBlockData blockData;
        blockData.flags = BranchBlockData::eDELETE;
        blockData.InitDataFromTx(tx);

        blockData.txHash = tx.GetHash();

        uint256 blockHash = blockData.header.GetHash();
        uint256 branchHash = tx.pBranchBlockData->branchID;

        bool bgotdata = FetchDataFromSource(branchHash);

        BranchData& bData = mapBranchsData[branchHash];
        if (bData.mapHeads.count(blockHash))//update map data
            bData.mapHeads[blockHash].flags = BranchBlockData::eDELETE;
        else                                // set map data
            bData.mapHeads[blockHash] = blockData;
    }
    if (tx.IsReport()) {
        uint256 reportFlagHash = GetReportTxHashKey(tx);
        mReortTxFlagCache.erase(reportFlagHash);
    }
    //TODO: test case
    if (tx.IsProve()){
        uint256 proveFlagHash = GetProveTxHashKey(tx);
        mReortTxFlagCache.erase(proveFlagHash);
    }
}

void BranchCache::RemoveFromBlock(const std::vector<CellTransactionRef>& vtx)
{
    for (const auto& tx : vtx)
    {
        RemoveFromCache(*tx);
    }
}

std::vector<uint256> BranchCache::GetAncestorsBlocksHash(const CellTransaction& tx)
{
    std::vector<uint256> vecRet;
    if (!tx.IsSyncBranchInfo()){
        return vecRet;
    }

    BranchBlockData blockData;
    blockData.InitDataFromTx(tx);

    //uint256 blockHash = blockData.header.GetHash();
    const uint256& branchHash = tx.pBranchBlockData->branchID;

    bool bgotdata = FetchDataFromSource(branchHash);
    
    if (mapBranchsData.count(branchHash))
    {
        std::shared_ptr<BranchData> pReadOnlyDbData = nullptr;
        if (readonly_db && readonly_db->HasBranchData(branchHash))
        {
            pReadOnlyDbData = std::make_shared<BranchData>(readonly_db->GetBranchData(branchHash));
        }

        BranchData& branchdata = mapBranchsData[branchHash];
        uint256& preblockhash = blockData.header.hashPrevBlock;
        while (branchdata.mapHeads.count(preblockhash)){
            if (pReadOnlyDbData && pReadOnlyDbData->mapHeads.count(preblockhash))
            {
                break;// Already in db
            }
            BranchBlockData& preblockdata = branchdata.mapHeads[preblockhash];
            vecRet.push_back(preblockdata.txHash);
            preblockhash = preblockdata.header.hashPrevBlock;
        }
    }

    return vecRet;
}

const BranchBlockData* BranchCache::GetBranchBlockData(const uint256 &branchhash, const uint256 &blockhash)
{
    bool bgotdata = FetchDataFromSource(branchhash);
    if (mapBranchsData.count(branchhash)){
        if (mapBranchsData[branchhash].mapHeads.count(blockhash)){
            if (mapBranchsData[branchhash].mapHeads[blockhash].flags == BranchBlockData::eADD)
            {
                return &mapBranchsData[branchhash].mapHeads[blockhash];
            }
        }
    }
    return nullptr;
}

bool BranchCache::FetchDataFromSource(const uint256& branchId)
{
    if (mapBranchsData.count(branchId))
        return true;

    if ((readonly_db && readonly_db->HasBranchData(branchId)))//load from source
    {
        mapBranchsData[branchId] = readonly_db->GetBranchData(branchId);//copy to local
        mapBranchsData[branchId].flags = BranchData::eOriginal;
        return true;
    }
    else
        return false;
}

void BranchCache::AddBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch)
{
    const uint256& branchHash = transaction->pBranchBlockData->branchID;
    bool bgotdata = FetchDataFromSource(branchHash);

    BranchDataProcesser::AddBlockInfoTxData(transaction, mainBlockHash, iTxVtxIndex, modifyBranch);
    if (!bgotdata)
    {
        mapBranchsData[branchHash].flags = BranchData::eADD;
    }
}
void BranchCache::DelBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch)
{
    const uint256& branchHash = transaction->pBranchBlockData->branchID;
    bool bgotdata = FetchDataFromSource(branchHash);

    BranchDataProcesser::DelBlockInfoTxData(transaction, mainBlockHash, iTxVtxIndex, modifyBranch);

}
bool BranchCache::AddReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch)
{
    const uint256& branchId = tx->pReportData->reportedBranchId;
    bool bgotdata = FetchDataFromSource(branchId);

    bool ret = BranchDataProcesser::AddReportTxData(tx, brokenChainBranch);
    return ret;
}
bool BranchCache::AddProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch)
{
    const uint256& branchId = tx->pProveData->branchId;
    bool bgotdata = FetchDataFromSource(branchId);

    bool ret = BranchDataProcesser::AddProveTxData(tx, brokenChainBranch);
    return ret;
}
bool BranchCache::DelReportTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch)
{
    const uint256& branchId = tx->pReportData->reportedBranchId;
    bool bgotdata = FetchDataFromSource(branchId);

    bool ret = BranchDataProcesser::DelReportTxData(tx, brokenChainBranch, modifyBranch);
    return ret;
}
bool BranchCache::DelProveTxData(CellTransactionRef &tx, std::set<uint256> &brokenChainBranch, std::set<uint256> &modifyBranch)
{
    const uint256& branchId = tx->pProveData->branchId;
    bool bgotdata = FetchDataFromSource(branchId);

    bool ret = BranchDataProcesser::DelProveTxData(tx, brokenChainBranch, modifyBranch);
    return ret;
}

BranchDb::BranchDb(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe)
    : db(path, nCacheSize, fMemory, fWipe, true)
{
}

void BranchDb::LoadData()
{
    //LogPrintf("===== 1-branch db load data: %s \n", Params().GetBranchId()); 
    if (!Params().IsMainChain()) {
        //LogPrintf("===== 2-branch db load data: %s \n", Params().GetBranchId());  
        return;
    }
    CellDBIterator* it = db.NewIterator();
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        uint256 keyHash;

        if (it->GetKey(keyHash))
        {
            BranchData data;
            if (it->GetValue(data))
            {
                mapBranchsData[keyHash] = data;
                //LogPrintf("===== 3-branch db load data: %s \n", branchHash.GetHex());
            }
        }
    }
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

