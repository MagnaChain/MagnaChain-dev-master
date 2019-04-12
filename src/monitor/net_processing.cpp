// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "net/net_processing.h"
#include "monitor/net_processing.h"
#include "monitor/database.h"

#include "address/addrman.h"
#include "coding/arith_uint256.h"
#include "transaction/blockencodings.h"
#include "chain/chainparams.h"
#include "consensus/validation.h"
#include "coding/hash.h"
#include "init.h"
#include "validation/validation.h"
#include "transaction/merkleblock.h"
#include "net/net.h"
#include "net/netmessagemaker.h"
#include "net/netbase.h"
#include "policy/fees.h"
#include "policy/policy.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "misc/random.h"
#include "misc/reverse_iterator.h"
#include "thread/scheduler.h"
#include "misc/tinyformat.h"
#include "transaction/txmempool.h"
#include "ui/ui_interface.h"
#include "utils/util.h"
#include "utils/utilmoneystr.h"
#include "utils/utilstrencodings.h"
#include "validation/validationinterface.h"

#if defined(NDEBUG)
# error "MagnaChain monitor cannot be compiled without assertions."
#endif

typedef std::map<uint256, std::shared_ptr<DatabaseBlock>> BlockHeaderMap;
BlockHeaderMap txSync;
std::shared_ptr<DatabaseBlock> bestKnownBlockHeader;
std::shared_ptr<DatabaseBlock> lastCommon;
std::set<uint256> blockSynced;

std::shared_ptr<DatabaseBlock> GetDatabaseBlock2(const uint256& hash)
{
    BlockHeaderMap::iterator it = txSync.find(hash);
    if (it != txSync.end()) {
        return it->second;
    }
    else {
        return GetDatabaseBlock(hash);
    }
}

std::shared_ptr<DatabaseBlock> GetAncestor(const std::shared_ptr<DatabaseBlock> child, int height)
{
    if (height > child->height || height < 0) {
        return nullptr;
    }

    std::shared_ptr<DatabaseBlock> parent = child;
    int heightWalk = child->height;
    while (heightWalk > height) {
        int heightSkip = GetSkipHeight(heightWalk);
        int heightSkipPrev = GetSkipHeight(heightWalk - 1);
        std::shared_ptr<DatabaseBlock> temp;
        if (!parent->hashSkipBlock.IsNull() && (heightSkip == height ||
            (heightSkip > height && !(heightSkipPrev < heightSkip - 2 && heightSkipPrev >= height)))) {
            //LogPrintf("%s:%d %d=>%d\n", __FUNCTION__, __LINE__, parent->height, GetSkipHeight(parent->height));
            temp = GetDatabaseBlock2(parent->hashSkipBlock);
            heightWalk = heightSkip;
        }
        else {
            temp = GetDatabaseBlock2(parent->hashPrevBlock);
            heightWalk--;
        }

        parent = temp;
    }
    return parent;
}

MCBlockLocator MonitorGetLocator(const MCBlockIndex *pindex)
{
    std::shared_ptr<DatabaseBlock> block = GetDatabaseBlock2(pindex->GetBlockHash());

    int nStep = 1;
    std::vector<uint256> vHave;
    vHave.reserve(32);
    while (true) {
        vHave.emplace_back(block->hashBlock);
        // Stop when we have added the genesis block.
        if (block->height == 0) {
            break;
        }
        // Exponentially larger steps back, plus the genesis block.
        int height = std::max(block->height - nStep, 0);
        block = GetAncestor(block, height);
        if (block == nullptr) {
            break;
        }
        if (vHave.size() > 10) {
            nStep *= 2;
        }
    }

    return MCBlockLocator(vHave);
}

