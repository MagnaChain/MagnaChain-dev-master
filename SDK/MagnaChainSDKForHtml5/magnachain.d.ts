
declare module magnachain
{
    class Buffer
    {

    }

    class Networks
    {
        name: string;
        alias: string;
        pubkeyhash: number;
        privatekey: number;
        scripthash: number;
        xpubkey: number;
        xprivkey: number;
        networkMagic: number;
        port: number;
        dnsseeds: Array<string>;

        static livenet: Networks;
        static testnet: Networks;
    }

    class PublicKey
    {
        // data 可以是 Point, hex DER string, DER buffer, PrivateKey
        // extra - additional options
        // extra.network - Which network should the address for this public key be for
        // extra.compressed - If the public key is compressed
        constructor(data?: any, extra?: any);

        toAddress(network?: Networks): Address;

        // DER hex encoded string
        toString(): string;
    }

    class Address
    {
        static PayToPublicKeyHash: string;
        static PayToContractHash : string;// system internal use
        static PayToScriptHash: string;

        // data 可以是 Buffer 或者 Uint8Array，或者 PublicKey, Script 对象，或地址 string
        // network 可以预定义 Networks，或者名字，默认 livenet
        // type 可以是 PayToPublicKeyHash 或 PayToScriptHash

        // 当data 为地址数组时，这时表示create multisig P2SH address, 这时 network 表示数量
        constructor(data: any, network?: any, type?: string);

        // to Wif
        toString(): string;
    }

    class PrivateKey
    {
        // data 可以是 Buffer 或者 Uint8Array，或者 wif 串， 或 hex 串
        // network 可以预定义 Networks，或者名字，默认 livenet
        // 不传返回一个随机的 key
        constructor(data?: any, network?: any);

        static fromBuffer(arg: any, network: any): PrivateKey;
        static fromWIF(strWif: string): PrivateKey;

        // to hex string
        toString(): string;

        // to Wif string
        toWIF(): string;

        // get public key
        toPublicKey(): PublicKey;

        toAddress(network?: Networks): Address;
    }

    class HDPrivateKey
    {
        // arg 可以是 xprivkey string; 也可以是networks type，这里返回一个随机的 hd private key
        // 也可以是一个类似下面的 struct
        // version: decoded.slice(HDPrivateKey.VersionStart, HDPrivateKey.VersionEnd),
        // depth: decoded.slice(HDPrivateKey.DepthStart, HDPrivateKey.DepthEnd),
        // parentFingerPrint: decoded.slice(HDPrivateKey.ParentFingerPrintStart,
        // HDPrivateKey.ParentFingerPrintEnd),
        // childIndex: decoded.slice(HDPrivateKey.ChildIndexStart, HDPrivateKey.ChildIndexEnd),
        // chainCode: decoded.slice(HDPrivateKey.ChainCodeStart, HDPrivateKey.ChainCodeEnd),
        // privateKey: decoded.slice(HDPrivateKey.PrivateKeyStart, HDPrivateKey.PrivateKeyEnd),
        // checksum: decoded.slice(HDPrivateKey.ChecksumStart, HDPrivateKey.ChecksumEnd),
        // xprivkey: arg
        constructor(arg?: any);

        static fromMnemonicWord(strWords: string, network?: any): HDPrivateKey;

        // hexa: hex string or Buffer
        // network: network name or network object
        static fromSeed(hexa: any, network?: any): HDPrivateKey

        // Returns the string representation of this private key (a string starting * with "xprv...")
        toString(): string;

        /* var parent = new HDPrivateKey('xprv...');
        * var child_0_1_2h = parent.deriveChild(0).deriveChild(1).deriveChild(2, true);
        * var copy_of_child_0_1_2h = parent.deriveChild("m/0/1/2'");
        * assert(child_0_1_2h.xprivkey === copy_of_child_0_1_2h);

        * @param {string|number} arg
        * @param {boolean?} hardened
        */
        deriveChild(arg: any, hardened?: boolean): HDPrivateKey

        network: Networks;

        privateKey: PrivateKey;

        depth: number;

        fingerPrint: Buffer;

        hdPublicKey: HDPublicKey;
    }

