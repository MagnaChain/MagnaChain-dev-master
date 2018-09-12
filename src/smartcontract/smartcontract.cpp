// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <stdlib.h>
#include <string.h>
#include <string>

#include <boost/foreach.hpp>

#include "coding/base58.h"
#include "smartcontract/smartcontract.h"
#include "script/standard.h"
#include "transaction/txmempool.h"
#include "univalue.h"
#include "validation/validation.h" //for mempool access
#include "wallet/wallet.h"
#include "utils/util.h"
#include "wallet/rpcwallet.h"
#include "consensus/consensus.h"
#include "wallet/rpcwallet.h"
#include "wallet/coincontrol.h"
#include "univalue.h"

#if defined(_WIN32)
#define strdup _strdup
#endif

/*
函数 regContract(filename)
参数 filename 智能合约脚本路径
ret1 bool 是否注册成功
ret2 ret1=true时表示指令执行数
ret3 如果注册失败,返回失败信息,如果成功,返回编译好的字节码
ret4 如果注册失败,返回nil,如果成功,初始化的数据,可能是nil

函数 callContract(strComplieFun, strData, funname, jsonparams)
strComplieFun 合约代码
strData msgpack后的数据
funname 用调用的合约函数名
jsonparams string json参数
ret1 调用是否成功
ret2 ret1=true时表示指令执行数
ret3 ret1 = false,返回错误信息,ret1 = true,返回新的数据
ret4 合约返回的json数据
*/

