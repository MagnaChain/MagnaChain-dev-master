// Copyright (c) 2014-2015 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_CHAINPARAMSBASE_H
#define CELLLINK_CHAINPARAMSBASE_H

#include <memory>
#include <string>
#include <vector>

/**
 * CellBaseChainParams defines the base parameters (shared between celllink-cli and celllinkd)
 * of a given instance of the CellLink system.
 */
class CellBaseChainParams
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
    CellBaseChainParams() {}

    int nRPCPort;
    std::string strDataDir;
};

/**
 * Creates and returns a std::unique_ptr<CellBaseChainParams> of the chosen chain.
 * @returns a CellBaseChainParams* of the chosen chain.
 * @throws a std::runtime_error if the chain is not supported.
 */
std::unique_ptr<CellBaseChainParams> CreateBaseChainParams(const std::string& chain);

/**
 * Append the help messages for the chainparams options to the
 * parameter string.
 */
void AppendParamsHelpMessages(std::string& strUsage, bool debugHelp=true);

/**
 * Return the currently selected parameters. This won't change after app
 * startup, except for unit tests.
 */
const CellBaseChainParams& BaseParams();

/** Sets the params returned by Params() to those for the given network. */
void SelectBaseParams(const std::string& chain);

/**
 * Looks for -regtest, -testnet and returns the appropriate BIP70 chain name.
 * @return CellBaseChainParams::MAX_NETWORK_TYPES if an invalid combination is given. CellBaseChainParams::MAIN by default.
 */
std::string ChainNameFromCommandLine();

#endif // CELLLINK_CHAINPARAMSBASE_H