MCCriticalSection cs_block;
bool stopSyncHeader = false;
const int MAX_HALF_TX_SYNC = 2000;
bool static MonitorProcessHeadersMessage(MCNode *pfrom, MCConnman *connman, const std::vector<MCBlockHeader>& headers, const MCChainParams& chainparams, bool punish_duplicate_invalid)
{
    static MCBlockIndex blockIndex;
    static std::shared_ptr<uint256> blockhash;
    const CNetMsgMaker msgMaker(pfrom->GetSendVersion());
    size_t nCount = headers.size();

    if (nCount == 0) {
        // Nothing interesting. Stop asking this peers for more headers.
        return true;
    }

    LOCK(cs_block);
    MCNodeState *nodestate = State(pfrom->GetId());

    // If this looks like it could be a block announcement (nCount <
    // MAX_BLOCKS_TO_ANNOUNCE), use special logic for handling headers that
    // don't connect:
    // - Send a getheaders message in response to try to connect the chain.
    // - The peer can send up to MAX_UNCONNECTING_HEADERS in a row that
    //   don't connect before giving DoS points
    // - Once a headers message is received that is valid and does connect,
    //   nUnconnectingHeaders gets reset back to 0.
    std::shared_ptr<DatabaseBlock> prevBlock = GetDatabaseBlock2(headers[0].hashPrevBlock);
    if (!prevBlock->hashBlock.IsNull() && nCount < MAX_BLOCKS_TO_ANNOUNCE) {
        nodestate->nUnconnectingHeaders++;
        const uint256& bestBlockHash = GetMaxHeightBlock();
        blockIndex.phashBlock = &bestBlockHash;
        connman->PushMessage(pfrom, msgMaker.Make(NetMsgType::GETHEADERS, MonitorGetLocator(&blockIndex), uint256()));
    }

    // Download as much as possible, from earliest to latest.
    std::vector<MCInv> vGetData;
    for (int i = 0; i < headers.size(); ++i) {
        const MCBlockHeader& header = headers[i];

        /*if (txSync.size() == 0 && vGetData.size() < MAX_BLOCKS_IN_TRANSIT_PER_PEER) {
            if (blockSynced.insert(header.GetHash()).second) {
                uint32_t nFetchFlags = GetFetchFlags(pfrom);
                vGetData.push_back(MCInv(MSG_BLOCK | nFetchFlags, header.GetHash()));
            }
        }*/

        std::shared_ptr<DatabaseBlock> blockHeader = std::make_shared<DatabaseBlock>();
        auto res = txSync.insert(std::make_pair(header.GetHash(), blockHeader));
        if (res.second) {
            prevBlock = GetDatabaseBlock2(header.hashPrevBlock);
            std::shared_ptr<DatabaseBlock> skipBlock = GetAncestor(prevBlock, GetSkipHeight(prevBlock->height + 1));
            blockHeader->hashBlock = header.GetHash();
            blockHeader->height = prevBlock->height + 1;
            blockHeader->hashPrevBlock = header.hashPrevBlock;
            blockHeader->hashSkipBlock = skipBlock->hashBlock;

            if (bestKnownBlockHeader == nullptr || blockHeader->height > bestKnownBlockHeader->height) {
                bestKnownBlockHeader = res.first->second;
            }
        }
        LogPrint(BCLog::NET, "Receive block header %s from peer=%d\n", header.GetHash().ToString(), pfrom->GetId());
    }

    if (vGetData.size() > 0) {
        connman->PushMessage(pfrom, msgMaker.Make(NetMsgType::GETDATA, vGetData));
    }

    if (txSync.size() >= 2 * MAX_HALF_TX_SYNC - 100) {
        stopSyncHeader = true;
    }
    if (!stopSyncHeader && nCount == MAX_HEADERS_RESULTS) {
        // Headers message had its maximum size; the peer may have more headers.
        // TODO: optimize: if pindexLast is an ancestor of chainActive.Tip or pindexBestHeader, continue
        // from there instead.
        const uint256& blockHash = bestKnownBlockHeader->hashBlock;
        blockIndex.phashBlock = &blockHash;
        connman->PushMessage(pfrom, msgMaker.Make(NetMsgType::GETHEADERS, MonitorGetLocator(&blockIndex), uint256()));
    }

    if (lastCommon == nullptr) {
        lastCommon = GetDatabaseBlock2(chainActive.Tip()->GetBlockHash());
    }
}

