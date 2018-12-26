// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "misc/amount.h"
#include "coding/base58.h"
#include "chain/chain.h"
#include "consensus/validation.h"
#include "io/core_io.h"
#include "init.h"
#include "net/http/httpserver.h"
#include "validation/validation.h"
#include "net/net.h"
#include "policy/feerate.h"
#include "policy/fees.h"
#include "policy/policy.h"
#include "policy/rbf.h"
#include "mining/mining.h"
#include "rpc/server.h"
#include "script/sign.h"
#include "misc/timedata.h"
#include "utils/util.h"
#include "utils/utilmoneystr.h"
#include "wallet/coincontrol.h"
#include "wallet/feebumper.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#include "script/standard.h"
#include "univalue.h"
#include "smartcontract/smartcontract.h"
#include "script/sign.h"

#include <boost/foreach.hpp>
#include <stdint.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

static const std::string WALLET_ENDPOINT_BASE = "/wallet/";

void SendFromToOther(MCWalletTx &wtxNew, const MagnaChainAddress &fromaddress, const MagnaChainAddress &toaddress, const MagnaChainAddress &changeaddress, const MCAmount nAmount, const MCAmount nUserFee, SmartLuaState* sls = nullptr);
void SendFromToOther(MCWalletTx &wtxNew, const MagnaChainAddress &fromaddress, const MCScript &toScript, const MagnaChainAddress &changeaddress, const MCAmount nAmount, const MCAmount nUserFee, SmartLuaState* sls = nullptr);

MCWallet *GetWalletForJSONRPCRequest(const JSONRPCRequest& request)
{
    if (request.URI.substr(0, WALLET_ENDPOINT_BASE.size()) == WALLET_ENDPOINT_BASE) {
        // wallet endpoint was used
        std::string requestedWallet = urlDecode(request.URI.substr(WALLET_ENDPOINT_BASE.size()));
        for (CWalletRef pwallet : ::vpwallets) {
            if (pwallet->GetName() == requestedWallet) {
                return pwallet;
            }
        }
        throw JSONRPCError(RPC_WALLET_NOT_FOUND, "Requested wallet does not exist or is not loaded");
    }
    return ::vpwallets.size() == 1 || (request.fHelp && ::vpwallets.size() > 0) ? ::vpwallets[0] : nullptr;
}

std::string HelpRequiringPassphrase(MCWallet * const pwallet)
{
    return pwallet && pwallet->IsCrypted()
        ? "\nRequires wallet passphrase to be set with walletpassphrase call."
        : "";
}

bool EnsureWalletIsAvailable(MCWallet * const pwallet, bool avoidException)
{
    if (pwallet) return true;
    if (avoidException) return false;
    if (::vpwallets.empty()) {
        // Note: It isn't currently possible to trigger this error because
        // wallet RPC methods aren't registered unless a wallet is loaded. But
        // this error is being kept as a precaution, because it's possible in
        // the future that wallet RPC methods might get or remain registered
        // when no wallets are loaded.
        throw JSONRPCError(
            RPC_METHOD_NOT_FOUND, "Method not found (wallet method is disabled because no wallet is loaded)");
    }
    throw JSONRPCError(RPC_WALLET_NOT_SPECIFIED,
        "Wallet file not specified (must request wallet RPC through /wallet/<filename> uri-path).");
}

void EnsureWalletIsUnlocked(MCWallet * const pwallet)
{
    if (pwallet->IsLocked()) {
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
    }
}

void WalletTxToJSON(const MCWalletTx& wtx, UniValue& entry)
{
    int confirms = wtx.GetDepthInMainChain();
    entry.push_back(Pair("confirmations", confirms));
    if (wtx.IsCoinBase())
        entry.push_back(Pair("generated", true));
    if (confirms > 0)
    {
        entry.push_back(Pair("blockhash", wtx.hashBlock.GetHex()));
        entry.push_back(Pair("blockindex", wtx.nIndex));
        entry.push_back(Pair("blocktime", mapBlockIndex[wtx.hashBlock]->GetBlockTime()));
    } else {
        entry.push_back(Pair("trusted", wtx.IsTrusted()));
    }
    uint256 hash = wtx.GetHash();
    entry.push_back(Pair("txid", hash.GetHex()));
    UniValue conflicts(UniValue::VARR);
    for (const uint256& conflict : wtx.GetConflicts())
        conflicts.push_back(conflict.GetHex());
    entry.push_back(Pair("walletconflicts", conflicts));
    entry.push_back(Pair("time", wtx.GetTxTime()));
    entry.push_back(Pair("timereceived", (int64_t)wtx.nTimeReceived));

    // Add opt-in RBF status
    std::string rbfStatus = "no";
    if (confirms <= 0) {
        LOCK(mempool.cs);
        RBFTransactionState rbfState = IsRBFOptIn(wtx, mempool);
        if (rbfState == RBF_TRANSACTIONSTATE_UNKNOWN)
            rbfStatus = "unknown";
        else if (rbfState == RBF_TRANSACTIONSTATE_REPLACEABLE_BIP125)
            rbfStatus = "yes";
    }
    entry.push_back(Pair("bip125-replaceable", rbfStatus));

    for (const std::pair<std::string, std::string>& item : wtx.mapValue)
        entry.push_back(Pair(item.first, item.second));
}

std::string AccountFromValue(const UniValue& value)
{
    std::string strAccount = value.get_str();
    if (strAccount == "*")
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid account name");
    return strAccount;
}

UniValue getnewaddress(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 1)
        throw std::runtime_error(
            "getnewaddress ( \"account\" )\n"
            "\nReturns a new MagnaChain address for receiving payments.\n"
            "If 'account' is specified (DEPRECATED), it is added to the address book \n"
            "so payments received with the address will be credited to 'account'.\n"
            "\nArguments:\n"
            "1. \"account\"        (string, optional) DEPRECATED. The account name for the address to be linked to. If not provided, the default account \"\" is used. It can also be set to the empty string \"\" to represent the default account. The account does not need to exist, it will be created if there is no account by the given name.\n"
            "\nResult:\n"
            "\"address\"    (string) The new magnachain address\n"
            "\nExamples:\n"
            + HelpExampleCli("getnewaddress", "")
            + HelpExampleRpc("getnewaddress", "")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    // Parse the account first so we don't generate a key if there's an error
    std::string strAccount;
    if (!request.params[0].isNull())
        strAccount = AccountFromValue(request.params[0]);

    if (!pwallet->IsLocked()) {
        pwallet->TopUpKeyPool();
    }

    // Generate a new key that is added to wallet
    MCPubKey newKey;
    if (!pwallet->GetKeyFromPool(newKey)) {
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    }
    MCKeyID keyID = newKey.GetID();

    pwallet->SetAddressBook(keyID, strAccount, "receive");

    return MagnaChainAddress(keyID).ToString();
}

MagnaChainAddress GetAccountAddress(MCWallet* const pwallet, std::string strAccount, bool bForceNew=false)
{
    MCPubKey pubKey;
    if (!pwallet->GetAccountPubkey(pubKey, strAccount, bForceNew)) {
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    }

    return MagnaChainAddress(pubKey.GetID());
}

UniValue getaccountaddress(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getaccountaddress \"account\"\n"
            "\nDEPRECATED. Returns the current MagnaChain address for receiving payments to this account.\n"
            "\nArguments:\n"
            "1. \"account\"       (string, required) The account name for the address. It can also be set to the empty string \"\" to represent the default account. The account does not need to exist, it will be created and a new address created  if there is no account by the given name.\n"
            "\nResult:\n"
            "\"address\"          (string) The account magnachain address\n"
            "\nExamples:\n"
            + HelpExampleCli("getaccountaddress", "")
            + HelpExampleCli("getaccountaddress", "\"\"")
            + HelpExampleCli("getaccountaddress", "\"myaccount\"")
            + HelpExampleRpc("getaccountaddress", "\"myaccount\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    // Parse the account first so we don't generate a key if there's an error
    std::string strAccount = AccountFromValue(request.params[0]);

    UniValue ret(UniValue::VSTR);

    ret = GetAccountAddress(pwallet, strAccount).ToString();
    return ret;
}

UniValue getrawchangeaddress(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 0)
        throw std::runtime_error(
            "getrawchangeaddress\n"
            "\nReturns a new MagnaChain address, for receiving change.\n"
            "This is for use with raw transactions, NOT normal use.\n"
            "\nResult:\n"
            "\"address\"    (string) The address\n"
            "\nExamples:\n"
            + HelpExampleCli("getrawchangeaddress", "")
            + HelpExampleRpc("getrawchangeaddress", "")
       );

    LOCK2(cs_main, pwallet->cs_wallet);

    if (!pwallet->IsLocked()) {
        pwallet->TopUpKeyPool();
    }

    MCReserveKey reservekey(pwallet);
    MCPubKey vchPubKey;
    if (!reservekey.GetReservedKey(vchPubKey, true))
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");

    reservekey.KeepKey();

    MCKeyID keyID = vchPubKey.GetID();

    return MagnaChainAddress(keyID).ToString();
}

UniValue setaccount(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "setaccount \"address\" \"account\"\n"
            "\nDEPRECATED. Sets the account associated with the given address.\n"
            "\nArguments:\n"
            "1. \"address\"         (string, required) The magnachain address to be associated with an account.\n"
            "2. \"account\"         (string, required) The account to assign the address to.\n"
            "\nExamples:\n"
            + HelpExampleCli("setaccount", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\" \"tabby\"")
            + HelpExampleRpc("setaccount", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\", \"tabby\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    MagnaChainAddress address(request.params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain address");

    std::string strAccount;
    if (request.params.size() > 1)
        strAccount = AccountFromValue(request.params[1]);

    // Only add the account if the address is yours.
    if (IsMine(*pwallet, address.Get())) {
        // Detect when changing the account of an address that is the 'unused current key' of another account:
        if (pwallet->mapAddressBook.count(address.Get())) {
            std::string strOldAccount = pwallet->mapAddressBook[address.Get()].name;
            if (address == GetAccountAddress(pwallet, strOldAccount)) {
                GetAccountAddress(pwallet, strOldAccount, true);
            }
        }
        pwallet->SetAddressBook(address.Get(), strAccount, "receive");
    }
    else
        throw JSONRPCError(RPC_MISC_ERROR, "setaccount can only be used with own address");

    return NullUniValue;
}

UniValue getaccount(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getaccount \"address\"\n"
            "\nDEPRECATED. Returns the account associated with the given address.\n"
            "\nArguments:\n"
            "1. \"address\"         (string, required) The magnachain address for account lookup.\n"
            "\nResult:\n"
            "\"accountname\"        (string) the account address\n"
            "\nExamples:\n"
            + HelpExampleCli("getaccount", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\"")
            + HelpExampleRpc("getaccount", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    MagnaChainAddress address(request.params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain address");

    std::string strAccount;
    std::map<MCTxDestination, MCAddressBookData>::iterator mi = pwallet->mapAddressBook.find(address.Get());
    if (mi != pwallet->mapAddressBook.end() && !(*mi).second.name.empty()) {
        strAccount = (*mi).second.name;
    }
    return strAccount;
}

UniValue getaddressesbyaccount(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getaddressesbyaccount \"account\"\n"
            "\nDEPRECATED. Returns the list of addresses for the given account.\n"
            "\nArguments:\n"
            "1. \"account\"        (string, required) The account name.\n"
            "\nResult:\n"
            "[                     (json array of string)\n"
            "  \"address\"         (string) a magnachain address associated with the given account\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("getaddressesbyaccount", "\"tabby\"")
            + HelpExampleRpc("getaddressesbyaccount", "\"tabby\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    std::string strAccount = AccountFromValue(request.params[0]);

    // Find all addresses that have the given account
    UniValue ret(UniValue::VARR);
    for (const std::pair<MagnaChainAddress, MCAddressBookData>& item : pwallet->mapAddressBook) {
        const MagnaChainAddress& address = item.first;
        const std::string& strName = item.second.name;
        if (strName == strAccount)
            ret.push_back(address.ToString());
    }
    return ret;
}

void SendMoney(MCWallet * const pwallet, const MCScript &scriptPubKey, MCAmount nValue, bool fSubtractFeeFromAmount, MCWalletTx& wtxNew, const MCCoinControl& coin_control, SmartLuaState* sls)
{
	MCAmount curBalance = pwallet->GetBalance();

	// Check amount
	if (nValue < 0)
		throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount");

	if (nValue > curBalance)
		throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

	if (pwallet->GetBroadcastTransactions() && !g_connman)
		throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");

	// Create and send the transaction
	MCReserveKey reservekey(pwallet);
	MCAmount nFeeRequired;
	std::string strError;
	std::vector<MCRecipient> vecSend;
    if (nValue > 0) {
        MCRecipient recipient = { scriptPubKey, nValue, fSubtractFeeFromAmount };
        vecSend.push_back(recipient);
    }
    int nChangePosRet = -1;
	if (!pwallet->CreateTransaction(vecSend, wtxNew, reservekey, nFeeRequired, nChangePosRet, strError, coin_control, true, sls)) {
		if (!fSubtractFeeFromAmount && nValue + nFeeRequired > curBalance)
			strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
		throw JSONRPCError(RPC_WALLET_ERROR, strError);
	}

	MCValidationState state;
	if (!pwallet->CommitTransaction(wtxNew, reservekey, g_connman.get(), state)) {
		strError = strprintf("Error: The transaction was rejected! Reason given: %s. txid %s, you can call abandontransaction to remove it", state.GetRejectReason(), wtxNew.tx->GetHash().GetHex());
		throw JSONRPCError(RPC_WALLET_ERROR, strError);
	}
}

void SendMoney(MCWallet * const pwallet, const MCTxDestination &address, MCAmount nValue, bool fSubtractFeeFromAmount, MCWalletTx& wtxNew, const MCCoinControl& coin_control)
{
	MCScript scriptPubKey = GetScriptForDestination(address);
	SendMoney(pwallet, scriptPubKey, nValue, fSubtractFeeFromAmount, wtxNew, coin_control);
}

UniValue sendtoaddress(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 8)
        throw std::runtime_error(
            "sendtoaddress \"address\" amount ( \"comment\" \"comment_to\" subtractfeefromamount replaceable conf_target \"estimate_mode\")\n"
            "\nSend an amount to a given address.\n"
            + HelpRequiringPassphrase(pwallet) +
            "\nArguments:\n"
            "1. \"address\"            (string, required) The magnachain address to send to.\n"
            "2. \"amount\"             (numeric or string, required) The amount in " + CURRENCY_UNIT + " to send. eg 0.1\n"
            "3. \"comment\"            (string, optional) A comment used to store what the transaction is for. \n"
            "                             This is not part of the transaction, just kept in your wallet.\n"
            "4. \"comment_to\"         (string, optional) A comment to store the name of the person or organization \n"
            "                             to which you're sending the transaction. This is not part of the \n"
            "                             transaction, just kept in your wallet.\n"
            "5. subtractfeefromamount  (boolean, optional, default=false) The fee will be deducted from the amount being sent.\n"
            "                             The recipient will receive less cells than you enter in the amount field.\n"
            "6. replaceable            (boolean, optional) Allow this transaction to be replaced by a transaction with higher fees via BIP 125\n"
            "7. conf_target            (numeric, optional) Confirmation target (in blocks)\n"
            "8. \"estimate_mode\"      (string, optional, default=UNSET) The fee estimate mode, must be one of:\n"
            "       \"UNSET\"\n"
            "       \"ECONOMICAL\"\n"
            "       \"CONSERVATIVE\"\n"
            "\nResult:\n"
            "\"txid\"                  (string) The transaction id.\n"
            "\nExamples:\n"
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1")
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1 \"donation\" \"seans outpost\"")
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1 \"\" \"\" true")
            + HelpExampleRpc("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.1, \"donation\", \"seans outpost\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    MagnaChainAddress address(request.params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain address");
    if (address.IsContractID())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "MagnaChain address can't be contract id");

    // Amount
    MCAmount nAmount = AmountFromValue(request.params[1]);
    if (nAmount <= 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

    // Wallet comments
    MCWalletTx wtx;
    if (request.params.size() > 2 && !request.params[2].isNull() && !request.params[2].get_str().empty())
        wtx.mapValue["comment"] = request.params[2].get_str();
    if (request.params.size() > 3 && !request.params[3].isNull() && !request.params[3].get_str().empty())
        wtx.mapValue["to"]      = request.params[3].get_str();

    bool fSubtractFeeFromAmount = false;
    if (request.params.size() > 4 && !request.params[4].isNull()) {
        fSubtractFeeFromAmount = request.params[4].get_bool();
    }

    MCCoinControl coin_control;
    if (request.params.size() > 5 && !request.params[5].isNull()) {
        coin_control.signalRbf = request.params[5].get_bool();
    }

    if (request.params.size() > 6 && !request.params[6].isNull()) {
        coin_control.m_confirm_target = ParseConfirmTarget(request.params[6]);
    }

    if (request.params.size() > 7 && !request.params[7].isNull()) {
        if (!FeeModeFromString(request.params[7].get_str(), coin_control.m_fee_mode)) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid estimate_mode parameter");
        }
    }

    EnsureWalletIsUnlocked(pwallet);
    SendMoney(pwallet, address.Get(), nAmount, fSubtractFeeFromAmount, wtx, coin_control);

    return wtx.GetHash().GetHex();
}

UniValue publishcontract(const JSONRPCRequest& request)
{
	MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
	if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
		return NullUniValue;
	}

	if (request.fHelp || request.params.size() < 1 )
		throw std::runtime_error(
			"publish \"filename\" \n"
			"\npublish a contract from a file.\n"
			+ HelpRequiringPassphrase(pwallet) +
			"\nArguments:\n"
			"1. \"filename\"            (string, required) The file need to publish.\n"
		);

	LOCK2(cs_main, pwallet->cs_wallet);

	std::string strFileName = request.params[0].get_str();
	FILE * fp = fopen(strFileName.c_str(), "rb");
	if (fp == NULL) {
		throw std::runtime_error(
			"publish \"filename\" \n"
			"\n file open error.\n"
		);
	}

	fseek(fp, 0, SEEK_END);
	long flen = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	std::string rawCode;
    rawCode.resize(flen);
	fread((char*)&rawCode[0], flen, 1, fp);
	fclose(fp);

    std::string strSenderAddr;
    if (request.params.size() > 1)
        strSenderAddr = request.params[1].get_str();

    SmartLuaState sls;
    UniValue ret(UniValue::VARR);
	if (!PublishContract(&sls, pwallet, strSenderAddr, rawCode, ret))
        throw JSONRPCError(RPC_CONTRACT_ERROR, ret[0].get_str());
    return ret;
}

UniValue publishcontractcode(const JSONRPCRequest& request)
{
	MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
	if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
		return NullUniValue;
	}

	if (request.fHelp || request.params.size() < 1)
		throw std::runtime_error(
			"publishcontractcode \"codedata\" \n"
			"\npublish a contract from code data.\n"
			+ HelpRequiringPassphrase(pwallet) +
			"\nArguments:\n"
			"1. \"codedata\"            (string, required) Code data to hex.\n"
		);

	LOCK2(cs_main, pwallet->cs_wallet);

	std::string strCodeDataHex = request.params[0].get_str();
	if (strCodeDataHex.empty())
		throw std::runtime_error("code data can not empty!!");

	if (!IsHexNumber(strCodeDataHex)){
		throw std::runtime_error("code data must hex data.");
	}

	std::vector<unsigned char> vCode = ParseHex(strCodeDataHex);
    std::string rawCode(vCode.begin(), vCode.end());

    std::string strSenderAddr;
    if (request.params.size() > 1)
        strSenderAddr = request.params[1].get_str();

    SmartLuaState sls;
    UniValue ret(UniValue::VARR);
    if (!PublishContract(&sls, pwallet, strSenderAddr, rawCode, ret))
        throw JSONRPCError(RPC_CONTRACT_ERROR, ret[0].get_str());
    return ret;
}

