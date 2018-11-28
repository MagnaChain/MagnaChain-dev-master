// Copyright (c) 2012-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "transaction/coins.h"

#include "consensus/consensus.h"
#include "misc/memusage.h"
#include "misc/random.h"
#include "validation/validation.h"

#include <assert.h>

bool Coin::IsCoinCreateBranch() const
{
    return IsCoinCreateBranchScript(out.scriptPubKey);
}

bool CellCoinsView::GetCoin(const CellOutPoint &outpoint, Coin &coin) const { return false; }
uint256 CellCoinsView::GetBestBlock() const { return uint256(); }
std::vector<uint256> CellCoinsView::GetHeadBlocks() const { return std::vector<uint256>(); }
bool CellCoinsView::BatchWrite(CellCoinsMap &mapCoins, const uint256 &hashBlock) { return false; }
CellCoinsViewCursor *CellCoinsView::Cursor() const { return 0; }

bool CellCoinsView::HaveCoin(const CellOutPoint &outpoint) const
{
    Coin coin;
    return GetCoin(outpoint, coin);
}

CellCoinsViewBacked::CellCoinsViewBacked(CellCoinsView *viewIn) : base(viewIn) { }
bool CellCoinsViewBacked::GetCoin(const CellOutPoint &outpoint, Coin &coin) const { return base->GetCoin(outpoint, coin); }
bool CellCoinsViewBacked::HaveCoin(const CellOutPoint &outpoint) const { return base->HaveCoin(outpoint); }
uint256 CellCoinsViewBacked::GetBestBlock() const { return base->GetBestBlock(); }
std::vector<uint256> CellCoinsViewBacked::GetHeadBlocks() const { return base->GetHeadBlocks(); }
void CellCoinsViewBacked::SetBackend(CellCoinsView &viewIn) { base = &viewIn; }
bool CellCoinsViewBacked::BatchWrite(CellCoinsMap &mapCoins, const uint256 &hashBlock) { return base->BatchWrite(mapCoins, hashBlock); }
CellCoinsViewCursor *CellCoinsViewBacked::Cursor() const { return base->Cursor(); }
size_t CellCoinsViewBacked::EstimateSize() const { return base->EstimateSize(); }

SaltedOutpointHasher::SaltedOutpointHasher() : k0(GetRand(std::numeric_limits<uint64_t>::max())), k1(GetRand(std::numeric_limits<uint64_t>::max())) {}

CellCoinsViewCache::CellCoinsViewCache(CellCoinsView *baseIn) : CellCoinsViewBacked(baseIn), cachedCoinsUsage(0) {}


size_t CellCoinsViewCache::DynamicMemoryUsage() const {
    return memusage::DynamicUsage(cacheCoins) + cachedCoinsUsage;
}

CellCoinsMap::iterator CellCoinsViewCache::FetchCoin(const CellOutPoint &outpoint) const {
    CellCoinsMap::iterator it = cacheCoins.find(outpoint);
    if (it != cacheCoins.end())
        return it;
    Coin tmp;
    if (!base->GetCoin(outpoint, tmp))
        return cacheCoins.end();
    CellCoinsMap::iterator ret = cacheCoins.emplace(std::piecewise_construct, std::forward_as_tuple(outpoint), std::forward_as_tuple(std::move(tmp))).first;
    if (ret->second.coin.IsSpent()) {
        // The parent only has an empty entry for this outpoint; we can consider our
        // version as fresh.
        ret->second.flags = CellCoinsCacheEntry::FRESH;
    }
    cachedCoinsUsage += ret->second.coin.DynamicMemoryUsage();
    return ret;
}

bool CellCoinsViewCache::GetCoin(const CellOutPoint &outpoint, Coin &coin) const {
    CellCoinsMap::const_iterator it = FetchCoin(outpoint);
    if (it != cacheCoins.end()) {
        coin = it->second.coin;
        return !coin.IsSpent();
    }
    return false;
}

