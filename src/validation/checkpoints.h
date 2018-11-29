// Copyright (c) 2009-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_CHECKPOINTS_H
#define MAGNACHAIN_CHECKPOINTS_H

#include "coding/uint256.h"

#include <map>

class MCBlockIndex;
struct MCCheckpointData;

/**
 * Block-chain checkpoints are compiled-in sanity checks.
 * They are updated every release or three.
 */
namespace Checkpoints
{

//! Returns last MCBlockIndex* in mapBlockIndex that is a checkpoint
MCBlockIndex* GetLastCheckpoint(const MCCheckpointData& data);

} //namespace Checkpoints

#endif // MAGNACHAIN_CHECKPOINTS_H
