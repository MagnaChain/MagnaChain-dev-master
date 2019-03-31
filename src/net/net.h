// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_NET_H
#define MAGNACHAIN_NET_H

#include "address/addrdb.h"
#include "address/addrman.h"
#include "misc/amount.h"
#include "transaction/bloom.h"
#include "net/compat.h"
#include "coding/hash.h"
#include "misc/limitedmap.h"
#include "net/netaddress.h"
#include "policy/feerate.h"
#include "net/protocol.h"
#include "misc/random.h"
#include "io/streams.h"
#include "thread/sync.h"
#include "coding/uint256.h"
#include "thread/threadinterrupt.h"

#include <atomic>
#include <deque>
#include <stdint.h>
#include <thread>
#include <memory>
#include <condition_variable>

#ifndef _WIN32
#include <arpa/inet.h>
#endif


class MCScheduler;
class MCNode;

namespace boost {
    class thread_group;
} // namespace boost

/** Time between pings automatically sent out for latency probing and keepalive (in seconds). */
static const int PING_INTERVAL = 2 * 60;
/** Time after which to disconnect, after waiting for a ping response (or inactivity). */
static const int TIMEOUT_INTERVAL = 20 * 60;
/** Run the feeler connection loop once every 2 minutes or 120 seconds. **/
static const int FEELER_INTERVAL = 120;
/** The maximum number of entries in an 'inv' protocol message */
static const unsigned int MAX_INV_SZ = 50000;
/** The maximum number of new addresses to accumulate before announcing. */
static const unsigned int MAX_ADDR_TO_SEND = 1000;
/** Maximum length of incoming protocol messages (no message over 4 MB is currently acceptable). */
static const unsigned int MAX_PROTOCOL_MESSAGE_LENGTH = 4 * 1000 * 1000;
/** Maximum length of strSubVer in `version` message */
static const unsigned int MAX_SUBVERSION_LENGTH = 256;
/** Maximum number of automatic outgoing nodes */
static const int MAX_OUTBOUND_CONNECTIONS = 8;
/** Maximum number of addnode outgoing nodes */
static const int MAX_ADDNODE_CONNECTIONS = 8;
/** -listen default */
static const bool DEFAULT_LISTEN = true;
/** -upnp default */
#ifdef USE_UPNP
static const bool DEFAULT_UPNP = true;
#else
static const bool DEFAULT_UPNP = false;
#endif
/** The maximum number of entries in mapAskFor */
static const size_t MAPASKFOR_MAX_SZ = MAX_INV_SZ;
/** The maximum number of entries in setAskFor (larger due to getdata latency)*/
static const size_t SETASKFOR_MAX_SZ = 2 * MAX_INV_SZ;
/** The maximum number of peer connections to maintain. */
static const unsigned int DEFAULT_MAX_PEER_CONNECTIONS = 125;
/** The default for -maxuploadtarget. 0 = Unlimited */
static const uint64_t DEFAULT_MAX_UPLOAD_TARGET = 0;
/** The default timeframe for -maxuploadtarget. 1 day. */
static const uint64_t MAX_UPLOAD_TIMEFRAME = 60 * 60 * 24;
/** Default for blocks only*/
static const bool DEFAULT_BLOCKSONLY = false;

static const bool DEFAULT_FORCEDNSSEED = false;
static const size_t DEFAULT_MAXRECEIVEBUFFER = 5 * 1000;
static const size_t DEFAULT_MAXSENDBUFFER    = 1 * 1000;

static const ServiceFlags REQUIRED_SERVICES = NODE_NETWORK;

// NOTE: When adjusting this, update rpcnet:setban's help ("24h")
static const unsigned int DEFAULT_MISBEHAVING_BANTIME = 60 * 60 * 24;  // Default 24-hour ban

typedef int64_t NodeId;

struct AddedNodeInfo
{
    std::string strAddedNode;
    MCService resolvedAddress;
    bool fConnected;
    bool fInbound;
};

class MCNodeStats;
class MCClientUIInterface;

struct CSerializedNetMsg
{
    CSerializedNetMsg() = default;
    CSerializedNetMsg(CSerializedNetMsg&&) = default;
    CSerializedNetMsg& operator=(CSerializedNetMsg&&) = default;
    // No copying, only moves.
    CSerializedNetMsg(const CSerializedNetMsg& msg) = delete;
    CSerializedNetMsg& operator=(const CSerializedNetMsg&) = delete;