bool MonitorProcessMessage(MCNode* pfrom, const std::string& strCommand, MCDataStream& vRecv, int64_t nTimeReceived, const MCChainParams& chainparams, MCConnman* connman, const std::atomic<bool>& interruptMsgProc)
{
    LogPrint(BCLog::NET, "received: %s (%u bytes) peer=%d\n", SanitizeString(strCommand), vRecv.size(), pfrom->GetId());
    if (gArgs.IsArgSet("-dropmessagestest") && GetRand(gArgs.GetArg("-dropmessagestest", 0)) == 0)
    {
        LogPrintf("dropmessagestest DROPPING RECV MESSAGE\n");
        return true;
    }


    if (!(pfrom->GetLocalServices() & NODE_BLOOM) &&
              (strCommand == NetMsgType::FILTERLOAD ||
               strCommand == NetMsgType::FILTERADD))
    {
        if (pfrom->nVersion >= NO_BLOOM_VERSION) {
            LOCK(cs_main);
            Misbehaving(pfrom->GetId(), 100);
            return false;
        } else {
            pfrom->fDisconnect = true;
            return false;
        }
    }

    if (strCommand == NetMsgType::REJECT)
    {
        if (LogAcceptCategory(BCLog::NET)) {
            try {
                std::string strMsg;
                unsigned char ccode;
                std::string strReason;
                vRecv >> LIMITED_STRING(strMsg, MCMessageHeader::COMMAND_SIZE) >> ccode >> LIMITED_STRING(strReason, MAX_REJECT_MESSAGE_LENGTH);

                std::ostringstream ss;
                ss << strMsg << " code " << itostr(ccode) << ": " << strReason;

                if (strMsg == NetMsgType::BLOCK || strMsg == NetMsgType::TX)
                {
                    uint256 hash;
                    vRecv >> hash;
                    ss << ": hash " << hash.ToString();
                }
                LogPrint(BCLog::NET, "Reject %s\n", SanitizeString(ss.str()));
            } catch (const std::ios_base::failure&) {
                // Avoid feedback loops by preventing reject messages from triggering a new reject message.
                LogPrint(BCLog::NET, "Unparseable reject message received\n");
            }
        }
    }

    else if (strCommand == NetMsgType::VERSION)
    {
        // Each connection can only send one version message
        if (pfrom->nVersion != 0)
        {
            connman->PushMessage(pfrom, CNetMsgMaker(INIT_PROTO_VERSION).Make(NetMsgType::REJECT, strCommand, REJECT_DUPLICATE, std::string("Duplicate version message")));
            LOCK(cs_main);
            Misbehaving(pfrom->GetId(), 1);
            return false;
        }

        int64_t nTime;
        MCAddress addrMe;
        MCAddress addrFrom;
        uint64_t nNonce = 1;
        uint64_t nServiceInt;
        ServiceFlags nServices;
        int nVersion;
        int nSendVersion;
        std::string strSubVer;
        std::string cleanSubVer;
        int nStartingHeight = -1;
        bool fRelay = true;
		std::string strBranchId;

        vRecv >> nVersion >> nServiceInt >> nTime >> strBranchId >> addrMe;
		if (strBranchId != Params().GetBranchId())
		{
			LogPrint(BCLog::NET, "peer=%d branchid not match, it's %s; disconnecting\n", pfrom->GetId(), strBranchId);
			connman->PushMessage(pfrom, CNetMsgMaker(INIT_PROTO_VERSION).Make(NetMsgType::REJECT, strCommand, REJECT_NONSTANDARD,
				strprintf("Expected to offer services %08x", pfrom->nServicesExpected)));
			pfrom->fDisconnect = true;
			return false;
		}

		nSendVersion = std::min(nVersion, PROTOCOL_VERSION);
        nServices = ServiceFlags(nServiceInt);
        if (!pfrom->fInbound)
        {
            connman->SetServices(pfrom->addr, nServices);
        }
        if (pfrom->nServicesExpected & ~nServices)
        {
            LogPrint(BCLog::NET, "peer=%d does not offer the expected services (%08x offered, %08x expected); disconnecting\n", pfrom->GetId(), nServices, pfrom->nServicesExpected);
            connman->PushMessage(pfrom, CNetMsgMaker(INIT_PROTO_VERSION).Make(NetMsgType::REJECT, strCommand, REJECT_NONSTANDARD,
                               strprintf("Expected to offer services %08x", pfrom->nServicesExpected)));
            pfrom->fDisconnect = true;
            return false;
        }

        if (nServices & ((1 << 7) | (1 << 5))) {
            if (GetTime() < 1533096000) {
                // Immediately disconnect peers that use service bits 6 or 8 until August 1st, 2018
                // These bits have been used as a flag to indicate that a node is running incompatible
                // consensus rules instead of changing the network magic, so we're stuck disconnecting
                // based on these service bits, at least for a while.
                pfrom->fDisconnect = true;
                return false;
            }
        }

        if (nVersion < MIN_PEER_PROTO_VERSION)
        {
            // disconnect from peers older than this proto version
            LogPrintf("peer=%d using obsolete version %i; disconnecting\n", pfrom->GetId(), nVersion);
            connman->PushMessage(pfrom, CNetMsgMaker(INIT_PROTO_VERSION).Make(NetMsgType::REJECT, strCommand, REJECT_OBSOLETE,
                               strprintf("Version must be %d or greater", MIN_PEER_PROTO_VERSION)));
            pfrom->fDisconnect = true;
            return false;
        }

        if (nVersion == 10300)
            nVersion = 300;
        if (!vRecv.empty())
            vRecv >> addrFrom >> nNonce;
        if (!vRecv.empty()) {
            vRecv >> LIMITED_STRING(strSubVer, MAX_SUBVERSION_LENGTH);
            cleanSubVer = SanitizeString(strSubVer);
        }
        if (!vRecv.empty()) {
            vRecv >> nStartingHeight;
        }
        if (!vRecv.empty())
            vRecv >> fRelay;
        // Disconnect if we connected to ourself
        if (pfrom->fInbound && !connman->CheckIncomingNonce(nNonce))
        {
            LogPrintf("connected to self at %s, disconnecting\n", pfrom->addr.ToString());
            pfrom->fDisconnect = true;
            return true;
        }

        if (pfrom->fInbound && addrMe.IsRoutable())
        {
            SeenLocal(addrMe);
        }

        // Be shy and don't send version until we hear
        if (pfrom->fInbound)
            PushNodeVersion(pfrom, connman, GetAdjustedTime());

        connman->PushMessage(pfrom, CNetMsgMaker(INIT_PROTO_VERSION).Make(NetMsgType::VERACK));

        pfrom->nServices = nServices;
        pfrom->SetAddrLocal(addrMe);
        {
            LOCK(pfrom->cs_SubVer);
            pfrom->strSubVer = strSubVer;
            pfrom->cleanSubVer = cleanSubVer;
        }
        pfrom->nStartingHeight = nStartingHeight;
        pfrom->fClient = !(nServices & NODE_NETWORK);
        {
            LOCK(pfrom->cs_filter);
            pfrom->fRelayTxes = fRelay; // set to true after we get the first filter* message
        }

        // Change version
        pfrom->SetSendVersion(nSendVersion);
        pfrom->nVersion = nVersion;

        if((nServices & NODE_WITNESS))
        {
            LOCK(cs_main);
            State(pfrom->GetId())->fHaveWitness = true;
        }

        // Potentially mark this peer as a preferred download peer.
        {
        LOCK(cs_main);
        UpdatePreferredDownload(pfrom, State(pfrom->GetId()));
        }

        if (!pfrom->fInbound)
        {
            // Advertise our address
            if (fListen && !IsInitialBlockDownload())
            {
                MCAddress addr = GetLocalAddress(&pfrom->addr, pfrom->GetLocalServices());
                FastRandomContext insecure_rand;
                if (addr.IsRoutable())
                {
                    LogPrint(BCLog::NET, "ProcessMessages: advertising address %s\n", addr.ToString());
                    pfrom->PushAddress(addr, insecure_rand);
                } else if (IsPeerAddrLocalGood(pfrom)) {
                    addr.SetIP(addrMe);
                    LogPrint(BCLog::NET, "ProcessMessages: advertising address %s\n", addr.ToString());
                    pfrom->PushAddress(addr, insecure_rand);
                }
            }

            // Get recent addresses
            if (pfrom->fOneShot || pfrom->nVersion >= CADDR_TIME_VERSION || connman->GetAddressCount() < 1000)
            {
                connman->PushMessage(pfrom, CNetMsgMaker(nSendVersion).Make(NetMsgType::GETADDR));
                pfrom->fGetAddr = true;
            }
            connman->MarkAddressGood(pfrom->addr);
        }

        std::string remoteAddr;
        if (fLogIPs)
            remoteAddr = ", peeraddr=" + pfrom->addr.ToString();

        LogPrintf("receive version message: %s: version %d, blocks=%d, us=%s, peer=%d%s\n",
                  cleanSubVer, pfrom->nVersion,
                  pfrom->nStartingHeight, addrMe.ToString(), pfrom->GetId(),
                  remoteAddr);

        int64_t nTimeOffset = nTime - GetTime();
        pfrom->nTimeOffset = nTimeOffset;
        AddTimeData(pfrom->addr, nTimeOffset);

        // If the peer is old enough to have the old alert system, send it the final alert.
        if (pfrom->nVersion <= 70012) {
            MCDataStream finalAlert(ParseHex("60010000000000000000000000ffffff7f00000000ffffff7ffeffff7f01ffffff7f00000000ffffff7f00ffffff7f002f555247454e543a20416c657274206b657920636f6d70726f6d697365642c2075706772616465207265717569726564004630440220653febd6410f470f6bae11cad19c48413becb1ac2c17f908fd0fd53bdc3abd5202206d0e9c96fe88d4a0f01ed9dedae2b6f9e00da94cad0fecaae66ecf689bf71b50"), SER_NETWORK, PROTOCOL_VERSION);
            connman->PushMessage(pfrom, CNetMsgMaker(nSendVersion).Make("alert", finalAlert));
        }

        // Feeler connections exist only to verify if address is online.
        if (pfrom->fFeeler) {
            assert(pfrom->fInbound == false);
            pfrom->fDisconnect = true;
        }
        return true;
    }

    else if (pfrom->nVersion == 0)
    {
        // Must have a version message before anything else
        LOCK(cs_main);
        Misbehaving(pfrom->GetId(), 1);
        return false;
    }

    // At this point, the outgoing message serialization version can't change.
    const CNetMsgMaker msgMaker(pfrom->GetSendVersion());

    if (strCommand == NetMsgType::VERACK)
    {
        pfrom->SetRecvVersion(std::min(pfrom->nVersion.load(), PROTOCOL_VERSION));

        if (!pfrom->fInbound) {
            // Mark this node as currently connected, so we update its timestamp later.
            LOCK(cs_main);
            State(pfrom->GetId())->fCurrentlyConnected = true;
        }

        if (pfrom->nVersion >= SENDHEADERS_VERSION) {
            // Tell our peer we prefer to receive headers rather than inv's
            // We send this to non-NODE NETWORK peers as well, because even
            // non-NODE NETWORK peers can announce blocks (such as pruning
            // nodes)
            connman->PushMessage(pfrom, msgMaker.Make(NetMsgType::SENDHEADERS));
        }
        if (pfrom->nVersion >= SHORT_IDS_BLOCKS_VERSION) {
            // Tell our peer we are willing to provide version 1 or 2 cmpctblocks
            // However, we do not request new block announcements using
            // cmpctblock messages.
            // We send this to non-NODE NETWORK peers as well, because
            // they may wish to request compact blocks from us
            bool fAnnounceUsingCMPCTBLOCK = false;
            uint64_t nCMPCTBLOCKVersion = 2;
            if (pfrom->GetLocalServices() & NODE_WITNESS)
                connman->PushMessage(pfrom, msgMaker.Make(NetMsgType::SENDCMPCT, fAnnounceUsingCMPCTBLOCK, nCMPCTBLOCKVersion));
            nCMPCTBLOCKVersion = 1;
            connman->PushMessage(pfrom, msgMaker.Make(NetMsgType::SENDCMPCT, fAnnounceUsingCMPCTBLOCK, nCMPCTBLOCKVersion));
        }
        pfrom->fSuccessfullyConnected = true;
    }

    else if (!pfrom->fSuccessfullyConnected)
    {
        // Must have a verack message before anything else
        LOCK(cs_main);
        Misbehaving(pfrom->GetId(), 1);
        return false;
    }

    else if (strCommand == NetMsgType::HEADERS && !fImporting && !fReindex) // Ignore headers received while importing
    {
        std::vector<MCBlockHeader> headers;

        // Bypass the normal MCBlock deserialization, as we don't want to risk deserializing 2000 full blocks.
        unsigned int nCount = ReadCompactSize(vRecv);
        if (nCount > MAX_HEADERS_RESULTS) {
            LOCK(cs_main);
            Misbehaving(pfrom->GetId(), 20);
            return error("headers message size = %u", nCount);
        }
        headers.resize(nCount);
        for (unsigned int n = 0; n < nCount; n++) {
            vRecv >> headers[n];
            ReadCompactSize(vRecv); // ignore tx count; assume it is 0.
            ReadCompactSize(vRecv); // groupSize is 0
            ReadCompactSize(vRecv); // prevContractData is 0
        }

        // Headers received via a HEADERS message should be valid, and reflect
        // the chain the peer is on. If we receive a known-invalid header,
        // disconnect the peer if it is using one of our outbound connection
        // slots.
        bool should_punish = !pfrom->fInbound && !pfrom->m_manual_connection;
        return MonitorProcessHeadersMessage(pfrom, connman, headers, chainparams, should_punish);
    }

    else if (strCommand == NetMsgType::BLOCK && !fImporting && !fReindex) // Ignore blocks received while importing
    {
        std::shared_ptr<MCBlock> pblock = std::make_shared<MCBlock>();
        size_t start = vRecv.size();
        vRecv >> *pblock;
        size_t sz = start - vRecv.size();
        LogPrint(BCLog::NET, "received block %s peer=%d\n", pblock->GetHash().ToString(), pfrom->GetId());

        {
            LOCK(cs_block);
            std::shared_ptr<DatabaseBlock> dbBlock = GetDatabaseBlock2(pblock->GetHash());
            if (dbBlock == nullptr) {
                dbBlock = std::make_shared<DatabaseBlock>();
                dbBlock->hashBlock = pblock->GetHash();
            }

            MCBlock& block = *pblock;
            if (WriteBlockToDatabase(block, dbBlock, sz)) {
                if (lastCommon == nullptr || dbBlock->height > lastCommon->height) {
                    lastCommon = dbBlock;
                }
                if (bestKnownBlockHeader == nullptr || dbBlock->height > bestKnownBlockHeader->height) {
                    bestKnownBlockHeader = dbBlock;
                }
                txSync.erase(pblock->GetHash());
                blockSynced.erase(block.GetHash());
                pfrom->nLastBlockTime = GetTime();
            }
        }
    }

    else if (strCommand == NetMsgType::PING)
    {
        if (pfrom->nVersion > BIP0031_VERSION)
        {
            uint64_t nonce = 0;
            vRecv >> nonce;
            // Echo the message back with the nonce. This allows for two useful features:
            //
            // 1) A remote node can quickly check if the connection is operational
            // 2) Remote nodes can measure the latency of the network thread. If this node
            //    is overloaded it won't respond to pings quickly and the remote node can
            //    avoid sending us more work, like chain download requests.
            //
            // The nonce stops the remote getting confused between different pings: without
            // it, if the remote node sends a ping once per second and this node takes 5
            // seconds to respond to each, the 5th ping the remote sends would appear to
            // return very quickly.
            connman->PushMessage(pfrom, msgMaker.Make(NetMsgType::PONG, nonce));
        }
    }

    else if (strCommand == NetMsgType::PONG)
    {
        int64_t pingUsecEnd = nTimeReceived;
        uint64_t nonce = 0;
        size_t nAvail = vRecv.in_avail();
        bool bPingFinished = false;
        std::string sProblem;

        if (nAvail >= sizeof(nonce)) {
            vRecv >> nonce;

            // Only process pong message if there is an outstanding ping (old ping without nonce should never pong)
            if (pfrom->nPingNonceSent != 0) {
                if (nonce == pfrom->nPingNonceSent) {
                    // Matching pong received, this ping is no longer outstanding
                    bPingFinished = true;
                    int64_t pingUsecTime = pingUsecEnd - pfrom->nPingUsecStart;
                    if (pingUsecTime > 0) {
                        // Successful ping time measurement, replace previous
                        pfrom->nPingUsecTime = pingUsecTime;
                        pfrom->nMinPingUsecTime = std::min(pfrom->nMinPingUsecTime.load(), pingUsecTime);
                    } else {
                        // This should never happen
                        sProblem = "Timing mishap";
                    }
                } else {
                    // Nonce mismatches are normal when pings are overlapping
                    sProblem = "Nonce mismatch";
                    if (nonce == 0) {
                        // This is most likely a bug in another implementation somewhere; cancel this ping
                        bPingFinished = true;
                        sProblem = "Nonce zero";
                    }
                }
            } else {
                sProblem = "Unsolicited pong without ping";
            }
        } else {
            // This is most likely a bug in another implementation somewhere; cancel this ping
            bPingFinished = true;
            sProblem = "Short payload";
        }

        if (!(sProblem.empty())) {
            LogPrint(BCLog::NET, "pong peer=%d: %s, %x expected, %x received, %u bytes\n",
                pfrom->GetId(),
                sProblem,
                pfrom->nPingNonceSent,
                nonce,
                nAvail);
        }
        if (bPingFinished) {
            pfrom->nPingNonceSent = 0;
        }
    }

    else if (strCommand == NetMsgType::NOTFOUND) {
        // We do not care about the NOTFOUND message, but logging an Unknown Command
        // message would be undesirable as we transmit it ourselves.
    }

    else {
        // Ignore unknown commands for extensibility
        LogPrint(BCLog::NET, "Unknown command \"%s\" from peer=%d\n", SanitizeString(strCommand), pfrom->GetId());
    }

    return true;
}