//sdk调用，编译代码，预先生成交易
UniValue prepublishcode(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() < 4 || request.params.size() > 5)
		throw std::runtime_error(
			"prepublishcode fromaddress toaddress changeaddress amount \n"
			"\n rebroadcast the branch chain transaction by txid, in case that transction has not be send to the target chain .\n"
			"\nArguments:\n"
			"1. \"code\"                         (string, required) The compiled code in hex format\n"
			"2. \"fundaddress\"                  (string, required) The magnachain address owns coins to use\n"
			"3. \"senderaddress\"                (string, required) The hex pubkey of address as constact caller address\n"
			"4. \"amount\"                       (numeric or string, required) The amount in " + CURRENCY_UNIT + " to send to constact. eg 0.1\n"
			"5. \"changeaddress\"                (string, required) The address for change may be\n"
			"\nReturns an obj\n"
			"\nResult:\n"
			"{\n"
			"  \"txhex\" : xxx,            (string) The transaction hex data\n"
			"}\n"
			"\nExamples:\n"
			+ HelpExampleCli("prepublishcode", "code XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPi XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj 1 0.0005")
			+ HelpExampleRpc("prepublishcode", "code XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPi XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj 1 ")
		);

	std::string strCodeDataHex = request.params[0].get_str();
	if (strCodeDataHex.empty())
		throw std::runtime_error("code data can not empty!!");

	if (!IsHexNumber(strCodeDataHex)) {
		throw std::runtime_error("code data must hex data.");
	}

	std::vector<unsigned char> vCode = ParseHex(strCodeDataHex);
	std::string rawCode(vCode.begin(), vCode.end());

	MagnaChainAddress fundAddr(request.params[1].get_str());
	if (!fundAddr.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain fund address");
	
	std::vector<unsigned char> data(ParseHex(request.params[2].get_str()));
	MCPubKey senderPubKey(data.begin(), data.end());
	MagnaChainAddress senderAddr(senderPubKey.GetID());
	if (!senderAddr.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain sender address");

	MCAmount amount = AmountFromValue(request.params[3]);
	if (amount < 0)
		throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

	MagnaChainAddress changeAddr;
	if (request.params.size() > 4)
	{
        changeAddr.SetString(request.params[4].get_str());
		if (!changeAddr.IsValid())
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain change address");
	}
	else
        changeAddr = fundAddr;

    MCContractID contractId = GenerateContractAddress(nullptr, senderAddr, rawCode);
	MagnaChainAddress contractAddr(contractId);

    SmartLuaState sls;
    UniValue ret(UniValue::VARR);
    sls.Initialize(GetTime(), chainActive.Height() + 1, -1, senderAddr, nullptr, nullptr, 0, nullptr);
    if (!PublishContract(&sls, contractAddr, rawCode, ret))
        throw JSONRPCError(RPC_CONTRACT_ERROR, ret[0].get_str());

    MCScript scriptPubKey = GetScriptForDestination(contractAddr.Get());

    sls.contractIds.erase(contractId);
    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::PUBLISH_CONTRACT_VERSION;
    wtx.pContractData.reset(new ContractData);
    wtx.pContractData->codeOrFunc = rawCode;
    wtx.pContractData->sender = senderPubKey;
    wtx.pContractData->address = contractId;
    wtx.pContractData->amountOut = 0;
    SendFromToOther(wtx, fundAddr, scriptPubKey, changeAddr, amount, 0, &sls);

    ret.setObject();
    ret.push_back(Pair("txhex", EncodeHexTx(*wtx.tx, RPCSerializationFlags())));
    
    //for debug
    //UniValue objTx(UniValue::VOBJ);
    //uint256 blocktemp;
    //TxToUniv(*wtx.tx, blocktemp, objTx);
    //ret.push_back(Pair("txobj", objTx));

    //check
    //const MCTransaction& tx = *wtx.tx;
    //MCContractID contractAddrtt = GenerateContractAddressByTx(tx);
    //if (contractAddrtt != tx.pContractData->address)
    //    throw JSONRPCError(RPC_CONTRACT_ERROR,"contract address error");
    //MCAmount nValueOut = 0;
    //MCAmount nContractAmountChange = 0;
    //for (const auto& tx_out : tx.vout) {
    //    nValueOut += tx_out.nValue;
    //    if (!MoneyRange(tx_out.nValue) || !MoneyRange(nValueOut))
    //        throw JSONRPCError(RPC_CONTRACT_ERROR, "outvalue error");

    //    if (tx_out.scriptPubKey.IsContract()) {
    //        MCContractID contractId;
    //        if (!tx_out.scriptPubKey.GetContractAddr(contractId) || contractId != tx.pContractData->address)
    //            throw JSONRPCError(RPC_CONTRACT_ERROR, "out contract id error");
    //        if (tx_out.scriptPubKey.IsContractChange()) {
    //            if (nContractAmountChange > 0)
    //                throw JSONRPCError(RPC_CONTRACT_ERROR, "multi contract recharge error");
    //            nContractAmountChange += tx_out.nValue;
    //        }
    //    }
    //}

    UniValue uvalCoins(UniValue::VARR);
    for (MCTxIn txin : wtx.tx->vin)
    {
        const Coin& coin = pcoinsTip->AccessCoin(txin.prevout);
        UniValue uvalCoin((UniValue::VOBJ));
        uvalCoin.push_back(Pair("txhash", txin.prevout.hash.GetHex()));
        uvalCoin.push_back(Pair("outn", int(txin.prevout.n)));
        uvalCoin.push_back(Pair("value", ValueFromAmount(coin.out.nValue)));
        uvalCoin.push_back(Pair("script", HexStr(coin.out.scriptPubKey.begin(), coin.out.scriptPubKey.end())));
        uvalCoins.push_back(uvalCoin);
    }
    ret.push_back(Pair("coins", uvalCoins));

	return ret;
}

UniValue callcontract(const JSONRPCRequest& request)
{
	MCWallet* const pwallet = GetWalletForJSONRPCRequest(request);
	if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
		return NullUniValue;
	}

	if (request.fHelp || request.params.size() < 4)
		throw std::runtime_error(
			"sendcall \"address\" \n"
			"\nsend to contract .\n"
			+ HelpRequiringPassphrase(pwallet) +
            "\nArguments:\n"
            "1. \"sendcall\"            (bool, required) If commit transaction.\n"
            "2. \"amount\"              (number, required) The mount need to send.If amount is 0, transaction will not commit.\n"
			"3. \"contractaddress\"     (string, required) The contract address need to send.\n"
			"4. \"senderaddress\"       (string, required) The sender address, can be empty,as \"\".\n"
			"5. \"function\"	        (string, required) The function need to call.\n"
			"6. \"params\"              (string, optional) The function params.\n"
		);

    LOCK2(cs_main, pwallet->cs_wallet);

    bool sendCall = false;
    UniValue uvSendCall = request.params[0];
    if (uvSendCall.isBool())
        sendCall = uvSendCall.getBool();
    if (uvSendCall.isStr() && (uvSendCall.get_str() == "1" || uvSendCall.get_str() == "true"))
        sendCall = true;

    MCAmount amount = AmountFromValue(request.params[1]);
    if (amount < 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

    std::string strContractAddr = request.params[2].get_str();
    MagnaChainAddress contractAddr(strContractAddr);
    if (!contractAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid contract address");

    std::string strSenderAddr = request.params[3].get_str();
    MagnaChainAddress senderAddr;
    if (!GetSenderAddr(pwallet, strSenderAddr, senderAddr))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid sender address, which is not in this wallet.");

    MCKeyID senderKey;
    MCPubKey senderPubKey;
    senderAddr.GetKeyID(senderKey);
    if (!pwallet->GetPubKey(senderKey, senderPubKey))
        throw JSONRPCError(RPC_TYPE_ERROR, "Get sender public key fail");

    std::string strFuncName = request.params[4].get_str();
    if (strFuncName.empty())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid function name");

    UniValue args(request.params);
    std::vector<UniValue>& vecArgs = args.getMutableValues();
    vecArgs.erase(vecArgs.begin(), vecArgs.begin() + 5);

    SmartLuaState sls;
    UniValue callRet(UniValue::VARR);
    long maxCallNum = MAX_CONTRACT_CALL;
    sls.Initialize(GetTime(), chainActive.Height() + 1, -1, senderAddr, nullptr, nullptr, 0, pCoinAmountCache);
    bool success = CallContract(&sls, contractAddr, amount, strFuncName, args, maxCallNum, callRet);
    if (success) {
        UniValue ret(UniValue::VType::VOBJ);
        if (sendCall) {
            MCScript scriptPubKey = GetScriptForDestination(contractAddr.Get());

            MCContractID contractId;
            contractAddr.GetContractID(contractId);

            MCWalletTx wtx;
            wtx.nVersion = MCTransaction::CALL_CONTRACT_VERSION;
            wtx.pContractData.reset(new ContractData);
            wtx.pContractData->sender = senderPubKey;
            wtx.pContractData->codeOrFunc = strFuncName;
            wtx.pContractData->args = args.write();
            wtx.pContractData->address = contractId;
            wtx.pContractData->amountOut = sls.contractOut;

            bool subtractFeeFromAmount = false;
            MCCoinControl coinCtrl;
            EnsureWalletIsUnlocked(pwallet);
            SendMoney(pwallet, scriptPubKey, amount, subtractFeeFromAmount, wtx, coinCtrl, &sls);
            ret.push_back(Pair("txid", wtx.tx->GetHash().ToString()));
        }
        ret.push_back(Pair("return", callRet));
        return ret;
    }
    else
        throw JSONRPCError(RPC_CONTRACT_ERROR, callRet[0].get_str());

    return NullUniValue;
}

UniValue getcontractcode(const JSONRPCRequest& request)
{
	MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
	if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
		return NullUniValue;
	}
	if (request.fHelp || request.params.size() < 4)
		throw std::runtime_error(
			"sendcall \"address\" \n"
			"\nsend to contract .\n"
			+ HelpRequiringPassphrase(pwallet) +
			"\nArguments:\n"
			"1. \"address\"            (string, required) The contract address need to send.\n"
		);

	LOCK2(cs_main, pwallet->cs_wallet);

	std::string strContractAddr = request.params[0].get_str();
	if (strContractAddr.empty())
		throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address for send");
}

UniValue precallcontract(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() < 7)
		throw std::runtime_error(
			"precallcontract \"address\" \n"
			"\nsend to contract .\n"
			"\nArguments:\n"
			"1. \"contractaddress\"       (string, required) The contract address need to send\n"
			"2. \"fundaddress\"	          (string, required) The magnachain address of fund to be used\n"
			"3. \"amount\"	              (number, required) The amount need to be send to contract\n"
			"4. \"senderpubkey\"          (string, required) The hex pubkey of address of sender\n"
			"5. \"changeaddress\"         (string, required) The address for change may be\n"
			"6. \"issendcall\"            (bool, required) Is need to call or just query, \"1\" is true\n"
			"7. \"function\"	          (string, required) The function name need to be called\n"
			"8. \"params\"                (string, optional) The function params, ...\n"

			"Return:\n"
			"{\n"
			"  \"call_return\" : \"data\",       (array) The return value of contract call\n"
			"  \"txhex\" : \"data\",             (string) The premake tranction without signature, this item \n"
			"  \"coins\": \"data\",              (array) The coins include by transaction,\n"
			"}\n"
		);

	LOCK(cs_main);

    MagnaChainAddress contractAddr(request.params[0].get_str());
	if (!contractAddr.IsValid())
		throw JSONRPCError(RPC_TYPE_ERROR, "Invalid contract address to call");

    MCContractID contractID;
    if (!contractAddr.GetContractID(contractID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid contract address");

	MagnaChainAddress fundAddr(request.params[1].get_str());
	if (!fundAddr.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain fund address");

	// Amount
	MCAmount amount = AmountFromValue(request.params[2]);
	if (amount < 0)
		throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

	std::vector<unsigned char> data(ParseHex(request.params[3].get_str()));
	MCPubKey senderPubKey(data.begin(), data.end());
	MCKeyID senderKey = senderPubKey.GetID();
	MagnaChainAddress senderAddr(senderKey);
	if (!senderAddr.IsValid())
		throw JSONRPCError(RPC_TYPE_ERROR, "Invalid sender address");

	MagnaChainAddress changeAddr(request.params[4].get_str());
	if (!changeAddr.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain address for change");

	bool bSendCall = false;
	UniValue uvIsSendCall = request.params[5];
	if (uvIsSendCall.isBool())
		bSendCall = uvIsSendCall.getBool();
	if (uvIsSendCall.isStr() && (uvIsSendCall.get_str() == "1" || uvIsSendCall.get_str() == "true"))
		bSendCall = true;

	std::string strFuncName = request.params[6].get_str();
	if (strFuncName.empty())
		throw JSONRPCError(RPC_TYPE_ERROR, "Invalid function name for call");

	UniValue args = request.params;
	std::vector<UniValue>& vecArgs = args.getMutableValues();
    vecArgs.erase(vecArgs.begin(), vecArgs.begin() + 7);

    SmartLuaState sls;
    UniValue callRet(UniValue::VARR);
    long maxCallNum = MAX_CONTRACT_CALL;
    sls.Initialize(GetTime(), chainActive.Height() + 1, -1, senderAddr, nullptr, nullptr, 0, pCoinAmountCache);
    bool success = CallContract(&sls, contractAddr, amount, strFuncName, args, maxCallNum, callRet);

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("return", callRet));
    if (success) {
        if (bSendCall) {
            //MCKeyID contractId;
            //contractAddr.GetKeyID(contractId);

            MCScript scriptPubKey = GetScriptForDestination(contractAddr.Get());

            sls.contractIds.erase(contractID);
            MCWalletTx wtx;
            wtx.nVersion = MCTransaction::CALL_CONTRACT_VERSION;
            wtx.pContractData.reset(new ContractData);
            wtx.pContractData->sender = senderPubKey;
            wtx.pContractData->codeOrFunc = strFuncName;
            wtx.pContractData->args = args.write();
            wtx.pContractData->address = contractID;
            wtx.pContractData->amountOut = sls.contractOut;
            SendFromToOther(wtx, fundAddr, scriptPubKey, changeAddr, amount, 0, &sls);

            ret.push_back(Pair("txhex", EncodeHexTx(*wtx.tx, RPCSerializationFlags())));
            UniValue coins(UniValue::VARR);
            for (MCTxIn txin : wtx.tx->vin)
            {
                const Coin& coin = pcoinsTip->AccessCoin(txin.prevout);

                UniValue uvalCoin((UniValue::VOBJ));
                uvalCoin.push_back(Pair("txhash", txin.prevout.hash.GetHex()));
                uvalCoin.push_back(Pair("outn", int(txin.prevout.n)));
                uvalCoin.push_back(Pair("value", ValueFromAmount(coin.out.nValue)));
                uvalCoin.push_back(Pair("script", HexStr(coin.out.scriptPubKey.begin(), coin.out.scriptPubKey.end())));
                coins.push_back(uvalCoin);
            }
            ret.push_back(Pair("coins", coins));
        }
	}
    else
        throw JSONRPCError(RPC_CONTRACT_ERROR, ret[0].get_str());

	return ret;
}

UniValue listaddressgroupings(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "listaddressgroupings\n"
            "\nLists groups of addresses which have had their common ownership\n"
            "made public by common use as inputs or as the resulting change\n"
            "in past transactions\n"
            "\nResult:\n"
            "[\n"
            "  [\n"
            "    [\n"
            "      \"address\",            (string) The magnachain address\n"
            "      amount,                 (numeric) The amount in " + CURRENCY_UNIT + "\n"
            "      \"account\"             (string, optional) DEPRECATED. The account\n"
            "    ]\n"
            "    ,...\n"
            "  ]\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("listaddressgroupings", "")
            + HelpExampleRpc("listaddressgroupings", "")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    UniValue jsonGroupings(UniValue::VARR);
    std::map<MCTxDestination, MCAmount> balances = pwallet->GetAddressBalances();
    for (std::set<MCTxDestination> grouping : pwallet->GetAddressGroupings()) {
        UniValue jsonGrouping(UniValue::VARR);
        for (MCTxDestination address : grouping)
        {
            UniValue addressInfo(UniValue::VARR);
            addressInfo.push_back(MagnaChainAddress(address).ToString());
            addressInfo.push_back(ValueFromAmount(balances[address]));
            {
                if (pwallet->mapAddressBook.find(MagnaChainAddress(address).Get()) != pwallet->mapAddressBook.end()) {
                    addressInfo.push_back(pwallet->mapAddressBook.find(MagnaChainAddress(address).Get())->second.name);
                }
            }
            jsonGrouping.push_back(addressInfo);
        }
        jsonGroupings.push_back(jsonGrouping);
    }
    return jsonGroupings;
}

UniValue signmessage(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 2)
        throw std::runtime_error(
            "signmessage \"address\" \"message\"\n"
            "\nSign a message with the private key of an address"
            + HelpRequiringPassphrase(pwallet) + "\n"
            "\nArguments:\n"
            "1. \"address\"         (string, required) The magnachain address to use for the private key.\n"
            "2. \"message\"         (string, required) The message to create a signature of.\n"
            "\nResult:\n"
            "\"signature\"          (string) The signature of the message encoded in base 64\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n"
            + HelpExampleCli("signmessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("signmessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\", \"my message\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    EnsureWalletIsUnlocked(pwallet);

    std::string strAddress = request.params[0].get_str();
    std::string strMessage = request.params[1].get_str();

    MagnaChainAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    MCKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    MCKey key;
    if (!pwallet->GetKey(keyID, key)) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");
    }

    MCHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(ss.GetHash(), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    return EncodeBase64(&vchSig[0], vchSig.size());
}

UniValue getreceivedbyaddress(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "getreceivedbyaddress \"address\" ( minconf )\n"
            "\nReturns the total amount received by the given address in transactions with at least minconf confirmations.\n"
            "\nArguments:\n"
            "1. \"address\"         (string, required) The magnachain address for transactions.\n"
            "2. minconf             (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "\nResult:\n"
            "amount   (numeric) The total amount in " + CURRENCY_UNIT + " received at this address.\n"
            "\nExamples:\n"
            "\nThe amount from transactions with at least 1 confirmation\n"
            + HelpExampleCli("getreceivedbyaddress", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\"") +
            "\nThe amount including unconfirmed transactions, zero confirmations\n"
            + HelpExampleCli("getreceivedbyaddress", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\" 0") +
            "\nThe amount with at least 6 confirmations\n"
            + HelpExampleCli("getreceivedbyaddress", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\" 6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getreceivedbyaddress", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\", 6")
       );

    LOCK2(cs_main, pwallet->cs_wallet);

    // MagnaChain address
    MagnaChainAddress address = MagnaChainAddress(request.params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain address");
    MCScript scriptPubKey = GetScriptForDestination(address.Get());
    if (!IsMine(*pwallet, scriptPubKey)) {
        return ValueFromAmount(0);
    }

    // Minimum confirmations
    int nMinDepth = 1;
    if (!request.params[1].isNull())
        nMinDepth = request.params[1].get_int();

    // Tally
    MCAmount nAmount = 0;
    for (const std::pair<uint256, MCWalletTx>& pairWtx : pwallet->mapWallet) {
        const MCWalletTx& wtx = pairWtx.second;
        if (wtx.IsCoinBase() || !CheckFinalTx(*wtx.tx))
            continue;

        for (const MCTxOut& txout : wtx.tx->vout)
            if (txout.scriptPubKey == scriptPubKey)
                if (wtx.GetDepthInMainChain() >= nMinDepth)
                    nAmount += txout.nValue;
    }

    return  ValueFromAmount(nAmount);
}

UniValue getreceivedbyaccount(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "getreceivedbyaccount \"account\" ( minconf )\n"
            "\nDEPRECATED. Returns the total amount received by addresses with <account> in transactions with at least [minconf] confirmations.\n"
            "\nArguments:\n"
            "1. \"account\"      (string, required) The selected account, may be the default account using \"\".\n"
            "2. minconf          (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "\nResult:\n"
            "amount              (numeric) The total amount in " + CURRENCY_UNIT + " received for this account.\n"
            "\nExamples:\n"
            "\nAmount received by the default account with at least 1 confirmation\n"
            + HelpExampleCli("getreceivedbyaccount", "\"\"") +
            "\nAmount received at the tabby account including unconfirmed amounts with zero confirmations\n"
            + HelpExampleCli("getreceivedbyaccount", "\"tabby\" 0") +
            "\nThe amount with at least 6 confirmations\n"
            + HelpExampleCli("getreceivedbyaccount", "\"tabby\" 6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getreceivedbyaccount", "\"tabby\", 6")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    // Minimum confirmations
    int nMinDepth = 1;
    if (!request.params[1].isNull())
        nMinDepth = request.params[1].get_int();

    // Get the set of pub keys assigned to account
    std::string strAccount = AccountFromValue(request.params[0]);
    std::set<MCTxDestination> setAddress = pwallet->GetAccountAddresses(strAccount);

    // Tally
    MCAmount nAmount = 0;
    for (const std::pair<uint256, MCWalletTx>& pairWtx : pwallet->mapWallet) {
        const MCWalletTx& wtx = pairWtx.second;
        if (wtx.IsCoinBase() || !CheckFinalTx(*wtx.tx))
            continue;

        for (const MCTxOut& txout : wtx.tx->vout)
        {
            MCTxDestination address;
            if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*pwallet, address) && setAddress.count(address)) {
                if (wtx.GetDepthInMainChain() >= nMinDepth)
                    nAmount += txout.nValue;
            }
        }
    }

    return ValueFromAmount(nAmount);
}

UniValue getbalance(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 3)
        throw std::runtime_error(
            "getbalance ( \"account\" minconf include_watchonly )\n"
            "\nIf account is not specified, returns the server's total available balance.\n"
            "If account is specified (DEPRECATED), returns the balance in the account.\n"
            "Note that the account \"\" is not the same as leaving the parameter out.\n"
            "The server total may be different to the balance in the default \"\" account.\n"
            "\nArguments:\n"
            "1. \"account\"         (string, optional) DEPRECATED. The account string may be given as a\n"
            "                     specific account name to find the balance associated with wallet keys in\n"
            "                     a named account, or as the empty string (\"\") to find the balance\n"
            "                     associated with wallet keys not in any named account, or as \"*\" to find\n"
            "                     the balance associated with all wallet keys regardless of account.\n"
            "                     When this option is specified, it calculates the balance in a different\n"
            "                     way than when it is not specified, and which can count spends twice when\n"
            "                     there are conflicting pending transactions (such as those created by\n"
            "                     the bumpfee command), temporarily resulting in low or even negative\n"
            "                     balances. In general, account balance calculation is not considered\n"
            "                     reliable and has resulted in confusing outcomes, so it is recommended to\n"
            "                     avoid passing this argument.\n"
            "2. minconf           (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "3. include_watchonly (bool, optional, default=false) Also include balance in watch-only addresses (see 'importaddress')\n"
            "\nResult:\n"
            "amount              (numeric) The total amount in " + CURRENCY_UNIT + " received for this account.\n"
            "\nExamples:\n"
            "\nThe total amount in the wallet with 1 or more confirmations\n"
            + HelpExampleCli("getbalance", "") +
            "\nThe total amount in the wallet at least 6 blocks confirmed\n"
            + HelpExampleCli("getbalance", "\"*\" 6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getbalance", "\"*\", 6")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    if (request.params.size() == 0)
        return  ValueFromAmount(pwallet->GetBalance());

    const std::string& account_param = request.params[0].get_str();
    const std::string* account = account_param != "*" ? &account_param : nullptr;

    int nMinDepth = 1;
    if (!request.params[1].isNull())
        nMinDepth = request.params[1].get_int();
    isminefilter filter = ISMINE_SPENDABLE;
    if(!request.params[2].isNull())
        if(request.params[2].get_bool())
            filter = filter | ISMINE_WATCH_ONLY;

    return ValueFromAmount(pwallet->GetLegacyBalance(filter, nMinDepth, account));
}

UniValue getunconfirmedbalance(const JSONRPCRequest &request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 0)
        throw std::runtime_error(
                "getunconfirmedbalance\n"
                "Returns the server's total unconfirmed balance\n");

    LOCK2(cs_main, pwallet->cs_wallet);

    return ValueFromAmount(pwallet->GetUnconfirmedBalance());
}

UniValue movecmd(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 3 || request.params.size() > 5)
        throw std::runtime_error(
            "move \"fromaccount\" \"toaccount\" amount ( minconf \"comment\" )\n"
            "\nDEPRECATED. Move a specified amount from one account in your wallet to another.\n"
            "\nArguments:\n"
            "1. \"fromaccount\"   (string, required) The name of the account to move funds from. May be the default account using \"\".\n"
            "2. \"toaccount\"     (string, required) The name of the account to move funds to. May be the default account using \"\".\n"
            "3. amount            (numeric) Quantity of " + CURRENCY_UNIT + " to move between accounts.\n"
            "4. (dummy)           (numeric, optional) Ignored. Remains for backward compatibility.\n"
            "5. \"comment\"       (string, optional) An optional comment, stored in the wallet only.\n"
            "\nResult:\n"
            "true|false           (boolean) true if successful.\n"
            "\nExamples:\n"
            "\nMove 0.01 " + CURRENCY_UNIT + " from the default account to the account named tabby\n"
            + HelpExampleCli("move", "\"\" \"tabby\" 0.01") +
            "\nMove 0.01 " + CURRENCY_UNIT + " timotei to akiko with a comment and funds have 6 confirmations\n"
            + HelpExampleCli("move", "\"timotei\" \"akiko\" 0.01 6 \"happy birthday!\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("move", "\"timotei\", \"akiko\", 0.01, 6, \"happy birthday!\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    std::string strFrom = AccountFromValue(request.params[0]);
    std::string strTo = AccountFromValue(request.params[1]);
    MCAmount nAmount = AmountFromValue(request.params[2]);
    if (nAmount <= 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");
    if (request.params.size() > 3)
        // unused parameter, used to be nMinDepth, keep type-checking it though
        (void)request.params[3].get_int();
    std::string strComment;
    if (request.params.size() > 4)
        strComment = request.params[4].get_str();

    if (!pwallet->AccountMove(strFrom, strTo, nAmount, strComment)) {
        throw JSONRPCError(RPC_DATABASE_ERROR, "database error");
    }

    return true;
}

UniValue sendfrom(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 3 || request.params.size() > 6)
        throw std::runtime_error(
            "sendfrom \"fromaccount\" \"toaddress\" amount ( minconf \"comment\" \"comment_to\" )\n"
            "\nDEPRECATED (use sendtoaddress). Sent an amount from an account to a magnachain address."
            + HelpRequiringPassphrase(pwallet) + "\n"
            "\nArguments:\n"
            "1. \"fromaccount\"       (string, required) The name of the account to send funds from. May be the default account using \"\".\n"
            "                       Specifying an account does not influence coin selection, but it does associate the newly created\n"
            "                       transaction with the account, so the account's balance computation and transaction history can reflect\n"
            "                       the spend.\n"
            "2. \"toaddress\"         (string, required) The magnachain address to send funds to.\n"
            "3. amount                (numeric or string, required) The amount in " + CURRENCY_UNIT + " (transaction fee is added on top).\n"
            "4. minconf               (numeric, optional, default=1) Only use funds with at least this many confirmations.\n"
            "5. \"comment\"           (string, optional) A comment used to store what the transaction is for. \n"
            "                                     This is not part of the transaction, just kept in your wallet.\n"
            "6. \"comment_to\"        (string, optional) An optional comment to store the name of the person or organization \n"
            "                                     to which you're sending the transaction. This is not part of the transaction, \n"
            "                                     it is just kept in your wallet.\n"
            "\nResult:\n"
            "\"txid\"                 (string) The transaction id.\n"
            "\nExamples:\n"
            "\nSend 0.01 " + CURRENCY_UNIT + " from the default account to the address, must have at least 1 confirmation\n"
            + HelpExampleCli("sendfrom", "\"\" \"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.01") +
            "\nSend 0.01 from the tabby account to the given address, funds must have at least 6 confirmations\n"
            + HelpExampleCli("sendfrom", "\"tabby\" \"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.01 6 \"donation\" \"seans outpost\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("sendfrom", "\"tabby\", \"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.01, 6, \"donation\", \"seans outpost\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    std::string strAccount = AccountFromValue(request.params[0]);

    MagnaChainAddress address(request.params[1].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain address");
    if (address.IsContractID())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "MagnaChain address can't be contract id");

    MCAmount nAmount = AmountFromValue(request.params[2]);
    if (nAmount <= 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

    int nMinDepth = 1;
    if (request.params.size() > 3)
        nMinDepth = request.params[3].get_int();

    MCWalletTx wtx;
    wtx.strFromAccount = strAccount;
    if (request.params.size() > 4 && !request.params[4].isNull() && !request.params[4].get_str().empty())
        wtx.mapValue["comment"] = request.params[4].get_str();
    if (request.params.size() > 5 && !request.params[5].isNull() && !request.params[5].get_str().empty())
        wtx.mapValue["to"]      = request.params[5].get_str();

    EnsureWalletIsUnlocked(pwallet);

    // Check funds
    MCAmount nBalance = pwallet->GetLegacyBalance(ISMINE_SPENDABLE, nMinDepth, &strAccount);
    if (nAmount > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    MCCoinControl no_coin_control; // This is a deprecated API
    SendMoney(pwallet, address.Get(), nAmount, false, wtx, no_coin_control);

    return wtx.GetHash().GetHex();
}

UniValue sendmany(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 8)
        throw std::runtime_error(
            "sendmany \"fromaccount\" {\"address\":amount,...} ( minconf \"comment\" [\"address\",...] replaceable conf_target \"estimate_mode\")\n"
            "\nSend multiple times. Amounts are double-precision floating point numbers."
            + HelpRequiringPassphrase(pwallet) + "\n"
            "\nArguments:\n"
            "1. \"fromaccount\"         (string, required) DEPRECATED. The account to send the funds from. Should be \"\" for the default account\n"
            "2. \"amounts\"             (string, required) A json object with addresses and amounts\n"
            "    {\n"
            "      \"address\":amount   (numeric or string) The magnachain address is the key, the numeric amount (can be string) in " + CURRENCY_UNIT + " is the value\n"
            "      ,...\n"
            "    }\n"
            "3. minconf                 (numeric, optional, default=1) Only use the balance confirmed at least this many times.\n"
            "4. \"comment\"             (string, optional) A comment\n"
            "5. subtractfeefrom         (array, optional) A json array with addresses.\n"
            "                           The fee will be equally deducted from the amount of each selected address.\n"
            "                           Those recipients will receive less cells than you enter in their corresponding amount field.\n"
            "                           If no addresses are specified here, the sender pays the fee.\n"
            "    [\n"
            "      \"address\"          (string) Subtract fee from this address\n"
            "      ,...\n"
            "    ]\n"
            "6. replaceable            (boolean, optional) Allow this transaction to be replaced by a transaction with higher fees via BIP 125\n"
            "7. conf_target            (numeric, optional) Confirmation target (in blocks)\n"
            "8. \"estimate_mode\"      (string, optional, default=UNSET) The fee estimate mode, must be one of:\n"
            "       \"UNSET\"\n"
            "       \"ECONOMICAL\"\n"
            "       \"CONSERVATIVE\"\n"
             "\nResult:\n"
            "\"txid\"                   (string) The transaction id for the send. Only 1 transaction is created regardless of \n"
            "                                    the number of addresses.\n"
            "\nExamples:\n"
            "\nSend two amounts to two different addresses:\n"
            + HelpExampleCli("sendmany", "\"\" \"{\\\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\\\":0.01,\\\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\\\":0.02}\"") +
            "\nSend two amounts to two different addresses setting the confirmation and comment:\n"
            + HelpExampleCli("sendmany", "\"\" \"{\\\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\\\":0.01,\\\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\\\":0.02}\" 6 \"testing\"") +
            "\nSend two amounts to two different addresses, subtract fee from amount:\n"
            + HelpExampleCli("sendmany", "\"\" \"{\\\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\\\":0.01,\\\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\\\":0.02}\" 1 \"\" \"[\\\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\\\",\\\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\\\"]\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("sendmany", "\"\", \"{\\\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\\\":0.01,\\\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\\\":0.02}\", 6, \"testing\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    std::string strAccount = AccountFromValue(request.params[0]);
    UniValue sendTo = request.params[1].get_obj();
    int nMinDepth = 1;
    if (!request.params[2].isNull())
        nMinDepth = request.params[2].get_int();

    MCWalletTx wtx;
    wtx.strFromAccount = strAccount;
    if (request.params.size() > 3 && !request.params[3].isNull() && !request.params[3].get_str().empty())
        wtx.mapValue["comment"] = request.params[3].get_str();

    UniValue subtractFeeFromAmount(UniValue::VARR);
    if (request.params.size() > 4 && !request.params[4].isNull())
        subtractFeeFromAmount = request.params[4].get_array();

    MCCoinControl coin_control;
    if (request.params.size() > 5 && !request.params[5].isNull()) {
        coin_control.signalRbf = request.params[5].get_bool();
    }

    if (request.params.size() > 6 && !request.params[6].isNull()) {
        coin_control.m_confirm_target = ParseConfirmTarget(request.params[6]);
    }

    if (request.params.size() > 7 && !request.params[7].isNull()) {
        if (!FeeModeFromString(request.params[7].get_str(), coin_control.m_fee_mode)) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid estimate_mode parameter");
        }
    }

    std::set<MagnaChainAddress> setAddress;
    std::vector<MCRecipient> vecSend;

    MCAmount totalAmount = 0;
    std::vector<std::string> keys = sendTo.getKeys();
    for (const std::string& name_ : keys)
    {
        MagnaChainAddress address(name_);
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid MagnaChain address: ") + name_);

        if (address.IsContractID())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("MagnaChain address can't be contract id: ") + name_);

        if (setAddress.count(address))
            throw JSONRPCError(RPC_INVALID_PARAMETER, std::string("Invalid parameter, duplicated address: ") + name_);
        setAddress.insert(address);

        MCScript scriptPubKey = GetScriptForDestination(address.Get());
        MCAmount nAmount = AmountFromValue(sendTo[name_]);
        if (nAmount <= 0)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");
        totalAmount += nAmount;

        bool fSubtractFeeFromAmount = false;
        for (unsigned int idx = 0; idx < subtractFeeFromAmount.size(); idx++) {
            const UniValue& addr = subtractFeeFromAmount[idx];
            if (addr.get_str() == name_)
                fSubtractFeeFromAmount = true;
        }

        MCRecipient recipient = {scriptPubKey, nAmount, fSubtractFeeFromAmount};
        vecSend.push_back(recipient);
    }

    EnsureWalletIsUnlocked(pwallet);

    // Check funds
    MCAmount nBalance = pwallet->GetLegacyBalance(ISMINE_SPENDABLE, nMinDepth, &strAccount);
    if (totalAmount > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    // Send
    MCReserveKey keyChange(pwallet);
    MCAmount nFeeRequired = 0;
    int nChangePosRet = -1;
    std::string strFailReason;
    bool fCreated = pwallet->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, nChangePosRet, strFailReason, coin_control);
    if (!fCreated)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, strFailReason);
    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, keyChange, g_connman.get(), state)) {
        strFailReason = strprintf("Transaction commit failed:: %s", state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strFailReason);
    }

    return wtx.GetHash().GetHex();
}

// Defined in rpc/misc.cpp
extern MCScript _createmultisig_redeemScript(MCWallet * const pwallet, const UniValue& params);

UniValue addmultisigaddress(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 3)
    {
        std::string msg = "addmultisigaddress nrequired [\"key\",...] ( \"account\" )\n"
            "\nAdd a nrequired-to-sign multisignature address to the wallet.\n"
            "Each key is a MagnaChain address or hex-encoded public key.\n"
            "If 'account' is specified (DEPRECATED), assign address to that account.\n"

            "\nArguments:\n"
            "1. nrequired        (numeric, required) The number of required signatures out of the n keys or addresses.\n"
            "2. \"keys\"         (string, required) A json array of magnachain addresses or hex-encoded public keys\n"
            "     [\n"
            "       \"address\"  (string) magnachain address or hex-encoded public key\n"
            "       ...,\n"
            "     ]\n"
            "3. \"account\"      (string, optional) DEPRECATED. An account to assign the addresses to.\n"

            "\nResult:\n"
            "\"address\"         (string) A magnachain address associated with the keys.\n"

            "\nExamples:\n"
            "\nAdd a multisig address from 2 addresses\n"
            + HelpExampleCli("addmultisigaddress", "2 \"[\\\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\"") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("addmultisigaddress", "2, \"[\\\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\"")
        ;
        throw std::runtime_error(msg);
    }

    LOCK2(cs_main, pwallet->cs_wallet);

    std::string strAccount;
    if (request.params.size() > 2)
        strAccount = AccountFromValue(request.params[2]);

    // Construct using pay-to-script-hash:
    MCScript inner = _createmultisig_redeemScript(pwallet, request.params);
    MCScriptID innerID(inner);
    pwallet->AddCScript(inner);

    pwallet->SetAddressBook(innerID, strAccount, "send");
    return MagnaChainAddress(innerID).ToString();
}

class Witnessifier : public boost::static_visitor<bool>
{
public:
    MCWallet * const pwallet;
    MCScriptID result;

    Witnessifier(MCWallet *_pwallet) : pwallet(_pwallet) {}

    bool operator()(const MCNoDestination &dest) const { return false; }

    bool operator()(const MCContractID &contractID) {
        // TODO: fill logic
        return false;
    }

    bool operator()(const MCKeyID &keyID) {
        if (pwallet) {
            MCScript basescript = GetScriptForDestination(keyID);
            MCScript witscript = GetScriptForWitness(basescript);
            SignatureData sigs;
            // This check is to make sure that the script we created can actually be solved for and signed by us
            // if we were to have the private keys. This is just to make sure that the script is valid and that,
            // if found in a transaction, we would still accept and relay that transcation.
            if (!ProduceSignature(DummySignatureCreator(pwallet), witscript, sigs) ||
                !VerifyScript(sigs.scriptSig, witscript, &sigs.scriptWitness, MANDATORY_SCRIPT_VERIFY_FLAGS | SCRIPT_VERIFY_WITNESS_PUBKEYTYPE, DummySignatureCreator(pwallet).Checker())) {
                return false;
            }
            pwallet->AddCScript(witscript);
            result = MCScriptID(witscript);
            return true;
        }
        return false;
    }

    bool operator()(const MCScriptID &scriptID) {
        MCScript subscript;
        if (pwallet && pwallet->GetCScript(scriptID, subscript)) {
            int witnessversion;
            std::vector<unsigned char> witprog;
            if (subscript.IsWitnessProgram(witnessversion, witprog)) {
                result = scriptID;
                return true;
            }
            MCScript witscript = GetScriptForWitness(subscript);
            SignatureData sigs;
            // This check is to make sure that the script we created can actually be solved for and signed by us
            // if we were to have the private keys. This is just to make sure that the script is valid and that,
            // if found in a transaction, we would still accept and relay that transcation.
            if (!ProduceSignature(DummySignatureCreator(pwallet), witscript, sigs) ||
                !VerifyScript(sigs.scriptSig, witscript, &sigs.scriptWitness, MANDATORY_SCRIPT_VERIFY_FLAGS | SCRIPT_VERIFY_WITNESS_PUBKEYTYPE, DummySignatureCreator(pwallet).Checker())) {
                return false;
            }
            pwallet->AddCScript(witscript);
            result = MCScriptID(witscript);
            return true;
        }
        return false;
    }
};

UniValue addwitnessaddress(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
    {
        std::string msg = "addwitnessaddress \"address\"\n"
            "\nAdd a witness address for a script (with pubkey or redeemscript known).\n"
            "It returns the witness script.\n"

            "\nArguments:\n"
            "1. \"address\"       (string, required) An address known to the wallet\n"

            "\nResult:\n"
            "\"witnessaddress\",  (string) The value of the new address (P2SH of witness script).\n"
            "}\n"
        ;
        throw std::runtime_error(msg);
    }

    {
        LOCK(cs_main);
        if (!IsWitnessEnabled(chainActive.Tip(), Params().GetConsensus()) && !gArgs.GetBoolArg("-walletprematurewitness", false)) {
            throw JSONRPCError(RPC_WALLET_ERROR, "Segregated witness not enabled on network");
        }
    }

    MagnaChainAddress address(request.params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain address");

    Witnessifier w(pwallet);
    MCTxDestination dest = address.Get();
    bool ret = boost::apply_visitor(w, dest);
    if (!ret) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Public key or redeemscript not known to wallet, or the key is uncompressed");
    }

    pwallet->SetAddressBook(w.result, "", "receive");

    return MagnaChainAddress(w.result).ToString();
}

struct tallyitem
{
    MCAmount nAmount;
    int nConf;
    std::vector<uint256> txids;
    bool fIsWatchonly;
    tallyitem()
    {
        nAmount = 0;
        nConf = std::numeric_limits<int>::max();
        fIsWatchonly = false;
    }
};

UniValue ListReceived(MCWallet * const pwallet, const UniValue& params, bool fByAccounts)
{
    // Minimum confirmations
    int nMinDepth = 1;
    if (!params[0].isNull())
        nMinDepth = params[0].get_int();

    // Whether to include empty accounts
    bool fIncludeEmpty = false;
    if (!params[1].isNull())
        fIncludeEmpty = params[1].get_bool();

    isminefilter filter = ISMINE_SPENDABLE;
    if(!params[2].isNull())
        if(params[2].get_bool())
            filter = filter | ISMINE_WATCH_ONLY;

    // Tally
    std::map<MagnaChainAddress, tallyitem> mapTally;
    for (const std::pair<uint256, MCWalletTx>& pairWtx : pwallet->mapWallet) {
        const MCWalletTx& wtx = pairWtx.second;

        if (wtx.IsCoinBase() || !CheckFinalTx(*wtx.tx))
            continue;

        int nDepth = wtx.GetDepthInMainChain();
        if (nDepth < nMinDepth)
            continue;

        for (const MCTxOut& txout : wtx.tx->vout)
        {
            MCTxDestination address;
            if (!ExtractDestination(txout.scriptPubKey, address))
                continue;

            isminefilter mine = IsMine(*pwallet, address);
            if(!(mine & filter))
                continue;

            tallyitem& item = mapTally[address];
            item.nAmount += txout.nValue;
            item.nConf = std::min(item.nConf, nDepth);
            item.txids.push_back(wtx.GetHash());
            if (mine & ISMINE_WATCH_ONLY)
                item.fIsWatchonly = true;
        }
    }

    // Reply
    UniValue ret(UniValue::VARR);
    std::map<std::string, tallyitem> mapAccountTally;
    for (const std::pair<MagnaChainAddress, MCAddressBookData>& item : pwallet->mapAddressBook) {
        const MagnaChainAddress& address = item.first;
        const std::string& strAccount = item.second.name;
        std::map<MagnaChainAddress, tallyitem>::iterator it = mapTally.find(address);
        if (it == mapTally.end() && !fIncludeEmpty)
            continue;

        MCAmount nAmount = 0;
        int nConf = std::numeric_limits<int>::max();
        bool fIsWatchonly = false;
        if (it != mapTally.end())
        {
            nAmount = (*it).second.nAmount;
            nConf = (*it).second.nConf;
            fIsWatchonly = (*it).second.fIsWatchonly;
        }

        if (fByAccounts)
        {
            tallyitem& _item = mapAccountTally[strAccount];
            _item.nAmount += nAmount;
            _item.nConf = std::min(_item.nConf, nConf);
            _item.fIsWatchonly = fIsWatchonly;
        }
        else
        {
            UniValue obj(UniValue::VOBJ);
            if(fIsWatchonly)
                obj.push_back(Pair("involvesWatchonly", true));
            obj.push_back(Pair("address",       address.ToString()));
            obj.push_back(Pair("account",       strAccount));
            obj.push_back(Pair("amount",        ValueFromAmount(nAmount)));
            obj.push_back(Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
            if (!fByAccounts)
                obj.push_back(Pair("label", strAccount));
            UniValue transactions(UniValue::VARR);
            if (it != mapTally.end())
            {
                for (const uint256& _item : (*it).second.txids)
                {
                    transactions.push_back(_item.GetHex());
                }
            }
            obj.push_back(Pair("txids", transactions));
            ret.push_back(obj);
        }
    }

    if (fByAccounts)
    {
        for (std::map<std::string, tallyitem>::iterator it = mapAccountTally.begin(); it != mapAccountTally.end(); ++it)
        {
            MCAmount nAmount = (*it).second.nAmount;
            int nConf = (*it).second.nConf;
            UniValue obj(UniValue::VOBJ);
            if((*it).second.fIsWatchonly)
                obj.push_back(Pair("involvesWatchonly", true));
            obj.push_back(Pair("account",       (*it).first));
            obj.push_back(Pair("amount",        ValueFromAmount(nAmount)));
            obj.push_back(Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
            ret.push_back(obj);
        }
    }

    return ret;
}

UniValue listreceivedbyaddress(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 3)
        throw std::runtime_error(
            "listreceivedbyaddress ( minconf include_empty include_watchonly)\n"
            "\nList balances by receiving address.\n"
            "\nArguments:\n"
            "1. minconf           (numeric, optional, default=1) The minimum number of confirmations before payments are included.\n"
            "2. include_empty     (bool, optional, default=false) Whether to include addresses that haven't received any payments.\n"
            "3. include_watchonly (bool, optional, default=false) Whether to include watch-only addresses (see 'importaddress').\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"involvesWatchonly\" : true,        (bool) Only returned if imported addresses were involved in transaction\n"
            "    \"address\" : \"receivingaddress\",  (string) The receiving address\n"
            "    \"account\" : \"accountname\",       (string) DEPRECATED. The account of the receiving address. The default account is \"\".\n"
            "    \"amount\" : x.xxx,                  (numeric) The total amount in " + CURRENCY_UNIT + " received by the address\n"
            "    \"confirmations\" : n,               (numeric) The number of confirmations of the most recent transaction included\n"
            "    \"label\" : \"label\",               (string) A comment for the address/transaction, if any\n"
            "    \"txids\": [\n"
            "       n,                                (numeric) The ids of transactions received with the address \n"
            "       ...\n"
            "    ]\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n"
            + HelpExampleCli("listreceivedbyaddress", "")
            + HelpExampleCli("listreceivedbyaddress", "6 true")
            + HelpExampleRpc("listreceivedbyaddress", "6, true, true")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    return ListReceived(pwallet, request.params, false);
}

UniValue listreceivedbyaccount(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 3)
        throw std::runtime_error(
            "listreceivedbyaccount ( minconf include_empty include_watchonly)\n"
            "\nDEPRECATED. List balances by account.\n"
            "\nArguments:\n"
            "1. minconf           (numeric, optional, default=1) The minimum number of confirmations before payments are included.\n"
            "2. include_empty     (bool, optional, default=false) Whether to include accounts that haven't received any payments.\n"
            "3. include_watchonly (bool, optional, default=false) Whether to include watch-only addresses (see 'importaddress').\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"involvesWatchonly\" : true,   (bool) Only returned if imported addresses were involved in transaction\n"
            "    \"account\" : \"accountname\",  (string) The account name of the receiving account\n"
            "    \"amount\" : x.xxx,             (numeric) The total amount received by addresses with this account\n"
            "    \"confirmations\" : n,          (numeric) The number of confirmations of the most recent transaction included\n"
            "    \"label\" : \"label\"           (string) A comment for the address/transaction, if any\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n"
            + HelpExampleCli("listreceivedbyaccount", "")
            + HelpExampleCli("listreceivedbyaccount", "6 true")
            + HelpExampleRpc("listreceivedbyaccount", "6, true, true")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    return ListReceived(pwallet, request.params, true);
}

static void MaybePushAddress(UniValue & entry, const MCTxDestination &dest)
{
    MagnaChainAddress addr;
    if (addr.Set(dest))
        entry.push_back(Pair("address", addr.ToString()));
}

/**
 * List transactions based on the given criteria.
 *
 * @param  pwallet    The wallet.
 * @param  wtx        The wallet transaction.
 * @param  strAccount The account, if any, or "*" for all.
 * @param  nMinDepth  The minimum confirmation depth.
 * @param  fLong      Whether to include the JSON version of the transaction.
 * @param  ret        The UniValue into which the result is stored.
 * @param  filter     The "is mine" filter bool.
 */
void ListTransactions(MCWallet* const pwallet, const MCWalletTx& wtx, const std::string& strAccount, int nMinDepth, bool fLong, UniValue& ret, const isminefilter& filter)
{
    MCAmount nFee;
    std::string strSentAccount;
    std::list<MCOutputEntry> listReceived;
    std::list<MCOutputEntry> listSent;

    wtx.GetAmounts(listReceived, listSent, nFee, strSentAccount, filter);

    bool fAllAccounts = (strAccount == std::string("*"));
    bool involvesWatchonly = wtx.IsFromMe(ISMINE_WATCH_ONLY);

    // Sent
    if ((!listSent.empty() || nFee != 0) && (fAllAccounts || strAccount == strSentAccount))
    {
        for (const MCOutputEntry& s : listSent)
        {
            UniValue entry(UniValue::VOBJ);
            if (involvesWatchonly || (::IsMine(*pwallet, s.destination) & ISMINE_WATCH_ONLY)) {
                entry.push_back(Pair("involvesWatchonly", true));
            }
            entry.push_back(Pair("account", strSentAccount));
            MaybePushAddress(entry, s.destination);
            entry.push_back(Pair("category", "send"));
            entry.push_back(Pair("amount", ValueFromAmount(-s.amount)));
            if (pwallet->mapAddressBook.count(s.destination)) {
                entry.push_back(Pair("label", pwallet->mapAddressBook[s.destination].name));
            }
            entry.push_back(Pair("vout", s.vout));
            entry.push_back(Pair("fee", ValueFromAmount(-nFee)));
            if (fLong)
                WalletTxToJSON(wtx, entry);
            entry.push_back(Pair("abandoned", wtx.isAbandoned()));
            ret.push_back(entry);
        }
    }

    // Received
    if (listReceived.size() > 0 && wtx.GetDepthInMainChain() >= nMinDepth)
    {
        for (const MCOutputEntry& r : listReceived)
        {
            std::string account;
            if (pwallet->mapAddressBook.count(r.destination)) {
                account = pwallet->mapAddressBook[r.destination].name;
            }
            if (fAllAccounts || (account == strAccount))
            {
                UniValue entry(UniValue::VOBJ);
                if (involvesWatchonly || (::IsMine(*pwallet, r.destination) & ISMINE_WATCH_ONLY)) {
                    entry.push_back(Pair("involvesWatchonly", true));
                }
                entry.push_back(Pair("account", account));
                MaybePushAddress(entry, r.destination);
                if (wtx.IsCoinBase())
                {
                    if (wtx.GetDepthInMainChain() < 1)
                        entry.push_back(Pair("category", "orphan"));
                    else if (wtx.GetBlocksToMaturity() > 0)
                        entry.push_back(Pair("category", "immature"));
                    else
                        entry.push_back(Pair("category", "generate"));
                }
                else
                {
                    entry.push_back(Pair("category", "receive"));
                }
                entry.push_back(Pair("amount", ValueFromAmount(r.amount)));
                if (pwallet->mapAddressBook.count(r.destination)) {
                    entry.push_back(Pair("label", account));
                }
                entry.push_back(Pair("vout", r.vout));
                if (fLong)
                    WalletTxToJSON(wtx, entry);
                ret.push_back(entry);
            }
        }
    }
}

void AcentryToJSON(const MCAccountingEntry& acentry, const std::string& strAccount, UniValue& ret)
{
    bool fAllAccounts = (strAccount == std::string("*"));

    if (fAllAccounts || acentry.strAccount == strAccount)
    {
        UniValue entry(UniValue::VOBJ);
        entry.push_back(Pair("account", acentry.strAccount));
        entry.push_back(Pair("category", "move"));
        entry.push_back(Pair("time", acentry.nTime));
        entry.push_back(Pair("amount", ValueFromAmount(acentry.nCreditDebit)));
        entry.push_back(Pair("otheraccount", acentry.strOtherAccount));
        entry.push_back(Pair("comment", acentry.strComment));
        ret.push_back(entry);
    }
}

UniValue listtransactions(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 4)
        throw std::runtime_error(
            "listtransactions ( \"account\" count skip include_watchonly)\n"
            "\nReturns up to 'count' most recent transactions skipping the first 'from' transactions for account 'account'.\n"
            "\nArguments:\n"
            "1. \"account\"    (string, optional) DEPRECATED. The account name. Should be \"*\".\n"
            "2. count          (numeric, optional, default=10) The number of transactions to return\n"
            "3. skip           (numeric, optional, default=0) The number of transactions to skip\n"
            "4. include_watchonly (bool, optional, default=false) Include transactions to watch-only addresses (see 'importaddress')\n"
            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"account\":\"accountname\",       (string) DEPRECATED. The account name associated with the transaction. \n"
            "                                                It will be \"\" for the default account.\n"
            "    \"address\":\"address\",    (string) The magnachain address of the transaction. Not present for \n"
            "                                                move transactions (category = move).\n"
            "    \"category\":\"send|receive|move\", (string) The transaction category. 'move' is a local (off blockchain)\n"
            "                                                transaction between accounts, and not associated with an address,\n"
            "                                                transaction id or block. 'send' and 'receive' transactions are \n"
            "                                                associated with an address, transaction id and block details\n"
            "    \"amount\": x.xxx,          (numeric) The amount in " + CURRENCY_UNIT + ". This is negative for the 'send' category, and for the\n"
            "                                         'move' category for moves outbound. It is positive for the 'receive' category,\n"
            "                                         and for the 'move' category for inbound funds.\n"
            "    \"label\": \"label\",       (string) A comment for the address/transaction, if any\n"
            "    \"vout\": n,                (numeric) the vout value\n"
            "    \"fee\": x.xxx,             (numeric) The amount of the fee in " + CURRENCY_UNIT + ". This is negative and only available for the \n"
            "                                         'send' category of transactions.\n"
            "    \"confirmations\": n,       (numeric) The number of confirmations for the transaction. Available for 'send' and \n"
            "                                         'receive' category of transactions. Negative confirmations indicate the\n"
            "                                         transaction conflicts with the block chain\n"
            "    \"trusted\": xxx,           (bool) Whether we consider the outputs of this unconfirmed transaction safe to spend.\n"
            "    \"blockhash\": \"hashvalue\", (string) The block hash containing the transaction. Available for 'send' and 'receive'\n"
            "                                          category of transactions.\n"
            "    \"blockindex\": n,          (numeric) The index of the transaction in the block that includes it. Available for 'send' and 'receive'\n"
            "                                          category of transactions.\n"
            "    \"blocktime\": xxx,         (numeric) The block time in seconds since epoch (1 Jan 1970 GMT).\n"
            "    \"txid\": \"transactionid\", (string) The transaction id. Available for 'send' and 'receive' category of transactions.\n"
            "    \"time\": xxx,              (numeric) The transaction time in seconds since epoch (midnight Jan 1 1970 GMT).\n"
            "    \"timereceived\": xxx,      (numeric) The time received in seconds since epoch (midnight Jan 1 1970 GMT). Available \n"
            "                                          for 'send' and 'receive' category of transactions.\n"
            "    \"comment\": \"...\",       (string) If a comment is associated with the transaction.\n"
            "    \"otheraccount\": \"accountname\",  (string) DEPRECATED. For the 'move' category of transactions, the account the funds came \n"
            "                                          from (for receiving funds, positive amounts), or went to (for sending funds,\n"
            "                                          negative amounts).\n"
            "    \"bip125-replaceable\": \"yes|no|unknown\",  (string) Whether this transaction could be replaced due to BIP125 (replace-by-fee);\n"
            "                                                     may be unknown for unconfirmed transactions not in the mempool\n"
            "    \"abandoned\": xxx          (bool) 'true' if the transaction has been abandoned (inputs are respendable). Only available for the \n"
            "                                         'send' category of transactions.\n"
            "  }\n"
            "]\n"

            "\nExamples:\n"
            "\nList the most recent 10 transactions in the systems\n"
            + HelpExampleCli("listtransactions", "") +
            "\nList transactions 100 to 120\n"
            + HelpExampleCli("listtransactions", "\"*\" 20 100") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("listtransactions", "\"*\", 20, 100")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    std::string strAccount = "*";
    if (!request.params[0].isNull())
        strAccount = request.params[0].get_str();
    int nCount = 10;
    if (!request.params[1].isNull())
        nCount = request.params[1].get_int();
    int nFrom = 0;
    if (!request.params[2].isNull())
        nFrom = request.params[2].get_int();
    isminefilter filter = ISMINE_SPENDABLE;
    if(!request.params[3].isNull())
        if(request.params[3].get_bool())
            filter = filter | ISMINE_WATCH_ONLY;

    if (nCount < 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");
    if (nFrom < 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative from");

    UniValue ret(UniValue::VARR);

    const MCWallet::TxItems & txOrdered = pwallet->wtxOrdered;

    // iterate backwards until we have nCount items to return:
    for (MCWallet::TxItems::const_reverse_iterator it = txOrdered.rbegin(); it != txOrdered.rend(); ++it)
    {
        MCWalletTx *const pwtx = (*it).second.first;
        if (pwtx != 0)
            ListTransactions(pwallet, *pwtx, strAccount, 0, true, ret, filter);
        MCAccountingEntry *const pacentry = (*it).second.second;
        if (pacentry != 0)
            AcentryToJSON(*pacentry, strAccount, ret);

        if ((int)ret.size() >= (nCount+nFrom)) break;
    }
    // ret is newest to oldest

    if (nFrom > (int)ret.size())
        nFrom = ret.size();
    if ((nFrom + nCount) > (int)ret.size())
        nCount = ret.size() - nFrom;

    std::vector<UniValue> arrTmp = ret.getValues();

    std::vector<UniValue>::iterator first = arrTmp.begin();
    std::advance(first, nFrom);
    std::vector<UniValue>::iterator last = arrTmp.begin();
    std::advance(last, nFrom+nCount);

    if (last != arrTmp.end()) arrTmp.erase(last, arrTmp.end());
    if (first != arrTmp.begin()) arrTmp.erase(arrTmp.begin(), first);

    std::reverse(arrTmp.begin(), arrTmp.end()); // Return oldest to newest

    ret.clear();
    ret.setArray();
    ret.push_backV(arrTmp);

    return ret;
}

UniValue listaccounts(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 2)
        throw std::runtime_error(
            "listaccounts ( minconf include_watchonly)\n"
            "\nDEPRECATED. Returns Object that has account names as keys, account balances as values.\n"
            "\nArguments:\n"
            "1. minconf             (numeric, optional, default=1) Only include transactions with at least this many confirmations\n"
            "2. include_watchonly   (bool, optional, default=false) Include balances in watch-only addresses (see 'importaddress')\n"
            "\nResult:\n"
            "{                      (json object where keys are account names, and values are numeric balances\n"
            "  \"account\": x.xxx,  (numeric) The property name is the account name, and the value is the total balance for the account.\n"
            "  ...\n"
            "}\n"
            "\nExamples:\n"
            "\nList account balances where there at least 1 confirmation\n"
            + HelpExampleCli("listaccounts", "") +
            "\nList account balances including zero confirmation transactions\n"
            + HelpExampleCli("listaccounts", "0") +
            "\nList account balances for 6 or more confirmations\n"
            + HelpExampleCli("listaccounts", "6") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("listaccounts", "6")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    int nMinDepth = 1;
    if (request.params.size() > 0)
        nMinDepth = request.params[0].get_int();
    isminefilter includeWatchonly = ISMINE_SPENDABLE;
    if(request.params.size() > 1)
        if(request.params[1].get_bool())
            includeWatchonly = includeWatchonly | ISMINE_WATCH_ONLY;

    std::map<std::string, MCAmount> mapAccountBalances;
    for (const std::pair<MCTxDestination, MCAddressBookData>& entry : pwallet->mapAddressBook) {
        if (IsMine(*pwallet, entry.first) & includeWatchonly) {  // This address belongs to me
            mapAccountBalances[entry.second.name] = 0;
        }
    }

    for (const std::pair<uint256, MCWalletTx>& pairWtx : pwallet->mapWallet) {
        const MCWalletTx& wtx = pairWtx.second;
        MCAmount nFee;
        std::string strSentAccount;
        std::list<MCOutputEntry> listReceived;
        std::list<MCOutputEntry> listSent;
        int nDepth = wtx.GetDepthInMainChain();
        if (wtx.GetBlocksToMaturity() > 0 || nDepth < 0)
            continue;
        wtx.GetAmounts(listReceived, listSent, nFee, strSentAccount, includeWatchonly);
        mapAccountBalances[strSentAccount] -= nFee;
        for (const MCOutputEntry& s : listSent)
            mapAccountBalances[strSentAccount] -= s.amount;
        if (nDepth >= nMinDepth)
        {
            for (const MCOutputEntry& r : listReceived)
                if (pwallet->mapAddressBook.count(r.destination)) {
                    mapAccountBalances[pwallet->mapAddressBook[r.destination].name] += r.amount;
                }
                else
                    mapAccountBalances[""] += r.amount;
        }
    }

    const std::list<MCAccountingEntry>& acentries = pwallet->laccentries;
    for (const MCAccountingEntry& entry : acentries)
        mapAccountBalances[entry.strAccount] += entry.nCreditDebit;

    UniValue ret(UniValue::VOBJ);
    for (const std::pair<std::string, MCAmount>& accountBalance : mapAccountBalances) {
        ret.push_back(Pair(accountBalance.first, ValueFromAmount(accountBalance.second)));
    }
    return ret;
}

UniValue listsinceblock(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 4)
        throw std::runtime_error(
            "listsinceblock ( \"blockhash\" target_confirmations include_watchonly include_removed )\n"
            "\nGet all transactions in blocks since block [blockhash], or all transactions if omitted.\n"
            "If \"blockhash\" is no longer a part of the main chain, transactions from the fork point onward are included.\n"
            "Additionally, if include_removed is set, transactions affecting the wallet which were removed are returned in the \"removed\" array.\n"
            "\nArguments:\n"
            "1. \"blockhash\"            (string, optional) The block hash to list transactions since\n"
            "2. target_confirmations:    (numeric, optional, default=1) Return the nth block hash from the main chain. e.g. 1 would mean the best block hash. Note: this is not used as a filter, but only affects [lastblock] in the return value\n"
            "3. include_watchonly:       (bool, optional, default=false) Include transactions to watch-only addresses (see 'importaddress')\n"
            "4. include_removed:         (bool, optional, default=true) Show transactions that were removed due to a reorg in the \"removed\" array\n"
            "                                                           (not guaranteed to work on pruned nodes)\n"
            "\nResult:\n"
            "{\n"
            "  \"transactions\": [\n"
            "    \"account\":\"accountname\",       (string) DEPRECATED. The account name associated with the transaction. Will be \"\" for the default account.\n"
            "    \"address\":\"address\",    (string) The magnachain address of the transaction. Not present for move transactions (category = move).\n"
            "    \"category\":\"send|receive\",     (string) The transaction category. 'send' has negative amounts, 'receive' has positive amounts.\n"
            "    \"amount\": x.xxx,          (numeric) The amount in " + CURRENCY_UNIT + ". This is negative for the 'send' category, and for the 'move' category for moves \n"
            "                                          outbound. It is positive for the 'receive' category, and for the 'move' category for inbound funds.\n"
            "    \"vout\" : n,               (numeric) the vout value\n"
            "    \"fee\": x.xxx,             (numeric) The amount of the fee in " + CURRENCY_UNIT + ". This is negative and only available for the 'send' category of transactions.\n"
            "    \"confirmations\": n,       (numeric) The number of confirmations for the transaction. Available for 'send' and 'receive' category of transactions.\n"
            "                                          When it's < 0, it means the transaction conflicted that many blocks ago.\n"
            "    \"blockhash\": \"hashvalue\",     (string) The block hash containing the transaction. Available for 'send' and 'receive' category of transactions.\n"
            "    \"blockindex\": n,          (numeric) The index of the transaction in the block that includes it. Available for 'send' and 'receive' category of transactions.\n"
            "    \"blocktime\": xxx,         (numeric) The block time in seconds since epoch (1 Jan 1970 GMT).\n"
            "    \"txid\": \"transactionid\",  (string) The transaction id. Available for 'send' and 'receive' category of transactions.\n"
            "    \"time\": xxx,              (numeric) The transaction time in seconds since epoch (Jan 1 1970 GMT).\n"
            "    \"timereceived\": xxx,      (numeric) The time received in seconds since epoch (Jan 1 1970 GMT). Available for 'send' and 'receive' category of transactions.\n"
            "    \"bip125-replaceable\": \"yes|no|unknown\",  (string) Whether this transaction could be replaced due to BIP125 (replace-by-fee);\n"
            "                                                   may be unknown for unconfirmed transactions not in the mempool\n"
            "    \"abandoned\": xxx,         (bool) 'true' if the transaction has been abandoned (inputs are respendable). Only available for the 'send' category of transactions.\n"
            "    \"comment\": \"...\",       (string) If a comment is associated with the transaction.\n"
            "    \"label\" : \"label\"       (string) A comment for the address/transaction, if any\n"
            "    \"to\": \"...\",            (string) If a comment to is associated with the transaction.\n"
            "  ],\n"
            "  \"removed\": [\n"
            "    <structure is the same as \"transactions\" above, only present if include_removed=true>\n"
            "    Note: transactions that were readded in the active chain will appear as-is in this array, and may thus have a positive confirmation count.\n"
            "  ],\n"
            "  \"lastblock\": \"lastblockhash\"     (string) The hash of the block (target_confirmations-1) from the best block on the main chain. This is typically used to feed back into listsinceblock the next time you call it. So you would generally use a target_confirmations of say 6, so you will be continually re-notified of transactions until they've reached 6 confirmations plus any new ones\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("listsinceblock", "")
            + HelpExampleCli("listsinceblock", "\"000000000000000bacf66f7497b7dc45ef753ee9a7d38571037cdb1a57f663ad\" 6")
            + HelpExampleRpc("listsinceblock", "\"000000000000000bacf66f7497b7dc45ef753ee9a7d38571037cdb1a57f663ad\", 6")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    const MCBlockIndex* pindex = nullptr;    // Block index of the specified block or the common ancestor, if the block provided was in a deactivated chain.
    const MCBlockIndex* paltindex = nullptr; // Block index of the specified block, even if it's in a deactivated chain.
    int target_confirms = 1;
    isminefilter filter = ISMINE_SPENDABLE;

    if (!request.params[0].isNull() && !request.params[0].get_str().empty()) {
        uint256 blockId;

        blockId.SetHex(request.params[0].get_str());
        BlockMap::iterator it = mapBlockIndex.find(blockId);
        if (it == mapBlockIndex.end()) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");
        }
        paltindex = pindex = it->second;
        if (chainActive[pindex->nHeight] != pindex) {
            // the block being asked for is a part of a deactivated chain;
            // we don't want to depend on its perceived height in the block
            // chain, we want to instead use the last common ancestor
            pindex = chainActive.FindFork(pindex);
        }
    }

    if (!request.params[1].isNull()) {
        target_confirms = request.params[1].get_int();

        if (target_confirms < 1) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter");
        }
    }

    if (!request.params[2].isNull() && request.params[2].get_bool()) {
        filter = filter | ISMINE_WATCH_ONLY;
    }

    bool include_removed = (request.params[3].isNull() || request.params[3].get_bool());

    int depth = pindex ? (1 + chainActive.Height() - pindex->nHeight) : -1;

    UniValue transactions(UniValue::VARR);

    for (const std::pair<uint256, MCWalletTx>& pairWtx : pwallet->mapWallet) {
        MCWalletTx tx = pairWtx.second;

        if (depth == -1 || tx.GetDepthInMainChain() < depth) {
            ListTransactions(pwallet, tx, "*", 0, true, transactions, filter);
        }
    }

    // when a reorg'd block is requested, we also list any relevant transactions
    // in the blocks of the chain that was detached
    UniValue removed(UniValue::VARR);
    while (include_removed && paltindex && paltindex != pindex) {
        MCBlock block;
        if (!ReadBlockFromDisk(block, paltindex, Params().GetConsensus())) {
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");
        }
        for (const MCTransactionRef& tx : block.vtx) {
            if (pwallet->mapWallet.count(tx->GetHash()) > 0) {
                // We want all transactions regardless of confirmation count to appear here,
                // even negative confirmation ones, hence the big negative.
                ListTransactions(pwallet, pwallet->mapWallet[tx->GetHash()], "*", -100000000, true, removed, filter);
            }
        }
        paltindex = paltindex->pprev;
    }

    MCBlockIndex *pblockLast = chainActive[chainActive.Height() + 1 - target_confirms];
    uint256 lastblock = pblockLast ? pblockLast->GetBlockHash() : uint256();

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("transactions", transactions));
    if (include_removed) ret.push_back(Pair("removed", removed));
    ret.push_back(Pair("lastblock", lastblock.GetHex()));

    return ret;
}

UniValue gettransaction(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "gettransaction \"txid\" ( include_watchonly )\n"
            "\nGet detailed information about in-wallet transaction <txid>\n"
            "\nArguments:\n"
            "1. \"txid\"                  (string, required) The transaction id\n"
            "2. \"include_watchonly\"     (bool, optional, default=false) Whether to include watch-only addresses in balance calculation and details[]\n"
            "\nResult:\n"
            "{\n"
            "  \"amount\" : x.xxx,        (numeric) The transaction amount in " + CURRENCY_UNIT + "\n"
            "  \"fee\": x.xxx,            (numeric) The amount of the fee in " + CURRENCY_UNIT + ". This is negative and only available for the \n"
            "                              'send' category of transactions.\n"
            "  \"confirmations\" : n,     (numeric) The number of confirmations\n"
            "  \"blockhash\" : \"hash\",  (string) The block hash\n"
            "  \"blockindex\" : xx,       (numeric) The index of the transaction in the block that includes it\n"
            "  \"blocktime\" : ttt,       (numeric) The time in seconds since epoch (1 Jan 1970 GMT)\n"
            "  \"txid\" : \"transactionid\",   (string) The transaction id.\n"
            "  \"time\" : ttt,            (numeric) The transaction time in seconds since epoch (1 Jan 1970 GMT)\n"
            "  \"timereceived\" : ttt,    (numeric) The time received in seconds since epoch (1 Jan 1970 GMT)\n"
            "  \"bip125-replaceable\": \"yes|no|unknown\",  (string) Whether this transaction could be replaced due to BIP125 (replace-by-fee);\n"
            "                                                   may be unknown for unconfirmed transactions not in the mempool\n"
            "  \"details\" : [\n"
            "    {\n"
            "      \"account\" : \"accountname\",      (string) DEPRECATED. The account name involved in the transaction, can be \"\" for the default account.\n"
            "      \"address\" : \"address\",          (string) The magnachain address involved in the transaction\n"
            "      \"category\" : \"send|receive\",    (string) The category, either 'send' or 'receive'\n"
            "      \"amount\" : x.xxx,                 (numeric) The amount in " + CURRENCY_UNIT + "\n"
            "      \"label\" : \"label\",              (string) A comment for the address/transaction, if any\n"
            "      \"vout\" : n,                       (numeric) the vout value\n"
            "      \"fee\": x.xxx,                     (numeric) The amount of the fee in " + CURRENCY_UNIT + ". This is negative and only available for the \n"
            "                                           'send' category of transactions.\n"
            "      \"abandoned\": xxx                  (bool) 'true' if the transaction has been abandoned (inputs are respendable). Only available for the \n"
            "                                           'send' category of transactions.\n"
            "    }\n"
            "    ,...\n"
            "  ],\n"
            "  \"hex\" : \"data\"         (string) Raw data for transaction\n"
            "}\n"

            "\nExamples:\n"
            + HelpExampleCli("gettransaction", "\"1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d\"")
            + HelpExampleCli("gettransaction", "\"1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d\" true")
            + HelpExampleRpc("gettransaction", "\"1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    uint256 hash;
    hash.SetHex(request.params[0].get_str());

    isminefilter filter = ISMINE_SPENDABLE;
    if(!request.params[1].isNull())
        if(request.params[1].get_bool())
            filter = filter | ISMINE_WATCH_ONLY;

    UniValue entry(UniValue::VOBJ);
    if (!pwallet->mapWallet.count(hash)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid or non-wallet transaction id");
    }
    const MCWalletTx& wtx = pwallet->mapWallet[hash];

    MCAmount nCredit = wtx.GetCredit(filter);
    MCAmount nDebit = wtx.GetDebit(filter);
    MCAmount nNet = nCredit - nDebit;
    MCAmount nFee = (wtx.IsFromMe(filter) ? wtx.tx->GetValueOut() - nDebit : 0);

    entry.push_back(Pair("amount", ValueFromAmount(nNet - nFee)));
    if (wtx.IsFromMe(filter))
        entry.push_back(Pair("fee", ValueFromAmount(nFee)));

    WalletTxToJSON(wtx, entry);

    UniValue details(UniValue::VARR);
    ListTransactions(pwallet, wtx, "*", 0, false, details, filter);
    entry.push_back(Pair("details", details));

    std::string strHex = EncodeHexTx(static_cast<MCTransaction>(wtx), RPCSerializationFlags());
    entry.push_back(Pair("hex", strHex));

    return entry;
}

UniValue abandontransaction(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "abandontransaction \"txid\"\n"
            "\nMark in-wallet transaction <txid> as abandoned\n"
            "This will mark this transaction and all its in-wallet descendants as abandoned which will allow\n"
            "for their inputs to be respent.  It can be used to replace \"stuck\" or evicted transactions.\n"
            "It only works on transactions which are not included in a block and are not currently in the mempool.\n"
            "It has no effect on transactions which are already conflicted or abandoned.\n"
            "\nArguments:\n"
            "1. \"txid\"    (string, required) The transaction id\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("abandontransaction", "\"1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d\"")
            + HelpExampleRpc("abandontransaction", "\"1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    uint256 hash;
    hash.SetHex(request.params[0].get_str());

    if (!pwallet->mapWallet.count(hash)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid or non-wallet transaction id");
    }
    if (!pwallet->AbandonTransaction(hash)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Transaction not eligible for abandonment");
    }

    return NullUniValue;
}

UniValue backupwallet(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "backupwallet \"destination\"\n"
            "\nSafely copies current wallet file to destination, which can be a directory or a path with filename.\n"
            "\nArguments:\n"
            "1. \"destination\"   (string) The destination directory or file\n"
            "\nExamples:\n"
            + HelpExampleCli("backupwallet", "\"backup.dat\"")
            + HelpExampleRpc("backupwallet", "\"backup.dat\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    std::string strDest = request.params[0].get_str();
    if (!pwallet->BackupWallet(strDest)) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Error: Wallet backup failed!");
    }

    return NullUniValue;
}

UniValue keypoolrefill(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 1)
        throw std::runtime_error(
            "keypoolrefill ( newsize )\n"
            "\nFills the keypool."
            + HelpRequiringPassphrase(pwallet) + "\n"
            "\nArguments\n"
            "1. newsize     (numeric, optional, default=100) The new keypool size\n"
            "\nExamples:\n"
            + HelpExampleCli("keypoolrefill", "")
            + HelpExampleRpc("keypoolrefill", "")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    // 0 is interpreted by TopUpKeyPool() as the default keypool size given by -keypool
    unsigned int kpSize = 0;
    if (!request.params[0].isNull()) {
        if (request.params[0].get_int() < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected valid size.");
        kpSize = (unsigned int)request.params[0].get_int();
    }

    EnsureWalletIsUnlocked(pwallet);
    pwallet->TopUpKeyPool(kpSize);

    if (pwallet->GetKeyPoolSize() < kpSize) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Error refreshing keypool.");
    }

    return NullUniValue;
}

static void LockWallet(MCWallet* pWallet)
{
    LOCK(pWallet->cs_wallet);
    pWallet->nRelockTime = 0;
    pWallet->Lock();
}

UniValue walletpassphrase(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (pwallet->IsCrypted() && (request.fHelp || request.params.size() != 2)) {
        throw std::runtime_error(
            "walletpassphrase \"passphrase\" timeout\n"
            "\nStores the wallet decryption key in memory for 'timeout' seconds.\n"
            "This is needed prior to performing transactions related to private keys such as sending cells\n"
            "\nArguments:\n"
            "1. \"passphrase\"     (string, required) The wallet passphrase\n"
            "2. timeout            (numeric, required) The time to keep the decryption key in seconds.\n"
            "\nNote:\n"
            "Issuing the walletpassphrase command while the wallet is already unlocked will set a new unlock\n"
            "time that overrides the old one.\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 60 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\" 60") +
            "\nLock the wallet again (before 60 seconds)\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("walletpassphrase", "\"my pass phrase\", 60")
        );
    }

    LOCK2(cs_main, pwallet->cs_wallet);

    if (request.fHelp)
        return true;
    if (!pwallet->IsCrypted()) {
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrase was called.");
    }

    // Note that the walletpassphrase is stored in request.params[0] which is not mlock()ed
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make request.params[0] mlock()'d to begin with.
    strWalletPass = request.params[0].get_str().c_str();

    if (strWalletPass.length() > 0)
    {
        if (!pwallet->Unlock(strWalletPass)) {
            throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
        }
    }
    else
        throw std::runtime_error(
            "walletpassphrase <passphrase> <timeout>\n"
            "Stores the wallet decryption key in memory for <timeout> seconds.");

    pwallet->TopUpKeyPool();

    int64_t nSleepTime = request.params[1].get_int64();
    pwallet->nRelockTime = GetTime() + nSleepTime;
    RPCRunLater(strprintf("lockwallet(%s)", pwallet->GetName()), boost::bind(LockWallet, pwallet), nSleepTime);

    return NullUniValue;
}

UniValue walletpassphrasechange(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (pwallet->IsCrypted() && (request.fHelp || request.params.size() != 2)) {
        throw std::runtime_error(
            "walletpassphrasechange \"oldpassphrase\" \"newpassphrase\"\n"
            "\nChanges the wallet passphrase from 'oldpassphrase' to 'newpassphrase'.\n"
            "\nArguments:\n"
            "1. \"oldpassphrase\"      (string) The current passphrase\n"
            "2. \"newpassphrase\"      (string) The new passphrase\n"
            "\nExamples:\n"
            + HelpExampleCli("walletpassphrasechange", "\"old one\" \"new one\"")
            + HelpExampleRpc("walletpassphrasechange", "\"old one\", \"new one\"")
        );
    }

    LOCK2(cs_main, pwallet->cs_wallet);

    if (request.fHelp)
        return true;
    if (!pwallet->IsCrypted()) {
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrasechange was called.");
    }

    // TODO: get rid of these .c_str() calls by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make request.params[0] mlock()'d to begin with.
    SecureString strOldWalletPass;
    strOldWalletPass.reserve(100);
    strOldWalletPass = request.params[0].get_str().c_str();

    SecureString strNewWalletPass;
    strNewWalletPass.reserve(100);
    strNewWalletPass = request.params[1].get_str().c_str();

    if (strOldWalletPass.length() < 1 || strNewWalletPass.length() < 1)
        throw std::runtime_error(
            "walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
            "Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");

    if (!pwallet->ChangeWalletPassphrase(strOldWalletPass, strNewWalletPass)) {
        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
    }

    return NullUniValue;
}

UniValue walletlock(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (pwallet->IsCrypted() && (request.fHelp || request.params.size() != 0)) {
        throw std::runtime_error(
            "walletlock\n"
            "\nRemoves the wallet encryption key from memory, locking the wallet.\n"
            "After calling this method, you will need to call walletpassphrase again\n"
            "before being able to call any methods which require the wallet to be unlocked.\n"
            "\nExamples:\n"
            "\nSet the passphrase for 2 minutes to perform a transaction\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\" 120") +
            "\nPerform a send (requires passphrase set)\n"
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 1.0") +
            "\nClear the passphrase since we are done before 2 minutes is up\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("walletlock", "")
        );
    }

    LOCK2(cs_main, pwallet->cs_wallet);

    if (request.fHelp)
        return true;
    if (!pwallet->IsCrypted()) {
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletlock was called.");
    }

    pwallet->Lock();
    pwallet->nRelockTime = 0;

    return NullUniValue;
}

