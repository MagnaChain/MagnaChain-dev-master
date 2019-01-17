// Copyright (c) 2016-2019 The MagnaChain Core developers
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
#include "mining/miner.h"
#include "consensus/merkle.h"
#include "policy/policy.h"

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
local function createSafeEnv()                                                  \n\
    local env = {}                                                              \n\
    env._G = env	                                                            \n\
    env._VERSION = _VERSION	                                                    \n\
    --env.arg = arg	                                                            \n\
    env.assert = assert	                                                        \n\
    --env.collectgarbage = collectgarbage										\n\
    --env.coroutine = coroutine	                                                \n\
    --env.debug = debug	                                                        \n\
    --env.dofile = dofile	                                                    \n\
    env.error = error	                                                        \n\
    --env.gcinfo = gcinfo														\n\
    --env.getfenv = getfenv                                                     \n\
    --env.getmetatable = getmetatable											\n\
    --env.io = io	                                                            \n\
    env.ipairs = ipairs	                                                        \n\
    --env.load = load	                                                        \n\
    --env.loadfile = loadfile	                                                \n\
    --env.loadstring = loadstring	                                            \n\
    env.math = {}	                                                            \n\
    env.math.pow = math.pow                                                     \n\
    env.math.max = math.max	                                                    \n\
    env.math.min = math.min                                                     \n\
    --env.module = module	                                                    \n\
    --env.newproxy = newproxy													\n\
    env.next = next	                                                            \n\
    --nv.os = copyTable(os, {})                                                 \n\
    --nv.os.execute = nil                                                       \n\
    --nv.os.remove = nil                                                        \n\
    --nv.os.rename = nil                                                        \n\
    --env.os.exit = nil                                                         \n\
    --env.package = package	                                                    \n\
    env.pairs = pairs	                                                        \n\
    --env.pcall = pcall	                                                        \n\
    --env.print = print	                                                        \n\
    --env.rawequal =                                                            \n\
    --env.rawget = rawget	                                                    \n\
    --env.rawset = rawset	                                                    \n\
    --env.require = require	                                                    \n\
    env.select = select	                                                        \n\
    --env.setfenv = setfenv	                                                    \n\
    env.setmetatable = setmetatable	                                            \n\
    --env.string = string	                                                    \n\
    env.table = table	                                                        \n\
    env.tonumber = tonumber	                                                    \n\
    env.tostring = tostring	                                                    \n\
    env.type = type	                                                            \n\
    env.unpack = unpack	                                                        \n\
	env.unpacktable = unpacktable												\n\
    --env.xpcall = xpcall	                                                    \n\
    env.msg = msg		                                                        \n\
    env.callcontract = callcontract		                                        \n\
    env.setfenv = setfenv				                                        \n\
    env.send = send						                                        \n\
	return env                                                                  \n\
end                                                                             \n\
                                                                                \n\
cjson.encode_sparse_array(true, 1,1)                                            \n\
                                                                                \n\
function regContract(maxCallNum, maxDataLen, _strScript)						\n\
	local fun, err = loadstring(_strScript)                                     \n\
	if err then                                                                 \n\
        print(_strScript)                                                       \n\
		return false, err                                                       \n\
	end                                                                         \n\
	                                                                            \n\
    local success                                                               \n\
	local myenv = createSafeEnv()                                               \n\
	setfenv(fun, myenv)                                                         \n\
	success, err = lpcall(maxCallNum, fun)							            \n\
	if not success then                                                         \n\
		return false, err                                  						\n\
	end                                                                         \n\
	                                                                            \n\
	if type(myenv.init) == 'function' then                                      \n\
		success, err = lpcall(-1, myenv.init)						            \n\
		if not success then                                                     \n\
			return false, err					                                \n\
		end                                                                     \n\
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
	end                                                                         \n\
	                                                                            \n\
	return true, strPackData, strCompileFun					                    \n\
end                                                                             \n\
                                                                                \n\
