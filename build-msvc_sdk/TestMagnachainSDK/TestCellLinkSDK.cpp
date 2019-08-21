// TestCellLinkSDK.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include "../magnachain-sdk/magnachain-sdk.h"
//#include "../celllink-sdk/celllink-base.h"


int main()
{
	IxMagnaChainBridge::Initialize(NETWORK_TYPE::MAIN);

	char chInput[1024];

	sprintf(chInput, "------------------------int:%d\n", 500);

	OutputDebugStringA(chInput);

	IxMagnaChainBridge* pCB = new IxMagnaChainBridge();
	pCB->CreateRootExtKey("ooxx is good");

	char mbzTmp[1024];

	MCExtKey* pCEK = pCB->GetRootExtKey();
	bool bRet = IxMagnaChainBridge::GetExtKeyWif(pCEK, mbzTmp, sizeof(mbzTmp));
	sprintf(chInput, "Ext key wif: %s\r\n", mbzTmp);
	OutputDebugStringA(chInput);

	MCKey* pCK = IxMagnaChainBridge::GetCellKey(pCEK);
	bRet = IxMagnaChainBridge::GetKeyWif(pCK, mbzTmp, sizeof(mbzTmp));
	sprintf(chInput, "Key wif: %s\r\n", mbzTmp);
	OutputDebugStringA(chInput);

	//CellPubKey* pCPK = &IxCellLinkBridge::GetCellPubKey(pCK);
	bRet = IxMagnaChainBridge::GetAddress(pCK, mbzTmp, sizeof(mbzTmp));
	sprintf(chInput, "Address: %s\r\n", mbzTmp);
	OutputDebugStringA(chInput);

    pCB->InitializeRPCInfo("94.191.82.80", "9102", "mgc-user", "mgc-pwd-2019");

    float balance = pCB->GetBalance("mZgWKLmHP4YL5XHcZbZ4oCVBZMM87ydesi");
	
	sprintf(chInput, "Balance: %f\r\n", balance);
	OutputDebugStringA(chInput);
	
    //XXUrA5NoD2L42atH8bdPN1UbrjsGVyR3Yw
    std::string strPrivKey = "L2bv5m5hTGy8ZaknHLpzpSfhFZFKBSP4seHJR37PgJedY4XBHfZV";// base58 code private key for from address
    bRet = pCB->Transfer(strPrivKey.c_str(), "XDfHyd7xaZvMQdv4MnoEL3qRPbrxQ6D6MC", 2, "XXUrA5NoD2L42atH8bdPN1UbrjsGVyR3Yw");
	
	sprintf(chInput, "Transfer result: %d\r\n", bRet);
	OutputDebugStringA(chInput);

    return 0;
}

