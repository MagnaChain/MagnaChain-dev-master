// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_NET_PROCESSING_H
#define MAGNACHAIN_NET_PROCESSING_H

#include "consensus/params.h"
#include "net/net.h"
#include "transaction/blockencodings.h"
#include "validation/validationinterface.h"

/** Default for -maxorphantx, maximum number of orphan transactions kept in memory */
static const unsigned int DEFAULT_MAX_ORPHAN_TRANSACTIONS = 100;
/** Expiration time for orphan transactions in seconds */
static const int64_t ORPHAN_TX_EXPIRE_TIME = 20 * 60;
/** Minimum time between orphan transactions expire time checks in seconds */
static const int64_t ORPHAN_TX_EXPIRE_INTERVAL = 5 * 60;
/** Default number of orphan+recently-replaced txn to keep around for block reconstruction */
static const unsigned int DEFAULT_BLOCK_RECONSTRUCTION_EXTRA_TXN = 100;
/** Headers download timeout expressed in microseconds
 *  Timeout = base + per_header * (expected number of headers) */
static constexpr int64_t HEADERS_DOWNLOAD_TIMEOUT_BASE = 15 * 60 * 1000000; // 15 minutes
static constexpr int64_t HEADERS_DOWNLOAD_TIMEOUT_PER_HEADER = 1000; // 1ms/header
/** Protect at least this many outbound peers from disconnection due to slow/
 * behind headers chain.
 */
static constexpr int32_t MAX_OUTBOUND_PEERS_TO_PROTECT_FROM_DISCONNECT = 4;
/** Timeout for (unprotected) outbound peers to sync to our chainwork, in seconds */
static constexpr int64_t CHAIN_SYNC_TIMEOUT = 20 * 60; // 20 minutes
/** How frequently to check for stale tips, in seconds */
static constexpr int64_t STALE_CHECK_INTERVAL = 10 * 60; // 10 minutes
/** How frequently to check for extra outbound peers and disconnect, in seconds */
static constexpr int64_t EXTRA_PEER_CHECK_INTERVAL = 45;
/** Minimum time an outbound-peer-eviction candidate must be connected for, in order to evict, in seconds */
static constexpr int64_t MINIMUM_CONNECT_TIME = 30;


/** Blocks that are in flight, and that are in the queue to be downloaded. Protected by cs_main. */
struct QueuedBlock {
    uint256 hash;
    const MCBlockIndex* pindex;                               //!< Optional.
    bool fValidatedHeaders;                                  //!< Whether this block has validated headers at the time of request.
    std::unique_ptr<PartiallyDownloadedBlock> partialBlock;  //!< Optional, used for CMPCTBLOCK downloads
};

struct MCBlockReject {
    unsigned char chRejectCode;
    std::string strRejectReason;
    uint256 hashBlock;
};

/**
* Maintain validation-specific state about nodes, protected by cs_main, instead
* by MCNode's own locks. This simplifies asynchronous operation, where
* processing of incoming data is done after the ProcessMessage call returns,
* and we're no longer holding the node's locks.
*/
struct MCNodeState {
    //! The peer's address
    const MCService address;
    //! Whether we have a fully established connection.
    bool fCurrentlyConnected;
    //! Accumulated misbehaviour score for this peer.
    int nMisbehavior;
    //! Whether this peer should be disconnected and banned (unless whitelisted).
    bool fShouldBan;
    //! String name of this peer (debugging/logging purposes).
    const std::string name;
    //! List of asynchronously-determined block rejections to notify this peer about.
    std::vector<MCBlockReject> rejects;
    //! The best known block we know this peer has announced.
    const MCBlockIndex *pindexBestKnownBlock;
    //! The hash of the last unknown block this peer has announced.
    uint256 hashLastUnknownBlock;
    //! The last full block we both have.
    const MCBlockIndex *pindexLastCommonBlock;
    //! The best header we have sent our peer.
    const MCBlockIndex *pindexBestHeaderSent;
    //! Length of current-streak of unconnecting headers announcements
    int nUnconnectingHeaders;
    //! Whether we've started headers synchronization with this peer.
    bool fSyncStarted;
    //! When to potentially disconnect peer for stalling headers download
    int64_t nHeadersSyncTimeout;
    //! Since when we're stalling block download progress (in microseconds), or 0.
    int64_t nStallingSince;
    std::list<QueuedBlock> vBlocksInFlight;
    //! When the first entry in vBlocksInFlight started downloading. Don't care when vBlocksInFlight is empty.
    int64_t nDownloadingSince;
    int nBlocksInFlight;
    int nBlocksInFlightValidHeaders;
    //! Whether we consider this a preferred download peer.
    bool fPreferredDownload;
    //! Whether this peer wants invs or headers (when possible) for block announcements.
    bool fPreferHeaders;
    //! Whether this peer wants invs or cmpctblocks (when possible) for block announcements.
    bool fPreferHeaderAndIDs;
    /**
    * Whether this peer will send us cmpctblocks if we request them.
    * This is not used to gate request logic, as we really only care about fSupportsDesiredCmpctVersion,
    * but is used as a flag to "lock in" the version of compact blocks (fWantsCmpctWitness) we send.
    */
    bool fProvidesHeaderAndIDs;
    //! Whether this peer can give us witnesses
    bool fHaveWitness;
    //! Whether this peer wants witnesses in cmpctblocks/blocktxns
    bool fWantsCmpctWitness;
    /**
    * If we've announced NODE_WITNESS to this peer: whether the peer sends witnesses in cmpctblocks/blocktxns,
    * otherwise: whether this peer sends non-witnesses in cmpctblocks/blocktxns.
    */
    bool fSupportsDesiredCmpctVersion;

