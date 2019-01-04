// Copyright (c) 2011-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// Unit tests for denial-of-service detection/prevention code

#include "chain/chainparams.h"
#include "key/keystore.h"
#include "net/net.h"
#include "net/net_processing.h"
#include "misc/pow.h"
#include "script/sign.h"
#include "io/serialize.h"
#include "utils/util.h"
#include "validation/validation.h"

#include "test/test_magnachain.h"

#include <stdint.h>

#include <boost/test/unit_test.hpp>

// Tests these internal-to-net_processing.cpp methods:
extern bool AddOrphanTx(const MCTransactionRef& tx, NodeId peer);
extern void EraseOrphansFor(NodeId peer);
extern unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans);
struct MCOrphanTx {
    MCTransactionRef tx;
    NodeId fromPeer;
    int64_t nTimeExpire;
};
extern std::map<uint256, MCOrphanTx> mapOrphanTransactions;

MCService ip(uint32_t i)
{
    struct in_addr s;
    s.s_addr = i;
    return MCService(MCNetAddr(s), Params().GetDefaultPort());
}

static NodeId id = 0;

void UpdateLastBlockAnnounceTime(NodeId node, int64_t time_in_seconds);

BOOST_FIXTURE_TEST_SUITE(DoS_tests, TestingSetup)

// Test eviction of an outbound peer whose chain never advances
// Mock a node connection, and use mocktime to simulate a peer
// which never sends any headers messages.  PeerLogic should
// decide to evict that outbound peer, after the appropriate timeouts.
// Note that we protect 4 outbound nodes from being subject to
// this logic; this test takes advantage of that protection only
// being applied to nodes which send headers with sufficient
// work.
BOOST_AUTO_TEST_CASE(outbound_slow_chain_eviction)
{
    std::atomic<bool> interruptDummy(false);

    // Mock an outbound peer
    MCAddress addr1(ip(0xa0b0c001), NODE_NONE);
    MCNode dummyNode1(id++, ServiceFlags(NODE_NETWORK|NODE_WITNESS), 0, INVALID_SOCKET, addr1, 0, 0, MCAddress(), "", /*fInboundIn=*/ false);
    dummyNode1.SetSendVersion(PROTOCOL_VERSION);

    peerLogic->InitializeNode(&dummyNode1);
    dummyNode1.nVersion = 1;
    dummyNode1.fSuccessfullyConnected = true;

    // This test requires that we have a chain with non-zero work.
    BOOST_CHECK(chainActive.Tip() != nullptr);
    BOOST_CHECK(chainActive.Tip()->nChainWork > 0);

    // Test starts here
    peerLogic->SendMessages(&dummyNode1, interruptDummy); // should result in getheaders
    BOOST_CHECK(dummyNode1.vSendMsg.size() > 0);
    dummyNode1.vSendMsg.clear();

    int64_t nStartTime = GetTime();
    // Wait 21 minutes
    SetMockTime(nStartTime+21*60);
    peerLogic->SendMessages(&dummyNode1, interruptDummy); // should result in getheaders
    BOOST_CHECK(dummyNode1.vSendMsg.size() > 0);
    // Wait 3 more minutes
    SetMockTime(nStartTime+24*60);
    peerLogic->SendMessages(&dummyNode1, interruptDummy); // should result in disconnect
    BOOST_CHECK(dummyNode1.fDisconnect == true);
    SetMockTime(0);

    bool dummy;
    peerLogic->FinalizeNode(dummyNode1.GetId(), dummy);
}

void AddRandomOutboundPeer(std::vector<MCNode *> &vNodes, PeerLogicValidation &peerLogic)
{
    MCAddress addr(ip(GetRandInt(0xffffffff)), NODE_NONE);
    vNodes.emplace_back(new MCNode(id++, ServiceFlags(NODE_NETWORK|NODE_WITNESS), 0, INVALID_SOCKET, addr, 0, 0, MCAddress(), "", /*fInboundIn=*/ false));
    MCNode &node = *vNodes.back();
    node.SetSendVersion(PROTOCOL_VERSION);

    peerLogic.InitializeNode(&node);
    node.nVersion = 1;
    node.fSuccessfullyConnected = true;

    MCConnmanTest::AddNode(node);
}

