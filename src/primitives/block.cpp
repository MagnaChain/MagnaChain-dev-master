// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "coding/hash.h"
#include "misc/tinyformat.h"
#include "utils/utilstrencodings.h"
#include "crypto/common.h"

uint256 MCBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

uint256 MCBlockHeader::GetHashNoSignData() const
{
	return SerializeHash(*this, SER_GETHASH | SER_WITHOUT_SIGN);
}
