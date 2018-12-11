// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "rpc/branchchainrpc.h"
#include "chain/branchchain.h"

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
#include "validation/validation.h"
#include "wallet/rpcwallet.h"
#include "univalue.h"
#include "utils/util.h"
#include "consensus/merkle.h"
#include "smartcontract/smartcontract.h"

#include <boost/foreach.hpp>
#include <stdint.h>
#include <sstream>

#include "chain/branchdb.h"

//创建侧链抵押金计算
MCAmount GetCreateBranchMortgage(const MCBlock* pBlock, const MCBlockIndex* pBlockIndex)
{
    if (pBlockIndex == nullptr && pBlock != nullptr)
    {
        if (mapBlockIndex.count(pBlock->GetHash()))
        {
            pBlockIndex = mapBlockIndex[pBlock->GetHash()];
        }
    }

    size_t nSize = 0;

    const BranchChainTxRecordsDb::CREATE_BRANCH_TX_CONTAINER& vCreated = pBranchChainTxRecordsDb->GetCreateBranchTxsInfo();
    if (pBlockIndex == nullptr)
        nSize = pBranchChainTxRecordsDb->GetCreateBranchSize();
    else
    {
        int64_t blockheigt = pBlockIndex->nHeight;
        for (auto v : vCreated)
        {
            if (mapBlockIndex.count(v.blockhash))//verifydb的时候会disconnect并检查最近的几个块
            {
                if (blockheigt > mapBlockIndex[v.blockhash]->nHeight)
                    nSize++;
            }
            else
                nSize++;//no info just inc for safe
        }
    }

    size_t powN = std::max((size_t)0, nSize);
    powN = std::min((size_t)MaxPowForCreateChainMortgage, powN);

    MCAmount mortgage = CreateBranchChainMortgage * std::pow(2, powN);
    return mortgage;
}

// OP: make a cache will be better
static bool GetTransactionDataByTxInfo(const uint256 &txhash, MCTransactionRef &tx, MCBlockIndex** ppblockindex, uint32_t &tx_vtx_index, MCBlock& block)
{
    BranchChainTxInfo chainsendinfo = pBranchChainTxRecordsDb->GetBranchChainTxInfo(txhash);
    if (chainsendinfo.IsInit() == false)
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("Load transaction sendinfo fail."));
    }

    uint256 blockhash = chainsendinfo.blockhash;
    if (mapBlockIndex.count(blockhash) == 0)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

    *ppblockindex = mapBlockIndex[blockhash];
    MCBlockIndex* pblockindex = *ppblockindex;
    if (fHavePruned && !(pblockindex->nStatus & BLOCK_HAVE_DATA) && pblockindex->nTx > 0)
        throw JSONRPCError(RPC_MISC_ERROR, "Block not available (pruned data)");

    if (!ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))
        throw JSONRPCError(RPC_MISC_ERROR, "Block not found on disk");

    if (chainsendinfo.txindex >= block.vtx.size())
    {
        throw JSONRPCError(RPC_MISC_ERROR, "Send Info data error!!");
    }

    tx = block.vtx[chainsendinfo.txindex];
    if (tx->GetHash() != txhash)
        throw JSONRPCError(RPC_MISC_ERROR, "Send Info data error!!");
    tx_vtx_index = chainsendinfo.txindex;

    return true;
}

bool MakeBranchTransStep2Tx(MCMutableTransaction& branchTx, const MCScript& scriptPubKey, const MCAmount& nAmount, MCAmount &fee, MCCoinControl &coin_control)
{
    const std::string strFromChain = Params().GetBranchId();

    branchTx.nVersion = MCTransaction::TRANS_BRANCH_VERSION_S2;
    branchTx.nLockTime = 0;
    branchTx.fromBranchId = strFromChain;
    //    branchTx.fromTx = ;//this value set later.
    branchTx.pPMT.reset(new MCSpvProof()); //will reset later, 这里防止序列化时报错

    branchTx.vin.resize(1);// (set v[0] diff from MCTransaction::IsCoinBase)
    branchTx.vin[0].prevout.hash.SetNull();
    branchTx.vin[0].prevout.n = 0;// make prevout is not Null any more
    branchTx.vin[0].scriptSig.clear();
    branchTx.vout.resize(1);
    branchTx.vout[0].scriptPubKey = scriptPubKey;
    branchTx.vout[0].nValue = nAmount;

    unsigned int nBytes = GetVirtualTransactionSize(branchTx);
    FeeCalculation feeCalc;
    fee = MCWallet::GetMinimumFee(nBytes, coin_control, ::mempool, ::feeEstimator, &feeCalc);// step 2 will include step 1.

    branchTx.inAmount = nAmount + fee;
    return true;
}

uint256 GetBranchTxHash(const MCTransaction& tx)
{
    if (tx.IsBranchChainTransStep2() && tx.fromBranchId != MCBaseChainParams::MAIN) {
        return RevertTransaction(tx, nullptr).GetHash();
    }
    return tx.GetHash();
}

UniValue createbranchchain(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 3 || request.params.size() > 3)
        throw std::runtime_error(
                "createbranchchain vseeds seedspec6\n"
                "\n create a branch chain.\n"
                "\nArguments:\n"
                "1. \"vseeds\"             (string, required) The vSeeds address, eg \"vseeds1.com;vseeds2.com;vseeds3.com\" \n"
                "2. \"seedspec6\"          (string, required) The SeedSpec6 a 16-byte IPv6 address and a port, eg \"00:00:00:00:00:00:00:00:00:00:ff:ff:c0:a8:3b:80:8333\" \n"
                "3. \"mortgageaddress\"     (string, required) The magnachain Address, use to receive mortgage in main chain\n"
                "\nReturns the hash of the created branch chain.\n"
                "\nResult:\n"
                "{\n"
                "  \"txid\" : xxxx,        (string) The txid\n"
                "  \"branchid\" : xxxx,    (string) The branch id\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("createbranchchain", "vseeds.com 00:00:00:00:00:00:00:00:00:00:ff:ff:c0:a8:3b:80:8333 XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB")
                + HelpExampleRpc("createbranchchain", "vseeds.com 00:00:00:00:00:00:00:00:00:00:ff:ff:c0:a8:3b:80:8333 XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB")
                );

    LOCK2(cs_main, pwallet->cs_wallet);

    if (!Params().IsMainChain())
    {
        throw JSONRPCError(RPC_TYPE_ERROR, "Only main chain can create branch");
    }
    if (mempool.GetCreateBranchChainTxCount() != 0)
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, "mempool has unconfirmed create branch chain transaction");
    }

    const MCAmount nAmount = GetCreateBranchMortgage();

    std::string strVSeeds = request.params[0].get_str();
    std::string strSeedSpec6 = request.params[1].get_str();
    std::string strMortgageAddress = request.params[2].get_str();
    MagnaChainAddress kAddress(strMortgageAddress);
    if (!kAddress.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid magnachain address for genesis block address");
    MCKeyID mortgagekey;
    if (!kAddress.GetKeyID(mortgagekey)){
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid MagnaChain public key address");
    }

    MCCoinControl coin_control;

    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::CREATE_BRANCH_VERSION;
    wtx.branchVSeeds = strVSeeds;
    wtx.branchSeedSpec6 = strSeedSpec6;

    MCScript scriptPubKey;// create branch pubkey hash
    scriptPubKey << OP_CREATE_BRANCH << OP_DUP << OP_HASH160 << ToByteVector(mortgagekey) << OP_EQUALVERIFY << OP_CHECKSIG;

    bool fSubtractFeeFromAmount = false;
    EnsureWalletIsUnlocked(pwallet);
    //SendMoney
    MCAmount curBalance = pwallet->GetBalance();
    if (nAmount > curBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Create and send the transaction
    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    int nChangePosRet = -1;
    MCRecipient recipient = { scriptPubKey, nAmount, fSubtractFeeFromAmount };
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    //end send money
    uint256 hashTx = wtx.GetHash();
    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("txid", hashTx.GetHex()));
    obj.push_back(Pair("branchid", hashTx.GetHex()));
    if (!state.GetRejectReason().empty())
        obj.push_back(Pair("commit_transaction_reject_reason", state.GetRejectReason()));
    return obj;
}

//获取创建分支的信息 
UniValue getbranchchaininfo(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "getbranchchaininfo branchid\n"
                "\n get a created branch chain info.\n"
                "\nArguments:\n"
                "1. \"branchid\"            (string, required) The branch txid.\n"
                "\nReturns the hash of the created branch chain.\n"
                "\nResult:\n"
                "{\n"
                "  \"txid\" : xxxxx,          (string) The txid\n"
                "  \"vseeds\" : xxxxx,        (string) The vseeds\n"
                "  \"seedspec6\" : xxxxx,     (string) The seedspec6\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("getbranchchaininfo", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e")
                + HelpExampleRpc("getbranchchaininfo", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e")
                );

    if (!Params().IsMainChain())
        throw std::runtime_error("Branch chain has not any branch chain info.");

    //std::string branchid = request.params[0].get_str();
    uint256 hash = ParseHashV(request.params[0], "parameter 1");
    
    MCTransactionRef tx;
    MCBlockIndex* pblockindex = nullptr;
    uint32_t tx_vtx_index = 0;
    MCBlock block;

    LOCK(cs_main);
    if (GetTransactionDataByTxInfo(hash, tx, &pblockindex, tx_vtx_index, block)==false)
        throw JSONRPCError(RPC_VERIFY_ERROR, "GetTransactDataByTxInfo fail");

    if (tx->IsBranchCreate() == false) {
        throw JSONRPCError(RPC_VERIFY_ERROR, "Invalid branchid");
    }

    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("txid", tx->GetHash().GetHex()));
    obj.push_back(Pair("vseeds", tx->branchVSeeds));
    obj.push_back(Pair("seedspec6", tx->branchSeedSpec6));
    return obj;
}

