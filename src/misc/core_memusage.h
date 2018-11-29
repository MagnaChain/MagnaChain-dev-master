// Copyright (c) 2015-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_CORE_MEMUSAGE_H
#define MAGNACHAIN_CORE_MEMUSAGE_H

#include "primitives/transaction.h"
#include "primitives/block.h"
#include "memusage.h"

static inline size_t RecursiveDynamicUsage(const MCScript& script) {
    return memusage::DynamicUsage(script);
}

static inline size_t RecursiveDynamicUsage(const MCOutPoint& out) {
    return 0;
}

static inline size_t RecursiveDynamicUsage(const MCTxIn& in) {
    size_t mem = RecursiveDynamicUsage(in.scriptSig) + RecursiveDynamicUsage(in.prevout) + memusage::DynamicUsage(in.scriptWitness.stack);
    for (std::vector<std::vector<unsigned char> >::const_iterator it = in.scriptWitness.stack.begin(); it != in.scriptWitness.stack.end(); it++) {
         mem += memusage::DynamicUsage(*it);
    }
    return mem;
}

static inline size_t RecursiveDynamicUsage(const MCTxOut& out) {
    return RecursiveDynamicUsage(out.scriptPubKey);
}

static inline size_t RecursiveDynamicUsage(const MCTransaction& tx) {
    size_t mem = memusage::DynamicUsage(tx.vin) + memusage::DynamicUsage(tx.vout);
    for (std::vector<MCTxIn>::const_iterator it = tx.vin.begin(); it != tx.vin.end(); it++) {
        mem += RecursiveDynamicUsage(*it);
    }
    for (std::vector<MCTxOut>::const_iterator it = tx.vout.begin(); it != tx.vout.end(); it++) {
        mem += RecursiveDynamicUsage(*it);
    }
    return mem;
}

static inline size_t RecursiveDynamicUsage(const MCMutableTransaction& tx) {
    size_t mem = memusage::DynamicUsage(tx.vin) + memusage::DynamicUsage(tx.vout);
    for (std::vector<MCTxIn>::const_iterator it = tx.vin.begin(); it != tx.vin.end(); it++) {
        mem += RecursiveDynamicUsage(*it);
    }
    for (std::vector<MCTxOut>::const_iterator it = tx.vout.begin(); it != tx.vout.end(); it++) {
        mem += RecursiveDynamicUsage(*it);
    }
    return mem;
}

static inline size_t RecursiveDynamicUsage(const MCBlock& block) {
    size_t mem = memusage::DynamicUsage(block.vtx);
    for (const auto& tx : block.vtx) {
        mem += memusage::DynamicUsage(tx) + RecursiveDynamicUsage(*tx);
    }
    return mem;
}

static inline size_t RecursiveDynamicUsage(const MCBlockLocator& locator) {
    return memusage::DynamicUsage(locator.vHave);
}

template<typename X>
static inline size_t RecursiveDynamicUsage(const std::shared_ptr<X>& p) {
    return p ? memusage::DynamicUsage(p) + RecursiveDynamicUsage(*p) : 0;
}

#endif // MAGNACHAIN_CORE_MEMUSAGE_H