    std::vector<unsigned char> data;
    std::string command;
};

class NetEventsInterface;
class MCConnman
{
public:

    enum NumConnections {
        CONNECTIONS_NONE = 0,
        CONNECTIONS_IN = (1U << 0),
        CONNECTIONS_OUT = (1U << 1),
        CONNECTIONS_ALL = (CONNECTIONS_IN | CONNECTIONS_OUT),
    };

    struct Options
    {
        ServiceFlags nLocalServices = NODE_NONE;
        ServiceFlags nRelevantServices = NODE_NONE;
        int nMaxConnections = 0;
        int nMaxOutbound = 0;
        int nMaxAddnode = 0;
        int nMaxFeeler = 0;
        int nBestHeight = 0;
        MCClientUIInterface* uiInterface = nullptr;
        NetEventsInterface* m_msgproc = nullptr;
        unsigned int nSendBufferMaxSize = 0;
        unsigned int nReceiveFloodSize = 0;
        uint64_t nMaxOutboundTimeframe = 0;
        uint64_t nMaxOutboundLimit = 0;
        std::vector<std::string> vSeedNodes;
        std::vector<CSubNet> vWhitelistedRange;
        std::vector<MCService> vBinds, vWhiteBinds;
    };

    void Init(const Options& connOptions) {
        nLocalServices = connOptions.nLocalServices;
        nRelevantServices = connOptions.nRelevantServices;
        nMaxConnections = connOptions.nMaxConnections;
        nMaxOutbound = std::min(connOptions.nMaxOutbound, connOptions.nMaxConnections);
        nMaxAddnode = connOptions.nMaxAddnode;
        nMaxFeeler = connOptions.nMaxFeeler;
        nBestHeight = connOptions.nBestHeight;
        clientInterface = connOptions.uiInterface;
        m_msgproc = connOptions.m_msgproc;
        nSendBufferMaxSize = connOptions.nSendBufferMaxSize;
        nReceiveFloodSize = connOptions.nReceiveFloodSize;
        nMaxOutboundTimeframe = connOptions.nMaxOutboundTimeframe;
        nMaxOutboundLimit = connOptions.nMaxOutboundLimit;
        vWhitelistedRange = connOptions.vWhitelistedRange;
    }

    MCConnman(uint64_t seed0, uint64_t seed1);
    ~MCConnman();
    bool Start(MCScheduler& scheduler, const Options& options);
    void Stop();
    void Interrupt();
    bool GetNetworkActive() const { return fNetworkActive; };
    void SetNetworkActive(bool active);
    bool OpenNetworkConnection(const MCAddress& addrConnect, bool fCountFailure, CSemaphoreGrant *grantOutbound = nullptr, const char *strDest = nullptr, bool fOneShot = false, bool fFeeler = false, bool manual_connection = false);
    bool CheckIncomingNonce(uint64_t nonce);

    bool ForNode(NodeId id, std::function<bool(MCNode* pnode)> func);

    void PushMessage(MCNode* pnode, CSerializedNetMsg&& msg);

    template<typename Callable>
    void ForEachNode(Callable&& func)
    {
        LOCK(cs_vNodes);
        for (auto&& node : vNodes) {
            if (NodeFullyConnected(node))
                func(node);
        }
    };

    template<typename Callable>
    void ForEachNode(Callable&& func) const
    {
        LOCK(cs_vNodes);
        for (auto&& node : vNodes) {
            if (NodeFullyConnected(node))
                func(node);
        }
    };

    template<typename Callable, typename CallableAfter>
    void ForEachNodeThen(Callable&& pre, CallableAfter&& post)
    {
        LOCK(cs_vNodes);
        for (auto&& node : vNodes) {
            if (NodeFullyConnected(node))
                pre(node);
        }
        post();
    };

    template<typename Callable, typename CallableAfter>
    void ForEachNodeThen(Callable&& pre, CallableAfter&& post) const
    {
        LOCK(cs_vNodes);
        for (auto&& node : vNodes) {
            if (NodeFullyConnected(node))
                pre(node);
        }
        post();
    };

    // Addrman functions
    size_t GetAddressCount() const;
    void SetServices(const MCService &addr, ServiceFlags nServices);
    void MarkAddressGood(const MCAddress& addr);
    void AddNewAddresses(const std::vector<MCAddress>& vAddr, const MCAddress& addrFrom, int64_t nTimePenalty = 0);
    std::vector<MCAddress> GetAddresses();

