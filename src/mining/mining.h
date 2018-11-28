// Copyright (c) 2017 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_RPC_MINING_H
#define CELLLINK_RPC_MINING_H

#include "script/script.h"

class MCKeyStore;
class MCOutput;
class UniValue;

typedef void(*GenerateBlockCB)();
/** Generate blocks (mine) */
UniValue generateBlocks(MCWallet* keystoreIn, std::vector<MCOutput>& vecOutputs, int nGenerate, uint64_t nMaxTries, bool keepScript, GenerateBlockCB pf = nullptr, MCChainParams* pp = nullptr, MCCoinsViewCache *pcoinsCache = nullptr );

void GenerateMCs(bool fGenerate, int nThreads, const MCChainParams& chainparams);


/** Check bounds on a command line confirm target */
unsigned int ParseConfirmTarget(const UniValue& value);

#endif
