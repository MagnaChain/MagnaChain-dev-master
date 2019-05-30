// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_CONSENSUS_CONSENSUS_H
#define MAGNACHAIN_CONSENSUS_CONSENSUS_H

#include <stdlib.h>
#include <stdint.h>
#include "misc/amount.h"

/** The maximum allowed size for a serialized block, in bytes (only for buffer size limits) */
extern const unsigned int MAX_BLOCK_SERIALIZED_SIZE;
/** The maximum allowed weight for a block, see BIP 141 (network rule) */
extern const unsigned int MAX_BLOCK_WEIGHT;
/** The maximum allowed number of signature check operations in a block (network rule) */
extern const uint64_t MAX_BLOCK_SIGOPS_COST;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
extern int32_t COINBASE_MATURITY; // @note regtest's maturity is 1, modify in init.cpp

extern const int WITNESS_SCALE_FACTOR;

extern const size_t MIN_TRANSACTION_WEIGHT; // 60 is the lower bound for the size of a valid serialized MCTransaction
extern const size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT; // 10 is the lower bound for the size of a serialized MCTransaction

//TODO: for test,发出前需要改成合适的值 
extern const MCAmount CreateBranchChainMortgage;// 创建支链抵押初始值
extern const uint32_t MaxPowForCreateChainMortgage; // (2^16) * CreateBranchChainMortgage = 655360000 COIN

extern const int32_t BRANCH_CHAIN_CREATE_COIN_MATURITY; // 半年才能赎回, 527040块 * 30s/块 = 183天 . 设定比较长的时间主要防止恶意创建很多支链。
extern int32_t BRANCH_CHAIN_MATURITY;// 至少需要 2000 块 * 30s/块 = 1000 分钟 = 16.67 hours
extern const MCAmount MIN_MINE_BRANCH_MORTGAGE; // 抵押挖矿最小值
extern int32_t REDEEM_SAFE_HEIGHT; // 10800 * 8s = 1 day (branch chain block time) 挖矿币安全高度（继续挖矿和赎回需要满足的高度）、举报高度
extern int32_t REPORT_OUTOF_HEIGHT; // 2880 * 30s = 1 day
extern int32_t REPORT_LOCK_COIN_HEIGHT; // 30 * 30s = 15 mins

extern const uint32_t CUSHION_HEIGHT;// 跨连交易需要在 （BRANCH_CHAIN_MATURITY + 这个缓冲值）成熟度后才把step2的交易发送给目标链，加这个缓冲高度是因为网络节点上链的同步需要时间。

/** Flags for nSequence and nLockTime locks */
enum {
    /* Interpret sequence numbers as relative lock-time constraints. */
    LOCKTIME_VERIFY_SEQUENCE = (1 << 0),

    /* Use GetMedianTimePast() instead of nTime for end point timestamp. */
    LOCKTIME_MEDIAN_TIME_PAST = (1 << 1),
};

#endif // MAGNACHAIN_CONSENSUS_CONSENSUS_H
