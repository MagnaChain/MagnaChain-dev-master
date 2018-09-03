// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"

#include "coding/base58.h"
#include "chain/chainparams.h"
#include "coding/hash.h"
#include "init.h"
#include "misc/pow.h"
#include "misc/random.h"
#include "ui/ui_interface.h"
#include "coding/uint256.h"
#include "utils/util.h"
#include "validation/validation.h"
#include "primitives/block.h"
#include "txmempool.h"
#include "script/standard.h"

#include <sstream>
#include <iostream>
#include <stdint.h>
#include <boost/thread.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

static const char DB_COIN = 'C';
static const char DB_COINS = 'c';
static const char DB_BLOCK_FILES = 'f';
static const char DB_TXINDEX = 't';
static const char DB_BLOCK_INDEX = 'b';

static const char DB_BEST_BLOCK = 'B';
static const char DB_HEAD_BLOCKS = 'H';
static const char DB_FLAG = 'F';
static const char DB_REINDEX_FLAG = 'R';
static const char DB_LAST_BLOCK = 'l';

static const char DB_COINLIST = 'A';

namespace
{
struct CoinEntry {
    CellOutPoint* outpoint;
    char key;
    CoinEntry(const CellOutPoint* ptr) : outpoint(const_cast<CellOutPoint*>(ptr)), key(DB_COIN) {}

    template <typename Stream>
    void Serialize(Stream& s) const
    {
        s << key;
        s << outpoint->hash;
        s << VARINT(outpoint->n);
    }

    template <typename Stream>
    void Unserialize(Stream& s)
    {
        s >> key;
        s >> outpoint->hash;
        s >> VARINT(outpoint->n);
    }
};
}

CellCoinsViewDB::CellCoinsViewDB(size_t nCacheSize, bool fMemory, bool fWipe) : db(GetDataDir() / "chainstate", nCacheSize, fMemory, fWipe, true)
{
}

bool CellCoinsViewDB::GetCoin(const CellOutPoint& outpoint, Coin& coin) const
{
    return db.Read(CoinEntry(&outpoint), coin);
}

bool CellCoinsViewDB::HaveCoin(const CellOutPoint& outpoint) const
{
    return db.Exists(CoinEntry(&outpoint));
}

uint256 CellCoinsViewDB::GetBestBlock() const
{
    uint256 hashBestChain;
    if (!db.Read(DB_BEST_BLOCK, hashBestChain))
        return uint256();
    return hashBestChain;
}

std::vector<uint256> CellCoinsViewDB::GetHeadBlocks() const
{
    std::vector<uint256> vhashHeadBlocks;
    if (!db.Read(DB_HEAD_BLOCKS, vhashHeadBlocks)) {
        return std::vector<uint256>();
    }
    return vhashHeadBlocks;
}

