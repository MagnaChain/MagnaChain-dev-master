// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "branchtxdb.h"
#include "rpc/branchchainrpc.h"
#include "transaction/txmempool.h"
#include "validation/validation.h"

namespace {
    class MineCoinEntry {
    public:
        char key;
        uint256 hash;// coin prevout tx hash
                     //uint32_t n; // n default is zero, so no need
        MineCoinEntry(const uint256& txid) : key(DB_MINE_COIN_LOCK), hash(txid) {}

        template <typename Stream>
        void Serialize(Stream& s) const
        {
            s << key;
            s << hash;
        }

        template <typename Stream>
        void Unserialize(Stream& s)
        {
            s >> key;
            s >> hash;
        }
    };
}

BranchChainTxRecordsDb* g_pBranchChainTxRecordsDb = nullptr;
BranchChainTxRecordsCache* g_pBranchTxRecordCache = nullptr;

void BranchChainTxRecordsCache::AddBranchChainTxRecord(const MCTransactionRef& tx, const uint256& blockhash, uint32_t txindex)
{
    if (!tx->IsPregnantTx() && !tx->IsBranchCreate())
        return;

    BranchChainTxEntry key(tx->GetHash(), DB_BRANCH_CHAIN_TX_DATA);
    BranchChainTxInfo& sendinfo = m_mapChainTxInfos[key];
    sendinfo.blockhash = blockhash;
    sendinfo.txindex = txindex;
    sendinfo.txnVersion = tx->nVersion;
    if (tx->IsBranchCreate()) {
        sendinfo.createchaininfo.txid = tx->GetHash();
        sendinfo.createchaininfo.branchSeedSpec6 = tx->branchSeedSpec6;
        sendinfo.createchaininfo.branchVSeeds = tx->branchVSeeds;
        sendinfo.createchaininfo.blockhash = blockhash;
    }
    sendinfo.flags = DbDataFlag::eADD;
}

void BranchChainTxRecordsCache::DelBranchChainTxRecord(const MCTransactionRef& tx)
{
    if (!tx->IsPregnantTx() && !tx->IsBranchCreate())
        return;

    BranchChainTxEntry key(tx->GetHash(), DB_BRANCH_CHAIN_TX_DATA); // may be delete one entry which not in m_mapChainTxInfos any more
    BranchChainTxInfo& sendinfo = m_mapChainTxInfos[key];
    if (tx->IsBranchCreate()) {
        sendinfo.createchaininfo.txid = tx->GetHash();
        sendinfo.createchaininfo.branchSeedSpec6 = tx->branchSeedSpec6;
        sendinfo.createchaininfo.branchVSeeds = tx->branchVSeeds;
    }
    sendinfo.flags = DbDataFlag::eDELETE;
}

void BranchChainTxRecordsCache::AddBranchChainRecvTxRecord(const MCTransactionRef& tx, const uint256& blockhash)
{
    if (tx->IsBranchChainTransStep2() == false)
        return;

    uint256 txid = mempool.GetOriTxHash(*tx, false);
    BranchChainTxEntry key(txid, DB_BRANCH_CHAIN_RECV_TX_DATA);
    BranchChainTxRecvInfo& data = m_mapRecvRecord[key];
    data.blockhash = blockhash;
    data.flags = DbDataFlag::eADD;
    //LogPrintf("branchtx cache add ori txid %s, %s\n", txid.GetHex(), tx->GetHash().GetHex());
}

void BranchChainTxRecordsCache::DelBranchChainRecvTxRecord(const MCTransactionRef& tx)
{
    if (tx->IsBranchChainTransStep2() == false)
        return;

    uint256 txid = mempool.GetOriTxHash(*tx, false);
    BranchChainTxEntry key(txid, DB_BRANCH_CHAIN_RECV_TX_DATA);
    BranchChainTxRecvInfo& data = m_mapRecvRecord[key];
    data.flags = DbDataFlag::eDELETE;
    //LogPrintf("branchtx cache del ori txid %s, %s\n", txid.GetHex(), tx->GetHash().GetHex());
}

void BranchChainTxRecordsCache::AddToCache(const MCTransactionRef& ptx, const uint256& blockhash, int blocktxindex)
{
    if (ptx->IsPregnantTx() || ptx->IsBranchCreate()) {
        AddBranchChainTxRecord(ptx, blockhash, blocktxindex);
    }
    if (ptx->IsBranchChainTransStep2()) {
        AddBranchChainRecvTxRecord(ptx, blockhash);
    }
    //if (ptx->IsLockMortgageMineCoin() || ptx->IsUnLockMortgageMineCoin()){
    //    UpdateLockMineCoin(ptx, true);
    //}
    if (ptx->IsBranchChainTransStep2() || ptx->IsRedeemMortgage())
    {
        MCTransactionRef pFromTx;
        MCDataStream cds(ptx->fromTx, SER_NETWORK, INIT_PROTO_VERSION);
        cds >> (pFromTx);
        CTxidMapping& kTxid = m_mapTxidMapping[pFromTx->GetHash()];
        kTxid.flags = DbDataFlag::eADD;
        kTxid.txid = ptx->GetHash();
    }
}

