// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "merkleblock.h"

#include "coding/hash.h"
#include "consensus/consensus.h"
#include "utils/utilstrencodings.h"

MCMerkleBlock::MCMerkleBlock(const MCBlock& block, MCBloomFilter* filter, const std::set<uint256>* txids)
{
    header = block.GetBlockHeader();

    std::vector<bool> vMatch;
    std::vector<uint256> vHashes;

    vMatch.reserve(block.vtx.size());
    vHashes.reserve(block.vtx.size());

    for (unsigned int i = 0; i < block.vtx.size(); i++)
    {
        const uint256& hash = block.vtx[i]->GetHash();
        if (txids && txids->count(hash)) {
            vMatch.push_back(true);
        }
        else if (filter && filter->IsRelevantAndUpdate(*block.vtx[i])) {
            vMatch.push_back(true);
            vMatchedTxn.emplace_back(i, hash);
        }
        else {
            vMatch.push_back(false);
        }
        vHashes.push_back(hash);
    }

    txn = MCPartialMerkleTree(vHashes, vMatch);
}
