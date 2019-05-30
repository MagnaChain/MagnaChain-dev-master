// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_VALIDATION_H
#define MAGNACHAIN_VALIDATION_H

#if defined(HAVE_CONFIG_H)
#include "magnachain-config.h"
#endif

#include "misc/amount.h"
#include "chain/branchtxdb.h"
#include "transaction/coins.h"
#include "io/fs.h"
#include "policy/feerate.h"
#include "net/protocol.h" // For MCMessageHeader::MessageStartChars
#include "script/script_error.h"
#include "thread/sync.h"
#include "transaction/txdb.h"
#include "misc/versionbits.h"
//#include "transaction/undo.h"

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include <atomic>

class MCBlockIndex;
class MCBlockTreeDB;
class MCChainParams;
class MCCoinsViewDB;
class MCInv;
class MCConnman;
class CScriptCheck;
class MCBlockPolicyEstimator;
class MCTxMemPool;
class MCTxMemPoolEntry;
class MCValidationState;
class ContractVM;
struct ChainTxData;
class BranchCache;
class ContractContext;
class CoinAmountCache;

struct VMOut;
struct PrecomputedTransactionData;
struct LockPoints;

/** Default for DEFAULT_WHITELISTRELAY. */
static const bool DEFAULT_WHITELISTRELAY = true;
/** Default for DEFAULT_WHITELISTFORCERELAY. */
static const bool DEFAULT_WHITELISTFORCERELAY = true;
/** Default for -minrelaytxfee, minimum relay fee for transactions */
static const unsigned int DEFAULT_MIN_RELAY_TX_FEE = 100000;
//! -maxtxfee default
static const MCAmount DEFAULT_TRANSACTION_MAXFEE = 100 * COIN;
//! Discourage users to set fees higher than this amount (in satoshis) per kB
static const MCAmount HIGH_TX_FEE_PER_KB = 30 * COIN;
//! -maxtxfee will warn if called with a higher fee than this amount (in satoshis)
static const MCAmount HIGH_MAX_TX_FEE = 100 * HIGH_TX_FEE_PER_KB;
/** Default for -limitancestorcount, max number of in-mempool ancestors */
static const unsigned int DEFAULT_ANCESTOR_LIMIT = 30;
/** Default for -limitancestorsize, maximum kilobytes of tx + all in-mempool ancestors */
static const unsigned int DEFAULT_ANCESTOR_SIZE_LIMIT = 101;
/** Default for -limitdescendantcount, max number of in-mempool descendants */
static const unsigned int DEFAULT_DESCENDANT_LIMIT = 30;
/** Default for -limitdescendantsize, maximum kilobytes of in-mempool descendants */
static const unsigned int DEFAULT_DESCENDANT_SIZE_LIMIT = 101;
/** Default for -mempoolexpiry, expiration time for mempool transactions in hours */
static const unsigned int DEFAULT_MEMPOOL_EXPIRY = 336;
/** Maximum kilobytes for transactions to store for processing during reorg */
static const unsigned int MAX_DISCONNECTED_TX_POOL_SIZE = 20000;
/** The maximum size of a blk?????.dat file (since 0.8) */
static const unsigned int MAX_BLOCKFILE_SIZE = 0x8000000; // 128 MiB
/** The pre-allocation chunk size for blk?????.dat files (since 0.8) */
static const unsigned int BLOCKFILE_CHUNK_SIZE = 0x1000000; // 16 MiB
/** The pre-allocation chunk size for rev?????.dat files (since 0.8) */
static const unsigned int UNDOFILE_CHUNK_SIZE = 0x100000; // 1 MiB

/** Maximum number of script-checking threads allowed */
static const int MAX_SCRIPTCHECK_THREADS = 16;
/** -par default (number of script-checking threads, 0 = auto) */
static const int DEFAULT_SCRIPTCHECK_THREADS = 0;
/** Number of blocks that can be requested at any given time from a single peer. */
static const int MAX_BLOCKS_IN_TRANSIT_PER_PEER = 16;
/** Timeout in seconds during which a peer must stall block download progress before being disconnected. */
static const unsigned int BLOCK_STALLING_TIMEOUT = 2;
/** Number of headers sent in one getheaders result. We rely on the assumption that if a peer sends
 *  less than this number, we reached its tip. Changing this value is a protocol upgrade. */
static const unsigned int MAX_HEADERS_RESULTS = 2000;
/** Maximum depth of blocks we're willing to serve as compact blocks to peers
 *  when requested. For older blocks, a regular BLOCK response will be sent. */
