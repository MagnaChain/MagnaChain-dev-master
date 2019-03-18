// Copyright (c) 2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "misc/amount.h"
#include "policy/feerate.h"
#include "test/test_magnachain.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(amount_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(MoneyRangeTest)
{
    BOOST_CHECK_EQUAL(MoneyRange(MCAmount(-1)), false);
    BOOST_CHECK_EQUAL(MoneyRange(MAX_MONEY + MCAmount(1)), false);
    BOOST_CHECK_EQUAL(MoneyRange(MCAmount(1)), true);
}

BOOST_AUTO_TEST_CASE(GetFeeTest)
{
    MCFeeRate feeRate, altFeeRate;

    feeRate = MCFeeRate(0);
    // Must always return 0
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1e5), 0);

    feeRate = MCFeeRate(1000);
    // Must always just return the arg
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1), 1);
    BOOST_CHECK_EQUAL(feeRate.GetFee(121), 121);
    BOOST_CHECK_EQUAL(feeRate.GetFee(999), 999);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1e3), 1e3);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9e3), 9e3);

    feeRate = MCFeeRate(-1000);
    // Must always just return -1 * arg
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1), -1);
    BOOST_CHECK_EQUAL(feeRate.GetFee(121), -121);
    BOOST_CHECK_EQUAL(feeRate.GetFee(999), -999);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1e3), -1e3);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9e3), -9e3);

    feeRate = MCFeeRate(123);
    // Truncates the result, if not integer
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(8), 1); // Special case: returns 1 instead of 0
    BOOST_CHECK_EQUAL(feeRate.GetFee(9), 1);
    BOOST_CHECK_EQUAL(feeRate.GetFee(121), 14);
    BOOST_CHECK_EQUAL(feeRate.GetFee(122), 15);
    BOOST_CHECK_EQUAL(feeRate.GetFee(999), 122);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1e3), 123);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9e3), 1107);

    feeRate = MCFeeRate(-123);
    // Truncates the result, if not integer
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(8), -1); // Special case: returns -1 instead of 0
    BOOST_CHECK_EQUAL(feeRate.GetFee(9), -1);

    // check alternate constructor
    feeRate = MCFeeRate(1000);
    altFeeRate = MCFeeRate(feeRate);
    BOOST_CHECK_EQUAL(feeRate.GetFee(100), altFeeRate.GetFee(100));

    // Check full constructor
    // default value
    BOOST_CHECK(MCFeeRate(MCAmount(-1), 1000) == MCFeeRate(-1));
    BOOST_CHECK(MCFeeRate(MCAmount(0), 1000) == MCFeeRate(0));
    BOOST_CHECK(MCFeeRate(MCAmount(1), 1000) == MCFeeRate(1));
    // lost precision (can only resolve satoshis per kB)
    BOOST_CHECK(MCFeeRate(MCAmount(1), 1001) == MCFeeRate(0));
    BOOST_CHECK(MCFeeRate(MCAmount(2), 1001) == MCFeeRate(1));
    // some more integer checks
    BOOST_CHECK(MCFeeRate(MCAmount(26), 789) == MCFeeRate(32));
    BOOST_CHECK(MCFeeRate(MCAmount(27), 789) == MCFeeRate(34));
    // Maximum size in bytes, should not crash
    MCFeeRate(MAX_MONEY, std::numeric_limits<size_t>::max() >> 1).GetFeePerK();
}

BOOST_AUTO_TEST_CASE(BinaryOperatorTest)
{
    MCFeeRate a, b;
    a = MCFeeRate(1);
    b = MCFeeRate(2);
    BOOST_CHECK(a < b);
    BOOST_CHECK(b > a);
    BOOST_CHECK(a == a);
    BOOST_CHECK(a <= b);
    BOOST_CHECK(a <= a);
    BOOST_CHECK(b >= a);
    BOOST_CHECK(b >= b);
    // a should be 0.00000002 BTC/kB now
    a += a;
    BOOST_CHECK(a == b);
}

BOOST_AUTO_TEST_CASE(ToStringTest)
{
    MCFeeRate feeRate;
    feeRate = MCFeeRate(1);
    BOOST_CHECK_EQUAL(feeRate.ToString(), "0.00000001 MGC/kB");
}

BOOST_AUTO_TEST_SUITE_END()
