// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "chain/branchchain.h"
#include "rpc/branchchainrpc.h"

#if defined(HAVE_CONFIG_H)
#include "magnachain-config.h"
#endif

#include "chainparamsbase.h"
#include "io/fs.h"
#include "misc/clientversion.h"
#include "rpc/client.h"
#include "rpc/protocol.h"
#include "utils/util.h"
#include "utils/utilstrencodings.h"

#include <stdio.h>

#include "support/events.h"
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include "chainparams.h"
#include "consensus/merkle.h"
#include "consensus/validation.h"
#include "misc/tinyformat.h"
#include "primitives/block.h"
#include "thread/sync.h"
#include "validation/validation.h"
#include <univalue.h>

#include "coding/base58.h"
#include "script/standard.h"

#include "chain/branchdb.h"
#include "io/core_io.h"
#include "misc/timedata.h"
#include "rpc/server.h"
#include "smartcontract/smartcontract.h"
#include "transaction/txmempool.h"

static const int DEFAULT_HTTP_CLIENT_TIMEOUT = 900;

//
// Exception thrown on connection error.  This error is used to determine
// when to wait if -rpcwait is given.
//
class MCConnectionFailed : public std::runtime_error
{
public:
    explicit inline MCConnectionFailed(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

/** Reply structure for request_done to fill in */
struct HTTPReply {
    HTTPReply() : status(0), error(-1) {}

    int status;
    int error;
    std::string body;
};

const char* http_errorstring2(int code)
{
    switch (code) {
#if LIBEVENT_VERSION_NUMBER >= 0x02010300
    case EVREQ_HTTP_TIMEOUT:
        return "timeout reached";
    case EVREQ_HTTP_EOF:
        return "EOF reached";
    case EVREQ_HTTP_INVALID_HEADER:
        return "error while reading header, or invalid header";
    case EVREQ_HTTP_BUFFER_ERROR:
        return "error encountered while reading or writing";
    case EVREQ_HTTP_REQUEST_CANCEL:
        return "request was canceled";
    case EVREQ_HTTP_DATA_TOO_LONG:
        return "response body is larger than allowed";
#endif
    default:
        return "unknown";
    }
}

static void http_request_done(struct evhttp_request* req, void* ctx)
{
    HTTPReply* reply = static_cast<HTTPReply*>(ctx);

    if (req == nullptr) {
        /* If req is nullptr, it means an error occurred while connecting: the
		* error code will have been passed to http_error_cb.
		*/
        reply->status = 0;
        return;
    }

    reply->status = evhttp_request_get_response_code(req);

    struct evbuffer* buf = evhttp_request_get_input_buffer(req);
    if (buf) {
        size_t size = evbuffer_get_length(buf);
        const char* data = (const char*)evbuffer_pullup(buf, size);
        if (data)
            reply->body = std::string(data, size);
        evbuffer_drain(buf, size);
    }
}

#if LIBEVENT_VERSION_NUMBER >= 0x02010300
static void http_error_cb(enum evhttp_request_error err, void* ctx)
{
    HTTPReply* reply = static_cast<HTTPReply*>(ctx);
    reply->error = err;
}
#endif

UniValue CallRPC(const std::string& host, const int port, const std::string& strMethod, const UniValue& params, const std::string& strRPCUserColonPass, const std::string& rpcwallet /*=""*/)
{
    // Obtain event base
    raii_event_base base = obtain_event_base();

    // Synchronously look up hostname
    raii_evhttp_connection evcon = obtain_evhttp_connection_base(base.get(), host, port);
    evhttp_connection_set_timeout(evcon.get(), DEFAULT_HTTP_CLIENT_TIMEOUT);

    HTTPReply response;
    raii_evhttp_request req = obtain_evhttp_request(http_request_done, (void*)&response);
    if (req == nullptr)
        throw std::runtime_error("create http request failed");
#if LIBEVENT_VERSION_NUMBER >= 0x02010300
    evhttp_request_set_error_cb(req.get(), http_error_cb);
#endif

    struct evkeyvalq* output_headers = evhttp_request_get_output_headers(req.get());
    assert(output_headers);
    evhttp_add_header(output_headers, "Host", host.c_str());
    evhttp_add_header(output_headers, "Connection", "close");
    evhttp_add_header(output_headers, "Authorization", (std::string("Basic ") + EncodeBase64(strRPCUserColonPass)).c_str());

    // Attach request data
    std::string strRequest = JSONRPCRequestObj(strMethod, params, 1).write() + "\n";
    struct evbuffer* output_buffer = evhttp_request_get_output_buffer(req.get());
    assert(output_buffer);
    evbuffer_add(output_buffer, strRequest.data(), strRequest.size());

    // check if we should use a special wallet endpoint
    std::string endpoint = "/";
    std::string walletName = rpcwallet;
    if (!walletName.empty()) {
        char* encodedURI = evhttp_uriencode(walletName.c_str(), walletName.size(), false);
        if (encodedURI) {
            endpoint = "/wallet/" + std::string(encodedURI);
            free(encodedURI);
        } else {
            throw MCConnectionFailed("uri-encode failed");
        }
    }
    int r = evhttp_make_request(evcon.get(), req.get(), EVHTTP_REQ_POST, endpoint.c_str());
    req.release(); // ownership moved to evcon in above call
    if (r != 0) {
        throw MCConnectionFailed("send http request failed");
    }

    event_base_dispatch(base.get());

    if (response.status == 0)
        throw MCConnectionFailed(strprintf("couldn't connect to server: %s (code %d)\n(make sure server is running and you are connecting to the correct RPC port)", http_errorstring2(response.error), response.error));
    else if (response.status == HTTP_UNAUTHORIZED)
        throw std::runtime_error("incorrect rpcuser or rpcpassword (authorization failed)");
    else if (response.status >= 400 && response.status != HTTP_BAD_REQUEST && response.status != HTTP_NOT_FOUND && response.status != HTTP_INTERNAL_SERVER_ERROR)
        throw std::runtime_error(strprintf("server returned HTTP error %d", response.status));
    else if (response.body.empty())
        throw std::runtime_error("no response from server");

    // Parse reply
    UniValue valReply(UniValue::VSTR);
    if (!valReply.read(response.body))
        throw std::runtime_error("couldn't parse reply from server");
    const UniValue& reply = valReply.get_obj();
    if (reply.empty())
        throw std::runtime_error("expected reply to have result, error and id properties");

    return reply;
}

UniValue CallRPC(MCRPCConfig& rpccfg, const std::string& strMethod, const UniValue& params)
{
    try {
        if (rpccfg.strRPCUserColonPass.empty() && rpccfg.getcookiefail > 0) {
            rpccfg.InitUserColonPass(true);
        }

        UniValue ret = CallRPC(rpccfg.strIp, rpccfg.iPort, strMethod, params, rpccfg.strRPCUserColonPass, rpccfg.strWallet);
        return ret;
    } catch (const MCConnectionFailed& e) {
        error("%s: CallRPC excetion , %s", __func__, e.what());
        return JSONRPCReplyObj(NullUniValue, e.what(), 1);
    } catch (const std::exception& e) {
        error("%s: may be CallRPC excetion cannot connect to main chain,%s", __func__, e.what());
        return JSONRPCReplyObj(NullUniValue, e.what(), 1);
    }
    return NullUniValue;
}

MCRPCConfig::MCRPCConfig() : iPort(0), getcookiefail(0)
{
}

void MCRPCConfig::Reset()
{
    strBranchId.clear();
    strIp.clear();
    iPort = 0;
    strUser.clear();
    strPassword.clear();
    strWallet.clear();
    strDataDir.clear();
    strRPCUserColonPass.clear();
    getcookiefail = 0;
}

bool MCRPCConfig::IsValid()
{
    if ((strRPCUserColonPass.empty() && getcookiefail == 0) || iPort == 0)
        return false;
    return true;
}

bool MCRPCConfig::InitUserColonPass(bool bthrowexcetion)
{
    // Get credentials
    if (!strDataDir.empty() && strPassword.empty()) // load datadir conf first.
    {
        ArgsManager args;
        fs::path path = fs::system_complete(strDataDir);
        if (!fs::is_directory(path)) {
            if (bthrowexcetion) throw std::runtime_error("Error: Invalid branch rpc config datadir.");
            error("%s Invalid branch rpc config datadir.\n", __func__, strDataDir);
            return false;
        }

        try {
            std::string configfilepath = (path / MAGNACHAIN_CONF_FILENAME).string();
            args.ReadConfigFile(configfilepath);
        }
        catch (const std::exception& e) { // no config file is ok
            fprintf(stderr, "%s:%d reading configuration file fail(%s)\n", __FILE__, __LINE__, e.what());
        }
        if (args.GetArg("-rpcpassword", "") == "") //get cookie file, the cookie file only exist when branch is running
        {
            bool fTestNet = gArgs.GetBoolArg("-testnet", false);
            bool fRegTest = gArgs.GetBoolArg("-regtest", false);

            fs::path cookiepath = path / ".cookie";
            if (fTestNet || fRegTest) {
                if (strBranchId == MCBaseChainParams::MAIN) //note that main branch datadir for testnet or regtest will be testnet3 or regtest
                {
                    std::string subdir = fTestNet ? SUB_TESTNET_DATADIR : SUB_REGTEST_DATADIR;
                    cookiepath = path / subdir / ".cookie";
                }
            }

            if (!GetAuthCookie(&strRPCUserColonPass, &cookiepath)) {
                getcookiefail++;
                if (bthrowexcetion) throw std::runtime_error("Error: No authentication cookie could be found");
                return false;
            }
        } else {
            strPassword = args.GetArg("-rpcpassword", "");
            if (args.GetArg("-rpcuser", "") != "") { // choise parameters in config file first.
                strUser = args.GetArg("-rpcuser", "");
            }
            strRPCUserColonPass = strUser + ":" + strPassword;
        }
    } else {
        strRPCUserColonPass = strUser + ":" + strPassword;
    }
    return !strRPCUserColonPass.empty();
}

std::unique_ptr<MCBranchChainMan> g_branchChainMan = nullptr;

MCBranchChainMan::MCBranchChainMan()
{
}

MCBranchChainMan::~MCBranchChainMan()
{
}

void MCBranchChainMan::Init()
{
    std::string strMainChainCfg = gArgs.GetArg("-mainchaincfg", "");
    if (strMainChainCfg.empty() == false) {
        std::string strName;
        MCRPCConfig rpccfg;
        if ((ParseRpcConfig(strMainChainCfg, rpccfg, strName) && rpccfg.IsValid()) || rpccfg.getcookiefail == 1) // if getcookiefail, we will give a chance for later try again,may the target chain is not running now.
        {
            mapRpcConfig[MCBaseChainParams::MAIN] = rpccfg;
        }
    }

    std::vector<std::string> vBranchChainRpcCfg = gArgs.GetArgs("-branchcfg");
    for (std::string var : vBranchChainRpcCfg) {
        std::string branchid;
        MCRPCConfig rpccfg;
        if ((ParseRpcConfig(var, rpccfg, branchid) && rpccfg.IsValid()) || rpccfg.getcookiefail == 1) {
            mapRpcConfig[branchid] = rpccfg;
        }
    }
}

bool MCBranchChainMan::ParseRpcConfig(const std::string& strCfg, MCRPCConfig& rpccfg, std::string& branchid)
{
    UniValue uv;
    if (uv.read(strCfg) == false)
        return false;

    UniValue uvBranchid = find_value(uv, "branchid");
    if (uvBranchid.isNull()) {
        branchid = MCBaseChainParams::MAIN;
    } else
        branchid = uvBranchid.get_str();
    rpccfg.strBranchId = branchid;

    UniValue uvIp = find_value(uv, "ip");
    if (uvIp.isNull())
        return false;
    rpccfg.strIp = uvIp.get_str();

    UniValue uvPort = find_value(uv, "port");
    if (uvPort.isNull())
        return false;
    rpccfg.iPort = uvPort.get_int();
    if (rpccfg.iPort == 0)
        return false;

    UniValue uvUserName = find_value(uv, "usrname");
    if (uvUserName.isNull() == false)
        rpccfg.strUser = uvUserName.get_str();

    UniValue uvPassworld = find_value(uv, "password");
    if (uvPassworld.isNull() == false)
        rpccfg.strPassword = uvPassworld.get_str();

    UniValue uvWallet = find_value(uv, "wallet");
    if (uvWallet.isNull() == false)
        rpccfg.strWallet = uvWallet.get_str();

    UniValue uvDataDir = find_value(uv, "datadir");
    if (uvDataDir.isNull() == false) {
        rpccfg.strDataDir = uvDataDir.get_str();
    }

    if (!rpccfg.InitUserColonPass(false)) {
        return false;
    }

    return true;
}

bool MCBranchChainMan::GetRpcConfig(const std::string& strName, MCRPCConfig& rpccfg)
{
    MAP_RPC_CONFIG::iterator mit = mapRpcConfig.find(strName);
    if (mit == mapRpcConfig.end())
        return false;

    rpccfg = mit->second;
    return true;
}

bool MCBranchChainMan::CheckRpcConfig(MCRPCConfig& rpccfg)
{
    if (rpccfg.iPort == 0)
        return false;
    return true;
}

void MCBranchChainMan::ReplaceRpcConfig(const std::string& strName, MCRPCConfig& rpccfg)
{
    mapRpcConfig[strName] = rpccfg;
}

MCAmount GetBranchChainCreateTxOut(const MCTransaction& tx)
{
    MCAmount nAmount(0);
    if (tx.IsBranchCreate() == false)
        return nAmount;

    for (MCTxOut txout : tx.vout) {
        opcodetype opcode;
        std::vector<unsigned char> vch;
        MCScript::const_iterator pc1 = txout.scriptPubKey.begin();
        txout.scriptPubKey.GetOp(pc1, opcode, vch);
        if (opcode == OP_CREATE_BRANCH) {
            nAmount += txout.nValue;
        }
    }
    return nAmount;
}

MCAmount GetBranchChainTransOut(const MCTransaction& branchTransStep1Tx)
{
    MCAmount nAmount(0);
    if (branchTransStep1Tx.IsBranchChainTransStep1() == false)
        return nAmount;

    for (MCTxOut txout : branchTransStep1Tx.vout) {
        opcodetype opcode;
        std::vector<unsigned char> vch;
        MCScript::const_iterator pc1 = txout.scriptPubKey.begin();
        if (txout.scriptPubKey.GetOp(pc1, opcode, vch)) {
            if (branchTransStep1Tx.sendToBranchid != MCBaseChainParams::MAIN) {
                if (opcode == OP_TRANS_BRANCH) {
                    if (txout.scriptPubKey.GetOp(pc1, opcode, vch) && vch.size() == sizeof(uint256)) {
                        uint256 branchhash(vch);
                        if (branchhash.ToString() == branchTransStep1Tx.sendToBranchid) { //branch id check
                            nAmount += txout.nValue;
                        }
                    }
                }
            } else {
                if (opcode == OP_RETURN) {
                    txout.scriptPubKey.GetOp(pc1, opcode, vch);
                    if (opcode == OP_TRANS_BRANCH) {
                        nAmount += txout.nValue;
                    }
                }
            }
        }
    }
    return nAmount;
}

//获取交易中的抵押币输出
MCAmount GetMortgageMineOut(const MCTransaction& tx, bool bWithBranchOut)
{
    MCAmount nAmount(0);
    std::vector<unsigned char> vch;
    for (MCTxOut txout : tx.vout) {
        opcodetype opcode;
        MCScript::const_iterator pc1 = txout.scriptPubKey.begin();
        if (txout.scriptPubKey.GetOp(pc1, opcode, vch) == false)
            continue;
        if (opcode == OP_MINE_BRANCH_MORTGAGE) {
            //script的其他部分判断
            nAmount += txout.nValue;
        }
        if (bWithBranchOut && opcode == OP_TRANS_BRANCH) {
            if (txout.scriptPubKey.GetOp(pc1, opcode, vch) && vch.size() == sizeof(uint256)) {
                uint256 branchhash(vch);
                if (branchhash.ToString() == tx.sendToBranchid) {
                    nAmount += txout.nValue;
                }
            }
        }
    }
    return nAmount;
}

//获取交易中的挖矿币输出
MCAmount GetMortgageCoinOut(const MCTransaction& tx, bool bWithBranchOut)
{
    MCAmount nAmount(0);
    std::vector<unsigned char> vch;
    for (MCTxOut txout : tx.vout) {
        opcodetype opcode;
        MCScript::const_iterator pc1 = txout.scriptPubKey.begin();
        txout.scriptPubKey.GetOp(pc1, opcode, vch);
        if (opcode == OP_MINE_BRANCH_COIN) {
            //script的其他部分判断
            nAmount += txout.nValue;
        }
        if (bWithBranchOut && opcode == OP_RETURN) {
            txout.scriptPubKey.GetOp(pc1, opcode, vch);
            if (opcode == OP_TRANS_BRANCH) {
                nAmount += txout.nValue;
            }
        }
    }
    return nAmount;
}

branch_script_type QuickGetBranchScriptType(const MCScript& scriptPubKey)
{
    opcodetype opcode;
    MCScript::const_iterator pc1 = scriptPubKey.begin();
    if (!scriptPubKey.GetOp(pc1, opcode))
        return BST_INVALID;
    if (opcode == OP_MINE_BRANCH_MORTGAGE) {
        return BST_MORTGAGE_MINE;
    }
    if (opcode == OP_MINE_BRANCH_COIN) {
        return BST_MORTGAGE_COIN;
    }

    return BST_INVALID;
}

// 获取抵押币脚本中的数据
//1. scriptPubKey (in)
//2. pBranchHash (out)
//3. pKeyID (out) the pubkey hash
bool GetMortgageMineData(const MCScript& scriptPubKey, uint256* pBranchHash /*= nullptr*/, MCKeyID* pKeyID /*= nullptr*/, int64_t* pnHeight)
{
    opcodetype opcode;
    std::vector<unsigned char> vch;
    MCScript::const_iterator pc1 = scriptPubKey.begin();
    if (scriptPubKey.GetOp(pc1, opcode, vch) == false || opcode != OP_MINE_BRANCH_MORTGAGE)
        return false;

    if (!scriptPubKey.GetOp(pc1, opcode, vch) || vch.size() != sizeof(uint256)) //branch hash256
        return false;
    if (pBranchHash)
        *pBranchHash = uint256(vch);

    if (!scriptPubKey.GetOp(pc1, opcode, vch)) //OP_BLOCK_HIGH
        return false;
    if (pnHeight) {
        try {
            *pnHeight = GetScriptInt64(opcode, vch);
        } catch (const scriptnum_error& err) {
            LogPrintf("%s:%d exception: %s\n", __FILE__, __LINE__, err.what());
            return false;
        }
    }

    //OP_2DROP OP_DUP OP_HASH160
    if (!scriptPubKey.GetOp(pc1, opcode, vch) && opcode != OP_2DROP || !scriptPubKey.GetOp(pc1, opcode, vch) && opcode != OP_DUP || !scriptPubKey.GetOp(pc1, opcode, vch) && opcode != OP_HASH160)
        return false;

    if (!scriptPubKey.GetOp(pc1, opcode, vch))
        return false;
    if (pKeyID)
        *pKeyID = uint160(vch);

    return true;
}

// 获取挖矿币脚本中的数据
//1. scriptPubKey (in)
//2. pFromTxid (out)
//3. pKeyID (out) the pubkey hash
bool GetMortgageCoinData(const MCScript& scriptPubKey, uint256* pFromTxid /*= nullptr*/, MCKeyID* pKeyID /*= nullptr*/, int64_t* pnHeight)
{
    opcodetype opcode;
    std::vector<unsigned char> vch;
    MCScript::const_iterator pc1 = scriptPubKey.begin();
    if (scriptPubKey.GetOp(pc1, opcode, vch) == false || opcode != OP_MINE_BRANCH_COIN)
        return false;

    if (!scriptPubKey.GetOp(pc1, opcode, vch) || vch.size() != sizeof(uint256)) //branch hash256
        return false;
    if (pFromTxid)
        *pFromTxid = uint256(vch);

    if (!scriptPubKey.GetOp(pc1, opcode, vch)) //OP_BLOCK_HIGH
        return false;
    if (pnHeight) {
        try {
            *pnHeight = GetScriptInt64(opcode, vch);
        }
        catch (const scriptnum_error& err) {
            LogPrintf("%s:%d exception: %s\n", __FILE__, __LINE__, err.what());
            return false;
        }
    }

    //OP_2DROP OP_DUP OP_HASH160
    if (!scriptPubKey.GetOp(pc1, opcode, vch) && opcode != OP_2DROP || !scriptPubKey.GetOp(pc1, opcode, vch) && opcode != OP_DUP || !scriptPubKey.GetOp(pc1, opcode, vch) && opcode != OP_HASH160)
        return false;

    if (!scriptPubKey.GetOp(pc1, opcode, vch))
        return false;
    if (pKeyID)
        *pKeyID = uint160(vch);

    return true;
}

bool GetRedeemSriptData(const MCScript& scriptPubKey, uint256* pFromTxid)
{
    opcodetype opcode;
    std::vector<unsigned char> vch;
    MCScript::const_iterator pc1 = scriptPubKey.begin();
    if (scriptPubKey.GetOp(pc1, opcode, vch) == false || opcode != OP_RETURN)
        return false;

    if (scriptPubKey.GetOp(pc1, opcode, vch) == false || opcode != OP_REDEEM_MORTGAGE)
        return false;

    if (!scriptPubKey.GetOp(pc1, opcode, vch) || vch.size() != sizeof(uint256)) //branch hash256
        return false;
    if (pFromTxid)
        *pFromTxid = uint256(vch);
    return true;
}

MCAmount GetBranchChainOut(const MCTransaction& tx)
{
    /*if (tx.IsBranchCreate()){
        return GetBranchChainCreateTxOut(tx);
    }
    else */
    if (tx.IsBranchChainTransStep1()) {
        return GetBranchChainTransOut(tx);
    } else if (tx.IsMortgage()) {
        return GetMortgageMineOut(tx, true);
    }
    return 0;
}

MCAmount GetContractAmountOut(const MCTransaction& tx)
{
    MCAmount amount = 0;
    std::vector<unsigned char> vch;
    for (int i = 0; i < tx.vout.size(); ++i) {
        MCTxOut txout = tx.vout[i];

        opcodetype opcode;
        MCScript::const_iterator pc1 = txout.scriptPubKey.begin();
        if (!txout.scriptPubKey.GetOp(pc1, opcode, vch))
            continue;

        if (opcode == OP_CONTRACT)
            amount += txout.nValue;
    }
    return amount;
}

//copy from merkleblock.cpp
MCSpvProof* NewSpvProof(const MCBlock& block, const std::set<uint256>& txids)
{
    std::vector<bool> vMatch;
    std::vector<uint256> vHashes;

    vMatch.reserve(block.vtx.size());
    vHashes.reserve(block.vtx.size());

    for (unsigned int i = 0; i < block.vtx.size(); i++) {
        const uint256& hash = block.vtx[i]->GetHash();
        if (txids.count(hash))
            vMatch.push_back(true);
        else
            vMatch.push_back(false);
        vHashes.push_back(hash);
    }

    return new MCSpvProof(vHashes, vMatch, block.GetHash());
}

int CheckSpvProof(const uint256& merkleRoot, MCPartialMerkleTree& pmt, const uint256& querytxhash)
{
    std::vector<uint256> vMatch;
    std::vector<unsigned int> vIndex;
    if (pmt.ExtractMatches(vMatch, vIndex) != merkleRoot)
        return -1;
    if (std::find(vMatch.begin(), vMatch.end(), querytxhash) == vMatch.end())
        return -1;
    if (vIndex.size() > 1)
        return -1;
    return vIndex[0];
}

// 跨链交易从发起链广播到目标链
bool BranchChainTransStep2(const MCTransactionRef& tx, const MCBlock& block, std::string* pStrErrorMsg)
{
    if (!tx->IsPregnantTx()) {
        return error_ex1(pStrErrorMsg, "%s: tx no a branch chain transaction", __func__);
    }

    //broadcast to target chain.
    const std::string strToChainId = tx->sendToBranchid;
    if (strToChainId == Params().GetBranchId())
        return error_ex1(pStrErrorMsg, "%s: can not to this chain!", __func__);

    MCRPCConfig chainrpccfg;
    if (g_branchChainMan->GetRpcConfig(strToChainId, chainrpccfg) == false || chainrpccfg.IsValid() == false) {
        return error_ex1(pStrErrorMsg, "%s: can not found branch rpc config for %s\n", __func__, strToChainId);
    }

    std::string strTxHexData;
    if (strToChainId == MCBaseChainParams::MAIN && tx->IsBranchChainTransStep1()) {
        //添加 部分默克尔树(spv证明)
        std::set<uint256> txids;
        txids.emplace(tx->GetHash());

        MCMutableTransaction mtx(*tx);
        mtx.pPMT.reset(NewSpvProof(block, txids));

        MCTransactionRef sendtx = MakeTransactionRef(mtx);
        strTxHexData = EncodeHexTx(*sendtx, RPCSerializationFlags());
    } else {
        strTxHexData = EncodeHexTx(*tx, RPCSerializationFlags());
    }

    // rpc to branch chain to create an branchtranstraction.
    std::string strMethod = "makebranchtransaction";
    UniValue params(UniValue::VARR);
    params.push_back(strTxHexData);

    UniValue reply = CallRPC(chainrpccfg, strMethod, params);

    const UniValue& result = find_value(reply, "result");
    const UniValue& errorVal = find_value(reply, "error");
    if (!errorVal.isNull()) {
        //throw JSONRPCError(RPC_WALLET_ERROR, strError);
        return error_ex1(pStrErrorMsg, "%s: RPC call makebranchtransaction fail: %s, txid %s\n", __func__, errorVal.write(), tx->GetHash().GetHex());
    }

    if (result.isNull() || result.get_str() != "ok") {
        return error_ex1(pStrErrorMsg, "%s RPC call not return ok", __func__);
    }
    return true;
}

//OP:移动独立的线程中去?或者满足高度后,相应的拥有者自己调用相关逻辑,但是这样跨链转账变得更麻烦
void ProcessBlockBranchChain()
{
    {
        uint32_t nBlockHeight = BRANCH_CHAIN_MATURITY + CUSHION_HEIGHT;
        MCBlockIndex* pbi = chainActive[chainActive.Tip()->nHeight - nBlockHeight];
        if (pbi != nullptr) {
            std::shared_ptr<MCBlock> pblock = std::make_shared<MCBlock>();
            MCBlock& block = *pblock;
            if (ReadBlockFromDisk(block, pbi, Params().GetConsensus())) {
                for (int i = 1; i < block.vtx.size(); i++) {
                    const MCTransactionRef& tx = block.vtx[i];
                    if (tx->IsBranchChainTransStep1() || tx->IsMortgage()) {
                        BranchChainTransStep2(tx, block, nullptr);
                    }
                    if (tx->IsRedeemMortgageStatement()) {
                        ReqMainChainRedeemMortgage(tx, block);
                    }
                }
            }
        }
    }
}

//chain transaction step 2 Check.
/*
 @param txBranchChainStep2
 @param state
 @param fVerifingDB 
 @param pFromTx The source transaction(step 1 tx) of txBranchChainStep2
*/
bool CheckBranchTransaction(const MCTransaction& txBranchChainStep2, MCValidationState& state, const bool fVerifingDB, const MCTransactionRef& pFromTx)
{
    if (txBranchChainStep2.IsBranchChainTransStep2() == false)
        return state.DoS(100, false, REJECT_INVALID, "is not a IsBranchChainTransStep2");

    const std::string& fromBranchId = txBranchChainStep2.fromBranchId;
    std::string fromTxHash = pFromTx->GetHash().ToString();
    if (fromBranchId == Params().GetBranchId()) {
        std::string strErr = strprintf("%s ctFromChain eq ctToChain", __func__);
        return state.DoS(100, false, REJECT_INVALID, strErr);
    }

    //-----------------------------------------------
    //检查fromtx
    const MCTransaction& txTrans1 = *pFromTx;
    if (txTrans1.IsMortgage()) { //
        MCKeyID keyid1;
        int64_t height1;
        if (!GetMortgageMineData(txTrans1.vout[0].scriptPubKey, nullptr, &keyid1, &height1)) {
            return state.DoS(100, false, REJECT_INVALID, "invalid mortgage mine script");
        }
        MCKeyID keyid2;
        int64_t height2;
        if (txBranchChainStep2.vout.size() != 1 || !GetMortgageCoinData(txBranchChainStep2.vout[0].scriptPubKey, nullptr, &keyid2, &height2)) {
            return error("%s invalid mortgage transaction,", __func__);
        }
        if (keyid1 != keyid2 || height1 != height2) {
            return state.DoS(100, false, REJECT_INVALID, "invalid mortgage coin script");
        }
    }

    MCMutableTransaction mtxTrans2;
    if (!DecodeHexTx(mtxTrans2, txTrans1.sendToTxHexData)) {
        return error("%s sendToTxHexData is not a valid transaction data.\n", __func__);
    }

    MCMutableTransaction mtxTrans2my = RevertTransaction(txBranchChainStep2, pFromTx, false);
    
    //Revert other fields exclude in txTrans1
    mtxTrans2my.fromTx.clear();
    if (txTrans1.IsMortgage()) {
        mtxTrans2my.vout[0].scriptPubKey.clear();
    }
    if (mtxTrans2my.fromBranchId != MCBaseChainParams::MAIN) {
        mtxTrans2my.pPMT.reset(new MCSpvProof());
    }
    
    if (mtxTrans2.GetHash() != mtxTrans2my.GetHash()) {
        std::string strErr = strprintf("%s transaction hash error\n", __func__);
        return state.DoS(100, false, REJECT_INVALID, strErr);
    }

    MCAmount nAmount = GetBranchChainOut(txTrans1);
    if (nAmount != txBranchChainStep2.inAmount || MoneyRange(txBranchChainStep2.inAmount) == false) {
        std::string strErr = strprintf(" %s Invalid inAmount!\n", __func__);
        return state.DoS(100, false, REJECT_INVALID, strErr);
    }
    //
    MCAmount nOrginalOut = txBranchChainStep2.GetValueOut();
    if (txBranchChainStep2.fromBranchId != MCBaseChainParams::MAIN) {
        nOrginalOut = 0; // recalc exclude branch tran recharge
        for (const auto& txout : txBranchChainStep2.vout) {
            if (!IsCoinBranchTranScript(txout.scriptPubKey)) {
                nOrginalOut += txout.nValue;
            }
        }
    }
    if (nOrginalOut > txBranchChainStep2.inAmount) {
        std::string strErr = strprintf("%s GetValueOut larger than inAmount\n", __func__);
        return state.DoS(100, false, REJECT_INVALID, strErr);
    }

    //-----------------------------------------------
    //rpc 侧链核对信息
    if (fVerifingDB && gArgs.GetBoolArg("-uncheckbranchtxinverifydb", true)) {
        LogPrintf("\nJump check branch tx in Verifing DB. From tx txid %s\n", fromTxHash);
        return true;
    }

    MCRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig(fromBranchId, branchrpccfg) == false || branchrpccfg.IsValid() == false) {
        if (Params().IsMainChain() && gArgs.GetBoolArg("-unchecknoconfigbranch", false))
            return true;

        std::string strErr = strprintf(" %s can not found branch rpc config for %s\n", __func__, fromBranchId);
        return state.DoS(1, false, REJECT_INVALID, strErr);
    }

    const std::string strMethod = "getbranchchaintransaction";
    UniValue params(UniValue::VARR);
    params.push_back(fromTxHash);

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);

