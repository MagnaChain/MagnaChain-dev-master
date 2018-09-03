// Copyright (c) 2011-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bench.h"
#include "policy/policy.h"
#include "transaction/txmempool.h"

#include <list>
#include <vector>

static void AddTx(const CellTransaction& tx, const CellAmount& nFee, CellTxMemPool& pool)
{
    int64_t nTime = 0;
    unsigned int nHeight = 1;
    bool spendsCoinbase = false;
    unsigned int sigOpCost = 4;
    LockPoints lp;
    pool.addUnchecked(tx.GetHash(), CellTxMemPoolEntry(
                                        MakeTransactionRef(tx), nFee, nTime, nHeight,
                                        spendsCoinbase, sigOpCost, lp));
}

// Right now this is only testing eviction performance in an extremely small
// mempool. Code needs to be written to generate a much wider variety of
// unique transactions for a more meaningful performance measurement.
static void MempoolEviction(benchmark::State& state)
{
    CellMutableTransaction tx1 = CellMutableTransaction();
    tx1.vin.resize(1);
    tx1.vin[0].scriptSig = CellScript() << OP_1;
    tx1.vout.resize(1);
    tx1.vout[0].scriptPubKey = CellScript() << OP_1 << OP_EQUAL;
    tx1.vout[0].nValue = 10 * COIN;

    CellMutableTransaction tx2 = CellMutableTransaction();
    tx2.vin.resize(1);
    tx2.vin[0].scriptSig = CellScript() << OP_2;
    tx2.vout.resize(1);
    tx2.vout[0].scriptPubKey = CellScript() << OP_2 << OP_EQUAL;
    tx2.vout[0].nValue = 10 * COIN;

    CellMutableTransaction tx3 = CellMutableTransaction();
    tx3.vin.resize(1);
    tx3.vin[0].prevout = CellOutPoint(tx2.GetHash(), 0);
    tx3.vin[0].scriptSig = CellScript() << OP_2;
    tx3.vout.resize(1);
    tx3.vout[0].scriptPubKey = CellScript() << OP_3 << OP_EQUAL;
    tx3.vout[0].nValue = 10 * COIN;

    CellMutableTransaction tx4 = CellMutableTransaction();
    tx4.vin.resize(2);
    tx4.vin[0].prevout.SetNull();
    tx4.vin[0].scriptSig = CellScript() << OP_4;
    tx4.vin[1].prevout.SetNull();
    tx4.vin[1].scriptSig = CellScript() << OP_4;
    tx4.vout.resize(2);
    tx4.vout[0].scriptPubKey = CellScript() << OP_4 << OP_EQUAL;
    tx4.vout[0].nValue = 10 * COIN;
    tx4.vout[1].scriptPubKey = CellScript() << OP_4 << OP_EQUAL;
    tx4.vout[1].nValue = 10 * COIN;

    CellMutableTransaction tx5 = CellMutableTransaction();
    tx5.vin.resize(2);
    tx5.vin[0].prevout = CellOutPoint(tx4.GetHash(), 0);
    tx5.vin[0].scriptSig = CellScript() << OP_4;
    tx5.vin[1].prevout.SetNull();
    tx5.vin[1].scriptSig = CellScript() << OP_5;
    tx5.vout.resize(2);
    tx5.vout[0].scriptPubKey = CellScript() << OP_5 << OP_EQUAL;
    tx5.vout[0].nValue = 10 * COIN;
    tx5.vout[1].scriptPubKey = CellScript() << OP_5 << OP_EQUAL;
    tx5.vout[1].nValue = 10 * COIN;

    CellMutableTransaction tx6 = CellMutableTransaction();
    tx6.vin.resize(2);
    tx6.vin[0].prevout = CellOutPoint(tx4.GetHash(), 1);
    tx6.vin[0].scriptSig = CellScript() << OP_4;
    tx6.vin[1].prevout.SetNull();
    tx6.vin[1].scriptSig = CellScript() << OP_6;
    tx6.vout.resize(2);
    tx6.vout[0].scriptPubKey = CellScript() << OP_6 << OP_EQUAL;
    tx6.vout[0].nValue = 10 * COIN;
    tx6.vout[1].scriptPubKey = CellScript() << OP_6 << OP_EQUAL;
    tx6.vout[1].nValue = 10 * COIN;

    CellMutableTransaction tx7 = CellMutableTransaction();
    tx7.vin.resize(2);
    tx7.vin[0].prevout = CellOutPoint(tx5.GetHash(), 0);
    tx7.vin[0].scriptSig = CellScript() << OP_5;
    tx7.vin[1].prevout = CellOutPoint(tx6.GetHash(), 0);
    tx7.vin[1].scriptSig = CellScript() << OP_6;
    tx7.vout.resize(2);
    tx7.vout[0].scriptPubKey = CellScript() << OP_7 << OP_EQUAL;
    tx7.vout[0].nValue = 10 * COIN;
    tx7.vout[1].scriptPubKey = CellScript() << OP_7 << OP_EQUAL;
    tx7.vout[1].nValue = 10 * COIN;

    CellTxMemPool pool;

    while (state.KeepRunning()) {
        AddTx(tx1, 10000LL, pool);
        AddTx(tx2, 5000LL, pool);
        AddTx(tx3, 20000LL, pool);
        AddTx(tx4, 7000LL, pool);
        AddTx(tx5, 1000LL, pool);
        AddTx(tx6, 1100LL, pool);
        AddTx(tx7, 9000LL, pool);
        pool.TrimToSize(pool.DynamicMemoryUsage() * 3 / 4);
        pool.TrimToSize(GetVirtualTransactionSize(tx1));
    }
}

BENCHMARK(MempoolEviction);
