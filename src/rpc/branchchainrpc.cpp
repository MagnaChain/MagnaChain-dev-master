// Copyright (c) 2016-2018 The CellLink Core developers
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

#include <boost/foreach.hpp>
#include <stdint.h>
#include <sstream>

#include "chain/branchdb.h"

const CellAmount CreateBranchChainMortgage = 20000*COIN;

//TODO: for test,发出前需要改成合适的值 
const uint32_t BRANCH_CHAIN_CREATE_COIN_MATURITY = 527040; // 半年才能赎回, 527040块 * 30s/块 = 183天 
const uint32_t BRANCH_CHAIN_MATURITY = 2000;// 至少需要 2000 块 * 30s/块 = 1000 分钟 = 16.67 hours
const CellAmount MIN_MINE_BRANCH_MORTGAGE = 100 * COIN; // 抵押挖矿最小值
const uint32_t REDEEM_SAFE_HEIGHT = 10800; // 10800 * 8s = 1 day (branch chain block time)
const uint32_t REPORT_OUTOF_HEIGHT = 2880; // 2880 * 30s = 1 day
const uint32_t REPORT_LOCK_COIN_HEIGHT = 30; // 30 * 30s = 15 mins

//创建侧链抵押金计算
CellAmount GetCreateBranchMortgage(const CellBlock* pBlock, const CellBlockIndex* pBlockIndex)
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
    powN = std::min((size_t)16, powN);// 655360000 COIN

    CellAmount mortgage = CreateBranchChainMortgage * std::pow(2, powN);
    return mortgage;
}

// make a cache will be better
static bool GetTransactionDataByTxInfo(const uint256 &txhash, CellTransactionRef &tx, CellBlockIndex** ppblockindex, uint32_t &tx_vtx_index, CellBlock& block)
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
    CellBlockIndex* pblockindex = *ppblockindex;
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

bool MakeBranchTransStep2Tx(CellMutableTransaction& branchTx, const CellScript& scriptPubKey, const CellAmount& nAmount, CellAmount &fee, CellCoinControl &coin_control)
{
    const std::string strFromChain = Params().GetBranchId();

    branchTx.nVersion = CellTransaction::TRANS_BRANCH_VERSION_S2;
    branchTx.nLockTime = 0;
    branchTx.fromBranchId = strFromChain;
    //    branchTx.fromTx = ;//this value set later.
    branchTx.pPMT.reset(new CellSpvProof()); //will reset later, 这里防止序列化时报错

    branchTx.vin.resize(1);// (CellTransaction::IsCoinBase function is amazing)
    branchTx.vin[0].prevout.hash.SetNull();
    branchTx.vin[0].prevout.n = 0;// make prevout is not Null any more
    branchTx.vin[0].scriptSig.clear();
    branchTx.vout.resize(1);
    branchTx.vout[0].scriptPubKey = scriptPubKey;
    branchTx.vout[0].nValue = nAmount;

    unsigned int nBytes = GetVirtualTransactionSize(branchTx);
    FeeCalculation feeCalc;
    fee = CellWallet::GetMinimumFee(nBytes, coin_control, ::mempool, ::feeEstimator, &feeCalc);

    branchTx.inAmount = nAmount + fee;
    return true;
}

UniValue createbranchchain(const JSONRPCRequest& request)
{
    CellWallet * const pwallet = GetWalletForJSONRPCRequest(request);
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
            "3. \"mortgageaddress\"     (string, required) The CellLink Address, use to receive mortgage in branch chain\n"
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

    const CellAmount nAmount = GetCreateBranchMortgage();

    std::string strVSeeds = request.params[0].get_str();
    std::string strSeedSpec6 = request.params[1].get_str();
    std::string strMortgageAddress = request.params[2].get_str();
    CellLinkAddress strBcAddress(strMortgageAddress);
    if (!strBcAddress.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid celllink address for genesis block address");
    if (strBcAddress.Get().type() != typeid(CellKeyID))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Celllink public key address");
    }
    
    CellCoinControl coin_control;

    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::CREATE_BRANCH_VERSION;
    wtx.branchVSeeds = strVSeeds;
    wtx.branchSeedSpec6 = strSeedSpec6;

    //make branch transaction output
    CellAmount fee2 = 0;
    CellScript sendToScriptPubKey = GetScriptForDestination(strBcAddress.Get());
    CellMutableTransaction branchStep2MTx;
    if (MakeBranchTransStep2Tx(branchStep2MTx, sendToScriptPubKey, nAmount, fee2, coin_control) == false) {
        throw JSONRPCError(RPC_TYPE_ERROR, "Make branch step2 tx error.");
    }
    CellTransactionRef branchStep2Tx = MakeTransactionRef(std::move(branchStep2MTx));
    wtx.sendToTxHexData = EncodeHexTx(*branchStep2Tx, RPCSerializationFlags());

    CellScript scriptPubKey;
    scriptPubKey << OP_RETURN << OP_CREATE_BRANCH;// save pubkey to script

    bool fSubtractFeeFromAmount = false;
    EnsureWalletIsUnlocked(pwallet);
    //SendMoney
    CellAmount curBalance = pwallet->GetBalance();
    if (nAmount > curBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Create and send the transaction
    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;
    int nChangePosRet = -1;
    CellRecipient recipient = { scriptPubKey, nAmount + fee2, fSubtractFeeFromAmount };
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + fee2 + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
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

    std::string branchid = request.params[0].get_str();
    uint256 hash;
    hash.SetHex(branchid);

    CellTransactionRef tx;
    CellBlockIndex* pblockindex = nullptr;
    uint32_t tx_vtx_index = 0;
    CellBlock block;

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
        obj.push_back(Pair("ismaturity", confirmations >= BRANCH_CHAIN_MATURITY));
        arr.push_back(obj);
    }
    return arr;
}