static const int MAX_CMPCTBLOCK_DEPTH = 5;
/** Maximum depth of blocks we're willing to respond to GETBLOCKTXN requests for. */
static const int MAX_BLOCKTXN_DEPTH = 10;
/** Size of the "block download window": how far ahead of our current height do we fetch?
 *  Larger windows tolerate larger download speed differences between peer, but increase the potential
 *  degree of disordering of blocks on disk (which make reindexing and in the future perhaps pruning
 *  harder). We'll probably want to make this a per-peer adaptive value at some point. */
static const unsigned int BLOCK_DOWNLOAD_WINDOW = 1024;
/** Time to wait (in seconds) between writing blocks/block index to disk. */
static const unsigned int DATABASE_WRITE_INTERVAL = 60 * 60;
/** Time to wait (in seconds) between flushing chainstate to disk. */
static const unsigned int DATABASE_FLUSH_INTERVAL = 24 * 60 * 60;
/** Maximum length of reject messages. */
static const unsigned int MAX_REJECT_MESSAGE_LENGTH = 111;
/** Average delay between local address broadcasts in seconds. */
static const unsigned int AVG_LOCAL_ADDRESS_BROADCAST_INTERVAL = 24 * 60 * 60;
/** Average delay between peer address broadcasts in seconds. */
static const unsigned int AVG_ADDRESS_BROADCAST_INTERVAL = 30;
/** Average delay between trickled inventory transmissions in seconds.
 *  Blocks and whitelisted receivers bypass this, outbound peers get half this delay. */
static const unsigned int INVENTORY_BROADCAST_INTERVAL = 5;
/** Maximum number of inventory items to send per transmission.
 *  Limits the impact of low-fee transaction floods. */
static const unsigned int INVENTORY_BROADCAST_MAX = 7 * INVENTORY_BROADCAST_INTERVAL;
/** Average delay between feefilter broadcasts in seconds. */
static const unsigned int AVG_FEEFILTER_BROADCAST_INTERVAL = 10 * 60;
/** Maximum feefilter broadcast delay after significant change. */
static const unsigned int MAX_FEEFILTER_CHANGE_DELAY = 5 * 60;
/** Block download timeout base, expressed in millionths of the block interval (i.e. 10 min) */
static const int64_t BLOCK_DOWNLOAD_TIMEOUT_BASE = 5000000;
/** Additional block download timeout per parallel downloading peer (i.e. 5 min) */
static const int64_t BLOCK_DOWNLOAD_TIMEOUT_PER_PEER = 500000;

static const int64_t DEFAULT_MAX_TIP_AGE = 24 * 60 * 60;
/** Maximum age of our tip in seconds for us to be considered current for fee estimation */
static const int64_t MAX_FEE_ESTIMATION_TIP_AGE = 3 * 60 * 60;

/** Default for -permitbaremultisig */
static const bool DEFAULT_PERMIT_BAREMULTISIG = true;
static const bool DEFAULT_CHECKPOINTS_ENABLED = true;
static const bool DEFAULT_TXINDEX = false;
static const unsigned int DEFAULT_BANSCORE_THRESHOLD = 100;
/** Default for -persistmempool */
static const bool DEFAULT_PERSIST_MEMPOOL = true;
/** Default for -mempoolreplacement */
static const bool DEFAULT_ENABLE_REPLACEMENT = true;
/** Default for using fee filter */
static const bool DEFAULT_FEEFILTER = true;

/** Maximum number of headers to announce when relaying blocks with headers message.*/
static const unsigned int MAX_BLOCKS_TO_ANNOUNCE = 8;

/** Maximum number of unconnecting headers announcements before DoS score */
static const int MAX_UNCONNECTING_HEADERS = 10;

static const bool DEFAULT_PEERBLOOMFILTERS = true;

/** Default for -stopatheight */
static const int DEFAULT_STOPATHEIGHT = 0;

struct BlockHasher {
    size_t operator()(const uint256& hash) const { return hash.GetCheapHash(); }
};


