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

IxCellLinkBridge::IxCellLinkBridge()
{
	m_pRootKey = NULL;
}

IxCellLinkBridge::~IxCellLinkBridge()
{
	if (m_pRootKey != NULL)
	{
		delete m_pRootKey;
		m_pRootKey = NULL;
	}
}

extern bool SignatureCoinbaseTransaction(int nHeight, const MCKeyStore* keystoreIn, MCMutableTransaction& txNew, MCAmount nValue, const MCScript& scriptPubKey);

void IxCellLinkBridge::Initialize(NETWORK_TYPE eNetworkType)
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

void IxCellLinkBridge::Release()
{
	bool hasstartecc = ECC_HasStarted();
	if (hasstartecc)
	{
		ECC_Stop();
	}
}

bool IxCellLinkBridge::GetExtKeyWif(MCExtKey* pExtKey, char* pOutWif, int iSize)
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

MCExtKey IxCellLinkBridge::ImportExtKey(const char* pExtKeyWif)
{
	std::string strK = pExtKeyWif;
    MagnaChainExtKey* pCLEK = new MagnaChainExtKey(strK);
	MCExtKey kCEK = pCLEK->GetKey();
	delete pCLEK;
	return kCEK;
}

MCKey* IxCellLinkBridge::GetCellKey(MCExtKey* pCEK)
{
	if (pCEK == NULL)
	{
		return NULL;
	}

	return &pCEK->key;
}

bool IxCellLinkBridge::GetKeyWif(MCKey* pKey, char* pOutWif, int iSize)
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

MCKey IxCellLinkBridge::ImportKey(const char* pWif)
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

bool IxCellLinkBridge::GetAddress(MCKey* pKey, char* pOutWif, int iSize)
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

bool IxCellLinkBridge::CreateRootExtKey(const char* pAid)
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

MCExtKey* IxCellLinkBridge::GetRootExtKey()
{
	return m_pRootKey;
}

void IxCellLinkBridge::InitializeRPCInfo(const char* pHost, const char* pPort, const char* pUser, const char* pPwd)
{
	m_strHost = pHost;
	m_strPort = pPort;
	m_strUser = pUser;
	m_strPwd = pPwd;

    m_arrRpcArg[0] = "application_name";

	static std::string strConnect = "-rpcconnect=" + m_strHost;
	m_arrRpcArg[1] = strConnect.c_str();

	static std::string strPort = "-rpcport=" + m_strPort;
	m_arrRpcArg[2] = strPort.c_str();

	static std::string strUser = "-rpcuser=" + m_strUser;
	m_arrRpcArg[3] = strUser.c_str();

	static std::string strPwd = "-rpcpassword=" + m_strPwd;
	m_arrRpcArg[4] = strPwd.c_str();
}

void IxCellLinkBridge::ResetArgs()
{
    for (int i=5; i < sizeof(m_arrRpcArg)/sizeof(m_arrRpcArg[0]); i++)
    {
        m_arrRpcArg[i] = "";
    }
}

int CommandLineRPC(int argc, char *argv[], UniValue& kRet);

float IxCellLinkBridge::GetBalance(const char* pAddress)
{
	if (pAddress == NULL)
	{
		return -1.0f;
	}

	UniValue kRet;

    ResetArgs();

	m_arrRpcArg[5] = "getbalanceof";
	m_arrRpcArg[6] = pAddress;

    int argc = 7;
    gArgs.ParseParameters(argc, (char**)m_arrRpcArg);
	int iF = CommandLineRPC(argc, (char**)m_arrRpcArg, kRet);
	if (iF == EXIT_FAILURE)
	{
		return -1.0f;
	}
    const UniValue& result = find_value(kRet, "result");
    const UniValue& error = find_value(kRet, "error");
    if (result.isNum())
    {
        return result.get_real();
    }
	return -1.0f;
}

bool IxCellLinkBridge::Transfer(const char* pFromKeyWif, const char* pDestAddr, float fAmount, const char* pChangeAddr)
{
	return false;
}