//添加分支节点
UniValue addbranchnode(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 5 || request.params.size() > 5)
        throw std::runtime_error(
            "addbranchnode branchid ip port username password\n"
            "\n get a created branch chain info.\n"
            "\nArguments:\n"
            "1. \"branchid\"            (string, required) The branch txid.\n"
            "2. \"ip\"                  (string, required) Branch node ip.\n"
            "3. \"port\"                (string, required) Branch node rpc port.\n"
            "4. \"usrname\"             (string, required) Branch node rpc username.\n"
            "5. \"password\"            (string, required) Branch node rpc password.\n"
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

    CellRPCConfig rpcconfig;
    rpcconfig.strIp = ip;
    rpcconfig.iPort = atoi(port);
    rpcconfig.strUser = username;
    rpcconfig.strPassword = password;

    if (branchid != CellBaseChainParams::MAIN && (branchid.length() != 64 || !IsHex(branchid)))
    {
        throw JSONRPCError(RPC_TYPE_ERROR, std::string("Invalid branchid"));
    }

    if (branchid != "main" && (branchid.length() != 64 || !IsHex(branchid)))
    {
        throw JSONRPCError(RPC_TYPE_ERROR, std::string("Invalid branchid"));
    }

    g_branchChainMan->ReplaceRpcConfig(branchid, rpcconfig);
    return "ok";
}

