// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain/chainparamsbase.h"

#include "misc/tinyformat.h"
#include "utils/util.h"

#include <assert.h>

const std::string MCBaseChainParams::MAIN = "main";
const std::string MCBaseChainParams::TESTNET = "test";
const std::string MCBaseChainParams::REGTEST = "regtest";
const std::string MCBaseChainParams::BRANCH = "branch";

void AppendParamsHelpMessages(std::string& strUsage, bool debugHelp)
{
    strUsage += HelpMessageGroup(_("Chain selection options:"));
    strUsage += HelpMessageOpt("-testnet", _("Use the test chain"));
    if (debugHelp) {
        strUsage += HelpMessageOpt("-regtest", "Enter regression test mode, which uses a special chain in which blocks can be solved instantly. "
                                   "This is intended for regression testing tools and app development.");
    }
}

/**
 * Main network
 */
class MCBaseMainParams : public MCBaseChainParams
{
public:
    MCBaseMainParams()
    {
        nRPCPort = 8332;
    }
};

/**
 * Testnet (v3)
 */
class MCBaseTestNetParams : public MCBaseChainParams
{
public:
    MCBaseTestNetParams()
    {
        nRPCPort = 18332;
        strDataDir = "testnet3";
    }
};

/*
 * Regression test
 */
class MCBaseRegTestParams : public MCBaseChainParams
{
public:
    MCBaseRegTestParams()
    {
        nRPCPort = 18332;
        strDataDir = "regtest";
    }
};

class MCBaseBranchParams : public MCBaseChainParams
{
public:
	MCBaseBranchParams()
	{
		nRPCPort = 8332;
	//	strDataDir = "branch";
	}
};

static std::unique_ptr<MCBaseChainParams> globalChainBaseParams;

const MCBaseChainParams& BaseParams()
{
    assert(globalChainBaseParams);
    return *globalChainBaseParams;
}

std::unique_ptr<MCBaseChainParams> CreateBaseChainParams(const std::string& chain)
{
    if (chain == MCBaseChainParams::MAIN)
        return std::unique_ptr<MCBaseChainParams>(new MCBaseMainParams());
    else if (chain == MCBaseChainParams::TESTNET)
        return std::unique_ptr<MCBaseChainParams>(new MCBaseTestNetParams());
    else if (chain == MCBaseChainParams::REGTEST)
        return std::unique_ptr<MCBaseChainParams>(new MCBaseRegTestParams());
	else if (chain == MCBaseChainParams::BRANCH)
		return std::unique_ptr<MCBaseChainParams>(new MCBaseBranchParams());
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectBaseParams(const std::string& chain)
{
    globalChainBaseParams = CreateBaseChainParams(chain);
}

std::string ChainNameFromCommandLine()
{
    if (gArgs.GetArg("-branchid", "") != "")
        return MCBaseChainParams::BRANCH;

    bool fRegTest = gArgs.GetBoolArg("-regtest", false);
    bool fTestNet = gArgs.GetBoolArg("-testnet", false);

    if (fTestNet && fRegTest)
        throw std::runtime_error("Invalid combination of -regtest and -testnet.");
    if (fRegTest)
        return MCBaseChainParams::REGTEST;
    if (fTestNet)
        return MCBaseChainParams::TESTNET;

    return MCBaseChainParams::MAIN;
}