UniValue encryptwallet(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (!pwallet->IsCrypted() && (request.fHelp || request.params.size() != 1)) {
        throw std::runtime_error(
            "encryptwallet \"passphrase\"\n"
            "\nEncrypts the wallet with 'passphrase'. This is for first time encryption.\n"
            "After this, any calls that interact with private keys such as sending or signing \n"
            "will require the passphrase to be set prior the making these calls.\n"
            "Use the walletpassphrase call for this, and then walletlock call.\n"
            "If the wallet is already encrypted, use the walletpassphrasechange call.\n"
            "Note that this will shutdown the server.\n"
            "\nArguments:\n"
            "1. \"passphrase\"    (string) The pass phrase to encrypt the wallet with. It must be at least 1 character, but should be long.\n"
            "\nExamples:\n"
            "\nEncrypt your wallet\n"
            + HelpExampleCli("encryptwallet", "\"my pass phrase\"") +
            "\nNow set the passphrase to use the wallet, such as for signing or sending cell\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\"") +
            "\nNow we can do something like sign\n"
            + HelpExampleCli("signmessage", "\"address\" \"test message\"") +
            "\nNow lock the wallet again by removing the passphrase\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("encryptwallet", "\"my pass phrase\"")
        );
    }

    LOCK2(cs_main, pwallet->cs_wallet);

    if (request.fHelp)
        return true;
    if (pwallet->IsCrypted()) {
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an encrypted wallet, but encryptwallet was called.");
    }

    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make request.params[0] mlock()'d to begin with.
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    strWalletPass = request.params[0].get_str().c_str();

    if (strWalletPass.length() < 1)
        throw std::runtime_error(
            "encryptwallet <passphrase>\n"
            "Encrypts the wallet with <passphrase>.");

    if (!pwallet->EncryptWallet(strWalletPass)) {
        throw JSONRPCError(RPC_WALLET_ENCRYPTION_FAILED, "Error: Failed to encrypt the wallet.");
    }

    // BDB seems to have a bad habit of writing old data into
    // slack space in .dat files; that is bad if the old data is
    // unencrypted private keys. So:
    StartShutdown();
    return "wallet encrypted; Magnachain server stopping, restart to run with encrypted wallet. The keypool has been flushed and a new HD seed was generated (if you are using HD). You need to make a new backup.";
}