extern MCScript COINBASE_FLAGS;
extern MCCriticalSection cs_main;
extern MCBlockPolicyEstimator feeEstimator;
extern MCTxMemPool mempool;
typedef std::unordered_map<uint256, MCBlockIndex*, BlockHasher> BlockMap;
extern BlockMap mapBlockIndex;
extern uint64_t nLastBlockTx;
extern uint64_t nLastBlockWeight;
extern const std::string strMessageMagic;
extern CWaitableCriticalSection csBestBlock;
extern MCConditionVariable cvBlockChange;
extern std::atomic_bool fImporting;
extern bool fReindex;
extern int nScriptCheckThreads;
extern bool fTxIndex;
extern bool fIsBareMultisigStd;
extern bool fRequireStandard;
extern bool fCheckBlockIndex;
extern bool fCheckpointsEnabled;
extern size_t nCoinCacheUsage;
/** A fee rate smaller than this is considered zero fee (for relaying, mining and transaction creation) */
extern MCFeeRate minRelayTxFee;
/** Absolute maximum transaction fee (in satoshis) used by wallet and mempool (rejects high fee in sendrawtransaction) */
extern MCAmount maxTxFee;
/** If the tip is older than this (in seconds), the node is considered to be in initial block download. */
extern int64_t nMaxTipAge;
extern bool fEnableReplacement;

/** Block hash whose ancestors we will assume to have valid scripts without checking them. */
extern uint256 hashAssumeValid;

/** Minimum work we will assume exists on some valid chain. */
extern arith_uint256 nMinimumChainWork;

/** Best header we've seen so far (used for getheaders queries' starting points). */
extern MCBlockIndex* pindexBestHeader;

/** Minimum disk space required - used in CheckDiskSpace() */
static const uint64_t nMinDiskSpace = 52428800;

/** Pruning-related variables and constants */
/** True if any block files have ever been pruned. */
extern bool fHavePruned;
/** True if we're running in -prune mode. */
extern bool fPruneMode;
/** Number of MiB of block files that we're trying to stay below. */
extern uint64_t nPruneTarget;
/** Block files containing a block-height within MIN_BLOCKS_TO_KEEP of chainActive.Tip() will not be pruned. */
static const int32_t MIN_BLOCKS_TO_KEEP = 288;

static const int32_t DEFAULT_CHECKBLOCKS = 1800; // 这里改为多少秒以内，以适配主链与支链的不同出块时间
static const uint32_t DEFAULT_CHECKLEVEL = 3;

// Require that user allocate at least 550MB for block & undo files (blk???.dat and rev???.dat)
// At 1MB per block, 288 blocks = 288MB.
// Add 15% for Undo data = 331MB
// Add 20% for Orphan block rate = 397MB
// We want the low water mark after pruning to be at least 397 MB and since we prune in
// full block file chunks, we need the high water mark which triggers the prune to be
// one 128MB block file + added 15% undo data = 147MB greater for a total of 545MB
// Setting the target to > than 550MB will make it likely we can respect the target.
static const uint64_t MIN_DISK_SPACE_FOR_BLOCK_FILES = 550 * 1024 * 1024;

bool CheckTranBranchScript(uint256 branchid, const MCScript& scriptPubKey);
/** 
 * Process an incoming block. This only returns after the best known valid
 * block is made active. Note that it does not, however, guarantee that the
 * specific block passed to it has been checked for validity!
 *
 * If you want to *possibly* get feedback on whether pblock is valid, you must
 * install a MCValidationInterface (see validationinterface.h) - this will have
 * its BlockChecked method called whenever *any* block completes validation.
 *
 * Note that we guarantee that either the proof-of-work is valid on pblock, or
 * (and possibly also) BlockChecked will have been called.
 * 
 * Call without cs_main held.
 *
 * @param[in]   pblock  The block we want to process.
 * @param[in]   fForceProcessing Process this block even if unrequested; used for non-network block sources and whitelisted peers.
 * @param[out]  fNewBlock A boolean which is set to indicate if the block was first received via this call
 * @return True if state.IsValid()
 */
bool ProcessNewBlock(const MCChainParams& chainparams, std::shared_ptr<MCBlock> pblock, bool fForceProcessing, bool* fNewBlock, bool ismining = false);

/**
 * Process incoming block headers.
 *
 * Call without cs_main held.
 *
 * @param[in]  block The block headers themselves
 * @param[out] state This may be set to an Error state if any error occurred processing them
 * @param[in]  chainparams The params for the chain we want to connect to
 * @param[out] ppindex If set, the pointer will be set to point to the last new block index object for the given headers
 * @param[out] first_invalid First header that fails validation, if one exists
 */
bool ProcessNewBlockHeaders(const std::vector<MCBlockHeader>& block, MCValidationState& state, const MCChainParams& chainparams, const MCBlockIndex** ppindex = nullptr, MCBlockHeader* first_invalid = nullptr);