BOOST_AUTO_TEST_CASE(stale_tip_peer_management)
{
    const Consensus::Params& consensusParams = Params().GetConsensus();
    constexpr int nMaxOutbound = 8;
    MCConnman::Options options;
    options.nMaxConnections = 125;
    options.nMaxOutbound = nMaxOutbound;
    options.nMaxFeeler = 1;

    connman->Init(options);
    std::vector<MCNode *> vNodes;

    // Mock some outbound peers
    for (int i=0; i<nMaxOutbound; ++i) {
        AddRandomOutboundPeer(vNodes, *peerLogic);
    }

    peerLogic->CheckForStaleTipAndEvictPeers(consensusParams);

    // No nodes should be marked for disconnection while we have no extra peers
    for (const MCNode *node : vNodes) {
        BOOST_CHECK(node->fDisconnect == false);
    }

    int64_t oldSpacing = 10 * 60;
    SetMockTime(GetTime() + 3 * oldSpacing + 1);

    // Now tip should definitely be stale, and we should look for an extra
    // outbound peer
    peerLogic->CheckForStaleTipAndEvictPeers(consensusParams);
    BOOST_CHECK(connman->GetTryNewOutboundPeer());

    // Still no peers should be marked for disconnection
    for (const MCNode *node : vNodes) {
        BOOST_CHECK(node->fDisconnect == false);
    }

    // If we add one more peer, something should get marked for eviction
    // on the next check (since we're mocking the time to be in the future, the
    // required time connected check should be satisfied).
    AddRandomOutboundPeer(vNodes, *peerLogic);

    peerLogic->CheckForStaleTipAndEvictPeers(consensusParams);
    for (int i=0; i<nMaxOutbound; ++i) {
        BOOST_CHECK(vNodes[i]->fDisconnect == false);
    }
    // Last added node should get marked for eviction
    BOOST_CHECK(vNodes.back()->fDisconnect == true);

    vNodes.back()->fDisconnect = false;

    // Update the last announced block time for the last
    // peer, and check that the next newest node gets evicted.
    UpdateLastBlockAnnounceTime(vNodes.back()->GetId(), GetTime());

    peerLogic->CheckForStaleTipAndEvictPeers(consensusParams);
    for (int i=0; i<nMaxOutbound-1; ++i) {
        BOOST_CHECK(vNodes[i]->fDisconnect == false);
    }
    BOOST_CHECK(vNodes[nMaxOutbound-1]->fDisconnect == true);
    BOOST_CHECK(vNodes.back()->fDisconnect == false);

    bool dummy;
    for (const MCNode *node : vNodes) {
        peerLogic->FinalizeNode(node->GetId(), dummy);
    }

    MCConnmanTest::ClearNodes();
}

BOOST_AUTO_TEST_CASE(DoS_banning)
{
    std::atomic<bool> interruptDummy(false);

    connman->ClearBanned();
    MCAddress addr1(ip(0xa0b0c001), NODE_NONE);
    MCNode dummyNode1(id++, NODE_NETWORK, 0, INVALID_SOCKET, addr1, 0, 0, MCAddress(), "", true);
    dummyNode1.SetSendVersion(PROTOCOL_VERSION);
    peerLogic->InitializeNode(&dummyNode1);
    dummyNode1.nVersion = 1;
    dummyNode1.fSuccessfullyConnected = true;
    Misbehaving(dummyNode1.GetId(), 100); // Should get banned
    peerLogic->SendMessages(&dummyNode1, interruptDummy);
    BOOST_CHECK(connman->IsBanned(addr1));
    BOOST_CHECK(!connman->IsBanned(ip(0xa0b0c001|0x0000ff00))); // Different IP, not banned

    MCAddress addr2(ip(0xa0b0c002), NODE_NONE);
    MCNode dummyNode2(id++, NODE_NETWORK, 0, INVALID_SOCKET, addr2, 1, 1, MCAddress(), "", true);
    dummyNode2.SetSendVersion(PROTOCOL_VERSION);
    peerLogic->InitializeNode(&dummyNode2);
    dummyNode2.nVersion = 1;
    dummyNode2.fSuccessfullyConnected = true;
    Misbehaving(dummyNode2.GetId(), 50);
    peerLogic->SendMessages(&dummyNode2, interruptDummy);
    BOOST_CHECK(!connman->IsBanned(addr2)); // 2 not banned yet...
    BOOST_CHECK(connman->IsBanned(addr1));  // ... but 1 still should be
    Misbehaving(dummyNode2.GetId(), 50);
    peerLogic->SendMessages(&dummyNode2, interruptDummy);
    BOOST_CHECK(connman->IsBanned(addr2));

    bool dummy;
    peerLogic->FinalizeNode(dummyNode1.GetId(), dummy);
    peerLogic->FinalizeNode(dummyNode2.GetId(), dummy);
}

BOOST_AUTO_TEST_CASE(DoS_banscore)
{
    std::atomic<bool> interruptDummy(false);

    connman->ClearBanned();
    gArgs.ForceSetArg("-banscore", "111"); // because 11 is my favorite number
    MCAddress addr1(ip(0xa0b0c001), NODE_NONE);
    MCNode dummyNode1(id++, NODE_NETWORK, 0, INVALID_SOCKET, addr1, 3, 1, MCAddress(), "", true);
    dummyNode1.SetSendVersion(PROTOCOL_VERSION);
    peerLogic->InitializeNode(&dummyNode1);
    dummyNode1.nVersion = 1;
    dummyNode1.fSuccessfullyConnected = true;
    Misbehaving(dummyNode1.GetId(), 100);
    peerLogic->SendMessages(&dummyNode1, interruptDummy);
    BOOST_CHECK(!connman->IsBanned(addr1));
    Misbehaving(dummyNode1.GetId(), 10);
    peerLogic->SendMessages(&dummyNode1, interruptDummy);
    BOOST_CHECK(!connman->IsBanned(addr1));
    Misbehaving(dummyNode1.GetId(), 1);
    peerLogic->SendMessages(&dummyNode1, interruptDummy);
    BOOST_CHECK(connman->IsBanned(addr1));
    gArgs.ForceSetArg("-banscore", std::to_string(DEFAULT_BANSCORE_THRESHOLD));

    bool dummy;
    peerLogic->FinalizeNode(dummyNode1.GetId(), dummy);
}

