// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_TXDB_H
#define CELLLINK_TXDB_H

#include "transaction/coins.h"
#include "io/dbwrapper.h"
#include "chain/chain.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

class CellBlock;
class CellBlockIndex;
class CellCoinsViewDBCursor;
class uint256;
class UniValue;

//! No need to periodic flush if at least this much space still available.
static constexpr int MAX_BLOCK_COINSDB_USAGE = 10;
//! -dbcache default (MiB)
static const int64_t nDefaultDbCache = 450;
//! -dbbatchsize default (bytes)
static const int64_t nDefaultDbBatchSize = 16 << 20;
//! max. -dbcache (MiB)
static const int64_t nMaxDbCache = sizeof(void*) > 4 ? 16384 : 1024;
//! min. -dbcache (MiB)
static const int64_t nMinDbCache = 4;
//! Max memory allocated to block tree DB specific cache, if no -txindex (MiB)
static const int64_t nMaxBlockDBCache = 2;
//! Max memory allocated to block tree DB specific cache, if -txindex (MiB)
// Unlike for the UTXO database, for the txindex scenario the leveldb cache make
// a meaningful difference: https://github.com/bitcoin/bitcoin/pull/8273#issuecomment-229601991
static const int64_t nMaxBlockDBAndTxIndexCache = 1024;
//! Max memory allocated to coin DB specific cache (MiB)
static const int64_t nMaxCoinsDBCache = 8;
//！Max number of storage blocks for smart contracts.
static const int64_t nMaxBlockContractDB = 100;

struct CellDiskTxPos : public CellDiskBlockPos
{
    unsigned int nTxOffset; // after header

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(*(CellDiskBlockPos*)this);
        READWRITE(VARINT(nTxOffset));
    }

    CellDiskTxPos(const CellDiskBlockPos &blockIn, unsigned int nTxOffsetIn) : CellDiskBlockPos(blockIn.nFile, blockIn.nPos), nTxOffset(nTxOffsetIn) {
    }

    CellDiskTxPos() {
        SetNull();
    }

    void SetNull() {
        CellDiskBlockPos::SetNull();
        nTxOffset = 0;
    }
};

/** CellCoinsView backed by the coin database (chainstate/) */
class CellCoinsViewDB : public CellCoinsView
{
protected:
    CellDBWrapper db;
public:
    CellCoinsViewDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

    bool GetCoin(const CellOutPoint &outpoint, Coin &coin) const override;
    bool HaveCoin(const CellOutPoint &outpoint) const override;
    uint256 GetBestBlock() const override;
    std::vector<uint256> GetHeadBlocks() const override;
    bool BatchWrite(CellCoinsMap &mapCoins, const uint256 &hashBlock) override;
    CellCoinsViewCursor *Cursor() const override;

    //! Attempt to update from an older database format. Returns whether an error occurred.
    bool Upgrade();
    size_t EstimateSize() const override;
	CellDBWrapper* GetDb() { return &db; }
};

/** Specialization of CellCoinsViewCursor to iterate over a CellCoinsViewDB */
class CellCoinsViewDBCursor: public CellCoinsViewCursor
{
public:
    ~CellCoinsViewDBCursor() {}

    bool GetKey(CellOutPoint &key) const override;
    bool GetValue(Coin &coin) const override;
    unsigned int GetValueSize() const override;

    bool Valid() const override;
    void Next() override;

private:
    CellCoinsViewDBCursor(CellDBIterator* pcursorIn, const uint256 &hashBlockIn):
        CellCoinsViewCursor(hashBlockIn), pcursor(pcursorIn) {}
    std::unique_ptr<CellDBIterator> pcursor;
    std::pair<char, CellOutPoint> keyTmp;

    friend class CellCoinsViewDB;
};

/** Access to the block database (blocks/index/) */
class CellBlockTreeDB : public CellDBWrapper
{
public:
    CellBlockTreeDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
private:
    CellBlockTreeDB(const CellBlockTreeDB&);
    void operator=(const CellBlockTreeDB&);
public:
    bool WriteBatchSync(const std::vector<std::pair<int, const CellBlockFileInfo*> >& fileInfo, int nLastFile, const std::vector<const CellBlockIndex*>& blockinfo);
    bool ReadBlockFileInfo(int nFile, CellBlockFileInfo &fileinfo);
    bool ReadLastBlockFile(int &nFile);
    bool WriteReindexing(bool fReindex);
    bool ReadReindexing(bool &fReindex);
    bool ReadTxIndex(const uint256 &txid, CellDiskTxPos &pos);
    bool WriteTxIndex(const std::vector<std::pair<uint256, CellDiskTxPos> > &list);
    bool WriteFlag(const std::string &name, bool fValue);
    bool ReadFlag(const std::string &name, bool &fValue);
    bool LoadBlockIndexGuts(const Consensus::Params& consensusParams, std::function<CellBlockIndex*(const uint256&)> insertBlockIndex);
};

class CoinList
{
public:
	std::vector<CellOutPoint> coins;

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action) {
		READWRITE(coins);
	}
};

class U160Hasher
{
public:
	inline size_t operator()(const uint160& id) const {
		size_t r = id.GetUint64(0) ^ id.GetUint64(1);
		return r;
	}
};

typedef std::unordered_map<uint160, std::shared_ptr<CoinList>, U160Hasher> CellCoinListMap;
typedef std::shared_ptr<CoinList> CoinListPtr;

// coin list db
class CoinListDB
{
public:
    CoinListDB(CellDBWrapper* pDB) : plistDB(pDB)
    {
        assert(pDB);
	}

protected:
	CellDBWrapper* plistDB;
	CellCoinListMap cache;

public:
	void Flush(void);

	void ImportCoins(CellCoinsMap& cacheCoins);

	CoinListPtr GetList(const uint160& kAddr) const;
};

#endif // CELLLINK_TXDB_H
