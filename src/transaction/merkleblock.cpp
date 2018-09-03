// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "merkleblock.h"

#include "coding/hash.h"
#include "consensus/consensus.h"
#include "utils/utilstrencodings.h"

CellMerkleBlock::CellMerkleBlock(const CellBlock& block, CellBloomFilter& filter)
{
    header = block.GetBlockHeader();

    std::vector<bool> vMatch;
    std::vector<uint256> vHashes;

    vMatch.reserve(block.vtx.size());
    vHashes.reserve(block.vtx.size());

    for (unsigned int i = 0; i < block.vtx.size(); i++)
    {
        const uint256& hash = block.vtx[i]->GetHash();
        if (filter.IsRelevantAndUpdate(*block.vtx[i]))
        {
            vMatch.push_back(true);
            vMatchedTxn.push_back(std::make_pair(i, hash));
        }
        else
            vMatch.push_back(false);
        vHashes.push_back(hash);
    }

    txn = CellPartialMerkleTree(vHashes, vMatch);
}

CellMerkleBlock::CellMerkleBlock(const CellBlock& block, const std::set<uint256>& txids)
{
    header = block.GetBlockHeader();

    std::vector<bool> vMatch;
    std::vector<uint256> vHashes;

    vMatch.reserve(block.vtx.size());
    vHashes.reserve(block.vtx.size());

    for (unsigned int i = 0; i < block.vtx.size(); i++)
    {
        const uint256& hash = block.vtx[i]->GetHash();
        if (txids.count(hash))
            vMatch.push_back(true);
        else
            vMatch.push_back(false);
        vHashes.push_back(hash);
    }

    txn = CellPartialMerkleTree(vHashes, vMatch);
}