    const UniValue& result = find_value(reply, "result");
    const UniValue& errorVal = find_value(reply, "error");
    if (!errorVal.isNull()) {
        return error(" %s RPC call getbranchchaintransaction fail: %s, txid %s\n", __func__, errorVal.write(), txBranchChainStep2.GetHash().GetHex());
    }
    if (result.isNull()) {
        return error(" %s RPC call getbranchchaintransaction fail: result null\n", __func__);
    }

    //const UniValue& txid = find_value(result, "txid");
    const UniValue& txhex = find_value(result, "hex");
    const UniValue& confirmations = find_value(result, "confirmations");
    if (txhex.isStr() == false) {
        std::string strErr = strprintf(" %s RPC call getbranchchaintransaction tx hex invalid.\n", __func__);
        return state.DoS(100, false, REJECT_INVALID, strErr);
    }

    MCMutableTransaction mtxTrans1;
    if (!DecodeHexTx(mtxTrans1, txhex.get_str())) {
        std::string strErr = strprintf(" %s RPC call getbranchchaintransaction DecodeHexTx tx hex fail.\n", __func__);
        return state.DoS(100, false, REJECT_INVALID, strErr);
    }

    if (mtxTrans1.GetHash().ToString() != fromTxHash) {
        std::string strErr = strprintf(" %s return transaction is not the one that i wanted.\n", __func__);
        return state.DoS(100, false, REJECT_INVALID, strErr);
    }