UniValue lockunspent(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
            "lockunspent unlock ([{\"txid\":\"txid\",\"vout\":n},...])\n"
            "\nUpdates list of temporarily unspendable outputs.\n"
            "Temporarily lock (unlock=false) or unlock (unlock=true) specified transaction outputs.\n"
            "If no transaction outputs are specified when unlocking then all current locked transaction outputs are unlocked.\n"
            "A locked transaction output will not be chosen by automatic coin selection, when spending cells.\n"
            "Locks are stored in memory only. Nodes start with zero locked outputs, and the locked output list\n"
            "is always cleared (by virtue of process exit) when a node stops or fails.\n"
            "Also see the listunspent call\n"
            "\nArguments:\n"
            "1. unlock            (boolean, required) Whether to unlock (true) or lock (false) the specified transactions\n"
            "2. \"transactions\"  (string, optional) A json array of objects. Each object the txid (string) vout (numeric)\n"
            "     [           (json array of json objects)\n"
            "       {\n"
            "         \"txid\":\"id\",    (string) The transaction id\n"
            "         \"vout\": n         (numeric) The output number\n"
            "       }\n"
            "       ,...\n"
            "     ]\n"

            "\nResult:\n"
            "true|false    (boolean) Whether the command was successful or not\n"

            "\nExamples:\n"
            "\nList the unspent transactions\n"
            + HelpExampleCli("listunspent", "") +
            "\nLock an unspent transaction\n"
            + HelpExampleCli("lockunspent", "false \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"") +
            "\nList the locked transactions\n"
            + HelpExampleCli("listlockunspent", "") +
            "\nUnlock the transaction again\n"
            + HelpExampleCli("lockunspent", "true \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("lockunspent", "false, \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    if (request.params.size() == 1)
        RPCTypeCheck(request.params, {UniValue::VBOOL});
    else
        RPCTypeCheck(request.params, {UniValue::VBOOL, UniValue::VARR});

    bool fUnlock = request.params[0].get_bool();

    if (request.params.size() == 1) {
        if (fUnlock)
            pwallet->UnlockAllCoins();
        return true;
    }

    UniValue outputs = request.params[1].get_array();
    for (unsigned int idx = 0; idx < outputs.size(); idx++) {
        const UniValue& output = outputs[idx];
        if (!output.isObject())
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected object");
        const UniValue& o = output.get_obj();

        RPCTypeCheckObj(o,
            {
                {"txid", UniValueType(UniValue::VSTR)},
                {"vout", UniValueType(UniValue::VNUM)},
            });

        std::string txid = find_value(o, "txid").get_str();
        if (!IsHex(txid))
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected hex txid");

        int nOutput = find_value(o, "vout").get_int();
        if (nOutput < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, vout must be positive");

        MCOutPoint outpt(uint256S(txid), nOutput);

        if (fUnlock)
            pwallet->UnlockCoin(outpt);
        else
            pwallet->LockCoin(outpt);
    }

    return true;
}

UniValue listlockunspent(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 0)
        throw std::runtime_error(
            "listlockunspent\n"
            "\nReturns list of temporarily unspendable outputs.\n"
            "See the lockunspent call to lock and unlock transactions for spending.\n"
            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"txid\" : \"transactionid\",     (string) The transaction id locked\n"
            "    \"vout\" : n                      (numeric) The vout value\n"
            "  }\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            "\nList the unspent transactions\n"
            + HelpExampleCli("listunspent", "") +
            "\nLock an unspent transaction\n"
            + HelpExampleCli("lockunspent", "false \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"") +
            "\nList the locked transactions\n"
            + HelpExampleCli("listlockunspent", "") +
            "\nUnlock the transaction again\n"
            + HelpExampleCli("lockunspent", "true \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("listlockunspent", "")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    std::vector<MCOutPoint> vOutpts;
    pwallet->ListLockedCoins(vOutpts);

    UniValue ret(UniValue::VARR);

    for (MCOutPoint &outpt : vOutpts) {
        UniValue o(UniValue::VOBJ);

        o.push_back(Pair("txid", outpt.hash.GetHex()));
        o.push_back(Pair("vout", (int)outpt.n));
        ret.push_back(o);
    }

    return ret;
}

UniValue settxfee(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
            "settxfee amount\n"
            "\nSet the transaction fee per kB. Overwrites the paytxfee parameter.\n"
            "\nArguments:\n"
            "1. amount         (numeric or string, required) The transaction fee in " + CURRENCY_UNIT + "/kB\n"
            "\nResult\n"
            "true|false        (boolean) Returns true if successful\n"
            "\nExamples:\n"
            + HelpExampleCli("settxfee", "0.00001")
            + HelpExampleRpc("settxfee", "0.00001")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    // Amount
    MCAmount nAmount = AmountFromValue(request.params[0]);

    payTxFee = MCFeeRate(nAmount, 1000);
    return true;
}

UniValue getwalletinfo(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "getwalletinfo\n"
            "Returns an object containing various wallet state info.\n"
            "\nResult:\n"
            "{\n"
            "  \"walletname\": xxxxx,             (string) the wallet name\n"
            "  \"walletversion\": xxxxx,          (numeric) the wallet version\n"
            "  \"balance\": xxxxxxx,              (numeric) the total confirmed balance of the wallet in " + CURRENCY_UNIT + "\n"
            "  \"unconfirmed_balance\": xxx,      (numeric) the total unconfirmed balance of the wallet in " + CURRENCY_UNIT + "\n"
            "  \"immature_balance\": xxxxxx,      (numeric) the total immature balance of the wallet in " + CURRENCY_UNIT + "\n"
            "  \"txcount\": xxxxxxx,              (numeric) the total number of transactions in the wallet\n"
            "  \"keypoololdest\": xxxxxx,         (numeric) the timestamp (seconds since Unix epoch) of the oldest pre-generated key in the key pool\n"
            "  \"keypoolsize\": xxxx,             (numeric) how many new keys are pre-generated (only counts external keys)\n"
            "  \"keypoolsize_hd_internal\": xxxx, (numeric) how many new keys are pre-generated for internal use (used for change outputs, only appears if the wallet is using this feature, otherwise external keys are used)\n"
            "  \"unlocked_until\": ttt,           (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked\n"
            "  \"paytxfee\": x.xxxx,              (numeric) the transaction fee configuration, set in " + CURRENCY_UNIT + "/kB\n"
            "  \"hdmasterkeyid\": \"<hash160>\"     (string) the Hash160 of the HD master pubkey\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getwalletinfo", "")
            + HelpExampleRpc("getwalletinfo", "")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    UniValue obj(UniValue::VOBJ);

    size_t kpExternalSize = pwallet->KeypoolCountExternalKeys();
    obj.push_back(Pair("walletname", pwallet->GetName()));
    obj.push_back(Pair("walletversion", pwallet->GetVersion()));
    obj.push_back(Pair("balance",       ValueFromAmount(pwallet->GetBalance())));
    obj.push_back(Pair("unconfirmed_balance", ValueFromAmount(pwallet->GetUnconfirmedBalance())));
    obj.push_back(Pair("immature_balance",    ValueFromAmount(pwallet->GetImmatureBalance())));
    obj.push_back(Pair("txcount",       (int)pwallet->mapWallet.size()));
    obj.push_back(Pair("keypoololdest", pwallet->GetOldestKeyPoolTime()));
    obj.push_back(Pair("keypoolsize", (int64_t)kpExternalSize));
    MCKeyID masterKeyID = pwallet->GetHDChain().masterKeyID;
    if (!masterKeyID.IsNull() && pwallet->CanSupportFeature(FEATURE_HD_SPLIT)) {
        obj.push_back(Pair("keypoolsize_hd_internal",   (int64_t)(pwallet->GetKeyPoolSize() - kpExternalSize)));
    }
    if (pwallet->IsCrypted()) {
        obj.push_back(Pair("unlocked_until", pwallet->nRelockTime));
    }
    obj.push_back(Pair("paytxfee",      ValueFromAmount(payTxFee.GetFeePerK())));
    if (!masterKeyID.IsNull())
         obj.push_back(Pair("hdmasterkeyid", masterKeyID.GetHex()));
    return obj;
}

UniValue listwallets(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "listwallets\n"
            "Returns a list of currently loaded wallets.\n"
            "For full information on the wallet, use \"getwalletinfo\"\n"
            "\nResult:\n"
            "[                         (json array of strings)\n"
            "  \"walletname\"            (string) the wallet name\n"
            "   ...\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("listwallets", "")
            + HelpExampleRpc("listwallets", "")
        );

    UniValue obj(UniValue::VARR);

    for (CWalletRef pwallet : vpwallets) {

        if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
            return NullUniValue;
        }

        LOCK(pwallet->cs_wallet);

        obj.push_back(pwallet->GetName());
    }

    return obj;
}

UniValue resendwallettransactions(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "resendwallettransactions\n"
            "Immediately re-broadcast unconfirmed wallet transactions to all peers.\n"
            "Intended only for testing; the wallet code periodically re-broadcasts\n"
            "automatically.\n"
            "Returns an RPC error if -walletbroadcast is set to false.\n"
            "Returns array of transaction ids that were re-broadcast.\n"
            );

    if (!g_connman)
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");

    LOCK2(cs_main, pwallet->cs_wallet);

    if (!pwallet->GetBroadcastTransactions()) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Error: Wallet transaction broadcasting is disabled with -walletbroadcast");
    }

    std::vector<uint256> txids = pwallet->ResendWalletTransactionsBefore(GetTime(), g_connman.get());
    UniValue result(UniValue::VARR);
    for (const uint256& txid : txids)
    {
        result.push_back(txid.ToString());
    }
    return result;
}

