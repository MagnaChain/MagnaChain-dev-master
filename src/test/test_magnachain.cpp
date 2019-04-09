// Copyright (c) 2011-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "test/test_magnachain.h"

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

void MCConnmanTest::AddNode(MCNode& node)
{
    LOCK(g_connman->cs_vNodes);
    g_connman->vNodes.push_back(&node);
}

void MCConnmanTest::ClearNodes()
{
    LOCK(g_connman->cs_vNodes);
    g_connman->vNodes.clear();
}

uint256 insecure_rand_seed = GetRandHash();
FastRandomContext insecure_rand_ctx(insecure_rand_seed);

extern bool fPrintToConsole;
extern void noui_connect();
extern bool SignatureCoinbaseTransaction(int nHeight, const MCKeyStore* keystoreIn, MCMutableTransaction& txNew, MCAmount nValue, const MCScript& scriptPubKey);

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
        gArgs.ForceSetArg("-powtargetspacing", "1");//force change mining space time
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
    const MCChainParams& chainparams = Params();
        // Ideally we'd move all the RPC tests to the functional testing framework
        // instead of unit tests, but for now we need these here.

        RegisterAllCoreRPCCommands(tableRPC);
        ClearDatadirCache();
        pathTemp = fs::temp_directory_path() / strprintf("test_magnachain_%lu_%i", (unsigned long)GetTime(), (int)(InsecureRandRange(100000)));
        fs::create_directories(pathTemp);
        gArgs.ForceSetArg("-datadir", pathTemp.string());

        // Note that because we don't bother running a scheduler thread here,
        // callbacks via MCValidationInterface are unreliable, but that's OK,
        // our unit tests aren't testing multiple parts of the code at once.
        GetMainSignals().RegisterBackgroundSignalScheduler(scheduler);

        mempool.SetSanityCheck(1.0);
        pblocktree = new MCBlockTreeDB(1 << 20, true);
        pcoinsdbview = new MCCoinsViewDB(1 << 23, true);
        pcoinsTip = new MCCoinsViewCache(pcoinsdbview);
		pcoinListDb = new CoinListDB(pcoinsdbview->GetDb());
		mpContractDb = new ContractDataDB(GetDataDir() / "contract", 1 << 23, false, false);
		g_pBranchChainTxRecordsDb = new BranchChainTxRecordsDb(GetDataDir() / "branchchaintx", 1 << 23, false, false);
        g_pBranchTxRecordCache = new BranchChainTxRecordsCache();
        pCoinAmountDB = new CoinAmountDB();
        pCoinAmountCache = new CoinAmountCache(pCoinAmountDB);
        if (!LoadGenesisBlock(chainparams)) {
            throw std::runtime_error("LoadGenesisBlock failed.");
        }
        {
            MCValidationState state;
            if (!ActivateBestChain(state, chainparams)) {
                throw std::runtime_error("ActivateBestChain failed.");
            }
        }
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
        g_connman = std::unique_ptr<MCConnman>(new MCConnman(0x1337, 0x1337)); // Deterministic randomness for tests.
        connman = g_connman.get();
        peerLogic.reset(new PeerLogicValidation(connman, scheduler, &ProcessMessage, &GetLocator));
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
		delete g_pBranchChainTxRecordsDb;
        delete g_pBranchTxRecordCache;
        delete pcoinsdbview;
        delete pblocktree;
        fs::remove_all(pathTemp);
}

TestChain100Setup::TestChain100Setup() : TestingSetup(MCBaseChainParams::REGTEST)
{
    // Generate a 100-block chain:
    coinbaseKey.MakeNewKey(true);
    MCScript scriptPubKey = MCScript() <<  ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;
    for (int i = 0; i < COINBASE_MATURITY; i++)
    {
        std::vector<MCMutableTransaction> noTxns;
        MCBlock b = CreateAndProcessBlock(noTxns, scriptPubKey);
        coinbaseTxns.push_back(*b.vtx[0]);
        //MilliSleep(Params().GetConsensus().nPowTargetSpacing*1000);
        SetMockTime(GetTime() + Params().GetConsensus().nPowTargetSpacing + 10000);
    }
}

extern MCAmount MakeCoinbaseTransaction(MCMutableTransaction& coinbaseTx, MCAmount nFees, MCBlockIndex* pindexPrev, const MCScript& scriptPubKeyIn, const MCChainParams& chainparams);
//
// Create a new block with just given transactions, coinbase paying to
// scriptPubKey, and try to add it to the current chain.
//
MCBlock
TestChain100Setup::CreateAndProcessBlock(const std::vector<MCMutableTransaction>& txns, const MCScript& scriptPubKey)
{
    const MCChainParams& chainparams = Params();
	MCWallet tempWallet;
	tempWallet.AddKey(coinbaseKey);

    //std::unique_ptr<MCBlockTemplate> pblocktemplate = BlockAssembler(chainparams).CreateNewBlock(scriptPubKey, true, &tempWallet);
    ContractContext contractContext;
    std::string strErr;
	std::unique_ptr<MCBlockTemplate> pblocktemplate = BlockAssembler(chainparams).CreateNewBlock(scriptPubKey, &contractContext, true, &tempWallet, pcoinsTip, strErr);
    MCBlock& block = pblocktemplate->block;

    // Replace mempool-selected txns with just coinbase plus passed-in txns:
    block.vtx.resize(1);
    for (const MCMutableTransaction& tx : txns){
        block.vtx.push_back(MakeTransactionRef(tx));
    }
    if (txns.size() > 0)
    {
        MCAmount nFees = 0;
        MCCoinsViewCache view(pcoinsTip);
        for (int i=1; i < block.vtx.size(); i++)
        {
            const MCTransactionRef& ptx = block.vtx[i];
            nFees += view.GetValueIn(*ptx) - ptx->GetValueOut();
        }

        MCBlockIndex* pindexPrev = mapBlockIndex[block.hashPrevBlock];
        MCMutableTransaction coinbaseTx(*block.vtx[0]);
        MCAmount nReward = MakeCoinbaseTransaction(coinbaseTx, nFees, pindexPrev, scriptPubKey, chainparams);
        block.vtx[0] = MakeTransactionRef(std::move(coinbaseTx));
        GenerateCoinbaseCommitment(block, pindexPrev, chainparams.GetConsensus());
    
        int nHeight = pindexPrev->nHeight + 1;
        MCMutableTransaction kSignTx(*block.vtx[0]);
        if (!SignatureCoinbaseTransaction(nHeight, &tempWallet, kSignTx, nReward, scriptPubKey))
            throw std::runtime_error("sign coin base transaction error");
        block.vtx[0] = MakeTransactionRef(std::move(kSignTx));
    }
    // IncrementExtraNonce creates a valid coinbase and merkleRoot
    unsigned int extraNonce = 0;
    IncrementExtraNonce(&block, chainActive.Tip(), extraNonce);

    while (!CheckProofOfWork(block.GetHash(), block.nBits, chainparams.GetConsensus())) ++block.nNonce;

    std::shared_ptr<MCBlock> shared_pblock = std::make_shared<MCBlock>(block);
    ProcessNewBlock(chainparams, shared_pblock, &contractContext, true, nullptr);

    MCBlock result = block;
    return result;
}

TestChain100Setup::~TestChain100Setup()
{
}


MCTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const MCMutableTransaction &tx) {
    MCTransaction txn(tx);
    return FromTx(txn);
}

MCTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const MCTransaction &txn) {
    return MCTxMemPoolEntry(MakeTransactionRef(txn), nFee, nTime, nHeight,
                           spendsCoinbase, sigOpCost, lp, 0);
}
