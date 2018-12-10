// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_COINS_H
#define MAGNACHAIN_COINS_H

#include "primitives/transaction.h"
#include "transaction/compressor.h"
#include "misc/core_memusage.h"
#include "coding/hash.h"
#include "misc/memusage.h"
#include "io/serialize.h"
#include "coding/uint256.h"
#include "thread/sync.h"

#include <assert.h>
#include <stdint.h>

#include <unordered_map>

/**
 * A UTXO entry.
 *
 * Serialized format:
 * - VARINT((coinbase ? 1 : 0) | (height << 1))
 * - the non-spent MCTxOut (via MCTxOutCompressor)
 */
class Coin
{
public:
    //! unspent transaction output
    MCTxOut out;

    //! whether containing transaction was a coinbase
    unsigned int fCoinBase : 1;

    //! at which height this containing transaction was included in the active block chain
    uint32_t nHeight : 31;

    //! construct a Coin from a MCTxOut and height/coinbase information.
    Coin(MCTxOut&& outIn, int nHeightIn, bool fCoinBaseIn) : out(std::move(outIn)), fCoinBase(fCoinBaseIn), nHeight(nHeightIn) {}
    Coin(const MCTxOut& outIn, int nHeightIn, bool fCoinBaseIn) : out(outIn), fCoinBase(fCoinBaseIn),nHeight(nHeightIn) {}

    void Clear() {
        out.SetNull();
        fCoinBase = false;
        nHeight = 0;
    }

    //! empty constructor
    Coin() : fCoinBase(false), nHeight(0) { }

    bool IsCoinBase() const {
        return fCoinBase;
    }

    bool IsCoinCreateBranch() const;

    template<typename Stream>
    void Serialize(Stream &s) const {
        assert(!IsSpent());
        uint32_t code = nHeight * 2 + fCoinBase;
        ::Serialize(s, VARINT(code));
        ::Serialize(s, MCTxOutCompressor(REF(out)));
    }

    template<typename Stream>
    void Unserialize(Stream &s) {
        uint32_t code = 0;
        ::Unserialize(s, VARINT(code));
        nHeight = code >> 1;
        fCoinBase = code & 1;
        ::Unserialize(s, REF(MCTxOutCompressor(out)));
    }

    bool IsSpent() const {
        return out.IsNull();
    }

    size_t DynamicMemoryUsage() const {
        return memusage::DynamicUsage(out.scriptPubKey);
    }
};

class SaltedOutpointHasher
{
private:
    /** Salt */
    const uint64_t k0, k1;

public:
    SaltedOutpointHasher();

    /**
     * This *must* return size_t. With Boost 1.46 on 32-bit systems the
     * unordered_map will behave unpredictably if the custom hasher returns a
     * uint64_t, resulting in failures when syncing the chain (#4634).
     */
    size_t operator()(const MCOutPoint& id) const {
        return SipHashUint256Extra(k0, k1, id.hash, id.n);
    }
};

struct MCCoinsCacheEntry
{
    Coin coin; // The actual cached data.
    unsigned char flags;

    enum Flags {
        DIRTY = (1 << 0), // This cache entry is potentially different from the version in the parent view.
        FRESH = (1 << 1), // The parent view does not have this entry (or it is pruned).
        /* Note that FRESH is a performance optimization with which we can
         * erase coins that are fully spent if we know we do not need to
         * flush the changes to the parent cache.  It is always safe to
         * not mark FRESH if that condition is not guaranteed.
         */
    };

    MCCoinsCacheEntry() : flags(0) {}
    explicit MCCoinsCacheEntry(Coin&& coin_) : coin(std::move(coin_)), flags(0) {}
};

typedef std::unordered_map<MCOutPoint, MCCoinsCacheEntry, SaltedOutpointHasher> MCCoinsMap;

/** Cursor for iterating over CoinsView state */
class MCCoinsViewCursor
{
public:
    MCCoinsViewCursor(const uint256 &hashBlockIn): hashBlock(hashBlockIn) {}
    virtual ~MCCoinsViewCursor() {}

    virtual bool GetKey(MCOutPoint &key) const = 0;
    virtual bool GetValue(Coin &coin) const = 0;
    virtual unsigned int GetValueSize() const = 0;

