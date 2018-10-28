// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_MINER_H
#define CELLLINK_MINER_H

#include "primitives/block.h"
#include "transaction/txmempool.h"

#include <stdint.h>
#include <memory>
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/ordered_index.hpp"

class CellBlockIndex;
class CellChainParams;
class CellScript;
class CellKeyStore;

namespace Consensus { struct Params; };

static const bool DEFAULT_PRINTPRIORITY = false;

struct CellBlockTemplate
{
    CellBlock block;
    std::vector<CellAmount> vTxFees;
    std::vector<int64_t> vTxSigOpsCost;
    std::vector<unsigned char> vchCoinbaseCommitment;
};

// Container for tracking updates to ancestor feerate as we include (parent)
// transactions in a block
struct CellTxMemPoolModifiedEntry {
    CellTxMemPoolModifiedEntry(CellTxMemPool::txiter entry)
    {
        iter = entry;
        nSizeWithAncestors = entry->GetSizeWithAncestors();
        nModFeesWithAncestors = entry->GetModFeesWithAncestors();
        nSigOpCostWithAncestors = entry->GetSigOpCostWithAncestors();
    }

    CellTxMemPool::txiter iter;
    uint64_t nSizeWithAncestors;
    CellAmount nModFeesWithAncestors;
    int64_t nSigOpCostWithAncestors;
};

/** Comparator for CellTxMemPool::txiter objects.
 *  It simply compares the internal memory address of the CellTxMemPoolEntry object
 *  pointed to. This means it has no meaning, and is only useful for using them
 *  as key in other indexes.
 */
struct CompareCTxMemPoolIter {
    bool operator()(const CellTxMemPool::txiter& a, const CellTxMemPool::txiter& b) const
    {
        return &(*a) < &(*b);
    }
};

struct modifiedentry_iter {
    typedef CellTxMemPool::txiter result_type;
    result_type operator() (const CellTxMemPoolModifiedEntry &entry) const
    {
        return entry.iter;
    }
};

// This matches the calculation in CompareTxMemPoolEntryByAncestorFee,
// except operating on CellTxMemPoolModifiedEntry.
// TODO: refactor to avoid duplication of this logic.
struct CompareModifiedEntry {
    bool operator()(const CellTxMemPoolModifiedEntry &a, const CellTxMemPoolModifiedEntry &b) const
    {
        double f1 = (double)a.nModFeesWithAncestors * b.nSizeWithAncestors;
        double f2 = (double)b.nModFeesWithAncestors * a.nSizeWithAncestors;
        if (f1 == f2) {
            return CellTxMemPool::CompareIteratorByHash()(a.iter, b.iter);
        }
        return f1 > f2;
    }
};

// A comparator that sorts transactions based on number of ancestors.
// This is sufficient to sort an ancestor package in an order that is valid
// to appear in a block.
struct CompareTxIterByAncestorCount {
    bool operator()(const CellTxMemPool::txiter &a, const CellTxMemPool::txiter &b)
    {
        if (a->GetCountWithAncestors() != b->GetCountWithAncestors())
            return a->GetCountWithAncestors() < b->GetCountWithAncestors();
        return CellTxMemPool::CompareIteratorByHash()(a, b);
    }
};

typedef boost::multi_index_container<
    CellTxMemPoolModifiedEntry,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            modifiedentry_iter,
            CompareCTxMemPoolIter
        >,
        // sorted by modified ancestor fee rate
        boost::multi_index::ordered_non_unique<
            // Reuse same tag from CellTxMemPool's similar index
            boost::multi_index::tag<ancestor_score>,
            boost::multi_index::identity<CellTxMemPoolModifiedEntry>,
            CompareModifiedEntry
        >
    >
> indexed_modified_transaction_set;

typedef indexed_modified_transaction_set::nth_index<0>::type::iterator modtxiter;
typedef indexed_modified_transaction_set::index<ancestor_score>::type::iterator modtxscoreiter;

struct update_for_parent_inclusion
{
    update_for_parent_inclusion(CellTxMemPool::txiter it) : iter(it) {}

    void operator() (CellTxMemPoolModifiedEntry &e) const
    {
        e.nModFeesWithAncestors -= iter->GetFee();
        e.nSizeWithAncestors -= iter->GetTxSize();
        e.nSigOpCostWithAncestors -= iter->GetSigOpCost();
    }

    CellTxMemPool::txiter iter;
};

class BranchUTXOCache
{
public:
    typedef std::map<CellOutPoint, CellTxOut> MAP_CACHE_COIN;

    CoinList coinlist;
    MAP_CACHE_COIN mapCacheCoin;
};

class MakeBranchTxUTXO
{
public:
    typedef std::map<uint160, BranchUTXOCache> MAP_BRANCH_COINS;
    typedef std::map<uint256, CellTransactionRef> MAP_MAKE_CACHE;
    
