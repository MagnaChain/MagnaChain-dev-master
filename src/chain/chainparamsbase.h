// Copyright (c) 2014-2015 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_CHAINPARAMSBASE_H
#define MAGNACHAIN_CHAINPARAMSBASE_H

#include <memory>
#include <string>
#include <vector>

/**
 * MCBaseChainParams defines the base parameters (shared between magnachain-cli and magnachaind)
 * of a given instance of the MagnaChain system.
 */
class MCBaseChainParams
{
public:
    /** BIP70 chain name strings (main, test or regtest) */
    static const std::string MAIN;
    static const std::string TESTNET;
    static const std::string REGTEST;
	static const std::string BRANCH;

    const std::string& DataDir() const { return strDataDir; }
    int RPCPort() const { return nRPCPort; }

protected:
    MCBaseChainParams() {}

    int nRPCPort;
    std::string strDataDir;
};

/**
 * Creates and returns a std::unique_ptr<MCBaseChainParams> of the chosen chain.
 * @returns a MCBaseChainParams* of the chosen chain.
 * @throws a std::runtime_error if the chain is not supported.
 */
std::unique_ptr<MCBaseChainParams> CreateBaseChainParams(const std::string& chain);

/**
 * Append the help messages for the chainparams options to the
 * parameter string.
 */
void AppendParamsHelpMessages(std::string& strUsage, bool debugHelp=true);

/**
 * Return the currently selected parameters. This won't change after app
 * startup, except for unit tests.
 */
const MCBaseChainParams& BaseParams();

/** Sets the params returned by Params() to those for the given network. */
void SelectBaseParams(const std::string& chain);

/**
 * Looks for -regtest, -testnet and returns the appropriate BIP70 chain name.
 * @return MCBaseChainParams::MAX_NETWORK_TYPES if an invalid combination is given. MCBaseChainParams::MAIN by default.
 */
std::string ChainNameFromCommandLine();

#endif // MAGNACHAIN_CHAINPARAMSBASE_H