void FindNextBlocksToDownload(unsigned int count, std::vector<std::shared_ptr<DatabaseBlock>>& vBlocks)
{
    if (lastCommon == nullptr || count == 0) {
        return;
    }

    std::vector<std::shared_ptr<DatabaseBlock>> vToFetch;
    std::shared_ptr<DatabaseBlock> pWalk = lastCommon;
    int nWindowEnd = pWalk->height + BLOCK_DOWNLOAD_WINDOW;
    int nMaxHeight = std::min<int>(bestKnownBlockHeader->height, nWindowEnd + 1);
    NodeId waitingfor = -1;
    while (pWalk->height < nMaxHeight) {
        // Read up to 128 (or more, if more blocks than that are needed) successors of pindexWalk (towards
        // pindexBestKnownBlock) into vToFetch. We fetch 128, because MCBlockIndex::GetAncestor may be as expensive
        // as iterating over ~100 MCBlockIndex* entries anyway.
        int nToFetch = std::min(nMaxHeight - pWalk->height, std::max<int>(count - vBlocks.size(), 128));
        vToFetch.resize(nToFetch);
        pWalk = GetAncestor(bestKnownBlockHeader, pWalk->height + nToFetch);
        vToFetch[nToFetch - 1] = pWalk;
        for (unsigned int i = nToFetch - 1; i > 0; i--) {
            vToFetch[i - 1] = txSync.find(vToFetch[i]->hashPrevBlock)->second;
        }

        // Iterate over those blocks in vToFetch (in forward direction), adding the ones that
        // are not yet downloaded and not in flight to vBlocks. In the mean time, update
        // pindexLastCommonBlock as long as all ancestors are already downloaded, or if it's
        // already part of our chain (and therefore don't need it even if pruned).
        for (const std::shared_ptr<DatabaseBlock> item : vToFetch) {
            if (blockSynced.count(item->hashBlock) == 0) {
                // The block is not already downloaded, and not yet in flight.
                if (item->height > nWindowEnd) {
                    return;
                }
                vBlocks.push_back(item);
                if (vBlocks.size() == count) {
                    return;
                }
            }
        }
    }
}