    // Denial-of-service detection/prevention
    // The idea is to detect peers that are behaving
    // badly and disconnect/ban them, but do it in a
    // one-coding-mistake-won't-shatter-the-entire-network
    // way.
    // IMPORTANT:  There should be nothing I can give a
    // node that it will forward on that will make that
    // node's peers drop it. If there is, an attacker
    // can isolate a node and/or try to split the network.
    // Dropping a node for sending stuff that is invalid
    // now but might be valid in a later version is also
    // dangerous, because it can cause a network split
    // between nodes running old code and nodes running
    // new code.
    void Ban(const MCNetAddr& netAddr, const BanReason& reason, int64_t bantimeoffset = 0, bool sinceUnixEpoch = false);
    void Ban(const CSubNet& subNet, const BanReason& reason, int64_t bantimeoffset = 0, bool sinceUnixEpoch = false);
    void ClearBanned(); // needed for unit testing
    bool IsBanned(MCNetAddr ip);
    bool IsBanned(CSubNet subnet);
    bool Unban(const MCNetAddr &ip);
    bool Unban(const CSubNet &ip);
    void GetBanned(banmap_t &banmap);
    void SetBanned(const banmap_t &banmap);

    // This allows temporarily exceeding nMaxOutbound, with the goal of finding
    // a peer that is better than all our current peers.
    void SetTryNewOutboundPeer(bool flag);
    bool GetTryNewOutboundPeer();

    // Return the number of outbound peers we have in excess of our target (eg,
    // if we previously called SetTryNewOutboundPeer(true), and have since set
    // to false, we may have extra peers that we wish to disconnect). This may
    // return a value less than (num_outbound_connections - num_outbound_slots)
    // in cases where some outbound connections are not yet fully connected, or
    // not yet fully disconnected.
    int GetExtraOutboundCount();

    bool AddNode(const std::string& node);
    bool RemoveAddedNode(const std::string& node);
    std::vector<AddedNodeInfo> GetAddedNodeInfo();

    size_t GetNodeCount(NumConnections num);
    void GetNodeStats(std::vector<MCNodeStats>& vstats);
    bool DisconnectNode(const std::string& node);
    bool DisconnectNode(NodeId id);

    ServiceFlags GetLocalServices() const;

    //!set the max outbound target in bytes
    void SetMaxOutboundTarget(uint64_t limit);
    uint64_t GetMaxOutboundTarget();

    //!set the timeframe for the max outbound target
    void SetMaxOutboundTimeframe(uint64_t timeframe);
    uint64_t GetMaxOutboundTimeframe();

    //!check if the outbound target is reached
    // if param historicalBlockServingLimit is set true, the function will
    // response true if the limit for serving historical blocks has been reached
    bool OutboundTargetReached(bool historicalBlockServingLimit);

    //!response the bytes left in the current max outbound cycle
    // in case of no limit, it will always response 0
    uint64_t GetOutboundTargetBytesLeft();

    //!response the time in second left in the current max outbound cycle
    // in case of no limit, it will always response 0
    uint64_t GetMaxOutboundTimeLeftInCycle();

    uint64_t GetTotalBytesRecv();
    uint64_t GetTotalBytesSent();

    void SetBestHeight(int height);
    int GetBestHeight() const;

    /** Get a unique deterministic randomizer. */
    CSipHasher GetDeterministicRandomizer(uint64_t id) const;

    unsigned int GetReceiveFloodSize() const;

    void WakeMessageHandler();
private:
    struct ListenSocket {
        SOCKET socket;
        bool whitelisted;

        ListenSocket(SOCKET socket_, bool whitelisted_) : socket(socket_), whitelisted(whitelisted_) {}
    };

    bool BindListenPort(const MCService &bindAddr, std::string& strError, bool fWhitelisted = false);
    bool Bind(const MCService &addr, unsigned int flags);
    bool InitBinds(const std::vector<MCService>& binds, const std::vector<MCService>& whiteBinds);
    void ThreadOpenAddedConnections();
    void AddOneShot(const std::string& strDest);
    void ProcessOneShot();
    void ThreadOpenConnections();
    void ThreadMessageHandler();
    void AcceptConnection(const ListenSocket& hListenSocket);
    void ThreadSocketHandler();
    void ThreadDNSAddressSeed();

    uint64_t CalculateKeyedNetGroup(const MCAddress& ad) const;

