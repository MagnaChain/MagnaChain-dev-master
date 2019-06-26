#  Blockchain  #  
## getbestblockhash ##  
  
Returns the hash of the best (tip) block in the longest blockchain.  
  
Result:  
"hex"      (string) the block hash hex encoded  
  
Examples:  
> magnachain-cli getbestblockhash   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getbestblockhash", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getblock "blockhash" ( verbosity )  ##  
  
If verbosity is 0, returns a string that is serialized, hex-encoded data for block 'hash'.  
If verbosity is 1, returns an Object with information about block <hash>.  
If verbosity is 2, returns an Object with information about block <hash> and information about each transaction.   
  
Arguments:  
1. "blockhash"          (string, required) The block hash  
2. verbosity              (numeric, optional, default=1) 0 for hex encoded data, 1 for a json object, and 2 for json object with transaction data  
3. showtx                 (numeric, optional, default=1) 0 1 show transaction data,0 not show   
  
Result (for verbosity = 0):  
"data"             (string) A string that is serialized, hex-encoded data for block 'hash'.  
  
Result (for verbosity = 1):  
{  
  "hash" : "hash",     (string) the block hash (same as provided)  
  "confirmations" : n,   (numeric) The number of confirmations, or -1 if the block is not on the main chain  
  "size" : n,            (numeric) The block size  
  "strippedsize" : n,    (numeric) The block size excluding witness data  
  "weight" : n           (numeric) The block weight as defined in BIP 141  
  "height" : n,          (numeric) The block height or index  
  "version" : n,         (numeric) The block version  
  "versionHex" : "00000000", (string) The block version formatted in hexadecimal  
  "merkleroot" : "xxxx", (string) The merkle root  
  "tx" : [               (array of string) The transaction ids  
     "transactionid"     (string) The transaction id  
     ,...  
  ],  
  "time" : ttt,          (numeric) The block time in seconds since epoch (Jan 1 1970 GMT)  
  "mediantime" : ttt,    (numeric) The median block time in seconds since epoch (Jan 1 1970 GMT)  
  "nonce" : n,           (numeric) The nonce  
  "bits" : "1d00ffff", (string) The bits  
  "difficulty" : x.xxx,  (numeric) The difficulty  
  "chainwork" : "xxxx",  (string) Expected number of hashes required to produce the chain up to this block (in hex)  
  "previousblockhash" : "hash",  (string) The hash of the previous block  
  "nextblockhash" : "hash"       (string) The hash of the next block  
}  
  
Result (for verbosity = 2):  
{  
  ...,                     Same output as verbosity = 1.  
  "tx" : [               (array of Objects) The transactions in the format of the getrawtransaction RPC. Different from verbosity = 1 "tx" result.  
         ,...  
  ],  
  ,...                     Same output as verbosity = 1.  
}  
  
Examples:  
> magnachain-cli getblock "00000000c937983704a73af28acdec37b049d214adbda81d7e2a3dd146f6ed09"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getblock", "params": ["00000000c937983704a73af28acdec37b049d214adbda81d7e2a3dd146f6ed09"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getblockchaininfo ##  
Returns an object containing various state info regarding blockchain processing.  
  
Result:  
{  
  "chain": "xxxx",        (string) current network name as defined in BIP70 (main, test, regtest)  
  "branchid": "xxxx",     (string) if this chain is  a branchchain will show this fild  
  "blocks": xxxxxx,         (numeric) the current number of blocks processed in the server  
  "headers": xxxxxx,        (numeric) the current number of headers we have validated  
  "bestblockhash": "...", (string) the hash of the currently best block  
  "difficulty": xxxxxx,     (numeric) the current difficulty  
  "mediantime": xxxxxx,     (numeric) median time for the current best block  
  "verificationprogress": xxxx, (numeric) estimate of verification progress [0..1]  
  "chainwork": "xxxx"     (string) total amount of work in active chain, in hexadecimal  
  "pruned": xx,             (boolean) if the blocks are subject to pruning  
  "pruneheight": xxxxxx,    (numeric) lowest-height complete block stored  
  "softforks": [            (array) status of softforks in progress  
     {  
        "id": "xxxx",        (string) name of softfork  
        "version": xx,         (numeric) block version  
        "reject": {            (object) progress toward rejecting pre-softfork blocks  
           "status": xx,       (boolean) true if threshold reached  
        },  
     }, ...  
  ],  
  "bip9_softforks": {          (object) status of BIP9 softforks in progress  
     "xxxx" : {                (string) name of the softfork  
        "status": "xxxx",    (string) one of "defined", "started", "locked_in", "active", "failed"  
        "bit": xx,             (numeric) the bit (0-28) in the block version field used to signal this softfork (only for "started" status)  
        "startTime": xx,       (numeric) the minimum median time past of a block at which the bit gains its meaning  
        "timeout": xx,         (numeric) the median time past of a block at which the deployment is considered failed if not yet locked in  
        "since": xx,           (numeric) height of the first block to which the status applies  
        "statistics": {        (object) numeric statistics about BIP9 signalling for a softfork (only for "started" status)  
           "period": xx,       (numeric) the length in blocks of the BIP9 signalling period   
           "threshold": xx,    (numeric) the number of blocks with the version bit set required to activate the feature   
           "elapsed": xx,      (numeric) the number of blocks elapsed since the beginning of the current period   
           "count": xx,        (numeric) the number of blocks with the version bit set in the current period   
           "possible": xx      (boolean) returns false if there are not enough blocks left in this period to pass activation threshold   
        }  
     }  
  }  
}  
  
Examples:  
> magnachain-cli getblockchaininfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getblockchaininfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getblockcount ##  
  
Returns the number of blocks in the longest blockchain.  
  
Result:  
n    (numeric) The current block count  
  
Examples:  
> magnachain-cli getblockcount   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getblockcount", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getblockhash height ##  
  
Returns hash of block in best-block-chain at height provided.  
  
Arguments:  
1. height         (numeric, required) The height index  
  
Result:  
"hash"         (string) The block hash  
  
Examples:  
> magnachain-cli getblockhash 1000  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getblockhash", "params": [1000] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getblockheader "hash" ( verbose ) ##  
  
If verbose is false, returns a string that is serialized, hex-encoded data for blockheader 'hash'.  
If verbose is true, returns an Object with information about blockheader <hash>.  
  
Arguments:  
1. "hash"          (string, required) The block hash  
2. verbose           (boolean, optional, default=true) true for a json object, false for the hex encoded data  
  
Result (for verbose = true):  
{  
  "hash" : "hash",     (string) the block hash (same as provided)  
  "confirmations" : n,   (numeric) The number of confirmations, or -1 if the block is not on the main chain  
  "height" : n,          (numeric) The block height or index  
  "version" : n,         (numeric) The block version  
  "versionHex" : "00000000", (string) The block version formatted in hexadecimal  
  "merkleroot" : "xxxx", (string) The merkle root  
  "time" : ttt,          (numeric) The block time in seconds since epoch (Jan 1 1970 GMT)  
  "mediantime" : ttt,    (numeric) The median block time in seconds since epoch (Jan 1 1970 GMT)  
  "nonce" : n,           (numeric) The nonce  
  "bits" : "1d00ffff", (string) The bits  
  "difficulty" : x.xxx,  (numeric) The difficulty  
  "chainwork" : "0000...1f3"     (string) Expected number of hashes required to produce the current chain (in hex)  
  "previousblockhash" : "hash",  (string) The hash of the previous block  
  "nextblockhash" : "hash",      (string) The hash of the next block  
}  
  
Result (for verbose=false):  
"data"             (string) A string that is serialized, hex-encoded data for block 'hash'.  
  
Examples:  
> magnachain-cli getblockheader "00000000c937983704a73af28acdec37b049d214adbda81d7e2a3dd146f6ed09"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getblockheader", "params": ["00000000c937983704a73af28acdec37b049d214adbda81d7e2a3dd146f6ed09"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getchaintips ##  
Return information about all known tips in the block tree, including the main chain as well as orphaned branches.  
  
Result:  
[  
  {  
    "height": xxxx,         (numeric) height of the chain tip  
    "hash": "xxxx",         (string) block hash of the tip  
    "branchlen": 0          (numeric) zero for main chain  
    "status": "active"      (string) "active" for the main chain  
  },  
  {  
    "height": xxxx,  
    "hash": "xxxx",  
    "branchlen": 1          (numeric) length of branch connecting the tip to the main chain  
    "status": "xxxx"        (string) status of the chain (active, valid-fork, valid-headers, headers-only, invalid)  
  }  
]  
Possible values for status:  
1.  "invalid"               This branch contains at least one invalid block  
2.  "headers-only"          Not all blocks for this branch are available, but the headers are valid  
3.  "valid-headers"         All blocks are available for this branch, but they were never fully validated  
4.  "valid-fork"            This branch is not part of the active chain, but is fully validated  
5.  "active"                This is the tip of the active main chain, which is certainly valid  
  
Examples:  
> magnachain-cli getchaintips   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getchaintips", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getchaintipwork ##  
  
Returns the tip block work.  
  
Result:  
n    (numeric) The current block count  
  
Examples:  
> magnachain-cli getchaintipwork   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getchaintipwork", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getchaintxstats ( nblocks blockhash ) ##  
  
Compute statistics about the total number and rate of transactions in the chain.  
  
Arguments:  
1. nblocks      (numeric, optional) Size of the window in number of blocks (default: one month).  
2. "blockhash"  (string, optional) The hash of the block that ends the window.  
  
Result:  
{  
  "time": xxxxx,        (numeric) The timestamp for the statistics in UNIX format.  
  "txcount": xxxxx,     (numeric) The total number of transactions in the chain up to that point.  
  "txrate": x.xx,       (numeric) The average rate of transactions per second in the window.  
}  
  
Examples:  
> magnachain-cli getchaintxstats   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getchaintxstats", "params": [2016] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getdifficulty ##  
  
Returns the proof-of-work difficulty as a multiple of the minimum difficulty.  
  
Result:  
n.nnn       (numeric) the proof-of-work difficulty as a multiple of the minimum difficulty.  
  
Examples:  
> magnachain-cli getdifficulty   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getdifficulty", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getmempoolancestors txid (verbose) ##  
  
If txid is in the mempool, returns all in-mempool ancestors.  
  
Arguments:  
1. "txid"                 (string, required) The transaction id (must be in mempool)  
2. verbose                  (boolean, optional, default=false) True for a json object, false for array of transaction ids  
  
Result (for verbose=false):  
[                       (json array of strings)  
  "transactionid"           (string) The transaction id of an in-mempool ancestor transaction  
  ,...  
]  
  
Result (for verbose=true):  
{                           (json object)  
  "transactionid" : {       (json object)  
    "size" : n,             (numeric) virtual transaction size as defined in BIP 141. This is different from actual serialized size for witness transactions as witness data is discounted.  
    "fee" : n,              (numeric) transaction fee in MGC  
    "modifiedfee" : n,      (numeric) transaction fee with fee deltas used for mining priority  
    "time" : n,             (numeric) local time transaction entered pool in seconds since 1 Jan 1970 GMT  
    "height" : n,           (numeric) block height when transaction entered pool  
    "descendantcount" : n,  (numeric) number of in-mempool descendant transactions (including this one)  
    "descendantsize" : n,   (numeric) virtual transaction size of in-mempool descendants (including this one)  
    "descendantfees" : n,   (numeric) modified fees (see above) of in-mempool descendants (including this one)  
    "ancestorcount" : n,    (numeric) number of in-mempool ancestor transactions (including this one)  
    "ancestorsize" : n,     (numeric) virtual transaction size of in-mempool ancestors (including this one)  
    "ancestorfees" : n,     (numeric) modified fees (see above) of in-mempool ancestors (including this one)  
    "depends" : [           (array) unconfirmed transactions used as inputs for this transaction  
        "transactionid",    (string) parent transaction id  
       ... ]  
  }, ...  
}  
  
Examples:  
> magnachain-cli getmempoolancestors "mytxid"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getmempoolancestors", "params": ["mytxid"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getmempooldescendants txid (verbose) ##  
  
If txid is in the mempool, returns all in-mempool descendants.  
  
Arguments:  
1. "txid"                 (string, required) The transaction id (must be in mempool)  
2. verbose                  (boolean, optional, default=false) True for a json object, false for array of transaction ids  
  
Result (for verbose=false):  
[                       (json array of strings)  
  "transactionid"           (string) The transaction id of an in-mempool descendant transaction  
  ,...  
]  
  
Result (for verbose=true):  
{                           (json object)  
  "transactionid" : {       (json object)  
    "size" : n,             (numeric) virtual transaction size as defined in BIP 141. This is different from actual serialized size for witness transactions as witness data is discounted.  
    "fee" : n,              (numeric) transaction fee in MGC  
    "modifiedfee" : n,      (numeric) transaction fee with fee deltas used for mining priority  
    "time" : n,             (numeric) local time transaction entered pool in seconds since 1 Jan 1970 GMT  
    "height" : n,           (numeric) block height when transaction entered pool  
    "descendantcount" : n,  (numeric) number of in-mempool descendant transactions (including this one)  
    "descendantsize" : n,   (numeric) virtual transaction size of in-mempool descendants (including this one)  
    "descendantfees" : n,   (numeric) modified fees (see above) of in-mempool descendants (including this one)  
    "ancestorcount" : n,    (numeric) number of in-mempool ancestor transactions (including this one)  
    "ancestorsize" : n,     (numeric) virtual transaction size of in-mempool ancestors (including this one)  
    "ancestorfees" : n,     (numeric) modified fees (see above) of in-mempool ancestors (including this one)  
    "depends" : [           (array) unconfirmed transactions used as inputs for this transaction  
        "transactionid",    (string) parent transaction id  
       ... ]  
  }, ...  
}  
  
Examples:  
> magnachain-cli getmempooldescendants "mytxid"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getmempooldescendants", "params": ["mytxid"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getmempoolentry txid ##  
  
Returns mempool data for given transaction  
  
Arguments:  
1. "txid"                   (string, required) The transaction id (must be in mempool)  
  
Result:  
{                           (json object)  
    "size" : n,             (numeric) virtual transaction size as defined in BIP 141. This is different from actual serialized size for witness transactions as witness data is discounted.  
    "fee" : n,              (numeric) transaction fee in MGC  
    "modifiedfee" : n,      (numeric) transaction fee with fee deltas used for mining priority  
    "time" : n,             (numeric) local time transaction entered pool in seconds since 1 Jan 1970 GMT  
    "height" : n,           (numeric) block height when transaction entered pool  
    "descendantcount" : n,  (numeric) number of in-mempool descendant transactions (including this one)  
    "descendantsize" : n,   (numeric) virtual transaction size of in-mempool descendants (including this one)  
    "descendantfees" : n,   (numeric) modified fees (see above) of in-mempool descendants (including this one)  
    "ancestorcount" : n,    (numeric) number of in-mempool ancestor transactions (including this one)  
    "ancestorsize" : n,     (numeric) virtual transaction size of in-mempool ancestors (including this one)  
    "ancestorfees" : n,     (numeric) modified fees (see above) of in-mempool ancestors (including this one)  
    "depends" : [           (array) unconfirmed transactions used as inputs for this transaction  
        "transactionid",    (string) parent transaction id  
       ... ]  
}  
  
Examples:  
> magnachain-cli getmempoolentry "mytxid"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getmempoolentry", "params": ["mytxid"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getmempoolinfo ##  
  
Returns details on the active state of the TX memory pool.  
  
Result:  
{  
  "size": xxxxx,               (numeric) Current tx count  
  "bytes": xxxxx,              (numeric) Sum of all virtual transaction sizes as defined in BIP 141. Differs from actual serialized size because witness data is discounted  
  "usage": xxxxx,              (numeric) Total memory usage for the mempool  
  "maxmempool": xxxxx,         (numeric) Maximum memory usage for the mempool  
  "mempoolminfee": xxxxx       (numeric) Minimum feerate (MGC per KB) for tx to be accepted  
}  
  