    virtual bool Valid() const = 0;
    virtual void Next() = 0;

    //! Get best block at the time this cursor was created
    const uint256 &GetBestBlock() const { return hashBlock; }
private:
    uint256 hashBlock;
};

/** Abstract view on the open txout dataset. */
class MCCoinsView
{
public:
    /** Retrieve the Coin (unspent transaction output) for a given outpoint.
     *  Returns true only when an unspent coin was found, which is returned in coin.
     *  When false is returned, coin's value is unspecified.
     */
    virtual bool GetCoin(const MCOutPoint &outpoint, Coin &coin) const;

    //! Just check whether a given outpoint is unspent.
    virtual bool HaveCoin(const MCOutPoint &outpoint) const;

    //! Retrieve the block hash whose state this MCCoinsView currently represents
    virtual uint256 GetBestBlock() const;

    //! Retrieve the range of blocks that may have been only partially written.
    //! If the database is in a consistent state, the result is the empty vector.
    //! Otherwise, a two-element vector is returned consisting of the new and
    //! the old block hash, in that order.
    virtual std::vector<uint256> GetHeadBlocks() const;

    //! Do a bulk modification (multiple Coin changes + BestBlock change).
    //! The passed mapCoins can be modified.
    virtual bool BatchWrite(MCCoinsMap &mapCoins, const uint256 &hashBlock);

    //! Get a cursor to iterate over the whole state
    virtual MCCoinsViewCursor *Cursor() const;

    //! As we use CCoinsViews polymorphically, have a virtual destructor
    virtual ~MCCoinsView() {}

    //! Estimate database size (0 if not implemented)
    virtual size_t EstimateSize() const { return 0; }
};


/** MCCoinsView backed by another MCCoinsView */
class MCCoinsViewBacked : public MCCoinsView
{
protected:
    MCCoinsView *base;

public:
    MCCoinsViewBacked(MCCoinsView *viewIn);
    bool GetCoin(const MCOutPoint &outpoint, Coin &coin) const override;
    bool HaveCoin(const MCOutPoint &outpoint) const override;
    uint256 GetBestBlock() const override;
    std::vector<uint256> GetHeadBlocks() const override;
    void SetBackend(MCCoinsView &viewIn);
    bool BatchWrite(MCCoinsMap &mapCoins, const uint256 &hashBlock) override;
    MCCoinsViewCursor *Cursor() const override;
    size_t EstimateSize() const override;
};


/** MCCoinsView that adds a memory cache for transactions to another MCCoinsView */
class MCCoinsViewCache : public MCCoinsViewBacked
{
protected:
    /**
     * Make mutable so that we can "fill the cache" even from Get-methods
     * declared as "const".  
     */
    mutable uint256 hashBlock;
    mutable MCCoinsMap cacheCoins;

    /* Cached dynamic memory usage for the inner Coin objects. */
    mutable size_t cachedCoinsUsage;


public:
	MCCoinsMap& GetCacheCoins() { return cacheCoins; }


public:
    MCCoinsViewCache(MCCoinsView *baseIn);

    // Standard MCCoinsView methods
    bool GetCoin(const MCOutPoint &outpoint, Coin &coin) const override;
    bool HaveCoin(const MCOutPoint &outpoint) const override;
    uint256 GetBestBlock() const override;
    void SetBestBlock(const uint256 &hashBlock);
    bool BatchWrite(MCCoinsMap &mapCoins, const uint256 &hashBlock) override;
    MCCoinsViewCursor* Cursor() const override {
        throw std::logic_error("MCCoinsViewCache cursor iteration not supported.");
    }

    /**
     * Check if we have the given utxo already loaded in this cache.
     * The semantics are the same as HaveCoin(), but no calls to
     * the backing MCCoinsView are made.
     */
    bool HaveCoinInCache(const MCOutPoint &outpoint) const;

    /**
     * Return a reference to Coin in the cache, or a pruned one if not found. This is
     * more efficient than GetCoin.
     *
     * Generally, do not hold the reference returned for more than a short scope.
     * While the current implementation allows for modifications to the contents
     * of the cache while holding the reference, this behavior should not be relied
     * on! To be safe, best to not hold the returned reference through any other
     * calls to this cache.
     */
    const Coin& AccessCoin(const MCOutPoint &output) const;

