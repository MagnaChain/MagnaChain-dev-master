// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef SMARTCONTRACT_H
#define SMARTCONTRACT_H

extern "C"
{
#include "lua/lvm.h"
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
class MCWallet;
class MCWalletTx;
class MagnaChainAddress;
class MakeBranchTxUTXO;

class SmartLuaState
{
public:
    static const int SAVE_TYPE_NONE = 0;
    static const int SAVE_TYPE_CACHE = 1;
    static const int SAVE_TYPE_DATA = 2;
    static const int MAX_INTERNAL_CALL_NUM = 30;

    std::vector<MCTxOut> recipients;
    std::set<MCContractID> contractIds;      // lua执行期间所有调用过的合约
    std::vector<MagnaChainAddress> contractAddrs;  // 以栈形式表示当前调用合约的合约地址
    MagnaChainAddress originAddr;    // 当前调用合约的调用者最原始公钥地址

    int saveType;
    int64_t timestamp;                          // 执行时的时间戳
    int blockHeight;                            // 执行时的区块高度
    int txIndex;
    MCAmount contractOut = 0;
    uint32_t runningTimes = 0;
    uint32_t deltaDataLen = 0;
    uint32_t codeLen = 0;
    int _internalCallNum = 0;
    CoinAmountCache* pCoinAmountCache;
    std::map<MCContractID, ContractInfo> contractDataFrom;

private:
    mutable MCCriticalSection _contractCS;
    ContractContext* _pContractContext;
    MCBlockIndex* _pPrevBlockIndex;
    std::queue<lua_State*> _luaStates;
    MCTransactionRef tx;

public:
    void SetContractInfo(const MCContractID& contractId, ContractInfo& contractInfo, bool cache);
    bool GetContractInfo(const MCContractID& contractId, ContractInfo& contractInfo);

    void Initialize(int64_t timestamp, int blockHeight, int txIndex, MagnaChainAddress& callerAddr, ContractContext* pContractContext, MCBlockIndex* pPrevBlockIndex, int saveType, CoinAmountCache* pCoinAmountCache);
    lua_State* GetLuaState(MagnaChainAddress& contractAddr);
    void ReleaseLuaState(lua_State* L);

    void Clear();
};

bool GetSenderAddr(MCWallet* pWallet, const std::string& strSenderAddr, MagnaChainAddress& senderAddr);
MCContractID GenerateContractAddress(MCWallet* pWallet, const MagnaChainAddress& senderAddr, const std::string& code);

//temp contract address for publish
template<typename TxType>
MCContractID GenerateContractAddressByTx(TxType& tx)
{
    MCHashWriter ss(SER_GETHASH, 0);
    for (auto v : tx.vin)
        ss << v.prevout;
    for (auto v : tx.vout)
        ss << v.nValue;

    ss << tx.pContractData->codeOrFunc;
    ss << tx.pContractData->sender;
    return MCContractID(Hash160(ParseHex(ss.GetHash().ToString())));
}

std::string TrimCode(const std::string& rawCode);
void SetContractMsg(lua_State* L, const std::string& contractAddr, const std::string& origin, const std::string& sender, lua_Number payment, uint32_t blockTime, lua_Number blockHeight);

bool PublishContract(SmartLuaState* sls, MCWallet* pWallet, const std::string& strSenderAddr, const std::string& rawCode, UniValue& ret);
bool PublishContract(SmartLuaState* sls, MagnaChainAddress& contractAddr, std::string& rawCode, UniValue& ret);

bool CallContract(SmartLuaState* sls, MagnaChainAddress& contractAddr, const MCAmount amount, const std::string& strFuncName, const UniValue& args, long& maxCallNum, UniValue& ret);
bool CallContractReal(SmartLuaState* sls, MagnaChainAddress& contractAddr, const MCAmount amount, const std::string& strFuncName, const UniValue& args, long& maxCallNum, UniValue& ret);
bool CallContract(lua_State* L, const std::string& code, const std::string& data, const std::string& strFuncName, const UniValue& args, long& maxCallNum, std::string& dataout, UniValue& ret);

bool ExecuteContract(SmartLuaState* sls, const MCTransactionRef tx, int txIndex, MCAmount coins, int64_t blockTime, int blockHeight, MCBlockIndex* pPrevBlockIndex, ContractContext* pContractContext);
bool ExecuteBlock(SmartLuaState* sls, MCBlock* pBlock, MCBlockIndex* pPrevBlockIndex, int offset, int count, ContractContext* pContractContext);

uint256 GetTxHashWithData(const uint256& txHash, const CONTRACT_DATA& contractData);
uint256 GetTxHashWithPrevData(const uint256& txHash, const ContractPrevData& contractPrevData);
bool VecTxMerkleLeavesWithData(const std::vector<MCTransactionRef>& vtx, const std::vector<ContractTxFinalData>& contractData, std::vector<uint256>& leaves);
bool VecTxMerkleLeavesWithPrevData(const std::vector<MCTransactionRef>& vtx, const std::vector<ContractPrevData>& contractData, std::vector<uint256>& leaves);
uint256 BlockMerkleRootWithData(const MCBlock& block, const ContractContext& contractContext, bool* mutated = nullptr);
uint256 BlockMerkleRootWithPrevData(const MCBlock& block, bool* mutated = nullptr);

// Lua内置函数
int InternalCallContract(lua_State *L);
int SendCoins(lua_State* L);

#endif
