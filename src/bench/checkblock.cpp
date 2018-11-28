// Copyright (c) 2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bench/bench.h"

#include "chain/chainparams.h"
#include "validation/validation.h"
#include "io/streams.h"
#include "consensus/validation.h"
#include "chain/branchdb.h"

namespace block_bench {
#include "bench/data/block413567.raw.h"
} // namespace block_bench

// These are the two major time-sinks which happen after we have fully received
// a block off the wire, but before we can relay the block on to peers using
// compact block relay.

static void DeserializeBlockTest(benchmark::State& state)
{
    CellDataStream stream((const char*)block_bench::block413567,
            (const char*)&block_bench::block413567[sizeof(block_bench::block413567)],
            SER_NETWORK, PROTOCOL_VERSION);
    char a = '\0';
    stream.write(&a, 1); // Prevent compaction

    while (state.KeepRunning()) {
        CellBlock block;
        stream >> block;
        assert(stream.Rewind(sizeof(block_bench::block413567)));
    }
}

static void DeserializeAndCheckBlockTest(benchmark::State& state)
{
    CellDataStream stream((const char*)block_bench::block413567,
            (const char*)&block_bench::block413567[sizeof(block_bench::block413567)],
            SER_NETWORK, PROTOCOL_VERSION);
    char a = '\0';
    stream.write(&a, 1); // Prevent compaction

    const auto chainParams = CreateChainParams(CellBaseChainParams::MAIN);
    BranchCache branhcache(nullptr);
    while (state.KeepRunning()) {
        CellBlock block; // Note that CBlock caches its checked state, so we need to recreate it here
        stream >> block;
        assert(stream.Rewind(sizeof(block_bench::block413567)));

        CellValidationState validationState;
        assert(CheckBlock(block, validationState, chainParams->GetConsensus(), &branhcache));
    }
}

BENCHMARK(DeserializeBlockTest);
BENCHMARK(DeserializeAndCheckBlockTest);