void CellCoinsViewCache::AddCoin(const CellOutPoint &outpoint, Coin&& coin, bool possible_overwrite) {
	//LogPrintf("AddCoin %s %d\n", outpoint.hash.ToString().c_str(), outpoint.n);
	assert(!coin.IsSpent());
    if (coin.out.scriptPubKey.IsUnspendable()) return;
    CellCoinsMap::iterator it;
    bool inserted;
    std::tie(it, inserted) = cacheCoins.emplace(std::piecewise_construct, std::forward_as_tuple(outpoint), std::tuple<>());
    bool fresh = false;
    if (!inserted) {
        cachedCoinsUsage -= it->second.coin.DynamicMemoryUsage();
    }
    if (!possible_overwrite) {
        if (!it->second.coin.IsSpent()) {
            throw std::logic_error("Adding new coin that replaces non-pruned entry");
        }
        fresh = !(it->second.flags & CellCoinsCacheEntry::DIRTY);
    }
    it->second.coin = std::move(coin);
    it->second.flags |= CellCoinsCacheEntry::DIRTY | (fresh ? CellCoinsCacheEntry::FRESH : 0);
    cachedCoinsUsage += it->second.coin.DynamicMemoryUsage();

	// add to first trans point
	//if (this == pcoinsTip) {
	//	pcoinListDb->OnAddCoin(outpoint, it->second.coin, possible_overwrite);
	//}
}


void AddCoins(CellCoinsViewCache& cache, const CellTransaction &tx, int nHeight, bool check) {
	//LogPrintf("AddCoins tx %s\n", tx.GetHash().ToString().c_str());
    bool fCoinbase = tx.IsCoinBase();
    const uint256& txid = tx.GetHash();
    for (size_t i = 0; i < tx.vout.size(); ++i) {
        bool overwrite = check ? cache.HaveCoin(CellOutPoint(txid, i)) : fCoinbase;
        // Always set the possible_overwrite flag to AddCoin for coinbase txn, in order to correctly
        // deal with the pre-BIP30 occurrences of duplicate coinbase transactions.
        cache.AddCoin(CellOutPoint(txid, i), Coin(tx.vout[i], nHeight, fCoinbase), overwrite);
    }
}

bool CellCoinsViewCache::SpendCoin(const CellOutPoint &outpoint, Coin* moveout) {
    CellCoinsMap::iterator it = FetchCoin(outpoint);
    if (it == cacheCoins.end()) return false;
    cachedCoinsUsage -= it->second.coin.DynamicMemoryUsage();
	//LogPrintf("SpendCoin %s %d\n", outpoint.hash.ToString().c_str(), outpoint.n);
	//if (this == pcoinsTip) {
	//	pcoinListDb->OnSpendCoin(outpoint, it->second.coin);
	//}

    if (moveout) {
        *moveout = std::move(it->second.coin);
    }
    if (it->second.flags & CellCoinsCacheEntry::FRESH) {
        cacheCoins.erase(it);
    } else {
        it->second.flags |= CellCoinsCacheEntry::DIRTY;
        it->second.coin.Clear();
    }
    return true;
}

static const Coin coinEmpty;

const Coin& CellCoinsViewCache::AccessCoin(const CellOutPoint &outpoint) const {
    CellCoinsMap::const_iterator it = FetchCoin(outpoint);
    if (it == cacheCoins.end()) {
        return coinEmpty;
    } else {
        return it->second.coin;
    }
}

bool CellCoinsViewCache::HaveCoin(const CellOutPoint &outpoint) const {
    CellCoinsMap::const_iterator it = FetchCoin(outpoint);
    return (it != cacheCoins.end() && !it->second.coin.IsSpent());
}

bool CellCoinsViewCache::HaveCoinInCache(const CellOutPoint &outpoint) const {
    CellCoinsMap::const_iterator it = cacheCoins.find(outpoint);
    return (it != cacheCoins.end() && !it->second.coin.IsSpent());
}

bool CellCoinsViewCache::Flush()
{
    if (base == pcoinsTip) {
        pcoinListDb->ImportCoins(cacheCoins);
    }

    bool fOk = base->BatchWrite(cacheCoins, hashBlock);
    cacheCoins.clear();
    cachedCoinsUsage = 0;
    return fOk;
}