UniValue sendtobranchchain(const JSONRPCRequest& request)
{
    CellWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 3 || request.params.size() > 3)
        throw std::runtime_error(
            "sendtobranchchain branchid address amount,main branchid address is \"main\". \n"
            "\n Send an amount to a branch chain's address.\n"
            "\nArguments:\n"
            "1. \"branchid\"             (string, required) Send to target chain's Branchid,if send to main chain, branchid is \"main\".\n"
            "2. \"address\"              (string, required) The target chain's celllink address to send to.\n"
            "3. \"amount\"               (numeric or string, required) The amount in " + CURRENCY_UNIT + " to send. eg 0.1\n"
            "\nReturns the hash of the created branch chain.\n"
            "\nResult:\n"
            "\"txid\"                  (string) The transaction id.\n"
            "\nExamples:\n"
            + HelpExampleCli("sendtobranchchain", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e XS8XpbMxF5qkDp61SEaa94pwKY6UW6UQd9 0.1")
            + HelpExampleRpc("sendtobranchchain", "93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e XS8XpbMxF5qkDp61SEaa94pwKY6UW6UQd9 0.1")
        );

    LOCK2(cs_main, pwallet->cs_wallet);

    std::string toBranchid = request.params[0].get_str();
    if (Params().GetBranchId() == toBranchid)
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "can not send to this chain.");
    }

    // comment following code
    //    CellRPCConfig branchrpccfg;
    //    if (g_branchChainMan->GetRpcConfig(toBranchid, branchrpccfg) == false || branchrpccfg.IsValid() == false)
    //        throw JSONRPCError(RPC_WALLET_ERROR, "can not found branch rpc config");

    uint256 branchhash;
    branchhash.SetHex(toBranchid);

    // only allow branch-chain to main-chain, main-chain to branch-chain, not allow branch to branch
    if (toBranchid != CellBaseChainParams::MAIN)
    {
        if (!Params().IsMainChain())
        {
            throw JSONRPCError(RPC_WALLET_ERROR, "can not trans from branch-chain to branch-chain");
        }

        //check is branch exist
        //创建分支交易有一定的高度后才允许跨链交易 
        CellTransactionRef txBranch;
        CellBlockIndex* pblockindex = nullptr;
        uint32_t tx_vtx_index = 0;
        CellBlock block;
        if (!GetTransactionDataByTxInfo(branchhash, txBranch, &pblockindex, tx_vtx_index, block))
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
    CellLinkAddress address(strAddress);
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid celllink str address");

    CellKeyID keyId;
    if (!address.GetKeyID(keyId))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid celllink keyid");
    }

    CellAmount nAmount = AmountFromValue(request.params[2]);
    if (!MoneyRange(nAmount))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

    CellCoinControl coin_control;

    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::TRANS_BRANCH_VERSION_S1;
    wtx.sendToBranchid = toBranchid;

    //make branch transaction output
    CellAmount fee2 = 0;
    CellScript sendToScriptPubKey = GetScriptForDestination(address.Get());
    CellMutableTransaction branchStep2MTx;
    if (MakeBranchTransStep2Tx(branchStep2MTx, sendToScriptPubKey, nAmount, fee2, coin_control) == false) {
        throw JSONRPCError(RPC_TYPE_ERROR, "Make branch step2 tx error.");
    }
    CellTransactionRef branchStep2Tx = MakeTransactionRef(std::move(branchStep2MTx));
    wtx.sendToTxHexData = EncodeHexTx(*branchStep2Tx, RPCSerializationFlags());

    CellScript scriptPubKey;
    scriptPubKey << OP_RETURN << OP_TRANS_BRANCH;//lock the output,output trans to /dev/null 

    EnsureWalletIsUnlocked(pwallet);

    bool fSubtractFeeFromAmount = false;
    //SendMoney
    CellAmount curBalance = pwallet->GetBalance();
    if (nAmount + fee2 > curBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Create and send the transaction
    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;
    int nChangePosRet = -1;
    CellRecipient recipient = { scriptPubKey, nAmount + fee2, fSubtractFeeFromAmount };
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + fee2 + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
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
    LOCK(cs_main);

    const std::string& strTx1HexData = request.params[0].get_str();
    CellMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, strTx1HexData))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "DecodeHexTx tx hex fail.\n");
    }

    if (!mtxTrans1.IsPregnantTx())
        throw JSONRPCError(RPC_WALLET_ERROR, "Transaction is not a valid chain trans step1");

    CellMutableTransaction mtxTrans2;
    if (!DecodeHexTx(mtxTrans2, mtxTrans1.sendToTxHexData))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "sendToTxHexData is not a valid transaction data.");
    }

    if (mtxTrans2.IsBranchChainTransStep2() == false)
        throw JSONRPCError(RPC_WALLET_ERROR, "mtxTrans2 is not a branch chain for step2.");

    const std::string strToChainId = mtxTrans1.IsBranchCreate() ? mtxTrans1.GetHash().ToString() : mtxTrans1.sendToBranchid;
    if (strToChainId != Params().GetBranchId())
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "Target branch id is not valid.");
    }

    if (mtxTrans2.fromBranchId == Params().GetBranchId())
        throw JSONRPCError(RPC_WALLET_ERROR, "From chain error,can not from this chain");

    const std::string& strFromChain = mtxTrans2.fromBranchId;
    // only allow branch-chain to main-chain, main-chain to branch-chain, not allow branch to branch
    if (strFromChain != CellBaseChainParams::MAIN)
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
        CellKeyID keyid;
        int64_t coinheight;
        if (GetMortgageMineData(mtxTrans1.vout[0].scriptPubKey, &branchid, &keyid, &coinheight))
        {
            mtxTrans2.vout[0].scriptPubKey = CellScript() << OP_MINE_BRANCH_COIN << ToByteVector(mtxTrans1.GetHash()) << coinheight << OP_2DROP << OP_DUP << OP_HASH160 << ToByteVector(keyid) << OP_EQUALVERIFY << OP_CHECKSIG;
        }
    }
    if (mtxTrans2.fromBranchId != CellBaseChainParams::MAIN){
        mtxTrans2.pPMT.reset(new CellSpvProof(*mtxTrans1.pPMT));
        mtxTrans1.pPMT.reset(new CellSpvProof()); // clear pPMT from mtxTrans1
    }
    CellTransaction tx1(mtxTrans1);
    CellVectorWriter cvw{ SER_NETWORK, INIT_PROTO_VERSION, mtxTrans2.fromTx, 0, tx1 };
    CellTransactionRef tx2 = MakeTransactionRef(std::move(mtxTrans2));

    const CellChainParams& chainparams = Params();
    CellValidationState state;
    CellAmount maxTxFee = DEFAULT_TRANSACTION_MAXFEE;
    bool ret = AcceptToMemoryPool(mempool, state, tx2, true, nullptr, nullptr, false, maxTxFee);
    if (ret == false)
    {
        std::string strError = strprintf("Error: accept to memory pool fail: %s", state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError.c_str());
    }

    //broadcast transaction
    if (!g_connman)
    {
        CellInv inv(MSG_TX, tx2->GetHash());
        g_connman->ForEachNode([&inv](CellNode* pnode)
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

    CellTransactionRef tx;
    CellBlockIndex* pblockindex = nullptr;
    uint32_t tx_vtx_index = 0;
    CellBlock block;

    LOCK(cs_main);
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
    CellTransactionRef tx;
    CellBlockIndex* pblockindex = nullptr;
    uint32_t tx_vtx_index = 0;
    CellBlock block;
    
    LOCK(cs_main);
    if (!GetTransactionDataByTxInfo(txhash, tx, &pblockindex, tx_vtx_index, block))
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("GetTransactDataByTxInfo fail"));

    if (!tx->IsPregnantTx())
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("Invalid branch transaction!"));
    }

    const uint32_t maturity = tx->IsBranchCreate() ? BRANCH_CHAIN_CREATE_COIN_MATURITY : BRANCH_CHAIN_MATURITY;
    int confirmations = chainActive.Height() - pblockindex->nHeight + 1;
    if (confirmations < maturity + 1)
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("can not broadcast because no enough confirmations"));
    }

    if (BranchChainTransStep2(tx, block) == false)
    {
        throw JSONRPCError(RPC_VERIFY_ERROR, std::string("send to target chain return fail."));
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
    CellWallet* const pwallet = GetWalletForJSONRPCRequest(request);
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
            "3. \"address\"              (string, required) The celllink address for Redeem\n"
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

    std::string strBranchid = request.params[0].get_str();
    uint256 branchHash;
    branchHash.SetHex(strBranchid);
    {// branch chain validation check
        CellTransactionRef txBranch;
        CellBlockIndex* pblockindex = nullptr;
        uint32_t tx_vtx_index = 0;
        CellBlock block;
        if (!GetTransactionDataByTxInfo(branchHash, txBranch, &pblockindex, tx_vtx_index, block))
            throw JSONRPCError(RPC_VERIFY_ERROR, "GetTransactDataByTxInfo fail");

        if (txBranch->IsBranchCreate() == false) {
            throw JSONRPCError(RPC_VERIFY_ERROR, "Invalid branchid");
        }

        int confirmations = chainActive.Height() - pblockindex->nHeight + 1;
        if (confirmations < BRANCH_CHAIN_MATURITY) {
            throw JSONRPCError(RPC_VERIFY_ERROR, "Invalid branch transaction,not get enough confirmations");
        }
    }

    CellAmount nAmount = AmountFromValue(request.params[1]);
    if (!MoneyRange(nAmount))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

    if (nAmount < MIN_MINE_BRANCH_MORTGAGE)
        throw JSONRPCError(RPC_TYPE_ERROR, strprintf("MINE MORTGAGE at least %d.%08d COIN", (double)MIN_MINE_BRANCH_MORTGAGE / (double)COIN));

    const std::string& strAddress = request.params[2].get_str();
    CellLinkAddress address(strAddress);
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid celllink pubkey hash address");
    if (address.IsScript())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Can not use script address");
    }
    CellKeyID pubKeyId;
    if (!address.GetKeyID(pubKeyId))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid celllink keyid");
    }

    int nCoinHeight = 0;//指定币高度
    // 抵押vout脚本 branch id , celllink coin address
    const CellScript scriptPubKey = CellScript() << OP_MINE_BRANCH_MORTGAGE << ToByteVector(branchHash) << (nCoinHeight) << OP_2DROP << OP_DUP << OP_HASH160 << ToByteVector(pubKeyId) << OP_EQUALVERIFY << OP_CHECKSIG;

    CellCoinControl coin_control;

    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::MINE_BRANCH_MORTGAGE;
    wtx.sendToBranchid = strBranchid;

    //make branch transaction output
    CellAmount fee2 = 0;
    const CellScript sendToScriptPubKey;// empty script, delay set in fucntion `makebranchtransaction` 
    CellMutableTransaction branchStep2MTx;// 侧链交易,将抵押币转成挖矿币,输出脚本先是个空脚本,在侧链收录该交易时,该输出地址会转成挖矿币脚本
    if (MakeBranchTransStep2Tx(branchStep2MTx, sendToScriptPubKey, nAmount, fee2, coin_control) == false) { 
        throw JSONRPCError(RPC_TYPE_ERROR, "Make branch step2 tx error.");
    }
    CellTransactionRef branchStep2Tx = MakeTransactionRef(std::move(branchStep2MTx));
    wtx.sendToTxHexData = EncodeHexTx(*branchStep2Tx, RPCSerializationFlags());

    //check balance
    CellAmount curBalance = pwallet->GetBalance();
    if (nAmount + fee2 > curBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // 特殊设定：vout 0号位是抵押币 1号位侧链交易手续费 2号位可能是找零、也可能是空（没找零的情况）。
    bool fSubtractFeeFromAmount = false;
    // Create and send the transaction
    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;

    CellRecipient recipient = {scriptPubKey, nAmount, fSubtractFeeFromAmount};
    vecSend.push_back(recipient);// vout 0 是抵押币
    //侧链手续费
    CellScript scriptNull;
    scriptNull << OP_RETURN << OP_TRANS_BRANCH;
    vecSend.push_back({scriptNull, fee2, fSubtractFeeFromAmount}); // vout 1 是侧链手续费

    int nChangePosRet = vecSend.size(); // vout end-1 找零, fixed change vout pos 
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + fee2 + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
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
    CellWallet* const pwallet = GetWalletForJSONRPCRequest(request);
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
    CellMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, strTx1HexData))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "DecodeHexTx tx hex fail.\n");
    }
    if (!mtxTrans1.IsSyncBranchInfo())
        throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid transaction data");
    
    ///////////
    CellCoinControl coin_control;

    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::SYNC_BRANCH_INFO;
    wtx.pBranchBlockData = std::move(mtxTrans1.pBranchBlockData);
    wtx.isDataTransaction = true;

    bool fSubtractFeeFromAmount = false;
    EnsureWalletIsUnlocked(pwallet);
    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    CellAmount curBalance = pwallet->GetBalance();
    // Create and send the transaction
    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;
    int nChangePosRet = -1;
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
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
    uint256 branchid = ParseHashV(request.params[0], "param 0");
    if (!pBranchChainTxRecordsDb->IsBranchCreated(branchid))
        throw JSONRPCError(RPC_WALLET_ERROR, "Branch which you query did not created");

    if (pBranchDb->mTopHashDatas.count(branchid) == 0)
        throw JSONRPCError(RPC_WALLET_ERROR, "No top hash data");

    TopHashData topHashData = pBranchDb->mTopHashDatas[branchid];
    UniValue retObj(UniValue::VOBJ);
    retObj.push_back(Pair("blockhash", topHashData.topHash.ToString()));
    retObj.push_back(Pair("height", (uint64_t)topHashData.topHeight));
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
    CellBlock block;
    CellBlockIndex* pBlockIndex = chainActive[blockheight];
    if (!ReadBlockFromDisk(block, pBlockIndex, Params().GetConsensus()))
        throw JSONRPCError(RPC_MISC_ERROR, "Block not found on disk");

    std::shared_ptr<const CellBlock> shared_pblock = std::make_shared<const CellBlock>(block);
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
    CellWallet * const pwallet = GetWalletForJSONRPCRequest(request);
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

    uint256 txid = ParseHashV(request.params[0], "param 0");
    int32_t n = 0;
    if (request.params.size() > 1){
        if (request.params[1].isNum())
            n = request.params[1].get_int();
        else
            ParseInt32(request.params[1].get_str(), &n);
    }

    CellOutPoint outpoint(txid, n);
    const Coin& coin = pcoinsTip->AccessCoin(outpoint);
    if (coin.IsSpent())
        throw JSONRPCError(RPC_WALLET_ERROR, "Coin is spent!");
    if (chainActive.Height() - coin.nHeight < REDEEM_SAFE_HEIGHT)// 挖矿币需要满足一定高度后才能赎回,给别人举报有时间窗口
        throw JSONRPCError(RPC_INVALID_REQUEST, "Coin need ");

    uint256 fromtxid;
    CellKeyID keyid;
    int64_t height;
    if (!GetMortgageCoinData(coin.out.scriptPubKey, &fromtxid, &keyid, &height))
        throw JSONRPCError(RPC_INVALID_REQUEST, "Invalid mortgage coin.");

    CellCoinControl coin_control;
    coin_control.Select(outpoint);
    coin_control.fAllowOtherInputs = true;

    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::REDEEM_MORTGAGE_STATEMENT;
    
    CellScript scriptPubKey;
    scriptPubKey << OP_RETURN << OP_REDEEM_MORTGAGE << ToByteVector(fromtxid);// output trans to /dev/null

    EnsureWalletIsUnlocked(pwallet);

    bool fSubtractFeeFromAmount = false;
    //SendMoney
    CellAmount curBalance = pwallet->GetBalance();
    CellAmount nAmount = coin.out.nValue;
    //if (nAmount > curBalance)
    //    throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Create and send the transaction
    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;
    int nChangePosRet = -1;
    CellRecipient recipient = { scriptPubKey, nAmount, fSubtractFeeFromAmount };
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
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
    CellWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() < 5 || request.params.size() > 5)
        throw std::runtime_error(
            "redeemmortgagecoin \"txid\" \"outindex\" \"statementtxid\"\n"
            "\nRedeem mortgage coin by outpoint info(txid and vout index of coin)\n"
            "\nArguments:\n"
            "1. \"txid\"             (string, required) The transaction hash of coin in main chain(CellOutPoint hash).\n"
            "2. \"voutindex\"        (number, required) The vout index of coin, default is 0(CellOutPoint n).\n"
            "3. \"fromtx\"           (string, required) The statement transactoin hex data in branch chain.\n"
            "4. \"frombranchid\"     (string, required) Which branch id the coin mortgage.\n"
            "5. \"svpproof\"         (string, required) CellSpvProof hex data.\n"
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
    
    uint256 mortgagecoinhash = ParseHashV(request.params[0], "param 0");
    int32_t nvoutindex = 0;
    if (request.params.size() > 1) {
        if (request.params[1].isNum())
            nvoutindex = request.params[1].get_int();
        else
            ParseInt32(request.params[1].get_str(), &nvoutindex);
    }

    std::string strTxHexData = request.params[2].get_str();
    CellMutableTransaction statementmtx;
    if (!DecodeHexTx(statementmtx, strTxHexData))
        throw JSONRPCError(RPC_INVALID_REQUEST, "Parameter 'fromtx' invalid hex tx data.");
    
    uint256 frombranchid = ParseHashV(request.params[3], "param 3");
    CellSpvProof spvProof;
    DecodeHexSpv(spvProof, request.params[4].get_str());

    CellOutPoint outpoint(mortgagecoinhash, nvoutindex);
    const Coin& coin = pcoinsTip->AccessCoin(outpoint);
    if (coin.IsSpent())
        throw JSONRPCError(RPC_WALLET_ERROR, "Coin is spent!");
    if (!pwallet->IsMine(coin.out))
        throw JSONRPCError(RPC_INVALID_REQUEST, "Coin is not mine!");

    uint256 coinMortgageBranchId;
    CellKeyID coinMortgageKeyId;
    if (!GetMortgageMineData(coin.out.scriptPubKey, &coinMortgageBranchId, &coinMortgageKeyId))
        throw JSONRPCError(RPC_WALLET_ERROR, "Is not a valid mortgage coin");

    if (coinMortgageBranchId != frombranchid)
        throw JSONRPCError(RPC_INVALID_REQUEST, "Branch id not match");

    CellCoinControl coin_control;
    coin_control.Select(outpoint); 
    coin_control.fAllowOtherInputs = true;

    //create transaction
    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::REDEEM_MORTGAGE;
    wtx.pPMT.reset(new CellSpvProof(spvProof));
    wtx.fromBranchId = frombranchid.ToString();

    CellScript scriptPubKey = GetScriptForDestination(coinMortgageKeyId);
    
    EnsureWalletIsUnlocked(pwallet);

    bool fSubtractFeeFromAmount = false;
    //SendMoney
    CellAmount curBalance = pwallet->GetBalance();
    CellAmount nAmount = coin.out.nValue;
    //if (nAmount > curBalance)
    //    throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    // Create and send the transaction
    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;
    int nChangePosRet = -1;
    CellRecipient recipient = { scriptPubKey, nAmount, fSubtractFeeFromAmount };
    vecSend.push_back(recipient);
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nAmount + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
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


UniValue sendreporttomain(const JSONRPCRequest& request)
{
    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not call this RPC in main chain!\n");

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
        throw std::runtime_error(
                "sendreporttomain \"blockhash\"  \"txid\"\n"
                "\nSend invalid transaction proof to main chain. \n"
                "\nArguments:\n"
                "1. \"blockhash\"        (string, required) The block hash.\n"
                "2. \"txid\"             (string, required) A transaction hash.\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "\n"
                "\nExamples:\n"
                + HelpExampleCli("sendprovetomain", "\"blockhash\" \"txid\"")
                + HelpExampleRpc("sendprovetomain", "\"blockhash\" \"txid\"")
                );

    uint256 blockHash = ParseHashV(request.params[0], "param 0");
    if (!mapBlockIndex.count(blockHash))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");
    CellBlockIndex*  pblockindex = mapBlockIndex[blockHash];

    CellBlock block;
    if (!ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))   
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");

    uint256 txHash = ParseHashV(request.params[1], "transaction hash");
    std::set<uint256> setTxids;
    setTxids.insert(txHash);

    std::vector<bool> vMatch;
    std::vector<uint256> vHashes;

    vMatch.reserve(block.vtx.size());
    vHashes.reserve(block.vtx.size());

    for (size_t i = 0; i < block.vtx.size(); ++i)
    {
        const uint256& hash = block.vtx[i]->GetHash();
        if (setTxids.count(hash))
            vMatch.push_back(true);
        else
            vMatch.push_back(false);
        vHashes.push_back(hash);
    }

    CellMutableTransaction mtx;
    mtx.nVersion = CellTransaction::REPORT_CHEAT;
    mtx.pPMT.reset(new CellSpvProof(vHashes, vMatch, block.GetHash()));

    ReportData* pReportData = new ReportData;
    mtx.pReportData.reset(pReportData);
    pReportData->reportedTxHash = txHash;
    pReportData->reportedBranchId = Params().GetBranchHash(); 
    pReportData->reportedBlockHash = block.GetHash();

    CellRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig("main", branchrpccfg) == false)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "invalid rpc config");

    const std::string strMethod = "handlebranchreport";
    UniValue params(UniValue::VARR);
    CellTransactionRef txRef = MakeTransactionRef(std::move(mtx));
    params.push_back(EncodeHexTx(*txRef, RPCSerializationFlags()));

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);
    const UniValue& error = find_value(reply, "error");
    if (!error.isNull())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "call rpc error");

    return "ok";
}