    MCNode* FindNode(const MCNetAddr& ip);
    MCNode* FindNode(const CSubNet& subNet);
    MCNode* FindNode(const std::string& addrName);
    MCNode* FindNode(const MCService& addr);

    bool AttemptToEvictConnection();
    MCNode* ConnectNode(MCAddress addrConnect, const char *pszDest, bool fCountFailure);
    bool IsWhitelistedRange(const MCNetAddr &addr);

    void DeleteNode(MCNode* pnode);

    NodeId GetNewNodeId();

    size_t SocketSendData(MCNode *pnode) const;
    //!check is the banlist has unwritten changes
    bool BannedSetIsDirty();
    //!set the "dirty" flag for the banlist
    void SetBannedSetDirty(bool dirty=true);
    //!clean unused entries (if bantime has expired)
    void SweepBanned();
    void DumpAddresses();
    void DumpData();
    void DumpBanlist();

    // Network stats
    void RecordBytesRecv(uint64_t bytes);
    void RecordBytesSent(uint64_t bytes);

    // Whether the node should be passed out in ForEach* callbacks
    static bool NodeFullyConnected(const MCNode* pnode);

    // Network usage totals
    MCCriticalSection cs_totalBytesRecv;
    MCCriticalSection cs_totalBytesSent;
    uint64_t nTotalBytesRecv;
    uint64_t nTotalBytesSent;

    // outbound limit & stats
    uint64_t nMaxOutboundTotalBytesSentInCycle;
    uint64_t nMaxOutboundCycleStartTime;
    uint64_t nMaxOutboundLimit;
    uint64_t nMaxOutboundTimeframe;

    // Whitelisted ranges. Any node connecting from these is automatically
    // whitelisted (as well as those connecting to whitelisted binds).
    std::vector<CSubNet> vWhitelistedRange;

    unsigned int nSendBufferMaxSize;
    unsigned int nReceiveFloodSize;

    std::vector<ListenSocket> vhListenSocket;
    std::atomic<bool> fNetworkActive;
    banmap_t setBanned;
    MCCriticalSection cs_setBanned;
    bool setBannedIsDirty;
    bool fAddressesInitialized;
    MCAddrMan addrman;
    std::deque<std::string> vOneShots;
    MCCriticalSection cs_vOneShots;
    std::vector<std::string> vAddedNodes;
    MCCriticalSection cs_vAddedNodes;
    std::vector<MCNode*> vNodes;
    std::list<MCNode*> vNodesDisconnected;
    mutable MCCriticalSection cs_vNodes;
    std::atomic<NodeId> nLastNodeId;

    /** Services this instance offers */
    ServiceFlags nLocalServices;

    /** Services this instance cares about */
    ServiceFlags nRelevantServices;

    CSemaphore *semOutbound;
    CSemaphore *semAddnode;
    int nMaxConnections;
    int nMaxOutbound;
    int nMaxAddnode;
    int nMaxFeeler;
    std::atomic<int> nBestHeight;
    MCClientUIInterface* clientInterface;
    NetEventsInterface* m_msgproc;

    /** SipHasher seeds for deterministic randomness */
    const uint64_t nSeed0, nSeed1;

    /** flag for waking the message processor. */
    bool fMsgProcWake;

    std::condition_variable condMsgProc;
    std::mutex mutexMsgProc;
    std::atomic<bool> flagInterruptMsgProc;

    CThreadInterrupt interruptNet;

    std::thread threadDNSAddressSeed;
    std::thread threadSocketHandler;
    std::thread threadOpenAddedConnections;
    std::thread threadOpenConnections;
    std::thread threadMessageHandler;

    /** flag for deciding to connect to an extra outbound peer,
     *  in excess of nMaxOutbound
     *  This takes the place of a feeler connection */
    std::atomic_bool m_try_another_outbound_peer;

    friend struct MCConnmanTest;
};
extern std::unique_ptr<MCConnman> g_connman;
void Discover(boost::thread_group& threadGroup);
void MapPort(bool fUseUPnP);
unsigned short GetListenPort();

struct CombinerAll
{
    typedef bool result_type;

    template<typename I>
    bool operator()(I first, I last) const
    {
        while (first != last) {
            if (!(*first)) return false;
            ++first;
        }
        return true;
    }
};

/**
 * Interface for message handling
 */
