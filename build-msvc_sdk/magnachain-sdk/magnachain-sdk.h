#pragma once

#include <string>

#ifdef CELLLINK_SDK_EXPORTS
#define CELLLINK_API __declspec(dllexport)
#else
#define CELLLINK_API __declspec(dllimport)
#endif

struct MCExtKey;
class MCKey;
class MCPubKey;
class ECCVerifyHandle;

enum NETWORK_TYPE
{
	MAIN,
	TESTNET,
	REGTEST,
	BRANCH
};

class CELLLINK_API IxMagnaChainBridge
{
private:
	MCExtKey* m_pRootKey;
	std::string m_strHost;
	std::string m_strPort;
	std::string m_strUser;
	std::string m_strPwd;

	std::string m_strRPCPrefix;

	const char* m_arrRpcArg[20];

    ECCVerifyHandle *m_pVerifyHandle;
    int initArgN;
public:
	IxMagnaChainBridge();
	~IxMagnaChainBridge();

	// this function must call first
	static void Initialize(NETWORK_TYPE eNetworkType = NETWORK_TYPE::TESTNET);

	static void Release();

	static bool GetExtKeyWif(MCExtKey* pExtKey, char* pOutWif, int iSize);
	static MCExtKey ImportExtKey(const char* pExtKeyWif);

	static MCKey* GetCellKey(MCExtKey* pCEK);

	static bool GetKeyWif(MCKey* pKey, char* pOutWif, int iSize);
	static MCKey ImportKey(const char* pWif);	

	//static CellPubKey GetCellPubKey(CellKey* pCK);

	//static std::string GetAddress(CellPubKey* pPubKey);
	static bool GetAddress(MCKey* pKey, char* pOutWif, int iSize);

	// create root key by aid
	bool CreateRootExtKey(const char* pAid);	

    MCExtKey* GetRootExtKey();

	void InitializeRPCInfo(const char* pHost, const char* pPort, const char* pUser, const char* pPwd);
    void ResetArgs();
	
	float GetBalance(const char* pAddress);

	bool Transfer(const char* pFromPirKeyWif, const char* pDestAddr, float fAmount, const char* pChangeAddr);
};