UniValue getallbranchinfo(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() > 0)
        throw std::runtime_error(
                "getallbranchinfo\n"
                "\n get all created branch chain info\n"
                "\nArguments:\n"
                "\nReturns the hash of the created branch chain.\n"
                "\nResult:\n"
                "[\n"
                "  {\n"
                "    \"txid\" : xxxxx,          (string) The txid\n"
                "    \"vseeds\" : xxxxx,        (string) The vseeds\n"
                "    \"seedspec6\" : xxxxx,     (string) The seedspec6\n"
                "    \"confirmations\" : heigh  (number) The confirmation of this transaction\n"
                "    \"ismaturity\" : bval      (bool) Whether the branch chain is maturity\n"
                "  }\n"
                "...\n"
                "]\n"
                "\nExamples:\n"
                + HelpExampleCli("getallbranchinfo", "")
                + HelpExampleRpc("getallbranchinfo", "")
                );

    if (!Params().IsMainChain())
        throw std::runtime_error("Branch chain has not any branch chain info.");

    LOCK(cs_main);
    UniValue arr(UniValue::VARR);
    const BranchChainTxRecordsDb::CREATE_BRANCH_TX_CONTAINER& vCreated = pBranchChainTxRecordsDb->GetCreateBranchTxsInfo();

    bool fRegTest = gArgs.GetBoolArg("-regtest", false);
    bool fTestNet = gArgs.GetBoolArg("-testnet", false);
    int branchInitDefaultPort = GetBranchInitDefaultPort(fTestNet, fRegTest);
    for (auto v: vCreated)
    {
        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("txid", v.txid.GetHex()));
        obj.push_back(Pair("vseeds", v.branchVSeeds));
        obj.push_back(Pair("seedspec6", v.branchSeedSpec6));
        int confirmations = 0;
        if (mapBlockIndex.count(v.blockhash))
        {
            confirmations = chainActive.Height() - mapBlockIndex[v.blockhash]->nHeight + 1;
        }
        obj.push_back(Pair("confirmations", confirmations));
        obj.push_back(Pair("defaultport", branchInitDefaultPort++));
        obj.push_back(Pair("ismaturity", confirmations >= BRANCH_CHAIN_MATURITY));
        arr.push_back(obj);
    }
    return arr;
}

//添加分支节点
UniValue addbranchnode(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 5 || request.params.size() > 6)
        throw std::runtime_error(
                "addbranchnode branchid ip port username password\n"
                "\n get a created branch chain info.\n"
                "\nArguments:\n"
                "1. \"branchid\"            (string, required) The branch txid.\n"
                "2. \"ip\"                  (string, required) Branch node ip.\n"
                "3. \"port\"                (string, required) Branch node rpc port.\n"
                "4. \"usrname\"             (string, required) Branch node rpc username.\n"
                "5. \"password\"            (string, required) Branch node rpc password.\n"
                "6. \"wallet\"              (string, optional) Rpc wallet\n"
                "\nReturns the hash of the created branch chain.\n"
                "\nResult:\n"
                "    Ok or fail\n"
                "\nExamples:\n"
                + HelpExampleCli("addbranchnode", "4bebbe9c21ab00ca6d899d6cfe6600dc4d7e2b7f0842beba95c44abeedb42ea2 127.0.0.1 9201 \"\" clpwd")
                + HelpExampleRpc("addbranchnode", "4bebbe9c21ab00ca6d899d6cfe6600dc4d7e2b7f0842beba95c44abeedb42ea2 127.0.0.1 9201 \"\" clpwd")
                );

    std::string branchid = request.params[0].get_str();
    std::string ip = request.params[1].get_str();
    std::string port = request.params[2].get_str();
    std::string username = request.params[3].get_str();
    std::string password = request.params[4].get_str();
    std::string strWallet;
    if (request.params.size() > 5){
        strWallet = request.params[5].get_str();
    }

    MCRPCConfig rpcconfig;
    rpcconfig.strIp = ip;
    rpcconfig.iPort = atoi(port);
    rpcconfig.strUser = username;
    rpcconfig.strPassword = password;
    rpcconfig.strWallet = strWallet;

    if (branchid != MCBaseChainParams::MAIN && (branchid.length() != 64 || !IsHex(branchid)))
    {
        throw JSONRPCError(RPC_TYPE_ERROR, std::string("Invalid branchid"));
    }

    if (branchid != MCBaseChainParams::MAIN)
    {
        uint256 branchhash = ParseHashV(branchid, "parameter 1");

        MCTransactionRef txBranch;
        MCBlockIndex* pblockindex = nullptr;
        uint32_t tx_vtx_index = 0;
        MCBlock block;
        if (!GetTransactionDataByTxInfo(branchhash, txBranch, &pblockindex, tx_vtx_index, block))
            throw JSONRPCError(RPC_VERIFY_ERROR, "GetTransactDataByTxInfo fail");

        if (txBranch->IsBranchCreate() == false) {
            throw JSONRPCError(RPC_VERIFY_ERROR, "Invalid branchid");
        }
    }

    g_branchChainMan->ReplaceRpcConfig(branchid, rpcconfig);

    //test connect
    UniValue params(UniValue::VARR);
    UniValue reply = CallRPC(rpcconfig, "getbalance", params);
    const UniValue& result = find_value(reply, "result");
    const UniValue& errorVal = find_value(reply, "error");
    if (!errorVal.isNull()){
        return errorVal.write();
    }

    return "ok";
}

/**
 * 跨链交易
 * 
 */ 
UniValue sendtobranchchain(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 3 || request.params.size() > 3)
        throw std::runtime_error(
                "sendtobranchchain branchid address amount,main branchid address is \"main\". \n"
                "\n Send an amount to a branch chain's address.\n"
                "\nArguments:\n"
                "1. \"branchid\"             (string, required) Send to target chain's Branchid,if send to main chain, branchid is \"main\".\n"
                "2. \"address\"              (string, required) The target chain's magnachain address to send to.\n"
                "3. \"amount\"               (numeric or string, required) The amount in " + CURRENCY_UNIT + " to send. eg 0.1\n"
                "\nReturns the hash of the created branch chain.\n"
                "\nResult:\n"
                "\"txid\"                  (string) The transaction id.\n"
                "\nExamples:\n"
                + HelpExampleCli("sendtobranchchain", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e XS8XpbMxF5qkDp61SEaa94pwKY6UW6UQd9 0.1")
                + HelpExampleRpc("sendtobranchchain", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e XS8XpbMxF5qkDp61SEaa94pwKY6UW6UQd9 0.1")
                );

    LOCK2(cs_main, pwallet->cs_wallet);

    std::string strToBranchid = request.params[0].get_str();
    if (Params().GetBranchId() == strToBranchid)
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "can not send to this chain.");
    }

    uint256 tobranchhash;
    // only allow branch-chain to main-chain, main-chain to branch-chain, not allow branch to branch
    if (strToBranchid != MCBaseChainParams::MAIN)
    {
        tobranchhash = ParseHashV(request.params[0], "parameter 1");
        if (!Params().IsMainChain())
        {
            throw JSONRPCError(RPC_WALLET_ERROR, "can not trans from branch-chain to branch-chain");
        }

        //check is branch exist
        //创建分支交易有一定的高度后才允许跨链交易 
        MCTransactionRef txBranch;
        MCBlockIndex* pblockindex = nullptr;
        uint32_t tx_vtx_index = 0;
        MCBlock block;
        if (!GetTransactionDataByTxInfo(tobranchhash, txBranch, &pblockindex, tx_vtx_index, block))
            throw JSONRPCError(RPC_VERIFY_ERROR, "GetTransactDataByTxInfo fail");

        if (txBranch->IsBranchCreate() == false) {
            throw JSONRPCError(RPC_VERIFY_ERROR, "Invalid branchid");
        }

        int confirmations = chainActive.Height() - pblockindex->nHeight + 1;
        if (confirmations < BRANCH_CHAIN_MATURITY) {
            throw JSONRPCError(RPC_VERIFY_ERROR, "Invalid branch transaction,not get enough confirmations");
        }
    }
    else
    {
        if (Params().IsMainChain())
        {
            throw JSONRPCError(RPC_WALLET_ERROR, "can not trans from main-chain to main-chain");
        }
    }

    const std::string& strAddress = request.params[1].get_str();
    MagnaChainAddress address(strAddress);
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid magnachain str address");

    MCKeyID keyId;
    if (!address.GetKeyID(keyId))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid magnachain keyid");
    }

    MCAmount nAmount = AmountFromValue(request.params[2]);
    if (!MoneyRange(nAmount))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

    MCCoinControl coin_control;

    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::TRANS_BRANCH_VERSION_S1;
    wtx.sendToBranchid = strToBranchid;

    //make branch transaction output
    MCAmount fee2 = 0;
    MCScript sendToScriptPubKey = GetScriptForDestination(address.Get());
    MCMutableTransaction branchStep2MTx;
    if (MakeBranchTransStep2Tx(branchStep2MTx, sendToScriptPubKey, nAmount, fee2, coin_control) == false) {
        throw JSONRPCError(RPC_TYPE_ERROR, "Make branch step2 tx error.");
    }
    MCTransactionRef branchStep2Tx = MakeTransactionRef(std::move(branchStep2MTx));
    wtx.sendToTxHexData = EncodeHexTx(*branchStep2Tx, RPCSerializationFlags());

    MCScript scriptPubKey;
    if (strToBranchid != MCBaseChainParams::MAIN){
        scriptPubKey << OP_TRANS_BRANCH << ToByteVector(tobranchhash);
    }
    else{
        scriptPubKey << OP_RETURN << OP_TRANS_BRANCH;//burn the output, trans output to /dev/null 
    }

    EnsureWalletIsUnlocked(pwallet);

    bool fSubtractFeeFromAmount = false;
    //SendMoney
    MCAmount curBalance = pwallet->GetBalance();
    if (nAmount + fee2 > curBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Create and send the transaction
    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    int nChangePosRet = -1;
    MCRecipient recipient = { scriptPubKey, nAmount + fee2, fSubtractFeeFromAmount };
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + fee2 + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    // end send money
    uint256 hashTx = wtx.GetHash();
    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txid", hashTx.GetHex()));
    if (!state.GetRejectReason().empty())
        ret.push_back(Pair("commit_transaction_reject_reason", state.GetRejectReason()));
    return ret;
}

