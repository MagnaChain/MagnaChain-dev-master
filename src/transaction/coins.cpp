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

bool MCCoinsView::GetCoin(const MCOutPoint &outpoint, Coin &coin) const { return false; }
uint256 MCCoinsView::GetBestBlock() const { return uint256(); }
std::vector<uint256> MCCoinsView::GetHeadBlocks() const { return std::vector<uint256>(); }
bool MCCoinsView::BatchWrite(MCCoinsMap &mapCoins, const uint256 &hashBlock) { return false; }
MCCoinsViewCursor *MCCoinsView::Cursor() const { return 0; }

bool MCCoinsView::HaveCoin(const MCOutPoint &outpoint) const
{
    Coin coin;
    return GetCoin(outpoint, coin);
}

MCCoinsViewBacked::MCCoinsViewBacked(MCCoinsView *viewIn) : base(viewIn) { }
bool MCCoinsViewBacked::GetCoin(const MCOutPoint &outpoint, Coin &coin) const { return base->GetCoin(outpoint, coin); }
bool MCCoinsViewBacked::HaveCoin(const MCOutPoint &outpoint) const { return base->HaveCoin(outpoint); }
uint256 MCCoinsViewBacked::GetBestBlock() const { return base->GetBestBlock(); }
std::vector<uint256> MCCoinsViewBacked::GetHeadBlocks() const { return base->GetHeadBlocks(); }
void MCCoinsViewBacked::SetBackend(MCCoinsView &viewIn) { base = &viewIn; }
bool MCCoinsViewBacked::BatchWrite(MCCoinsMap &mapCoins, const uint256 &hashBlock) { return base->BatchWrite(mapCoins, hashBlock); }
MCCoinsViewCursor *MCCoinsViewBacked::Cursor() const { return base->Cursor(); }
size_t MCCoinsViewBacked::EstimateSize() const { return base->EstimateSize(); }

SaltedOutpointHasher::SaltedOutpointHasher() : k0(GetRand(std::numeric_limits<uint64_t>::max())), k1(GetRand(std::numeric_limits<uint64_t>::max())) {}

MCCoinsViewCache::MCCoinsViewCache(MCCoinsView *baseIn) : MCCoinsViewBacked(baseIn), cachedCoinsUsage(0) {}


size_t MCCoinsViewCache::DynamicMemoryUsage() const {
    return memusage::DynamicUsage(cacheCoins) + cachedCoinsUsage;
}

MCCoinsMap::iterator MCCoinsViewCache::FetchCoin(const MCOutPoint &outpoint) const {
    MCCoinsMap::iterator it = cacheCoins.find(outpoint);
    if (it != cacheCoins.end())
        return it;
    Coin tmp;
    if (!base->GetCoin(outpoint, tmp))
        return cacheCoins.end();
    MCCoinsMap::iterator ret = cacheCoins.emplace(std::piecewise_construct, std::forward_as_tuple(outpoint), std::forward_as_tuple(std::move(tmp))).first;
    if (ret->second.coin.IsSpent()) {
        // The parent only has an empty entry for this outpoint; we can consider our
        // version as fresh.
        ret->second.flags = MCCoinsCacheEntry::FRESH;
    }
    cachedCoinsUsage += ret->second.coin.DynamicMemoryUsage();
    return ret;
}

bool MCCoinsViewCache::GetCoin(const MCOutPoint &outpoint, Coin &coin) const {
    MCCoinsMap::const_iterator it = FetchCoin(outpoint);
    if (it != cacheCoins.end()) {
        coin = it->second.coin;
        return !coin.IsSpent();
    }
    return false;
}

void MCCoinsViewCache::AddCoin(const MCOutPoint &outpoint, Coin&& coin, bool possible_overwrite) {
	//LogPrintf("AddCoin %s %d\n", outpoint.hash.ToString().c_str(), outpoint.n);
	assert(!coin.IsSpent());
    if (coin.out.scriptPubKey.IsUnspendable()) return;
    MCCoinsMap::iterator it;
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
        fresh = !(it->second.flags & MCCoinsCacheEntry::DIRTY);
    }
    it->second.coin = std::move(coin);
    it->second.flags |= MCCoinsCacheEntry::DIRTY | (fresh ? MCCoinsCacheEntry::FRESH : 0);
    cachedCoinsUsage += it->second.coin.DynamicMemoryUsage();

	// add to first trans point
	//if (this == pcoinsTip) {
	//	pcoinListDb->OnAddCoin(outpoint, it->second.coin, possible_overwrite);
	//}
}


