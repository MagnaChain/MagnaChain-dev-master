// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "coding/hash.h"
#include "misc/tinyformat.h"
#include "utils/utilstrencodings.h"
#include "crypto/common.h"

uint256 CellBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

uint256 CellBlockHeader::GetHashNoSignData() const
{
	return SerializeHash(*this, SER_GETHASH | SER_WITHOUT_SIGN);
}

std::string CellBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CellBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
