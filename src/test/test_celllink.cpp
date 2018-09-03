// Copyright (c) 2011-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "test/test_celllink.h"

#include "chain/chainparams.h"
#include "consensus/consensus.h"
#include "consensus/validation.h"
#include "crypto/sha256.h"
#include "io/fs.h"
#include "key/key.h"
#include "validation/validation.h"
#include "mining/miner.h"
#include "net/net_processing.h"
#include "key/pubkey.h"
#include "misc/random.h"
#include "transaction/txdb.h"
#include "transaction/txmempool.h"
#include "ui/ui_interface.h"
#include "rpc/server.h"
#include "rpc/register.h"
#include "script/sigcache.h"
#include "wallet/wallet.h"

#include <memory>

void CellConnmanTest::AddNode(CellNode& node)
{
    LOCK(g_connman->cs_vNodes);
    g_connman->vNodes.push_back(&node);
}

void CellConnmanTest::ClearNodes()
{
    LOCK(g_connman->cs_vNodes);
    g_connman->vNodes.clear();
}

uint256 insecure_rand_seed = GetRandHash();
FastRandomContext insecure_rand_ctx(insecure_rand_seed);

extern bool fPrintToConsole;
extern void noui_connect();
extern bool SignatureCoinbaseTransaction(int nHeight, const CellKeyStore* keystoreIn, CellMutableTransaction& txNew, CellAmount nValue, const CellScript& scriptPubKey);

BasicTestingSetup::BasicTestingSetup(const std::string& chainName)
{
		SignatureCoinbaseTransactionPF = &SignatureCoinbaseTransaction;

        SHA256AutoDetect();
        RandomInit();
        ECC_Start();
        SetupEnvironment();
        SetupNetworking();
        InitSignatureCache();
        InitScriptExecutionCache();
        fPrintToDebugLog = false; // don't want to write to debug.log file
        fCheckBlockIndex = true;
		ECC_Stop();// in SelectParams has a pair function call(ECC_Start and ECC_Stop)
        SelectParams(chainName);
		ECC_Start();
        noui_connect();
}

BasicTestingSetup::~BasicTestingSetup()
{
        ECC_Stop();
}

TestingSetup::TestingSetup(const std::string& chainName) : BasicTestingSetup(chainName)
{
    const CellChainParams& chainparams = Params();
        // Ideally we'd move all the RPC tests to the functional testing framework
        // instead of unit tests, but for now we need these here.

        RegisterAllCoreRPCCommands(tableRPC);
        ClearDatadirCache();
        pathTemp = fs::temp_directory_path() / strprintf("test_celllink_%lu_%i", (unsigned long)GetTime(), (int)(InsecureRandRange(100000)));
        fs::create_directories(pathTemp);
        gArgs.ForceSetArg("-datadir", pathTemp.string());

        // Note that because we don't bother running a scheduler thread here,
        // callbacks via CellValidationInterface are unreliable, but that's OK,
        // our unit tests aren't testing multiple parts of the code at once.
        GetMainSignals().RegisterBackgroundSignalScheduler(scheduler);

        mempool.setSanityCheck(1.0);
        pblocktree = new CellBlockTreeDB(1 << 20, true);
        pcoinsdbview = new CellCoinsViewDB(1 << 23, true);
        pcoinsTip = new CellCoinsViewCache(pcoinsdbview);
		pcoinListDb = new CoinListDB(pcoinsdbview->GetDb());
		mpContractDb = new ContractDataDB(GetDataDir() / "contract", 1 << 23, false, false);
		pBranchChainTxRecordsDb = new BranchChainTxRecordsDb(GetDataDir() / "branchchaintx", 1 << 23, false, false);
        if (!LoadGenesisBlock(chainparams)) {
            throw std::runtime_error("LoadGenesisBlock failed.");
        }
        {
            CellValidationState state;
            if (!ActivateBestChain(state, chainparams)) {
                throw std::runtime_error("ActivateBestChain failed.");
            }
        }
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
        g_connman = std::unique_ptr<CellConnman>(new CellConnman(0x1337, 0x1337)); // Deterministic randomness for tests.
        connman = g_connman.get();
        peerLogic.reset(new PeerLogicValidation(connman, scheduler));
}

TestingSetup::~TestingSetup()
{
        threadGroup.interrupt_all();
        threadGroup.join_all();
        GetMainSignals().FlushBackgroundCallbacks();
        GetMainSignals().UnregisterBackgroundSignalScheduler();
        g_connman.reset();
        peerLogic.reset();
        UnloadBlockIndex();
		delete pcoinListDb;
		delete pcoinsTip;
		delete mpContractDb;
		delete pBranchChainTxRecordsDb;
        delete pcoinsdbview;
        delete pblocktree;
        fs::remove_all(pathTemp);
}

TestChain100Setup::TestChain100Setup() : TestingSetup(CellBaseChainParams::REGTEST)
{
    // Generate a 100-block chain:
    coinbaseKey.MakeNewKey(true);
    CellScript scriptPubKey = CellScript() <<  ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;
    for (int i = 0; i < COINBASE_MATURITY; i++)
    {
        std::vector<CellMutableTransaction> noTxns;
        CellBlock b = CreateAndProcessBlock(noTxns, scriptPubKey);
        coinbaseTxns.push_back(*b.vtx[0]);
    }
}

//
// Create a new block with just given transactions, coinbase paying to
// scriptPubKey, and try to add it to the current chain.
//
CellBlock
TestChain100Setup::CreateAndProcessBlock(const std::vector<CellMutableTransaction>& txns, const CellScript& scriptPubKey)
{
    const CellChainParams& chainparams = Params();
	//CellWallet tempWallet;
	//tempWallet.AddKey(coinbaseKey);
    //std::unique_ptr<CellBlockTemplate> pblocktemplate = BlockAssembler(chainparams).CreateNewBlock(scriptPubKey, true, &tempWallet);
	std::unique_ptr<CellBlockTemplate> pblocktemplate = BlockAssembler(chainparams).CreateNewBlock(scriptPubKey);
    CellBlock& block = pblocktemplate->block;

    // Replace mempool-selected txns with just coinbase plus passed-in txns:
    block.vtx.resize(1);
    for (const CellMutableTransaction& tx : txns)
        block.vtx.push_back(MakeTransactionRef(tx));
    // IncrementExtraNonce creates a valid coinbase and merkleRoot
    unsigned int extraNonce = 0;
    IncrementExtraNonce(&block, chainActive.Tip(), extraNonce);

    while (!CheckProofOfWork(block.GetHash(), block.nBits, chainparams.GetConsensus())) ++block.nNonce;

    std::shared_ptr<const CellBlock> shared_pblock = std::make_shared<const CellBlock>(block);
    ProcessNewBlock(chainparams, shared_pblock, true, nullptr);

    CellBlock result = block;
    return result;
}

TestChain100Setup::~TestChain100Setup()
{
}


CellTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CellMutableTransaction &tx) {
    CellTransaction txn(tx);
    return FromTx(txn);
}

CellTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CellTransaction &txn) {
    return CellTxMemPoolEntry(MakeTransactionRef(txn), nFee, nTime, nHeight,
                           spendsCoinbase, sigOpCost, lp);
}