class NetEventsInterface
{
public:
    virtual bool ProcessMessages(MCNode* pnode, std::atomic<bool>& interrupt) = 0;
    virtual bool SendMessages(MCNode* pnode, std::atomic<bool>& interrupt) = 0;
    virtual void InitializeNode(MCNode* pnode) = 0;
    virtual void FinalizeNode(NodeId id, bool& update_connection_time) = 0;
};

enum
{
    LOCAL_NONE,   // unknown
    LOCAL_IF,     // address a local interface listens on
    LOCAL_BIND,   // address explicit bound to
    LOCAL_UPNP,   // address reported by UPnP
    LOCAL_MANUAL, // address explicitly specified (-externalip=)

    LOCAL_MAX
};

bool IsPeerAddrLocalGood(MCNode *pnode);
void AdvertiseLocal(MCNode *pnode);
void SetLimited(enum Network net, bool fLimited = true);
bool IsLimited(enum Network net);
bool IsLimited(const MCNetAddr& addr);
bool AddLocal(const MCService& addr, int nScore = LOCAL_NONE);
bool AddLocal(const MCNetAddr& addr, int nScore = LOCAL_NONE);
bool RemoveLocal(const MCService& addr);
bool SeenLocal(const MCService& addr);
bool IsLocal(const MCService& addr);
bool GetLocal(MCService &addr, const MCNetAddr *paddrPeer = nullptr);
bool IsReachable(enum Network net);
bool IsReachable(const MCNetAddr &addr);
MCAddress GetLocalAddress(const MCNetAddr *paddrPeer, ServiceFlags nLocalServices);


extern bool fDiscover;
extern bool fListen;
extern bool fRelayTxes;

extern limitedmap<uint256, int64_t> mapAlreadyAskedFor;

/** Subversion as sent to the P2P network in `version` messages */
extern std::string strSubVersion;

struct LocalServiceInfo {
    int nScore;
    int nPort;
};

extern MCCriticalSection cs_mapLocalHost;
extern std::map<MCNetAddr, LocalServiceInfo> mapLocalHost;
typedef std::map<std::string, uint64_t> mapMsgCmdSize; //command, total bytes

class MCNodeStats
{
public:
    NodeId nodeid;
    ServiceFlags nServices;
    bool fRelayTxes;
    int64_t nLastSend;
    int64_t nLastRecv;
    int64_t nTimeConnected;
    int64_t nTimeOffset;
    std::string addrName;
    int nVersion;
    std::string cleanSubVer;
    bool fInbound;
    bool m_manual_connection;
    int nStartingHeight;
    uint64_t nSendBytes;
    mapMsgCmdSize mapSendBytesPerMsgCmd;
    uint64_t nRecvBytes;
    mapMsgCmdSize mapRecvBytesPerMsgCmd;
    bool fWhitelisted;
    double dPingTime;
    double dPingWait;
    double dMinPing;
    // Our address, as reported by the peer
    std::string addrLocal;
    // Address of this peer
    MCAddress addr;
    // Bind address of our side of the connection
    MCAddress addrBind;
};




class CNetMessage {
private:
    mutable CHash256 hasher;
    mutable uint256 data_hash;
public:
    bool in_data;                   // parsing header (false) or data (true)

    MCDataStream hdrbuf;             // partially received header
    MCMessageHeader hdr;             // complete header
    unsigned int nHdrPos;

    MCDataStream vRecv;              // received message data
    unsigned int nDataPos;

    int64_t nTime;                  // time (in microseconds) of message receipt.

    CNetMessage(const MCMessageHeader::MessageStartChars& pchMessageStartIn, int nTypeIn, int nVersionIn) : hdrbuf(nTypeIn, nVersionIn), hdr(pchMessageStartIn), vRecv(nTypeIn, nVersionIn) {
        hdrbuf.resize(24);
        in_data = false;
        nHdrPos = 0;
        nDataPos = 0;
        nTime = 0;
    }

    bool complete() const
    {
        if (!in_data)
            return false;
        return (hdr.nMessageSize == nDataPos);
    }

    const uint256& GetMessageHash() const;

    void SetVersion(int nVersionIn)
    {
        hdrbuf.SetVersion(nVersionIn);
        vRecv.SetVersion(nVersionIn);
    }

    int readHeader(const char *pch, unsigned int nBytes);
    int readData(const char *pch, unsigned int nBytes);
};