function callContract(maxCallNum, maxDataLen, code, data, funcname, ...)	    \n\
	if data and #data >0 then                                                   \n\
		data = cmsgpack.unpack(data)                                            \n\
	end                                                                         \n\
                                                                                \n\
	local lf = loadstring(code)	                                            	\n\
	local myenv = createSafeEnv()                                               \n\
    myenv.PersistentData = data                                                 \n\
	setfenv(lf, myenv)                                                          \n\
	local success, err = lpcall(maxCallNum, lf)							        \n\
	if not success then                                                         \n\
		return false, err                                                       \n\
	end                                                                         \n\
                                                                                \n\
	local ret		                                                            \n\
	local callfun = myenv[funcname]                                             \n\
	if type(callfun) == 'function' and funcname ~= 'init' then                  \n\
		ret = { lpcall(-1, callfun, ...) }							            \n\
		if not ret[1] then                                                      \n\
			return false, ret[2]  			                                    \n\
		else																	\n\
			local temp = {}														\n\
			for i = 2, #ret do												    \n\
				table.insert(temp, ret[i])									    \n\
			end																	\n\
			ret = temp														    \n\
		end																		\n\
	else                                                                        \n\
		return false, string.format('can not find function %s.', funcname)      \n\
	end                                                                         \n\
	                                                                            \n\
	local strPackData                                                           \n\
	if type(myenv.PersistentData) == 'table' then                               \n\
		strPackData = cmsgpack.pack(myenv.PersistentData)                       \n\
		local dataLen = string.len(strPackData)									\n\
		if dataLen > maxDataLen then											\n\
			return false, 'Lua:callContract dataLen > maxDataLen'				\n\
		end																		\n\
	end                                                                         \n\
	return true, strPackData, unpacktable(ret)				                    \n\
end                                                                             \n";

bool GetPubKey(const MCWallet* pWallet, const MagnaChainAddress& addr, MCPubKey& pubKey)
{
    MCKeyID key;
    if (!addr.GetKeyID(key))
        return false;

    return pWallet->GetPubKey(key, pubKey);
}

bool GenerateContractSender(const MCWallet* pWallet, MagnaChainAddress& sendAddr)
{
    std::vector<MCOutput> coins;
    pWallet->AvailableCoins(coins);
    if (coins.size() == 0)
        return false;

    MCTxDestination dest;
    const MCOutput& out = coins[0];
    const MCTxOut& txo = out.tx->tx->vout[out.i];
    ExtractDestination(txo.scriptPubKey, dest);
    sendAddr.Set(dest);
    return true;
}