    class HDPublicKey
    {
        // arg 可以是 xpub... 的 string, Buffer, HDPrivateKey 或类似结构
        // version: decoded.slice(HDPublicKey.VersionStart, HDPublicKey.VersionEnd),
        // depth: decoded.slice(HDPublicKey.DepthStart, HDPublicKey.DepthEnd),
        // parentFingerPrint: decoded.slice(HDPublicKey.ParentFingerPrintStart,
        //    HDPublicKey.ParentFingerPrintEnd),
        // childIndex: decoded.slice(HDPublicKey.ChildIndexStart, HDPublicKey.ChildIndexEnd),
        // chainCode: decoded.slice(HDPublicKey.ChainCodeStart, HDPublicKey.ChainCodeEnd),
        // publicKey: decoded.slice(HDPublicKey.PublicKeyStart, HDPublicKey.PublicKeyEnd),
        // checksum: decoded.slice(HDPublicKey.ChecksumStart, HDPublicKey.ChecksumEnd),
        // xpubkey: arg
        constructor(arg?: any);

        network: Networks;

        depth: number;

        publicKey: PublicKey;

        fingerPrint: Buffer;

        /* var parent = new HDPublicKey('xpub...');
        * var child_0_1_2 = parent.deriveChild(0).deriveChild(1).deriveChild(2);
        * var copy_of_child_0_1_2 = parent.deriveChild("m/0/1/2");
        * assert(child_0_1_2.xprivkey === copy_of_child_0_1_2);
        *
        * @param {string|number} arg
        * */
        deriveChild(arg: any, hardened?: boolean): HDPublicKey;

        // xpub....
        toString(): string;
    }

    class Script
    {
        // {Object|string|Buffer=} from optional data to populate script
        constructor(from: any);

        // buffer: Uint8
        static fromBuffer(buffer: Buffer): Script;

        // strASM: " " 隔开的字符串
        static fromASM(strASM: string): Script;

        // strHex: hex string
        static fromHex(strHex: string): Script;

        toBuffer(): Buffer;

        // return: " " 隔开的字符串
        toASM(): string;

        toHex(): string;

        isPublicKeyHashOut(): boolean;

        isPublicKeyHashIn(): boolean;

        getPublicKey(): Buffer;

        getPublicKeyHash(): Buffer;

        isPublicKeyOut(): boolean;

        isScriptHashOut(): boolean;

        isScriptHashIn(): boolean;

        isMultisigOut(): boolean;

        isMultisigIn(): boolean;

        isDataOut(): boolean;

        /**
        * Retrieve the associated data for this script.
        * In the case of a pay to public key hash or P2SH, return the hash.
        * In the case of a standard OP_RETURN, return the data
        * @returns {Buffer}
        */
        getData(): Buffer;

        isPushOnly(): boolean;

        isStandard(): boolean;

        /**
        * Adds a script element at the start of the script.
        * @param {*} obj a string, number, Opcode, Buffer, or object to add
        * @returns {Script} this script instance
        */
        prepend(obj: any): Script;

        /**
        * Compares a script with another script
        */
        equals(script: Script): boolean;


        /**
        * Adds a script element to the end of the script.
        *
        * @param {*} obj a string, number, Opcode, Buffer, or object to add
        * @returns {Script} this script instance
        *
        */
        add(obj: any): Script;

        // reture this
        removeCodeseparators(): Script;

        /**
        * @returns {Script} a new Multisig output script for given public keys,
        * requiring m of those public keys to spend
        * @param {PublicKey[]} publicKeys - list of all public keys controlling the output
        * @param {number} threshold - amount of required signatures to spend the output
        * @param {Object=} opts - Several options:
        *        - noSorting: defaults to false, if true, don't sort the given
        *                      public keys before creating the script
        */
        static buildMultisigOut(publicKeys: Array<PublicKey>, threshold: number, opts: any): Script;

        /**
        * A new Multisig input script for the given public keys, requiring m of those public keys to spend
        * @param {PublicKey[]} pubkeys list of all public keys controlling the output
        * @param {number} threshold amount of required signatures to spend the output
        * @param {Array} signatures and array of signature buffers to append to the script
        * @param {Object=} opts
        * @param {boolean=} opts.noSorting don't sort the given public keys before creating the script (false by default)
        * @param {Script=} opts.cachedMultisig don't recalculate the redeemScript
        * @returns {Script}
        */
        static buildMultisigIn(pubkeys: Array<PublicKey>, threshold: number, signatures: Array<Buffer>, opts: any): Script;