static const char* initscript = "                                               \n\
function printTable(t, indent)                                                  \n\
	local strIndent = ''                                                        \n\
	local istart = indent == nil                                                \n\
	indent = indent or 0                                                        \n\
	for i=1, indent do                                                          \n\
		strIndent = strIndent..'  '                                             \n\
	end                                                                         \n\
	if istart then print(strIndent..'{') end                                    \n\
	for k,v in pairs(t) do                                                      \n\
		if type(v) == 'table' then                                              \n\
			print(strIndent..tostring(k)..'={')                                 \n\
			printTable(v, indent+1)                                             \n\
		else                                                                    \n\
			print(strIndent..tostring(k)..'='..tostring(v)..',')                \n\
		end                                                                     \n\
	end                                                                         \n\
	print(strIndent..'}'..(istart and '' or ','))                               \n\
end                                                                             \n\
																				\n\
local function copyTable(from, to)                                              \n\
	for k,v in pairs(from) do                                                   \n\
		if type(v) == 'table' then                                              \n\
			to[k] = {}                                                          \n\
			copyTable(v, to[k])                                                 \n\
		else                                                                    \n\
			to[k] = v                                                           \n\
		end                                                                     \n\
	end                                                                         \n\
	return to                                                                   \n\
end                                                                             \n\
                                                                                \n\
--lua 5.1                                                                       \n\
local function createSafeEnv()                                                  \n\
    local env = {}                                                              \n\
    env._G = env	                                                            \n\
    env._VERSION = _VERSION	                                                    \n\
--    env.arg = arg	                                                            \n\
    env.assert = assert	                                                        \n\
    env.cmsgpack = nil                                                          \n\
--    env.collectgarbage = collectgarbage										\n\
--    env.coroutine = coroutine	                                                \n\
--    env.debug = debug	                                                        \n\
--    env.dofile = dofile	                                                    \n\
    env.error = error	                                                        \n\
--    env.gcinfo = gcinfo														\n\
--    env.getfenv = getfenv                                                     \n\
--    env.getmetatable = getmetatable											\n\
--    env.io = io	                                                            \n\
    env.ipairs = ipairs	                                                        \n\
--    env.load = load	                                                        \n\
--    env.loadfile = loadfile	                                                \n\
--    env.loadstring = loadstring	                                            \n\
                                                                                \n\
    env.math = {}	                                                            \n\
    env.math.pow = math.pow                                                     \n\
    env.math.max = math.max	                                                    \n\
    env.math.min = math.min                                                     \n\
	                                                                            \n\
--    env.module = module	                                                    \n\
--    env.newproxy = newproxy													\n\
    env.next = next	                                                            \n\
	                                                                            \n\
--	env.os = copyTable(os, {})                                                  \n\
--	env.os.execute = nil                                                        \n\
--	env.os.remove = nil                                                         \n\
--	env.os.rename = nil                                                         \n\
--	env.os.exit = nil                                                           \n\
	                                                                            \n\
--    env.package = package	                                                    \n\
    env.pairs = pairs	                                                        \n\
--    env.pcall = pcall	                                                        \n\
    env.print = print	                                                        \n\
--    env.rawequal =                                                            \n\
--    env.rawget = rawget	                                                    \n\
--    env.rawset = rawset	                                                    \n\
--    env.require = require	                                                    \n\
    env.select = select	                                                        \n\
--    env.setfenv = setfenv	                                                    \n\
    env.setmetatable = setmetatable	                                            \n\
    env.string = copyTable(string, {})	                                        \n\
    env.string.dump = nil                                                       \n\
    env.table = table	                                                        \n\
    env.tonumber = tonumber	                                                    \n\
    env.tostring = tostring	                                                    \n\
    env.type = type	                                                            \n\
    env.unpack = unpack	                                                        \n\
	env.unpacktable = unpacktable												\n\
--    env.xpcall = xpcall	                                                    \n\
	--lpcall                                                                    \n\
																				\n\
    env.msg = msg		                                                        \n\
    env.callcontract = callcontract		                                        \n\
    env.setfenv = setfenv				                                        \n\
    env.innerCall = innerCall			                                        \n\
    env.send = send						                                        \n\
	return env                                                                  \n\
end                                                                             \n\
                                                                                \n\
cjson.encode_sparse_array(true, 1,1)                                            \n\
                                                                                \n\
function regContract(maxCallNum, maxDataLen, _strScript)						\n\
	local fun, err = loadstring(_strScript)                                     \n\
	if err then                                                                 \n\
		print(err)                                                              \n\
		return false, 'load contract fail:'..err                                \n\
	end                                                                         \n\
    collectgarbage('collect')                                                   \n\
	                                                                            \n\
	local myenv = createSafeEnv()                                               \n\
	setfenv(fun, myenv)                                                         \n\
	local runOk1, rt1, err = lpcall(maxCallNum, fun)							\n\
	if not runOk1 then                                                          \n\
		print(err)                                                              \n\
		return false, 'do contract fail:'..err                                  \n\
	end                                                                         \n\
																				\n\
	maxCallNum = maxCallNum - rt1												\n\
	local runOk2, rt2, err2														\n\
	if type(myenv.init) == 'function' then                                      \n\
		runOk2, rt2, err2 = lpcall(maxCallNum, myenv.init)						\n\
		if not runOk2 then                                                      \n\
			print(err2)															\n\
			return false, 'contract init function fail:'..err2					\n\
		end                                                                     \n\
	else                                                                        \n\
		print('no init function to call.')                                      \n\
	end                                                                         \n\
	                                                                            \n\
	local strCompileFun = string.dump(fun)                                      \n\
	local strPackData                                                           \n\
	if type(myenv.PersistentData) == 'table' then                               \n\
		strPackData = cmsgpack.pack(myenv.PersistentData)                       \n\
		local dataLen = string.len(strPackData)									\n\
		if dataLen > maxDataLen then											\n\
			return false, 'Lua:regContract dataLen > maxDataLen'                \n\
		end																		\n\
	else                                                                        \n\
		print('no init PersistentData to save')                                 \n\
	end                                                                         \n\
	                                                                            \n\
	return true, rt1 + (rt2 or 0), strPackData, strCompileFun					\n\
end                                                                             \n\
                                                                                \n\
function callContract(maxCallNum, maxDataLen, code, data, funcname, ...)	    \n\
	if data and #data >0 then                                                   \n\
		data = cmsgpack.unpack(data)                                            \n\
	end                                                                         \n\
    collectgarbage('collect')                                                   \n\
	local lf = loadstring(code)	                                            	\n\
	local myenv = createSafeEnv()                                               \n\
    myenv.PersistentData = data                                                 \n\
	--printTable(myenv.PersistentData)											\n\
	setfenv(lf, myenv)                                                          \n\
	local runOk1, rt1, err1 = lpcall(maxCallNum, lf)							\n\
	if not runOk1 then                                                          \n\
		print(err1)                                                             \n\
		return false, err1                                                      \n\
	end                                                                         \n\
                                                                                \n\
	maxCallNum = maxCallNum - rt1												\n\
	local runRet		                                                        \n\
	local callfun = myenv[funcname]                                             \n\
	local runOk2, rt2, err2														\n\
	if type(callfun) == 'function' and funcname ~= 'init' then                  \n\
		runRet = { lpcall(maxCallNum, callfun, ...) }							\n\
		runOk2 = runRet[1]														\n\
		rt2 = runRet[2]															\n\
		err2 = runRet[3]														\n\
		if not runOk2 then                                                      \n\
			print(err2)															\n\
			return false, 'callContract function fail:'..err2					\n\
		else																	\n\
			local temp = {}														\n\
			for i = 3, #runRet do												\n\
				table.insert(temp, runRet[i])									\n\
			end																	\n\
			runRet = temp														\n\
		end																		\n\
	else                                                                        \n\
		return false, 'no function for '..tostring(funcname)                    \n\
	end                                                                         \n\
	                                                                            \n\
	--printTable(myenv.PersistentData)											\n\
	local strPackData                                                           \n\
	if type(myenv.PersistentData) == 'table' then                               \n\
		strPackData = cmsgpack.pack(myenv.PersistentData)                       \n\
		local dataLen = string.len(strPackData)									\n\
		if dataLen > maxDataLen then											\n\
			return false, 'Lua:callContract dataLen > maxDataLen'				\n\
		end																		\n\
	end                                                                         \n\
	return true, rt1 + (rt2 or 0), strPackData, unpacktable(runRet)				\n\
end                                                                             \n";