UniValue makebranchtransaction(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "makebranchtransaction branchid address amount \n"
                "\n Send an amount to a branch chain's address.\n"
                "\nArguments:\n"
                "1. \"hexdata\"               (string, required) The transaction hex data.\n"
                "\nReturns the hash of the created branch chain.\n"
                "\nResult:\n"
                "\"txid\"                  (string) The transaction id.\n"
                "\nExamples:\n"
                + HelpExampleCli("makebranchtransaction", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e")
                + HelpExampleRpc("makebranchtransaction", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e")
                );

    const std::string& strTx1HexData = request.params[0].get_str();
    MCMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, strTx1HexData))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "DecodeHexTx tx hex fail.\n");
    }

    if (!mtxTrans1.IsPregnantTx())
        throw JSONRPCError(RPC_WALLET_ERROR, "Transaction is not a valid chain trans step1");

    MCMutableTransaction mtxTrans2;
    if (!DecodeHexTx(mtxTrans2, mtxTrans1.sendToTxHexData))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "sendToTxHexData is not a valid transaction data.");
    }

    if (mtxTrans2.IsBranchChainTransStep2() == false)
        throw JSONRPCError(RPC_WALLET_ERROR, "mtxTrans2 is not a branch chain for step2.");

    const std::string strToChainId = mtxTrans1.sendToBranchid;
    if (strToChainId != Params().GetBranchId())
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "Target branch id is not valid.");
    }

    if (mtxTrans2.fromBranchId == Params().GetBranchId())
        throw JSONRPCError(RPC_WALLET_ERROR, "From chain error,can not from this chain");

    const std::string& strFromChain = mtxTrans2.fromBranchId;
    // only allow branch-chain to main-chain, main-chain to branch-chain, not allow branch to branch
    if (strFromChain != MCBaseChainParams::MAIN)
    {
        if (mtxTrans1.IsMortgage())
        {
            throw JSONRPCError(RPC_WALLET_ERROR, "mortgage transaction must create in main chain");
        }
        if (!Params().IsMainChain())
        {
            throw JSONRPCError(RPC_WALLET_ERROR, "can not trans from branch-chain to branch-chain");
        }
    }
    else
    {
        if (Params().IsMainChain())
        {
            throw JSONRPCError(RPC_WALLET_ERROR, "can not trans from main-chain to main-chain");
        }
    }

    std::string strFromtxid = mtxTrans1.GetHash().ToString();
    //set delay fields
    if (mtxTrans1.IsMortgage())//change 将主链抵押币转成挖矿币
    {
        uint256 branchid;
        MCKeyID keyid;
        int64_t coinheight;
        if (GetMortgageMineData(mtxTrans1.vout[0].scriptPubKey, &branchid, &keyid, &coinheight))
        {
            mtxTrans2.vout[0].scriptPubKey = MCScript() << OP_MINE_BRANCH_COIN << ToByteVector(mtxTrans1.GetHash()) << coinheight << OP_2DROP << OP_DUP << OP_HASH160 << ToByteVector(keyid) << OP_EQUALVERIFY << OP_CHECKSIG;
        }
    }
    if (mtxTrans2.fromBranchId != MCBaseChainParams::MAIN){
        mtxTrans2.pPMT.reset(new MCSpvProof(*mtxTrans1.pPMT));
        mtxTrans1.pPMT.reset(new MCSpvProof()); // clear pPMT from mtxTrans1
    }
    MCTransaction tx1(mtxTrans1);
    MCVectorWriter cvw{ SER_NETWORK, INIT_PROTO_VERSION, mtxTrans2.fromTx, 0, tx1 };
    MCTransactionRef tx2 = MakeTransactionRef(std::move(mtxTrans2));

    LOCK(cs_main);

    const MCChainParams& chainparams = Params();
    MCValidationState state;
    MCAmount maxTxFee = DEFAULT_TRANSACTION_MAXFEE;
    bool ret = AcceptToMemoryPool(mempool, state, tx2, true, nullptr, nullptr, false, maxTxFee);
    if (ret == false)
    {
        std::string strError = strprintf("Error: accept to memory pool fail: %s", state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError.c_str());
    }

    //broadcast transaction
    if (!g_connman)
    {
        MCInv inv(MSG_TX, tx2->GetHash());
        g_connman->ForEachNode([&inv](MCNode* pnode)
                {
                pnode->PushInventory(inv);
                });
    }

    return "ok";
}

//获取发起跨链交易的tx是否存在 
//跨链交易查询,用于验证跨链验证,核对其他链是否真的发起跨连交易 
UniValue getbranchchaintransaction(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "getbranchchaintransaction txid amount \n"
                "\n Send an amount to a branch chain's address.\n"
                "\nArguments:\n"
                "1. \"txid\"                  (string, required) The txid.\n"
                "\nReturns the hash of the created branch chain.\n"
                "\nResult:\n"
                "{\n"
                "  \"txid\" : xxx,           (string) The txid\n"
                "  \"hex\" : xxx,            (string) The tx hex data\n"
                "  \"confirmations\" : n     (int) The confirmations\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("getbranchchaintransaction", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e")
                + HelpExampleRpc("getbranchchaintransaction", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e")
                );

    uint256 txhash = ParseHashV(request.params[0], "parameter 1");

    MCTransactionRef tx;
    MCBlockIndex* pblockindex = nullptr;
    uint32_t tx_vtx_index = 0;
    MCBlock block;

    //LOCK(cs_main);
    if(!GetTransactionDataByTxInfo(txhash, tx, &pblockindex, tx_vtx_index, block))
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("GetTransactDataByTxInfo fail"));

    if (!tx->IsPregnantTx())
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("Invalid branch transaction."));
    }

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txid", tx->GetHash().GetHex()));
    ret.push_back(Pair("hex", EncodeHexTx(*tx, RPCSerializationFlags())));

    //block info
    int confirmations = chainActive.Height() - pblockindex->nHeight + 1;
    ret.push_back(Pair("confirmations", confirmations));

    return ret;
}

UniValue rebroadcastchaintransaction(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "rebroadcastchaintransaction txid \n"
                "\n rebroadcast the branch chain transaction by txid, in case that transction has not be send to the target chain .\n"
                "\nArguments:\n"
                "1. \"txid\"                  (string, required) The txid.\n"
                "\nReturns the hash of the created branch chain.\n"
                "\nResult:\n"
                "\"ret\"                  (string) ok or false\n"
                "\nExamples:\n"
                + HelpExampleCli("rebroadcastchaintransaction", "5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db")
                + HelpExampleRpc("rebroadcastchaintransaction", "5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db")
                );

    uint256 txhash = ParseHashV(request.params[0], "parameter 1");
    MCTransactionRef tx;
    MCBlockIndex* pblockindex = nullptr;
    uint32_t tx_vtx_index = 0;
    MCBlock block;

    //LOCK(cs_main);
    if (!GetTransactionDataByTxInfo(txhash, tx, &pblockindex, tx_vtx_index, block))
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("GetTransactDataByTxInfo fail"));

    if (!tx->IsPregnantTx())
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("Invalid branch transaction!"));
    }

    const uint32_t maturity = BRANCH_CHAIN_MATURITY;
    int confirmations = chainActive.Height() - pblockindex->nHeight + 1;
    if (confirmations < maturity + 1)
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("can not broadcast because no enough confirmations"));
    }

    std::string strError;
    if (BranchChainTransStep2(tx, block, &strError) == false)
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, strprintf("send to target chain return fail: %s", strError));
    }

    return "ok";
}


// 侧链挖矿抵押。在主链产生抵押币,侧链产生对于的挖矿币
// 1、主链将币抵押到抵押脚本,脚本包含侧链的id,赎回地址
//    MINE_BRANCH_MORTGAGE交易 
//         vout[0] 是抵押币 vout[1] 是侧链手续费,发送到null地址上 vout[2] 可能的找零 
//         
UniValue mortgageminebranch(const JSONRPCRequest& request)
{
    MCWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;

    }
    if (request.fHelp || request.params.size() < 3 || request.params.size() > 3)
        throw std::runtime_error(
                "mortgageminebranch branchid coinamount\n"
                "\n mortgage coin to mine branch chain\n"
                "\nArguments:\n"
                "1. \"branchid\"             (string, required) The branch id\n"
                "2. \"amount\"               (numeric or string, required) The amount in " + CURRENCY_UNIT + " to send. eg 100.0\n"
                "3. \"address\"              (string, required) The magnachain address for Redeem\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "\"ret\"                  (string) ok or fail\n"
                "\nExamples:\n"
                + HelpExampleCli("mortgageminebranch", "5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db 100 XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB")
                + HelpExampleRpc("mortgageminebranch", "5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db 100 XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB")
                );

    if (!Params().IsMainChain())
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("Only in main chain can mortgage coin for mining branch!"));
    }

    LOCK2(cs_main, pwallet->cs_wallet);

    uint256 branchHash = ParseHashV(request.params[0], "parameter 1");
    std::string strBranchid = request.params[0].get_str();

    // branch chain validation check
        MCTransactionRef txBranch;
        MCBlockIndex* pblockindex = nullptr;
        uint32_t tx_vtx_index = 0;
        MCBlock block;
        if (!GetTransactionDataByTxInfo(branchHash, txBranch, &pblockindex, tx_vtx_index, block))
            throw JSONRPCError(RPC_VERIFY_ERROR, "GetTransactDataByTxInfo fail");

        if (txBranch->IsBranchCreate() == false) {
            throw JSONRPCError(RPC_VERIFY_ERROR, "Invalid branchid");
        }

        int confirmations = chainActive.Height() - pblockindex->nHeight + 1;
        if (confirmations < BRANCH_CHAIN_MATURITY) {
            throw JSONRPCError(RPC_VERIFY_ERROR, "Invalid branch transaction,not get enough confirmations");
        }
    
    MCAmount nAmount = AmountFromValue(request.params[1]);
    if (!MoneyRange(nAmount))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

    int64_t quotient = MIN_MINE_BRANCH_MORTGAGE / COIN;
    int64_t remainder = MIN_MINE_BRANCH_MORTGAGE % COIN;
    if (nAmount < MIN_MINE_BRANCH_MORTGAGE)
        throw JSONRPCError(RPC_TYPE_ERROR, strprintf("MINE MORTGAGE at least %d.%08d COIN", quotient, remainder));

    const std::string& strAddress = request.params[2].get_str();
    MagnaChainAddress address(strAddress);
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid magnachain pubkey hash address");
    if (address.IsScript())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Can not use script address");
    }
    MCKeyID pubKeyId;
    if (!address.GetKeyID(pubKeyId))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid magnachain keyid");
    }

    int nCoinHeight = 0;//指定币高度
    // 抵押vout脚本 branch id , magnachain coin address
    const MCScript scriptPubKey = MCScript() << OP_MINE_BRANCH_MORTGAGE << ToByteVector(branchHash) << (nCoinHeight) << OP_2DROP << OP_DUP << OP_HASH160 << ToByteVector(pubKeyId) << OP_EQUALVERIFY << OP_CHECKSIG;

    MCCoinControl coin_control;

    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::MINE_BRANCH_MORTGAGE;
    wtx.sendToBranchid = strBranchid;

    //make branch transaction output
    MCAmount fee2 = 0;
    const MCScript sendToScriptPubKey;// empty script, delay set in fucntion `makebranchtransaction` 
    MCMutableTransaction branchStep2MTx;// 侧链交易,将抵押币转成挖矿币,输出脚本先是个空脚本,在侧链收录该交易时,该输出地址会转成挖矿币脚本
    if (MakeBranchTransStep2Tx(branchStep2MTx, sendToScriptPubKey, nAmount, fee2, coin_control) == false) { 
        throw JSONRPCError(RPC_TYPE_ERROR, "Make branch step2 tx error.");
    }
    MCTransactionRef branchStep2Tx = MakeTransactionRef(std::move(branchStep2MTx));
    wtx.sendToTxHexData = EncodeHexTx(*branchStep2Tx, RPCSerializationFlags());

    //check balance
    MCAmount curBalance = pwallet->GetBalance();
    if (nAmount + fee2 > curBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // 特殊设定：vout 0号位是抵押币 1号位侧链交易手续费 2号位可能是找零、也可能是空（没找零的情况）。
    bool fSubtractFeeFromAmount = false;
    // Create and send the transaction
    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;

    MCRecipient recipient = {scriptPubKey, nAmount, fSubtractFeeFromAmount};
    vecSend.push_back(recipient);// vout 0 是抵押币
    //侧链手续费
    MCScript scriptBranchFee;
    scriptBranchFee << OP_TRANS_BRANCH << ToByteVector(branchHash);
    vecSend.push_back({scriptBranchFee, fee2, fSubtractFeeFromAmount}); // vout 1 是侧链手续费

    int nChangePosRet = vecSend.size(); // vout end-1 找零, fixed change vout pos 
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + fee2 + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    // end send money
    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txid", wtx.GetHash().GetHex()));
    if (!state.GetRejectReason().empty())
    {
        ret.push_back(Pair("commit_reject_reason", state.GetRejectReason()));
    }
    return ret;
}