/** Check whether enough disk space is available for an incoming block */
bool CheckDiskSpace(uint64_t nAdditionalBytes = 0);
/** Open a block file (blk?????.dat) */
FILE* OpenBlockFile(const MCDiskBlockPos& pos, bool fReadOnly = false);
/** Translation to a filesystem path */
fs::path GetBlockPosFilename(const MCDiskBlockPos& pos, const char* prefix);
/** Import blocks from an external file */
bool LoadExternalBlockFile(const MCChainParams& chainparams, FILE* fileIn, MCDiskBlockPos* dbp = nullptr);
/** Ensures we have a genesis block in the block tree, possibly writing one to disk. */
bool LoadGenesisBlock(const MCChainParams& chainparams);
/** Load the block tree and coins database from disk,
 * initializing state if we're running with -reindex. */
bool LoadBlockIndex(const MCChainParams& chainparams);
/** Update the chain tip based on database information. */
bool LoadChainTip(const MCChainParams& chainparams);
/** Unload database information */
void UnloadBlockIndex();
/** Run an instance of the script checking thread */
void ThreadScriptCheck();
/** Check whether we are doing an initial block download (synchronizing from disk or network) */
bool IsInitialBlockDownload(int * downloadno = nullptr);
/** Retrieve a transaction (from memory pool, or from disk, if possible) */
bool GetTransaction(const uint256& hash, MCTransactionRef& tx, const Consensus::Params& params, uint256& hashBlock, bool fAllowSlow = false);
class MCOutPoint;
class Coin;
bool GetTransactionWithCoin(const MCOutPoint& outpoint, const Coin& coin, MCTransactionRef& txOut, const Consensus::Params& consensusParams, uint256& hashBlock);
bool GetTransactionWithOutpoint(const MCOutPoint& outpoint, MCTransactionRef& txOut, const Consensus::Params& consensusParams, uint256& hashBlock);

/** Find the best known block, and make it the tip of the block chain */
bool ActivateBestChain(MCValidationState& state, const MCChainParams& chainparams, std::shared_ptr<const MCBlock> pblock = std::shared_ptr<const MCBlock>());
MCAmount GetBlockSubsidy(int nHeight, const Consensus::Params& consensusParams);

/** Guess verification progress (as a fraction between 0.0=genesis and 1.0=current tip). */
double GuessVerificationProgress(const ChainTxData& data, MCBlockIndex* pindex);

/**
 *  Mark one block file as pruned.
 */
void PruneOneBlockFile(const int fileNumber);

/**
 *  Actually unlink the specified files
 */
void UnlinkPrunedFiles(const std::set<int>& setFilesToPrune);

/** Create a new block index entry for a given block hash */
MCBlockIndex* InsertBlockIndex(uint256 hash);
/** Flush all state, indexes and buffers to disk. */
void FlushStateToDisk();
/** Prune block files and flush state to disk. */
void PruneAndFlush();
/** Prune block files up to a given height */
void PruneBlockFilesManual(int nManualPruneHeight);

// int* pNMissingInputs return value
enum eMissingInputTypes
{
    eOk = 0,
    eMissingInputs = 1 << 0,
    eMissingBranchPreHeadTx = 1 << 1, 
};

/** (try to) add transaction to memory pool
 * plTxnReplaced will be appended to with all transactions replaced from mempool **/
bool AcceptToMemoryPool(MCTxMemPool& pool, MCValidationState& state, const MCTransactionRef& tx, bool fLimitFree, int* pNMissingInputs,
    std::list<MCTransactionRef>* plTxnReplaced, bool fOverrideMempoolLimit, const MCAmount nAbsurdFee, bool executeSmartContract, uint64_t order);

bool AcceptChainTransStep2ToMemoryPool(const MCChainParams& chainparams, MCTxMemPool& pool, MCValidationState& state, const MCTransactionRef& tx, 
    bool fLimitFree, int* pNMissingInputs, int64_t nAcceptTime, std::list<MCTransactionRef>* plTxnReplaced, bool fOverrideMempoolLimit, const MCAmount nAbsurdFee, uint64_t order);

/** Convert MCValidationState to a human-readable message for logging */
std::string FormatStateMessage(const MCValidationState& state);

/** Get the BIP9 state for a given deployment at the current tip. */
ThresholdState VersionBitsTipState(const Consensus::Params& params, Consensus::DeploymentPos pos);

