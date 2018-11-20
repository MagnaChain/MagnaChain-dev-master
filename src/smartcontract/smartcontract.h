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
//#include "lua/ldebug.h"
}

#include <set>
#include <stack>
#include <unordered_map>
#include "key/pubkey.h"
#include "univalue.h"
#include "smartcontract/contractdb.h"
#include "coding/base58.h"

const int MAX_CONTRACT_CALL = 10000;
const int MAX_DATA_LEN = 1024 * 1024;

class Coin;
class CellWallet;
class CellWalletTx;
class CellLinkAddress;
class MakeBranchTxUTXO;

class SmartLuaState
{
public:
    static const int SAVE_TYPE_NONE = 0;
    static const int SAVE_TYPE_CACHE = 1;
    static const int SAVE_TYPE_DATA = 2;
    static const int MAX_INTERNAL_CALL_NUM = 30;

    std::vector<CellTxOut> recipients;
    std::set<CellContractID> contractIds;      // lua执行期间所有调用过的合约
    std::vector<CellLinkAddress> contractAddrs;  // 以栈形式表示当前调用合约的合约地址
    CellLinkAddress originAddr;    // 当前调用合约的调用者最原始公钥地址

    int saveType;
    int64_t timestamp;                          // 执行时的时间戳
    int blockHeight;                            // 执行时的区块高度
    int txIndex;
    CellAmount contractOut = 0;
    uint32_t runningTimes = 0;
    size_t deltaDataLen = 0;
    size_t codeLen = 0;
    int _internalCallNum = 0;
    CoinAmountCache* pCoinAmountCache;
    std::map<CellContractID, ContractInfo> contractDataFrom;

private:
    mutable CellCriticalSection _contractCS;
    ContractContext* _pContractContext;
    CellBlockIndex* _pPrevBlockIndex;
    std::queue<lua_State*> _luaStates;
    CellTransactionRef tx;

public:
    void SetContractInfo(const CellContractID& contractId, ContractInfo& contractInfo, bool cache);
    bool GetContractInfo(const CellContractID& contractId, ContractInfo& contractInfo);

    void Initialize(int64_t timestamp, int blockHeight, int txIndex, CellLinkAddress& callerAddr, ContractContext* pContractContext, CellBlockIndex* pPrevBlockIndex, int saveType, CoinAmountCache* pCoinAmountCache);
    lua_State* GetLuaState(CellLinkAddress& contractAddr);
    void ReleaseLuaState(lua_State* L);

    void Clear();
};

extern bool GetSenderAddr(CellWallet* pWallet, const std::string& strSenderAddr, CellLinkAddress& senderAddr);
extern CellContractID GenerateContractAddress(CellWallet* pWallet, const CellLinkAddress& senderAddr, const std::string& code);

template<typename TxType>
CellContractID GenerateContractAddressByTx(TxType& tx)
{
    CellHashWriter ss(SER_GETHASH, 0);
    for (auto v : tx.vin)
        ss << v.prevout;
    for (auto v : tx.vout)
        ss << v.nValue;

    ss << tx.contractCode;
    ss << tx.contractSender;
    return CellContractID(Hash160(ParseHex(ss.GetHash().ToString())));
}

extern void SetContractMsg(lua_State* L, const std::string& contractAddr, const std::string& origin, const std::string& sender, lua_Number payment, uint32_t blockTime, lua_Number blockHeight);

extern bool PublishContract(SmartLuaState* sls, CellWallet* pWallet, const std::string& strSenderAddr, std::string& rawCode, UniValue& ret);
extern bool PublishContract(SmartLuaState* sls, CellLinkAddress& contractAddr, const std::string& rawCode, UniValue& ret);
extern bool PublishContract(lua_State* L, const std::string& rawCode, long& maxCallNum, std::string& codeout, std::string& dataout, UniValue& ret);

extern bool CallContract(SmartLuaState* sls, CellLinkAddress& contractAddr, const CellAmount amount, const std::string& strFuncName, const UniValue& args, long& maxCallNum, UniValue& ret);
extern bool CallContractReal(SmartLuaState* sls, CellLinkAddress& contractAddr, const CellAmount amount, const std::string& strFuncName, const UniValue& args, long& maxCallNum, UniValue& ret);
extern bool CallContract(lua_State* L, const std::string& code, const std::string& data, const std::string& strFuncName, const UniValue& args, long& maxCallNum, std::string& dataout, UniValue& ret);

bool ExecuteContract(SmartLuaState* sls, const CellTransactionRef tx, int txIndex, CellAmount coins, int64_t blockTime, int blockHeight, CellBlockIndex* pPrevBlockIndex, ContractContext* pContractContext);
bool ExecuteBlock(SmartLuaState* sls, CellBlock* pBlock, CellBlockIndex* pPrevBlockIndex, int offset, int count, ContractContext* pContractContext);

uint256 GetTxHashWithData(const uint256& txHash, const CONTRACT_DATA& contractData);
uint256 GetTxHashWithPrevData(const uint256& txHash, const ContractPrevData& contractPrevData);
bool VecTxMerkleLeavesWithData(const std::vector<CellTransactionRef>& vtx, const std::vector<CONTRACT_DATA>& contractData, std::vector<uint256>& leaves);
bool VecTxMerkleLeavesWithPrevData(const std::vector<CellTransactionRef>& vtx, const std::vector<ContractPrevData>& contractData, std::vector<uint256>& leaves);
uint256 BlockMerkleRootWithData(const CellBlock& block, const ContractContext& contractContext, bool* mutated = nullptr);
uint256 BlockMerkleRootWithPrevData(const CellBlock& block, bool* mutated = nullptr);

// Lua内置函数
extern int InternalCallContract(lua_State *L);
extern int SendCoins(lua_State* L);

#endif