//call in DisconnectBlock
void BranchChainTxRecordsCache::RemoveFromCache(const MCTransactionRef& ptx)
{
    if (ptx->IsPregnantTx() || ptx->IsBranchCreate()) {
        DelBranchChainTxRecord(ptx);
    }
    if (ptx->IsBranchChainTransStep2()) {
        DelBranchChainRecvTxRecord(ptx);
    }
    //if (ptx->IsLockMortgageMineCoin() || ptx->IsUnLockMortgageMineCoin()) {
    //    UpdateLockMineCoin(ptx, false);
    //}
    if (ptx->IsBranchChainTransStep2() || ptx->IsRedeemMortgage())
    {
        MCTransactionRef pFromTx;
        MCDataStream cds(ptx->fromTx, SER_NETWORK, INIT_PROTO_VERSION);
        cds >> (pFromTx);
        CTxidMapping& kTxid = m_mapTxidMapping[pFromTx->GetHash()];
        kTxid.flags = DbDataFlag::eDELETE;
        //kTxid.txid = ptx->GetHash();
    }
}

bool BranchChainTxRecordsCache::HasInCache(const MCTransaction& tx)
{
    if (tx.IsBranchChainTransStep2()) {
        uint256 txid = mempool.GetOriTxHash(tx, false);
        BranchChainTxEntry key(txid, DB_BRANCH_CHAIN_RECV_TX_DATA);
        if (m_mapRecvRecord.count(key) && m_mapRecvRecord[key].flags == DbDataFlag::eADD) {// do it necessary to check the flags.
            return true;
        }
    }
    return false;
}

uint256 BranchChainTxRecordsCache::GetBlockTxid(const uint256& txid)
{
    if (m_mapTxidMapping.count(txid) && m_mapTxidMapping[txid].flags == DbDataFlag::eADD) {
        return m_mapTxidMapping[txid].txid;
    }
    return uint256();
}

//void BranchChainTxRecordsCache::RemoveFromBlock(const std::vector<MCTransactionRef>& vtx)
//{
//    for (int i=0; i<vtx.size(); i++)// use foreach shuang(cool?) but 
//    {
//        //erase not remove as RemoveFromCache
//        const MCTransactionRef& ptx = vtx[i];
//        if (ptx->IsBranchChainTransStep2()){
//            uint256 txid = mempool.GetOriTxHash(*ptx, false);
//            BranchChainTxEntry key(txid, DB_BRANCH_CHAIN_RECV_TX_DATA);
//            m_mapRecvRecord.erase(key);
//        }
//    }
//}

void BranchChainTxRecordsCache::RemoveFromMempool(const MCTransaction& tx)
{
    if (tx.IsBranchChainTransStep2()) {
        uint256 txid = mempool.GetOriTxHash(tx, false);
            BranchChainTxEntry key(txid, DB_BRANCH_CHAIN_RECV_TX_DATA);
            m_mapRecvRecord.erase(key);
        }
    if (tx.IsBranchChainTransStep2() || tx.IsRedeemMortgage()) {
        MCTransactionRef pFromTx;
        MCDataStream cds(tx.fromTx, SER_NETWORK, INIT_PROTO_VERSION);
        cds >> (pFromTx);
        m_mapTxidMapping.erase(pFromTx->GetHash());
    }
}