UniValue handlebranchreport(const JSONRPCRequest& request)
{
    if (!Params().IsMainChain())
        throw JSONRPCRequest();

    CellWallet* const pwallet = GetWalletForJSONRPCRequest(request);
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
                + HelpExampleCli("submitbranchblockinfo", "5754f9e...630db")
                + HelpExampleRpc("submitbranchblockinfo", "5754f9e...630db")
                );

    const std::string& strTx1HexData = request.params[0].get_str();
    CellMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, strTx1HexData))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "DecodeHexTx tx hex fail.\n");
    }
    if (!mtxTrans1.IsReport())
        throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid transaction data");

    CellTransactionRef tx = MakeTransactionRef(std::move(mtxTrans1));

    const uint256 reportedBranchId = tx->pReportData->reportedBranchId;
    if (!pBranchDb->HasBranchData(reportedBranchId))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid reported branch id");
    BranchData branchData = pBranchDb->GetBranchData(reportedBranchId);
    if (branchData.mapHeads.count(tx->pReportData->reportedBlockHash))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not found block data in mapHeads");

    std::vector<uint256> vMatch;
    std::vector<unsigned int> vIndex;
    CellSpvProof svpProof(*tx->pPMT);
    if (svpProof.pmt.ExtractMatches(vMatch, vIndex) != branchData.mapHeads[tx->pPMT->blockhash].header.hashMerkleRoot)
    {
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid transaction spv");
    }

    uint256 reportFlagHash = Hash(reportedBranchId.begin(), reportedBranchId.end(),
                                  tx->pReportData->reportedBlockHash.begin(), tx->pReportData->reportedBlockHash.end(),
                                  tx->pReportData->reportedTxHash.begin(), tx->pReportData->reportedTxHash.end());
    if (pBranchDb->mReortTxFlag.count(reportFlagHash))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Tansaction had been reported!");

    CellPubKey newKey;
    if(!pwallet->GetKeyFromPool(newKey))
    {
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    }
    CellKeyID keyID = newKey.GetID();
    pwallet->SetAddressBook(keyID, "", "handlebranchreport");
    CellTxDestination kDest(keyID);
    CellScript scriptPubKey = GetScriptForDestination(kDest);

    CellCoinControl coin_control;
    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::REPORT_CHEAT;
    wtx.pReportData.reset(new ReportData(*tx->pReportData)); 

    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;
    bool fSubtractFeeFromAmount = false;
    CellAmount curBalance = pwallet->GetBalance();
    CellAmount nValue  = DUST_RELAY_TX_FEE; 
    CellRecipient recipient = { scriptPubKey, nValue, fSubtractFeeFromAmount};
    vecSend.push_back(recipient);
   int nChangePosRet = vecSend.size(); 

    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nValue + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    pBranchDb->mReortTxFlag[reportFlagHash] = FLAG_REPORTED;
    
    return "ok";
}


