// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "consensus/consensus.h"


/** The maximum allowed size for a serialized block, in bytes (only for buffer size limits) */
const unsigned int MAX_BLOCK_SERIALIZED_SIZE = 10000000;
/** The maximum allowed weight for a block, see BIP 141 (network rule) */
const unsigned int MAX_BLOCK_WEIGHT = 10000000;
/** The maximum allowed number of signature check operations in a block (network rule) */
const uint64_t MAX_BLOCK_SIGOPS_COST = 200000;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
int COINBASE_MATURITY = 500; // @note regtest's maturity is 1, modify in init.cpp

const int WITNESS_SCALE_FACTOR = 4;

const size_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 60; // 60 is the lower bound for the size of a valid serialized MCTransaction
const size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 10; // 10 is the lower bound for the size of a serialized MCTransaction

                                                                                     //TODO: for test,发出前需要改成合适的值 
const MCAmount CreateBranchChainMortgage = 20000 * COIN;// 创建支链抵押初始值
const uint32_t MaxPowForCreateChainMortgage = 16; // (2^16) * CreateBranchChainMortgage = 655360000 COIN

// following non const vars will be change by regtest.
const int BRANCH_CHAIN_CREATE_COIN_MATURITY = 1054080; // 半年才能赎回, 1054080块 * 15s/块 = 183天 . 设定比较长的时间主要防止恶意创建很多支链。
int BRANCH_CHAIN_MATURITY = 4000;// 至少需要 4000 块 * 15s/块 = 1000 分钟 = 16.67 hours
const MCAmount MIN_MINE_BRANCH_MORTGAGE = 1000 * COIN; // 抵押挖矿最小值
int REDEEM_SAFE_HEIGHT = 10800; // 10800 * 8s = 1 day (branch chain block time) 挖矿币安全高度（继续挖矿和赎回需要满足的高度）、举报高度
int REPORT_OUTOF_HEIGHT = 5760; // 5760 * 15s = 1 day
int REPORT_LOCK_COIN_HEIGHT = 60; // 60 * 15s = 15 mins

const uint32_t CUSHION_HEIGHT = 6;// 跨连交易需要在 （BRANCH_CHAIN_MATURITY + 这个缓冲值）成熟度后才把step2的交易发送给目标链，加这个缓冲高度是因为网络节点上链的同步需要时间。
