// celllink-sdk.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "magnachain-sdk.h"
#include "key/key.h"
#include "coding/base58.h"
#include "univalue/include/univalue.h"
//#include "consensus/tx_verify.h"
#include "key/keystore.h"
#include "primitives/transaction.h"
#include "misc/amount.h"
#include "script/script.h"
#include "chain/chainparams.h"
#include "utils/util.h"
#include "io/core_io.h"
#include "wallet/wallet.h"
#include "script/sign.h"
#include "rpc/server.h"

typedef std::map<MCOutPoint, MCTxOut> MAP_OUTPUT_COINS;

IxMagnaChainBridge::IxMagnaChainBridge()
{
	m_pRootKey = NULL;
    m_pVerifyHandle = new ECCVerifyHandle();
}

IxMagnaChainBridge::~IxMagnaChainBridge()
{
	if (m_pRootKey != NULL)
	{
		delete m_pRootKey;
		m_pRootKey = NULL;
	}
    if (m_pVerifyHandle)
        delete m_pVerifyHandle;
    m_pVerifyHandle = nullptr;
}

extern bool SignatureCoinbaseTransaction(int nHeight, const MCKeyStore* keystoreIn, MCMutableTransaction& txNew, MCAmount nValue, const MCScript& scriptPubKey);

void IxMagnaChainBridge::Initialize(NETWORK_TYPE eNetworkType)
{
	SignatureCoinbaseTransactionPF = &SignatureCoinbaseTransaction;

	bool hasstartecc = ECC_HasStarted();
	if (!hasstartecc)
	{
		ECC_Start();
	}		

    if (!SetupNetworking()) {
        fprintf(stderr, "Error: Initializing networking failed\n");
        //throw std::exception("setup network fail");
    }

	std::string strNetwork;

	if (eNetworkType == NETWORK_TYPE::MAIN)
	{
		strNetwork = MCBaseChainParams::MAIN;
	}
	else if (eNetworkType == NETWORK_TYPE::REGTEST)
	{
		strNetwork = MCBaseChainParams::REGTEST;
	}
	else if (eNetworkType == NETWORK_TYPE::BRANCH)
	{
		strNetwork = MCBaseChainParams::BRANCH;
	}
	else
	{
		strNetwork = MCBaseChainParams::TESTNET;
	}

	SelectParams(strNetwork);
}

void IxMagnaChainBridge::Release()
{
	bool hasstartecc = ECC_HasStarted();
	if (hasstartecc)
	{
		ECC_Stop();
	}
}

bool IxMagnaChainBridge::GetExtKeyWif(MCExtKey* pExtKey, char* pOutWif, int iSize)
{
	if (pExtKey == NULL || pOutWif == NULL)
	{
		return false;
	}

	//MCExtKey masterKey;
	//masterKey.SetMaster(key.begin(), key.size());

    MagnaChainExtKey b58extkey;
	b58extkey.SetKey(*pExtKey);
	std::string strK = b58extkey.ToString();

	::memset(pOutWif, 0, iSize);
	if (iSize <= strK.length())
	{
		return false;
	}
	::strcpy(pOutWif, strK.c_str());
	return true;
}

MCExtKey IxMagnaChainBridge::ImportExtKey(const char* pExtKeyWif)
{
	std::string strK = pExtKeyWif;
    MagnaChainExtKey* pCLEK = new MagnaChainExtKey(strK);
	MCExtKey kCEK = pCLEK->GetKey();
	delete pCLEK;
	return kCEK;
}

MCKey* IxMagnaChainBridge::GetCellKey(MCExtKey* pCEK)
{
	if (pCEK == NULL)
	{
		return NULL;
	}

	return &pCEK->key;
}

bool IxMagnaChainBridge::GetKeyWif(MCKey* pKey, char* pOutWif, int iSize)
{
	if (pKey == NULL || pOutWif == NULL)
	{
		return false;
	}

	std::string strK = MagnaChainSecret(*pKey).ToString();
	if (iSize <= strK.length())
	{
		return false;
	}

	::memset(pOutWif, 0, iSize);
	::strcpy(pOutWif, strK.c_str());
	return true;
}

MCKey IxMagnaChainBridge::ImportKey(const char* pWif)
{
	MagnaChainSecret kCS;

	std::string strK = pWif;
	kCS.SetString(strK);
	return kCS.GetKey();
}

//CellPubKey IxCellLinkBridge::GetCellPubKey(MCKey* pCK)
//{
//	return pCK->GetPubKey();
//}