// 侧链向主链提交区块头
UniValue submitbranchblockinfo(const JSONRPCRequest& request)
{
    MCWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "submitbranchblockinfo \"CTransaction hex data\"\n"
                "\nInclude branch block data to a transaction, then send to main chain\n"
                "\nArguments:\n"
                "1. \"transaction_of_branch_block_data\"             (string, required) The transaction data in dex string format.\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "\"ret\"                  (string) ok or fail\n"
                "\nExamples:\n"
                + HelpExampleCli("submitbranchblockinfo", "5754f9e...630db")
                + HelpExampleRpc("submitbranchblockinfo", "5754f9e...630db")
                );

    if (!Params().IsMainChain())
        throw JSONRPCError(RPC_INVALID_PARAMS, "This rpc api only be called in branch chain");

    LOCK2(cs_main, pwallet->cs_wallet);

    const std::string& strTx1HexData = request.params[0].get_str();
    MCMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, strTx1HexData))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "DecodeHexTx tx hex fail.\n");
    }
    if (!mtxTrans1.IsSyncBranchInfo())
        throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid transaction data");

    ///////////
    MCCoinControl coin_control;

    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::SYNC_BRANCH_INFO;
    wtx.pBranchBlockData = std::move(mtxTrans1.pBranchBlockData);
    wtx.isDataTransaction = true;

    bool fSubtractFeeFromAmount = false;
    EnsureWalletIsUnlocked(pwallet);
    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    MCAmount curBalance = pwallet->GetBalance();
    // Create and send the transaction
    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    int nChangePosRet = -1;
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    ///////////
    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txid", wtx.GetHash().GetHex()));
    if (!state.GetRejectReason().empty())
    {
        ret.push_back(Pair("commit_reject_reason", state.GetRejectReason()));
    }
    return ret;
}

// 赎回挖矿抵押

//查询侧链有效头高度
UniValue getbranchchainheight(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "getbranchchainheight \"branchid\"\n"
                "\nget branch chain top block height\n"
                "\nArguments:\n"
                "1. \"branchid\"             (string, required) The branch id.\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "{\n"
                "  \"blockhash\" : xxx,           (string) Block hash\n"
                "  \"height\"    : xxx,           (number) Block height\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("getbranchchainheight", "5754f9e...630db")
                + HelpExampleRpc("getbranchchainheight", "5754f9e...630db")
                );

    if (!Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not call this RPC in branch chain!\n");

    LOCK(cs_main);
    uint256 branchid = ParseHashV(request.params[0], "parameter 1");
    if (!pBranchChainTxRecordsDb->IsBranchCreated(branchid))
        throw JSONRPCError(RPC_WALLET_ERROR, "Branch which you query did not created");

    UniValue retObj(UniValue::VOBJ);
    retObj.push_back(Pair("blockhash", g_pBranchDb->GetBranchTipHash(branchid).ToString()));
    retObj.push_back(Pair("height", (uint64_t)g_pBranchDb->GetBranchHeight(branchid)));
    return retObj;
}

//重发侧链头到主链
UniValue resendbranchchainblockinfo(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "resendbranchchainblockinfo \"height\"\n"
                "\nResend branch chain block info by height\n"
                "\nArguments:\n"
                "1. \"height\"             (number, required) The block height.\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "\n"
                "\nExamples:\n"
                + HelpExampleCli("resendbranchchainblockinfo", "height")
                + HelpExampleRpc("resendbranchchainblockinfo", "height")
                );

    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not call this RPC in main chain!\n");

    int64_t blockheight;
    if (request.params[0].isNum())
        blockheight = request.params[0].get_int64();
    else{
        if (!ParseInt64(request.params[0].get_str(), &blockheight))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Params[0] is a invalid number\n");
    }

    if (blockheight > chainActive.Height())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Request height larger than chain height\n");

    LOCK(cs_main);
    MCBlock block;
    MCBlockIndex* pBlockIndex = chainActive[blockheight];
    if (!ReadBlockFromDisk(block, pBlockIndex, Params().GetConsensus()))
        throw JSONRPCError(RPC_MISC_ERROR, "Block not found on disk");

    std::shared_ptr<const MCBlock> shared_pblock = std::make_shared<const MCBlock>(block);
    std::string strErr;
    if (!SendBranchBlockHeader(shared_pblock, &strErr))
    {
        return "Fail:" + strErr;
    }
    return "OK";
}

//赎回挖矿币, 步骤
// 1).侧链提起赎回请求.(侧链先销毁挖矿币,防止继续挖矿)
// 2).主链收到,创造新的交易,抵押币作为输入,赎回到正常地址,需要指定来自那个侧链请求
// 如果是主链先发起请求的,而且是先拿回抵押币的话,可能侧链还在继续挖矿.
// 这个交易和前面跨链交易不一样,原先"转到"侧链成为挖矿币的输入并没有销毁,可以作为转入s2时的输入.
// 赎回挖矿币, 步骤1
UniValue redeemmortgagecoinstatement(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 2)
        throw std::runtime_error(
                "redeemmortgagecoinstatement \"txid\" \"outindex\"\n"
                "\nRedeem mortgage coin by outpoint info(txid and vout index of coin)\n"
                "\nArguments:\n"
                "1. \"txid\"             (string, required) The transaction hash of coin.\n"
                "2. \"voutindex\"         (number, required) The vout index of coin, default is 0.\n"
                "\nReturns txid.\n"
                "\nResult:\n"
                "{\n"
                "  \"txid\": xx,   (string) The new create transaction txid\n"
                "  \"commit_transaction_reject_reason\": xxx, (string) If has reject reason will contain this field\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("redeemmortgagecoinstatement", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 0")
                + HelpExampleRpc("redeemmortgagecoinstatement", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 0")
                );

    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "This RPC API Only be called in branch chain!\n");

    LOCK2(cs_main, pwallet->cs_wallet);

    uint256 txid = ParseHashV(request.params[0], "parameter 1");
    int32_t n = 0;
    if (request.params.size() > 1){
        if (request.params[1].isNum())
            n = request.params[1].get_int();
        else
            ParseInt32(request.params[1].get_str(), &n);
    }

    MCOutPoint outpoint(txid, n);
    const Coin& coin = pcoinsTip->AccessCoin(outpoint);
    if (coin.IsSpent())
        throw JSONRPCError(RPC_WALLET_ERROR, "Coin is spent!");
    if (chainActive.Height() - coin.nHeight < REDEEM_SAFE_HEIGHT)// 挖矿币需要满足一定高度后才能赎回,给别人举报有时间窗口
        throw JSONRPCError(RPC_INVALID_REQUEST, strprintf(std::string("Coin need %s confirmation", REDEEM_SAFE_HEIGHT)));

    uint256 fromtxid;
    MCKeyID keyid;
    int64_t height;
    if (!GetMortgageCoinData(coin.out.scriptPubKey, &fromtxid, &keyid, &height))
        throw JSONRPCError(RPC_INVALID_REQUEST, "Invalid mortgage coin.");

    MCCoinControl coin_control;
    coin_control.Select(outpoint);
    coin_control.fAllowOtherInputs = true;

    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::REDEEM_MORTGAGE_STATEMENT;

    MCScript scriptPubKey;
    scriptPubKey << OP_RETURN << OP_REDEEM_MORTGAGE << ToByteVector(fromtxid);// output trans to /dev/null

    EnsureWalletIsUnlocked(pwallet);

    bool fSubtractFeeFromAmount = false;
    //SendMoney
    MCAmount curBalance = pwallet->GetBalance();
    MCAmount nAmount = coin.out.nValue;
    //if (nAmount > curBalance)
    //    throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Create and send the transaction
    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    int nChangePosRet = -1;
    MCRecipient recipient = { scriptPubKey, nAmount, fSubtractFeeFromAmount };
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    // end send money

    uint256 hashTx = wtx.GetHash();
    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("txid", hashTx.GetHex()));
    if (!state.GetRejectReason().empty())
        obj.push_back(Pair("commit_transaction_reject_reason", state.GetRejectReason()));
    return obj;
}