bool GetSenderAddr(MCWallet* pWallet, const std::string& strSenderAddr, MagnaChainAddress& senderAddr)
{
    MCKeyID key;
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
// format: sender address keyid + block address + new magnachain address + contract script file hash
MCContractID GenerateContractAddress(MCWallet* pWallet, const MagnaChainAddress& senderAddr, const std::string& code)
{
    MCHashWriter ss(SER_GETHASH, 0);

    // sender address keyid
    MCKeyID senderId;
    senderAddr.GetKeyID(senderId);
    ss << senderId;

    // block address
    std::string blockAddress;
    if (chainActive.Height() < COINBASE_MATURITY)
        blockAddress = chainActive.Tip()->GetBlockHash().GetHex();
    else
        blockAddress = chainActive[chainActive.Height() - COINBASE_MATURITY]->GetBlockHash().GetHex();
    ss << blockAddress;

    // new magnachain address
    if (pWallet != nullptr) {
        MCPubKey newKey;
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
    return MCContractID(Hash160(ParseHex(ss.GetHash().ToString())));
}

void SetContractMsg(lua_State* L, const std::string& contractAddr, const std::string& origin, const std::string& sender, lua_Number payment, uint32_t blockTime, lua_Number blockHeight)
{
    // 创建msg表
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "msg");

    // 设置相关参数
    lua_pushstring(L, contractAddr.c_str());
    lua_setfield(L, -2, "thisaddress"); //合约本身的地址
    lua_pushstring(L, origin.c_str());
    lua_setfield(L, -2, "origin"); // 原始发起调用合约者公钥地址
    lua_pushstring(L, sender.c_str());
    lua_setfield(L, -2, "sender"); // 当前发起调用合约者地址(可能为合约或公钥地址)
    lua_pushnumber(L, payment);
    lua_setfield(L, -2, "payment"); //msg.value: number of wei sent with the message
    lua_pushnumber(L, blockTime);
    lua_setfield(L, -2, "timestamp");
    lua_pushnumber(L, blockHeight);
    lua_setfield(L, -2, "blockheight");
    lua_pop(L, 1);
}

std::string TrimCode(const std::string& rawCode)
{
    std::string line;
    std::string codeOut;

    int lineCount = 0;
    int leftCount = 0;
    size_t lastCodeOffset = 0;
    while (true) {
        size_t newCodeOffset = rawCode.find('\n', lastCodeOffset);
        if (newCodeOffset == std::string::npos) {
            if (lastCodeOffset >= rawCode.length())
                break;
            else
                newCodeOffset = rawCode.length();
        }
        std::string line = rawCode.substr(lastCodeOffset, newCodeOffset - lastCodeOffset);
        lineCount++;
        lastCodeOffset = newCodeOffset + 1;

        char symbol = 0;
        size_t pos = 0, start = 0;
        for (size_t pos = 0; pos < line.length(); ++pos) {
            size_t len = line.length();
            if (leftCount == 0 && line[pos] == '\"' || line[pos] == '\'') {
                if (symbol == line[pos])
                    symbol = 0;
                else
                    symbol = line[pos];
            }
            else if (symbol != 0 && line[pos] == '\\')
                ++pos;
            else if (symbol == 0 && line[pos] == '-' && pos + 1 < len && line[pos + 1] == '-') {
                if (pos + 3 < len) {
                    if (line[pos + 2] == '[' && line[pos + 3] == '[') {
                        if (++leftCount == 1)
                            start = pos;
                        pos += 3;
                    }
                    else if (leftCount == 0)
                        line = line.replace(pos, len - pos, "");
                }
                else if (leftCount == 0)
                    line = line.replace(pos, len - pos, "");
            }
            else if (leftCount > 0 && line[pos] == ']' && pos + 1 < len && line[pos + 1] == ']') {
                if (--leftCount == 0)
                    line = line.replace(start, pos - start + 4, " ");
                pos = start;
            }
        }

        if (leftCount > 0)
            line = line.replace(start, line.length() - start, "");

        int i = 0;
        for (; i < line.length(); ++i) {
            if (line[i] != ' ' && line[i] != '\t')
                break;
        }
        int j = line.length() - 1;
        for (; j >= 0; --j) {
            if (line[j] != ' ' && line[j] != '\t')
                break;
        }
        if (j > i) {
            char symbol2 = 0;
            for (int k = i; k <= j;) {
                if (line[k] == '\"' || line[k] == '\'') {
                    if (symbol2 == line[k])
                        symbol2 = 0;
                    else
                        symbol2 = line[k];
                    codeOut += line[k];
                    ++k;
                    continue;
                }
                else if (symbol2 != 0 && line[k] == '\\') {
                    codeOut += line[k];
                    if (k + 1 <= j)
                        codeOut += line[++k];
                    ++k;
                    continue;
                }
                if (symbol2 == 0) {
                    if (line[k] != ';') {
                        codeOut += line[k];
                        if (line[k] == ' ' || line[k] == '\t') {
                            for (k = k + 1; k <= j; ++k) {
                                if (line[k] != ' ' && line[k] != '\t')
                                    break;
                            }
                        }
                        else
                            ++k;
                    }
                    else
                        ++k;
                }
                else {
                    codeOut += line[k];
                    ++k;
                }
            }
        }
        codeOut += "\n";
    }

    return codeOut;
}

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
std::string Compress(const std::string& buffer)
{
    std::string zipData;
    boost::iostreams::filtering_ostream zout(boost::iostreams::zlib_compressor() | boost::iostreams::back_inserter(zipData));
    boost::iostreams::copy(boost::make_iterator_range(buffer), zout);
    return zipData;
}

std::string Decompress(const std::string& buffer)
{
    std::string unzipData;
    boost::iostreams::filtering_ostream uzout(boost::iostreams::zlib_decompressor() | boost::iostreams::back_inserter(unzipData));
    boost::iostreams::copy(boost::make_iterator_range(buffer), uzout);
    return unzipData;
}

bool PublishContract(lua_State* L, std::string& rawCode, long& maxCallNum, std::string& dataout, UniValue& ret)
{
    bool success = false;
    int top = lua_gettop(L);
    lua_getglobal(L, "regContract");
    lua_pushnumber(L, maxCallNum);
    lua_pushnumber(L, MAX_DATA_LEN);
    lua_pushlstring(L, rawCode.c_str(), rawCode.size());
    int argc = 3;

    if (lua_pcall(L, argc, LUA_MULTRET, 0) != 0) {
        const char* err = lua_tostring(L, -1);
        ret.push_back(strprintf("%s error: %s", __FUNCTION__, err));
    }
    else {
        maxCallNum = L->limit_instruction;
        success = lua_toboolean(L, top + 1) != 0;
        if (success) {
            size_t dl = 0;
            const char* temp = lua_tolstring(L, top + 2, &dl);
            dataout.assign(temp, dl);

            size_t cl = 0;
            temp = lua_tolstring(L, top + 3, &cl);
            rawCode.assign(temp, cl);
        }
        else {
            const char* err = lua_tostring(L, -1);
            ret.push_back(strprintf("%s error: %s", __FUNCTION__, err));
        }
    }

    lua_settop(L, top);
    return success;
}

bool PublishContract(SmartLuaState* sls, MagnaChainAddress& contractAddr, std::string& rawCode, UniValue& ret)
{
    MCContractID contractId;
    contractAddr.GetContractID(contractId);
    ContractInfo contractInfo;
    if (sls->GetContractInfo(contractId, contractInfo))
        throw std::runtime_error(strprintf("%s GetContractInfo fail", __FUNCTION__));

    std::string data;
    long maxCallNum = MAX_CONTRACT_CALL;
    lua_State* L = sls->GetLuaState(contractAddr);
    SetContractMsg(L, contractAddr.ToString(), sls->originAddr.ToString(), sls->originAddr.ToString(), 0, sls->timestamp, sls->blockHeight);
    bool success = PublishContract(L, rawCode, maxCallNum, data, ret);
    if (success) {
        sls->runningTimes = MAX_CONTRACT_CALL - maxCallNum;
        sls->codeLen = rawCode.size();
        sls->deltaDataLen = data.size();

        if (sls->saveType > 0) {
            contractInfo.txIndex = sls->txIndex;
            contractInfo.data = data;
            contractInfo.code = Compress(rawCode);
            sls->SetContractInfo(contractId, contractInfo, sls->saveType == SmartLuaState::SAVE_TYPE_CACHE);
        }
    }
    sls->ReleaseLuaState(L);

    return success;
}

bool PublishContract(SmartLuaState* sls, MCWallet* pWallet, const std::string& strSenderAddr, const std::string& rawCode, UniValue& ret)
{
    MagnaChainAddress senderAddr;
    if (!GetSenderAddr(pWallet, strSenderAddr, senderAddr))
        throw std::runtime_error("GetSenderAddr fail.");

    MCKeyID senderKey;
    MCPubKey senderPubKey;
    if (!senderAddr.GetKeyID(senderKey) || !pWallet->GetPubKey(senderKey, senderPubKey))
        throw std::runtime_error("Get Key or PubKey fail.");

    // temp addresss, replace in MCWallet::CreateTransaction
    std::string& trimRawCode = TrimCode(rawCode);
    if (trimRawCode.empty())
        throw std::runtime_error("code is empty");

    MCContractID contractId = GenerateContractAddress(pWallet, senderAddr, trimRawCode);
    MagnaChainAddress contractAddr(contractId);

    sls->Initialize(GetTime(), chainActive.Height() + 1, -1, senderAddr, nullptr, nullptr, 0, nullptr);
    bool success = PublishContract(sls, contractAddr, trimRawCode, ret);
    if (success) {
        MCScript scriptPubKey = GetScriptForDestination(contractAddr.Get());

        sls->contractIds.erase(contractId);
        MCWalletTx wtx;
        wtx.nVersion = MCTransaction::PUBLISH_CONTRACT_VERSION;
        wtx.pContractData.reset(new ContractData);
        wtx.pContractData->codeOrFunc = trimRawCode;
        wtx.pContractData->sender = senderPubKey;
        wtx.pContractData->address = contractId;
        wtx.pContractData->amountOut = 0;

        bool subtractFeeFromAmount = false;
        MCCoinControl coinCtrl;
        EnsureWalletIsUnlocked(pWallet);
        SendMoney(pWallet, scriptPubKey, 0, subtractFeeFromAmount, wtx, coinCtrl, sls);

        ret.setObject();
        ret.push_back(Pair("txid", wtx.tx->GetHash().ToString()));
        ret.push_back(Pair("contractaddress", MagnaChainAddress(wtx.tx->pContractData->address).ToString()));
        ret.push_back(Pair("senderaddress", senderAddr.ToString()));
    }

    return success;
}

bool CallContract(SmartLuaState* sls, MagnaChainAddress& contractAddr, const MCAmount amount, const std::string& strFuncName, const UniValue& args, long& maxCallNum, UniValue& ret)
{
    MCContractID contractID;
    if (!contractAddr.GetContractID(contractID))
        return false;

    bool success = CallContractReal(sls, contractAddr, amount, strFuncName, args, maxCallNum, ret);
    if (success) {
        sls->runningTimes = MAX_CONTRACT_CALL - maxCallNum;
        sls->codeLen = 0;
    }
    return success;
}

bool CallContractReal(SmartLuaState* sls, MagnaChainAddress& contractAddr, const MCAmount amount, const std::string& strFuncName, const UniValue& args, long& maxCallNum, UniValue& ret)
{
    if (amount < 0)
        throw std::runtime_error(strprintf("%s amount < 0", __FUNCTION__));

    MCContractID contractId;
    contractAddr.GetContractID(contractId);
    ContractInfo contractInfo;
    if (!sls->GetContractInfo(contractId, contractInfo) || contractInfo.code.size() <= 0)
        throw std::runtime_error(strprintf("%s GetContractInfo fail, contractid is %s", __FUNCTION__, contractAddr.ToString()));

    if (sls->_internalCallNum >= SmartLuaState::MAX_INTERNAL_CALL_NUM)
        throw std::runtime_error(strprintf("%s no more max internal call number", __FUNCTION__));

    sls->_internalCallNum++;
    std::string data;
    std::string senderAddr = (sls->contractAddrs.size() > 0 ? sls->contractAddrs[sls->contractAddrs.size() - 1].ToString() : sls->originAddr.ToString());
    lua_State* L = sls->GetLuaState(contractAddr);
    SetContractMsg(L, contractAddr.ToString(), sls->originAddr.ToString(), senderAddr, amount, sls->timestamp, sls->blockHeight);
    bool success = CallContract(L, contractInfo.code, contractInfo.data, strFuncName, args, maxCallNum, data, ret);
    if (success) {
        sls->deltaDataLen += std::max(0, (int32_t)(data.size() - contractInfo.data.size()));

        if (sls->saveType > 0) {
            contractInfo.txIndex = sls->txIndex;
            contractInfo.data = data;
            sls->SetContractInfo(contractId, contractInfo, sls->saveType == SmartLuaState::SAVE_TYPE_CACHE);
        }
    }
    sls->ReleaseLuaState(L);
    sls->_internalCallNum--;

    return success;
}

bool CallContract(lua_State* L, const std::string& rawCode, const std::string& data, const std::string& strFuncName, const UniValue& args, long& maxCallNum, std::string& dataout, UniValue& ret)
{
    const std::string& code = Decompress(rawCode);

    maxCallNum -= GAS_CONTRACT_BYTE;
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
    maxCallNum = L->limit_instruction;
    bool success = ((result == 0) && (lua_toboolean(L, top + 1) != 0));
    if (success) {
        size_t dl = 0;
        const char* temp = lua_tolstring(L, top + 2, &dl);
        dataout.assign(temp, dl);

        int newTop = lua_gettop(L);
        for (int i = top + 3; i <= newTop; ++i) {
            int t = lua_type(L, i);
            switch (t) {
            case LUA_TNUMBER:
                ret.push_back(UniValue((int64_t)lua_tonumber(L, i)));
                break;
            case LUA_TBOOLEAN:
                ret.push_back(UniValue((bool)lua_toboolean(L, i)));
                break;
            case LUA_TSTRING:
            {
                size_t sl = 0;
                const char* sv = lua_tolstring(L, i, &sl);
                ret.push_back(std::string(sv, sl));
                break;
            }
            default:
                ret.push_back(UniValue());
                break;
            }
        }
    }
    else {
        const char* err = lua_tostring(L, -1);
        ret.push_back(strprintf("%s error: %s", __FUNCTION__, err));
        LogPrintf("%s:%d %s\n", __FUNCTION__, __LINE__, err);
    }

    lua_settop(L, top);
    return success;
}

// Lua内部嵌套调用合约
int InternalCallContract(lua_State* L)
{
    SmartLuaState* sls = (SmartLuaState*)L->userData;
    if (sls == nullptr)
        throw std::runtime_error(strprintf("%s smartLuaState == nullptr", __FUNCTION__));

    std::string strContractAddr = lua_tostring(L, 1);
    MagnaChainAddress contractAddr(strContractAddr);
    if (!contractAddr.IsValid())
        throw std::runtime_error(strprintf("%s contractAddr is invalid", __FUNCTION__));

    std::string strFuncName = lua_tostring(L, 2);
    if (strFuncName.empty())
        throw std::runtime_error(strprintf("%s function name is empty", __FUNCTION__));

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

    UniValue ret(UniValue::VARR);
    long maxCallNum = L->limit_instruction;
    bool success = CallContractReal(sls, contractAddr, 0, strFuncName, args, maxCallNum, ret);
    L->limit_instruction = maxCallNum;
    if (!success)
        throw std::runtime_error(ret[0].get_str().c_str());

    lua_pushboolean(L, (int)success);
    for (int i = 0; i < ret.size(); ++i) {
        switch (ret[i].type()) {
        case UniValue::VNUM:
            lua_pushnumber(L, ret[i].get_int64());
            break;
        case UniValue::VBOOL:
            lua_pushboolean(L, ret[i].get_bool());
            break;
        case UniValue::VSTR:
            lua_pushstring(L, ret[i].get_str().c_str());
            break;
        default:
            lua_pushnil(L);
            break;
        }
    }
    return ret.size() + 1;
}

// Lua内部向指定地址发送代币
int SendCoins(lua_State* L)
{
    SmartLuaState* sls = (SmartLuaState*)L->userData;
    if (sls == nullptr)
        throw std::runtime_error(strprintf("%s smartLuaState == nullptr", __FUNCTION__));

    if (!lua_isstring(L, 1))
        throw std::runtime_error(strprintf("%s param1 is not a string", __FUNCTION__));

    if (!lua_isnumber(L, 2))
        throw std::runtime_error(strprintf("%s param2 is not a number", __FUNCTION__));

    if (sls->pCoinAmountCache == nullptr)
        throw std::runtime_error(strprintf("%s smartLuaState == nullptr", __FUNCTION__));

    std::string strDest = lua_tostring(L, 1);
    MagnaChainAddress kDest(strDest);
    if (kDest.IsContractID() || !kDest.IsValid())
        throw std::runtime_error(strprintf("Invalid destination address"));

    MCAmount amount = lua_tonumber(L, 2);
    if (amount < DUST_RELAY_TX_FEE)
        throw std::runtime_error(strprintf("%s Dust amount", __FUNCTION__));

    MCContractID contractID;
    sls->contractAddrs[0].GetContractID(contractID);
    MCAmount totalAmount = sls->pCoinAmountCache->GetAmount(contractID);
    if (sls->contractOut + amount > totalAmount)
        throw std::runtime_error(strprintf("Contract %s has not enough amount", contractID.ToString().c_str()));

    MCTxOut out;
    out.nValue = amount;
    out.scriptPubKey = GetScriptForDestination(kDest.Get());

    sls->recipients.emplace_back(out);
    sls->contractOut += amount;

    lua_pushboolean(L, true);
    return 1;
}

void SmartLuaState::Initialize(int64_t timestamp, int blockHeight, int txIndex, MagnaChainAddress& originAddr, ContractContext* pContractContext, MCBlockIndex* pPrevBlockIndex, int saveType, CoinAmountCache* pCoinAmountCache)
{
    Clear();

    this->timestamp = timestamp;
    this->blockHeight = blockHeight;
    this->txIndex = txIndex;
    this->originAddr = originAddr;
    this->_pContractContext = pContractContext;
    this->_pPrevBlockIndex = pPrevBlockIndex;
    this->saveType = saveType;
    this->pCoinAmountCache = pCoinAmountCache;

    if (_pContractContext == nullptr) {
        _pContractContext = &mpContractDb->contractContext;
    }
}

lua_State* SmartLuaState::GetLuaState(MagnaChainAddress& contractAddr)
{
    lua_State* L = nullptr;
    if (_luaStates.size() > 0) {
        L = _luaStates.back();
        _luaStates.pop();
        lua_settop(L, 0);
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

        L->userData = this;
    }

    MCContractID contractId;
    contractAddr.GetContractID(contractId);
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

void SmartLuaState::Clear()
{
    saveType = SAVE_TYPE_NONE;
    timestamp = 0;
    blockHeight = -1;
    txIndex = -1;
    contractOut = 0;
    runningTimes = 0;
    deltaDataLen = 0;
    codeLen = 0;
    _internalCallNum = 0;
    pCoinAmountCache = nullptr;
    _pContractContext = nullptr;
    _pPrevBlockIndex = nullptr;
    recipients.clear();
    contractIds.clear();
    contractAddrs.clear();
    contractDataFrom.clear();
}

void SmartLuaState::SetContractInfo(const MCContractID& contractId, ContractInfo& contractInfo, bool cache)
{
    LOCK(_contractCS);
    if (cache)
        _pContractContext->SetCache(contractId, contractInfo);
    else
        _pContractContext->SetData(contractId, contractInfo);
}

bool SmartLuaState::GetContractInfo(const MCContractID& contractId, ContractInfo& contractInfo)
{
    LOCK(_contractCS);

    // 直接从快照缓存中读取
    if (!_pContractContext->GetData(contractId, contractInfo)) {
        if (mpContractDb->GetContractInfo(contractId, contractInfo, _pPrevBlockIndex) < 0)
            return false;
    }

    if (contractDataFrom.count(contractId) == 0)
        contractDataFrom[contractId] = contractInfo;

    return true;
}

bool ExecuteContract(SmartLuaState* sls, const MCTransactionRef tx, int txIndex, MCAmount coins, int64_t blockTime, int blockHeight, MCBlockIndex* pPrevBlockIndex, ContractContext* pContractContext)
{
    const MCContractID& contractId = tx->pContractData->address;
    MagnaChainAddress contractAddr(contractId);
    MagnaChainAddress senderAddr(tx->pContractData->sender.GetID());
    MCAmount amount = GetTxContractOut(*tx);

    CoinAmountTemp coinAmountTemp;
    coinAmountTemp.IncAmount(contractId, coins);
    CoinAmountCache coinAmountCache(&coinAmountTemp);

    UniValue ret(UniValue::VARR);
    if (tx->nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION) {
        std::string rawCode = tx->pContractData->codeOrFunc;
        sls->Initialize(blockTime, blockHeight, txIndex, senderAddr, pContractContext, pPrevBlockIndex, SmartLuaState::SAVE_TYPE_CACHE, nullptr);
        if (!PublishContract(sls, contractAddr, rawCode, ret))
            return false;
    }
    else if (tx->nVersion == MCTransaction::CALL_CONTRACT_VERSION) {
        const std::string& strFuncName = tx->pContractData->codeOrFunc;
        UniValue args;
        args.read(tx->pContractData->args);

        long maxCallNum = MAX_CONTRACT_CALL;
        sls->Initialize(blockTime, blockHeight, txIndex, senderAddr, pContractContext, pPrevBlockIndex, SmartLuaState::SAVE_TYPE_CACHE, &coinAmountCache);
        if (!CallContract(sls, contractAddr, amount, strFuncName, args, maxCallNum, ret) || tx->pContractData->amountOut != sls->contractOut)
            return false;

        if (tx->pContractData->amountOut > 0 && sls->recipients.size() == 0)
            return false;

        MCAmount total = 0;
        for (int j = 0; j < sls->recipients.size(); ++j) {
            if (!tx->IsExistVout(sls->recipients[j]))
                return false;
            total += sls->recipients[j].nValue;
        }

        if (total != tx->pContractData->amountOut) {
            return false;
        }

        // 这里只执行一次合约调用，所以不需要更新coinAmountCache
    }

    pContractContext->txFinalData[txIndex].data = pContractContext->cache;
    pContractContext->Commit();
    return true;
}

// 只在主链执行分支的智能合约
bool ExecuteBlock(SmartLuaState* sls, MCBlock* pBlock, MCBlockIndex* pPrevBlockIndex, int offset, int count, ContractContext* pContractContext)
{
    std::map<MCContractID, uint256> contract2txid;
    pContractContext->txFinalData.resize(pBlock->vtx.size());
    for (int i = offset; i < offset + count; ++i) {
        const MCTransactionRef tx = pBlock->vtx[i];
        if (tx->IsNull()) {
            return false;
        }

        assert(!tx->GetHash().IsNull());
        if (tx->IsSmartContract()) {
            if (i >= pBlock->prevContractData.size()) {
                return false;
            }
            ExecuteContract(sls, tx, i, pBlock->prevContractData[i].coins, pBlock->GetBlockTime(), pPrevBlockIndex->nHeight + 1, pPrevBlockIndex, pContractContext);
        }
    }

    return true;
}

uint256 GetTxHashWithData(const uint256& txHash, const CONTRACT_DATA& contractData)
{
    MCHashWriter ss(SER_GETHASH, 0);
    ss << txHash;
    for (auto item : contractData) {
        ss << item.first << item.second.txIndex << item.second.code << item.second.data;
    }
    return ss.GetHash();
}

uint256 GetTxHashWithPrevData(const uint256& txHash, const ContractPrevData& contractPrevData)
{
    MCHashWriter ss(SER_GETHASH, 0);
    ss << txHash << contractPrevData;
    return ss.GetHash();
}

bool VecTxMerkleLeavesWithData(const std::vector<MCTransactionRef>& vtx, const std::vector<ContractTxFinalData>& contractData, std::vector<uint256>& leaves)
{
    if (vtx.size() != contractData.size()) {
        return false;
    }
    leaves.resize(vtx.size());
    for (size_t i = 0; i < vtx.size(); ++i) {
        leaves[i] = GetTxHashWithData(vtx[i]->GetHash(), contractData[i].data);
    }
    return true;
}

bool VecTxMerkleLeavesWithPrevData(const std::vector<MCTransactionRef>& vtx, const std::vector<ContractPrevData>& contractData, std::vector<uint256>& leaves)
{
    if (vtx.size() != contractData.size()) {
        return false;
    }
    leaves.resize(vtx.size(), uint256());
    for (size_t i = 0; i < vtx.size(); ++i) {
        leaves[i] = GetTxHashWithPrevData(vtx[i]->GetHash(), contractData[i]);
    }
    return true;
}

uint256 BlockMerkleRootWithData(const MCBlock& block, const ContractContext& contractContext, bool*mutated)
{
    std::vector<uint256> leaves;
    if (!VecTxMerkleLeavesWithData(block.vtx, contractContext.txFinalData, leaves)) {
        return uint256();
    }
    return ComputeMerkleRoot(leaves, mutated);
}

uint256 BlockMerkleRootWithPrevData(const MCBlock& block, bool* mutated)
{
    std::vector<uint256> leaves;
    if (!VecTxMerkleLeavesWithPrevData(block.vtx, block.prevContractData, leaves)) {
        return uint256();
    }
    return ComputeMerkleRoot(leaves, mutated);
}