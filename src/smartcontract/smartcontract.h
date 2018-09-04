// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef SMARTCONTRACT_H
#define SMARTCONTRACT_H

extern "C"
{
#include "lua/lstate.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#include <set>
#include <stack>
#include <unordered_map>
#include "key/pubkey.h"
#include "univalue.h"
#include "smartcontract/contractdb.h"

const int MAX_CONTRACT_CALL = 10000;
const int MAX_DATA_LEN = 1024 * 1024;

class Coin;
class CellWallet;
class CellWalletTx;
class CellLinkAddress;

struct SmartContractRet
{
    UniValue result;
    std::string data;
    uint32_t runningTimes = 0;

    SmartContractRet()
        : result(UniValue::VARR) {
    }
};

class SmartLuaState
{
public:
    static const int SAVE_TYPE_CACHE = 1;
    static const int SAVE_TYPE_DATA = 2;

    std::vector<std::pair<Coin, CellOutPoint>> inputs;
    std::vector<CellTxOut> outputs;
    std::set<CellContractID> contractIds;      // lua执行期间所有调用过的合约
    std::stack<CellLinkAddress> contractAddrs;  // 以栈形式表示当前调用合约的合约地址
    std::stack<CellLinkAddress> senderAddrs;    // 以栈形式表示当前调用合约的调用者地址

    int saveType;
    int64_t timestamp;                          // 执行时的时间戳
    int blockHeight;                            // 执行时的区块高度
    CellAmount totalAmount = -1;
    CellAmount sendAmount = 0;
    uint32_t runningTimes = 0;
    size_t deltaDataLen = 0;
    size_t codeLen = 0;

private:
    mutable CellCriticalSection _contractCS;
    ContractContext* _pContractContext;
    CellBlockIndex* _pPrevBlockIndex;
    std::queue<lua_State*> _luaStates;

public:
    void SetContractInfo(const CellContractID& contractId, ContractInfo& contractInfo, bool cache);
    bool GetContractInfo(const CellContractID& contractId, ContractInfo& contractInfo);

    void Initialize(int64_t timestamp, int blockHeight, ContractContext* pContractContext, CellBlockIndex* pPrevBlockIndex, int saveType);
    lua_State* GetLuaState(CellLinkAddress& contractAddr, CellLinkAddress& senderAddr);
    void ReleaseLuaState(lua_State* L);
};

extern bool GetSenderAddr(CellWallet* pWallet, const std::string& strSenderAddr, CellLinkAddress& senderAddr);
extern CellContractID GenerateContractAddress(CellWallet* pWallet, const CellLinkAddress& senderAddr, const std::string& code);

extern void SetContractMsg(lua_State* L, const std::string& contractAddr, const std::string& sender, lua_Number payment, uint32_t blockTime, lua_Number blockHeight);

extern int PublishContract(SmartLuaState* sls, CellWallet* pWallet, CellAmount amount, const std::string& strSenderAddr, std::string& rawCode, std::string& code, UniValue& ret);
extern int PublishContract(SmartLuaState* sls, CellAmount amount, CellLinkAddress& contractAddr, CellLinkAddress& senderAddr, const std::string& rawCode, std::string& code, SmartContractRet& scr);
extern int PublishContract(lua_State* L, const std::string& rawCode, std::string& code, SmartContractRet& ret);

extern int CallContract(SmartLuaState* sls, long& maxCallNum, CellAmount amount, CellLinkAddress& contractAddr, CellLinkAddress& senderAddr, const std::string& strFuncName, const UniValue& args, SmartContractRet& scr);
extern int CallContract(lua_State* L, long maxCallNum, const std::string& code, const std::string& data, const std::string& strFuncName, const UniValue& args, SmartContractRet& ret);

// Lua内置函数
extern int InternalCallContract(lua_State *L);
extern int SendCoins(lua_State* L);

#endif