bool GetPubKey(const CellWallet* pWallet, const CellLinkAddress& addr, CellPubKey& pubKey)
{
    CellKeyID key;
    if (!addr.GetKeyID(key))
        return false;

    return pWallet->GetPubKey(key, pubKey);
}

bool GenerateContractSender(const CellWallet* pWallet, CellLinkAddress& sendAddr)
{
    std::vector<CellOutput> coins;
    pWallet->AvailableCoins(coins);
    if (coins.size() == 0)
        return false;

    CellTxDestination dest;
    const CellOutput& out = coins[0];
    const CellTxOut& txo = out.tx->tx->vout[out.i];
    ExtractDestination(txo.scriptPubKey, dest);
    sendAddr.Set(dest);
    return true;
}

bool GetSenderAddr(CellWallet* pWallet, const std::string& strSenderAddr, CellLinkAddress& senderAddr)
{
    CellKeyID key;
    bool ret = false;
    if (!strSenderAddr.empty()) {
        senderAddr.SetString(strSenderAddr);
        senderAddr.GetKeyID(key);
        ret = pWallet->HaveKey(key);
    }
    else {
        if (pWallet->_senderAddr.IsValid()) {
            senderAddr = pWallet->_senderAddr;
            senderAddr.GetKeyID(key);
            ret = pWallet->HaveKey(key);
        }
        
        if (!ret && GenerateContractSender(pWallet, senderAddr)) {
            senderAddr.GetKeyID(key);
            ret = pWallet->HaveKey(key);
        }
    }

    if (ret)
        pWallet->_senderAddr = senderAddr;

    return ret;
}