    const uint32_t maturity = BRANCH_CHAIN_MATURITY;
    if (!confirmations.isNum() || confirmations.get_int() < maturity + 1) {
        return error(" %s RPC confirmations not satisfy.\n", __func__);
    }
    return true;
}

#define SetStrErr(strMsg)                 \
    {                                     \
        if (pStrErr) *pStrErr = (strMsg); \
    }
//提交侧链区块头
//call in branch chain
bool SendBranchBlockHeader(const std::shared_ptr<const MCBlock> pBlock, std::string* pStrErr, bool onlySendMy)
{
    SetStrErr("Unknow error\n");
    if (Params().IsMainChain() || pBlock == nullptr) {
        SetStrErr("Can not called in main chain or pPlock is null\n");
        return false;
    }

    MCBlockIndex* pBlockIndex = nullptr;
    if (mapBlockIndex.count(pBlock->GetHash())) {
        pBlockIndex = mapBlockIndex[pBlock->GetHash()];
    }
    if (pBlockIndex == nullptr) {
        SetStrErr("get block index fail\n");
        return false;
    }

    if (onlySendMy) {
        //get block pubkey
        std::vector<unsigned char> vchPubKey;
        MCScript::const_iterator pc = pBlock->vchBlockSig.begin();
        opcodetype opcode;
        if (!pBlock->vchBlockSig.GetOp2(pc, opcode, &vchPubKey)) {
            SetStrErr("get block block signature pubkey fail\n");
            return false;
        }

        MCPubKey pubKey(vchPubKey);
        MCKeyID keyid = pubKey.GetID();
        extern bool IsMineForAllWallets(const MCKeyID& keyid);
        if (!IsMineForAllWallets(keyid)) {
            SetStrErr("I don't care others block's info\n");
            return false;
        }
    }

    MCMutableTransaction mtx;
    mtx.nVersion = MCTransaction::SYNC_BRANCH_INFO;
    MCBranchBlockInfo* pBlockInfo = new MCBranchBlockInfo;
    mtx.pBranchBlockData.reset(pBlockInfo);

    //header info
    pBlockInfo->SetBlockHeader(*pBlock);

    //other info
    pBlockInfo->blockHeight = pBlockIndex->nHeight;
    pBlockInfo->branchID.SetHex(Params().GetBranchId());
    if (pBlock->vtx.size() < 2) {
        SetStrErr("block vtx size error\n");
        return false;
    }
    MCVectorWriter cvw{SER_NETWORK, INIT_PROTO_VERSION, pBlockInfo->vchStakeTxData, 0, pBlock->vtx[1]};

    //call rpc
    MCRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig(MCBaseChainParams::MAIN, branchrpccfg) == false || branchrpccfg.IsValid() == false) {
        SetStrErr("can not found main chain rpc connnect info\n");
        return false;
    }

    const std::string strMethod = "submitbranchblockinfo";
    UniValue params(UniValue::VARR);
    MCTransactionRef tx = MakeTransactionRef(std::move(mtx));
    params.push_back(EncodeHexTx(*tx, RPCSerializationFlags()));

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);

    const UniValue& result = find_value(reply, "result");
    const UniValue& errorVal = find_value(reply, "error");
    if (!errorVal.isNull()) {
        SetStrErr(errorVal.write());
        return false;
    }
    if (result.isNull()) {
        SetStrErr("SendBranchBlockHeader rpc result is null.\n");
        return false;
    }

    if (result.isObject()) {
        const UniValue& commitreject = find_value(result, "commit_reject_reason");
        if (!commitreject.isNull()) {
            SetStrErr(commitreject.get_str());
            return false;
        }
    }

    SetStrErr("");
    return true;
}

