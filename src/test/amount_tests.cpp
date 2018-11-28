// Copyright (c) 2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "misc/amount.h"
#include "policy/feerate.h"
#include "test/test_celllink.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(amount_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(MoneyRangeTest)
{
    BOOST_CHECK_EQUAL(MoneyRange(CellAmount(-1)), false);
    BOOST_CHECK_EQUAL(MoneyRange(MAX_MONEY + CellAmount(1)), false);
    BOOST_CHECK_EQUAL(MoneyRange(CellAmount(1)), true);
}

BOOST_AUTO_TEST_CASE(GetFeeTest)
{
    CellFeeRate feeRate, altFeeRate;

    feeRate = CellFeeRate(0);
    // Must always return 0
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1e5), 0);

    feeRate = CellFeeRate(1000);
    // Must always just return the arg
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1), 1);
    BOOST_CHECK_EQUAL(feeRate.GetFee(121), 121);
    BOOST_CHECK_EQUAL(feeRate.GetFee(999), 999);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1e3), 1e3);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9e3), 9e3);

    feeRate = CellFeeRate(-1000);
    // Must always just return -1 * arg
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1), -1);
    BOOST_CHECK_EQUAL(feeRate.GetFee(121), -121);
    BOOST_CHECK_EQUAL(feeRate.GetFee(999), -999);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1e3), -1e3);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9e3), -9e3);

    feeRate = CellFeeRate(123);
    // Truncates the result, if not integer
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(8), 1); // Special case: returns 1 instead of 0
    BOOST_CHECK_EQUAL(feeRate.GetFee(9), 1);
    BOOST_CHECK_EQUAL(feeRate.GetFee(121), 14);
    BOOST_CHECK_EQUAL(feeRate.GetFee(122), 15);
    BOOST_CHECK_EQUAL(feeRate.GetFee(999), 122);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1e3), 123);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9e3), 1107);

    feeRate = CellFeeRate(-123);
    // Truncates the result, if not integer
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), 0);
    BOOST_CHECK_EQUAL(feeRate.GetFee(8), -1); // Special case: returns -1 instead of 0
    BOOST_CHECK_EQUAL(feeRate.GetFee(9), -1);

    // check alternate constructor
    feeRate = CellFeeRate(1000);
    altFeeRate = CellFeeRate(feeRate);
    BOOST_CHECK_EQUAL(feeRate.GetFee(100), altFeeRate.GetFee(100));

    // Check full constructor
    // default value
    BOOST_CHECK(CellFeeRate(CellAmount(-1), 1000) == CellFeeRate(-1));
    BOOST_CHECK(CellFeeRate(CellAmount(0), 1000) == CellFeeRate(0));
    BOOST_CHECK(CellFeeRate(CellAmount(1), 1000) == CellFeeRate(1));
    // lost precision (can only resolve satoshis per kB)
    BOOST_CHECK(CellFeeRate(CellAmount(1), 1001) == CellFeeRate(0));
    BOOST_CHECK(CellFeeRate(CellAmount(2), 1001) == CellFeeRate(1));
    // some more integer checks
    BOOST_CHECK(CellFeeRate(CellAmount(26), 789) == CellFeeRate(32));
    BOOST_CHECK(CellFeeRate(CellAmount(27), 789) == CellFeeRate(34));
    // Maximum size in bytes, should not crash
    CellFeeRate(MAX_MONEY, std::numeric_limits<size_t>::max() >> 1).GetFeePerK();
}

BOOST_AUTO_TEST_CASE(BinaryOperatorTest)
{
    CellFeeRate a, b;
    a = CellFeeRate(1);
    b = CellFeeRate(2);
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
    CellFeeRate feeRate;
    feeRate = CellFeeRate(1);
    BOOST_CHECK_EQUAL(feeRate.ToString(), "0.00000001 CELL/kB");
}

BOOST_AUTO_TEST_SUITE_END()
