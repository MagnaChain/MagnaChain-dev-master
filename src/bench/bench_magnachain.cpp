// Copyright (c) 2015-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bench/bench.h"

#include "crypto/sha256.h"
#include "key/key.h"
#include "validation/validation.h"
#include "utils/util.h"
#include "misc/random.h"

int
main(int argc, char** argv)
{
    SHA256AutoDetect();
    RandomInit();
    ECC_Start();
    SetupEnvironment();
    fPrintToDebugLog = false; // don't want to write to debug.log file

    benchmark::BenchRunner::RunAll();

    ECC_Stop();
}