extern bool CheckBlockHeaderWork(const MCBranchBlockInfo& block, MCValidationState& state, const MCChainParams& params, BranchData& branchdata, BranchCache* pBranchCache, MCCoinsViewCache* pCoins);
extern bool BranchContextualCheckBlockHeader(const MCBlockHeader& block, MCValidationState& state, const MCChainParams& params, BranchData& branchdata, int64_t nAdjustedTime, BranchCache* pBranchCache, int* pNMissingInputs);

bool CheckBranchBlockInfoTx(const MCTransaction& tx, MCValidationState& state, BranchCache* pBranchCache, MCCoinsViewCache* pCoins, int* pNMissingInputs)
{
    if (!tx.IsSyncBranchInfo()) {
        return state.DoS(100, false, REJECT_INVALID, "Sync branch info fail");
    }

    MCBlockHeader blockheader;
    tx.pBranchBlockData->GetBlockHeader(blockheader);

    if (!g_pBranchChainTxRecordsDb->IsBranchCreated(tx.pBranchBlockData->branchID)) {
        return state.DoS(100, false, REJECT_INVALID, "Branch chain has not created");
    }

    //block signature check
    if (blockheader.prevoutStake.IsNull() || blockheader.vchBlockSig.size() == 0) {
        return state.DoS(100, false, REJECT_INVALID, "Submit branch chain block header must contain prevoutStake and vchBlockSig");
    }
    if (!CheckBlockHeaderSignature(blockheader)) {
        return state.DoS(100, false, REJECT_INVALID, "Submit branch chain block header sig check fail");
    }

    if (pBranchCache && pBranchCache->HasInCache(tx)) {
        return state.DoS(100, false, REJECT_DUPLICATE, "branch block info duplicate");
    }

    BranchData branchdata = pBranchCache->GetBranchData(tx.pBranchBlockData->branchID);
    //ContextualCheckBlockHeader
    const MCChainParams& bparams = BranchParams(tx.pBranchBlockData->branchID);
    if (!BranchContextualCheckBlockHeader(blockheader, state, bparams, branchdata, GetAdjustedTime(), pBranchCache, pNMissingInputs)) {
        return false;
    }

    //检查工作量
    if (!CheckBlockHeaderWork(*(tx.pBranchBlockData), state, bparams, branchdata, pBranchCache, pCoins)) {
        return false;
    }

    return true;
}

