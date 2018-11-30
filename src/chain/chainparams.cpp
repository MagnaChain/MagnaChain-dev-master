// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain/chainparams.h"
#include "consensus/merkle.h"

#include "misc/tinyformat.h"
#include "utils/util.h"
#include "utils/utilstrencodings.h"

#include <assert.h>

#include "chain/chainparamsseeds.h"
#include "key/keystore.h"
#include "consensus/tx_verify.h"

SignatureCoinbaseTransactionPf SignatureCoinbaseTransactionPF = nullptr;


static MCBlock CreateGenesisBlock(const char* pszTimestamp, const MCScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const MCAmount& genesisReward)
{
    MCMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = MCScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

	// make new coin base signature
	{
        bool hasstartecc = ECC_HasStarted();
		if (!hasstartecc)
            ECC_Start();
		ECCVerifyHandle handler;

		size_t iCodeSize = strlen(pszTimestamp) -1;
		unsigned char code[BIP32_EXTKEY_SIZE];
		for (int i = 0; i < BIP32_EXTKEY_SIZE; ++i) {
			code[i] = (unsigned char)pszTimestamp[i%iCodeSize];
		}

		MCExtKey kExtKey;
		kExtKey.Decode(code);

		MCBasicKeyStore kKeyStore;
		kKeyStore.AddKeyPubKey(kExtKey.key, kExtKey.key.GetPubKey());

		MCTxDestination kDest(kExtKey.key.GetPubKey().GetID());
		MCScript kScript = GetScriptForDestination(kDest);
		txNew.vout[0].scriptPubKey = kScript;

		(*SignatureCoinbaseTransactionPF)( 0, &kKeyStore, txNew, genesisReward, kScript);
		//assert(CheckCoinbaseSignature( 0, MCTransaction(txNew)));
        if (!hasstartecc)
    		ECC_Stop();
	}


    MCBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

const std::string DEFAULT_TIMESTAMP = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * MCBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   MCTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     MCTxIn(MCOutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     MCTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static MCBlock CreateGenesisBlock(const std::string& pszTimestamp, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const MCAmount& genesisReward)
{
    const MCScript genesisOutputScript = MCScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp.c_str(), genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void MCChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

// key base 58 prefixes
void MCChainParams::InitMainBase58Prefixes()
{
    base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 75); //X
    base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 62); //S
    base58Prefixes[CONTRACT_ADDRESS] = std::vector<unsigned char>(1, 69);
    base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 128);
    base58Prefixes[EXT_PUBLIC_KEY] = { 0x04, 0x88, 0xB2, 0x1E };
    base58Prefixes[EXT_SECRET_KEY] = { 0x04, 0x88, 0xAD, 0xE4 };
}

void MCChainParams::InitTestnetBase58Prefixes()
{
    base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 110);
    base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 195);
    base58Prefixes[CONTRACT_ADDRESS] = std::vector<unsigned char>(1, 199);
    base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
    base58Prefixes[EXT_PUBLIC_KEY] = { 0x04, 0x35, 0x87, 0xCF };
    base58Prefixes[EXT_SECRET_KEY] = { 0x04, 0x35, 0x83, 0x94 };
}