// generate contract address
// format: sender address keyid + block address + new celllink address + contract script file hash
CellContractID GenerateContractAddress(CellWallet* pWallet, const CellLinkAddress& senderAddr, const std::string& code)
{
    CellHashWriter ss(SER_GETHASH, 0);

    // sender address keyid
    CellKeyID senderId;
    senderAddr.GetKeyID(senderId);
    ss << senderId;

    // block address
    std::string blockAddress;
    if (chainActive.Height() < COINBASE_MATURITY)
        blockAddress = chainActive.Tip()->GetBlockHash().GetHex();
    else
        blockAddress = chainActive[chainActive.Height() - COINBASE_MATURITY]->GetBlockHash().GetHex();
    ss << blockAddress;

    // new celllink address
    if (pWallet != nullptr) {
        CellPubKey newKey;
        if (!pWallet->GetKeyFromPool(newKey))
            throw std::runtime_error(strprintf("%s:%d Keypool ran out, please call keypoolrefill first", __FILE__, __LINE__));
        ss << newKey.GetID();
    }
    else {
        // sdk用户没有钱包在本地 
        ss << GetTimeMillis();
        ss << (int64_t)(&senderAddr); // get random value by address point
    }

    // contract script file hash
    ss << Hash(code.begin(), code.end()).GetHex();
    return CellContractID(Hash160(ParseHex(ss.GetHash().ToString())));
}

void SetContractMsg(lua_State* L, const std::string& contractAddr, const std::string& origin, const std::string& sender, lua_Number payment, uint32_t blockTime, lua_Number blockHeight)
{
    // 创建msg表
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "msg");

    // 设置相关参数
    lua_pushstring(L, contractAddr.c_str());
    lua_setfield(L, -2, "thisAddress"); //合约本身的地址
    lua_pushstring(L, origin.c_str());
    lua_setfield(L, -2, "origin"); // 原始发起调用合约者公钥地址
    lua_pushstring(L, sender.c_str());
    lua_setfield(L, -2, "sender"); // 当前发起调用合约者地址(可能为合约或公钥地址)
    lua_pushnumber(L, payment);
    lua_setfield(L, -2, "payment"); //msg.value: number of wei sent with the message
    lua_pushnumber(L, blockTime);
    lua_setfield(L, -2, "blocktimestamp");
    lua_pushnumber(L, blockHeight);
    lua_setfield(L, -2, "blocknumber");
    lua_pop(L, 1);
}

int PublishContract(SmartLuaState* sls, CellWallet* pWallet, CellAmount amount, const std::string& strSenderAddr, std::string& rawCode, std::string& code, UniValue& ret)
{
    ret.setArray();

    CellLinkAddress senderAddr;
    if (!GetSenderAddr(pWallet, strSenderAddr, senderAddr)) {
        ret.push_back("GetSenderAddr fail.");
        return -1;
    }

    CellKeyID senderKey;
    CellPubKey senderPubKey;
    if (!senderAddr.GetKeyID(senderKey) || !pWallet->GetPubKey(senderKey, senderPubKey)) {
        ret.push_back("Get Key or PubKey fail.");
        return -1;
    }

    // temp addresss, replace in CellWallet::CreateTransaction
    CellContractID contractId = GenerateContractAddress(pWallet, senderAddr, rawCode);
    CellLinkAddress contractAddr(contractId);

    SmartContractRet scr;
    sls->Initialize(GetTime(), chainActive.Height() + 1, amount, senderAddr, nullptr, nullptr, 0);
    int result = PublishContract(sls, contractAddr, rawCode, code, scr);
    if (result == 0) {
        CellScript scriptPubKey = GetScriptForDestination(contractAddr.Get());

        sls->contractIds.erase(contractId);
        CellWalletTx wtx;
        wtx.transaction_version = CellTransaction::PUBLISH_CONTRACT_VERSION;
        wtx.contractCode = rawCode;
        wtx.contractSender = senderPubKey;
        wtx.contractAddrs.emplace_back(contractId);
        wtx.contractAddrs.insert(wtx.contractAddrs.end(), sls->contractIds.begin(), sls->contractIds.end());

        bool subtractFeeFromAmount = false;
        CellCoinControl coinCtrl;
        EnsureWalletIsUnlocked(pWallet);
        SendMoney(pWallet, scriptPubKey, amount, subtractFeeFromAmount, wtx, coinCtrl, sls);

        ret.setObject();
        ret.push_back(Pair("txid", wtx.tx->GetHash().ToString()));
        ret.push_back(Pair("contractaddress", CellLinkAddress(wtx.tx->contractAddrs[0]).ToString()));
        ret.push_back(Pair("senderaddress", senderAddr.ToString()));
    }
    else
        ret.push_back(strprintf("PublishContract fail, code %d", result));

    return result;
}