// 如果是自己的交易则,向自己的主链发起赎回请求,把抵押币解锁
bool ReqMainChainRedeemMortgage(const MCTransactionRef& tx, const MCBlock& block, std::string* pStrErr)
{
    SetStrErr("Unknow error");
    if (tx->IsRedeemMortgageStatement() == false) {
        SetStrErr("Is not a redeem mortgage transaction");
        return false;
    }

    uint256 coinfromtxid;
    for (const auto& tx_out : tx->vout) {
        if (GetRedeemSriptData(tx_out.scriptPubKey, &coinfromtxid)) {
            break;
        }
    }

    std::set<uint256> txids;
    txids.emplace(tx->GetHash());
    std::shared_ptr<MCSpvProof> spvProof(NewSpvProof(block, txids));

    const std::string strMethod = "redeemmortgagecoin";
    UniValue params(UniValue::VARR);
    params.push_back(coinfromtxid.ToString());
    params.push_back(UniValue(int(0)));
    params.push_back(EncodeHexTx(*tx));
    params.push_back(Params().GetBranchId());
    params.push_back(EncodeHexSpvProof(*spvProof));

    //call rpc
    MCRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig(MCBaseChainParams::MAIN, branchrpccfg) == false || branchrpccfg.IsValid() == false) {
        SetStrErr("Can not found main chain rpc connnect config");
        return false;
    }

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);

    const UniValue& result = find_value(reply, "result");
    const UniValue& errorVal = find_value(reply, "error");
    if (!errorVal.isNull()) {
        SetStrErr(errorVal.write());
        return false;
    }
    if (result.isNull()) {
        SetStrErr("ReqMainChainRedeemMortgage rpc result is NULL");
        return false;
    }

    SetStrErr("");
    return true;
}

//GetReportTxHashKey 和 GetProveTxHashKey 需要计算出同样的值
uint256 GetReportTxHashKey(const MCTransaction& tx)
{
    if (!tx.IsReport()) {
        return uint256();
    }

    int nType = SER_GETHASH;
    int nVersion = PROTOCOL_VERSION;
    MCHashWriter ss(nType, nVersion);
    ss << tx.pReportData->reporttype;
    if (tx.pReportData->reporttype == ReportType::REPORT_TX || tx.pReportData->reporttype == ReportType::REPORT_COINBASE || tx.pReportData->reporttype == ReportType::REPORT_MERKLETREE || tx.pReportData->reporttype == ReportType::REPORT_CONTRACT_DATA) {
        ss << tx.pReportData->reportedBranchId;
        ss << tx.pReportData->reportedBlockHash;
        ss << tx.pReportData->reportedTxHash;
    }
    return ss.GetHash();
}
uint256 GetProveTxHashKey(const MCTransaction& tx)
{
    int nType = SER_GETHASH;
    int nVersion = PROTOCOL_VERSION;
    MCHashWriter ss(nType, nVersion);
    ss << tx.pProveData->provetype;
    if (tx.pProveData->provetype == ReportType::REPORT_TX || tx.pProveData->provetype == ReportType::REPORT_COINBASE || tx.pProveData->provetype == ReportType::REPORT_MERKLETREE) {
        ss << tx.pProveData->branchId;
        ss << tx.pProveData->blockHash;
        ss << tx.pProveData->txHash;
    }
    return ss.GetHash();
}

// 调用地方 主要还是follow CheckInputs : 1.accepttomempool 2.connectblock
bool CheckBranchDuplicateTx(const MCTransaction& tx, MCValidationState& state, BranchChainTxRecordsCache* pBranchTxRecordCache, BranchCache* pBranchCache)
{
    if (tx.IsSyncBranchInfo()) {
        if (pBranchCache && pBranchCache->HasInCache(tx))
            return state.DoS(0, false, REJECT_DUPLICATE, "branch block info duplicate");

        BranchData branchdata = g_pBranchDb->GetBranchData(tx.pBranchBlockData->branchID);
        MCBlockHeader blockheader;
        tx.pBranchBlockData->GetBlockHeader(blockheader);
        if (branchdata.mapHeads.count(blockheader.GetHash())) {
            return state.DoS(0, false, REJECT_DUPLICATE, "blockheader info has include before"); //防止重复
        }
    }

    if (tx.IsBranchChainTransStep2()) {
        // check in cache pBranchCache
        if (pBranchTxRecordCache && pBranchTxRecordCache->HasInCache(tx)) {
            return state.DoS(0, false, REJECT_DUPLICATE, "branchchaintransstep2 tx duplicate");
        }
        if (g_pBranchChainTxRecordsDb->IsTxRecvRepeat(tx, nullptr)) {
            uint256 oritxid = mempool.GetOriTxHash(tx, false);
            return state.Invalid(false, REJECT_DUPLICATE, strprintf("txn-already-in-records ori txid %s, %s", oritxid.GetHex(), tx.GetHash().GetHex()));
        }
    }

    if (tx.IsReport()) {
        uint256 reportFlagHash = GetReportTxHashKey(tx);
        const uint256& rpBranchId = tx.pReportData->reportedBranchId;
        const uint256& rpBlockId = tx.pReportData->reportedBlockHash;
        if (pBranchCache && pBranchCache->mReortTxFlagCache.count(reportFlagHash)) {
            return state.DoS(0, false, REJECT_DUPLICATE, "duplicate report in cache");
        }
        if (g_pBranchDb->GetTxReportState(rpBranchId, rpBlockId, reportFlagHash) != RP_INVALID) {
            return state.DoS(0, false, REJECT_DUPLICATE, "duplicate report in db");
        }
    }

    if (tx.IsProve()) {
        uint256 proveFlagHash = GetProveTxHashKey(tx);
        const uint256& rpBranchId = tx.pProveData->branchId;
        const uint256& rpBlockId = tx.pProveData->blockHash;
        if (pBranchCache && pBranchCache->mReortTxFlagCache.count(proveFlagHash) && pBranchCache->mReortTxFlagCache[proveFlagHash] == RP_FLAG_PROVED) {
            return state.DoS(0, false, REJECT_DUPLICATE, "duplicate prove in cache");
        }
        if (g_pBranchDb->GetTxReportState(rpBranchId, rpBlockId, proveFlagHash) == RP_FLAG_PROVED) {
            return state.DoS(0, false, REJECT_DUPLICATE, "duplicate prove in db");
        }
    }
    return true;
}

bool CheckReportTxCommonly(const MCTransaction& tx, MCValidationState& state, BranchData& branchdata)
{
    BranchBlockData* pBlockData = branchdata.GetBranchBlockData(tx.pReportData->reportedBlockHash);
    if (pBlockData == nullptr)
        return state.DoS(0, false, REJECT_INVALID, "CheckReportCheatTx Can not found block data in mapHeads");
    if (branchdata.Height() < pBlockData->nHeight)
        return state.DoS(0, false, REJECT_INVALID, strprintf("Report block height larger than branchdata height, chainheight %d, blockheight %d", branchdata.Height(), pBlockData->nHeight));
    if (branchdata.Height() - pBlockData->nHeight > REDEEM_SAFE_HEIGHT)
        return state.DoS(0, false, REJECT_INVALID, strprintf("Report block too old, chainheight %d, blockheight %d", branchdata.Height(), pBlockData->nHeight));
    return true;
}