UniValue sendprovetomain(const JSONRPCRequest& request)
{
    if (Params().IsMainChain())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can not call this RPC in main chain!\n");

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
        throw std::runtime_error(
                "sendreporttomain \"blockhash\"  \"txid\"\n"
                "\nSend invalid transaction proof to main chain. \n"
                "\nArguments:\n"
                "1. \"blockhash\"        (string, required) The block hash.\n"
                "2. \"txid\"             (string, required) A transaction hash.\n"
                "\nReturns ok or fail.\n"
                "\nResult:\n"
                "\n"
                "\nExamples:\n"
                + HelpExampleCli("sendprovetomain", "\"blockhash\" \"txid\"")
                + HelpExampleRpc("sendprovetomain", "\"blockhash\" \"txid\"")
                );

    uint256 blockHash = ParseHashV(request.params[0], "param 0");
    if (!mapBlockIndex.count(blockHash))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");
    CellBlockIndex*  pblockindex = mapBlockIndex[blockHash];

    CellBlock block;
    if (!ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))   
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");

    uint256 txHash = ParseHashV(request.params[1], "transaction hash");

    std::vector<ProveData> vectProveData;
    if (!GetProveInfo(block, txHash, vectProveData))
    {
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Get transaction prove data failed");
    }

    CellMutableTransaction mtx;
    mtx.nVersion = CellTransaction::PROVE;
    mtx.vectProveData = vectProveData;

    CellRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig("main", branchrpccfg) == false)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "invalid rpc config");

    const std::string strMethod = "handlebranchprove";
    UniValue params(UniValue::VARR);
    CellTransactionRef txRef = MakeTransactionRef(std::move(mtx));
    params.push_back(EncodeHexTx(*txRef, RPCSerializationFlags()));

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);
    const UniValue& error = find_value(reply, "error");
    if (!error.isNull())
        throw JSONRPCError(RPC_INTERNAL_ERROR, "call rpc error");

    return "ok";
}

