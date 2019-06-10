// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "coding/base58.h"
#include "consensus/consensus.h"
#include "consensus/merkle.h"
#include "mining/miner.h"
#include "policy/policy.h"
#include "primitives/transaction.h"
#include "script/standard.h"
#include "transaction/txmempool.h"
#include "utils/util.h"
#include "validation/validation.h" //for mempool access
#include "vm/contractdb.h"
#include "vm/contractvm.h"
#include "wallet/coincontrol.h"
#include "wallet/rpcwallet.h"

extern "C"
{
#include "lua/lvm.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/ldebug.h"
}

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>

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
function regContract(maxDataLen, _strScript)						            \n\
	local contract, err = loadstring(_strScript)                                \n\
	if err then                                                                 \n\
        print(_strScript)                                                       \n\
		return false, err                                                       \n\
	end                                                                         \n\
	                                                                            \n\
    local success                                                               \n\
	local myenv = createSafeEnv()                                               \n\
	setfenv(contract, myenv)                                                    \n\
	success, err = pcall(contract)							                    \n\
	if not success then                                                         \n\
		return false, err                                  						\n\
	end                                                                         \n\
	                                                                            \n\
	if type(myenv.init) == 'function' then                                      \n\
		success, err = pcall(myenv.init)						                \n\
		if not success then                                                     \n\
			return false, err					                                \n\
		end                                                                     \n\
	end                                                                         \n\
	                                                                            \n\
	local strCompileFun = string.dump(contract)                                 \n\
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
function callContract(maxDataLen, code, data, funcname, ...)	                \n\
	if data and #data >0 then                                                   \n\
		data = cmsgpack.unpack(data)                                            \n\
	end                                                                         \n\
                                                                                \n\
	myenv = createSafeEnv()                                                     \n\
    if myenv[funcname] ~= nil then                                              \n\
        myenv = nil                                                             \n\
        return false, 'can not call lua internal function directly'             \n\
    end                                                                         \n\
                                                                                \n\
	local contract = loadstring(code)	                                        \n\
    myenv.PersistentData = data                                                 \n\
	setfenv(contract, myenv)                                                    \n\
	local success, err = pcall(contract)							            \n\
	if not success then                                                         \n\
        myenv = nil                                                             \n\
		return false, err                                                       \n\
	end                                                                         \n\
                                                                                \n\
	local ret		                                                            \n\
	local func = myenv[funcname]                                                \n\
	if type(func) == 'function' and funcname ~= 'init' then                     \n\
		ret = { pcall(func, ...) }							                    \n\
		if not ret[1] then                                                      \n\
            myenv = nil                                                         \n\
			return false, ret[2]  			                                    \n\
		else																	\n\
			local temp = {}														\n\
			for i = 2, #ret do												    \n\
				table.insert(temp, ret[i])									    \n\
			end																	\n\
			ret = temp														    \n\
		end																		\n\
	else                                                                        \n\
        myenv = nil                                                             \n\
		return false, string.format('can not find function %s.', funcname)      \n\
	end                                                                         \n\
	                                                                            \n\
	local strPackData                                                           \n\
	if type(myenv.PersistentData) == 'table' then                               \n\
		strPackData = cmsgpack.pack(myenv.PersistentData)                       \n\
		local dataLen = string.len(strPackData)									\n\
		if dataLen > maxDataLen then											\n\
            myenv = nil                                                         \n\
			return false, 'Lua:callContract dataLen > maxDataLen'				\n\
		end																		\n\
	end                                                                         \n\
    myenv = nil                                                                 \n\
	return true, strPackData, unpack(ret)				                        \n\
end                                                                             \n\
                                                                                \n\
function callExistContract(funcname, ...)	                                    \n\
    local temp = createSafeEnv()                                                \n\
    if temp[funcname] ~= nil then                                               \n\
        return false, 'can not call lua internal function directly'             \n\
    end                                                                         \n\
	local func = myenv[funcname]                                                \n\
	if type(func) == 'function' and funcname ~= 'init' then                     \n\
		ret = { pcall(func, ...) }							                    \n\
		if not ret[1] then                                                      \n\
			return false, ret[2]  			                                    \n\
		else																	\n\
			local temp = {}														\n\
			for i = 2, #ret do												    \n\
				table.insert(temp, ret[i])									    \n\
			end																	\n\
			return true, unpack(temp)								            \n\
		end																		\n\
	else                                                                        \n\
		return false, string.format('can not find function %s.', funcname)      \n\
	end                                                                         \n\
