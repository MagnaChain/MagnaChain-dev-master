// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_CONSENSUS_CONSENSUS_H
#define MAGNACHAIN_CONSENSUS_CONSENSUS_H

#include <stdlib.h>
#include <stdint.h>
#include "misc/amount.h"

/** The maximum allowed size for a serialized block, in bytes (only for buffer size limits) */
static const unsigned int MAX_BLOCK_SERIALIZED_SIZE = 16000000;
/** The maximum allowed weight for a block, see BIP 141 (network rule) */
static const unsigned int MAX_BLOCK_WEIGHT = 64000000;
/** The maximum allowed number of signature check operations in a block (network rule) */
static const int64_t MAX_BLOCK_SIGOPS_COST = 480000;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static int COINBASE_MATURITY = 500;

static const int WITNESS_SCALE_FACTOR = 4;

static const size_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 60; // 60 is the lower bound for the size of a valid serialized MCTransaction
static const size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 10; // 10 is the lower bound for the size of a serialized MCTransaction

//TODO: for test,发出前需要改成合适的值 
static const MCAmount CreateBranchChainMortgage = 20000 * COIN;// 创建支链抵押初始值
static const uint32_t MaxPowForCreateChainMortgage = 16; // (2^16) * CreateBranchChainMortgage = 655360000 COIN

static const int32_t BRANCH_CHAIN_CREATE_COIN_MATURITY = 527040; // 半年才能赎回, 527040块 * 30s/块 = 183天 . 设定比较长的时间主要防止恶意创建很多支链。
static const uint32_t BRANCH_CHAIN_MATURITY = 2000;// 至少需要 2000 块 * 30s/块 = 1000 分钟 = 16.67 hours
static const MCAmount MIN_MINE_BRANCH_MORTGAGE = 100 * COIN; // 抵押挖矿最小值
static const uint32_t REDEEM_SAFE_HEIGHT = 10800; // 10800 * 8s = 1 day (branch chain block time) 挖矿币安全高度（继续挖矿和赎回需要满足的高度）、举报高度
static const uint32_t REPORT_OUTOF_HEIGHT = 2880; // 2880 * 30s = 1 day
const uint32_t REPORT_LOCK_COIN_HEIGHT = 30; // 30 * 30s = 15 mins

const uint32_t CUSHION_HEIGHT = 6;// 跨连交易需要在 （BRANCH_CHAIN_MATURITY + 这个缓冲值）成熟度后才把step2的交易发送给目标链，加这个缓冲高度是因为网络节点上链的同步需要时间。

/** Flags for nSequence and nLockTime locks */
enum {
    /* Interpret sequence numbers as relative lock-time constraints. */
    LOCKTIME_VERIFY_SEQUENCE = (1 << 0),

    /* Use GetMedianTimePast() instead of nTime for end point timestamp. */
    LOCKTIME_MEDIAN_TIME_PAST = (1 << 1),
};

#endif // MAGNACHAIN_CONSENSUS_CONSENSUS_H
