# Introduction  

MagnaChain SDK For Unity3D provides convenient blockchain-related functionality for a wide range of Unity3D developers. Through the SDK, the development can directly query the MagnaChain main chain and branch data, transfer funds, release smart contracts, call functions in smart contracts, and even call the functions of each blockchain directly through RPC. In any development stage of the game, developers can easily and quickly integrate MagnaChainSDK, which is so easy to play on the chain.

# Download and install  

Users can download the MagnaChain SDK for Unity3D plug-in directly from this repository and import the project directly. The directory is as follows:  

![image](https://github.com/MagnaChain/MagnaChain-SDK/blob/master/MagnaChainSDkForUnity/image/folders.png?raw=true)  

After the correct import, the MagnaChain SDK function menu will appear:  

![image](https://github.com/MagnaChain/MagnaChain-SDK/blob/master/MagnaChainSDkForUnity/image/menu.png?raw=true)  

# RPC console description  

The SDK provides a MagnaChain blockchain function debugging console. Users can easily execute various RPC commands directly in the console, issue smart contracts, and call functions in smart contracts. Execute the menu command: MagnaChain/RPCConsole to open the console:  

![image](https://github.com/MagnaChain/MagnaChain-SDK/blob/master/MagnaChainSDkForUnity/image/console.png?raw=true)  

Here you can choose different types of commands, click the Execute button to execute. The results will be displayed in the console of Unity3D.
In addition, the user can directly select the preset smart contract template in the Template Contract (the file is located in the Assets/MagnaChain/Contract/Src directory), or click Browser to select any smart contract file (.lua file) to be uploaded, and set the address. And consumption directly uploads smart contracts to the blockchain network. You can also edit the smart contract by clicking Edit before uploading, and then upload it after saving. The menu provides commands for checking for errors in smart contracts: MagnaChain/CheckSelectedContract and MagnaChain/CheckAllContract, which require smart contract files to be placed in the MagnaChain/Contract/Src directory.
The bottom part can test the functions in the calling smart contract.

In the Settings tab, you need to set the node address, account number, password and other information of the remote MagnaChain chain, and the grid type:  

![image](https://github.com/MagnaChain/MagnaChain-SDK/blob/master/MagnaChainSDkForUnity/image/setting.png?raw=true)  

The Master key name and Master key are described here in the fourth section of the SDK integration.

# SDK integration  

#### Private key and address  

For the original MagnaChain address, each one corresponds to a private key. Many blockchain operations require a private key. If we store the private key for each address, it will inevitably increase the difficulty of management and reduce security. Therefore, the SDK uses the standard BIP32 address protocol, which requires only one root key (Master Key) to generate countless addresses and corresponding private keys. Each address corresponds to a PathKey. The PathKey and the root private key can be used to calculate the sub-private key corresponding to the address. In order to facilitate the management and use of the developer, the native address is encapsulated and encoded, and each native address generates an MCL address. For each root private key, we can assign it a unique name in the current client environment when using it. (Note: This name is only for SDK and user management, it will not be uploaded to the network, so only the current client is needed. The only one in the end can be.)
The relationship between the MCL address and the native address is as follows:  

![image](https://github.com/MagnaChain/MagnaChain-SDK/blob/master/MagnaChainSDkForUnity/image/MCL.png?raw=true)  

It can be seen that the MCL Address is composed of Raw Address, Path Key, Master Key Name, and the SDK provides an interface to encode and decode the MCL Address.  

#### Code integration  

- Initialize the SDK and RPC  

After the IxMagnaChainBridge object is created, it must be called:

```
Initialize(stringstrRPCAddress, intiRPCPort, stringstrCredUserName = "", stringstrCredPwd = "", NETWORK_TYPEeNetType = NETWORK_TYPE.TEST_NET)
```

Initialize the RPC remote call system.

- Create or import a Master Key to generate a new MCL address.Each client should create or import at least one Master Key, similar to the private key of the wallet. The Master Key can create a large number of addresses, and only one Master Key can manage all. Related interface:  

```
CreateMasterExtKey(stringstrName)  
ImportMasterExtKey(stringstrName,stringstrMasterWifExtKey,stringstrLastKeyPath = null)  
```

Mnemonic creation and restoration of Master Key related interfaces:  

```
CreateMasterExtKeyByAid(string strName, string strAid)
RestoreMasterExtKeyByAid(string strAid)
```

Here strName is the name of the Master Key, the client specifies, only need to ensure that the current client does not repeat. After creating or importing a Master Key, you can use:  

```
GenerateNewMCLAddress(stringstrMasterKeyName)
```

Generate a new MCL address. As mentioned earlier, an MCL address includes Raw Address, Path Key, and Master Key Name. by: 

```
EncodeMCLAddress(stringstrName, stringstrPath, stringstrCelAddress)
DecodeMCLAddress(stringstrMCLAddress, outstringstrName, outstringstrPath, outstringstrCelAddress)
```

Can be encoded / decoded. Note that for the MagnaChain network, it only has Raw Address, where the client is encapsulated into MCL Address, which can implement a Master Key to manage countless sub-addresses, because the Raw Address and Path Key are included in the MCL Address, and the Master Key and Path Key, you can calculate the private key corresponding to the Raw Address, because many operations in the blockchain must have the private key corresponding to the address. In this way, the client only needs to ensure the security of the Master Key, so that the security of all MCL Address can be guaranteed.

- Query balance, transfer.Related interfaces:  

```
GetBalance(stringstrMCLAddress)
GetBalanceAsync(stringstrMCLAddress, ON_GET_BALANCEfnGetBalance)

TransferMCLToMCLAddress(stringstrFromMCLAddress, stringstrToMCLAddress, floatfMoney, outstringstrTxid, floatfFee = -1.0f, stringstrChangeMCLAddress = null)
TransferMCLToMCLAddressAsync(stringstrFromMCLAddress, stringstrToMCLAddress, floatfMoney, ON_TRANSFER_DONEfnTransferDone, floatfFee = -1.0f, stringstrChangeMCLAddress = null)
```

The function with the Async suffix is asynchronous.

- Smart contract release and call related functions.Related interfaces:  

```
PublishContract(outstringstrContractAddress, stringstrContractContent, stringstrCostFromMCLAddress, floatfCostAmount, stringstrSendMCLAddress, stringstrChargeMCLAdrres = null)
PublishContractAsync(ON_PUBLISH_CONTRACT_DONEfnDone, stringstrContractContent, stringstrCostFromMCLAddress, floatfCostAmount, stringstrSendMCLAddress, stringstrChargeMCLAdrres = null, objectkArg = null)

CallContractFunction(stringstrContractAddress, stringstrCostMCLAddress, floatfCostAmount, stringstrSenderMCLAddress, stringstrFuncName, object[] arrArgs, stringstrChargeMCLAddress = null)
CallContractFunctionAsync(ON_CALL_CONTRACT_FUNCTIONfnCallDone, stringstrContractAddress, stringstrCostMCLAddress, floatfCostAmount, stringstrSenderMCLAddress, stringstrFuncName, object[] arrArgs, stringstrChargeMCLAddress = null)
```

The function with the Async suffix is asynchronous.