//void BranchChainTxRecordsCache::UpdateLockMineCoin(const MCTransactionRef& ptx, bool fBlockConnect)
//{
//    if (fBlockConnect) {
//        if (ptx->IsLockMortgageMineCoin()) { // 锁定
//            std::vector<CoinReportInfo>& vec = m_mapCoinBeReport[ptx->coinpreouthash];
//            for (CoinReportInfo& info : vec)
//            {
//                if (info.reporttxid == ptx->reporttxid){
//                    info.flags = DbDataFlag::eADD;
//                    return;
//                }
//            }
//            vec.push_back(CoinReportInfo(ptx->reporttxid, DbDataFlag::eADD));
//        }
//        if (ptx->IsUnLockMortgageMineCoin()) { // 解锁
//            std::vector<CoinReportInfo>& vec = m_mapCoinBeReport[ptx->coinpreouthash];
//            for (CoinReportInfo& info : vec)
//            {
//                if (info.reporttxid == ptx->reporttxid) {
//                    info.flags = DbDataFlag::eDELETE;
//                    return;
//                }
//            }
//            vec.push_back(CoinReportInfo(ptx->reporttxid, DbDataFlag::eDELETE));
//        }
//    }
//    else{// block disconnect
//        if (ptx->IsLockMortgageMineCoin()) {// 锁定回滚
//            std::vector<CoinReportInfo>& vec = m_mapCoinBeReport[ptx->coinpreouthash];
//            for (CoinReportInfo& info : vec)
//            {
//                if (info.reporttxid == ptx->reporttxid) {
//                    info.flags = DbDataFlag::eDELETE;
//                    return;
//                }
//            }
//            vec.push_back(CoinReportInfo(ptx->reporttxid, DbDataFlag::eDELETE));
//        }
//        if (ptx->IsUnLockMortgageMineCoin()) {// 解锁回滚
//            std::vector<CoinReportInfo>& vec = m_mapCoinBeReport[ptx->coinpreouthash];
//            for (CoinReportInfo& info : vec)
//            {
//                if (info.reporttxid == ptx->reporttxid) {
//                    info.flags = DbDataFlag::eADD;
//                    return;
//                }
//            }
//            vec.push_back(CoinReportInfo(ptx->reporttxid, DbDataFlag::eADD));
//        }
//    }
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BranchChainTxRecordsDb::BranchChainTxRecordsDb(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe)
    : m_db(path, nCacheSize, fMemory, fWipe, true)
{
    m_db.Read(DB_BRANCH_CHAIN_LIST, m_vCreatedBranchTxs);
}

BranchChainTxInfo BranchChainTxRecordsDb::GetBranchChainTxInfo(const uint256& txid)
{
    BranchChainTxEntry key(txid, DB_BRANCH_CHAIN_TX_DATA);
    BranchChainTxInfo sendinfo;
    if (!m_db.Read(key, sendinfo)) {
        sendinfo.blockhash.SetNull();
        return sendinfo;
    }
    return sendinfo;
}

//param tx
//param pBlock which block contain tx
bool BranchChainTxRecordsDb::IsTxRecvRepeat(const MCTransaction& tx, const MCBlock* pBlock /*=nullptr*/)
{
    if (tx.IsBranchChainTransStep2() == false)
        return false;

    uint256 txid = mempool.GetOriTxHash(tx, false);
    BranchChainTxEntry keyentry(txid, DB_BRANCH_CHAIN_RECV_TX_DATA);
    BranchChainTxRecvInfo recvInfo;
    if (!m_db.Read(keyentry, recvInfo))
        return false;

    if (pBlock && pBlock->GetHash() == recvInfo.blockhash) {//same block is not duplicate
        return false;
    }

    return true;
}