void AddCoins(MCCoinsViewCache& cache, const MCTransaction &tx, int nHeight, bool check) {
	//LogPrintf("AddCoins tx %s\n", tx.GetHash().ToString().c_str());
    bool fCoinbase = tx.IsCoinBase();
    const uint256& txid = tx.GetHash();
    for (size_t i = 0; i < tx.vout.size(); ++i) {
        bool overwrite = check ? cache.HaveCoin(MCOutPoint(txid, i)) : fCoinbase;
        // Always set the possible_overwrite flag to AddCoin for coinbase txn, in order to correctly
        // deal with the pre-BIP30 occurrences of duplicate coinbase transactions.
        cache.AddCoin(MCOutPoint(txid, i), Coin(tx.vout[i], nHeight, fCoinbase), overwrite);
    }
}

bool MCCoinsViewCache::SpendCoin(const MCOutPoint &outpoint, Coin* moveout) {
    MCCoinsMap::iterator it = FetchCoin(outpoint);
    if (it == cacheCoins.end()) return false;
    cachedCoinsUsage -= it->second.coin.DynamicMemoryUsage();
	//LogPrintf("SpendCoin %s %d\n", outpoint.hash.ToString().c_str(), outpoint.n);
	//if (this == pcoinsTip) {
	//	pcoinListDb->OnSpendCoin(outpoint, it->second.coin);
	//}

    if (moveout) {
        *moveout = std::move(it->second.coin);
    }
    if (it->second.flags & MCCoinsCacheEntry::FRESH) {
        cacheCoins.erase(it);
    } else {
        it->second.flags |= MCCoinsCacheEntry::DIRTY;
        it->second.coin.Clear();
    }
    return true;
}

static const Coin coinEmpty;

const Coin& MCCoinsViewCache::AccessCoin(const MCOutPoint &outpoint) const {
    MCCoinsMap::const_iterator it = FetchCoin(outpoint);
    if (it == cacheCoins.end()) {
        return coinEmpty;
    } else {
        return it->second.coin;
    }
}

bool MCCoinsViewCache::HaveCoin(const MCOutPoint &outpoint) const {
    MCCoinsMap::const_iterator it = FetchCoin(outpoint);
    return (it != cacheCoins.end() && !it->second.coin.IsSpent());
}

bool MCCoinsViewCache::HaveCoinInCache(const MCOutPoint &outpoint) const {
    MCCoinsMap::const_iterator it = cacheCoins.find(outpoint);
    return (it != cacheCoins.end() && !it->second.coin.IsSpent());
}

bool MCCoinsViewCache::Flush()
{
    if (base == pcoinsTip) {
        pcoinListDb->ImportCoins(cacheCoins);
    }

    bool fOk = base->BatchWrite(cacheCoins, hashBlock);
    cacheCoins.clear();
    cachedCoinsUsage = 0;
    return fOk;
}

uint256 MCCoinsViewCache::GetBestBlock() const {
    if (hashBlock.IsNull())
        hashBlock = base->GetBestBlock();
    return hashBlock;
}

void MCCoinsViewCache::SetBestBlock(const uint256 &hashBlockIn) {
    hashBlock = hashBlockIn;
}

bool MCCoinsViewCache::BatchWrite(MCCoinsMap &mapCoins, const uint256 &hashBlockIn) {
    for (MCCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end();) {
        if (it->second.flags & MCCoinsCacheEntry::DIRTY) { // Ignore non-dirty entries (optimization).
            MCCoinsMap::iterator itUs = cacheCoins.find(it->first);
            if (itUs == cacheCoins.end()) {
                // The parent cache does not have an entry, while the child does
                // We can ignore it if it's both FRESH and pruned in the child
                if (!(it->second.flags & MCCoinsCacheEntry::FRESH && it->second.coin.IsSpent())) {
                    // Otherwise we will need to create it in the parent
                    // and move the data up and mark it as dirty
                    MCCoinsCacheEntry& entry = cacheCoins[it->first];
                    entry.coin = std::move(it->second.coin);
                    cachedCoinsUsage += entry.coin.DynamicMemoryUsage();
                    entry.flags = MCCoinsCacheEntry::DIRTY;
                    // We can mark it FRESH in the parent if it was FRESH in the child
                    // Otherwise it might have just been flushed from the parent's cache
                    // and already exist in the grandparent
                    if (it->second.flags & MCCoinsCacheEntry::FRESH)
                        entry.flags |= MCCoinsCacheEntry::FRESH;
                }
            } else {
                // Assert that the child cache entry was not marked FRESH if the
                // parent cache entry has unspent outputs. If this ever happens,
                // it means the FRESH flag was misapplied and there is a logic
                // error in the calling code.
                if ((it->second.flags & MCCoinsCacheEntry::FRESH) && !itUs->second.coin.IsSpent())
                    throw std::logic_error("FRESH flag misapplied to cache entry for base transaction with spendable outputs");

                // Found the entry in the parent cache
                if ((itUs->second.flags & MCCoinsCacheEntry::FRESH) && it->second.coin.IsSpent()) {
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
                    itUs->second.flags |= MCCoinsCacheEntry::DIRTY;
                    // NOTE: It is possible the child has a FRESH flag here in
                    // the event the entry we found in the parent is pruned. But
                    // we must not copy that FRESH flag to the parent as that
                    // pruned state likely still needs to be communicated to the
                    // grandparent.
                }
            }
        }
        MCCoinsMap::iterator itOld = it++;
        mapCoins.erase(itOld);
    }
    hashBlock = hashBlockIn;
    return true;
}