int PublishContract(SmartLuaState* sls, CellLinkAddress& contractAddr, const std::string& rawCode, std::string& code, SmartContractRet& scr)
{
    if (sls->amount < 0) {
        scr.result.push_back("amount <= 0.");
        return -1;
    }

    CellContractID contractId;
    contractAddr.GetContractID(contractId);
    ContractInfo contractInfo;
    if (sls->GetContractInfo(contractId, contractInfo)) {
        scr.result.push_back("Contract exists.");
        return -1;
    }

    lua_State* L = sls->GetLuaState(contractAddr);
    SetContractMsg(L, contractAddr.ToString(), sls->originAddr.ToString(), sls->originAddr.ToString(), sls->amount, sls->timestamp, sls->blockHeight);

    int result = PublishContract(L, rawCode, code, scr);
    if (result == 0) {
        sls->runningTimes = scr.runningTimes;
        sls->codeLen = code.size();
        sls->deltaDataLen = scr.data.size();

        if (sls->saveType > 0) {
            contractInfo.code = code;
            contractInfo.data = scr.data;
            sls->SetContractInfo(contractId, contractInfo, sls->saveType == SmartLuaState::SAVE_TYPE_CACHE);
        }
    }
    sls->ReleaseLuaState(L);

    return result;
}

int PublishContract(lua_State* L, const std::string& rawCode, std::string& code, SmartContractRet& scr)
{
    int top = lua_gettop(L);
    lua_getglobal(L, "regContract");
    lua_pushnumber(L, MAX_CONTRACT_CALL);
    lua_pushnumber(L, MAX_DATA_LEN);
    lua_pushlstring(L, rawCode.c_str(), rawCode.size());
    int argc = 3;

    int result = lua_pcall(L, argc, LUA_MULTRET, 0);
    if (result == 0 && lua_toboolean(L, top + 1) && lua_gettop(L) >= top + 4) {
        scr.runningTimes = lua_tointeger(L, top + 2);

        size_t dl = 0;
        const char* temp = lua_tolstring(L, top + 3, &dl);
        scr.data.assign(temp, dl);

        size_t cl = 0;
        temp = lua_tolstring(L, top + 4, &cl);
        code.assign(temp, cl);
    }
    else {
        if (result == 0) {
            result = -1;
        }
        const char* err = lua_tostring(L, -1);
        LogPrintf("%s:%d %s\n", __FILE__, __LINE__, err);
        scr.result.push_back(err);
    }

    lua_settop(L, top);
    return result;
}

