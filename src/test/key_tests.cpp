// Copyright (c) 2012-2015 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key/key.h"

#include "coding/base58.h"
#include "script/script.h"
#include "coding/uint256.h"
#include "utils/util.h"
#include "utils/utilstrencodings.h"
#include "test/test_magnachain.h"

#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

static const std::string strSecret1     ("5KGiYSmx759rbBHw73cVzXmNhKzFbGbQx7MyiQ3Vq4xtkjQWKUB");
static const std::string strSecret2     ("5JdC4gErqmQqNzUDrCE4vt56GWfoTvbJQvs4WxeRWyYcmn2DAqp");
static const std::string strSecret1C    ("L3eNMnmQDoL5srRjogXgxvnrkzohQFptiF5bKii2FAVhRhU21GUE");
static const std::string strSecret2C    ("KznjqECAzF2CY4f9i8YgLpR4FTxG5qYPrcjxXEcBDJaYtPtTh6DK");
static const MagnaChainAddress addr1 ("XFJqmerKAacLBqGq1Fc5So5AiVyjY6z7az");
static const MagnaChainAddress addr2 ("XTWjWzJLepUUEizBTVNGL8rHszhfdC7gXW");
static const MagnaChainAddress addr1C("XYymxsAfYWYMpB35CzYLyBMwoSeYUw1yQb");
static const MagnaChainAddress addr2C("XV4aR4Dpj4KykCFkV15Ez6dPZN2hcVW1VJ");


static const std::string strAddressBad("1HV9Lc3sNHZxwj4Zk6fB38tEmBryq2cBiF");


BOOST_FIXTURE_TEST_SUITE(key_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(key_test1)
{
    MagnaChainSecret bsecret1, bsecret2, bsecret1C, bsecret2C, baddress1;
    BOOST_CHECK( bsecret1.SetString (strSecret1));
    BOOST_CHECK( bsecret2.SetString (strSecret2));
    BOOST_CHECK( bsecret1C.SetString(strSecret1C));
    BOOST_CHECK( bsecret2C.SetString(strSecret2C));
    BOOST_CHECK(!baddress1.SetString(strAddressBad));

    MCKey key1  = bsecret1.GetKey();
    BOOST_CHECK(key1.IsCompressed() == false);
    MCKey key2  = bsecret2.GetKey();
    BOOST_CHECK(key2.IsCompressed() == false);
    MCKey key1C = bsecret1C.GetKey();
    BOOST_CHECK(key1C.IsCompressed() == true);
    MCKey key2C = bsecret2C.GetKey();
    BOOST_CHECK(key2C.IsCompressed() == true);

    MCPubKey pubkey1  = key1. GetPubKey();
    MCPubKey pubkey2  = key2. GetPubKey();
    MCPubKey pubkey1C = key1C.GetPubKey();
    MCPubKey pubkey2C = key2C.GetPubKey();

    BOOST_CHECK(key1.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key1C.VerifyPubKey(pubkey1));
    BOOST_CHECK(key1C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey1C));
    BOOST_CHECK(key2.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey2));
    BOOST_CHECK(key2C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(addr1.Get()  == MCTxDestination(pubkey1.GetID()));
    BOOST_CHECK(addr2.Get()  == MCTxDestination(pubkey2.GetID()));
    BOOST_CHECK(addr1C.Get() == MCTxDestination(pubkey1C.GetID()));
    BOOST_CHECK(addr2C.Get() == MCTxDestination(pubkey2C.GetID()));

    for (int n=0; n<16; n++)
    {
        std::string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        std::vector<unsigned char> sign1, sign2, sign1C, sign2C;

        BOOST_CHECK(key1.Sign (hashMsg, sign1));
        BOOST_CHECK(key2.Sign (hashMsg, sign2));
        BOOST_CHECK(key1C.Sign(hashMsg, sign1C));
        BOOST_CHECK(key2C.Sign(hashMsg, sign2C));

        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2));
        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1C));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2C));

        BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2));
        BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1C));
        BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2C));

        // compact signatures (with key recovery)

        std::vector<unsigned char> csign1, csign2, csign1C, csign2C;

        BOOST_CHECK(key1.SignCompact (hashMsg, csign1));
        BOOST_CHECK(key2.SignCompact (hashMsg, csign2));
        BOOST_CHECK(key1C.SignCompact(hashMsg, csign1C));
        BOOST_CHECK(key2C.SignCompact(hashMsg, csign2C));

        MCPubKey rkey1, rkey2, rkey1C, rkey2C;

        BOOST_CHECK(rkey1.RecoverCompact (hashMsg, csign1));
        BOOST_CHECK(rkey2.RecoverCompact (hashMsg, csign2));
        BOOST_CHECK(rkey1C.RecoverCompact(hashMsg, csign1C));
        BOOST_CHECK(rkey2C.RecoverCompact(hashMsg, csign2C));

        BOOST_CHECK(rkey1  == pubkey1);
        BOOST_CHECK(rkey2  == pubkey2);
        BOOST_CHECK(rkey1C == pubkey1C);
        BOOST_CHECK(rkey2C == pubkey2C);
    }

    // test deterministic signing

    std::vector<unsigned char> detsig, detsigc;
    std::string strMsg = "Very deterministic message";
    uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());
    BOOST_CHECK(key1.Sign(hashMsg, detsig));
    BOOST_CHECK(key1C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc); 
    BOOST_CHECK(detsig == ParseHex("3045022100d1efc587f0b2827da8fcb264a12ab89344d689e42c7f921ec85a0dd6be1fffc0022068a6161ee46bca74d7fdc30fdee35659eefdb2a08bca5a90b7be2e384208d50d"));
    BOOST_CHECK(key2.Sign(hashMsg, detsig));
    BOOST_CHECK(key2C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc);
    BOOST_CHECK(detsig == ParseHex("304402200bf830d6d3fc753580f086fb44946bfa22d7271616f7c24fb5117c34388d0b5702207f0461dd0a560bc2cf1479b97db0f02c4e1393fb22b300f7f7029252a9b1a835"));
    BOOST_CHECK(key1.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key1C.SignCompact(hashMsg, detsigc));
    BOOST_CHECK(detsig == ParseHex("1bd1efc587f0b2827da8fcb264a12ab89344d689e42c7f921ec85a0dd6be1fffc068a6161ee46bca74d7fdc30fdee35659eefdb2a08bca5a90b7be2e384208d50d"));
    BOOST_CHECK(detsigc == ParseHex("1fd1efc587f0b2827da8fcb264a12ab89344d689e42c7f921ec85a0dd6be1fffc068a6161ee46bca74d7fdc30fdee35659eefdb2a08bca5a90b7be2e384208d50d"));
    BOOST_CHECK(key2.SignCompact(hashMsg, detsig)); 
    BOOST_CHECK(key2C.SignCompact(hashMsg, detsigc)); 
    BOOST_CHECK(detsig == ParseHex("1b0bf830d6d3fc753580f086fb44946bfa22d7271616f7c24fb5117c34388d0b577f0461dd0a560bc2cf1479b97db0f02c4e1393fb22b300f7f7029252a9b1a835"));
    BOOST_CHECK(detsigc == ParseHex("1f0bf830d6d3fc753580f086fb44946bfa22d7271616f7c24fb5117c34388d0b577f0461dd0a560bc2cf1479b97db0f02c4e1393fb22b300f7f7029252a9b1a835"));
}

BOOST_AUTO_TEST_SUITE_END()
