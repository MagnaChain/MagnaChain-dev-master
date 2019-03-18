// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_POW_H
#define MAGNACHAIN_POW_H

#include "consensus/params.h"

#include <stdint.h>

class MCBlockHeader;
class MCBlockIndex;
class uint256;

unsigned int GetNextWorkRequired(const MCBlockIndex* pindexLast, const MCBlockHeader *pblock, const Consensus::Params&);
unsigned int CalculateNextWorkRequired(const MCBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params&);

/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params&);

#endif // MAGNACHAIN_POW_H
