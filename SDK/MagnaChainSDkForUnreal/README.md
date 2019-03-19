# Introduction  

MagnaChain SDK For Unreal provides convenient blockchain-related functionality for Unreal Engine users. At any stage of development of the game, developers can easily and quickly integrate the MagnaChain SDK, which is so easy to get on the game.  

# Integration instructions  

After clone this repository, copy the Binaries and ThirdParty directories to the project root directory.MagnaChainForUnreal.Build.cs is an example that needs to be modified for integration compilation. Change the relevant directory name as shown in the example. In the Project Game property page, add the ThirdParty\MagnaChain\Include directory to the Configuration Properties->NMake->IntelliSense->Include Search path.  

# API description  

```
// this function must call first
static void Initialize(NETWORK_TYPE eNetworkType = NETWORK_TYPE::TESTNET);
static void Release();
```

Initialize the SDK environment and clear SDK related resources. Usually called when the program starts and exits.  

```
static bool GetExtKeyWif(MCExtKey* pExtKey, char* pOutWif, int iSize);
static MCExtKey ImportExtKey(const char* pExtKeyWif);

static MCKey* GetCellKey(MCExtKey* pCEK);

static bool GetKeyWif(MCKey* pKey, char* pOutWif, int iSize);
static MCKey ImportKey(const char* pWif);		
static bool GetAddress(MCKey* pKey, char* pOutWif, int iSize);
```  

The Key and address class interfaces need to be called after the SDK is initialized. MCExtKey determines the private key for the Bip32 standard layer, and the MCKey is the original private key.  

```
// create root key by aid
bool CreateRootExtKey(const char* pAid);	
MCExtKey* GetRootExtKey();
```

Create or restore the root ExtKey based on the mnemonic.

```
void InitializeRPCInfo(const char* pHost, const char* pPort, const char* pUser, const char* pPwd);
    	
float GetBalance(const char* pAddress);
bool Transfer(const char* pFromPirKeyWif, const char* pDestAddr, float fAmount, const char* pChangeAddr);
``` 

Check the balance, transfer function. First initialize the RPC.