//当步骤1交易满足一定块高度后会自动调用一次,只有本地钱包包含抵押地址的私钥才能成功, 也可以在满足高度后手动调用
//赎回挖矿币,步骤2
UniValue redeemmortgagecoin(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 5 || request.params.size() > 5)
        throw std::runtime_error(
                "redeemmortgagecoin \"txid\" \"outindex\" \"statementtxid\"\n"
                "\nRedeem mortgage coin by outpoint info(txid and vout index of coin)\n"
                "\nArguments:\n"
                "1. \"txid\"             (string, required) The transaction hash of coin in main chain(MCOutPoint hash).\n"
                "2. \"voutindex\"        (number, required) The vout index of coin, default is 0(MCOutPoint n).\n"
                "3. \"fromtx\"           (string, required) The statement transactoin hex data in branch chain.\n"
                "4. \"frombranchid\"     (string, required) Which branch id the coin mortgage.\n"
                "5. \"svpproof\"         (string, required) MCSpvProof hex data.\n"
                "\nReturns txid.\n"
                "\nResult:\n"
                "{\n"
                "  \"txid\": xx,   (string) The new create transaction txid\n"
                "  \"commit_transaction_reject_reason\": xxx, (string) If has reject reason will contain this field\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("redeemmortgagecoin", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 0 5dc9b823827e883e7d16988f8810be93ae8bc682df054f9b044527c388a95a89")
                + HelpExampleRpc("redeemmortgagecoin", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 0 5dc9b823827e883e7d16988f8810be93ae8bc682df054f9b044527c388a95a89")
                );

    if (!Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "This RPC API Only be called in main chain!\n");

    LOCK2(cs_main, pwallet->cs_wallet);

    uint256 mortgagecoinhash = ParseHashV(request.params[0], "parameter 1");
    int32_t nvoutindex = 0;
    if (request.params.size() > 1) {
        if (request.params[1].isNum())
            nvoutindex = request.params[1].get_int();
        else
            ParseInt32(request.params[1].get_str(), &nvoutindex);
    }

    std::string strTxHexData = request.params[2].get_str();
    MCMutableTransaction statementmtx;
    if (!DecodeHexTx(statementmtx, strTxHexData))
        throw JSONRPCError(RPC_INVALID_REQUEST, "Parameter 'fromtx' invalid hex tx data.");

    uint256 frombranchid = ParseHashV(request.params[3], "parameter 3");
    MCSpvProof spvProof;
    DecodeHexSpv(spvProof, request.params[4].get_str());

    MCOutPoint outpoint(mortgagecoinhash, nvoutindex);
    const Coin& coin = pcoinsTip->AccessCoin(outpoint);
    if (coin.IsSpent())
        throw JSONRPCError(RPC_WALLET_ERROR, "Coin is spent!");
    if (!pwallet->IsMine(coin.out))
        throw JSONRPCError(RPC_INVALID_REQUEST, "Coin is not mine!");

    uint256 coinMortgageBranchId;
    MCKeyID coinMortgageKeyId;
    if (!GetMortgageMineData(coin.out.scriptPubKey, &coinMortgageBranchId, &coinMortgageKeyId))
        throw JSONRPCError(RPC_WALLET_ERROR, "Is not a valid mortgage coin");

    if (coinMortgageBranchId != frombranchid)
        throw JSONRPCError(RPC_INVALID_REQUEST, "Branch id not match");

    MCCoinControl coin_control;
    coin_control.Select(outpoint); 
    coin_control.fAllowOtherInputs = true;

    //create transaction
    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::REDEEM_MORTGAGE;
    wtx.pPMT.reset(new MCSpvProof(spvProof));
    wtx.fromBranchId = frombranchid.ToString();        
    MCVectorWriter cvw{ SER_NETWORK, INIT_PROTO_VERSION, wtx.fromTx, 0, statementmtx };

    MCScript scriptPubKey = GetScriptForDestination(coinMortgageKeyId);

    EnsureWalletIsUnlocked(pwallet);

    bool fSubtractFeeFromAmount = false;
    //SendMoney
    MCAmount curBalance = pwallet->GetBalance();
    MCAmount nAmount = coin.out.nValue;
    //if (nAmount > curBalance)
    //    throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Create and send the transaction
    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    int nChangePosRet = -1;
    MCRecipient recipient = { scriptPubKey, nAmount, fSubtractFeeFromAmount };
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    // end send money

    uint256 hashTx = wtx.GetHash();
    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("txid", hashTx.GetHex()));
    if (!state.GetRejectReason().empty())
        obj.push_back(Pair("commit_transaction_reject_reason", state.GetRejectReason()));
    return obj;
}

//当赎回抵押挖矿币交易满足成熟度后，可以手动触发主链的赎回动作。默认请求下回自动调用，但是考虑到有可能失败的情况，如发生了，只好手动去调
UniValue rebroadcastredeemtransaction(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
            "rebroadcastredeemtransaction txid \n"
            "\n rebroadcast the redeem mortgage coin transaction by txid, in case that transction has not be send to the main chain .\n"
            "\nArguments:\n"
            "1. \"txid\"                  (string, required) The txid.\n"
            "\nReturns the hash of the created branch chain.\n"
            "\nResult:\n"
            "\"ret\"                  (string) ok or false\n"
            "\nExamples:\n"
            + HelpExampleCli("rebroadcastredeemtransaction", "5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db")
            + HelpExampleRpc("rebroadcastredeemtransaction", "5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db")
        );

    if (Params().IsMainChain()){
        throw JSONRPCError(RPC_VERIFY_ERROR, "This rpc can not call in main chain.");
    }

    uint256 txhash = ParseHashV(request.params[0], "parameter 1");
    MCTransactionRef tx;
    MCBlockIndex* pblockindex = nullptr;
    uint32_t tx_vtx_index = 0;
    MCBlock block;
    uint256 hashBlock;
    //LOCK(cs_main);

    //GetTransaction(txhash, tx, Params().GetConsensus(), )
    bool retflag;
    bool retval = ReadTxDataByTxIndex(txhash, tx, hashBlock, retflag);
    if (!retval){
        throw JSONRPCError(RPC_VERIFY_ERROR, "Can not read tx data by parameter 1(txid)");
    }

    if (!tx->IsRedeemMortgageStatement()) {
        throw JSONRPCError(RPC_VERIFY_ERROR, "Tx(parameter 1) is not a redeem mortgage statement transaction!");
    }

    if (mapBlockIndex.count(hashBlock) == 0){
        throw JSONRPCError(RPC_VERIFY_ERROR, "Tx's block no block index in mapBlockIndex");
    }

    pblockindex = mapBlockIndex[hashBlock];
    if (!ReadBlockFromDisk(block, pblockindex, Params().GetConsensus())){
        throw JSONRPCError(RPC_VERIFY_ERROR, "Read Block From Disk fail when rebroadcast redeem.");
    }

    const uint32_t maturity = BRANCH_CHAIN_MATURITY;
    int confirmations = chainActive.Height() - pblockindex->nHeight + 1;
    if (confirmations < maturity + 1){
        throw JSONRPCError(RPC_VERIFY_ERROR, "can not broadcast because no enough confirmations");
    }

    std::string strError;
    if (!ReqMainChainRedeemMortgage(tx, block, &strError))
        throw JSONRPCError(RPC_VERIFY_ERROR, strprintf(std::string("Call ReqMainChainRedeemMortgage fail: %s"), strError));

    return "ok";
}

//在支链发起举报某个交易，或者举报coinbase交易
UniValue sendreporttomain(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
        throw std::runtime_error(
                "sendreporttomain \"blockhash\"  \"txid\"\n"
                "\nSend invalid transaction proof to main chain. \n"
                "\nArguments:\n"
                "1. \"blockhash\"        (string, required) The block hash.\n"
                "2. \"txid\"             (string, required) The txid of transaction that will be reported.\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "\n"
                "\nExamples:\n"
                + HelpExampleCli("sendprovetomain", "\"blockhash\" \"txid\"")
                + HelpExampleRpc("sendprovetomain", "\"blockhash\" \"txid\"")
                );

    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not call this RPC in main chain!\n");

    uint256 blockHash = ParseHashV(request.params[0], "parameter 1");
    if (mapBlockIndex.count(blockHash) == 0)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

    MCBlockIndex* pblockindex = mapBlockIndex[blockHash];
    if (chainActive.Height() - pblockindex->nHeight > REDEEM_SAFE_HEIGHT)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Report block is too old.");

    MCBlock block;
    if (!ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))   
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");

    uint256 txHash = ParseHashV(request.params[1], "parameter 2");
    //check tx is a normal transaction
    int txIndex = -1;
    MCTransactionRef pReportTx;
    for (int i = 0; i < block.vtx.size(); ++i) {
        if (block.vtx[i]->GetHash() == txHash) {
            txIndex = i;
            pReportTx = block.vtx[i];
            break;
        }
    }

    if (pReportTx == nullptr)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "block did not contain the reported transaction");

    std::set<uint256> setTxids;
    setTxids.insert(txHash);

    MCMutableTransaction mtx;
    mtx.nVersion = MCTransaction::REPORT_CHEAT;
    mtx.pPMT.reset(NewSpvProof(block, setTxids));

    ReportData* pReportData = new ReportData;
    mtx.pReportData.reset(pReportData);
    if (pReportTx->IsCoinBase())
        pReportData->reporttype = ReportType::REPORT_COINBASE;
    else
        pReportData->reporttype = ReportType::REPORT_TX;
    pReportData->reportedTxHash = txHash;
    pReportData->reportedBranchId = Params().GetBranchHash();
    pReportData->reportedBlockHash = block.GetHash();

    MCRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig(MCBaseChainParams::MAIN, branchrpccfg) == false)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "invalid rpc config");

    const std::string strMethod = "handlebranchreport";
    UniValue params(UniValue::VARR);
    MCTransactionRef txRef = MakeTransactionRef(std::move(mtx));
    params.push_back(EncodeHexTx(*txRef, RPCSerializationFlags()));

    MCMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, params[0].get_str()))
        throw JSONRPCError(RPC_WALLET_ERROR, "DecodeHexTx tx hex fail.\n");

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);
    const UniValue& error = find_value(reply, "error");
    if (!error.isNull())
        throw JSONRPCError(RPC_INTERNAL_ERROR,strprintf(std::string("call rpc error %s"), error.write()));
    const UniValue& result = find_value(reply, "result");
    if (result.isNull())
        throw JSONRPCError(RPC_VERIFY_ERROR, "RPC return value result is null");

    return result;
}