        /**
        * A new P2SH Multisig input script for the given public keys, requiring m of those public keys to spend
        * @param {PublicKey[]} pubkeys list of all public keys controlling the output
        * @param {number} threshold amount of required signatures to spend the output
        * @param {Array} signatures and array of signature buffers to append to the script
        * @param {Object=} opts
        * @param {boolean=} opts.noSorting don't sort the given public keys before creating the script (false by default)
        * @param {Script=} opts.cachedMultisig don't recalculate the redeemScript
        * @returns {Script}
        */
        static buildP2SHMultisigIn(pubkeys: Array<PublicKey>, threshold: number, signatures: Array<Buffer>, opts: any): Script;

        /**
        * @returns {Script} a new pay to public key hash output for the given
        * address or public key
        * @param {(Address|PublicKey)} to - destination address or public key
        */
        static buildPublicKeyHashOut(to: any): Script;

        /**
        * @returns {Script} a new pay to public key output for the given
        *  public key
        */
        static buildPublicKeyOut(pubkey: PublicKey): Script;

        /**
        * @returns {Script} a new OP_RETURN script with data
        * @param {(string|Buffer)} data - the data to embed in the output
        * @param {(string)} encoding - the type of encoding of the string
        */
        static buildDataOut(data: any, encoding: string): Script;

        /**
        * @param {Script|Address} script - the redeemScript for the new p2sh output.
        *    It can also be a p2sh address
        * @returns {Script} new pay to script hash script for given script
        */
        static buildScriptHashOut(script: any): Script;

        /**
        * Builds a scriptSig (a script for an input) that signs a public key output script.
        *
        * @param {Signature|Buffer} signature - a Signature object, or the signature in DER canonical encoding
        * @param {number=} sigtype - the type of the signature (defaults to SIGHASH_ALL)
        */
        static buildPublicKeyIn(signature: any, sigtype: number): Script;

        /**
        * Builds a scriptSig (a script for an input) that signs a public key hash
        * output script.
        * @param {Buffer|string|PublicKey} publicKey
        * @param {Signature|Buffer} signature - a Signature object, or the signature in DER canonical encoding
        * @param {number=} sigtype - the type of the signature (defaults to SIGHASH_ALL)
        */
        static buildPublicKeyHashIn(publicKey: any, signature: any, sigtype: number): Script;

        toScriptHashOut(): Script;

        /**
        * @return {Script} an output script built from the address
        */
        static fromAddress(address: any): Script;

        /**
        * @param {Network=} network
        * @return {Address|boolean} the associated address for this script if possible, or false
        */
        toAddress(network: any): Address;

        /**
        * Analogous to bitcoind's FindAndDelete. Find and delete equivalent chunks,
        * typically used with push data chunks.  Note that this will find and delete
        * not just the same data, but the same data with the same push data op as
        * produced by default. i.e., if a pushdata in a tx does not use the minimal
        * pushdata op, then when you try to remove the data it is pushing, it will not
        * be removed, because they do not use the same pushdata op.
        * return this
        */
        findAndDelete(script: Script): Script;

        /**
        * Comes from bitcoind's script interpreter CheckMinimalPush function
        * @returns {boolean} if the chunk {i} is the smallest way to push that particular data.
        */
        checkMinimalPush(i: number): boolean;

        /**
        * Comes from bitcoind's script GetSigOpCount(boolean) function
        * @param {boolean} use current (true) or pre-version-0.6 (false) logic
        * @returns {number} number of signature operations required by this script
        */
        getSignatureOperationsCount(accurate: boolean): number
    }

    class Signature
    {
        constructor(r: any, s: any);

        static fromCompact(buf: Buffer): Signature;

        static parseDER(buf: Buffer, strict: boolean): any;

        static fromBuffer(buf: Buffer, strict: boolean): Signature;

        // str: hex string
        static fromString(str: string): Signature;

        // i: 0, 1, 2, 3
        toCompact(i: number, compressed: boolean): Buffer

        toBuffer(): Buffer;

        // return hex string
        toString(): string;

        static isTxDER(buf: Buffer): boolean;

        hasLowS(): boolean;

        hasDefinedHashtype(): boolean;

        toTxFormat(): Buffer;
    }

    class Output
    {
        /*
        args 为一对象，有以下成员：
        satoshis: number
        script: Buffer or hex string or Script
        */
        constructor(args: any);