end                                                                             \n"
;

std::string static CompressCode(const std::string& buffer)
{
    std::string zipData;
    boost::iostreams::filtering_ostream zout(boost::iostreams::zlib_compressor() | boost::iostreams::back_inserter(zipData));
    boost::iostreams::copy(boost::make_iterator_range(buffer), zout);
    return zipData;
}

std::string static DecompressCode(const std::string& buffer)
{
    std::string unzipData;
    boost::iostreams::filtering_ostream uzout(boost::iostreams::zlib_decompressor() | boost::iostreams::back_inserter(unzipData));
    boost::iostreams::copy(boost::make_iterator_range(buffer), uzout);
    return unzipData;
}

int GetDeltaDataLen(const VMOut* vmOut)
{
    uint32_t deltaDataLen = 0;
    for (const auto& item : vmOut->txFinalData) {
        int prevSize = 0;
        auto prevIt = vmOut->txPrevData.find(item.first);
        if (prevIt != vmOut->txPrevData.end()) {
            prevSize = prevIt->second.data.size();
        }
        deltaDataLen += (int)item.second.data.size() - prevSize;
    }
    return deltaDataLen;
}

uint256 BlockMerkleLeavesWithPrevData(const MCBlock* pBlock, const std::vector<VMOut>& vmOuts, std::vector<uint256>& leaves, bool* mutated)
{
    assert(pBlock->vtx.size() == vmOuts.size());
    leaves.resize(pBlock->vtx.size());
    for (size_t i = 0; i < pBlock->vtx.size(); ++i) {
        leaves[i] = GetTxHashWithData(pBlock->vtx[i]->GetHash(), vmOuts[i].txPrevData);
    }
    return ComputeMerkleRoot(leaves, mutated);
}

uint256 BlockMerkleLeavesWithFinalData(const MCBlock* pBlock, const std::vector<VMOut>& vmOuts, std::vector<uint256>& leaves, bool* mutated)
{
    assert(pBlock->vtx.size() == vmOuts.size());
    leaves.resize(pBlock->vtx.size());
    for (size_t i = 0; i < pBlock->vtx.size(); ++i) {
        leaves[i] = GetTxHashWithData(pBlock->vtx[i]->GetHash(), vmOuts[i].txFinalData);
    }
    return ComputeMerkleRoot(leaves, mutated);
}

static bool PublishContract(lua_State* L, std::string& rawCode, long& maxCallNum, std::string& dataout, UniValue& ret)
{
    int top = lua_gettop(L);

    lua_getglobal(L, "regContract");
    lua_pushnumber(L, MAX_DATA_LEN);
    lua_pushlstring(L, rawCode.c_str(), rawCode.size());
    int argc = 2;

    int result = lua_pcall(L, argc, LUA_MULTRET, 0);
    bool success = ((result == 0) && (lua_toboolean(L, top + 1) != 0));
    if (success) {
        size_t dl = 0;
        const char* temp = lua_tolstring(L, top + 2, &dl);
        dataout.assign(temp, dl);

        size_t cl = 0;
        temp = lua_tolstring(L, top + 3, &cl);
        rawCode.assign(temp, cl);
    } else {
        const char* err = lua_tostring(L, -1);
        if (err != nullptr) {
            ret.push_back(strprintf("%s error: %s", __FUNCTION__, err));
        }
    }

    lua_settop(L, top);
    return success;
}