UniValue handlebranchprove(const JSONRPCRequest& request)
{
      if (!Params().IsMainChain())
        throw JSONRPCRequest();
    CellWallet* const pwallet = GetWalletForJSONRPCRequest(request);
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
                + HelpExampleCli("submitbranchblockinfo", "5754f9e...630db")
                + HelpExampleRpc("submitbranchblockinfo", "5754f9e...630db")
                );

    const std::string& strTx1HexData = request.params[0].get_str();
    CellMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, strTx1HexData))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "DecodeHexTx tx hex fail.\n");
    }
    if (!mtxTrans1.IsProve())
        throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid transaction data");

    CellTransactionRef tx = MakeTransactionRef(std::move(mtxTrans1));

    ProveData proveData = tx->vectProveData[1];
    uint256 proveFlagHash = Hash(proveData.branchId.begin(), proveData.branchId.end(),
                                 proveData.blockHash.begin(), proveData.blockHash.end(),
                                 proveData.txHash.begin(), proveData.txHash.end());

    if (!pBranchDb->mReortTxFlag.count(proveFlagHash) || pBranchDb->mReortTxFlag[proveFlagHash] != FLAG_REPORTED)
    {
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid report transaction");
    }

    CellPubKey newKey;
    if(!pwallet->GetKeyFromPool(newKey))
    {
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    }
    CellKeyID keyID = newKey.GetID();
    pwallet->SetAddressBook(keyID, "", "handlebranchprove");

    CellTxDestination kDest(keyID);
    CellScript scriptPubKey = GetScriptForDestination(kDest);

    CellCoinControl coin_control;
    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::PROVE;
    wtx.vectProveData = tx->vectProveData;

    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;
    bool fSubtractFeeFromAmount = false;
    CellAmount curBalance = pwallet->GetBalance();
    CellAmount nValue = DUST_RELAY_TX_FEE;
    CellRecipient recipient = { scriptPubKey, nValue, fSubtractFeeFromAmount };
    vecSend.push_back(recipient);
   int nChangePosRet = vecSend.size(); 
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nValue + nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    pBranchDb->mReortTxFlag[proveFlagHash] = FLAG_PROVED;

    return "ok";
}

