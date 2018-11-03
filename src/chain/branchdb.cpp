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
#include "chain/branchchain.h"

static const std::string DB_MAP_REPORT_PROVE_DATA = "db_mao_report_prove_data";

BranchDb* pBranchDb = nullptr;

BranchCache branchDataMemCache;

BranchBlockData::BranchBlockData():nHeight(0), txIndex(0), deadstatus(eLive)
{

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

void BranchData::AddNewBlockData(BranchBlockData& blockdata)
{
    const uint256 newTipHash = blockdata.header.GetHash();
    const uint256& hashPrevBlock = blockdata.header.hashPrevBlock;
    //add new block head data
    mapHeads[newTipHash] = blockdata;
    //update parent's son hashs
    if (mapHeads.count(hashPrevBlock)) {
        mapHeads[hashPrevBlock].vecSonHashs.push_back(newTipHash);

        //继承hashPrevBlock的死亡属性
        if (mapHeads[hashPrevBlock].deadstatus){
            blockdata.deadstatus = blockdata.deadstatus | BranchBlockData::eDeadInherit;
        }
    }
    
    if (!blockdata.deadstatus)
    {
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

bool BranchCache::HasInCache(const CellTransaction& tx)
{
    if (tx.IsSyncBranchInfo())
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

        BranchData& bData = this->mapBranchCache[branchHash];
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

        BranchData& bData = mapBranchCache[branchHash];
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

bool BranchBlockData::IsDead()
{
    bool isAllProve = true;
    for (const std::pair<uint256, uint16_t>& p: mapReportStatus){
        if (p.second == RP_FLAG_REPORTED){
            isAllProve = false;
            break;
        }
    }
    return !isAllProve;
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
    std::set<uint256> brokenChainBranch;
    const CellBlock& block = *pblock;
    const uint256 mainBlockHash = block.GetHash();
    for (size_t i = 0; i < block.vtx.size(); ++i) {
        CellTransactionRef tx = block.vtx[i];
        if (tx->IsSyncBranchInfo()) {
            AddBlockInfoTxData(tx, mainBlockHash, i, modifyBranch);
        }
        if (tx->IsReport()) {
            uint256 reportFlagHash = GetReportTxHashKey(*tx);
            mReortTxFlag[reportFlagHash] = RP_FLAG_REPORTED;

            //-----------
            const uint256& rpBranchId = tx->pReportData->reportedBranchId;
            const uint256& rpBlockId = tx->pReportData->reportedBlockHash;
            if (mapBranchsData.count(rpBranchId)){// ok, we must assert(mapBranchsData.count(rpBranchId));
                BranchData& branchdata = mapBranchsData[rpBranchId];
                if (branchdata.mapHeads.count(rpBlockId)){
                    branchdata.mapHeads[rpBlockId].mapReportStatus[reportFlagHash] = RP_FLAG_REPORTED;
                    //update dead status, dead transmit
                    bool deadchanged = false;
                    branchdata.UpdateDeadStatus(rpBlockId, deadchanged);
                    if (deadchanged){
                        brokenChainBranch.insert(rpBranchId);
                    }
                }
                else{
                    LogPrint(BCLog::BRANCH, "Fatal error: report tx block data not exist!\n");
                }
            }
            else{
                LogPrint(BCLog::BRANCH, "Fatal error: report tx branch data not exist!\n");
            }
        }
        if (tx->IsProve())
        {
            uint256 proveFlagHash = GetProveTxHashKey(*tx);

            //assert first
            //if(this->mReortTxFlag.count(proveFlagHash) == 0 || this->mReortTxFlag[proveFlagHash] != RP_FLAG_REPORTED)
            //{
            //    AbortNode;
            //}

            this->mReortTxFlag[proveFlagHash] = RP_FLAG_PROVED;

            //-----------
            const uint256& rpBranchId = tx->pProveData->branchId;
            const uint256& rpBlockId = tx->pProveData->blockHash;
            if (mapBranchsData.count(rpBranchId)) {// ok, we must assert(mapBranchsData.count(rpBranchId));
                BranchData& branchdata = mapBranchsData[rpBranchId];
                if (branchdata.mapHeads.count(rpBlockId)) {
                    branchdata.mapHeads[rpBlockId].mapReportStatus[proveFlagHash] = RP_FLAG_PROVED;
                    //update dead status, reborn transmit
                    bool deadchanged = false;
                    branchdata.UpdateRebornStatus(rpBlockId, deadchanged);
                    if (deadchanged){
                        brokenChainBranch.insert(rpBranchId);
                    }
                }
                else {
                    LogPrint(BCLog::BRANCH, "Fatal error: prove tx block data not exist!\n");
                }
            }
            else {
                LogPrint(BCLog::BRANCH, "Fatal error: prove tx branch data not exist!\n");
            }
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

void BranchDb::OnDisconnectBlock(const std::shared_ptr<const CellBlock>& pblock)
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
            uint256 reportFlagHash = GetReportTxHashKey(*tx);
            mReortTxFlag.erase(reportFlagHash);
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
                    if (deadchanged){
                        brokenChainBranch.insert(rpBranchId);
                        modifyBranch.insert(rpBranchId);
                    }
                }
                else {
                    LogPrint(BCLog::BRANCH, "Fatal error: disconnect report tx block data not exist!\n");
                }
            }
            else {
                LogPrint(BCLog::BRANCH, "Fatal error: disconnect report tx branch data not exist!\n");
            }
        }
        if (tx->IsProve())
        {
            uint256 proveFlagHash = GetProveTxHashKey(*tx);
            this->mReortTxFlag[proveFlagHash] = RP_FLAG_REPORTED; // revert to previous value
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
                    if (deadchanged){
                        brokenChainBranch.insert(rpBranchId);
                        modifyBranch.insert(rpBranchId);
                    }
                }
                else {
                    LogPrint(BCLog::BRANCH, "Fatal error: disconnect prove tx block data not exist!\n");
                }
            }
            else {
                LogPrint(BCLog::BRANCH, "Fatal error: disconnect prove tx branch data not exist!\n");
            }
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
    
    bData.AddNewBlockData(bBlockData);

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
    return branchdata.GetBlockMinedHeight(blockHash);
}