/** Information about a peer */
class MCNode
{
    friend class MCConnman;
public:
    // socket
    std::atomic<ServiceFlags> nServices;
    ServiceFlags nServicesExpected;
    SOCKET hSocket;
    size_t nSendSize; // total size of all vSendMsg entries
    size_t nSendOffset; // offset inside the first vSendMsg already sent
    uint64_t nSendBytes;
    std::deque<std::vector<unsigned char>> vSendMsg;
    MCCriticalSection cs_vSend;
    MCCriticalSection cs_hSocket;
    MCCriticalSection cs_vRecv;

    MCCriticalSection cs_vProcessMsg;
    std::list<CNetMessage> vProcessMsg;
    size_t nProcessQueueSize;

    MCCriticalSection cs_sendProcessing;

    std::deque<MCInv> vRecvGetData;
    uint64_t nRecvBytes;
    std::atomic<int> nRecvVersion;

    std::atomic<int64_t> nLastSend;
    std::atomic<int64_t> nLastRecv;
    const int64_t nTimeConnected;
    std::atomic<int64_t> nTimeOffset;
    // Address of this peer
    const MCAddress addr;
    // Bind address of our side of the connection
    const MCAddress addrBind;
    std::atomic<int> nVersion;
    // strSubVer is whatever byte array we read from the wire. However, this field is intended
    // to be printed out, displayed to humans in various forms and so on. So we sanitize it and
    // store the sanitized version in cleanSubVer. The original should be used when dealing with
    // the network or wire types and the cleaned string used when displayed or logged.
    std::string strSubVer, cleanSubVer;
    MCCriticalSection cs_SubVer; // used for both cleanSubVer and strSubVer
    bool fWhitelisted; // This peer can bypass DoS banning.
    bool fFeeler; // If true this node is being used as a short lived feeler.
    bool fOneShot;
    bool m_manual_connection;
    bool fClient;
    const bool fInbound;
    std::atomic_bool fSuccessfullyConnected;
    std::atomic_bool fDisconnect;
    // We use fRelayTxes for two purposes -
    // a) it allows us to not relay tx invs before receiving the peer's version message
    // b) the peer may tell us in its version message that we should not relay tx invs
    //    unless it loads a bloom filter.
    bool fRelayTxes; //protected by cs_filter
    bool fSentAddr;
    CSemaphoreGrant grantOutbound;
    MCCriticalSection cs_filter;
    MCBloomFilter* pfilter;
    std::atomic<int> nRefCount;

    const uint64_t nKeyedNetGroup;
    std::atomic_bool fPauseRecv;
    std::atomic_bool fPauseSend;
protected:

    mapMsgCmdSize mapSendBytesPerMsgCmd;
    mapMsgCmdSize mapRecvBytesPerMsgCmd;

public:
    uint256 hashContinue;
    std::atomic<int> nStartingHeight;

    // flood relay
    std::vector<MCAddress> vAddrToSend;
    MCRollingBloomFilter addrKnown;
    bool fGetAddr;
    std::set<uint256> setKnown;
    int64_t nNextAddrSend;
    int64_t nNextLocalAddrSend;

    // inventory based relay
    MCRollingBloomFilter filterInventoryKnown;
    // Set of transaction ids we still have to announce.
    // They are sorted by the mempool before relay, so the order is not important.
    std::set<uint256> setInventoryTxToSend;
    // List of block ids we still have announce.
    // There is no final sorting before sending, as they are always sent immediately
    // and in the order requested.
    std::vector<uint256> vInventoryBlockToSend;
    MCCriticalSection cs_inventory;
    std::set<uint256> setAskFor;
    std::multimap<int64_t, MCInv> mapAskFor;
    int64_t nNextInvSend;
    // Used for headers announcements - unfiltered blocks to relay
    // Also protected by cs_inventory
    std::vector<uint256> vBlockHashesToAnnounce;
    // Used for BIP35 mempool sending, also protected by cs_inventory
    bool fSendMempool;

    // Last time a "MEMPOOL" request was serviced.
    std::atomic<int64_t> timeLastMempoolReq;

    // Block and TXN accept times
    std::atomic<int64_t> nLastBlockTime;
    std::atomic<int64_t> nLastTXTime;