Examples:  
> magnachain-cli getmempoolinfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getmempoolinfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getrawmempool ( verbose ) ##  
  
Returns all transaction ids in memory pool as a json array of string transaction ids.  
  
Hint: use getmempoolentry to fetch a specific transaction from the mempool.  
  
Arguments:  
1. verbose (boolean, optional, default=false) True for a json object, false for array of transaction ids  
  
Result: (for verbose = false):  
[                     (json array of string)  
  "transactionid"     (string) The transaction id  
  ,...  
]  
  
Result: (for verbose = true):  
{                           (json object)  
  "transactionid" : {       (json object)  
    "size" : n,             (numeric) virtual transaction size as defined in BIP 141. This is different from actual serialized size for witness transactions as witness data is discounted.  
    "fee" : n,              (numeric) transaction fee in MGC  
    "modifiedfee" : n,      (numeric) transaction fee with fee deltas used for mining priority  
    "time" : n,             (numeric) local time transaction entered pool in seconds since 1 Jan 1970 GMT  
    "height" : n,           (numeric) block height when transaction entered pool  
    "descendantcount" : n,  (numeric) number of in-mempool descendant transactions (including this one)  
    "descendantsize" : n,   (numeric) virtual transaction size of in-mempool descendants (including this one)  
    "descendantfees" : n,   (numeric) modified fees (see above) of in-mempool descendants (including this one)  
    "ancestorcount" : n,    (numeric) number of in-mempool ancestor transactions (including this one)  
    "ancestorsize" : n,     (numeric) virtual transaction size of in-mempool ancestors (including this one)  
    "ancestorfees" : n,     (numeric) modified fees (see above) of in-mempool ancestors (including this one)  
    "depends" : [           (array) unconfirmed transactions used as inputs for this transaction  
        "transactionid",    (string) parent transaction id  
       ... ]  
  }, ...  
}  
  
Examples:  
> magnachain-cli getrawmempool true  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getrawmempool", "params": [true] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## gettxout "txid" n ( include_mempool ) ##  
  
Returns details about an unspent transaction output.  
  
Arguments:  
1. "txid"             (string, required) The transaction id  
2. "n"                (numeric, required) vout number  
3. "include_mempool"  (boolean, optional) Whether to include the mempool. Default: true.     Note that an unspent output that is spent in the mempool won't appear.  
  
Result:  
{  
  "bestblock" : "hash",    (string) the block hash  
  "confirmations" : n,       (numeric) The number of confirmations  
  "value" : x.xxx,           (numeric) The transaction value in MGC  
  "scriptPubKey" : {         (json object)  
     "asm" : "code",       (string)   
     "hex" : "hex",        (string)   
     "reqSigs" : n,          (numeric) Number of required signatures  
     "type" : "pubkeyhash", (string) The type, eg pubkeyhash  
     "addresses" : [          (array of string) array of magnachain addresses  
        "address"     (string) magnachain address  
        ,...  
     ]  
  },  
  "coinbase" : true|false   (boolean) Coinbase or not  
}  
  
Examples:  
  
Get unspent transactions  
> magnachain-cli listunspent   
  
View the details  
> magnachain-cli gettxout "txid" 1  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "gettxout", "params": ["txid", 1] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## gettxoutproof ["txid",...] ( blockhash ) ##  
  
Returns a hex-encoded proof that "txid" was included in a block.  
  
NOTE: By default this function only works sometimes. This is when there is an  
unspent output in the utxo for this transaction. To make it always work,  
you need to maintain a transaction index, using the -txindex command line option or  
specify the block in which the transaction is included manually (by blockhash).  
  
Arguments:  
1. "txids"       (string) A json array of txids to filter  
    [  
      "txid"     (string) A transaction hash  
      ,...  
    ]  
2. "blockhash"   (string, optional) If specified, looks for txid in the block with this hash  
  
Result:  
"data"           (string) A string that is a serialized, hex-encoded data for the proof.  
  
## gettxoutsetinfo ##  
  
Returns statistics about the unspent transaction output set.  
Note this call may take some time.  
  
Result:  
{  
  "height":n,     (numeric) The current block height (index)  
  "bestblock": "hex",   (string) the best block hash hex  
  "transactions": n,      (numeric) The number of transactions  
  "txouts": n,            (numeric) The number of output transactions  
  "bogosize": n,          (numeric) A meaningless metric for UTXO set size  
  "hash_serialized_2": "hash", (string) The serialized hash  
  "disk_size": n,         (numeric) The estimated size of the chainstate on disk  
  "total_amount": x.xxx          (numeric) The total amount  
}  
  
Examples:  
> magnachain-cli gettxoutsetinfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "gettxoutsetinfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## preciousblock "blockhash" ##  
  
Treats a block as if it were received before others with the same work.  
  
A later preciousblock call can override the effect of an earlier one.  
  
The effects of preciousblock are not retained across restarts.  
  
Arguments:  
1. "blockhash"   (string, required) the hash of the block to mark as precious  
  
Result:  
  
