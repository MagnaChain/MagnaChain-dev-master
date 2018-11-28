// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "validation/checkpoints.h"

#include "chain/chain.h"
#include "chain/chainparams.h"
#include "misc/reverse_iterator.h"
#include "validation/validation.h"
#include "coding/uint256.h"

#include <stdint.h>


namespace Checkpoints {

    CellBlockIndex* GetLastCheckpoint(const CellCheckpointData& data)
    {
        const MapCheckpoints& checkpoints = data.mapCheckpoints;

        for (const MapCheckpoints::value_type& i : reverse_iterate(checkpoints))
        {
            const uint256& hash = i.second;
            BlockMap::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return nullptr;
    }

} // namespace Checkpoints