MonitorPeerLogicValidation::MonitorPeerLogicValidation(MCConnman* connman, MCScheduler &scheduler, ProcessMessageFunc processMessageFunc, GetLocatorFunc getLocatorFunc)
    : PeerLogicValidation(connman, scheduler, processMessageFunc, getLocatorFunc)
{
}

void MonitorPeerLogicValidation::GetBlockData(MCNode* pto, MCNodeState& state, bool fFetch, std::vector<MCInv>& vGetData)
{
    std::vector<std::shared_ptr<DatabaseBlock>> vToDownload;
    FindNextBlocksToDownload(MAX_BLOCKS_IN_TRANSIT_PER_PEER - blockSynced.size(), vToDownload);
    for (const std::shared_ptr<DatabaseBlock> item : vToDownload) {
        if (blockSynced.size() >= MAX_BLOCKS_IN_TRANSIT_PER_PEER) {
            break;
        }
        uint32_t nFetchFlags = GetFetchFlags(pto);
        vGetData.push_back(MCInv(MSG_BLOCK | nFetchFlags, item->hashBlock));
        blockSynced.insert(item->hashBlock);
        LogPrint(BCLog::NET, "Requesting block %s (%d)\n", item->hashBlock.ToString(), item->height);
    }

    if (stopSyncHeader && txSync.size() < MAX_HALF_TX_SYNC) {
        // Headers message had its maximum size; the peer may have more headers.
        // TODO: optimize: if pindexLast is an ancestor of chainActive.Tip or pindexBestHeader, continue
        // from there instead.
        stopSyncHeader = false;
        MCBlockIndex blockIndex;
        const uint256& blockHash = bestKnownBlockHeader->hashBlock;
        blockIndex.phashBlock = &blockHash;
        const CNetMsgMaker msgMaker(pto->GetSendVersion());
        connman->PushMessage(pto, msgMaker.Make(NetMsgType::GETHEADERS, MonitorGetLocator(&blockIndex), uint256()));
    }
}