Examples:  
> magnachain-cli preciousblock "blockhash"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "preciousblock", "params": ["blockhash"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## pruneblockchain ##  
  
Arguments:  
1. "height"       (numeric, required) The block height to prune up to. May be set to a discrete height, or a unix timestamp  
                  to prune blocks whose block time is at least 2 hours older than the provided timestamp.  
  
Result:  
n    (numeric) Height of the last block pruned.  
  
Examples:  
> magnachain-cli pruneblockchain 1000  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "pruneblockchain", "params": [1000] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## verifychain ( checklevel nblocks ) ##  
  
Verifies blockchain database.  
  
Arguments:  
1. checklevel   (numeric, optional, 0-4, default=3) How thorough the block verification is.  
2. nblocks      (numeric, optional, default=360, 0=all) The number of blocks to check.  
  
Result:  
true|false       (boolean) Verified or not  
  
Examples:  
> magnachain-cli verifychain   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "verifychain", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## verifytxoutproof "proof" ##  
  
Verifies that a proof points to a transaction in a block, returning the transaction it commits to  
and throwing an RPC error if the block is not in our best chain  
  
Arguments:  
1. "proof"    (string, required) The hex-encoded proof generated by gettxoutproof  
  
Result:  
["txid"]      (array, strings) The txid(s) which the proof commits to, or empty array if the proof is invalid  
  
#  Branchchain  #  
## addbranchnode branchid ip port username password ##  
  
Set branch rpc connection config.  
  
Arguments:  
1. "branchid"            (string, required) The branch txid.  
2. "ip"                  (string, required) Branch node ip.  
3. "port"                (string, required) Branch node rpc port.  
4. "usrname"             (string, required) Branch node rpc username.  
5. "password"            (string, required) Branch node rpc password.  
6. "wallet"              (string, optional) Rpc wallet  
7. "datadir"             (string, optional) taget blanch datadir  
  
Returns connection result.  
  
Result:  
    Ok or fail  
  
Examples:  
> magnachain-cli addbranchnode 4bebbe9c21ab00ca6d899d6cfe6600dc4d7e2b7f0842beba95c44abeedb42ea2 127.0.0.1 9201 "user" "clpwd" "wallet" "datadir"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "addbranchnode", "params": [4bebbe9c21ab00ca6d899d6cfe6600dc4d7e2b7f0842beba95c44abeedb42ea2 127.0.0.1 9201 "user" "clpwd" "wallet" "datadir"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## createbranchchain vseeds seedspec6 ##  
  
 create a branch chain.  
  
Arguments:  
1. "vseeds"             (string, required) The vSeeds address, eg "vseeds1.com;vseeds2.com;vseeds3.com"   
2. "seedspec6"          (string, required) The SeedSpec6 a 16-byte IPv6 address and a port, eg "00:00:00:00:00:00:00:00:00:00:ff:ff:c0:a8:3b:80:8333"   
3. "mortgageaddress"     (string, required) The magnachain Address, use to receive mortgage in main chain  
  
Returns the hash of the created branch chain.  
  
Result:  
{  
  "txid" : xxxx,        (string) The txid  
  "branchid" : xxxx,    (string) The branch id  
}  
  
Examples:  
> magnachain-cli createbranchchain vseeds.com 00:00:00:00:00:00:00:00:00:00:ff:ff:c0:a8:3b:80:8333 XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "createbranchchain", "params": [vseeds.com 00:00:00:00:00:00:00:00:00:00:ff:ff:c0:a8:3b:80:8333 XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getallbranchinfo ##  
  
 get all created branch chain info  
  
Arguments:  
  
Returns the hash of the created branch chain.  
  
Result:  
[  
  {  
    "txid" : xxxxx,          (string) The txid  
    "vseeds" : xxxxx,        (string) The vseeds  
    "seedspec6" : xxxxx,     (string) The seedspec6  
    "confirmations" : heigh  (number) The confirmation of this transaction  
    "ismaturity" : bval      (bool) Whether the branch chain is maturity  
  }  
...  
]  
  
Examples:  
> magnachain-cli getallbranchinfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getallbranchinfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getbranchchaininfo branchid ##  
  
 get a created branch chain info.  
  
Arguments:  
1. "branchid"            (string, required) The branch txid.  
  
Returns the hash of the created branch chain.  
  
Result:  
{  
  "txid" : xxxxx,          (string) The txid  
  "vseeds" : xxxxx,        (string) The vseeds  
  "seedspec6" : xxxxx,     (string) The seedspec6  
}  
  
Examples:  
> magnachain-cli getbranchchaininfo 93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getbranchchaininfo", "params": [93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getbranchchaintransaction txid  ##  
  
 Get branchchain transaction info by txid.  
  
Arguments:  
1. "txid"                  (string, required) The txid.  
  
Returns the hash of the created branch chain.  
  
Result:  
{  
  "txid" : xxx,           (string) The txid  
  "hex" : xxx,            (string) The tx hex data  
  "confirmations" : n     (int) The confirmations  
}  
  
Examples:  
> magnachain-cli getbranchchaintransaction 93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getbranchchaintransaction", "params": [93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getbranchrpcconfig branchid ##  
  
 get a created branch chain info.  
  
Arguments:  
1. "branchid"            (string, required) The branch txid.  
  
Returns the hash of the created branch chain.  
  
Result obj:  
{  
    "ip": ipaddress,  
    "port": ipaddress,  
    "rpcuser": "rpcuser",  
    "rpcpassword": "rpcpassword",  
    "wallet": "wallet",  
    "datadir": "datadir",  
}  
  
Examples:  
> magnachain-cli getbranchrpcconfig 4bebbe9c21ab00ca6d899d6cfe6600dc4d7e2b7f0842beba95c44abeedb42ea2  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getbranchrpcconfig", "params": [4bebbe9c21ab00ca6d899d6cfe6600dc4d7e2b7f0842beba95c44abeedb42ea2] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getvarietytxid "txid"  ##  
  
Get prove transaction data by txid.  
  
Arguments:  
1. "txid"             (string, required) The variety txid.  
  
Result:  
txid  
  
Examples:  
> magnachain-cli getvarietytxid 7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getvarietytxid", "params": [7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## makebranchtransaction hexstring ##  
  
 Send branch transaction to mempool of target block chain.  
  
Arguments:  
1. "hexdata"               (string, required) The transaction hex data.  
  
Returns the hash of transaction.  
  
Result:  
"txid"                  (string) The transaction id.  
  
Examples:  
> magnachain-cli makebranchtransaction 93ac2c05aebde2ff835f09cc...44dde535b845d852e  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "makebranchtransaction", "params": [93ac2c05aebde2ff835f09cc...44dde535b845d852e] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## mortgageminebranch branchid coinamount ##  
  
 mortgage coin to mine branch chain  
  
Arguments:  
1. "branchid"             (string, required) The branch id  
2. "amount"               (numeric or string, required) The amount in MGC to send. eg 100.0  
3. "address"              (string, required) The magnachain address for Redeem  
  
Returns ok or fail.  
  
Result:  
"ret"                  (string) ok or fail  
  
Examples:  
> magnachain-cli mortgageminebranch 5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db 100 XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "mortgageminebranch", "params": [5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db 100 XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## rebroadcastchaintransaction txid  ##  
  
 rebroadcast the branch chain transaction by txid, in case that transction has not be send to the target chain .  
  
Arguments:  
1. "txid"                  (string, required) The txid.  
  
Returns the hash of the created branch chain.  
  
Result:  
"ret"                  (string) ok or false  
  
Examples:  
> magnachain-cli rebroadcastchaintransaction 5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "rebroadcastchaintransaction", "params": [5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## rebroadcastredeemtransaction txid  ##  
  
 rebroadcast the redeem mortgage coin transaction by txid, in case that transction has not be send to the main chain .  
  
Arguments:  
1. "txid"                  (string, required) The txid.  
  
Returns the hash of the created branch chain.  
  
Result:  
"ret"                  (string) ok or false  
  
Examples:  
> magnachain-cli rebroadcastredeemtransaction 5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "rebroadcastredeemtransaction", "params": [5754f9e659cdf365dc2da4198046c631333d8d70e4f38f15f20e46ed5bf630db] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## redeemmortgagecoin "txid" "outindex" "statementtxid" ##  
  
Redeem mortgage coin by outpoint info(txid and vout index of coin)  
  
Arguments:  
1. "txid"             (string, required) The transaction hash of coin in main chain(MCOutPoint hash).  
2. "voutindex"        (number, required) The vout index of coin, default is 0(MCOutPoint n).  
3. "fromtx"           (string, required) The statement transactoin hex data in branch chain.  
4. "frombranchid"     (string, required) Which branch id the coin mortgage.  
5. "svpproof"         (string, required) MCSpvProof hex data.  
  
Returns txid.  
  
Result:  
{  
  "txid": xx,   (string) The new create transaction txid  
  "commit_transaction_reject_reason": xxx, (string) If has reject reason will contain this field  
}  
  
Examples:  
> magnachain-cli redeemmortgagecoin 7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 0 5dc9b823827e883e7d16988f8810be93ae8bc682df054f9b044527c388a95a89  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "redeemmortgagecoin", "params": [7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 0 5dc9b823827e883e7d16988f8810be93ae8bc682df054f9b044527c388a95a89] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## redeemmortgagecoinstatement "txid" "outindex" ##  
  
Redeem mortgage coin by outpoint info(txid and vout index of coin)  
  
Arguments:  
1. "txid"             (string, required) The transaction hash of coin.  
2. "voutindex"         (number, required) The vout index of coin, default is 0.  
  
Returns txid.  
  
Result:  
{  
  "txid": xx,   (string) The new create transaction txid  
  "commit_transaction_reject_reason": xxx, (string) If has reject reason will contain this field  
}  
  
Examples:  
> magnachain-cli redeemmortgagecoinstatement 7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 0  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "redeemmortgagecoinstatement", "params": [7de1dc8ae60b924ed68d9088b376e185cabfde330db625a6ec2234def965600a 0] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## sendtobranchchain branchid address amount,main branchid address is "main".  ##  
  
 Send an amount to a branch chain's address.  
  
Arguments:  
1. "branchid"             (string, required) Send to target chain's Branchid,if send to main chain, branchid is "main".  
2. "address"              (string, required) The target chain's magnachain address to send to.  
3. "amount"               (numeric or string, required) The amount in MGC to send. eg 0.1  
  
Returns the hash of the created branch chain.  
  
Result:  
"txid"                  (string) The transaction id.  
  
Examples:  
> magnachain-cli sendtobranchchain 93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e XS8XpbMxF5qkDp61SEaa94pwKY6UW6UQd9 0.1  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "sendtobranchchain", "params": [93ac2c05aebde2ff835f09cc3f8a4413942b0fbad9b0d7a44dde535b845d852e XS8XpbMxF5qkDp61SEaa94pwKY6UW6UQd9 0.1] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
#  Control  #  
## getinfo ##  
  
DEPRECATED. Returns an object containing various state info.  
  
Result:  
{  
  "deprecation-warning": "..." (string) warning that the getinfo command is deprecated and will be removed in 0.16  
  "version": xxxxx,           (numeric) the server version  
  "protocolversion": xxxxx,   (numeric) the protocol version  
  "walletversion": xxxxx,     (numeric) the wallet version  
  "balance": xxxxxxx,         (numeric) the total cell balance of the wallet  
  "blocks": xxxxxx,           (numeric) the current number of blocks processed in the server  
  "timeoffset": xxxxx,        (numeric) the time offset  
  "connections": xxxxx,       (numeric) the number of connections  
  "proxy": "host:port",       (string, optional) the proxy used by the server  
  "difficulty": xxxxxx,       (numeric) the current difficulty  
  "testnet": true|false,      (boolean) if the server is using testnet or not  
  "keypoololdest": xxxxxx,    (numeric) the timestamp (seconds since Unix epoch) of the oldest pre-generated key in the key pool  
  "keypoolsize": xxxx,        (numeric) how many new keys are pre-generated  
  "unlocked_until": ttt,      (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked  
  "paytxfee": x.xxxx,         (numeric) the transaction fee set in MGC/kB  
  "relayfee": x.xxxx,         (numeric) minimum relay fee for transactions in MGC/kB  
  "errors": "..."             (string) any error messages  
}  
  
Examples:  
> magnachain-cli getinfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getinfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getmemoryinfo ("mode") ##  
Returns an object containing information about memory usage.  
Arguments:  
1. "mode" determines what kind of information is returned. This argument is optional, the default mode is "stats".  
  - "stats" returns general statistics about memory usage in the daemon.  
  - "mallocinfo" returns an XML string describing low-level heap state (only available if compiled with glibc 2.10+).  
  
Result (mode "stats"):  
{  
  "locked": {               (json object) Information about locked memory manager  
    "used": xxxxx,          (numeric) Number of bytes used  
    "free": xxxxx,          (numeric) Number of bytes available in current arenas  
    "total": xxxxxxx,       (numeric) Total number of bytes managed  
    "locked": xxxxxx,       (numeric) Amount of bytes that succeeded locking. If this number is smaller than total, locking pages failed at some point and key data could be swapped to disk.  
    "chunks_used": xxxxx,   (numeric) Number allocated chunks  
    "chunks_free": xxxxx,   (numeric) Number unused chunks  
  }  
}  
  
Result (mode "mallocinfo"):  
"<malloc version="1">..."  
  
Examples:  
> magnachain-cli getmemoryinfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getmemoryinfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## help ( "command" ) ##  
  
List all commands, or get help for a specified command.  
  
Arguments:  
1. "command"     (string, optional) The command to get help on  
  
Result:  
"text"     (string) The help text  
  
## stop ##  
  
Stop Magnachain server.  
## uptime ##  
  
Returns the total uptime of the server.  
  
Result:  
ttt        (numeric) The number of seconds that the server has been running  
  
Examples:  
> magnachain-cli uptime   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "uptime", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
#  Generating  #  
## generate nblocks ( maxtries ) ##  
  
Mine up to nblocks blocks immediately (before the RPC call returns) to an address in the wallet.  
  
Arguments:  
1. nblocks      (numeric, required) How many blocks are generated immediately.  
2. maxtries     (numeric, optional) How many iterations to try (default = 1000000).  
  
Result:  
[ blockhashes ]     (array) hashes of blocks generated  
  
Examples:  
  
Generate 11 blocks  
> magnachain-cli generate 11  
  
## generateforbigboom nblocks ( maxtries ) ##  
  
Mine up to nblocks blocks immediately (before the RPC call returns) to an address in the wallet.  
  
Arguments:  
1. nblocks      (numeric, required) How many blocks are generated immediately.  
2. maxtries     (numeric, optional) How many iterations to try (default = 1000000).  
  
Result:  
[ blockhashes ]     (array) hashes of blocks generated  
  
Examples:  
  
Generate 11 blocks  
> magnachain-cli generate 11  
  
## generatetoaddress nblocks address (maxtries) ##  
  
Mine blocks immediately to a specified address (before the RPC call returns)  
  
Arguments:  
1. nblocks      (numeric, required) How many blocks are generated immediately.  
2. address      (string, required) The address to send the newly generated cell to.  
3. maxtries     (numeric, optional) How many iterations to try (default = 1000000).  
  
Result:  
[ blockhashes ]     (array) hashes of blocks generated  
  
Examples:  
  
Generate 11 blocks to myaddress  
> magnachain-cli generatetoaddress 11 "myaddress"  
  
#  Mining  #  
## getmininginfo ##  
  
Returns a json object containing mining-related information.  
Result:  
{  
  "blocks": nnn,             (numeric) The current block  
  "currentblockweight": nnn, (numeric) The last block weight  
  "currentblocktx": nnn,     (numeric) The last block transaction  
  "difficulty": xxx.xxxxx    (numeric) The current difficulty  
  "errors": "..."            (string) Current errors  
  "networkhashps": nnn,      (numeric) The network hashes per second  
  "pooledtx": n              (numeric) The size of the mempool  
  "chain": "xxxx",           (string) current network name as defined in BIP70 (main, test, regtest)  
}  
  
Examples:  
> magnachain-cli getmininginfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getmininginfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getnetworkhashps ( nblocks height ) ##  
  
Returns the estimated network hashes per second based on the last n blocks.  
Pass in [blocks] to override # of blocks, -1 specifies since last difficulty change.  
Pass in [height] to estimate the network speed at the time when a certain block was found.  
  
Arguments:  
1. nblocks     (numeric, optional, default=120) The number of blocks, or -1 for blocks since last difficulty change.  
2. height      (numeric, optional, default=-1) To estimate at the time of the given height.  
  
Result:  
x             (numeric) Hashes per second estimated  
  
Examples:  
> magnachain-cli getnetworkhashps   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getnetworkhashps", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## mineblanch2ndblock  ##  
  
Try to mine the 2nd block for branch chain.  
  
Arguments:  
  
Examples:  
  
Mine the 2nd block  
> magnachain-cli mineblanch2ndblock   
  
Using json rpc  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "mineblanch2ndblock", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## prioritisetransaction <txid> <dummy value> <fee delta> ##  
Accepts the transaction into mined blocks at a higher (or lower) priority  
  
Arguments:  
1. "txid"       (string, required) The transaction id.  
2. dummy          (numeric, optional) API-Compatibility for previous API. Must be zero or null.  
                  DEPRECATED. For forward compatibility use named arguments and omit this parameter.  
3. fee_delta      (numeric, required) The fee value (in atomes) to add (or subtract, if negative).  
                  The fee is not actually paid, only the algorithm for selecting transactions into a block  
                  considers the transaction as it would have paid a higher (or lower) fee.  
  
Result:  
true              (boolean) Returns true  
  
Examples:  
> magnachain-cli prioritisetransaction "txid" 0.0 10000  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "prioritisetransaction", "params": ["txid", 0.0, 10000] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## submitblock "hexdata"  ( "dummy" ) ##  
  
Attempts to submit new block to network.  
See https://en.magnachain.it/wiki/BIP_0022 for full specification.  
  
Arguments  
1. "hexdata"        (string, required) the hex-encoded block data to submit  
2. "dummy"          (optional) dummy value, for compatibility with BIP22. This value is ignored.  
  
Result:  
  
Examples:  
> magnachain-cli submitblock "mydata"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "submitblock", "params": ["mydata"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## updateminingreservetxsize pubcontractsize callcontractsize branchtxsize ##  
  
 set / get tx reserve size for addPackageTxs.  
  
Arguments:  
1. pubcontractsize (numeric, optional) ReservePubContractBlockDataSize  
2. callcontractsize   (numeric, optional) ReserveCallContractBlockDataSize  
3. branchtxsize      (numeric, optional) ReserveBranchTxBlockDataSize  
  
  
Result:  
{  
  "ReservePubContractBlockDataSize" : ReservePubContractBlockDataSize  
  "ReserveCallContractBlockDataSize" : ReserveCallContractBlockDataSize  
  "ReserveBranchTxBlockDataSize" : ReserveBranchTxBlockDataSize  
}  
  
Results are returned for any horizon which tracks blocks up to the confirmation target.  
  
Example:  
> magnachain-cli updateminingreservetxsize 100 1000 1000  
  
#  Network  #  
## addnode "node" "add|remove|onetry" ##  
  
Attempts to add or remove a node from the addnode list.  
Or try a connection to a node once.  
  
Arguments:  
1. "node"     (string, required) The node (see getpeerinfo for nodes)  
2. "command"  (string, required) 'add' to add a node to the list, 'remove' to remove a node from the list, 'onetry' to try a connection to the node once  
  
Examples:  
> magnachain-cli addnode "192.168.0.6:8333" "onetry"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "addnode", "params": ["192.168.0.6:8333", "onetry"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## clearbanned ##  
  
Clear all banned IPs.  
  
Examples:  
> magnachain-cli clearbanned   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "clearbanned", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## disconnectnode "[address]" [nodeid] ##  
  
Immediately disconnects from the specified peer node.  
  
Strictly one out of 'address' and 'nodeid' can be provided to identify the node.  
  
To disconnect by nodeid, either set 'address' to the empty string, or call using the named 'nodeid' argument only.  
  
Arguments:  
1. "address"     (string, optional) The IP address/port of the node  
2. "nodeid"      (number, optional) The node ID (see getpeerinfo for node IDs)  
  
Examples:  
> magnachain-cli disconnectnode "192.168.0.6:8333"  
> magnachain-cli disconnectnode "" 1  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "disconnectnode", "params": ["192.168.0.6:8333"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "disconnectnode", "params": ["", 1] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getaddednodeinfo ( "node" ) ##  
  
Returns information about the given added node, or all added nodes  
(note that onetry addnodes are not listed here)  
  
Arguments:  
1. "node"   (string, optional) If provided, return information about this specific node, otherwise all nodes are returned.  
  
Result:  
[  
  {  
    "addednode" : "192.168.0.201",   (string) The node IP address or name (as provided to addnode)  
    "connected" : true|false,          (boolean) If connected  
    "addresses" : [                    (list of objects) Only when connected = true  
       {  
         "address" : "192.168.0.201:8333",  (string) The magnachain server IP and port we're connected to  
         "connected" : "outbound"           (string) connection, inbound or outbound  
       }  
     ]  
  }  
  ,...  
]  
  
Examples:  
> magnachain-cli getaddednodeinfo "192.168.0.201"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getaddednodeinfo", "params": ["192.168.0.201"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getconnectioncount ##  
  
Returns the number of connections to other nodes.  
  
Result:  
n          (numeric) The connection count  
  
Examples:  
> magnachain-cli getconnectioncount   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getconnectioncount", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getnettotals ##  
  
Returns information about network traffic, including bytes in, bytes out,  
and current time.  
  
Result:  
{  
  "totalbytesrecv": n,   (numeric) Total bytes received  
  "totalbytessent": n,   (numeric) Total bytes sent  
  "timemillis": t,       (numeric) Current UNIX time in milliseconds  
  "uploadtarget":  
  {  
    "timeframe": n,                         (numeric) Length of the measuring timeframe in seconds  
    "target": n,                            (numeric) Target in bytes  
    "target_reached": true|false,           (boolean) True if target is reached  
    "serve_historical_blocks": true|false,  (boolean) True if serving historical blocks  
    "bytes_left_in_cycle": t,               (numeric) Bytes left in current time cycle  
    "time_left_in_cycle": t                 (numeric) Seconds left in current time cycle  
  }  
}  
  
Examples:  
> magnachain-cli getnettotals   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getnettotals", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getnetworkinfo ##  
Returns an object containing various state info regarding P2P networking.  
  
Result:  
{  
  "version": xxxxx,                      (numeric) the server version  
  "subversion": "/MagnaChain:x.x.x/",     (string) the server subversion string  
  "protocolversion": xxxxx,              (numeric) the protocol version  
  "localservices": "xxxxxxxxxxxxxxxx", (string) the services we offer to the network  
  "localrelay": true|false,              (bool) true if transaction relay is requested from peers  
  "timeoffset": xxxxx,                   (numeric) the time offset  
  "connections": xxxxx,                  (numeric) the number of connections  
  "networkactive": true|false,           (bool) whether p2p networking is enabled  
  "networks": [                          (array) information per network  
  {  
    "name": "xxx",                     (string) network (ipv4, ipv6 or onion)  
    "limited": true|false,               (boolean) is the network limited using -onlynet?  
    "reachable": true|false,             (boolean) is the network reachable?  
    "proxy": "host:port"               (string) the proxy that is used for this network, or empty if none  
    "proxy_randomize_credentials": true|false,  (string) Whether randomized credentials are used  
  }  
  ,...  
  ],  
  "relayfee": x.xxxxxxxx,                (numeric) minimum relay fee for transactions in MGC/kB  
  "incrementalfee": x.xxxxxxxx,          (numeric) minimum fee increment for mempool limiting or BIP 125 replacement in MGC/kB  
  "localaddresses": [                    (array) list of local addresses  
  {  
    "address": "xxxx",                 (string) network address  
    "port": xxx,                         (numeric) network port  
    "score": xxx                         (numeric) relative score  
  }  
  ,...  
  ]  
  "warnings": "..."                    (string) any network warnings  
}  
  
Examples:  
> magnachain-cli getnetworkinfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getnetworkinfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getpeerinfo ##  
  
Returns data about each connected network node as a json array of objects.  
  
Result:  
[  
  {  
    "id": n,                   (numeric) Peer index  
    "addr":"host:port",      (string) The IP address and port of the peer  
    "addrbind":"ip:port",    (string) Bind address of the connection to the peer  
    "addrlocal":"ip:port",   (string) Local address as reported by the peer  
    "services":"xxxxxxxxxxxxxxxx",   (string) The services offered  
    "relaytxes":true|false,    (boolean) Whether peer has asked us to relay transactions to it  
    "lastsend": ttt,           (numeric) The time in seconds since epoch (Jan 1 1970 GMT) of the last send  
    "lastrecv": ttt,           (numeric) The time in seconds since epoch (Jan 1 1970 GMT) of the last receive  
    "bytessent": n,            (numeric) The total bytes sent  
    "bytesrecv": n,            (numeric) The total bytes received  
    "conntime": ttt,           (numeric) The connection time in seconds since epoch (Jan 1 1970 GMT)  
    "timeoffset": ttt,         (numeric) The time offset in seconds  
    "pingtime": n,             (numeric) ping time (if available)  
    "minping": n,              (numeric) minimum observed ping time (if any at all)  
    "pingwait": n,             (numeric) ping wait (if non-zero)  
    "version": v,              (numeric) The peer version, such as 7001  
    "subver": "/MagnaChain:0.8.5/",  (string) The string version  
    "inbound": true|false,     (boolean) Inbound (true) or Outbound (false)  
    "addnode": true|false,     (boolean) Whether connection was due to addnode and is using an addnode slot  
    "startingheight": n,       (numeric) The starting height (block) of the peer  
    "banscore": n,             (numeric) The ban score  
    "synced_headers": n,       (numeric) The last header we have in common with this peer  
    "synced_blocks": n,        (numeric) The last block we have in common with this peer  
    "inflight": [  
       n,                        (numeric) The heights of blocks we're currently asking from this peer  
       ...  
    ],  
    "whitelisted": true|false, (boolean) Whether the peer is whitelisted  
    "bytessent_per_msg": {  
       "addr": n,              (numeric) The total bytes sent aggregated by message type  
       ...  
    },  
    "bytesrecv_per_msg": {  
       "addr": n,              (numeric) The total bytes received aggregated by message type  
       ...  
    }  
  }  
  ,...  
]  
  
Examples:  
> magnachain-cli getpeerinfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getpeerinfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listbanned ##  
  
List all banned IPs/Subnets.  
  
Examples:  
> magnachain-cli listbanned   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listbanned", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## ping ##  
  
Requests that a ping be sent to all other nodes, to measure ping time.  
Results provided in getpeerinfo, pingtime and pingwait fields are decimal seconds.  
Ping command is handled in queue with all other commands, so it measures processing backlog, not just network ping.  
  
Examples:  
> magnachain-cli ping   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "ping", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## setban "subnet" "add|remove" (bantime) (absolute) ##  
  
Attempts to add or remove an IP/Subnet from the banned list.  
  
Arguments:  
1. "subnet"       (string, required) The IP/Subnet (see getpeerinfo for nodes IP) with an optional netmask (default is /32 = single IP)  
2. "command"      (string, required) 'add' to add an IP/Subnet to the list, 'remove' to remove an IP/Subnet from the list  
3. "bantime"      (numeric, optional) time in seconds how long (or until when if [absolute] is set) the IP is banned (0 or empty means using the default time of 24h which can also be overwritten by the -bantime startup argument)  
4. "absolute"     (boolean, optional) If set, the bantime must be an absolute timestamp in seconds since epoch (Jan 1 1970 GMT)  
  
Examples:  
> magnachain-cli setban "192.168.0.6" "add" 86400  
> magnachain-cli setban "192.168.0.0/24" "add"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "setban", "params": ["192.168.0.6", "add", 86400] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## setnetworkactive true|false ##  
  
Disable/enable all p2p network activity.  
  
Arguments:  
1. "state"        (boolean, required) true to enable networking, false to disable  
  
#  Rawtransactions  #  
## combinerawtransaction ["hexstring",...] ##  
  
Combine multiple partially signed transactions into one transaction.  
The combined transaction may be another partially signed transaction or a   
fully signed transaction.  
Arguments:  
1. "txs"         (string) A json array of hex strings of partially signed transactions  
    [  
      "hexstring"     (string) A transaction hash  
      ,...  
    ]  
  
Result:  
"hex"            (string) The hex-encoded raw transaction with signature(s)  
  
Examples:  
> magnachain-cli combinerawtransaction ["myhex1", "myhex2", "myhex3"]  
  
## createrawtransaction [{"txid":"id","vout":n},...] {"address":amount,"data":"hex",...} ( locktime ) ( replaceable ) ##  
  
Create a transaction spending the given inputs and creating new outputs.  
Outputs can be addresses or data.  
Returns hex-encoded raw transaction.  
Note that the transaction's inputs are not signed, and  
it is not stored in the wallet or transmitted to the network.  
  
Arguments:  
1. "inputs"                (array, required) A json array of json objects  
     [  
       {  
         "txid":"id",    (string, required) The transaction id  
         "vout":n,         (numeric, required) The output number  
         "sequence":n      (numeric, optional) The sequence number  
       }   
       ,...  
     ]  
2. "outputs"               (object, required) a json object with outputs  
    {  
      "address": x.xxx,    (numeric or string, required) The key is the magnachain address, the numeric value (can be string) is the MGC amount  
      "data": "hex"      (string, required) The key is "data", the value is hex encoded data  
      ,...  
    }  
3. locktime                  (numeric, optional, default=0) Raw locktime. Non-0 value also locktime-activates inputs  
4. replaceable               (boolean, optional, default=false) Marks this transaction as BIP125 replaceable.  
                             Allows this transaction to be replaced by a transaction with higher fees. If provided, it is an error if explicit sequence numbers are incompatible.  
  
Result:  
"transaction"              (string) hex string of the transaction  
  
Examples:  
> magnachain-cli createrawtransaction "[{\"txid\":\"myid\",\"vout\":0}]" "{\"address\":0.01}"  
> magnachain-cli createrawtransaction "[{\"txid\":\"myid\",\"vout\":0}]" "{\"data\":\"00010203\"}"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "createrawtransaction", "params": ["[{\"txid\":\"myid\",\"vout\":0}]", "{\"address\":0.01}"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "createrawtransaction", "params": ["[{\"txid\":\"myid\",\"vout\":0}]", "{\"data\":\"00010203\"}"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## decoderawtransaction "hexstring" ##  
  
Return a JSON object representing the serialized, hex-encoded transaction.  
  
Arguments:  
1. "hexstring"      (string, required) The transaction hex string  
  
Result:  
{  
  "txid" : "id",        (string) The transaction id  
  "hash" : "id",        (string) The transaction hash (differs from txid for witness transactions)  
  "size" : n,             (numeric) The transaction size  
  "vsize" : n,            (numeric) The virtual transaction size (differs from size for witness transactions)  
  "version" : n,          (numeric) The version  
  "locktime" : ttt,       (numeric) The lock time  
  "vin" : [               (array of json objects)  
     {  
       "txid": "id",    (string) The transaction id  
       "vout": n,         (numeric) The output number  
       "scriptSig": {     (json object) The script  
         "asm": "asm",  (string) asm  
         "hex": "hex"   (string) hex  
       },  
       "txinwitness": ["hex", ...] (array of string) hex-encoded witness data (if any)  
       "sequence": n     (numeric) The script sequence number  
     }  
     ,...  
  ],  
  "vout" : [             (array of json objects)  
     {  
       "value" : x.xxx,            (numeric) The value in MGC  
       "n" : n,                    (numeric) index  
       "scriptPubKey" : {          (json object)  
         "asm" : "asm",          (string) the asm  
         "hex" : "hex",          (string) the hex  
         "reqSigs" : n,            (numeric) The required sigs  
         "type" : "pubkeyhash",  (string) The type, eg 'pubkeyhash'  
         "addresses" : [           (json array of string)  
           "12tvKAXCxZjSmdNbao16dKXC8tRWfcF5oc"   (string) magnachain address  
           ,...  
         ]  
       }  
     }  
     ,...  
  ],  
}  
  
Examples:  
> magnachain-cli decoderawtransaction "hexstring"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "decoderawtransaction", "params": ["hexstring"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## decodescript "hexstring" ##  
  
Decode a hex-encoded script.  
  
Arguments:  
1. "hexstring"     (string) the hex encoded script  
  
Result:  
{  
  "asm":"asm",   (string) Script public key  
  "hex":"hex",   (string) hex encoded public key  
  "type":"type", (string) The output type  
  "reqSigs": n,    (numeric) The required signatures  
  "addresses": [   (json array of string)  
     "address"     (string) magnachain address  
     ,...  
  ],  
  "p2sh","address" (string) address of P2SH script wrapping this redeem script (not returned if the script is already a P2SH).  
}  
  
Examples:  
> magnachain-cli decodescript "hexstring"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "decodescript", "params": ["hexstring"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## fundrawtransaction "hexstring" ( options ) ##  
  
Add inputs to a transaction until it has enough in value to meet its out value.  
This will not modify existing inputs, and will add at most one change output to the outputs.  
No existing outputs will be modified unless "subtractFeeFromOutputs" is specified.  
Note that inputs which were signed may need to be resigned after completion since in/outputs have been added.  
The inputs added will not be signed, use signrawtransaction for that.  
Note that all existing inputs must have their previous output transaction be in the wallet.  
Note that all inputs selected must be of standard form and P2SH scripts must be  
in the wallet using importaddress or addmultisigaddress (to calculate fees).  
You can see whether this is the case by checking the "solvable" field in the listunspent output.  
Only pay-to-pubkey, multisig, and P2SH versions thereof are currently supported for watch-only  
  
Arguments:  
1. "hexstring"           (string, required) The hex string of the raw transaction  
2. options                 (object, optional)  
   {  
     "changeAddress"          (string, optional, default pool address) The magnachain address to receive the change  
     "changePosition"         (numeric, optional, default random) The index of the change output  
     "includeWatching"        (boolean, optional, default false) Also select inputs which are watch only  
     "lockUnspents"           (boolean, optional, default false) Lock selected unspent outputs  
     "feeRate"                (numeric, optional, default not set: makes wallet determine the fee) Set a specific feerate (MGC per KB)  
     "subtractFeeFromOutputs" (array, optional) A json array of integers.  
                              The fee will be equally deducted from the amount of each specified output.  
                              The outputs are specified by their zero-based index, before any change output is added.  
                              Those recipients will receive less cells than you enter in their corresponding amount field.  
                              If no outputs are specified here, the sender pays the fee.  
                                  [vout_index,...]  
     "replaceable"            (boolean, optional) Marks this transaction as BIP125 replaceable.  
                              Allows this transaction to be replaced by a transaction with higher fees  
     "conf_target"            (numeric, optional) Confirmation target (in blocks)  
     "estimate_mode"          (string, optional, default=UNSET) The fee estimate mode, must be one of:  
         "UNSET"  
         "ECONOMICAL"  
         "CONSERVATIVE"  
   }  
                         for backward compatibility: passing in a true instead of an object will result in {"includeWatching":true}  
  
Result:  
{  
  "hex":       "value", (string)  The resulting raw transaction (hex-encoded string)  
  "fee":       n,         (numeric) Fee in MGC the resulting transaction pays  
  "changepos": n          (numeric) The position of the added change output, or -1  
}  
  
Examples:  
  
Create a transaction with no inputs  
> magnachain-cli createrawtransaction "[]" "{\"myaddress\":0.01}"  
  
Add sufficient unsigned inputs to meet the output value  
> magnachain-cli fundrawtransaction "rawtransactionhex"  
  
Sign the transaction  
> magnachain-cli signrawtransaction "fundedtransactionhex"  
  
Send the transaction  
> magnachain-cli sendrawtransaction "signedtransactionhex"  
  
## getbytxdiskpos "txid" ( verbose ) ##  
  
 get trransaction data by MCDiskTxPos, return value info see getrawtransaction API  
  
## getrawtransaction "txid" ( verbose ) ##  
  
NOTE: By default this function only works for mempool transactions. If the -txindex option is  
enabled, it also works for blockchain transactions.  
DEPRECATED: for now, it also works for transactions with unspent outputs.  
  
Return the raw transaction data.  
  
If verbose is 'true', returns an Object with information about 'txid'.  
If verbose is 'false' or omitted, returns a string that is serialized, hex-encoded data for 'txid'.  
  
Arguments:  
1. "txid"      (string, required) The transaction id  
2. verbose       (bool, optional, default=false) If false, return a string, otherwise return a json object  
  
Result (if verbose is not set or set to false):  
"data"      (string) The serialized, hex-encoded data for 'txid'  
  
Result (if verbose is set to true):  
{  
  "hex" : "data",       (string) The serialized, hex-encoded data for 'txid'  
  "txid" : "id",        (string) The transaction id (same as provided)  
  "hash" : "id",        (string) The transaction hash (differs from txid for witness transactions)  
  "size" : n,             (numeric) The serialized transaction size  
  "vsize" : n,            (numeric) The virtual transaction size (differs from size for witness transactions)  
  "version" : n,          (numeric) The version  
  "locktime" : ttt,       (numeric) The lock time  
  "vin" : [               (array of json objects)  
     {  
       "txid": "id",    (string) The transaction id  
       "vout": n,         (numeric)   
       "scriptSig": {     (json object) The script  
         "asm": "asm",  (string) asm  
         "hex": "hex"   (string) hex  
       },  
       "sequence": n      (numeric) The script sequence number  
       "txinwitness": ["hex", ...] (array of string) hex-encoded witness data (if any)  
     }  
     ,...  
  ],  
  "vout" : [              (array of json objects)  
     {  
       "value" : x.xxx,            (numeric) The value in MGC  
       "n" : n,                    (numeric) index  
       "scriptPubKey" : {          (json object)  
         "asm" : "asm",          (string) the asm  
         "hex" : "hex",          (string) the hex  
         "reqSigs" : n,            (numeric) The required sigs  
         "type" : "pubkeyhash",  (string) The type, eg 'pubkeyhash'  
         "addresses" : [           (json array of string)  
           "address"        (string) magnachain address  
           ,...  
         ]  
       }  
     }  
     ,...  
  ],  
  "blockhash" : "hash",   (string) the block hash  
  "confirmations" : n,      (numeric) The confirmations  
  "time" : ttt,             (numeric) The transaction time in seconds since epoch (Jan 1 1970 GMT)  
  "blocktime" : ttt         (numeric) The block time in seconds since epoch (Jan 1 1970 GMT)  
}  
  
Examples:  
> magnachain-cli getrawtransaction "mytxid"  
> magnachain-cli getrawtransaction "mytxid" true  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getrawtransaction", "params": ["mytxid", true] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## sendrawtransaction "hexstring" ( allowhighfees ) ##  
  
Submits raw transaction (serialized, hex-encoded) to local node and network.  
  
Also see createrawtransaction and signrawtransaction calls.  
  
Arguments:  
1. "hexstring"    (string, required) The hex string of the raw transaction)  
2. allowhighfees    (boolean, optional, default=false) Allow high fees  
  
Result:  
"hex"             (string) The transaction hash in hex  
  
Examples:  
  
Create a transaction  
> magnachain-cli createrawtransaction "[{\"txid\" : \"mytxid\",\"vout\":0}]" "{\"myaddress\":0.01}"  
Sign the transaction, and get back the hex  
> magnachain-cli signrawtransaction "myhex"  
  
Send the transaction (signed hex)  
> magnachain-cli sendrawtransaction "signedhex"  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "sendrawtransaction", "params": ["signedhex"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## signrawtransaction "hexstring" ( [{"txid":"id","vout":n,"scriptPubKey":"hex","redeemScript":"hex"},...] ["privatekey1",...] sighashtype ) ##  
  
Sign inputs for raw transaction (serialized, hex-encoded).  
The second optional argument (may be null) is an array of previous transaction outputs that  
this transaction depends on but may not yet be in the block chain.  
The third optional argument (may be null) is an array of base58-encoded private  
keys that, if given, will be the only keys used to sign the transaction.  
  
  
Arguments:  
1. "hexstring"     (string, required) The transaction hex string  
2. "prevtxs"       (string, optional) An json array of previous dependent transaction outputs  
     [               (json array of json objects, or 'null' if none provided)  
       {  
         "txid":"id",             (string, required) The transaction id  
         "vout":n,                  (numeric, required) The output number  
         "scriptPubKey": "hex",   (string, required) script key  
         "redeemScript": "hex",   (string, required for P2SH or P2WSH) redeem script  
         "amount": value            (numeric, required) The amount spent  
       }  
       ,...  
    ]  
3. "privkeys"     (string, optional) A json array of base58-encoded private keys for signing  
    [                  (json array of strings, or 'null' if none provided)  
      "privatekey"   (string) private key in base58-encoding  
      ,...  
    ]  
4. "sighashtype"     (string, optional, default=ALL) The signature hash type. Must be one of  
       "ALL"  
       "NONE"  
       "SINGLE"  
       "ALL|ANYONECANPAY"  
       "NONE|ANYONECANPAY"  
       "SINGLE|ANYONECANPAY"  
  
Result:  
{  
  "hex" : "value",           (string) The hex-encoded raw transaction with signature(s)  
  "complete" : true|false,   (boolean) If the transaction has a complete set of signatures  
  "errors" : [                 (json array of objects) Script verification errors (if there are any)  
    {  
      "txid" : "hash",           (string) The hash of the referenced, previous transaction  
      "vout" : n,                (numeric) The index of the output to spent and used as input  
      "scriptSig" : "hex",       (string) The hex-encoded signature script  
      "sequence" : n,            (numeric) Script sequence number  
      "error" : "text"           (string) Verification or signing error related to the input  
    }  
    ,...  
  ]  
}  
  
Examples:  
> magnachain-cli signrawtransaction "myhex"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "signrawtransaction", "params": ["myhex"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
#  Setgenerate  #  
## setgenerate generate ( genproclimit ) ##  
  
Set 'generate' true or false to turn generation on or off.  
Generation is limited to 'genproclimit' processors, -1 is unlimited.  
See the getgenerate call for the current setting.  
  
Arguments:  
1. generate         (boolean, required) Set to true to turn on generation, off to turn off.  
  
Examples:  
  
Set the generation on with a limit of one processor  
> magnachain-cli setgenerate true 1  
  
Check the setting  
> magnachain-cli getgenerate   
  
Turn off generation  
> magnachain-cli setgenerate false  
  
Using json rpc  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "setgenerate", "params": [true, 1] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
#  Util  #  
## createmultisig nrequired ["key",...] ##  
  
Creates a multi-signature address with n signature of m keys required.  
It returns a json object with the address and redeemScript.  
  
Arguments:  
1. nrequired      (numeric, required) The number of required signatures out of the n keys or addresses.  
2. "keys"       (string, required) A json array of keys which are magnachain addresses or hex-encoded public keys  
     [  
       "key"    (string) magnachain address or hex-encoded public key  
       ,...  
     ]  
  
Result:  
{  
  "address":"multisigaddress",  (string) The value of the new multisig address.  
  "redeemScript":"script"       (string) The string value of the hex-encoded redemption script.  
}  
  
Examples:  
  
Create a multisig address from 2 addresses  
> magnachain-cli createmultisig 2 "[\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\",\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\"]"  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "createmultisig", "params": [2, "[\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\",\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\"]"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## estimatefee nblocks ##  
  
DEPRECATED. Please use estimatesmartfee for more intelligent estimates.  
Estimates the approximate fee per kilobyte needed for a transaction to begin  
confirmation within nblocks blocks. Uses virtual transaction size of transaction  
as defined in BIP 141 (witness data is discounted).  
  
Arguments:  
1. nblocks     (numeric, required)  
  
Result:  
n              (numeric) estimated fee-per-kilobyte  
  
A negative value is returned if not enough transactions and blocks  
have been observed to make an estimate.  
-1 is always returned for nblocks == 1 as it is impossible to calculate  
a fee that is high enough to get reliably included in the next block.  
  
Example:  
> magnachain-cli estimatefee 6  
  
## estimatesmartfee conf_target ("estimate_mode") ##  
  
Estimates the approximate fee per kilobyte needed for a transaction to begin  
confirmation within conf_target blocks if possible and return the number of blocks  
for which the estimate is valid. Uses virtual transaction size as defined  
in BIP 141 (witness data is discounted).  
  
Arguments:  
1. conf_target     (numeric) Confirmation target in blocks (1 - 1008)  
2. "estimate_mode" (string, optional, default=CONSERVATIVE) The fee estimate mode.  
                   Whether to return a more conservative estimate which also satisfies  
                   a longer history. A conservative estimate potentially returns a  
                   higher feerate and is more likely to be sufficient for the desired  
                   target, but is not as responsive to short term drops in the  
                   prevailing fee market.  Must be one of:  
       "UNSET" (defaults to CONSERVATIVE)  
       "ECONOMICAL"  
       "CONSERVATIVE"  
  
Result:  
{  
  "feerate" : x.x,     (numeric, optional) estimate fee rate in MGC/kB  
  "errors": [ str... ] (json array of strings, optional) Errors encountered during processing  
  "blocks" : n         (numeric) block number where estimate was found  
}  
  
The request target will be clamped between 2 and the highest target  
fee estimation is able to return based on how long it has been running.  
An error is returned if not enough transactions and blocks  
have been observed to make an estimate for any number of blocks.  
  
Example:  
> magnachain-cli estimatesmartfee 6  
  
## signmessagewithprivkey "privkey" "message" ##  
  
Sign a message with the private key of an address  
  
Arguments:  
1. "privkey"         (string, required) The private key to sign the message with.  
2. "message"         (string, required) The message to create a signature of.  
  
Result:  
"signature"          (string) The signature of the message encoded in base 64  
  
Examples:  
  
Create the signature  
> magnachain-cli signmessagewithprivkey "privkey" "my message"  
  
Verify the signature  
> magnachain-cli verifymessage "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX" "signature" "my message"  
  
As json rpc  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "signmessagewithprivkey", "params": ["privkey", "my message"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## validateaddress "address" ##  
  
Return information about the given MagnaChain address.  
  
Arguments:  
1. "address"     (string, required) The MagnaChain address to validate  
  
Result:  
{  
  "isvalid" : true|false,       (boolean) If the address is valid or not. If not, this is the only property returned.  
  "address" : "address", (string) The MagnaChain address validated  
  "scriptPubKey" : "hex",       (string) The hex encoded scriptPubKey generated by the address  
  "ismine" : true|false,        (boolean) If the address is yours or not  
  "iswatchonly" : true|false,   (boolean) If the address is watchonly  
  "isscript" : true|false,      (boolean) If the key is a script  
  "script" : "type"             (string, optional) The output script type. Possible types: nonstandard, pubkey, pubkeyhash, scripthash, multisig, nulldata, witness_v0_keyhash, witness_v0_scripthash  
  "hex" : "hex",                (string, optional) The redeemscript for the p2sh address  
  "addresses"                   (string, optional) Array of addresses associated with the known redeemscript  
    [  
      "address"  
      ,...  
    ]  
  "sigsrequired" : xxxxx        (numeric, optional) Number of signatures required to spend multisig output  
  "pubkey" : "publickeyhex",    (string) The hex value of the raw public key  
  "iscompressed" : true|false,  (boolean) If the address is compressed  
  "account" : "account"         (string) DEPRECATED. The account associated with the address, "" is the default account  
  "timestamp" : timestamp,        (number, optional) The creation time of the key if available in seconds since epoch (Jan 1 1970 GMT)  
  "hdkeypath" : "keypath"       (string, optional) The HD keypath if the key is HD and available  
  "hdmasterkeyid" : "<hash160>" (string, optional) The Hash160 of the HD master pubkey  
}  
  
Examples:  
> magnachain-cli validateaddress "1PSSGeFHDnKNxiEyFrD1wcEaHr9hrQDDWc"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "validateaddress", "params": ["1PSSGeFHDnKNxiEyFrD1wcEaHr9hrQDDWc"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## verifymessage "address" "signature" "message" ##  
  
Verify a signed message  
  
Arguments:  
1. "address"         (string, required) The magnachain address to use for the signature.  
2. "signature"       (string, required) The signature provided by the signer in base 64 encoding (see signmessage).  
3. "message"         (string, required) The message that was signed.  
  
Result:  
true|false   (boolean) If the signature is verified or not.  
  
Examples:  
  
Unlock the wallet for 30 seconds  
> magnachain-cli walletpassphrase "mypassphrase" 30  
  
Create the signature  
> magnachain-cli signmessage "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX" "my message"  
  
Verify the signature  
> magnachain-cli verifymessage "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX" "signature" "my message"  
  
As json rpc  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "verifymessage", "params": ["1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX", "signature", "my message"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
#  Wallet  #  
## abandontransaction "txid" ##  
  
Mark in-wallet transaction <txid> as abandoned  
This will mark this transaction and all its in-wallet descendants as abandoned which will allow  
for their inputs to be respent.  It can be used to replace "stuck" or evicted transactions.  
It only works on transactions which are not included in a block and are not currently in the mempool.  
It has no effect on transactions which are already conflicted or abandoned.  
  
Arguments:  
1. "txid"    (string, required) The transaction id  
  
Result:  
  
Examples:  
> magnachain-cli abandontransaction "1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "abandontransaction", "params": ["1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## abortrescan ##  
  
Stops current wallet rescan triggered e.g. by an importprivkey call.  
  
Examples:  
  
Import a private key  
> magnachain-cli importprivkey "mykey"  
  
Abort the running wallet rescan  
> magnachain-cli abortrescan   
  
As a JSON-RPC call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "abortrescan", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## addmultisigaddress nrequired ["key",...] ( "account" ) ##  
  
Add a nrequired-to-sign multisignature address to the wallet.  
Each key is a MagnaChain address or hex-encoded public key.  
If 'account' is specified (DEPRECATED), assign address to that account.  
  
Arguments:  
1. nrequired        (numeric, required) The number of required signatures out of the n keys or addresses.  
2. "keys"         (string, required) A json array of magnachain addresses or hex-encoded public keys  
     [  
       "address"  (string) magnachain address or hex-encoded public key  
       ...,  
     ]  
3. "account"      (string, optional) DEPRECATED. An account to assign the addresses to.  
  
Result:  
"address"         (string) A magnachain address associated with the keys.  
  
Examples:  
  
Add a multisig address from 2 addresses  
> magnachain-cli addmultisigaddress 2 "[\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\",\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\"]"  
  
As json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "addmultisigaddress", "params": [2, "[\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\",\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\"]"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## addwitnessaddress "address" ##  
  
Add a witness address for a script (with pubkey or redeemscript known).  
It returns the witness script.  
  
Arguments:  
1. "address"       (string, required) An address known to the wallet  
  
Result:  
"witnessaddress",  (string) The value of the new address (P2SH of witness script).  
}  
  
## backupwallet "destination" ##  
  
Safely copies current wallet file to destination, which can be a directory or a path with filename.  
  
Arguments:  
1. "destination"   (string) The destination directory or file  
  
Examples:  
> magnachain-cli backupwallet "backup.dat"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "backupwallet", "params": ["backup.dat"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## bumpfee "txid" ( options )  ##  
  
Bumps the fee of an opt-in-RBF transaction T, replacing it with a new transaction B.  
An opt-in RBF transaction with the given txid must be in the wallet.  
The command will pay the additional fee by decreasing (or perhaps removing) its change output.  
If the change output is not big enough to cover the increased fee, the command will currently fail  
instead of adding new inputs to compensate. (A future implementation could improve this.)  
The command will fail if the wallet or mempool contains a transaction that spends one of T's outputs.  
By default, the new fee will be calculated automatically using estimatefee.  
The user can specify a confirmation target for estimatefee.  
Alternatively, the user can specify totalFee, or use RPC settxfee to set a higher fee rate.  
At a minimum, the new fee rate must be high enough to pay an additional new relay fee (incrementalfee  
returned by getnetworkinfo) to enter the node's mempool.  
  
Arguments:  
1. txid                  (string, required) The txid to be bumped  
2. options               (object, optional)  
   {  
     "confTarget"        (numeric, optional) Confirmation target (in blocks)  
     "totalFee"          (numeric, optional) Total fee (NOT feerate) to pay, in atomes.  
                         In rare cases, the actual fee paid might be slightly higher than the specified  
                         totalFee if the tx change output has to be removed because it is too close to  
                         the dust threshold.  
     "replaceable"       (boolean, optional, default true) Whether the new transaction should still be  
                         marked bip-125 replaceable. If true, the sequence numbers in the transaction will  
                         be left unchanged from the original. If false, any input sequence numbers in the  
                         original transaction that were less than 0xfffffffe will be increased to 0xfffffffe  
                         so the new transaction will not be explicitly bip-125 replaceable (though it may  
                         still be replaceable in practice, for example if it has unconfirmed ancestors which  
                         are replaceable).  
     "estimate_mode"     (string, optional, default=UNSET) The fee estimate mode, must be one of:  
         "UNSET"  
         "ECONOMICAL"  
         "CONSERVATIVE"  
   }  
  
Result:  
{  
  "txid":    "value",   (string)  The id of the new transaction  
  "origfee":  n,         (numeric) Fee of the replaced transaction  
  "fee":      n,         (numeric) Fee of the new transaction  
  "errors":  [ str... ] (json array of strings) Errors encountered during processing (may be empty)  
}  
  
Examples:  
  
Bump the fee, get the new transaction's txid  
> magnachain-cli bumpfee <txid>  
  
## callcontract issendcall amount "contractaddress" "senderaddress" "contract_fun_name" "params" ##  
  
send to contract .  
  
Arguments:  
1. "sendcall"            (bool, required) If commit transaction.  
2. "amount"              (number, required) The mount need to send.If amount is 0, transaction will not commit.  
3. "contractaddress"     (string, required) The contract address need to send.  
4. "senderaddress"       (string, required) The sender address, can be empty,as "".  
5. "function"	        (string, required) The function need to call.  
6. "params"              (string, optional) The function params.  
  
Examples:  
> magnachain-cli callcontract true 0 "2P8DtWVXv3mxPndCVrJaF5HqviLB4rEnpqw" "mHi1uojMVdTtksRp563oFe1PKJgeCDPTu7" testfun p1 p2 p3  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "callcontract", "params": [true 0 "2P8DtWVXv3mxPndCVrJaF5HqviLB4rEnpqw" "mHi1uojMVdTtksRp563oFe1PKJgeCDPTu7" testfun p1 p2 p3] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## dumpprivkey "address" ##  
  
Reveals the private key corresponding to 'address'.  
Then the importprivkey can be used with this output  
  
Arguments:  
1. "address"   (string, required) The magnachain address for the private key  
  
Result:  
"key"                (string) The private key  
  
Examples:  
> magnachain-cli dumpprivkey "myaddress"  
> magnachain-cli importprivkey "mykey"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "dumpprivkey", "params": ["myaddress"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## dumpprivkey "address" ##  
  
Reveals the private key corresponding to 'address'.  
Then the importprivkey can be used with this output  
  
Arguments:  
1. "address"   (string, required) The magnachain address for the private key  
  
Result:  
"key"                (string) The private key  
  
Examples:  
> magnachain-cli dumpprivkey "myaddress"  
> magnachain-cli importprivkey "mykey"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "dumpprivkey", "params": ["myaddress"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## dumpwallet "filename" ##  
  
Dumps all wallet keys in a human-readable format to a server-side file. This does not allow overwriting existing files.  
  
Arguments:  
1. "filename"    (string, required) The filename with path (either absolute or relative to magnachaind)  
  
Result:  
{                           (json object)  
  "filename" : {        (string) The filename with full absolute path  
}  
  
Examples:  
> magnachain-cli dumpwallet "test"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "dumpwallet", "params": ["test"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## encryptwallet "passphrase" ##  
  
Encrypts the wallet with 'passphrase'. This is for first time encryption.  
After this, any calls that interact with private keys such as sending or signing   
will require the passphrase to be set prior the making these calls.  
Use the walletpassphrase call for this, and then walletlock call.  
If the wallet is already encrypted, use the walletpassphrasechange call.  
Note that this will shutdown the server.  
  
Arguments:  
1. "passphrase"    (string) The pass phrase to encrypt the wallet with. It must be at least 1 character, but should be long.  
  
Examples:  
  
Encrypt your wallet  
> magnachain-cli encryptwallet "my pass phrase"  
  
Now set the passphrase to use the wallet, such as for signing or sending cell  
> magnachain-cli walletpassphrase "my pass phrase"  
  
Now we can do something like sign  
> magnachain-cli signmessage "address" "test message"  
  
Now lock the wallet again by removing the passphrase  
> magnachain-cli walletlock   
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "encryptwallet", "params": ["my pass phrase"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getaccount "address" ##  
  
DEPRECATED. Returns the account associated with the given address.  
  
Arguments:  
1. "address"         (string, required) The magnachain address for account lookup.  
  
Result:  
"accountname"        (string) the account address  
  
Examples:  
> magnachain-cli getaccount "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getaccount", "params": ["1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getaccountaddress "account" ##  
  
DEPRECATED. Returns the current MagnaChain address for receiving payments to this account.  
  
Arguments:  
1. "account"       (string, required) The account name for the address. It can also be set to the empty string "" to represent the default account. The account does not need to exist, it will be created and a new address created  if there is no account by the given name.  
  
Result:  
"address"          (string) The account magnachain address  
  
Examples:  
> magnachain-cli getaccountaddress   
> magnachain-cli getaccountaddress ""  
> magnachain-cli getaccountaddress "myaccount"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getaccountaddress", "params": ["myaccount"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getaddresscoins fromaddress ##  
  
Get coins by magnachain address  
  
Arguments:  
1. "address"                      (string, required) The address for input coins  
2. "withscript"                   (bool, optional) Option for return script or not, default false.  
  
Returns the coins of the address  
  
Result:  
[                   (array of json object)  
  {  
    "txhash" : xxx,            (string) The txid in hex  
    "outn" : xxx,              (string) The output index of vout array,number of MGC  
    "value" : xxx,             (string) The amount of output  
    "script" : xxx,            (string) The scriptPubKey in hex format  
    "confirmations" : xxx,     (number) The coin height  
  }  
  ,...  
]  
  
Examples:  
> magnachain-cli getaddresscoins XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getaddresscoins", "params": [XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getaddressesbyaccount "account" ##  
  
DEPRECATED. Returns the list of addresses for the given account.  
  
Arguments:  
1. "account"        (string, required) The account name.  
  
Result:  
[                     (json array of string)  
  "address"         (string) a magnachain address associated with the given account  
  ,...  
]  
  
Examples:  
> magnachain-cli getaddressesbyaccount "tabby"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getaddressesbyaccount", "params": ["tabby"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getbalance ( "account" minconf include_watchonly ) ##  
  
If account is not specified, returns the server's total available balance.  
If account is specified (DEPRECATED), returns the balance in the account.  
Note that the account "" is not the same as leaving the parameter out.  
The server total may be different to the balance in the default "" account.  
  
Arguments:  
1. "account"         (string, optional) DEPRECATED. The account string may be given as a  
                     specific account name to find the balance associated with wallet keys in  
                     a named account, or as the empty string ("") to find the balance  
                     associated with wallet keys not in any named account, or as "*" to find  
                     the balance associated with all wallet keys regardless of account.  
                     When this option is specified, it calculates the balance in a different  
                     way than when it is not specified, and which can count spends twice when  
                     there are conflicting pending transactions (such as those created by  
                     the bumpfee command), temporarily resulting in low or even negative  
                     balances. In general, account balance calculation is not considered  
                     reliable and has resulted in confusing outcomes, so it is recommended to  
                     avoid passing this argument.  
2. minconf           (numeric, optional, default=1) Only include transactions confirmed at least this many times.  
3. include_watchonly (bool, optional, default=false) Also include balance in watch-only addresses (see 'importaddress')  
  
Result:  
amount              (numeric) The total amount in MGC received for this account.  
  
Examples:  
  
The total amount in the wallet with 1 or more confirmations  
> magnachain-cli getbalance   
  
The total amount in the wallet at least 6 blocks confirmed  
> magnachain-cli getbalance "*" 6  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getbalance", "params": ["*", 6] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getbalanceof ( "account" minconf include_watchonly ) ##  
  
If account is not specified, returns the server's total available balance.  
If account is specified (DEPRECATED), returns the balance in the account.  
Note that the account "" is not the same as leaving the parameter out.  
The server total may be different to the balance in the default "" account.  
  
Arguments:  
1. "account"         (string, optional) DEPRECATED. The account string may be given as a  
                     specific account name to find the balance associated with wallet keys in  
                     a named account, or as the empty string ("") to find the balance  
                     associated with wallet keys not in any named account, or as "*" to find  
                     the balance associated with all wallet keys regardless of account.  
                     When this option is specified, it calculates the balance in a different  
                     way than when it is not specified, and which can count spends twice when  
                     there are conflicting pending transactions (such as those created by  
                     the bumpfee command), temporarily resulting in low or even negative  
                     balances. In general, account balance calculation is not considered  
                     reliable and has resulted in confusing outcomes, so it is recommended to  
                     avoid passing this argument.  
2. minconf           (numeric, optional, default=1) Only include transactions confirmed at least this many times.  
3. include_watchonly (bool, optional, default=false) Also include balance in watch-only addresses (see 'importaddress')  
  
Result:  
amount              (numeric) The total amount in MGC received for this account.  
  
Examples:  
  
The total amount in the wallet with 1 or more confirmations  
> magnachain-cli getbalance   
  
The total amount in the wallet at least 6 blocks confirmed  
> magnachain-cli getbalance "*" 6  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getbalance", "params": ["*", 6] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getnewaddress ( "account" ) ##  
  
Returns a new MagnaChain address for receiving payments.  
If 'account' is specified (DEPRECATED), it is added to the address book   
so payments received with the address will be credited to 'account'.  
  
Arguments:  
1. "account"        (string, optional) DEPRECATED. The account name for the address to be linked to. If not provided, the default account "" is used. It can also be set to the empty string "" to represent the default account. The account does not need to exist, it will be created if there is no account by the given name.  
  
Result:  
"address"    (string) The new magnachain address  
  
Examples:  
> magnachain-cli getnewaddress   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getnewaddress", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getrawchangeaddress ##  
  
Returns a new MagnaChain address, for receiving change.  
This is for use with raw transactions, NOT normal use.  
  
Result:  
"address"    (string) The address  
  
Examples:  
> magnachain-cli getrawchangeaddress   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getrawchangeaddress", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getreceivedbyaccount "account" ( minconf ) ##  
  
DEPRECATED. Returns the total amount received by addresses with <account> in transactions with at least [minconf] confirmations.  
  
Arguments:  
1. "account"      (string, required) The selected account, may be the default account using "".  
2. minconf          (numeric, optional, default=1) Only include transactions confirmed at least this many times.  
  
Result:  
amount              (numeric) The total amount in MGC received for this account.  
  
Examples:  
  
Amount received by the default account with at least 1 confirmation  
> magnachain-cli getreceivedbyaccount ""  
  
Amount received at the tabby account including unconfirmed amounts with zero confirmations  
> magnachain-cli getreceivedbyaccount "tabby" 0  
  
The amount with at least 6 confirmations  
> magnachain-cli getreceivedbyaccount "tabby" 6  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getreceivedbyaccount", "params": ["tabby", 6] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getreceivedbyaddress "address" ( minconf ) ##  
  
Returns the total amount received by the given address in transactions with at least minconf confirmations.  
  
Arguments:  
1. "address"         (string, required) The magnachain address for transactions.  
2. minconf             (numeric, optional, default=1) Only include transactions confirmed at least this many times.  
  
Result:  
amount   (numeric) The total amount in MGC received at this address.  
  
Examples:  
  
The amount from transactions with at least 1 confirmation  
> magnachain-cli getreceivedbyaddress "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX"  
  
The amount including unconfirmed transactions, zero confirmations  
> magnachain-cli getreceivedbyaddress "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX" 0  
  
The amount with at least 6 confirmations  
> magnachain-cli getreceivedbyaddress "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX" 6  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getreceivedbyaddress", "params": ["1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX", 6] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## gettransaction "txid" ( include_watchonly ) ##  
  
Get detailed information about in-wallet transaction <txid>  
  
Arguments:  
1. "txid"                  (string, required) The transaction id  
2. "include_watchonly"     (bool, optional, default=false) Whether to include watch-only addresses in balance calculation and details[]  
  
Result:  
{  
  "amount" : x.xxx,        (numeric) The transaction amount in MGC  
  "fee": x.xxx,            (numeric) The amount of the fee in MGC. This is negative and only available for the   
                              'send' category of transactions.  
  "confirmations" : n,     (numeric) The number of confirmations  
  "blockhash" : "hash",  (string) The block hash  
  "blockindex" : xx,       (numeric) The index of the transaction in the block that includes it  
  "blocktime" : ttt,       (numeric) The time in seconds since epoch (1 Jan 1970 GMT)  
  "txid" : "transactionid",   (string) The transaction id.  
  "time" : ttt,            (numeric) The transaction time in seconds since epoch (1 Jan 1970 GMT)  
  "timereceived" : ttt,    (numeric) The time received in seconds since epoch (1 Jan 1970 GMT)  
  "bip125-replaceable": "yes|no|unknown",  (string) Whether this transaction could be replaced due to BIP125 (replace-by-fee);  
                                                   may be unknown for unconfirmed transactions not in the mempool  
  "details" : [  
    {  
      "account" : "accountname",      (string) DEPRECATED. The account name involved in the transaction, can be "" for the default account.  
      "address" : "address",          (string) The magnachain address involved in the transaction  
      "category" : "send|receive",    (string) The category, either 'send' or 'receive'  
      "amount" : x.xxx,                 (numeric) The amount in MGC  
      "label" : "label",              (string) A comment for the address/transaction, if any  
      "vout" : n,                       (numeric) the vout value  
      "fee": x.xxx,                     (numeric) The amount of the fee in MGC. This is negative and only available for the   
                                           'send' category of transactions.  
      "abandoned": xxx                  (bool) 'true' if the transaction has been abandoned (inputs are respendable). Only available for the   
                                           'send' category of transactions.  
    }  
    ,...  
  ],  
  "hex" : "data"         (string) Raw data for transaction  
}  
  
Examples:  
> magnachain-cli gettransaction "1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d"  
> magnachain-cli gettransaction "1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d" true  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "gettransaction", "params": ["1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## getunconfirmedbalance ##  
Returns the server's total unconfirmed balance  
  
## getwalletinfo ##  
Returns an object containing various wallet state info.  
  
Result:  
{  
  "walletname": xxxxx,             (string) the wallet name  
  "walletversion": xxxxx,          (numeric) the wallet version  
  "balance": xxxxxxx,              (numeric) the total confirmed balance of the wallet in MGC  
  "unconfirmed_balance": xxx,      (numeric) the total unconfirmed balance of the wallet in MGC  
  "immature_balance": xxxxxx,      (numeric) the total immature balance of the wallet in MGC  
  "txcount": xxxxxxx,              (numeric) the total number of transactions in the wallet  
  "keypoololdest": xxxxxx,         (numeric) the timestamp (seconds since Unix epoch) of the oldest pre-generated key in the key pool  
  "keypoolsize": xxxx,             (numeric) how many new keys are pre-generated (only counts external keys)  
  "keypoolsize_hd_internal": xxxx, (numeric) how many new keys are pre-generated for internal use (used for change outputs, only appears if the wallet is using this feature, otherwise external keys are used)  
  "unlocked_until": ttt,           (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked  
  "paytxfee": x.xxxx,              (numeric) the transaction fee configuration, set in MGC/kB  
  "hdmasterkeyid": "<hash160>"     (string) the Hash160 of the HD master pubkey  
}  
  
Examples:  
> magnachain-cli getwalletinfo   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getwalletinfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## importaddress "address" ( "label" rescan p2sh ) ##  
  
Adds a script (in hex) or address that can be watched as if it were in your wallet but cannot be used to spend.  
  
Arguments:  
1. "script"           (string, required) The hex-encoded script (or address)  
2. "label"            (string, optional, default="") An optional label  
3. rescan               (boolean, optional, default=true) Rescan the wallet for transactions  
4. p2sh                 (boolean, optional, default=false) Add the P2SH version of the script as well  
  
Note: This call can take minutes to complete if rescan is true.  
If you have the full public key, you should call importpubkey instead of this.  
  
Note: If you import a non-standard raw script in hex form, outputs sending to it will be treated  
as change, and not show up in many RPCs.  
  
Examples:  
  
Import a script with rescan  
> magnachain-cli importaddress "myscript"  
  
Import using a label without rescan  
> magnachain-cli importaddress "myscript" "testing" false  
  
As a JSON-RPC call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "importaddress", "params": ["myscript", "testing", false] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## importmulti "requests" ( "options" ) ##  
  
Import addresses/scripts (with private or public keys, redeem script (P2SH)), rescanning all addresses in one-shot-only (rescan can be disabled via options).  
  
Arguments:  
1. requests     (array, required) Data to be imported  
  [     (array of json objects)  
    {  
      "scriptPubKey": "<script>" | { "address":"<address>" }, (string / json, required) Type of scriptPubKey (string for script, json for address)  
      "timestamp": timestamp | "now"                        , (integer / string, required) Creation time of the key in seconds since epoch (Jan 1 1970 GMT),  
                                                              or the string "now" to substitute the current synced blockchain time. The timestamp of the oldest  
                                                              key will determine how far back blockchain rescans need to begin for missing wallet transactions.  
                                                              "now" can be specified to bypass scanning, for keys which are known to never have been used, and  
                                                              0 can be specified to scan the entire blockchain. Blocks up to 2 hours before the earliest key  
                                                              creation time of all keys being imported by the importmulti call will be scanned.  
      "redeemscript": "<script>"                            , (string, optional) Allowed only if the scriptPubKey is a P2SH address or a P2SH scriptPubKey  
      "pubkeys": ["<pubKey>", ... ]                         , (array, optional) Array of strings giving pubkeys that must occur in the output or redeemscript  
      "keys": ["<key>", ... ]                               , (array, optional) Array of strings giving private keys whose corresponding public keys must occur in the output or redeemscript  
      "internal": <true>                                    , (boolean, optional, default: false) Stating whether matching outputs should be treated as not incoming payments  
      "watchonly": <true>                                   , (boolean, optional, default: false) Stating whether matching outputs should be considered watched even when they're not spendable, only allowed if keys are empty  
      "label": <label>                                      , (string, optional, default: '') Label to assign to the address (aka account name, for now), only allowed with internal=false  
    }  
  ,...  
  ]  
2. options                 (json, optional)  
  {  
     "rescan": <false>,         (boolean, optional, default: true) Stating if should rescan the blockchain after all imports  
  }  
  
Examples:  
> magnachain-cli importmulti '[{ "scriptPubKey": { "address": "<my address>" }, "timestamp":1455191478 }, { "scriptPubKey": { "address": "<my 2nd address>" }, "label": "example 2", "timestamp": 1455191480 }]'  
> magnachain-cli importmulti '[{ "scriptPubKey": { "address": "<my address>" }, "timestamp":1455191478 }]' '{ "rescan": false}'  
  
Response is an array with the same size as the input that has the execution result :  
  [{ "success": true } , { "success": false, "error": { "code": -1, "message": "Internal Server Error"} }, ... ]  
  
## importprivkey "privkey" ( "label" ) ( rescan ) ##  
  
Adds a private key (as returned by dumpprivkey) to your wallet.  
  
Arguments:  
1. "privkey"          (string, required) The private key (see dumpprivkey)  
2. "label"            (string, optional, default="") An optional label  
3. rescan               (boolean, optional, default=true) Rescan the wallet for transactions  
  
Note: This call can take minutes to complete if rescan is true.  
  
Examples:  
  
Dump a private key  
> magnachain-cli dumpprivkey "myaddress"  
  
Import the private key with rescan  
> magnachain-cli importprivkey "mykey"  
  
Import using a label and without rescan  
> magnachain-cli importprivkey "mykey" "testing" false  
  
Import using default blank label and without rescan  
> magnachain-cli importprivkey "mykey" "" false  
  
As a JSON-RPC call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "importprivkey", "params": ["mykey", "testing", false] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## importprunedfunds ##  
  
Imports funds without rescan. Corresponding address or script must previously be included in wallet. Aimed towards pruned wallets. The end-user is responsible to import additional transactions that subsequently spend the imported outputs or rescan after the point in the blockchain the transaction is included.  
  
Arguments:  
1. "rawtransaction" (string, required) A raw transaction in hex funding an already-existing address in wallet  
2. "txoutproof"     (string, required) The hex output from gettxoutproof that contains the transaction  
  
## importpubkey "pubkey" ( "label" rescan ) ##  
  
Adds a public key (in hex) that can be watched as if it were in your wallet but cannot be used to spend.  
  
Arguments:  
1. "pubkey"           (string, required) The hex-encoded public key  
2. "label"            (string, optional, default="") An optional label  
3. rescan               (boolean, optional, default=true) Rescan the wallet for transactions  
  
Note: This call can take minutes to complete if rescan is true.  
  
Examples:  
  
Import a public key with rescan  
> magnachain-cli importpubkey "mypubkey"  
  
Import using a label without rescan  
> magnachain-cli importpubkey "mypubkey" "testing" false  
  
As a JSON-RPC call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "importpubkey", "params": ["mypubkey", "testing", false] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## importwallet "filename" ##  
  
Imports keys from a wallet dump file (see dumpwallet).  
  
Arguments:  
1. "filename"    (string, required) The wallet file  
  
Examples:  
  
Dump the wallet  
> magnachain-cli dumpwallet "test"  
  
Import the wallet  
> magnachain-cli importwallet "test"  
  
Import using the json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "importwallet", "params": ["test"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## keypoolrefill ( newsize ) ##  
  
Fills the keypool.  
  
Arguments  
1. newsize     (numeric, optional, default=100) The new keypool size  
  
Examples:  
> magnachain-cli keypoolrefill   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "keypoolrefill", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listaccounts ( minconf include_watchonly) ##  
  
DEPRECATED. Returns Object that has account names as keys, account balances as values.  
  
Arguments:  
1. minconf             (numeric, optional, default=1) Only include transactions with at least this many confirmations  
2. include_watchonly   (bool, optional, default=false) Include balances in watch-only addresses (see 'importaddress')  
  
Result:  
{                      (json object where keys are account names, and values are numeric balances  
  "account": x.xxx,  (numeric) The property name is the account name, and the value is the total balance for the account.  
  ...  
}  
  
Examples:  
  
List account balances where there at least 1 confirmation  
> magnachain-cli listaccounts   
  
List account balances including zero confirmation transactions  
> magnachain-cli listaccounts 0  
  
List account balances for 6 or more confirmations  
> magnachain-cli listaccounts 6  
  
As json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listaccounts", "params": [6] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listaddressgroupings ##  
  
Lists groups of addresses which have had their common ownership  
made public by common use as inputs or as the resulting change  
in past transactions  
  
Result:  
[  
  [  
    [  
      "address",            (string) The magnachain address  
      amount,                 (numeric) The amount in MGC  
      "account"             (string, optional) DEPRECATED. The account  
    ]  
    ,...  
  ]  
  ,...  
]  
  
Examples:  
> magnachain-cli listaddressgroupings   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listaddressgroupings", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listlockunspent ##  
  
Returns list of temporarily unspendable outputs.  
See the lockunspent call to lock and unlock transactions for spending.  
  
Result:  
[  
  {  
    "txid" : "transactionid",     (string) The transaction id locked  
    "vout" : n                      (numeric) The vout value  
  }  
  ,...  
]  
  
Examples:  
  
List the unspent transactions  
> magnachain-cli listunspent   
  
Lock an unspent transaction  
> magnachain-cli lockunspent false "[{\"txid\":\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\",\"vout\":1}]"  
  
List the locked transactions  
> magnachain-cli listlockunspent   
  
Unlock the transaction again  
> magnachain-cli lockunspent true "[{\"txid\":\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\",\"vout\":1}]"  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listlockunspent", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listmortgagecoins ( minconf maxconf  ["addresses",...] [include_unsafe] [query_options]) ##  
  
Returns array of unspent transaction outputs  
with between minconf and maxconf (inclusive) confirmations.  
Optionally filter to only include txouts paid to specified addresses.  
  
Arguments:  
1. minconf          (numeric, optional, default=1) The minimum confirmations to filter  
2. maxconf          (numeric, optional, default=9999999) The maximum confirmations to filter  
3. "addresses"      (string) A json array of bitcoin addresses to filter  
    [  
      "address"     (string) bitcoin address  
      ,...  
    ]  
4. include_unsafe (bool, optional, default=true) Include outputs that are not safe to spend  
                  See description of "safe" attribute below.  
5. query_options    (json, optional) JSON with query options  
    {  
      "minimumAmount"    (numeric or string, default=0) Minimum value of each UTXO in MGC  
      "maximumAmount"    (numeric or string, default=unlimited) Maximum value of each UTXO in MGC  
      "maximumCount"     (numeric or string, default=unlimited) Maximum number of UTXOs  
      "minimumSumAmount" (numeric or string, default=unlimited) Minimum sum value of all UTXOs in MGC  
    }  
  
Result  
[                   (array of json object)  
  {  
    "txid" : "txid",          (string) the transaction id   
    "vout" : n,               (numeric) the vout value  
    "address" : "address",    (string) the bitcoin address  
    "account" : "account",    (string) DEPRECATED. The associated account, or "" for the default account  
    "scriptPubKey" : "key",   (string) the script key  
    "amount" : x.xxx,         (numeric) the transaction output amount in MGC  
    "confirmations" : n,      (numeric) The number of confirmations  
    "redeemScript" : n        (string) The redeemScript if scriptPubKey is P2SH  
    "spendable" : xxx,        (bool) Whether we have the private keys to spend this output  
    "solvable" : xxx,         (bool) Whether we know how to spend this output, ignoring the lack of keys  
    "safe" : xxx              (bool) Whether this output is considered safe to spend. Unconfirmed transactions  
                              from outside keys and unconfirmed replacement transactions are considered unsafe  
                              and are not eligible for spending by fundrawtransaction and sendtoaddress.  
  }  
  ,...  
]  
  
Examples  
> magnachain-cli listmortgagecoins   
> magnachain-cli listmortgagecoins 6 9999999 "[\"1PGFqEzfmQch1gKD3ra4k18PNj3tTUUSqg\",\"1LtvqCaApEdUGFkpKMM4MstjcaL4dKg8SP\"]"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listmortgagecoins", "params": [6, 9999999 "[\"1PGFqEzfmQch1gKD3ra4k18PNj3tTUUSqg\",\"1LtvqCaApEdUGFkpKMM4MstjcaL4dKg8SP\"]"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
> magnachain-cli listmortgagecoins 6 9999999 '[]' true '{ "minimumAmount": 0.005 }'  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listmortgagecoins", "params": [6, 9999999, [] , true, { "minimumAmount": 0.005 } ] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listreceivedbyaccount ( minconf include_empty include_watchonly) ##  
  
DEPRECATED. List balances by account.  
  
Arguments:  
1. minconf           (numeric, optional, default=1) The minimum number of confirmations before payments are included.  
2. include_empty     (bool, optional, default=false) Whether to include accounts that haven't received any payments.  
3. include_watchonly (bool, optional, default=false) Whether to include watch-only addresses (see 'importaddress').  
  
Result:  
[  
  {  
    "involvesWatchonly" : true,   (bool) Only returned if imported addresses were involved in transaction  
    "account" : "accountname",  (string) The account name of the receiving account  
    "amount" : x.xxx,             (numeric) The total amount received by addresses with this account  
    "confirmations" : n,          (numeric) The number of confirmations of the most recent transaction included  
    "label" : "label"           (string) A comment for the address/transaction, if any  
  }  
  ,...  
]  
  
Examples:  
> magnachain-cli listreceivedbyaccount   
> magnachain-cli listreceivedbyaccount 6 true  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listreceivedbyaccount", "params": [6, true, true] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listreceivedbyaddress ( minconf include_empty include_watchonly) ##  
  
List balances by receiving address.  
  
Arguments:  
1. minconf           (numeric, optional, default=1) The minimum number of confirmations before payments are included.  
2. include_empty     (bool, optional, default=false) Whether to include addresses that haven't received any payments.  
3. include_watchonly (bool, optional, default=false) Whether to include watch-only addresses (see 'importaddress').  
  
Result:  
[  
  {  
    "involvesWatchonly" : true,        (bool) Only returned if imported addresses were involved in transaction  
    "address" : "receivingaddress",  (string) The receiving address  
    "account" : "accountname",       (string) DEPRECATED. The account of the receiving address. The default account is "".  
    "amount" : x.xxx,                  (numeric) The total amount in MGC received by the address  
    "confirmations" : n,               (numeric) The number of confirmations of the most recent transaction included  
    "label" : "label",               (string) A comment for the address/transaction, if any  
    "txids": [  
       n,                                (numeric) The ids of transactions received with the address   
       ...  
    ]  
  }  
  ,...  
]  
  
Examples:  
> magnachain-cli listreceivedbyaddress   
> magnachain-cli listreceivedbyaddress 6 true  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listreceivedbyaddress", "params": [6, true, true] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listsinceblock ( "blockhash" target_confirmations include_watchonly include_removed ) ##  
  
Get all transactions in blocks since block [blockhash], or all transactions if omitted.  
If "blockhash" is no longer a part of the main chain, transactions from the fork point onward are included.  
Additionally, if include_removed is set, transactions affecting the wallet which were removed are returned in the "removed" array.  
  
Arguments:  
1. "blockhash"            (string, optional) The block hash to list transactions since  
2. target_confirmations:    (numeric, optional, default=1) Return the nth block hash from the main chain. e.g. 1 would mean the best block hash. Note: this is not used as a filter, but only affects [lastblock] in the return value  
3. include_watchonly:       (bool, optional, default=false) Include transactions to watch-only addresses (see 'importaddress')  
4. include_removed:         (bool, optional, default=true) Show transactions that were removed due to a reorg in the "removed" array  
                                                           (not guaranteed to work on pruned nodes)  
  
Result:  
{  
  "transactions": [  
    "account":"accountname",       (string) DEPRECATED. The account name associated with the transaction. Will be "" for the default account.  
    "address":"address",    (string) The magnachain address of the transaction. Not present for move transactions (category = move).  
    "category":"send|receive",     (string) The transaction category. 'send' has negative amounts, 'receive' has positive amounts.  
    "amount": x.xxx,          (numeric) The amount in MGC. This is negative for the 'send' category, and for the 'move' category for moves   
                                          outbound. It is positive for the 'receive' category, and for the 'move' category for inbound funds.  
    "vout" : n,               (numeric) the vout value  
    "fee": x.xxx,             (numeric) The amount of the fee in MGC. This is negative and only available for the 'send' category of transactions.  
    "confirmations": n,       (numeric) The number of confirmations for the transaction. Available for 'send' and 'receive' category of transactions.  
                                          When it's < 0, it means the transaction conflicted that many blocks ago.  
    "blockhash": "hashvalue",     (string) The block hash containing the transaction. Available for 'send' and 'receive' category of transactions.  
    "blockindex": n,          (numeric) The index of the transaction in the block that includes it. Available for 'send' and 'receive' category of transactions.  
    "blocktime": xxx,         (numeric) The block time in seconds since epoch (1 Jan 1970 GMT).  
    "txid": "transactionid",  (string) The transaction id. Available for 'send' and 'receive' category of transactions.  
    "time": xxx,              (numeric) The transaction time in seconds since epoch (Jan 1 1970 GMT).  
    "timereceived": xxx,      (numeric) The time received in seconds since epoch (Jan 1 1970 GMT). Available for 'send' and 'receive' category of transactions.  
    "bip125-replaceable": "yes|no|unknown",  (string) Whether this transaction could be replaced due to BIP125 (replace-by-fee);  
                                                   may be unknown for unconfirmed transactions not in the mempool  
    "abandoned": xxx,         (bool) 'true' if the transaction has been abandoned (inputs are respendable). Only available for the 'send' category of transactions.  
    "comment": "...",       (string) If a comment is associated with the transaction.  
    "label" : "label"       (string) A comment for the address/transaction, if any  
    "to": "...",            (string) If a comment to is associated with the transaction.  
  ],  
  "removed": [  
    <structure is the same as "transactions" above, only present if include_removed=true>  
    Note: transactions that were readded in the active chain will appear as-is in this array, and may thus have a positive confirmation count.  
  ],  
  "lastblock": "lastblockhash"     (string) The hash of the block (target_confirmations-1) from the best block on the main chain. This is typically used to feed back into listsinceblock the next time you call it. So you would generally use a target_confirmations of say 6, so you will be continually re-notified of transactions until they've reached 6 confirmations plus any new ones  
}  
  
Examples:  
> magnachain-cli listsinceblock   
> magnachain-cli listsinceblock "000000000000000bacf66f7497b7dc45ef753ee9a7d38571037cdb1a57f663ad" 6  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listsinceblock", "params": ["000000000000000bacf66f7497b7dc45ef753ee9a7d38571037cdb1a57f663ad", 6] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listtransactions ( "account" count skip include_watchonly) ##  
  
Returns up to 'count' most recent transactions skipping the first 'from' transactions for account 'account'.  
  
Arguments:  
1. "account"    (string, optional) DEPRECATED. The account name. Should be "*".  
2. count          (numeric, optional, default=10) The number of transactions to return  
3. skip           (numeric, optional, default=0) The number of transactions to skip  
4. include_watchonly (bool, optional, default=false) Include transactions to watch-only addresses (see 'importaddress')  
  
Result:  
[  
  {  
    "account":"accountname",       (string) DEPRECATED. The account name associated with the transaction.   
                                                It will be "" for the default account.  
    "address":"address",    (string) The magnachain address of the transaction. Not present for   
                                                move transactions (category = move).  
    "category":"send|receive|move|sendfee", (string) The transaction category. 'move' is a local (off blockchain)  
                                                transaction between accounts, and not associated with an address,  
                                                transaction id or block. 'send' and 'receive' transactions are   
                                                associated with an address, transaction id and block details  
                                                'sendfee' is for transaction like smartcontract, pay for fee, is not a transfer  
    "amount": x.xxx,          (numeric) The amount in MGC. This is negative for the 'send' category, and for the  
                                         'move' category for moves outbound. It is positive for the 'receive' category,  
                                         and for the 'move' category for inbound funds.  
    "label": "label",       (string) A comment for the address/transaction, if any  
    "vout": n,                (numeric) the vout value  
    "fee": x.xxx,             (numeric) The amount of the fee in MGC. This is negative and only available for the   
                                         'send' category of transactions.  
    "confirmations": n,       (numeric) The number of confirmations for the transaction. Available for 'send' and   
                                         'receive' category of transactions. Negative confirmations indicate the  
                                         transaction conflicts with the block chain  
    "trusted": xxx,           (bool) Whether we consider the outputs of this unconfirmed transaction safe to spend.  
    "blockhash": "hashvalue", (string) The block hash containing the transaction. Available for 'send' and 'receive'  
                                          category of transactions.  
    "blockindex": n,          (numeric) The index of the transaction in the block that includes it. Available for 'send' and 'receive'  
                                          category of transactions.  
    "blocktime": xxx,         (numeric) The block time in seconds since epoch (1 Jan 1970 GMT).  
    "txid": "transactionid", (string) The transaction id. Available for 'send' and 'receive' category of transactions.  
    "time": xxx,              (numeric) The transaction time in seconds since epoch (midnight Jan 1 1970 GMT).  
    "timereceived": xxx,      (numeric) The time received in seconds since epoch (midnight Jan 1 1970 GMT). Available   
                                          for 'send' and 'receive' category of transactions.  
    "comment": "...",       (string) If a comment is associated with the transaction.  
    "otheraccount": "accountname",  (string) DEPRECATED. For the 'move' category of transactions, the account the funds came   
                                          from (for receiving funds, positive amounts), or went to (for sending funds,  
                                          negative amounts).  
    "bip125-replaceable": "yes|no|unknown",  (string) Whether this transaction could be replaced due to BIP125 (replace-by-fee);  
                                                     may be unknown for unconfirmed transactions not in the mempool  
    "abandoned": xxx          (bool) 'true' if the transaction has been abandoned (inputs are respendable). Only available for the   
                                         'send' category of transactions.  
  }  
]  
  
Examples:  
  
List the most recent 10 transactions in the systems  
> magnachain-cli listtransactions   
  
List transactions 100 to 120  
> magnachain-cli listtransactions "*" 20 100  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listtransactions", "params": ["*", 20, 100] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listunspent ( minconf maxconf  ["addresses",...] [include_unsafe] [query_options]) ##  
  
Returns array of unspent transaction outputs  
with between minconf and maxconf (inclusive) confirmations.  
Optionally filter to only include txouts paid to specified addresses.  
  
Arguments:  
1. minconf          (numeric, optional, default=1) The minimum confirmations to filter  
2. maxconf          (numeric, optional, default=9999999) The maximum confirmations to filter  
3. "addresses"      (string) A json array of magnachain addresses to filter  
    [  
      "address"     (string) magnachain address  
      ,...  
    ]  
4. include_unsafe (bool, optional, default=true) Include outputs that are not safe to spend  
                  See description of "safe" attribute below.  
5. query_options    (json, optional) JSON with query options  
    {  
      "minimumAmount"    (numeric or string, default=0) Minimum value of each UTXO in MGC  
      "maximumAmount"    (numeric or string, default=unlimited) Maximum value of each UTXO in MGC  
      "maximumCount"     (numeric or string, default=unlimited) Maximum number of UTXOs  
      "minimumSumAmount" (numeric or string, default=unlimited) Minimum sum value of all UTXOs in MGC  
    }  
  
Result  
[                   (array of json object)  
  {  
    "txid" : "txid",          (string) the transaction id   
    "vout" : n,               (numeric) the vout value  
    "address" : "address",    (string) the magnachain address  
    "account" : "account",    (string) DEPRECATED. The associated account, or "" for the default account  
    "scriptPubKey" : "key",   (string) the script key  
    "amount" : x.xxx,         (numeric) the transaction output amount in MGC  
    "confirmations" : n,      (numeric) The number of confirmations  
    "redeemScript" : n        (string) The redeemScript if scriptPubKey is P2SH  
    "spendable" : xxx,        (bool) Whether we have the private keys to spend this output  
    "solvable" : xxx,         (bool) Whether we know how to spend this output, ignoring the lack of keys  
    "safe" : xxx              (bool) Whether this output is considered safe to spend. Unconfirmed transactions  
                              from outside keys and unconfirmed replacement transactions are considered unsafe  
                              and are not eligible for spending by fundrawtransaction and sendtoaddress.  
  }  
  ,...  
]  
  
Examples  
> magnachain-cli listunspent   
> magnachain-cli listunspent 6 9999999 "[\"1PGFqEzfmQch1gKD3ra4k18PNj3tTUUSqg\",\"1LtvqCaApEdUGFkpKMM4MstjcaL4dKg8SP\"]"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listunspent", "params": [6, 9999999 "[\"1PGFqEzfmQch1gKD3ra4k18PNj3tTUUSqg\",\"1LtvqCaApEdUGFkpKMM4MstjcaL4dKg8SP\"]"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
> magnachain-cli listunspent 6 9999999 '[]' true '{ "minimumAmount": 0.005 }'  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listunspent", "params": [6, 9999999, [] , true, { "minimumAmount": 0.005 } ] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## listwallets ##  
Returns a list of currently loaded wallets.  
For full information on the wallet, use "getwalletinfo"  
  
Result:  
[                         (json array of strings)  
  "walletname"            (string) the wallet name  
   ...  
]  
  
Examples:  
> magnachain-cli listwallets   
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "listwallets", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## lockunspent unlock ([{"txid":"txid","vout":n},...]) ##  
  
Updates list of temporarily unspendable outputs.  
Temporarily lock (unlock=false) or unlock (unlock=true) specified transaction outputs.  
If no transaction outputs are specified when unlocking then all current locked transaction outputs are unlocked.  
A locked transaction output will not be chosen by automatic coin selection, when spending cells.  
Locks are stored in memory only. Nodes start with zero locked outputs, and the locked output list  
is always cleared (by virtue of process exit) when a node stops or fails.  
Also see the listunspent call  
  
Arguments:  
1. unlock            (boolean, required) Whether to unlock (true) or lock (false) the specified transactions  
2. "transactions"  (string, optional) A json array of objects. Each object the txid (string) vout (numeric)  
     [           (json array of json objects)  
       {  
         "txid":"id",    (string) The transaction id  
         "vout": n         (numeric) The output number  
       }  
       ,...  
     ]  
  
Result:  
true|false    (boolean) Whether the command was successful or not  
  
Examples:  
  
List the unspent transactions  
> magnachain-cli listunspent   
  
Lock an unspent transaction  
> magnachain-cli lockunspent false "[{\"txid\":\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\",\"vout\":1}]"  
  
List the locked transactions  
> magnachain-cli listlockunspent   
  
Unlock the transaction again  
> magnachain-cli lockunspent true "[{\"txid\":\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\",\"vout\":1}]"  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "lockunspent", "params": [false, "[{\"txid\":\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\",\"vout\":1}]"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## move "fromaccount" "toaccount" amount ( minconf "comment" ) ##  
  
DEPRECATED. Move a specified amount from one account in your wallet to another.  
  
Arguments:  
1. "fromaccount"   (string, required) The name of the account to move funds from. May be the default account using "".  
2. "toaccount"     (string, required) The name of the account to move funds to. May be the default account using "".  
3. amount            (numeric) Quantity of MGC to move between accounts.  
4. (dummy)           (numeric, optional) Ignored. Remains for backward compatibility.  
5. "comment"       (string, optional) An optional comment, stored in the wallet only.  
  
Result:  
true|false           (boolean) true if successful.  
  
Examples:  
  
Move 0.01 MGC from the default account to the account named tabby  
> magnachain-cli move "" "tabby" 0.01  
  
Move 0.01 MGC timotei to akiko with a comment and funds have 6 confirmations  
> magnachain-cli move "timotei" "akiko" 0.01 6 "happy birthday!"  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "move", "params": ["timotei", "akiko", 0.01, 6, "happy birthday!"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## precallcontract "address"  ##  
  
send to contract .  
  
Arguments:  
1. "contractaddress"       (string, required) The contract address need to send  
2. "fundaddress"	          (string, required) The magnachain address of fund to be used  
3. "amount"	              (number, required) The amount need to be send to contract  
4. "senderpubkey"          (string, required) The hex pubkey of address of sender  
5. "changeaddress"         (string, required) The address for change may be  
6. "issendcall"            (bool, required) Is need to call or just query, "1" is true  
7. "function"	          (string, required) The function name need to be called  
8. "params"                (string, optional) The function params, ...  
Return:  
{  
  "call_return" : "data",       (array) The return value of contract call  
  "txhex" : "data",             (string) The premake tranction without signature, this item   
  "coins": "data",              (array) The coins include by transaction,  
}  
  
## premaketransaction fromaddress toaddress changeaddress amount  ##  
  
 SDK interface, make a transaction no sign   
  
Arguments:  
1. "fromaddress"                  (string, required) The address for input coins  
2. "toaddress"                    (string, required) Send to address  
3. "changeaddress"                (string, required) The address for change coins  
4. "amount"                       (numeric or string, required) The amount in MGC to send. eg 0.1  
5. "fee"                          (numeric or string, optional) The amount in MGC to for fee eg 0.0001, default 0 and will calc fee by system  
  
Returns transaction data   
  
Result:  
{  
  "txhex" : xxx,            (string) The transaction hex data  
  "coins" :  
      [  
        {  
           "txhash" : xxx   (string) txid  
           "outn" : xxx     (string) vout index  
           "value" : xxx    (string) vout value  
           "script" : xxx   (string) vout lock script in hex format  
        },...  
      ]  
}  
  
Examples:  
> magnachain-cli premaketransaction XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPX XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPi XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj 1 0.0005  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "premaketransaction", "params": [XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPX XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPi XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj 1 ] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## prepublishcode fromaddress toaddress changeaddress amount  ##  
  
 rebroadcast the branch chain transaction by txid, in case that transction has not be send to the target chain .  
  
Arguments:  
1. "code"                         (string, required) The compiled code in hex format  
2. "fundaddress"                  (string, required) The magnachain address owns coins to use  
3. "senderaddress"                (string, required) The hex pubkey of address as constact caller address  
4. "amount"                       (numeric or string, required) The amount in MGC to send to constact. eg 0.1  
5. "changeaddress"                (string, required) The address for change may be  
  
Returns an obj  
  
Result:  
{  
  "txhex" : xxx,            (string) The transaction hex data  
}  
  
Examples:  
> magnachain-cli prepublishcode code XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPi XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj 1 0.0005  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "prepublishcode", "params": [code XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPi XWzFXFXehphGkSHHebNo3NwR3wMBfTeiPj 1 ] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## publishcontract "contractfilepath"  ##  
  
publish a contract from a file.  
  
Arguments:  
1. "filename"            (string, required) The file need to publish.  
  
Examples:  
> magnachain-cli publishcontract "filepath"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "publishcontract", "params": ["filepath"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## publishcontractcode "codedata"  ##  
  
publish a contract from code data.  
  
Arguments:  
1. "codedata"            (string, required) Code data to hex.  
  
Examples:  
> magnachain-cli publishcontractcode "code_in_hex_string"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "publishcontractcode", "params": ["code_in_hex_string"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## removeprunedfunds "txid" ##  
  
Deletes the specified transaction from the wallet. Meant for use with pruned wallets and as a companion to importprunedfunds. This will affect wallet balances.  
  
Arguments:  
1. "txid"           (string, required) The hex-encoded id of the transaction you are deleting  
  
Examples:  
> magnachain-cli removeprunedfunds "a8d0c0184dde994a09ec054286f1ce581bebf46446a512166eae7628734ea0a5"  
  
As a JSON-RPC call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "removeprunedfunds", "params": ["a8d0c0184dde994a09ec054286f1ce581bebf46446a512166eae7628734ea0a5"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## sendfrom "fromaccount" "toaddress" amount ( minconf "comment" "comment_to" ) ##  
  
DEPRECATED (use sendtoaddress). Sent an amount from an account to a magnachain address.  
  
Arguments:  
1. "fromaccount"       (string, required) The name of the account to send funds from. May be the default account using "".  
                       Specifying an account does not influence coin selection, but it does associate the newly created  
                       transaction with the account, so the account's balance computation and transaction history can reflect  
                       the spend.  
2. "toaddress"         (string, required) The magnachain address to send funds to.  
3. amount                (numeric or string, required) The amount in MGC (transaction fee is added on top).  
4. minconf               (numeric, optional, default=1) Only use funds with at least this many confirmations.  
5. "comment"           (string, optional) A comment used to store what the transaction is for.   
                                     This is not part of the transaction, just kept in your wallet.  
6. "comment_to"        (string, optional) An optional comment to store the name of the person or organization   
                                     to which you're sending the transaction. This is not part of the transaction,   
                                     it is just kept in your wallet.  
  
Result:  
"txid"                 (string) The transaction id.  
  
Examples:  
  
Send 0.01 MGC from the default account to the address, must have at least 1 confirmation  
> magnachain-cli sendfrom "" "1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd" 0.01  
  
Send 0.01 from the tabby account to the given address, funds must have at least 6 confirmations  
> magnachain-cli sendfrom "tabby" "1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd" 0.01 6 "donation" "seans outpost"  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "sendfrom", "params": ["tabby", "1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd", 0.01, 6, "donation", "seans outpost"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## sendmany "fromaccount" {"address":amount,...} ( minconf "comment" ["address",...] replaceable conf_target "estimate_mode") ##  
  
Send multiple times. Amounts are double-precision floating point numbers.  
  
Arguments:  
1. "fromaccount"         (string, required) DEPRECATED. The account to send the funds from. Should be "" for the default account  
2. "amounts"             (string, required) A json object with addresses and amounts  
    {  
      "address":amount   (numeric or string) The magnachain address is the key, the numeric amount (can be string) in MGC is the value  
      ,...  
    }  
3. minconf                 (numeric, optional, default=1) Only use the balance confirmed at least this many times.  
4. "comment"             (string, optional) A comment  
5. subtractfeefrom         (array, optional) A json array with addresses.  
                           The fee will be equally deducted from the amount of each selected address.  
                           Those recipients will receive less cells than you enter in their corresponding amount field.  
                           If no addresses are specified here, the sender pays the fee.  
    [  
      "address"          (string) Subtract fee from this address  
      ,...  
    ]  
6. replaceable            (boolean, optional) Allow this transaction to be replaced by a transaction with higher fees via BIP 125  
7. conf_target            (numeric, optional) Confirmation target (in blocks)  
8. "estimate_mode"      (string, optional, default=UNSET) The fee estimate mode, must be one of:  
       "UNSET"  
       "ECONOMICAL"  
       "CONSERVATIVE"  
  
Result:  
"txid"                   (string) The transaction id for the send. Only 1 transaction is created regardless of   
                                    the number of addresses.  
  
Examples:  
  
Send two amounts to two different addresses:  
> magnachain-cli sendmany "" "{\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\":0.01,\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\":0.02}"  
  
Send two amounts to two different addresses setting the confirmation and comment:  
> magnachain-cli sendmany "" "{\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\":0.01,\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\":0.02}" 6 "testing"  
  
Send two amounts to two different addresses, subtract fee from amount:  
> magnachain-cli sendmany "" "{\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\":0.01,\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\":0.02}" 1 "" "[\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\",\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\"]"  
  
As a json rpc call  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "sendmany", "params": ["", "{\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\":0.01,\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\":0.02}", 6, "testing"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## sendtoaddress "address" amount ( "comment" "comment_to" subtractfeefromamount replaceable conf_target "estimate_mode") ##  
  
Send an amount to a given address.  
  
Arguments:  
1. "address"            (string, required) The magnachain address to send to.  
2. "amount"             (numeric or string, required) The amount in MGC to send. eg 0.1  
3. "comment"            (string, optional) A comment used to store what the transaction is for.   
                             This is not part of the transaction, just kept in your wallet.  
4. "comment_to"         (string, optional) A comment to store the name of the person or organization   
                             to which you're sending the transaction. This is not part of the   
                             transaction, just kept in your wallet.  
5. subtractfeefromamount  (boolean, optional, default=false) The fee will be deducted from the amount being sent.  
                             The recipient will receive less cells than you enter in the amount field.  
6. replaceable            (boolean, optional) Allow this transaction to be replaced by a transaction with higher fees via BIP 125  
7. conf_target            (numeric, optional) Confirmation target (in blocks)  
8. "estimate_mode"      (string, optional, default=UNSET) The fee estimate mode, must be one of:  
       "UNSET"  
       "ECONOMICAL"  
       "CONSERVATIVE"  
  
Result:  
"txid"                  (string) The transaction id.  
  
Examples:  
> magnachain-cli sendtoaddress "1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd" 0.1  
> magnachain-cli sendtoaddress "1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd" 0.1 "donation" "seans outpost"  
> magnachain-cli sendtoaddress "1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd" 0.1 "" "" true  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "sendtoaddress", "params": ["1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd", 0.1, "donation", "seans outpost"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## setaccount "address" "account" ##  
  
DEPRECATED. Sets the account associated with the given address.  
  
Arguments:  
1. "address"         (string, required) The magnachain address to be associated with an account.  
2. "account"         (string, required) The account to assign the address to.  
  
Examples:  
> magnachain-cli setaccount "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX" "tabby"  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "setaccount", "params": ["1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX", "tabby"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## settxfee amount ##  
  
Set the transaction fee per kB. Overwrites the paytxfee parameter.  
  
Arguments:  
1. amount         (numeric or string, required) The transaction fee in MGC/kB  
  
Result  
true|false        (boolean) Returns true if successful  
  
Examples:  
> magnachain-cli settxfee 0.00001  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "settxfee", "params": [0.00001] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
## signmessage "address" "message" ##  
  
Sign a message with the private key of an address  
  
Arguments:  
1. "address"         (string, required) The magnachain address to use for the private key.  
2. "message"         (string, required) The message to create a signature of.  
  
Result:  
"signature"          (string) The signature of the message encoded in base 64  
  
Examples:  
  
Unlock the wallet for 30 seconds  
> magnachain-cli walletpassphrase "mypassphrase" 30  
  
Create the signature  
> magnachain-cli signmessage "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX" "my message"  
  
Verify the signature  
> magnachain-cli verifymessage "1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX" "signature" "my message"  
  
As json rpc  
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "signmessage", "params": ["1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX", "my message"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/  
  