    // Ping time measurement:
    // The pong reply we're expecting, or 0 if no pong expected.
    std::atomic<uint64_t> nPingNonceSent;
    // Time (in usec) the last ping was sent, or 0 if no ping was ever sent.
    std::atomic<int64_t> nPingUsecStart;
    // Last measured round-trip time.
    std::atomic<int64_t> nPingUsecTime;
    // Best measured round-trip time.
    std::atomic<int64_t> nMinPingUsecTime;
    // Whether a ping is requested.
    std::atomic<bool> fPingQueued;
    // Minimum fee rate with which to filter inv's to this node
    MCAmount minFeeFilter;
    MCCriticalSection cs_feeFilter;
    MCAmount lastSentFeeFilter;
    int64_t nextSendTimeFeeFilter;

    MCNode(NodeId id, ServiceFlags nLocalServicesIn, int nMyStartingHeightIn, SOCKET hSocketIn, const MCAddress &addrIn, uint64_t nKeyedNetGroupIn, uint64_t nLocalHostNonceIn, const MCAddress &addrBindIn, const std::string &addrNameIn = "", bool fInboundIn = false);
    ~MCNode();

private:
    MCNode(const MCNode&);
    void operator=(const MCNode&);
    const NodeId id;


    const uint64_t nLocalHostNonce;
    // Services offered to this peer
    const ServiceFlags nLocalServices;
    const int nMyStartingHeight;
    int nSendVersion;
    std::list<CNetMessage> vRecvMsg;  // Used only by SocketHandler thread

    mutable MCCriticalSection cs_addrName;
    std::string addrName;

    // Our address, as reported by the peer
    MCService addrLocal;
    mutable MCCriticalSection cs_addrLocal;
public:

    NodeId GetId() const {
        return id;
    }

    uint64_t GetLocalNonce() const {
        return nLocalHostNonce;
    }

    int GetMyStartingHeight() const {
        return nMyStartingHeight;
    }

    int GetRefCount() const
    {
        assert(nRefCount >= 0);
        return nRefCount;
    }

    bool ReceiveMsgBytes(const char *pch, unsigned int nBytes, bool& complete);

    void SetRecvVersion(int nVersionIn)
    {
        nRecvVersion = nVersionIn;
    }
    int GetRecvVersion() const
    {
        return nRecvVersion;
    }
    void SetSendVersion(int nVersionIn);
    int GetSendVersion() const;

    MCService GetAddrLocal() const;
    //! May not be called more than once
    void SetAddrLocal(const MCService& addrLocalIn);

    MCNode* AddRef()
    {
        nRefCount++;
        return this;
    }

    void Release()
    {
        nRefCount--;
    }



    void AddAddressKnown(const MCAddress& _addr)
    {
        addrKnown.insert(_addr.GetKey());
    }

    void PushAddress(const MCAddress& _addr, FastRandomContext &insecure_rand)
    {
        // Known checking here is only to save space from duplicates.
        // SendMessages will filter it again for knowns that were added
        // after addresses were pushed.
        if (_addr.IsValid() && !addrKnown.contains(_addr.GetKey())) {
            if (vAddrToSend.size() >= MAX_ADDR_TO_SEND) {
                vAddrToSend[insecure_rand.randrange(vAddrToSend.size())] = _addr;
            } else {
                vAddrToSend.push_back(_addr);
            }
        }
    }


    void AddInventoryKnown(const MCInv& inv)
    {
        {
            LOCK(cs_inventory);
            filterInventoryKnown.insert(inv.hash);
        }
    }

    void PushInventory(const MCInv& inv)
    {
        LOCK(cs_inventory);
        if (inv.type == MSG_TX) {
            if (!filterInventoryKnown.contains(inv.hash)) {
                setInventoryTxToSend.insert(inv.hash);
            }
        } else if (inv.type == MSG_BLOCK) {
            vInventoryBlockToSend.push_back(inv.hash);
        }
    }

    void PushBlockHash(const uint256 &hash)
    {
        LOCK(cs_inventory);
        vBlockHashesToAnnounce.push_back(hash);
    }

    void AskFor(const MCInv& inv);

    void CloseSocketDisconnect();

    void copyStats(MCNodeStats &stats);

    ServiceFlags GetLocalServices() const
    {
        return nLocalServices;
    }

    std::string GetAddrName() const;
    //! Sets the addrName only if it was not previously set
    void MaybeSetAddrName(const std::string& addrNameIn);
};





/** Return a timestamp in the future (in microseconds) for exponentially distributed events. */
int64_t PoissonNextSend(int64_t nNow, int average_interval_seconds);

#endif // MAGNACHAIN_NET_H