    /**
     * Add a coin. Set potential_overwrite to true if a non-pruned version may
     * already exist.
     */
    void AddCoin(const MCOutPoint& outpoint, Coin&& coin, bool potential_overwrite);

    /**
     * Spend a coin. Pass moveto in order to get the deleted data.
     * If no unspent output exists for the passed outpoint, this call
     * has no effect.
     */
    bool SpendCoin(const MCOutPoint &outpoint, Coin* moveto = nullptr);

    /**
     * Push the modifications applied to this cache to its base.
     * Failure to call this method before destruction will cause the changes to be forgotten.
     * If false is returned, the state of this cache (and its backing view) will be undefined.
     */
    bool Flush();

    /**
     * Removes the UTXO with the given outpoint from the cache, if it is
     * not modified.
     */
    void Uncache(const MCOutPoint &outpoint);

    //! Calculate the size of the cache (in number of transaction outputs)
    unsigned int GetCacheSize() const;

    //! Calculate the size of the cache (in bytes)
    size_t DynamicMemoryUsage() const;

    /** 
     * Amount of cells coming in to a transaction
     * Note that lightweight clients may not know anything besides the hash of previous transactions,
     * so may not be able to calculate this.
     *
     * @param[in] tx	transaction for which we are checking input total
     * @return	Sum of value of all inputs (scriptSigs)
     */
    MCAmount GetValueIn(const MCTransaction& tx) const;

    //! Check whether all prevouts of the transaction are present in the UTXO set represented by this view
    bool HaveInputs(const MCTransaction& tx) const;

private:
    MCCoinsMap::iterator FetchCoin(const MCOutPoint &outpoint) const;

    /**
     * By making the copy constructor private, we prevent accidentally using it when one intends to create a cache on top of a base cache.
     */
    MCCoinsViewCache(const MCCoinsViewCache &);
};

class CoinAmountCacheBase
{
public:
    virtual MCAmount GetAmount(const uint160& key) const
    {
        return 0;
    }
};

class CoinAmountDB : public CoinAmountCacheBase
{
public:
    MCAmount GetAmount(const uint160& key) const override;
};

class CoinAmountTemp : public CoinAmountCacheBase
{
public:
    MCAmount GetAmount(const uint160& key) const override;
    void IncAmount(const uint160& key, MCAmount delta);

private:
    std::map<uint160, MCAmount> coinAmountCache;
};

class CoinAmountCache
{
public:
    CoinAmountCache(CoinAmountCacheBase* amountBase) : base(amountBase) {}

    bool HasKeyInCache(const uint160& key) const;
    MCAmount GetAmount(const uint160& key);
    bool IncAmount(const uint160& key, MCAmount delta);
    bool DecAmount(const uint160& key, MCAmount delta);

    void TakeSnapshot(const uint160& key);
    void RemoveSnapshot(const uint160& key, bool reverse);

    void Clear();

private:
    bool takeSnapshot;
    CoinAmountCacheBase* base;
    std::map<uint160, MCAmount> snapshots;
    std::map<uint160, MCAmount> coinAmountCache;
    mutable MCCriticalSection cs;
};

//! Utility function to add all of a transaction's outputs to a cache.
// When check is false, this assumes that overwrites are only possible for coinbase transactions.
// When check is true, the underlying view may be queried to determine whether an addition is
// an overwrite.
// TODO: pass in a boolean to limit these possible overwrites to known
// (pre-BIP34) cases.
void AddCoins(MCCoinsViewCache& cache, const MCTransaction& tx, int nHeight, bool check = false);

//! Utility function to find any unspent output with a given txid.
// This function can be quite expensive because in the event of a transaction
// which is not found in the cache, it can cause up to MAX_OUTPUTS_PER_BLOCK
// lookups to database, so it should be used with care.
const Coin& AccessByTxid(const MCCoinsViewCache& cache, const uint256& txid);

#endif // MAGNACHAIN_COINS_H
