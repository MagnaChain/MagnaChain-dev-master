// Fill out your copyright notice in the Description page of Project Settings.

#include "TestMagnaChain.h"
#include "magnachain-sdk.h"


// Sets default values for this component's properties
UTestMagnaChain::UTestMagnaChain()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Test.....
}


// Called when the game starts
void UTestMagnaChain::BeginPlay()
{
	Super::BeginPlay();

	// Test

	UE_LOG(LogTemp, Display, TEXT("%s"), TEXT("Test CellLink begin---------------------------"));

	IxMagnaChainBridge::Initialize(NETWORK_TYPE::REGTEST);

	//char mbzInfo[1024];
	//wchar_t wbzInfo[1024];

	IxMagnaChainBridge* pCB = new IxMagnaChainBridge();
	pCB->CreateRootExtKey("magnachain is good!!!");
	MCExtKey* pCEK = pCB->GetRootExtKey();

	char mbzTmp[1024];

	bool bRet;

	bRet = IxMagnaChainBridge::GetExtKeyWif(pCEK, mbzTmp, sizeof(mbzTmp));

	//sprintf(mbzInfo, "Ext key wif: %s\r\n", mbzTmp);
	//mbtowc(wbzInfo, mbzInfo, sizeof(mbzInfo));
	UE_LOG(LogTemp, Display, TEXT("Ext key wif: %s\r\n"), ANSI_TO_TCHAR(mbzTmp));

	MCKey* pCK = IxMagnaChainBridge::GetCellKey(pCEK);
	bRet = IxMagnaChainBridge::GetKeyWif(pCK, mbzTmp, sizeof(mbzTmp));
	//sprintf(mbzInfo, "Key wif: %s\r\n", mbzTmp);
	//mbtowc(wbzInfo, mbzInfo, sizeof(mbzInfo));
	UE_LOG(LogTemp, Display, TEXT("Key wif : %s\r\n"), ANSI_TO_TCHAR(mbzTmp));

	//CellPubKey* pCPK = &IxCellLinkBridge::GetCellPubKey(pCK);
	bRet = IxMagnaChainBridge::GetAddress(pCK, mbzTmp, sizeof(mbzTmp));
	//sprintf(mbzInfo, "Address: %s\r\n", mbzTmp);
	//mbtowc(wbzInfo, mbzInfo, sizeof(mbzInfo));
	UE_LOG(LogTemp, Display, TEXT("Address: %s\r\n"), ANSI_TO_TCHAR(mbzTmp));

	pCB->InitializeRPCInfo("127.0.0.1", "8201", "user", "pwd");

	float balance = pCB->GetBalance("mRaaXAsAZkecRc4BMnMKHKNNkjPwXqEBVs");
	sprintf(mbzTmp, "Balance: %f\r\n", balance);
	UE_LOG(LogTemp, Display, TEXT("%s\r\n"), ANSI_TO_TCHAR(mbzTmp));

	//XXUrA5NoD2L42atH8bdPN1UbrjsGVyR3Yw
	std::string strPrivKey = "cN5XUr2z3224rzFzugYNvbFjBYeYfBN7Q6HcUprm3dCnEveaVXkv";// base58 code private key for from address
	bRet = pCB->Transfer(strPrivKey.c_str(), "mWSyVNJQj4G5UxJUHdyQQr7PJxPd8fesdF", 2, "mRaaXAsAZkecRc4BMnMKHKNNkjPwXqEBVs");

	sprintf(mbzTmp, "Transfer result: %d\r\n", bRet);
	UE_LOG(LogTemp, Display, TEXT("%s\r\n"), ANSI_TO_TCHAR(mbzTmp));
}


// Called every frame
void UTestMagnaChain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