void MCChainParams::InitRegtestBase58Prefixes()
{
    base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 110);
    base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 195);
    base58Prefixes[CONTRACT_ADDRESS] = std::vector<unsigned char>(1, 199);
    base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
    base58Prefixes[EXT_PUBLIC_KEY] = { 0x04, 0x35, 0x87, 0xCF };
    base58Prefixes[EXT_SECRET_KEY] = { 0x04, 0x35, 0x83, 0x94 };
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public MCChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
		consensus.BigBoomHeight = 1000;
		consensus.BigBoomValue = 1500000 * COIN;
        consensus.nSubsidyHalvingInterval = 210000 * 40;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.BIP66Height = 0; // 00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
		consensus.powLimit = uint256S("0xefffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = gArgs.GetArg("-powtargetspacing", MAIN_CHAIN_POW_TARGET_SPACING);
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
		consensus.nRuleChangeActivationThreshold = 10;// 1916; // 95% of 2016
		consensus.nMinerConfirmationWindow = 10;// 2016; // nPowTargetTimespan / nPowTargetSpacing
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00"); //477890

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xce;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x16;
        pchMessageStart[3] = 0x89;
        nDefaultPort = 8833;
        nPruneAfterHeight = 100000;

		genesis = CreateGenesisBlock(DEFAULT_TIMESTAMP, 1231006505, 2, 0x207fffff, 1, 10 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"));
        //assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

		// Note that of those which support the service bits prefix, most only support a subset of
		// possible options.
		// This is fine at runtime as we'll fall back to using them as a oneshot if they don't support the
		// service bits we want, but we should get them updated to support all service bits wanted by any
		// release ASAP to avoid it where possible.

		vSeeds.emplace_back("seed.magnachainseed.io", false);

        InitMainBase58Prefixes();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        //checkpointData = (MCCheckpointData) {
        //    {
        //        // { 11111, uint256S("0x0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d")},
        //        // { 33333, uint256S("0x000000002dd5588a74784eaa7ab0507a18ad16a236e7b1ce69f00d7ddfb5d0a6")},
        //        // { 74000, uint256S("0x0000000000573993a3c9e41ce34471c079dcf5f52a0e824a81e7f953b8661a20")},
        //        // {105000, uint256S("0x00000000000291ce28027faea320c8d2b054b2e0fe44a773f3eefb151d6bdc97")},
        //        // {134444, uint256S("0x00000000000005b12ffd4cd315cd34ffd4a594f430ac814c91184a0d42d2b0fe")},
        //        // {168000, uint256S("0x000000000000099e61ea72015e79632f216fe6cb33d7899acb35b75c8303b763")},
        //        // {193000, uint256S("0x000000000000059f452a5f7340de6682a977387c17010ff6e6c3bd83ca8b1317")},
        //        // {210000, uint256S("0x000000000000048b95347e83192f69cf0366076336c639f9b7228e9ba171342e")},
        //        // {216116, uint256S("0x00000000000001b4f4b433e81ee46494af945cf96014816a4e2370f11b23df4e")},
        //        // {225430, uint256S("0x00000000000001c108384350f74090433e7fcf79a606b8e797f065b130575932")},
        //        // {250000, uint256S("0x000000000000003887df1f29024b06fc2200b55f8af8f35453d7be294df2d214")},
        //        // {279000, uint256S("0x0000000000000001ae8c72a0b0c301f67e3afca10e819efa9041e458e9bd7e40")},
        //        // {295000, uint256S("0x00000000000000004d9b4ef50f0f9d686fd69db2e03af35a100370c64632a983")},
        //    }
        //};

        chainTxData = ChainTxData{
            // Data as of block 000000000000000000d97e53664d17967bd4ee50b23abb92e54a34eb222d15ae (height 478913).
            0, // * UNIX timestamp of last known number of transactions
            0,  // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0,         // * estimated number of transactions per second after that timestamp
        };
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public MCChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
		consensus.BigBoomHeight = 1000;
		consensus.BigBoomValue = 1500000 * COIN;
		consensus.nSubsidyHalvingInterval = 210000 * 20;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0; // 00000000007f6655f22f98e72ed80d8b06dc761d5da09df0fa1dc4be4f861eb6
        consensus.BIP66Height = 0; // 000000002104c8c45e99a8853285a3b592602a3ccde2b832481da85e9e4ba182
        consensus.powLimit = uint256S("0xefffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
		consensus.nPowTargetSpacing = gArgs.GetArg("-powtargetspacing", TEST_CHAIN_POW_TARGET_SPACING);
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 10; // 75% for testchains
        consensus.nMinerConfirmationWindow = 10; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0; 
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;


        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00"); //1135275

        pchMessageStart[0] = 0xce;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x07;
        nDefaultPort = 18833;
        nPruneAfterHeight = 1000;

		
        genesis = CreateGenesisBlock(DEFAULT_TIMESTAMP, 1296688602, 1, 0x207fffff, 1, 10 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943"));
        //assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

		vFixedSeeds.clear();
		vSeeds.clear();
		// nodes with support for servicebits filtering should be at the top
		vSeeds.emplace_back("seedtest.magnachainseed.io", false);

        InitTestnetBase58Prefixes();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        //checkpointData = (MCCheckpointData) {
        //    {
        //        {546, uint256S("000000002a936ca763904c3c35fce2f3556c559c0214345d31b1bcebf76acb70")},
        //    }
        //};

        chainTxData = ChainTxData{
            // Data as of block 00000000000001c200b9790dc637d3bb141fe77d155b966ed775b17e109f7c6c (height 1156179)
            1501802953,
            14706531,
            0.15
        };

    }
};

/**
 * Regression test
 */
class CRegTestParams : public MCChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
		consensus.BigBoomHeight = 1000;
		consensus.BigBoomValue = 1500000 * COIN;
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 0; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 0; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("0xefffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
		consensus.nPowTargetSpacing = gArgs.GetArg("-powtargetspacing", TEST_CHAIN_POW_TARGET_SPACING);
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 10; // 75% for testchains
        consensus.nMinerConfirmationWindow = 10; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xce;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18844;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(DEFAULT_TIMESTAMP, 1296688602, 2, 0x207fffff, 1, 10 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"));
        //assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        //checkpointData = (MCCheckpointData) {
        //    {
        //        {0, uint256S("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")},
        //    }
        //};

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        InitRegtestBase58Prefixes();
    }
};

class MCBranchParams : public MCChainParams {
public:
	MCBranchParams(const std::string& strBranchIdParams = "") {
		strNetworkID = "branch";
		consensus.BigBoomHeight = 0;
		consensus.BigBoomValue = 0 * COIN;
		consensus.nSubsidyHalvingInterval = 210000 * 20;
		consensus.BIP34Height = 0;
		consensus.BIP34Hash = uint256();
		consensus.BIP65Height = 0; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
		consensus.BIP66Height = 0; // 00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
		consensus.powLimit = uint256S("0xefffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
		consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
		consensus.nPowTargetSpacing = gArgs.GetArg("-powtargetspacing", BRANCH_CHAIN_POW_TARGET_SPACING);
		consensus.fPowAllowMinDifficultyBlocks = false;
		consensus.fPowNoRetargeting = false;
		consensus.nRuleChangeActivationThreshold = 10;// 1916; // 95% of 2016
		consensus.nMinerConfirmationWindow = 10;// 2016; // nPowTargetTimespan / nPowTargetSpacing
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

		// The best chain should have at least this much work.
		consensus.nMinimumChainWork = uint256S("0x00");

		// By default assume that the signatures in ancestors of this block are valid.
		consensus.defaultAssumeValid = uint256S("0x00"); //477890

		/**
		* The message start string is designed to be unlikely to occur in normal data.
		* The characters are rarely used upper ASCII, not valid as UTF-8, and produce
		* a large 32-bit integer with any alignment.
		*/
		pchMessageStart[0] = 0xce;
		pchMessageStart[1] = 0x11;
		pchMessageStart[2] = 0x68;
		pchMessageStart[3] = 0x99;
		nDefaultPort = 28833;
		nPruneAfterHeight = 100000;

		//change branch dir
        strBranchId = !strBranchIdParams.empty() ? strBranchIdParams : gArgs.GetArg("-branchid", "");
		if (strBranchId == MCBaseChainParams::MAIN)
			throw std::runtime_error("main chain no need this option -branchid");
		if (strBranchId.length() != 64 || !IsHex(strBranchId))
			throw std::runtime_error(strprintf("%s: Invalid branch id %s, it must a txid", __func__, strBranchId.c_str()));

        branchhash.SetHex(strBranchId);

		gArgs.SoftSetArg("datadir", "branch" + strBranchId);
		std::string strTimestamp = "branch_" + strBranchId;
		genesis = CreateGenesisBlock(strTimestamp, 1231006505, 2, 0x207fffff, 1, 10 * COIN);
		consensus.hashGenesisBlock = genesis.GetHash();

		//assert(consensus.hashGenesisBlock == uint256S("0x000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"));
		//assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        //侧链跟主链的key一致,根据不同网络进行切换
        bool fRegTest = gArgs.GetBoolArg("-regtest", false);
        bool fTestNet = gArgs.GetBoolArg("-testnet", false);
        InitMainBase58Prefixes();
        if (fTestNet)
            InitTestnetBase58Prefixes();
        else if (fRegTest)
            InitRegtestBase58Prefixes();

		vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));


		fDefaultConsistencyChecks = false;
		fRequireStandard = true;
		fMineBlocksOnDemand = false;

		chainTxData = ChainTxData{
			// Data as of block 000000000000000000d97e53664d17967bd4ee50b23abb92e54a34eb222d15ae (height 478913).
			0, // * UNIX timestamp of last known number of transactions
			0,  // * total number of transactions between genesis and that timestamp
				//   (the tx=... number in the SetBestChain debug.log lines)
			0,         // * estimated number of transactions per second after that timestamp
		};
	}
};

static std::unique_ptr<MCChainParams> globalChainParams;
static std::map<uint256, std::unique_ptr<MCChainParams>> g_mapBranchParams;

const MCChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

const MCChainParams& BranchParams(const uint256& branchHash)
{
    if (!g_mapBranchParams.count(branchHash))
    {
        g_mapBranchParams[branchHash] = std::unique_ptr<MCChainParams>(new MCBranchParams(branchHash.GetHex()));
    }
    return *g_mapBranchParams[branchHash];
}

std::unique_ptr<MCChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == MCBaseChainParams::MAIN)
        return std::unique_ptr<MCChainParams>(new CMainParams());
    else if (chain == MCBaseChainParams::TESTNET)
        return std::unique_ptr<MCChainParams>(new CTestNetParams());
    else if (chain == MCBaseChainParams::REGTEST)
        return std::unique_ptr<MCChainParams>(new CRegTestParams());
	else if (chain == MCBaseChainParams::BRANCH)
		return std::unique_ptr<MCChainParams>(new MCBranchParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

static std::vector< std::shared_ptr<MCChainParams> > vecParams;

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);

	std::vector<std::string> vseeds = gArgs.GetArgs("-vseeds");
	if (vseeds.size() > 0)
	{
		globalChainParams->ClearDNSSeeds();
		for (const std::string& var : vseeds)
			globalChainParams->PushBackDNSSeeds(var);
	}

	std::vector<std::string> ssp6s = gArgs.GetArgs("-seedspec6");
	if (ssp6s.size() > 0)
	{
		globalChainParams->ClearFixedSeeds();
		for (const std::string& var : ssp6s)
		{
			SeedSpec6 ssp6;
			if (StringToSeedSpec6(var, ssp6) == false)
				throw std::runtime_error(strprintf("%s: can not convert string %s to SeedSpec6", __func__, var));
			globalChainParams->PushFixedSeeds(ssp6);
		}
	}
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
