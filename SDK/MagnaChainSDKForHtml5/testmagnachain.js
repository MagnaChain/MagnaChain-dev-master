console.log("Test MagnaChain begin.....");

// var config = 
// {    
//     rpcuser: 'user',
//     rpcpassword: 'pwd',
//     host: 'http://127.0.0.1',
//     port: 8201
// };

// var rpc = new RpcClient(config);

// var OnRpcCallback = function(status, error, jsonRet)
// {
// 	console.log("OnRpcCallback status: " + status + " error: " + error + " msg: " + JSON.stringify(jsonRet));
// }

// rpc.sendCommand(OnRpcCallback, "getinfo");

// var srcPriKey = "cMq3Wex8xRh1YrY3KhKRX2LBiLD7wGHPe5AGSUtwfAH6EM5s4ABz";
// var srcAddr = "mdUmmGLVdsYxMgKnsEJJ77m7ASmbxMdTKU";     // 100 coins
// var destAddr = "mXboeSw6t4Y6SgWZ3Ct1pKYVtXwizMGfT7";    // 0 coins

// var OnRpcSigned = function(status, error, jsonRet)
// {
//     console.log("OnRpcSigned status: " + status + " error: " + error + " msg: " + JSON.stringify(jsonRet));
// }

// var OnRpcPreTransaction = function(status, error, jsonRet)
// {
//     console.log("OnRpcPreTransaction status: " + status + " error: " + error + " msg: " + JSON.stringify(jsonRet));

//     var kTras = new Transaction(jsonRet.txhex);
// 	kTras.setOutputsFromCoins(jsonRet.coins);
	
// 	//kTras.sign(prikeys);
// 	kTras.sign(srcPriKey);
	
// 	var txsignedhex = kTras.toString();
// 	console.log("signed txhex: "+txsignedhex);

//     rpc.sendCommand(OnRpcSigned, "sendrawtransaction", txsignedhex);
// }

// // premaketransaction
// // Arguments:
// // 1. "fromaddress"                  (string, required) The address for input coins
// // 2. "toaddress"                    (string, required) Send to address
// // 3. "changeaddress"                (string, required) The address for change coins
// // 4. "amount"                       (numeric or string, required) The amount in MGC to send. eg 0.1
// // 5. "fee"                          (numeric or string, optional) The amount in MGC to for fee eg 0.0001, default 0 and will calc fee by system

// rpc.sendCommand(OnRpcPreTransaction, "premaketransaction", "mdUmmGLVdsYxMgKnsEJJ77m7ASmbxMdTKU", "mXboeSw6t4Y6SgWZ3Ct1pKYVtXwizMGfT7", "mdUmmGLVdsYxMgKnsEJJ77m7ASmbxMdTKU", 3.5);

magnachain.initializeRpc("http://127.0.0.1", 8201, "user", "pwd");

var OnTransfer = function(bSucc)
{
    console.log("Transfer result: " + bSucc);
}

magnachain.MiscFunc.transferByRpc(OnTransfer, "cMq3Wex8xRh1YrY3KhKRX2LBiLD7wGHPe5AGSUtwfAH6EM5s4ABz", "mXboeSw6t4Y6SgWZ3Ct1pKYVtXwizMGfT7", 10.0);