int CallContract(SmartLuaState* sls, long& maxCallNum, CellLinkAddress& contractAddr, const std::string& strFuncName, const UniValue& args, SmartContractRet& scr)
{
    if (sls->amount < 0) {
        scr.result.push_back("amount <= 0.");
        return -1;
    }

    scr.result.clear();

    CellContractID contractId;
    contractAddr.GetContractID(contractId);
    ContractInfo contractInfo;
    if (!sls->GetContractInfo(contractId, contractInfo) || contractInfo.code.size() <= 0) {
        scr.result.push_back("GetContractInfo fail");
        return -1;
    }

    std::string senderAddr = (sls->contractAddrs.size() > 0 ? sls->contractAddrs[sls->contractAddrs.size() - 1].ToString() : sls->originAddr.ToString());
    lua_State* L = sls->GetLuaState(contractAddr);
    SetContractMsg(L, contractAddr.ToString(), sls->originAddr.ToString(), senderAddr, sls->amount, sls->timestamp, sls->blockHeight);
    int result = CallContract(L, maxCallNum, contractInfo.code, contractInfo.data, strFuncName, args, scr);
    if (result == 0 && contractInfo.data != scr.data) {
        maxCallNum = L->limit_instruction;
        assert(maxCallNum >= 0);

        sls->deltaDataLen += std::max(0u, (uint32_t)(scr.data.size() - contractInfo.data.size()));
        if (sls->saveType > 0) {
            contractInfo.data = scr.data;
            sls->SetContractInfo(contractId, contractInfo, sls->saveType == SmartLuaState::SAVE_TYPE_CACHE);
        }
    }
    sls->ReleaseLuaState(L);

    return result;
}

int CallContract(lua_State* L, long maxCallNum, const std::string& code, const std::string& data, const std::string& strFuncName, const UniValue& args, SmartContractRet& ret)
{
    int top = lua_gettop(L);
    lua_getglobal(L, "callContract");
    lua_pushnumber(L, maxCallNum);
    lua_pushnumber(L, MAX_DATA_LEN);
    lua_pushlstring(L, code.c_str(), code.size());
    if (data.size() > 0)
        lua_pushlstring(L, data.c_str(), data.size());
    else
        lua_pushnil(L);
    lua_pushstring(L, strFuncName.c_str());
    int argc = 5;

    /* other use params */
    for (int i = 0; i < args.size(); ++i) {
        UniValue v = args[i];
        switch (v.type()) {
        case UniValue::VSTR:
            lua_pushstring(L, v.get_str().c_str());
            break;
        case UniValue::VNUM:
            lua_pushnumber(L, v.get_int64());
            break;
        case UniValue::VBOOL:
            lua_pushboolean(L, v.get_bool());
            break;
        default:
            lua_pushnil(L);
            break;
        }
        argc++;
    }

    int result = lua_pcall(L, argc, LUA_MULTRET, 0);
    if (result == 0 && lua_toboolean(L, top + 1) && lua_gettop(L) >= top + 3) {
        ret.runningTimes = lua_tonumber(L, top + 2);

        size_t dl = 0;
        const char* temp = lua_tolstring(L, top + 3, &dl);
        ret.data.assign(temp, dl);

        int newTop = lua_gettop(L);
        for (int i = top + 4; i <= newTop; ++i) {
            int t = lua_type(L, i);
            switch (t) {
            case LUA_TNUMBER:
                ret.result.push_back(UniValue((int64_t)lua_tonumber(L, i)));
                break;
            case LUA_TBOOLEAN:
                ret.result.push_back(UniValue((bool)lua_toboolean(L, i)));
                break;
            case LUA_TSTRING:
            {
                size_t sl = 0;
                const char* sv = lua_tolstring(L, i, &sl);
                ret.result.push_back(std::string(sv, sl));
                break;
            }
            default:
                ret.result.push_back(UniValue());
                break;
            }
        }
    }
    else {
        if (result == 0) {
            result = -1;
        }
        const char* err = lua_tostring(L, -1);
        LogPrintf("%s:%d %s\n", __FILE__, __LINE__, err);
        ret.result.push_back(err);
    }

    lua_settop(L, top);
    return result;
}

