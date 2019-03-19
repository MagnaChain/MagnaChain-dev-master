# SDK Content  

The SDK includes source code (magnachain.js, bn.js and other js files) in the src directory, and the interface declaration file magnachain.t.ts. With the SDK, developers can quickly and easily access blockchain support in Html5 projects.  

# SDK important class description
1. Networks：Blockchain network type  
2. PublicKey：Public key class  
3. Address：Blockchain address  
4. PrivateKey：Private key class  
5. HDPrivateKey：Layered address private key based on BIP32  
6. HDPublicKey：BIP32-based public layer to determine the address public key  
7. Script：Script  
8. Signature：Class for signing  
9. Output, Input：Input and output objects in the transaction  
10. Transaction：Trading partners
11. RpcClient: RPC function module
12. MiscFunc: Miscellaneous and convenient function modules (such as transfer)

# Instructions  

1. Create, import private key, get address  

```javascript
    // Create
    var kPirKey : PrivateKey = new PrivateKey();
        
    // import
    var kPriKey2 : PrivateKey = PrivateKey.fromWIF("XBtmtPctUv7ChMyaaN2oHxHwL58Nyd94no");
        
    // pubKey
    var kPubKey : PublicKey = kPirKey.toPublicKey();
        
    // address
    var strAddr : string = kPirKey.toAddress();
    var strAddr2 : string = kPubKey.toAddress();
```
2. BIP32 hierarchical address 

```javascript
    // Create HD private key
    var kHDKey1 : HDPrivateKey = new HDPrivateKey();
    
    // Create an HD private key from the mnemonic
    var kHDKey2 : HDPrivateKey = HDPrivateKey.fromMnemonicWord("magna chain is good");
    
    // Get the original private key
    var kPriKey : PrivateKey = kHDKey1.privateKey;
    
    // Create a sub-private key
     var kHDChildKey : HDPrivateKey = kHDKey1.deriveChild(1);
```

3. RpcClient  

```javascript
	console.log("Test MagnaChain begin.....");

	var config = 
	{    
	    rpcuser: 'user',
	    rpcpassword: 'pwd',
	    host: 'http://127.0.0.1',
	    port: 8201
	};

	var rpc = new RpcClient(config);

	var OnRpcCallback = function(status, error, jsonRet)
	{
	  console.log("OnRpcCallback status: " + status + " error: " + error + " msg: " + JSON.stringify(jsonRet));
	}

	rpc.sendCommand(OnRpcCallback, "getinfo");

	var srcPriKey = "cMq3Wex8xRh1YrY3KhKRX2LBiLD7wGHPe5AGSUtwfAH6EM5s4ABz";
	var srcAddr = "mdUmmGLVdsYxMgKnsEJJ77m7ASmbxMdTKU";     // 100 coins
	var destAddr = "mXboeSw6t4Y6SgWZ3Ct1pKYVtXwizMGfT7";    // 0 coins

	var OnRpcSigned = function(status, error, jsonRet)
	{
	    console.log("OnRpcSigned status: " + status + " error: " + error + " msg: " + JSON.stringify(jsonRet));
	}

	var OnRpcPreTransaction = function(status, error, jsonRet)
	{
	    console.log("OnRpcPreTransaction status: " + status + " error: " + error + " msg: " + JSON.stringify(jsonRet));

	    var kTras = new Transaction(jsonRet.txhex);
	  kTras.setOutputsFromCoins(jsonRet.coins);
	  
	  //kTras.sign(prikeys);
	  kTras.sign(srcPriKey);
	  
	  var txsignedhex = kTras.toString();
	  console.log("signed txhex: "+txsignedhex);

	    rpc.sendCommand(OnRpcSigned, "sendrawtransaction", txsignedhex);
	}

	// premaketransaction
	// Arguments:
	// 1. "fromaddress"                  (string, required) The address for input coins
	// 2. "toaddress"                    (string, required) Send to address
	// 3. "changeaddress"                (string, required) The address for change coins
	// 4. "amount"                       (numeric or string, required) The amount in MGC to send. eg 0.1
	// 5. "fee"                          (numeric or string, optional) The amount in MGC to for fee eg 0.0001, default 0 and will calc fee by system

	rpc.sendCommand(OnRpcPreTransaction, "premaketransaction", "mdUmmGLVdsYxMgKnsEJJ77m7ASmbxMdTKU", "mXboeSw6t4Y6SgWZ3Ct1pKYVtXwizMGfT7", "mdUmmGLVdsYxMgKnsEJJ77m7ASmbxMdTKU", 3.5);
```

4. Transaction  

    Ordinary transactions need to initialize the RPC function. 
```javascript
    console.log("Initialize Rpc...");

    magnachain.initializeRpc("http://127.0.0.1", 8201, "user", "pwd");

    var OnTransfer = function(bSucc : boolean)
    {
        console.log("Transfer result: " + bSucc);
    }

    magnachain.MiscFunc.transferByRpc(OnTransfer, "cMq3Wex8xRh1YrY3KhKRX2LBiLD7wGHPe5AGSUtwfAH6EM5s4ABz", "mXboeSw6t4Y6SgWZ3Ct1pKYVtXwizMGfT7", 10.0);
```  

    For coins issued through smart contracts, the function transfer of the smart contract can be invoked via the RPC command. 