UniValue listunspent(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 5)
        throw std::runtime_error(
            "listunspent ( minconf maxconf  [\"addresses\",...] [include_unsafe] [query_options])\n"
            "\nReturns array of unspent transaction outputs\n"
            "with between minconf and maxconf (inclusive) confirmations.\n"
            "Optionally filter to only include txouts paid to specified addresses.\n"
            "\nArguments:\n"
            "1. minconf          (numeric, optional, default=1) The minimum confirmations to filter\n"
            "2. maxconf          (numeric, optional, default=9999999) The maximum confirmations to filter\n"
            "3. \"addresses\"      (string) A json array of magnachain addresses to filter\n"
            "    [\n"
            "      \"address\"     (string) magnachain address\n"
            "      ,...\n"
            "    ]\n"
            "4. include_unsafe (bool, optional, default=true) Include outputs that are not safe to spend\n"
            "                  See description of \"safe\" attribute below.\n"
            "5. query_options    (json, optional) JSON with query options\n"
            "    {\n"
            "      \"minimumAmount\"    (numeric or string, default=0) Minimum value of each UTXO in " + CURRENCY_UNIT + "\n"
            "      \"maximumAmount\"    (numeric or string, default=unlimited) Maximum value of each UTXO in " + CURRENCY_UNIT + "\n"
            "      \"maximumCount\"     (numeric or string, default=unlimited) Maximum number of UTXOs\n"
            "      \"minimumSumAmount\" (numeric or string, default=unlimited) Minimum sum value of all UTXOs in " + CURRENCY_UNIT + "\n"
            "    }\n"
            "\nResult\n"
            "[                   (array of json object)\n"
            "  {\n"
            "    \"txid\" : \"txid\",          (string) the transaction id \n"
            "    \"vout\" : n,               (numeric) the vout value\n"
            "    \"address\" : \"address\",    (string) the magnachain address\n"
            "    \"account\" : \"account\",    (string) DEPRECATED. The associated account, or \"\" for the default account\n"
            "    \"scriptPubKey\" : \"key\",   (string) the script key\n"
            "    \"amount\" : x.xxx,         (numeric) the transaction output amount in " + CURRENCY_UNIT + "\n"
            "    \"confirmations\" : n,      (numeric) The number of confirmations\n"
            "    \"redeemScript\" : n        (string) The redeemScript if scriptPubKey is P2SH\n"
            "    \"spendable\" : xxx,        (bool) Whether we have the private keys to spend this output\n"
            "    \"solvable\" : xxx,         (bool) Whether we know how to spend this output, ignoring the lack of keys\n"
            "    \"safe\" : xxx              (bool) Whether this output is considered safe to spend. Unconfirmed transactions\n"
            "                              from outside keys and unconfirmed replacement transactions are considered unsafe\n"
            "                              and are not eligible for spending by fundrawtransaction and sendtoaddress.\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples\n"
            + HelpExampleCli("listunspent", "")
            + HelpExampleCli("listunspent", "6 9999999 \"[\\\"1PGFqEzfmQch1gKD3ra4k18PNj3tTUUSqg\\\",\\\"1LtvqCaApEdUGFkpKMM4MstjcaL4dKg8SP\\\"]\"")
            + HelpExampleRpc("listunspent", "6, 9999999 \"[\\\"1PGFqEzfmQch1gKD3ra4k18PNj3tTUUSqg\\\",\\\"1LtvqCaApEdUGFkpKMM4MstjcaL4dKg8SP\\\"]\"")
            + HelpExampleCli("listunspent", "6 9999999 '[]' true '{ \"minimumAmount\": 0.005 }'")
            + HelpExampleRpc("listunspent", "6, 9999999, [] , true, { \"minimumAmount\": 0.005 } ")
        );

    int nMinDepth = 1;
    if (request.params.size() > 0 && !request.params[0].isNull()) {
        RPCTypeCheckArgument(request.params[0], UniValue::VNUM);
        nMinDepth = request.params[0].get_int();
    }

    int nMaxDepth = 9999999;
    if (request.params.size() > 1 && !request.params[1].isNull()) {
        RPCTypeCheckArgument(request.params[1], UniValue::VNUM);
        nMaxDepth = request.params[1].get_int();
    }

    std::set<MagnaChainAddress> setAddress;
    if (request.params.size() > 2 && !request.params[2].isNull()) {
        RPCTypeCheckArgument(request.params[2], UniValue::VARR);
        UniValue inputs = request.params[2].get_array();
        for (unsigned int idx = 0; idx < inputs.size(); idx++) {
            const UniValue& input = inputs[idx];
            MagnaChainAddress address(input.get_str());
            if (!address.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid MagnaChain address: ")+input.get_str());
            if (setAddress.count(address))
                throw JSONRPCError(RPC_INVALID_PARAMETER, std::string("Invalid parameter, duplicated address: ")+input.get_str());
           setAddress.insert(address);
        }
    }

    bool include_unsafe = true;
    if (request.params.size() > 3 && !request.params[3].isNull()) {
        RPCTypeCheckArgument(request.params[3], UniValue::VBOOL);
        include_unsafe = request.params[3].get_bool();
    }

    MCAmount nMinimumAmount = 0;
    MCAmount nMaximumAmount = MAX_MONEY;
    MCAmount nMinimumSumAmount = MAX_MONEY;
    uint64_t nMaximumCount = 0;

    if (!request.params[4].isNull()) {
        const UniValue& options = request.params[4].get_obj();

        if (options.exists("minimumAmount"))
            nMinimumAmount = AmountFromValue(options["minimumAmount"]);

        if (options.exists("maximumAmount"))
            nMaximumAmount = AmountFromValue(options["maximumAmount"]);

        if (options.exists("minimumSumAmount"))
            nMinimumSumAmount = AmountFromValue(options["minimumSumAmount"]);

        if (options.exists("maximumCount"))
            nMaximumCount = options["maximumCount"].get_int64();
    }

    UniValue results(UniValue::VARR);
    std::vector<MCOutput> vecOutputs;
    assert(pwallet != nullptr);
    LOCK2(cs_main, pwallet->cs_wallet);

    pwallet->AvailableCoins(vecOutputs, nullptr, !include_unsafe, nullptr, nMinimumAmount, nMaximumAmount, nMinimumSumAmount, nMaximumCount, nMinDepth, nMaxDepth);
    for (const MCOutput& out : vecOutputs) {
        MCTxDestination address;
        const MCScript& scriptPubKey = out.tx->tx->vout[out.i].scriptPubKey;
        bool fValidAddress = ExtractDestination(scriptPubKey, address);

        if (setAddress.size() && (!fValidAddress || !setAddress.count(address)))
            continue;

        UniValue entry(UniValue::VOBJ);
        entry.push_back(Pair("txid", out.tx->GetHash().GetHex()));
        entry.push_back(Pair("vout", out.i));

        if (fValidAddress) {
            entry.push_back(Pair("address", MagnaChainAddress(address).ToString()));

            if (pwallet->mapAddressBook.count(address)) {
                entry.push_back(Pair("account", pwallet->mapAddressBook[address].name));
            }

            if (scriptPubKey.IsPayToScriptHash()) {
                const MCScriptID& hash = boost::get<MCScriptID>(address);
                MCScript redeemScript;
                if (pwallet->GetCScript(hash, redeemScript)) {
                    entry.push_back(Pair("redeemScript", HexStr(redeemScript.begin(), redeemScript.end())));
                }
            }
        }

        entry.push_back(Pair("scriptPubKey", HexStr(scriptPubKey.begin(), scriptPubKey.end())));
        entry.push_back(Pair("amount", ValueFromAmount(out.tx->tx->vout[out.i].nValue)));
        entry.push_back(Pair("confirmations", out.nDepth));
        entry.push_back(Pair("spendable", out.fSpendable));
        entry.push_back(Pair("solvable", out.fSolvable));
        entry.push_back(Pair("safe", out.fSafe));
        results.push_back(entry);
    }

    return results;
}