        _script: Script;

        satoshis: number;

        // return error string or false
        invalidSatoshis(): any

        // return object(JSON)
        toObject(): any;

        // return this
        setScript(script: Script): Output;
    }

    class Input
    {
        /*
        params 为一对象，有以下成员：
        prevTxId: Buffer or hex string
        output: Output or create output object param
        txidbuf: Buffer
        outputIndex: number
        txoutnum: number
        seqnum： number(sequenceNumber 二选一)
        sequenceNumber: number
        script: Script(scriptBuffer 二选一)
        scriptBuffer: Buffer
        */
        constructor(params: any);

        /*
        prevTxId: this.prevTxId.toString('hex'),
        outputIndex: this.outputIndex,
        sequenceNumber: this.sequenceNumber,
        script: this._scriptBuffer.toString('hex'),
        maybe has:
        obj.scriptString = this.script.toString();
        obj.output = this.output.toObject();
        */
        toObject(): any;

        _script: Script;

        // return this
        setScript(script: Script): Input;

        // return this
        setOutput(output: Output): Input;

        isFinal(): boolean;

        isValidSignature(transaction: Transaction, signature: Signature): boolean;

        isNull(): boolean;
    }

// ContractData
//   address; // contract address
//   sender;  // sender address
//   codeOrFunc; // code or function name
//   args; // call contraction args
//   amountOut; //
//   signature;  // contract data signature

    class Transaction
    {
        // serialized: hex string or Buffer or Object(JSON))
        constructor(serialized?: any);

        inputAmount: number;

        outputAmount: number;

        inputs: Array<Input>;

        outputs: Array<Output>;

        contractdata: any;// ContractData

        // return hex string
        serialize(unsafe: boolean): string;

        // return unchecked hex string
        toString(): string;

        // return Buffer
        toBuffer(): Buffer;

        // return json object
        toObject(): any;

        invalidSatoshis(): boolean;

        //  {Date | Number} time
        // return this
        lockUntilDate(time: any): Transaction;

        // return this
        lockUntilBlockHeight(height: number): Transaction;

        /*  If nLockTime is 0, it returns null,
        *  if it is < 500000000, it returns a block height (number)
        *  else it returns a Date object.
        */
        getLockTime(): any;

        // return this
        fromBuffer(buffer: Buffer): Transaction;

        // return this
        fromString(strHex: string): Transaction;

        // return this
        fromObject(arg: any): Transaction;

        /* utxo: {
                    "txId" : "115e8f72f39fad874cfab0deed11a80f24f967a84079fb56ddf53ea02e308986",
                    "outputIndex" : 0,
                    "address" : "17XBj6iFEsf8kzDMGQk5ghZipxX49VXuaV",
                    "script" : "76a91447862fe165e6121af80d5dde1ecb478ed170565b88ac",
                    "satoshis" : 50000
                }*/
        // pubkeys: Array
        // return this
        from(utxo: any, pubkeys?: Array<PublicKey>, threshold?: number): Transaction;

        /**
        * Add an input to this transaction. The input must be an instance of the `Input` class.
        * It should have information about the Output that it's spending, but if it's not already
        * set, two additional parameters, `outputScript` and `satoshis` can be provided. 
        * @param {Input} input
        * @param {String|Script} outputScript
        * @param {number} satoshis
        * @return Transaction this, for chaining
        */
        addInput(input: Input, outputScript: any, satoshis: number): Transaction;

        /**
        * Add an input to this transaction, without checking that the input has information about
        * the output that it's spending. 
        * @param {Input} input
        * @return Transaction this, for chaining
        */
        uncheckedAddInput(input: Input): Transaction;

        hasAllUtxoInfo(): boolean;

        /**
        * Manually set the fee for this transaction. Beware that this resets all the signatures
        * for inputs (in further versions, SIGHASH_SINGLE or SIGHASH_NONE signatures will not
        * be reset). 
        * @param {number} amount satoshis to be sent
        * @return {Transaction} this, for chaining
        */
        fee(amount: number): Transaction;

        /**
        * Manually set the fee per KB for this transaction. Beware that this resets all the signatures
        * for inputs (in further versions, SIGHASH_SINGLE or SIGHASH_NONE signatures will not
        * be reset). 
        * @param {number} amount satoshis per KB to be sent
        * @return {Transaction} this, for chaining
        */
        feePerKb(amount: number): Transaction;

