// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain/chainparamsbase.h"

#include "misc/tinyformat.h"
#include "utils/util.h"

#include <assert.h>

const std::string CellBaseChainParams::MAIN = "main";
const std::string CellBaseChainParams::TESTNET = "test";
const std::string CellBaseChainParams::REGTEST = "regtest";
const std::string CellBaseChainParams::BRANCH = "branch";

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
class CellBaseMainParams : public CellBaseChainParams
{
public:
    CellBaseMainParams()
    {
        nRPCPort = 8332;
    }
};

/**
 * Testnet (v3)
 */
class CellBaseTestNetParams : public CellBaseChainParams
{
public:
    CellBaseTestNetParams()
    {
        nRPCPort = 18332;
        strDataDir = "testnet3";
    }
};

/*
 * Regression test
 */
class CellBaseRegTestParams : public CellBaseChainParams
{
public:
    CellBaseRegTestParams()
    {
        nRPCPort = 18332;
        strDataDir = "regtest";
    }
};

class CellBaseBranchParams : public CellBaseChainParams
{
public:
	CellBaseBranchParams()
	{
		nRPCPort = 8332;
	//	strDataDir = "branch";
	}
};

static std::unique_ptr<CellBaseChainParams> globalChainBaseParams;

const CellBaseChainParams& BaseParams()
{
    assert(globalChainBaseParams);
    return *globalChainBaseParams;
}

std::unique_ptr<CellBaseChainParams> CreateBaseChainParams(const std::string& chain)
{
    if (chain == CellBaseChainParams::MAIN)
        return std::unique_ptr<CellBaseChainParams>(new CellBaseMainParams());
    else if (chain == CellBaseChainParams::TESTNET)
        return std::unique_ptr<CellBaseChainParams>(new CellBaseTestNetParams());
    else if (chain == CellBaseChainParams::REGTEST)
        return std::unique_ptr<CellBaseChainParams>(new CellBaseRegTestParams());
	else if (chain == CellBaseChainParams::BRANCH)
		return std::unique_ptr<CellBaseChainParams>(new CellBaseBranchParams());
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
        return CellBaseChainParams::BRANCH;

    bool fRegTest = gArgs.GetBoolArg("-regtest", false);
    bool fTestNet = gArgs.GetBoolArg("-testnet", false);

    if (fTestNet && fRegTest)
        throw std::runtime_error("Invalid combination of -regtest and -testnet.");
    if (fRegTest)
        return CellBaseChainParams::REGTEST;
    if (fTestNet)
        return CellBaseChainParams::TESTNET;

    return CellBaseChainParams::MAIN;
}