UniValue reportcontractdata(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 4)
        throw std::runtime_error(
            "sendreporttomain \"blockhash\"  \"txid\"\n"
            "\nSend invalid transaction proof to main chain. \n"
            "\nArguments:\n"
            "1. \"blockhash\"        (string, required) The block hash.\n"
            "2. \"txid\"             (string, required) The txid of transaction that will be reported.\n"
            "\nReturns ok or fail.\n"
            "\nResult:\n"
            "\n"
            "\nExamples:\n"
            + HelpExampleCli("sendprovetomain", "\"blockhash\" \"txid\"")
            + HelpExampleRpc("sendprovetomain", "\"blockhash\" \"txid\"")
        );

    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not call this RPC in main chain!\n");

    uint256 reportedBlockHash = ParseHashV(request.params[0], "parameter 1");
    if (mapBlockIndex.count(reportedBlockHash) == 0)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

    MCBlock reportedBlock;
    MCBlockIndex* pReportedBlockIndex = mapBlockIndex[reportedBlockHash];
    if (!ReadBlockFromDisk(reportedBlock, pReportedBlockIndex, Params().GetConsensus()))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");

    int reportedTxIndex = -1;
    uint256 reportedTxHash = ParseHashV(request.params[1], "parameter 2");
    //check tx is a normal transaction
    for (int i = 0; i < reportedBlock.vtx.size(); ++i) {
        if (reportedBlock.vtx[i]->GetHash() == reportedTxHash) {
            reportedTxIndex = i;
            break;
        }
    }
    if (reportedTxIndex == -1)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't find transaction");

    uint256 proveBlockHash = ParseHashV(request.params[2], "parameter 2");
    if (mapBlockIndex.count(proveBlockHash) == 0)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

    MCBlock proveBlock;
    MCBlockIndex* pProveBlockIndex = mapBlockIndex[proveBlockHash];
    if (!ReadBlockFromDisk(proveBlock, pProveBlockIndex, Params().GetConsensus()))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");

    int proveTxIndex = -1;
    uint256 proveTxHash = ParseHashV(request.params[3], "parameter 3");
    //check tx is a normal transaction
    for (int i = 0; i < proveBlock.vtx.size(); ++i) {
        if (proveBlock.vtx[i]->GetHash() == proveTxHash) {
            proveTxIndex = i;
            break;
        }
    }
    if (proveTxIndex == -1)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't find transaction");

    MCMutableTransaction mtx;
    mtx.nVersion = MCTransaction::REPORT_CHEAT;

    std::set<uint256> setTxids;
    setTxids.insert(reportedTxHash);
    mtx.pPMT.reset(NewSpvProof(reportedBlock, setTxids));

    ReportData* pReportData = new ReportData;
    mtx.pReportData.reset(pReportData);
    pReportData->reporttype = ReportType::REPORT_CONTRACT_DATA;
    pReportData->reportedBranchId = Params().GetBranchHash();

    MCTransactionRef reportTx = reportedBlock.vtx[reportedTxIndex];
    pReportData->reportedBlockHash = reportedBlockHash;
    pReportData->reportedTxHash = reportedTxHash;
    pReportData->contractData.reset(new ReportContractData);
    pReportData->contractData->reportedContractPrevData = reportedBlock.prevContractData[reportedTxIndex];
    std::vector<uint256> reprotedLeaves;
    VecTxMerkleLeavesWithPrevData(reportedBlock.vtx, reportedBlock.prevContractData, reprotedLeaves);
    std::vector<bool> reportedMatch(reportedBlock.vtx.size(), false);
    reportedMatch[reportedTxIndex] = true;
    pReportData->contractData->reportedSpvProof = std::move(MCSpvProof(reprotedLeaves, reportedMatch, reportedBlockHash));

    MCTransactionRef proveTx = proveBlock.vtx[proveTxIndex];
    pReportData->contractData->proveTxHash = proveTxHash;

    SmartLuaState sls;
    ContractContext contractContext;
    contractContext.txFinalData.data.resize(proveBlock.vtx.size());
    if (!ExecuteBlock(&sls, &proveBlock, pProveBlockIndex->pprev, 0, proveTxIndex + 1, &contractContext))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "executive contract fail");
    pReportData->contractData->proveContractData = contractContext.txFinalData.data[proveTxIndex];

    std::vector<bool> proveMatch(proveBlock.vtx.size(), false);
    proveMatch[proveTxIndex] = true;
    std::vector<uint256> proveLeaves;
    VecTxMerkleLeavesWithData(proveBlock.vtx, contractContext.txFinalData.data, proveLeaves);
    pReportData->contractData->proveSpvProof = std::move(MCSpvProof(proveLeaves, proveMatch, proveBlockHash));

    MCRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig(MCBaseChainParams::MAIN, branchrpccfg) == false)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "invalid rpc config");

    const std::string strMethod = "handlebranchreport";
    UniValue params(UniValue::VARR);
    MCTransactionRef txRef = MakeTransactionRef(std::move(mtx));
    params.push_back(EncodeHexTx(*txRef, RPCSerializationFlags()));

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);
    const UniValue& error = find_value(reply, "error");
    if (!error.isNull())
        throw JSONRPCError(RPC_INTERNAL_ERROR, strprintf(std::string("call rpc error %s"), error.write()));
    const UniValue& result = find_value(reply, "result");
    if (result.isNull())
        throw JSONRPCError(RPC_VERIFY_ERROR, "RPC return value result is null");

    return result;
}

//主链接收处理举报交易
UniValue handlebranchreport(const JSONRPCRequest& request)
{
    MCWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "handlebranchreport\"CTransaction hex data\"\n"
                "\nInclude branch block data to a transaction, then send to main chain\n"
                "\nArguments:\n"
                "1. \"transaction_of_branch_block_data\"             (string, required) The transaction data in dex string format.\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "\"ret\"                  (string) ok or fail\n"
                "\nExamples:\n"
                + HelpExampleCli("handlebranchreport", "5754f9e...630db")
                + HelpExampleRpc("handlebranchreport", "5754f9e...630db")
                );
    if (!Params().IsMainChain())
        throw JSONRPCError(RPC_WALLET_ERROR, "can not call in branchchain.\n");

    const std::string& strTx1HexData = request.params[0].get_str();
    MCMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, strTx1HexData))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "DecodeHexTx tx hex fail.\n");
    }
    if (!mtxTrans1.IsReport())
        throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid transaction data");

    MCTransactionRef tx = MakeTransactionRef(std::move(mtxTrans1));

    const uint256 reportedBranchId = tx->pReportData->reportedBranchId;
    if (!g_pBranchDb->HasBranchData(reportedBranchId))
        throw JSONRPCError(RPC_INTERNAL_ERROR, strprintf("Invalid reported branch id %s", reportedBranchId.ToString().c_str()));
    BranchData branchData = g_pBranchDb->GetBranchData(reportedBranchId);
    if (branchData.mapHeads.count(tx->pReportData->reportedBlockHash) == 0)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not found block data in mapHeads");

    BranchBlockData* pBlockData = branchData.GetBranchBlockData(tx->pPMT->blockhash);
    if (pBlockData == nullptr)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not found block data in mapHeads");

    MCSpvProof spvProof(*tx->pPMT);
    int txIndex = CheckSpvProof(pBlockData->header.hashMerkleRoot, spvProof.pmt, tx->pReportData->reportedTxHash);
    if (txIndex < 0)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid transaction spv");

    uint256 reportFlagHash = GetReportTxHashKey(*tx);
    const uint256& rpBranchId = tx->pReportData->reportedBranchId;
    const uint256& rpBlockId = tx->pReportData->reportedBlockHash;
    if (g_pBranchDb->GetTxReportState(rpBranchId, rpBlockId, reportFlagHash) != RP_INVALID)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Tansaction had been reported!");

    MCCoinControl coin_control;
    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::REPORT_CHEAT;
    wtx.isDataTransaction = true;
    wtx.pReportData.reset(new ReportData(*tx->pReportData));
    wtx.pPMT.reset(new MCSpvProof(*tx->pPMT));

    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    bool fSubtractFeeFromAmount = false;
    MCAmount curBalance = pwallet->GetBalance();
    int nChangePosRet = -1;
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txid", wtx.GetHash().GetHex()));
    if (!state.GetRejectReason().empty())
    {
        ret.push_back(Pair("commit_reject_reason", state.GetRejectReason()));
    }
    return ret;
}

// 举报merkle
UniValue reportbranchchainblockmerkle(const JSONRPCRequest& request)
{
    MCWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }
    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
        throw std::runtime_error(
            "reportbranchchainblockmerkle \"blockhash\"  \"txid\"\n"
            "\nReport a branchchain block merkle error.\n"
            "\nArguments:\n"
            "1. \"branchid\"         (string, required) The branchid.\n"
            "2. \"blockhash\"        (string, required) The block hash.\n"
            "\nReturns ok or fail.\n"
            "\nResult:\n"
            "\n"
            "\nExamples:\n"
            + HelpExampleCli("reportbranchchainblockmerkle", "\"branchid\" \"blockhash\"")
            + HelpExampleRpc("reportbranchchainblockmerkle", "\"branchid\" \"blockhash\"")
        );

    if (!Params().IsMainChain())
        throw JSONRPCError(RPC_WALLET_ERROR, "can not call in branchchain.\n");

    uint256 branchid = ParseHashV(request.params[0], "parameter 1");
    uint256 blockHash = ParseHashV(request.params[1], "parameter 2");

    std::shared_ptr<ReportData> pReportData = std::make_shared< ReportData>();
    pReportData->reportedBranchId = branchid;
    pReportData->reportedBlockHash = blockHash;
    pReportData->reporttype = ReportType::REPORT_MERKLETREE;
    pReportData->reportedTxHash.SetNull();

    MCCoinControl coin_control;
    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::REPORT_CHEAT;
    wtx.isDataTransaction = true;
    wtx.pReportData = pReportData;
    wtx.pPMT.reset(new MCSpvProof());

    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    bool fSubtractFeeFromAmount = false;
    MCAmount curBalance = pwallet->GetBalance();
    int nChangePosRet = -1;
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txid", wtx.GetHash().GetHex()));
    if (!state.GetRejectReason().empty())
    {
        ret.push_back(Pair("commit_reject_reason", state.GetRejectReason()));
    }
    return ret;
}