uint256 CellCoinsViewCache::GetBestBlock() const {
    if (hashBlock.IsNull())
        hashBlock = base->GetBestBlock();
    return hashBlock;
}

void CellCoinsViewCache::SetBestBlock(const uint256 &hashBlockIn) {
    hashBlock = hashBlockIn;
}

bool CellCoinsViewCache::BatchWrite(CellCoinsMap &mapCoins, const uint256 &hashBlockIn) {
    for (CellCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end();) {
        if (it->second.flags & CellCoinsCacheEntry::DIRTY) { // Ignore non-dirty entries (optimization).
            CellCoinsMap::iterator itUs = cacheCoins.find(it->first);
            if (itUs == cacheCoins.end()) {
                // The parent cache does not have an entry, while the child does
                // We can ignore it if it's both FRESH and pruned in the child
                if (!(it->second.flags & CellCoinsCacheEntry::FRESH && it->second.coin.IsSpent())) {
                    // Otherwise we will need to create it in the parent
                    // and move the data up and mark it as dirty
                    CellCoinsCacheEntry& entry = cacheCoins[it->first];
                    entry.coin = std::move(it->second.coin);
                    cachedCoinsUsage += entry.coin.DynamicMemoryUsage();
                    entry.flags = CellCoinsCacheEntry::DIRTY;
                    // We can mark it FRESH in the parent if it was FRESH in the child
                    // Otherwise it might have just been flushed from the parent's cache
                    // and already exist in the grandparent
                    if (it->second.flags & CellCoinsCacheEntry::FRESH)
                        entry.flags |= CellCoinsCacheEntry::FRESH;
                }
            } else {
                // Assert that the child cache entry was not marked FRESH if the
                // parent cache entry has unspent outputs. If this ever happens,
                // it means the FRESH flag was misapplied and there is a logic
                // error in the calling code.
                if ((it->second.flags & CellCoinsCacheEntry::FRESH) && !itUs->second.coin.IsSpent())
                    throw std::logic_error("FRESH flag misapplied to cache entry for base transaction with spendable outputs");

                // Found the entry in the parent cache
                if ((itUs->second.flags & CellCoinsCacheEntry::FRESH) && it->second.coin.IsSpent()) {
                    // The grandparent does not have an entry, and the child is
                    // modified and being pruned. This means we can just delete
                    // it from the parent.
                    cachedCoinsUsage -= itUs->second.coin.DynamicMemoryUsage();
                    cacheCoins.erase(itUs);
                } else {
                    // A normal modification.
                    cachedCoinsUsage -= itUs->second.coin.DynamicMemoryUsage();
                    itUs->second.coin = std::move(it->second.coin);
                    cachedCoinsUsage += itUs->second.coin.DynamicMemoryUsage();
                    itUs->second.flags |= CellCoinsCacheEntry::DIRTY;
                    // NOTE: It is possible the child has a FRESH flag here in
                    // the event the entry we found in the parent is pruned. But
                    // we must not copy that FRESH flag to the parent as that
                    // pruned state likely still needs to be communicated to the
                    // grandparent.
                }
            }
        }
        CellCoinsMap::iterator itOld = it++;
        mapCoins.erase(itOld);
    }
    hashBlock = hashBlockIn;
    return true;
}


void CellCoinsViewCache::Uncache(const CellOutPoint& hash)
{
    CellCoinsMap::iterator it = cacheCoins.find(hash);
    if (it != cacheCoins.end() && it->second.flags == 0) {
        cachedCoinsUsage -= it->second.coin.DynamicMemoryUsage();
        cacheCoins.erase(it);
    }
}

unsigned int CellCoinsViewCache::GetCacheSize() const {
    return cacheCoins.size();
}

CellAmount CellCoinsViewCache::GetValueIn(const CellTransaction& tx) const
{
    if (tx.IsCoinBase())
        return 0;

    bool includeContract = false;
    CellAmount nResult = 0;
    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        nResult += AccessCoin(tx.vin[i].prevout).out.nValue;
        if (tx.vin[i].scriptSig.IsContract())
            includeContract = true;
    }

    if (tx.nVersion == CellTransaction::CALL_CONTRACT_VERSION && !includeContract && tx.pContractData->amountOut > 0) {
        nResult += tx.pContractData->amountOut;
        if (!MoneyRange(nResult) || !MoneyRange(tx.pContractData->amountOut))
            return 0;
    }

    return nResult;
}