UniValue getbalanceof(const JSONRPCRequest& request)
{
	MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
	if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
		return NullUniValue;
	}

	if (request.fHelp || request.params.size() > 3)
		throw std::runtime_error(
			"getbalance ( \"account\" minconf include_watchonly )\n"
			"\nIf account is not specified, returns the server's total available balance.\n"
			"If account is specified (DEPRECATED), returns the balance in the account.\n"
			"Note that the account \"\" is not the same as leaving the parameter out.\n"
			"The server total may be different to the balance in the default \"\" account.\n"
			"\nArguments:\n"
			"1. \"account\"         (string, optional) DEPRECATED. The account string may be given as a\n"
			"                     specific account name to find the balance associated with wallet keys in\n"
			"                     a named account, or as the empty string (\"\") to find the balance\n"
			"                     associated with wallet keys not in any named account, or as \"*\" to find\n"
			"                     the balance associated with all wallet keys regardless of account.\n"
			"                     When this option is specified, it calculates the balance in a different\n"
			"                     way than when it is not specified, and which can count spends twice when\n"
			"                     there are conflicting pending transactions (such as those created by\n"
			"                     the bumpfee command), temporarily resulting in low or even negative\n"
			"                     balances. In general, account balance calculation is not considered\n"
			"                     reliable and has resulted in confusing outcomes, so it is recommended to\n"
			"                     avoid passing this argument.\n"
			"2. minconf           (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
			"3. include_watchonly (bool, optional, default=false) Also include balance in watch-only addresses (see 'importaddress')\n"
			"\nResult:\n"
			"amount              (numeric) The total amount in " + CURRENCY_UNIT + " received for this account.\n"
			"\nExamples:\n"
			"\nThe total amount in the wallet with 1 or more confirmations\n"
			+ HelpExampleCli("getbalance", "") +
			"\nThe total amount in the wallet at least 6 blocks confirmed\n"
			+ HelpExampleCli("getbalance", "\"*\" 6") +
			"\nAs a json rpc call\n"
			+ HelpExampleRpc("getbalance", "\"*\", 6")
		);

	LOCK2(cs_main, pwallet->cs_wallet);

	if (request.params.size() == 0)
		return  ValueFromAmount(pwallet->GetBalance());

	const std::string& account_param = request.params[0].get_str();
	const std::string* account = account_param != "*" ? &account_param : nullptr;

	int nMinDepth = 1;
	if (!request.params[1].isNull())
		nMinDepth = request.params[1].get_int();
	isminefilter filter = ISMINE_SPENDABLE;
	if (!request.params[2].isNull())
		if (request.params[2].get_bool())
			filter = filter | ISMINE_WATCH_ONLY;

	MagnaChainAddress btcaddr(account_param);
	if (!btcaddr.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain address");

	if (btcaddr.Get().type() != typeid(MCKeyID))
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain public key address");
	}

	const MCKeyID& kAddr = boost::get<MCKeyID>(btcaddr.Get());
	CoinListPtr plist = pcoinListDb->GetList((const uint160&)kAddr);
	//LogPrintf("Send contract address %s\n", addr.ToString());

	MCAmount nValue = 0;
	if (plist != nullptr) {
		BOOST_FOREACH(const MCOutPoint& outpoint, plist->coins) {
			const Coin& coin = pcoinsTip->AccessCoin(outpoint);
			if (coin.IsSpent()) {
				continue;
			}
			if (coin.IsCoinBase() && chainActive.Height() - coin.nHeight < COINBASE_MATURITY) {
				continue;
			}
			nValue += coin.out.nValue;
		}
	}
	return ValueFromAmount(nValue);
	//return ValueFromAmount(pwallet->GetLegacyBalance(filter, nMinDepth, account));
}