void MCCoinsViewCache::Uncache(const MCOutPoint& hash)
{
    MCCoinsMap::iterator it = cacheCoins.find(hash);
    if (it != cacheCoins.end() && it->second.flags == 0) {
        cachedCoinsUsage -= it->second.coin.DynamicMemoryUsage();
        cacheCoins.erase(it);
    }
}

unsigned int MCCoinsViewCache::GetCacheSize() const {
    return cacheCoins.size();
}

MCAmount MCCoinsViewCache::GetValueIn(const MCTransaction& tx) const
{
    if (tx.IsCoinBase())
        return 0;

    bool includeContract = false;
    MCAmount nResult = 0;
    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        nResult += AccessCoin(tx.vin[i].prevout).out.nValue;
        if (tx.vin[i].scriptSig.IsContract())
            includeContract = true;
    }

    if (tx.nVersion == MCTransaction::CALL_CONTRACT_VERSION && !includeContract && tx.pContractData->amountOut > 0) {
        nResult += tx.pContractData->amountOut;
        if (!MoneyRange(nResult) || !MoneyRange(tx.pContractData->amountOut))
            return 0;
    }

    return nResult;
}

bool MCCoinsViewCache::HaveInputs(const MCTransaction& tx) const
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

MCAmount CoinAmountDB::GetAmount(const uint160& key) const
{
    MCAmount nValue = 0;
    CoinListPtr plist = pcoinListDb->GetList(key);
    for (auto it = plist->coins.begin(); it != plist->coins.end(); ++it) {
        const MCOutPoint& outpoint = *it;
        const Coin& coin = pcoinsTip->AccessCoin(outpoint);

        if (coin.IsSpent())
            continue;
        if (coin.IsCoinBase() && chainActive.Height() - coin.nHeight < COINBASE_MATURITY)
            continue;

        nValue += coin.out.nValue;
    }
    return nValue;
}

MCAmount CoinAmountTemp::GetAmount(const uint160& key) const
{
    auto it = coinAmountCache.find(key);
    if (it == coinAmountCache.end())
        return 0;
    return it->second;
}

void CoinAmountTemp::IncAmount(const uint160& key, MCAmount delta)
{
    if (delta <= 0)
        return;
    MCAmount nValue = GetAmount(key);
    nValue += delta;
    coinAmountCache[key] = nValue;
}

bool CoinAmountCache::HasKeyInCache(const uint160& key) const
{
    LOCK(cs);
    return (coinAmountCache.count(key) > 0);
}

MCAmount CoinAmountCache::GetAmount(const uint160& key)
{
    LOCK(cs);
    MCAmount nValue = 0;
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

bool CoinAmountCache::IncAmount(const uint160& key, MCAmount delta)
{
    if (delta < 0)
        return false;

    if (delta == 0)
        return true;

    LOCK(cs);
    MCAmount value = GetAmount(key);
    value += delta;
    coinAmountCache[key] = value;
    return true;
}

bool CoinAmountCache::DecAmount(const uint160& key, MCAmount delta)
{
    if (delta < 0)
        return false;

    if (delta == 0)
        return true;

    LOCK(cs);
    MCAmount value = GetAmount(key);
    if (value < delta)
        return false;
    value -= delta;
    coinAmountCache[key] = value;
    return true;
}

void CoinAmountCache::TakeSnapshot(const uint160& key)
{
    LOCK(cs);
    takeSnapshot = true;
}

void CoinAmountCache::RemoveSnapshot(const uint160& key, bool reverse)
{
    LOCK(cs);
    if (snapshots.count(key)) {
        if (reverse) {
            coinAmountCache[key] = snapshots[key];
        }
        snapshots.erase(key);
    }
}

void CoinAmountCache::Clear()
{
    LOCK(cs);
    snapshots.clear();
    coinAmountCache.clear();
}

static const size_t MIN_TRANSACTION_OUTPUT_WEIGHT = WITNESS_SCALE_FACTOR * ::GetSerializeSize(MCTxOut(), SER_NETWORK, PROTOCOL_VERSION);
static const size_t MAX_OUTPUTS_PER_BLOCK = MAX_BLOCK_WEIGHT / MIN_TRANSACTION_OUTPUT_WEIGHT;

const Coin& AccessByTxid(const MCCoinsViewCache& view, const uint256& txid)
{
    MCOutPoint iter(txid, 0);
    while (iter.n < MAX_OUTPUTS_PER_BLOCK) {
        const Coin& alternate = view.AccessCoin(iter);
        if (!alternate.IsSpent()) return alternate;
        ++iter.n;
    }
    return coinEmpty;
}
