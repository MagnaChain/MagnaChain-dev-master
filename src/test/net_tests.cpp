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

class CellAddrManSerializationMock : public CellAddrMan
{
public:
    virtual void Serialize(CellDataStream& s) const = 0;

    //! Ensure that bucket placement is always the same for testing purposes.
    void MakeDeterministic()
    {
        nKey.SetNull();
        insecure_rand = FastRandomContext(true);
    }
};

class CellAddrManUncorrupted : public CellAddrManSerializationMock
{
public:
    void Serialize(CellDataStream& s) const override
    {
        CellAddrMan::Serialize(s);
    }
};

class CellAddrManCorrupted : public CellAddrManSerializationMock
{
public:
    void Serialize(CellDataStream& s) const override
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

        CellService serv;
        Lookup("252.1.1.1", serv, 7777, false);
        CellAddress addr = CellAddress(serv, NODE_NONE);
        CellNetAddr resolved;
        LookupHost("252.2.2.2", resolved, false);
        CellAddrInfo info = CellAddrInfo(addr, resolved);
        s << info;
    }
};

CellDataStream AddrmanToStream(CellAddrManSerializationMock& _addrman)
{
    CellDataStream ssPeersIn(SER_DISK, CLIENT_VERSION);
    ssPeersIn << FLATDATA(Params().MessageStart());
    ssPeersIn << _addrman;
    std::string str = ssPeersIn.str();
    std::vector<unsigned char> vchData(str.begin(), str.end());
    return CellDataStream(vchData, SER_DISK, CLIENT_VERSION);
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
    CellAddrManUncorrupted addrmanUncorrupted;
    addrmanUncorrupted.MakeDeterministic();

    CellService addr1, addr2, addr3;
    Lookup("250.7.1.1", addr1, 8333, false);
    Lookup("250.7.2.2", addr2, 9999, false);
    Lookup("250.7.3.3", addr3, 9999, false);

    // Add three addresses to new table.
    CellService source;
    Lookup("252.5.1.1", source, 8333, false);
    addrmanUncorrupted.Add(CellAddress(addr1, NODE_NONE), source);
    addrmanUncorrupted.Add(CellAddress(addr2, NODE_NONE), source);
    addrmanUncorrupted.Add(CellAddress(addr3, NODE_NONE), source);

    // Test that the de-serialization does not throw an exception.
    CellDataStream ssPeers1 = AddrmanToStream(addrmanUncorrupted);
    bool exceptionThrown = false;
    CellAddrMan addrman1;

    BOOST_CHECK(addrman1.size() == 0);
    try {
        unsigned char pchMsgTmp[4];
        ssPeers1 >> FLATDATA(pchMsgTmp);
        ssPeers1 >> addrman1;
    } catch (const std::exception& e) {
        exceptionThrown = true;
    }

    BOOST_CHECK(addrman1.size() == 3);
    BOOST_CHECK(exceptionThrown == false);

    // Test that CellAddrDB::Read creates an addrman with the correct number of addrs.
    CellDataStream ssPeers2 = AddrmanToStream(addrmanUncorrupted);

    CellAddrMan addrman2;
    CellAddrDB adb;
    BOOST_CHECK(addrman2.size() == 0);
    adb.Read(addrman2, ssPeers2);
    BOOST_CHECK(addrman2.size() == 3);
}


BOOST_AUTO_TEST_CASE(caddrdb_read_corrupted)
{
    CellAddrManCorrupted addrmanCorrupted;
    addrmanCorrupted.MakeDeterministic();

    // Test that the de-serialization of corrupted addrman throws an exception.
    CellDataStream ssPeers1 = AddrmanToStream(addrmanCorrupted);
    bool exceptionThrown = false;
    CellAddrMan addrman1;
    BOOST_CHECK(addrman1.size() == 0);
    try {
        unsigned char pchMsgTmp[4];
        ssPeers1 >> FLATDATA(pchMsgTmp);
        ssPeers1 >> addrman1;
    } catch (const std::exception& e) {
        exceptionThrown = true;
    }
    // Even through de-serialization failed addrman is not left in a clean state.
    BOOST_CHECK(addrman1.size() == 1);
    BOOST_CHECK(exceptionThrown);

    // Test that CellAddrDB::Read leaves addrman in a clean state if de-serialization fails.
    CellDataStream ssPeers2 = AddrmanToStream(addrmanCorrupted);

    CellAddrMan addrman2;
    CellAddrDB adb;
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
    
    CellAddress addr = CellAddress(CellService(ipv4Addr, 7777), NODE_NETWORK);
    std::string pszDest = "";
    bool fInboundIn = false;

    // Test that fFeeler is false by default.
    std::unique_ptr<CellNode> pnode1(new CellNode(id++, NODE_NETWORK, height, hSocket, addr, 0, 0, CellAddress(), pszDest, fInboundIn));
    BOOST_CHECK(pnode1->fInbound == false);
    BOOST_CHECK(pnode1->fFeeler == false);

    fInboundIn = true;
    std::unique_ptr<CellNode> pnode2(new CellNode(id++, NODE_NETWORK, height, hSocket, addr, 1, 1, CellAddress(), pszDest, fInboundIn));
    BOOST_CHECK(pnode2->fInbound == true);
    BOOST_CHECK(pnode2->fFeeler == false);
}

BOOST_AUTO_TEST_SUITE_END()