/** Get the numerical statistics for the BIP9 state for a given deployment at the current tip. */
BIP9Stats VersionBitsTipStatistics(const Consensus::Params& params, Consensus::DeploymentPos pos);

/** Get the block height at which the BIP9 deployment switched into the state for the block building on the current tip. */
int VersionBitsTipStateSinceHeight(const Consensus::Params& params, Consensus::DeploymentPos pos);


/** Apply the effects of this transaction on the UTXO set represented by view */
void UpdateCoins(const MCTransaction& tx, MCCoinsViewCache& inputs, int nHeight, bool ismempoolchecking);

/** Transaction validation functions */

/**
 * Check if transaction will be final in the next block to be created.
 *
 * Calls IsFinalTx() with current block height and appropriate block time.
 *
 * See consensus/consensus.h for flag definitions.
 */
bool CheckFinalTx(const MCTransaction& tx, int flags = -1);

bool CheckCoinbaseTx(const MCBlock& block, MCBlockIndex* pindex, MCAmount nFees, const MCChainParams& chainparams);


/**
 * Test whether the LockPoints height and time are still valid on the current chain
 */
bool TestLockPointValidity(const LockPoints* lp);

/**
 * Check if transaction will be BIP 68 final in the next block to be created.
 *
 * Simulates calling SequenceLocks() with data from the tip of the current active chain.
 * Optionally stores in LockPoints the resulting height and time calculated and the hash
 * of the block needed for calculation or skips the calculation and uses the LockPoints
 * passed in for evaluation.
 * The LockPoints should not be considered valid if CheckSequenceLocks returns false.
 *
 * See consensus/consensus.h for flag definitions.
 */
bool CheckSequenceLocks(const MCTransaction& tx, int flags, LockPoints* lp = nullptr, bool useExistingLockPoints = false);

bool CheckContractCoins(const MCTransactionRef& tx, const VMOut* vmOut);

/**
 * Closure representing one script verification
 * Note that this stores references to the spending transaction 
 */
class CScriptCheck
{
private:
    MCScript scriptPubKey;
    MCAmount amount;
    const MCTransaction* ptxTo;
    unsigned int nIn;
    unsigned int nFlags;
    bool cacheStore;
    ScriptError error;
    PrecomputedTransactionData* txdata;

public:
    CScriptCheck() : amount(0), ptxTo(0), nIn(0), nFlags(0), cacheStore(false), error(SCRIPT_ERR_UNKNOWN_ERROR), txdata(nullptr) {}
    CScriptCheck(const MCScript& scriptPubKeyIn, const MCAmount amountIn, const MCTransaction& txToIn, unsigned int nInIn, unsigned int nFlagsIn, bool cacheIn, PrecomputedTransactionData* txdataIn) : scriptPubKey(scriptPubKeyIn), amount(amountIn),
                                                                                                                                                                                                     ptxTo(&txToIn), nIn(nInIn), nFlags(nFlagsIn), cacheStore(cacheIn), error(SCRIPT_ERR_UNKNOWN_ERROR), txdata(txdataIn) {}

    bool operator()();

    void swap(CScriptCheck& check)
    {
        scriptPubKey.swap(check.scriptPubKey);
        std::swap(ptxTo, check.ptxTo);
        std::swap(amount, check.amount);
        std::swap(nIn, check.nIn);
        std::swap(nFlags, check.nFlags);
        std::swap(cacheStore, check.cacheStore);
        std::swap(error, check.error);
        std::swap(txdata, check.txdata);
    }

    ScriptError GetScriptError() const { return error; }
};

/** Initializes the script-execution cache */
void InitScriptExecutionCache();


/** Functions for disk access for blocks */
bool ReadBlockFromDisk(MCBlock& block, const MCDiskBlockPos& pos, const Consensus::Params& consensusParams);
bool ReadBlockFromDisk(MCBlock& block, const MCBlockIndex* pindex, const Consensus::Params& consensusParams);

/** Functions for validating blocks and updating the block tree */

/** Context-independent validity checks */
bool CheckBlock(const MCBlock& block, MCValidationState& state, const Consensus::Params& consensusParams, BranchCache* pBranchCache, 
    bool fCheckPOW, bool fCheckMerkleRoot, bool fVerifingDB, MCCoinsViewCache* pCoins);
bool CheckBlockWork(const MCBlock& block, MCValidationState& state, const Consensus::Params& consensusParams);