// Lua内部嵌套调用合约
int InternalCallContract(lua_State* L)
{
    SmartLuaState* sls = (SmartLuaState*)L->userData;
    if (sls == nullptr)
        throw std::runtime_error(strprintf("%s:%d smartLuaState == nullptr", __FILE__, __LINE__));

    std::string strContractAddr = lua_tostring(L, 1);
    CellLinkAddress contractAddr(strContractAddr);
    if (!contractAddr.IsValid())
        throw std::runtime_error(strprintf("%s:%d contractAddr is invalid", __FILE__, __LINE__));

    std::string strFuncName = lua_tostring(L, 2);
    if (strFuncName.empty())
        throw std::runtime_error(strprintf("%s:%d function name is empty", __FILE__, __LINE__));

    int top = lua_gettop(L);
    UniValue args(UniValue::VType::VARR);
    for (int i = 3; i <= top; ++i) {
        switch (lua_type(L, i)) {
        case LUA_TSTRING:
            args.push_back(lua_tostring(L, i));
            break;
        case LUA_TNUMBER:
            args.push_back((int64_t)lua_tonumber(L, i));
            break;
        case LUA_TBOOLEAN:
            args.push_back((bool)lua_toboolean(L, i));
            break;
        default:
            break;
        }
    }

    SmartContractRet scr;
    long maxCallNum = L->limit_instruction;
    int result = CallContract(sls, maxCallNum, contractAddr, strFuncName, args, scr);
    if (result == 0)
        L->limit_instruction = maxCallNum;
    else
        throw std::runtime_error(strprintf("InternalCallContract fail, error %d.", result));

    lua_pushnumber(L, result);
    for (int i = 0; i < scr.result.size(); ++i) {
        switch (scr.result[i].type()) {
        case UniValue::VNUM:
            lua_pushnumber(L, scr.result[i].get_int64());
            break;
        case UniValue::VBOOL:
            lua_pushboolean(L, scr.result[i].get_bool());
            break;
        case UniValue::VSTR:
            lua_pushstring(L, scr.result[i].get_str().c_str());
            break;
        default:
            lua_pushnil(L);
            break;
        }
    }

    return scr.result.size() + 1;
}

// Lua内部向指定地址发送代币
int SendCoins(lua_State* L)
{
    SmartLuaState* sls = (SmartLuaState*)L->userData;
    if (sls == nullptr) {
        error("%s smartLuaState == nullptr", __FUNCTION__);
        return 0;
    }

    std::string strDest = lua_tostring(L, 1);
    CellAmount sendAmount = lua_tonumber(L, 2);

    if (sls->totalAmount < 0) {
        const int delaySend = 6;
        bool bGetCoinsFromCoinsDb = true;
        int curHeight = chainActive.Height();

        sls->totalAmount = 0;
        if (!bGetCoinsFromCoinsDb) {
            CellScript scriptPubKey = GetScriptForDestination(sls->contractAddrs[sls->contractAddrs.size() - 1].Get());

            CellCoinsViewCursor* pCursor = pcoinsdbview->Cursor();
            while (pCursor->Valid()) {
                Coin coin;
                pCursor->GetValue(coin);
                CellOutPoint outPoint;
                pCursor->GetKey(outPoint);
                pCursor->Next();

                if (coin.nHeight + delaySend > curHeight) {
                    continue;
                }
                if (coin.out.scriptPubKey != scriptPubKey) {
                    continue;
                }
                sls->inputs.emplace_back(coin, outPoint);
                sls->totalAmount += coin.out.nValue;
            }
        }
        else {
            const CellContractID& kAddr = boost::get<CellContractID>(sls->contractAddrs[sls->contractAddrs.size() - 1].Get());
            CoinListPtr plist = pcoinListDb->GetList((const uint160&)kAddr);
            CellLinkAddress addr(kAddr);
            //LogPrintf("Send contract address %s\n", addr.ToString());

            if (plist != nullptr) {
                BOOST_FOREACH(const CellOutPoint& outPoint, plist->coins) {
                    const Coin& coin = pcoinsTip->AccessCoin(outPoint);
                    if (coin.nHeight + delaySend > curHeight) {
                        continue;
                    }
                    if (coin.IsSpent()) {
                        continue;
                    }
                    CellKeyID kDestKey;
                    if (!coin.out.scriptPubKey.GetContractAddr(kDestKey)) {
                        continue;
                    }
                    sls->inputs.emplace_back(coin, outPoint);
                    sls->totalAmount += coin.out.nValue;
                }
            }
        }
    }

    if (sls->sendAmount + sendAmount > sls->totalAmount) {
        lua_pushboolean(L, false);
        return 1;
    }

    CellTxOut out;
    out.nValue = sendAmount;
    CellLinkAddress kDest(strDest);
    CellScript scriptPubKey = GetScriptForDestination(kDest.Get());
    out.scriptPubKey = scriptPubKey;
    sls->outputs.emplace_back(out);
    sls->sendAmount += sendAmount;

    lua_pushboolean(L, true);
    return 1;
}