void BranchChainTxRecordsDb::Flush(BranchChainTxRecordsCache& cache)
{
    LogPrint(BCLog::COINDB, "flush branch chain tx data to db\n");
    MCDBBatch batch(m_db);
    size_t batch_size = (size_t)gArgs.GetArg("-dbbatchsize", nDefaultDbBatchSize);
    bool bCreatedChainTxChanged = false;
    for (auto mit = cache.m_mapChainTxInfos.begin(); mit != cache.m_mapChainTxInfos.end(); mit++) {
        const BranchChainTxEntry& keyentry = mit->first;
        const BranchChainTxInfo& txinfo = mit->second;
        if (txinfo.flags == DbDataFlag::eADD){
            batch.Write(keyentry, txinfo);
            //LogPrintf("branchtxdb add ori txid %s, key %c\n", keyentry.txhash.GetHex(), keyentry.key);
        }
        else if (txinfo.flags == DbDataFlag::eDELETE){
            batch.Erase(keyentry);
            //LogPrintf("branchtxdb add ori txid %s, key %c\n", keyentry.txhash.GetHex(), keyentry.key);
        }

        if (batch.SizeEstimate() > batch_size) {
            LogPrint(BCLog::COINDB, "BranchChainTxRecordsDb 0, Writing partial batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
            m_db.WriteBatch(batch);
            batch.Clear();
        }
        //update create branch chain vector
        if (txinfo.txnVersion == MCTransaction::CREATE_BRANCH_VERSION) {
            if (txinfo.flags == DbDataFlag::eADD) {
                // add before then (del and add) in same case
                if (std::find(m_vCreatedBranchTxs.begin(), m_vCreatedBranchTxs.end(), txinfo.createchaininfo) == m_vCreatedBranchTxs.end()) {
                    m_vCreatedBranchTxs.push_back(txinfo.createchaininfo);
                    bCreatedChainTxChanged = true;
                }
            }
            else if (txinfo.flags == DbDataFlag::eDELETE) {
                for (CREATE_BRANCH_TX_CONTAINER::iterator it = m_vCreatedBranchTxs.begin(); it != m_vCreatedBranchTxs.end(); it++) {
                    if (it->txid == txinfo.createchaininfo.txid) {
                        m_vCreatedBranchTxs.erase(it);
                        bCreatedChainTxChanged = true;
                        break;
                    }
                }
            }
        }
    }
    cache.m_mapChainTxInfos.clear();

    LogPrint(BCLog::COINDB, "BranchChainTxRecordsDb 0, Writing final batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
    m_db.WriteBatch(batch);
    batch.Clear();

    for (auto mit = cache.m_mapRecvRecord.begin(); mit != cache.m_mapRecvRecord.end(); mit++) {
        const BranchChainTxEntry& keyentry = mit->first;
        const BranchChainTxRecvInfo& txinfo = mit->second;
        if (txinfo.flags == DbDataFlag::eADD){
            batch.Write(keyentry, txinfo);
            //LogPrintf("branchtxdb add ori txid %s, key %c\n", keyentry.txhash.GetHex(), keyentry.key);
        }
        else if (txinfo.flags == DbDataFlag::eDELETE){
            batch.Erase(keyentry);
            //LogPrintf("branchtxdb add ori txid %s, key %c\n", keyentry.txhash.GetHex(), keyentry.key);
        }

        if (batch.SizeEstimate() > batch_size) {
            LogPrint(BCLog::COINDB, "BranchChainTxRecordsDb 1, Writing partial batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
            m_db.WriteBatch(batch);
            batch.Clear();
        }
    }

    if (bCreatedChainTxChanged) {
        batch.Write(DB_BRANCH_CHAIN_LIST, m_vCreatedBranchTxs);
    }

    LogPrint(BCLog::COINDB, "BranchChainTxRecordsDb 1, Writing final batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
    cache.m_mapRecvRecord.clear();

    //for (auto mit = cache.m_mapCoinBeReport.begin(); mit != cache.m_mapCoinBeReport.end(); mit++)
    //{
    //    const uint256& coinprehash = mit->first;
    //    const std::vector<CoinReportInfo>& vec = mit->second;
    //    
    //    MineCoinEntry key(coinprehash);
    //    //merge with db
    //    std::vector<CoinReportInfo> vecDb;
    //    if (m_db.Read(key, vecDb)){
    //        for (auto nv : vec){// cache data
    //            bool found = false;
    //            for (auto it = vecDb.begin(); it != vecDb.end(); it++) {
    //                if ((*it).reporttxid == nv.reporttxid){
    //                    if (nv.flags == DbDataFlag::eDELETE){
    //                        vecDb.erase(it); 
    //                    }
    //                    else if (nv.flags == DbDataFlag::eADD) {
    //                        // duplicate add
    //                    }
    //                    found = true;
    //                    break;
    //                }
    //            }
    //            if (!found){
    //                if (nv.flags == DbDataFlag::eADD)
    //                    vecDb.push_back(nv);
    //                //else if (nv.flags == DbDataFlag::eDELETE)
    //                // delete not exist ??
    //            }
    //        }
    //    }
    //    // write back to db
    //    if (vecDb.size() > 0)
    //        m_db.Write(key, vecDb);
    //    else
    //        m_db.Erase(key);
    //}
    //cache.m_mapCoinBeReport.clear();

    for (auto mit = cache.m_mapTxidMapping.begin(); mit != cache.m_mapTxidMapping.end(); mit++)
    {
        const uint256& origtxid = mit->first;
        const CTxidMapping& kTxid = mit->second;
        BranchChainTxEntry key(origtxid, DB_DYNAMIC_TXID_MAPING);
        if (kTxid.flags == DbDataFlag::eADD) { 
            batch.Write(key, kTxid);
        }
        else if (kTxid.flags == DbDataFlag::eDELETE) {
            batch.Erase(key);
        }
    }
    m_db.WriteBatch(batch);// final batch
    batch.Clear();
    cache.m_mapTxidMapping.clear();
    LogPrint(BCLog::COINDB, "finsh flush branch tx data.\n");
}

bool BranchChainTxRecordsDb::IsBranchCreated(const uint256 &branchid) const
{
    for (auto v: m_vCreatedBranchTxs)
    {
        if (v.txid == branchid)
        {
            return true;
        }
    }
    return false;
}

//bool BranchChainTxRecordsDb::IsMineCoinLock(const uint256& coinhash) const
//{
//    MineCoinEntry key(coinhash);
//    //merge with db
//    std::vector<CoinReportInfo> vecDb;
//    if (m_db.Read(key, vecDb))
//    {
//        if (vecDb.size() > 0){
//            return true;
//        }
//    }
//    return false;
//}

uint256 BranchChainTxRecordsDb::GetBlockTxid(const uint256& txid)
{
    BranchChainTxEntry key(txid, DB_DYNAMIC_TXID_MAPING);
    CTxidMapping value;
    if (m_db.Read(key, value)) {
        return value.txid;
    }
    return uint256();
}