bool CheckReportCheatTx(const MCTransaction& tx, MCValidationState& state, BranchCache* pBranchCache)
{
    if (tx.IsReport()) {
        const uint256 reportedBranchId = tx.pReportData->reportedBranchId;
        if (!pBranchCache->HasBranchData(reportedBranchId))
            return state.DoS(100, false, REJECT_INVALID, "CheckReportCheatTx branchid error");
        BranchData branchdata = pBranchCache->GetBranchData(reportedBranchId);

        if (tx.pReportData->reporttype == ReportType::REPORT_TX || tx.pReportData->reporttype == ReportType::REPORT_COINBASE) {
            MCSpvProof spvProof(*tx.pPMT);
            BranchBlockData* pBlockData = branchdata.GetBranchBlockData(spvProof.blockhash);
            if (pBlockData == nullptr)
                return state.DoS(100, false, REJECT_INVALID, "pBlockData == nullptr");
            if (CheckSpvProof(pBlockData->header.hashMerkleRoot, spvProof.pmt, tx.pReportData->reportedTxHash) < 0)
                return state.DoS(100, false, REJECT_INVALID, "CheckSpvProof fail");
            ;
            if (!CheckReportTxCommonly(tx, state, branchdata))
                return state.DoS(100, false, REJECT_INVALID, "CheckReportTxCommonly fail");
            ;
        } else if (tx.pReportData->reporttype == ReportType::REPORT_MERKLETREE) {
            if (!CheckReportTxCommonly(tx, state, branchdata))
                return state.DoS(100, false, REJECT_INVALID, "CheckProveContractData fail");
            ;
        } else if (tx.pReportData->reporttype == ReportType::REPORT_CONTRACT_DATA) {
            if (!CheckProveContractData(tx, state, pBranchCache))
                return false;
        } else
            return state.DoS(100, false, REJECT_INVALID, "Invalid report type!");
    }
    return true;
}

bool CheckTransactionProveWithProveData(const MCTransactionRef& pProveTx, MCValidationState& state, const std::vector<ProveDataItem>& vectProveData, BranchData& branchData, MCAmount& fee, bool jumpFrist)
{
    if (pProveTx->IsCoinBase()) {
        return state.DoS(0, false, REJECT_INVALID, "CheckProveReportTx Prove tx can not a coinbase transaction");
    }

    int baseIndex = jumpFrist ? 1 : 0;
    if (vectProveData.size() != pProveTx->vin.size() + baseIndex) {
        return state.DoS(0, false, REJECT_INVALID, "vectProveData size invalid for prove each input");
    }

    MCAmount nInAmount = 0;
    std::map<MCContractID, MCAmount> contractCoinsOut;
    MCScript contractScript = GetScriptForDestination(pProveTx->pContractData->address);
    for (size_t i = 0; i < pProveTx->vin.size(); ++i) {
        const ProveDataItem& provDataItem = vectProveData[i + baseIndex];
        if (branchData.mapHeads.count(provDataItem.blockHash) == 0)
            return state.DoS(0, false, REJECT_INVALID, "proveitem's block not exist");

        MCTransactionRef pTx;
        MCDataStream cds(provDataItem.tx, SER_NETWORK, INIT_PROTO_VERSION);
        cds >> (pTx);

        MCSpvProof spvProof(provDataItem.pCSP);
        BranchBlockData* pBlockData = branchData.GetBranchBlockData(spvProof.blockhash);
        if (pBlockData == nullptr)
            return state.DoS(0, false, REJECT_INVALID, "pBlockData == nullptr");
        if (CheckSpvProof(pBlockData->header.hashMerkleRoot, spvProof.pmt, pTx->GetHash()) < 0)
            return state.DoS(0, false, REJECT_INVALID, "Check Prove ReportTx spv check fail");

        const MCOutPoint& outpoint = pProveTx->vin[i].prevout;
        if (pTx->GetHash() != outpoint.hash)
            return state.DoS(0, false, REJECT_INVALID, "Check Prove ReportTx provide tx not match");

        if (outpoint.n >= pTx->vout.size())
            return state.DoS(0, false, REJECT_INVALID, "Check Prove ReportTx ");

        //check sign
        const MCScript& scriptPubKey = pTx->vout[outpoint.n].scriptPubKey;
        const MCAmount amount = pTx->vout[outpoint.n].nValue;
        nInAmount += amount;

        if (scriptPubKey.IsContract()) {
            MCContractID contractId;
            scriptPubKey.GetContractAddr(contractId);
            contractCoinsOut[contractId] += amount;
        }

        bool fCacheResults = false;
        unsigned int flags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_DERSIG | SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY | SCRIPT_VERIFY_CHECKSEQUENCEVERIFY | SCRIPT_VERIFY_WITNESS | SCRIPT_VERIFY_NULLDUMMY;

        PrecomputedTransactionData txdata(*pProveTx);
        CScriptCheck check(scriptPubKey, amount, *pProveTx, i, flags, fCacheResults, &txdata);
        if (!check()) {
            bool checkok = true;
            if (pProveTx->IsCallContract()) { //智能合约转币不用签名的
                checkok = false;
                MCContractID kDestKey;
                if (!scriptPubKey.GetContractAddr(kDestKey)) {
                    return state.DoS(0, false, REJECT_NONSTANDARD, "check smartcontract sign fail, contract addr fail");
                }
                checkok = true;
            }
            if (!checkok)
                return state.DoS(0, false, REJECT_INVALID, "CheckProveReportTx scriptcheck fail");
        }
    }

    //check input >= output value
    MCAmount nValueOut = 0;
    std::map<MCContractID, MCAmount> contractChange;
    for (const auto& txout : pProveTx->vout) {
        if (txout.nValue < 0)
            return state.DoS(100, false, REJECT_INVALID, "CheckProveReportTx bad-txns-vout-negative");
        if (txout.nValue > MAX_MONEY)
            return state.DoS(100, false, REJECT_INVALID, "CheckProveReportTx bad-txns-vout-toolarge");
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return state.DoS(100, false, REJECT_INVALID, "CheckProveReportTx bad-txns-txouttotal-toolarge");

        if (txout.scriptPubKey.IsContractChange()) {
            MCContractID contractId;
            if (!txout.scriptPubKey.GetContractAddr(contractId)) {
                return state.DoS(0, false, REJECT_INVALID, "Invalid contract out public key");
            }
            if (contractChange.find(contractId) != contractChange.end()) {
                return state.DoS(0, false, REJECT_INVALID, "Dumplicate contract change");
            }
            contractChange[contractId] = txout.nValue;
        }
    }
    if (pProveTx->IsSmartContract()) {
        for (auto it : pProveTx->pContractData->contractCoinsOut) {
            if (contractCoinsOut[it.first] - contractChange[it.first] != it.second) {
                return state.DoS(0, false, REJECT_INVALID, "contract coins out not match");
            }
            contractCoinsOut.erase(it.first);
            contractChange.erase(it.first);
        }

        if (contractCoinsOut.size() != 0 || contractChange.size() != 0) {
            return state.DoS(100, false, REJECT_INVALID, "contract coins out amount not match");
        }
    }

    if (!MoneyRange(nValueOut))
        return state.DoS(100, false, REJECT_INVALID, "CheckProveReportTx bad-txns-txouttotal-toolarge");
    if (nInAmount < nValueOut) {
        return state.DoS(100, false, REJECT_INVALID, "value in/out error");
    }

    fee = nInAmount - nValueOut;
    return true;
}

int CheckProveSmartContract(const std::shared_ptr<const ProveData> pProveData, const MCTransactionRef proveTx, const BranchBlockData* pBlockData, const BranchBlockData* pPrevBlockData)
{
    ContractPrevData prevData;
    for (auto item : pProveData->contractData->contractPrevData) {
        prevData.items[item.first].blockHash = item.second.blockHash;
        prevData.items[item.first].txIndex = item.second.txIndex;
    }
    prevData.coins = pProveData->contractData->coins;

    uint256 hashWithPrevData = GetTxHashWithPrevData(proveTx->GetHash(), prevData);
    int txIndex = CheckSpvProof(pBlockData->header.hashMerkleRootWithPrevData, pProveData->contractData->prevDataSPV, hashWithPrevData);
    if (txIndex < 0) {
        return -1;
    }

    ContractContext contractContext;
    for (auto item : pProveData->contractData->contractPrevData) {
        contractContext.data[item.first] = std::move(item.second);
    }

    SmartLuaState sls;
    contractContext.txFinalData.resize(txIndex + 1);
    if (!ExecuteContract(&sls, proveTx, txIndex, prevData.coins, pPrevBlockData->header.GetBlockTime(), pBlockData->nHeight, nullptr, &contractContext)) {
        return -1;
    }

    uint256 hashWithData = GetTxHashWithData(proveTx->GetHash(), contractContext.txFinalData[txIndex]);
    int txIndexFinal = CheckSpvProof(pBlockData->header.hashMerkleRootWithData, pProveData->contractData->dataSPV, hashWithData);
    if (txIndexFinal < 0) {
        return -1;
    }

    if (txIndex != txIndexFinal) {
        return -1;
    }

    return txIndex;
}

bool CheckProveReportTx(const MCTransaction& tx, MCValidationState& state, BranchCache* pBranchCache)
{
    if (!tx.IsProve() || tx.pProveData == nullptr || tx.pProveData->provetype != ReportType::REPORT_TX) {
        return state.DoS(0, false, REJECT_INVALID, "CheckProveReportTx param fail");
    }

    const uint256 branchId = tx.pProveData->branchId;
    if (!pBranchCache->HasBranchData(branchId)) {
        return state.DoS(0, false, REJECT_INVALID, "Branch data missing");
    }

    const std::vector<ProveDataItem>& vectProveData = tx.pProveData->vectProveData;
    if (vectProveData.size() < 1) {
        return state.DoS(0, false, REJECT_INVALID, "vectProveData size invalid can not zero");
    }

    // unserialize prove tx
    MCTransactionRef pProveTx;
    MCDataStream cds(vectProveData[0].tx, SER_NETWORK, INIT_PROTO_VERSION);
    cds >> (pProveTx);

    //check txid
    if (pProveTx->GetHash() != tx.pProveData->txHash) {
        return state.DoS(0, false, REJECT_INVALID, "Prove tx data error, first tx's hasdid is not eq proved txid");
    }

    // spv check
    BranchData branchData = pBranchCache->GetBranchData(branchId);
    MCSpvProof spvProof(vectProveData[0].pCSP);
    BranchBlockData* pBlockData = branchData.GetBranchBlockData(spvProof.blockhash);
    if (pBlockData == nullptr) {
        return state.DoS(0, false, REJECT_INVALID, "pBlockData == nullptr");
    }
    int txIndex = CheckSpvProof(pBlockData->header.hashMerkleRoot, spvProof.pmt, pProveTx->GetHash());
    if (txIndex < 0) {
        return state.DoS(0, false, REJECT_INVALID, "Check Prove ReportTx spv check fail");
    }

    //check input/output/sign
    MCAmount fee;
    if (!CheckTransactionProveWithProveData(pProveTx, state, vectProveData, branchData, fee, true)) {
        return false;
    }

    if (pProveTx->IsSmartContract()) {
        BranchBlockData* pPrevBlockData = branchData.GetBranchBlockData(pBlockData->header.hashPrevBlock);
        if (CheckProveSmartContract(tx.pProveData, pProveTx, pBlockData, pPrevBlockData) != txIndex) {
            return state.DoS(0, false, REJECT_INVALID, "CheckProveSmartContract fail");
        }
    }

    return true;
}

