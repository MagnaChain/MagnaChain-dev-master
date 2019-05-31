// Copyright (c) 2012-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "address/addrman.h"
#include "test/test_magnachain.h"
#include <string>
#include <boost/test/unit_test.hpp>
#include "coding/hash.h"
#include "io/serialize.h"
#include "io/streams.h"
#include "net/net.h"
#include "net/netbase.h"
#include "chain/chainparams.h"
#include "utils/util.h"

class MCAddrManSerializationMock : public MCAddrMan
{
public:
    virtual void Serialize(MCDataStream& s) const = 0;

    //! Ensure that bucket placement is always the same for testing purposes.
    void MakeDeterministic()
    {
        nKey.SetNull();
        insecure_rand = FastRandomContext(true);
    }
};

class MCAddrManUncorrupted : public MCAddrManSerializationMock
{
public:
    void Serialize(MCDataStream& s) const override
    {
        MCAddrMan::Serialize(s);
    }
};

class MCAddrManCorrupted : public MCAddrManSerializationMock
{
public:
    void Serialize(MCDataStream& s) const override
    {
        // Produces corrupt output that claims addrman has 20 addrs when it only has one addr.
        unsigned char nVersion = 1;
        s << nVersion;
        s << ((unsigned char)32);
        s << nKey;
        s << 10; // nNew
        s << 10; // nTried

        int nUBuckets = ADDRMAN_NEW_BUCKET_COUNT ^ (1 << 30);
        s << nUBuckets;

        MCService serv;
        Lookup("252.1.1.1", serv, 7777, false);
        MCAddress addr = MCAddress(serv, NODE_NONE);
        MCNetAddr resolved;
        LookupHost("252.2.2.2", resolved, false);
        MCAddrInfo info = MCAddrInfo(addr, resolved);
        s << info;
    }
};

MCDataStream AddrmanToStream(MCAddrManSerializationMock& _addrman)
{
    MCDataStream ssPeersIn(SER_DISK, CLIENT_VERSION);
    ssPeersIn << FLATDATA(Params().MessageStart());
    ssPeersIn << _addrman;
    std::string str = ssPeersIn.str();
    std::vector<unsigned char> vchData(str.begin(), str.end());
    return MCDataStream(vchData, SER_DISK, CLIENT_VERSION);
}

BOOST_FIXTURE_TEST_SUITE(net_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(cnode_listen_port)
{
    // test default
    unsigned short port = GetListenPort();
    BOOST_CHECK(port == Params().GetDefaultPort());
    // test set port
    unsigned short altPort = 12345;
    gArgs.SoftSetArg("-port", std::to_string(altPort));
    port = GetListenPort();
    BOOST_CHECK(port == altPort);
}

BOOST_AUTO_TEST_CASE(caddrdb_read)
{
    MCAddrManUncorrupted addrmanUncorrupted;
    addrmanUncorrupted.MakeDeterministic();

    MCService addr1, addr2, addr3;
    Lookup("250.7.1.1", addr1, 8333, false);
    Lookup("250.7.2.2", addr2, 9999, false);
    Lookup("250.7.3.3", addr3, 9999, false);

    // Add three addresses to new table.
    MCService source;
    Lookup("252.5.1.1", source, 8333, false);
    addrmanUncorrupted.Add(MCAddress(addr1, NODE_NONE), source);
    addrmanUncorrupted.Add(MCAddress(addr2, NODE_NONE), source);
    addrmanUncorrupted.Add(MCAddress(addr3, NODE_NONE), source);

    // Test that the de-serialization does not throw an exception.
    MCDataStream ssPeers1 = AddrmanToStream(addrmanUncorrupted);
    bool exceptionThrown = false;
    MCAddrMan addrman1;

    BOOST_CHECK(addrman1.size() == 0);
    try {
        unsigned char pchMsgTmp[4];
        ssPeers1 >> FLATDATA(pchMsgTmp);
        ssPeers1 >> addrman1;
    } catch (const std::exception&) {
        exceptionThrown = true;
    }

    BOOST_CHECK(addrman1.size() == 3);
    BOOST_CHECK(exceptionThrown == false);

    // Test that MCAddrDB::Read creates an addrman with the correct number of addrs.
    MCDataStream ssPeers2 = AddrmanToStream(addrmanUncorrupted);

    MCAddrMan addrman2;
    MCAddrDB adb;
    BOOST_CHECK(addrman2.size() == 0);
    adb.Read(addrman2, ssPeers2);
    BOOST_CHECK(addrman2.size() == 3);
}


BOOST_AUTO_TEST_CASE(caddrdb_read_corrupted)
{
    MCAddrManCorrupted addrmanCorrupted;
    addrmanCorrupted.MakeDeterministic();

    // Test that the de-serialization of corrupted addrman throws an exception.
    MCDataStream ssPeers1 = AddrmanToStream(addrmanCorrupted);
    bool exceptionThrown = false;
    MCAddrMan addrman1;
    BOOST_CHECK(addrman1.size() == 0);
    try {
        unsigned char pchMsgTmp[4];
        ssPeers1 >> FLATDATA(pchMsgTmp);
        ssPeers1 >> addrman1;
    } catch (const std::exception&) {
        exceptionThrown = true;
    }
    // Even through de-serialization failed addrman is not left in a clean state.
    BOOST_CHECK(addrman1.size() == 1);
    BOOST_CHECK(exceptionThrown);

    // Test that MCAddrDB::Read leaves addrman in a clean state if de-serialization fails.
    MCDataStream ssPeers2 = AddrmanToStream(addrmanCorrupted);

    MCAddrMan addrman2;
    MCAddrDB adb;
    BOOST_CHECK(addrman2.size() == 0);
    adb.Read(addrman2, ssPeers2);
    BOOST_CHECK(addrman2.size() == 0);
}

BOOST_AUTO_TEST_CASE(cnode_simple_test)
{
    SOCKET hSocket = INVALID_SOCKET;
    NodeId id = 0;
    int height = 0;

    in_addr ipv4Addr;
    ipv4Addr.s_addr = 0xa0b0c001;
    
    MCAddress addr = MCAddress(MCService(ipv4Addr, 7777), NODE_NETWORK);
    std::string pszDest = "";
    bool fInboundIn = false;

    // Test that fFeeler is false by default.
    std::unique_ptr<MCNode> pnode1(new MCNode(id++, NODE_NETWORK, height, hSocket, addr, 0, 0, MCAddress(), pszDest, fInboundIn));
    BOOST_CHECK(pnode1->fInbound == false);
    BOOST_CHECK(pnode1->fFeeler == false);

    fInboundIn = true;
    std::unique_ptr<MCNode> pnode2(new MCNode(id++, NODE_NETWORK, height, hSocket, addr, 1, 1, MCAddress(), pszDest, fInboundIn));
    BOOST_CHECK(pnode2->fInbound == true);
    BOOST_CHECK(pnode2->fFeeler == false);
}

BOOST_AUTO_TEST_SUITE_END()