//在支链上发起证明某个交易
// 先是在支链创建好证明数据
UniValue sendprovetomain(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
        throw std::runtime_error(
                "sendreporttomain \"blockhash\"  \"txid\"\n"
                "\nSend valid transaction proof to main chain. \n"
                "\nArguments:\n"
                "1. \"blockhash\"        (string, required) The block hash.\n"
                "2. \"txid\"             (string, required) A transaction hash to be prove.\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "\n"
                "\nExamples:\n"
                + HelpExampleCli("sendprovetomain", "\"blockhash\" \"txid\"")
                + HelpExampleRpc("sendprovetomain", "\"blockhash\" \"txid\"")
                );

    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not call this RPC in main chain!\n");

    uint256 blockHash = ParseHashV(request.params[0], "parameter 1");
    if (!mapBlockIndex.count(blockHash))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");
    MCBlockIndex*  pBlockIndex = mapBlockIndex[blockHash];

    MCBlock block;
    if (!ReadBlockFromDisk(block, pBlockIndex, Params().GetConsensus()))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");

    uint256 txHash = ParseHashV(request.params[1], "parameter 2");

    int targetTxIndex = -1;
    MCTransactionRef pProveTx;
    for (int i = 0; i < block.vtx.size(); ++i)
    {
        if (block.vtx[i]->GetHash() == txHash)
        {
            targetTxIndex = i;
            pProveTx = block.vtx[i];
            break;
        }
    }
    if (pProveTx == nullptr) {
        throw JSONRPCError(RPC_INTERNAL_ERROR, "block did not contain the need proved transaction");
    }

    MCMutableTransaction mtx;
    mtx.nVersion = MCTransaction::PROVE;
    mtx.pProveData.reset(new ProveData);
    if (!pProveTx->IsCoinBase()) {
        mtx.pProveData->provetype = ReportType::REPORT_TX;
        mtx.pProveData->contractData.reset(new ContractProveData);
    }
    else
        mtx.pProveData->provetype = ReportType::REPORT_COINBASE;
    mtx.pProveData->branchId = Params().GetBranchHash();
    mtx.pProveData->blockHash = blockHash;
    mtx.pProveData->txHash = txHash;
    if (pProveTx->IsCoinBase()) { 
        if (!GetProveOfCoinbase(mtx.pProveData, block))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Get coinbase transaction prove data failed");
    }
    else {
        if (!GetProveInfo(block, pBlockIndex->nHeight, pBlockIndex->pprev, targetTxIndex, mtx.pProveData))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Get transaction prove data failed");
    }

    if (pProveTx->IsSmartContract()) {
        SmartLuaState sls;
        ContractContext contractContext;
        if (!ExecuteBlock(&sls, &block, pBlockIndex->pprev, 0, targetTxIndex + 1, &contractContext))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Execute contract fail");

        // 先证明合约数据来源合法
        mtx.pProveData->contractData->coins = block.prevContractData[targetTxIndex].coins;
        mtx.pProveData->contractData->contractPrevData = std::move(sls.contractDataFrom);

        std::vector<uint256> reportedPrevDataLeaves;
        VecTxMerkleLeavesWithPrevData(block.vtx, block.prevContractData, reportedPrevDataLeaves);
        std::vector<bool> reportedMatch(block.vtx.size(), false);
        reportedMatch[targetTxIndex] = true;
        mtx.pProveData->contractData->prevDataSPV = std::move(MCPartialMerkleTree(reportedPrevDataLeaves, reportedMatch));
        
        if (!ExecuteBlock(&sls, &block, pBlockIndex->pprev, targetTxIndex + 1, block.vtx.size() - targetTxIndex - 1, &contractContext))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Execute contract fail");

        std::vector<uint256> reportedDataLeaves;
        VecTxMerkleLeavesWithData(block.vtx, contractContext.txFinalData.data, reportedDataLeaves);
        mtx.pProveData->contractData->dataSPV = std::move(MCPartialMerkleTree(reportedDataLeaves, reportedMatch));
    }

    MCRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig(MCBaseChainParams::MAIN, branchrpccfg) == false)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "invalid rpc config");

    const std::string strMethod = "handlebranchprove";
    UniValue params(UniValue::VARR);
    MCTransactionRef txRef = MakeTransactionRef(std::move(mtx));
    params.push_back(EncodeHexTx(*txRef, RPCSerializationFlags()));

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);
    const UniValue& error = find_value(reply, "error");
    if (!error.isNull())
        throw JSONRPCError(RPC_INTERNAL_ERROR, strprintf(std::string("call rpc error %s"), error.write()));

    const UniValue& result = find_value(reply, "result");
    if (result.isNull())
        throw JSONRPCError(RPC_VERIFY_ERROR, "RPC return value result is null");

    return result;
}

//send block merkle tree prove , the same as prove coinbase transaction
//直接在主链上举报，原因是举报者没法获得目标block的详情，或者block数据本身有问题而被丢弃了，通过这样强制矿工提供完整证明
//提交内容和证明coinbase是一样。
UniValue sendmerkleprovetomain(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
            "sendreporttomain \"blockhash\"  \"txid\"\n"
            "\nSend valid block merkle proof to main chain.\n"
            "\nArguments:\n"
            "1. \"blockhash\"        (string, required) The block hash.\n"
            
            "\nReturns ok or fail.\n"
            "\nResult:\n"
            "\n"
            "\nExamples:\n"
            + HelpExampleCli("sendmerkleprovetomain", "\"blockhash\"")
            + HelpExampleRpc("sendmerkleprovetomain", "\"blockhash\"")
        );

    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not call this RPC in main chain!\n");

    uint256 blockHash = ParseHashV(request.params[0], "parameter 1");
    if (!mapBlockIndex.count(blockHash))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");
    MCBlockIndex*  pblockindex = mapBlockIndex[blockHash];

    MCBlock block;
    if (!ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");

    MCMutableTransaction mtx;
    mtx.nVersion = MCTransaction::PROVE;
    mtx.pProveData.reset(new ProveData);
    mtx.pProveData->provetype = ReportType::REPORT_MERKLETREE;
    mtx.pProveData->branchId = Params().GetBranchHash();
    mtx.pProveData->blockHash = blockHash;
    mtx.pProveData->txHash.SetNull();// when report merkle, this field is null, so set it null to match.
    if (!GetProveOfCoinbase(mtx.pProveData, block))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Get coinbase transaction prove data failed");

    MCRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig(MCBaseChainParams::MAIN, branchrpccfg) == false)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "invalid rpc config");

    const std::string strMethod = "handlebranchprove";
    UniValue params(UniValue::VARR);
    MCTransactionRef txRef = MakeTransactionRef(std::move(mtx));
    params.push_back(EncodeHexTx(*txRef, RPCSerializationFlags()));

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);
    const UniValue& error = find_value(reply, "error");
    if (!error.isNull())
        throw JSONRPCError(RPC_INTERNAL_ERROR, strprintf(std::string("call rpc error %s"), error.write()));

    const UniValue& result = find_value(reply, "result");
    if (result.isNull())
        throw JSONRPCError(RPC_VERIFY_ERROR, "RPC return value result is null");

    return result;
}

//主链接收举报的证明
UniValue handlebranchprove(const JSONRPCRequest& request)
{
    MCWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "handlebranchreport\"CTransaction hex data\"\n"
                "\nInclude branch block data to a transaction, then send to main chain\n"
                "\nArguments:\n"
                "1. \"transaction_of_branch_block_data\"             (string, required) The transaction data in dex string format.\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "\"ret\"                  (string) ok or fail\n"
                "\nExamples:\n"
                + HelpExampleCli("handlebranchprove", "5754f9e...630db")
                + HelpExampleRpc("handlebranchprove", "5754f9e...630db")
                );
    if (!Params().IsMainChain())
        throw JSONRPCError(RPC_WALLET_ERROR, "Can not call in branchchain.\n");
    const std::string& strTx1HexData = request.params[0].get_str();
    MCMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, strTx1HexData)){
        throw JSONRPCError(RPC_WALLET_ERROR, "DecodeHexTx tx hex fail.\n");
    }

    if (!mtxTrans1.IsProve())
        throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid transaction data");

    MCTransactionRef tx = MakeTransactionRef(std::move(mtxTrans1));
    uint256 proveFlagHash = GetProveTxHashKey(*tx);
    const uint256& rpBranchId = tx->pProveData->branchId;
    const uint256& rpBlockId = tx->pProveData->blockHash;
    if (g_pBranchDb->GetTxReportState(rpBranchId, rpBlockId, proveFlagHash) != RP_FLAG_REPORTED){
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid report transaction");
    }

    MCCoinControl coin_control;
    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::PROVE;
    wtx.pProveData.reset(tx->pProveData == nullptr ? nullptr : new ProveData(*tx->pProveData));
    wtx.isDataTransaction = true;

    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    bool fSubtractFeeFromAmount = false;
    MCAmount curBalance = pwallet->GetBalance();
    int nChangePosRet = -1;
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txid", wtx.GetHash().GetHex()));
    if (!state.GetRejectReason().empty())
        ret.push_back(Pair("commit_reject_reason", state.GetRejectReason()));
    return ret;
}

// 举报后,在举报交易被打包后 REPORT_LOCK_COIN_HEIGHT + 1个块后手动在支链调用该接口
// 锁定挖矿币
UniValue lockmortgageminecoin(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
        throw std::runtime_error(
                "lockmortgageminecoin \"txid\" \"cointxid\" \n"
                "\nLock the mortgage mine coin when is report tx is valid.\n"
                "\nArguments:\n"
                "1. \"txid\"             (string, required) The txid of report transaction that in main chain.\n"
                "2. \"coinhash\"         (string, required) The tx hash of the mortgage coin's preout.\n"
                "\nReturns txid.\n"
                "\nResult:\n"
                "{\n"
                "  \"txid\": xx,   (string) The new create transaction txid\n"
                "  \"commit_transaction_reject_reason\": xxx, (string) If has reject reason will contain this field\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("lockmortgageminecoin", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 89e1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a")
                + HelpExampleRpc("lockmortgageminecoin", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 89e1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a")
                );

    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "This RPC API Only be called in branch chain!\n");

    LOCK2(cs_main, pwallet->cs_wallet);

    // check: 
    // 1 branch, get report tx data
    // 2 coin preout hash
    // 3 历史久远的不能举报?
    uint256 reporttxid = ParseHashV(request.params[0], "parameter 1");
    uint256 coinprevouthash = ParseHashV(request.params[1], "parameter 2");

    MCCoinControl coin_control;

    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::LOCK_MORTGAGE_MINE_COIN;
    wtx.reporttxid = reporttxid;//被主链打包的举报交易id
    wtx.coinpreouthash = coinprevouthash;//锁定目标币的txid
    wtx.isDataTransaction = true;

    bool fSubtractFeeFromAmount = false;
    EnsureWalletIsUnlocked(pwallet);
    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    MCAmount curBalance = pwallet->GetBalance();
    // Create and send the transaction
    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    int nChangePosRet = -1;
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    ///////////
    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txid", wtx.GetHash().GetHex()));
    if (!state.GetRejectReason().empty())
    {
        ret.push_back(Pair("commit_reject_reason", state.GetRejectReason()));
    }
    return ret;
}