    CellAmount UseUTXO(uint160& key, CellAmount nAmount, std::vector<CellOutPoint>& vInOutPoints);
    bool MakeTxUTXO(CellMutableTransaction& tx, uint160& key, CellAmount nAmount, CellScript& scriptSig, CellScript& changeScriptPubKey);

    MAP_MAKE_CACHE mapCache;
    MAP_BRANCH_COINS mapBranchCoins;
};

/** Generate a new block, without valid proof-of-work */
class BlockAssembler
{
private:
    // The constructed block template
    std::unique_ptr<CellBlockTemplate> pblocktemplate;
    // A convenience pointer that always refers to the CellBlock in pblocktemplate
    CellBlock* pblock;

    // Configuration parameters for the block size
    bool fIncludeWitness;
    unsigned int nBlockMaxWeight;
    CellFeeRate blockMinFeeRate;

    // Information on the current status of the block
    uint64_t nBlockWeight;
    uint64_t nBlockTx;
    uint64_t nBlockSigOpsCost;
    CellAmount nFees;
    CellTxMemPool::setEntries inBlock;

    // Chain context for the block
    int nHeight;
    int64_t nLockTimeCutoff;
    const CellChainParams& chainparams;
	CellOutPoint outpoint;

public:
    struct Options {
        Options();
        size_t nBlockMaxWeight;
        size_t nBlockMaxSize;
        CellFeeRate blockMinFeeRate;
		CellOutPoint outpoint;
    };
	static Options DefaultOptions(const CellChainParams& params );

    BlockAssembler(const CellChainParams& params);
    BlockAssembler(const CellChainParams& params, const Options& options);

    /** Construct a new block template with coinbase to scriptPubKeyIn */
    std::unique_ptr<CellBlockTemplate> CreateNewBlock(const CellScript& scriptPubKeyIn, ContractContext* pContractContext, bool fMineWitnessTx=true, const CellKeyStore* keystoreIn = nullptr, CellCoinsViewCache *pcoinsCache = nullptr);

private:
    // utility functions
    /** Clear the block's state and prepare for assembling a new block */
    void resetBlock();
    /** Add a tx to the block */
    void AddToBlock(CellTxMemPool::txiter iter, MakeBranchTxUTXO& utxoMaker);

    void GroupingTransaction(int offset, std::vector<const CellTxMemPoolEntry*>& blockTxEntries);

    // Methods for how to add transactions to a block.
    /** Add transactions based on feerate including unconfirmed ancestors
      * Increments nPackagesSelected / nDescendantsUpdated with corresponding
      * statistics from the package selection (for logging statistics). */
    void addPackageTxs(int &nPackagesSelected, int &nDescendantsUpdated);

    // helper functions for addPackageTxs()
    /** Remove confirmed (inBlock) entries from given set */
    void onlyUnconfirmed(CellTxMemPool::setEntries& testSet);
    /** Test if a new package would "fit" in the block */
    bool TestPackage(uint64_t packageSize, int64_t packageSigOpsCost);
    /** Perform checks on each transaction in a package:
      * locktime, premature-witness, serialized size (if necessary)
      * These checks should always succeed, and they're here
      * only as an extra check in case of suboptimal node configuration */
    bool TestPackageTransactions(const CellTxMemPool::setEntries& package);
    /** Return true if given transaction from mapTx has already been evaluated,
      * or if the transaction's cached data in mapTx is incorrect. */
    bool SkipMapTxEntry(CellTxMemPool::txiter it, indexed_modified_transaction_set &mapModifiedTx, CellTxMemPool::setEntries &failedTx);
    /** Sort the package in an order that is valid to appear in a block */
    void SortForBlock(const CellTxMemPool::setEntries& package, CellTxMemPool::txiter entry, std::vector<CellTxMemPool::txiter>& sortedEntries);
    /** Add descendants of given transactions to mapModifiedTx with ancestor
      * state updated assuming given transactions are inBlock. Returns number
      * of updated descendants. */
    int UpdatePackagesForAdded(const CellTxMemPool::setEntries& alreadyAdded, indexed_modified_transaction_set &mapModifiedTx);

    //
    void addReportProofTx(const CellTransactionRef &ptxReport, const CellScript &minerpkey, const CellCoinsViewCache* pCoinsCache);
    void addReportProofTxs(const CellScript& scriptPubKeyIn, CellCoinsViewCache *pcoinsCache);
    bool UpdateBranchTx(CellTxMemPool::txiter iter, MakeBranchTxUTXO& utxoMaker);
};

/** Modify the extranonce in a block */
void IncrementExtraNonce(CellBlock* pblock, const CellBlockIndex* pindexPrev, unsigned int& nExtraNonce);
int64_t UpdateTime(CellBlockHeader* pblock, const Consensus::Params& consensusParams, const CellBlockIndex* pindexPrev);

#endif // CELLLINK_MINER_H
