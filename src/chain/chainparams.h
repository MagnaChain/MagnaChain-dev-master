// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_CHAINPARAMS_H
#define MAGNACHAIN_CHAINPARAMS_H

#include "chainparamsbase.h"
#include "consensus/params.h"
#include "primitives/block.h"
#include "net/protocol.h"

#include <memory>
#include <vector>

static const int MAIN_CHAIN_POW_TARGET_SPACING = 10;
static const int BRANCH_CHAIN_POW_TARGET_SPACING = 8;
static const int TEST_CHAIN_POW_TARGET_SPACING = 5;

struct MCDNSSeedData {
    std::string host;
    bool supportsServiceBitsFiltering;
    MCDNSSeedData(const std::string &strHost, bool supportsServiceBitsFilteringIn) : host(strHost), supportsServiceBitsFiltering(supportsServiceBitsFilteringIn) {}
};

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

typedef std::map<int, uint256> MapCheckpoints;

struct MCCheckpointData {
    MapCheckpoints mapCheckpoints;
};

struct ChainTxData {
    int64_t nTime;
    int64_t nTxCount;
    double dTxRate;
};

/**
 * MCChainParams defines various tweakable parameters of a given instance of the
 * MagnaChain system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class MCChainParams
{
public:
    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,
        EXT_PUBLIC_KEY,
        EXT_SECRET_KEY,
        CONTRACT_ADDRESS,

        MAX_BASE58_TYPES
    };

    const Consensus::Params& GetConsensus() const { return consensus; }
    const MCMessageHeader::MessageStartChars& MessageStart() const { return pchMessageStart; }
    int GetDefaultPort() const { return nDefaultPort; }

    const MCBlock& GenesisBlock() const { return genesis; }
    /** Default value for -checkmempool and -checkblockindex argument */
    bool DefaultConsistencyChecks() const { return fDefaultConsistencyChecks; }
    /** Policy: Filter transactions that do not match well-defined patterns */
    bool RequireStandard() const { return fRequireStandard; }
    uint64_t PruneAfterHeight() const { return nPruneAfterHeight; }
    /** Make miner stop after a block is found. In RPC, don't return until nGenProcLimit blocks are generated */
    bool MineBlocksOnDemand() const { return fMineBlocksOnDemand; }
    /** Return the BIP70 network string (main, test or regtest) */
    std::string NetworkIDString() const { return strNetworkID; }
    const std::vector<MCDNSSeedData>& DNSSeeds() const { return vSeeds; }
	void ClearDNSSeeds() { vSeeds.clear(); }
	void PushBackDNSSeeds(const std::string strSeed) { vSeeds.emplace_back(strSeed, false); }
    const std::vector<unsigned char>& Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    const std::vector<SeedSpec6>& FixedSeeds() const { return vFixedSeeds; }
	void ClearFixedSeeds() { vFixedSeeds.clear(); }
	void PushFixedSeeds(const SeedSpec6 &ssp) { vFixedSeeds.emplace_back(ssp); }
    const MCCheckpointData& Checkpoints() const { return checkpointData; }
    const ChainTxData& TxData() const { return chainTxData; }
    void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout);

	//void SetBranchId(const std::string& name) { strBranchId = name; }
	std::string GetBranchId(void) const { return strBranchId; }
    inline bool IsMainChain() const {
        return GetBranchId() == MCBaseChainParams::MAIN;
    }
    const uint256& GetBranchHash()const { return branchhash; }

    void InitMainBase58Prefixes();
    void InitTestnetBase58Prefixes();
    void InitRegtestBase58Prefixes();
protected:
    MCChainParams() {}

    Consensus::Params consensus;
    MCMessageHeader::MessageStartChars pchMessageStart;
    int nDefaultPort;
    uint64_t nPruneAfterHeight;
    std::vector<MCDNSSeedData> vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
    std::string strNetworkID;
    MCBlock genesis;
    std::vector<SeedSpec6> vFixedSeeds;
    bool fDefaultConsistencyChecks;
    bool fRequireStandard;
    bool fMineBlocksOnDemand;
    MCCheckpointData checkpointData;
    ChainTxData chainTxData;
	std::string strBranchId = MCBaseChainParams::MAIN;
    uint256 branchhash;
};

/**
 * Creates and returns a std::unique_ptr<MCChainParams> of the chosen chain.
 * @returns a MCChainParams* of the chosen chain.
 * @throws a std::runtime_error if the chain is not supported.
 */
std::unique_ptr<MCChainParams> CreateChainParams(const std::string& chain);

/**
 * Return the currently selected parameters. This won't change after app
 * startup, except for unit tests.
 */
const MCChainParams &Params();

const MCChainParams& BranchParams(const uint256& branchHash);
/**
 * Sets the params returned by Params() to those for the given BIP70 chain name.
 * @throws std::runtime_error when the chain is not supported.
 */
void SelectParams(const std::string& chain);

/**
 * Allows modifying the Version Bits regtest parameters.
 */
void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout);

class MCKeyStore;
class MCMutableTransaction;
class MCScript;

typedef bool(*SignatureCoinbaseTransactionPf)(int nHeight, const MCKeyStore* keystoreIn, MCMutableTransaction& tx, MCAmount nValue, const MCScript& scriptPubKey);
extern SignatureCoinbaseTransactionPf SignatureCoinbaseTransactionPF;

#endif // MAGNACHAIN_CHAINPARAMS_H