        /**
        * Set the change address for this transaction 
        * Beware that this resets all the signatures for inputs (in further versions,
        * SIGHASH_SINGLE or SIGHASH_NONE signatures will not be reset).
        * @param {Address} address An address for change to be sent to.
        * @return {Transaction} this, for chaining
        */
        change(address: any): Transaction;

        getChangeOutput(): Output;

        /**
        * Add an output to the transaction. 
        * Beware that this resets all the signatures for inputs (in further versions,
        * SIGHASH_SINGLE or SIGHASH_NONE signatures will not be reset).
        *
        * @param {(string|Address|Array.<Transaction~toObject>)} address
        * @param {number} amount in satoshis
        * @return {Transaction} this, for chaining
        */
        to(address: any, amount: number): Transaction;

        /**
        * Add an OP_RETURN output to the transaction. 
        * Beware that this resets all the signatures for inputs (in further versions,
        * SIGHASH_SINGLE or SIGHASH_NONE signatures will not be reset). 
        * @param {Buffer|string} value the data to be stored in the OP_RETURN output.
        *    In case of a string, the UTF-8 representation will be stored
        * @return {Transaction} this, for chaining
        */
        addData(value: any): Transaction


        /**
        * Add an output to the transaction.
        *
        * @param {Output} output the output to add.
        * @return {Transaction} this, for chaining
        */
        addOutput(output: Output): Transaction;

        // return this
        clearOutputs(): Transaction;

        getFee(): number;

        removeOutput(index: number): void;

        // Sort a transaction's inputs and outputs according to BIP69
        // return this
        sort(): Transaction;

        /**
        * Randomize this transaction's outputs ordering. The shuffling algorithm is a
        * version of the Fisher-Yates shuffle, provided by lodash's _.shuffle().
        *
        * @return {Transaction} this
        */
        shuffleOutputs(): Transaction;

        removeInput(txId: any, outputIndex: number): void;

        /**
        * Sign the transaction using one or more private keys.
        * It tries to sign each input, verifying that the signature will be valid
        * (matches a public key).
        *
        * @param {Array|String|PrivateKey} privateKey
        * @param {number} sigtype
        * @return {Transaction} this, for chaining
        */
        sign(privateKey: any, sigtype?: number): Transaction;

        getSignatures(privKey: any, sigtype: number): Array<Signature>;

        /**
        * Add a signature to the transaction 
        * @param {Object} signature
        * @param {number} signature.inputIndex
        * @param {number} signature.sigtype
        * @param {PublicKey} signature.publicKey
        * @param {Signature} signature.signature
        * @return {Transaction} this, for chaining
        */
        applySignature(signature: any): Transaction;

        isFullySigned(): boolean;

        isValidSignature(signature: any): boolean;

        /*
        * @param {Signature} signature
        * @param {PublicKey} publicKey
        * @param {number} inputIndex
        * @param {Script} subscript
        * @return {boolean}*/
        verifySignature(sig, pubkey, nin, subscript): boolean;

        /**
        * Check that a transaction passes basic sanity tests. If not, return a string
        * describing the error. This function contains the same logic as
        * CheckTransaction in bitcoin core.
        * return true or error string
        */
        verify(): any;

        isCoinbase(): boolean;

        /**
        * Determines if this transaction can be replaced in the mempool with another
        * transaction that provides a sufficiently higher fee (RBF).
        */
        isRBF(): boolean;

        /**
        * Enable this transaction to be replaced in the mempool (RBF) if a transaction
        * includes a sufficiently higher fee. It will set the sequenceNumber to
        * DEFAULT_RBF_SEQNUMBER for all inputs if the sequence number does not
        * already enable RBF.
        *
        * return this
        */
        enableRBF(): Transaction;

        // return this
        setOutputsFromCoins(coins: Array<any>): Transaction;
    }

    class UNITS
    {
        static BTC : string;
        static mBTC : string;
        static uBTC : string;
        static bits : string;
        static satoshis : string;
    }