void SmartLuaState::Initialize(int64_t timestamp, int blockHeight, CellAmount amount, CellLinkAddress& originAddr, ContractContext* pContractContext, CellBlockIndex* pPrevBlockIndex, int saveType)
{
    this->timestamp = timestamp;
    this->blockHeight = blockHeight;
    this->originAddr = originAddr;
    this->_pContractContext = pContractContext;
    this->_pPrevBlockIndex = pPrevBlockIndex;
    this->saveType = saveType;
    this->amount = amount;

    if (_pContractContext == nullptr)
        _pContractContext = &mpContractDb->_contractContext;

    this->inputs.clear();
    this->outputs.clear();
    this->contractIds.clear();
    this->contractAddrs.clear();
    totalAmount = -1;
    sendAmount = 0;
    runningTimes = 0;
    deltaDataLen = 0;
    codeLen = 0;
}

lua_State* SmartLuaState::GetLuaState(CellLinkAddress& contractAddr)
{
    lua_State* L = nullptr;
    if (_luaStates.size() > 0) {
        L = _luaStates.back();
        _luaStates.pop();
    }
    else {
        L = lua_open();
        if (L == nullptr) {
            error("cannot create state: not enough memory\n");
            return nullptr;
        }

        luaL_openlibs(L);
        luaopen_cmsgpack(L);
        luaopen_cjson(L);

        if (luaL_dostring(L, initscript)) {
            error("%s\n", lua_tostring(L, -1));
            return nullptr;
        }

        lua_pushcfunction(L, InternalCallContract);
        lua_setglobal(L, "callcontract");
        lua_pushcfunction(L, SendCoins);
        lua_setglobal(L, "send");

        lua_gc(L, LUA_GCRESTART, 0);
        L->userData = this;
    }

    CellKeyID contractId;
    contractAddr.GetKeyID(contractId);
    contractIds.insert(contractId);
    contractAddrs.emplace_back(contractAddr);

    return L;
}

void SmartLuaState::ReleaseLuaState(lua_State* L)
{
    contractAddrs.resize(contractAddrs.size() - 1);

    lua_gc(L, LUA_GCCOLLECT, 0); /* stop collector during initialization */
    _luaStates.push(L);
}

void SmartLuaState::SetContractInfo(const CellContractID& contractId, ContractInfo& contractInfo, bool cache)
{
    LOCK(_contractCS);
    if (cache)
        _pContractContext->SetCache(contractId, contractInfo);
    else
        _pContractContext->SetData(contractId, contractInfo);
}

bool SmartLuaState::GetContractInfo(const CellContractID& contractId, ContractInfo& contractInfo)
{
    LOCK(_contractCS);

    // 直接从快照缓存中读取
    if (_pContractContext->GetData(contractId, contractInfo))
        return true;

    return mpContractDb->GetContractInfo(contractId, contractInfo, _pPrevBlockIndex);
}
