// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_CHECKPOINTS_H
#define CELLLINK_CHECKPOINTS_H

#include "coding/uint256.h"

#include <map>

class CellBlockIndex;
struct CellCheckpointData;

/**
 * Block-chain checkpoints are compiled-in sanity checks.
 * They are updated every release or three.
 */
namespace Checkpoints
{

//! Returns last CellBlockIndex* in mapBlockIndex that is a checkpoint
CellBlockIndex* GetLastCheckpoint(const CellCheckpointData& data);

} //namespace Checkpoints

#endif // CELLLINK_CHECKPOINTS_H