//std::string IxCellLinkBridge::GetAddress(CellPubKey* pPubKey)
//{
//	if (pPubKey == NULL)
//	{
//		return "NULL pub key!";
//	}
//
//	CellKeyID kCKI = pPubKey->GetID();
//	return CellLinkAddress(kCKI).ToString();
//}

bool IxMagnaChainBridge::GetAddress(MCKey* pKey, char* pOutWif, int iSize)
{
	if (pKey == NULL || pOutWif == NULL)
	{
		return false;
	}

	MCPubKey kCPK = pKey->GetPubKey();
	MCKeyID kCKI = kCPK.GetID();
	std::string strK = MagnaChainAddress(kCKI).ToString();

	if (iSize <= strK.length())
	{
		return false;
	}

	::memset(pOutWif, 0, iSize);
	::strcpy(pOutWif, strK.c_str());
	return true;
}

bool IxMagnaChainBridge::CreateRootExtKey(const char* pAid)
{
	int iLen = ::strlen(pAid);
	if (iLen == 0)
	{
		return false;
	}	

	/*int i;
	
	unsigned char* pSeed = new unsigned char[iLen + 1];
	for (i = 0; i < iLen; i++)
	{
		pSeed[i] = strAid[i];
	}*/

	m_pRootKey = new MCExtKey();
	m_pRootKey->SetMaster((unsigned char*)pAid, iLen);
	//delete[] pSeed;
	return true;
}

MCExtKey* IxMagnaChainBridge::GetRootExtKey()
{
	return m_pRootKey;
}

void IxMagnaChainBridge::InitializeRPCInfo(const char* pHost, const char* pPort, const char* pUser, const char* pPwd)
{
	m_strHost = pHost;
	m_strPort = pPort;
	m_strUser = pUser;
	m_strPwd = pPwd;

    initArgN = 0;
    m_arrRpcArg[initArgN++] = "application_name";

	static std::string strConnect = "-rpcconnect=" + m_strHost;
	m_arrRpcArg[initArgN++] = strConnect.c_str();

	static std::string strPort = "-rpcport=" + m_strPort;
	m_arrRpcArg[initArgN++] = strPort.c_str();

	static std::string strUser = "-rpcuser=" + m_strUser;
	m_arrRpcArg[initArgN++] = strUser.c_str();

	static std::string strPwd = "-rpcpassword=" + m_strPwd;
	m_arrRpcArg[initArgN++] = strPwd.c_str();
}

void IxMagnaChainBridge::ResetArgs()
{
    for (int i=5; i < sizeof(m_arrRpcArg)/sizeof(m_arrRpcArg[0]); i++)
    {
        m_arrRpcArg[i] = "";
    }
}

int CommandLineRPC(int argc, char *argv[], UniValue& kRet);

float IxMagnaChainBridge::GetBalance(const char* pAddress)
{
	if (pAddress == NULL)
	{
		return -1.0f;
	}

	UniValue kRet;

    ResetArgs();

    int argc = initArgN;
	m_arrRpcArg[argc++] = "getbalanceof";
	m_arrRpcArg[argc++] = pAddress;

    gArgs.ParseParameters(argc, (char**)m_arrRpcArg);
	int iF = CommandLineRPC(argc, (char**)m_arrRpcArg, kRet);
	if (iF == EXIT_FAILURE)
	{
		return -1.0f;
	}
    const UniValue& result = find_value(kRet, "result");
    const UniValue& error = find_value(kRet, "error");
    if (!error.isNull())
    {
        return -1.0f;
    }
    if (result.isNum())
    {
        return result.get_real();
    }
	return -1.0f;
}

MAP_OUTPUT_COINS TransUvcoinsArrToMap(const UniValue& uvcoins)
{
    MAP_OUTPUT_COINS mapCoins;
    for (int i = 0; i < uvcoins.size(); i++)
    {
        const UniValue& uvcoin = uvcoins[i];
        const UniValue& uvtxhash = find_value(uvcoin, "txhash");
        const UniValue& uvoutn = find_value(uvcoin, "outn");

        const UniValue& uvAmount = find_value(uvcoin, "value");
        const UniValue& uvscript = find_value(uvcoin, "script");

        uint256 txhash = uint256S(uvtxhash.get_str());
        uint32_t n = uvoutn.get_int();

        MCOutPoint output(txhash, n);

        std::vector<unsigned char> vecScript = ParseHex(uvscript.get_str());
        MCScript script(vecScript.begin(), vecScript.end());

        MCTxOut txout;
        txout.nValue = AmountFromValue(uvAmount);
        txout.scriptPubKey = script;

        mapCoins.insert(std::make_pair(output, txout));
    }
    return mapCoins;
}