static bool CallContract(lua_State* L, const std::string& rawCode, const std::string& data, const std::string& strFuncName, const UniValue& args, std::string& dataout, UniValue& ret)
{
    const std::string& code = DecompressCode(rawCode);

    L->limit_instruction -= GAS_CONTRACT_BYTE;
    int top = lua_gettop(L);

    lua_getglobal(L, "callContract");
    lua_pushnumber(L, MAX_DATA_LEN);
    lua_pushlstring(L, code.c_str(), code.size());
    if (data.size() > 0) {
        lua_pushlstring(L, data.c_str(), data.size());
    } else {
        lua_pushnil(L);
    }
    lua_pushstring(L, strFuncName.c_str());
    int argc = 4;

    /* other use params */
    for (size_t i = 0; i < args.size(); ++i) {
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
            case LUA_TSTRING: {
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
    } else {
        const char* err = lua_tostring(L, -1);
        if (err != nullptr) {
            ret.push_back(strprintf("%s error: %s", __FUNCTION__, err));
            LogPrintf("%s:%d %s\n", __FUNCTION__, __LINE__, err);
        }
    }

    lua_settop(L, top);
    return success;
}

static bool CallExistContract(lua_State* L, const std::string& strFuncName, const UniValue& args, UniValue& ret)
{
    L->limit_instruction -= GAS_CONTRACT_BYTE;
    int top = lua_gettop(L);

    lua_getglobal(L, "callExistContract");
    lua_pushstring(L, strFuncName.c_str());
    int argc = 1;

    /* other use params */
    for (size_t i = 0; i < args.size(); ++i) {
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
    bool success = ((result == 0) && (lua_toboolean(L, top + 1) != 0));
    if (success) {
        int newTop = lua_gettop(L);
        for (int i = top + 2; i <= newTop; ++i) {
            int t = lua_type(L, i);
            switch (t) {
            case LUA_TNUMBER:
                ret.push_back(UniValue((int64_t)lua_tonumber(L, i)));
                break;
            case LUA_TBOOLEAN:
                ret.push_back(UniValue((bool)lua_toboolean(L, i)));
                break;
            case LUA_TSTRING: {
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
        if (err != nullptr) {
            ret.push_back(strprintf("%s error: %s", __FUNCTION__, err));
            LogPrintf("%s:%d %s\n", __FUNCTION__, __LINE__, err);
        }
    }

    lua_settop(L, top);
    return success;
}

int ContractVM::InternalCallContract(lua_State* L)
{
    ContractVM* vm = (ContractVM*)L->userData;
    if (vm == nullptr) {
        throw std::runtime_error(strprintf("%s => smartLuaState == nullptr", __FUNCTION__));
    }

    if (vm->IsPublish()) {
        throw std::runtime_error(strprintf("%s => can't call callcontract when publishcontract", __FUNCTION__));
    }

    std::string strContractAddr = lua_tostring(L, 1);
    MagnaChainAddress contractAddr(strContractAddr);
    if (!contractAddr.IsValid()) {
        throw std::runtime_error(strprintf("%s => contractAddr is invalid", __FUNCTION__));
    }

    std::string strFuncName = lua_tostring(L, 2);
    if (strFuncName.empty()) {
        throw std::runtime_error(strprintf("%s => function name is empty", __FUNCTION__));
    }

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

    long maxCallNum = L->limit_instruction;
    bool success = vm->CallContract(contractAddr, strFuncName, args, maxCallNum);
    L->limit_instruction = maxCallNum;
    if (!success) {
        throw std::runtime_error(strprintf(vm->vmOut->ret[0].get_str().c_str()));
    }

    UniValue& ret = vm->vmOut->ret;
    size_t sz = ret.size() + 1;
    lua_pushboolean(L, (int)success);
    for (size_t i = 0; i < ret.size(); ++i) {
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
    ret.setArray();

    return sz + 1;
}

int ContractVM::SendCoins(lua_State* L)
{
    ContractVM* vm = (ContractVM*)L->userData;
    if (vm == nullptr) {
        throw std::runtime_error(strprintf("%s => smartLuaState == nullptr", __FUNCTION__));
    }

    if (vm->IsPublish()) {
        throw std::runtime_error(strprintf("%s => can't call send when publishcontract", __FUNCTION__));
    }

    if (!lua_isstring(L, 1)) {
        throw std::runtime_error(strprintf("%s => param1 is not a string", __FUNCTION__));
    }

    std::string strDest(lua_tostring(L, 1));
    MagnaChainAddress dest(strDest);
    if (!dest.IsValid() || dest.IsContractID()) {
        throw std::runtime_error(strprintf("%s => Invalid destination address", __FUNCTION__));
    }

    if (!lua_isnumber(L, 2)) {
        throw std::runtime_error(strprintf("%s => param2 is not a number", __FUNCTION__));
    }

    MCAmount amount = lua_tonumber(L, 2);
    if (amount <= 0 || amount > MAX_MONEY) {
        throw std::runtime_error(strprintf("%s => amount(%lld) out of range", __FUNCTION__, amount));
    }

    const MCContractID contractId = vm->GetCurrentContractID();
    ContractContext context;
    if (!vm->GetContractContext(contractId, context)) {
        throw std::runtime_error(strprintf("%s => GetContractInfo fail", __FUNCTION__));
    }

    MCAmount totalAmount = vm->GetContractCoins(contractId);
    MCAmount coinsOut = vm->GetContractCoinOut(contractId);
    if (coinsOut + amount > totalAmount) {
        throw std::runtime_error(strprintf("%s => Contract %s has not enough amount", __FUNCTION__, contractId.ToString().c_str()));
    }
    vm->IncContractCoinsOut(contractId, amount);
    vm->AddRecipient(amount, GetScriptForDestination(dest.Get()));

    lua_pushboolean(L, true);
    return 1;
}

const MCContractID ContractVM::GetCurrentContractID()
{
    if (contractAddrs.size() == 0) {
        return MCContractID();
    }

    MCContractID contractId;
    const MagnaChainAddress& contractAddr = contractAddrs[contractAddrs.size() - 1];
    contractAddr.GetContractID(contractId);
    return contractId;
}

void ContractVM::AddRecipient(MCAmount amount, const MCScript& scriptPubKey)
{
    vmOut->recipients.emplace_back(amount, scriptPubKey);
}

bool ContractVM::PublishContract(const MagnaChainAddress& contractAddr, const std::string& rawCode, bool decompress)
{
    std::string code = (decompress ? DecompressCode(rawCode) : rawCode);
    if (code.empty()) {
        throw std::runtime_error("code is empty");
    }

    if (code.size() > MAX_CONTRACT_FILE_LEN) {
        throw std::runtime_error("code is too large");
    }

    MCContractID contractId;
    contractAddr.GetContractID(contractId);
    ContractContext context;
    if (GetContractContext(contractId, context)) {
        throw std::runtime_error(strprintf("%s GetContractInfo fail", __FUNCTION__));
    }

    std::string data;
    long maxCallNum = MAX_CONTRACT_CALL;
    lua_State* L = GetLuaState(contractAddr, nullptr);
    L->limit_instruction = maxCallNum;
    SetMsgField(L, false);
    bool success = ::PublishContract(L, code, maxCallNum, data, vmOut->ret);
    maxCallNum = L->limit_instruction;
    if (success) {
        vmOut->runningTimes = MAX_CONTRACT_CALL - maxCallNum;
        context.data = data;
        context.coins = vmIn.payment;
        context.code = CompressCode(code);
        SetContractContext(contractId, context);
    }
    ReleaseLuaState(L);

    return success;
}

bool ContractVM::CallContract(const MagnaChainAddress& contractAddr, const std::string& strFuncName, const UniValue& args, long& maxCallNum)
{
    if (args.size() > 12) {
        throw std::runtime_error("Too many args in lua function, max num is 12");
    }

    MCContractID contractId;
    contractAddr.GetContractID(contractId);
    ContractContext context;
    if (!GetContractContext(contractId, context)) {
        LogPrintf("%s:%d GetContractInfo fail %s %s\n", __FUNCTION__, __LINE__, contractId.ToString(), contractAddr.ToString());
        throw std::runtime_error(strprintf("%s => GetContractInfo fail, contractid is %s", __FUNCTION__, contractAddr.ToString()));
    }

    if (context.code.size() <= 0) {
        throw std::runtime_error(strprintf("%s => contract code size <= 0, contractid is %s", __FUNCTION__, contractAddr.ToString()));
    }

    bool exist = false;
    lua_State* L = GetLuaState(contractAddr, &exist);
    if (L == nullptr) {
        throw std::runtime_error(strprintf("%s => too many internal calls"));
    }

    std::string data;
    L->limit_instruction = maxCallNum;
    bool success = false;
    if (!exist) {
        SetMsgField(L, false);
        success = ::CallContract(L, context.code, context.data, strFuncName, args, data, vmOut->ret);
    }
    else {
        SetMsgField(L, false);
        success = ::CallExistContract(L, strFuncName, args, vmOut->ret);
        SetMsgField(L, true);
    }
    maxCallNum = L->limit_instruction;
    if (success) {
        if (!exist) {
            context.data = std::move(data);
            context.coins = GetContractCoins(contractId) - GetContractCoinOut(contractId);
            SetContractContext(contractId, context);
        }
    }
    ReleaseLuaState(L);

    return success;
}

bool ContractVM::CallContract(const MagnaChainAddress& contractAddr, const std::string& strFuncName, const UniValue& args)
{
    MCContractID contractId;
    if (!contractAddr.GetContractID(contractId)) {
        return false;
    }

    long maxCallNum = MAX_CONTRACT_CALL;
    bool success = CallContract(contractAddr, strFuncName, args, maxCallNum);
    if (success) {
        if (vmIn.payment > 0) {
            ContractContext context;
            if (GetContractContext(contractId, context)) {
                context.coins += vmIn.payment;
                SetContractContext(contractId, context);
            }
        }
        vmOut->runningTimes = MAX_CONTRACT_CALL - maxCallNum;
    }
    return success;
}

ContractVM::~ContractVM()
{
    cache.clear();
    data.clear();
    while (luaStates.size()) {
        lua_State* L = luaStates.front();
        luaStates.pop();
        L->userData = nullptr;
        lua_close(L);
    }
}

void ContractVM::Initialize(const VMIn* vmIn, VMOut* vmOut)
{
    assert(vmOut != nullptr);
    for (auto& item : usingLuaStates) {
        lua_State* L = item.second;
        lua_settop(L, 0);
        lua_gc(L, LUA_GCCOLLECT, 0);
        luaStates.push(L);
    }
    cache.clear();
    contractAddrs.clear();
    usingLuaStates.clear();
    vmOut->ret.setArray();
    this->vmIn.Copy(*vmIn);
    this->vmOut = vmOut;
}

lua_State* ContractVM::GetLuaState(const MagnaChainAddress& contractAddr, bool* exist)
{
    if (contractAddrs.size() >= ContractVM::MAX_INTERNAL_CALL_NUM) {
        return NULL;
    }

    MCContractID contractId;
    contractAddr.GetContractID(contractId);

    lua_State* L = nullptr;
    auto iter = usingLuaStates.find(contractAddr);
    if (iter != usingLuaStates.end()) {
        L = iter->second;
        if (exist != nullptr) {
            *exist = true;
        }
    }
    else {
        if (luaStates.size() > 0) {
            L = luaStates.front();
            luaStates.pop();
        }
        else {
            L = lua_open();
            if (L == nullptr) {
                error("cannot create state: not enough memory\n");
                return nullptr;
            }
            
            int top = lua_gettop(L);
            luaL_openlibs(L);
            luaopen_cmsgpack(L);

            if (luaL_dostring(L, initscript)) {
                error("%s\n", lua_tostring(L, -1));
                return nullptr;
            }

            lua_pushcfunction(L, InternalCallContract);
            lua_setglobal(L, "callcontract");
            lua_pushcfunction(L, SendCoins);
            lua_setglobal(L, "send");
            assert(lua_gettop(L) == top);

            L->userData = this;
            L->limit_on = 1;
        }

        if (exist != nullptr) {
            *exist = false;
        }

        usingLuaStates[contractAddr] = L;
    }
    contractAddrs.emplace_back(contractAddr);

    return L;
}

void ContractVM::ReleaseLuaState(lua_State* L)
{
    MagnaChainAddress& contractAddr = contractAddrs[contractAddrs.size() - 1];

    bool found = false;
    for (size_t i = 0; i < contractAddrs.size() - 1; ++i) {
        if (contractAddrs[i] == contractAddr) {
            found = true;
            break;
        }
    }

    if (!found) {
        lua_settop(L, 0);
        lua_gc(L, LUA_GCCOLLECT, 0);
        usingLuaStates.erase(contractAddr);
        luaStates.push(L);
    }

    assert(contractAddrs.size() > 0);
    contractAddrs.resize(contractAddrs.size() - 1);
}

void ContractVM::SetMsgField(lua_State* L, bool rollBackLast)
{
    MagnaChainAddress& contractAddr = contractAddrs[contractAddrs.size() - 1];
    std::string senderAddr;
    if (rollBackLast) {
        for (int i = contractAddrs.size() - 2; i >= 0; i--) {
            if (contractAddrs[i] == contractAddr) {
                senderAddr = i > 0 ? contractAddrs[i - 1].ToString() : vmIn.vmCaller.ToString();
                break;
            }
        }
    }
    else {
        senderAddr = (contractAddrs.size() > 1 ? contractAddrs[contractAddrs.size() - 2].ToString() : vmIn.vmCaller.ToString());
    }

    int top = lua_gettop(L);
    lua_newtable(L);
    lua_pushvalue(L, -1); // copy
    lua_setglobal(L, "msg");

    // set Msg fields
    lua_pushstring(L, contractAddr.ToString().c_str());
    lua_setfield(L, -2, "thisaddress");
    lua_pushstring(L, vmIn.vmCaller.ToString().c_str());
    lua_setfield(L, -2, "origin");
    lua_pushstring(L, senderAddr.c_str());
    lua_setfield(L, -2, "sender");
    lua_pushnumber(L, vmIn.payment);
    lua_setfield(L, -2, "payment");
    lua_pushnumber(L, vmIn.prevBlockIndex->GetBlockTime());
    lua_setfield(L, -2, "timestamp");
    lua_pushnumber(L, vmIn.prevBlockIndex->nHeight + 1);
    lua_setfield(L, -2, "blockheight");
    lua_settop(L, top);
}

void ContractVM::SetContractContext(const MCContractID& contractId, ContractContext& context)
{
    context.txIndex = vmIn.txIndex;
    SetData(contractId, context);
    vmOut->txFinalData[contractId] = context;
}

bool ContractVM::GetContractContext(const MCContractID& contractId, ContractContext& context)
{
    // 直接从快照缓存中读取
    if (!GetData(contractId, context)) {
        if (mpContractDb->GetContractContext(contractId, context, vmIn.prevBlockIndex) < 0) {
            return false;
        }
    }
    
    if (vmOut->txPrevData.count(contractId) == 0) {
        vmOut->txPrevData[contractId] = context;
    }

    return true;
}

MCAmount ContractVM::GetContractCoins(const MCContractID& contractId)
{
    auto it = vmOut->txPrevData.find(contractId);
    assert(it != vmOut->txPrevData.end());
    return it->second.coins;
}

MCAmount ContractVM::GetContractCoinOut(const MCContractID& contractId)
{
    auto it = vmOut->contractCoinsOut.find(contractId);
    return (it == vmOut->contractCoinsOut.end() ? 0 : it->second);
}

MCAmount ContractVM::IncContractCoinsOut(const MCContractID& contractId, MCAmount delta)
{
    assert(!contractId.IsNull() && MoneyRange(delta));
    MCAmount value = GetContractCoinOut(contractId) + delta;
    vmOut->contractCoinsOut[contractId] = value;
    return value;
}

const MapContractContext& ContractVM::GetAllData() const
{
    return data;
}

void ContractVM::CommitData()
{
    for (auto it : cache)
        data[it.first] = std::move(it.second);
    cache.clear();
}

void ContractVM::ClearData(bool onlyCache)
{
    cache.clear();
    if (!onlyCache) {
        data.clear();
    }
}

void ContractVM::SetData(const MCContractID& contractId, const ContractContext& context)
{
    cache[contractId] = std::move(context);
}

bool ContractVM::GetData(const MCContractID& contractId, ContractContext& context)
{
    if (cache.size() > 0) {
        auto it = cache.find(contractId);
        if (it != cache.end()) {
            context.txIndex = it->second.txIndex;
            context.coins = it->second.coins;
            context.code = it->second.code;
            context.data = it->second.data;
            return true;
        }
    }

    auto it = data.find(contractId);
    if (it != data.end()) {
        context.txIndex = it->second.txIndex;
        context.coins = it->second.coins;
        context.code = it->second.code;
        context.data = it->second.data;
        return true;
    }

    return false;
}

bool ContractVM::ExecuteContract(const MCTransactionRef tx, int txIndex, const MCBlockIndex* prevBlockIndex, VMOut* vmOut)
{
    if (!tx->IsSmartContract()) {
        return true;
    }

    MagnaChainAddress contractAddr(tx->pContractData->address);
    MCAmount payment = tx->pContractData->contractCoinsIn;
    MagnaChainAddress vmCaller(tx->pContractData->sender.GetID());

    VMIn vmIn{ txIndex, payment, vmCaller, prevBlockIndex };
    Initialize(&vmIn, vmOut);

    if (tx->nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION) {
        return PublishContract(contractAddr, tx->pContractData->codeOrFunc, true);
    }
    else if (tx->nVersion == MCTransaction::CALL_CONTRACT_VERSION) {
        UniValue args;
        args.read(tx->pContractData->args);
        return CallContract(contractAddr, tx->pContractData->codeOrFunc, args);
    }
    return false;
}

int ContractVM::ExecuteBlockContract(const MCBlock* pBlock, const MCBlockIndex* prevBlockIndex, int offset, int count, std::vector<VMOut>* vmOuts)
{
    for (int i = offset; i < offset + count; ++i) {
        if (!ExecuteContract(pBlock->vtx[i], i, prevBlockIndex, &(*vmOuts)[i])) {
            return i;
        }
        CommitData();
    }
    return -1;
}

MultiContractVM::MultiContractVM() : threadPool(MAX_GROUP_NUM)
{
    for (size_t i = 0; i < threadPool.size(); ++i) {
        threadPool.schedule(boost::bind(&MultiContractVM::InitializeThread, this));
    }
    threadPool.wait();
}

void MultiContractVM::InitializeThread()
{
    {
        LOCK(cs);
        threadIdToVM.insert(std::make_pair(boost::this_thread::get_id(), std::move(ContractVM())));
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));
}

bool MultiContractVM::Execute(const MCBlock* pBlock, const MCBlockIndex* prevBlockIndex, std::vector<VMOut>* vmOuts)
{
    this->prevBlockIndex = prevBlockIndex;
    this->vmOuts = vmOuts;

    this->vmOuts->clear();
    this->vmOuts->resize(pBlock->vtx.size());

    for (auto& item : threadIdToVM) {
        item.second.ClearData(false);
    }

    size_t offset = 0;
    interrupt = false;
    for (size_t i = 0; i < pBlock->groupSize.size(); ++i) {
        threadPool.schedule(boost::bind(&MultiContractVM::DoExecute, this, pBlock, offset, pBlock->groupSize[i]));
        offset += pBlock->groupSize[i];
    }
    threadPool.wait();
    return !interrupt;
}

void MultiContractVM::DoExecute(const MCBlock* pBlock, int offset, int count)
{
    boost::thread::id threadId = boost::this_thread::get_id();
    auto it = threadIdToVM.find(threadId);
    if (it == threadIdToVM.end()) {
        throw std::runtime_error(strprintf("%s:%d it == threadId2SmartLuaState.end()\n", __FUNCTION__, __LINE__));
    }

    ContractVM& vm = it->second;
    try {
        for (int i = offset; i < offset + count; ++i) {
            if (interrupt) {
                return;
            }
            if (!vm.ExecuteContract(pBlock->vtx[i], i, prevBlockIndex, &(*vmOuts)[i])) {
                interrupt = true;
                return;
            }
            vm.CommitData();
        }
    }
    catch (std::exception e) {
        LogPrintf("%s:%d %s\n", __FUNCTION__, __LINE__, e.what());
    }
}

bool MultiContractVM::CheckCross(const MCBlock* pBlock, MapContractContext& finalData)
{
    finalData.clear();

    // check if group have txin cross
    size_t offset = 0;
    std::set<uint256> blockDependencies;
    for (size_t i = 0; i < pBlock->groupSize.size(); ++i) {
        int groupSize = pBlock->groupSize[i];
        std::set<uint256> groupDependencies;
        for (size_t j = offset; j < offset + groupSize; ++j) {
            const MCTransactionRef& tx = pBlock->vtx[j];
            groupDependencies.insert(tx->GetHash());
            for (size_t k = 0; k < tx->vin.size(); ++k) {
                if (!tx->vin[k].prevout.hash.IsNull() && !tx->IsStake()) { // branch first block's stake tx's input is from the same block(支链第一个块的stake交易的输入来自同一区块中的交易，其他情况下stake的输入不可能来自同一区块)
                    groupDependencies.insert(tx->vin[k].prevout.hash);
                }
            }
        }
        for (const uint256& hash : groupDependencies) {
            auto ret = blockDependencies.insert(hash);
            if (!ret.second) {
                return false;
            }
        }
        offset += groupSize;
    }

    // check if group have contractid cross
    for (const auto& item : threadIdToVM) {
        const MapContractContext& contractData = item.second.GetAllData();
        for (const auto& iter : contractData) {
            auto ret = finalData.insert(iter);
            if (!ret.second) {
                finalData.clear();
                return false;
            }
        }
    }

    return true;
}
