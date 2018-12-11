// TestCellLinkSDK.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include "../magnachain-sdk/magnachain-sdk.h"
//#include "../celllink-sdk/celllink-base.h"


int main()
{
	IxCellLinkBridge::Initialize(NETWORK_TYPE::MAIN);

	char chInput[1024];

	sprintf(chInput, "------------------------int:%d\n", 500);

	OutputDebugStringA(chInput);

	IxCellLinkBridge* pCB = new IxCellLinkBridge();
	pCB->CreateRootExtKey("ooxx is good");

	char mbzTmp[1024];

	MCExtKey* pCEK = pCB->GetRootExtKey();
	bool bRet = IxCellLinkBridge::GetExtKeyWif(pCEK, mbzTmp, sizeof(mbzTmp));
	sprintf(chInput, "Ext key wif: %s\r\n", mbzTmp);
	OutputDebugStringA(chInput);

	MCKey* pCK = IxCellLinkBridge::GetCellKey(pCEK);
	bRet = IxCellLinkBridge::GetKeyWif(pCK, mbzTmp, sizeof(mbzTmp));
	sprintf(chInput, "Key wif: %s\r\n", mbzTmp);
	OutputDebugStringA(chInput);

	//CellPubKey* pCPK = &IxCellLinkBridge::GetCellPubKey(pCK);
	bRet = IxCellLinkBridge::GetAddress(pCK, mbzTmp, sizeof(mbzTmp));
	sprintf(chInput, "Address: %s\r\n", mbzTmp);
	OutputDebugStringA(chInput);

    pCB->InitializeRPCInfo("127.0.0.1", "8201", "user", "pwd");

    float balance = pCB->GetBalance("XXUrA5NoD2L42atH8bdPN1UbrjsGVyR3Yw");

    //XXUrA5NoD2L42atH8bdPN1UbrjsGVyR3Yw
    std::string strPrivKey = "L2bv5m5hTGy8ZaknHLpzpSfhFZFKBSP4seHJR37PgJedY4XBHfZV";// private key for from address
    pCB->Transfer("XXUrA5NoD2L42atH8bdPN1UbrjsGVyR3Yw", "XDfHyd7xaZvMQdv4MnoEL3qRPbrxQ6D6MC", 1, "XXUrA5NoD2L42atH8bdPN1UbrjsGVyR3Yw", strPrivKey);

    return 0;
}
