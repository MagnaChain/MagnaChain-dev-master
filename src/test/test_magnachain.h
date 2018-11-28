// Copyright (c) 2015-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_TEST_TEST_CELLLINK_H
#define CELLLINK_TEST_TEST_CELLLINK_H

#include "chain/chainparamsbase.h"
#include "io/fs.h"
#include "key/key.h"
#include "key/pubkey.h"
#include "misc/random.h"
#include "thread/scheduler.h"
#include "transaction/txdb.h"
#include "transaction/txmempool.h"

#include <boost/thread.hpp>

extern uint256 insecure_rand_seed;
extern FastRandomContext insecure_rand_ctx;

static inline void SeedInsecureRand(bool fDeterministic = false)
{
    if (fDeterministic) {
        insecure_rand_seed = uint256();
    } else {
        insecure_rand_seed = GetRandHash();
    }
    insecure_rand_ctx = FastRandomContext(insecure_rand_seed);
}

static inline uint32_t InsecureRand32() { return insecure_rand_ctx.rand32(); }
static inline uint256 InsecureRand256() { return insecure_rand_ctx.rand256(); }
static inline uint64_t InsecureRandBits(int bits) { return insecure_rand_ctx.randbits(bits); }
static inline uint64_t InsecureRandRange(uint64_t range) { return insecure_rand_ctx.randrange(range); }
static inline bool InsecureRandBool() { return insecure_rand_ctx.randbool(); }

/** Basic testing setup.
 * This just configures logging and chain parameters.
 */
struct BasicTestingSetup {
    ECCVerifyHandle globalVerifyHandle;

    BasicTestingSetup(const std::string& chainName = MCBaseChainParams::MAIN);
    ~BasicTestingSetup();
};

/** Testing setup that configures a complete environment.
 * Included are data directory, coins database, script check threads setup.
 */
class MCConnman;
class MCNode;
struct MCConnmanTest {
    static void AddNode(MCNode& node);
    static void ClearNodes();
};

class PeerLogicValidation;
struct TestingSetup: public BasicTestingSetup {
    MCCoinsViewDB *pcoinsdbview;
    fs::path pathTemp;
    boost::thread_group threadGroup;
    MCConnman* connman;
    MCScheduler scheduler;
    std::unique_ptr<PeerLogicValidation> peerLogic;

    TestingSetup(const std::string& chainName = MCBaseChainParams::MAIN);
    ~TestingSetup();
};

class MCBlock;
struct MCMutableTransaction;
class MCScript;

//
// Testing fixture that pre-creates a
// 100-block REGTEST-mode block chain
//
struct TestChain100Setup : public TestingSetup {
    TestChain100Setup();

    // Create a new block with just given transactions, coinbase paying to
    // scriptPubKey, and try to add it to the current chain.
    MCBlock CreateAndProcessBlock(const std::vector<MCMutableTransaction>& txns,
                                 const MCScript& scriptPubKey);

    ~TestChain100Setup();

    std::vector<MCTransaction> coinbaseTxns; // For convenience, coinbase transactions
    MCKey coinbaseKey; // private/public key needed to spend coinbase transactions
};

class MCTxMemPoolEntry;

struct TestMemPoolEntryHelper
{
    // Default values
    MCAmount nFee;
    int64_t nTime;
    unsigned int nHeight;
    bool spendsCoinbase;
    unsigned int sigOpCost;
    LockPoints lp;

    TestMemPoolEntryHelper() :
        nFee(0), nTime(0), nHeight(1),
        spendsCoinbase(false), sigOpCost(4) { }
    
    MCTxMemPoolEntry FromTx(const MCMutableTransaction &tx);
    MCTxMemPoolEntry FromTx(const MCTransaction &tx);

    // Change the default value
    TestMemPoolEntryHelper &Fee(MCAmount _fee) { nFee = _fee; return *this; }
    TestMemPoolEntryHelper &Time(int64_t _time) { nTime = _time; return *this; }
    TestMemPoolEntryHelper &Height(unsigned int _height) { nHeight = _height; return *this; }
    TestMemPoolEntryHelper &SpendsCoinbase(bool _flag) { spendsCoinbase = _flag; return *this; }
    TestMemPoolEntryHelper &SigOpsCost(unsigned int _sigopsCost) { sigOpCost = _sigopsCost; return *this; }
};
#endif