UniValue fundrawtransaction(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
                            "fundrawtransaction \"hexstring\" ( options )\n"
                            "\nAdd inputs to a transaction until it has enough in value to meet its out value.\n"
                            "This will not modify existing inputs, and will add at most one change output to the outputs.\n"
                            "No existing outputs will be modified unless \"subtractFeeFromOutputs\" is specified.\n"
                            "Note that inputs which were signed may need to be resigned after completion since in/outputs have been added.\n"
                            "The inputs added will not be signed, use signrawtransaction for that.\n"
                            "Note that all existing inputs must have their previous output transaction be in the wallet.\n"
                            "Note that all inputs selected must be of standard form and P2SH scripts must be\n"
                            "in the wallet using importaddress or addmultisigaddress (to calculate fees).\n"
                            "You can see whether this is the case by checking the \"solvable\" field in the listunspent output.\n"
                            "Only pay-to-pubkey, multisig, and P2SH versions thereof are currently supported for watch-only\n"
                            "\nArguments:\n"
                            "1. \"hexstring\"           (string, required) The hex string of the raw transaction\n"
                            "2. options                 (object, optional)\n"
                            "   {\n"
                            "     \"changeAddress\"          (string, optional, default pool address) The magnachain address to receive the change\n"
                            "     \"changePosition\"         (numeric, optional, default random) The index of the change output\n"
                            "     \"includeWatching\"        (boolean, optional, default false) Also select inputs which are watch only\n"
                            "     \"lockUnspents\"           (boolean, optional, default false) Lock selected unspent outputs\n"
                            "     \"feeRate\"                (numeric, optional, default not set: makes wallet determine the fee) Set a specific feerate (" + CURRENCY_UNIT + " per KB)\n"
                            "     \"subtractFeeFromOutputs\" (array, optional) A json array of integers.\n"
                            "                              The fee will be equally deducted from the amount of each specified output.\n"
                            "                              The outputs are specified by their zero-based index, before any change output is added.\n"
                            "                              Those recipients will receive less cells than you enter in their corresponding amount field.\n"
                            "                              If no outputs are specified here, the sender pays the fee.\n"
                            "                                  [vout_index,...]\n"
                            "     \"replaceable\"            (boolean, optional) Marks this transaction as BIP125 replaceable.\n"
                            "                              Allows this transaction to be replaced by a transaction with higher fees\n"
                            "     \"conf_target\"            (numeric, optional) Confirmation target (in blocks)\n"
                            "     \"estimate_mode\"          (string, optional, default=UNSET) The fee estimate mode, must be one of:\n"
                            "         \"UNSET\"\n"
                            "         \"ECONOMICAL\"\n"
                            "         \"CONSERVATIVE\"\n"
                            "   }\n"
                            "                         for backward compatibility: passing in a true instead of an object will result in {\"includeWatching\":true}\n"
                            "\nResult:\n"
                            "{\n"
                            "  \"hex\":       \"value\", (string)  The resulting raw transaction (hex-encoded string)\n"
                            "  \"fee\":       n,         (numeric) Fee in " + CURRENCY_UNIT + " the resulting transaction pays\n"
                            "  \"changepos\": n          (numeric) The position of the added change output, or -1\n"
                            "}\n"
                            "\nExamples:\n"
                            "\nCreate a transaction with no inputs\n"
                            + HelpExampleCli("createrawtransaction", "\"[]\" \"{\\\"myaddress\\\":0.01}\"") +
                            "\nAdd sufficient unsigned inputs to meet the output value\n"
                            + HelpExampleCli("fundrawtransaction", "\"rawtransactionhex\"") +
                            "\nSign the transaction\n"
                            + HelpExampleCli("signrawtransaction", "\"fundedtransactionhex\"") +
                            "\nSend the transaction\n"
                            + HelpExampleCli("sendrawtransaction", "\"signedtransactionhex\"")
                            );

    RPCTypeCheck(request.params, {UniValue::VSTR});

    MCCoinControl coinControl;
    int changePosition = -1;
    bool lockUnspents = false;
    UniValue subtractFeeFromOutputs;
    std::set<int> setSubtractFeeFromOutputs;

    if (!request.params[1].isNull()) {
      if (request.params[1].type() == UniValue::VBOOL) {
        // backward compatibility bool only fallback
        coinControl.fAllowWatchOnly = request.params[1].get_bool();
      }
      else {
        RPCTypeCheck(request.params, {UniValue::VSTR, UniValue::VOBJ});

        UniValue options = request.params[1];

        RPCTypeCheckObj(options,
            {
                {"changeAddress", UniValueType(UniValue::VSTR)},
                {"changePosition", UniValueType(UniValue::VNUM)},
                {"includeWatching", UniValueType(UniValue::VBOOL)},
                {"lockUnspents", UniValueType(UniValue::VBOOL)},
                {"reserveChangeKey", UniValueType(UniValue::VBOOL)}, // DEPRECATED (and ignored), should be removed in 0.16 or so.
                {"feeRate", UniValueType()}, // will be checked below
                {"subtractFeeFromOutputs", UniValueType(UniValue::VARR)},
                {"replaceable", UniValueType(UniValue::VBOOL)},
                {"conf_target", UniValueType(UniValue::VNUM)},
                {"estimate_mode", UniValueType(UniValue::VSTR)},
            },
            true, true);

        if (options.exists("changeAddress")) {
            MagnaChainAddress address(options["changeAddress"].get_str());

            if (!address.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "changeAddress must be a valid magnachain address");

            coinControl.destChange = address.Get();
        }

        if (options.exists("changePosition"))
            changePosition = options["changePosition"].get_int();

        if (options.exists("includeWatching"))
            coinControl.fAllowWatchOnly = options["includeWatching"].get_bool();

        if (options.exists("lockUnspents"))
            lockUnspents = options["lockUnspents"].get_bool();

        if (options.exists("feeRate"))
        {
            coinControl.m_feerate = MCFeeRate(AmountFromValue(options["feeRate"]));
            coinControl.fOverrideFeeRate = true;
        }

        if (options.exists("subtractFeeFromOutputs"))
            subtractFeeFromOutputs = options["subtractFeeFromOutputs"].get_array();

        if (options.exists("replaceable")) {
            coinControl.signalRbf = options["replaceable"].get_bool();
        }
        if (options.exists("conf_target")) {
            if (options.exists("feeRate")) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot specify both conf_target and feeRate");
            }
            coinControl.m_confirm_target = ParseConfirmTarget(options["conf_target"]);
        }
        if (options.exists("estimate_mode")) {
            if (options.exists("feeRate")) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot specify both estimate_mode and feeRate");
            }
            if (!FeeModeFromString(options["estimate_mode"].get_str(), coinControl.m_fee_mode)) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid estimate_mode parameter");
            }
        }
      }
    }

    // parse hex string from parameter
    MCMutableTransaction tx;
    if (!DecodeHexTx(tx, request.params[0].get_str(), true))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");

    if (tx.vout.size() == 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "TX must have at least one output");

    if (changePosition != -1 && (changePosition < 0 || (unsigned int)changePosition > tx.vout.size()))
        throw JSONRPCError(RPC_INVALID_PARAMETER, "changePosition out of bounds");

    for (unsigned int idx = 0; idx < subtractFeeFromOutputs.size(); idx++) {
        int pos = subtractFeeFromOutputs[idx].get_int();
        if (setSubtractFeeFromOutputs.count(pos))
            throw JSONRPCError(RPC_INVALID_PARAMETER, strprintf("Invalid parameter, duplicated position: %d", pos));
        if (pos < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, strprintf("Invalid parameter, negative position: %d", pos));
        if (pos >= int(tx.vout.size()))
            throw JSONRPCError(RPC_INVALID_PARAMETER, strprintf("Invalid parameter, position too large: %d", pos));
        setSubtractFeeFromOutputs.insert(pos);
    }

    MCAmount nFeeOut;
    std::string strFailReason;

    if (!pwallet->FundTransaction(tx, nFeeOut, changePosition, strFailReason, lockUnspents, setSubtractFeeFromOutputs, coinControl)) {
        throw JSONRPCError(RPC_WALLET_ERROR, strFailReason);
    }

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("hex", EncodeHexTx(tx)));
    result.push_back(Pair("changepos", changePosition));
    result.push_back(Pair("fee", ValueFromAmount(nFeeOut)));

    return result;
}

UniValue bumpfee(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    if (!EnsureWalletIsAvailable(pwallet, request.fHelp))
        return NullUniValue;

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2) {
        throw std::runtime_error(
            "bumpfee \"txid\" ( options ) \n"
            "\nBumps the fee of an opt-in-RBF transaction T, replacing it with a new transaction B.\n"
            "An opt-in RBF transaction with the given txid must be in the wallet.\n"
            "The command will pay the additional fee by decreasing (or perhaps removing) its change output.\n"
            "If the change output is not big enough to cover the increased fee, the command will currently fail\n"
            "instead of adding new inputs to compensate. (A future implementation could improve this.)\n"
            "The command will fail if the wallet or mempool contains a transaction that spends one of T's outputs.\n"
            "By default, the new fee will be calculated automatically using estimatefee.\n"
            "The user can specify a confirmation target for estimatefee.\n"
            "Alternatively, the user can specify totalFee, or use RPC settxfee to set a higher fee rate.\n"
            "At a minimum, the new fee rate must be high enough to pay an additional new relay fee (incrementalfee\n"
            "returned by getnetworkinfo) to enter the node's mempool.\n"
            "\nArguments:\n"
            "1. txid                  (string, required) The txid to be bumped\n"
            "2. options               (object, optional)\n"
            "   {\n"
            "     \"confTarget\"        (numeric, optional) Confirmation target (in blocks)\n"
            "     \"totalFee\"          (numeric, optional) Total fee (NOT feerate) to pay, in atomes.\n"
            "                         In rare cases, the actual fee paid might be slightly higher than the specified\n"
            "                         totalFee if the tx change output has to be removed because it is too close to\n"
            "                         the dust threshold.\n"
            "     \"replaceable\"       (boolean, optional, default true) Whether the new transaction should still be\n"
            "                         marked bip-125 replaceable. If true, the sequence numbers in the transaction will\n"
            "                         be left unchanged from the original. If false, any input sequence numbers in the\n"
            "                         original transaction that were less than 0xfffffffe will be increased to 0xfffffffe\n"
            "                         so the new transaction will not be explicitly bip-125 replaceable (though it may\n"
            "                         still be replaceable in practice, for example if it has unconfirmed ancestors which\n"
            "                         are replaceable).\n"
            "     \"estimate_mode\"     (string, optional, default=UNSET) The fee estimate mode, must be one of:\n"
            "         \"UNSET\"\n"
            "         \"ECONOMICAL\"\n"
            "         \"CONSERVATIVE\"\n"
            "   }\n"
            "\nResult:\n"
            "{\n"
            "  \"txid\":    \"value\",   (string)  The id of the new transaction\n"
            "  \"origfee\":  n,         (numeric) Fee of the replaced transaction\n"
            "  \"fee\":      n,         (numeric) Fee of the new transaction\n"
            "  \"errors\":  [ str... ] (json array of strings) Errors encountered during processing (may be empty)\n"
            "}\n"
            "\nExamples:\n"
            "\nBump the fee, get the new transaction\'s txid\n" +
            HelpExampleCli("bumpfee", "<txid>"));
    }

    RPCTypeCheck(request.params, {UniValue::VSTR, UniValue::VOBJ});
    uint256 hash;
    hash.SetHex(request.params[0].get_str());

    // optional parameters
    MCAmount totalFee = 0;
    MCCoinControl coin_control;
    coin_control.signalRbf = true;
    if (!request.params[1].isNull()) {
        UniValue options = request.params[1];
        RPCTypeCheckObj(options,
            {
                {"confTarget", UniValueType(UniValue::VNUM)},
                {"totalFee", UniValueType(UniValue::VNUM)},
                {"replaceable", UniValueType(UniValue::VBOOL)},
                {"estimate_mode", UniValueType(UniValue::VSTR)},
            },
            true, true);

        if (options.exists("confTarget") && options.exists("totalFee")) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "confTarget and totalFee options should not both be set. Please provide either a confirmation target for fee estimation or an explicit total fee for the transaction.");
        } else if (options.exists("confTarget")) { // TODO: alias this to conf_target
            coin_control.m_confirm_target = ParseConfirmTarget(options["confTarget"]);
        } else if (options.exists("totalFee")) {
            totalFee = options["totalFee"].get_int64();
            if (totalFee <= 0) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, strprintf("Invalid totalFee %s (must be greater than 0)", FormatMoney(totalFee)));
            }
        }

        if (options.exists("replaceable")) {
            coin_control.signalRbf = options["replaceable"].get_bool();
        }
        if (options.exists("estimate_mode")) {
            if (!FeeModeFromString(options["estimate_mode"].get_str(), coin_control.m_fee_mode)) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid estimate_mode parameter");
            }
        }
    }

    LOCK2(cs_main, pwallet->cs_wallet);
    EnsureWalletIsUnlocked(pwallet);

    MCFeeBumper feeBump(pwallet, hash, coin_control, totalFee);
    BumpFeeResult res = feeBump.getResult();
    if (res != BumpFeeResult::OK)
    {
        switch(res) {
            case BumpFeeResult::INVALID_ADDRESS_OR_KEY:
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, feeBump.getErrors()[0]);
                break;
            case BumpFeeResult::INVALID_REQUEST:
                throw JSONRPCError(RPC_INVALID_REQUEST, feeBump.getErrors()[0]);
                break;
            case BumpFeeResult::INVALID_PARAMETER:
                throw JSONRPCError(RPC_INVALID_PARAMETER, feeBump.getErrors()[0]);
                break;
            case BumpFeeResult::WALLET_ERROR:
                throw JSONRPCError(RPC_WALLET_ERROR, feeBump.getErrors()[0]);
                break;
            default:
                throw JSONRPCError(RPC_MISC_ERROR, feeBump.getErrors()[0]);
                break;
        }
    }

    // sign bumped transaction
    if (!feeBump.signTransaction(pwallet)) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Can't sign transaction.");
    }
    // commit the bumped transaction
    if(!feeBump.commit(pwallet)) {
        throw JSONRPCError(RPC_WALLET_ERROR, feeBump.getErrors()[0]);
    }
    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("txid", feeBump.getBumpedTxId().GetHex()));
    result.push_back(Pair("origfee", ValueFromAmount(feeBump.getOldFee())));
    result.push_back(Pair("fee", ValueFromAmount(feeBump.getNewFee())));
    UniValue errors(UniValue::VARR);
    for (const std::string& err: feeBump.getErrors())
        errors.push_back(err);
    result.push_back(Pair("errors", errors));

    return result;
}

UniValue getaddresscoins(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() < 1 || request.params.size() > 5)
		throw std::runtime_error(
			"getaddresscoins fromaddress\n"
			"\nGet coins by magnachain address\n"
			"\nArguments:\n"
			"1. \"address\"                      (string, required) The address for input coins\n"
            "2. \"withscript\"                   (bool, optional) Option for return script or not, default false.\n"
			"\nReturns the coins of the address\n"
			"\nResult:\n"
			"[                   (array of json object)\n"
			"  {\n"
			"    \"txhash\" : xxx,            (string) The txid in hex\n"
			"    \"outn\" : xxx,              (string) The output index of vout array,number of " + CURRENCY_UNIT + "\n"
			"    \"value\" : xxx,             (string) The amount of output\n"
			"    \"script\" : xxx,            (string) The scriptPubKey in hex format\n"
            "    \"confirmations\" : xxx,     (number) The coin height\n"
			"  }\n"
			"  ,...\n"
			"]\n"
			"\nExamples:\n"
			+ HelpExampleCli("getaddresscoins", "XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj")
			+ HelpExampleRpc("getaddresscoins", "XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj")
		);

	MagnaChainAddress fromaddress(request.params[0].get_str());
	if (!fromaddress.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain from address");

	MCKeyID kFromKeyId;
    if (!fromaddress.GetKeyID(kFromKeyId)){
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain public key address");
    }

    bool fwithscript = false;
    if (request.params.size() >= 2)
    {
        if (request.params[1].isBool())
            fwithscript = request.params[1].get_bool();
        else if (request.params[1].isStr() && (request.params[1].get_str() == "true" || request.params[1].get_str() == "1"))
            fwithscript = true;
    }

	CoinListPtr plist = pcoinListDb->GetList((const uint160&)kFromKeyId);

	UniValue uvalCoins(UniValue::VARR);
	MCAmount nValue = 0;
	if (plist != nullptr) {
		BOOST_FOREACH(const MCOutPoint& outpoint, plist->coins) {
			const Coin& coin = pcoinsTip->AccessCoin(outpoint);
			if (coin.IsSpent()) {
				continue;
			}
			if (coin.IsCoinBase() && chainActive.Height() - coin.nHeight < COINBASE_MATURITY) {
				continue;
			}

			nValue += coin.out.nValue;
			UniValue uvalCoin((UniValue::VOBJ));
			uvalCoin.push_back(Pair("txhash", outpoint.hash.GetHex()));
			uvalCoin.push_back(Pair("outn", int(outpoint.n)));
			uvalCoin.push_back(Pair("value", ValueFromAmount(coin.out.nValue)));
            if(fwithscript){
			    uvalCoin.push_back(Pair("script", HexStr(coin.out.scriptPubKey.begin(), coin.out.scriptPubKey.end())));
                uvalCoin.push_back(Pair("script_asm", ScriptToAsmStr(coin.out.scriptPubKey, true)));
            }
            uvalCoin.push_back(Pair("confirmations", int(chainActive.Height() - coin.nHeight)));
			uvalCoins.push_back(uvalCoin);
		}
	}

	return uvalCoins;
}

//test code
UniValue posttransaction(const std::string& strHexTx)
{
	MCMutableTransaction mtx;
	if (!DecodeHexTx(mtx, strHexTx))
	{
		return "";
	}
	MCTransaction txNewConst(mtx);
	for (int i = 0; i < mtx.vin.size(); i++)
	{
		MCTxIn in = mtx.vin[i];
		Coin coin = pcoinsTip->AccessCoin(in.prevout);
		const MCScript& scriptPubKey = coin.out.scriptPubKey;
		SignatureData sigdata;

		if (!ProduceSignature(TransactionSignatureCreator(::vpwallets[0], &txNewConst, i, coin.out.nValue, SIGHASH_ALL), scriptPubKey, sigdata))
		{
			return "";
		}
		else {
			UpdateTransaction(mtx, i, sigdata);
		}
	}

	MCTransactionRef tx = MakeTransactionRef(std::move(mtx));
	MCValidationState state;
	MCAmount maxTxFee = DEFAULT_TRANSACTION_MAXFEE;
	bool ret = AcceptToMemoryPool(mempool, state, tx, true, nullptr, nullptr, false, maxTxFee);

	return "";
}

//构造一个交易，从一个地址上的转币到另外一个地址，尚未签名 
void SendFromToOther(MCWalletTx &wtxNew, const MagnaChainAddress &fromaddress, const MagnaChainAddress &toaddress, const MagnaChainAddress &changeaddress, const MCAmount nAmount, const MCAmount nUserFee, SmartLuaState* sls)
{
	MCScript scriptPubKey = GetScriptForDestination(toaddress.Get());
	SendFromToOther(wtxNew, fromaddress, scriptPubKey, changeaddress, nAmount, nUserFee, sls);
}

void SendFromToOther(MCWalletTx &wtxNew, const MagnaChainAddress &fromaddress, const MCScript &toScript, const MagnaChainAddress &changeaddress, const MCAmount nAmount, const MCAmount nUserFee, SmartLuaState* sls)
{
//	if (fromaddress.Get().type() != typeid(MCKeyID))
//	{
//		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain public key address");
//	}

    MCKeyID kFromKeyId;
    if (!fromaddress.GetKeyID(kFromKeyId)){
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain public key address");
    }

	CoinListPtr plist = pcoinListDb->GetList((const uint160&)kFromKeyId);

	std::set<MCOutPoint> setInOutPoints;
	std::vector<Coin> vCoin;
	MCAmount nValue = 0;
	if (plist != nullptr) {
		BOOST_FOREACH(const MCOutPoint& outpoint, plist->coins) {
			const Coin& coin = pcoinsTip->AccessCoin(outpoint);
			if (coin.IsSpent()) {
				continue;
			}
			if (coin.IsCoinBase() && chainActive.Height() - coin.nHeight < COINBASE_MATURITY) {
				continue;
			}

			nValue += coin.out.nValue;
			setInOutPoints.insert(outpoint);
			vCoin.push_back(coin);
			// TODO 
			//	if ((nUserFee > 0 && nValue >= nAmount + nUserFee)) {// include more fee to make sure later success
			//		break;
			//	}
		}
	}

	if (nUserFee > 0 && nValue < nAmount + nUserFee)
	{
		throw JSONRPCError(RPC_VERIFY_REJECTED, "Not enough spendable coin for send and fee");
	}

	MCAmount curBalance = nValue;
	bool fSubtractFeeFromAmount = false;
	
	std::vector<MCRecipient> vecSend;
    if (nAmount > 0)
    {
        MCRecipient recip = { toScript, nAmount, fSubtractFeeFromAmount };
        vecSend.push_back(recip);
    }

	MCFakeWallet fakeWallet;
	fakeWallet.m_ownKeys.insert(kFromKeyId);

	MCCoinControl coin_control;
	coin_control.destChange = changeaddress.Get();
	coin_control.fAllowOtherInputs = false;
	for (MCOutPoint outpoint : setInOutPoints)
	{
		//coin_control.Select(outpoint);//select by MCWallet later ,AvailableCoins 有问题 
		MCWalletTx wtxIn;
		MCTransactionRef txOutpoint;
		uint256 hash = outpoint.hash;
		uint256 hashBlock;
		if (GetTransaction(hash, txOutpoint, Params().GetConsensus(), hashBlock, true))
		{
			wtxIn.tx = txOutpoint;
			wtxIn.hashBlock = hashBlock;
			wtxIn.nIndex = 0;//just for cheat //wtxIn.SetMerkleBranch();
			wtxIn.BindWallet(&fakeWallet);
			std::pair<std::map<uint256, MCWalletTx>::iterator, bool> ret = fakeWallet.mapWallet.insert(std::make_pair(hash, wtxIn));
		}
	}

	MCReserveKey reservekey(&fakeWallet);
	int nChangePosRet = -1;
	MCAmount nFeeRequired;
	std::string strError;
	if (!fakeWallet.CreateTransaction(vecSend, wtxNew, reservekey, nFeeRequired, nChangePosRet, strError, coin_control, false, sls)) {
		if (!fSubtractFeeFromAmount && nValue + nFeeRequired > curBalance)
			strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
		throw JSONRPCError(RPC_WALLET_ERROR, strError);
	}
}