bool CellCoinsViewDB::BatchWrite(CellCoinsMap& mapCoins, const uint256& hashBlock)
{
    CellDBBatch batch(db);
    size_t count = 0;
    size_t changed = 0;
    size_t batch_size = (size_t)gArgs.GetArg("-dbbatchsize", nDefaultDbBatchSize);
    int crash_simulate = gArgs.GetArg("-dbcrashratio", 0);
    assert(!hashBlock.IsNull());

    uint256 old_tip = GetBestBlock();
    if (old_tip.IsNull()) {
        // We may be in the middle of replaying.
        std::vector<uint256> old_heads = GetHeadBlocks();
        if (old_heads.size() == 2) {
            assert(old_heads[0] == hashBlock);
            old_tip = old_heads[1];
        }
    }

    // In the first batch, mark the database as being in the middle of a
    // transition from old_tip to hashBlock.
    // A vector is used for future extensibility, as we may want to support
    // interrupting after partial writes from multiple independent reorgs.
    batch.Erase(DB_BEST_BLOCK);
    batch.Write(DB_HEAD_BLOCKS, std::vector<uint256>{hashBlock, old_tip});

    for (auto it = mapCoins.begin(); it != mapCoins.end();) {
        if (it->second.flags & CellCoinsCacheEntry::DIRTY) {
            CoinEntry entry(&it->first);
            if (it->second.coin.IsSpent())
                batch.Erase(entry);
            else
                batch.Write(entry, it->second.coin);
            changed++;
        }
        count++;
        CellCoinsMap::iterator itOld = it++;
        mapCoins.erase(itOld);
        if (batch.SizeEstimate() > batch_size) {
            LogPrint(BCLog::COINDB, "Writing partial batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
            db.WriteBatch(batch);
            batch.Clear();
            if (crash_simulate) {
                static FastRandomContext rng;
                if (rng.randrange(crash_simulate) == 0) {
                    LogPrintf("Simulating a crash. Goodbye.\n");
                    _Exit(0);
                }
            }
        }
    }

    // write coin list
    pcoinListDb->Flush();

    // In the last batch, mark the database as consistent with hashBlock again.
    batch.Erase(DB_HEAD_BLOCKS);
    batch.Write(DB_BEST_BLOCK, hashBlock);

    LogPrint(BCLog::COINDB, "Writing final batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
    bool ret = db.WriteBatch(batch);
    LogPrint(BCLog::COINDB, "Committed %u changed transaction outputs (out of %u) to coin database...\n", (unsigned int)changed, (unsigned int)count);
    return ret;
}

size_t CellCoinsViewDB::EstimateSize() const
{
    return db.EstimateSize(DB_COIN, (char)(DB_COIN + 1));
}

CellBlockTreeDB::CellBlockTreeDB(size_t nCacheSize, bool fMemory, bool fWipe) : CellDBWrapper(GetDataDir() / "blocks" / "index", nCacheSize, fMemory, fWipe)
{
}

bool CellBlockTreeDB::ReadBlockFileInfo(int nFile, CellBlockFileInfo& info)
{
    return Read(std::make_pair(DB_BLOCK_FILES, nFile), info);
}

bool CellBlockTreeDB::WriteReindexing(bool fReindexing)
{
    if (fReindexing)
        return Write(DB_REINDEX_FLAG, '1');
    else
        return Erase(DB_REINDEX_FLAG);
}

bool CellBlockTreeDB::ReadReindexing(bool& fReindexing)
{
    fReindexing = Exists(DB_REINDEX_FLAG);
    return true;
}

bool CellBlockTreeDB::ReadLastBlockFile(int& nFile)
{
    return Read(DB_LAST_BLOCK, nFile);
}

CellCoinsViewCursor* CellCoinsViewDB::Cursor() const
{
    CellCoinsViewDBCursor* i = new CellCoinsViewDBCursor(const_cast<CellDBWrapper&>(db).NewIterator(), GetBestBlock());
    /* It seems that there are no "const iterators" for LevelDB.  Since we
       only need read operations on it, use a const-cast to get around
       that restriction.  */
    i->pcursor->Seek(DB_COIN);
    // Cache key of first record
    if (i->pcursor->Valid()) {
        CoinEntry entry(&i->keyTmp.second);
        i->pcursor->GetKey(entry);
        i->keyTmp.first = entry.key;
    } else {
        i->keyTmp.first = 0; // Make sure Valid() and GetKey() return false
    }
    return i;
}

bool CellCoinsViewDBCursor::GetKey(CellOutPoint& key) const
{
    // Return cached key
    if (keyTmp.first == DB_COIN) {
        key = keyTmp.second;
        return true;
    }
    return false;
}

bool CellCoinsViewDBCursor::GetValue(Coin& coin) const
{
    return pcursor->GetValue(coin);
}

unsigned int CellCoinsViewDBCursor::GetValueSize() const
{
    return pcursor->GetValueSize();
}

bool CellCoinsViewDBCursor::Valid() const
{
    return keyTmp.first == DB_COIN;
}

void CellCoinsViewDBCursor::Next()
{
    pcursor->Next();
    CoinEntry entry(&keyTmp.second);
    if (!pcursor->Valid() || !pcursor->GetKey(entry)) {
        keyTmp.first = 0; // Invalidate cached key after last record so that Valid() and GetKey() return false
    } else {
        keyTmp.first = entry.key;
    }
}

bool CellBlockTreeDB::WriteBatchSync(const std::vector<std::pair<int, const CellBlockFileInfo*>>& fileInfo, int nLastFile, const std::vector<const CellBlockIndex*>& blockinfo)
{
    CellDBBatch batch(*this);
    for (std::vector<std::pair<int, const CellBlockFileInfo*>>::const_iterator it = fileInfo.begin(); it != fileInfo.end(); it++) {
        batch.Write(std::make_pair(DB_BLOCK_FILES, it->first), *it->second);
    }
    batch.Write(DB_LAST_BLOCK, nLastFile);
    for (std::vector<const CellBlockIndex*>::const_iterator it = blockinfo.begin(); it != blockinfo.end(); it++) {
        batch.Write(std::make_pair(DB_BLOCK_INDEX, (*it)->GetBlockHash()), CellDiskBlockIndex(*it));
    }
    return WriteBatch(batch, true);
}

bool CellBlockTreeDB::ReadTxIndex(const uint256& txid, CellDiskTxPos& pos)
{
    return Read(std::make_pair(DB_TXINDEX, txid), pos);
}

bool CellBlockTreeDB::WriteTxIndex(const std::vector<std::pair<uint256, CellDiskTxPos>>& vect)
{
    CellDBBatch batch(*this);
    for (std::vector<std::pair<uint256, CellDiskTxPos>>::const_iterator it = vect.begin(); it != vect.end(); it++)
        batch.Write(std::make_pair(DB_TXINDEX, it->first), it->second);
    return WriteBatch(batch);
}

bool CellBlockTreeDB::WriteFlag(const std::string& name, bool fValue)
{
    return Write(std::make_pair(DB_FLAG, name), fValue ? '1' : '0');
}

bool CellBlockTreeDB::ReadFlag(const std::string& name, bool& fValue)
{
    char ch;
    if (!Read(std::make_pair(DB_FLAG, name), ch))
        return false;
    fValue = ch == '1';
    return true;
}

bool CellBlockTreeDB::LoadBlockIndexGuts(const Consensus::Params& consensusParams, std::function<CellBlockIndex*(const uint256&)> insertBlockIndex)
{
    std::unique_ptr<CellDBIterator> pcursor(NewIterator());

    pcursor->Seek(std::make_pair(DB_BLOCK_INDEX, uint256()));

    // Load mapBlockIndex
    while (pcursor->Valid()) {
        boost::this_thread::interruption_point();
        std::pair<char, uint256> key;
        if (pcursor->GetKey(key) && key.first == DB_BLOCK_INDEX) {
            CellDiskBlockIndex diskindex;
            if (pcursor->GetValue(diskindex)) {
                // Construct block index object
                CellBlockIndex* pindexNew = insertBlockIndex(diskindex.GetBlockHash());
                pindexNew->pprev = insertBlockIndex(diskindex.hashPrev);
                pindexNew->nHeight = diskindex.nHeight;
                pindexNew->nFile = diskindex.nFile;
                pindexNew->nDataPos = diskindex.nDataPos;
                pindexNew->nUndoPos = diskindex.nUndoPos;
                pindexNew->nVersion = diskindex.nVersion;
                pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
                pindexNew->nTime = diskindex.nTime;
                pindexNew->nBits = diskindex.nBits;
                pindexNew->nNonce = diskindex.nNonce;
                pindexNew->nStatus = diskindex.nStatus;
                pindexNew->nTx = diskindex.nTx;
                pindexNew->prevoutStake = diskindex.prevoutStake;
                pindexNew->vchBlockSig = diskindex.vchBlockSig;
                if (!CheckProofOfWork(pindexNew->GetBlockHash(), pindexNew->nBits, consensusParams))
                    return error("%s: CheckProofOfWork failed: %s", __func__, pindexNew->ToString());

                pcursor->Next();
            } else {
                return error("%s: failed to read value", __func__);
            }
        } else {
            break;
        }
    }

    return true;
}

namespace
{
//! Legacy class to deserialize pre-pertxout database entries without reindex.
class CellCoins
{
public:
    //! whether transaction is a coinbase
    bool fCoinBase;

    //! unspent transaction outputs; spent outputs are .IsNull(); spent outputs at the end of the array are dropped
    std::vector<CellTxOut> vout;

    //! at which height this transaction was included in the active block chain
    int nHeight;

    //! empty constructor
    CellCoins() : fCoinBase(false), vout(0), nHeight(0) {}

    template <typename Stream>
    void Unserialize(Stream& s)
    {
        unsigned int nCode = 0;
        // version
        int nVersionDummy;
        ::Unserialize(s, VARINT(nVersionDummy));
        // header code
        ::Unserialize(s, VARINT(nCode));
        fCoinBase = nCode & 1;
        std::vector<bool> vAvail(2, false);
        vAvail[0] = (nCode & 2) != 0;
        vAvail[1] = (nCode & 4) != 0;
        unsigned int nMaskCode = (nCode / 8) + ((nCode & 6) != 0 ? 0 : 1);
        // spentness bitmask
        while (nMaskCode > 0) {
            unsigned char chAvail = 0;
            ::Unserialize(s, chAvail);
            for (unsigned int p = 0; p < 8; p++) {
                bool f = (chAvail & (1 << p)) != 0;
                vAvail.push_back(f);
            }
            if (chAvail != 0)
                nMaskCode--;
        }
        // txouts themself
        vout.assign(vAvail.size(), CellTxOut());
        for (unsigned int i = 0; i < vAvail.size(); i++) {
            if (vAvail[i])
                ::Unserialize(s, REF(CellTxOutCompressor(vout[i])));
        }
        // coinbase height
        ::Unserialize(s, VARINT(nHeight));
    }
};
}

/** Upgrade the database from older formats.
 *
 * Currently implemented: from the per-tx utxo model (0.8..0.14.x) to per-txout.
 */
bool CellCoinsViewDB::Upgrade()
{
    std::unique_ptr<CellDBIterator> pcursor(db.NewIterator());
    pcursor->Seek(std::make_pair(DB_COINS, uint256()));
    if (!pcursor->Valid()) {
        return true;
    }

    int64_t count = 0;
    LogPrintf("Upgrading utxo-set database...\n");
    LogPrintf("[0%%]...");
    size_t batch_size = 1 << 24;
    CellDBBatch batch(db);
    uiInterface.SetProgressBreakAction(StartShutdown);
    int reportDone = 0;
    std::pair<unsigned char, uint256> key;
    std::pair<unsigned char, uint256> prev_key = {DB_COINS, uint256()};
    while (pcursor->Valid()) {
        boost::this_thread::interruption_point();
        if (ShutdownRequested()) {
            break;
        }
        if (pcursor->GetKey(key) && key.first == DB_COINS) {
            if (count++ % 256 == 0) {
                uint32_t high = 0x100 * *key.second.begin() + *(key.second.begin() + 1);
                int percentageDone = (int)(high * 100.0 / 65536.0 + 0.5);
                uiInterface.ShowProgress(_("Upgrading UTXO database") + "\n" + _("(press q to shutdown and continue later)") + "\n", percentageDone);
                if (reportDone < percentageDone / 10) {
                    // report max. every 10% step
                    LogPrintf("[%d%%]...", percentageDone);
                    reportDone = percentageDone / 10;
                }
            }
            CellCoins old_coins;
            if (!pcursor->GetValue(old_coins)) {
                return error("%s: cannot parse CellCoins record", __func__);
            }
            CellOutPoint outpoint(key.second, 0);
            for (size_t i = 0; i < old_coins.vout.size(); ++i) {
                if (!old_coins.vout[i].IsNull() && !old_coins.vout[i].scriptPubKey.IsUnspendable()) {
                    Coin newcoin(std::move(old_coins.vout[i]), old_coins.nHeight, old_coins.fCoinBase);
                    outpoint.n = i;
                    CoinEntry entry(&outpoint);
                    batch.Write(entry, newcoin);
                }
            }
            batch.Erase(key);
            if (batch.SizeEstimate() > batch_size) {
                db.WriteBatch(batch);
                batch.Clear();
                db.CompactRange(prev_key, key);
                prev_key = key;
            }
            pcursor->Next();
        } else {
            break;
        }
    }
    db.WriteBatch(batch);
    db.CompactRange({DB_COINS, uint256()}, key);
    uiInterface.SetProgressBreakAction(std::function<void(void)>());
    LogPrintf("[%s].\n", ShutdownRequested() ? "CANCELLED" : "DONE");
    return !ShutdownRequested();
}

struct CoinListEntry {
    uint160* addr;
    char key;
    CoinListEntry(const uint160* ptr) : addr(const_cast<uint160*>(ptr)), key(DB_COINLIST) {}

    template <typename Stream>
    void Serialize(Stream& s) const
    {
        s << key;
        s << *addr;
    }

    template <typename Stream>
    void Unserialize(Stream& s)
    {
        s >> key;
        s >> *addr;
    }
};


// coin list db
static void CoinListGetParent(const CellOutPoint& outpoint, const Coin& coin, CoinList& kList)
{
    CellTxDestination kChildDest;
    if (!ExtractDestination(coin.out.scriptPubKey, kChildDest))
        return;

    CellLinkAddress kChildAddr(kChildDest);
    if (!kChildAddr.IsValid() || kChildAddr.IsScript() || coin.IsCoinBase())
        return;

    CellTransactionRef kTx;
    uint256 hashBlock;
    if (!GetTransactionWithCoin(outpoint, coin, kTx, Params().GetConsensus(), hashBlock)) {
        assert(false);
        return;
    }
    CellTxIn kTxIn = kTx->vin[0];
    if (kTxIn.prevout.hash.IsNull())
        return;
    CellTransactionRef kTrans;
    if (!GetTransactionWithOutpoint(kTxIn.prevout, kTrans, Params().GetConsensus(), hashBlock)) {
        assert(false);
        return;
    }

    CellTxDestination kParentDest;
    ExtractDestination(kTrans->vout[kTxIn.prevout.n].scriptPubKey, kParentDest);
    CellLinkAddress kParentAddr(kParentDest);
    if (!kParentAddr.IsValid() || kParentAddr.IsScript())
        return;
    //std::string strChildAddr = kChildAddr.ToString();
    //std::string strParent = kParentAddr.ToString();

    //LogPrintf("%s: Set parent:%s - %s \n", __func__, strChildAddr, strParent);
    const CellKeyID& kParentKey = boost::get<CellKeyID>(kParentDest);
    //kList.parent = (const uint160&)kParentKey;
}

static inline bool GetCoinDest(const CellOutPoint& outpoint, const Coin& coin, CellTxDestination& kDest)
{
    CellScript pScript = coin.out.scriptPubKey;
    CellTransactionRef kTx;

    if (coin.IsSpent()) {
        Coin dbCoin;
        pcoinsdbview->GetCoin(outpoint, dbCoin);
        if (dbCoin.IsSpent()) {
            uint256 hashBlock;
            if (!GetTransaction(outpoint.hash, kTx, Params().GetConsensus(), hashBlock, true)) {
                return false;
            }
            pScript = kTx->vout[outpoint.n].scriptPubKey;
        } else {
            pScript = dbCoin.out.scriptPubKey;
        }
    }

    if (!ExtractDestination(pScript, kDest)) {
        opcodetype opcode;
        std::vector<unsigned char> vch;
        CellScript::const_iterator pc1 = coin.out.scriptPubKey.begin();
        coin.out.scriptPubKey.GetOp(pc1, opcode, vch);

        if (opcode == OP_PUB_CONTRACT || opcode == OP_TRANS_CONTRACT) {
            vch.clear();
            vch.assign(pc1 + 1, coin.out.scriptPubKey.end());
            kDest = CellKeyID(uint160(vch));
            CellLinkAddress addrTest(kDest);
            std::string strTest = addrTest.ToString();
            LogPrint(BCLog::COINDB, "COIN_LIST, get contract addr : %s\n", strTest);
        } else
            return false;
    }

    CellLinkAddress kAddr(kDest);
    if (!kAddr.IsValid() || kAddr.IsScript())
        return false;
    return true;
}

void CoinListDB::ImportCoins(CellCoinsMap& mapCoins)
{
    CellCoinListMap& map = cache;
    for (CellCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end(); ++it) {
        if (it->second.flags & CellCoinsCacheEntry::DIRTY) {
            const Coin& coin = it->second.coin;
            const CellOutPoint& outpoint = it->first;

            CellTxDestination kDest;
            if (!GetCoinDest(outpoint, coin, kDest))
                continue;

            CellKeyID& kKey = boost::get<CellKeyID&>(kDest);

            CellCoinListMap::iterator mit = cache.find(kKey);
            CoinListPtr pList = nullptr;
            if (mit == cache.end()) {
                pList.reset(new CoinList());
                plistDB->Read(CoinListEntry(&kKey), *pList);
                cache[kKey] = pList;
            } else {
                pList = mit->second;
            }

            if (coin.IsSpent()) {
                for (std::vector<CellOutPoint>::iterator vit = pList->coins.begin(); vit != pList->coins.end(); ++vit) {
                    const CellOutPoint& to = *vit;
                    if (to.hash == outpoint.hash && to.n == outpoint.n) {
                        pList->coins.erase(vit);
                        break;
                    }
                }
            } else {
                bool bGot = false;
                // safe check
                for (std::vector<CellOutPoint>::iterator vit = pList->coins.begin(); vit != pList->coins.end(); ++vit) {
                    const CellOutPoint& to = *vit;
                    if (to.hash == outpoint.hash && to.n == outpoint.n) {
                        bGot = true;
                        LogPrint(BCLog::COINDB, "COIN_LIST, Readd trans : %s %d \n", to.hash.ToString(), to.n);
                        assert(false);
                        break;
                    }
                }
                if (!bGot)
                    pList->coins.push_back(outpoint);
                //if (!pList->parentInited)
                //    CoinListGetParent(outpoint, coin, *pList);
            }
        }
    }
}

void CoinListDB::Flush(void)
{
    CellDBBatch batch(*plistDB);

    size_t iTotalCoin = 0;
    size_t batch_size = (size_t)gArgs.GetArg("-dbbatchsize", nDefaultDbBatchSize);

    for (CellCoinListMap::iterator it = cache.begin(); it != cache.end(); ++it) {
        const uint160& kKey = it->first;
        const CoinList& kList = *it->second;
        iTotalCoin += kList.coins.size();

        batch.Write(CoinListEntry(&kKey), kList);

        if (batch.SizeEstimate() > batch_size) {
            LogPrint(BCLog::COINDB, "COIN_LIST, Writing partial batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
            plistDB->WriteBatch(batch);
            batch.Clear();
        }
    }

    LogPrint(BCLog::COINDB, "COIN_LIST, Writing final batch of %.2f MiB\n", batch.SizeEstimate() * (1.0 / 1048576.0));
    bool ret = plistDB->WriteBatch(batch);
    LogPrint(BCLog::COINDB, "COIN_LIST, Writing final batch, Result: %d TotalCoin:%d \n", ret, iTotalCoin);

    // clear all cache if writed to db
    cache.clear();
}

CoinListPtr CoinListDB::GetList(const uint160& kAddr) const
{
    // not cache list read from db, cause it's not modified
    CellCoinListMap::const_iterator mit = cache.find(kAddr);
    if (mit == cache.end()) {
        CoinListPtr pList(new CoinList());
        plistDB->Read(CoinListEntry(&kAddr), *pList);
        return pList;
    }
    return mit->second;
}
