// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_POW_H
#define CELLLINK_POW_H

#include "consensus/params.h"

#include <stdint.h>

class CellBlockHeader;
class CellBlockIndex;
class uint256;

unsigned int GetNextWorkRequired(const CellBlockIndex* pindexLast, const CellBlockHeader *pblock, const Consensus::Params&);
unsigned int CalculateNextWorkRequired(const CellBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params&);

/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params&);

#endif // CELLLINK_POW_H