    /**
     * Utility for handling and converting bitcoins units. The supported units are
     * BTC, mBTC, bits (also named uBTC) and satoshis. A unit instance can be created with an
     * amount and a unit code, or alternatively using static methods like {fromBTC}.
     * It also allows to be created from a fiat amount and the exchange rate, or
     * alternatively using the {fromFiat} static method.
     * You can consult for different representation of a unit instance using it's
     * {to} method, the fixed unit methods like {toSatoshis} or alternatively using
     * the unit accessors. It also can be converted to a fiat amount by providing the
     * corresponding BTC/fiat exchange rate.
     *
     * @example
     * ```javascript
     * var sats = Unit.fromBTC(1.3).toSatoshis();
     * var mili = Unit.fromBits(1.3).to(Unit.mBTC);
     * var bits = Unit.fromFiat(1.3, 350).bits;
     * var btc = new Unit(1.3, Unit.bits).BTC;
     */
    class Unit
    {
        /* @param {Number} amount - The amount to be represented
        * @param {String|Number} code - The unit of the amount or the exchange rate
        * @returns {Unit} A new instance of an Unit
        * @constructor
        * */
        constructor(amount : Number, code : any);

        /**
        * Returns a Unit instance created from JSON string or object
        *
        * @param {String|Object} json - JSON with keys: amount and code
        * @returns {Unit} A Unit instance
        */
        static fromObject(data : any) : Unit;

        /**
        * Returns a Unit instance created from an amount in BTC
        *
        * @param {Number} amount - The amount in BTC
        * @returns {Unit} A Unit instance
        */
        static fromBTC(amount : Number) : Unit;

        /**
        * Returns a Unit instance created from an amount in mBTC
        *
        * @param {Number} amount - The amount in mBTC
        * @returns {Unit} A Unit instance
        */
        static fromMillis(amount : Number) : Unit;

        /**
        * Returns a Unit instance created from an amount in satoshis
        *
        * @param {Number} amount - The amount in satoshis
        * @returns {Unit} A Unit instance        
        */ 
        static fromSatoshis(amount : Number) : Unit;

        /**
        * Returns a Unit instance created from a fiat amount and exchange rate.
        *
        * @param {Number} amount - The amount in fiat
        * @param {Number} rate - The exchange rate BTC/fiat
        * @returns {Unit} A Unit instance
        */
        static fromFiat(amount : Number, rate : Number) : Unit;

        /**
        * Returns the value represented in the specified unit
        *
        * @param {String|Number} code - The unit code or exchange rate
        * @returns {Number} The converted value
        */
        to(code : any) : Number;

        /**
        * Returns the value represented in BTC
        *
        * @returns {Number} The value converted to BTC
        */
        toBTC() : Number;

        /**
        * Returns the value represented in mBTC
        *
        * @returns {Number} The value converted to mBTC
        */
        toMillis() : Number;

        /**
        * Returns the value represented in bits
        *
        * @returns {Number} The value converted to bits
        */
        toMicros() : Number;

        /**
        * Returns the value represented in satoshis
        *
        * @returns {Number} The value converted to satoshis
        */
        toSatoshis() : Number;
    }

    class RpcClient
    {
        // opts: map struct
        // host {string}, rpcport {int}, rpcuser {string} rpcpassword{string} 
        constructor(opts : any);

        // arguments: callback, method, args...
        // fnFinish(status : int, errors : string, jsonRet : ?)
        sendCommand(fnFinish : Function, strMethod : string, ...args : any[]) : void
    }

    function initializeRpc(strHost : string, iPort : Number, strRpcUser : string, strRpcPassword : string) : void;

    // arguments: callback, method, args...
    // fnFinish(status : int, errors : string, jsonRet : ?)
    function sendRpcCommand(fnFinish : Function, strMethod : string, ...args : any[]) : void

    class MiscFunc
    {
        // 0. fnCallback                     like OnFinish(bSucc)
        // 1. "strFromPriKey"                (string, required) The wif private key for input coins
        // 2. "strToAddress"                 (string, required) Send to address
        // 3. "fAmount"                      (numeric or string, required) The amount in MGC to send. eg 0.1
        // 4. "strChargeAddress"             (string, optional) The address for change coins, empty will use from address
        // 5. "fFee"                         (numeric or string, optional) The amount in MGC to for fee eg 0.0001, default 0 and will calc fee by system
        static transferByRpc(fnCallback : Function, strFromPriKey : string, strToAddress : string, fAmount : Number, strChargeAddress ?: string, fFee ?: Number);
    }
}