/** Check a block is completely valid from start to finish (only works on top of our current best block, with cs_main held) */
bool TestBlockValidity(MCValidationState& state, const MCChainParams& chainparams, const MCBlock& block, MCBlockIndex* pindexPrev, bool fCheckPOW = true, bool fCheckMerkleRoot = true);

/** Check whether witness commitments are required for block. */
bool IsWitnessEnabled(const MCBlockIndex* pindexPrev, const Consensus::Params& params);

/** When there are blocks in the active chain with missing data, rewind the chainstate and remove them from the block index */
bool RewindBlockIndex(const MCChainParams& params);

/** Update uncommitted block structures (currently: only the witness nonce). This is safe for submitted blocks. */
void UpdateUncommittedBlockStructures(MCBlock& block, const MCBlockIndex* pindexPrev, const Consensus::Params& consensusParams);

/** Produce the necessary coinbase commitment for a block (modifies the hash, don't call for mined blocks). */
std::vector<unsigned char> GenerateCoinbaseCommitment(MCBlock& block, const MCBlockIndex* pindexPrev, const Consensus::Params& consensusParams);

/** RAII wrapper for VerifyDB: Verify consistency of the block and coin databases */
class CVerifyDB
{
public:
    CVerifyDB();
    ~CVerifyDB();
    bool VerifyDB(const MCChainParams& chainparams, MCCoinsView* coinsview, int nCheckLevel, int nCheckDepth);
};

/** Replay blocks that aren't fully applied to the database. */
bool ReplayBlocks(const MCChainParams& params, MCCoinsView* view);

/** Find the last common block between the parameter chain and a locator. */
MCBlockIndex* FindForkInGlobalIndex(const MCChain& chain, const MCBlockLocator& locator);

/** Mark a block as precious and reorganize. */
bool PreciousBlock(MCValidationState& state, const MCChainParams& params, MCBlockIndex* pindex);

/** Mark a block as invalid. */
bool InvalidateBlock(MCValidationState& state, const MCChainParams& chainparams, MCBlockIndex* pindex);

/** Remove invalidity status from a block and its descendants. */
bool ResetBlockFailureFlags(MCBlockIndex* pindex);

/** The currently-connected chain of blocks (protected by cs_main). */
extern MCChain chainActive;

/** Global variable that points to the coins database (protected by cs_main) */
extern MCCoinsViewDB* pcoinsdbview;

/** Global variable that points to the active MCCoinsView (protected by cs_main) */
extern MCCoinsViewCache* pcoinsTip;

/** Global variable that points to the active block tree (protected by cs_main) */
extern MCBlockTreeDB* pblocktree;

extern CoinListDB* pcoinListDb;

/**
 * Return the spend height, which is one more than the inputs.GetBestBlock().
 * While checking, GetBestBlock() refers to the parent block. (protected by cs_main)
 * This is also true for mempool checks.
 */
int GetSpendHeight(const MCCoinsViewCache& inputs);

extern VersionBitsCache versionbitscache;

/**
 * Determine what nVersion a new block should use.
 */
int32_t ComputeBlockVersion(const MCBlockIndex* pindexPrev, const Consensus::Params& params);

/** Add block to MCBlockIndex */
MCBlockIndex* AddToBlockIndex(const MCBlockHeader& block);

/** Reject codes greater or equal to this can be returned by AcceptToMemPool
 * for transactions, to signal internal conditions. They cannot and should not
 * be sent over the P2P network.
 */
static const unsigned int REJECT_INTERNAL = 0x100;
/** Too high fee. Can not be triggered by P2P transactions */
static const unsigned int REJECT_HIGHFEE = 0x100;

/** Get block file info entry for one block file */
MCBlockFileInfo* GetBlockFileInfo(size_t n);

/** Dump the mempool to disk. */
void DumpMempool();

/** Load the mempool from disk. */
bool LoadMempool();

bool ReadTxDataByTxIndex(const uint256& hash, MCTransactionRef& txOut, uint256& hashBlock, bool& retflag);

std::string GetBranchTxProof(const MCBlock& block,  const std::set<uint256>& setTxids);
//bool VerifyBranchTxProof(const uint256& branchHash, const MCBlock& block, const std::string& txProof);

bool GetProveInfo(const MCBlock& block, int blockHeight, MCBlockIndex* pPrevBlockIndex, const int txIndex, std::shared_ptr<ProveData> pProveData);
bool GetProveOfCoinbase(std::shared_ptr<ProveData>& pProveData, MCBlock& block);
#endif // MAGNACHAIN_VALIDATION_H