bool CheckProveCoinbaseTx(const MCTransaction& tx, MCValidationState& state, BranchCache* pBranchCache)
{
    if (!tx.IsProve() || tx.pProveData == nullptr || !(tx.pProveData->provetype == ReportType::REPORT_COINBASE || tx.pProveData->provetype == ReportType::REPORT_MERKLETREE)) {
        return state.DoS(0, false, REJECT_INVALID, "CheckProveCoinbaseTx param invalid");
    }

    const uint256& branchId = tx.pProveData->branchId;
    if (!pBranchCache->HasBranchData(branchId)) {
        return state.DoS(0, false, REJECT_INVALID, "prove coinbase tx no branchid data");
    }

    BranchData branchData = pBranchCache->GetBranchData(branchId);
    if (branchData.mapHeads.count(tx.pProveData->blockHash) == 0) {
        return state.DoS(0, false, REJECT_INVALID, "prove coinbase tx no block data");
    }
    BranchBlockData& branchblockdata = branchData.mapHeads[tx.pProveData->blockHash];

    std::vector<MCTransactionRef> vtx;
    MCDataStream cds(tx.pProveData->vtxData, SER_NETWORK, INIT_PROTO_VERSION);
    cds >> vtx;
    if (vtx.size() < 2) {
        return state.DoS(100, false, REJECT_INVALID, "invalid vtx size");
    }
    if (tx.pProveData->provetype == ReportType::REPORT_COINBASE && vtx[0]->GetHash() != tx.pProveData->txHash) {
        return state.DoS(100, false, REJECT_INVALID, "coinbase tx is eq txHash");
    }
    if (tx.pProveData->provetype == ReportType::REPORT_MERKLETREE && !tx.pProveData->txHash.IsNull()) {
        return state.DoS(100, false, REJECT_INVALID, "merkle poof txhash is invalid,must null");
    }

    //prove merkle tree root
    bool mutated;
    uint256 hashMerkleRoot2 = VecTxMerkleRoot(vtx, &mutated);
    if (branchblockdata.header.hashMerkleRoot != hashMerkleRoot2) {
        return state.DoS(100, false, REJECT_INVALID, "Invalid merkle tree for vtx");
    }
    if (mutated)
        return state.DoS(100, false, REJECT_INVALID, "duplicate transaction in vtx");

    // size valid
    if (vtx.size() != tx.pProveData->vecBlockTxProve.size() + 2) {
        return state.DoS(100, false, REJECT_INVALID, "provide vecblocktxprove size invalid");
    }

    // check tx and collect input/output, calc fees
    MCAmount totalFee = 0;
    for (int i = 2; i < vtx.size(); i++) {
        const MCTransactionRef& toProveTx = vtx[i];
        const std::vector<ProveDataItem>& vectProveData = tx.pProveData->vecBlockTxProve[i - 2];

        MCAmount fee;
        if (!CheckTransactionProveWithProveData(toProveTx, state, vectProveData, branchData, fee, false)) {
            return false;
        }
        totalFee += fee;
    }

    //目前设计支链是不产生块奖励，只有收取手续费
    if (vtx[0]->GetValueOut() != totalFee) {
        return state.DoS(100, false, REJECT_INVALID, "Prove coinbase transaction fail, fee invalid");
    }

    return true;
}

bool CheckProveContractData(const MCTransaction& tx, MCValidationState& state, BranchCache* pBranchCache)
{
    if (!tx.IsReport() || tx.pReportData == nullptr || tx.pReportData->reporttype != ReportType::REPORT_CONTRACT_DATA) {
        return state.DoS(0, false, REJECT_INVALID, "Invalid params");
    }

    const uint256& branchId = tx.pReportData->reportedBranchId;
    if (!pBranchCache->HasBranchData(branchId)) {
        return state.DoS(0, false, REJECT_INVALID, "Prove coinbase tx no branchid data");
    }

    // 先验证被举报交易及对应合约数据属于指定区块
    BranchData branchData = pBranchCache->GetBranchData(branchId);
    BranchBlockData* pReportedBlockData = branchData.GetBranchBlockData(tx.pReportData->reportedBlockHash);
    if (pReportedBlockData == nullptr) {
        return state.DoS(0, false, REJECT_INVALID, "Get branch reported block data fail");
    }

    uint256 reportedTxHashWithPrevData = GetTxHashWithPrevData(tx.pReportData->reportedTxHash, tx.pReportData->contractData->reportedContractPrevData);
    int reportedTxIndex = CheckSpvProof(pReportedBlockData->header.hashMerkleRootWithPrevData, tx.pReportData->contractData->reportedSpvProof.pmt, reportedTxHashWithPrevData);
    if (reportedTxIndex < 0) {
        return state.DoS(0, false, REJECT_INVALID, "Check tx prev data fail");
    }

    // 再验证替换的交易数据是否属于指定区块
    BranchBlockData* pProveBlockData = branchData.GetBranchBlockData(tx.pReportData->contractData->proveSpvProof.blockhash);
    if (pProveBlockData == nullptr) {
        return state.DoS(0, false, REJECT_INVALID, "Prove coinbase tx no block data");
    }

    uint256 proveTxHashWithData = GetTxHashWithData(tx.pReportData->contractData->proveTxHash, tx.pReportData->contractData->proveContractData);
    int proveTxIndex = CheckSpvProof(pProveBlockData->header.hashMerkleRootWithData, tx.pReportData->contractData->proveSpvProof.pmt, proveTxHashWithData);
    if (proveTxIndex < 0) {
        return state.DoS(0, false, REJECT_INVALID, "Check tx contract data fail");
    }

    if (pReportedBlockData->nHeight < pProveBlockData->nHeight) {
        return state.DoS(0, false, REJECT_INVALID, "Report block height less than prove block height");
    }

    const BranchBlockData* proveAncestorBlockData = branchData.GetAncestor(pReportedBlockData, pProveBlockData->nHeight);
    if (proveAncestorBlockData->mBlockHash != pProveBlockData->mBlockHash) {
        return state.DoS(0, false, REJECT_INVALID, "Report block and prove block are not in the same chain");
    }

    for (auto& item : tx.pReportData->contractData->proveContractData) {
        auto it = tx.pReportData->contractData->reportedContractPrevData.items.find(item.first);
        if (it != tx.pReportData->contractData->reportedContractPrevData.items.end()) {
            BranchBlockData& targetBlockData = branchData.mapHeads[it->second.blockHash];
            const BranchBlockData* subAncestorBlockData = branchData.GetAncestor(pReportedBlockData, targetBlockData.nHeight);
            if (subAncestorBlockData->mBlockHash != targetBlockData.mBlockHash) {
                return true;
            }

            if (pProveBlockData->nHeight > targetBlockData.nHeight || 
                (pProveBlockData->nHeight == targetBlockData.nHeight && proveTxIndex > it->second.txIndex && proveTxIndex < reportedTxIndex)) {
                return true;
            }
        }
    }

    return state.DoS(0, false, REJECT_INVALID, "The reported target do not have any problem");
}

bool CheckProveTx(const MCTransaction& tx, MCValidationState& state, BranchCache* pBranchCache)
{
    if (tx.IsProve()) {
        /*
        //uint256 proveFlagHash = GetProveTxHashKey(tx);
        //check report exist, don't check in cache now. let a report tx in a mined block may be better.
        if (pBranchCache->mReortTxFlag.count(proveFlagHash) == 0
            || pBranchCache->mReortTxFlag[proveFlagHash] != RP_FLAG_REPORTED)
        {
            return state.DoS(0, false, REJECT_INVALID, "prove to report tx not exist.");
        }
        */

        if (tx.pProveData->provetype == ReportType::REPORT_TX) {
            if (!CheckProveReportTx(tx, state, pBranchCache))
                return false;
        } else if (tx.pProveData->provetype == ReportType::REPORT_COINBASE) {
            if (!CheckProveCoinbaseTx(tx, state, pBranchCache))
                return false;
        } else if (tx.pProveData->provetype == ReportType::REPORT_MERKLETREE) {
            if (!CheckProveCoinbaseTx(tx, state, pBranchCache))
                return false;
        } else
            return state.DoS(0, false, REJECT_INVALID, "Invalid report type");
    }
    return true;
}