bool CellCoinsViewCache::HaveInputs(const CellTransaction& tx) const
{
    if (!tx.IsCoinBase()) {
        for (unsigned int i = 0; i < tx.vin.size(); i++) {
            if (!HaveCoin(tx.vin[i].prevout)) {
                return false;
            }
        }
    }
    return true;
}

CellAmount CoinAmountDB::GetAmount(const uint160& key) const
{
    CellAmount nValue = 0;
    CoinListPtr plist = pcoinListDb->GetList(key);
    for (auto it = plist->coins.begin(); it != plist->coins.end(); ++it) {
        const CellOutPoint& outpoint = *it;
        const Coin& coin = pcoinsTip->AccessCoin(outpoint);

        if (coin.IsSpent())
            continue;
        if (coin.IsCoinBase() && chainActive.Height() - coin.nHeight < COINBASE_MATURITY)
            continue;

        nValue += coin.out.nValue;
    }
    return nValue;
}

CellAmount CoinAmountTemp::GetAmount(const uint160& key) const
{
    auto it = coinAmountCache.find(key);
    if (it == coinAmountCache.end())
        return 0;
    return it->second;
}

void CoinAmountTemp::IncAmount(const uint160& key, CellAmount delta)
{
    if (delta <= 0)
        return;
    CellAmount nValue = GetAmount(key);
    nValue += delta;
    coinAmountCache[key] = nValue;
}

bool CoinAmountCache::HasKeyInCache(const uint160& key) const
{
    return (coinAmountCache.count(key) > 0);
}

CellAmount CoinAmountCache::GetAmount(const uint160& key)
{
    CellAmount nValue = 0;
    if (coinAmountCache.count(key) == 0) {
        if (base != nullptr)
            nValue = base->GetAmount(key);
        coinAmountCache[key] = nValue;
    }
    else
        nValue = coinAmountCache[key];

    if (takeSnapshot) {
        takeSnapshot = false;
        snapshots[key] = nValue;
    }

    return nValue;
}

bool CoinAmountCache::IncAmount(const uint160& key, CellAmount delta)
{
    if (delta < 0)
        return false;

    if (delta == 0)
        return true;

    CellAmount value = GetAmount(key);
    value += delta;
    coinAmountCache[key] = value;
    return true;
}

bool CoinAmountCache::DecAmount(const uint160& key, CellAmount delta)
{
    if (delta < 0)
        return false;

    if (delta == 0)
        return true;

    CellAmount value = GetAmount(key);
    if (value < delta)
        return false;
    value -= delta;
    coinAmountCache[key] = value;
    return true;
}

void CoinAmountCache::TakeSnapshot(const uint160& key)
{
    takeSnapshot = true;
}

void CoinAmountCache::RemoveSnapshot(const uint160& key, bool reverse)
{
    if (!takeSnapshot) {
        CellAmount value = GetAmount(key);
        snapshots.erase(key);
        if (reverse)
            coinAmountCache[key] = value;
    }
}

void CoinAmountCache::Clear()
{
    snapshots.clear();
    coinAmountCache.clear();
}

static const size_t MIN_TRANSACTION_OUTPUT_WEIGHT = WITNESS_SCALE_FACTOR * ::GetSerializeSize(CellTxOut(), SER_NETWORK, PROTOCOL_VERSION);
static const size_t MAX_OUTPUTS_PER_BLOCK = MAX_BLOCK_WEIGHT / MIN_TRANSACTION_OUTPUT_WEIGHT;

const Coin& AccessByTxid(const CellCoinsViewCache& view, const uint256& txid)
{
    CellOutPoint iter(txid, 0);
    while (iter.n < MAX_OUTPUTS_PER_BLOCK) {
        const Coin& alternate = view.AccessCoin(iter);
        if (!alternate.IsSpent()) return alternate;
        ++iter.n;
    }
    return coinEmpty;
}