UniValue premaketransaction(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() < 4 || request.params.size() > 5)
		throw std::runtime_error(
			"premaketransaction fromaddress toaddress changeaddress amount \n"
			"\n rebroadcast the branch chain transaction by txid, in case that transction has not be send to the target chain .\n"
			"\nArguments:\n"
			"1. \"fromaddress\"                  (string, required) The address for input coins\n"
			"2. \"toaddress\"                    (string, required) Send to address\n"
			"3. \"changeaddress\"                (string, required) The address for change coins\n"
			"4. \"amount\"                       (numeric or string, required) The amount in " + CURRENCY_UNIT + " to send. eg 0.1\n"
			"5. \"fee\"                          (numeric or string, optional) The amount in " + CURRENCY_UNIT + " to for fee eg 0.0001, default 0 and will calc fee by system\n"
			"\nReturns the hash of the created branch chain.\n"
			"\nResult:\n"
			"{\n"
			"  \"txhex\" : xxx,            (string) The transaction hex data\n"
			"  \"coins\" :\n"
			"      [\n"
			"        {\n"
			"           \"txhash\" : xxx   (string) txid\n"
			"           \"outn\" : xxx     (string) vout index\n"
			"           \"value\" : xxx    (string) vout value\n"
			"           \"script\" : xxx   (string) vout lock script in hex format\n"
			"        },...\n"
			"      ]\n"
			"}\n"
			"\nExamples:\n"
			+ HelpExampleCli("premaketransaction", "XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPX XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPi XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj 1 0.0005")
			+ HelpExampleRpc("premaketransaction", "XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPX XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPi XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj 1 ")
		);

	MagnaChainAddress fromaddress(request.params[0].get_str());
	if (!fromaddress.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain from address");

	MagnaChainAddress toaddress(request.params[1].get_str());
	if (!toaddress.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain to address");

	MagnaChainAddress changeaddress(request.params[2].get_str());
	if (!changeaddress.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain change address");

	// Amount
	MCAmount nAmount = AmountFromValue(request.params[3]);
	if (nAmount <= 0)
		throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

	MCAmount nUserFee = 0;
	if (request.params.size() > 4)
	{
		nUserFee = AmountFromValue(request.params[4]);
	}

	MCWalletTx wtxNew;
	///////////////////////////////////////////////////////
	SendFromToOther(wtxNew, fromaddress, toaddress, changeaddress, nAmount, nUserFee);
	
	UniValue ret(UniValue::VOBJ);
	ret.push_back(Pair("txhex", EncodeHexTx(*wtxNew.tx, RPCSerializationFlags())));
	//return coins info
	UniValue uvalCoins(UniValue::VARR);
	for (MCTxIn txin : wtxNew.tx->vin)
	{
		const Coin& coin = pcoinsTip->AccessCoin(txin.prevout);
		
		UniValue uvalCoin((UniValue::VOBJ));
		uvalCoin.push_back(Pair("txhash", txin.prevout.hash.GetHex()));
		uvalCoin.push_back(Pair("outn", int(txin.prevout.n)));
		uvalCoin.push_back(Pair("value", ValueFromAmount(coin.out.nValue)));
		uvalCoin.push_back(Pair("script", HexStr(coin.out.scriptPubKey.begin(), coin.out.scriptPubKey.end())));
		uvalCoins.push_back(uvalCoin);
	}
	ret.push_back(Pair("coins", uvalCoins));
// test
//	posttransaction(EncodeHexTx(tx, RPCSerializationFlags()));
	return ret;
}

//获取当期挖矿币 copy from `listunspent` and modify
UniValue listmortgagecoins(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 5)
        throw std::runtime_error(
            "listmortgagecoins ( minconf maxconf  [\"addresses\",...] [include_unsafe] [query_options])\n"
            "\nReturns array of unspent transaction outputs\n"
            "with between minconf and maxconf (inclusive) confirmations.\n"
            "Optionally filter to only include txouts paid to specified addresses.\n"
            "\nArguments:\n"
            "1. minconf          (numeric, optional, default=1) The minimum confirmations to filter\n"
            "2. maxconf          (numeric, optional, default=9999999) The maximum confirmations to filter\n"
            "3. \"addresses\"      (string) A json array of bitcoin addresses to filter\n"
            "    [\n"
            "      \"address\"     (string) bitcoin address\n"
            "      ,...\n"
            "    ]\n"
            "4. include_unsafe (bool, optional, default=true) Include outputs that are not safe to spend\n"
            "                  See description of \"safe\" attribute below.\n"
            "5. query_options    (json, optional) JSON with query options\n"
            "    {\n"
            "      \"minimumAmount\"    (numeric or string, default=0) Minimum value of each UTXO in " + CURRENCY_UNIT + "\n"
            "      \"maximumAmount\"    (numeric or string, default=unlimited) Maximum value of each UTXO in " + CURRENCY_UNIT + "\n"
            "      \"maximumCount\"     (numeric or string, default=unlimited) Maximum number of UTXOs\n"
            "      \"minimumSumAmount\" (numeric or string, default=unlimited) Minimum sum value of all UTXOs in " + CURRENCY_UNIT + "\n"
            "    }\n"
            "\nResult\n"
            "[                   (array of json object)\n"
            "  {\n"
            "    \"txid\" : \"txid\",          (string) the transaction id \n"
            "    \"vout\" : n,               (numeric) the vout value\n"
            "    \"address\" : \"address\",    (string) the bitcoin address\n"
            "    \"account\" : \"account\",    (string) DEPRECATED. The associated account, or \"\" for the default account\n"
            "    \"scriptPubKey\" : \"key\",   (string) the script key\n"
            "    \"amount\" : x.xxx,         (numeric) the transaction output amount in " + CURRENCY_UNIT + "\n"
            "    \"confirmations\" : n,      (numeric) The number of confirmations\n"
            "    \"redeemScript\" : n        (string) The redeemScript if scriptPubKey is P2SH\n"
            "    \"spendable\" : xxx,        (bool) Whether we have the private keys to spend this output\n"
            "    \"solvable\" : xxx,         (bool) Whether we know how to spend this output, ignoring the lack of keys\n"
            "    \"safe\" : xxx              (bool) Whether this output is considered safe to spend. Unconfirmed transactions\n"
            "                              from outside keys and unconfirmed replacement transactions are considered unsafe\n"
            "                              and are not eligible for spending by fundrawtransaction and sendtoaddress.\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples\n"
            + HelpExampleCli("listmortgagecoins", "")
            + HelpExampleCli("listmortgagecoins", "6 9999999 \"[\\\"1PGFqEzfmQch1gKD3ra4k18PNj3tTUUSqg\\\",\\\"1LtvqCaApEdUGFkpKMM4MstjcaL4dKg8SP\\\"]\"")
            + HelpExampleRpc("listmortgagecoins", "6, 9999999 \"[\\\"1PGFqEzfmQch1gKD3ra4k18PNj3tTUUSqg\\\",\\\"1LtvqCaApEdUGFkpKMM4MstjcaL4dKg8SP\\\"]\"")
            + HelpExampleCli("listmortgagecoins", "6 9999999 '[]' true '{ \"minimumAmount\": 0.005 }'")
            + HelpExampleRpc("listmortgagecoins", "6, 9999999, [] , true, { \"minimumAmount\": 0.005 } ")
        );

    int nMinDepth = 1;
    if (request.params.size() > 0 && !request.params[0].isNull()) {
        RPCTypeCheckArgument(request.params[0], UniValue::VNUM);
        nMinDepth = request.params[0].get_int();
    }

    int nMaxDepth = 9999999;
    if (request.params.size() > 1 && !request.params[1].isNull()) {
        RPCTypeCheckArgument(request.params[1], UniValue::VNUM);
        nMaxDepth = request.params[1].get_int();
    }

    std::set<MagnaChainAddress> setAddress;
    if (request.params.size() > 2 && !request.params[2].isNull()) {
        RPCTypeCheckArgument(request.params[2], UniValue::VARR);
        UniValue inputs = request.params[2].get_array();
        for (unsigned int idx = 0; idx < inputs.size(); idx++) {
            const UniValue& input = inputs[idx];
            MagnaChainAddress address(input.get_str());
            if (!address.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid Bitcoin address: ") + input.get_str());
            if (setAddress.count(address))
                throw JSONRPCError(RPC_INVALID_PARAMETER, std::string("Invalid parameter, duplicated address: ") + input.get_str());
            setAddress.insert(address);
        }
    }

    bool include_unsafe = true;
    if (request.params.size() > 3 && !request.params[3].isNull()) {
        RPCTypeCheckArgument(request.params[3], UniValue::VBOOL);
        include_unsafe = request.params[3].get_bool();
    }

    MCAmount nMinimumAmount = 0;
    MCAmount nMaximumAmount = MAX_MONEY;
    MCAmount nMinimumSumAmount = MAX_MONEY;
    uint64_t nMaximumCount = 0;

    if (!request.params[4].isNull()) {
        const UniValue& options = request.params[4].get_obj();

        if (options.exists("minimumAmount"))
            nMinimumAmount = AmountFromValue(options["minimumAmount"]);

        if (options.exists("maximumAmount"))
            nMaximumAmount = AmountFromValue(options["maximumAmount"]);

        if (options.exists("minimumSumAmount"))
            nMinimumSumAmount = AmountFromValue(options["minimumSumAmount"]);

        if (options.exists("maximumCount"))
            nMaximumCount = options["maximumCount"].get_int64();
    }

    UniValue results(UniValue::VARR);
    std::vector<MCOutput> vecOutputs;
    assert(pwallet != nullptr);
    LOCK2(cs_main, pwallet->cs_wallet);

    pwallet->AvailableMortgageCoins(vecOutputs, !include_unsafe, BST_MORTGAGE_ALL, nullptr, nMinimumAmount, nMaximumAmount, nMinimumSumAmount, nMaximumCount, nMinDepth, nMaxDepth);
    for (const MCOutput& out : vecOutputs) {
        MCTxDestination address;
        const MCScript& scriptPubKey = out.tx->tx->vout[out.i].scriptPubKey;
        bool fValidAddress = ExtractDestination(scriptPubKey, address);

        if (setAddress.size() && (!fValidAddress || !setAddress.count(address)))
            continue;

        UniValue entry(UniValue::VOBJ);
        entry.push_back(Pair("txid", out.tx->GetHash().GetHex()));
        entry.push_back(Pair("vout", out.i));

        if (fValidAddress) {
            entry.push_back(Pair("address", MagnaChainAddress(address).ToString()));

            if (pwallet->mapAddressBook.count(address)) {
                entry.push_back(Pair("account", pwallet->mapAddressBook[address].name));
            }

            if (scriptPubKey.IsPayToScriptHash()) {
                const MCScriptID& hash = boost::get<MCScriptID>(address);
                MCScript redeemScript;
                if (pwallet->GetCScript(hash, redeemScript)) {
                    entry.push_back(Pair("redeemScript", HexStr(redeemScript.begin(), redeemScript.end())));
                }
            }
        }

        entry.push_back(Pair("scriptPubKey", HexStr(scriptPubKey.begin(), scriptPubKey.end())));
        entry.push_back(Pair("amount", ValueFromAmount(out.tx->tx->vout[out.i].nValue)));
        entry.push_back(Pair("confirmations", out.nDepth));
        entry.push_back(Pair("spendable", out.fSpendable));
        entry.push_back(Pair("solvable", out.fSolvable));
        entry.push_back(Pair("safe", out.fSafe));
        results.push_back(entry);
    }

    return results;
}

extern UniValue abortrescan(const JSONRPCRequest& request); // in rpcdump.cpp
extern UniValue dumpprivkey(const JSONRPCRequest& request); // in rpcdump.cpp
extern UniValue dumppubkey(const JSONRPCRequest& request);
extern UniValue importprivkey(const JSONRPCRequest& request);
extern UniValue importaddress(const JSONRPCRequest& request);
extern UniValue importpubkey(const JSONRPCRequest& request);
extern UniValue dumpwallet(const JSONRPCRequest& request);
extern UniValue importwallet(const JSONRPCRequest& request);
extern UniValue importprunedfunds(const JSONRPCRequest& request);
extern UniValue removeprunedfunds(const JSONRPCRequest& request);
extern UniValue importmulti(const JSONRPCRequest& request);

UniValue startbranch(const JSONRPCRequest& request);


UniValue generateforbranch(const JSONRPCRequest& request);


UniValue getbranchinfo(const JSONRPCRequest& request);
UniValue switchbranch(const JSONRPCRequest& request);


static const CRPCCommand commands[] =
{ //  category              name                        actor (function)           okSafeMode
    //  --------------------- ------------------------    -----------------------    ----------
    { "rawtransactions",    "fundrawtransaction",       &fundrawtransaction,       false,  {"hexstring","options"} },
    { "hidden",             "resendwallettransactions", &resendwallettransactions, true,   {} },
    { "wallet",             "abandontransaction",       &abandontransaction,       false,  {"txid"} },
    { "wallet",             "abortrescan",              &abortrescan,              false,  {} },
    { "wallet",             "addmultisigaddress",       &addmultisigaddress,       true,   {"nrequired","keys","account"} },
    { "wallet",             "addwitnessaddress",        &addwitnessaddress,        true,   {"address"} },
    { "wallet",             "backupwallet",             &backupwallet,             true,   {"destination"} },
    { "wallet",             "bumpfee",                  &bumpfee,                  true,   {"txid", "options"} },
    { "wallet",             "dumpprivkey",              &dumpprivkey,              true,   {"address"}  },
    { "wallet",             "dumppubkey",               &dumppubkey,               true,   {"address" } },
    { "wallet",             "dumpwallet",               &dumpwallet,               true,   {"filename"} },
    { "wallet",             "encryptwallet",            &encryptwallet,            true,   {"passphrase"} },
    { "wallet",             "getaccountaddress",        &getaccountaddress,        true,   {"account"} },
    { "wallet",             "getaccount",               &getaccount,               true,   {"address"} },
    { "wallet",             "getaddressesbyaccount",    &getaddressesbyaccount,    true,   {"account"} },
    { "wallet",             "getbalance",               &getbalance,               false,  {"account","minconf","include_watchonly"} },
	{ "wallet",             "getbalanceof",             &getbalanceof,             false,  { "address" } },
	{ "wallet",             "getnewaddress",            &getnewaddress,            true,   {"account"} },
    { "wallet",             "getrawchangeaddress",      &getrawchangeaddress,      true,   {} },
    { "wallet",             "getreceivedbyaccount",     &getreceivedbyaccount,     false,  {"account","minconf"} },
    { "wallet",             "getreceivedbyaddress",     &getreceivedbyaddress,     false,  {"address","minconf"} },
    { "wallet",             "gettransaction",           &gettransaction,           false,  {"txid","include_watchonly"} },
    { "wallet",             "getunconfirmedbalance",    &getunconfirmedbalance,    false,  {} },
    { "wallet",             "getwalletinfo",            &getwalletinfo,            false,  {} },
    { "wallet",             "importmulti",              &importmulti,              true,   {"requests","options"} },
    { "wallet",             "importprivkey",            &importprivkey,            true,   {"privkey","label","rescan"} },
    { "wallet",             "importwallet",             &importwallet,             true,   {"filename"} },
    { "wallet",             "importaddress",            &importaddress,            true,   {"address","label","rescan","p2sh"} },
    { "wallet",             "importprunedfunds",        &importprunedfunds,        true,   {"rawtransaction","txoutproof"} },
    { "wallet",             "importpubkey",             &importpubkey,             true,   {"pubkey","label","rescan"} },
    { "wallet",             "keypoolrefill",            &keypoolrefill,            true,   {"newsize"} },
    { "wallet",             "listaccounts",             &listaccounts,             false,  {"minconf","include_watchonly"} },
    { "wallet",             "listaddressgroupings",     &listaddressgroupings,     false,  {} },
    { "wallet",             "listlockunspent",          &listlockunspent,          false,  {} },
    { "wallet",             "listreceivedbyaccount",    &listreceivedbyaccount,    false,  {"minconf","include_empty","include_watchonly"} },
    { "wallet",             "listreceivedbyaddress",    &listreceivedbyaddress,    false,  {"minconf","include_empty","include_watchonly"} },
    { "wallet",             "listsinceblock",           &listsinceblock,           false,  {"blockhash","target_confirmations","include_watchonly","include_removed"} },
    { "wallet",             "listtransactions",         &listtransactions,         false,  {"account","count","skip","include_watchonly"} },
    { "wallet",             "listunspent",              &listunspent,              false,  {"minconf","maxconf","addresses","include_unsafe","query_options"} },
    { "wallet",             "listwallets",              &listwallets,              true,   {} },
    { "wallet",             "lockunspent",              &lockunspent,              true,   {"unlock","transactions"} },
    { "wallet",             "move",                     &movecmd,                  false,  {"fromaccount","toaccount","amount","minconf","comment"} },
    { "wallet",             "sendfrom",                 &sendfrom,                 false,  {"fromaccount","toaddress","amount","minconf","comment","comment_to"} },
    { "wallet",             "sendmany",                 &sendmany,                 false,  {"fromaccount","amounts","minconf","comment","subtractfeefrom","replaceable","conf_target","estimate_mode"} },
    { "wallet",             "sendtoaddress",            &sendtoaddress,            false,  {"address","amount","comment","comment_to","subtractfeefromamount","replaceable","conf_target","estimate_mode"} },

	{ "wallet",             "publishcontract",			&publishcontract,          false,  {"filename"} },
	{ "wallet",             "publishcontractcode",      &publishcontractcode,      false,  {"codehex"} },
	{ "wallet",             "callcontract",				&callcontract,                     false,  {"contract address"} },
	{ "wallet",             "setaccount",               &setaccount,               true,   {"address","account"} },
    { "wallet",             "settxfee",                 &settxfee,                 true,   {"amount"} },
    { "wallet",             "signmessage",              &signmessage,              true,   {"address","message"} },
    { "wallet",             "walletlock",               &walletlock,               true,   {} },
    { "wallet",             "walletpassphrasechange",   &walletpassphrasechange,   true,   {"oldpassphrase","newpassphrase"} },
    { "wallet",             "walletpassphrase",         &walletpassphrase,         true,   {"passphrase","timeout"} },
    { "wallet",             "removeprunedfunds",        &removeprunedfunds,        true,   {"txid"} },

    { "wallet",             "getaddresscoins",          &getaddresscoins,          true,  { "address", "withscript",} },
    { "wallet",             "premaketransaction",       &premaketransaction,       false,  { "fromaddress","toaddress","changeaddress", "amount" } },
	{ "wallet",             "prepublishcode",           &prepublishcode,           false,  {}},
	{ "wallet",             "precallcontract",          &precallcontract,          false,  {}},

    { "wallet",             "listmortgagecoins",        &listmortgagecoins,        false,{ "minconf","maxconf","addresses","include_unsafe","query_options" } },
  //  { "wallet",             "posttransaction",          &posttransaction,       false,{ "hextx", } },
};

void RegisterWalletRPCCommands(CRPCTable &t)
{
    if (gArgs.GetBoolArg("-disablewallet", false))
        return;

    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