    /** State used to enforce CHAIN_SYNC_TIMEOUT
    * Only in effect for outbound, non-manual connections, with
    * m_protect == false
    * Algorithm: if a peer's best known block has less work than our tip,
    * set a timeout CHAIN_SYNC_TIMEOUT seconds in the future:
    *   - If at timeout their best known block now has more work than our tip
    *     when the timeout was set, then either reset the timeout or clear it
    *     (after comparing against our current tip's work)
    *   - If at timeout their best known block still has less work than our
    *     tip did when the timeout was set, then send a getheaders message,
    *     and set a shorter timeout, HEADERS_RESPONSE_TIME seconds in future.
    *     If their best known block is still behind when that new timeout is
    *     reached, disconnect.
    */
    struct ChainSyncTimeoutState {
        //! A timeout used for checking whether our peer has sufficiently synced
        int64_t m_timeout;
        //! A header with the work we require on our peer's chain
        const MCBlockIndex * m_work_header;
        //! After timeout is reached, set to true after sending getheaders
        bool m_sent_getheaders;
        //! Whether this peer is protected from disconnection due to a bad/slow chain
        bool m_protect;
    };

    ChainSyncTimeoutState m_chain_sync;

    //! Time of last new block announcement
    int64_t m_last_block_announcement;

    MCNodeState(MCAddress addrIn, std::string addrNameIn) : address(addrIn), name(addrNameIn) {
        fCurrentlyConnected = false;
        nMisbehavior = 0;
        fShouldBan = false;
        pindexBestKnownBlock = nullptr;
        hashLastUnknownBlock.SetNull();
        pindexLastCommonBlock = nullptr;
        pindexBestHeaderSent = nullptr;
        nUnconnectingHeaders = 0;
        fSyncStarted = false;
        nHeadersSyncTimeout = 0;
        nStallingSince = 0;
        nDownloadingSince = 0;
        nBlocksInFlight = 0;
        nBlocksInFlightValidHeaders = 0;
        fPreferredDownload = false;
        fPreferHeaders = false;
        fPreferHeaderAndIDs = false;
        fProvidesHeaderAndIDs = false;
        fHaveWitness = false;
        fWantsCmpctWitness = false;
        fSupportsDesiredCmpctVersion = false;
        m_chain_sync = { 0, nullptr, false, false };
        m_last_block_announcement = 0;
    }
};

MCNodeState *State(NodeId pnode);
void UpdatePreferredDownload(MCNode* node, MCNodeState* state);
void PushNodeVersion(MCNode *pnode, MCConnman* connman, int64_t nTime);
uint32_t GetFetchFlags(MCNode* pfrom);

class MCChainParams;
typedef bool (*ProcessMessageFunc)(MCNode*, const std::string&, MCDataStream&, int64_t, const MCChainParams&, MCConnman*, const std::atomic<bool>&);
bool ProcessMessage(MCNode* pfrom, const std::string& strCommand, MCDataStream& vRecv, int64_t nTimeReceived, const MCChainParams& chainparams, MCConnman* connman, const std::atomic<bool>& interruptMsgProc);
typedef MCBlockLocator (*GetLocatorFunc)(const MCBlockIndex *pindex);
MCBlockLocator GetLocator(const MCBlockIndex *pindex);

class PeerLogicValidation : public MCValidationInterface, public NetEventsInterface {
private:
    MCConnman* const connman;
    ProcessMessageFunc processMessageFunc;
    GetLocatorFunc getLocatorFunc;

public:
    explicit PeerLogicValidation(MCConnman* connman, MCScheduler &scheduler, ProcessMessageFunc processMessageFunc, GetLocatorFunc getLocatorFunc);

    void BlockConnected(const std::shared_ptr<const MCBlock>& pblock, const MCBlockIndex* pindexConnected, const std::vector<MCTransactionRef>& vtxConflicted) override;
    void UpdatedBlockTip(const MCBlockIndex *pindexNew, const MCBlockIndex *pindexFork, bool fInitialDownload) override;
    void BlockChecked(const MCBlock& block, const MCValidationState& state) override;
    void NewPoWValidBlock(const MCBlockIndex *pindex, const std::shared_ptr<const MCBlock>& pblock) override;


    void InitializeNode(MCNode* pnode) override;
    void FinalizeNode(NodeId nodeid, bool& fUpdateConnectionTime) override;
    /** Process protocol messages received from a given node */
    bool ProcessMessages(MCNode* pfrom, std::atomic<bool>& interrupt) override;
    /**
    * Send queued protocol messages to be sent to a give node.
    *
    * @param[in]   pto             The node which we are sending messages to.
    * @param[in]   interrupt       Interrupt condition for processing threads
    * @return                      True if there is more work to be done
    */
    bool SendMessages(MCNode* pto, std::atomic<bool>& interrupt) override;

    void ConsiderEviction(MCNode *pto, int64_t time_in_seconds);
    void CheckForStaleTipAndEvictPeers(const Consensus::Params &consensusParams);
    void EvictExtraOutboundPeers(int64_t time_in_seconds);

private:
    int64_t m_stale_tip_check_time; //! Next time to check for stale tip
};

struct CNodeStateStats {
    int nMisbehavior;
    int nSyncHeight;
    int nCommonHeight;
    std::vector<int> vHeightInFlight;
};

/** Get statistics from node state */
bool GetNodeStateStats(NodeId nodeid, CNodeStateStats &stats);
/** Increase a node's misbehavior score. */
void Misbehaving(NodeId nodeid, int howmuch);

#endif // MAGNACHAIN_NET_PROCESSING_H
