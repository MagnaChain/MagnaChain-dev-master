// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_MERKLEBLOCK_H
#define CELLLINK_MERKLEBLOCK_H

#include "io/serialize.h"
#include "coding/uint256.h"
#include "primitives/block.h"
#include "transaction/bloom.h"
#include "transaction/partialmerkletree.h"
#include <vector>

/**
 * Used to relay blocks as header + vector<merkle branch>
 * to filtered nodes.
 *
 * NOTE: The class assumes that the given CellBlock has *at least* 1 transaction. If the CellBlock has 0 txs, it will hit an assertion.
 */
class CellMerkleBlock
{
public:
    /** Public only for unit testing */
    CellBlockHeader header;
    CellPartialMerkleTree txn;

public:
    /** Public only for unit testing and relay testing (not relayed) */
    std::vector<std::pair<unsigned int, uint256> > vMatchedTxn;

    /**
     * Create from a CellBlock, filtering transactions according to filter
     * Note that this will call IsRelevantAndUpdate on the filter for each transaction,
     * thus the filter will likely be modified.
     */
    CellMerkleBlock(const CellBlock& block, CellBloomFilter& filter);

    // Create from a CellBlock, matching the txids in the set
    CellMerkleBlock(const CellBlock& block, const std::set<uint256>& txids);

    CellMerkleBlock() {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(header);
        READWRITE(txn);
    }
};

#endif // CELLLINK_MERKLEBLOCK_H
