// Copyright (c) 2017 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_RPC_BLOCKCHAIN_H
#define CELLLINK_RPC_BLOCKCHAIN_H

class MCBlock;
class MCBlockIndex;
class UniValue;

/**
 * Get the difficulty of the net wrt to the given block index, or the chain tip if
 * not provided.
 *
 * @return A floating point number that is a multiple of the main net minimum
 * difficulty (4295032833 hashes).
 */
double GetDifficulty(const MCBlockIndex* blockindex = nullptr);

/** Callback for when block tip changed. */
void RPCNotifyBlockChange(bool ibd, const MCBlockIndex *);

/** Block description to JSON */
UniValue blockToJSON(const MCBlock& block, const MCBlockIndex* blockindex, bool txDetails = false, bool txDetailsOut = true);

/** Mempool information to JSON */
UniValue mempoolInfoToJSON();

/** Mempool to JSON */
UniValue mempoolToJSON(bool fVerbose = false);

/** Block header to JSON */
UniValue blockheaderToJSON(const MCBlockIndex* blockindex);

#endif