//获取举报交易数据
UniValue getreporttxdata(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
        throw std::runtime_error(
                "getreporttxdata \"txid\" \n"
                "\nGet report transaction data by txid.\n"
                "\nArguments:\n"
                "1. \"txid\"             (string, required) The txid of report transaction that in main chain.\n"
                "\nResult:\n"
                "{\n"
                "  \"txhex\": xx,   (string) The new create transaction txid\n"
                "  \"commit_transaction_reject_reason\": xxx, (string) If has reject reason will contain this field\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("getreporttxdata", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a")
                + HelpExampleRpc("getreporttxdata", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a")
                );
    if (!Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "This RPC API Only be called in main chain!\n");

    uint256 reporttxid = ParseHashV(request.params[0], "parameter 1");

    MCTransactionRef ptxReport;
    uint256 hashBlock;
    bool retflag;
    bool retval = ReadTxDataByTxIndex(reporttxid, ptxReport, hashBlock, retflag);
    if (ptxReport == nullptr)
        throw JSONRPCError(RPC_INVALID_REQUEST, "read tx data fail");

    int confirmations = 0;
    if (mapBlockIndex.count(hashBlock))
        confirmations = chainActive.Height() - mapBlockIndex[hashBlock]->nHeight;

    // get mine coin prevouthash
    //LOCK(cs_main);// protect g_pBranchDb
    uint256 prevouthash;
    BranchData branchdata = g_pBranchDb->GetBranchData(ptxReport->pReportData->reportedBranchId);// don't check
    if (branchdata.mapHeads.count(ptxReport->pReportData->reportedBlockHash)){
        if (!GetMortgageCoinData(branchdata.mapHeads[ptxReport->pReportData->reportedBlockHash].pStakeTx->vout[0].scriptPubKey, &prevouthash))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid-block-data");
    }

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txhex", EncodeHexTx(*ptxReport, RPCSerializationFlags())));
    ret.push_back(Pair("confirmations", confirmations));
    ret.push_back(Pair("preminecoinvouthash", prevouthash.GetHex()));

    return ret;
}

// 解锁挖矿币
UniValue unlockmortgageminecoin(const JSONRPCRequest& request)
{
    MCWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 3 || request.params.size() > 3)
        throw std::runtime_error(
                "unlockmortgageminecoin \"txid\" \"cointxid\" \"provetxid\"\n"
                "\nUnlock the mortgage mine coin when is report tx is valid.\n"
                "\nArguments:\n"
                "1. \"txid\"             (string, required) The txid of report transaction that in main chain.\n"
                "2. \"coinhash\"         (string, required) The tx hash of the mortgage coin's preout.\n"
                "3. \"coinhash\"         (string, required) The tx hash of the prove the report transaction.\n"
                "\nReturns txid.\n"
                "\nResult:\n"
                "{\n"
                "  \"txid\": xx,   (string) The new create transaction txid\n"
                "  \"commit_transaction_reject_reason\": xxx, (string) If has reject reason will contain this field\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("unlockmortgageminecoin", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 89e1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 89e1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def9656ccc")
                + HelpExampleRpc("unlockmortgageminecoin", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 89e1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 89e1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def9656ccc")
                );

    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "This RPC API Only be called in branch chain!\n");

    LOCK2(cs_main, pwallet->cs_wallet);

    uint256 reporttxid = ParseHashV(request.params[0], "parameter 1");
    uint256 coinprevouthash = ParseHashV(request.params[1],"parameter 2");
    uint256 provetxid = ParseHashV(request.params[2], "parameter 3");

    MCCoinControl coin_control;

    MCWalletTx wtx;
    wtx.nVersion = MCTransaction::UNLOCK_MORTGAGE_MINE_COIN;
    wtx.reporttxid = reporttxid;// 举报交易txid
    wtx.coinpreouthash = coinprevouthash;
    wtx.provetxid = provetxid;//证明交易txid
    wtx.isDataTransaction = true;

    bool fSubtractFeeFromAmount = false;
    EnsureWalletIsUnlocked(pwallet);
    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    MCAmount curBalance = pwallet->GetBalance();
    // Create and send the transaction
    MCReserveKey reservekey(pwallet);
    MCAmount nFeeRequired;
    std::string strError;
    std::vector<MCRecipient> vecSend;
    int nChangePosRet = -1;
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    MCValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction(%s) was rejected! Reason given: %s", wtx.GetHash().ToString(), state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    ///////////
    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txid", wtx.GetHash().GetHex()));
    if (!state.GetRejectReason().empty())
    {
        ret.push_back(Pair("commit_reject_reason", state.GetRejectReason()));
    }
    return ret;
}

//获取举报交易数据
UniValue getprovetxdata(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
        throw std::runtime_error(
                "getprovetxdata \"txid\" \n"
                "\nGet prove transaction data by txid.\n"
                "\nArguments:\n"
                "1. \"txid\"             (string, required) The txid of prove transaction that in main chain.\n"
                "\nResult:\n"
                "{\n"
                "  \"txhex\": xx,   (string) The new create transaction txid\n"
                "  \"commit_transaction_reject_reason\": xxx, (string) If has reject reason will contain this field\n"
                "}\n"
                "\nExamples:\n"
                + HelpExampleCli("getprovetxdata", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a")
                + HelpExampleRpc("getprovetxdata", "7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a")
                );
    if (!Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "This RPC API Only be called in main chain!\n");

    uint256 provetxid = ParseHashV(request.params[0], "parameter 0");

    MCTransactionRef ptxProve;
    uint256 hashBlock;
    bool retflag;
    bool retval = ReadTxDataByTxIndex(provetxid, ptxProve, hashBlock, retflag);
    if (ptxProve == nullptr)
        throw JSONRPCError(RPC_INVALID_REQUEST, "read tx data fail");

    int confirmations = 0;
    if (mapBlockIndex.count(hashBlock))
        confirmations = chainActive.Height() - mapBlockIndex[hashBlock]->nHeight;

    // get mine coin prevouthash
    // LOCK(cs_main);// protect g_pBranchDb
    
    if (!g_pBranchDb->HasBranchData(ptxProve->pProveData->branchId)){
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid prove transaction data.");
    }

    uint256 reportblockhash = ptxProve->pProveData->blockHash;
    
    uint256 prevouthash;
    BranchData branchdata = g_pBranchDb->GetBranchData(ptxProve->pProveData->branchId);
    if (branchdata.mapHeads.count(reportblockhash)) {
        if (!GetMortgageCoinData(branchdata.mapHeads[reportblockhash].pStakeTx->vout[0].scriptPubKey, &prevouthash))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid-block-data");
    }

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("txhex", EncodeHexTx(*ptxProve, RPCSerializationFlags())));
    ret.push_back(Pair("confirmations", confirmations));
    ret.push_back(Pair("preminecoinvouthash", prevouthash.GetHex()));

    return ret;
}

static const CRPCCommand commands[] =
{ //  category              name                         actor (function)              okSafeMode
    //  --------------------- ------------------------     -----------------------       ----------
    { "branchchain",        "createbranchchain",         &createbranchchain,           false,{"vseeds","seedspec6"} },
    { "branchchain",        "getbranchchaininfo",        &getbranchchaininfo,          true,{"branchid"} },
    { "branchchain",        "getallbranchinfo",          &getallbranchinfo,            false,{} },
    { "branchchain",        "addbranchnode",             &addbranchnode,               true,{ "branchid" ,"ip","port","usrname","password" } },
    { "branchchain",        "sendtobranchchain",         &sendtobranchchain,           false,{ "branchid","address","amount" } },
    { "branchchain",        "makebranchtransaction",     &makebranchtransaction,       false,{"hexdata"} },
    { "branchchain",        "getbranchchaintransaction", &getbranchchaintransaction,   true,{"txid"} },
    { "branchchain",        "rebroadcastchaintransaction",&rebroadcastchaintransaction,false,{"txid"} },

    { "branchchain",        "mortgageminebranch",        &mortgageminebranch,          true, {"branchid","amount", "address"}},
    { "branchchain",        "submitbranchblockinfo",     &submitbranchblockinfo,       true, {"tx_hex_data"}},
    { "branchchain",        "getbranchchainheight",      &getbranchchainheight,        false,{ "branchid" } },
    { "branchchain",        "resendbranchchainblockinfo",&resendbranchchainblockinfo,  false,{ "height" } },

    { "branchchain",        "redeemmortgagecoinstatement",&redeemmortgagecoinstatement,false, {"txid", "voutindex"}},
    { "branchchain",        "redeemmortgagecoin",        &redeemmortgagecoin,          false,{ "txid", "voutindex" } },
    { "branchchain",        "rebroadcastredeemtransaction",&rebroadcastredeemtransaction, false,{ "txid" }, },

    // report and provre api
    { "branchchain",        "sendreporttomain",          &sendreporttomain,            false, {"blockhash", "txid"}},
    { "branchchain",        "reportcontractdata",          &reportcontractdata,        false,{ "reportedblockhash", "reportedtxid", "proveblockhash", "provetxid" } },
    { "branchchain",        "handlebranchreport",        &handlebranchreport,          true,  {"tx_hex_data"}},
    { "branchchain",        "reportbranchchainblockmerkle",&reportbranchchainblockmerkle, false, {"branchid","blockhash"},},
    // 证明交易数据还需要修改和完善
    { "branchchain",        "sendprovetomain",           &sendprovetomain,             false, {"blockhash", "txid"}},
    { "branchchain",        "sendmerkleprovetomain",     &sendmerkleprovetomain,       false, {"blockhash"}},
    { "branchchain",        "handlebranchprove",         &handlebranchprove,           true,  {"tx_hex_data"}},

    { "branchchain",        "lockmortgageminecoin",      &lockmortgageminecoin,        false, { "txid", "coinpreouthash"}},
    { "branchchain",        "getreporttxdata",           &getreporttxdata,             false, { "txid" } },
    { "branchchain",        "unlockmortgageminecoin",    &unlockmortgageminecoin,      false,{ "txid", "coinpreouthash", "provetxid" } }, 
    { "branchchain",        "getprovetxdata",            &getprovetxdata,              false,{ "txid" } },
};

void RegisterBranchChainRPCCommands(CRPCTable &t)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