//
bool CheckReportRewardTransaction(const MCTransaction& tx, MCValidationState& state, MCBlockIndex* pindex, BranchCache* pBranchCache)
{
    if (!tx.IsReportReward())
        return false;
    if (!Params().IsMainChain())
        return state.DoS(100, false, REJECT_INVALID, "mainchain-not-accept-reportreward-tx");

    MCTransactionRef ptxReport;
    uint256 reporthashBlock;
    bool retflag;
    bool retval = ReadTxDataByTxIndex(tx.reporttxid, ptxReport, reporthashBlock, retflag);
    if (ptxReport == nullptr)
        return false; // report tx not exist

    if (!ptxReport->IsReport() || ptxReport->pReportData == nullptr)
        return state.DoS(100, false, REJECT_INVALID, "invalid-report-tx");

    // 检查是否满足高度
    if (!mapBlockIndex.count(reporthashBlock)) // block not exist any more ?
        return false;
    MCBlockIndex* rpBlockIndex = mapBlockIndex[reporthashBlock];
    if (!chainActive.Contains(rpBlockIndex)) // report tx not in active chain
        return false;
    if (rpBlockIndex->nHeight - pindex->nHeight < REPORT_OUTOF_HEIGHT)
        return state.DoS(100, false, REJECT_INVALID, "Still in prove stage.");

    //get data from ptxReport
    uint256 reportbranchid = ptxReport->pReportData->reportedBranchId;
    uint256 reportblockhash = ptxReport->pReportData->reportedBlockHash;
    if (!pBranchCache->HasBranchData(reportbranchid))
        return false;

    BranchData branchdata = pBranchCache->GetBranchData(reportbranchid);
    if (!branchdata.mapHeads.count(reportblockhash)) // best chain check? 1. no, 作弊过，但是数据在分叉上，也可以举报。带来麻烦是，矿工需要监控自己挖出来的分叉有没有监控。
        return false;

    // 从stake交易取出prevout(抵押币)
    BranchBlockData blockdata = branchdata.mapHeads[reportblockhash];
    //检查举报有没有被证明
    uint256 reportFlagHash = GetReportTxHashKey(*ptxReport);
    if (blockdata.mapReportStatus.count(reportFlagHash) == 0 || blockdata.mapReportStatus[reportFlagHash] == RP_FLAG_PROVED) {
        return false;
    }

    uint256 coinfromtxid;
    if (!GetMortgageCoinData(blockdata.pStakeTx->vout[0].scriptPubKey, &coinfromtxid))
        return state.DoS(100, false, REJECT_INVALID, "invalid-stake-pubkey");
    if (tx.vin[0].prevout.hash != coinfromtxid || tx.vin[0].prevout.n != 0)
        return state.DoS(100, false, REJECT_INVALID, "Invalid-report-reward-input");

    MCAmount nValueIn = blockdata.pStakeTx->vout[0].nValue;

    // 举报者地址
    const MCScript reporterAddress = ptxReport->vout[0].scriptPubKey;
    MCAmount nReporterValue = nValueIn / 2;
    MCAmount nMinerValue = nValueIn - nReporterValue;
    if (tx.vout[0].scriptPubKey != reporterAddress)
        return state.DoS(100, false, REJECT_INVALID, "vout[0]-must-to-reporter");
    if (tx.vout[0].nValue < nReporterValue)
        return state.DoS(100, false, REJECT_INVALID, "invalid-reporter-out-value");

    return true;
}

// 检查锁挖矿币交易
bool CheckLockMortgageMineCoinTx(const MCTransaction& tx, MCValidationState& state)
{
    if (!tx.IsLockMortgageMineCoin())
        return false;

    const std::string& fromBranchId = MCBaseChainParams::MAIN;
    // check report transactoin is in main chain
    MCRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig(fromBranchId, branchrpccfg) == false || branchrpccfg.IsValid() == false) {
        //if (Params().IsMainChain() && gArgs.GetBoolArg("-unchecknoconfigbranch", false))
        //    return true;

        std::string strErr = strprintf(" %s can not found branch rpc config for %s\n", __func__, fromBranchId);
        return state.DoS(1, false, REJECT_INVALID, strErr);
    }

    const std::string strMethod = "getreporttxdata";
    UniValue params(UniValue::VARR);
    params.push_back(tx.reporttxid.ToString());

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);

    const UniValue& result = find_value(reply, "result");
    const UniValue& errorVal = find_value(reply, "error");
    if (!errorVal.isNull()) {
        return error(" %s RPC call fail: %s\n", __func__, errorVal.write());
    }
    if (result.isNull()) {
        return error(" %s RPC call fail: result null\n", __func__);
    }

    const UniValue& uvtxhex = find_value(result, "txhex");
    const UniValue& uvconfirmations = find_value(result, "confirmations");
    const UniValue& uvprevouthash = find_value(result, "preminecoinvouthash");
    if (uvtxhex.isNull() || !uvtxhex.isStr() || uvconfirmations.isNull() || !uvconfirmations.isNum() || uvprevouthash.isNull())
        return error("%s RPC return invalid value\n", __func__);

    int32_t confirmations = uvconfirmations.get_int();
    if (confirmations < REPORT_LOCK_COIN_HEIGHT)
        return error("%s: Need 60 blocks to be mature, now is %d\n", __func__, confirmations);

    MCMutableTransaction mtxReport;
    if (!DecodeHexTx(mtxReport, uvtxhex.get_str()))
        return error("%s decode hex tx fail\n", __func__);

    if (!mtxReport.IsReport() || mtxReport.pReportData == nullptr)
        return false;

    if (mtxReport.pReportData->reportedBranchId != Params().GetBranchHash())
        return state.DoS(100, false, REJECT_INVALID, "Report-branchid-not-match");

    uint256 minecoinfromhash;
    if (!SafeParseHashV(uvprevouthash, minecoinfromhash))
        return error("%s parse uvprevouthash fail\n", __func__);

    if (tx.coinpreouthash != minecoinfromhash) {
        return state.DoS(0, false, REJECT_INVALID, "lock-mine-coin-error!");
    }

    //TODO: 是否要加判断还没有被证明? 加了会不会引出问题：主链下载数据比侧链领先一截，主链加完证明数据，侧链才加载锁定交易，然后进来此判断
    //      不加也ok吧，因为还是可以解锁

    //举报块是否在记录里?? 被举报的块有可能被丢弃??
    //   if (mapBlockIndex.count(mtxReport.pReportData->reportedBlockHash) == 0)
    //       return false;

    return true;
}

//检查解锁挖矿币交易
bool CheckUnlockMortgageMineCoinTx(const MCTransaction& tx, MCValidationState& state)
{
    if (!tx.IsUnLockMortgageMineCoin())
        return false;

    std::string fromBranchId = "main";
    // check prove transactoin is in main chain
    MCRPCConfig branchrpccfg;
    if (g_branchChainMan->GetRpcConfig(fromBranchId, branchrpccfg) == false || branchrpccfg.IsValid() == false) {
        if (Params().IsMainChain() && gArgs.GetBoolArg("-unchecknoconfigbranch", false))
            return true;

        std::string strErr = strprintf(" %s can not found branch rpc config for %s\n", __func__, fromBranchId);
        return state.DoS(1, false, REJECT_INVALID, strErr);
    }

    const std::string strMethod = "getprovetxdata";
    UniValue params(UniValue::VARR);
    params.push_back(tx.provetxid.ToString());

    UniValue reply = CallRPC(branchrpccfg, strMethod, params);

    const UniValue& result = find_value(reply, "result");
    const UniValue& errorVal = find_value(reply, "error");
    if (!errorVal.isNull()) {
        return state.DoS(0, false, REJECT_INVALID,
            strprintf(" %s RPC call fail: %s\n", __func__, errorVal.write()));
    }
    if (result.isNull()) {
        return state.DoS(0, false, REJECT_INVALID,
            "CheckUnlockMortgageMineCoinTx RPC call fail: result null.");
    }

    const UniValue& uvtxhex = find_value(result, "txhex");
    const UniValue& uvconfirmations = find_value(result, "confirmations");
    const UniValue& uvprevouthash = find_value(result, "preminecoinvouthash");
    if (uvtxhex.isNull() || !uvtxhex.isStr() || uvconfirmations.isNull() || !uvconfirmations.isNum() || uvprevouthash.isNull())
        return state.DoS(0, false, REJECT_INVALID, "CheckUnlockMortgageMineCoinTx RPC return invalid value");

    int32_t confirmations = uvconfirmations.get_int();
    if (confirmations < REPORT_LOCK_COIN_HEIGHT)
        return state.DoS(0, false, REJECT_INVALID,
            strprintf("%s: Need 60 blocks to be mature, now is %d\n", __func__, confirmations));

    MCMutableTransaction mtxProve;
    if (!DecodeHexTx(mtxProve, uvtxhex.get_str()))
        return state.DoS(0, false, REJECT_INVALID, "CheckUnlockMortgageMineCoinTx decode hex tx fail");

    if (mtxProve.pProveData == nullptr)
        return false;

    if (mtxProve.pProveData->branchId != Params().GetBranchHash())
        return state.DoS(100, false, REJECT_INVALID, "prove-branchid-not-match");

    uint256 minecoinfromhash;
    if (!SafeParseHashV(uvprevouthash, minecoinfromhash))
        return state.DoS(0, false, REJECT_INVALID, "CheckUnlockMortgageMineCoinTx parse minecoinfromhash fail");

    if (tx.coinpreouthash != minecoinfromhash) {
        return state.DoS(0, false, REJECT_INVALID, "lock-mine-coin-error!");
    }

    return true;
}