// 举报后,在举报交易被打包后 REPORT_LOCK_COIN_HEIGHT + 1个块后手动在支链调用该接口
// 锁定挖矿币
UniValue lockmortgageminecoin(const JSONRPCRequest& request)
{
    CellWallet * const pwallet = GetWalletForJSONRPCRequest(request);
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
    uint256 reporttxid = ParseHashV(request.params[0], "param 0");
    uint256 coinprevouthash = ParseHashV(request.params[1], "param 1");

    CellCoinControl coin_control;

    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::LOCK_MORTGAGE_MINE_COIN;
    wtx.reporttxid = reporttxid;
    wtx.coinpreouthash = coinprevouthash;
    wtx.isDataTransaction = true;

    bool fSubtractFeeFromAmount = false;
    EnsureWalletIsUnlocked(pwallet);
    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    CellAmount curBalance = pwallet->GetBalance();
    // Create and send the transaction
    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;
    int nChangePosRet = -1;
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
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

    uint256 reporttxid = ParseHashV(request.params[0], "param 0");

    CellTransactionRef ptxReport;
    uint256 hashBlock;
    bool retflag;
    bool retval = ReadTxDataByTxIndex(reporttxid, ptxReport, hashBlock, retflag);
    if (ptxReport == nullptr)
        throw JSONRPCError(RPC_INVALID_REQUEST, "read tx data fail");

    int confirmations = 0;
    if (mapBlockIndex.count(hashBlock))
        confirmations = chainActive.Height() - mapBlockIndex[hashBlock]->nHeight;
    
    // get mine coin prevouthash
    LOCK(cs_main);// protect pBranchDb
    uint256 prevouthash;
    BranchData branchdata = pBranchDb->GetBranchData(ptxReport->pReportData->reportedBranchId);// don't check
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
    CellWallet * const pwallet = GetWalletForJSONRPCRequest(request);
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

    uint256 reporttxid = ParseHashV(request.params[0], "param 0");
    uint256 coinprevouthash = ParseHashV(request.params[1],"param 1");
    uint256 provetxid = ParseHashV(request.params[2], "param 2");

    CellCoinControl coin_control;

    CellWalletTx wtx;
    wtx.transaction_version = CellTransaction::UNLOCK_MORTGAGE_MINE_COIN;
    wtx.reporttxid = reporttxid;
    wtx.coinpreouthash = coinprevouthash;
    wtx.provetxid = provetxid;
    wtx.isDataTransaction = true;

    bool fSubtractFeeFromAmount = false;
    EnsureWalletIsUnlocked(pwallet);
    if (pwallet->GetBroadcastTransactions() && !g_connman) {
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");
    }

    CellAmount curBalance = pwallet->GetBalance();
    // Create and send the transaction
    CellReserveKey reservekey(pwallet);
    CellAmount nFeeRequired;
    std::string strError;
    std::vector<CellRecipient> vecSend;
    int nChangePosRet = -1;
    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coin_control)) {
        if (!fSubtractFeeFromAmount && nFeeRequired > curBalance)
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    CellValidationState state;
    if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state)) {
        strError = strprintf("Error: The transaction was rejected! Reason given: %s", state.GetRejectReason());
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

    uint256 provetxid = ParseHashV(request.params[0], "param 0");

    CellTransactionRef ptxProve;
    uint256 hashBlock;
    bool retflag;
    bool retval = ReadTxDataByTxIndex(provetxid, ptxProve, hashBlock, retflag);
    if (ptxProve == nullptr)
        throw JSONRPCError(RPC_INVALID_REQUEST, "read tx data fail");

    int confirmations = 0;
    if (mapBlockIndex.count(hashBlock))
        confirmations = chainActive.Height() - mapBlockIndex[hashBlock]->nHeight;

    // get mine coin prevouthash
    LOCK(cs_main);// protect pBranchDb
    uint256 prevouthash;
    //BranchData branchdata = pBranchDb->GetBranchData(ptxProve->vectProveData->reportedBranchId);// don't check
    //if (branchdata.mapHeads.count(ptxProve->pReportData->reportedBlockHash)) {
    //    if (!GetMortgageCoinData(branchdata.mapHeads[ptxProve->pReportData->reportedBlockHash].pStakeTx->vout[0].scriptPubKey, &prevouthash))
    //        throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid-block-data");
    //}

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

    // report and provre api
    { "branchchain",        "sendreporttomain",          &sendreporttomain,            false, {"blockhash", "txid"}},
    { "branchchain",        "handlebranchreport",        &handlebranchreport,          true,  {"tx_hex_data"}},
    // 证明交易数据还需要修改和完善
    { "branchchain",        "sendprovetomain",           &sendprovetomain,             false, {"blockhash", "txid"}},
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
