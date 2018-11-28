// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BRANCHCHAIN_H
#define BRANCHCHAIN_H

#include <string>
#include <univalue.h>
#include <map>
#include <memory>

#include "primitives/transaction.h"

class CellBlockIndex;
class CellValidationState;
class CellBlockHeader;
class CellBlock;
class BranchCache;
class BranchData;

class CellRPCConfig {
public:
	std::string strIp;
	uint16_t    iPort;
	std::string strUser;
	std::string strPassword;
    std::string strWallet;

	void Reset();
	bool IsValid();
};

//链配置管理 
typedef std::map<std::string, CellRPCConfig> MAP_RPC_CONFIG; //
class CellBranchChainMan {
public:
	CellBranchChainMan();
	~CellBranchChainMan();
	
	void Init();
	static bool ParseRpcConfig(const std::string& strCfg, CellRPCConfig& rpccfg, std::string& branchid);
	bool GetRpcConfig(const std::string& strName, CellRPCConfig& rpccfg);
	bool CheckRpcConfig(CellRPCConfig& rpccfg);
	void ReplaceRpcConfig(const std::string& strName, CellRPCConfig& rpccfg);
private:
	MAP_RPC_CONFIG mapRpcConfig;
};

extern std::unique_ptr<CellBranchChainMan> g_branchChainMan;

enum branch_script_type
{
    BST_INVALID = 0,
    BST_MORTGAGE_MINE = 1, // 抵押
    BST_MORTGAGE_COIN = 1 << 1, // 挖矿
    BST_MORTGAGE_ALL = BST_MORTGAGE_MINE | BST_MORTGAGE_COIN,
};

UniValue CallRPC(const std::string& host, const int port, const std::string& strMethod, const UniValue& params,
	const std::string& rpcuser = "", const std::string& rpcpassword = "", const std::string& rpcwallet = "");

UniValue CallRPC(const CellRPCConfig& rpccfg, const std::string& strMethod, const UniValue& params);

void ProcessBlockBranchChain();

CellSpvProof* NewSpvProof(const CellBlock &block, const std::set<uint256>& txids);
int CheckSpvProof(const uint256& merkleRoot, CellPartialMerkleTree& pmt, const uint256 &querytxhash);
bool CheckBranchTransaction(const CellTransaction& tx, CellValidationState &state, const bool fVerifingDB, const CellTransactionRef& pFromTx);

CellAmount GetBranchChainCreateTxOut(const CellTransaction& tx);
CellAmount GetBranchChainTransOut(const CellTransaction& branchTransStep1Tx);
CellAmount GetBranchChainOut(const CellTransaction& tx);
CellAmount GetMortgageMineOut(const CellTransaction& tx, bool bWithBranchOut);
CellAmount GetMortgageCoinOut(const CellTransaction& tx, bool bWithBranchOut);

branch_script_type QuickGetBranchScriptType(const CellScript& scriptPubKey);
bool GetMortgageMineData(const CellScript& scriptPubKey, uint256* pBranchHash = nullptr, CellKeyID *pKeyID = nullptr, int64_t *pnHeight = nullptr);
bool GetMortgageCoinData(const CellScript& scriptPubKey, uint256* pFromTxid = nullptr, CellKeyID *pKeyID = nullptr, int64_t *pnHeight = nullptr);
bool GetRedeemSriptData(const CellScript& scriptPubKey, uint256* pFromTxid);

bool BranchChainTransStep2(const CellTransactionRef& tx, const CellBlock &block, std::string* pStrErrorMsg);

bool SendBranchBlockHeader(const std::shared_ptr<const CellBlock> pBlockHeader, std::string *pStrErr=nullptr);
bool CheckBranchBlockInfoTx(const CellTransaction& tx, CellValidationState& state, BranchCache* pBranchCache);
bool CheckBranchDuplicateTx(const CellTransaction& tx, CellValidationState& state, BranchCache* pBranchCache);

uint256 GetReportTxHashKey(const CellTransaction& tx);
uint256 GetProveTxHashKey(const CellTransaction& tx);

bool CheckReportCheatTx(const CellTransaction& tx, CellValidationState& state, BranchCache *pBranchCache);
bool CheckProveTx(const CellTransaction& tx, CellValidationState& state, BranchCache *pBranchCache);
bool CheckReportRewardTransaction(const CellTransaction& tx, CellValidationState& state, CellBlockIndex* pindex, BranchCache *pBranchCache);
bool CheckLockMortgageMineCoinTx(const CellTransaction& tx, CellValidationState& state);
bool CheckUnlockMortgageMineCoinTx(const CellTransaction& tx, CellValidationState& state);
bool CheckProveContractData(const CellTransaction& tx, CellValidationState& state, BranchCache *pBranchCache);

CellMutableTransaction RevertTransaction(const CellTransaction& tx, const CellTransactionRef &pFromTx, bool fDeepRevert = false);

bool ReqMainChainRedeemMortgage(const CellTransactionRef& tx, const CellBlock& block, std::string *pStrErr = nullptr);
#endif //  BRANCHCHAIN_H