BOOST_AUTO_TEST_CASE(DoS_bantime)
{
    std::atomic<bool> interruptDummy(false);

    connman->ClearBanned();
    int64_t nStartTime = GetTime();
    SetMockTime(nStartTime); // Overrides future calls to GetTime()

    MCAddress addr(ip(0xa0b0c001), NODE_NONE);
    MCNode dummyNode(id++, NODE_NETWORK, 0, INVALID_SOCKET, addr, 4, 4, MCAddress(), "", true);
    dummyNode.SetSendVersion(PROTOCOL_VERSION);
    peerLogic->InitializeNode(&dummyNode);
    dummyNode.nVersion = 1;
    dummyNode.fSuccessfullyConnected = true;

    Misbehaving(dummyNode.GetId(), 100);
    peerLogic->SendMessages(&dummyNode, interruptDummy);
    BOOST_CHECK(connman->IsBanned(addr));

    SetMockTime(nStartTime+60*60);
    BOOST_CHECK(connman->IsBanned(addr));

    SetMockTime(nStartTime+60*60*24+1);
    BOOST_CHECK(!connman->IsBanned(addr));

    bool dummy;
    peerLogic->FinalizeNode(dummyNode.GetId(), dummy);
}

MCTransactionRef RandomOrphan()
{
    std::map<uint256, MCOrphanTx>::iterator it;
    it = mapOrphanTransactions.lower_bound(InsecureRand256());
    if (it == mapOrphanTransactions.end())
        it = mapOrphanTransactions.begin();
    return it->second.tx;
}

BOOST_AUTO_TEST_CASE(DoS_mapOrphans)
{
    MCKey key;
    key.MakeNewKey(true);
    MCBasicKeyStore keystore;
    keystore.AddKey(key);

    // 50 orphan transactions:
    for (int i = 0; i < 50; i++)
    {
        MCMutableTransaction tx;
        tx.vin.resize(1);
        tx.vin[0].prevout.n = 0;
        tx.vin[0].prevout.hash = InsecureRand256();
        tx.vin[0].scriptSig << OP_1;
        tx.vout.resize(1);
        tx.vout[0].nValue = 1*CENT;
        tx.vout[0].scriptPubKey = GetScriptForDestination(key.GetPubKey().GetID());

        AddOrphanTx(MakeTransactionRef(tx), i);
    }

    // ... and 50 that depend on other orphans:
    for (int i = 0; i < 50; i++)
    {
        MCTransactionRef txPrev = RandomOrphan();

        MCMutableTransaction tx;
        tx.vin.resize(1);
        tx.vin[0].prevout.n = 0;
        tx.vin[0].prevout.hash = txPrev->GetHash();
        tx.vout.resize(1);
        tx.vout[0].nValue = 1*CENT;
        tx.vout[0].scriptPubKey = GetScriptForDestination(key.GetPubKey().GetID());
        SignSignature(keystore, *txPrev, tx, 0, SIGHASH_ALL);

        AddOrphanTx(MakeTransactionRef(tx), i);
    }

    // This really-big orphan should be ignored:
    for (int i = 0; i < 10; i++)
    {
        MCTransactionRef txPrev = RandomOrphan();

        MCMutableTransaction tx;
        tx.vout.resize(1);
        tx.vout[0].nValue = 1*CENT;
        tx.vout[0].scriptPubKey = GetScriptForDestination(key.GetPubKey().GetID());
        tx.vin.resize(2777);
        for (unsigned int j = 0; j < tx.vin.size(); j++)
        {
            tx.vin[j].prevout.n = j;
            tx.vin[j].prevout.hash = txPrev->GetHash();
        }
        SignSignature(keystore, *txPrev, tx, 0, SIGHASH_ALL);
        // Re-use same signature for other inputs
        // (they don't have to be valid for this test)
        for (unsigned int j = 1; j < tx.vin.size(); j++)
            tx.vin[j].scriptSig = tx.vin[0].scriptSig;

        BOOST_CHECK(!AddOrphanTx(MakeTransactionRef(tx), i));
    }

    // Test EraseOrphansFor:
    for (NodeId i = 0; i < 3; i++)
    {
        size_t sizeBefore = mapOrphanTransactions.size();
        EraseOrphansFor(i);
        BOOST_CHECK(mapOrphanTransactions.size() < sizeBefore);
    }

    // Test LimitOrphanTxSize() function:
    LimitOrphanTxSize(40);
    BOOST_CHECK(mapOrphanTransactions.size() <= 40);
    LimitOrphanTxSize(10);
    BOOST_CHECK(mapOrphanTransactions.size() <= 10);
    LimitOrphanTxSize(0);
    BOOST_CHECK(mapOrphanTransactions.empty());
}

BOOST_AUTO_TEST_SUITE_END()
