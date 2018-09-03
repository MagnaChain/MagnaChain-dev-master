// Copyright (c) 2017 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_RPC_MINING_H
#define CELLLINK_RPC_MINING_H

#include "script/script.h"

class CellKeyStore;
class CellOutput;
class UniValue;

typedef void(*GenerateBlockCB)();
/** Generate blocks (mine) */
UniValue generateBlocks( CellKeyStore* keystoreIn, std::vector<CellOutput>& vecOutputs, int nGenerate, uint64_t nMaxTries, bool keepScript, GenerateBlockCB pf = nullptr, CellChainParams* pp = nullptr, CellCoinsViewCache *pcoinsCache = nullptr );

void GenerateCells(bool fGenerate, int nThreads, const CellChainParams& chainparams);


/** Check bounds on a command line confirm target */
unsigned int ParseConfirmTarget(const UniValue& value);

#endif