bool SignTransaction(MCMutableTransaction &mtx, MAP_OUTPUT_COINS &mapCoins, MCBasicKeyStore& keystore)
{
    MCTransaction txNewConst(mtx);
    int nIn = 0;
    for (const auto& input : mtx.vin) {
        MAP_OUTPUT_COINS::iterator mit = mapCoins.find(input.prevout);
        if (mit == mapCoins.end())
        {
            return false;
        }

        const MCScript& scriptPubKey = mit->second.scriptPubKey;
        const MCAmount& amount = mit->second.nValue;

        SignatureData sigdata;
        if (!ProduceSignature(TransactionSignatureCreator(&keystore, &txNewConst, nIn, amount, SIGHASH_ALL), scriptPubKey, sigdata)) {
            return false;
        }
        UpdateTransaction(mtx, nIn, sigdata);
        nIn++;
    }
    // sign with contractSender addr's private key.
    if (mtx.IsSmartContract())
    {
        MCTransaction txNewConst(mtx);
        MCScript constractSig;
        if (!SignContract(&keystore, &txNewConst, constractSig))
        {
            return false;
        }
        else {
            mtx.pContractData->signature = constractSig;
        }
    }
}

bool IxMagnaChainBridge::Transfer(const char* pFromPirKeyWif, const char* pDestAddr, float fAmount, const char* pChangeAddr)
{
	if (pFromPirKeyWif == NULL || pDestAddr == NULL)
	{
		return false;
	}

    ResetArgs();

	// get from address first
	char arrFromAddress[256];

	MCKey kPriKey = ImportKey(pFromPirKeyWif);
	if (GetAddress(&kPriKey, arrFromAddress, sizeof(arrFromAddress))== false)
	{
		return false;
	}

	if (pChangeAddr == NULL)
	{
		pChangeAddr = arrFromAddress;
	}

    // avoid to use pointer which may be deleted by other user.
    int argc = initArgN;
    m_arrRpcArg[argc++] = "premaketransaction";
    m_arrRpcArg[argc++] = arrFromAddress;
    m_arrRpcArg[argc++] = pDestAddr;
    m_arrRpcArg[argc++] = pChangeAddr;
    std::string strAmount = strprintf("%f", fAmount);
    m_arrRpcArg[argc++] = strAmount.c_str();

    UniValue kRet;
    gArgs.ParseParameters(argc, (char**)m_arrRpcArg);
    int iF = CommandLineRPC(argc, (char**)m_arrRpcArg, kRet);
    if (iF == EXIT_FAILURE)
    {
        return false;
    }

    const UniValue& result = find_value(kRet, "result");
    const UniValue& error = find_value(kRet, "error");
    if (!error.isNull())
    {
        return false;
    }

    const UniValue& uvtxhex = find_value(result, "txhex");
    const UniValue& uvcoins = find_value(result, "coins");
    if (uvtxhex.isNull() || !uvtxhex.isStr() || uvcoins.isNull() || !uvcoins.isArray())
    {
        return false;
    }

    //反序列化出交易结构
    MCMutableTransaction mtx;
    if (!DecodeHexTx(mtx, uvtxhex.get_str()))
    {
        return false;
    }

    MAP_OUTPUT_COINS mapCoins = TransUvcoinsArrToMap(uvcoins);
    
    //把私钥添加进 keystore
    MagnaChainSecret vchSecret;
    if (!vchSecret.SetString(pFromPirKeyWif)) return false;

    MCKey key = vchSecret.GetKey();
    if (!key.IsValid()) return false;

    MCBasicKeyStore keystore;
    keystore.AddKey(key);

    //签名
    if (!SignTransaction(mtx, mapCoins, keystore))
    {
        return false;
    }

    //send to node
    MCTransaction tx(mtx);
    std::string strTxHex = EncodeHexTx(tx, RPCSerializationFlags());
    //----------------------------------------------
    // next rpc
    {
        ResetArgs();

        // avoid to use pointer which may be deleted by other user.
        int argc = initArgN;
        m_arrRpcArg[argc++] = "sendrawtransaction";
        m_arrRpcArg[argc++] = strTxHex.c_str();

        UniValue kRet;
        gArgs.ParseParameters(argc, (char**)m_arrRpcArg);
        int iF = CommandLineRPC(argc, (char**)m_arrRpcArg, kRet);
        if (iF == EXIT_FAILURE)
        {
            return false;
        }

        const UniValue& result = find_value(kRet, "result");
        const UniValue& error = find_value(kRet, "error");
        if (!error.isNull())
        {
            return false;
        }
    }
    
	return true;
}


