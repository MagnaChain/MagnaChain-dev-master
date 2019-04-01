'use strict';

var magnachain = {};

// function assert(val, msg)
// {
//     if (!val) throw new Error(msg || 'Assertion failed');
// }

// errors begin----------------------------------------------------------------------
var docsURL = 'http://matgz.com/';

var errors = {};

var _xerrorData = [{
    name: 'InvalidB58Char',
    message: 'Invalid Base58 character: {0} in {1}'
}, {
    name: 'InvalidB58Checksum',
    message: 'Invalid Base58 checksum for {0}'
}, {
    name: 'InvalidNetwork',
    message: 'Invalid version for network: got {0}'
}, {
    name: 'InvalidState',
    message: 'Invalid state: {0}'
}, {
    name: 'NotImplemented',
    message: 'Function {0} was not implemented yet'
}, {
    name: 'InvalidNetworkArgument',
    message: 'Invalid network: must be "livenet" or "testnet", got {0}'
}, {
    name: 'InvalidArgument',
    message: function ()
    {
        return 'Invalid Argument' + (arguments[0] ? (': ' + arguments[0]) : '') +
            (arguments[1] ? (' Documentation: ' + docsURL + arguments[1]) : '');
    }
}, {
    name: 'AbstractMethodInvoked',
    message: 'Abstract Method Invocation: {0}'
}, {
    name: 'InvalidArgumentType',
    message: function ()
    {
        return 'Invalid Argument for ' + arguments[2] + ', expected ' + arguments[1] + ' but got ' + typeof arguments[0];
    }
}, {
    name: 'Unit',
    message: 'Internal Error on Unit {0}',
    errors: [{
        'name': 'UnknownCode',
        'message': 'Unrecognized unit code: {0}'
    }, {
        'name': 'InvalidRate',
        'message': 'Invalid exchange rate: {0}'
    }]
}, {
    name: 'MerkleBlock',
    message: 'Internal Error on MerkleBlock {0}',
    errors: [{
        'name': 'InvalidMerkleTree',
        'message': 'This MerkleBlock contain an invalid Merkle Tree'
    }]
}, {
    name: 'Transaction',
    message: 'Internal Error on Transaction {0}',
    errors: [{
        name: 'Input',
        message: 'Internal Error on Input {0}',
        errors: [{
            name: 'MissingScript',
            message: 'Need a script to create an input'
        }, {
            name: 'UnsupportedScript',
            message: 'Unsupported input script type: {0}'
        }, {
            name: 'MissingPreviousOutput',
            message: 'No previous output information.'
        }]
    }, {
        name: 'NeedMoreInfo',
        message: '{0}'
    }, {
        name: 'InvalidSorting',
        message: 'The sorting function provided did not return the change output as one of the array elements'
    }, {
        name: 'InvalidOutputAmountSum',
        message: '{0}'
    }, {
        name: 'MissingSignatures',
        message: 'Some inputs have not been fully signed'
    }, {
        name: 'InvalidIndex',
        message: 'Invalid index: {0} is not between 0, {1}'
    }, {
        name: 'UnableToVerifySignature',
        message: 'Unable to verify signature: {0}'
    }, {
        name: 'DustOutputs',
        message: 'Dust amount detected in one output'
    }, {
        name: 'InvalidSatoshis',
        message: 'Output satoshis are invalid',
    }, {
        name: 'FeeError',
        message: 'Internal Error on Fee {0}',
        errors: [{
            name: 'TooSmall',
            message: 'Fee is too small: {0}',
        }, {
            name: 'TooLarge',
            message: 'Fee is too large: {0}',
        }, {
            name: 'Different',
            message: 'Unspent value is different from specified fee: {0}',
        }]
    }, {
        name: 'ChangeAddressMissing',
        message: 'Change address is missing'
    }, {
        name: 'BlockHeightTooHigh',
        message: 'Block Height can be at most 2^32 -1'
    }, {
        name: 'NLockTimeOutOfRange',
        message: 'Block Height can only be between 0 and 499 999 999'
    }, {
        name: 'LockTimeTooEarly',
        message: 'Lock Time can\'t be earlier than UNIX date 500 000 000'
    }]
}, {
    name: 'Script',
    message: 'Internal Error on Script {0}',
    errors: [{
        name: 'UnrecognizedAddress',
        message: 'Expected argument {0} to be an address'
    }, {
        name: 'CantDeriveAddress',
        message: 'Can\'t derive address associated with script {0}, needs to be p2pkh in, p2pkh out, p2sh in, or p2sh out.'
    }, {
        name: 'InvalidBuffer',
        message: 'Invalid script buffer: can\'t parse valid script from given buffer {0}'
    }]
}, {
    name: 'HDPrivateKey',
    message: 'Internal Error on HDPrivateKey {0}',
    errors: [{
        name: 'InvalidDerivationArgument',
        message: 'Invalid derivation argument {0}, expected string, or number and boolean'
    }, {
        name: 'InvalidEntropyArgument',
        message: 'Invalid entropy: must be an hexa string or binary buffer, got {0}',
        errors: [{
            name: 'TooMuchEntropy',
            message: 'Invalid entropy: more than 512 bits is non standard, got "{0}"'
        }, {
            name: 'NotEnoughEntropy',
            message: 'Invalid entropy: at least 128 bits needed, got "{0}"'
        }]
    }, {
        name: 'InvalidLength',
        message: 'Invalid length for xprivkey string in {0}'
    }, {
        name: 'InvalidPath',
        message: 'Invalid derivation path: {0}'
    }, {
        name: 'UnrecognizedArgument',
        message: 'Invalid argument: creating a HDPrivateKey requires a string, buffer, json or object, got "{0}"'
    }]
}, {
    name: 'HDPublicKey',
    message: 'Internal Error on HDPublicKey {0}',
    errors: [{
        name: 'ArgumentIsPrivateExtended',
        message: 'Argument is an extended private key: {0}'
    }, {
        name: 'InvalidDerivationArgument',
        message: 'Invalid derivation argument: got {0}'
    }, {
        name: 'InvalidLength',
        message: 'Invalid length for xpubkey: got "{0}"'
    }, {
        name: 'InvalidPath',
        message: 'Invalid derivation path, it should look like: "m/1/100", got "{0}"'
    }, {
        name: 'InvalidIndexCantDeriveHardened',
        message: 'Invalid argument: creating a hardened path requires an HDPrivateKey'
    }, {
        name: 'MustSupplyArgument',
        message: 'Must supply an argument to create a HDPublicKey'
    }, {
        name: 'UnrecognizedArgument',
        message: 'Invalid argument for creation, must be string, json, buffer, or object'
    }]
}];

function format(message, args)
{
    return message
        .replace('{0}', args[0])
        .replace('{1}', args[1])
        .replace('{2}', args[2]);
}

var traverseNode = function (parent, errorDefinition)
{
    var NodeError = function ()
    {
        if (_.isString(errorDefinition.message))
        {
            this.message = format(errorDefinition.message, arguments);
        } else if (_.isFunction(errorDefinition.message))
        {
            this.message = errorDefinition.message.apply(null, arguments);
        } else
        {
            throw new Error('Invalid error definition for ' + errorDefinition.name);
        }
        this.stack = this.message + '\n' + (new Error()).stack;
    };
    NodeError.prototype = Object.create(parent.prototype);
    NodeError.prototype.name = parent.prototype.name + errorDefinition.name;
    parent[errorDefinition.name] = NodeError;
    if (errorDefinition.errors)
    {
        childDefinitions(NodeError, errorDefinition.errors);
    }
    return NodeError;
};

/* jshint latedef: false */
var childDefinitions = function (parent, childDefinitions)
{
    _.each(childDefinitions, function (childDefinition)
    {
        traverseNode(parent, childDefinition);
    });
};
/* jshint latedef: true */

var traverseRoot = function (parent, errorsDefinition)
{
    childDefinitions(parent, errorsDefinition);
    return parent;
};

magnachain.Error = function ()
{
    this.message = 'Internal error';
    this.stack = this.message + '\n' + (new Error()).stack;
};
magnachain.Error.prototype = Object.create(Error.prototype);
magnachain.Error.prototype.name = 'bitcore.Error';

traverseRoot(magnachain.Error, _xerrorData);

// module.exports = bitcore.Error;
// module.exports.extend = function (spec)
// {
//   return traverseNode(bitcore.Error, spec);
// };
//bitcore.Error.extend = traverseNode(bitcore.Error, spec);
errors = magnachain.Error;
errors.extend = function (spec)
{
    return traverseNode(magnachain.Error, spec);
};
// errors end-------------------------------------------------------------------------------

// util/js begin----------------------------------------------------------------------------
var JSUtil = {};

var isHexa = function isHexa(value)
{
    if (!_.isString(value))
    {
        return false;
    }
    return /^[0-9a-fA-F]+$/.test(value);
};

var isValidJSON = function isValidJSON(arg)
{
    var parsed;
    if (!_.isString(arg))
    {
        return false;
    }
    try
    {
        parsed = JSON.parse(arg);
    } catch (e)
    {
        return false;
    }
    if (typeof (parsed) === 'object')
    {
        return true;
    }
    return false;
};

//bitcore.isValidJSON = isValidJSON;
JSUtil.isValidJSON = isValidJSON;

//bitcore.isHexa = isHexa;
JSUtil.isHexa = isHexa;
//bitcore.isHexaString = isHexa;
JSUtil.isHexaString = isHexa;

/**
 * Clone an array
 */
var cloneArray = function (array)
{
    return [].concat(array);
};

//bitcore.cloneArray = cloneArray;
JSUtil.cloneArray = cloneArray;

/**
 * Define immutable properties on a target object
 *
 * @param {Object} target - An object to be extended
 * @param {Object} values - An object of properties
 * @return {Object} The target object
 */
var defineImmutable = function (target, values)
{
    Object.keys(values).forEach(function (key)
    {
        Object.defineProperty(target, key, {
            configurable: false,
            enumerable: true,
            value: values[key]
        });
    });
    return target;
};

//bitcore.defineImmutable = defineImmutable;
JSUtil.defineImmutable = defineImmutable;
// /**
//  * Checks that a value is a natural number, a positive integer or zero.
//  *
//  * @param {*} value
//  * @return {Boolean}
//  */
var isNaturalNumber = function (value)
{
    return typeof value === 'number' &&
        isFinite(value) &&
        Math.floor(value) === value &&
        value >= 0;
}

//bitcore.isNaturalNumber = isNaturalNumber;
JSUtil.isNaturalNumber = isNaturalNumber;

magnachain.JSUtil = JSUtil;
// util/js end------------------------------------------------------------------------------

// util/preconditions being-----------------------------------------------------------------
var $ = {};

var checkState = function (condition, message)
{
    if (!condition)
    {
        throw new errors.InvalidState(message);
    }
}
$.checkState = checkState;
//bitcore.checkState = checkState;

var checkArgument = function (condition, argumentName, message, docsPath)
{
    if (!condition)
    {
        throw new errors.InvalidArgument(argumentName, message, docsPath);
    }
}
$.checkArgument = checkArgument;
//bitcore.checkArgument = checkArgument;

var checkArgumentType = function (argument, type, argumentName)
{
    argumentName = argumentName || '(unknown name)';
    if (_.isString(type))
    {
        if (type === 'Buffer')
        {
            //var buffer = require('buffer'); // './buffer' fails on cordova & RN
            //if (!buffer.Buffer.isBuffer(argument))
            if (!Buffer.isBuffer(argument))
            {
                throw new errors.InvalidArgumentType(argument, type, argumentName);
            }
        } else if (typeof argument !== type)
        {
            throw new errors.InvalidArgumentType(argument, type, argumentName);
        }
    } else
    {
        if (!(argument instanceof type))
        {
            throw new errors.InvalidArgumentType(argument, type.name, argumentName);
        }
    }
}
$.checkArgumentType = checkArgumentType;
//bitcore.checkArgumentType = checkArgumentType;
magnachain.$ = $;
// util/preconditions end-------------------------------------------------------------------

// util/buffer begin------------------------------------------------------------------------
var BufferUtil = {}

//var assert = require('assert');

//var js = require('./js');
//var $ = require('./preconditions');

function equals(a, b)
{
    if (a.length !== b.length)
    {
        return false;
    }
    var length = a.length;
    for (var i = 0; i < length; i++)
    {
        if (a[i] !== b[i])
        {
            return false;
        }
    }
    return true;
}
BufferUtil.equals = equals;

/**
 * Fill a buffer with a value.
 *
 * @param {Buffer} buffer
 * @param {number} value
 * @return {Buffer}
 */
var fill = function fill(buffer, value)
{
    $.checkArgumentType(buffer, 'Buffer', 'buffer');
    $.checkArgumentType(value, 'number', 'value');
    var length = buffer.length;
    for (var i = 0; i < length; i++)
    {
        buffer[i] = value;
    }
    return buffer;
}
BufferUtil.fill = fill;
//bitcore.fill = fill;

/**
 * Return a copy of a buffer
 *
 * @param {Buffer} original
 * @return {Buffer}
 */
var copy = function (original)
{
    var buffer = new Buffer(original.length);
    original.copy(buffer);
    return buffer;
}
BufferUtil.copy = copy;

/**
 * Returns true if the given argument is an instance of a buffer. Tests for
 * both node's Buffer and Uint8Array
 *
 * @param {*} arg
 * @return {boolean}
 */
var isBuffer = function isBuffer(arg)
{
    //console.log("VVVVV isBuffer");
    //console.log("QQQQ: " + arg);
    //console.log("QQQQ: " + arg.constructor.name);
    var left = Buffer.isBuffer(arg);
    var right = arg instanceof Uint8Array;
    //console.log("VVVVV LR： " + left + " " + right);
    return left || right;
}
BufferUtil.isBuffer = isBuffer;

/**
 * Returns a zero-filled byte array
 *
 * @param {number} bytes
 * @return {Buffer}
 */
var emptyBuffer = function emptyBuffer(bytes)
{
    $.checkArgumentType(bytes, 'number', 'bytes');
    var result = new Buffer(bytes);
    for (var i = 0; i < bytes; i++)
    {
        result.write('\0', i);
    }
    return result;
}
BufferUtil.emptyBuffer = emptyBuffer;

/**
 * Concatenates a buffer
 *
 * Shortcut for <tt>buffer.Buffer.concat</tt>
 */
BufferUtil.concat = Buffer.concat;

//buffer.equals = equals,
BufferUtil.equal = equals;

/**
 * Transforms a number from 0 to 255 into a Buffer of size 1 with that value
 *
 * @param {number} integer
 * @return {Buffer}
 */
var integerAsSingleByteBuffer = function (integer)
{
    $.checkArgumentType(integer, 'number', 'integer');
    return new Buffer([integer & 0xff]);
}
BufferUtil.integerAsSingleByteBuffer = integerAsSingleByteBuffer;

/**
 * Transform a 4-byte integer into a Buffer of length 4.
 *
 * @param {number} integer
 * @return {Buffer}
 */
var integerAsBuffer = function integerAsBuffer(integer)
{
    $.checkArgumentType(integer, 'number', 'integer');
    var bytes = [];
    bytes.push((integer >> 24) & 0xff);
    bytes.push((integer >> 16) & 0xff);
    bytes.push((integer >> 8) & 0xff);
    bytes.push(integer & 0xff);
    return new Buffer(bytes);
}
BufferUtil.integerAsBuffer = integerAsBuffer;

/**
 * Transform the first 4 values of a Buffer into a number, in little endian encoding
 *
 * @param {Buffer} buffer
 * @return {number}
 */
var integerFromBuffer = function integerFromBuffer(buffer)
{
    $.checkArgumentType(buffer, 'Buffer', 'buffer');
    return buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
}
BufferUtil.integerFromBuffer = integerFromBuffer;

/**
 * Transforms the first byte of an array into a number ranging from -128 to 127
 * @param {Buffer} buffer
 * @return {number}
 */
var integerFromSingleByteBuffer = function integerFromBuffer(buffer)
{
    $.checkArgumentType(buffer, 'Buffer', 'buffer');
    return buffer[0];
}
BufferUtil.integerFromSingleByteBuffer = integerFromSingleByteBuffer;

/**
 * Transforms a buffer into a string with a number in hexa representation
 *
 * Shorthand for <tt>buffer.toString('hex')</tt>
 *
 * @param {Buffer} buffer
 * @return {string}
 */
var bufferToHex = function bufferToHex(buffer)
{
    $.checkArgumentType(buffer, 'Buffer', 'buffer');
    return buffer.toString('hex');
}
BufferUtil.bufferToHex = bufferToHex;

/**
 * Reverse a buffer
 * @param {Buffer} param
 * @return {Buffer}
 */
var reverse = function reverse(param)
{
    var ret = new Buffer(param.length);
    for (var i = 0; i < param.length; i++)
    {
        ret[i] = param[param.length - i - 1];
    }
    return ret;
}
BufferUtil.reverse = reverse;

/**
 * Transforms an hexa encoded string into a Buffer with binary values
 *
 * Shorthand for <tt>Buffer(string, 'hex')</tt>
 *
 * @param {string} string
 * @return {Buffer}
 */
var hexToBuffer = function hexToBuffer(string)
{
    ix_base.assert(js.isHexa(string));
    return new Buffer(string, 'hex');
}
BufferUtil.hexToBuffer = hexToBuffer;

BufferUtil.NULL_HASH = fill(new Buffer(32), 0);
BufferUtil.EMPTY_BUFFER = new Buffer(0);

magnachain.BufferUtil = BufferUtil;
// util/buffer end--------------------------------------------------------------------------

// opcode begin-----------------------------------------------------------------------------
//var _ = require('lodash');
//var $ = require('./util/preconditions');
//var BufferUtil = require('./util/buffer');
//var JSUtil = require('./util/js');

function Opcode(num)
{
    if (!(this instanceof Opcode))
    {
        return new Opcode(num);
    }

    var value;

    if (_.isNumber(num))
    {
        value = num;
    } else if (_.isString(num))
    {
        value = Opcode.map[num];
    } else
    {
        throw new TypeError('Unrecognized num type: "' + typeof (num) + '" for Opcode');
    }

    JSUtil.defineImmutable(this, {
        num: value
    });

    return this;
}

Opcode.fromBuffer = function (buf)
{
    $.checkArgument(BufferUtil.isBuffer(buf));
    return new Opcode(Number('0x' + buf.toString('hex')));
};

Opcode.fromNumber = function (num)
{
    $.checkArgument(_.isNumber(num));
    return new Opcode(num);
};

Opcode.fromString = function (str)
{
    $.checkArgument(_.isString(str));
    var value = Opcode.map[str];
    if (typeof value === 'undefined')
    {
        throw new TypeError('Invalid opcodestr');
    }
    return new Opcode(value);
};

Opcode.prototype.toHex = function ()
{
    return this.num.toString(16);
};

Opcode.prototype.toBuffer = function ()
{
    return new Buffer(this.toHex(), 'hex');
};

Opcode.prototype.toNumber = function ()
{
    return this.num;
};

Opcode.prototype.toString = function ()
{
    var str = Opcode.reverseMap[this.num];
    if (typeof str === 'undefined')
    {
        throw new Error('Opcode does not have a string representation');
    }
    return str;
};

Opcode.smallInt = function (n)
{
    $.checkArgument(_.isNumber(n), 'Invalid Argument: n should be number');
    $.checkArgument(n >= 0 && n <= 16, 'Invalid Argument: n must be between 0 and 16');
    if (n === 0)
    {
        return Opcode('OP_0');
    }
    return new Opcode(Opcode.map.OP_1 + n - 1);
};

Opcode.map = {
    // push value
    OP_FALSE: 0,
    OP_0: 0,
    OP_PUSHDATA1: 76,
    OP_PUSHDATA2: 77,
    OP_PUSHDATA4: 78,
    OP_1NEGATE: 79,
    OP_RESERVED: 80,
    OP_TRUE: 81,
    OP_1: 81,
    OP_2: 82,
    OP_3: 83,
    OP_4: 84,
    OP_5: 85,
    OP_6: 86,
    OP_7: 87,
    OP_8: 88,
    OP_9: 89,
    OP_10: 90,
    OP_11: 91,
    OP_12: 92,
    OP_13: 93,
    OP_14: 94,
    OP_15: 95,
    OP_16: 96,

    // control
    OP_NOP: 97,
    OP_VER: 98,
    OP_IF: 99,
    OP_NOTIF: 100,
    OP_VERIF: 101,
    OP_VERNOTIF: 102,
    OP_ELSE: 103,
    OP_ENDIF: 104,
    OP_VERIFY: 105,
    OP_RETURN: 106,

    // stack ops
    OP_TOALTSTACK: 107,
    OP_FROMALTSTACK: 108,
    OP_2DROP: 109,
    OP_2DUP: 110,
    OP_3DUP: 111,
    OP_2OVER: 112,
    OP_2ROT: 113,
    OP_2SWAP: 114,
    OP_IFDUP: 115,
    OP_DEPTH: 116,
    OP_DROP: 117,
    OP_DUP: 118,
    OP_NIP: 119,
    OP_OVER: 120,
    OP_PICK: 121,
    OP_ROLL: 122,
    OP_ROT: 123,
    OP_SWAP: 124,
    OP_TUCK: 125,

    // splice ops
    OP_CAT: 126,
    OP_SUBSTR: 127,
    OP_LEFT: 128,
    OP_RIGHT: 129,
    OP_SIZE: 130,

    // bit logic
    OP_INVERT: 131,
    OP_AND: 132,
    OP_OR: 133,
    OP_XOR: 134,
    OP_EQUAL: 135,
    OP_EQUALVERIFY: 136,
    OP_RESERVED1: 137,
    OP_RESERVED2: 138,

    // numeric
    OP_1ADD: 139,
    OP_1SUB: 140,
    OP_2MUL: 141,
    OP_2DIV: 142,
    OP_NEGATE: 143,
    OP_ABS: 144,
    OP_NOT: 145,
    OP_0NOTEQUAL: 146,

    OP_ADD: 147,
    OP_SUB: 148,
    OP_MUL: 149,
    OP_DIV: 150,
    OP_MOD: 151,
    OP_LSHIFT: 152,
    OP_RSHIFT: 153,

    OP_BOOLAND: 154,
    OP_BOOLOR: 155,
    OP_NUMEQUAL: 156,
    OP_NUMEQUALVERIFY: 157,
    OP_NUMNOTEQUAL: 158,
    OP_LESSTHAN: 159,
    OP_GREATERTHAN: 160,
    OP_LESSTHANOREQUAL: 161,
    OP_GREATERTHANOREQUAL: 162,
    OP_MIN: 163,
    OP_MAX: 164,

    OP_WITHIN: 165,

    // crypto
    OP_RIPEMD160: 166,
    OP_SHA1: 167,
    OP_SHA256: 168,
    OP_HASH160: 169,
    OP_HASH256: 170,
    OP_CODESEPARATOR: 171,
    OP_CHECKSIG: 172,
    OP_CHECKSIGVERIFY: 173,
    OP_CHECKMULTISIG: 174,
    OP_CHECKMULTISIGVERIFY: 175,

    OP_CHECKLOCKTIMEVERIFY: 177,

    // expansion
    OP_NOP1: 176,
    OP_NOP2: 177,
    OP_NOP3: 178,
    OP_NOP4: 179,
    OP_NOP5: 180,
    OP_NOP6: 181,
    OP_NOP7: 182,
    OP_NOP8: 183,
    OP_NOP9: 184,
    OP_NOP10: 185,

    // smart contract
    OP_CONTRACT_ADDR: 192,
    OP_CONTRACT: 193,
    OP_CONTRACT_CHANGE: 194,

    OP_CREATE_BRANCH: 208,
    OP_TRANS_BRANCH: 209,

    // template matching params
    OP_PUBKEYHASH: 253,
    OP_PUBKEY: 254,
    OP_INVALIDOPCODE: 255
};

Opcode.reverseMap = [];

for (var k in Opcode.map)
{
    Opcode.reverseMap[Opcode.map[k]] = k;
}

// Easier access to opcodes
_.extend(Opcode, Opcode.map);

/**
 * @returns true if opcode is one of OP_0, OP_1, ..., OP_16
 */
Opcode.isSmallIntOp = function (opcode)
{
    if (opcode instanceof Opcode)
    {
        opcode = opcode.toNumber();
    }
    return ((opcode === Opcode.map.OP_0) ||
        ((opcode >= Opcode.map.OP_1) && (opcode <= Opcode.map.OP_16)));
};

/**
 * Will return a string formatted for the console
 *
 * @returns {string} Script opcode
 */
Opcode.prototype.inspect = function ()
{
    return '<Opcode: ' + this.toString() + ', hex: ' + this.toHex() + ', decimal: ' + this.num + '>';
};

//module.exports = Opcode;
magnachain.Opcode = Opcode;
// opcode end-------------------------------------------------------------------------------

// crypto/bn begin-------------------------------------------------------------------------------
//var BN = require('bn.js');
//var $ = require('../util/preconditions');
//var _ = require('lodash');

var reversebuf = function (buf)
{
    var buf2 = new Buffer(buf.length);
    for (var i = 0; i < buf.length; i++)
    {
        buf2[i] = buf[buf.length - 1 - i];
    }
    return buf2;
};

BN.Zero = new BN(0);
BN.One = new BN(1);
BN.Minus1 = new BN(-1);

BN.fromNumber = function (n)
{
    $.checkArgument(_.isNumber(n));
    return new BN(n);
};

BN.fromString = function (str, base)
{
    $.checkArgument(_.isString(str));
    return new BN(str, base);
};

BN.fromBuffer = function (buf, opts)
{
    //console.log("BN.fromBuffer: " + buf + " opts: " + opts);
    if (typeof opts !== 'undefined' && opts.endian === 'little')
    {
        buf = reversebuf(buf);
    }
    var hex = buf.toString('hex');
    //console.log("Buf Hex: " + hex);
    var bn = new BN(hex, 16);
    return bn;
};

/**
 * Instantiate a BigNumber from a "signed magnitude buffer"
 * (a buffer where the most significant bit represents the sign (0 = positive, -1 = negative))
 */
BN.fromSM = function (buf, opts)
{
    var ret;
    if (buf.length === 0)
    {
        return BN.fromBuffer(new Buffer([0]));
    }

    var endian = 'big';
    if (opts)
    {
        endian = opts.endian;
    }
    if (endian === 'little')
    {
        buf = reversebuf(buf);
    }

    if (buf[0] & 0x80)
    {
        buf[0] = buf[0] & 0x7f;
        ret = BN.fromBuffer(buf);
        ret.neg().copy(ret);
    } else
    {
        ret = BN.fromBuffer(buf);
    }
    return ret;
};


BN.prototype.toNumber = function ()
{
    return parseInt(this.toString(10), 10);
};

BN.prototype.toBuffer = function (opts)
{
    var buf, hex;
    if (opts && opts.size)
    {
        hex = this.toString(16, 2);
        var natlen = hex.length / 2;
        buf = new Buffer(hex, 'hex');

        if (natlen === opts.size)
        {
            buf = buf;
        } else if (natlen > opts.size)
        {
            buf = BN.trim(buf, natlen);
        } else if (natlen < opts.size)
        {
            buf = BN.pad(buf, natlen, opts.size);
        }
    } else
    {
        hex = this.toString(16, 2);
        buf = new Buffer(hex, 'hex');
    }

    if (typeof opts !== 'undefined' && opts.endian === 'little')
    {
        buf = reversebuf(buf);
    }

    return buf;
};

BN.prototype.toSMBigEndian = function ()
{
    var buf;
    if (this.cmp(BN.Zero) === -1)
    {
        buf = this.neg().toBuffer();
        if (buf[0] & 0x80)
        {
            buf = Buffer.concat([new Buffer([0x80]), buf]);
        } else
        {
            buf[0] = buf[0] | 0x80;
        }
    } else
    {
        buf = this.toBuffer();
        if (buf[0] & 0x80)
        {
            buf = Buffer.concat([new Buffer([0x00]), buf]);
        }
    }

    if (buf.length === 1 & buf[0] === 0)
    {
        buf = new Buffer([]);
    }
    return buf;
};

BN.prototype.toSM = function (opts)
{
    var endian = opts ? opts.endian : 'big';
    var buf = this.toSMBigEndian();

    if (endian === 'little')
    {
        buf = reversebuf(buf);
    }
    return buf;
};

/**
 * Create a BN from a "ScriptNum":
 * This is analogous to the constructor for CScriptNum in bitcoind. Many ops in
 * bitcoind's script interpreter use CScriptNum, which is not really a proper
 * bignum. Instead, an error is thrown if trying to input a number bigger than
 * 4 bytes. We copy that behavior here. A third argument, `size`, is provided to
 * extend the hard limit of 4 bytes, as some usages require more than 4 bytes.
 */
BN.fromScriptNumBuffer = function (buf, fRequireMinimal, size)
{
    var nMaxNumSize = size || 4;
    $.checkArgument(buf.length <= nMaxNumSize, new Error('script number overflow'));
    if (fRequireMinimal && buf.length > 0)
    {
        // Check that the number is encoded with the minimum possible
        // number of bytes.
        //
        // If the most-significant-byte - excluding the sign bit - is zero
        // then we're not minimal. Note how this test also rejects the
        // negative-zero encoding, 0x80.
        if ((buf[buf.length - 1] & 0x7f) === 0)
        {
            // One exception: if there's more than one byte and the most
            // significant bit of the second-most-significant-byte is set
            // it would conflict with the sign bit. An example of this case
            // is +-255, which encode to 0xff00 and 0xff80 respectively.
            // (big-endian).
            if (buf.length <= 1 || (buf[buf.length - 2] & 0x80) === 0)
            {
                throw new Error('non-minimally encoded script number');
            }
        }
    }
    return BN.fromSM(buf, {
        endian: 'little'
    });
};

/**
 * The corollary to the above, with the notable exception that we do not throw
 * an error if the output is larger than four bytes. (Which can happen if
 * performing a numerical operation that results in an overflow to more than 4
 * bytes).
 */
BN.prototype.toScriptNumBuffer = function ()
{
    return this.toSM({
        endian: 'little'
    });
};

BN.prototype.gt = function (b)
{
    return this.cmp(b) > 0;
};

BN.prototype.gte = function (b)
{
    return this.cmp(b) >= 0;
};

BN.prototype.lt = function (b)
{
    return this.cmp(b) < 0;
};

BN.trim = function (buf, natlen)
{
    return buf.slice(natlen - buf.length, buf.length);
};

BN.pad = function (buf, natlen, size)
{
    var rbuf = new Buffer(size);
    for (var i = 0; i < buf.length; i++)
    {
        rbuf[rbuf.length - 1 - i] = buf[buf.length - 1 - i];
    }
    for (i = 0; i < size - natlen; i++)
    {
        rbuf[i] = 0;
    }
    return rbuf;
};

magnachain.BN = BN;
// crypto/bn end-------------------------------------------------------------------------------

// crypto/random begin-------------------------------------------------------------------------------
function Random()
{
}

/* secure random bytes that sometimes throws an error due to lack of entropy */
Random.getRandomBuffer = function (size)
{
    // if (process.browser)
    //     return Random.getRandomBufferBrowser(size);
    // else
    //     return Random.getRandomBufferNode(size);

    return Random.getRandomBufferBrowser(size);
};

Random.getRandomBufferNode = function (size)
{
    //var crypto = require('crypto');
    return ix_crypto.randomBytes(size);
};

Random.getRandomBufferBrowser = function (size)
{
    if (!window.crypto && !window.msCrypto)
        throw new Error('window.crypto not available');

    if (window.crypto && window.crypto.getRandomValues)
        var crypto = window.crypto;
    else if (window.msCrypto && window.msCrypto.getRandomValues) //internet explorer
        var crypto = window.msCrypto;
    else
        throw new Error('window.crypto.getRandomValues not available');

    var bbuf = new Uint8Array(size);
    crypto.getRandomValues(bbuf);
    var buf = new Buffer(bbuf);

    return buf;
};

/* insecure random bytes, but it never fails */
Random.getPseudoRandomBuffer = function (size)
{
    var b32 = 0x100000000;
    var b = new Buffer(size);
    var r;

    for (var i = 0; i <= size; i++)
    {
        var j = Math.floor(i / 4);
        var k = i - j * 4;
        if (k === 0)
        {
            r = Math.random() * b32;
            b[i] = r & 0xff;
        } else
        {
            b[i] = (r = r >>> 8) & 0xff;
        }
    }

    return b;
};

magnachain.Random = Random;
// crypto/random end-------------------------------------------------------------------------------

// crypto/hash begin-------------------------------------------------------------------------------
//var ix_crypto = require('crypto');
//var BufferUtil = require('../util/buffer');
//var $ = require('../util/preconditions');

var Hash = {}

Hash.sha1 = function (buf)
{
    $.checkArgument(BufferUtil.isBuffer(buf));
    var kS = ix_crypto.createHash('sha1');
    kS.update(buf, 0);
    var arrR = kS.digest();    // hex ?
    return Buffer._fromArrayBuffer(arrR, 0, arrR.length);
    //return crypto.createHash('sha1').update(buf).digest();
};

Hash.sha1.blocksize = 512;

Hash.sha256 = function (buf)
{
    $.checkArgument(BufferUtil.isBuffer(buf));
    var kS = ix_crypto.createHash('sha256');
    kS.update(buf, 0);
    var arrR = kS.digest();    // hex ?
    return Buffer._fromArrayBuffer(arrR, 0, arrR.length);
    //return crypto.createHash('sha256').update(buf).digest();
};

Hash.sha256.blocksize = 512;

Hash.sha256sha256 = function (buf)
{
    $.checkArgument(BufferUtil.isBuffer(buf));
    return Hash.sha256(Hash.sha256(buf));
};

Hash.ripemd160 = function (buf)
{
    //console.log("AAAAAA: " + buf);
    var bRet = BufferUtil.isBuffer(buf);
    $.checkArgument(bRet);
    var kS = ix_crypto.createHash('ripemd160');
    kS.update(buf, 0);
    var arrR = kS.digest();    // hex ?
    return Buffer._fromArrayBuffer(arrR, 0, arrR.length);
    //return crypto.createHash('ripemd160').update(buf).digest();
};

Hash.sha256ripemd160 = function (buf)
{
    $.checkArgument(BufferUtil.isBuffer(buf));
    var ibuf = Hash.sha256(buf);

    return Hash.ripemd160(ibuf);
};

// input Buffer
Hash.sha512 = function (buf)
{
    $.checkArgument(BufferUtil.isBuffer(buf));
    var kS = ix_crypto.createHash('sha512');
    kS.update(buf, 0);
    var arrR = kS.digest();    // hex ?
    return Buffer._fromArrayBuffer(arrR, 0, arrR.length);
    //return crypto.createHash('sha512').update(buf).digest();
};

Hash.sha512.blocksize = 1024;

Hash.hmac = function (hashf, data, key)
{
    //http://en.wikipedia.org/wiki/Hash-based_message_authentication_code
    //http://tools.ietf.org/html/rfc4868#section-2
    $.checkArgument(BufferUtil.isBuffer(data));
    $.checkArgument(BufferUtil.isBuffer(key));
    $.checkArgument(hashf.blocksize);

    //console.log("UUUU: " + hashf + " data: " + data + " key: " + key);

    var blocksize = hashf.blocksize / 8;

    if (key.length > blocksize)
    {
        key = hashf(key);
    } else if (key < blocksize)
    {
        var fill = new Buffer(blocksize);
        fill.fill(0);
        key.copy(fill);
        key = fill;
    }

    //console.log("XXXXKey: " + key);

    var o_key = new Buffer(blocksize);
    o_key.fill(0x5c);

    var i_key = new Buffer(blocksize);
    i_key.fill(0x36);

    var o_key_pad = new Buffer(blocksize);
    var i_key_pad = new Buffer(blocksize);
    for (var i = 0; i < blocksize; i++)
    {
        o_key_pad[i] = o_key[i] ^ key[i];
        i_key_pad[i] = i_key[i] ^ key[i];
    }

    var p0 = Buffer.concat([i_key_pad, data]);
    //var p0 = Buffer.concat([data, i_key_pad]);      
    //console.log("PPP0: " + p0);
    var p1 = hashf(p0);
    //console.log("PPP1: " + p1 + " len: " + p1.length);
    var p2 = Buffer.concat([o_key_pad, p1]);
    //var p2 = Buffer.concat([p1, o_key_pad]);    
    //console.log("PPP2: " + p2);

    var pf = hashf(p2);
    //console.log("PPPF: " + pf + " len: " + pf.length);
    return pf;
};

Hash.sha256hmac = function (data, key)
{
    return Hash.hmac(Hash.sha256, data, key);
};

// data is a Buffer
Hash.sha512hmac = function (data, key)
{
    return Hash.hmac(Hash.sha512, data, key);
};

magnachain.Hash = Hash;
// crypto/hash end-------------------------------------------------------------------------------

// crypto/signature begin-------------------------------------------------------------------------------
// var BN = require('./bn');
// var _ = require('lodash');
// var $ = require('../util/preconditions');
// var BufferUtil = require('../util/buffer');
// var JSUtil = require('../util/js');

var Signature = function (r, s)
{
    if (!(this instanceof Signature))
    {
        return new Signature(r, s);
    }
    if (r instanceof BN)
    {
        this.set({
            r: r,
            s: s
        });
    } else if (r)
    {
        var obj = r;
        this.set(obj);
    }
};

/* jshint maxcomplexity: 7 */
Signature.prototype.set = function (obj)
{
    this.r = obj.r || this.r || undefined;
    this.s = obj.s || this.s || undefined;

    this.i = typeof obj.i !== 'undefined' ? obj.i : this.i; //public key recovery parameter in range [0, 3]
    this.compressed = typeof obj.compressed !== 'undefined' ?
        obj.compressed : this.compressed; //whether the recovered pubkey is compressed
    this.nhashtype = obj.nhashtype || this.nhashtype || undefined;
    return this;
};

Signature.fromCompact = function (buf)
{
    $.checkArgument(BufferUtil.isBuffer(buf), 'Argument is expected to be a Buffer');

    var sig = new Signature();

    var compressed = true;
    var i = buf.slice(0, 1)[0] - 27 - 4;
    if (i < 0)
    {
        compressed = false;
        i = i + 4;
    }

    var b2 = buf.slice(1, 33);
    var b3 = buf.slice(33, 65);

    $.checkArgument(i === 0 || i === 1 || i === 2 || i === 3, new Error('i must be 0, 1, 2, or 3'));
    $.checkArgument(b2.length === 32, new Error('r must be 32 bytes'));
    $.checkArgument(b3.length === 32, new Error('s must be 32 bytes'));

    sig.compressed = compressed;
    sig.i = i;
    sig.r = BN.fromBuffer(b2);
    sig.s = BN.fromBuffer(b3);

    return sig;
};

Signature.fromDER = Signature.fromBuffer = function (buf, strict)
{
    var obj = Signature.parseDER(buf, strict);
    var sig = new Signature();

    sig.r = obj.r;
    sig.s = obj.s;

    return sig;
};

// The format used in a tx
Signature.fromTxFormat = function (buf)
{
    var nhashtype = buf.readUInt8(buf.length - 1);
    var derbuf = buf.slice(0, buf.length - 1);
    var sig = new Signature.fromDER(derbuf, false);
    sig.nhashtype = nhashtype;
    return sig;
};

Signature.fromString = function (str)
{
    var buf = new Buffer(str, 'hex');
    return Signature.fromDER(buf);
};


/**
 * In order to mimic the non-strict DER encoding of OpenSSL, set strict = false.
 */
Signature.parseDER = function (buf, strict)
{
    $.checkArgument(BufferUtil.isBuffer(buf), new Error('DER formatted signature should be a buffer'));
    if (_.isUndefined(strict))
    {
        strict = true;
    }

    var header = buf[0];
    $.checkArgument(header === 0x30, new Error('Header byte should be 0x30'));

    var length = buf[1];
    var buflength = buf.slice(2).length;
    $.checkArgument(!strict || length === buflength, new Error('Length byte should length of what follows'));

    length = length < buflength ? length : buflength;

    var rheader = buf[2 + 0];
    $.checkArgument(rheader === 0x02, new Error('Integer byte for r should be 0x02'));

    var rlength = buf[2 + 1];
    var rbuf = buf.slice(2 + 2, 2 + 2 + rlength);
    var r = BN.fromBuffer(rbuf);
    var rneg = buf[2 + 1 + 1] === 0x00 ? true : false;
    $.checkArgument(rlength === rbuf.length, new Error('Length of r incorrect'));

    var sheader = buf[2 + 2 + rlength + 0];
    $.checkArgument(sheader === 0x02, new Error('Integer byte for s should be 0x02'));

    var slength = buf[2 + 2 + rlength + 1];
    var sbuf = buf.slice(2 + 2 + rlength + 2, 2 + 2 + rlength + 2 + slength);
    var s = BN.fromBuffer(sbuf);
    var sneg = buf[2 + 2 + rlength + 2 + 2] === 0x00 ? true : false;
    $.checkArgument(slength === sbuf.length, new Error('Length of s incorrect'));

    var sumlength = 2 + 2 + rlength + 2 + slength;
    $.checkArgument(length === sumlength - 2, new Error('Length of signature incorrect'));

    var obj = {
        header: header,
        length: length,
        rheader: rheader,
        rlength: rlength,
        rneg: rneg,
        rbuf: rbuf,
        r: r,
        sheader: sheader,
        slength: slength,
        sneg: sneg,
        sbuf: sbuf,
        s: s
    };

    return obj;
};

Signature.prototype.toCompact = function (i, compressed)
{
    i = typeof i === 'number' ? i : this.i;
    compressed = typeof compressed === 'boolean' ? compressed : this.compressed;

    if (!(i === 0 || i === 1 || i === 2 || i === 3))
    {
        throw new Error('i must be equal to 0, 1, 2, or 3');
    }

    var val = i + 27 + 4;
    if (compressed === false)
    {
        val = val - 4;
    }
    var b1 = new Buffer([val]);
    var b2 = this.r.toBuffer({
        size: 32
    });
    var b3 = this.s.toBuffer({
        size: 32
    });
    return Buffer.concat([b1, b2, b3]);
};

Signature.prototype.toBuffer = Signature.prototype.toDER = function ()
{
    var rnbuf = this.r.toBuffer();
    var snbuf = this.s.toBuffer();

    var rneg = rnbuf[0] & 0x80 ? true : false;
    var sneg = snbuf[0] & 0x80 ? true : false;

    var rbuf = rneg ? Buffer.concat([new Buffer([0x00]), rnbuf]) : rnbuf;
    var sbuf = sneg ? Buffer.concat([new Buffer([0x00]), snbuf]) : snbuf;

    var rlength = rbuf.length;
    var slength = sbuf.length;
    var length = 2 + rlength + 2 + slength;
    var rheader = 0x02;
    var sheader = 0x02;
    var header = 0x30;

    var der = Buffer.concat([new Buffer([header, length, rheader, rlength]), rbuf, new Buffer([sheader, slength]), sbuf]);
    return der;
};

Signature.prototype.toString = function ()
{
    var buf = this.toDER();
    return buf.toString('hex');
};

/**
 * This function is translated from bitcoind's IsDERSignature and is used in
 * the script interpreter.  This "DER" format actually includes an extra byte,
 * the nhashtype, at the end. It is really the tx format, not DER format.
 *
 * A canonical signature exists of: [30] [total len] [02] [len R] [R] [02] [len S] [S] [hashtype]
 * Where R and S are not negative (their first byte has its highest bit not set), and not
 * excessively padded (do not start with a 0 byte, unless an otherwise negative number follows,
 * in which case a single 0 byte is necessary and even required).
 *
 * See https://bitcointalk.org/index.php?topic=8392.msg127623#msg127623
 */
Signature.isTxDER = function (buf)
{
    if (buf.length < 9)
    {
        //  Non-canonical signature: too short
        return false;
    }
    if (buf.length > 73)
    {
        // Non-canonical signature: too long
        return false;
    }
    if (buf[0] !== 0x30)
    {
        //  Non-canonical signature: wrong type
        return false;
    }
    if (buf[1] !== buf.length - 3)
    {
        //  Non-canonical signature: wrong length marker
        return false;
    }
    var nLenR = buf[3];
    if (5 + nLenR >= buf.length)
    {
        //  Non-canonical signature: S length misplaced
        return false;
    }
    var nLenS = buf[5 + nLenR];
    if ((nLenR + nLenS + 7) !== buf.length)
    {
        //  Non-canonical signature: R+S length mismatch
        return false;
    }

    var R = buf.slice(4);
    if (buf[4 - 2] !== 0x02)
    {
        //  Non-canonical signature: R value type mismatch
        return false;
    }
    if (nLenR === 0)
    {
        //  Non-canonical signature: R length is zero
        return false;
    }
    if (R[0] & 0x80)
    {
        //  Non-canonical signature: R value negative
        return false;
    }
    if (nLenR > 1 && (R[0] === 0x00) && !(R[1] & 0x80))
    {
        //  Non-canonical signature: R value excessively padded
        return false;
    }

    var S = buf.slice(6 + nLenR);
    if (buf[6 + nLenR - 2] !== 0x02)
    {
        //  Non-canonical signature: S value type mismatch
        return false;
    }
    if (nLenS === 0)
    {
        //  Non-canonical signature: S length is zero
        return false;
    }
    if (S[0] & 0x80)
    {
        //  Non-canonical signature: S value negative
        return false;
    }
    if (nLenS > 1 && (S[0] === 0x00) && !(S[1] & 0x80))
    {
        //  Non-canonical signature: S value excessively padded
        return false;
    }
    return true;
};

/**
 * Compares to bitcoind's IsLowDERSignature
 * See also ECDSA signature algorithm which enforces this.
 * See also BIP 62, "low S values in signatures"
 */
Signature.prototype.hasLowS = function ()
{
    if (this.s.lt(new BN(1)) ||
        this.s.gt(new BN('7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5D576E7357A4501DDFE92F46681B20A0', 'hex')))
    {
        return false;
    }
    return true;
};

/**
 * @returns true if the nhashtype is exactly equal to one of the standard options or combinations thereof.
 * Translated from bitcoind's IsDefinedHashtypeSignature
 */
Signature.prototype.hasDefinedHashtype = function ()
{
    if (!JSUtil.isNaturalNumber(this.nhashtype))
    {
        return false;
    }
    // accept with or without Signature.SIGHASH_ANYONECANPAY by ignoring the bit
    var temp = this.nhashtype & ~Signature.SIGHASH_ANYONECANPAY;
    if (temp < Signature.SIGHASH_ALL || temp > Signature.SIGHASH_SINGLE)
    {
        return false;
    }
    return true;
};

Signature.prototype.toTxFormat = function ()
{
    var derbuf = this.toDER();
    var buf = new Buffer(1);
    buf.writeUInt8(this.nhashtype, 0);
    return Buffer.concat([derbuf, buf]);
};

Signature.SIGHASH_ALL = 0x01;
Signature.SIGHASH_NONE = 0x02;
Signature.SIGHASH_SINGLE = 0x03;
Signature.SIGHASH_ANYONECANPAY = 0x80;

//module.exports = Signature;
magnachain.Signature = Signature;
// crypto/signature end-------------------------------------------------------------------------------

// elliptic/utils begin----------------------------------------------------------------------
var elliptic_utils = {};
//var BN = require('bn.js');
var minAssert = ix_base.asset;
var minUtils = ix_base.minic_utils;

elliptic_utils.assert = minAssert;
elliptic_utils.toArray = minUtils.toArray;
elliptic_utils.zero2 = minUtils.zero2;
elliptic_utils.toHex = minUtils.toHex;
elliptic_utils.encode = minUtils.encode;

// Represent num in a w-NAF form
function getNAF(num, w)
{
    var naf = [];
    var ws = 1 << (w + 1);
    var k = num.clone();
    while (k.cmpn(1) >= 0)
    {
        var z;
        if (k.isOdd())
        {
            var mod = k.andln(ws - 1);
            if (mod > (ws >> 1) - 1)
                z = (ws >> 1) - mod;
            else
                z = mod;
            k.isubn(z);
        } else
        {
            z = 0;
        }
        naf.push(z);

        // Optimization, shift by word if possible
        var shift = (k.cmpn(0) !== 0 && k.andln(ws - 1) === 0) ? (w + 1) : 1;
        for (var i = 1; i < shift; i++)
            naf.push(0);
        k.iushrn(shift);
    }

    return naf;
}
elliptic_utils.getNAF = getNAF;

// Represent k1, k2 in a Joint Sparse Form
function getJSF(k1, k2)
{
    var jsf = [
        [],
        []
    ];

    k1 = k1.clone();
    k2 = k2.clone();
    var d1 = 0;
    var d2 = 0;
    while (k1.cmpn(-d1) > 0 || k2.cmpn(-d2) > 0)
    {

        // First phase
        var m14 = (k1.andln(3) + d1) & 3;
        var m24 = (k2.andln(3) + d2) & 3;
        if (m14 === 3)
            m14 = -1;
        if (m24 === 3)
            m24 = -1;
        var u1;
        if ((m14 & 1) === 0)
        {
            u1 = 0;
        } else
        {
            var m8 = (k1.andln(7) + d1) & 7;
            if ((m8 === 3 || m8 === 5) && m24 === 2)
                u1 = -m14;
            else
                u1 = m14;
        }
        jsf[0].push(u1);

        var u2;
        if ((m24 & 1) === 0)
        {
            u2 = 0;
        } else
        {
            var m8 = (k2.andln(7) + d2) & 7;
            if ((m8 === 3 || m8 === 5) && m14 === 2)
                u2 = -m24;
            else
                u2 = m24;
        }
        jsf[1].push(u2);

        // Second phase
        if (2 * d1 === u1 + 1)
            d1 = 1 - d1;
        if (2 * d2 === u2 + 1)
            d2 = 1 - d2;
        k1.iushrn(1);
        k2.iushrn(1);
    }

    return jsf;
}
elliptic_utils.getJSF = getJSF;

function cachedProperty(obj, name, computer)
{
    var key = '_' + name;
    obj.prototype[name] = function cachedProperty()
    {
        return this[key] !== undefined ? this[key] :
            this[key] = computer.call(this);
    };
}
elliptic_utils.cachedProperty = cachedProperty;

function parseBytes(bytes)
{
    return typeof bytes === 'string' ? elliptic_utils.toArray(bytes, 'hex') :
        bytes;
}
elliptic_utils.parseBytes = parseBytes;

function intFromLE(bytes)
{
    return new BN(bytes, 'hex', 'le');
}
elliptic_utils.intFromLE = intFromLE;

magnachain.elliptic_utils = elliptic_utils;
// elliptic/utils end----------------------------------------------------------------------

// elliptic/curve/base begin----------------------------------------------------------------------
var elliptic_curve = {};

//var BN = require('bn.js');
//var elliptic = require('../../elliptic');
var elliptic_utils = elliptic_utils;
var getNAF = elliptic_utils.getNAF;
var getJSF = elliptic_utils.getJSF;
//var ix_base.assert = elliptic_utils.assert;

function BaseCurve(type, conf)
{
    this.type = type;
    this.p = new BN(conf.p, 16);

    // Use Montgomery, when there is no fast reduction for the prime
    this.red = conf.prime ? BN.red(conf.prime) : BN.mont(this.p);

    // Useful for many curves
    this.zero = new BN(0).toRed(this.red);
    this.one = new BN(1).toRed(this.red);
    this.two = new BN(2).toRed(this.red);

    // Curve configuration, optional
    this.n = conf.n && new BN(conf.n, 16);
    this.g = conf.g && this.pointFromJSON(conf.g, conf.gRed);

    // Temporary arrays
    this._wnafT1 = new Array(4);
    this._wnafT2 = new Array(4);
    this._wnafT3 = new Array(4);
    this._wnafT4 = new Array(4);

    // Generalized Greg Maxwell's trick
    var adjustCount = this.n && this.p.div(this.n);
    if (!adjustCount || adjustCount.cmpn(100) > 0)
    {
        this.redN = null;
    } else
    {
        this._maxwellTrick = true;
        this.redN = this.n.toRed(this.red);
    }
}
elliptic_curve.base = BaseCurve;

BaseCurve.prototype.point = function ()
{
    throw new Error('Not implemented');
};

BaseCurve.prototype.validate = function validate()
{
    throw new Error('Not implemented');
};

BaseCurve.prototype._fixedNafMul = function _fixedNafMul(p, k)
{
    ix_base.assert(p.precomputed);
    var doubles = p._getDoubles();

    var naf = getNAF(k, 1);
    var I = (1 << (doubles.step + 1)) - (doubles.step % 2 === 0 ? 2 : 1);
    I /= 3;

    // Translate into more windowed form
    var repr = [];
    for (var j = 0; j < naf.length; j += doubles.step)
    {
        var nafW = 0;
        for (var k = j + doubles.step - 1; k >= j; k--)
            nafW = (nafW << 1) + naf[k];
        repr.push(nafW);
    }

    var a = this.jpoint(null, null, null);
    var b = this.jpoint(null, null, null);
    for (var i = I; i > 0; i--)
    {
        for (var j = 0; j < repr.length; j++)
        {
            var nafW = repr[j];
            if (nafW === i)
                b = b.mixedAdd(doubles.points[j]);
            else if (nafW === -i)
                b = b.mixedAdd(doubles.points[j].neg());
        }
        a = a.add(b);
    }
    return a.toP();
};

BaseCurve.prototype._wnafMul = function _wnafMul(p, k)
{
    var w = 4;

    // Precompute window
    var nafPoints = p._getNAFPoints(w);
    w = nafPoints.wnd;
    var wnd = nafPoints.points;

    // Get NAF form
    var naf = getNAF(k, w);

    // Add `this`*(N+1) for every w-NAF index
    var acc = this.jpoint(null, null, null);
    for (var i = naf.length - 1; i >= 0; i--)
    {
        // Count zeroes
        for (var k = 0; i >= 0 && naf[i] === 0; i--)
            k++;
        if (i >= 0)
            k++;
        acc = acc.dblp(k);

        if (i < 0)
            break;
        var z = naf[i];
        ix_base.assert(z !== 0);
        if (p.type === 'affine')
        {
            // J +- P
            if (z > 0)
                acc = acc.mixedAdd(wnd[(z - 1) >> 1]);
            else
                acc = acc.mixedAdd(wnd[(-z - 1) >> 1].neg());
        } else
        {
            // J +- J
            if (z > 0)
                acc = acc.add(wnd[(z - 1) >> 1]);
            else
                acc = acc.add(wnd[(-z - 1) >> 1].neg());
        }
    }
    return p.type === 'affine' ? acc.toP() : acc;
};

BaseCurve.prototype._wnafMulAdd = function _wnafMulAdd(defW,
    points,
    coeffs,
    len,
    jacobianResult)
{
    var wndWidth = this._wnafT1;
    var wnd = this._wnafT2;
    var naf = this._wnafT3;

    // Fill all arrays
    var max = 0;
    for (var i = 0; i < len; i++)
    {
        var p = points[i];
        var nafPoints = p._getNAFPoints(defW);
        wndWidth[i] = nafPoints.wnd;
        wnd[i] = nafPoints.points;
    }

    // Comb small window NAFs
    for (var i = len - 1; i >= 1; i -= 2)
    {
        var a = i - 1;
        var b = i;
        if (wndWidth[a] !== 1 || wndWidth[b] !== 1)
        {
            naf[a] = getNAF(coeffs[a], wndWidth[a]);
            naf[b] = getNAF(coeffs[b], wndWidth[b]);
            max = Math.max(naf[a].length, max);
            max = Math.max(naf[b].length, max);
            continue;
        }

        var comb = [
            points[a], /* 1 */
            null, /* 3 */
            null, /* 5 */
            points[b] /* 7 */
        ];

        // Try to avoid Projective points, if possible
        if (points[a].y.cmp(points[b].y) === 0)
        {
            comb[1] = points[a].add(points[b]);
            comb[2] = points[a].toJ().mixedAdd(points[b].neg());
        } else if (points[a].y.cmp(points[b].y.redNeg()) === 0)
        {
            comb[1] = points[a].toJ().mixedAdd(points[b]);
            comb[2] = points[a].add(points[b].neg());
        } else
        {
            comb[1] = points[a].toJ().mixedAdd(points[b]);
            comb[2] = points[a].toJ().mixedAdd(points[b].neg());
        }

        var index = [
            -3, /* -1 -1 */
            -1, /* -1 0 */
            -5, /* -1 1 */
            -7, /* 0 -1 */
            0, /* 0 0 */
            7, /* 0 1 */
            5, /* 1 -1 */
            1, /* 1 0 */
            3  /* 1 1 */
        ];

        var jsf = getJSF(coeffs[a], coeffs[b]);
        max = Math.max(jsf[0].length, max);
        naf[a] = new Array(max);
        naf[b] = new Array(max);
        for (var j = 0; j < max; j++)
        {
            var ja = jsf[0][j] | 0;
            var jb = jsf[1][j] | 0;

            naf[a][j] = index[(ja + 1) * 3 + (jb + 1)];
            naf[b][j] = 0;
            wnd[a] = comb;
        }
    }

    var acc = this.jpoint(null, null, null);
    var tmp = this._wnafT4;
    for (var i = max; i >= 0; i--)
    {
        var k = 0;

        while (i >= 0)
        {
            var zero = true;
            for (var j = 0; j < len; j++)
            {
                tmp[j] = naf[j][i] | 0;
                if (tmp[j] !== 0)
                    zero = false;
            }
            if (!zero)
                break;
            k++;
            i--;
        }
        if (i >= 0)
            k++;
        acc = acc.dblp(k);
        if (i < 0)
            break;

        for (var j = 0; j < len; j++)
        {
            var z = tmp[j];
            var p;
            if (z === 0)
                continue;
            else if (z > 0)
                p = wnd[j][(z - 1) >> 1];
            else if (z < 0)
                p = wnd[j][(-z - 1) >> 1].neg();

            if (p.type === 'affine')
                acc = acc.mixedAdd(p);
            else
                acc = acc.add(p);
        }
    }
    // Zeroify references
    for (var i = 0; i < len; i++)
        wnd[i] = null;

    if (jacobianResult)
        return acc;
    else
        return acc.toP();
};

function BasePoint(curve, type)
{
    this.curve = curve;
    this.type = type;
    this.precomputed = null;
}
BaseCurve.BasePoint = BasePoint;

BasePoint.prototype.eq = function eq(/*other*/)
{
    throw new Error('Not implemented');
};

BasePoint.prototype.validate = function validate()
{
    return this.curve.validate(this);
};

BaseCurve.prototype.decodePoint = function decodePoint(bytes, enc)
{
    bytes = elliptic_utils.toArray(bytes, enc);

    var len = this.p.byteLength();

    // uncompressed, hybrid-odd, hybrid-even
    if ((bytes[0] === 0x04 || bytes[0] === 0x06 || bytes[0] === 0x07) &&
        bytes.length - 1 === 2 * len)
    {
        if (bytes[0] === 0x06)
            ix_base.assert(bytes[bytes.length - 1] % 2 === 0);
        else if (bytes[0] === 0x07)
            ix_base.assert(bytes[bytes.length - 1] % 2 === 1);

        var res = this.point(bytes.slice(1, 1 + len),
            bytes.slice(1 + len, 1 + 2 * len));

        return res;
    } else if ((bytes[0] === 0x02 || bytes[0] === 0x03) &&
        bytes.length - 1 === len)
    {
        return this.pointFromX(bytes.slice(1, 1 + len), bytes[0] === 0x03);
    }
    throw new Error('Unknown point format');
};

BasePoint.prototype.encodeCompressed = function encodeCompressed(enc)
{
    return this.encode(enc, true);
};

BasePoint.prototype._encode = function _encode(compact)
{
    var len = this.curve.p.byteLength();
    var x = this.getX().toArray('be', len);

    if (compact)
        return [this.getY().isEven() ? 0x02 : 0x03].concat(x);

    return [0x04].concat(x, this.getY().toArray('be', len));
};

BasePoint.prototype.encode = function encode(enc, compact)
{
    return elliptic_utils.encode(this._encode(compact), enc);
};

BasePoint.prototype.precompute = function precompute(power)
{
    if (this.precomputed)
        return this;

    var precomputed = {
        doubles: null,
        naf: null,
        beta: null
    };
    precomputed.naf = this._getNAFPoints(8);
    precomputed.doubles = this._getDoubles(4, power);
    precomputed.beta = this._getBeta();
    this.precomputed = precomputed;

    return this;
};

BasePoint.prototype._hasDoubles = function _hasDoubles(k)
{
    if (!this.precomputed)
        return false;

    var doubles = this.precomputed.doubles;
    if (!doubles)
        return false;

    return doubles.points.length >= Math.ceil((k.bitLength() + 1) / doubles.step);
};

BasePoint.prototype._getDoubles = function _getDoubles(step, power)
{
    if (this.precomputed && this.precomputed.doubles)
        return this.precomputed.doubles;

    var doubles = [this];
    var acc = this;
    for (var i = 0; i < power; i += step)
    {
        for (var j = 0; j < step; j++)
            acc = acc.dbl();
        doubles.push(acc);
    }
    return {
        step: step,
        points: doubles
    };
};

BasePoint.prototype._getNAFPoints = function _getNAFPoints(wnd)
{
    if (this.precomputed && this.precomputed.naf)
        return this.precomputed.naf;

    var res = [this];
    var max = (1 << wnd) - 1;
    var dbl = max === 1 ? null : this.dbl();
    for (var i = 1; i < max; i++)
        res[i] = res[i - 1].add(dbl);
    return {
        wnd: wnd,
        points: res
    };
};

BasePoint.prototype._getBeta = function _getBeta()
{
    return null;
};

BasePoint.prototype.dblp = function dblp(k)
{
    var r = this;
    for (var i = 0; i < k; i++)
        r = r.dbl();
    return r;
};
// elliptic/curve/bvse end----------------------------------------------------------------------

// elliptic/curve/edward begin----------------------------------------------------------------------
//var elliptic = require('../../elliptic');
//var BN = require('bn.js');
var inherits = ix_base.inherits;
var Base = elliptic_curve.base;

//var assert = elliptic.utils.assert;

function EdwardsCurve(conf)
{
    // NOTE: Important as we are creating point in Base.call()
    this.twisted = (conf.a | 0) !== 1;
    this.mOneA = this.twisted && (conf.a | 0) === -1;
    this.extended = this.mOneA;

    Base.call(this, 'edwards', conf);

    this.a = new BN(conf.a, 16).umod(this.red.m);
    this.a = this.a.toRed(this.red);
    this.c = new BN(conf.c, 16).toRed(this.red);
    this.c2 = this.c.redSqr();
    this.d = new BN(conf.d, 16).toRed(this.red);
    this.dd = this.d.redAdd(this.d);

    ix_base.assert(!this.twisted || this.c.fromRed().cmpn(1) === 0);
    this.oneC = (conf.c | 0) === 1;
}
inherits(EdwardsCurve, Base);
elliptic_curve.edwards = EdwardsCurve;

EdwardsCurve.prototype._mulA = function _mulA(num)
{
    if (this.mOneA)
        return num.redNeg();
    else
        return this.a.redMul(num);
};

EdwardsCurve.prototype._mulC = function _mulC(num)
{
    if (this.oneC)
        return num;
    else
        return this.c.redMul(num);
};

// Just for compatibility with Short curve
EdwardsCurve.prototype.jpoint = function jpoint(x, y, z, t)
{
    return this.point(x, y, z, t);
};

EdwardsCurve.prototype.pointFromX = function pointFromX(x, odd)
{
    x = new BN(x, 16);
    if (!x.red)
        x = x.toRed(this.red);

    var x2 = x.redSqr();
    var rhs = this.c2.redSub(this.a.redMul(x2));
    var lhs = this.one.redSub(this.c2.redMul(this.d).redMul(x2));

    var y2 = rhs.redMul(lhs.redInvm());
    var y = y2.redSqrt();
    if (y.redSqr().redSub(y2).cmp(this.zero) !== 0)
        throw new Error('invalid point');

    var isOdd = y.fromRed().isOdd();
    if (odd && !isOdd || !odd && isOdd)
        y = y.redNeg();

    return this.point(x, y);
};

EdwardsCurve.prototype.pointFromY = function pointFromY(y, odd)
{
    y = new BN(y, 16);
    if (!y.red)
        y = y.toRed(this.red);

    // x^2 = (y^2 - 1) / (d y^2 + 1)
    var y2 = y.redSqr();
    var lhs = y2.redSub(this.one);
    var rhs = y2.redMul(this.d).redAdd(this.one);
    var x2 = lhs.redMul(rhs.redInvm());

    if (x2.cmp(this.zero) === 0)
    {
        if (odd)
            throw new Error('invalid point');
        else
            return this.point(this.zero, y);
    }

    var x = x2.redSqrt();
    if (x.redSqr().redSub(x2).cmp(this.zero) !== 0)
        throw new Error('invalid point');

    if (x.isOdd() !== odd)
        x = x.redNeg();

    return this.point(x, y);
};

EdwardsCurve.prototype.validate = function validate(point)
{
    if (point.isInfinity())
        return true;

    // Curve: A * X^2 + Y^2 = C^2 * (1 + D * X^2 * Y^2)
    point.normalize();

    var x2 = point.x.redSqr();
    var y2 = point.y.redSqr();
    var lhs = x2.redMul(this.a).redAdd(y2);
    var rhs = this.c2.redMul(this.one.redAdd(this.d.redMul(x2).redMul(y2)));

    return lhs.cmp(rhs) === 0;
};

function EdwardsPoint(curve, x, y, z, t)
{
    Base.BasePoint.call(this, curve, 'projective');
    if (x === null && y === null && z === null)
    {
        this.x = this.curve.zero;
        this.y = this.curve.one;
        this.z = this.curve.one;
        this.t = this.curve.zero;
        this.zOne = true;
    } else
    {
        this.x = new BN(x, 16);
        this.y = new BN(y, 16);
        this.z = z ? new BN(z, 16) : this.curve.one;
        this.t = t && new BN(t, 16);
        if (!this.x.red)
            this.x = this.x.toRed(this.curve.red);
        if (!this.y.red)
            this.y = this.y.toRed(this.curve.red);
        if (!this.z.red)
            this.z = this.z.toRed(this.curve.red);
        if (this.t && !this.t.red)
            this.t = this.t.toRed(this.curve.red);
        this.zOne = this.z === this.curve.one;

        // Use extended coordinates
        if (this.curve.extended && !this.t)
        {
            this.t = this.x.redMul(this.y);
            if (!this.zOne)
                this.t = this.t.redMul(this.z.redInvm());
        }
    }
}
inherits(EdwardsPoint, Base.BasePoint);

EdwardsCurve.prototype.pointFromJSON = function pointFromJSON(obj)
{
    return EdwardsPoint.fromJSON(this, obj);
};

EdwardsCurve.prototype.point = function (x, y, z, t)
{
    return new EdwardsPoint(this, x, y, z, t);
};

EdwardsPoint.fromJSON = function fromJSON(curve, obj)
{
    return new EdwardsPoint(curve, obj[0], obj[1], obj[2]);
};

EdwardsPoint.prototype.inspect = function inspect()
{
    if (this.isInfinity())
        return '<EC Point Infinity>';
    return '<EC Point x: ' + this.x.fromRed().toString(16, 2) +
        ' y: ' + this.y.fromRed().toString(16, 2) +
        ' z: ' + this.z.fromRed().toString(16, 2) + '>';
};

EdwardsPoint.prototype.isInfinity = function isInfinity()
{
    // XXX This code assumes that zero is always zero in red
    return this.x.cmpn(0) === 0 &&
        this.y.cmp(this.z) === 0;
};

EdwardsPoint.prototype._extDbl = function _extDbl()
{
    // hyperelliptic.org/EFD/g1p/auto-twisted-extended-1.html
    //     #doubling-dbl-2008-hwcd
    // 4M + 4S

    // A = X1^2
    var a = this.x.redSqr();
    // B = Y1^2
    var b = this.y.redSqr();
    // C = 2 * Z1^2
    var c = this.z.redSqr();
    c = c.redIAdd(c);
    // D = a * A
    var d = this.curve._mulA(a);
    // E = (X1 + Y1)^2 - A - B
    var e = this.x.redAdd(this.y).redSqr().redISub(a).redISub(b);
    // G = D + B
    var g = d.redAdd(b);
    // F = G - C
    var f = g.redSub(c);
    // H = D - B
    var h = d.redSub(b);
    // X3 = E * F
    var nx = e.redMul(f);
    // Y3 = G * H
    var ny = g.redMul(h);
    // T3 = E * H
    var nt = e.redMul(h);
    // Z3 = F * G
    var nz = f.redMul(g);
    return this.curve.point(nx, ny, nz, nt);
};

EdwardsPoint.prototype._projDbl = function _projDbl()
{
    // hyperelliptic.org/EFD/g1p/auto-twisted-projective.html
    //     #doubling-dbl-2008-bbjlp
    //     #doubling-dbl-2007-bl
    // and others
    // Generally 3M + 4S or 2M + 4S

    // B = (X1 + Y1)^2
    var b = this.x.redAdd(this.y).redSqr();
    // C = X1^2
    var c = this.x.redSqr();
    // D = Y1^2
    var d = this.y.redSqr();

    var nx;
    var ny;
    var nz;
    if (this.curve.twisted)
    {
        // E = a * C
        var e = this.curve._mulA(c);
        // F = E + D
        var f = e.redAdd(d);
        if (this.zOne)
        {
            // X3 = (B - C - D) * (F - 2)
            nx = b.redSub(c).redSub(d).redMul(f.redSub(this.curve.two));
            // Y3 = F * (E - D)
            ny = f.redMul(e.redSub(d));
            // Z3 = F^2 - 2 * F
            nz = f.redSqr().redSub(f).redSub(f);
        } else
        {
            // H = Z1^2
            var h = this.z.redSqr();
            // J = F - 2 * H
            var j = f.redSub(h).redISub(h);
            // X3 = (B-C-D)*J
            nx = b.redSub(c).redISub(d).redMul(j);
            // Y3 = F * (E - D)
            ny = f.redMul(e.redSub(d));
            // Z3 = F * J
            nz = f.redMul(j);
        }
    } else
    {
        // E = C + D
        var e = c.redAdd(d);
        // H = (c * Z1)^2
        var h = this.curve._mulC(this.c.redMul(this.z)).redSqr();
        // J = E - 2 * H
        var j = e.redSub(h).redSub(h);
        // X3 = c * (B - E) * J
        nx = this.curve._mulC(b.redISub(e)).redMul(j);
        // Y3 = c * E * (C - D)
        ny = this.curve._mulC(e).redMul(c.redISub(d));
        // Z3 = E * J
        nz = e.redMul(j);
    }
    return this.curve.point(nx, ny, nz);
};

EdwardsPoint.prototype.dbl = function dbl()
{
    if (this.isInfinity())
        return this;

    // Double in extended coordinates
    if (this.curve.extended)
        return this._extDbl();
    else
        return this._projDbl();
};

EdwardsPoint.prototype._extAdd = function _extAdd(p)
{
    // hyperelliptic.org/EFD/g1p/auto-twisted-extended-1.html
    //     #addition-add-2008-hwcd-3
    // 8M

    // A = (Y1 - X1) * (Y2 - X2)
    var a = this.y.redSub(this.x).redMul(p.y.redSub(p.x));
    // B = (Y1 + X1) * (Y2 + X2)
    var b = this.y.redAdd(this.x).redMul(p.y.redAdd(p.x));
    // C = T1 * k * T2
    var c = this.t.redMul(this.curve.dd).redMul(p.t);
    // D = Z1 * 2 * Z2
    var d = this.z.redMul(p.z.redAdd(p.z));
    // E = B - A
    var e = b.redSub(a);
    // F = D - C
    var f = d.redSub(c);
    // G = D + C
    var g = d.redAdd(c);
    // H = B + A
    var h = b.redAdd(a);
    // X3 = E * F
    var nx = e.redMul(f);
    // Y3 = G * H
    var ny = g.redMul(h);
    // T3 = E * H
    var nt = e.redMul(h);
    // Z3 = F * G
    var nz = f.redMul(g);
    return this.curve.point(nx, ny, nz, nt);
};

EdwardsPoint.prototype._projAdd = function _projAdd(p)
{
    // hyperelliptic.org/EFD/g1p/auto-twisted-projective.html
    //     #addition-add-2008-bbjlp
    //     #addition-add-2007-bl
    // 10M + 1S

    // A = Z1 * Z2
    var a = this.z.redMul(p.z);
    // B = A^2
    var b = a.redSqr();
    // C = X1 * X2
    var c = this.x.redMul(p.x);
    // D = Y1 * Y2
    var d = this.y.redMul(p.y);
    // E = d * C * D
    var e = this.curve.d.redMul(c).redMul(d);
    // F = B - E
    var f = b.redSub(e);
    // G = B + E
    var g = b.redAdd(e);
    // X3 = A * F * ((X1 + Y1) * (X2 + Y2) - C - D)
    var tmp = this.x.redAdd(this.y).redMul(p.x.redAdd(p.y)).redISub(c).redISub(d);
    var nx = a.redMul(f).redMul(tmp);
    var ny;
    var nz;
    if (this.curve.twisted)
    {
        // Y3 = A * G * (D - a * C)
        ny = a.redMul(g).redMul(d.redSub(this.curve._mulA(c)));
        // Z3 = F * G
        nz = f.redMul(g);
    } else
    {
        // Y3 = A * G * (D - C)
        ny = a.redMul(g).redMul(d.redSub(c));
        // Z3 = c * F * G
        nz = this.curve._mulC(f).redMul(g);
    }
    return this.curve.point(nx, ny, nz);
};

EdwardsPoint.prototype.add = function add(p)
{
    if (this.isInfinity())
        return p;
    if (p.isInfinity())
        return this;

    if (this.curve.extended)
        return this._extAdd(p);
    else
        return this._projAdd(p);
};

EdwardsPoint.prototype.mul = function mul(k)
{
    if (this._hasDoubles(k))
        return this.curve._fixedNafMul(this, k);
    else
        return this.curve._wnafMul(this, k);
};

EdwardsPoint.prototype.mulAdd = function mulAdd(k1, p, k2)
{
    return this.curve._wnafMulAdd(1, [this, p], [k1, k2], 2, false);
};

EdwardsPoint.prototype.jmulAdd = function jmulAdd(k1, p, k2)
{
    return this.curve._wnafMulAdd(1, [this, p], [k1, k2], 2, true);
};

EdwardsPoint.prototype.normalize = function normalize()
{
    if (this.zOne)
        return this;

    // Normalize coordinates
    var zi = this.z.redInvm();
    this.x = this.x.redMul(zi);
    this.y = this.y.redMul(zi);
    if (this.t)
        this.t = this.t.redMul(zi);
    this.z = this.curve.one;
    this.zOne = true;
    return this;
};

EdwardsPoint.prototype.neg = function neg()
{
    return this.curve.point(this.x.redNeg(),
        this.y,
        this.z,
        this.t && this.t.redNeg());
};

EdwardsPoint.prototype.getX = function getX()
{
    this.normalize();
    return this.x.fromRed();
};

EdwardsPoint.prototype.getY = function getY()
{
    this.normalize();
    return this.y.fromRed();
};

EdwardsPoint.prototype.eq = function eq(other)
{
    return this === other ||
        this.getX().cmp(other.getX()) === 0 &&
        this.getY().cmp(other.getY()) === 0;
};

EdwardsPoint.prototype.eqXToP = function eqXToP(x)
{
    var rx = x.toRed(this.curve.red).redMul(this.z);
    if (this.x.cmp(rx) === 0)
        return true;

    var xc = x.clone();
    var t = this.curve.redN.redMul(this.z);
    for (; ;)
    {
        xc.iadd(this.curve.n);
        if (xc.cmp(this.curve.p) >= 0)
            return false;

        rx.redIAdd(t);
        if (this.x.cmp(rx) === 0)
            return true;
    }
    return false;
};

// Compatibility with BaseCurve
EdwardsPoint.prototype.toP = EdwardsPoint.prototype.normalize;
EdwardsPoint.prototype.mixedAdd = EdwardsPoint.prototype.add;
// elliptic/curve/edward end----------------------------------------------------------------------

// elliptic/curve/mont begin----------------------------------------------------------------------
//var BN = require('bn.js');
//var inherits = require('inherits');
//var Base = curve.base;

//var elliptic = require('../../elliptic');
//var elliptic_utils = elliptic.utils;

function MontCurve(conf)
{
    Base.call(this, 'mont', conf);

    this.a = new BN(conf.a, 16).toRed(this.red);
    this.b = new BN(conf.b, 16).toRed(this.red);
    this.i4 = new BN(4).toRed(this.red).redInvm();
    this.two = new BN(2).toRed(this.red);
    this.a24 = this.i4.redMul(this.a.redAdd(this.two));
}
inherits(MontCurve, Base);
elliptic_curve.mont = MontCurve;

MontCurve.prototype.validate = function validate(point)
{
    var x = point.normalize().x;
    var x2 = x.redSqr();
    var rhs = x2.redMul(x).redAdd(x2.redMul(this.a)).redAdd(x);
    var y = rhs.redSqrt();

    return y.redSqr().cmp(rhs) === 0;
};

function MontPoint(curve, x, z)
{
    Base.BasePoint.call(this, curve, 'projective');
    if (x === null && z === null)
    {
        this.x = this.curve.one;
        this.z = this.curve.zero;
    } else
    {
        this.x = new BN(x, 16);
        this.z = new BN(z, 16);
        if (!this.x.red)
            this.x = this.x.toRed(this.curve.red);
        if (!this.z.red)
            this.z = this.z.toRed(this.curve.red);
    }
}
inherits(MontPoint, Base.BasePoint);

MontCurve.prototype.decodePoint = function decodePoint(bytes, enc)
{
    return this.point(elliptic_utils.toArray(bytes, enc), 1);
};

MontCurve.prototype.point = function (x, z)
{
    return new MontPoint(this, x, z);
};

MontCurve.prototype.pointFromJSON = function pointFromJSON(obj)
{
    return MontPoint.fromJSON(this, obj);
};

MontPoint.prototype.precompute = function precompute()
{
    // No-op
};

MontPoint.prototype._encode = function _encode()
{
    return this.getX().toArray('be', this.curve.p.byteLength());
};

MontPoint.fromJSON = function fromJSON(curve, obj)
{
    return new MontPoint(curve, obj[0], obj[1] || curve.one);
};

MontPoint.prototype.inspect = function inspect()
{
    if (this.isInfinity())
        return '<EC Point Infinity>';
    return '<EC Point x: ' + this.x.fromRed().toString(16, 2) +
        ' z: ' + this.z.fromRed().toString(16, 2) + '>';
};

MontPoint.prototype.isInfinity = function isInfinity()
{
    // XXX This code assumes that zero is always zero in red
    return this.z.cmpn(0) === 0;
};

MontPoint.prototype.dbl = function dbl()
{
    // http://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#doubling-dbl-1987-m-3
    // 2M + 2S + 4A

    // A = X1 + Z1
    var a = this.x.redAdd(this.z);
    // AA = A^2
    var aa = a.redSqr();
    // B = X1 - Z1
    var b = this.x.redSub(this.z);
    // BB = B^2
    var bb = b.redSqr();
    // C = AA - BB
    var c = aa.redSub(bb);
    // X3 = AA * BB
    var nx = aa.redMul(bb);
    // Z3 = C * (BB + A24 * C)
    var nz = c.redMul(bb.redAdd(this.curve.a24.redMul(c)));
    return this.curve.point(nx, nz);
};

MontPoint.prototype.add = function add()
{
    throw new Error('Not supported on Montgomery curve');
};

MontPoint.prototype.diffAdd = function diffAdd(p, diff)
{
    // http://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#diffadd-dadd-1987-m-3
    // 4M + 2S + 6A

    // A = X2 + Z2
    var a = this.x.redAdd(this.z);
    // B = X2 - Z2
    var b = this.x.redSub(this.z);
    // C = X3 + Z3
    var c = p.x.redAdd(p.z);
    // D = X3 - Z3
    var d = p.x.redSub(p.z);
    // DA = D * A
    var da = d.redMul(a);
    // CB = C * B
    var cb = c.redMul(b);
    // X5 = Z1 * (DA + CB)^2
    var nx = diff.z.redMul(da.redAdd(cb).redSqr());
    // Z5 = X1 * (DA - CB)^2
    var nz = diff.x.redMul(da.redISub(cb).redSqr());
    return this.curve.point(nx, nz);
};

MontPoint.prototype.mul = function mul(k)
{
    var t = k.clone();
    var a = this; // (N / 2) * Q + Q
    var b = this.curve.point(null, null); // (N / 2) * Q
    var c = this; // Q

    for (var bits = []; t.cmpn(0) !== 0; t.iushrn(1))
        bits.push(t.andln(1));

    for (var i = bits.length - 1; i >= 0; i--)
    {
        if (bits[i] === 0)
        {
            // N * Q + Q = ((N / 2) * Q + Q)) + (N / 2) * Q
            a = a.diffAdd(b, c);
            // N * Q = 2 * ((N / 2) * Q + Q))
            b = b.dbl();
        } else
        {
            // N * Q = ((N / 2) * Q + Q) + ((N / 2) * Q)
            b = a.diffAdd(b, c);
            // N * Q + Q = 2 * ((N / 2) * Q + Q)
            a = a.dbl();
        }
    }
    return b;
};

MontPoint.prototype.mulAdd = function mulAdd()
{
    throw new Error('Not supported on Montgomery curve');
};

MontPoint.prototype.jumlAdd = function jumlAdd()
{
    throw new Error('Not supported on Montgomery curve');
};

MontPoint.prototype.eq = function eq(other)
{
    return this.getX().cmp(other.getX()) === 0;
};

MontPoint.prototype.normalize = function normalize()
{
    this.x = this.x.redMul(this.z.redInvm());
    this.z = this.curve.one;
    return this;
};

MontPoint.prototype.getX = function getX()
{
    // Normalize coordinates
    this.normalize();

    return this.x.fromRed();
};
// elliptic/curve/mont end----------------------------------------------------------------------

// elliptic/curve/short begin----------------------------------------------------------------------
//var elliptic = require('../../elliptic');
//var BN = require('bn.js');
//var inherits = require('inherits');
//var Base = curve.base;

//var assert = elliptic.utils.assert;

function ShortCurve(conf)
{
    Base.call(this, 'short', conf);

    this.a = new BN(conf.a, 16).toRed(this.red);
    this.b = new BN(conf.b, 16).toRed(this.red);
    this.tinv = this.two.redInvm();

    this.zeroA = this.a.fromRed().cmpn(0) === 0;
    this.threeA = this.a.fromRed().sub(this.p).cmpn(-3) === 0;

    // If the curve is endomorphic, precalculate beta and lambda
    this.endo = this._getEndomorphism(conf);
    this._endoWnafT1 = new Array(4);
    this._endoWnafT2 = new Array(4);
}
inherits(ShortCurve, Base);
elliptic_curve.short = ShortCurve;

ShortCurve.prototype._getEndomorphism = function _getEndomorphism(conf)
{
    // No efficient endomorphism
    if (!this.zeroA || !this.g || !this.n || this.p.modn(3) !== 1)
        return;

    // Compute beta and lambda, that lambda * P = (beta * Px; Py)
    var beta;
    var lambda;
    if (conf.beta)
    {
        beta = new BN(conf.beta, 16).toRed(this.red);
    } else
    {
        var betas = this._getEndoRoots(this.p);
        // Choose the smallest beta
        beta = betas[0].cmp(betas[1]) < 0 ? betas[0] : betas[1];
        beta = beta.toRed(this.red);
    }
    if (conf.lambda)
    {
        lambda = new BN(conf.lambda, 16);
    } else
    {
        // Choose the lambda that is matching selected beta
        var lambdas = this._getEndoRoots(this.n);
        if (this.g.mul(lambdas[0]).x.cmp(this.g.x.redMul(beta)) === 0)
        {
            lambda = lambdas[0];
        } else
        {
            lambda = lambdas[1];
            ix_base.assert(this.g.mul(lambda).x.cmp(this.g.x.redMul(beta)) === 0);
        }
    }

    // Get basis vectors, used for balanced length-two representation
    var basis;
    if (conf.basis)
    {
        basis = conf.basis.map(function (vec)
        {
            return {
                a: new BN(vec.a, 16),
                b: new BN(vec.b, 16)
            };
        });
    } else
    {
        basis = this._getEndoBasis(lambda);
    }

    return {
        beta: beta,
        lambda: lambda,
        basis: basis
    };
};

ShortCurve.prototype._getEndoRoots = function _getEndoRoots(num)
{
    // Find roots of for x^2 + x + 1 in F
    // Root = (-1 +- Sqrt(-3)) / 2
    //
    var red = num === this.p ? this.red : BN.mont(num);
    var tinv = new BN(2).toRed(red).redInvm();
    var ntinv = tinv.redNeg();

    var s = new BN(3).toRed(red).redNeg().redSqrt().redMul(tinv);

    var l1 = ntinv.redAdd(s).fromRed();
    var l2 = ntinv.redSub(s).fromRed();
    return [l1, l2];
};

ShortCurve.prototype._getEndoBasis = function _getEndoBasis(lambda)
{
    // aprxSqrt >= sqrt(this.n)
    var aprxSqrt = this.n.ushrn(Math.floor(this.n.bitLength() / 2));

    // 3.74
    // Run EGCD, until r(L + 1) < aprxSqrt
    var u = lambda;
    var v = this.n.clone();
    var x1 = new BN(1);
    var y1 = new BN(0);
    var x2 = new BN(0);
    var y2 = new BN(1);

    // NOTE: all vectors are roots of: a + b * lambda = 0 (mod n)
    var a0;
    var b0;
    // First vector
    var a1;
    var b1;
    // Second vector
    var a2;
    var b2;

    var prevR;
    var i = 0;
    var r;
    var x;
    while (u.cmpn(0) !== 0)
    {
        var q = v.div(u);
        r = v.sub(q.mul(u));
        x = x2.sub(q.mul(x1));
        var y = y2.sub(q.mul(y1));

        if (!a1 && r.cmp(aprxSqrt) < 0)
        {
            a0 = prevR.neg();
            b0 = x1;
            a1 = r.neg();
            b1 = x;
        } else if (a1 && ++i === 2)
        {
            break;
        }
        prevR = r;

        v = u;
        u = r;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
    }
    a2 = r.neg();
    b2 = x;

    var len1 = a1.sqr().add(b1.sqr());
    var len2 = a2.sqr().add(b2.sqr());
    if (len2.cmp(len1) >= 0)
    {
        a2 = a0;
        b2 = b0;
    }

    // Normalize signs
    if (a1.negative)
    {
        a1 = a1.neg();
        b1 = b1.neg();
    }
    if (a2.negative)
    {
        a2 = a2.neg();
        b2 = b2.neg();
    }

    return [
        { a: a1, b: b1 },
        { a: a2, b: b2 }
    ];
};

ShortCurve.prototype._endoSplit = function _endoSplit(k)
{
    var basis = this.endo.basis;
    var v1 = basis[0];
    var v2 = basis[1];

    var c1 = v2.b.mul(k).divRound(this.n);
    var c2 = v1.b.neg().mul(k).divRound(this.n);

    var p1 = c1.mul(v1.a);
    var p2 = c2.mul(v2.a);
    var q1 = c1.mul(v1.b);
    var q2 = c2.mul(v2.b);

    // Calculate answer
    var k1 = k.sub(p1).sub(p2);
    var k2 = q1.add(q2).neg();
    return { k1: k1, k2: k2 };
};

ShortCurve.prototype.pointFromX = function pointFromX(x, odd)
{
    x = new BN(x, 16);
    if (!x.red)
        x = x.toRed(this.red);

    var y2 = x.redSqr().redMul(x).redIAdd(x.redMul(this.a)).redIAdd(this.b);
    var y = y2.redSqrt();
    if (y.redSqr().redSub(y2).cmp(this.zero) !== 0)
        throw new Error('invalid point');

    // XXX Is there any way to tell if the number is odd without converting it
    // to non-red form?
    var isOdd = y.fromRed().isOdd();
    if (odd && !isOdd || !odd && isOdd)
        y = y.redNeg();

    return this.point(x, y);
};

ShortCurve.prototype.validate = function validate(point)
{
    if (point.inf)
        return true;

    var x = point.x;
    var y = point.y;

    var ax = this.a.redMul(x);
    var rhs = x.redSqr().redMul(x).redIAdd(ax).redIAdd(this.b);
    return y.redSqr().redISub(rhs).cmpn(0) === 0;
};

ShortCurve.prototype._endoWnafMulAdd =
    function _endoWnafMulAdd(points, coeffs, jacobianResult)
    {
        var npoints = this._endoWnafT1;
        var ncoeffs = this._endoWnafT2;
        for (var i = 0; i < points.length; i++)
        {
            var split = this._endoSplit(coeffs[i]);
            var p = points[i];
            var beta = p._getBeta();

            if (split.k1.negative)
            {
                split.k1.ineg();
                p = p.neg(true);
            }
            if (split.k2.negative)
            {
                split.k2.ineg();
                beta = beta.neg(true);
            }

            npoints[i * 2] = p;
            npoints[i * 2 + 1] = beta;
            ncoeffs[i * 2] = split.k1;
            ncoeffs[i * 2 + 1] = split.k2;
        }
        var res = this._wnafMulAdd(1, npoints, ncoeffs, i * 2, jacobianResult);

        // Clean-up references to points and coefficients
        for (var j = 0; j < i * 2; j++)
        {
            npoints[j] = null;
            ncoeffs[j] = null;
        }
        return res;
    };

function ShortPoint(curve, x, y, isRed)
{
    Base.BasePoint.call(this, curve, 'affine');
    if (x === null && y === null)
    {
        this.x = null;
        this.y = null;
        this.inf = true;
    } else
    {
        this.x = new BN(x, 16);
        this.y = new BN(y, 16);
        // Force redgomery representation when loading from JSON
        if (isRed)
        {
            this.x.forceRed(this.curve.red);
            this.y.forceRed(this.curve.red);
        }
        if (!this.x.red)
            this.x = this.x.toRed(this.curve.red);
        if (!this.y.red)
            this.y = this.y.toRed(this.curve.red);
        this.inf = false;
    }
}
inherits(ShortPoint, Base.BasePoint);

ShortCurve.prototype.point = function (x, y, isRed)
{
    return new ShortPoint(this, x, y, isRed);
};

ShortCurve.prototype.pointFromJSON = function pointFromJSON(obj, red)
{
    return ShortPoint.fromJSON(this, obj, red);
};

ShortPoint.prototype._getBeta = function _getBeta()
{
    if (!this.curve.endo)
        return;

    var pre = this.precomputed;
    if (pre && pre.beta)
        return pre.beta;

    var beta = this.curve.point(this.x.redMul(this.curve.endo.beta), this.y);
    if (pre)
    {
        var curve = this.curve;
        var endoMul = function (p)
        {
            return curve.point(p.x.redMul(curve.endo.beta), p.y);
        };
        pre.beta = beta;
        beta.precomputed = {
            beta: null,
            naf: pre.naf && {
                wnd: pre.naf.wnd,
                points: pre.naf.points.map(endoMul)
            },
            doubles: pre.doubles && {
                step: pre.doubles.step,
                points: pre.doubles.points.map(endoMul)
            }
        };
    }
    return beta;
};

ShortPoint.prototype.toJSON = function toJSON()
{
    if (!this.precomputed)
        return [this.x, this.y];

    return [this.x, this.y, this.precomputed && {
        doubles: this.precomputed.doubles && {
            step: this.precomputed.doubles.step,
            points: this.precomputed.doubles.points.slice(1)
        },
        naf: this.precomputed.naf && {
            wnd: this.precomputed.naf.wnd,
            points: this.precomputed.naf.points.slice(1)
        }
    }];
};

ShortPoint.fromJSON = function fromJSON(curve, obj, red)
{
    if (typeof obj === 'string')
        obj = JSON.parse(obj);
    var res = curve.point(obj[0], obj[1], red);
    if (!obj[2])
        return res;

    function obj2point(obj)
    {
        return curve.point(obj[0], obj[1], red);
    }

    var pre = obj[2];
    res.precomputed = {
        beta: null,
        doubles: pre.doubles && {
            step: pre.doubles.step,
            points: [res].concat(pre.doubles.points.map(obj2point))
        },
        naf: pre.naf && {
            wnd: pre.naf.wnd,
            points: [res].concat(pre.naf.points.map(obj2point))
        }
    };
    return res;
};

ShortPoint.prototype.inspect = function inspect()
{
    if (this.isInfinity())
        return '<EC Point Infinity>';
    return '<EC Point x: ' + this.x.fromRed().toString(16, 2) +
        ' y: ' + this.y.fromRed().toString(16, 2) + '>';
};

ShortPoint.prototype.isInfinity = function isInfinity()
{
    return this.inf;
};

ShortPoint.prototype.add = function add(p)
{
    // O + P = P
    if (this.inf)
        return p;

    // P + O = P
    if (p.inf)
        return this;

    // P + P = 2P
    if (this.eq(p))
        return this.dbl();

    // P + (-P) = O
    if (this.neg().eq(p))
        return this.curve.point(null, null);

    // P + Q = O
    if (this.x.cmp(p.x) === 0)
        return this.curve.point(null, null);

    var c = this.y.redSub(p.y);
    if (c.cmpn(0) !== 0)
        c = c.redMul(this.x.redSub(p.x).redInvm());
    var nx = c.redSqr().redISub(this.x).redISub(p.x);
    var ny = c.redMul(this.x.redSub(nx)).redISub(this.y);
    return this.curve.point(nx, ny);
};

ShortPoint.prototype.dbl = function dbl()
{
    if (this.inf)
        return this;

    // 2P = O
    var ys1 = this.y.redAdd(this.y);
    if (ys1.cmpn(0) === 0)
        return this.curve.point(null, null);

    var a = this.curve.a;

    var x2 = this.x.redSqr();
    var dyinv = ys1.redInvm();
    var c = x2.redAdd(x2).redIAdd(x2).redIAdd(a).redMul(dyinv);

    var nx = c.redSqr().redISub(this.x.redAdd(this.x));
    var ny = c.redMul(this.x.redSub(nx)).redISub(this.y);
    return this.curve.point(nx, ny);
};

ShortPoint.prototype.getX = function getX()
{
    return this.x.fromRed();
};

ShortPoint.prototype.getY = function getY()
{
    return this.y.fromRed();
};

ShortPoint.prototype.mul = function mul(k)
{
    k = new BN(k, 16);

    if (this._hasDoubles(k))
        return this.curve._fixedNafMul(this, k);
    else if (this.curve.endo)
        return this.curve._endoWnafMulAdd([this], [k]);
    else
        return this.curve._wnafMul(this, k);
};

ShortPoint.prototype.mulAdd = function mulAdd(k1, p2, k2)
{
    var points = [this, p2];
    var coeffs = [k1, k2];
    if (this.curve.endo)
        return this.curve._endoWnafMulAdd(points, coeffs);
    else
        return this.curve._wnafMulAdd(1, points, coeffs, 2);
};

ShortPoint.prototype.jmulAdd = function jmulAdd(k1, p2, k2)
{
    var points = [this, p2];
    var coeffs = [k1, k2];
    if (this.curve.endo)
        return this.curve._endoWnafMulAdd(points, coeffs, true);
    else
        return this.curve._wnafMulAdd(1, points, coeffs, 2, true);
};

ShortPoint.prototype.eq = function eq(p)
{
    return this === p ||
        this.inf === p.inf &&
        (this.inf || this.x.cmp(p.x) === 0 && this.y.cmp(p.y) === 0);
};

ShortPoint.prototype.neg = function neg(_precompute)
{
    if (this.inf)
        return this;

    var res = this.curve.point(this.x, this.y.redNeg());
    if (_precompute && this.precomputed)
    {
        var pre = this.precomputed;
        var negate = function (p)
        {
            return p.neg();
        };
        res.precomputed = {
            naf: pre.naf && {
                wnd: pre.naf.wnd,
                points: pre.naf.points.map(negate)
            },
            doubles: pre.doubles && {
                step: pre.doubles.step,
                points: pre.doubles.points.map(negate)
            }
        };
    }
    return res;
};

ShortPoint.prototype.toJ = function toJ()
{
    if (this.inf)
        return this.curve.jpoint(null, null, null);

    var res = this.curve.jpoint(this.x, this.y, this.curve.one);
    return res;
};

function JPoint(curve, x, y, z)
{
    Base.BasePoint.call(this, curve, 'jacobian');
    if (x === null && y === null && z === null)
    {
        this.x = this.curve.one;
        this.y = this.curve.one;
        this.z = new BN(0);
    } else
    {
        this.x = new BN(x, 16);
        this.y = new BN(y, 16);
        this.z = new BN(z, 16);
    }
    if (!this.x.red)
        this.x = this.x.toRed(this.curve.red);
    if (!this.y.red)
        this.y = this.y.toRed(this.curve.red);
    if (!this.z.red)
        this.z = this.z.toRed(this.curve.red);

    this.zOne = this.z === this.curve.one;
}
inherits(JPoint, Base.BasePoint);

ShortCurve.prototype.jpoint = function jpoint(x, y, z)
{
    return new JPoint(this, x, y, z);
};

JPoint.prototype.toP = function toP()
{
    if (this.isInfinity())
        return this.curve.point(null, null);

    var zinv = this.z.redInvm();
    var zinv2 = zinv.redSqr();
    var ax = this.x.redMul(zinv2);
    var ay = this.y.redMul(zinv2).redMul(zinv);

    return this.curve.point(ax, ay);
};

JPoint.prototype.neg = function neg()
{
    return this.curve.jpoint(this.x, this.y.redNeg(), this.z);
};

JPoint.prototype.add = function add(p)
{
    // O + P = P
    if (this.isInfinity())
        return p;

    // P + O = P
    if (p.isInfinity())
        return this;

    // 12M + 4S + 7A
    var pz2 = p.z.redSqr();
    var z2 = this.z.redSqr();
    var u1 = this.x.redMul(pz2);
    var u2 = p.x.redMul(z2);
    var s1 = this.y.redMul(pz2.redMul(p.z));
    var s2 = p.y.redMul(z2.redMul(this.z));

    var h = u1.redSub(u2);
    var r = s1.redSub(s2);
    if (h.cmpn(0) === 0)
    {
        if (r.cmpn(0) !== 0)
            return this.curve.jpoint(null, null, null);
        else
            return this.dbl();
    }

    var h2 = h.redSqr();
    var h3 = h2.redMul(h);
    var v = u1.redMul(h2);

    var nx = r.redSqr().redIAdd(h3).redISub(v).redISub(v);
    var ny = r.redMul(v.redISub(nx)).redISub(s1.redMul(h3));
    var nz = this.z.redMul(p.z).redMul(h);

    return this.curve.jpoint(nx, ny, nz);
};

JPoint.prototype.mixedAdd = function mixedAdd(p)
{
    // O + P = P
    if (this.isInfinity())
        return p.toJ();

    // P + O = P
    if (p.isInfinity())
        return this;

    // 8M + 3S + 7A
    var z2 = this.z.redSqr();
    var u1 = this.x;
    var u2 = p.x.redMul(z2);
    var s1 = this.y;
    var s2 = p.y.redMul(z2).redMul(this.z);

    var h = u1.redSub(u2);
    var r = s1.redSub(s2);
    if (h.cmpn(0) === 0)
    {
        if (r.cmpn(0) !== 0)
            return this.curve.jpoint(null, null, null);
        else
            return this.dbl();
    }

    var h2 = h.redSqr();
    var h3 = h2.redMul(h);
    var v = u1.redMul(h2);

    var nx = r.redSqr().redIAdd(h3).redISub(v).redISub(v);
    var ny = r.redMul(v.redISub(nx)).redISub(s1.redMul(h3));
    var nz = this.z.redMul(h);

    return this.curve.jpoint(nx, ny, nz);
};

JPoint.prototype.dblp = function dblp(pow)
{
    if (pow === 0)
        return this;
    if (this.isInfinity())
        return this;
    if (!pow)
        return this.dbl();

    if (this.curve.zeroA || this.curve.threeA)
    {
        var r = this;
        for (var i = 0; i < pow; i++)
            r = r.dbl();
        return r;
    }

    // 1M + 2S + 1A + N * (4S + 5M + 8A)
    // N = 1 => 6M + 6S + 9A
    var a = this.curve.a;
    var tinv = this.curve.tinv;

    var jx = this.x;
    var jy = this.y;
    var jz = this.z;
    var jz4 = jz.redSqr().redSqr();

    // Reuse results
    var jyd = jy.redAdd(jy);
    for (var i = 0; i < pow; i++)
    {
        var jx2 = jx.redSqr();
        var jyd2 = jyd.redSqr();
        var jyd4 = jyd2.redSqr();
        var c = jx2.redAdd(jx2).redIAdd(jx2).redIAdd(a.redMul(jz4));

        var t1 = jx.redMul(jyd2);
        var nx = c.redSqr().redISub(t1.redAdd(t1));
        var t2 = t1.redISub(nx);
        var dny = c.redMul(t2);
        dny = dny.redIAdd(dny).redISub(jyd4);
        var nz = jyd.redMul(jz);
        if (i + 1 < pow)
            jz4 = jz4.redMul(jyd4);

        jx = nx;
        jz = nz;
        jyd = dny;
    }

    return this.curve.jpoint(jx, jyd.redMul(tinv), jz);
};

JPoint.prototype.dbl = function dbl()
{
    if (this.isInfinity())
        return this;

    if (this.curve.zeroA)
        return this._zeroDbl();
    else if (this.curve.threeA)
        return this._threeDbl();
    else
        return this._dbl();
};

JPoint.prototype._zeroDbl = function _zeroDbl()
{
    var nx;
    var ny;
    var nz;
    // Z = 1
    if (this.zOne)
    {
        // hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html
        //     #doubling-mdbl-2007-bl
        // 1M + 5S + 14A

        // XX = X1^2
        var xx = this.x.redSqr();
        // YY = Y1^2
        var yy = this.y.redSqr();
        // YYYY = YY^2
        var yyyy = yy.redSqr();
        // S = 2 * ((X1 + YY)^2 - XX - YYYY)
        var s = this.x.redAdd(yy).redSqr().redISub(xx).redISub(yyyy);
        s = s.redIAdd(s);
        // M = 3 * XX + a; a = 0
        var m = xx.redAdd(xx).redIAdd(xx);
        // T = M ^ 2 - 2*S
        var t = m.redSqr().redISub(s).redISub(s);

        // 8 * YYYY
        var yyyy8 = yyyy.redIAdd(yyyy);
        yyyy8 = yyyy8.redIAdd(yyyy8);
        yyyy8 = yyyy8.redIAdd(yyyy8);

        // X3 = T
        nx = t;
        // Y3 = M * (S - T) - 8 * YYYY
        ny = m.redMul(s.redISub(t)).redISub(yyyy8);
        // Z3 = 2*Y1
        nz = this.y.redAdd(this.y);
    } else
    {
        // hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html
        //     #doubling-dbl-2009-l
        // 2M + 5S + 13A

        // A = X1^2
        var a = this.x.redSqr();
        // B = Y1^2
        var b = this.y.redSqr();
        // C = B^2
        var c = b.redSqr();
        // D = 2 * ((X1 + B)^2 - A - C)
        var d = this.x.redAdd(b).redSqr().redISub(a).redISub(c);
        d = d.redIAdd(d);
        // E = 3 * A
        var e = a.redAdd(a).redIAdd(a);
        // F = E^2
        var f = e.redSqr();

        // 8 * C
        var c8 = c.redIAdd(c);
        c8 = c8.redIAdd(c8);
        c8 = c8.redIAdd(c8);

        // X3 = F - 2 * D
        nx = f.redISub(d).redISub(d);
        // Y3 = E * (D - X3) - 8 * C
        ny = e.redMul(d.redISub(nx)).redISub(c8);
        // Z3 = 2 * Y1 * Z1
        nz = this.y.redMul(this.z);
        nz = nz.redIAdd(nz);
    }

    return this.curve.jpoint(nx, ny, nz);
};

JPoint.prototype._threeDbl = function _threeDbl()
{
    var nx;
    var ny;
    var nz;
    // Z = 1
    if (this.zOne)
    {
        // hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-3.html
        //     #doubling-mdbl-2007-bl
        // 1M + 5S + 15A

        // XX = X1^2
        var xx = this.x.redSqr();
        // YY = Y1^2
        var yy = this.y.redSqr();
        // YYYY = YY^2
        var yyyy = yy.redSqr();
        // S = 2 * ((X1 + YY)^2 - XX - YYYY)
        var s = this.x.redAdd(yy).redSqr().redISub(xx).redISub(yyyy);
        s = s.redIAdd(s);
        // M = 3 * XX + a
        var m = xx.redAdd(xx).redIAdd(xx).redIAdd(this.curve.a);
        // T = M^2 - 2 * S
        var t = m.redSqr().redISub(s).redISub(s);
        // X3 = T
        nx = t;
        // Y3 = M * (S - T) - 8 * YYYY
        var yyyy8 = yyyy.redIAdd(yyyy);
        yyyy8 = yyyy8.redIAdd(yyyy8);
        yyyy8 = yyyy8.redIAdd(yyyy8);
        ny = m.redMul(s.redISub(t)).redISub(yyyy8);
        // Z3 = 2 * Y1
        nz = this.y.redAdd(this.y);
    } else
    {
        // hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-3.html#doubling-dbl-2001-b
        // 3M + 5S

        // delta = Z1^2
        var delta = this.z.redSqr();
        // gamma = Y1^2
        var gamma = this.y.redSqr();
        // beta = X1 * gamma
        var beta = this.x.redMul(gamma);
        // alpha = 3 * (X1 - delta) * (X1 + delta)
        var alpha = this.x.redSub(delta).redMul(this.x.redAdd(delta));
        alpha = alpha.redAdd(alpha).redIAdd(alpha);
        // X3 = alpha^2 - 8 * beta
        var beta4 = beta.redIAdd(beta);
        beta4 = beta4.redIAdd(beta4);
        var beta8 = beta4.redAdd(beta4);
        nx = alpha.redSqr().redISub(beta8);
        // Z3 = (Y1 + Z1)^2 - gamma - delta
        nz = this.y.redAdd(this.z).redSqr().redISub(gamma).redISub(delta);
        // Y3 = alpha * (4 * beta - X3) - 8 * gamma^2
        var ggamma8 = gamma.redSqr();
        ggamma8 = ggamma8.redIAdd(ggamma8);
        ggamma8 = ggamma8.redIAdd(ggamma8);
        ggamma8 = ggamma8.redIAdd(ggamma8);
        ny = alpha.redMul(beta4.redISub(nx)).redISub(ggamma8);
    }

    return this.curve.jpoint(nx, ny, nz);
};

JPoint.prototype._dbl = function _dbl()
{
    var a = this.curve.a;

    // 4M + 6S + 10A
    var jx = this.x;
    var jy = this.y;
    var jz = this.z;
    var jz4 = jz.redSqr().redSqr();

    var jx2 = jx.redSqr();
    var jy2 = jy.redSqr();

    var c = jx2.redAdd(jx2).redIAdd(jx2).redIAdd(a.redMul(jz4));

    var jxd4 = jx.redAdd(jx);
    jxd4 = jxd4.redIAdd(jxd4);
    var t1 = jxd4.redMul(jy2);
    var nx = c.redSqr().redISub(t1.redAdd(t1));
    var t2 = t1.redISub(nx);

    var jyd8 = jy2.redSqr();
    jyd8 = jyd8.redIAdd(jyd8);
    jyd8 = jyd8.redIAdd(jyd8);
    jyd8 = jyd8.redIAdd(jyd8);
    var ny = c.redMul(t2).redISub(jyd8);
    var nz = jy.redAdd(jy).redMul(jz);

    return this.curve.jpoint(nx, ny, nz);
};

JPoint.prototype.trpl = function trpl()
{
    if (!this.curve.zeroA)
        return this.dbl().add(this);

    // hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-0.html#tripling-tpl-2007-bl
    // 5M + 10S + ...

    // XX = X1^2
    var xx = this.x.redSqr();
    // YY = Y1^2
    var yy = this.y.redSqr();
    // ZZ = Z1^2
    var zz = this.z.redSqr();
    // YYYY = YY^2
    var yyyy = yy.redSqr();
    // M = 3 * XX + a * ZZ2; a = 0
    var m = xx.redAdd(xx).redIAdd(xx);
    // MM = M^2
    var mm = m.redSqr();
    // E = 6 * ((X1 + YY)^2 - XX - YYYY) - MM
    var e = this.x.redAdd(yy).redSqr().redISub(xx).redISub(yyyy);
    e = e.redIAdd(e);
    e = e.redAdd(e).redIAdd(e);
    e = e.redISub(mm);
    // EE = E^2
    var ee = e.redSqr();
    // T = 16*YYYY
    var t = yyyy.redIAdd(yyyy);
    t = t.redIAdd(t);
    t = t.redIAdd(t);
    t = t.redIAdd(t);
    // U = (M + E)^2 - MM - EE - T
    var u = m.redIAdd(e).redSqr().redISub(mm).redISub(ee).redISub(t);
    // X3 = 4 * (X1 * EE - 4 * YY * U)
    var yyu4 = yy.redMul(u);
    yyu4 = yyu4.redIAdd(yyu4);
    yyu4 = yyu4.redIAdd(yyu4);
    var nx = this.x.redMul(ee).redISub(yyu4);
    nx = nx.redIAdd(nx);
    nx = nx.redIAdd(nx);
    // Y3 = 8 * Y1 * (U * (T - U) - E * EE)
    var ny = this.y.redMul(u.redMul(t.redISub(u)).redISub(e.redMul(ee)));
    ny = ny.redIAdd(ny);
    ny = ny.redIAdd(ny);
    ny = ny.redIAdd(ny);
    // Z3 = (Z1 + E)^2 - ZZ - EE
    var nz = this.z.redAdd(e).redSqr().redISub(zz).redISub(ee);

    return this.curve.jpoint(nx, ny, nz);
};

JPoint.prototype.mul = function mul(k, kbase)
{
    k = new BN(k, kbase);

    return this.curve._wnafMul(this, k);
};

JPoint.prototype.eq = function eq(p)
{
    if (p.type === 'affine')
        return this.eq(p.toJ());

    if (this === p)
        return true;

    // x1 * z2^2 == x2 * z1^2
    var z2 = this.z.redSqr();
    var pz2 = p.z.redSqr();
    if (this.x.redMul(pz2).redISub(p.x.redMul(z2)).cmpn(0) !== 0)
        return false;

    // y1 * z2^3 == y2 * z1^3
    var z3 = z2.redMul(this.z);
    var pz3 = pz2.redMul(p.z);
    return this.y.redMul(pz3).redISub(p.y.redMul(z3)).cmpn(0) === 0;
};

JPoint.prototype.eqXToP = function eqXToP(x)
{
    var zs = this.z.redSqr();
    var rx = x.toRed(this.curve.red).redMul(zs);
    if (this.x.cmp(rx) === 0)
        return true;

    var xc = x.clone();
    var t = this.curve.redN.redMul(zs);
    for (; ;)
    {
        xc.iadd(this.curve.n);
        if (xc.cmp(this.curve.p) >= 0)
            return false;

        rx.redIAdd(t);
        if (this.x.cmp(rx) === 0)
            return true;
    }
    return false;
};

JPoint.prototype.inspect = function inspect()
{
    if (this.isInfinity())
        return '<EC JPoint Infinity>';
    return '<EC JPoint x: ' + this.x.toString(16, 2) +
        ' y: ' + this.y.toString(16, 2) +
        ' z: ' + this.z.toString(16, 2) + '>';
};

JPoint.prototype.isInfinity = function isInfinity()
{
    // XXX This code assumes that zero is always zero in red
    return this.z.cmpn(0) === 0;
};
// elliptic/curve/short end----------------------------------------------------------------------

// elliptic/ec/key begin----------------------------------------------------------------------
//var BN = require('bn.js');
//var elliptic = require('../../elliptic');
//var elliptic_utils = elliptic.utils;
//var ix_base.assert = elliptic_utils.assert;

function ec_KeyPair(ec, options)
{
    this.ec = ec;
    this.priv = null;
    this.pub = null;

    // KeyPair(ec, { priv: ..., pub: ... })
    if (options.priv)
        this._importPrivate(options.priv, options.privEnc);
    if (options.pub)
        this._importPublic(options.pub, options.pubEnc);
}
//module.exports = KeyPair;

ec_KeyPair.fromPublic = function fromPublic(ec, pub, enc)
{
    if (pub instanceof ec_KeyPair)
        return pub;

    return new ec_KeyPair(ec, {
        pub: pub,
        pubEnc: enc
    });
};

ec_KeyPair.fromPrivate = function fromPrivate(ec, priv, enc)
{
    if (priv instanceof ec_KeyPair)
        return priv;

    return new ec_KeyPair(ec, {
        priv: priv,
        privEnc: enc
    });
};

ec_KeyPair.prototype.validate = function validate()
{
    var pub = this.getPublic();

    if (pub.isInfinity())
        return { result: false, reason: 'Invalid public key' };
    if (!pub.validate())
        return { result: false, reason: 'Public key is not a point' };
    if (!pub.mul(this.ec.curve.n).isInfinity())
        return { result: false, reason: 'Public key * N != O' };

    return { result: true, reason: null };
};

ec_KeyPair.prototype.getPublic = function getPublic(compact, enc)
{
    // compact is optional argument
    if (typeof compact === 'string')
    {
        enc = compact;
        compact = null;
    }

    if (!this.pub)
        this.pub = this.ec.g.mul(this.priv);

    if (!enc)
        return this.pub;

    return this.pub.encode(enc, compact);
};

ec_KeyPair.prototype.getPrivate = function getPrivate(enc)
{
    if (enc === 'hex')
        return this.priv.toString(16, 2);
    else
        return this.priv;
};

ec_KeyPair.prototype._importPrivate = function _importPrivate(key, enc)
{
    this.priv = new BN(key, enc || 16);

    // Ensure that the priv won't be bigger than n, otherwise we may fail
    // in fixed multiplication method
    this.priv = this.priv.umod(this.ec.curve.n);
};

ec_KeyPair.prototype._importPublic = function _importPublic(key, enc)
{
    if (key.x || key.y)
    {
        // Montgomery points only have an `x` coordinate.
        // Weierstrass/Edwards points on the other hand have both `x` and
        // `y` coordinates.
        if (this.ec.curve.type === 'mont')
        {
            ix_base.assert(key.x, 'Need x coordinate');
        } else if (this.ec.curve.type === 'short' ||
            this.ec.curve.type === 'edwards')
        {
            ix_base.assert(key.x && key.y, 'Need both x and y coordinate');
        }
        this.pub = this.ec.curve.point(key.x, key.y);
        return;
    }
    this.pub = this.ec.curve.decodePoint(key, enc);
};

// ECDH
ec_KeyPair.prototype.derive = function derive(pub)
{
    return pub.mul(this.priv).getX();
};

// ECDSA
ec_KeyPair.prototype.sign = function sign(msg, enc, options)
{
    return this.ec.sign(msg, this, enc, options);
};

ec_KeyPair.prototype.verify = function verify(msg, signature)
{
    return this.ec.verify(msg, signature, this);
};

ec_KeyPair.prototype.inspect = function inspect()
{
    return '<Key priv: ' + (this.priv && this.priv.toString(16, 2)) +
        ' pub: ' + (this.pub && this.pub.inspect()) + ' >';
};
// elliptic/ec/key end----------------------------------------------------------------------

// elliptic/ec/signature begin----------------------------------------------------------------------
//var BN = require('bn.js');

//var elliptic = require('../../elliptic');
//var elliptic_utils = elliptic.utils;
//var assert = elliptic_utils.assert;

function ec_Signature(options, enc)
{
    if (options instanceof ec_Signature)
        return options;

    if (this._importDER(options, enc))
        return;

    ix_base.assert(options.r && options.s, 'Signature without r or s');
    this.r = new BN(options.r, 16);
    this.s = new BN(options.s, 16);
    if (options.recoveryParam === undefined)
        this.recoveryParam = null;
    else
        this.recoveryParam = options.recoveryParam;
}
//module.exports = Signature;

function Position()
{
    this.place = 0;
}

function getLength(buf, p)
{
    var initial = buf[p.place++];
    if (!(initial & 0x80))
    {
        return initial;
    }
    var octetLen = initial & 0xf;
    var val = 0;
    for (var i = 0, off = p.place; i < octetLen; i++ , off++)
    {
        val <<= 8;
        val |= buf[off];
    }
    p.place = off;
    return val;
}

function rmPadding(buf)
{
    var i = 0;
    var len = buf.length - 1;
    while (!buf[i] && !(buf[i + 1] & 0x80) && i < len)
    {
        i++;
    }
    if (i === 0)
    {
        return buf;
    }
    return buf.slice(i);
}

ec_Signature.prototype._importDER = function _importDER(data, enc)
{
    data = elliptic_utils.toArray(data, enc);
    var p = new Position();
    if (data[p.place++] !== 0x30)
    {
        return false;
    }
    var len = getLength(data, p);
    if ((len + p.place) !== data.length)
    {
        return false;
    }
    if (data[p.place++] !== 0x02)
    {
        return false;
    }
    var rlen = getLength(data, p);
    var r = data.slice(p.place, rlen + p.place);
    p.place += rlen;
    if (data[p.place++] !== 0x02)
    {
        return false;
    }
    var slen = getLength(data, p);
    if (data.length !== slen + p.place)
    {
        return false;
    }
    var s = data.slice(p.place, slen + p.place);
    if (r[0] === 0 && (r[1] & 0x80))
    {
        r = r.slice(1);
    }
    if (s[0] === 0 && (s[1] & 0x80))
    {
        s = s.slice(1);
    }

    this.r = new BN(r);
    this.s = new BN(s);
    this.recoveryParam = null;

    return true;
};

function constructLength(arr, len)
{
    if (len < 0x80)
    {
        arr.push(len);
        return;
    }
    var octets = 1 + (Math.log(len) / Math.LN2 >>> 3);
    arr.push(octets | 0x80);
    while (--octets)
    {
        arr.push((len >>> (octets << 3)) & 0xff);
    }
    arr.push(len);
}

ec_Signature.prototype.toDER = function toDER(enc)
{
    var r = this.r.toArray();
    var s = this.s.toArray();

    // Pad values
    if (r[0] & 0x80)
        r = [0].concat(r);
    // Pad values
    if (s[0] & 0x80)
        s = [0].concat(s);

    r = rmPadding(r);
    s = rmPadding(s);

    while (!s[0] && !(s[1] & 0x80))
    {
        s = s.slice(1);
    }
    var arr = [0x02];
    constructLength(arr, r.length);
    arr = arr.concat(r);
    arr.push(0x02);
    constructLength(arr, s.length);
    var backHalf = arr.concat(s);
    var res = [0x30];
    constructLength(res, backHalf.length);
    res = res.concat(backHalf);
    return elliptic_utils.encode(res, enc);
};
// elliptic/ec/signature end----------------------------------------------------------------------

// elliptic/precomputed/secp256k1 begin----------------------------------------------------------------------
var secp256k1_pre = {
    doubles: {
        step: 4,
        points: [
            [
                'e60fce93b59e9ec53011aabc21c23e97b2a31369b87a5ae9c44ee89e2a6dec0a',
                'f7e3507399e595929db99f34f57937101296891e44d23f0be1f32cce69616821'
            ],
            [
                '8282263212c609d9ea2a6e3e172de238d8c39cabd5ac1ca10646e23fd5f51508',
                '11f8a8098557dfe45e8256e830b60ace62d613ac2f7b17bed31b6eaff6e26caf'
            ],
            [
                '175e159f728b865a72f99cc6c6fc846de0b93833fd2222ed73fce5b551e5b739',
                'd3506e0d9e3c79eba4ef97a51ff71f5eacb5955add24345c6efa6ffee9fed695'
            ],
            [
                '363d90d447b00c9c99ceac05b6262ee053441c7e55552ffe526bad8f83ff4640',
                '4e273adfc732221953b445397f3363145b9a89008199ecb62003c7f3bee9de9'
            ],
            [
                '8b4b5f165df3c2be8c6244b5b745638843e4a781a15bcd1b69f79a55dffdf80c',
                '4aad0a6f68d308b4b3fbd7813ab0da04f9e336546162ee56b3eff0c65fd4fd36'
            ],
            [
                '723cbaa6e5db996d6bf771c00bd548c7b700dbffa6c0e77bcb6115925232fcda',
                '96e867b5595cc498a921137488824d6e2660a0653779494801dc069d9eb39f5f'
            ],
            [
                'eebfa4d493bebf98ba5feec812c2d3b50947961237a919839a533eca0e7dd7fa',
                '5d9a8ca3970ef0f269ee7edaf178089d9ae4cdc3a711f712ddfd4fdae1de8999'
            ],
            [
                '100f44da696e71672791d0a09b7bde459f1215a29b3c03bfefd7835b39a48db0',
                'cdd9e13192a00b772ec8f3300c090666b7ff4a18ff5195ac0fbd5cd62bc65a09'
            ],
            [
                'e1031be262c7ed1b1dc9227a4a04c017a77f8d4464f3b3852c8acde6e534fd2d',
                '9d7061928940405e6bb6a4176597535af292dd419e1ced79a44f18f29456a00d'
            ],
            [
                'feea6cae46d55b530ac2839f143bd7ec5cf8b266a41d6af52d5e688d9094696d',
                'e57c6b6c97dce1bab06e4e12bf3ecd5c981c8957cc41442d3155debf18090088'
            ],
            [
                'da67a91d91049cdcb367be4be6ffca3cfeed657d808583de33fa978bc1ec6cb1',
                '9bacaa35481642bc41f463f7ec9780e5dec7adc508f740a17e9ea8e27a68be1d'
            ],
            [
                '53904faa0b334cdda6e000935ef22151ec08d0f7bb11069f57545ccc1a37b7c0',
                '5bc087d0bc80106d88c9eccac20d3c1c13999981e14434699dcb096b022771c8'
            ],
            [
                '8e7bcd0bd35983a7719cca7764ca906779b53a043a9b8bcaeff959f43ad86047',
                '10b7770b2a3da4b3940310420ca9514579e88e2e47fd68b3ea10047e8460372a'
            ],
            [
                '385eed34c1cdff21e6d0818689b81bde71a7f4f18397e6690a841e1599c43862',
                '283bebc3e8ea23f56701de19e9ebf4576b304eec2086dc8cc0458fe5542e5453'
            ],
            [
                '6f9d9b803ecf191637c73a4413dfa180fddf84a5947fbc9c606ed86c3fac3a7',
                '7c80c68e603059ba69b8e2a30e45c4d47ea4dd2f5c281002d86890603a842160'
            ],
            [
                '3322d401243c4e2582a2147c104d6ecbf774d163db0f5e5313b7e0e742d0e6bd',
                '56e70797e9664ef5bfb019bc4ddaf9b72805f63ea2873af624f3a2e96c28b2a0'
            ],
            [
                '85672c7d2de0b7da2bd1770d89665868741b3f9af7643397721d74d28134ab83',
                '7c481b9b5b43b2eb6374049bfa62c2e5e77f17fcc5298f44c8e3094f790313a6'
            ],
            [
                '948bf809b1988a46b06c9f1919413b10f9226c60f668832ffd959af60c82a0a',
                '53a562856dcb6646dc6b74c5d1c3418c6d4dff08c97cd2bed4cb7f88d8c8e589'
            ],
            [
                '6260ce7f461801c34f067ce0f02873a8f1b0e44dfc69752accecd819f38fd8e8',
                'bc2da82b6fa5b571a7f09049776a1ef7ecd292238051c198c1a84e95b2b4ae17'
            ],
            [
                'e5037de0afc1d8d43d8348414bbf4103043ec8f575bfdc432953cc8d2037fa2d',
                '4571534baa94d3b5f9f98d09fb990bddbd5f5b03ec481f10e0e5dc841d755bda'
            ],
            [
                'e06372b0f4a207adf5ea905e8f1771b4e7e8dbd1c6a6c5b725866a0ae4fce725',
                '7a908974bce18cfe12a27bb2ad5a488cd7484a7787104870b27034f94eee31dd'
            ],
            [
                '213c7a715cd5d45358d0bbf9dc0ce02204b10bdde2a3f58540ad6908d0559754',
                '4b6dad0b5ae462507013ad06245ba190bb4850f5f36a7eeddff2c27534b458f2'
            ],
            [
                '4e7c272a7af4b34e8dbb9352a5419a87e2838c70adc62cddf0cc3a3b08fbd53c',
                '17749c766c9d0b18e16fd09f6def681b530b9614bff7dd33e0b3941817dcaae6'
            ],
            [
                'fea74e3dbe778b1b10f238ad61686aa5c76e3db2be43057632427e2840fb27b6',
                '6e0568db9b0b13297cf674deccb6af93126b596b973f7b77701d3db7f23cb96f'
            ],
            [
                '76e64113f677cf0e10a2570d599968d31544e179b760432952c02a4417bdde39',
                'c90ddf8dee4e95cf577066d70681f0d35e2a33d2b56d2032b4b1752d1901ac01'
            ],
            [
                'c738c56b03b2abe1e8281baa743f8f9a8f7cc643df26cbee3ab150242bcbb891',
                '893fb578951ad2537f718f2eacbfbbbb82314eef7880cfe917e735d9699a84c3'
            ],
            [
                'd895626548b65b81e264c7637c972877d1d72e5f3a925014372e9f6588f6c14b',
                'febfaa38f2bc7eae728ec60818c340eb03428d632bb067e179363ed75d7d991f'
            ],
            [
                'b8da94032a957518eb0f6433571e8761ceffc73693e84edd49150a564f676e03',
                '2804dfa44805a1e4d7c99cc9762808b092cc584d95ff3b511488e4e74efdf6e7'
            ],
            [
                'e80fea14441fb33a7d8adab9475d7fab2019effb5156a792f1a11778e3c0df5d',
                'eed1de7f638e00771e89768ca3ca94472d155e80af322ea9fcb4291b6ac9ec78'
            ],
            [
                'a301697bdfcd704313ba48e51d567543f2a182031efd6915ddc07bbcc4e16070',
                '7370f91cfb67e4f5081809fa25d40f9b1735dbf7c0a11a130c0d1a041e177ea1'
            ],
            [
                '90ad85b389d6b936463f9d0512678de208cc330b11307fffab7ac63e3fb04ed4',
                'e507a3620a38261affdcbd9427222b839aefabe1582894d991d4d48cb6ef150'
            ],
            [
                '8f68b9d2f63b5f339239c1ad981f162ee88c5678723ea3351b7b444c9ec4c0da',
                '662a9f2dba063986de1d90c2b6be215dbbea2cfe95510bfdf23cbf79501fff82'
            ],
            [
                'e4f3fb0176af85d65ff99ff9198c36091f48e86503681e3e6686fd5053231e11',
                '1e63633ad0ef4f1c1661a6d0ea02b7286cc7e74ec951d1c9822c38576feb73bc'
            ],
            [
                '8c00fa9b18ebf331eb961537a45a4266c7034f2f0d4e1d0716fb6eae20eae29e',
                'efa47267fea521a1a9dc343a3736c974c2fadafa81e36c54e7d2a4c66702414b'
            ],
            [
                'e7a26ce69dd4829f3e10cec0a9e98ed3143d084f308b92c0997fddfc60cb3e41',
                '2a758e300fa7984b471b006a1aafbb18d0a6b2c0420e83e20e8a9421cf2cfd51'
            ],
            [
                'b6459e0ee3662ec8d23540c223bcbdc571cbcb967d79424f3cf29eb3de6b80ef',
                '67c876d06f3e06de1dadf16e5661db3c4b3ae6d48e35b2ff30bf0b61a71ba45'
            ],
            [
                'd68a80c8280bb840793234aa118f06231d6f1fc67e73c5a5deda0f5b496943e8',
                'db8ba9fff4b586d00c4b1f9177b0e28b5b0e7b8f7845295a294c84266b133120'
            ],
            [
                '324aed7df65c804252dc0270907a30b09612aeb973449cea4095980fc28d3d5d',
                '648a365774b61f2ff130c0c35aec1f4f19213b0c7e332843967224af96ab7c84'
            ],
            [
                '4df9c14919cde61f6d51dfdbe5fee5dceec4143ba8d1ca888e8bd373fd054c96',
                '35ec51092d8728050974c23a1d85d4b5d506cdc288490192ebac06cad10d5d'
            ],
            [
                '9c3919a84a474870faed8a9c1cc66021523489054d7f0308cbfc99c8ac1f98cd',
                'ddb84f0f4a4ddd57584f044bf260e641905326f76c64c8e6be7e5e03d4fc599d'
            ],
            [
                '6057170b1dd12fdf8de05f281d8e06bb91e1493a8b91d4cc5a21382120a959e5',
                '9a1af0b26a6a4807add9a2daf71df262465152bc3ee24c65e899be932385a2a8'
            ],
            [
                'a576df8e23a08411421439a4518da31880cef0fba7d4df12b1a6973eecb94266',
                '40a6bf20e76640b2c92b97afe58cd82c432e10a7f514d9f3ee8be11ae1b28ec8'
            ],
            [
                '7778a78c28dec3e30a05fe9629de8c38bb30d1f5cf9a3a208f763889be58ad71',
                '34626d9ab5a5b22ff7098e12f2ff580087b38411ff24ac563b513fc1fd9f43ac'
            ],
            [
                '928955ee637a84463729fd30e7afd2ed5f96274e5ad7e5cb09eda9c06d903ac',
                'c25621003d3f42a827b78a13093a95eeac3d26efa8a8d83fc5180e935bcd091f'
            ],
            [
                '85d0fef3ec6db109399064f3a0e3b2855645b4a907ad354527aae75163d82751',
                '1f03648413a38c0be29d496e582cf5663e8751e96877331582c237a24eb1f962'
            ],
            [
                'ff2b0dce97eece97c1c9b6041798b85dfdfb6d8882da20308f5404824526087e',
                '493d13fef524ba188af4c4dc54d07936c7b7ed6fb90e2ceb2c951e01f0c29907'
            ],
            [
                '827fbbe4b1e880ea9ed2b2e6301b212b57f1ee148cd6dd28780e5e2cf856e241',
                'c60f9c923c727b0b71bef2c67d1d12687ff7a63186903166d605b68baec293ec'
            ],
            [
                'eaa649f21f51bdbae7be4ae34ce6e5217a58fdce7f47f9aa7f3b58fa2120e2b3',
                'be3279ed5bbbb03ac69a80f89879aa5a01a6b965f13f7e59d47a5305ba5ad93d'
            ],
            [
                'e4a42d43c5cf169d9391df6decf42ee541b6d8f0c9a137401e23632dda34d24f',
                '4d9f92e716d1c73526fc99ccfb8ad34ce886eedfa8d8e4f13a7f7131deba9414'
            ],
            [
                '1ec80fef360cbdd954160fadab352b6b92b53576a88fea4947173b9d4300bf19',
                'aeefe93756b5340d2f3a4958a7abbf5e0146e77f6295a07b671cdc1cc107cefd'
            ],
            [
                '146a778c04670c2f91b00af4680dfa8bce3490717d58ba889ddb5928366642be',
                'b318e0ec3354028add669827f9d4b2870aaa971d2f7e5ed1d0b297483d83efd0'
            ],
            [
                'fa50c0f61d22e5f07e3acebb1aa07b128d0012209a28b9776d76a8793180eef9',
                '6b84c6922397eba9b72cd2872281a68a5e683293a57a213b38cd8d7d3f4f2811'
            ],
            [
                'da1d61d0ca721a11b1a5bf6b7d88e8421a288ab5d5bba5220e53d32b5f067ec2',
                '8157f55a7c99306c79c0766161c91e2966a73899d279b48a655fba0f1ad836f1'
            ],
            [
                'a8e282ff0c9706907215ff98e8fd416615311de0446f1e062a73b0610d064e13',
                '7f97355b8db81c09abfb7f3c5b2515888b679a3e50dd6bd6cef7c73111f4cc0c'
            ],
            [
                '174a53b9c9a285872d39e56e6913cab15d59b1fa512508c022f382de8319497c',
                'ccc9dc37abfc9c1657b4155f2c47f9e6646b3a1d8cb9854383da13ac079afa73'
            ],
            [
                '959396981943785c3d3e57edf5018cdbe039e730e4918b3d884fdff09475b7ba',
                '2e7e552888c331dd8ba0386a4b9cd6849c653f64c8709385e9b8abf87524f2fd'
            ],
            [
                'd2a63a50ae401e56d645a1153b109a8fcca0a43d561fba2dbb51340c9d82b151',
                'e82d86fb6443fcb7565aee58b2948220a70f750af484ca52d4142174dcf89405'
            ],
            [
                '64587e2335471eb890ee7896d7cfdc866bacbdbd3839317b3436f9b45617e073',
                'd99fcdd5bf6902e2ae96dd6447c299a185b90a39133aeab358299e5e9faf6589'
            ],
            [
                '8481bde0e4e4d885b3a546d3e549de042f0aa6cea250e7fd358d6c86dd45e458',
                '38ee7b8cba5404dd84a25bf39cecb2ca900a79c42b262e556d64b1b59779057e'
            ],
            [
                '13464a57a78102aa62b6979ae817f4637ffcfed3c4b1ce30bcd6303f6caf666b',
                '69be159004614580ef7e433453ccb0ca48f300a81d0942e13f495a907f6ecc27'
            ],
            [
                'bc4a9df5b713fe2e9aef430bcc1dc97a0cd9ccede2f28588cada3a0d2d83f366',
                'd3a81ca6e785c06383937adf4b798caa6e8a9fbfa547b16d758d666581f33c1'
            ],
            [
                '8c28a97bf8298bc0d23d8c749452a32e694b65e30a9472a3954ab30fe5324caa',
                '40a30463a3305193378fedf31f7cc0eb7ae784f0451cb9459e71dc73cbef9482'
            ],
            [
                '8ea9666139527a8c1dd94ce4f071fd23c8b350c5a4bb33748c4ba111faccae0',
                '620efabbc8ee2782e24e7c0cfb95c5d735b783be9cf0f8e955af34a30e62b945'
            ],
            [
                'dd3625faef5ba06074669716bbd3788d89bdde815959968092f76cc4eb9a9787',
                '7a188fa3520e30d461da2501045731ca941461982883395937f68d00c644a573'
            ],
            [
                'f710d79d9eb962297e4f6232b40e8f7feb2bc63814614d692c12de752408221e',
                'ea98e67232d3b3295d3b535532115ccac8612c721851617526ae47a9c77bfc82'
            ]
        ]
    },
    naf: {
        wnd: 7,
        points: [
            [
                'f9308a019258c31049344f85f89d5229b531c845836f99b08601f113bce036f9',
                '388f7b0f632de8140fe337e62a37f3566500a99934c2231b6cb9fd7584b8e672'
            ],
            [
                '2f8bde4d1a07209355b4a7250a5c5128e88b84bddc619ab7cba8d569b240efe4',
                'd8ac222636e5e3d6d4dba9dda6c9c426f788271bab0d6840dca87d3aa6ac62d6'
            ],
            [
                '5cbdf0646e5db4eaa398f365f2ea7a0e3d419b7e0330e39ce92bddedcac4f9bc',
                '6aebca40ba255960a3178d6d861a54dba813d0b813fde7b5a5082628087264da'
            ],
            [
                'acd484e2f0c7f65309ad178a9f559abde09796974c57e714c35f110dfc27ccbe',
                'cc338921b0a7d9fd64380971763b61e9add888a4375f8e0f05cc262ac64f9c37'
            ],
            [
                '774ae7f858a9411e5ef4246b70c65aac5649980be5c17891bbec17895da008cb',
                'd984a032eb6b5e190243dd56d7b7b365372db1e2dff9d6a8301d74c9c953c61b'
            ],
            [
                'f28773c2d975288bc7d1d205c3748651b075fbc6610e58cddeeddf8f19405aa8',
                'ab0902e8d880a89758212eb65cdaf473a1a06da521fa91f29b5cb52db03ed81'
            ],
            [
                'd7924d4f7d43ea965a465ae3095ff41131e5946f3c85f79e44adbcf8e27e080e',
                '581e2872a86c72a683842ec228cc6defea40af2bd896d3a5c504dc9ff6a26b58'
            ],
            [
                'defdea4cdb677750a420fee807eacf21eb9898ae79b9768766e4faa04a2d4a34',
                '4211ab0694635168e997b0ead2a93daeced1f4a04a95c0f6cfb199f69e56eb77'
            ],
            [
                '2b4ea0a797a443d293ef5cff444f4979f06acfebd7e86d277475656138385b6c',
                '85e89bc037945d93b343083b5a1c86131a01f60c50269763b570c854e5c09b7a'
            ],
            [
                '352bbf4a4cdd12564f93fa332ce333301d9ad40271f8107181340aef25be59d5',
                '321eb4075348f534d59c18259dda3e1f4a1b3b2e71b1039c67bd3d8bcf81998c'
            ],
            [
                '2fa2104d6b38d11b0230010559879124e42ab8dfeff5ff29dc9cdadd4ecacc3f',
                '2de1068295dd865b64569335bd5dd80181d70ecfc882648423ba76b532b7d67'
            ],
            [
                '9248279b09b4d68dab21a9b066edda83263c3d84e09572e269ca0cd7f5453714',
                '73016f7bf234aade5d1aa71bdea2b1ff3fc0de2a887912ffe54a32ce97cb3402'
            ],
            [
                'daed4f2be3a8bf278e70132fb0beb7522f570e144bf615c07e996d443dee8729',
                'a69dce4a7d6c98e8d4a1aca87ef8d7003f83c230f3afa726ab40e52290be1c55'
            ],
            [
                'c44d12c7065d812e8acf28d7cbb19f9011ecd9e9fdf281b0e6a3b5e87d22e7db',
                '2119a460ce326cdc76c45926c982fdac0e106e861edf61c5a039063f0e0e6482'
            ],
            [
                '6a245bf6dc698504c89a20cfded60853152b695336c28063b61c65cbd269e6b4',
                'e022cf42c2bd4a708b3f5126f16a24ad8b33ba48d0423b6efd5e6348100d8a82'
            ],
            [
                '1697ffa6fd9de627c077e3d2fe541084ce13300b0bec1146f95ae57f0d0bd6a5',
                'b9c398f186806f5d27561506e4557433a2cf15009e498ae7adee9d63d01b2396'
            ],
            [
                '605bdb019981718b986d0f07e834cb0d9deb8360ffb7f61df982345ef27a7479',
                '2972d2de4f8d20681a78d93ec96fe23c26bfae84fb14db43b01e1e9056b8c49'
            ],
            [
                '62d14dab4150bf497402fdc45a215e10dcb01c354959b10cfe31c7e9d87ff33d',
                '80fc06bd8cc5b01098088a1950eed0db01aa132967ab472235f5642483b25eaf'
            ],
            [
                '80c60ad0040f27dade5b4b06c408e56b2c50e9f56b9b8b425e555c2f86308b6f',
                '1c38303f1cc5c30f26e66bad7fe72f70a65eed4cbe7024eb1aa01f56430bd57a'
            ],
            [
                '7a9375ad6167ad54aa74c6348cc54d344cc5dc9487d847049d5eabb0fa03c8fb',
                'd0e3fa9eca8726909559e0d79269046bdc59ea10c70ce2b02d499ec224dc7f7'
            ],
            [
                'd528ecd9b696b54c907a9ed045447a79bb408ec39b68df504bb51f459bc3ffc9',
                'eecf41253136e5f99966f21881fd656ebc4345405c520dbc063465b521409933'
            ],
            [
                '49370a4b5f43412ea25f514e8ecdad05266115e4a7ecb1387231808f8b45963',
                '758f3f41afd6ed428b3081b0512fd62a54c3f3afbb5b6764b653052a12949c9a'
            ],
            [
                '77f230936ee88cbbd73df930d64702ef881d811e0e1498e2f1c13eb1fc345d74',
                '958ef42a7886b6400a08266e9ba1b37896c95330d97077cbbe8eb3c7671c60d6'
            ],
            [
                'f2dac991cc4ce4b9ea44887e5c7c0bce58c80074ab9d4dbaeb28531b7739f530',
                'e0dedc9b3b2f8dad4da1f32dec2531df9eb5fbeb0598e4fd1a117dba703a3c37'
            ],
            [
                '463b3d9f662621fb1b4be8fbbe2520125a216cdfc9dae3debcba4850c690d45b',
                '5ed430d78c296c3543114306dd8622d7c622e27c970a1de31cb377b01af7307e'
            ],
            [
                'f16f804244e46e2a09232d4aff3b59976b98fac14328a2d1a32496b49998f247',
                'cedabd9b82203f7e13d206fcdf4e33d92a6c53c26e5cce26d6579962c4e31df6'
            ],
            [
                'caf754272dc84563b0352b7a14311af55d245315ace27c65369e15f7151d41d1',
                'cb474660ef35f5f2a41b643fa5e460575f4fa9b7962232a5c32f908318a04476'
            ],
            [
                '2600ca4b282cb986f85d0f1709979d8b44a09c07cb86d7c124497bc86f082120',
                '4119b88753c15bd6a693b03fcddbb45d5ac6be74ab5f0ef44b0be9475a7e4b40'
            ],
            [
                '7635ca72d7e8432c338ec53cd12220bc01c48685e24f7dc8c602a7746998e435',
                '91b649609489d613d1d5e590f78e6d74ecfc061d57048bad9e76f302c5b9c61'
            ],
            [
                '754e3239f325570cdbbf4a87deee8a66b7f2b33479d468fbc1a50743bf56cc18',
                '673fb86e5bda30fb3cd0ed304ea49a023ee33d0197a695d0c5d98093c536683'
            ],
            [
                'e3e6bd1071a1e96aff57859c82d570f0330800661d1c952f9fe2694691d9b9e8',
                '59c9e0bba394e76f40c0aa58379a3cb6a5a2283993e90c4167002af4920e37f5'
            ],
            [
                '186b483d056a033826ae73d88f732985c4ccb1f32ba35f4b4cc47fdcf04aa6eb',
                '3b952d32c67cf77e2e17446e204180ab21fb8090895138b4a4a797f86e80888b'
            ],
            [
                'df9d70a6b9876ce544c98561f4be4f725442e6d2b737d9c91a8321724ce0963f',
                '55eb2dafd84d6ccd5f862b785dc39d4ab157222720ef9da217b8c45cf2ba2417'
            ],
            [
                '5edd5cc23c51e87a497ca815d5dce0f8ab52554f849ed8995de64c5f34ce7143',
                'efae9c8dbc14130661e8cec030c89ad0c13c66c0d17a2905cdc706ab7399a868'
            ],
            [
                '290798c2b6476830da12fe02287e9e777aa3fba1c355b17a722d362f84614fba',
                'e38da76dcd440621988d00bcf79af25d5b29c094db2a23146d003afd41943e7a'
            ],
            [
                'af3c423a95d9f5b3054754efa150ac39cd29552fe360257362dfdecef4053b45',
                'f98a3fd831eb2b749a93b0e6f35cfb40c8cd5aa667a15581bc2feded498fd9c6'
            ],
            [
                '766dbb24d134e745cccaa28c99bf274906bb66b26dcf98df8d2fed50d884249a',
                '744b1152eacbe5e38dcc887980da38b897584a65fa06cedd2c924f97cbac5996'
            ],
            [
                '59dbf46f8c94759ba21277c33784f41645f7b44f6c596a58ce92e666191abe3e',
                'c534ad44175fbc300f4ea6ce648309a042ce739a7919798cd85e216c4a307f6e'
            ],
            [
                'f13ada95103c4537305e691e74e9a4a8dd647e711a95e73cb62dc6018cfd87b8',
                'e13817b44ee14de663bf4bc808341f326949e21a6a75c2570778419bdaf5733d'
            ],
            [
                '7754b4fa0e8aced06d4167a2c59cca4cda1869c06ebadfb6488550015a88522c',
                '30e93e864e669d82224b967c3020b8fa8d1e4e350b6cbcc537a48b57841163a2'
            ],
            [
                '948dcadf5990e048aa3874d46abef9d701858f95de8041d2a6828c99e2262519',
                'e491a42537f6e597d5d28a3224b1bc25df9154efbd2ef1d2cbba2cae5347d57e'
            ],
            [
                '7962414450c76c1689c7b48f8202ec37fb224cf5ac0bfa1570328a8a3d7c77ab',
                '100b610ec4ffb4760d5c1fc133ef6f6b12507a051f04ac5760afa5b29db83437'
            ],
            [
                '3514087834964b54b15b160644d915485a16977225b8847bb0dd085137ec47ca',
                'ef0afbb2056205448e1652c48e8127fc6039e77c15c2378b7e7d15a0de293311'
            ],
            [
                'd3cc30ad6b483e4bc79ce2c9dd8bc54993e947eb8df787b442943d3f7b527eaf',
                '8b378a22d827278d89c5e9be8f9508ae3c2ad46290358630afb34db04eede0a4'
            ],
            [
                '1624d84780732860ce1c78fcbfefe08b2b29823db913f6493975ba0ff4847610',
                '68651cf9b6da903e0914448c6cd9d4ca896878f5282be4c8cc06e2a404078575'
            ],
            [
                '733ce80da955a8a26902c95633e62a985192474b5af207da6df7b4fd5fc61cd4',
                'f5435a2bd2badf7d485a4d8b8db9fcce3e1ef8e0201e4578c54673bc1dc5ea1d'
            ],
            [
                '15d9441254945064cf1a1c33bbd3b49f8966c5092171e699ef258dfab81c045c',
                'd56eb30b69463e7234f5137b73b84177434800bacebfc685fc37bbe9efe4070d'
            ],
            [
                'a1d0fcf2ec9de675b612136e5ce70d271c21417c9d2b8aaaac138599d0717940',
                'edd77f50bcb5a3cab2e90737309667f2641462a54070f3d519212d39c197a629'
            ],
            [
                'e22fbe15c0af8ccc5780c0735f84dbe9a790badee8245c06c7ca37331cb36980',
                'a855babad5cd60c88b430a69f53a1a7a38289154964799be43d06d77d31da06'
            ],
            [
                '311091dd9860e8e20ee13473c1155f5f69635e394704eaa74009452246cfa9b3',
                '66db656f87d1f04fffd1f04788c06830871ec5a64feee685bd80f0b1286d8374'
            ],
            [
                '34c1fd04d301be89b31c0442d3e6ac24883928b45a9340781867d4232ec2dbdf',
                '9414685e97b1b5954bd46f730174136d57f1ceeb487443dc5321857ba73abee'
            ],
            [
                'f219ea5d6b54701c1c14de5b557eb42a8d13f3abbcd08affcc2a5e6b049b8d63',
                '4cb95957e83d40b0f73af4544cccf6b1f4b08d3c07b27fb8d8c2962a400766d1'
            ],
            [
                'd7b8740f74a8fbaab1f683db8f45de26543a5490bca627087236912469a0b448',
                'fa77968128d9c92ee1010f337ad4717eff15db5ed3c049b3411e0315eaa4593b'
            ],
            [
                '32d31c222f8f6f0ef86f7c98d3a3335ead5bcd32abdd94289fe4d3091aa824bf',
                '5f3032f5892156e39ccd3d7915b9e1da2e6dac9e6f26e961118d14b8462e1661'
            ],
            [
                '7461f371914ab32671045a155d9831ea8793d77cd59592c4340f86cbc18347b5',
                '8ec0ba238b96bec0cbdddcae0aa442542eee1ff50c986ea6b39847b3cc092ff6'
            ],
            [
                'ee079adb1df1860074356a25aa38206a6d716b2c3e67453d287698bad7b2b2d6',
                '8dc2412aafe3be5c4c5f37e0ecc5f9f6a446989af04c4e25ebaac479ec1c8c1e'
            ],
            [
                '16ec93e447ec83f0467b18302ee620f7e65de331874c9dc72bfd8616ba9da6b5',
                '5e4631150e62fb40d0e8c2a7ca5804a39d58186a50e497139626778e25b0674d'
            ],
            [
                'eaa5f980c245f6f038978290afa70b6bd8855897f98b6aa485b96065d537bd99',
                'f65f5d3e292c2e0819a528391c994624d784869d7e6ea67fb18041024edc07dc'
            ],
            [
                '78c9407544ac132692ee1910a02439958ae04877151342ea96c4b6b35a49f51',
                'f3e0319169eb9b85d5404795539a5e68fa1fbd583c064d2462b675f194a3ddb4'
            ],
            [
                '494f4be219a1a77016dcd838431aea0001cdc8ae7a6fc688726578d9702857a5',
                '42242a969283a5f339ba7f075e36ba2af925ce30d767ed6e55f4b031880d562c'
            ],
            [
                'a598a8030da6d86c6bc7f2f5144ea549d28211ea58faa70ebf4c1e665c1fe9b5',
                '204b5d6f84822c307e4b4a7140737aec23fc63b65b35f86a10026dbd2d864e6b'
            ],
            [
                'c41916365abb2b5d09192f5f2dbeafec208f020f12570a184dbadc3e58595997',
                '4f14351d0087efa49d245b328984989d5caf9450f34bfc0ed16e96b58fa9913'
            ],
            [
                '841d6063a586fa475a724604da03bc5b92a2e0d2e0a36acfe4c73a5514742881',
                '73867f59c0659e81904f9a1c7543698e62562d6744c169ce7a36de01a8d6154'
            ],
            [
                '5e95bb399a6971d376026947f89bde2f282b33810928be4ded112ac4d70e20d5',
                '39f23f366809085beebfc71181313775a99c9aed7d8ba38b161384c746012865'
            ],
            [
                '36e4641a53948fd476c39f8a99fd974e5ec07564b5315d8bf99471bca0ef2f66',
                'd2424b1b1abe4eb8164227b085c9aa9456ea13493fd563e06fd51cf5694c78fc'
            ],
            [
                '336581ea7bfbbb290c191a2f507a41cf5643842170e914faeab27c2c579f726',
                'ead12168595fe1be99252129b6e56b3391f7ab1410cd1e0ef3dcdcabd2fda224'
            ],
            [
                '8ab89816dadfd6b6a1f2634fcf00ec8403781025ed6890c4849742706bd43ede',
                '6fdcef09f2f6d0a044e654aef624136f503d459c3e89845858a47a9129cdd24e'
            ],
            [
                '1e33f1a746c9c5778133344d9299fcaa20b0938e8acff2544bb40284b8c5fb94',
                '60660257dd11b3aa9c8ed618d24edff2306d320f1d03010e33a7d2057f3b3b6'
            ],
            [
                '85b7c1dcb3cec1b7ee7f30ded79dd20a0ed1f4cc18cbcfcfa410361fd8f08f31',
                '3d98a9cdd026dd43f39048f25a8847f4fcafad1895d7a633c6fed3c35e999511'
            ],
            [
                '29df9fbd8d9e46509275f4b125d6d45d7fbe9a3b878a7af872a2800661ac5f51',
                'b4c4fe99c775a606e2d8862179139ffda61dc861c019e55cd2876eb2a27d84b'
            ],
            [
                'a0b1cae06b0a847a3fea6e671aaf8adfdfe58ca2f768105c8082b2e449fce252',
                'ae434102edde0958ec4b19d917a6a28e6b72da1834aff0e650f049503a296cf2'
            ],
            [
                '4e8ceafb9b3e9a136dc7ff67e840295b499dfb3b2133e4ba113f2e4c0e121e5',
                'cf2174118c8b6d7a4b48f6d534ce5c79422c086a63460502b827ce62a326683c'
            ],
            [
                'd24a44e047e19b6f5afb81c7ca2f69080a5076689a010919f42725c2b789a33b',
                '6fb8d5591b466f8fc63db50f1c0f1c69013f996887b8244d2cdec417afea8fa3'
            ],
            [
                'ea01606a7a6c9cdd249fdfcfacb99584001edd28abbab77b5104e98e8e3b35d4',
                '322af4908c7312b0cfbfe369f7a7b3cdb7d4494bc2823700cfd652188a3ea98d'
            ],
            [
                'af8addbf2b661c8a6c6328655eb96651252007d8c5ea31be4ad196de8ce2131f',
                '6749e67c029b85f52a034eafd096836b2520818680e26ac8f3dfbcdb71749700'
            ],
            [
                'e3ae1974566ca06cc516d47e0fb165a674a3dabcfca15e722f0e3450f45889',
                '2aeabe7e4531510116217f07bf4d07300de97e4874f81f533420a72eeb0bd6a4'
            ],
            [
                '591ee355313d99721cf6993ffed1e3e301993ff3ed258802075ea8ced397e246',
                'b0ea558a113c30bea60fc4775460c7901ff0b053d25ca2bdeee98f1a4be5d196'
            ],
            [
                '11396d55fda54c49f19aa97318d8da61fa8584e47b084945077cf03255b52984',
                '998c74a8cd45ac01289d5833a7beb4744ff536b01b257be4c5767bea93ea57a4'
            ],
            [
                '3c5d2a1ba39c5a1790000738c9e0c40b8dcdfd5468754b6405540157e017aa7a',
                'b2284279995a34e2f9d4de7396fc18b80f9b8b9fdd270f6661f79ca4c81bd257'
            ],
            [
                'cc8704b8a60a0defa3a99a7299f2e9c3fbc395afb04ac078425ef8a1793cc030',
                'bdd46039feed17881d1e0862db347f8cf395b74fc4bcdc4e940b74e3ac1f1b13'
            ],
            [
                'c533e4f7ea8555aacd9777ac5cad29b97dd4defccc53ee7ea204119b2889b197',
                '6f0a256bc5efdf429a2fb6242f1a43a2d9b925bb4a4b3a26bb8e0f45eb596096'
            ],
            [
                'c14f8f2ccb27d6f109f6d08d03cc96a69ba8c34eec07bbcf566d48e33da6593',
                'c359d6923bb398f7fd4473e16fe1c28475b740dd098075e6c0e8649113dc3a38'
            ],
            [
                'a6cbc3046bc6a450bac24789fa17115a4c9739ed75f8f21ce441f72e0b90e6ef',
                '21ae7f4680e889bb130619e2c0f95a360ceb573c70603139862afd617fa9b9f'
            ],
            [
                '347d6d9a02c48927ebfb86c1359b1caf130a3c0267d11ce6344b39f99d43cc38',
                '60ea7f61a353524d1c987f6ecec92f086d565ab687870cb12689ff1e31c74448'
            ],
            [
                'da6545d2181db8d983f7dcb375ef5866d47c67b1bf31c8cf855ef7437b72656a',
                '49b96715ab6878a79e78f07ce5680c5d6673051b4935bd897fea824b77dc208a'
            ],
            [
                'c40747cc9d012cb1a13b8148309c6de7ec25d6945d657146b9d5994b8feb1111',
                '5ca560753be2a12fc6de6caf2cb489565db936156b9514e1bb5e83037e0fa2d4'
            ],
            [
                '4e42c8ec82c99798ccf3a610be870e78338c7f713348bd34c8203ef4037f3502',
                '7571d74ee5e0fb92a7a8b33a07783341a5492144cc54bcc40a94473693606437'
            ],
            [
                '3775ab7089bc6af823aba2e1af70b236d251cadb0c86743287522a1b3b0dedea',
                'be52d107bcfa09d8bcb9736a828cfa7fac8db17bf7a76a2c42ad961409018cf7'
            ],
            [
                'cee31cbf7e34ec379d94fb814d3d775ad954595d1314ba8846959e3e82f74e26',
                '8fd64a14c06b589c26b947ae2bcf6bfa0149ef0be14ed4d80f448a01c43b1c6d'
            ],
            [
                'b4f9eaea09b6917619f6ea6a4eb5464efddb58fd45b1ebefcdc1a01d08b47986',
                '39e5c9925b5a54b07433a4f18c61726f8bb131c012ca542eb24a8ac07200682a'
            ],
            [
                'd4263dfc3d2df923a0179a48966d30ce84e2515afc3dccc1b77907792ebcc60e',
                '62dfaf07a0f78feb30e30d6295853ce189e127760ad6cf7fae164e122a208d54'
            ],
            [
                '48457524820fa65a4f8d35eb6930857c0032acc0a4a2de422233eeda897612c4',
                '25a748ab367979d98733c38a1fa1c2e7dc6cc07db2d60a9ae7a76aaa49bd0f77'
            ],
            [
                'dfeeef1881101f2cb11644f3a2afdfc2045e19919152923f367a1767c11cceda',
                'ecfb7056cf1de042f9420bab396793c0c390bde74b4bbdff16a83ae09a9a7517'
            ],
            [
                '6d7ef6b17543f8373c573f44e1f389835d89bcbc6062ced36c82df83b8fae859',
                'cd450ec335438986dfefa10c57fea9bcc521a0959b2d80bbf74b190dca712d10'
            ],
            [
                'e75605d59102a5a2684500d3b991f2e3f3c88b93225547035af25af66e04541f',
                'f5c54754a8f71ee540b9b48728473e314f729ac5308b06938360990e2bfad125'
            ],
            [
                'eb98660f4c4dfaa06a2be453d5020bc99a0c2e60abe388457dd43fefb1ed620c',
                '6cb9a8876d9cb8520609af3add26cd20a0a7cd8a9411131ce85f44100099223e'
            ],
            [
                '13e87b027d8514d35939f2e6892b19922154596941888336dc3563e3b8dba942',
                'fef5a3c68059a6dec5d624114bf1e91aac2b9da568d6abeb2570d55646b8adf1'
            ],
            [
                'ee163026e9fd6fe017c38f06a5be6fc125424b371ce2708e7bf4491691e5764a',
                '1acb250f255dd61c43d94ccc670d0f58f49ae3fa15b96623e5430da0ad6c62b2'
            ],
            [
                'b268f5ef9ad51e4d78de3a750c2dc89b1e626d43505867999932e5db33af3d80',
                '5f310d4b3c99b9ebb19f77d41c1dee018cf0d34fd4191614003e945a1216e423'
            ],
            [
                'ff07f3118a9df035e9fad85eb6c7bfe42b02f01ca99ceea3bf7ffdba93c4750d',
                '438136d603e858a3a5c440c38eccbaddc1d2942114e2eddd4740d098ced1f0d8'
            ],
            [
                '8d8b9855c7c052a34146fd20ffb658bea4b9f69e0d825ebec16e8c3ce2b526a1',
                'cdb559eedc2d79f926baf44fb84ea4d44bcf50fee51d7ceb30e2e7f463036758'
            ],
            [
                '52db0b5384dfbf05bfa9d472d7ae26dfe4b851ceca91b1eba54263180da32b63',
                'c3b997d050ee5d423ebaf66a6db9f57b3180c902875679de924b69d84a7b375'
            ],
            [
                'e62f9490d3d51da6395efd24e80919cc7d0f29c3f3fa48c6fff543becbd43352',
                '6d89ad7ba4876b0b22c2ca280c682862f342c8591f1daf5170e07bfd9ccafa7d'
            ],
            [
                '7f30ea2476b399b4957509c88f77d0191afa2ff5cb7b14fd6d8e7d65aaab1193',
                'ca5ef7d4b231c94c3b15389a5f6311e9daff7bb67b103e9880ef4bff637acaec'
            ],
            [
                '5098ff1e1d9f14fb46a210fada6c903fef0fb7b4a1dd1d9ac60a0361800b7a00',
                '9731141d81fc8f8084d37c6e7542006b3ee1b40d60dfe5362a5b132fd17ddc0'
            ],
            [
                '32b78c7de9ee512a72895be6b9cbefa6e2f3c4ccce445c96b9f2c81e2778ad58',
                'ee1849f513df71e32efc3896ee28260c73bb80547ae2275ba497237794c8753c'
            ],
            [
                'e2cb74fddc8e9fbcd076eef2a7c72b0ce37d50f08269dfc074b581550547a4f7',
                'd3aa2ed71c9dd2247a62df062736eb0baddea9e36122d2be8641abcb005cc4a4'
            ],
            [
                '8438447566d4d7bedadc299496ab357426009a35f235cb141be0d99cd10ae3a8',
                'c4e1020916980a4da5d01ac5e6ad330734ef0d7906631c4f2390426b2edd791f'
            ],
            [
                '4162d488b89402039b584c6fc6c308870587d9c46f660b878ab65c82c711d67e',
                '67163e903236289f776f22c25fb8a3afc1732f2b84b4e95dbda47ae5a0852649'
            ],
            [
                '3fad3fa84caf0f34f0f89bfd2dcf54fc175d767aec3e50684f3ba4a4bf5f683d',
                'cd1bc7cb6cc407bb2f0ca647c718a730cf71872e7d0d2a53fa20efcdfe61826'
            ],
            [
                '674f2600a3007a00568c1a7ce05d0816c1fb84bf1370798f1c69532faeb1a86b',
                '299d21f9413f33b3edf43b257004580b70db57da0b182259e09eecc69e0d38a5'
            ],
            [
                'd32f4da54ade74abb81b815ad1fb3b263d82d6c692714bcff87d29bd5ee9f08f',
                'f9429e738b8e53b968e99016c059707782e14f4535359d582fc416910b3eea87'
            ],
            [
                '30e4e670435385556e593657135845d36fbb6931f72b08cb1ed954f1e3ce3ff6',
                '462f9bce619898638499350113bbc9b10a878d35da70740dc695a559eb88db7b'
            ],
            [
                'be2062003c51cc3004682904330e4dee7f3dcd10b01e580bf1971b04d4cad297',
                '62188bc49d61e5428573d48a74e1c655b1c61090905682a0d5558ed72dccb9bc'
            ],
            [
                '93144423ace3451ed29e0fb9ac2af211cb6e84a601df5993c419859fff5df04a',
                '7c10dfb164c3425f5c71a3f9d7992038f1065224f72bb9d1d902a6d13037b47c'
            ],
            [
                'b015f8044f5fcbdcf21ca26d6c34fb8197829205c7b7d2a7cb66418c157b112c',
                'ab8c1e086d04e813744a655b2df8d5f83b3cdc6faa3088c1d3aea1454e3a1d5f'
            ],
            [
                'd5e9e1da649d97d89e4868117a465a3a4f8a18de57a140d36b3f2af341a21b52',
                '4cb04437f391ed73111a13cc1d4dd0db1693465c2240480d8955e8592f27447a'
            ],
            [
                'd3ae41047dd7ca065dbf8ed77b992439983005cd72e16d6f996a5316d36966bb',
                'bd1aeb21ad22ebb22a10f0303417c6d964f8cdd7df0aca614b10dc14d125ac46'
            ],
            [
                '463e2763d885f958fc66cdd22800f0a487197d0a82e377b49f80af87c897b065',
                'bfefacdb0e5d0fd7df3a311a94de062b26b80c61fbc97508b79992671ef7ca7f'
            ],
            [
                '7985fdfd127c0567c6f53ec1bb63ec3158e597c40bfe747c83cddfc910641917',
                '603c12daf3d9862ef2b25fe1de289aed24ed291e0ec6708703a5bd567f32ed03'
            ],
            [
                '74a1ad6b5f76e39db2dd249410eac7f99e74c59cb83d2d0ed5ff1543da7703e9',
                'cc6157ef18c9c63cd6193d83631bbea0093e0968942e8c33d5737fd790e0db08'
            ],
            [
                '30682a50703375f602d416664ba19b7fc9bab42c72747463a71d0896b22f6da3',
                '553e04f6b018b4fa6c8f39e7f311d3176290d0e0f19ca73f17714d9977a22ff8'
            ],
            [
                '9e2158f0d7c0d5f26c3791efefa79597654e7a2b2464f52b1ee6c1347769ef57',
                '712fcdd1b9053f09003a3481fa7762e9ffd7c8ef35a38509e2fbf2629008373'
            ],
            [
                '176e26989a43c9cfeba4029c202538c28172e566e3c4fce7322857f3be327d66',
                'ed8cc9d04b29eb877d270b4878dc43c19aefd31f4eee09ee7b47834c1fa4b1c3'
            ],
            [
                '75d46efea3771e6e68abb89a13ad747ecf1892393dfc4f1b7004788c50374da8',
                '9852390a99507679fd0b86fd2b39a868d7efc22151346e1a3ca4726586a6bed8'
            ],
            [
                '809a20c67d64900ffb698c4c825f6d5f2310fb0451c869345b7319f645605721',
                '9e994980d9917e22b76b061927fa04143d096ccc54963e6a5ebfa5f3f8e286c1'
            ],
            [
                '1b38903a43f7f114ed4500b4eac7083fdefece1cf29c63528d563446f972c180',
                '4036edc931a60ae889353f77fd53de4a2708b26b6f5da72ad3394119daf408f9'
            ]
        ]
    }
};
// elliptic/precomputed/secp256k1 end----------------------------------------------------------------------

// elliptic/curves begin----------------------------------------------------------------------
var elliptic_curves = {};

//var ix_hash = require('hash.js');
//var elliptic = require('../elliptic');

//var assert = elliptic.utils.assert;

function PresetCurve(options)
{
    if (options.type === 'short')
        this.curve = new elliptic_curve.short(options);
    else if (options.type === 'edwards')
        this.curve = new elliptic_curve.edwards(options);
    else
        this.curve = new elliptic_curve.mont(options);
    this.g = this.curve.g;
    this.n = this.curve.n;
    this.hash = options.hash;

    ix_base.assert(this.g.validate(), 'Invalid curve');
    ix_base.assert(this.g.mul(this.n).isInfinity(), 'Invalid curve, G*N != O');
}
elliptic_curves.PresetCurve = PresetCurve;

function defineCurve(name, options)
{
    Object.defineProperty(elliptic_curves, name,
        {
            configurable: true,
            enumerable: true,
            get: function ()
            {
                var curve = new PresetCurve(options);
                Object.defineProperty(elliptic_curves, name, {
                    configurable: true,
                    enumerable: true,
                    value: curve
                });
                return curve;
            }
        });
}

defineCurve('p192', {
    type: 'short',
    prime: 'p192',
    p: 'ffffffff ffffffff ffffffff fffffffe ffffffff ffffffff',
    a: 'ffffffff ffffffff ffffffff fffffffe ffffffff fffffffc',
    b: '64210519 e59c80e7 0fa7e9ab 72243049 feb8deec c146b9b1',
    n: 'ffffffff ffffffff ffffffff 99def836 146bc9b1 b4d22831',
    hash: ix_hash.sha256,
    gRed: false,
    g: [
        '188da80e b03090f6 7cbf20eb 43a18800 f4ff0afd 82ff1012',
        '07192b95 ffc8da78 631011ed 6b24cdd5 73f977a1 1e794811'
    ]
});

defineCurve('p224', {
    type: 'short',
    prime: 'p224',
    p: 'ffffffff ffffffff ffffffff ffffffff 00000000 00000000 00000001',
    a: 'ffffffff ffffffff ffffffff fffffffe ffffffff ffffffff fffffffe',
    b: 'b4050a85 0c04b3ab f5413256 5044b0b7 d7bfd8ba 270b3943 2355ffb4',
    n: 'ffffffff ffffffff ffffffff ffff16a2 e0b8f03e 13dd2945 5c5c2a3d',
    hash: ix_hash.sha256,
    gRed: false,
    g: [
        'b70e0cbd 6bb4bf7f 321390b9 4a03c1d3 56c21122 343280d6 115c1d21',
        'bd376388 b5f723fb 4c22dfe6 cd4375a0 5a074764 44d58199 85007e34'
    ]
});

defineCurve('p256', {
    type: 'short',
    prime: null,
    p: 'ffffffff 00000001 00000000 00000000 00000000 ffffffff ffffffff ffffffff',
    a: 'ffffffff 00000001 00000000 00000000 00000000 ffffffff ffffffff fffffffc',
    b: '5ac635d8 aa3a93e7 b3ebbd55 769886bc 651d06b0 cc53b0f6 3bce3c3e 27d2604b',
    n: 'ffffffff 00000000 ffffffff ffffffff bce6faad a7179e84 f3b9cac2 fc632551',
    hash: ix_hash.sha256,
    gRed: false,
    g: [
        '6b17d1f2 e12c4247 f8bce6e5 63a440f2 77037d81 2deb33a0 f4a13945 d898c296',
        '4fe342e2 fe1a7f9b 8ee7eb4a 7c0f9e16 2bce3357 6b315ece cbb64068 37bf51f5'
    ]
});

defineCurve('p384', {
    type: 'short',
    prime: null,
    p: 'ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff ' +
    'fffffffe ffffffff 00000000 00000000 ffffffff',
    a: 'ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff ' +
    'fffffffe ffffffff 00000000 00000000 fffffffc',
    b: 'b3312fa7 e23ee7e4 988e056b e3f82d19 181d9c6e fe814112 0314088f ' +
    '5013875a c656398d 8a2ed19d 2a85c8ed d3ec2aef',
    n: 'ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff c7634d81 ' +
    'f4372ddf 581a0db2 48b0a77a ecec196a ccc52973',
    hash: ix_hash.sha384,
    gRed: false,
    g: [
        'aa87ca22 be8b0537 8eb1c71e f320ad74 6e1d3b62 8ba79b98 59f741e0 82542a38 ' +
        '5502f25d bf55296c 3a545e38 72760ab7',
        '3617de4a 96262c6f 5d9e98bf 9292dc29 f8f41dbd 289a147c e9da3113 b5f0b8c0 ' +
        '0a60b1ce 1d7e819d 7a431d7c 90ea0e5f'
    ]
});

defineCurve('p521', {
    type: 'short',
    prime: null,
    p: '000001ff ffffffff ffffffff ffffffff ffffffff ffffffff ' +
    'ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff ' +
    'ffffffff ffffffff ffffffff ffffffff ffffffff',
    a: '000001ff ffffffff ffffffff ffffffff ffffffff ffffffff ' +
    'ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff ' +
    'ffffffff ffffffff ffffffff ffffffff fffffffc',
    b: '00000051 953eb961 8e1c9a1f 929a21a0 b68540ee a2da725b ' +
    '99b315f3 b8b48991 8ef109e1 56193951 ec7e937b 1652c0bd ' +
    '3bb1bf07 3573df88 3d2c34f1 ef451fd4 6b503f00',
    n: '000001ff ffffffff ffffffff ffffffff ffffffff ffffffff ' +
    'ffffffff ffffffff fffffffa 51868783 bf2f966b 7fcc0148 ' +
    'f709a5d0 3bb5c9b8 899c47ae bb6fb71e 91386409',
    hash: ix_hash.sha512,
    gRed: false,
    g: [
        '000000c6 858e06b7 0404e9cd 9e3ecb66 2395b442 9c648139 ' +
        '053fb521 f828af60 6b4d3dba a14b5e77 efe75928 fe1dc127 ' +
        'a2ffa8de 3348b3c1 856a429b f97e7e31 c2e5bd66',
        '00000118 39296a78 9a3bc004 5c8a5fb4 2c7d1bd9 98f54449 ' +
        '579b4468 17afbd17 273e662c 97ee7299 5ef42640 c550b901 ' +
        '3fad0761 353c7086 a272c240 88be9476 9fd16650'
    ]
});

defineCurve('curve25519', {
    type: 'mont',
    prime: 'p25519',
    p: '7fffffffffffffff ffffffffffffffff ffffffffffffffff ffffffffffffffed',
    a: '76d06',
    b: '1',
    n: '1000000000000000 0000000000000000 14def9dea2f79cd6 5812631a5cf5d3ed',
    hash: ix_hash.sha256,
    gRed: false,
    g: [
        '9'
    ]
});

defineCurve('ed25519', {
    type: 'edwards',
    prime: 'p25519',
    p: '7fffffffffffffff ffffffffffffffff ffffffffffffffff ffffffffffffffed',
    a: '-1',
    c: '1',
    // -121665 * (121666^(-1)) (mod P)
    d: '52036cee2b6ffe73 8cc740797779e898 00700a4d4141d8ab 75eb4dca135978a3',
    n: '1000000000000000 0000000000000000 14def9dea2f79cd6 5812631a5cf5d3ed',
    hash: ix_hash.sha256,
    gRed: false,
    g: [
        '216936d3cd6e53fec0a4e231fdd6dc5c692cc7609525a7b2c9562d608f25d51a',

        // 4/5
        '6666666666666666666666666666666666666666666666666666666666666658'
    ]
});

// var specp256k1_pre;
// try
// {
//   specp256k1_pre = require('./precomputed/secp256k1');
// } catch (e)
// {
//   specp256k1_pre = undefined;
// }

defineCurve('secp256k1', {
    type: 'short',
    prime: 'k256',
    p: 'ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff fffffffe fffffc2f',
    a: '0',
    b: '7',
    n: 'ffffffff ffffffff ffffffff fffffffe baaedce6 af48a03b bfd25e8c d0364141',
    h: '1',
    hash: ix_hash.sha256,

    // Precomputed endomorphism
    beta: '7ae96a2b657c07106e64479eac3434e99cf0497512f58995c1396c28719501ee',
    lambda: '5363ad4cc05c30e0a5261c028812645a122e22ea20816678df02967c1b23bd72',
    basis: [
        {
            a: '3086d221a7d46bcde86c90e49284eb15',
            b: '-e4437ed6010e88286f547fa90abfe4c3'
        },
        {
            a: '114ca50f7a8e2f3f657c1108d9d44cfd8',
            b: '3086d221a7d46bcde86c90e49284eb15'
        }
    ],

    gRed: false,
    g: [
        '79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798',
        '483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8',
        secp256k1_pre
    ]
});
// elliptic/curves end----------------------------------------------------------------------

// brorand begin----------------------------------------------------------------------
var _brorand_r;

var elliptic_rand = function rand(len)
{
    if (!_brorand_r)
        _brorand_r = new Rand(null);

    return _brorand_r.generate(len);
};

function Rand(rand)
{
    this.rand = rand;
}
//module.exports.Rand = Rand;

Rand.prototype.generate = function generate(len)
{
    return this._rand(len);
};

// Emulate crypto API using randy
Rand.prototype._rand = function _rand(n)
{
    if (this.rand.getBytes)
        return this.rand.getBytes(n);

    var res = new Uint8Array(n);
    for (var i = 0; i < res.length; i++)
        res[i] = this.rand.getByte();
    return res;
};

if (typeof self === 'object')
{
    if (self.crypto && self.crypto.getRandomValues)
    {
        // Modern browsers
        Rand.prototype._rand = function _rand(n)
        {
            var arr = new Uint8Array(n);
            self.crypto.getRandomValues(arr);
            return arr;
        };
    } else if (self.msCrypto && self.msCrypto.getRandomValues)
    {
        // IE
        Rand.prototype._rand = function _rand(n)
        {
            var arr = new Uint8Array(n);
            self.msCrypto.getRandomValues(arr);
            return arr;
        };

        // Safari's WebWorkers do not have `crypto`
    } else if (typeof window === 'object')
    {
        // Old junk
        Rand.prototype._rand = function ()
        {
            throw new Error('Not implemented yet');
        };
    }
} else
{
    // Node.js or Web worker with no crypto support
    try
    {
        //var crypto = require('crypto');
        if (typeof ix_crypto.randomBytes !== 'function')
            throw new Error('Not supported');

        Rand.prototype._rand = function _rand(n)
        {
            return ix_crypto.randomBytes(n);
        };
    } catch (e)
    {
    }
}
// brorand end----------------------------------------------------------------------

// elliptic/ec/index begin----------------------------------------------------------------------
//var BN = require('bn.js');
//var ix_crypto.HmacDRBG = ix_crypto.HmacDRBG;
//var elliptic = require('../../elliptic');
//var utils = elliptic.utils;
//var assert = utils.assert;

//var ec_KeyPair = require('./key');
//var ec_Signature = require('./signature');

function EC(options)
{
    if (!(this instanceof EC))
    {
        return new EC(options);
    }

    //console.log("XXXX: " + options);

    // Shortcut `elliptic.ec(curve-name)`
    if (typeof options === 'string')
    {
        ix_base.assert(elliptic_curves.hasOwnProperty(options), 'Unknown curve ' + options);

        options = elliptic_curves[options];
    }

    // Shortcut for `elliptic.ec(elliptic.curves.curveName)`
    if (options instanceof elliptic_curves.PresetCurve)
    {
        options = { curve: options };
    }

    //console.log("IIII: " + options.curve);
    //console.log("MMMM: " + options.curve.curve);

    this.curve = options.curve.curve;
    //console.log(this.curve);
    this.n = this.curve.n;
    this.nh = this.n.ushrn(1);
    this.g = this.curve.g;

    // Point on curve
    this.g = options.curve.g;
    this.g.precompute(options.curve.n.bitLength() + 1);

    // Hash for function for DRBG
    this.hash = options.hash || options.curve.hash;
}
//module.exports = EC;

EC.prototype.keyPair = function keyPair(options)
{
    return new ec_KeyPair(this, options);
};

EC.prototype.keyFromPrivate = function keyFromPrivate(priv, enc)
{
    return ec_KeyPair.fromPrivate(this, priv, enc);
};

EC.prototype.keyFromPublic = function keyFromPublic(pub, enc)
{
    return ec_KeyPair.fromPublic(this, pub, enc);
};

EC.prototype.genKeyPair = function genKeyPair(options)
{
    if (!options)
        options = {};

    // Instantiate Hmac_DRBG
    var drbg = new ix_crypto.HmacDRBG({
        hash: this.hash,
        pers: options.pers,
        persEnc: options.persEnc || 'utf8',
        entropy: options.entropy || elliptic_rand(this.hash.hmacStrength),
        entropyEnc: options.entropy && options.entropyEnc || 'utf8',
        nonce: this.n.toArray()
    });

    var bytes = this.n.byteLength();
    var ns2 = this.n.sub(new BN(2));
    do
    {
        var priv = new BN(drbg.generate(bytes));
        if (priv.cmp(ns2) > 0)
            continue;

        priv.iaddn(1);
        return this.keyFromPrivate(priv);
    } while (true);
};

EC.prototype._truncateToN = function truncateToN(msg, truncOnly)
{
    var delta = msg.byteLength() * 8 - this.n.bitLength();
    if (delta > 0)
        msg = msg.ushrn(delta);
    if (!truncOnly && msg.cmp(this.n) >= 0)
        return msg.sub(this.n);
    else
        return msg;
};

EC.prototype.sign = function sign(msg, key, enc, options)
{
    if (typeof enc === 'object')
    {
        options = enc;
        enc = null;
    }
    if (!options)
        options = {};

    key = this.keyFromPrivate(key, enc);
    msg = this._truncateToN(new BN(msg, 16));

    // Zero-extend key to provide enough entropy
    var bytes = this.n.byteLength();
    var bkey = key.getPrivate().toArray('be', bytes);

    // Zero-extend nonce to have the same byte size as N
    var nonce = msg.toArray('be', bytes);

    // Instantiate Hmac_DRBG
    var drbg = new ix_crypto.HmacDRBG({
        hash: this.hash,
        entropy: bkey,
        nonce: nonce,
        pers: options.pers,
        persEnc: options.persEnc || 'utf8'
    });

    // Number of bytes to generate
    var ns1 = this.n.sub(new BN(1));

    for (var iter = 0; true; iter++)
    {
        var k = options.k ?
            options.k(iter) :
            new BN(drbg.generate(this.n.byteLength()));
        k = this._truncateToN(k, true);
        if (k.cmpn(1) <= 0 || k.cmp(ns1) >= 0)
            continue;

        var kp = this.g.mul(k);
        if (kp.isInfinity())
            continue;

        var kpX = kp.getX();
        var r = kpX.umod(this.n);
        if (r.cmpn(0) === 0)
            continue;

        var s = k.invm(this.n).mul(r.mul(key.getPrivate()).iadd(msg));
        s = s.umod(this.n);
        if (s.cmpn(0) === 0)
            continue;

        var recoveryParam = (kp.getY().isOdd() ? 1 : 0) |
            (kpX.cmp(r) !== 0 ? 2 : 0);

        // Use complement of `s`, if it is > `n / 2`
        if (options.canonical && s.cmp(this.nh) > 0)
        {
            s = this.n.sub(s);
            recoveryParam ^= 1;
        }

        return new ec_Signature({ r: r, s: s, recoveryParam: recoveryParam });
    }
};

EC.prototype.verify = function verify(msg, signature, key, enc)
{
    msg = this._truncateToN(new BN(msg, 16));
    key = this.keyFromPublic(key, enc);
    signature = new ec_Signature(signature, 'hex');

    // Perform primitive values validation
    var r = signature.r;
    var s = signature.s;
    if (r.cmpn(1) < 0 || r.cmp(this.n) >= 0)
        return false;
    if (s.cmpn(1) < 0 || s.cmp(this.n) >= 0)
        return false;

    // Validate signature
    var sinv = s.invm(this.n);
    var u1 = sinv.mul(msg).umod(this.n);
    var u2 = sinv.mul(r).umod(this.n);

    if (!this.curve._maxwellTrick)
    {
        var p = this.g.mulAdd(u1, key.getPublic(), u2);
        if (p.isInfinity())
            return false;

        return p.getX().umod(this.n).cmp(r) === 0;
    }

    // NOTE: Greg Maxwell's trick, inspired by:
    // https://git.io/vad3K

    var p = this.g.jmulAdd(u1, key.getPublic(), u2);
    if (p.isInfinity())
        return false;

    // Compare `p.x` of Jacobian point with `r`,
    // this will do `p.x == r * p.z^2` instead of multiplying `p.x` by the
    // inverse of `p.z^2`
    return p.eqXToP(r);
};

EC.prototype.recoverPubKey = function (msg, signature, j, enc)
{
    ix_base.assert((3 & j) === j, 'The recovery param is more than two bits');
    signature = new ec_Signature(signature, enc);

    var n = this.n;
    var e = new BN(msg);
    var r = signature.r;
    var s = signature.s;

    // A set LSB signifies that the y-coordinate is odd
    var isYOdd = j & 1;
    var isSecondKey = j >> 1;
    if (r.cmp(this.curve.p.umod(this.curve.n)) >= 0 && isSecondKey)
        throw new Error('Unable to find sencond key candinate');

    // 1.1. Let x = r + jn.
    if (isSecondKey)
        r = this.curve.pointFromX(r.add(this.curve.n), isYOdd);
    else
        r = this.curve.pointFromX(r, isYOdd);

    var rInv = signature.r.invm(n);
    var s1 = n.sub(e).mul(rInv).umod(n);
    var s2 = s.mul(rInv).umod(n);

    // 1.6.1 Compute Q = r^-1 (sR -  eG)
    //               Q = r^-1 (sR + -eG)
    return this.g.mulAdd(s1, r, s2);
};

EC.prototype.getKeyRecoveryParam = function (e, signature, Q, enc)
{
    signature = new ec_Signature(signature, enc);
    if (signature.recoveryParam !== null)
        return signature.recoveryParam;

    for (var i = 0; i < 4; i++)
    {
        var Qprime;
        try
        {
            Qprime = this.recoverPubKey(e, signature, i);
        } catch (e)
        {
            continue;
        }

        if (Qprime.eq(Q))
            return i;
    }
    throw new Error('Unable to find valid recovery factor');
};
// elliptic/ec/index end----------------------------------------------------------------------

// crypto/point begin----------------------------------------------------------------------
//var BN = require('./bn');
//var BufferUtil = require('../util/buffer');

//var EC = require('elliptic').ec;
var ec = new EC('secp256k1');
var ecPoint = ec.curve.point.bind(ec.curve);
//console.log(ecPoint);
var ecPointFromX = ec.curve.pointFromX.bind(ec.curve);

/**
 *
 * Instantiate a valid secp256k1 Point from the X and Y coordinates.
 *
 * @param {BN|String} x - The X coordinate
 * @param {BN|String} y - The Y coordinate
 * @link https://github.com/indutny/elliptic
 * @augments elliptic.curve.point
 * @throws {Error} A validation error if exists
 * @returns {Point} An instance of Point
 * @constructor
 */
var Point = function Point(x, y, isRed)
{
    try
    {
        var point = ecPoint(x, y, isRed);
    } catch (e)
    {
        throw new Error('Invalid Point');
    }
    point.validate();
    return point;
};

Point.prototype = Object.getPrototypeOf(ec.curve.point());

/**
 *
 * Instantiate a valid secp256k1 Point from only the X coordinate
 *
 * @param {boolean} odd - If the Y coordinate is odd
 * @param {BN|String} x - The X coordinate
 * @throws {Error} A validation error if exists
 * @returns {Point} An instance of Point
 */
Point.fromX = function fromX(odd, x)
{
    try
    {
        var point = ecPointFromX(x, odd);
    } catch (e)
    {
        throw new Error('Invalid X');
    }
    point.validate();
    return point;
};

/**
 *
 * Will return a secp256k1 ECDSA base point.
 *
 * @link https://en.bitcoin.it/wiki/Secp256k1
 * @returns {Point} An instance of the base point.
 */
Point.getG = function getG()
{
    return ec.curve.g;
};

/**
 *
 * Will return the max of range of valid private keys as governed by the secp256k1 ECDSA standard.
 *
 * @link https://en.bitcoin.it/wiki/Private_key#Range_of_valid_ECDSA_private_keys
 * @returns {BN} A BN instance of the number of points on the curve
 */
Point.getN = function getN()
{
    return new BN(ec.curve.n.toArray());
};

Point.prototype._getX = Point.prototype.getX;

/**
 *
 * Will return the X coordinate of the Point
 *
 * @returns {BN} A BN instance of the X coordinate
 */
Point.prototype.getX = function getX()
{
    return new BN(this._getX().toArray());
};

Point.prototype._getY = Point.prototype.getY;

/**
 *
 * Will return the Y coordinate of the Point
 *
 * @returns {BN} A BN instance of the Y coordinate
 */
Point.prototype.getY = function getY()
{
    return new BN(this._getY().toArray());
};

/**
 *
 * Will determine if the point is valid
 *
 * @link https://www.iacr.org/archive/pkc2003/25670211/25670211.pdf
 * @param {Point} An instance of Point
 * @throws {Error} A validation error if exists
 * @returns {Point} An instance of the same Point
 */
Point.prototype.validate = function validate()
{

    if (this.isInfinity())
    {
        throw new Error('Point cannot be equal to Infinity');
    }

    var p2;
    try
    {
        p2 = ecPointFromX(this.getX(), this.getY().isOdd());
    } catch (e)
    {
        throw new Error('Point does not lie on the curve');
    }

    if (p2.y.cmp(this.y) !== 0)
    {
        throw new Error('Invalid y value for curve.');
    }


    //todo: needs test case
    if (!(this.mul(Point.getN()).isInfinity()))
    {
        throw new Error('Point times N must be infinity');
    }

    return this;

};

Point.pointToCompressed = function pointToCompressed(point)
{
    var xbuf = point.getX().toBuffer({ size: 32 });
    var ybuf = point.getY().toBuffer({ size: 32 });

    var prefix;
    var odd = ybuf[ybuf.length - 1] % 2;
    if (odd)
    {
        prefix = new Buffer([0x03]);
    } else
    {
        prefix = new Buffer([0x02]);
    }
    return BufferUtil.concat([prefix, xbuf]);
};

magnachain.Point = Point;
// crypto/point end----------------------------------------------------------------------

// networks begin----------------------------------------------------------------------
// var _ = require('lodash');

// var BufferUtil = require('./util/buffer');
// var JSUtil = require('./util/js');
var networks = [];
var networkMaps = {};

/**
 * A network is merely a map containing values that correspond to version
 * numbers for each bitcoin network. Currently only supporting "livenet"
 * (a.k.a. "mainnet") and "testnet".
 * @constructor
 */
function Networks() { }

Networks.prototype.toString = function toString()
{
    return this.name;
};

/**
 * @function
 * @member Networks#get
 * Retrieves the network associated with a magic number or string.
 * @param {string|number|Network} arg
 * @param {string|Array} keys - if set, only check if the magic number associated with this name matches
 * @return Network
 */
function get(arg, keys)
{
    if (~networks.indexOf(arg))
    {
        return arg;
    }
    if (keys)
    {
        if (!_.isArray(keys))
        {
            keys = [keys];
        }
        var containsArg = function (key)
        {
            return networks[index][key] === arg;
        };
        for (var index in networks)
        {
            if (_.some(keys, containsArg))
            {
                return networks[index];
            }
        }
        return undefined;
    }
    return networkMaps[arg];
}

/**
 * @function
 * @member Networks#add
 * Will add a custom Network
 * @param {Object} data
 * @param {string} data.name - The name of the network
 * @param {string} data.alias - The aliased name of the network
 * @param {Number} data.pubkeyhash - The publickey hash prefix
 * @param {Number} data.privatekey - The privatekey prefix
 * @param {Number} data.scripthash - The scripthash prefix
 * @param {Number} data.contracthash - The contracthash prefix
 * @param {Number} data.xpubkey - The extended public key magic
 * @param {Number} data.xprivkey - The extended private key magic
 * @param {Number} data.networkMagic - The network magic number
 * @param {Number} data.port - The network port
 * @param {Array}  data.dnsSeeds - An array of dns seeds
 * @return Network
 */
function addNetwork(data)
{
    var network = new Networks();

    JSUtil.defineImmutable(network, {
        name: data.name,
        alias: data.alias,
        pubkeyhash: data.pubkeyhash,
        privatekey: data.privatekey,
        scripthash: data.scripthash,
        contracthash: data.contracthash,
        xpubkey: data.xpubkey,
        xprivkey: data.xprivkey
    });

    if (data.networkMagic)
    {
        JSUtil.defineImmutable(network, {
            networkMagic: BufferUtil.integerAsBuffer(data.networkMagic)
        });
    }

    if (data.port)
    {
        JSUtil.defineImmutable(network, {
            port: data.port
        });
    }

    if (data.dnsSeeds)
    {
        JSUtil.defineImmutable(network, {
            dnsSeeds: data.dnsSeeds
        });
    }
    _.each(network, function (value)
    {
        if (!_.isUndefined(value) && !_.isObject(value))
        {
            networkMaps[value] = network;
        }
    });

    networks.push(network);

    return network;
}

/**
 * @function
 * @member Networks#remove
 * Will remove a custom network
 * @param {Network} network
 */
function removeNetwork(network)
{
    for (var i = 0; i < networks.length; i++)
    {
        if (networks[i] === network)
        {
            networks.splice(i, 1);
        }
    }
    for (var key in networkMaps)
    {
        if (networkMaps[key] === network)
        {
            delete networkMaps[key];
        }
    }
}

addNetwork({
    name: 'livenet',
    alias: 'mainnet',
    pubkeyhash: 75,     // 0x4B X pubkeyhash: 0x00,
    scripthash: 62,     // 0x3e S scripthash: 0x05,
    contracthash: 69,   //0x45
    privatekey: 0x80,

    xpubkey: 0x0488b21e,
    xprivkey: 0x0488ade4,
    networkMagic: 0xf9beb4d9,
    port: 8333,
    dnsSeeds: [
        'seed.bitcoin.sipa.be',
        'dnsseed.bluematt.me',
        'dnsseed.bitcoin.dashjr.org',
        'seed.bitcoinstats.com',
        'seed.bitnodes.io',
        'bitseed.xf2.org'
    ]
});

/**
 * @instance
 * @member Networks#livenet
 */
var livenet = get('livenet');

addNetwork({
    name: 'testnet',
    alias: 'regtest',
    pubkeyhash: 110, //0x6e
    scripthash: 195, //0xc3
    contracthash: 199,//0xc7

    privatekey: 0xef,
    xpubkey: 0x043587cf,
    xprivkey: 0x04358394
});

/**
 * @instance
 * @member Networks#testnet
 */
var testnet = get('testnet');

// Add configurable values for testnet/regtest

var TESTNET = {
    PORT: 18333,
    NETWORK_MAGIC: BufferUtil.integerAsBuffer(0x0b110907),
    DNS_SEEDS: [
        'testnet-seed.bitcoin.petertodd.org',
        'testnet-seed.bluematt.me',
        'testnet-seed.alexykot.me',
        'testnet-seed.bitcoin.schildbach.de'
    ]
};

for (var key in TESTNET)
{
    if (!_.isObject(TESTNET[key]))
    {
        networkMaps[TESTNET[key]] = testnet;
    }
}

var REGTEST = {
    PORT: 18444,
    NETWORK_MAGIC: BufferUtil.integerAsBuffer(0xfabfb5da),
    DNS_SEEDS: []
};

for (var key in REGTEST)
{
    if (!_.isObject(REGTEST[key]))
    {
        networkMaps[REGTEST[key]] = testnet;
    }
}

Object.defineProperty(testnet, 'port', {
    enumerable: true,
    configurable: false,
    get: function ()
    {
        if (this.regtestEnabled)
        {
            return REGTEST.PORT;
        } else
        {
            return TESTNET.PORT;
        }
    }
});

Object.defineProperty(testnet, 'networkMagic', {
    enumerable: true,
    configurable: false,
    get: function ()
    {
        if (this.regtestEnabled)
        {
            return REGTEST.NETWORK_MAGIC;
        } else
        {
            return TESTNET.NETWORK_MAGIC;
        }
    }
});

Object.defineProperty(testnet, 'dnsSeeds', {
    enumerable: true,
    configurable: false,
    get: function ()
    {
        if (this.regtestEnabled)
        {
            return REGTEST.DNS_SEEDS;
        } else
        {
            return TESTNET.DNS_SEEDS;
        }
    }
});

/**
 * @function
 * @member Networks#enableRegtest
 * Will enable regtest features for testnet
 */
function enableRegtest()
{
    testnet.regtestEnabled = true;
}

/**
 * @function
 * @member Networks#disableRegtest
 * Will disable regtest features for testnet
 */
function disableRegtest()
{
    testnet.regtestEnabled = false;
}

/**
 * @namespace Networks
 */
// module.exports = {
//     add: addNetwork,
//     remove: removeNetwork,
//     defaultNetwork: livenet,
//     livenet: livenet,
//     mainnet: livenet,
//     testnet: testnet,
//     get: get,
//     enableRegtest: enableRegtest,
//     disableRegtest: disableRegtest
// };
Networks.add = addNetwork;
Networks.remove = removeNetwork;
Networks.defaultNetwork = livenet;
Networks.livenet = livenet;
Networks.mainnet = livenet;
Networks.testnet = testnet;
Networks.get = get;
Networks.enableRegtest = enableRegtest;
Networks.disableRegtest = disableRegtest;

magnachain.Networks = Networks;
// networks end----------------------------------------------------------------------

// publickey begin----------------------------------------------------------------------
//var BN = require('./crypto/bn');
//var Point = require('./crypto/point');
//var Hash = require('./crypto/hash');
//var JSUtil = require('./util/js');
//var Network = require('./networks');
//var _ = require('lodash');
//var $ = require('./util/preconditions');

/**
 * Instantiate a PublicKey from a {@link PrivateKey}, {@link Point}, `string`, or `Buffer`.
 *
 * There are two internal properties, `network` and `compressed`, that deal with importing
 * a PublicKey from a PrivateKey in WIF format. More details described on {@link PrivateKey}
 *
 * @example
 * ```javascript
 * // instantiate from a private key
 * var key = PublicKey(privateKey, true);
 *
 * // export to as a DER hex encoded string
 * var exported = key.toString();
 *
 * // import the public key
 * var imported = PublicKey.fromString(exported);
 * ```
 *
 * @param {string} data - The encoded data in various formats
 * @param {Object} extra - additional options
 * @param {Network=} extra.network - Which network should the address for this public key be for
 * @param {String=} extra.compressed - If the public key is compressed
 * @returns {PublicKey} A new valid instance of an PublicKey
 * @constructor
 */
function PublicKey(data, extra)
{
    if (!(this instanceof PublicKey))
    {
        return new PublicKey(data, extra);
    }

    $.checkArgument(data, 'First argument is required, please include public key data.');

    if (data instanceof PublicKey)
    {
        // Return copy, but as it's an immutable object, return same argument
        return data;
    }
    extra = extra || {};

    var info = this._classifyArgs(data, extra);

    // validation
    info.point.validate();

    JSUtil.defineImmutable(this, {
        point: info.point,
        compressed: info.compressed,
        network: info.network || Networks.defaultNetwork
    });

    return this;
};

/**
 * Internal function to differentiate between arguments passed to the constructor
 * @param {*} data
 * @param {Object} extra
 */
PublicKey.prototype._classifyArgs = function (data, extra)
{
    /* jshint maxcomplexity: 10 */
    var info = {
        compressed: _.isUndefined(extra.compressed) || extra.compressed
    };

    // detect type of data
    if (data instanceof Point)
    {
        info.point = data;
    } else if (data.x && data.y)
    {
        info = PublicKey._transformObject(data);
    } else if (typeof (data) === 'string')
    {
        info = PublicKey._transformDER(new Buffer(data, 'hex'));
    } else if (PublicKey._isBuffer(data))
    {
        info = PublicKey._transformDER(data);
    } else if (PublicKey._isPrivateKey(data))
    {
        info = PublicKey._transformPrivateKey(data);
    } else
    {
        throw new TypeError('First argument is an unrecognized data format.');
    }
    if (!info.network)
    {
        info.network = _.isUndefined(extra.network) ? undefined : Networks.get(extra.network);
    }
    return info;
};

/**
 * Internal function to detect if an object is a {@link PrivateKey}
 *
 * @param {*} param - object to test
 * @returns {boolean}
 * @private
 */
PublicKey._isPrivateKey = function (param)
{
    //var PrivateKey = require('./privatekey');
    return param instanceof PrivateKey;
};

/**
 * Internal function to detect if an object is a Buffer
 *
 * @param {*} param - object to test
 * @returns {boolean}
 * @private
 */
PublicKey._isBuffer = function (param)
{
    return (param instanceof Buffer) || (param instanceof Uint8Array);
};

/**
 * Internal function to transform a private key into a public key point
 *
 * @param {PrivateKey} privkey - An instance of PrivateKey
 * @returns {Object} An object with keys: point and compressed
 * @private
 */
PublicKey._transformPrivateKey = function (privkey)
{
    $.checkArgument(PublicKey._isPrivateKey(privkey), 'Must be an instance of PrivateKey');
    var info = {};
    info.point = Point.getG().mul(privkey.bn);
    info.compressed = privkey.compressed;
    info.network = privkey.network;
    return info;
};

/**
 * Internal function to transform DER into a public key point
 *
 * @param {Buffer} buf - An hex encoded buffer
 * @param {bool=} strict - if set to false, will loosen some conditions
 * @returns {Object} An object with keys: point and compressed
 * @private
 */
PublicKey._transformDER = function (buf, strict)
{
    /* jshint maxstatements: 30 */
    /* jshint maxcomplexity: 12 */
    $.checkArgument(PublicKey._isBuffer(buf), 'Must be a hex buffer of DER encoded public key');
    var info = {};

    strict = _.isUndefined(strict) ? true : strict;

    var x;
    var y;
    var xbuf;
    var ybuf;

    if (buf[0] === 0x04 || (!strict && (buf[0] === 0x06 || buf[0] === 0x07)))
    {
        xbuf = buf.slice(1, 33);
        ybuf = buf.slice(33, 65);
        if (xbuf.length !== 32 || ybuf.length !== 32 || buf.length !== 65)
        {
            throw new TypeError('Length of x and y must be 32 bytes');
        }
        x = new BN(xbuf);
        y = new BN(ybuf);
        info.point = new Point(x, y);
        info.compressed = false;
    } else if (buf[0] === 0x03)
    {
        xbuf = buf.slice(1);
        x = new BN(xbuf);
        info = PublicKey._transformX(true, x);
        info.compressed = true;
    } else if (buf[0] === 0x02)
    {
        xbuf = buf.slice(1);
        x = new BN(xbuf);
        info = PublicKey._transformX(false, x);
        info.compressed = true;
    } else
    {
        throw new TypeError('Invalid DER format public key');
    }
    return info;
};

/**
 * Internal function to transform X into a public key point
 *
 * @param {Boolean} odd - If the point is above or below the x axis
 * @param {Point} x - The x point
 * @returns {Object} An object with keys: point and compressed
 * @private
 */
PublicKey._transformX = function (odd, x)
{
    $.checkArgument(typeof odd === 'boolean', 'Must specify whether y is odd or not (true or false)');
    var info = {};
    info.point = Point.fromX(odd, x);
    return info;
};

/**
 * Internal function to transform a JSON into a public key point
 *
 * @param {String|Object} json - a JSON string or plain object
 * @returns {Object} An object with keys: point and compressed
 * @private
 */
PublicKey._transformObject = function (json)
{
    var x = new BN(json.x, 'hex');
    var y = new BN(json.y, 'hex');
    var point = new Point(x, y);
    return new PublicKey(point, {
        compressed: json.compressed
    });
};

/**
 * Instantiate a PublicKey from a PrivateKey
 *
 * @param {PrivateKey} privkey - An instance of PrivateKey
 * @returns {PublicKey} A new valid instance of PublicKey
 */
PublicKey.fromPrivateKey = function (privkey)
{
    $.checkArgument(PublicKey._isPrivateKey(privkey), 'Must be an instance of PrivateKey');
    var info = PublicKey._transformPrivateKey(privkey);
    return new PublicKey(info.point, {
        compressed: info.compressed,
        network: info.network
    });
};

/**
 * Instantiate a PublicKey from a Buffer
 * @param {Buffer} buf - A DER hex buffer
 * @param {bool=} strict - if set to false, will loosen some conditions
 * @returns {PublicKey} A new valid instance of PublicKey
 */
PublicKey.fromDER = PublicKey.fromBuffer = function (buf, strict)
{
    $.checkArgument(PublicKey._isBuffer(buf), 'Must be a hex buffer of DER encoded public key');
    var info = PublicKey._transformDER(buf, strict);
    return new PublicKey(info.point, {
        compressed: info.compressed
    });
};

/**
 * Instantiate a PublicKey from a Point
 *
 * @param {Point} point - A Point instance
 * @param {boolean=} compressed - whether to store this public key as compressed format
 * @returns {PublicKey} A new valid instance of PublicKey
 */
PublicKey.fromPoint = function (point, compressed)
{
    $.checkArgument(point instanceof Point, 'First argument must be an instance of Point.');
    return new PublicKey(point, {
        compressed: compressed
    });
};

/**
 * Instantiate a PublicKey from a DER hex encoded string
 *
 * @param {string} str - A DER hex string
 * @param {String=} encoding - The type of string encoding
 * @returns {PublicKey} A new valid instance of PublicKey
 */
PublicKey.fromString = function (str, encoding)
{
    var buf = new Buffer(str, encoding || 'hex');
    var info = PublicKey._transformDER(buf);
    return new PublicKey(info.point, {
        compressed: info.compressed
    });
};

/**
 * Instantiate a PublicKey from an X Point
 *
 * @param {Boolean} odd - If the point is above or below the x axis
 * @param {Point} x - The x point
 * @returns {PublicKey} A new valid instance of PublicKey
 */
PublicKey.fromX = function (odd, x)
{
    var info = PublicKey._transformX(odd, x);
    return new PublicKey(info.point, {
        compressed: info.compressed
    });
};

/**
 * Check if there would be any errors when initializing a PublicKey
 *
 * @param {string} data - The encoded data in various formats
 * @returns {null|Error} An error if exists
 */
PublicKey.getValidationError = function (data)
{
    var error;
    try
    {
        /* jshint nonew: false */
        new PublicKey(data);
    } catch (e)
    {
        error = e;
    }
    return error;
};

/**
 * Check if the parameters are valid
 *
 * @param {string} data - The encoded data in various formats
 * @returns {Boolean} If the public key would be valid
 */
PublicKey.isValid = function (data)
{
    return !PublicKey.getValidationError(data);
};

/**
 * @returns {Object} A plain object of the PublicKey
 */
PublicKey.prototype.toObject = PublicKey.prototype.toJSON = function toObject()
{
    return {
        x: this.point.getX().toString('hex', 2),
        y: this.point.getY().toString('hex', 2),
        compressed: this.compressed
    };
};

/**
 * Will output the PublicKey to a DER Buffer
 *
 * @returns {Buffer} A DER hex encoded buffer
 */
PublicKey.prototype.toBuffer = PublicKey.prototype.toDER = function ()
{
    //console.log("VVVVV toBuffer");

    var x = this.point.getX();
    var y = this.point.getY();

    var xbuf = x.toBuffer({
        size: 32
    });
    var ybuf = y.toBuffer({
        size: 32
    });

    var prefix;
    if (!this.compressed)
    {
        prefix = new Buffer([0x04]);
        return Buffer.concat([prefix, xbuf, ybuf]);
    } else
    {
        var odd = ybuf[ybuf.length - 1] % 2;
        if (odd)
        {
            prefix = new Buffer([0x03]);
        } else
        {
            prefix = new Buffer([0x02]);
        }
        return Buffer.concat([prefix, xbuf]);
    }
};

PublicKey.prototype.WriteForCl = function (writer)
{
    // byte len = (byte)this.vch.Length;
    // stream.ReadWrite(ref len);
    // stream.ReadWrite(ref this.vch);

    var buf = this.toBuffer();
    writer.writeUInt8(buf.length);
    //console.log("WriteForCl: " + buf);
    writer.write(buf);
};

PublicKey.ReadForCl = function (br)
{
    //byte len = 0;
    //stream.ReadWrite(ref len);
    //this.vch = new byte[len];
    //stream.ReadWrite(ref this.vch);
    //_ECKey = new ECKey(vch, false);

    var len = br.readUInt8();
    var bufVch = br.read(len);
    //console.log("ReadForCl: " + bufVch);
    return PublicKey.fromBuffer(bufVch, true);
};

/**
 * Will return a sha256 + ripemd160 hash of the serialized public key
 * @see https://github.com/bitcoin/bitcoin/blob/master/src/pubkey.h#L141
 * @returns {Buffer}
 */
PublicKey.prototype._getID = function _getID()
{
    return Hash.sha256ripemd160(this.toBuffer());
};

/**
 * Will return an address for the public key
 *
 * @param {String|Network=} network - Which network should the address be for
 * @returns {Address} An address generated from the public key
 */
PublicKey.prototype.toAddress = function (network)
{
    //var Address = require('./address');
    return Address.fromPublicKey(this, network || this.network);
};

/**
 * Will output the PublicKey to a DER encoded hex string
 *
 * @returns {string} A DER hex encoded string
 */
PublicKey.prototype.toString = function ()
{
    return this.toDER().toString('hex');
};

/**
 * Will return a string formatted for the console
 *
 * @returns {string} Public key
 */
PublicKey.prototype.inspect = function ()
{
    return '<PublicKey: ' + this.toString() +
        (this.compressed ? '' : ', uncompressed') + '>';
};

magnachain.PublicKey = PublicKey;
// publickey end----------------------------------------------------------------------

// crypto/ecdsa begin----------------------------------------------------------------------
// var BN = require('./bn');
// var Point = require('./point');
// var Signature = require('./signature');
// var PublicKey = require('../publickey');
// var Random = require('./random');
// var Hash = require('./hash');
// var BufferUtil = require('../util/buffer');
// var _ = require('lodash');
// var $ = require('../util/preconditions');

var ECDSA = function ECDSA(obj)
{
    if (!(this instanceof ECDSA))
    {
        return new ECDSA(obj);
    }
    if (obj)
    {
        this.set(obj);
    }
};

/* jshint maxcomplexity: 9 */
ECDSA.prototype.set = function (obj)
{
    this.hashbuf = obj.hashbuf || this.hashbuf;
    this.endian = obj.endian || this.endian; //the endianness of hashbuf
    this.privkey = obj.privkey || this.privkey;
    this.pubkey = obj.pubkey || (this.privkey ? this.privkey.publicKey : this.pubkey);
    this.sig = obj.sig || this.sig;
    this.k = obj.k || this.k;
    this.verified = obj.verified || this.verified;
    return this;
};

ECDSA.prototype.privkey2pubkey = function ()
{
    this.pubkey = this.privkey.toPublicKey();
};

ECDSA.prototype.calci = function ()
{
    for (var i = 0; i < 4; i++)
    {
        this.sig.i = i;
        var Qprime;
        try
        {
            Qprime = this.toPublicKey();
        } catch (e)
        {
            console.error(e);
            continue;
        }

        if (Qprime.point.eq(this.pubkey.point))
        {
            this.sig.compressed = this.pubkey.compressed;
            return this;
        }
    }

    this.sig.i = undefined;
    throw new Error('Unable to find valid recovery factor');
};

ECDSA.fromString = function (str)
{
    var obj = JSON.parse(str);
    return new ECDSA(obj);
};

ECDSA.prototype.randomK = function ()
{
    var N = Point.getN();
    var k;
    do
    {
        k = BN.fromBuffer(Random.getRandomBuffer(32));
    } while (!(k.lt(N) && k.gt(BN.Zero)));
    this.k = k;
    return this;
};


// https://tools.ietf.org/html/rfc6979#section-3.2
ECDSA.prototype.deterministicK = function (badrs)
{
    /* jshint maxstatements: 25 */
    // if r or s were invalid when this function was used in signing,
    // we do not want to actually compute r, s here for efficiency, so,
    // we can increment badrs. explained at end of RFC 6979 section 3.2
    if (_.isUndefined(badrs))
    {
        badrs = 0;
    }
    var v = new Buffer(32);
    v.fill(0x01);
    var k = new Buffer(32);
    k.fill(0x00);
    var x = this.privkey.bn.toBuffer({
        size: 32
    });
    var hashbuf = this.endian === 'little' ? BufferUtil.reverse(this.hashbuf) : this.hashbuf
    k = Hash.sha256hmac(Buffer.concat([v, new Buffer([0x00]), x, hashbuf]), k);
    v = Hash.sha256hmac(v, k);
    k = Hash.sha256hmac(Buffer.concat([v, new Buffer([0x01]), x, hashbuf]), k);
    v = Hash.sha256hmac(v, k);
    v = Hash.sha256hmac(v, k);
    var T = BN.fromBuffer(v);
    var N = Point.getN();

    // also explained in 3.2, we must ensure T is in the proper range (0, N)
    for (var i = 0; i < badrs || !(T.lt(N) && T.gt(BN.Zero)); i++)
    {
        k = Hash.sha256hmac(Buffer.concat([v, new Buffer([0x00])]), k);
        v = Hash.sha256hmac(v, k);
        v = Hash.sha256hmac(v, k);
        T = BN.fromBuffer(v);
    }

    this.k = T;
    return this;
};

// Information about public key recovery:
// https://bitcointalk.org/index.php?topic=6430.0
// http://stackoverflow.com/questions/19665491/how-do-i-get-an-ecdsa-public-key-from-just-a-bitcoin-signature-sec1-4-1-6-k
ECDSA.prototype.toPublicKey = function ()
{
    /* jshint maxstatements: 25 */
    var i = this.sig.i;
    $.checkArgument(i === 0 || i === 1 || i === 2 || i === 3, new Error('i must be equal to 0, 1, 2, or 3'));

    var e = BN.fromBuffer(this.hashbuf);
    var r = this.sig.r;
    var s = this.sig.s;

    // A set LSB signifies that the y-coordinate is odd
    var isYOdd = i & 1;

    // The more significant bit specifies whether we should use the
    // first or second candidate key.
    var isSecondKey = i >> 1;

    var n = Point.getN();
    var G = Point.getG();

    // 1.1 Let x = r + jn
    var x = isSecondKey ? r.add(n) : r;
    var R = Point.fromX(isYOdd, x);

    // 1.4 Check that nR is at infinity
    var nR = R.mul(n);

    if (!nR.isInfinity())
    {
        throw new Error('nR is not a valid curve point');
    }

    // Compute -e from e
    var eNeg = e.neg().umod(n);

    // 1.6.1 Compute Q = r^-1 (sR - eG)
    // Q = r^-1 (sR + -eG)
    var rInv = r.invm(n);

    //var Q = R.multiplyTwo(s, G, eNeg).mul(rInv);
    var Q = R.mul(s).add(G.mul(eNeg)).mul(rInv);

    var pubkey = PublicKey.fromPoint(Q, this.sig.compressed);

    return pubkey;
};

ECDSA.prototype.sigError = function ()
{
    /* jshint maxstatements: 25 */
    if (!BufferUtil.isBuffer(this.hashbuf) || this.hashbuf.length !== 32)
    {
        return 'hashbuf must be a 32 byte buffer';
    }

    var r = this.sig.r;
    var s = this.sig.s;
    if (!(r.gt(BN.Zero) && r.lt(Point.getN())) || !(s.gt(BN.Zero) && s.lt(Point.getN())))
    {
        return 'r and s not in range';
    }

    var e = BN.fromBuffer(this.hashbuf, this.endian ? {
        endian: this.endian
    } : undefined);
    var n = Point.getN();
    var sinv = s.invm(n);
    var u1 = sinv.mul(e).umod(n);
    var u2 = sinv.mul(r).umod(n);

    var p = Point.getG().mulAdd(u1, this.pubkey.point, u2);
    if (p.isInfinity())
    {
        return 'p is infinity';
    }

    if (p.getX().umod(n).cmp(r) !== 0)
    {
        return 'Invalid signature';
    } else
    {
        return false;
    }
};

ECDSA.toLowS = function (s)
{
    //enforce low s
    //see BIP 62, "low S values in signatures"
    if (s.gt(BN.fromBuffer(new Buffer('7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5D576E7357A4501DDFE92F46681B20A0', 'hex'))))
    {
        s = Point.getN().sub(s);
    }
    return s;
};

ECDSA.prototype._findSignature = function (d, e)
{
    var N = Point.getN();
    var G = Point.getG();
    // try different values of k until r, s are valid
    var badrs = 0;
    var k, Q, r, s;
    do
    {
        if (!this.k || badrs > 0)
        {
            this.deterministicK(badrs);
        }
        badrs++;
        k = this.k;
        Q = G.mul(k);
        r = Q.x.umod(N);
        s = k.invm(N).mul(e.add(d.mul(r))).umod(N);
    } while (r.cmp(BN.Zero) <= 0 || s.cmp(BN.Zero) <= 0);

    s = ECDSA.toLowS(s);
    return {
        s: s,
        r: r
    };

};

ECDSA.prototype.sign = function ()
{
    var hashbuf = this.hashbuf;
    var privkey = this.privkey;
    var d = privkey.bn;

    $.checkState(hashbuf && privkey && d, new Error('invalid parameters'));
    $.checkState(BufferUtil.isBuffer(hashbuf) && hashbuf.length === 32, new Error('hashbuf must be a 32 byte buffer'));

    var e = BN.fromBuffer(hashbuf, this.endian ? {
        endian: this.endian
    } : undefined);

    var obj = this._findSignature(d, e);
    obj.compressed = this.pubkey.compressed;

    this.sig = new Signature(obj);
    return this;
};

ECDSA.prototype.signRandomK = function ()
{
    this.randomK();
    return this.sign();
};

ECDSA.prototype.toString = function ()
{
    var obj = {};
    if (this.hashbuf)
    {
        obj.hashbuf = this.hashbuf.toString('hex');
    }
    if (this.privkey)
    {
        obj.privkey = this.privkey.toString();
    }
    if (this.pubkey)
    {
        obj.pubkey = this.pubkey.toString();
    }
    if (this.sig)
    {
        obj.sig = this.sig.toString();
    }
    if (this.k)
    {
        obj.k = this.k.toString();
    }
    return JSON.stringify(obj);
};

ECDSA.prototype.verify = function ()
{
    if (!this.sigError())
    {
        this.verified = true;
    } else
    {
        this.verified = false;
    }
    return this;
};

ECDSA.sign = function (hashbuf, privkey, endian)
{
    return ECDSA().set({
        hashbuf: hashbuf,
        endian: endian,
        privkey: privkey
    }).sign().sig;
};

ECDSA.verify = function (hashbuf, sig, pubkey, endian)
{
    return ECDSA().set({
        hashbuf: hashbuf,
        endian: endian,
        sig: sig,
        pubkey: pubkey
    }).verify().verified;
};

magnachain.ECDSA = ECDSA;
// crypto/ecdsa end----------------------------------------------------------------------

// unit begin----------------------------------------------------------------------
// var _ = require('lodash');

// var errors = require('./errors');
// var $ = require('./util/preconditions');

var UNITS = {
    'BTC': [1e8, 8],
    'mBTC': [1e5, 5],
    'uBTC': [1e2, 2],
    'bits': [1e2, 2],
    'satoshis': [1, 0]
};

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
 * ```
 *
 * @param {Number} amount - The amount to be represented
 * @param {String|Number} code - The unit of the amount or the exchange rate
 * @returns {Unit} A new instance of an Unit
 * @constructor
 */
function Unit(amount, code)
{
    if (!(this instanceof Unit))
    {
        return new Unit(amount, code);
    }

    // convert fiat to BTC
    if (_.isNumber(code))
    {
        if (code <= 0)
        {
            throw new errors.Unit.InvalidRate(code);
        }
        amount = amount / code;
        code = Unit.BTC;
    }

    this._value = this._from(amount, code);

    var self = this;
    var defineAccesor = function (key)
    {
        Object.defineProperty(self, key, {
            get: function () { return self.to(key); },
            enumerable: true,
        });
    };

    Object.keys(UNITS).forEach(defineAccesor);
}

Object.keys(UNITS).forEach(function (key)
{
    Unit[key] = key;
});

/**
 * Returns a Unit instance created from JSON string or object
 *
 * @param {String|Object} json - JSON with keys: amount and code
 * @returns {Unit} A Unit instance
 */
Unit.fromObject = function fromObject(data)
{
    $.checkArgument(_.isObject(data), 'Argument is expected to be an object');
    return new Unit(data.amount, data.code);
};

/**
 * Returns a Unit instance created from an amount in BTC
 *
 * @param {Number} amount - The amount in BTC
 * @returns {Unit} A Unit instance
 */
Unit.fromBTC = function (amount)
{
    return new Unit(amount, Unit.BTC);
};

/**
 * Returns a Unit instance created from an amount in mBTC
 *
 * @param {Number} amount - The amount in mBTC
 * @returns {Unit} A Unit instance
 */
Unit.fromMillis = Unit.fromMilis = function (amount)
{
    return new Unit(amount, Unit.mBTC);
};

/**
 * Returns a Unit instance created from an amount in bits
 *
 * @param {Number} amount - The amount in bits
 * @returns {Unit} A Unit instance
 */
Unit.fromMicros = Unit.fromBits = function (amount)
{
    return new Unit(amount, Unit.bits);
};

/**
 * Returns a Unit instance created from an amount in satoshis
 *
 * @param {Number} amount - The amount in satoshis
 * @returns {Unit} A Unit instance
 */
Unit.fromSatoshis = function (amount)
{
    return new Unit(amount, Unit.satoshis);
};

/**
 * Returns a Unit instance created from a fiat amount and exchange rate.
 *
 * @param {Number} amount - The amount in fiat
 * @param {Number} rate - The exchange rate BTC/fiat
 * @returns {Unit} A Unit instance
 */
Unit.fromFiat = function (amount, rate)
{
    return new Unit(amount, rate);
};

Unit.prototype._from = function (amount, code)
{
    if (!UNITS[code])
    {
        throw new errors.Unit.UnknownCode(code);
    }
    return parseInt((amount * UNITS[code][0]).toFixed());
};

/**
 * Returns the value represented in the specified unit
 *
 * @param {String|Number} code - The unit code or exchange rate
 * @returns {Number} The converted value
 */
Unit.prototype.to = function (code)
{
    if (_.isNumber(code))
    {
        if (code <= 0)
        {
            throw new errors.Unit.InvalidRate(code);
        }
        return parseFloat((this.BTC * code).toFixed(2));
    }

    if (!UNITS[code])
    {
        throw new errors.Unit.UnknownCode(code);
    }

    var value = this._value / UNITS[code][0];
    return parseFloat(value.toFixed(UNITS[code][1]));
};

/**
 * Returns the value represented in BTC
 *
 * @returns {Number} The value converted to BTC
 */
Unit.prototype.toBTC = function ()
{
    return this.to(Unit.BTC);
};

/**
 * Returns the value represented in mBTC
 *
 * @returns {Number} The value converted to mBTC
 */
Unit.prototype.toMillis = Unit.prototype.toMilis = function ()
{
    return this.to(Unit.mBTC);
};

/**
 * Returns the value represented in bits
 *
 * @returns {Number} The value converted to bits
 */
Unit.prototype.toMicros = Unit.prototype.toBits = function ()
{
    return this.to(Unit.bits);
};

/**
 * Returns the value represented in satoshis
 *
 * @returns {Number} The value converted to satoshis
 */
Unit.prototype.toSatoshis = function ()
{
    return this.to(Unit.satoshis);
};

/**
 * Returns the value represented in fiat
 *
 * @param {string} rate - The exchange rate between BTC/currency
 * @returns {Number} The value converted to satoshis
 */
Unit.prototype.atRate = function (rate)
{
    return this.to(rate);
};

/**
 * Returns a the string representation of the value in satoshis
 *
 * @returns {string} the value in satoshis
 */
Unit.prototype.toString = function ()
{
    return this.satoshis + ' satoshis';
};

/**
 * Returns a plain object representation of the Unit
 *
 * @returns {Object} An object with the keys: amount and code
 */
Unit.prototype.toObject = Unit.prototype.toJSON = function toObject()
{
    return {
        amount: this.BTC,
        code: Unit.BTC
    };
};

/**
 * Returns a string formatted for the console
 *
 * @returns {string} the value in satoshis
 */
Unit.prototype.inspect = function ()
{
    return '<Unit: ' + this.toString() + '>';
};

magnachain.Unit = Unit;
// unit end----------------------------------------------------------------------

// safe buffer begin----------------------------------------------------------------------
/* eslint-disable node/no-deprecated-api */
//var Buffer = buffer.Buffer

var safe_buffer = function ()
{ };

// alternative to using Object.keys for old browsers
function copyProps(src, dst)
{
    for (var key in src)
    {
        dst[key] = src[key]
    }
}
if (Buffer.from && Buffer.alloc && Buffer.allocUnsafe && Buffer.allocUnsafeSlow)
{
    safe_buffer = Buffer
} else
{
    // Copy properties from require('buffer')
    copyProps(Buffer, SafeBuffer)
    safe_buffer = SafeBuffer
}

function SafeBuffer(arg, encodingOrOffset, length)
{
    return Buffer(arg, encodingOrOffset, length)
}

// Copy static methods from Buffer
copyProps(Buffer, SafeBuffer)

SafeBuffer.from = function (arg, encodingOrOffset, length)
{
    if (typeof arg === 'number')
    {
        throw new TypeError('Argument must not be a number')
    }
    return Buffer(arg, encodingOrOffset, length)
}

SafeBuffer.alloc = function (size, fill, encoding)
{
    if (typeof size !== 'number')
    {
        throw new TypeError('Argument must be a number')
    }
    var buf = Buffer(size)
    if (fill !== undefined)
    {
        if (typeof encoding === 'string')
        {
            buf.fill(fill, encoding)
        } else
        {
            buf.fill(fill)
        }
    } else
    {
        buf.fill(0)
    }
    return buf
}

SafeBuffer.allocUnsafe = function (size)
{
    if (typeof size !== 'number')
    {
        throw new TypeError('Argument must be a number')
    }
    return Buffer(size)
}

SafeBuffer.allocUnsafeSlow = function (size)
{
    if (typeof size !== 'number')
    {
        throw new TypeError('Argument must be a number')
    }
    //return buffer.SlowBuffer(size)
    return SlowBuffer(size);
}
// safe buffer end----------------------------------------------------------------------

// base-x begin----------------------------------------------------------------------
// base-x encoding
// Forked from https://github.com/cryptocoinjs/bs58
// Originally written by Mike Hearn for BitcoinJ
// Copyright (c) 2011 Google Inc
// Ported to JavaScript by Stefan Thomas
// Merged Buffer refactorings from base58-native by Stephen Pair
// Copyright (c) 2013 BitPay Inc

//var safe_buffer = require('safe-buffer').Buffer
//var _safebuff = safe_buffer.Buffer;
var _safebuff = safe_buffer;

var basex = function (ALPHABET)
{
    var ALPHABET_MAP = {}
    var BASE = ALPHABET.length
    var LEADER = ALPHABET.charAt(0)

    // pre-compute lookup table
    for (var z = 0; z < ALPHABET.length; z++)
    {
        var x = ALPHABET.charAt(z)

        if (ALPHABET_MAP[x] !== undefined) throw new TypeError(x + ' is ambiguous')
        ALPHABET_MAP[x] = z
    }

    function encode(source)
    {
        if (source.length === 0) return ''

        var digits = [0]
        for (var i = 0; i < source.length; ++i)
        {
            for (var j = 0, carry = source[i]; j < digits.length; ++j)
            {
                carry += digits[j] << 8
                digits[j] = carry % BASE
                carry = (carry / BASE) | 0
            }

            while (carry > 0)
            {
                digits.push(carry % BASE)
                carry = (carry / BASE) | 0
            }
        }

        var string = ''

        // deal with leading zeros
        for (var k = 0; source[k] === 0 && k < source.length - 1; ++k) string += LEADER
        // convert digits to a string
        for (var q = digits.length - 1; q >= 0; --q) string += ALPHABET[digits[q]]

        return string
    }

    function decodeUnsafe(string)
    {
        if (typeof string !== 'string') throw new TypeError('Expected String')
        if (string.length === 0) return _safebuff.allocUnsafe(0)

        var bytes = [0]
        for (var i = 0; i < string.length; i++)
        {
            var value = ALPHABET_MAP[string[i]]
            if (value === undefined) return

            for (var j = 0, carry = value; j < bytes.length; ++j)
            {
                carry += bytes[j] * BASE
                bytes[j] = carry & 0xff
                carry >>= 8
            }

            while (carry > 0)
            {
                bytes.push(carry & 0xff)
                carry >>= 8
            }
        }

        // deal with leading zeros
        for (var k = 0; string[k] === LEADER && k < string.length - 1; ++k)
        {
            bytes.push(0)
        }

        return _safebuff.from(bytes.reverse())
    }

    function decode(string)
    {
        var buffer = decodeUnsafe(string)
        if (buffer) return buffer

        throw new Error('Non-base' + BASE + ' character')
    }

    return {
        encode: encode,
        decodeUnsafe: decodeUnsafe,
        decode: decode
    }
}
// base-x end----------------------------------------------------------------------

// bs58 begin----------------------------------------------------------------------
//var basex = require('base-x')
var ALPHABET = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'

var bs58 = basex(ALPHABET)
// bs58 end----------------------------------------------------------------------

// encoding/base58 begin----------------------------------------------------------------------
//var _ = require('lodash');
//var bs58 = require('bs58');
//var buffer = require('buffer');

var ALPHABET2 = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'.split('');

var Base58 = function Base58(obj)
{
    /* jshint maxcomplexity: 8 */
    if (!(this instanceof Base58))
    {
        return new Base58(obj);
    }
    if (Buffer.isBuffer(obj))
    {
        var buf = obj;
        this.fromBuffer(buf);
    } else if (typeof obj === 'string')
    {
        var str = obj;
        this.fromString(str);
    } else if (obj)
    {
        this.set(obj);
    }
};

Base58.validCharacters = function validCharacters(chars)
{
    if (Buffer.isBuffer(chars))
    {
        chars = chars.toString();
    }
    return _.every(_.map(chars, function (char) { return _.includes(ALPHABET2, char); }));
};

Base58.prototype.set = function (obj)
{
    this.buf = obj.buf || this.buf || undefined;
    return this;
};

Base58.encode = function (buf)
{
    if (!Buffer.isBuffer(buf))
    {
        throw new Error('Input should be a buffer');
    }
    return bs58.encode(buf);
};

Base58.decode = function (str)
{
    if (typeof str !== 'string')
    {
        throw new Error('Input should be a string');
    }
    return new Buffer(bs58.decode(str));
};

Base58.prototype.fromBuffer = function (buf)
{
    this.buf = buf;
    return this;
};

Base58.prototype.fromString = function (str)
{
    var buf = Base58.decode(str);
    this.buf = buf;
    return this;
};

Base58.prototype.toBuffer = function ()
{
    return this.buf;
};

Base58.prototype.toString = function ()
{
    return Base58.encode(this.buf);
};

magnachain.Base58 = Base58;
// encoding/base58 end----------------------------------------------------------------------

// encoding/base58check begin----------------------------------------------------------------------
// var _ = require('lodash');
// var Base58 = require('./base58');
// var buffer = require('buffer');
//var sha256sha256 = require('../crypto/hash').sha256sha256;
var sha256sha256 = Hash.sha256sha256;

var Base58Check = function Base58Check(obj)
{
    if (!(this instanceof Base58Check))
        return new Base58Check(obj);
    if (Buffer.isBuffer(obj))
    {
        var buf = obj;
        this.fromBuffer(buf);
    } else if (typeof obj === 'string')
    {
        var str = obj;
        this.fromString(str);
    } else if (obj)
    {
        this.set(obj);
    }
};

Base58Check.prototype.set = function (obj)
{
    this.buf = obj.buf || this.buf || undefined;
    return this;
};

Base58Check.validChecksum = function validChecksum(data, checksum)
{
    if (_.isString(data))
    {
        data = new Buffer(Base58.decode(data));
    }
    if (_.isString(checksum))
    {
        checksum = new Buffer(Base58.decode(checksum));
    }
    if (!checksum)
    {
        checksum = data.slice(-4);
        data = data.slice(0, -4);
    }
    return Base58Check.checksum(data).toString('hex') === checksum.toString('hex');
};

Base58Check.decode = function (s)
{
    if (typeof s !== 'string')
        throw new Error('Input must be a string');

    var buf = new Buffer(Base58.decode(s));

    if (buf.length < 4)
        throw new Error("Input string too short");

    var data = buf.slice(0, -4);
    var csum = buf.slice(-4);

    var hash = sha256sha256(data);
    var hash4 = hash.slice(0, 4);

    if (csum.toString('hex') !== hash4.toString('hex'))
        throw new Error("Checksum mismatch");

    return data;
};

Base58Check.checksum = function (buffer)
{
    return sha256sha256(buffer).slice(0, 4);
};

Base58Check.encode = function (buf)
{
    if (!Buffer.isBuffer(buf))
        throw new Error('Input must be a buffer');
    var checkedBuf = new Buffer(buf.length + 4);
    var hash = Base58Check.checksum(buf);
    buf.copy(checkedBuf);
    hash.copy(checkedBuf, buf.length);
    return Base58.encode(checkedBuf);
};

Base58Check.prototype.fromBuffer = function (buf)
{
    this.buf = buf;
    return this;
};

Base58Check.prototype.fromString = function (str)
{
    var buf = Base58Check.decode(str);
    this.buf = buf;
    return this;
};

Base58Check.prototype.toBuffer = function ()
{
    return this.buf;
};

Base58Check.prototype.toString = function ()
{
    return Base58Check.encode(this.buf);
};

magnachain.Base58Check = Base58Check;
// encoding/base58check end----------------------------------------------------------------------

// encoding/bufferreader begin----------------------------------------------------------------------
// var _ = require('lodash');
// var $ = require('../util/preconditions');
// var BufferUtil = require('../util/buffer');
// var BN = require('../crypto/bn');

var BufferReader = function BufferReader(buf)
{
    if (!(this instanceof BufferReader))
    {
        return new BufferReader(buf);
    }
    if (_.isUndefined(buf))
    {
        return;
    }
    if (Buffer.isBuffer(buf))
    {
        this.set({
            buf: buf
        });
    } else if (_.isString(buf))
    {
        this.set({
            buf: new Buffer(buf, 'hex'),
        });
    } else if (_.isObject(buf))
    {
        var obj = buf;
        this.set(obj);
    } else
    {
        throw new TypeError('Unrecognized argument for BufferReader');
    }
};

BufferReader.prototype.set = function (obj)
{
    this.buf = obj.buf || this.buf || undefined;
    this.pos = obj.pos || this.pos || 0;
    return this;
};

BufferReader.prototype.eof = function ()
{
    return this.pos >= this.buf.length;
};

BufferReader.prototype.finished = BufferReader.prototype.eof;

BufferReader.prototype.read = function (len)
{
    $.checkArgument(!_.isUndefined(len), 'Must specify a length');
    var buf = this.buf.slice(this.pos, this.pos + len);
    this.pos = this.pos + len;
    return buf;
};

BufferReader.prototype.readAll = function ()
{
    var buf = this.buf.slice(this.pos, this.buf.length);
    this.pos = this.buf.length;
    return buf;
};

BufferReader.prototype.readUInt8 = function ()
{
    var val = this.buf.readUInt8(this.pos);
    this.pos = this.pos + 1;
    return val;
};

BufferReader.prototype.readUInt16BE = function ()
{
    var val = this.buf.readUInt16BE(this.pos);
    this.pos = this.pos + 2;
    return val;
};

BufferReader.prototype.readUInt16LE = function ()
{
    var val = this.buf.readUInt16LE(this.pos);
    this.pos = this.pos + 2;
    return val;
};

BufferReader.prototype.readUInt32BE = function ()
{
    var val = this.buf.readUInt32BE(this.pos);
    this.pos = this.pos + 4;
    return val;
};

BufferReader.prototype.readUInt32LE = function ()
{
    var val = this.buf.readUInt32LE(this.pos);
    this.pos = this.pos + 4;
    return val;
};

BufferReader.prototype.readInt32LE = function ()
{
    var val = this.buf.readInt32LE(this.pos);
    this.pos = this.pos + 4;
    return val;
};

BufferReader.prototype.readUInt64BEBN = function ()
{
    var buf = this.buf.slice(this.pos, this.pos + 8);
    var bn = BN.fromBuffer(buf);
    this.pos = this.pos + 8;
    return bn;
};

BufferReader.prototype.readUInt64LEBN = function ()
{
    var second = this.buf.readUInt32LE(this.pos);
    var first = this.buf.readUInt32LE(this.pos + 4);
    var combined = (first * 0x100000000) + second;
    // Instantiating an instance of BN with a number is faster than with an
    // array or string. However, the maximum safe number for a double precision
    // floating point is 2 ^ 52 - 1 (0x1fffffffffffff), thus we can safely use
    // non-floating point numbers less than this amount (52 bits). And in the case
    // that the number is larger, we can instatiate an instance of BN by passing
    // an array from the buffer (slower) and specifying the endianness.
    var bn;
    if (combined <= 0x1fffffffffffff)
    {
        bn = new BN(combined);
    } else
    {
        var data = Array.prototype.slice.call(this.buf, this.pos, this.pos + 8);
        bn = new BN(data, 10, 'le');
    }
    this.pos = this.pos + 8;
    return bn;
};

BufferReader.prototype.readVarintNum = function ()
{
    var first = this.readUInt8();
    switch (first)
    {
        case 0xFD:
            return this.readUInt16LE();
        case 0xFE:
            return this.readUInt32LE();
        case 0xFF:
            var bn = this.readUInt64LEBN();
            var n = bn.toNumber();
            if (n <= Math.pow(2, 53))
            {
                return n;
            } else
            {
                throw new Error('number too large to retain precision - use readVarintBN');
            }
            break;
        default:
            return first;
    }
};

/**
 * reads a length prepended buffer
 */
BufferReader.prototype.readVarLengthBuffer = function ()
{
    var len = this.readVarintNum();
    var buf = this.read(len);
    $.checkState(buf.length === len, 'Invalid length while reading varlength buffer. ' +
        'Expected to read: ' + len + ' and read ' + buf.length);
    return buf;
};

BufferReader.prototype.readVarintBuf = function ()
{
    var first = this.buf.readUInt8(this.pos);
    switch (first)
    {
        case 0xFD:
            return this.read(1 + 2);
        case 0xFE:
            return this.read(1 + 4);
        case 0xFF:
            return this.read(1 + 8);
        default:
            return this.read(1);
    }
};

BufferReader.prototype.readVarintBN = function ()
{
    var first = this.readUInt8();
    switch (first)
    {
        case 0xFD:
            return new BN(this.readUInt16LE());
        case 0xFE:
            return new BN(this.readUInt32LE());
        case 0xFF:
            return this.readUInt64LEBN();
        default:
            return new BN(first);
    }
};

BufferReader.prototype.reverse = function ()
{
    var buf = new Buffer(this.buf.length);
    for (var i = 0; i < buf.length; i++)
    {
        buf[i] = this.buf[this.buf.length - 1 - i];
    }
    this.buf = buf;
    return this;
};

BufferReader.prototype.readReverse = function (len)
{
    if (_.isUndefined(len))
    {
        len = this.buf.length;
    }
    var buf = this.buf.slice(this.pos, this.pos + len);
    this.pos = this.pos + len;
    return BufferUtil.reverse(buf);
};

magnachain.BufferReader = BufferReader;
// encoding/bufferreader end----------------------------------------------------------------------

// encoding/bufferwriter begin----------------------------------------------------------------------
//ar BufferUtil = require('../util/buffer');
//var assert = require('assert');

var BufferWriter = function BufferWriter(obj)
{
    if (!(this instanceof BufferWriter))
        return new BufferWriter(obj);
    if (obj)
        this.set(obj);
    else
        this.bufs = [];
};

BufferWriter.prototype.set = function (obj)
{
    this.bufs = obj.bufs || this.bufs || [];
    return this;
};

BufferWriter.prototype.toBuffer = function ()
{
    return this.concat();
};

BufferWriter.prototype.concat = function ()
{
    return Buffer.concat(this.bufs);
};

BufferWriter.prototype.write = function (buf)
{
    assert(BufferUtil.isBuffer(buf));
    this.bufs.push(buf);
    return this;
};

BufferWriter.prototype.writeReverse = function (buf)
{
    assert(BufferUtil.isBuffer(buf));
    this.bufs.push(BufferUtil.reverse(buf));
    return this;
};

BufferWriter.prototype.writeUInt8 = function (n)
{
    var buf = new Buffer(1);
    buf.writeUInt8(n, 0);
    this.write(buf);
    return this;
};

BufferWriter.prototype.writeUInt16BE = function (n)
{
    var buf = new Buffer(2);
    buf.writeUInt16BE(n, 0);
    this.write(buf);
    return this;
};

BufferWriter.prototype.writeUInt16LE = function (n)
{
    var buf = new Buffer(2);
    buf.writeUInt16LE(n, 0);
    this.write(buf);
    return this;
};

BufferWriter.prototype.writeUInt32BE = function (n)
{
    var buf = new Buffer(4);
    buf.writeUInt32BE(n, 0);
    this.write(buf);
    return this;
};

BufferWriter.prototype.writeInt32LE = function (n)
{
    var buf = new Buffer(4);
    buf.writeInt32LE(n, 0);
    this.write(buf);
    return this;
};

BufferWriter.prototype.writeUInt32LE = function (n)
{
    var buf = new Buffer(4);
    buf.writeUInt32LE(n, 0);
    this.write(buf);
    return this;
};

BufferWriter.prototype.writeUInt64BEBN = function (bn)
{
    var buf = bn.toBuffer({ size: 8 });
    this.write(buf);
    return this;
};

BufferWriter.prototype.writeUInt64LEBN = function (bn)
{
    var buf = bn.toBuffer({ size: 8 });
    this.writeReverse(buf);
    return this;
};

BufferWriter.prototype.writeVarintNum = function (n)
{
    var buf = BufferWriter.varintBufNum(n);
    this.write(buf);
    return this;
};

BufferWriter.prototype.writeVarintBN = function (bn)
{
    var buf = BufferWriter.varintBufBN(bn);
    this.write(buf);
    return this;
};

BufferWriter.varintBufNum = function (n)
{
    var buf = undefined;
    if (n < 253)
    {
        buf = new Buffer(1);
        buf.writeUInt8(n, 0);
    } else if (n < 0x10000)
    {
        buf = new Buffer(1 + 2);
        buf.writeUInt8(253, 0);
        buf.writeUInt16LE(n, 1);
    } else if (n < 0x100000000)
    {
        buf = new Buffer(1 + 4);
        buf.writeUInt8(254, 0);
        buf.writeUInt32LE(n, 1);
    } else
    {
        buf = new Buffer(1 + 8);
        buf.writeUInt8(255, 0);
        buf.writeInt32LE(n & -1, 1);
        buf.writeUInt32LE(Math.floor(n / 0x100000000), 5);
    }
    return buf;
};

BufferWriter.varintBufBN = function (bn)
{
    var buf = undefined;
    var n = bn.toNumber();
    if (n < 253)
    {
        buf = new Buffer(1);
        buf.writeUInt8(n, 0);
    } else if (n < 0x10000)
    {
        buf = new Buffer(1 + 2);
        buf.writeUInt8(253, 0);
        buf.writeUInt16LE(n, 1);
    } else if (n < 0x100000000)
    {
        buf = new Buffer(1 + 4);
        buf.writeUInt8(254, 0);
        buf.writeUInt32LE(n, 1);
    } else
    {
        var bw = new BufferWriter();
        bw.writeUInt8(255);
        bw.writeUInt64LEBN(bn);
        var buf = bw.concat();
    }
    return buf;
};

magnachain.BufferWriter = BufferWriter;
// encoding/bufferwriter end----------------------------------------------------------------------

// encoding/varint begin----------------------------------------------------------------------
// var BufferWriter = require('./bufferwriter');
// var BufferReader = require('./bufferreader');
// var BN = require('../crypto/bn');

var Varint = function Varint(buf)
{
    if (!(this instanceof Varint))
        return new Varint(buf);
    if (Buffer.isBuffer(buf))
    {
        this.buf = buf;
    } else if (typeof buf === 'number')
    {
        var num = buf;
        this.fromNumber(num);
    } else if (buf instanceof BN)
    {
        var bn = buf;
        this.fromBN(bn);
    } else if (buf)
    {
        var obj = buf;
        this.set(obj);
    }
};

Varint.prototype.set = function (obj)
{
    this.buf = obj.buf || this.buf;
    return this;
};

Varint.prototype.fromString = function (str)
{
    this.set({
        buf: new Buffer(str, 'hex')
    });
    return this;
};

Varint.prototype.toString = function ()
{
    return this.buf.toString('hex');
};

Varint.prototype.fromBuffer = function (buf)
{
    this.buf = buf;
    return this;
};

Varint.prototype.fromBufferReader = function (br)
{
    this.buf = br.readVarintBuf();
    return this;
};

Varint.prototype.fromBN = function (bn)
{
    this.buf = BufferWriter().writeVarintBN(bn).concat();
    return this;
};

Varint.prototype.fromNumber = function (num)
{
    this.buf = BufferWriter().writeVarintNum(num).concat();
    return this;
};

Varint.prototype.toBuffer = function ()
{
    return this.buf;
};

Varint.prototype.toBN = function ()
{
    return BufferReader(this.buf).readVarintBN();
};

Varint.prototype.toNumber = function ()
{
    return BufferReader(this.buf).readVarintNum();
};

//module.exports = Varint;
magnachain.Varint = Varint;
// encoding/varint end----------------------------------------------------------------------

// address begin----------------------------------------------------------------------
// var _ = require('lodash');
// var $ = require('./util/preconditions');
// var errors = require('./errors');
// var Base58Check = require('./encoding/base58check');
// var Networks = require('./networks');
// var Hash = require('./crypto/hash');
// var JSUtil = require('./util/js');
// var PublicKey = require('./publickey');

/**
 * Instantiate an address from an address String or Buffer, a public key or script hash Buffer,
 * or an instance of {@link PublicKey} or {@link Script}.
 *
 * This is an immutable class, and if the first parameter provided to this constructor is an
 * `Address` instance, the same argument will be returned.
 *
 * An address has two key properties: `network` and `type`. The type is either
 * `Address.PayToPublicKeyHash` (value is the `'pubkeyhash'` string)
 * or `Address.PayToScriptHash` (the string `'scripthash'`). The network is an instance of {@link Network}.
 * You can quickly check whether an address is of a given kind by using the methods
 * `isPayToPublicKeyHash` and `isPayToScriptHash`
 *
 * @example
 * ```javascript
 * // validate that an input field is valid
 * var error = Address.getValidationError(input, 'testnet');
 * if (!error) {
 *   var address = Address(input, 'testnet');
 * } else {
 *   // invalid network or checksum (typo?)
 *   var message = error.messsage;
 * }
 *
 * // get an address from a public key
 * var address = Address(publicKey, 'testnet').toString();
 * ```
 *
 * @param {*} data - The encoded data in various formats
 * @param {Network|String|number=} network - The network: 'livenet' or 'testnet'
 * @param {string=} type - The type of address: 'script' or 'pubkey'
 * @returns {Address} A new valid and frozen instance of an Address
 * @constructor
 */
function Address(data, network, type)
{
    /* jshint maxcomplexity: 12 */
    /* jshint maxstatements: 20 */

    if (!(this instanceof Address))
    {
        return new Address(data, network, type);
    }

    if (_.isArray(data) && _.isNumber(network))
    {
        return Address.createMultisig(data, network, type);
    }

    if (data instanceof Address)
    {
        // Immutable instance
        return data;
    }

    $.checkArgument(data, 'First argument is required, please include address data.', 'guide/address.html');

    if (network && !Networks.get(network))
    {
        throw new TypeError('Second argument must be "livenet" or "testnet".');
    }

    if (type && (type !== Address.PayToPublicKeyHash && type !== Address.PayToScriptHash && type !== Address.PayToContractHash))
    {
        throw new TypeError('Third argument must be "pubkeyhash" or "scripthash".');
    }

    var info = this._classifyArguments(data, network, type);

    // set defaults if not set
    info.network = info.network || Networks.get(network) || Networks.defaultNetwork;
    info.type = info.type || type || Address.PayToPublicKeyHash;

    JSUtil.defineImmutable(this, {
        hashBuffer: info.hashBuffer,
        network: info.network,
        type: info.type
    });

    return this;
}

/**
 * Internal function used to split different kinds of arguments of the constructor
 * @param {*} data - The encoded data in various formats
 * @param {Network|String|number=} network - The network: 'livenet' or 'testnet'
 * @param {string=} type - The type of address: 'script' or 'pubkey'
 * @returns {Object} An "info" object with "type", "network", and "hashBuffer"
 */
Address.prototype._classifyArguments = function (data, network, type)
{
    /* jshint maxcomplexity: 10 */
    // transform and validate input data
    if ((data instanceof Buffer || data instanceof Uint8Array) && data.length === 20)
    {
        return Address._transformHash(data);
    } else if ((data instanceof Buffer || data instanceof Uint8Array) && data.length === 21)
    {
        return Address._transformBuffer(data, network, type);
    } else if (data instanceof PublicKey)
    {
        return Address._transformPublicKey(data);
    } else if (data instanceof Script)
    {
        return Address._transformScript(data, network);
    } else if (typeof (data) === 'string')
    {
        return Address._transformString(data, network, type);
    } else if (_.isObject(data))
    {
        return Address._transformObject(data);
    } else
    {
        throw new TypeError('First argument is an unrecognized data format.');
    }
};

/** @static */
Address.PayToPublicKeyHash = 'pubkeyhash';
/** @static */
Address.PayToContractHash = 'contracthash';
/** @static */
Address.PayToScriptHash = 'scripthash';

/**
 * @param {Buffer} hash - An instance of a hash Buffer
 * @returns {Object} An object with keys: hashBuffer
 * @private
 */
Address._transformHash = function (hash)
{
    var info = {};
    if (!(hash instanceof Buffer) && !(hash instanceof Uint8Array))
    {
        throw new TypeError('Address supplied is not a buffer.');
    }
    if (hash.length !== 20)
    {
        throw new TypeError('Address hashbuffers must be exactly 20 bytes.');
    }
    info.hashBuffer = hash;
    return info;
};

/**
 * Deserializes an address serialized through `Address#toObject()`
 * @param {Object} data
 * @param {string} data.hash - the hash that this address encodes
 * @param {string} data.type - either 'pubkeyhash' or 'scripthash'
 * @param {Network=} data.network - the name of the network associated
 * @return {Address}
 */
Address._transformObject = function (data)
{
    $.checkArgument(data.hash || data.hashBuffer, 'Must provide a `hash` or `hashBuffer` property');
    $.checkArgument(data.type, 'Must provide a `type` property');
    return {
        hashBuffer: data.hash ? new Buffer(data.hash, 'hex') : data.hashBuffer,
        network: Networks.get(data.network) || Networks.defaultNetwork,
        type: data.type
    };
};

/**
 * Internal function to discover the network and type based on the first data byte
 *
 * @param {Buffer} buffer - An instance of a hex encoded address Buffer
 * @returns {Object} An object with keys: network and type
 * @private
 */
Address._classifyFromVersion = function (buffer)
{
    var version = {};

    var pubkeyhashNetwork = Networks.get(buffer[0], 'pubkeyhash');
    var scripthashNetwork = Networks.get(buffer[0], 'scripthash');
    var contracthashNetwork = Networks.get(buffer[0], 'contracthash');;

    if (pubkeyhashNetwork)
    {
        version.network = pubkeyhashNetwork;
        version.type = Address.PayToPublicKeyHash;
    } else if (scripthashNetwork)
    {
        version.network = scripthashNetwork;
        version.type = Address.PayToScriptHash;
    } else if (contracthashNetwork)
    {
        version.network = contracthashNetwork;
        version.type = Address.PayToContractHash;
    }

    return version;
};

/**
 * Internal function to transform a bitcoin address buffer
 *
 * @param {Buffer} buffer - An instance of a hex encoded address Buffer
 * @param {string=} network - The network: 'livenet' or 'testnet'
 * @param {string=} type - The type: 'pubkeyhash' or 'scripthash'
 * @returns {Object} An object with keys: hashBuffer, network and type
 * @private
 */
Address._transformBuffer = function (buffer, network, type)
{
    /* jshint maxcomplexity: 9 */
    var info = {};
    if (!(buffer instanceof Buffer) && !(buffer instanceof Uint8Array))
    {
        throw new TypeError('Address supplied is not a buffer.');
    }
    if (buffer.length !== 1 + 20)
    {
        throw new TypeError('Address buffers must be exactly 21 bytes.');
    }

    var networkObj = Networks.get(network);
    var bufferVersion = Address._classifyFromVersion(buffer);

    if (network && !networkObj)
    {
        throw new TypeError('Unknown network');
    }

    if (!bufferVersion.network || (networkObj && networkObj !== bufferVersion.network))
    {
        throw new TypeError('Address has mismatched network type.');
    }

    if (!bufferVersion.type || (type && type !== bufferVersion.type))
    {
        throw new TypeError('Address has mismatched type.');
    }

    info.hashBuffer = buffer.slice(1);
    info.network = bufferVersion.network;
    info.type = bufferVersion.type;
    return info;
};

/**
 * Internal function to transform a {@link PublicKey}
 *
 * @param {PublicKey} pubkey - An instance of PublicKey
 * @returns {Object} An object with keys: hashBuffer, type
 * @private
 */
Address._transformPublicKey = function (pubkey)
{
    var info = {};
    if (!(pubkey instanceof PublicKey))
    {
        throw new TypeError('Address must be an instance of PublicKey.');
    }
    info.hashBuffer = Hash.sha256ripemd160(pubkey.toBuffer());
    info.type = Address.PayToPublicKeyHash;
    return info;
};

/**
 * Internal function to transform a {@link Script} into a `info` object.
 *
 * @param {Script} script - An instance of Script
 * @returns {Object} An object with keys: hashBuffer, type
 * @private
 */
Address._transformScript = function (script, network)
{
    $.checkArgument(script instanceof Script, 'script must be a Script instance');
    var info = script.getAddressInfo(network);
    if (!info)
    {
        throw new errors.Script.CantDeriveAddress(script);
    }
    return info;
};

//

/**
 * Creates a P2SH address from a set of public keys and a threshold.
 *
 * The addresses will be sorted lexicographically, as that is the trend in bitcoin.
 * To create an address from unsorted public keys, use the {@link Script#buildMultisigOut}
 * interface.
 *
 * @param {Array} publicKeys - a set of public keys to create an address
 * @param {number} threshold - the number of signatures needed to release the funds
 * @param {String|Network} network - either a Network instance, 'livenet', or 'testnet'
 * @return {Address}
 */
Address.createMultisig = function (publicKeys, threshold, network)
{
    network = network || publicKeys[0].network || Networks.defaultNetwork;
    return Address.payingTo(Script.buildMultisigOut(publicKeys, threshold), network);
};

/**
 * Internal function to transform a bitcoin address string
 *
 * @param {string} data
 * @param {String|Network=} network - either a Network instance, 'livenet', or 'testnet'
 * @param {string=} type - The type: 'pubkeyhash' or 'scripthash'
 * @returns {Object} An object with keys: hashBuffer, network and type
 * @private
 */
Address._transformString = function (data, network, type)
{
    if (typeof (data) !== 'string')
    {
        throw new TypeError('data parameter supplied is not a string.');
    }
    data = data.trim();
    var addressBuffer = Base58Check.decode(data);
    var info = Address._transformBuffer(addressBuffer, network, type);
    return info;
};

/**
 * Instantiate an address from a PublicKey instance
 *
 * @param {PublicKey} data
 * @param {String|Network} network - either a Network instance, 'livenet', or 'testnet'
 * @returns {Address} A new valid and frozen instance of an Address
 */
Address.fromPublicKey = function (data, network)
{
    var info = Address._transformPublicKey(data);
    network = network || Networks.defaultNetwork;
    return new Address(info.hashBuffer, network, info.type);
};

/**
 * Instantiate an address from a ripemd160 public key hash
 *
 * @param {Buffer} hash - An instance of buffer of the hash
 * @param {String|Network} network - either a Network instance, 'livenet', or 'testnet'
 * @returns {Address} A new valid and frozen instance of an Address
 */
Address.fromPublicKeyHash = function (hash, network)
{
    var info = Address._transformHash(hash);
    return new Address(info.hashBuffer, network, Address.PayToPublicKeyHash);
};

/**
 * Instantiate an address from a ripemd160 script hash
 *
 * @param {Buffer} hash - An instance of buffer of the hash
 * @param {String|Network} network - either a Network instance, 'livenet', or 'testnet'
 * @returns {Address} A new valid and frozen instance of an Address
 */
Address.fromScriptHash = function (hash, network)
{
    $.checkArgument(hash, 'hash parameter is required');
    var info = Address._transformHash(hash);
    return new Address(info.hashBuffer, network, Address.PayToScriptHash);
};

/**
 * Builds a p2sh address paying to script. This will hash the script and
 * use that to create the address.
 * If you want to extract an address associated with a script instead,
 * see {{Address#fromScript}}
 *
 * @param {Script} script - An instance of Script
 * @param {String|Network} network - either a Network instance, 'livenet', or 'testnet'
 * @returns {Address} A new valid and frozen instance of an Address
 */
Address.payingTo = function (script, network)
{
    $.checkArgument(script, 'script is required');
    $.checkArgument(script instanceof Script, 'script must be instance of Script');

    return Address.fromScriptHash(Hash.sha256ripemd160(script.toBuffer()), network);
};

/**
 * Extract address from a Script. The script must be of one
 * of the following types: p2pkh input, p2pkh output, p2sh input
 * or p2sh output.
 * This will analyze the script and extract address information from it.
 * If you want to transform any script to a p2sh Address paying
 * to that script's hash instead, use {{Address#payingTo}}
 *
 * @param {Script} script - An instance of Script
 * @param {String|Network} network - either a Network instance, 'livenet', or 'testnet'
 * @returns {Address} A new valid and frozen instance of an Address
 */
Address.fromScript = function (script, network)
{
    $.checkArgument(script instanceof Script, 'script must be a Script instance');
    var info = Address._transformScript(script, network);
    return new Address(info.hashBuffer, network, info.type);
};

/**
 * Instantiate an address from a buffer of the address
 *
 * @param {Buffer} buffer - An instance of buffer of the address
 * @param {String|Network=} network - either a Network instance, 'livenet', or 'testnet'
 * @param {string=} type - The type of address: 'script' or 'pubkey'
 * @returns {Address} A new valid and frozen instance of an Address
 */
Address.fromBuffer = function (buffer, network, type)
{
    var info = Address._transformBuffer(buffer, network, type);
    return new Address(info.hashBuffer, info.network, info.type);
};

/**
 * Instantiate an address from an address string
 *
 * @param {string} str - An string of the bitcoin address
 * @param {String|Network=} network - either a Network instance, 'livenet', or 'testnet'
 * @param {string=} type - The type of address: 'script' or 'pubkey'
 * @returns {Address} A new valid and frozen instance of an Address
 */
Address.fromString = function (str, network, type)
{
    var info = Address._transformString(str, network, type);
    return new Address(info.hashBuffer, info.network, info.type);
};

/**
 * Instantiate an address from an Object
 *
 * @param {string} json - An JSON string or Object with keys: hash, network and type
 * @returns {Address} A new valid instance of an Address
 */
Address.fromObject = function fromObject(obj)
{
    $.checkState(
        JSUtil.isHexa(obj.hash),
        'Unexpected hash property, "' + obj.hash + '", expected to be hex.'
    );
    var hashBuffer = new Buffer(obj.hash, 'hex');
    return new Address(hashBuffer, obj.network, obj.type);
};

/**
 * Will return a validation error if exists
 *
 * @example
 * ```javascript
 * // a network mismatch error
 * var error = Address.getValidationError('15vkcKf7gB23wLAnZLmbVuMiiVDc1Nm4a2', 'testnet');
 * ```
 *
 * @param {string} data - The encoded data
 * @param {String|Network} network - either a Network instance, 'livenet', or 'testnet'
 * @param {string} type - The type of address: 'script' or 'pubkey'
 * @returns {null|Error} The corresponding error message
 */
Address.getValidationError = function (data, network, type)
{
    var error;
    try
    {
        /* jshint nonew: false */
        new Address(data, network, type);
    } catch (e)
    {
        error = e;
    }
    return error;
};

/**
 * Will return a boolean if an address is valid
 *
 * @example
 * ```javascript
 * assert(Address.isValid('15vkcKf7gB23wLAnZLmbVuMiiVDc1Nm4a2', 'livenet'));
 * ```
 *
 * @param {string} data - The encoded data
 * @param {String|Network} network - either a Network instance, 'livenet', or 'testnet'
 * @param {string} type - The type of address: 'script' or 'pubkey'
 * @returns {boolean} The corresponding error message
 */
Address.isValid = function (data, network, type)
{
    return !Address.getValidationError(data, network, type);
};

/**
 * Returns true if an address is of pay to public key hash type
 * @return boolean
 */
Address.prototype.isPayToPublicKeyHash = function ()
{
    return this.type === Address.PayToPublicKeyHash;
};

/**
 * Returns true if an address is of pay to script hash type
 * @return boolean
 */
Address.prototype.isPayToScriptHash = function ()
{
    return this.type === Address.PayToScriptHash;
};

/**
 * Will return a buffer representation of the address
 *
 * @returns {Buffer} Bitcoin address buffer
 */
Address.prototype.toBuffer = function ()
{
    var version = new Buffer([this.network[this.type]]);
    var buf = Buffer.concat([version, this.hashBuffer]);
    return buf;
};

/**
 * @returns {Object} A plain object with the address information
 */
Address.prototype.toObject = Address.prototype.toJSON = function toObject()
{
    return {
        hash: this.hashBuffer.toString('hex'),
        type: this.type,
        network: this.network.toString()
    };
};

/**
 * Will return a the string representation of the address
 *
 * @returns {string} Bitcoin address
 */
Address.prototype.toString = function ()
{
    return Base58Check.encode(this.toBuffer());
};

/**
 * Will return a string formatted for the console
 *
 * @returns {string} Bitcoin address
 */
Address.prototype.inspect = function ()
{
    return '<Address: ' + this.toString() + ', type: ' + this.type + ', network: ' + this.network + '>';
};

//module.exports = Address;
magnachain.Address = Address;

//var Script = require('./script');
// address end----------------------------------------------------------------------

// privatekey begin----------------------------------------------------------------------
// var _ = require('lodash');
// var Address = require('./address');
// var Base58Check = require('./encoding/base58check');
// var BN = require('./crypto/bn');
// var JSUtil = require('./util/js');
// var Networks = require('./networks');
// var Point = require('./crypto/point');
// var PublicKey = require('./publickey');
// var Random = require('./crypto/random');
// var $ = require('./util/preconditions');

/**
 * Instantiate a PrivateKey from a BN, Buffer and WIF.
 *
 * @example
 * ```javascript
 * // generate a new random key
 * var key = PrivateKey();
 *
 * // get the associated address
 * var address = key.toAddress();
 *
 * // encode into wallet export format
 * var exported = key.toWIF();
 *
 * // instantiate from the exported (and saved) private key
 * var imported = PrivateKey.fromWIF(exported);
 * ```
 *
 * @param {string} data - The encoded data in various formats
 * @param {Network|string=} network - a {@link Network} object, or a string with the network name
 * @returns {PrivateKey} A new valid instance of an PrivateKey
 * @constructor
 */
function PrivateKey(data, network)
{
    /* jshint maxstatements: 20 */
    /* jshint maxcomplexity: 8 */

    if (!(this instanceof PrivateKey))
    {
        return new PrivateKey(data, network);
    }
    if (data instanceof PrivateKey)
    {
        return data;
    }

    var info = this._classifyArguments(data, network);

    // validation
    if (!info.bn || info.bn.cmp(new BN(0)) === 0)
    {
        throw new TypeError('Number can not be equal to zero, undefined, null or false');
    }
    if (!info.bn.lt(Point.getN()))
    {
        throw new TypeError('Number must be less than N');
    }
    if (typeof (info.network) === 'undefined')
    {
        throw new TypeError('Must specify the network ("livenet" or "testnet")');
    }

    JSUtil.defineImmutable(this, {
        bn: info.bn,
        compressed: info.compressed,
        network: info.network
    });

    Object.defineProperty(this, 'publicKey', {
        configurable: false,
        enumerable: true,
        get: this.toPublicKey.bind(this)
    });

    return this;

};

/**
 * Internal helper to instantiate PrivateKey internal `info` object from
 * different kinds of arguments passed to the constructor.
 *
 * @param {*} data
 * @param {Network|string=} network - a {@link Network} object, or a string with the network name
 * @return {Object}
 */
PrivateKey.prototype._classifyArguments = function (data, network)
{
    /* jshint maxcomplexity: 10 */
    var info = {
        compressed: true,
        network: network ? Networks.get(network) : Networks.defaultNetwork
    };
    //console.log("pppp");

    // detect type of data
    if (_.isUndefined(data) || _.isNull(data))
    {
        info.bn = PrivateKey._getRandomBN();
        //console.log("KKK: " + info.bn);
    } else if (data instanceof BN)
    {
        info.bn = data;
    } else if (data instanceof Buffer || data instanceof Uint8Array)
    {
        info = PrivateKey._transformBuffer(data, network);
    } else if (data.bn && data.network)
    {
        info = PrivateKey._transformObject(data);
    } else if (!network && Networks.get(data))
    {
        info.bn = PrivateKey._getRandomBN();
        info.network = Networks.get(data);
    } else if (typeof (data) === 'string')
    {
        if (JSUtil.isHexa(data))
        {
            info.bn = new BN(new Buffer(data, 'hex'));
        } else
        {
            info = PrivateKey._transformWIF(data, network);
        }
    } else
    {
        throw new TypeError('First argument is an unrecognized data type.');
    }
    return info;
};

/**
 * Internal function to get a random Big Number (BN)
 *
 * @returns {BN} A new randomly generated BN
 * @private
 */
PrivateKey._getRandomBN = function ()
{
    var condition;
    var bn;
    do
    {
        var privbuf = Random.getRandomBuffer(32);
        bn = BN.fromBuffer(privbuf);
        condition = bn.lt(Point.getN());
    } while (!condition);
    return bn;
};

/**
 * Internal function to transform a WIF Buffer into a private key
 *
 * @param {Buffer} buf - An WIF string
 * @param {Network|string=} network - a {@link Network} object, or a string with the network name
 * @returns {Object} An object with keys: bn, network and compressed
 * @private
 */
PrivateKey._transformBuffer = function (buf, network)
{

    var info = {};

    if (buf.length === 32)
    {
        return PrivateKey._transformBNBuffer(buf, network);
    }

    info.network = Networks.get(buf[0], 'privatekey');

    if (!info.network)
    {
        throw new Error('Invalid network');
    }

    if (network && info.network !== Networks.get(network))
    {
        throw new TypeError('Private key network mismatch');
    }

    if (buf.length === 1 + 32 + 1 && buf[1 + 32 + 1 - 1] === 1)
    {
        info.compressed = true;
    } else if (buf.length === 1 + 32)
    {
        info.compressed = false;
    } else
    {
        throw new Error('Length of buffer must be 33 (uncompressed) or 34 (compressed)');
    }

    info.bn = BN.fromBuffer(buf.slice(1, 32 + 1));

    return info;
};

/**
 * Internal function to transform a BN buffer into a private key
 *
 * @param {Buffer} buf
 * @param {Network|string=} network - a {@link Network} object, or a string with the network name
 * @returns {object} an Object with keys: bn, network, and compressed
 * @private
 */
PrivateKey._transformBNBuffer = function (buf, network)
{
    var info = {};
    info.network = Networks.get(network) || Networks.defaultNetwork;
    info.bn = BN.fromBuffer(buf);
    info.compressed = false;
    return info;
};

/**
 * Internal function to transform a WIF string into a private key
 *
 * @param {string} buf - An WIF string
 * @returns {Object} An object with keys: bn, network and compressed
 * @private
 */
PrivateKey._transformWIF = function (str, network)
{
    return PrivateKey._transformBuffer(Base58Check.decode(str), network);
};

/**
 * Instantiate a PrivateKey from a Buffer with the DER or WIF representation
 *
 * @param {Buffer} arg
 * @param {Network} network
 * @return {PrivateKey}
 */
PrivateKey.fromBuffer = function (arg, network)
{
    return new PrivateKey(arg, network);
};

/**
 * Internal function to transform a JSON string on plain object into a private key
 * return this.
 *
 * @param {string} json - A JSON string or plain object
 * @returns {Object} An object with keys: bn, network and compressed
 * @private
 */
PrivateKey._transformObject = function (json)
{
    var bn = new BN(json.bn, 'hex');
    var network = Networks.get(json.network);
    return {
        bn: bn,
        network: network,
        compressed: json.compressed
    };
};

/**
 * Instantiate a PrivateKey from a WIF string
 *
 * @param {string} str - The WIF encoded private key string
 * @returns {PrivateKey} A new valid instance of PrivateKey
 */
PrivateKey.fromString = PrivateKey.fromWIF = function (str)
{
    $.checkArgument(_.isString(str), 'First argument is expected to be a string.');
    return new PrivateKey(str);
};

/**
 * Instantiate a PrivateKey from a plain JavaScript object
 *
 * @param {Object} obj - The output from privateKey.toObject()
 */
PrivateKey.fromObject = function (obj)
{
    $.checkArgument(_.isObject(obj), 'First argument is expected to be an object.');
    return new PrivateKey(obj);
};

/**
 * Instantiate a PrivateKey from random bytes
 *
 * @param {string=} network - Either "livenet" or "testnet"
 * @returns {PrivateKey} A new valid instance of PrivateKey
 */
PrivateKey.fromRandom = function (network)
{
    var bn = PrivateKey._getRandomBN();
    return new PrivateKey(bn, network);
};

/**
 * Check if there would be any errors when initializing a PrivateKey
 *
 * @param {string} data - The encoded data in various formats
 * @param {string=} network - Either "livenet" or "testnet"
 * @returns {null|Error} An error if exists
 */

PrivateKey.getValidationError = function (data, network)
{
    var error;
    try
    {
        /* jshint nonew: false */
        new PrivateKey(data, network);
    } catch (e)
    {
        error = e;
    }
    return error;
};

/**
 * Check if the parameters are valid
 *
 * @param {string} data - The encoded data in various formats
 * @param {string=} network - Either "livenet" or "testnet"
 * @returns {Boolean} If the private key is would be valid
 */
PrivateKey.isValid = function (data, network)
{
    if (!data)
    {
        return false;
    }
    return !PrivateKey.getValidationError(data, network);
};

/**
 * Will output the PrivateKey encoded as hex string
 *
 * @returns {string}
 */
PrivateKey.prototype.toString = function ()
{
    return this.toBuffer().toString('hex');
};

/**
 * Will output the PrivateKey to a WIF string
 *
 * @returns {string} A WIP representation of the private key
 */
PrivateKey.prototype.toWIF = function ()
{
    var network = this.network;
    var compressed = this.compressed;

    var buf;
    if (compressed)
    {
        buf = Buffer.concat([new Buffer([network.privatekey]),
        this.bn.toBuffer({ size: 32 }),
        new Buffer([0x01])]);
    } else
    {
        buf = Buffer.concat([new Buffer([network.privatekey]),
        this.bn.toBuffer({ size: 32 })]);
    }

    return Base58Check.encode(buf);
};

/**
 * Will return the private key as a BN instance
 *
 * @returns {BN} A BN instance of the private key
 */
PrivateKey.prototype.toBigNumber = function ()
{
    return this.bn;
};

/**
 * Will return the private key as a BN buffer
 *
 * @returns {Buffer} A buffer of the private key
 */
PrivateKey.prototype.toBuffer = function ()
{
    // TODO: use `return this.bn.toBuffer({ size: 32 })` in v1.0.0
    return this.bn.toBuffer();
};

/**
 * WARNING: This method will not be officially supported until v1.0.0.
 *
 *
 * Will return the private key as a BN buffer without leading zero padding
 *
 * @returns {Buffer} A buffer of the private key
 */
PrivateKey.prototype.toBufferNoPadding = function ()
{
    return this.bn.toBuffer();
};

/**
 * Will return the corresponding public key
 *
 * @returns {PublicKey} A public key generated from the private key
 */
PrivateKey.prototype.toPublicKey = function ()
{
    if (!this._pubkey)
    {
        this._pubkey = PublicKey.fromPrivateKey(this);
    }
    return this._pubkey;
};

/**
 * Will return an address for the private key
 * @param {Network=} network - optional parameter specifying
 * the desired network for the address
 *
 * @returns {Address} An address generated from the private key
 */
PrivateKey.prototype.toAddress = function (network)
{
    var pubkey = this.toPublicKey();
    return Address.fromPublicKey(pubkey, network || this.network);
};

/**
 * @returns {Object} A plain object representation
 */
PrivateKey.prototype.toObject = PrivateKey.prototype.toJSON = function toObject()
{
    return {
        bn: this.bn.toString('hex'),
        compressed: this.compressed,
        network: this.network.toString()
    };
};

/**
 * Will return a string formatted for the console
 *
 * @returns {string} Private key
 */
PrivateKey.prototype.inspect = function ()
{
    var uncompressed = !this.compressed ? ', uncompressed' : '';
    return '<PrivateKey: ' + this.toString() + ', network: ' + this.network + uncompressed + '>';
};

//module.exports = PrivateKey;
magnachain.PrivateKey = PrivateKey;
// privatekey end----------------------------------------------------------------------

// hdprivatekey begin----------------------------------------------------------------------
// var assert = require('assert');
// var buffer = require('buffer');
// var _ = require('lodash');
// var $ = require('./util/preconditions');

// var BN = require('./crypto/bn');
// var Base58 = require('./encoding/base58');
// var Base58Check = require('./encoding/base58check');
// var Hash = require('./crypto/hash');
// var Networks = require('./networks');
// var Point = require('./crypto/point');
// var PrivateKey = require('./privatekey');
// var Random = require('./crypto/random');

// var errors = require('./errors');
var hdPrivateKeyErrors = errors.HDPrivateKey;
//var BufferUtil = require('./util/buffer');
//var JSUtil = require('./util/js');

var MINIMUM_ENTROPY_BITS = 128;
var BITS_TO_BYTES = 1 / 8;
var MAXIMUM_ENTROPY_BITS = 512;

/**
 * Represents an instance of an hierarchically derived private key.
 *
 * More info on https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki
 *
 * @constructor
 * @param {string|Buffer|Object} arg
 */
function HDPrivateKey(arg)
{
    /* jshint maxcomplexity: 10 */
    if (arg instanceof HDPrivateKey)
    {
        return arg;
    }
    if (!(this instanceof HDPrivateKey))
    {
        return new HDPrivateKey(arg);
    }
    if (!arg)
    {
        return this._generateRandomly();
    }

    if (Networks.get(arg))
    {
        return this._generateRandomly(arg);
    } else if (_.isString(arg) || BufferUtil.isBuffer(arg))
    {
        if (HDPrivateKey.isValidSerialized(arg))
        {
            this._buildFromSerialized(arg);
        } else if (JSUtil.isValidJSON(arg))
        {
            this._buildFromJSON(arg);
        } else if (BufferUtil.isBuffer(arg) && HDPrivateKey.isValidSerialized(arg.toString()))
        {
            this._buildFromSerialized(arg.toString());
        } else
        {
            throw HDPrivateKey.getSerializedError(arg);
        }
    } else if (_.isObject(arg))
    {
        this._buildFromObject(arg);
    } else
    {
        throw new hdPrivateKeyErrors.UnrecognizedArgument(arg);
    }
}

/**
 * Verifies that a given path is valid.
 *
 * @param {string|number} arg
 * @param {boolean?} hardened
 * @return {boolean}
 */
HDPrivateKey.isValidPath = function (arg, hardened)
{
    if (_.isString(arg))
    {
        var indexes = HDPrivateKey._getDerivationIndexes(arg);
        return indexes !== null && _.every(indexes, HDPrivateKey.isValidPath);
    }

    if (_.isNumber(arg))
    {
        if (arg < HDPrivateKey.Hardened && hardened === true)
        {
            arg += HDPrivateKey.Hardened;
        }
        return arg >= 0 && arg < HDPrivateKey.MaxIndex;
    }

    return false;
};

/**
 * Internal function that splits a string path into a derivation index array.
 * It will return null if the string path is malformed.
 * It does not validate if indexes are in bounds.
 *
 * @param {string} path
 * @return {Array}
 */
HDPrivateKey._getDerivationIndexes = function (path)
{
    var steps = path.split('/');

    // Special cases:
    if (_.includes(HDPrivateKey.RootElementAlias, path))
    {
        return [];
    }

    if (!_.includes(HDPrivateKey.RootElementAlias, steps[0]))
    {
        return null;
    }

    var indexes = steps.slice(1).map(function (step)
    {
        var isHardened = step.slice(-1) === '\'';
        if (isHardened)
        {
            step = step.slice(0, -1);
        }
        if (!step || step[0] === '-')
        {
            return NaN;
        }
        var index = +step; // cast to number
        if (isHardened)
        {
            index += HDPrivateKey.Hardened;
        }

        return index;
    });

    return _.some(indexes, isNaN) ? null : indexes;
};

/**
 * WARNING: This method is deprecated. Use deriveChild or deriveNonCompliantChild instead. This is not BIP32 compliant
 *
 *
 * Get a derived child based on a string or number.
 *
 * If the first argument is a string, it's parsed as the full path of
 * derivation. Valid values for this argument include "m" (which returns the
 * same private key), "m/0/1/40/2'/1000", where the ' quote means a hardened
 * derivation.
 *
 * If the first argument is a number, the child with that index will be
 * derived. If the second argument is truthy, the hardened version will be
 * derived. See the example usage for clarification.
 *
 * @example
 * ```javascript
 * var parent = new HDPrivateKey('xprv...');
 * var child_0_1_2h = parent.derive(0).derive(1).derive(2, true);
 * var copy_of_child_0_1_2h = parent.derive("m/0/1/2'");
 * assert(child_0_1_2h.xprivkey === copy_of_child_0_1_2h);
 * ```
 *
 * @param {string|number} arg
 * @param {boolean?} hardened
 */
HDPrivateKey.prototype.derive = function (arg, hardened)
{
    return this.deriveNonCompliantChild(arg, hardened);
};

/**
 * WARNING: This method will not be officially supported until v1.0.0.
 *
 *
 * Get a derived child based on a string or number.
 *
 * If the first argument is a string, it's parsed as the full path of
 * derivation. Valid values for this argument include "m" (which returns the
 * same private key), "m/0/1/40/2'/1000", where the ' quote means a hardened
 * derivation.
 *
 * If the first argument is a number, the child with that index will be
 * derived. If the second argument is truthy, the hardened version will be
 * derived. See the example usage for clarification.
 *
 * WARNING: The `nonCompliant` option should NOT be used, except for older implementation
 * that used a derivation strategy that used a non-zero padded private key.
 *
 * @example
 * ```javascript
 * var parent = new HDPrivateKey('xprv...');
 * var child_0_1_2h = parent.deriveChild(0).deriveChild(1).deriveChild(2, true);
 * var copy_of_child_0_1_2h = parent.deriveChild("m/0/1/2'");
 * assert(child_0_1_2h.xprivkey === copy_of_child_0_1_2h);
 * ```
 *
 * @param {string|number} arg
 * @param {boolean?} hardened
 */
HDPrivateKey.prototype.deriveChild = function (arg, hardened)
{
    if (_.isNumber(arg))
    {
        return this._deriveWithNumber(arg, hardened);
    } else if (_.isString(arg))
    {
        return this._deriveFromString(arg);
    } else
    {
        throw new hdPrivateKeyErrors.InvalidDerivationArgument(arg);
    }
};

/**
 * WARNING: This method will not be officially supported until v1.0.0
 *
 *
 * WARNING: If this is a new implementation you should NOT use this method, you should be using
 * `derive` instead.
 *
 * This method is explicitly for use and compatibility with an implementation that
 * was not compliant with BIP32 regarding the derivation algorithm. The private key
 * must be 32 bytes hashing, and this implementation will use the non-zero padded
 * serialization of a private key, such that it's still possible to derive the privateKey
 * to recover those funds.
 *
 * @param {string|number} arg
 * @param {boolean?} hardened
 */
HDPrivateKey.prototype.deriveNonCompliantChild = function (arg, hardened)
{
    if (_.isNumber(arg))
    {
        return this._deriveWithNumber(arg, hardened, true);
    } else if (_.isString(arg))
    {
        return this._deriveFromString(arg, true);
    } else
    {
        throw new hdPrivateKeyErrors.InvalidDerivationArgument(arg);
    }
};

HDPrivateKey.prototype._deriveWithNumber = function (index, hardened, nonCompliant)
{
    /* jshint maxstatements: 20 */
    /* jshint maxcomplexity: 10 */
    if (!HDPrivateKey.isValidPath(index, hardened))
    {
        throw new hdPrivateKeyErrors.InvalidPath(index);
    }

    hardened = index >= HDPrivateKey.Hardened ? true : hardened;
    if (index < HDPrivateKey.Hardened && hardened === true)
    {
        index += HDPrivateKey.Hardened;
    }

    var indexBuffer = BufferUtil.integerAsBuffer(index);
    var data;
    if (hardened && nonCompliant)
    {
        // The private key serialization in this case will not be exactly 32 bytes and can be
        // any value less, and the value is not zero-padded.
        var nonZeroPadded = this.privateKey.bn.toBuffer();
        data = BufferUtil.concat([new Buffer([0]), nonZeroPadded, indexBuffer]);
    } else if (hardened)
    {
        // This will use a 32 byte zero padded serialization of the private key
        var privateKeyBuffer = this.privateKey.bn.toBuffer({ size: 32 });
        assert(privateKeyBuffer.length === 32, 'length of private key buffer is expected to be 32 bytes');
        data = BufferUtil.concat([new Buffer([0]), privateKeyBuffer, indexBuffer]);
    } else
    {
        data = BufferUtil.concat([this.publicKey.toBuffer(), indexBuffer]);
    }
    var hash = Hash.sha512hmac(data, this._buffers.chainCode);
    var leftPart = BN.fromBuffer(hash.slice(0, 32), {
        size: 32
    });
    var chainCode = hash.slice(32, 64);

    var privateKey = leftPart.add(this.privateKey.toBigNumber()).umod(Point.getN()).toBuffer({
        size: 32
    });

    if (!PrivateKey.isValid(privateKey))
    {
        // Index at this point is already hardened, we can pass null as the hardened arg
        return this._deriveWithNumber(index + 1, null, nonCompliant);
    }

    var derived = new HDPrivateKey({
        network: this.network,
        depth: this.depth + 1,
        parentFingerPrint: this.fingerPrint,
        childIndex: index,
        chainCode: chainCode,
        privateKey: privateKey
    });

    return derived;
};

HDPrivateKey.prototype._deriveFromString = function (path, nonCompliant)
{
    if (!HDPrivateKey.isValidPath(path))
    {
        throw new hdPrivateKeyErrors.InvalidPath(path);
    }

    var indexes = HDPrivateKey._getDerivationIndexes(path);
    var derived = indexes.reduce(function (prev, index)
    {
        return prev._deriveWithNumber(index, null, nonCompliant);
    }, this);

    return derived;
};

/**
 * Verifies that a given serialized private key in base58 with checksum format
 * is valid.
 *
 * @param {string|Buffer} data - the serialized private key
 * @param {string|Network=} network - optional, if present, checks that the
 *     network provided matches the network serialized.
 * @return {boolean}
 */
HDPrivateKey.isValidSerialized = function (data, network)
{
    return !HDPrivateKey.getSerializedError(data, network);
};

/**
 * Checks what's the error that causes the validation of a serialized private key
 * in base58 with checksum to fail.
 *
 * @param {string|Buffer} data - the serialized private key
 * @param {string|Network=} network - optional, if present, checks that the
 *     network provided matches the network serialized.
 * @return {errors.InvalidArgument|null}
 */
HDPrivateKey.getSerializedError = function (data, network)
{
    /* jshint maxcomplexity: 10 */
    if (!(_.isString(data) || BufferUtil.isBuffer(data)))
    {
        return new hdPrivateKeyErrors.UnrecognizedArgument('Expected string or buffer');
    }
    if (!Base58.validCharacters(data))
    {
        return new errors.InvalidB58Char('(unknown)', data);
    }
    try
    {
        data = Base58Check.decode(data);
    } catch (e)
    {
        return new errors.InvalidB58Checksum(data);
    }
    if (data.length !== HDPrivateKey.DataLength)
    {
        return new hdPrivateKeyErrors.InvalidLength(data);
    }
    if (!_.isUndefined(network))
    {
        var error = HDPrivateKey._validateNetwork(data, network);
        if (error)
        {
            return error;
        }
    }
    return null;
};

HDPrivateKey._validateNetwork = function (data, networkArg)
{
    var network = Networks.get(networkArg);
    if (!network)
    {
        return new errors.InvalidNetworkArgument(networkArg);
    }
    var version = data.slice(0, 4);
    if (BufferUtil.integerFromBuffer(version) !== network.xprivkey)
    {
        return new errors.InvalidNetwork(version);
    }
    return null;
};

HDPrivateKey.fromString = function (arg)
{
    $.checkArgument(_.isString(arg), 'No valid string was provided');
    return new HDPrivateKey(arg);
};

HDPrivateKey.fromObject = function (arg)
{
    $.checkArgument(_.isObject(arg), 'No valid argument was provided');
    return new HDPrivateKey(arg);
};

HDPrivateKey.prototype._buildFromJSON = function (arg)
{
    return this._buildFromObject(JSON.parse(arg));
};

HDPrivateKey.prototype._buildFromObject = function (arg)
{
    /* jshint maxcomplexity: 12 */
    // TODO: Type validation
    var buffers = {
        version: arg.network ? BufferUtil.integerAsBuffer(Networks.get(arg.network).xprivkey) : arg.version,
        depth: _.isNumber(arg.depth) ? BufferUtil.integerAsSingleByteBuffer(arg.depth) : arg.depth,
        parentFingerPrint: _.isNumber(arg.parentFingerPrint) ? BufferUtil.integerAsBuffer(arg.parentFingerPrint) : arg.parentFingerPrint,
        childIndex: _.isNumber(arg.childIndex) ? BufferUtil.integerAsBuffer(arg.childIndex) : arg.childIndex,
        chainCode: _.isString(arg.chainCode) ? BufferUtil.hexToBuffer(arg.chainCode) : arg.chainCode,
        privateKey: (_.isString(arg.privateKey) && JSUtil.isHexa(arg.privateKey)) ? BufferUtil.hexToBuffer(arg.privateKey) : arg.privateKey,
        checksum: arg.checksum ? (arg.checksum.length ? arg.checksum : BufferUtil.integerAsBuffer(arg.checksum)) : undefined
    };
    return this._buildFromBuffers(buffers);
};

HDPrivateKey.prototype._buildFromSerialized = function (arg)
{
    var decoded = Base58Check.decode(arg);
    var buffers = {
        version: decoded.slice(HDPrivateKey.VersionStart, HDPrivateKey.VersionEnd),
        depth: decoded.slice(HDPrivateKey.DepthStart, HDPrivateKey.DepthEnd),
        parentFingerPrint: decoded.slice(HDPrivateKey.ParentFingerPrintStart,
            HDPrivateKey.ParentFingerPrintEnd),
        childIndex: decoded.slice(HDPrivateKey.ChildIndexStart, HDPrivateKey.ChildIndexEnd),
        chainCode: decoded.slice(HDPrivateKey.ChainCodeStart, HDPrivateKey.ChainCodeEnd),
        privateKey: decoded.slice(HDPrivateKey.PrivateKeyStart, HDPrivateKey.PrivateKeyEnd),
        checksum: decoded.slice(HDPrivateKey.ChecksumStart, HDPrivateKey.ChecksumEnd),
        xprivkey: arg
    };
    return this._buildFromBuffers(buffers);
};

HDPrivateKey.prototype._generateRandomly = function (network)
{
    //console.log("XXXX: " + network);
    return HDPrivateKey.fromSeed(Random.getRandomBuffer(64), network);
};

/**
 * Generate a private key from a seed, as described in BIP32
 *
 * @param {string|Buffer} hexa
 * @param {*} network
 * @return HDPrivateKey
 */
HDPrivateKey.fromSeed = function (hexa, network)
{
    //console.log("PPPPP: " + hexa);

    /* jshint maxcomplexity: 8 */
    if (JSUtil.isHexaString(hexa))
    {
        hexa = BufferUtil.hexToBuffer(hexa);
    }
    if (!Buffer.isBuffer(hexa))
    {
        throw new hdPrivateKeyErrors.InvalidEntropyArgument(hexa);
    }
    if (hexa.length < MINIMUM_ENTROPY_BITS * BITS_TO_BYTES)
    {
        throw new hdPrivateKeyErrors.InvalidEntropyArgument.NotEnoughEntropy(hexa);
    }
    if (hexa.length > MAXIMUM_ENTROPY_BITS * BITS_TO_BYTES)
    {
        throw new hdPrivateKeyErrors.InvalidEntropyArgument.TooMuchEntropy(hexa);
    }
    var hash = Hash.sha512hmac(hexa, new Buffer('oh@god#celllinkF-s*e%ed'));

    //console.log("KKKKK: " + hash);

    return new HDPrivateKey({
        network: Networks.get(network) || Networks.defaultNetwork,
        depth: 0,
        parentFingerPrint: 0,
        childIndex: 0,
        privateKey: hash.slice(0, 32),
        chainCode: hash.slice(32, 64)
    });
};

HDPrivateKey.fromMnemonicWord = function (strWords, network)
{
    var buf = Buffer._fromString(strWords);
    var arrR = Hash.sha512(buf);
    var buf2 = Buffer._fromArrayBuffer(arrR, 0, arrR.length);

    return HDPrivateKey.fromSeed(buf2, network);
};

HDPrivateKey.prototype._calcHDPublicKey = function ()
{
    if (!this._hdPublicKey)
    {
        //var HDPublicKey = require('./hdpublickey');
        this._hdPublicKey = new HDPublicKey(this);
    }
};

/**
 * Receives a object with buffers in all the properties and populates the
 * internal structure
 *
 * @param {Object} arg
 * @param {buffer.Buffer} arg.version
 * @param {buffer.Buffer} arg.depth
 * @param {buffer.Buffer} arg.parentFingerPrint
 * @param {buffer.Buffer} arg.childIndex
 * @param {buffer.Buffer} arg.chainCode
 * @param {buffer.Buffer} arg.privateKey
 * @param {buffer.Buffer} arg.checksum
 * @param {string=} arg.xprivkey - if set, don't recalculate the base58
 *      representation
 * @return {HDPrivateKey} this
 */
HDPrivateKey.prototype._buildFromBuffers = function (arg)
{
    /* jshint maxcomplexity: 8 */
    /* jshint maxstatements: 20 */

    HDPrivateKey._validateBufferArguments(arg);

    JSUtil.defineImmutable(this, {
        _buffers: arg
    });

    var sequence = [
        arg.version, arg.depth, arg.parentFingerPrint, arg.childIndex, arg.chainCode,
        BufferUtil.emptyBuffer(1), arg.privateKey
    ];
    var concat = Buffer.concat(sequence);
    if (!arg.checksum || !arg.checksum.length)
    {
        arg.checksum = Base58Check.checksum(concat);
    } else
    {
        if (arg.checksum.toString() !== Base58Check.checksum(concat).toString())
        {
            throw new errors.InvalidB58Checksum(concat);
        }
    }

    var network = Networks.get(BufferUtil.integerFromBuffer(arg.version));
    var xprivkey;
    xprivkey = Base58Check.encode(Buffer.concat(sequence));
    arg.xprivkey = new Buffer(xprivkey);

    var privateKey = new PrivateKey(BN.fromBuffer(arg.privateKey), network);
    var publicKey = privateKey.toPublicKey();
    var size = HDPrivateKey.ParentFingerPrintSize;
    var fingerPrint = Hash.sha256ripemd160(publicKey.toBuffer()).slice(0, size);

    JSUtil.defineImmutable(this, {
        xprivkey: xprivkey,
        network: network,
        depth: BufferUtil.integerFromSingleByteBuffer(arg.depth),
        privateKey: privateKey,
        publicKey: publicKey,
        fingerPrint: fingerPrint
    });

    this._hdPublicKey = null;

    Object.defineProperty(this, 'hdPublicKey', {
        configurable: false,
        enumerable: true,
        get: function ()
        {
            this._calcHDPublicKey();
            return this._hdPublicKey;
        }
    });
    Object.defineProperty(this, 'xpubkey', {
        configurable: false,
        enumerable: true,
        get: function ()
        {
            this._calcHDPublicKey();
            return this._hdPublicKey.xpubkey;
        }
    });
    return this;
};

HDPrivateKey._validateBufferArguments = function (arg)
{
    var checkBuffer = function (name, size)
    {
        var buff = arg[name];
        assert(BufferUtil.isBuffer(buff), name + ' argument is not a buffer');
        assert(
            buff.length === size,
            name + ' has not the expected size: found ' + buff.length + ', expected ' + size
        );
    };
    checkBuffer('version', HDPrivateKey.VersionSize);
    checkBuffer('depth', HDPrivateKey.DepthSize);
    checkBuffer('parentFingerPrint', HDPrivateKey.ParentFingerPrintSize);
    checkBuffer('childIndex', HDPrivateKey.ChildIndexSize);
    checkBuffer('chainCode', HDPrivateKey.ChainCodeSize);
    checkBuffer('privateKey', HDPrivateKey.PrivateKeySize);
    if (arg.checksum && arg.checksum.length)
    {
        checkBuffer('checksum', HDPrivateKey.CheckSumSize);
    }
};

/**
 * Returns the string representation of this private key (a string starting
 * with "xprv..."
 *
 * @return string
 */
HDPrivateKey.prototype.toString = function ()
{
    return this.xprivkey;
};

/**
 * Returns the console representation of this extended private key.
 * @return string
 */
HDPrivateKey.prototype.inspect = function ()
{
    return '<HDPrivateKey: ' + this.xprivkey + '>';
};

/**
 * Returns a plain object with a representation of this private key.
 *
 * Fields include:<ul>
 * <li> network: either 'livenet' or 'testnet'
 * <li> depth: a number ranging from 0 to 255
 * <li> fingerPrint: a number ranging from 0 to 2^32-1, taken from the hash of the
 * <li>     associated public key
 * <li> parentFingerPrint: a number ranging from 0 to 2^32-1, taken from the hash
 * <li>     of this parent's associated public key or zero.
 * <li> childIndex: the index from which this child was derived (or zero)
 * <li> chainCode: an hexa string representing a number used in the derivation
 * <li> privateKey: the private key associated, in hexa representation
 * <li> xprivkey: the representation of this extended private key in checksum
 * <li>     base58 format
 * <li> checksum: the base58 checksum of xprivkey
 * </ul>
 *  @return {Object}
 */
HDPrivateKey.prototype.toObject = HDPrivateKey.prototype.toJSON = function toObject()
{
    return {
        network: Networks.get(BufferUtil.integerFromBuffer(this._buffers.version), 'xprivkey').name,
        depth: BufferUtil.integerFromSingleByteBuffer(this._buffers.depth),
        fingerPrint: BufferUtil.integerFromBuffer(this.fingerPrint),
        parentFingerPrint: BufferUtil.integerFromBuffer(this._buffers.parentFingerPrint),
        childIndex: BufferUtil.integerFromBuffer(this._buffers.childIndex),
        chainCode: BufferUtil.bufferToHex(this._buffers.chainCode),
        privateKey: this.privateKey.toBuffer().toString('hex'),
        checksum: BufferUtil.integerFromBuffer(this._buffers.checksum),
        xprivkey: this.xprivkey
    };
};

/**
 * Build a HDPrivateKey from a buffer
 *
 * @param {Buffer} arg
 * @return {HDPrivateKey}
 */
HDPrivateKey.fromBuffer = function (arg)
{
    return new HDPrivateKey(arg.toString());
};

/**
 * Returns a buffer representation of the HDPrivateKey
 *
 * @return {string}
 */
HDPrivateKey.prototype.toBuffer = function ()
{
    return BufferUtil.copy(this._buffers.xprivkey);
};

HDPrivateKey.DefaultDepth = 0;
HDPrivateKey.DefaultFingerprint = 0;
HDPrivateKey.DefaultChildIndex = 0;
HDPrivateKey.Hardened = 0x80000000;
HDPrivateKey.MaxIndex = 2 * HDPrivateKey.Hardened;

HDPrivateKey.RootElementAlias = ['m', 'M', 'm\'', 'M\''];

HDPrivateKey.VersionSize = 4;
HDPrivateKey.DepthSize = 1;
HDPrivateKey.ParentFingerPrintSize = 4;
HDPrivateKey.ChildIndexSize = 4;
HDPrivateKey.ChainCodeSize = 32;
HDPrivateKey.PrivateKeySize = 32;
HDPrivateKey.CheckSumSize = 4;

HDPrivateKey.DataLength = 78;
HDPrivateKey.SerializedByteSize = 82;

HDPrivateKey.VersionStart = 0;
HDPrivateKey.VersionEnd = HDPrivateKey.VersionStart + HDPrivateKey.VersionSize;
HDPrivateKey.DepthStart = HDPrivateKey.VersionEnd;
HDPrivateKey.DepthEnd = HDPrivateKey.DepthStart + HDPrivateKey.DepthSize;
HDPrivateKey.ParentFingerPrintStart = HDPrivateKey.DepthEnd;
HDPrivateKey.ParentFingerPrintEnd = HDPrivateKey.ParentFingerPrintStart + HDPrivateKey.ParentFingerPrintSize;
HDPrivateKey.ChildIndexStart = HDPrivateKey.ParentFingerPrintEnd;
HDPrivateKey.ChildIndexEnd = HDPrivateKey.ChildIndexStart + HDPrivateKey.ChildIndexSize;
HDPrivateKey.ChainCodeStart = HDPrivateKey.ChildIndexEnd;
HDPrivateKey.ChainCodeEnd = HDPrivateKey.ChainCodeStart + HDPrivateKey.ChainCodeSize;
HDPrivateKey.PrivateKeyStart = HDPrivateKey.ChainCodeEnd + 1;
HDPrivateKey.PrivateKeyEnd = HDPrivateKey.PrivateKeyStart + HDPrivateKey.PrivateKeySize;
HDPrivateKey.ChecksumStart = HDPrivateKey.PrivateKeyEnd;
HDPrivateKey.ChecksumEnd = HDPrivateKey.ChecksumStart + HDPrivateKey.CheckSumSize;

assert(HDPrivateKey.ChecksumEnd === HDPrivateKey.SerializedByteSize);

magnachain.HDPrivateKey = HDPrivateKey;
// hdprivatekey end----------------------------------------------------------------------

// hdpublickey begin----------------------------------------------------------------------
// var _ = require('lodash');
// var $ = require('./util/preconditions');

// var BN = require('./crypto/bn');
// var Base58 = require('./encoding/base58');
// var Base58Check = require('./encoding/base58check');
// var Hash = require('./crypto/hash');
// var HDPrivateKey = require('./hdprivatekey');
// var Networks = require('./networks');
// var Point = require('./crypto/point');
// var PublicKey = require('./publickey');

// var bitcoreErrors = require('./errors');
//var errors = bitcoreErrors;
var hdPublicKeyErrors = errors.HDPublicKey;
// var assert = require('assert');

// var JSUtil = require('./util/js');
// var BufferUtil = require('./util/buffer');

/**
 * The representation of an hierarchically derived public key.
 *
 * See https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki
 *
 * @constructor
 * @param {Object|string|Buffer} arg
 */
function HDPublicKey(arg)
{
    /* jshint maxcomplexity: 12 */
    /* jshint maxstatements: 20 */
    if (arg instanceof HDPublicKey)
    {
        return arg;
    }
    if (!(this instanceof HDPublicKey))
    {
        return new HDPublicKey(arg);
    }
    if (arg)
    {
        if (_.isString(arg) || BufferUtil.isBuffer(arg))
        {
            var error = HDPublicKey.getSerializedError(arg);
            if (!error)
            {
                return this._buildFromSerialized(arg);
            } else if (BufferUtil.isBuffer(arg) && !HDPublicKey.getSerializedError(arg.toString()))
            {
                return this._buildFromSerialized(arg.toString());
            } else
            {
                if (error instanceof hdPublicKeyErrors.ArgumentIsPrivateExtended)
                {
                    return new HDPrivateKey(arg).hdPublicKey;
                }
                throw error;
            }
        } else
        {
            if (_.isObject(arg))
            {
                if (arg instanceof HDPrivateKey)
                {
                    return this._buildFromPrivate(arg);
                } else
                {
                    return this._buildFromObject(arg);
                }
            } else
            {
                throw new hdPublicKeyErrors.UnrecognizedArgument(arg);
            }
        }
    } else
    {
        throw new hdPublicKeyErrors.MustSupplyArgument();
    }
}

/**
 * Verifies that a given path is valid.
 *
 * @param {string|number} arg
 * @return {boolean}
 */
HDPublicKey.isValidPath = function (arg)
{
    if (_.isString(arg))
    {
        var indexes = HDPrivateKey._getDerivationIndexes(arg);
        return indexes !== null && _.every(indexes, HDPublicKey.isValidPath);
    }

    if (_.isNumber(arg))
    {
        return arg >= 0 && arg < HDPublicKey.Hardened;
    }

    return false;
};

/**
 * WARNING: This method is deprecated. Use deriveChild instead.
 *
 *
 * Get a derivated child based on a string or number.
 *
 * If the first argument is a string, it's parsed as the full path of
 * derivation. Valid values for this argument include "m" (which returns the
 * same public key), "m/0/1/40/2/1000".
 *
 * Note that hardened keys can't be derived from a public extended key.
 *
 * If the first argument is a number, the child with that index will be
 * derived. See the example usage for clarification.
 *
 * @example
 * ```javascript
 * var parent = new HDPublicKey('xpub...');
 * var child_0_1_2 = parent.derive(0).derive(1).derive(2);
 * var copy_of_child_0_1_2 = parent.derive("m/0/1/2");
 * assert(child_0_1_2.xprivkey === copy_of_child_0_1_2);
 * ```
 *
 * @param {string|number} arg
 */
HDPublicKey.prototype.derive = function (arg, hardened)
{
    return this.deriveChild(arg, hardened);
};

/**
 * WARNING: This method will not be officially supported until v1.0.0.
 *
 *
 * Get a derivated child based on a string or number.
 *
 * If the first argument is a string, it's parsed as the full path of
 * derivation. Valid values for this argument include "m" (which returns the
 * same public key), "m/0/1/40/2/1000".
 *
 * Note that hardened keys can't be derived from a public extended key.
 *
 * If the first argument is a number, the child with that index will be
 * derived. See the example usage for clarification.
 *
 * @example
 * ```javascript
 * var parent = new HDPublicKey('xpub...');
 * var child_0_1_2 = parent.deriveChild(0).deriveChild(1).deriveChild(2);
 * var copy_of_child_0_1_2 = parent.deriveChild("m/0/1/2");
 * assert(child_0_1_2.xprivkey === copy_of_child_0_1_2);
 * ```
 *
 * @param {string|number} arg
 */
HDPublicKey.prototype.deriveChild = function (arg, hardened)
{
    if (_.isNumber(arg))
    {
        return this._deriveWithNumber(arg, hardened);
    } else if (_.isString(arg))
    {
        return this._deriveFromString(arg);
    } else
    {
        throw new hdPublicKeyErrors.InvalidDerivationArgument(arg);
    }
};

HDPublicKey.prototype._deriveWithNumber = function (index, hardened)
{
    if (index >= HDPublicKey.Hardened || hardened)
    {
        throw new hdPublicKeyErrors.InvalidIndexCantDeriveHardened();
    }
    if (index < 0)
    {
        throw new hdPublicKeyErrors.InvalidPath(index);
    }

    var indexBuffer = BufferUtil.integerAsBuffer(index);
    var data = BufferUtil.concat([this.publicKey.toBuffer(), indexBuffer]);
    var hash = Hash.sha512hmac(data, this._buffers.chainCode);
    var leftPart = BN.fromBuffer(hash.slice(0, 32), { size: 32 });
    var chainCode = hash.slice(32, 64);

    var publicKey;
    try
    {
        publicKey = PublicKey.fromPoint(Point.getG().mul(leftPart).add(this.publicKey.point));
    } catch (e)
    {
        return this._deriveWithNumber(index + 1);
    }

    var derived = new HDPublicKey({
        network: this.network,
        depth: this.depth + 1,
        parentFingerPrint: this.fingerPrint,
        childIndex: index,
        chainCode: chainCode,
        publicKey: publicKey
    });

    return derived;
};

HDPublicKey.prototype._deriveFromString = function (path)
{
    /* jshint maxcomplexity: 8 */
    if (_.includes(path, "'"))
    {
        throw new hdPublicKeyErrors.InvalidIndexCantDeriveHardened();
    } else if (!HDPublicKey.isValidPath(path))
    {
        throw new hdPublicKeyErrors.InvalidPath(path);
    }

    var indexes = HDPrivateKey._getDerivationIndexes(path);
    var derived = indexes.reduce(function (prev, index)
    {
        return prev._deriveWithNumber(index);
    }, this);

    return derived;
};

/**
 * Verifies that a given serialized public key in base58 with checksum format
 * is valid.
 *
 * @param {string|Buffer} data - the serialized public key
 * @param {string|Network=} network - optional, if present, checks that the
 *     network provided matches the network serialized.
 * @return {boolean}
 */
HDPublicKey.isValidSerialized = function (data, network)
{
    return _.isNull(HDPublicKey.getSerializedError(data, network));
};

/**
 * Checks what's the error that causes the validation of a serialized public key
 * in base58 with checksum to fail.
 *
 * @param {string|Buffer} data - the serialized public key
 * @param {string|Network=} network - optional, if present, checks that the
 *     network provided matches the network serialized.
 * @return {errors|null}
 */
HDPublicKey.getSerializedError = function (data, network)
{
    /* jshint maxcomplexity: 10 */
    /* jshint maxstatements: 20 */
    if (!(_.isString(data) || BufferUtil.isBuffer(data)))
    {
        return new hdPublicKeyErrors.UnrecognizedArgument('expected buffer or string');
    }
    if (!Base58.validCharacters(data))
    {
        return new errors.InvalidB58Char('(unknown)', data);
    }
    try
    {
        data = Base58Check.decode(data);
    } catch (e)
    {
        return new errors.InvalidB58Checksum(data);
    }
    if (data.length !== HDPublicKey.DataSize)
    {
        return new hdPublicKeyErrors.InvalidLength(data);
    }
    if (!_.isUndefined(network))
    {
        var error = HDPublicKey._validateNetwork(data, network);
        if (error)
        {
            return error;
        }
    }
    var version = BufferUtil.integerFromBuffer(data.slice(0, 4));
    if (version === Networks.livenet.xprivkey || version === Networks.testnet.xprivkey)
    {
        return new hdPublicKeyErrors.ArgumentIsPrivateExtended();
    }
    return null;
};

HDPublicKey._validateNetwork = function (data, networkArg)
{
    var network = Networks.get(networkArg);
    if (!network)
    {
        return new errors.InvalidNetworkArgument(networkArg);
    }
    var version = data.slice(HDPublicKey.VersionStart, HDPublicKey.VersionEnd);
    if (BufferUtil.integerFromBuffer(version) !== network.xpubkey)
    {
        return new errors.InvalidNetwork(version);
    }
    return null;
};

HDPublicKey.prototype._buildFromPrivate = function (arg)
{
    var args = _.clone(arg._buffers);
    var point = Point.getG().mul(BN.fromBuffer(args.privateKey));
    args.publicKey = Point.pointToCompressed(point);
    args.version = BufferUtil.integerAsBuffer(Networks.get(BufferUtil.integerFromBuffer(args.version)).xpubkey);
    args.privateKey = undefined;
    args.checksum = undefined;
    args.xprivkey = undefined;
    return this._buildFromBuffers(args);
};

HDPublicKey.prototype._buildFromObject = function (arg)
{
    /* jshint maxcomplexity: 10 */
    // TODO: Type validation
    var buffers = {
        version: arg.network ? BufferUtil.integerAsBuffer(Networks.get(arg.network).xpubkey) : arg.version,
        depth: _.isNumber(arg.depth) ? BufferUtil.integerAsSingleByteBuffer(arg.depth) : arg.depth,
        parentFingerPrint: _.isNumber(arg.parentFingerPrint) ? BufferUtil.integerAsBuffer(arg.parentFingerPrint) : arg.parentFingerPrint,
        childIndex: _.isNumber(arg.childIndex) ? BufferUtil.integerAsBuffer(arg.childIndex) : arg.childIndex,
        chainCode: _.isString(arg.chainCode) ? BufferUtil.hexToBuffer(arg.chainCode) : arg.chainCode,
        publicKey: _.isString(arg.publicKey) ? BufferUtil.hexToBuffer(arg.publicKey) :
            BufferUtil.isBuffer(arg.publicKey) ? arg.publicKey : arg.publicKey.toBuffer(),
        checksum: _.isNumber(arg.checksum) ? BufferUtil.integerAsBuffer(arg.checksum) : arg.checksum
    };
    return this._buildFromBuffers(buffers);
};

HDPublicKey.prototype._buildFromSerialized = function (arg)
{
    var decoded = Base58Check.decode(arg);
    var buffers = {
        version: decoded.slice(HDPublicKey.VersionStart, HDPublicKey.VersionEnd),
        depth: decoded.slice(HDPublicKey.DepthStart, HDPublicKey.DepthEnd),
        parentFingerPrint: decoded.slice(HDPublicKey.ParentFingerPrintStart,
            HDPublicKey.ParentFingerPrintEnd),
        childIndex: decoded.slice(HDPublicKey.ChildIndexStart, HDPublicKey.ChildIndexEnd),
        chainCode: decoded.slice(HDPublicKey.ChainCodeStart, HDPublicKey.ChainCodeEnd),
        publicKey: decoded.slice(HDPublicKey.PublicKeyStart, HDPublicKey.PublicKeyEnd),
        checksum: decoded.slice(HDPublicKey.ChecksumStart, HDPublicKey.ChecksumEnd),
        xpubkey: arg
    };
    return this._buildFromBuffers(buffers);
};

/**
 * Receives a object with buffers in all the properties and populates the
 * internal structure
 *
 * @param {Object} arg
 * @param {buffer.Buffer} arg.version
 * @param {buffer.Buffer} arg.depth
 * @param {buffer.Buffer} arg.parentFingerPrint
 * @param {buffer.Buffer} arg.childIndex
 * @param {buffer.Buffer} arg.chainCode
 * @param {buffer.Buffer} arg.publicKey
 * @param {buffer.Buffer} arg.checksum
 * @param {string=} arg.xpubkey - if set, don't recalculate the base58
 *      representation
 * @return {HDPublicKey} this
 */
HDPublicKey.prototype._buildFromBuffers = function (arg)
{
    /* jshint maxcomplexity: 8 */
    /* jshint maxstatements: 20 */

    HDPublicKey._validateBufferArguments(arg);

    JSUtil.defineImmutable(this, {
        _buffers: arg
    });

    var sequence = [
        arg.version, arg.depth, arg.parentFingerPrint, arg.childIndex, arg.chainCode,
        arg.publicKey
    ];
    var concat = BufferUtil.concat(sequence);
    var checksum = Base58Check.checksum(concat);
    if (!arg.checksum || !arg.checksum.length)
    {
        arg.checksum = checksum;
    } else
    {
        if (arg.checksum.toString('hex') !== checksum.toString('hex'))
        {
            throw new errors.InvalidB58Checksum(concat, checksum);
        }
    }
    var network = Networks.get(BufferUtil.integerFromBuffer(arg.version));

    var xpubkey;
    xpubkey = Base58Check.encode(BufferUtil.concat(sequence));
    arg.xpubkey = new Buffer(xpubkey);

    var publicKey = new PublicKey(arg.publicKey, { network: network });
    var size = HDPublicKey.ParentFingerPrintSize;
    var fingerPrint = Hash.sha256ripemd160(publicKey.toBuffer()).slice(0, size);

    JSUtil.defineImmutable(this, {
        xpubkey: xpubkey,
        network: network,
        depth: BufferUtil.integerFromSingleByteBuffer(arg.depth),
        publicKey: publicKey,
        fingerPrint: fingerPrint
    });

    return this;
};

HDPublicKey._validateBufferArguments = function (arg)
{
    var checkBuffer = function (name, size)
    {
        var buff = arg[name];
        assert(BufferUtil.isBuffer(buff), name + ' argument is not a buffer, it\'s ' + typeof buff);
        assert(
            buff.length === size,
            name + ' has not the expected size: found ' + buff.length + ', expected ' + size
        );
    };
    checkBuffer('version', HDPublicKey.VersionSize);
    checkBuffer('depth', HDPublicKey.DepthSize);
    checkBuffer('parentFingerPrint', HDPublicKey.ParentFingerPrintSize);
    checkBuffer('childIndex', HDPublicKey.ChildIndexSize);
    checkBuffer('chainCode', HDPublicKey.ChainCodeSize);
    checkBuffer('publicKey', HDPublicKey.PublicKeySize);
    if (arg.checksum && arg.checksum.length)
    {
        checkBuffer('checksum', HDPublicKey.CheckSumSize);
    }
};

HDPublicKey.fromString = function (arg)
{
    $.checkArgument(_.isString(arg), 'No valid string was provided');
    return new HDPublicKey(arg);
};

HDPublicKey.fromObject = function (arg)
{
    $.checkArgument(_.isObject(arg), 'No valid argument was provided');
    return new HDPublicKey(arg);
};

/**
 * Returns the base58 checked representation of the public key
 * @return {string} a string starting with "xpub..." in livenet
 */
HDPublicKey.prototype.toString = function ()
{
    return this.xpubkey;
};

/**
 * Returns the console representation of this extended public key.
 * @return string
 */
HDPublicKey.prototype.inspect = function ()
{
    return '<HDPublicKey: ' + this.xpubkey + '>';
};

/**
 * Returns a plain JavaScript object with information to reconstruct a key.
 *
 * Fields are: <ul>
 *  <li> network: 'livenet' or 'testnet'
 *  <li> depth: a number from 0 to 255, the depth to the master extended key
 *  <li> fingerPrint: a number of 32 bits taken from the hash of the public key
 *  <li> fingerPrint: a number of 32 bits taken from the hash of this key's
 *  <li>     parent's public key
 *  <li> childIndex: index with which this key was derived
 *  <li> chainCode: string in hexa encoding used for derivation
 *  <li> publicKey: string, hexa encoded, in compressed key format
 *  <li> checksum: BufferUtil.integerFromBuffer(this._buffers.checksum),
 *  <li> xpubkey: the string with the base58 representation of this extended key
 *  <li> checksum: the base58 checksum of xpubkey
 * </ul>
 */
HDPublicKey.prototype.toObject = HDPublicKey.prototype.toJSON = function toObject()
{
    return {
        network: Networks.get(BufferUtil.integerFromBuffer(this._buffers.version)).name,
        depth: BufferUtil.integerFromSingleByteBuffer(this._buffers.depth),
        fingerPrint: BufferUtil.integerFromBuffer(this.fingerPrint),
        parentFingerPrint: BufferUtil.integerFromBuffer(this._buffers.parentFingerPrint),
        childIndex: BufferUtil.integerFromBuffer(this._buffers.childIndex),
        chainCode: BufferUtil.bufferToHex(this._buffers.chainCode),
        publicKey: this.publicKey.toString(),
        checksum: BufferUtil.integerFromBuffer(this._buffers.checksum),
        xpubkey: this.xpubkey
    };
};

/**
 * Create a HDPublicKey from a buffer argument
 *
 * @param {Buffer} arg
 * @return {HDPublicKey}
 */
HDPublicKey.fromBuffer = function (arg)
{
    return new HDPublicKey(arg);
};

/**
 * Return a buffer representation of the xpubkey
 *
 * @return {Buffer}
 */
HDPublicKey.prototype.toBuffer = function ()
{
    return BufferUtil.copy(this._buffers.xpubkey);
};

HDPublicKey.Hardened = 0x80000000;
HDPublicKey.RootElementAlias = ['m', 'M'];

HDPublicKey.VersionSize = 4;
HDPublicKey.DepthSize = 1;
HDPublicKey.ParentFingerPrintSize = 4;
HDPublicKey.ChildIndexSize = 4;
HDPublicKey.ChainCodeSize = 32;
HDPublicKey.PublicKeySize = 33;
HDPublicKey.CheckSumSize = 4;

HDPublicKey.DataSize = 78;
HDPublicKey.SerializedByteSize = 82;

HDPublicKey.VersionStart = 0;
HDPublicKey.VersionEnd = HDPublicKey.VersionStart + HDPublicKey.VersionSize;
HDPublicKey.DepthStart = HDPublicKey.VersionEnd;
HDPublicKey.DepthEnd = HDPublicKey.DepthStart + HDPublicKey.DepthSize;
HDPublicKey.ParentFingerPrintStart = HDPublicKey.DepthEnd;
HDPublicKey.ParentFingerPrintEnd = HDPublicKey.ParentFingerPrintStart + HDPublicKey.ParentFingerPrintSize;
HDPublicKey.ChildIndexStart = HDPublicKey.ParentFingerPrintEnd;
HDPublicKey.ChildIndexEnd = HDPublicKey.ChildIndexStart + HDPublicKey.ChildIndexSize;
HDPublicKey.ChainCodeStart = HDPublicKey.ChildIndexEnd;
HDPublicKey.ChainCodeEnd = HDPublicKey.ChainCodeStart + HDPublicKey.ChainCodeSize;
HDPublicKey.PublicKeyStart = HDPublicKey.ChainCodeEnd;
HDPublicKey.PublicKeyEnd = HDPublicKey.PublicKeyStart + HDPublicKey.PublicKeySize;
HDPublicKey.ChecksumStart = HDPublicKey.PublicKeyEnd;
HDPublicKey.ChecksumEnd = HDPublicKey.ChecksumStart + HDPublicKey.CheckSumSize;

assert(HDPublicKey.PublicKeyEnd === HDPublicKey.DataSize);
assert(HDPublicKey.ChecksumEnd === HDPublicKey.SerializedByteSize);

magnachain.HDPublicKey = HDPublicKey;
// hdpublickey end----------------------------------------------------------------------

// script/script begin----------------------------------------------------------------------
// var Address = require('../address');
// var BufferReader = require('../encoding/bufferreader');
// var BufferWriter = require('../encoding/bufferwriter');
// var Hash = require('../crypto/hash');
// var Opcode = require('../opcode');
// var PublicKey = require('../publickey');
// var Signature = require('../crypto/signature');
// var Networks = require('../networks');
// var $ = require('../util/preconditions');
// var _ = require('lodash');
// var errors = require('../errors');
// var buffer = require('buffer');
// var BufferUtil = require('../util/buffer');
// var JSUtil = require('../util/js');

/**
 * A bitcoin transaction script. Each transaction's inputs and outputs
 * has a script that is evaluated to validate it's spending.
 *
 * See https://en.bitcoin.it/wiki/Script
 *
 * @constructor
 * @param {Object|string|Buffer=} from optional data to populate script
 */
var Script = function Script(from)
{
    if (!(this instanceof Script))
    {
        return new Script(from);
    }
    this.chunks = [];

    if (BufferUtil.isBuffer(from))
    {
        return Script.fromBuffer(from);
    } else if (from instanceof Address)
    {
        return Script.fromAddress(from);
    } else if (from instanceof Script)
    {
        return Script.fromBuffer(from.toBuffer());
    } else if (_.isString(from))
    {
        return Script.fromString(from);
    } else if (_.isObject(from) && _.isArray(from.chunks))
    {
        this.set(from);
    }
};

Script.prototype.set = function (obj)
{
    $.checkArgument(_.isObject(obj));
    $.checkArgument(_.isArray(obj.chunks));
    this.chunks = obj.chunks;
    return this;
};

Script.fromBuffer = function (buffer)
{
    var script = new Script();
    script.chunks = [];

    var br = new BufferReader(buffer);
    while (!br.finished())
    {
        try
        {
            var opcodenum = br.readUInt8();

            var len, buf;
            if (opcodenum > 0 && opcodenum < Opcode.OP_PUSHDATA1)
            {
                len = opcodenum;
                script.chunks.push({
                    buf: br.read(len),
                    len: len,
                    opcodenum: opcodenum
                });
            } else if (opcodenum === Opcode.OP_PUSHDATA1)
            {
                len = br.readUInt8();
                buf = br.read(len);
                script.chunks.push({
                    buf: buf,
                    len: len,
                    opcodenum: opcodenum
                });
            } else if (opcodenum === Opcode.OP_PUSHDATA2)
            {
                len = br.readUInt16LE();
                buf = br.read(len);
                script.chunks.push({
                    buf: buf,
                    len: len,
                    opcodenum: opcodenum
                });
            } else if (opcodenum === Opcode.OP_PUSHDATA4)
            {
                len = br.readUInt32LE();
                buf = br.read(len);
                script.chunks.push({
                    buf: buf,
                    len: len,
                    opcodenum: opcodenum
                });
            } else
            {
                script.chunks.push({
                    opcodenum: opcodenum
                });
            }
        } catch (e)
        {
            if (e instanceof RangeError)
            {
                throw new errors.Script.InvalidBuffer(buffer.toString('hex'));
            }
            throw e;
        }
    }

    return script;
};

Script.prototype.toBuffer = function ()
{
    var bw = new BufferWriter();

    for (var i = 0; i < this.chunks.length; i++)
    {
        var chunk = this.chunks[i];
        var opcodenum = chunk.opcodenum;
        bw.writeUInt8(chunk.opcodenum);
        if (chunk.buf)
        {
            if (opcodenum < Opcode.OP_PUSHDATA1)
            {
                bw.write(chunk.buf);
            } else if (opcodenum === Opcode.OP_PUSHDATA1)
            {
                bw.writeUInt8(chunk.len);
                bw.write(chunk.buf);
            } else if (opcodenum === Opcode.OP_PUSHDATA2)
            {
                bw.writeUInt16LE(chunk.len);
                bw.write(chunk.buf);
            } else if (opcodenum === Opcode.OP_PUSHDATA4)
            {
                bw.writeUInt32LE(chunk.len);
                bw.write(chunk.buf);
            }
        }
    }

    return bw.concat();
};

Script.fromASM = function (str)
{
    var script = new Script();
    script.chunks = [];

    var tokens = str.split(' ');
    var i = 0;
    while (i < tokens.length)
    {
        var token = tokens[i];
        var opcode = Opcode(token);
        var opcodenum = opcode.toNumber();

        if (_.isUndefined(opcodenum))
        {
            var buf = new Buffer(tokens[i], 'hex');
            script.chunks.push({
                buf: buf,
                len: buf.length,
                opcodenum: buf.length
            });
            i = i + 1;
        } else if (opcodenum === Opcode.OP_PUSHDATA1 ||
            opcodenum === Opcode.OP_PUSHDATA2 ||
            opcodenum === Opcode.OP_PUSHDATA4)
        {
            script.chunks.push({
                buf: new Buffer(tokens[i + 2], 'hex'),
                len: parseInt(tokens[i + 1]),
                opcodenum: opcodenum
            });
            i = i + 3;
        } else
        {
            script.chunks.push({
                opcodenum: opcodenum
            });
            i = i + 1;
        }
    }
    return script;
};

Script.fromHex = function (str)
{
    return new Script(new Buffer(str, 'hex'));
};

Script.fromString = function (str)
{
    if (JSUtil.isHexa(str) || str.length === 0)
    {
        return new Script(new Buffer(str, 'hex'));
    }
    var script = new Script();
    script.chunks = [];

    var tokens = str.split(' ');
    var i = 0;
    while (i < tokens.length)
    {
        var token = tokens[i];
        var opcode = Opcode(token);
        var opcodenum = opcode.toNumber();

        if (_.isUndefined(opcodenum))
        {
            opcodenum = parseInt(token);
            if (opcodenum > 0 && opcodenum < Opcode.OP_PUSHDATA1)
            {
                script.chunks.push({
                    buf: new Buffer(tokens[i + 1].slice(2), 'hex'),
                    len: opcodenum,
                    opcodenum: opcodenum
                });
                i = i + 2;
            } else
            {
                throw new Error('Invalid script: ' + JSON.stringify(str));
            }
        } else if (opcodenum === Opcode.OP_PUSHDATA1 ||
            opcodenum === Opcode.OP_PUSHDATA2 ||
            opcodenum === Opcode.OP_PUSHDATA4)
        {
            if (tokens[i + 2].slice(0, 2) !== '0x')
            {
                throw new Error('Pushdata data must start with 0x');
            }
            script.chunks.push({
                buf: new Buffer(tokens[i + 2].slice(2), 'hex'),
                len: parseInt(tokens[i + 1]),
                opcodenum: opcodenum
            });
            i = i + 3;
        } else
        {
            script.chunks.push({
                opcodenum: opcodenum
            });
            i = i + 1;
        }
    }
    return script;
};

// public static uint256 SignatureHashForContract(Transaction txTo)
//         {
//             var stream = CreateHashWriter(HashVersion.Original);
//             var txCopy = new Transaction(txTo.ToBytes());
//             txCopy.ReadWriteForContractSign(stream);
//             return GetHash(stream);
//         }

Script.SignatureHashForContract = function (txTo)
{
    var stream = new BufferWriter();
    var txCopy = new Transaction(txTo.toBuffer());
    txCopy.WriteForContractSign(stream);
    return Script.GetHash(stream);
}

// private static uint256 GetHash(BitcoinStream stream)
// 		{
// 			var preimage = ((MemoryStream)stream.Inner).ToArrayEfficient();
// 			return Hashes.Hash256(preimage);
// 		}

Script.GetHash = function (stream)
{
    var ret = Hash.sha256sha256(stream.toBuffer());
    ret = new BufferReader(ret).readReverse();
    return ret;
    //return Hash.sha256(stream.toBuffer());
}

Script.prototype._chunkToString = function (chunk, type)
{
    var opcodenum = chunk.opcodenum;
    var asm = (type === 'asm');
    var str = '';
    if (!chunk.buf)
    {
        // no data chunk
        if (typeof Opcode.reverseMap[opcodenum] !== 'undefined')
        {
            if (asm)
            {
                // A few cases where the opcode name differs from reverseMap
                // aside from 1 to 16 data pushes.
                if (opcodenum === 0)
                {
                    // OP_0 -> 0
                    str = str + ' 0';
                } else if (opcodenum === 79)
                {
                    // OP_1NEGATE -> 1
                    str = str + ' -1';
                } else
                {
                    str = str + ' ' + Opcode(opcodenum).toString();
                }
            } else
            {
                str = str + ' ' + Opcode(opcodenum).toString();
            }
        } else
        {
            var numstr = opcodenum.toString(16);
            if (numstr.length % 2 !== 0)
            {
                numstr = '0' + numstr;
            }
            if (asm)
            {
                str = str + ' ' + numstr;
            } else
            {
                str = str + ' ' + '0x' + numstr;
            }
        }
    } else
    {
        // data chunk
        if (!asm && opcodenum === Opcode.OP_PUSHDATA1 ||
            opcodenum === Opcode.OP_PUSHDATA2 ||
            opcodenum === Opcode.OP_PUSHDATA4)
        {
            str = str + ' ' + Opcode(opcodenum).toString();
        }
        if (chunk.len > 0)
        {
            if (asm)
            {
                str = str + ' ' + chunk.buf.toString('hex');
            } else
            {
                str = str + ' ' + chunk.len + ' ' + '0x' + chunk.buf.toString('hex');
            }
        }
    }
    return str;
};

Script.prototype.toASM = function ()
{
    var str = '';
    for (var i = 0; i < this.chunks.length; i++)
    {
        var chunk = this.chunks[i];
        str += this._chunkToString(chunk, 'asm');
    }

    return str.substr(1);
};

Script.prototype.toString = function ()
{
    var str = '';
    for (var i = 0; i < this.chunks.length; i++)
    {
        var chunk = this.chunks[i];
        str += this._chunkToString(chunk);
    }

    return str.substr(1);
};

Script.prototype.toHex = function ()
{
    return this.toBuffer().toString('hex');
};

Script.prototype.inspect = function ()
{
    return '<Script: ' + this.toString() + '>';
};

// script classification methods

/**
 * @returns {boolean} if this is a pay to pubkey hash output script
 */
Script.prototype.isPublicKeyHashOut = function ()
{
    return !!(this.chunks.length === 5 &&
        this.chunks[0].opcodenum === Opcode.OP_DUP &&
        this.chunks[1].opcodenum === Opcode.OP_HASH160 &&
        this.chunks[2].buf &&
        this.chunks[2].buf.length === 20 &&
        this.chunks[3].opcodenum === Opcode.OP_EQUALVERIFY &&
        this.chunks[4].opcodenum === Opcode.OP_CHECKSIG);
};

/**
 * @returns {boolean} if this is a pay to public key hash input script
 */
Script.prototype.isPublicKeyHashIn = function ()
{
    if (this.chunks.length === 2)
    {
        var signatureBuf = this.chunks[0].buf;
        var pubkeyBuf = this.chunks[1].buf;
        if (signatureBuf &&
            signatureBuf.length &&
            signatureBuf[0] === 0x30 &&
            pubkeyBuf &&
            pubkeyBuf.length
        )
        {
            var version = pubkeyBuf[0];
            if ((version === 0x04 ||
                version === 0x06 ||
                version === 0x07) && pubkeyBuf.length === 65)
            {
                return true;
            } else if ((version === 0x03 || version === 0x02) && pubkeyBuf.length === 33)
            {
                return true;
            }
        }
    }
    return false;
};

Script.prototype.getPublicKey = function ()
{
    $.checkState(this.isPublicKeyOut(), 'Can\'t retrieve PublicKey from a non-PK output');
    return this.chunks[0].buf;
};

Script.prototype.getPublicKeyHash = function ()
{
    $.checkState(this.isPublicKeyHashOut(), 'Can\'t retrieve PublicKeyHash from a non-PKH output');
    return this.chunks[2].buf;
};

/**
 * @returns {boolean} if this is a public key output script
 */
Script.prototype.isPublicKeyOut = function ()
{
    if (this.chunks.length === 2 &&
        this.chunks[0].buf &&
        this.chunks[0].buf.length &&
        this.chunks[1].opcodenum === Opcode.OP_CHECKSIG)
    {
        var pubkeyBuf = this.chunks[0].buf;
        var version = pubkeyBuf[0];
        var isVersion = false;
        if ((version === 0x04 ||
            version === 0x06 ||
            version === 0x07) && pubkeyBuf.length === 65)
        {
            isVersion = true;
        } else if ((version === 0x03 || version === 0x02) && pubkeyBuf.length === 33)
        {
            isVersion = true;
        }
        if (isVersion)
        {
            return PublicKey.isValid(pubkeyBuf);
        }
    }
    return false;
};

/**
 * @returns {boolean} if this is a pay to public key input script
 */
Script.prototype.isPublicKeyIn = function ()
{
    if (this.chunks.length === 1)
    {
        var signatureBuf = this.chunks[0].buf;
        if (signatureBuf &&
            signatureBuf.length &&
            signatureBuf[0] === 0x30)
        {
            return true;
        }
    }
    return false;
};

Script.prototype.isContractOut = function ()
{
    if (this.chunks.length === 2)
    {
        if ((this.chunks[0].opcodenum === Opcode.OP_CONTRACT || this.chunks[0].opcodenum === Opcode.OP_CONTRACT_CHANGE)
            && this.chunks[1].buf && this.chunks[1].buf.length === 20)
        {
            return true;
        }
    }
    return false;
}

/**
 * @returns {boolean} if this is a p2sh output script
 */
Script.prototype.isScriptHashOut = function ()
{
    var buf = this.toBuffer();
    return (buf.length === 23 &&
        buf[0] === Opcode.OP_HASH160 &&
        buf[1] === 0x14 &&
        buf[buf.length - 1] === Opcode.OP_EQUAL);
};

/**
 * @returns {boolean} if this is a p2sh input script
 * Note that these are frequently indistinguishable from pubkeyhashin
 */
Script.prototype.isScriptHashIn = function ()
{
    if (this.chunks.length <= 1)
    {
        return false;
    }
    var redeemChunk = this.chunks[this.chunks.length - 1];
    var redeemBuf = redeemChunk.buf;
    if (!redeemBuf)
    {
        return false;
    }

    var redeemScript;
    try
    {
        redeemScript = Script.fromBuffer(redeemBuf);
    } catch (e)
    {
        if (e instanceof errors.Script.InvalidBuffer)
        {
            return false;
        }
        throw e;
    }
    var type = redeemScript.classify();
    return type !== Script.types.UNKNOWN;
};

/**
 * @returns {boolean} if this is a mutlsig output script
 */
Script.prototype.isMultisigOut = function ()
{
    return (this.chunks.length > 3 &&
        Opcode.isSmallIntOp(this.chunks[0].opcodenum) &&
        this.chunks.slice(1, this.chunks.length - 2).every(function (obj)
        {
            return obj.buf && BufferUtil.isBuffer(obj.buf);
        }) &&
        Opcode.isSmallIntOp(this.chunks[this.chunks.length - 2].opcodenum) &&
        this.chunks[this.chunks.length - 1].opcodenum === Opcode.OP_CHECKMULTISIG);
};


/**
 * @returns {boolean} if this is a multisig input script
 */
Script.prototype.isMultisigIn = function ()
{
    return this.chunks.length >= 2 &&
        this.chunks[0].opcodenum === 0 &&
        this.chunks.slice(1, this.chunks.length).every(function (obj)
        {
            return obj.buf &&
                BufferUtil.isBuffer(obj.buf) &&
                Signature.isTxDER(obj.buf);
        });
};

/**
 * @returns {boolean} true if this is a valid standard OP_RETURN output
 */
Script.prototype.isDataOut = function ()
{
    return this.chunks.length >= 1 &&
        this.chunks[0].opcodenum === Opcode.OP_RETURN &&
        (this.chunks.length === 1 ||
            (this.chunks.length === 2 &&
                this.chunks[1].buf &&
                this.chunks[1].buf.length <= Script.OP_RETURN_STANDARD_SIZE &&
                this.chunks[1].length === this.chunks.len));
};

/**
 * Retrieve the associated data for this script.
 * In the case of a pay to public key hash or P2SH, return the hash.
 * In the case of a standard OP_RETURN, return the data
 * @returns {Buffer}
 */
Script.prototype.getData = function ()
{
    if (this.isDataOut() || this.isScriptHashOut())
    {
        if (_.isUndefined(this.chunks[1]))
        {
            return new Buffer(0);
        } else
        {
            return new Buffer(this.chunks[1].buf);
        }
    }
    if (this.isPublicKeyHashOut())
    {
        return new Buffer(this.chunks[2].buf);
    }
    throw new Error('Unrecognized script type to get data from');
};

/**
 * @returns {boolean} if the script is only composed of data pushing
 * opcodes or small int opcodes (OP_0, OP_1, ..., OP_16)
 */
Script.prototype.isPushOnly = function ()
{
    return _.every(this.chunks, function (chunk)
    {
        return chunk.opcodenum <= Opcode.OP_16;
    });
};


Script.types = {};
Script.types.UNKNOWN = 'Unknown';
Script.types.PUBKEY_OUT = 'Pay to public key';
Script.types.PUBKEY_IN = 'Spend from public key';
Script.types.PUBKEYHASH_OUT = 'Pay to public key hash';
Script.types.PUBKEYHASH_IN = 'Spend from public key hash';
Script.types.SCRIPTHASH_OUT = 'Pay to script hash';
Script.types.SCRIPTHASH_IN = 'Spend from script hash';
Script.types.MULTISIG_OUT = 'Pay to multisig';
Script.types.MULTISIG_IN = 'Spend from multisig';
Script.types.DATA_OUT = 'Data push';

Script.OP_RETURN_STANDARD_SIZE = 80;

/**
 * @returns {object} The Script type if it is a known form,
 * or Script.UNKNOWN if it isn't
 */
Script.prototype.classify = function ()
{
    if (this._isInput)
    {
        return this.classifyInput();
    } else if (this._isOutput)
    {
        return this.classifyOutput();
    } else
    {
        var outputType = this.classifyOutput();
        return outputType != Script.types.UNKNOWN ? outputType : this.classifyInput();
    }
};

Script.outputIdentifiers = {};
Script.outputIdentifiers.PUBKEY_OUT = Script.prototype.isPublicKeyOut;
Script.outputIdentifiers.PUBKEYHASH_OUT = Script.prototype.isPublicKeyHashOut;
Script.outputIdentifiers.MULTISIG_OUT = Script.prototype.isMultisigOut;
Script.outputIdentifiers.SCRIPTHASH_OUT = Script.prototype.isScriptHashOut;
Script.outputIdentifiers.DATA_OUT = Script.prototype.isDataOut;

/**
 * @returns {object} The Script type if it is a known form,
 * or Script.UNKNOWN if it isn't
 */
Script.prototype.classifyOutput = function ()
{
    for (var type in Script.outputIdentifiers)
    {
        if (Script.outputIdentifiers[type].bind(this)())
        {
            return Script.types[type];
        }
    }
    return Script.types.UNKNOWN;
};

Script.inputIdentifiers = {};
Script.inputIdentifiers.PUBKEY_IN = Script.prototype.isPublicKeyIn;
Script.inputIdentifiers.PUBKEYHASH_IN = Script.prototype.isPublicKeyHashIn;
Script.inputIdentifiers.MULTISIG_IN = Script.prototype.isMultisigIn;
Script.inputIdentifiers.SCRIPTHASH_IN = Script.prototype.isScriptHashIn;

/**
 * @returns {object} The Script type if it is a known form,
 * or Script.UNKNOWN if it isn't
 */
Script.prototype.classifyInput = function ()
{
    for (var type in Script.inputIdentifiers)
    {
        if (Script.inputIdentifiers[type].bind(this)())
        {
            return Script.types[type];
        }
    }
    return Script.types.UNKNOWN;
};


/**
 * @returns {boolean} if script is one of the known types
 */
Script.prototype.isStandard = function ()
{
    // TODO: Add BIP62 compliance
    return this.classify() !== Script.types.UNKNOWN;
};


// Script construction methods

/**
 * Adds a script element at the start of the script.
 * @param {*} obj a string, number, Opcode, Buffer, or object to add
 * @returns {Script} this script instance
 */
Script.prototype.prepend = function (obj)
{
    this._addByType(obj, true);
    return this;
};

/**
 * Compares a script with another script
 */
Script.prototype.equals = function (script)
{
    $.checkState(script instanceof Script, 'Must provide another script');
    if (this.chunks.length !== script.chunks.length)
    {
        return false;
    }
    var i;
    for (i = 0; i < this.chunks.length; i++)
    {
        if (BufferUtil.isBuffer(this.chunks[i].buf) && !BufferUtil.isBuffer(script.chunks[i].buf))
        {
            return false;
        }
        if (BufferUtil.isBuffer(this.chunks[i].buf) && !BufferUtil.equals(this.chunks[i].buf, script.chunks[i].buf))
        {
            return false;
        } else if (this.chunks[i].opcodenum !== script.chunks[i].opcodenum)
        {
            return false;
        }
    }
    return true;
};

/**
 * Adds a script element to the end of the script.
 *
 * @param {*} obj a string, number, Opcode, Buffer, or object to add
 * @returns {Script} this script instance
 *
 */
Script.prototype.add = function (obj)
{
    this._addByType(obj, false);
    return this;
};

Script.prototype._addByType = function (obj, prepend)
{
    if (typeof obj === 'string')
    {
        this._addOpcode(obj, prepend);
    } else if (typeof obj === 'number')
    {
        this._addOpcode(obj, prepend);
    } else if (obj instanceof Opcode)
    {
        this._addOpcode(obj, prepend);
    } else if (BufferUtil.isBuffer(obj))
    {
        this._addBuffer(obj, prepend);
    } else if (obj instanceof Script)
    {
        this.chunks = this.chunks.concat(obj.chunks);
    } else if (typeof obj === 'object')
    {
        this._insertAtPosition(obj, prepend);
    } else
    {
        throw new Error('Invalid script chunk');
    }
};

Script.prototype._insertAtPosition = function (op, prepend)
{
    if (prepend)
    {
        this.chunks.unshift(op);
    } else
    {
        this.chunks.push(op);
    }
};

Script.prototype._addOpcode = function (opcode, prepend)
{
    var op;
    if (typeof opcode === 'number')
    {
        op = opcode;
    } else if (opcode instanceof Opcode)
    {
        op = opcode.toNumber();
    } else
    {
        op = Opcode(opcode).toNumber();
    }
    this._insertAtPosition({
        opcodenum: op
    }, prepend);
    return this;
};

Script.prototype._addBuffer = function (buf, prepend)
{
    var opcodenum;
    var len = buf.length;
    if (len >= 0 && len < Opcode.OP_PUSHDATA1)
    {
        opcodenum = len;
    } else if (len < Math.pow(2, 8))
    {
        opcodenum = Opcode.OP_PUSHDATA1;
    } else if (len < Math.pow(2, 16))
    {
        opcodenum = Opcode.OP_PUSHDATA2;
    } else if (len < Math.pow(2, 32))
    {
        opcodenum = Opcode.OP_PUSHDATA4;
    } else
    {
        throw new Error('You can\'t push that much data');
    }
    this._insertAtPosition({
        buf: buf,
        len: len,
        opcodenum: opcodenum
    }, prepend);
    return this;
};

Script.prototype.removeCodeseparators = function ()
{
    var chunks = [];
    for (var i = 0; i < this.chunks.length; i++)
    {
        if (this.chunks[i].opcodenum !== Opcode.OP_CODESEPARATOR)
        {
            chunks.push(this.chunks[i]);
        }
    }
    this.chunks = chunks;
    return this;
};

// high level script builder methods

/**
 * @returns {Script} a new Multisig output script for given public keys,
 * requiring m of those public keys to spend
 * @param {PublicKey[]} publicKeys - list of all public keys controlling the output
 * @param {number} threshold - amount of required signatures to spend the output
 * @param {Object=} opts - Several options:
 *        - noSorting: defaults to false, if true, don't sort the given
 *                      public keys before creating the script
 */
Script.buildMultisigOut = function (publicKeys, threshold, opts)
{
    $.checkArgument(threshold <= publicKeys.length,
        'Number of required signatures must be less than or equal to the number of public keys');
    opts = opts || {};
    var script = new Script();
    script.add(Opcode.smallInt(threshold));
    publicKeys = _.map(publicKeys, PublicKey);
    var sorted = publicKeys;
    if (!opts.noSorting)
    {
        sorted = _.sortBy(publicKeys, function (publicKey)
        {
            return publicKey.toString('hex');
        });
    }
    for (var i = 0; i < sorted.length; i++)
    {
        var publicKey = sorted[i];
        script.add(publicKey.toBuffer());
    }
    script.add(Opcode.smallInt(publicKeys.length));
    script.add(Opcode.OP_CHECKMULTISIG);
    return script;
};

/**
 * A new Multisig input script for the given public keys, requiring m of those public keys to spend
 *
 * @param {PublicKey[]} pubkeys list of all public keys controlling the output
 * @param {number} threshold amount of required signatures to spend the output
 * @param {Array} signatures and array of signature buffers to append to the script
 * @param {Object=} opts
 * @param {boolean=} opts.noSorting don't sort the given public keys before creating the script (false by default)
 * @param {Script=} opts.cachedMultisig don't recalculate the redeemScript
 *
 * @returns {Script}
 */
Script.buildMultisigIn = function (pubkeys, threshold, signatures, opts)
{
    $.checkArgument(_.isArray(pubkeys));
    $.checkArgument(_.isNumber(threshold));
    $.checkArgument(_.isArray(signatures));
    opts = opts || {};
    var s = new Script();
    s.add(Opcode.OP_0);
    _.each(signatures, function (signature)
    {
        $.checkArgument(BufferUtil.isBuffer(signature), 'Signatures must be an array of Buffers');
        // TODO: allow signatures to be an array of Signature objects
        s.add(signature);
    });
    return s;
};

/**
 * A new P2SH Multisig input script for the given public keys, requiring m of those public keys to spend
 *
 * @param {PublicKey[]} pubkeys list of all public keys controlling the output
 * @param {number} threshold amount of required signatures to spend the output
 * @param {Array} signatures and array of signature buffers to append to the script
 * @param {Object=} opts
 * @param {boolean=} opts.noSorting don't sort the given public keys before creating the script (false by default)
 * @param {Script=} opts.cachedMultisig don't recalculate the redeemScript
 *
 * @returns {Script}
 */
Script.buildP2SHMultisigIn = function (pubkeys, threshold, signatures, opts)
{
    $.checkArgument(_.isArray(pubkeys));
    $.checkArgument(_.isNumber(threshold));
    $.checkArgument(_.isArray(signatures));
    opts = opts || {};
    var s = new Script();
    s.add(Opcode.OP_0);
    _.each(signatures, function (signature)
    {
        $.checkArgument(BufferUtil.isBuffer(signature), 'Signatures must be an array of Buffers');
        // TODO: allow signatures to be an array of Signature objects
        s.add(signature);
    });
    s.add((opts.cachedMultisig || Script.buildMultisigOut(pubkeys, threshold, opts)).toBuffer());
    return s;
};

/**
 * @returns {Script} a new pay to public key hash output for the given
 * address or public key
 * @param {(Address|PublicKey)} to - destination address or public key
 */
Script.buildPublicKeyHashOut = function (to)
{
    $.checkArgument(!_.isUndefined(to));
    $.checkArgument(to instanceof PublicKey || to instanceof Address || _.isString(to));
    if (to instanceof PublicKey)
    {
        to = to.toAddress();
    } else if (_.isString(to))
    {
        to = new Address(to);
    }
    var s = new Script();
    s.add(Opcode.OP_DUP)
        .add(Opcode.OP_HASH160)
        .add(to.hashBuffer)
        .add(Opcode.OP_EQUALVERIFY)
        .add(Opcode.OP_CHECKSIG);
    s._network = to.network;
    return s;
};

/**
 * @returns {Script} a new pay to public key output for the given
 *  public key
 */
Script.buildPublicKeyOut = function (pubkey)
{
    $.checkArgument(pubkey instanceof PublicKey);
    var s = new Script();
    s.add(pubkey.toBuffer())
        .add(Opcode.OP_CHECKSIG);
    return s;
};

/**
 * @returns {Script} a new OP_RETURN script with data
 * @param {(string|Buffer)} data - the data to embed in the output
 * @param {(string)} encoding - the type of encoding of the string
 */
Script.buildDataOut = function (data, encoding)
{
    $.checkArgument(_.isUndefined(data) || _.isString(data) || BufferUtil.isBuffer(data));
    if (_.isString(data))
    {
        data = new Buffer(data, encoding);
    }
    var s = new Script();
    s.add(Opcode.OP_RETURN);
    if (!_.isUndefined(data))
    {
        s.add(data);
    }
    return s;
};

/**
 * @param {Script|Address} script - the redeemScript for the new p2sh output.
 *    It can also be a p2sh address
 * @returns {Script} new pay to script hash script for given script
 */
Script.buildScriptHashOut = function (script)
{
    $.checkArgument(script instanceof Script ||
        (script instanceof Address && script.isPayToScriptHash()));
    var s = new Script();
    s.add(Opcode.OP_HASH160)
        .add(script instanceof Address ? script.hashBuffer : Hash.sha256ripemd160(script.toBuffer()))
        .add(Opcode.OP_EQUAL);

    s._network = script._network || script.network;
    return s;
};

/**
 * Builds a scriptSig (a script for an input) that signs a public key output script.
 *
 * @param {Signature|Buffer} signature - a Signature object, or the signature in DER canonical encoding
 * @param {number=} sigtype - the type of the signature (defaults to SIGHASH_ALL)
 */
Script.buildPublicKeyIn = function (signature, sigtype)
{
    $.checkArgument(signature instanceof Signature || BufferUtil.isBuffer(signature));
    $.checkArgument(_.isUndefined(sigtype) || _.isNumber(sigtype));
    if (signature instanceof Signature)
    {
        signature = signature.toBuffer();
    }
    var script = new Script();
    script.add(BufferUtil.concat([
        signature,
        BufferUtil.integerAsSingleByteBuffer(sigtype || Signature.SIGHASH_ALL)
    ]));
    return script;
};

/**
 * Builds a scriptSig (a script for an input) that signs a public key hash
 * output script.
 *
 * @param {Buffer|string|PublicKey} publicKey
 * @param {Signature|Buffer} signature - a Signature object, or the signature in DER canonical encoding
 * @param {number=} sigtype - the type of the signature (defaults to SIGHASH_ALL)
 */
Script.buildPublicKeyHashIn = function (publicKey, signature, sigtype)
{
    $.checkArgument(signature instanceof Signature || BufferUtil.isBuffer(signature));
    $.checkArgument(_.isUndefined(sigtype) || _.isNumber(sigtype));
    if (signature instanceof Signature)
    {
        signature = signature.toBuffer();
    }
    var script = new Script()
        .add(BufferUtil.concat([
            signature,
            BufferUtil.integerAsSingleByteBuffer(sigtype || Signature.SIGHASH_ALL)
        ]))
        .add(new PublicKey(publicKey).toBuffer());
    return script;
};

/**
 * @returns {Script} an empty script
 */
Script.empty = function ()
{
    return new Script();
};

/**
 * @returns {Script} a new pay to script hash script that pays to this script
 */
Script.prototype.toScriptHashOut = function ()
{
    return Script.buildScriptHashOut(this);
};

/**
 * @return {Script} an output script built from the address
 */
Script.fromAddress = function (address)
{
    address = Address(address);
    if (address.isPayToScriptHash())
    {
        return Script.buildScriptHashOut(address);
    } else if (address.isPayToPublicKeyHash())
    {
        return Script.buildPublicKeyHashOut(address);
    }
    throw new errors.Script.UnrecognizedAddress(address);
};

/**
 * Will return the associated address information object
 * @return {Address|boolean}
 */
Script.prototype.getAddressInfo = function (opts)
{
    if (this._isInput)
    {
        return this._getInputAddressInfo();
    } else if (this._isOutput)
    {
        return this._getOutputAddressInfo();
    } else
    {
        var info = this._getOutputAddressInfo();
        if (!info)
        {
            return this._getInputAddressInfo();
        }
        return info;
    }
};

/**
 * Will return the associated output scriptPubKey address information object
 * @return {Address|boolean}
 * @private
 */
Script.prototype._getOutputAddressInfo = function ()
{
    var info = {};
    if (this.isScriptHashOut())
    {
        info.hashBuffer = this.getData();
        info.type = Address.PayToScriptHash;
    } else if (this.isPublicKeyHashOut())
    {
        info.hashBuffer = this.getData();
        info.type = Address.PayToPublicKeyHash;
    } else
    {
        return false;
    }
    return info;
};

/**
 * Will return the associated input scriptSig address information object
 * @return {Address|boolean}
 * @private
 */
Script.prototype._getInputAddressInfo = function ()
{
    var info = {};
    if (this.isPublicKeyHashIn())
    {
        // hash the publickey found in the scriptSig
        info.hashBuffer = Hash.sha256ripemd160(this.chunks[1].buf);
        info.type = Address.PayToPublicKeyHash;
    } else if (this.isScriptHashIn())
    {
        // hash the redeemscript found at the end of the scriptSig
        info.hashBuffer = Hash.sha256ripemd160(this.chunks[this.chunks.length - 1].buf);
        info.type = Address.PayToScriptHash;
    } else
    {
        return false;
    }
    return info;
};

/**
 * @param {Network=} network
 * @return {Address|boolean} the associated address for this script if possible, or false
 */
Script.prototype.toAddress = function (network)
{
    var info = this.getAddressInfo();
    if (!info)
    {
        return false;
    }
    info.network = Networks.get(network) || this._network || Networks.defaultNetwork;
    return new Address(info);
};

/**
 * Analogous to bitcoind's FindAndDelete. Find and delete equivalent chunks,
 * typically used with push data chunks.  Note that this will find and delete
 * not just the same data, but the same data with the same push data op as
 * produced by default. i.e., if a pushdata in a tx does not use the minimal
 * pushdata op, then when you try to remove the data it is pushing, it will not
 * be removed, because they do not use the same pushdata op.
 */
Script.prototype.findAndDelete = function (script)
{
    var buf = script.toBuffer();
    var hex = buf.toString('hex');
    for (var i = 0; i < this.chunks.length; i++)
    {
        var script2 = Script({
            chunks: [this.chunks[i]]
        });
        var buf2 = script2.toBuffer();
        var hex2 = buf2.toString('hex');
        if (hex === hex2)
        {
            this.chunks.splice(i, 1);
        }
    }
    return this;
};

/**
 * Comes from bitcoind's script interpreter CheckMinimalPush function
 * @returns {boolean} if the chunk {i} is the smallest way to push that particular data.
 */
Script.prototype.checkMinimalPush = function (i)
{
    var chunk = this.chunks[i];
    var buf = chunk.buf;
    var opcodenum = chunk.opcodenum;
    if (!buf)
    {
        return true;
    }
    if (buf.length === 0)
    {
        // Could have used OP_0.
        return opcodenum === Opcode.OP_0;
    } else if (buf.length === 1 && buf[0] >= 1 && buf[0] <= 16)
    {
        // Could have used OP_1 .. OP_16.
        return opcodenum === Opcode.OP_1 + (buf[0] - 1);
    } else if (buf.length === 1 && buf[0] === 0x81)
    {
        // Could have used OP_1NEGATE
        return opcodenum === Opcode.OP_1NEGATE;
    } else if (buf.length <= 75)
    {
        // Could have used a direct push (opcode indicating number of bytes pushed + those bytes).
        return opcodenum === buf.length;
    } else if (buf.length <= 255)
    {
        // Could have used OP_PUSHDATA.
        return opcodenum === Opcode.OP_PUSHDATA1;
    } else if (buf.length <= 65535)
    {
        // Could have used OP_PUSHDATA2.
        return opcodenum === Opcode.OP_PUSHDATA2;
    }
    return true;
};

/**
 * Comes from bitcoind's script DecodeOP_N function
 * @param {number} opcode
 * @returns {number} numeric value in range of 0 to 16
 */
Script.prototype._decodeOP_N = function (opcode)
{
    if (opcode === Opcode.OP_0)
    {
        return 0;
    } else if (opcode >= Opcode.OP_1 && opcode <= Opcode.OP_16)
    {
        return opcode - (Opcode.OP_1 - 1);
    } else
    {
        throw new Error('Invalid opcode: ' + JSON.stringify(opcode));
    }
};

/**
 * Comes from bitcoind's script GetSigOpCount(boolean) function
 * @param {boolean} use current (true) or pre-version-0.6 (false) logic
 * @returns {number} number of signature operations required by this script
 */
Script.prototype.getSignatureOperationsCount = function (accurate)
{
    accurate = (_.isUndefined(accurate) ? true : accurate);
    var self = this;
    var n = 0;
    var lastOpcode = Opcode.OP_INVALIDOPCODE;
    _.each(self.chunks, function getChunk(chunk)
    {
        var opcode = chunk.opcodenum;
        if (opcode == Opcode.OP_CHECKSIG || opcode == Opcode.OP_CHECKSIGVERIFY)
        {
            n++;
        } else if (opcode == Opcode.OP_CHECKMULTISIG || opcode == Opcode.OP_CHECKMULTISIGVERIFY)
        {
            if (accurate && lastOpcode >= Opcode.OP_1 && lastOpcode <= Opcode.OP_16)
            {
                n += self._decodeOP_N(lastOpcode);
            } else
            {
                n += 20;
            }
        }
        lastOpcode = opcode;
    });
    return n;
};

magnachain.Script = Script;
// script/script end----------------------------------------------------------------------

// script/interpreter begin----------------------------------------------------------------------
// var _ = require('lodash');

// var Script = require('./script');
// var Opcode = require('../opcode');
// var BN = require('../crypto/bn');
// var Hash = require('../crypto/hash');
// var Signature = require('../crypto/signature');
// var PublicKey = require('../publickey');

/**
 * Bitcoin transactions contain scripts. Each input has a script called the
 * scriptSig, and each output has a script called the scriptPubkey. To validate
 * an input, the input's script is concatenated with the referenced output script,
 * and the result is executed. If at the end of execution the stack contains a
 * "true" value, then the transaction is valid.
 *
 * The primary way to use this class is via the verify function.
 * e.g., Interpreter().verify( ... );
 */
var Interpreter = function Interpreter(obj)
{
    if (!(this instanceof Interpreter))
    {
        return new Interpreter(obj);
    }
    if (obj)
    {
        this.initialize();
        this.set(obj);
    } else
    {
        this.initialize();
    }
};

/**
 * Verifies a Script by executing it and returns true if it is valid.
 * This function needs to be provided with the scriptSig and the scriptPubkey
 * separately.
 * @param {Script} scriptSig - the script's first part (corresponding to the tx input)
 * @param {Script} scriptPubkey - the script's last part (corresponding to the tx output)
 * @param {Transaction=} tx - the Transaction containing the scriptSig in one input (used
 *    to check signature validity for some opcodes like OP_CHECKSIG)
 * @param {number} nin - index of the transaction input containing the scriptSig verified.
 * @param {number} flags - evaluation flags. See Interpreter.SCRIPT_* constants
 *
 * Translated from bitcoind's VerifyScript
 */
Interpreter.prototype.verify = function (scriptSig, scriptPubkey, tx, nin, flags)
{
    //var Transaction = require('../transaction');
    if (_.isUndefined(tx))
    {
        tx = new Transaction();
    }
    if (_.isUndefined(nin))
    {
        nin = 0;
    }
    if (_.isUndefined(flags))
    {
        flags = 0;
    }
    this.set({
        script: scriptSig,
        tx: tx,
        nin: nin,
        flags: flags
    });
    var stackCopy;

    if ((flags & Interpreter.SCRIPT_VERIFY_SIGPUSHONLY) !== 0 && !scriptSig.isPushOnly())
    {
        this.errstr = 'SCRIPT_ERR_SIG_PUSHONLY';
        return false;
    }

    // evaluate scriptSig
    if (!this.evaluate())
    {
        return false;
    }

    if (flags & Interpreter.SCRIPT_VERIFY_P2SH)
    {
        stackCopy = this.stack.slice();
    }

    var stack = this.stack;
    this.initialize();
    this.set({
        script: scriptPubkey,
        stack: stack,
        tx: tx,
        nin: nin,
        flags: flags
    });

    // evaluate scriptPubkey
    if (!this.evaluate())
    {
        return false;
    }

    if (this.stack.length === 0)
    {
        this.errstr = 'SCRIPT_ERR_EVAL_FALSE_NO_RESULT';
        return false;
    }

    var buf = this.stack[this.stack.length - 1];
    if (!Interpreter.castToBool(buf))
    {
        this.errstr = 'SCRIPT_ERR_EVAL_FALSE_IN_STACK';
        return false;
    }

    // Additional validation for spend-to-script-hash transactions:
    if ((flags & Interpreter.SCRIPT_VERIFY_P2SH) && scriptPubkey.isScriptHashOut())
    {
        // scriptSig must be literals-only or validation fails
        if (!scriptSig.isPushOnly())
        {
            this.errstr = 'SCRIPT_ERR_SIG_PUSHONLY';
            return false;
        }

        // stackCopy cannot be empty here, because if it was the
        // P2SH  HASH <> EQUAL  scriptPubKey would be evaluated with
        // an empty stack and the EvalScript above would return false.
        if (stackCopy.length === 0)
        {
            throw new Error('internal error - stack copy empty');
        }

        var redeemScriptSerialized = stackCopy[stackCopy.length - 1];
        var redeemScript = Script.fromBuffer(redeemScriptSerialized);
        stackCopy.pop();

        this.initialize();
        this.set({
            script: redeemScript,
            stack: stackCopy,
            tx: tx,
            nin: nin,
            flags: flags
        });

        // evaluate redeemScript
        if (!this.evaluate())
        {
            return false;
        }

        if (stackCopy.length === 0)
        {
            this.errstr = 'SCRIPT_ERR_EVAL_FALSE_NO_P2SH_STACK';
            return false;
        }

        if (!Interpreter.castToBool(stackCopy[stackCopy.length - 1]))
        {
            this.errstr = 'SCRIPT_ERR_EVAL_FALSE_IN_P2SH_STACK';
            return false;
        } else
        {
            return true;
        }
    }

    return true;
};

magnachain.Interpreter = Interpreter;

Interpreter.prototype.initialize = function (obj)
{
    this.stack = [];
    this.altstack = [];
    this.pc = 0;
    this.pbegincodehash = 0;
    this.nOpCount = 0;
    this.vfExec = [];
    this.errstr = '';
    this.flags = 0;
};

Interpreter.prototype.set = function (obj)
{
    this.script = obj.script || this.script;
    this.tx = obj.tx || this.tx;
    this.nin = typeof obj.nin !== 'undefined' ? obj.nin : this.nin;
    this.stack = obj.stack || this.stack;
    this.altstack = obj.altack || this.altstack;
    this.pc = typeof obj.pc !== 'undefined' ? obj.pc : this.pc;
    this.pbegincodehash = typeof obj.pbegincodehash !== 'undefined' ? obj.pbegincodehash : this.pbegincodehash;
    this.nOpCount = typeof obj.nOpCount !== 'undefined' ? obj.nOpCount : this.nOpCount;
    this.vfExec = obj.vfExec || this.vfExec;
    this.errstr = obj.errstr || this.errstr;
    this.flags = typeof obj.flags !== 'undefined' ? obj.flags : this.flags;
};

Interpreter.true = new Buffer([1]);
Interpreter.false = new Buffer([]);

Interpreter.MAX_SCRIPT_ELEMENT_SIZE = 520;

Interpreter.LOCKTIME_THRESHOLD = 500000000;
Interpreter.LOCKTIME_THRESHOLD_BN = new BN(Interpreter.LOCKTIME_THRESHOLD);

// flags taken from bitcoind
// bitcoind commit: b5d1b1092998bc95313856d535c632ea5a8f9104
Interpreter.SCRIPT_VERIFY_NONE = 0;

// Evaluate P2SH subscripts (softfork safe, BIP16).
Interpreter.SCRIPT_VERIFY_P2SH = (1 << 0);

// Passing a non-strict-DER signature or one with undefined hashtype to a checksig operation causes script failure.
// Passing a pubkey that is not (0x04 + 64 bytes) or (0x02 or 0x03 + 32 bytes) to checksig causes that pubkey to be
// skipped (not softfork safe: this flag can widen the validity of OP_CHECKSIG OP_NOT).
Interpreter.SCRIPT_VERIFY_STRICTENC = (1 << 1);

// Passing a non-strict-DER signature to a checksig operation causes script failure (softfork safe, BIP62 rule 1)
Interpreter.SCRIPT_VERIFY_DERSIG = (1 << 2);

// Passing a non-strict-DER signature or one with S > order/2 to a checksig operation causes script failure
// (softfork safe, BIP62 rule 5).
Interpreter.SCRIPT_VERIFY_LOW_S = (1 << 3);

// verify dummy stack item consumed by CHECKMULTISIG is of zero-length (softfork safe, BIP62 rule 7).
Interpreter.SCRIPT_VERIFY_NULLDUMMY = (1 << 4);

// Using a non-push operator in the scriptSig causes script failure (softfork safe, BIP62 rule 2).
Interpreter.SCRIPT_VERIFY_SIGPUSHONLY = (1 << 5);

// Require minimal encodings for all push operations (OP_0... OP_16, OP_1NEGATE where possible, direct
// pushes up to 75 bytes, OP_PUSHDATA up to 255 bytes, OP_PUSHDATA2 for anything larger). Evaluating
// any other push causes the script to fail (BIP62 rule 3).
// In addition, whenever a stack element is interpreted as a number, it must be of minimal length (BIP62 rule 4).
// (softfork safe)
Interpreter.SCRIPT_VERIFY_MINIMALDATA = (1 << 6);

// Discourage use of NOPs reserved for upgrades (NOP1-10)
//
// Provided so that nodes can avoid accepting or mining transactions
// containing executed NOP's whose meaning may change after a soft-fork,
// thus rendering the script invalid; with this flag set executing
// discouraged NOPs fails the script. This verification flag will never be
// a mandatory flag applied to scripts in a block. NOPs that are not
// executed, e.g.  within an unexecuted IF ENDIF block, are *not* rejected.
Interpreter.SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS = (1 << 7);

// CLTV See BIP65 for details.
Interpreter.SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY = (1 << 9);

Interpreter.castToBool = function (buf)
{
    for (var i = 0; i < buf.length; i++)
    {
        if (buf[i] !== 0)
        {
            // can be negative zero
            if (i === buf.length - 1 && buf[i] === 0x80)
            {
                return false;
            }
            return true;
        }
    }
    return false;
};

/**
 * Translated from bitcoind's CheckSignatureEncoding
 */
Interpreter.prototype.checkSignatureEncoding = function (buf)
{
    var sig;
    if ((this.flags & (Interpreter.SCRIPT_VERIFY_DERSIG | Interpreter.SCRIPT_VERIFY_LOW_S | Interpreter.SCRIPT_VERIFY_STRICTENC)) !== 0 && !Signature.isTxDER(buf))
    {
        this.errstr = 'SCRIPT_ERR_SIG_DER_INVALID_FORMAT';
        return false;
    } else if ((this.flags & Interpreter.SCRIPT_VERIFY_LOW_S) !== 0)
    {
        sig = Signature.fromTxFormat(buf);
        if (!sig.hasLowS())
        {
            this.errstr = 'SCRIPT_ERR_SIG_DER_HIGH_S';
            return false;
        }
    } else if ((this.flags & Interpreter.SCRIPT_VERIFY_STRICTENC) !== 0)
    {
        sig = Signature.fromTxFormat(buf);
        if (!sig.hasDefinedHashtype())
        {
            this.errstr = 'SCRIPT_ERR_SIG_HASHTYPE';
            return false;
        }
    }
    return true;
};

/**
 * Translated from bitcoind's CheckPubKeyEncoding
 */
Interpreter.prototype.checkPubkeyEncoding = function (buf)
{
    if ((this.flags & Interpreter.SCRIPT_VERIFY_STRICTENC) !== 0 && !PublicKey.isValid(buf))
    {
        this.errstr = 'SCRIPT_ERR_PUBKEYTYPE';
        return false;
    }
    return true;
};

/**
 * Based on bitcoind's EvalScript function, with the inner loop moved to
 * Interpreter.prototype.step()
 * bitcoind commit: b5d1b1092998bc95313856d535c632ea5a8f9104
 */
Interpreter.prototype.evaluate = function ()
{
    if (this.script.toBuffer().length > 10000)
    {
        this.errstr = 'SCRIPT_ERR_SCRIPT_SIZE';
        return false;
    }

    try
    {
        while (this.pc < this.script.chunks.length)
        {
            var fSuccess = this.step();
            if (!fSuccess)
            {
                return false;
            }
        }

        // Size limits
        if (this.stack.length + this.altstack.length > 1000)
        {
            this.errstr = 'SCRIPT_ERR_STACK_SIZE';
            return false;
        }
    } catch (e)
    {
        this.errstr = 'SCRIPT_ERR_UNKNOWN_ERROR: ' + e;
        return false;
    }

    if (this.vfExec.length > 0)
    {
        this.errstr = 'SCRIPT_ERR_UNBALANCED_CONDITIONAL';
        return false;
    }

    return true;
};

/**
 * Checks a locktime parameter with the transaction's locktime.
 * There are two times of nLockTime: lock-by-blockheight and lock-by-blocktime,
 * distinguished by whether nLockTime < LOCKTIME_THRESHOLD = 500000000
 *
 * See the corresponding code on bitcoin core:
 * https://github.com/bitcoin/bitcoin/blob/ffd75adce01a78b3461b3ff05bcc2b530a9ce994/src/script/interpreter.cpp#L1129
 *
 * @param {BN} nLockTime the locktime read from the script
 * @return {boolean} true if the transaction's locktime is less than or equal to
 *                   the transaction's locktime
 */
Interpreter.prototype.checkLockTime = function (nLockTime)
{

    // We want to compare apples to apples, so fail the script
    // unless the type of nLockTime being tested is the same as
    // the nLockTime in the transaction.
    if (!(
        (this.tx.nLockTime < Interpreter.LOCKTIME_THRESHOLD && nLockTime.lt(Interpreter.LOCKTIME_THRESHOLD_BN)) ||
        (this.tx.nLockTime >= Interpreter.LOCKTIME_THRESHOLD && nLockTime.gte(Interpreter.LOCKTIME_THRESHOLD_BN))
    ))
    {
        return false;
    }

    // Now that we know we're comparing apples-to-apples, the
    // comparison is a simple numeric one.
    if (nLockTime.gt(new BN(this.tx.nLockTime)))
    {
        return false;
    }

    // Finally the nLockTime feature can be disabled and thus
    // CHECKLOCKTIMEVERIFY bypassed if every txin has been
    // finalized by setting nSequence to maxint. The
    // transaction would be allowed into the blockchain, making
    // the opcode ineffective.
    //
    // Testing if this vin is not final is sufficient to
    // prevent this condition. Alternatively we could test all
    // inputs, but testing just this input minimizes the data
    // required to prove correct CHECKLOCKTIMEVERIFY execution.
    if (!this.tx.inputs[this.nin].isFinal())
    {
        return false;
    }

    return true;
}

/** 
 * Based on the inner loop of bitcoind's EvalScript function
 * bitcoind commit: b5d1b1092998bc95313856d535c632ea5a8f9104
 */
Interpreter.prototype.step = function ()
{

    var fRequireMinimal = (this.flags & Interpreter.SCRIPT_VERIFY_MINIMALDATA) !== 0;

    //bool fExec = !count(vfExec.begin(), vfExec.end(), false);
    var fExec = (this.vfExec.indexOf(false) === -1);
    var buf, buf1, buf2, spliced, n, x1, x2, bn, bn1, bn2, bufSig, bufPubkey, subscript;
    var sig, pubkey;
    var fValue, fSuccess;

    // Read instruction
    var chunk = this.script.chunks[this.pc];
    this.pc++;
    var opcodenum = chunk.opcodenum;
    if (_.isUndefined(opcodenum))
    {
        this.errstr = 'SCRIPT_ERR_UNDEFINED_OPCODE';
        return false;
    }
    if (chunk.buf && chunk.buf.length > Interpreter.MAX_SCRIPT_ELEMENT_SIZE)
    {
        this.errstr = 'SCRIPT_ERR_PUSH_SIZE';
        return false;
    }

    // Note how Opcode.OP_RESERVED does not count towards the opcode limit.
    if (opcodenum > Opcode.OP_16 && ++(this.nOpCount) > 201)
    {
        this.errstr = 'SCRIPT_ERR_OP_COUNT';
        return false;
    }


    if (opcodenum === Opcode.OP_CAT ||
        opcodenum === Opcode.OP_SUBSTR ||
        opcodenum === Opcode.OP_LEFT ||
        opcodenum === Opcode.OP_RIGHT ||
        opcodenum === Opcode.OP_INVERT ||
        opcodenum === Opcode.OP_AND ||
        opcodenum === Opcode.OP_OR ||
        opcodenum === Opcode.OP_XOR ||
        opcodenum === Opcode.OP_2MUL ||
        opcodenum === Opcode.OP_2DIV ||
        opcodenum === Opcode.OP_MUL ||
        opcodenum === Opcode.OP_DIV ||
        opcodenum === Opcode.OP_MOD ||
        opcodenum === Opcode.OP_LSHIFT ||
        opcodenum === Opcode.OP_RSHIFT)
    {
        this.errstr = 'SCRIPT_ERR_DISABLED_OPCODE';
        return false;
    }

    if (fExec && 0 <= opcodenum && opcodenum <= Opcode.OP_PUSHDATA4)
    {
        if (fRequireMinimal && !this.script.checkMinimalPush(this.pc - 1))
        {
            this.errstr = 'SCRIPT_ERR_MINIMALDATA';
            return false;
        }
        if (!chunk.buf)
        {
            this.stack.push(Interpreter.false);
        } else if (chunk.len !== chunk.buf.length)
        {
            throw new Error('Length of push value not equal to length of data');
        } else
        {
            this.stack.push(chunk.buf);
        }
    } else if (fExec || (Opcode.OP_IF <= opcodenum && opcodenum <= Opcode.OP_ENDIF))
    {
        switch (opcodenum)
        {
            // Push value
            case Opcode.OP_1NEGATE:
            case Opcode.OP_1:
            case Opcode.OP_2:
            case Opcode.OP_3:
            case Opcode.OP_4:
            case Opcode.OP_5:
            case Opcode.OP_6:
            case Opcode.OP_7:
            case Opcode.OP_8:
            case Opcode.OP_9:
            case Opcode.OP_10:
            case Opcode.OP_11:
            case Opcode.OP_12:
            case Opcode.OP_13:
            case Opcode.OP_14:
            case Opcode.OP_15:
            case Opcode.OP_16:
                {
                    // ( -- value)
                    // ScriptNum bn((int)opcode - (int)(Opcode.OP_1 - 1));
                    n = opcodenum - (Opcode.OP_1 - 1);
                    buf = new BN(n).toScriptNumBuffer();
                    this.stack.push(buf);
                    // The result of these opcodes should always be the minimal way to push the data
                    // they push, so no need for a CheckMinimalPush here.
                }
                break;


            //
            // Control
            //
            case Opcode.OP_NOP:
                break;

            case Opcode.OP_NOP2:
            case Opcode.OP_CHECKLOCKTIMEVERIFY:

                if (!(this.flags & Interpreter.SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY))
                {
                    // not enabled; treat as a NOP2
                    if (this.flags & Interpreter.SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS)
                    {
                        this.errstr = 'SCRIPT_ERR_DISCOURAGE_UPGRADABLE_NOPS';
                        return false;
                    }
                    break;
                }

                if (this.stack.length < 1)
                {
                    this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                    return false;
                }

                // Note that elsewhere numeric opcodes are limited to
                // operands in the range -2**31+1 to 2**31-1, however it is
                // legal for opcodes to produce results exceeding that
                // range. This limitation is implemented by CScriptNum's
                // default 4-byte limit.
                //
                // If we kept to that limit we'd have a year 2038 problem,
                // even though the nLockTime field in transactions
                // themselves is uint32 which only becomes meaningless
                // after the year 2106.
                //
                // Thus as a special case we tell CScriptNum to accept up
                // to 5-byte bignums, which are good until 2**39-1, well
                // beyond the 2**32-1 limit of the nLockTime field itself.
                var nLockTime = BN.fromScriptNumBuffer(this.stack[this.stack.length - 1], fRequireMinimal, 5);

                // In the rare event that the argument may be < 0 due to
                // some arithmetic being done first, you can always use
                // 0 MAX CHECKLOCKTIMEVERIFY.
                if (nLockTime.lt(new BN(0)))
                {
                    this.errstr = 'SCRIPT_ERR_NEGATIVE_LOCKTIME';
                    return false;
                }

                // Actually compare the specified lock time with the transaction.
                if (!this.checkLockTime(nLockTime))
                {
                    this.errstr = 'SCRIPT_ERR_UNSATISFIED_LOCKTIME';
                    return false;
                }
                break;

            case Opcode.OP_NOP1:
            case Opcode.OP_NOP3:
            case Opcode.OP_NOP4:
            case Opcode.OP_NOP5:
            case Opcode.OP_NOP6:
            case Opcode.OP_NOP7:
            case Opcode.OP_NOP8:
            case Opcode.OP_NOP9:
            case Opcode.OP_NOP10:
                {
                    if (this.flags & Interpreter.SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS)
                    {
                        this.errstr = 'SCRIPT_ERR_DISCOURAGE_UPGRADABLE_NOPS';
                        return false;
                    }
                }
                break;

            case Opcode.OP_IF:
            case Opcode.OP_NOTIF:
                {
                    // <expression> if [statements] [else [statements]] endif
                    // bool fValue = false;
                    fValue = false;
                    if (fExec)
                    {
                        if (this.stack.length < 1)
                        {
                            this.errstr = 'SCRIPT_ERR_UNBALANCED_CONDITIONAL';
                            return false;
                        }
                        buf = this.stack.pop();
                        fValue = Interpreter.castToBool(buf);
                        if (opcodenum === Opcode.OP_NOTIF)
                        {
                            fValue = !fValue;
                        }
                    }
                    this.vfExec.push(fValue);
                }
                break;

            case Opcode.OP_ELSE:
                {
                    if (this.vfExec.length === 0)
                    {
                        this.errstr = 'SCRIPT_ERR_UNBALANCED_CONDITIONAL';
                        return false;
                    }
                    this.vfExec[this.vfExec.length - 1] = !this.vfExec[this.vfExec.length - 1];
                }
                break;

            case Opcode.OP_ENDIF:
                {
                    if (this.vfExec.length === 0)
                    {
                        this.errstr = 'SCRIPT_ERR_UNBALANCED_CONDITIONAL';
                        return false;
                    }
                    this.vfExec.pop();
                }
                break;

            case Opcode.OP_VERIFY:
                {
                    // (true -- ) or
                    // (false -- false) and return
                    if (this.stack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf = this.stack[this.stack.length - 1];
                    fValue = Interpreter.castToBool(buf);
                    if (fValue)
                    {
                        this.stack.pop();
                    } else
                    {
                        this.errstr = 'SCRIPT_ERR_VERIFY';
                        return false;
                    }
                }
                break;

            case Opcode.OP_RETURN:
                {
                    this.errstr = 'SCRIPT_ERR_OP_RETURN';
                    return false;
                }
                break;


            //
            // Stack ops
            //
            case Opcode.OP_TOALTSTACK:
                {
                    if (this.stack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    this.altstack.push(this.stack.pop());
                }
                break;

            case Opcode.OP_FROMALTSTACK:
                {
                    if (this.altstack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_ALTSTACK_OPERATION';
                        return false;
                    }
                    this.stack.push(this.altstack.pop());
                }
                break;

            case Opcode.OP_2DROP:
                {
                    // (x1 x2 -- )
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    this.stack.pop();
                    this.stack.pop();
                }
                break;

            case Opcode.OP_2DUP:
                {
                    // (x1 x2 -- x1 x2 x1 x2)
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf1 = this.stack[this.stack.length - 2];
                    buf2 = this.stack[this.stack.length - 1];
                    this.stack.push(buf1);
                    this.stack.push(buf2);
                }
                break;

            case Opcode.OP_3DUP:
                {
                    // (x1 x2 x3 -- x1 x2 x3 x1 x2 x3)
                    if (this.stack.length < 3)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf1 = this.stack[this.stack.length - 3];
                    buf2 = this.stack[this.stack.length - 2];
                    var buf3 = this.stack[this.stack.length - 1];
                    this.stack.push(buf1);
                    this.stack.push(buf2);
                    this.stack.push(buf3);
                }
                break;

            case Opcode.OP_2OVER:
                {
                    // (x1 x2 x3 x4 -- x1 x2 x3 x4 x1 x2)
                    if (this.stack.length < 4)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf1 = this.stack[this.stack.length - 4];
                    buf2 = this.stack[this.stack.length - 3];
                    this.stack.push(buf1);
                    this.stack.push(buf2);
                }
                break;

            case Opcode.OP_2ROT:
                {
                    // (x1 x2 x3 x4 x5 x6 -- x3 x4 x5 x6 x1 x2)
                    if (this.stack.length < 6)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    spliced = this.stack.splice(this.stack.length - 6, 2);
                    this.stack.push(spliced[0]);
                    this.stack.push(spliced[1]);
                }
                break;

            case Opcode.OP_2SWAP:
                {
                    // (x1 x2 x3 x4 -- x3 x4 x1 x2)
                    if (this.stack.length < 4)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    spliced = this.stack.splice(this.stack.length - 4, 2);
                    this.stack.push(spliced[0]);
                    this.stack.push(spliced[1]);
                }
                break;

            case Opcode.OP_IFDUP:
                {
                    // (x - 0 | x x)
                    if (this.stack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf = this.stack[this.stack.length - 1];
                    fValue = Interpreter.castToBool(buf);
                    if (fValue)
                    {
                        this.stack.push(buf);
                    }
                }
                break;

            case Opcode.OP_DEPTH:
                {
                    // -- stacksize
                    buf = new BN(this.stack.length).toScriptNumBuffer();
                    this.stack.push(buf);
                }
                break;

            case Opcode.OP_DROP:
                {
                    // (x -- )
                    if (this.stack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    this.stack.pop();
                }
                break;

            case Opcode.OP_DUP:
                {
                    // (x -- x x)
                    if (this.stack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    this.stack.push(this.stack[this.stack.length - 1]);
                }
                break;

            case Opcode.OP_NIP:
                {
                    // (x1 x2 -- x2)
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    this.stack.splice(this.stack.length - 2, 1);
                }
                break;

            case Opcode.OP_OVER:
                {
                    // (x1 x2 -- x1 x2 x1)
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    this.stack.push(this.stack[this.stack.length - 2]);
                }
                break;

            case Opcode.OP_PICK:
            case Opcode.OP_ROLL:
                {
                    // (xn ... x2 x1 x0 n - xn ... x2 x1 x0 xn)
                    // (xn ... x2 x1 x0 n - ... x2 x1 x0 xn)
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf = this.stack[this.stack.length - 1];
                    bn = BN.fromScriptNumBuffer(buf, fRequireMinimal);
                    n = bn.toNumber();
                    this.stack.pop();
                    if (n < 0 || n >= this.stack.length)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf = this.stack[this.stack.length - n - 1];
                    if (opcodenum === Opcode.OP_ROLL)
                    {
                        this.stack.splice(this.stack.length - n - 1, 1);
                    }
                    this.stack.push(buf);
                }
                break;

            case Opcode.OP_ROT:
                {
                    // (x1 x2 x3 -- x2 x3 x1)
                    //  x2 x1 x3  after first swap
                    //  x2 x3 x1  after second swap
                    if (this.stack.length < 3)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    x1 = this.stack[this.stack.length - 3];
                    x2 = this.stack[this.stack.length - 2];
                    var x3 = this.stack[this.stack.length - 1];
                    this.stack[this.stack.length - 3] = x2;
                    this.stack[this.stack.length - 2] = x3;
                    this.stack[this.stack.length - 1] = x1;
                }
                break;

            case Opcode.OP_SWAP:
                {
                    // (x1 x2 -- x2 x1)
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    x1 = this.stack[this.stack.length - 2];
                    x2 = this.stack[this.stack.length - 1];
                    this.stack[this.stack.length - 2] = x2;
                    this.stack[this.stack.length - 1] = x1;
                }
                break;

            case Opcode.OP_TUCK:
                {
                    // (x1 x2 -- x2 x1 x2)
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    this.stack.splice(this.stack.length - 2, 0, this.stack[this.stack.length - 1]);
                }
                break;


            case Opcode.OP_SIZE:
                {
                    // (in -- in size)
                    if (this.stack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    bn = new BN(this.stack[this.stack.length - 1].length);
                    this.stack.push(bn.toScriptNumBuffer());
                }
                break;


            //
            // Bitwise logic
            //
            case Opcode.OP_EQUAL:
            case Opcode.OP_EQUALVERIFY:
                //case Opcode.OP_NOTEQUAL: // use Opcode.OP_NUMNOTEQUAL
                {
                    // (x1 x2 - bool)
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf1 = this.stack[this.stack.length - 2];
                    buf2 = this.stack[this.stack.length - 1];
                    var fEqual = buf1.toString('hex') === buf2.toString('hex');
                    this.stack.pop();
                    this.stack.pop();
                    this.stack.push(fEqual ? Interpreter.true : Interpreter.false);
                    if (opcodenum === Opcode.OP_EQUALVERIFY)
                    {
                        if (fEqual)
                        {
                            this.stack.pop();
                        } else
                        {
                            this.errstr = 'SCRIPT_ERR_EQUALVERIFY';
                            return false;
                        }
                    }
                }
                break;


            //
            // Numeric
            //
            case Opcode.OP_1ADD:
            case Opcode.OP_1SUB:
            case Opcode.OP_NEGATE:
            case Opcode.OP_ABS:
            case Opcode.OP_NOT:
            case Opcode.OP_0NOTEQUAL:
                {
                    // (in -- out)
                    if (this.stack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf = this.stack[this.stack.length - 1];
                    bn = BN.fromScriptNumBuffer(buf, fRequireMinimal);
                    switch (opcodenum)
                    {
                        case Opcode.OP_1ADD:
                            bn = bn.add(BN.One);
                            break;
                        case Opcode.OP_1SUB:
                            bn = bn.sub(BN.One);
                            break;
                        case Opcode.OP_NEGATE:
                            bn = bn.neg();
                            break;
                        case Opcode.OP_ABS:
                            if (bn.cmp(BN.Zero) < 0)
                            {
                                bn = bn.neg();
                            }
                            break;
                        case Opcode.OP_NOT:
                            bn = new BN((bn.cmp(BN.Zero) === 0) + 0);
                            break;
                        case Opcode.OP_0NOTEQUAL:
                            bn = new BN((bn.cmp(BN.Zero) !== 0) + 0);
                            break;
                        //default:      assert(!'invalid opcode'); break; // TODO: does this ever occur?
                    }
                    this.stack.pop();
                    this.stack.push(bn.toScriptNumBuffer());
                }
                break;

            case Opcode.OP_ADD:
            case Opcode.OP_SUB:
            case Opcode.OP_BOOLAND:
            case Opcode.OP_BOOLOR:
            case Opcode.OP_NUMEQUAL:
            case Opcode.OP_NUMEQUALVERIFY:
            case Opcode.OP_NUMNOTEQUAL:
            case Opcode.OP_LESSTHAN:
            case Opcode.OP_GREATERTHAN:
            case Opcode.OP_LESSTHANOREQUAL:
            case Opcode.OP_GREATERTHANOREQUAL:
            case Opcode.OP_MIN:
            case Opcode.OP_MAX:
                {
                    // (x1 x2 -- out)
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    bn1 = BN.fromScriptNumBuffer(this.stack[this.stack.length - 2], fRequireMinimal);
                    bn2 = BN.fromScriptNumBuffer(this.stack[this.stack.length - 1], fRequireMinimal);
                    bn = new BN(0);

                    switch (opcodenum)
                    {
                        case Opcode.OP_ADD:
                            bn = bn1.add(bn2);
                            break;

                        case Opcode.OP_SUB:
                            bn = bn1.sub(bn2);
                            break;

                        // case Opcode.OP_BOOLAND:       bn = (bn1 != bnZero && bn2 != bnZero); break;
                        case Opcode.OP_BOOLAND:
                            bn = new BN(((bn1.cmp(BN.Zero) !== 0) && (bn2.cmp(BN.Zero) !== 0)) + 0);
                            break;
                        // case Opcode.OP_BOOLOR:        bn = (bn1 != bnZero || bn2 != bnZero); break;
                        case Opcode.OP_BOOLOR:
                            bn = new BN(((bn1.cmp(BN.Zero) !== 0) || (bn2.cmp(BN.Zero) !== 0)) + 0);
                            break;
                        // case Opcode.OP_NUMEQUAL:      bn = (bn1 == bn2); break;
                        case Opcode.OP_NUMEQUAL:
                            bn = new BN((bn1.cmp(bn2) === 0) + 0);
                            break;
                        // case Opcode.OP_NUMEQUALVERIFY:    bn = (bn1 == bn2); break;
                        case Opcode.OP_NUMEQUALVERIFY:
                            bn = new BN((bn1.cmp(bn2) === 0) + 0);
                            break;
                        // case Opcode.OP_NUMNOTEQUAL:     bn = (bn1 != bn2); break;
                        case Opcode.OP_NUMNOTEQUAL:
                            bn = new BN((bn1.cmp(bn2) !== 0) + 0);
                            break;
                        // case Opcode.OP_LESSTHAN:      bn = (bn1 < bn2); break;
                        case Opcode.OP_LESSTHAN:
                            bn = new BN((bn1.cmp(bn2) < 0) + 0);
                            break;
                        // case Opcode.OP_GREATERTHAN:     bn = (bn1 > bn2); break;
                        case Opcode.OP_GREATERTHAN:
                            bn = new BN((bn1.cmp(bn2) > 0) + 0);
                            break;
                        // case Opcode.OP_LESSTHANOREQUAL:   bn = (bn1 <= bn2); break;
                        case Opcode.OP_LESSTHANOREQUAL:
                            bn = new BN((bn1.cmp(bn2) <= 0) + 0);
                            break;
                        // case Opcode.OP_GREATERTHANOREQUAL:  bn = (bn1 >= bn2); break;
                        case Opcode.OP_GREATERTHANOREQUAL:
                            bn = new BN((bn1.cmp(bn2) >= 0) + 0);
                            break;
                        case Opcode.OP_MIN:
                            bn = (bn1.cmp(bn2) < 0 ? bn1 : bn2);
                            break;
                        case Opcode.OP_MAX:
                            bn = (bn1.cmp(bn2) > 0 ? bn1 : bn2);
                            break;
                        // default:           assert(!'invalid opcode'); break; //TODO: does this ever occur?
                    }
                    this.stack.pop();
                    this.stack.pop();
                    this.stack.push(bn.toScriptNumBuffer());

                    if (opcodenum === Opcode.OP_NUMEQUALVERIFY)
                    {
                        // if (CastToBool(stacktop(-1)))
                        if (Interpreter.castToBool(this.stack[this.stack.length - 1]))
                        {
                            this.stack.pop();
                        } else
                        {
                            this.errstr = 'SCRIPT_ERR_NUMEQUALVERIFY';
                            return false;
                        }
                    }
                }
                break;

            case Opcode.OP_WITHIN:
                {
                    // (x min max -- out)
                    if (this.stack.length < 3)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    bn1 = BN.fromScriptNumBuffer(this.stack[this.stack.length - 3], fRequireMinimal);
                    bn2 = BN.fromScriptNumBuffer(this.stack[this.stack.length - 2], fRequireMinimal);
                    var bn3 = BN.fromScriptNumBuffer(this.stack[this.stack.length - 1], fRequireMinimal);
                    //bool fValue = (bn2 <= bn1 && bn1 < bn3);
                    fValue = (bn2.cmp(bn1) <= 0) && (bn1.cmp(bn3) < 0);
                    this.stack.pop();
                    this.stack.pop();
                    this.stack.pop();
                    this.stack.push(fValue ? Interpreter.true : Interpreter.false);
                }
                break;


            //
            // Crypto
            //
            case Opcode.OP_RIPEMD160:
            case Opcode.OP_SHA1:
            case Opcode.OP_SHA256:
            case Opcode.OP_HASH160:
            case Opcode.OP_HASH256:
                {
                    // (in -- hash)
                    if (this.stack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    buf = this.stack[this.stack.length - 1];
                    //valtype vchHash((opcode == Opcode.OP_RIPEMD160 ||
                    //                 opcode == Opcode.OP_SHA1 || opcode == Opcode.OP_HASH160) ? 20 : 32);
                    var bufHash;
                    if (opcodenum === Opcode.OP_RIPEMD160)
                    {
                        bufHash = Hash.ripemd160(buf);
                    } else if (opcodenum === Opcode.OP_SHA1)
                    {
                        bufHash = Hash.sha1(buf);
                    } else if (opcodenum === Opcode.OP_SHA256)
                    {
                        bufHash = Hash.sha256(buf);
                    } else if (opcodenum === Opcode.OP_HASH160)
                    {
                        bufHash = Hash.sha256ripemd160(buf);
                    } else if (opcodenum === Opcode.OP_HASH256)
                    {
                        bufHash = Hash.sha256sha256(buf);
                    }
                    this.stack.pop();
                    this.stack.push(bufHash);
                }
                break;

            case Opcode.OP_CODESEPARATOR:
                {
                    // Hash starts after the code separator
                    this.pbegincodehash = this.pc;
                }
                break;

            case Opcode.OP_CHECKSIG:
            case Opcode.OP_CHECKSIGVERIFY:
                {
                    // (sig pubkey -- bool)
                    if (this.stack.length < 2)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }

                    bufSig = this.stack[this.stack.length - 2];
                    bufPubkey = this.stack[this.stack.length - 1];

                    // Subset of script starting at the most recent codeseparator
                    // CScript scriptCode(pbegincodehash, pend);
                    subscript = new Script().set({
                        chunks: this.script.chunks.slice(this.pbegincodehash)
                    });

                    // Drop the signature, since there's no way for a signature to sign itself
                    var tmpScript = new Script().add(bufSig);
                    subscript.findAndDelete(tmpScript);

                    if (!this.checkSignatureEncoding(bufSig) || !this.checkPubkeyEncoding(bufPubkey))
                    {
                        return false;
                    }

                    try
                    {
                        sig = Signature.fromTxFormat(bufSig);
                        pubkey = PublicKey.fromBuffer(bufPubkey, false);
                        fSuccess = this.tx.verifySignature(sig, pubkey, this.nin, subscript);
                    } catch (e)
                    {
                        //invalid sig or pubkey
                        fSuccess = false;
                    }

                    this.stack.pop();
                    this.stack.pop();
                    // stack.push_back(fSuccess ? vchTrue : vchFalse);
                    this.stack.push(fSuccess ? Interpreter.true : Interpreter.false);
                    if (opcodenum === Opcode.OP_CHECKSIGVERIFY)
                    {
                        if (fSuccess)
                        {
                            this.stack.pop();
                        } else
                        {
                            this.errstr = 'SCRIPT_ERR_CHECKSIGVERIFY';
                            return false;
                        }
                    }
                }
                break;

            case Opcode.OP_CHECKMULTISIG:
            case Opcode.OP_CHECKMULTISIGVERIFY:
                {
                    // ([sig ...] num_of_signatures [pubkey ...] num_of_pubkeys -- bool)

                    var i = 1;
                    if (this.stack.length < i)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }

                    var nKeysCount = BN.fromScriptNumBuffer(this.stack[this.stack.length - i], fRequireMinimal).toNumber();
                    if (nKeysCount < 0 || nKeysCount > 20)
                    {
                        this.errstr = 'SCRIPT_ERR_PUBKEY_COUNT';
                        return false;
                    }
                    this.nOpCount += nKeysCount;
                    if (this.nOpCount > 201)
                    {
                        this.errstr = 'SCRIPT_ERR_OP_COUNT';
                        return false;
                    }
                    // int ikey = ++i;
                    var ikey = ++i;
                    i += nKeysCount;
                    if (this.stack.length < i)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }

                    var nSigsCount = BN.fromScriptNumBuffer(this.stack[this.stack.length - i], fRequireMinimal).toNumber();
                    if (nSigsCount < 0 || nSigsCount > nKeysCount)
                    {
                        this.errstr = 'SCRIPT_ERR_SIG_COUNT';
                        return false;
                    }
                    // int isig = ++i;
                    var isig = ++i;
                    i += nSigsCount;
                    if (this.stack.length < i)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }

                    // Subset of script starting at the most recent codeseparator
                    subscript = new Script().set({
                        chunks: this.script.chunks.slice(this.pbegincodehash)
                    });

                    // Drop the signatures, since there's no way for a signature to sign itself
                    for (var k = 0; k < nSigsCount; k++)
                    {
                        bufSig = this.stack[this.stack.length - isig - k];
                        subscript.findAndDelete(new Script().add(bufSig));
                    }

                    fSuccess = true;
                    while (fSuccess && nSigsCount > 0)
                    {
                        // valtype& vchSig  = stacktop(-isig);
                        bufSig = this.stack[this.stack.length - isig];
                        // valtype& vchPubKey = stacktop(-ikey);
                        bufPubkey = this.stack[this.stack.length - ikey];

                        if (!this.checkSignatureEncoding(bufSig) || !this.checkPubkeyEncoding(bufPubkey))
                        {
                            return false;
                        }

                        var fOk;
                        try
                        {
                            sig = Signature.fromTxFormat(bufSig);
                            pubkey = PublicKey.fromBuffer(bufPubkey, false);
                            fOk = this.tx.verifySignature(sig, pubkey, this.nin, subscript);
                        } catch (e)
                        {
                            //invalid sig or pubkey
                            fOk = false;
                        }

                        if (fOk)
                        {
                            isig++;
                            nSigsCount--;
                        }
                        ikey++;
                        nKeysCount--;

                        // If there are more signatures left than keys left,
                        // then too many signatures have failed
                        if (nSigsCount > nKeysCount)
                        {
                            fSuccess = false;
                        }
                    }

                    // Clean up stack of actual arguments
                    while (i-- > 1)
                    {
                        this.stack.pop();
                    }

                    // A bug causes CHECKMULTISIG to consume one extra argument
                    // whose contents were not checked in any way.
                    //
                    // Unfortunately this is a potential source of mutability,
                    // so optionally verify it is exactly equal to zero prior
                    // to removing it from the stack.
                    if (this.stack.length < 1)
                    {
                        this.errstr = 'SCRIPT_ERR_INVALID_STACK_OPERATION';
                        return false;
                    }
                    if ((this.flags & Interpreter.SCRIPT_VERIFY_NULLDUMMY) && this.stack[this.stack.length - 1].length)
                    {
                        this.errstr = 'SCRIPT_ERR_SIG_NULLDUMMY';
                        return false;
                    }
                    this.stack.pop();

                    this.stack.push(fSuccess ? Interpreter.true : Interpreter.false);

                    if (opcodenum === Opcode.OP_CHECKMULTISIGVERIFY)
                    {
                        if (fSuccess)
                        {
                            this.stack.pop();
                        } else
                        {
                            this.errstr = 'SCRIPT_ERR_CHECKMULTISIGVERIFY';
                            return false;
                        }
                    }
                }
                break;
            case Opcode.OP_CONTRACT:
            case Opcode.OP_CONTRACT_CHANGE:
                {
                    // do nothing
                }
                break;
            default:
                this.errstr = 'SCRIPT_ERR_BAD_OPCODE';
                return false;
        }
    }

    return true;
};
// script/interpreter end----------------------------------------------------------------------

// transaction/unspentoutput begin----------------------------------------------------------------------
// var _ = require('lodash');
// var $ = require('../util/preconditions');
// var JSUtil = require('../util/js');

// var Script = require('../script');
// var Address = require('../address');
// var Unit = require('../unit');

/**
 * Represents an unspent output information: its script, associated amount and address,
 * transaction id and output index.
 *
 * @constructor
 * @param {object} data
 * @param {string} data.txid the previous transaction id
 * @param {string=} data.txId alias for `txid`
 * @param {number} data.vout the index in the transaction
 * @param {number=} data.outputIndex alias for `vout`
 * @param {string|Script} data.scriptPubKey the script that must be resolved to release the funds
 * @param {string|Script=} data.script alias for `scriptPubKey`
 * @param {number} data.amount amount of bitcoins associated
 * @param {number=} data.satoshis alias for `amount`, but expressed in satoshis (1 BTC = 1e8 satoshis)
 * @param {string|Address=} data.address the associated address to the script, if provided
 */
function UnspentOutput(data)
{
    /* jshint maxcomplexity: 20 */
    /* jshint maxstatements: 20 */
    if (!(this instanceof UnspentOutput))
    {
        return new UnspentOutput(data);
    }
    $.checkArgument(_.isObject(data), 'Must provide an object from where to extract data');
    var address = data.address ? new Address(data.address) : undefined;
    var txId = data.txid ? data.txid : data.txId;
    if (!txId || !JSUtil.isHexaString(txId) || txId.length > 64)
    {
        // TODO: Use the errors library
        throw new Error('Invalid TXID in object', data);
    }
    var outputIndex = _.isUndefined(data.vout) ? data.outputIndex : data.vout;
    if (!_.isNumber(outputIndex))
    {
        throw new Error('Invalid outputIndex, received ' + outputIndex);
    }
    $.checkArgument(!_.isUndefined(data.scriptPubKey) || !_.isUndefined(data.script),
        'Must provide the scriptPubKey for that output!');
    var script = new Script(data.scriptPubKey || data.script);
    $.checkArgument(!_.isUndefined(data.amount) || !_.isUndefined(data.satoshis),
        'Must provide an amount for the output');
    var amount = !_.isUndefined(data.amount) ? new Unit.fromBTC(data.amount).toSatoshis() : data.satoshis;
    $.checkArgument(_.isNumber(amount), 'Amount must be a number');
    JSUtil.defineImmutable(this, {
        address: address,
        txId: txId,
        outputIndex: outputIndex,
        script: script,
        satoshis: amount
    });
}

/**
 * Provide an informative output when displaying this object in the console
 * @returns string
 */
UnspentOutput.prototype.inspect = function ()
{
    return '<UnspentOutput: ' + this.txId + ':' + this.outputIndex +
        ', satoshis: ' + this.satoshis + ', address: ' + this.address + '>';
};

/**
 * String representation: just "txid:index"
 * @returns string
 */
UnspentOutput.prototype.toString = function ()
{
    return this.txId + ':' + this.outputIndex;
};

/**
 * Deserialize an UnspentOutput from an object
 * @param {object|string} data
 * @return UnspentOutput
 */
UnspentOutput.fromObject = function (data)
{
    return new UnspentOutput(data);
};

/**
 * Returns a plain object (no prototype or methods) with the associated info for this output
 * @return {object}
 */
UnspentOutput.prototype.toObject = UnspentOutput.prototype.toJSON = function toObject()
{
    return {
        address: this.address ? this.address.toString() : undefined,
        txid: this.txId,
        vout: this.outputIndex,
        scriptPubKey: this.script.toBuffer().toString('hex'),
        amount: Unit.fromSatoshis(this.satoshis).toBTC()
    };
};
magnachain.UnspentOutput = UnspentOutput;
// transaction/unspentoutput end----------------------------------------------------------------------

// transaction/signature begin----------------------------------------------------------------------
// var _ = require('lodash');
// var $ = require('../util/preconditions');
// var inherits = require('inherits');
// var BufferUtil = require('../util/buffer');
// var JSUtil = require('../util/js');

// var PublicKey = require('../publickey');
// var errors = require('../errors');
// var Signature = require('../crypto/signature');

/**
 * @desc
 * Wrapper around Signature with fields related to signing a transaction specifically
 *
 * @param {Object|string|TransactionSignature} arg
 * @constructor
 */
function TransactionSignature(arg)
{
    if (!(this instanceof TransactionSignature))
    {
        return new TransactionSignature(arg);
    }
    if (arg instanceof TransactionSignature)
    {
        return arg;
    }
    if (_.isObject(arg))
    {
        return this._fromObject(arg);
    }
    throw new errors.InvalidArgument('TransactionSignatures must be instantiated from an object');
}
inherits(TransactionSignature, Signature);

TransactionSignature.prototype._fromObject = function (arg)
{
    this._checkObjectArgs(arg);
    this.publicKey = new PublicKey(arg.publicKey);
    this.prevTxId = BufferUtil.isBuffer(arg.prevTxId) ? arg.prevTxId : new Buffer(arg.prevTxId, 'hex');
    this.outputIndex = arg.outputIndex;
    this.inputIndex = arg.inputIndex;
    this.signature = (arg.signature instanceof Signature) ? arg.signature :
        BufferUtil.isBuffer(arg.signature) ? Signature.fromBuffer(arg.signature) :
            Signature.fromString(arg.signature);
    this.sigtype = arg.sigtype;
    return this;
};

TransactionSignature.prototype._checkObjectArgs = function (arg)
{
    $.checkArgument(PublicKey(arg.publicKey), 'publicKey');
    $.checkArgument(!_.isUndefined(arg.inputIndex), 'inputIndex');
    $.checkArgument(!_.isUndefined(arg.outputIndex), 'outputIndex');
    $.checkState(_.isNumber(arg.inputIndex), 'inputIndex must be a number');
    $.checkState(_.isNumber(arg.outputIndex), 'outputIndex must be a number');
    $.checkArgument(arg.signature, 'signature');
    $.checkArgument(arg.prevTxId, 'prevTxId');
    $.checkState(arg.signature instanceof Signature ||
        BufferUtil.isBuffer(arg.signature) ||
        JSUtil.isHexa(arg.signature), 'signature must be a buffer or hexa value');
    $.checkState(BufferUtil.isBuffer(arg.prevTxId) ||
        JSUtil.isHexa(arg.prevTxId), 'prevTxId must be a buffer or hexa value');
    $.checkArgument(arg.sigtype, 'sigtype');
    $.checkState(_.isNumber(arg.sigtype), 'sigtype must be a number');
};

/**
 * Serializes a transaction to a plain JS object
 * @return {Object}
 */
TransactionSignature.prototype.toObject = TransactionSignature.prototype.toJSON = function toObject()
{
    return {
        publicKey: this.publicKey.toString(),
        prevTxId: this.prevTxId.toString('hex'),
        outputIndex: this.outputIndex,
        inputIndex: this.inputIndex,
        signature: this.signature.toString(),
        sigtype: this.sigtype
    };
};

/**
 * Builds a TransactionSignature from an object
 * @param {Object} object
 * @return {TransactionSignature}
 */
TransactionSignature.fromObject = function (object)
{
    $.checkArgument(object);
    return new TransactionSignature(object);
};

magnachain.TransactionSignature = TransactionSignature;
// transaction/signature end----------------------------------------------------------------------

// transaction/output begin----------------------------------------------------------------------
// var _ = require('lodash');
// var BN = require('../crypto/bn');
// var buffer = require('buffer');
// var bufferUtil = require('../util/buffer');
// var JSUtil = require('../util/js');
// var BufferWriter = require('../encoding/bufferwriter');
// var Script = require('../script');
// var $ = require('../util/preconditions');
// var errors = require('../errors');

var MAX_SAFE_INTEGER = 0x1fffffffffffff;

function Output(args)
{
    if (!(this instanceof Output))
    {
        return new Output(args);
    }
    if (_.isObject(args))
    {
        this.satoshis = args.satoshis;
        if (BufferUtil.isBuffer(args.script))
        {
            this._scriptBuffer = args.script;
        } else
        {
            var script;
            if (_.isString(args.script) && JSUtil.isHexa(args.script))
            {
                script = new Buffer(args.script, 'hex');
            } else
            {
                script = args.script;
            }
            this.setScript(script);
        }
    } else
    {
        throw new TypeError('Unrecognized argument for Output');
    }
}

Object.defineProperty(Output.prototype, 'script', {
    configurable: false,
    enumerable: true,
    get: function ()
    {
        if (this._script)
        {
            return this._script;
        } else
        {
            this.setScriptFromBuffer(this._scriptBuffer);
            return this._script;
        }

    }
});

Object.defineProperty(Output.prototype, 'satoshis', {
    configurable: false,
    enumerable: true,
    get: function ()
    {
        return this._satoshis;
    },
    set: function (num)
    {
        if (num instanceof BN)
        {
            this._satoshisBN = num;
            this._satoshis = num.toNumber();
        } else if (_.isString(num))
        {
            this._satoshis = parseInt(num);
            this._satoshisBN = BN.fromNumber(this._satoshis);
        } else
        {
            $.checkArgument(
                JSUtil.isNaturalNumber(num),
                'Output satoshis is not a natural number'
            );
            this._satoshisBN = BN.fromNumber(num);
            this._satoshis = num;
        }
        $.checkState(
            JSUtil.isNaturalNumber(this._satoshis),
            'Output satoshis is not a natural number'
        );
    }
});

Output.prototype.invalidSatoshis = function ()
{
    if (this._satoshis > MAX_SAFE_INTEGER)
    {
        return 'transaction txout satoshis greater than max safe integer';
    }
    if (this._satoshis !== this._satoshisBN.toNumber())
    {
        return 'transaction txout satoshis has corrupted value';
    }
    if (this._satoshis < 0)
    {
        return 'transaction txout negative';
    }
    return false;
};

Output.prototype.toObject = Output.prototype.toJSON = function toObject()
{
    var obj = {
        satoshis: this.satoshis
    };
    obj.script = this._scriptBuffer.toString('hex');
    return obj;
};

Output.fromObject = function (data)
{
    return new Output(data);
};

Output.prototype.setScriptFromBuffer = function (buffer)
{
    this._scriptBuffer = buffer;
    try
    {
        this._script = Script.fromBuffer(this._scriptBuffer);
        this._script._isOutput = true;
    } catch (e)
    {
        if (e instanceof errors.Script.InvalidBuffer)
        {
            this._script = null;
        } else
        {
            throw e;
        }
    }
};

Output.prototype.setScript = function (script)
{
    if (script instanceof Script)
    {
        this._scriptBuffer = script.toBuffer();
        this._script = script;
        this._script._isOutput = true;
    } else if (_.isString(script))
    {
        this._script = Script.fromString(script);
        this._scriptBuffer = this._script.toBuffer();
        this._script._isOutput = true;
    } else if (BufferUtil.isBuffer(script))
    {
        this.setScriptFromBuffer(script);
    } else
    {
        throw new TypeError('Invalid argument type: script');
    }
    return this;
};

Output.prototype.inspect = function ()
{
    var scriptStr;
    if (this.script)
    {
        scriptStr = this.script.inspect();
    } else
    {
        scriptStr = this._scriptBuffer.toString('hex');
    }
    return '<Output (' + this.satoshis + ' sats) ' + scriptStr + '>';
};

Output.fromBufferReader = function (br)
{
    var obj = {};
    obj.satoshis = br.readUInt64LEBN();
    var size = br.readVarintNum();
    if (size !== 0)
    {
        obj.script = br.read(size);
    } else
    {
        obj.script = new Buffer([]);
    }
    return new Output(obj);
};

Output.prototype.toBufferWriter = function (writer)
{
    if (!writer)
    {
        writer = new BufferWriter();
    }
    writer.writeUInt64LEBN(this._satoshisBN);
    var script = this._scriptBuffer;
    writer.writeVarintNum(script.length);
    writer.write(script);
    return writer;
};

magnachain.Output = Output;
// transaction/output end----------------------------------------------------------------------

// transaction/sighash begin----------------------------------------------------------------------
// var buffer = require('buffer');

// var Signature = require('../crypto/signature');
// var Script = require('../script');
// var Output = require('./output');
// var BufferReader = require('../encoding/bufferreader');
// var BufferWriter = require('../encoding/bufferwriter');
// var BN = require('../crypto/bn');
// var Hash = require('../crypto/hash');
// var ECDSA = require('../crypto/ecdsa');
// var $ = require('../util/preconditions');
// var _ = require('lodash');

var SIGHASH_SINGLE_BUG = '0000000000000000000000000000000000000000000000000000000000000001';
var BITS_64_ON = 'ffffffffffffffff';

/**
 * Returns a buffer of length 32 bytes with the hash that needs to be signed
 * for OP_CHECKSIG.
 *
 * @name Signing.sighash
 * @param {Transaction} transaction the transaction to sign
 * @param {number} sighashType the type of the hash
 * @param {number} inputNumber the input index for the signature
 * @param {Script} subscript the script that will be signed
 */
var sighash = function sighash(transaction, sighashType, inputNumber, subscript)
{
    //var Transaction = require('./transaction');
    //var Input = require('./input');

    var i;
    // Copy transaction
    var txcopy = Transaction.shallowCopy(transaction);

    // Copy script
    subscript = new Script(subscript);
    subscript.removeCodeseparators();

    for (i = 0; i < txcopy.inputs.length; i++)
    {
        // Blank signatures for other inputs
        txcopy.inputs[i] = new Input(txcopy.inputs[i]).setScript(Script.empty());
    }

    txcopy.inputs[inputNumber] = new Input(txcopy.inputs[inputNumber]).setScript(subscript);

    if ((sighashType & 31) === Signature.SIGHASH_NONE ||
        (sighashType & 31) === Signature.SIGHASH_SINGLE)
    {

        // clear all sequenceNumbers
        for (i = 0; i < txcopy.inputs.length; i++)
        {
            if (i !== inputNumber)
            {
                txcopy.inputs[i].sequenceNumber = 0;
            }
        }
    }

    if ((sighashType & 31) === Signature.SIGHASH_NONE)
    {
        txcopy.outputs = [];

    } else if ((sighashType & 31) === Signature.SIGHASH_SINGLE)
    {
        // The SIGHASH_SINGLE bug.
        // https://bitcointalk.org/index.php?topic=260595.0
        if (inputNumber >= txcopy.outputs.length)
        {
            return new Buffer(SIGHASH_SINGLE_BUG, 'hex');
        }

        txcopy.outputs.length = inputNumber + 1;

        for (i = 0; i < inputNumber; i++)
        {
            txcopy.outputs[i] = new Output({
                satoshis: BN.fromBuffer(new Buffer(BITS_64_ON, 'hex')),
                script: Script.empty()
            });
        }
    }

    if (sighashType & Signature.SIGHASH_ANYONECANPAY)
    {
        txcopy.inputs = [txcopy.inputs[inputNumber]];
    }

    var buf = new BufferWriter()
        .write(txcopy.toBufferForSign())
        .writeInt32LE(sighashType)
        .toBuffer();
    var ret = Hash.sha256sha256(buf);
    ret = new BufferReader(ret).readReverse();
    return ret;
};

/**
 * Create a signature
 *
 * @name Signing.sign
 * @param {Transaction} transaction
 * @param {PrivateKey} privateKey
 * @param {number} sighash
 * @param {number} inputIndex
 * @param {Script} subscript
 * @return {Signature}
 */
function sign(transaction, privateKey, sighashType, inputIndex, subscript)
{
    var hashbuf = sighash(transaction, sighashType, inputIndex, subscript);
    var sig = ECDSA.sign(hashbuf, privateKey, 'little').set({
        nhashtype: sighashType
    });
    return sig;
}

/**
 * Verify a signature
 *
 * @name Signing.verify
 * @param {Transaction} transaction
 * @param {Signature} signature
 * @param {PublicKey} publicKey
 * @param {number} inputIndex
 * @param {Script} subscript
 * @return {boolean}
 */
function verify(transaction, signature, publicKey, inputIndex, subscript)
{
    $.checkArgument(!_.isUndefined(transaction));
    $.checkArgument(!_.isUndefined(signature) && !_.isUndefined(signature.nhashtype));
    var hashbuf = sighash(transaction, signature.nhashtype, inputIndex, subscript);
    return ECDSA.verify(hashbuf, signature, publicKey, 'little');
}

/**
 * @namespace Signing
 */

var Sighash = {};

Sighash.sighash = sighash;
Sighash.sign = sign;
Sighash.verify = verify;

magnachain.Sighash = Sighash;
// module.exports = {
//     sighash: sighash,
//     sign: sign,
//     verify: verify
// };
// transaction/sighash end----------------------------------------------------------------------

// transaction/input/input begin----------------------------------------------------------------------
// var _ = require('lodash');
// var $ = require('../../util/preconditions');
// var errors = require('../../errors');
// var BufferWriter = require('../../encoding/bufferwriter');
// var buffer = require('buffer');
// var BufferUtil = require('../../util/buffer');
// var JSUtil = require('../../util/js');
// var Script = require('../../script');
// var Sighash = require('../sighash');
// var Output = require('../output');

var MAXINT = 0xffffffff; // Math.pow(2, 32) - 1;
var DEFAULT_RBF_SEQNUMBER = MAXINT - 2;
var DEFAULT_SEQNUMBER = MAXINT;
var DEFAULT_LOCKTIME_SEQNUMBER = MAXINT - 1;

function Input(params)
{
    if (!(this instanceof Input))
    {
        return new Input(params);
    }
    if (params)
    {
        return this._fromObject(params);
    }
}

Input.MAXINT = MAXINT;
Input.DEFAULT_SEQNUMBER = DEFAULT_SEQNUMBER;
Input.DEFAULT_LOCKTIME_SEQNUMBER = DEFAULT_LOCKTIME_SEQNUMBER;
Input.DEFAULT_RBF_SEQNUMBER = DEFAULT_RBF_SEQNUMBER;

Object.defineProperty(Input.prototype, 'script', {
    configurable: false,
    enumerable: true,
    get: function ()
    {
        if (this.isNull())
        {
            return null;
        }
        if (!this._script)
        {
            this._script = new Script(this._scriptBuffer);
            this._script._isInput = true;
        }
        return this._script;
    }
});

Input.fromObject = function (obj)
{
    $.checkArgument(_.isObject(obj));
    var input = new Input();
    return input._fromObject(obj);
};

Input.prototype._fromObject = function (params)
{
    var prevTxId;
    if (_.isString(params.prevTxId) && JSUtil.isHexa(params.prevTxId))
    {
        prevTxId = new Buffer(params.prevTxId, 'hex');
    } else
    {
        prevTxId = params.prevTxId;
    }
    this.output = params.output ?
        (params.output instanceof Output ? params.output : new Output(params.output)) : undefined;
    this.prevTxId = prevTxId || params.txidbuf;
    this.outputIndex = _.isUndefined(params.outputIndex) ? params.txoutnum : params.outputIndex;
    this.sequenceNumber = _.isUndefined(params.sequenceNumber) ?
        (_.isUndefined(params.seqnum) ? DEFAULT_SEQNUMBER : params.seqnum) : params.sequenceNumber;
    if (_.isUndefined(params.script) && _.isUndefined(params.scriptBuffer))
    {
        throw new errors.Transaction.Input.MissingScript();
    }
    this.setScript(params.scriptBuffer || params.script);
    return this;
};

Input.prototype.toObject = Input.prototype.toJSON = function toObject()
{
    var obj = {
        prevTxId: this.prevTxId.toString('hex'),
        outputIndex: this.outputIndex,
        sequenceNumber: this.sequenceNumber,
        script: this._scriptBuffer.toString('hex'),
    };
    // add human readable form if input contains valid script
    if (this.script)
    {
        obj.scriptString = this.script.toString();
    }
    if (this.output)
    {
        obj.output = this.output.toObject();
    }
    return obj;
};

Input.fromBufferReader = function (br)
{
    //var input = new Input();
    var input = new PublicKeyHashInput();

    input.prevTxId = br.readReverse(32);
    input.outputIndex = br.readUInt32LE();
    input._scriptBuffer = br.readVarLengthBuffer();
    input.sequenceNumber = br.readUInt32LE();
    // TODO: return different classes according to which input it is
    // e.g: CoinbaseInput, PublicKeyHashInput, MultiSigScriptHashInput, ContactInput etc.
    return input;
};

Input.prototype.toBufferWriter = function (writer)
{
    if (!writer)
    {
        writer = new BufferWriter();
    }
    writer.writeReverse(this.prevTxId);
    writer.writeUInt32LE(this.outputIndex);
    var script = this._scriptBuffer;
    writer.writeVarintNum(script.length);
    writer.write(script);
    writer.writeUInt32LE(this.sequenceNumber);
    return writer;
};

Input.prototype.setScript = function (script)
{
    this._script = null;
    if (script instanceof Script)
    {
        this._script = script;
        this._script._isInput = true;
        this._scriptBuffer = script.toBuffer();
    } else if (JSUtil.isHexa(script))
    {
        // hex string script
        this._scriptBuffer = new Buffer(script, 'hex');
    } else if (_.isString(script))
    {
        // human readable string script
        this._script = new Script(script);
        this._script._isInput = true;
        this._scriptBuffer = this._script.toBuffer();
    } else if (BufferUtil.isBuffer(script))
    {
        // buffer script
        this._scriptBuffer = new Buffer(script);
    } else
    {
        throw new TypeError('Invalid argument type: script');
    }
    return this;
};

// 手动设置 output
Input.prototype.setOutput = function (output)
{
    this.output = output;
    return this;
}

/**
 * Retrieve signatures for the provided PrivateKey.
 *
 * @param {Transaction} transaction - the transaction to be signed
 * @param {PrivateKey} privateKey - the private key to use when signing
 * @param {number} inputIndex - the index of this input in the provided transaction
 * @param {number} sigType - defaults to Signature.SIGHASH_ALL
 * @param {Buffer} addressHash - if provided, don't calculate the hash of the
 *     public key associated with the private key provided
 * @abstract
 */
Input.prototype.getSignatures = function ()
{
    throw new errors.AbstractMethodInvoked(
        'Trying to sign unsupported output type (only P2PKH and P2SH multisig inputs are supported)' +
        ' for input: ' + JSON.stringify(this)
    );
};

Input.prototype.isFullySigned = function ()
{
    throw new errors.AbstractMethodInvoked('Input#isFullySigned');
};

Input.prototype.isFinal = function ()
{
    return this.sequenceNumber !== 4294967295;
};

Input.prototype.addSignature = function ()
{
    throw new errors.AbstractMethodInvoked('Input#addSignature');
};

Input.prototype.clearSignatures = function ()
{
    throw new errors.AbstractMethodInvoked('Input#clearSignatures');
};

Input.prototype.isValidSignature = function (transaction, signature)
{
    // FIXME: Refactor signature so this is not necessary
    signature.signature.nhashtype = signature.sigtype;
    return Sighash.verify(
        transaction,
        signature.signature,
        signature.publicKey,
        signature.inputIndex,
        this.output.script
    );
};

/**
 * @returns true if this is a coinbase input (represents no input)
 */
Input.prototype.isNull = function ()
{
    return this.prevTxId.toString('hex') === '0000000000000000000000000000000000000000000000000000000000000000' &&
        this.outputIndex === 0xffffffff;
};

Input.prototype._estimateSize = function ()
{
    return this.toBufferWriter().toBuffer().length;
};

magnachain.Input = Input;
// transaction/input/input end----------------------------------------------------------------------

// transaction/input/multisig begin----------------------------------------------------------------------
// var _ = require('lodash');
// var inherits = require('inherits');
// var Transaction = require('../transaction');
// var Input = require('./input');
// var Output = require('../output');
// var $ = require('../../util/preconditions');

// var Script = require('../../script');
// var Signature = require('../../crypto/signature');
// var Sighash = require('../sighash');
// var PublicKey = require('../../publickey');
// var BufferUtil = require('../../util/buffer');
// var TransactionSignature = require('../signature');

/**
 * @constructor
 */
function MultiSigInput(input, pubkeys, threshold, signatures)
{
    Input.apply(this, arguments);
    var self = this;
    pubkeys = pubkeys || input.publicKeys;
    threshold = threshold || input.threshold;
    signatures = signatures || input.signatures;
    this.publicKeys = _.sortBy(pubkeys, function (publicKey) { return publicKey.toString('hex'); });
    $.checkState(Script.buildMultisigOut(this.publicKeys, threshold).equals(this.output.script),
        'Provided public keys don\'t match to the provided output script');
    this.publicKeyIndex = {};
    _.each(this.publicKeys, function (publicKey, index)
    {
        self.publicKeyIndex[publicKey.toString()] = index;
    });
    this.threshold = threshold;
    // Empty array of signatures
    this.signatures = signatures ? this._deserializeSignatures(signatures) : new Array(this.publicKeys.length);
}
inherits(MultiSigInput, Input);

MultiSigInput.prototype.toObject = function ()
{
    var obj = Input.prototype.toObject.apply(this, arguments);
    obj.threshold = this.threshold;
    obj.publicKeys = _.map(this.publicKeys, function (publicKey) { return publicKey.toString(); });
    obj.signatures = this._serializeSignatures();
    return obj;
};

MultiSigInput.prototype._deserializeSignatures = function (signatures)
{
    return _.map(signatures, function (signature)
    {
        if (!signature)
        {
            return undefined;
        }
        return new TransactionSignature(signature);
    });
};

MultiSigInput.prototype._serializeSignatures = function ()
{
    return _.map(this.signatures, function (signature)
    {
        if (!signature)
        {
            return undefined;
        }
        return signature.toObject();
    });
};

MultiSigInput.prototype.getSignatures = function (transaction, privateKey, index, sigtype)
{
    $.checkState(this.output instanceof Output);
    sigtype = sigtype || Signature.SIGHASH_ALL;

    var self = this;
    var results = [];
    _.each(this.publicKeys, function (publicKey)
    {
        if (publicKey.toString() === privateKey.publicKey.toString())
        {
            results.push(new TransactionSignature({
                publicKey: privateKey.publicKey,
                prevTxId: self.prevTxId,
                outputIndex: self.outputIndex,
                inputIndex: index,
                signature: Sighash.sign(transaction, privateKey, sigtype, index, self.output.script),
                sigtype: sigtype
            }));
        }
    });

    return results;
};

MultiSigInput.prototype.addSignature = function (transaction, signature)
{
    $.checkState(!this.isFullySigned(), 'All needed signatures have already been added');
    $.checkArgument(!_.isUndefined(this.publicKeyIndex[signature.publicKey.toString()]),
        'Signature has no matching public key');
    $.checkState(this.isValidSignature(transaction, signature));
    this.signatures[this.publicKeyIndex[signature.publicKey.toString()]] = signature;
    this._updateScript();
    return this;
};

MultiSigInput.prototype._updateScript = function ()
{
    this.setScript(Script.buildMultisigIn(
        this.publicKeys,
        this.threshold,
        this._createSignatures()
    ));
    return this;
};

MultiSigInput.prototype._createSignatures = function ()
{
    return _.map(
        _.filter(this.signatures, function (signature) { return !_.isUndefined(signature); }),
        function (signature)
        {
            return BufferUtil.concat([
                signature.signature.toDER(),
                BufferUtil.integerAsSingleByteBuffer(signature.sigtype)
            ]);
        }
    );
};

MultiSigInput.prototype.clearSignatures = function ()
{
    this.signatures = new Array(this.publicKeys.length);
    this._updateScript();
};

MultiSigInput.prototype.isFullySigned = function ()
{
    return this.countSignatures() === this.threshold;
};

MultiSigInput.prototype.countMissingSignatures = function ()
{
    return this.threshold - this.countSignatures();
};

MultiSigInput.prototype.countSignatures = function ()
{
    return _.reduce(this.signatures, function (sum, signature)
    {
        return sum + (!!signature);
    }, 0);
};

MultiSigInput.prototype.publicKeysWithoutSignature = function ()
{
    var self = this;
    return _.filter(this.publicKeys, function (publicKey)
    {
        return !(self.signatures[self.publicKeyIndex[publicKey.toString()]]);
    });
};

MultiSigInput.prototype.isValidSignature = function (transaction, signature)
{
    // FIXME: Refactor signature so this is not necessary
    signature.signature.nhashtype = signature.sigtype;
    return Sighash.verify(
        transaction,
        signature.signature,
        signature.publicKey,
        signature.inputIndex,
        this.output.script
    );
};

/**
 *
 * @param {Buffer[]} signatures
 * @param {PublicKey[]} publicKeys
 * @param {Transaction} transaction
 * @param {Integer} inputIndex
 * @param {Input} input
 * @returns {TransactionSignature[]}
 */
MultiSigInput.normalizeSignatures = function (transaction, input, inputIndex, signatures, publicKeys)
{
    return publicKeys.map(function (pubKey)
    {
        var signatureMatch = null;
        signatures = signatures.filter(function (signatureBuffer)
        {
            if (signatureMatch)
            {
                return true;
            }

            var signature = new TransactionSignature({
                signature: Signature.fromTxFormat(signatureBuffer),
                publicKey: pubKey,
                prevTxId: input.prevTxId,
                outputIndex: input.outputIndex,
                inputIndex: inputIndex,
                sigtype: Signature.SIGHASH_ALL
            });

            signature.signature.nhashtype = signature.sigtype;
            var isMatch = Sighash.verify(
                transaction,
                signature.signature,
                signature.publicKey,
                signature.inputIndex,
                input.output.script
            );

            if (isMatch)
            {
                signatureMatch = signature;
                return false;
            }

            return true;
        });

        return signatureMatch ? signatureMatch : null;
    });
};

MultiSigInput.OPCODES_SIZE = 1; // 0
MultiSigInput.SIGNATURE_SIZE = 73; // size (1) + DER (<=72)

MultiSigInput.prototype._estimateSize = function ()
{
    return MultiSigInput.OPCODES_SIZE +
        this.threshold * MultiSigInput.SIGNATURE_SIZE;
};

magnachain.MultiSigInput = MultiSigInput;
// transaction/input/multisig end----------------------------------------------------------------------

// transaction/input/multisigscripthash begin----------------------------------------------------------------------
// var _ = require('lodash');
// var inherits = require('inherits');
// var Input = require('./input');
// var Output = require('../output');
// var $ = require('../../util/preconditions');

// var Script = require('../../script');
// var Signature = require('../../crypto/signature');
// var Sighash = require('../sighash');
// var PublicKey = require('../../publickey');
// var BufferUtil = require('../../util/buffer');
// var TransactionSignature = require('../signature');

/**
 * @constructor
 */
function MultiSigScriptHashInput(input, pubkeys, threshold, signatures)
{
    Input.apply(this, arguments);
    var self = this;
    pubkeys = pubkeys || input.publicKeys;
    threshold = threshold || input.threshold;
    signatures = signatures || input.signatures;
    this.publicKeys = _.sortBy(pubkeys, function (publicKey) { return publicKey.toString('hex'); });
    this.redeemScript = Script.buildMultisigOut(this.publicKeys, threshold);
    $.checkState(Script.buildScriptHashOut(this.redeemScript).equals(this.output.script),
        'Provided public keys don\'t hash to the provided output');
    this.publicKeyIndex = {};
    _.each(this.publicKeys, function (publicKey, index)
    {
        self.publicKeyIndex[publicKey.toString()] = index;
    });
    this.threshold = threshold;
    // Empty array of signatures
    this.signatures = signatures ? this._deserializeSignatures(signatures) : new Array(this.publicKeys.length);
}
inherits(MultiSigScriptHashInput, Input);

MultiSigScriptHashInput.prototype.toObject = function ()
{
    var obj = Input.prototype.toObject.apply(this, arguments);
    obj.threshold = this.threshold;
    obj.publicKeys = _.map(this.publicKeys, function (publicKey) { return publicKey.toString(); });
    obj.signatures = this._serializeSignatures();
    return obj;
};

MultiSigScriptHashInput.prototype._deserializeSignatures = function (signatures)
{
    return _.map(signatures, function (signature)
    {
        if (!signature)
        {
            return undefined;
        }
        return new TransactionSignature(signature);
    });
};

MultiSigScriptHashInput.prototype._serializeSignatures = function ()
{
    return _.map(this.signatures, function (signature)
    {
        if (!signature)
        {
            return undefined;
        }
        return signature.toObject();
    });
};

MultiSigScriptHashInput.prototype.getSignatures = function (transaction, privateKey, index, sigtype)
{
    $.checkState(this.output instanceof Output);
    sigtype = sigtype || Signature.SIGHASH_ALL;

    var self = this;
    var results = [];
    _.each(this.publicKeys, function (publicKey)
    {
        if (publicKey.toString() === privateKey.publicKey.toString())
        {
            results.push(new TransactionSignature({
                publicKey: privateKey.publicKey,
                prevTxId: self.prevTxId,
                outputIndex: self.outputIndex,
                inputIndex: index,
                signature: Sighash.sign(transaction, privateKey, sigtype, index, self.redeemScript),
                sigtype: sigtype
            }));
        }
    });
    return results;
};

MultiSigScriptHashInput.prototype.addSignature = function (transaction, signature)
{
    $.checkState(!this.isFullySigned(), 'All needed signatures have already been added');
    $.checkArgument(!_.isUndefined(this.publicKeyIndex[signature.publicKey.toString()]),
        'Signature has no matching public key');
    $.checkState(this.isValidSignature(transaction, signature));
    this.signatures[this.publicKeyIndex[signature.publicKey.toString()]] = signature;
    this._updateScript();
    return this;
};

MultiSigScriptHashInput.prototype._updateScript = function ()
{
    this.setScript(Script.buildP2SHMultisigIn(
        this.publicKeys,
        this.threshold,
        this._createSignatures(),
        { cachedMultisig: this.redeemScript }
    ));
    return this;
};

MultiSigScriptHashInput.prototype._createSignatures = function ()
{
    return _.map(
        _.filter(this.signatures, function (signature) { return !_.isUndefined(signature); }),
        function (signature)
        {
            return BufferUtil.concat([
                signature.signature.toDER(),
                BufferUtil.integerAsSingleByteBuffer(signature.sigtype)
            ]);
        }
    );
};

MultiSigScriptHashInput.prototype.clearSignatures = function ()
{
    this.signatures = new Array(this.publicKeys.length);
    this._updateScript();
};

MultiSigScriptHashInput.prototype.isFullySigned = function ()
{
    return this.countSignatures() === this.threshold;
};

MultiSigScriptHashInput.prototype.countMissingSignatures = function ()
{
    return this.threshold - this.countSignatures();
};

MultiSigScriptHashInput.prototype.countSignatures = function ()
{
    return _.reduce(this.signatures, function (sum, signature)
    {
        return sum + (!!signature);
    }, 0);
};

MultiSigScriptHashInput.prototype.publicKeysWithoutSignature = function ()
{
    var self = this;
    return _.filter(this.publicKeys, function (publicKey)
    {
        return !(self.signatures[self.publicKeyIndex[publicKey.toString()]]);
    });
};

MultiSigScriptHashInput.prototype.isValidSignature = function (transaction, signature)
{
    // FIXME: Refactor signature so this is not necessary
    signature.signature.nhashtype = signature.sigtype;
    return Sighash.verify(
        transaction,
        signature.signature,
        signature.publicKey,
        signature.inputIndex,
        this.redeemScript
    );
};

MultiSigScriptHashInput.OPCODES_SIZE = 7; // serialized size (<=3) + 0 .. N .. M OP_CHECKMULTISIG
MultiSigScriptHashInput.SIGNATURE_SIZE = 74; // size (1) + DER (<=72) + sighash (1)
MultiSigScriptHashInput.PUBKEY_SIZE = 34; // size (1) + DER (<=33)

MultiSigScriptHashInput.prototype._estimateSize = function ()
{
    return MultiSigScriptHashInput.OPCODES_SIZE +
        this.threshold * MultiSigScriptHashInput.SIGNATURE_SIZE +
        this.publicKeys.length * MultiSigScriptHashInput.PUBKEY_SIZE;
};

magnachain.MultiSigScriptHashInput = MultiSigScriptHashInput;
// transaction/input/multisigscripthash end----------------------------------------------------------------------

// transaction/input/publickey begin----------------------------------------------------------------------
// var inherits = require('inherits');

// var $ = require('../../util/preconditions');
// var BufferUtil = require('../../util/buffer');

// var Input = require('./input');
// var Output = require('../output');
// var Sighash = require('../sighash');
// var Script = require('../../script');
// var Signature = require('../../crypto/signature');
// var TransactionSignature = require('../signature');

/**
 * Represents a special kind of input of PayToPublicKey kind.
 * @constructor
 */
function PublicKeyInput()
{
    Input.apply(this, arguments);
}
inherits(PublicKeyInput, Input);

/**
 * @param {Transaction} transaction - the transaction to be signed
 * @param {PrivateKey} privateKey - the private key with which to sign the transaction
 * @param {number} index - the index of the input in the transaction input vector
 * @param {number=} sigtype - the type of signature, defaults to Signature.SIGHASH_ALL
 * @return {Array} of objects that can be
 */
PublicKeyInput.prototype.getSignatures = function (transaction, privateKey, index, sigtype)
{
    $.checkState(this.output instanceof Output);
    sigtype = sigtype || Signature.SIGHASH_ALL;
    var publicKey = privateKey.toPublicKey();
    if (publicKey.toString() === this.output.script.getPublicKey().toString('hex'))
    {
        return [new TransactionSignature({
            publicKey: publicKey,
            prevTxId: this.prevTxId,
            outputIndex: this.outputIndex,
            inputIndex: index,
            signature: Sighash.sign(transaction, privateKey, sigtype, index, this.output.script),
            sigtype: sigtype
        })];
    }
    return [];
};

/**
 * Add the provided signature
 *
 * @param {Object} signature
 * @param {PublicKey} signature.publicKey
 * @param {Signature} signature.signature
 * @param {number=} signature.sigtype
 * @return {PublicKeyInput} this, for chaining
 */
PublicKeyInput.prototype.addSignature = function (transaction, signature)
{
    $.checkState(this.isValidSignature(transaction, signature), 'Signature is invalid');
    this.setScript(Script.buildPublicKeyIn(
        signature.signature.toDER(),
        signature.sigtype
    ));
    return this;
};

/**
 * Clear the input's signature
 * @return {PublicKeyHashInput} this, for chaining
 */
PublicKeyInput.prototype.clearSignatures = function ()
{
    this.setScript(Script.empty());
    return this;
};

/**
 * Query whether the input is signed
 * @return {boolean}
 */
PublicKeyInput.prototype.isFullySigned = function ()
{
    return this.script.isPublicKeyIn();
};

PublicKeyInput.SCRIPT_MAX_SIZE = 73; // sigsize (1 + 72)

PublicKeyInput.prototype._estimateSize = function ()
{
    return PublicKeyInput.SCRIPT_MAX_SIZE;
};

magnachain.PublicKeyInput = PublicKeyInput;
// transaction/input/publickey end----------------------------------------------------------------------

// transaction/input/publickeyhash begin----------------------------------------------------------------------
// var inherits = require('inherits');

// var $ = require('../../util/preconditions');
// var BufferUtil = require('../../util/buffer');

// var Hash = require('../../crypto/hash');
// var Input = require('./input');
// var Output = require('../output');
// var Sighash = require('../sighash');
// var Script = require('../../script');
// var Signature = require('../../crypto/signature');
// var TransactionSignature = require('../signature');

/**
 * Represents a special kind of input of PayToPublicKeyHash kind.
 * @constructor
 */
function PublicKeyHashInput()
{
    Input.apply(this, arguments);
}
inherits(PublicKeyHashInput, Input);

/* jshint maxparams: 5 */
/**
 * @param {Transaction} transaction - the transaction to be signed
 * @param {PrivateKey} privateKey - the private key with which to sign the transaction
 * @param {number} index - the index of the input in the transaction input vector
 * @param {number=} sigtype - the type of signature, defaults to Signature.SIGHASH_ALL
 * @param {Buffer=} hashData - the precalculated hash of the public key associated with the privateKey provided
 * @return {Array} of objects that can be
 */
PublicKeyHashInput.prototype.getSignatures = function (transaction, privateKey, index, sigtype, hashData)
{
    $.checkState(this.output instanceof Output);
    hashData = hashData || Hash.sha256ripemd160(privateKey.publicKey.toBuffer());
    sigtype = sigtype || Signature.SIGHASH_ALL;

    if (BufferUtil.equals(hashData, this.output.script.getPublicKeyHash()))
    {
        return [new TransactionSignature({
            publicKey: privateKey.publicKey,
            prevTxId: this.prevTxId,
            outputIndex: this.outputIndex,
            inputIndex: index,
            signature: Sighash.sign(transaction, privateKey, sigtype, index, this.output.script),
            sigtype: sigtype
        })];
    }
    return [];
};
/* jshint maxparams: 3 */

/**
 * Add the provided signature
 *
 * @param {Object} signature
 * @param {PublicKey} signature.publicKey
 * @param {Signature} signature.signature
 * @param {number=} signature.sigtype
 * @return {PublicKeyHashInput} this, for chaining
 */
PublicKeyHashInput.prototype.addSignature = function (transaction, signature)
{
    $.checkState(this.isValidSignature(transaction, signature), 'Signature is invalid');
    this.setScript(Script.buildPublicKeyHashIn(
        signature.publicKey,
        signature.signature.toDER(),
        signature.sigtype
    ));
    return this;
};

/**
 * Clear the input's signature
 * @return {PublicKeyHashInput} this, for chaining
 */
PublicKeyHashInput.prototype.clearSignatures = function ()
{
    this.setScript(Script.empty());
    return this;
};

/**
 * Query whether the input is signed
 * @return {boolean}
 */
PublicKeyHashInput.prototype.isFullySigned = function ()
{
    return this.script.isPublicKeyHashIn();
};

PublicKeyHashInput.SCRIPT_MAX_SIZE = 73 + 34; // sigsize (1 + 72) + pubkey (1 + 33)

PublicKeyHashInput.prototype._estimateSize = function ()
{
    return PublicKeyHashInput.SCRIPT_MAX_SIZE;
};

magnachain.PublicKeyHashInput = PublicKeyHashInput;
// transaction/input/publickeyhash end----------------------------------------------------------------------

function ContactInput()
{
    Input.apply(this, arguments);
}
inherits(ContactInput, Input);

ContactInput.prototype.getSignatures = function (transaction, privateKey, index, sigtype, hashData)
{
    return [];
};
ContactInput.prototype.addSignature = function (transaction, signature)
{
    return this;
};
ContactInput.prototype.clearSignatures = function ()
{
    this.setScript(Script.empty());
    return this;
};
ContactInput.prototype.isFullySigned = function ()
{
    return true;
};
ContactInput.SCRIPT_MAX_SIZE = 0;
ContactInput.prototype._estimateSize = function ()
{
    return ContactInput.SCRIPT_MAX_SIZE;
};
magnachain.ContactInput = ContactInput;


//------------------------------------------
//ContractData


// transaction/transaction begin----------------------------------------------------------------------
// var _ = require('lodash');
// var $ = require('../util/preconditions');
// var buffer = require('buffer');
//var compare = Buffer.compare || require('buffer-compare');
var compare = Buffer.compare;

// var errors = require('../errors');
// var BufferUtil = require('../util/buffer');
// var JSUtil = require('../util/js');
// var BufferReader = require('../encoding/bufferreader');
// var BufferWriter = require('../encoding/bufferwriter');
// var Hash = require('../crypto/hash');
// var Signature = require('../crypto/signature');
// var Sighash = require('./sighash');

// var Address = require('../address');
// var UnspentOutput = require('./unspentoutput');
// var Input = require('./input');
// var PublicKeyHashInput = Input.PublicKeyHash;
// var PublicKeyInput = Input.PublicKey;
// var MultiSigScriptHashInput = Input.MultiSigScriptHash;
// var MultiSigInput = Input.MultiSig;
// var Output = require('./output');
// var Script = require('../script');
// var PrivateKey = require('../privatekey');
// var BN = require('../crypto/bn');

/**
 * Represents a transaction, a set of inputs and outputs to change ownership of tokens
 *
 * @param {*} serialized
 * @constructor
 */
function Transaction(serialized)
{
    if (!(this instanceof Transaction))
    {
        return new Transaction(serialized);
    }
    this.inputs = [];
    this.outputs = [];
    this._inputAmount = undefined;
    this._outputAmount = undefined;

    if (serialized)
    {
        if (serialized instanceof Transaction)
        {
            return Transaction.shallowCopy(serialized);
        } else if (JSUtil.isHexa(serialized))
        {
            this.fromString(serialized);
        } else if (BufferUtil.isBuffer(serialized))
        {
            this.fromBuffer(serialized);
        } else if (_.isObject(serialized))
        {
            this.fromObject(serialized);
        } else
        {
            throw new errors.InvalidArgument('Must provide an object or string to deserialize a transaction');
        }
    } else
    {
        this._newTransaction();
    }
}

// CURRENT_VERSION = 2,// 普通交易
// PUBLISH_CONTRACT_VERSION = 3, //创建智能合约
// CALL_CONTRACT_VERSION = 4,//调用智能合约
// CREATE_BRANCH_VERSION = 5,// 创建支链 
// TRANS_BRANCH_VERSION_S1 = 6,// 跨链交易的发起链方 
// TRANS_BRANCH_VERSION_S2 = 7,// 跨链交易的接收链方 
// MAX_STANDARD_VERSION = 8

//var CURRENT_VERSION = 1;
var CURRENT_VERSION = 2;
var PUBLISH_CONTRACT_VERSION = 3;
var CALL_CONTRACT_VERSION = 4;
var CREATE_BRANCH_VERSION = 5;
var TRANS_BRANCH_VERSION_S1 = 6;
var TRANS_BRANCH_VERSION_S2 = 7;
var MAX_STANDARD_VERSION = 8;

var DEFAULT_NLOCKTIME = 0;
var MAX_BLOCK_SIZE = 1000000;

// Minimum amount for an output for it not to be considered a dust output
Transaction.DUST_AMOUNT = 546;

// Margin of error to allow fees in the vecinity of the expected value but doesn't allow a big difference
Transaction.FEE_SECURITY_MARGIN = 150;

// max amount of satoshis in circulation
Transaction.MAX_MONEY = 21000000 * 1e8;

// nlocktime limit to be considered block height rather than a timestamp
Transaction.NLOCKTIME_BLOCKHEIGHT_LIMIT = 5e8;

// Max value for an unsigned 32 bit value
Transaction.NLOCKTIME_MAX_VALUE = 4294967295;

// Value used for fee estimation (satoshis per kilobyte)
Transaction.FEE_PER_KB = 100000;

// Safe upper bound for change address script size in bytes
Transaction.CHANGE_OUTPUT_MAX_SIZE = 20 + 4 + 34 + 4;
Transaction.MAXIMUM_EXTRA_SIZE = 4 + 9 + 9 + 4;

/* Constructors and Serialization */

/**
 * Create a 'shallow' copy of the transaction, by serializing and deserializing
 * it dropping any additional information that inputs and outputs may have hold
 *
 * @param {Transaction} transaction
 * @return {Transaction}
 */
Transaction.shallowCopy = function (transaction)
{
    var copy = new Transaction(transaction.toBuffer());
    return copy;
};

var hashProperty = {
    configurable: false,
    enumerable: true,
    get: function ()
    {
        return new BufferReader(this._getHash()).readReverse().toString('hex');
    }
};
Object.defineProperty(Transaction.prototype, 'hash', hashProperty);
Object.defineProperty(Transaction.prototype, 'id', hashProperty);

var ioProperty = {
    configurable: false,
    enumerable: true,
    get: function ()
    {
        return this._getInputAmount();
    }
};
Object.defineProperty(Transaction.prototype, 'inputAmount', ioProperty);
ioProperty.get = function ()
{
    return this._getOutputAmount();
};
Object.defineProperty(Transaction.prototype, 'outputAmount', ioProperty);

/**
 * Retrieve the little endian hash of the transaction (used for serialization)
 * @return {Buffer}
 */
Transaction.prototype._getHash = function ()
{
    return Hash.sha256sha256(this.toBuffer());
};

/**
 * Retrieve a hexa string that can be used with bitcoind's CLI interface
 * (decoderawtransaction, sendrawtransaction)
 *
 * @param {Object|boolean=} unsafe if true, skip all tests. if it's an object,
 *   it's expected to contain a set of flags to skip certain tests:
 * * `disableAll`: disable all checks
 * * `disableSmallFees`: disable checking for fees that are too small
 * * `disableLargeFees`: disable checking for fees that are too large
 * * `disableIsFullySigned`: disable checking if all inputs are fully signed
 * * `disableDustOutputs`: disable checking if there are no outputs that are dust amounts
 * * `disableMoreOutputThanInput`: disable checking if the transaction spends more bitcoins than the sum of the input amounts
 * @return {string}
 */
Transaction.prototype.serialize = function (unsafe)
{
    if (true === unsafe || unsafe && unsafe.disableAll)
    {
        return this.uncheckedSerialize();
    } else
    {
        return this.checkedSerialize(unsafe);
    }
};

Transaction.prototype.uncheckedSerialize = Transaction.prototype.toString = function ()
{
    return this.toBuffer().toString('hex');
};

/**
 * Retrieve a hexa string that can be used with bitcoind's CLI interface
 * (decoderawtransaction, sendrawtransaction)
 *
 * @param {Object} opts allows to skip certain tests. {@see Transaction#serialize}
 * @return {string}
 */
Transaction.prototype.checkedSerialize = function (opts)
{
    var serializationError = this.getSerializationError(opts);
    if (serializationError)
    {
        serializationError.message += ' - For more information please see: ' +
            'https://bitcore.io/api/lib/transaction#serialization-checks';
        throw serializationError;
    }
    return this.uncheckedSerialize();
};

Transaction.prototype.invalidSatoshis = function ()
{
    var invalid = false;
    for (var i = 0; i < this.outputs.length; i++)
    {
        if (this.outputs[i].invalidSatoshis())
        {
            invalid = true;
        }
    }
    return invalid;
};

/**
 * Retrieve a possible error that could appear when trying to serialize and
 * broadcast this transaction.
 *
 * @param {Object} opts allows to skip certain tests. {@see Transaction#serialize}
 * @return {bitcore.Error}
 */
Transaction.prototype.getSerializationError = function (opts)
{
    opts = opts || {};

    if (this.invalidSatoshis())
    {
        return new errors.Transaction.InvalidSatoshis();
    }

    var unspent = this._getUnspentValue();
    var unspentError;
    if (unspent < 0)
    {
        if (!opts.disableMoreOutputThanInput)
        {
            unspentError = new errors.Transaction.InvalidOutputAmountSum();
        }
    } else
    {
        unspentError = this._hasFeeError(opts, unspent);
    }

    return unspentError ||
        this._hasDustOutputs(opts) ||
        this._isMissingSignatures(opts);
};

Transaction.prototype._hasFeeError = function (opts, unspent)
{

    if (!_.isUndefined(this._fee) && this._fee !== unspent)
    {
        return new errors.Transaction.FeeError.Different(
            'Unspent value is ' + unspent + ' but specified fee is ' + this._fee
        );
    }

    if (!opts.disableLargeFees)
    {
        var maximumFee = Math.floor(Transaction.FEE_SECURITY_MARGIN * this._estimateFee());
        if (unspent > maximumFee)
        {
            if (this._missingChange())
            {
                return new errors.Transaction.ChangeAddressMissing(
                    'Fee is too large and no change address was provided'
                );
            }
            return new errors.Transaction.FeeError.TooLarge(
                'expected less than ' + maximumFee + ' but got ' + unspent
            );
        }
    }

    if (!opts.disableSmallFees)
    {
        var minimumFee = Math.ceil(this._estimateFee() / Transaction.FEE_SECURITY_MARGIN);
        if (unspent < minimumFee)
        {
            return new errors.Transaction.FeeError.TooSmall(
                'expected more than ' + minimumFee + ' but got ' + unspent
            );
        }
    }
};

Transaction.prototype._missingChange = function ()
{
    return !this._changeScript;
};

Transaction.prototype._hasDustOutputs = function (opts)
{
    if (opts.disableDustOutputs)
    {
        return;
    }
    var index, output;
    for (index in this.outputs)
    {
        output = this.outputs[index];
        if (output.satoshis < Transaction.DUST_AMOUNT && !output.script.isDataOut())
        {
            return new errors.Transaction.DustOutputs();
        }
    }
};

Transaction.prototype._isMissingSignatures = function (opts)
{
    if (opts.disableIsFullySigned)
    {
        return;
    }
    if (!this.isFullySigned())
    {
        return new errors.Transaction.MissingSignatures();
    }
};

Transaction.prototype.inspect = function ()
{
    return '<Transaction: ' + this.uncheckedSerialize() + '>';
};

Transaction.prototype.toBuffer = function ()
{
    var writer = new BufferWriter();
    return this.toBufferWriter(writer).toBuffer();
};

Transaction.prototype.toBufferForSign = function ()
{
    var writer = new BufferWriter();
    return this.toBufferWriterForSign(writer).toBuffer();
}

Transaction.prototype.toBufferWriter = function (writer)
{
    writer.writeInt32LE(this.version);
    writer.writeVarintNum(this.inputs.length);
    _.each(this.inputs, function (input)
    {
        input.toBufferWriter(writer);
    });
    writer.writeVarintNum(this.outputs.length);
    _.each(this.outputs, function (output)
    {
        output.toBufferWriter(writer);
    });
    writer.writeUInt32LE(this.nLockTime);

    // 此处需要判断是在序列化还是在签名
    this.WriteExtra(writer, false);

    return writer;
};

Transaction.prototype.toBufferWriterForSign = function (writer)
{
    writer.writeVarintNum(this.inputs.length);
    _.each(this.inputs, function (input)
    {
        input.toBufferWriter(writer);
    });
    writer.writeVarintNum(this.outputs.length);
    _.each(this.outputs, function (output)
    {
        output.toBufferWriter(writer);
    });
    writer.writeUInt32LE(this.nLockTime);

    writer.writeInt32LE(this.version);

    // 此处需要判断是在序列化还是在签名
    this.WriteExtra(writer, true);

    return writer;
};

Transaction.prototype.WriteExtra = function (bw, sign)
{
    if (this.version == PUBLISH_CONTRACT_VERSION || this.version == CALL_CONTRACT_VERSION)
    {
        bw.write(this.contractdata.address);
        this.contractdata.sender.WriteForCl(bw);

        bw.writeVarintNum(this.contractdata.codeOrFunc.length);
        bw.write(this.contractdata.codeOrFunc);

        bw.writeVarintNum(this.contractdata.args.length);
        bw.write(this.contractdata.args);

        bw.writeUInt64LEBN(this.contractdata.amountOut);
        if (!sign)
        {
            var scriptBuf;
            if (this.contractdata.signature == null)
            {
                scriptBuf = Script.empty().toBuffer();
            } else
            {
                scriptBuf = this.contractdata.signature.toBuffer();
            }
            bw.writeVarintNum(scriptBuf.length);
            bw.write(scriptBuf);
        }
    }
};

Transaction.prototype.fromBuffer = function (buffer)
{
    var reader = new BufferReader(buffer);
    return this.fromBufferReader(reader);
};

Transaction.prototype.fromBufferReader = function (reader)
{
    $.checkArgument(!reader.finished(), 'No transaction data received');
    var i, sizeTxIns, sizeTxOuts;

    this.version = reader.readInt32LE();
    sizeTxIns = reader.readVarintNum();
    for (i = 0; i < sizeTxIns; i++)
    {
        var input = Input.fromBufferReader(reader);
        this.inputs.push(input);
    }
    sizeTxOuts = reader.readVarintNum();
    for (i = 0; i < sizeTxOuts; i++)
    {
        this.outputs.push(Output.fromBufferReader(reader));
    }
    this.nLockTime = reader.readUInt32LE();

    this.ReadExtra(reader);

    return this;
};

Transaction.prototype.ReadExtra = function (br)
{
    if (this.version == PUBLISH_CONTRACT_VERSION || this.version == CALL_CONTRACT_VERSION)
    {
        this.contractdata = {} // TODO: make a class for contractdata???

        this.contractdata.address = br.read(20);//this is a KeyId type in C++, 
        this.contractdata.sender = PublicKey.ReadForCl(br);

        this.contractdata.codeOrFunc = br.readVarLengthBuffer();
        this.contractdata.args = br.readVarLengthBuffer();

        this.contractdata.amountOut = br.readUInt64LEBN();
        if (!sign)
        {
            var scriptBuffer = br.readVarLengthBuffer();
            this.contractdata.signature = Script.fromBuffer(scriptBuffer);
        }
    }
}

Transaction.prototype.toObject = Transaction.prototype.toJSON = function toObject()
{
    var inputs = [];
    this.inputs.forEach(function (input)
    {
        inputs.push(input.toObject());
    });
    var outputs = [];
    this.outputs.forEach(function (output)
    {
        outputs.push(output.toObject());
    });
    var obj = {
        hash: this.hash,
        version: this.version,
        inputs: inputs,
        outputs: outputs,
        nLockTime: this.nLockTime
    };
    if (this._changeScript)
    {
        obj.changeScript = this._changeScript.toString();
    }
    if (!_.isUndefined(this._changeIndex))
    {
        obj.changeIndex = this._changeIndex;
    }
    if (!_.isUndefined(this._fee))
    {
        obj.fee = this._fee;
    }
    return obj;
};

Transaction.prototype.fromObject = function fromObject(arg)
{
    /* jshint maxstatements: 20 */
    $.checkArgument(_.isObject(arg) || arg instanceof Transaction);
    var self = this;
    var transaction;
    if (arg instanceof Transaction)
    {
        transaction = transaction.toObject();
    } else
    {
        transaction = arg;
    }
    _.each(transaction.inputs, function (input)
    {
        if (!input.output || !input.output.script)
        {
            self.uncheckedAddInput(new Input(input));
            return;
        }
        var script = new Script(input.output.script);
        var txin;
        if (script.isPublicKeyHashOut())
        {
            txin = new Input.PublicKeyHash(input);
        } else if (script.isScriptHashOut() && input.publicKeys && input.threshold)
        {
            txin = new Input.MultiSigScriptHash(
                input, input.publicKeys, input.threshold, input.signatures
            );
        } else if (script.isPublicKeyOut())
        {
            txin = new Input.PublicKey(input);
        } else
        {
            throw new errors.Transaction.Input.UnsupportedScript(input.output.script);
        }
        self.addInput(txin);
    });
    _.each(transaction.outputs, function (output)
    {
        self.addOutput(new Output(output));
    });
    if (transaction.changeIndex)
    {
        this._changeIndex = transaction.changeIndex;
    }
    if (transaction.changeScript)
    {
        this._changeScript = new Script(transaction.changeScript);
    }
    if (transaction.fee)
    {
        this._fee = transaction.fee;
    }
    this.nLockTime = transaction.nLockTime;
    this.version = transaction.version;
    this._checkConsistency(arg);
    return this;
};

Transaction.prototype._checkConsistency = function (arg)
{
    if (!_.isUndefined(this._changeIndex))
    {
        $.checkState(this._changeScript, 'Change script is expected.');
        $.checkState(this.outputs[this._changeIndex], 'Change index points to undefined output.');
        $.checkState(this.outputs[this._changeIndex].script.toString() ===
            this._changeScript.toString(), 'Change output has an unexpected script.');
    }
    if (arg && arg.hash)
    {
        $.checkState(arg.hash === this.hash, 'Hash in object does not match transaction hash.');
    }
};

/**
 * Sets nLockTime so that transaction is not valid until the desired date(a
 * timestamp in seconds since UNIX epoch is also accepted)
 *
 * @param {Date | Number} time
 * @return {Transaction} this
 */
Transaction.prototype.lockUntilDate = function (time)
{
    $.checkArgument(time);
    if (_.isNumber(time) && time < Transaction.NLOCKTIME_BLOCKHEIGHT_LIMIT)
    {
        throw new errors.Transaction.LockTimeTooEarly();
    }
    if (_.isDate(time))
    {
        time = time.getTime() / 1000;
    }

    for (var i = 0; i < this.inputs.length; i++)
    {
        if (this.inputs[i].sequenceNumber === Input.DEFAULT_SEQNUMBER)
        {
            this.inputs[i].sequenceNumber = Input.DEFAULT_LOCKTIME_SEQNUMBER;
        }
    }

    this.nLockTime = time;
    return this;
};

/**
 * Sets nLockTime so that transaction is not valid until the desired block
 * height.
 *
 * @param {Number} height
 * @return {Transaction} this
 */
Transaction.prototype.lockUntilBlockHeight = function (height)
{
    $.checkArgument(_.isNumber(height));
    if (height >= Transaction.NLOCKTIME_BLOCKHEIGHT_LIMIT)
    {
        throw new errors.Transaction.BlockHeightTooHigh();
    }
    if (height < 0)
    {
        throw new errors.Transaction.NLockTimeOutOfRange();
    }

    for (var i = 0; i < this.inputs.length; i++)
    {
        if (this.inputs[i].sequenceNumber === Input.DEFAULT_SEQNUMBER)
        {
            this.inputs[i].sequenceNumber = Input.DEFAULT_LOCKTIME_SEQNUMBER;
        }
    }


    this.nLockTime = height;
    return this;
};

/**
 *  Returns a semantic version of the transaction's nLockTime.
 *  @return {Number|Date}
 *  If nLockTime is 0, it returns null,
 *  if it is < 500000000, it returns a block height (number)
 *  else it returns a Date object.
 */
Transaction.prototype.getLockTime = function ()
{
    if (!this.nLockTime)
    {
        return null;
    }
    if (this.nLockTime < Transaction.NLOCKTIME_BLOCKHEIGHT_LIMIT)
    {
        return this.nLockTime;
    }
    return new Date(1000 * this.nLockTime);
};

Transaction.prototype.fromString = function (string)
{
    this.fromBuffer(new Buffer(string, 'hex'));
};

Transaction.prototype._newTransaction = function ()
{
    this.version = CURRENT_VERSION;
    this.nLockTime = DEFAULT_NLOCKTIME;
};

/* Transaction creation interface */

/**
 * @typedef {Object} Transaction~fromObject
 * @property {string} prevTxId
 * @property {number} outputIndex
 * @property {(Buffer|string|Script)} script
 * @property {number} satoshis
 */

/**
 * Add an input to this transaction. This is a high level interface
 * to add an input, for more control, use @{link Transaction#addInput}.
 *
 * Can receive, as output information, the output of bitcoind's `listunspent` command,
 * and a slightly fancier format recognized by bitcore:
 *
 * ```
 * {
 *  address: 'mszYqVnqKoQx4jcTdJXxwKAissE3Jbrrc1',
 *  txId: 'a477af6b2667c29670467e4e0728b685ee07b240235771862318e29ddbe58458',
 *  outputIndex: 0,
 *  script: Script.empty(),
 *  satoshis: 1020000
 * }
 * ```
 * Where `address` can be either a string or a bitcore Address object. The
 * same is true for `script`, which can be a string or a bitcore Script.
 *
 * Beware that this resets all the signatures for inputs (in further versions,
 * SIGHASH_SINGLE or SIGHASH_NONE signatures will not be reset).
 *
 * @example
 * ```javascript
 * var transaction = new Transaction();
 *
 * // From a pay to public key hash output from bitcoind's listunspent
 * transaction.from({'txid': '0000...', vout: 0, amount: 0.1, scriptPubKey: 'OP_DUP ...'});
 *
 * // From a pay to public key hash output
 * transaction.from({'txId': '0000...', outputIndex: 0, satoshis: 1000, script: 'OP_DUP ...'});
 *
 * // From a multisig P2SH output
 * transaction.from({'txId': '0000...', inputIndex: 0, satoshis: 1000, script: '... OP_HASH'},
 *                  ['03000...', '02000...'], 2);
 * ```
 *
 * @param {(Array.<Transaction~fromObject>|Transaction~fromObject)} utxo
 * @param {Array=} pubkeys
 * @param {number=} threshold
 */
Transaction.prototype.from = function (utxo, pubkeys, threshold)
{
    if (_.isArray(utxo))
    {
        var self = this;
        _.each(utxo, function (utxo)
        {
            self.from(utxo, pubkeys, threshold);
        });
        return this;
    }
    var exists = _.some(this.inputs, function (input)
    {
        // TODO: Maybe prevTxId should be a string? Or defined as read only property?
        return input.prevTxId.toString('hex') === utxo.txId && input.outputIndex === utxo.outputIndex;
    });
    if (exists)
    {
        return this;
    }
    if (pubkeys && threshold)
    {
        this._fromMultisigUtxo(utxo, pubkeys, threshold);
    } else
    {
        this._fromNonP2SH(utxo);
    }
    return this;
};

Transaction.prototype.setOutputsFromCoins = function (coins)
{
    var outs = new Array();
    for (var i = 0; i < coins.length; i++)
    {
        var coin = coins[i];
        outs.push(new Output({
            "satoshis": Unit.fromBTC(coin.value)._value,
            "script": coin.script,
        }));
    }
    this.setOutputs(outs);
}

Transaction.prototype.setOutputs = function (outputs)
{
    for (var i = 0; i < this.inputs.length; i++)
    {
        var output = outputs[i];// TODO: match output by prevTxId outputIndex
        this.inputs[i].setOutput(output);
        //
        // e.g: CoinbaseInput, PublicKeyHashInput, MultiSigScriptHashInput, ContactInput etc.
        var scriptbuf = output.script.toBuffer()
        // if (scriptbuf.length == 25 &&
        //     scriptbuf[0] == Opcode.map.OP_DUP &&
        //     scriptbuf[1] == Opcode.map.OP_HASH160 &&
        //     scriptbuf[2] == 0x14 &&
        //     scriptbuf[24] == Opcode.map.OP_CHECKSIG)
        if (output.script.isPublicKeyHashOut())
        {
            //default is PublicKeyHashInput
            //console.log("default is PublicKeyHashInput");
        }
        if (output.script.isPublicKeyOut())
        {
            this.inputs[i] = new PublicKeyInput(this.inputs[i]);
        }
        //if (scriptbuf.length == 22 &&
        //    (scriptbuf[0] == Opcode.map.OP_CONTRACT || scriptbuf[0] == Opcode.map.OP_CONTRACT_CHANGE))
        if (output.script.isContractOut())
        {
            this.inputs[i] = new ContactInput(this.inputs[i]);
        }
    }
}

Transaction.prototype._fromNonP2SH = function (utxo)
{
    var clazz;
    utxo = new UnspentOutput(utxo);
    if (utxo.script.isPublicKeyHashOut())
    {
        clazz = PublicKeyHashInput;
    } else if (utxo.script.isPublicKeyOut())
    {
        clazz = PublicKeyInput;
    } else
    {
        clazz = Input;
    }
    this.addInput(new clazz({
        output: new Output({
            script: utxo.script,
            satoshis: utxo.satoshis
        }),
        prevTxId: utxo.txId,
        outputIndex: utxo.outputIndex,
        script: Script.empty()
    }));
};

Transaction.prototype._fromMultisigUtxo = function (utxo, pubkeys, threshold)
{
    $.checkArgument(threshold <= pubkeys.length,
        'Number of required signatures must be greater than the number of public keys');
    var clazz;
    utxo = new UnspentOutput(utxo);
    if (utxo.script.isMultisigOut())
    {
        clazz = MultiSigInput;
    } else if (utxo.script.isScriptHashOut())
    {
        clazz = MultiSigScriptHashInput;
    } else
    {
        throw new Error("@TODO");
    }
    this.addInput(new clazz({
        output: new Output({
            script: utxo.script,
            satoshis: utxo.satoshis
        }),
        prevTxId: utxo.txId,
        outputIndex: utxo.outputIndex,
        script: Script.empty()
    }, pubkeys, threshold));
};

/**
 * Add an input to this transaction. The input must be an instance of the `Input` class.
 * It should have information about the Output that it's spending, but if it's not already
 * set, two additional parameters, `outputScript` and `satoshis` can be provided.
 *
 * @param {Input} input
 * @param {String|Script} outputScript
 * @param {number} satoshis
 * @return Transaction this, for chaining
 */
Transaction.prototype.addInput = function (input, outputScript, satoshis)
{
    $.checkArgumentType(input, Input, 'input');
    if (!input.output && (_.isUndefined(outputScript) || _.isUndefined(satoshis)))
    {
        throw new errors.Transaction.NeedMoreInfo('Need information about the UTXO script and satoshis');
    }
    if (!input.output && outputScript && !_.isUndefined(satoshis))
    {
        outputScript = outputScript instanceof Script ? outputScript : new Script(outputScript);
        $.checkArgumentType(satoshis, 'number', 'satoshis');
        input.output = new Output({
            script: outputScript,
            satoshis: satoshis
        });
    }
    return this.uncheckedAddInput(input);
};

/**
 * Add an input to this transaction, without checking that the input has information about
 * the output that it's spending.
 *
 * @param {Input} input
 * @return Transaction this, for chaining
 */
Transaction.prototype.uncheckedAddInput = function (input)
{
    $.checkArgumentType(input, Input, 'input');
    this.inputs.push(input);
    this._inputAmount = undefined;
    this._updateChangeOutput();
    return this;
};

/**
 * Returns true if the transaction has enough info on all inputs to be correctly validated
 *
 * @return {boolean}
 */
Transaction.prototype.hasAllUtxoInfo = function ()
{
    return _.every(this.inputs.map(function (input)
    {
        return !!input.output;
    }));
};

/**
 * Manually set the fee for this transaction. Beware that this resets all the signatures
 * for inputs (in further versions, SIGHASH_SINGLE or SIGHASH_NONE signatures will not
 * be reset).
 *
 * @param {number} amount satoshis to be sent
 * @return {Transaction} this, for chaining
 */
Transaction.prototype.fee = function (amount)
{
    $.checkArgument(_.isNumber(amount), 'amount must be a number');
    this._fee = amount;
    this._updateChangeOutput();
    return this;
};

/**
 * Manually set the fee per KB for this transaction. Beware that this resets all the signatures
 * for inputs (in further versions, SIGHASH_SINGLE or SIGHASH_NONE signatures will not
 * be reset).
 *
 * @param {number} amount satoshis per KB to be sent
 * @return {Transaction} this, for chaining
 */
Transaction.prototype.feePerKb = function (amount)
{
    $.checkArgument(_.isNumber(amount), 'amount must be a number');
    this._feePerKb = amount;
    this._updateChangeOutput();
    return this;
};

/* Output management */

/**
 * Set the change address for this transaction
 *
 * Beware that this resets all the signatures for inputs (in further versions,
 * SIGHASH_SINGLE or SIGHASH_NONE signatures will not be reset).
 *
 * @param {Address} address An address for change to be sent to.
 * @return {Transaction} this, for chaining
 */
Transaction.prototype.change = function (address)
{
    $.checkArgument(address, 'address is required');
    this._changeScript = Script.fromAddress(address);
    this._updateChangeOutput();
    return this;
};


/**
 * @return {Output} change output, if it exists
 */
Transaction.prototype.getChangeOutput = function ()
{
    if (!_.isUndefined(this._changeIndex))
    {
        return this.outputs[this._changeIndex];
    }
    return null;
};

/**
 * @typedef {Object} Transaction~toObject
 * @property {(string|Address)} address
 * @property {number} satoshis
 */

/**
 * Add an output to the transaction.
 *
 * Beware that this resets all the signatures for inputs (in further versions,
 * SIGHASH_SINGLE or SIGHASH_NONE signatures will not be reset).
 *
 * @param {(string|Address|Array.<Transaction~toObject>)} address
 * @param {number} amount in satoshis
 * @return {Transaction} this, for chaining
 */
Transaction.prototype.to = function (address, amount)
{
    if (_.isArray(address))
    {
        var self = this;
        _.each(address, function (to)
        {
            self.to(to.address, to.satoshis);
        });
        return this;
    }

    $.checkArgument(
        JSUtil.isNaturalNumber(amount),
        'Amount is expected to be a positive integer'
    );
    this.addOutput(new Output({
        script: Script(new Address(address)),
        satoshis: amount
    }));
    return this;
};

/**
 * Add an OP_RETURN output to the transaction.
 *
 * Beware that this resets all the signatures for inputs (in further versions,
 * SIGHASH_SINGLE or SIGHASH_NONE signatures will not be reset).
 *
 * @param {Buffer|string} value the data to be stored in the OP_RETURN output.
 *    In case of a string, the UTF-8 representation will be stored
 * @return {Transaction} this, for chaining
 */
Transaction.prototype.addData = function (value)
{
    this.addOutput(new Output({
        script: Script.buildDataOut(value),
        satoshis: 0
    }));
    return this;
};


/**
 * Add an output to the transaction.
 *
 * @param {Output} output the output to add.
 * @return {Transaction} this, for chaining
 */
Transaction.prototype.addOutput = function (output)
{
    $.checkArgumentType(output, Output, 'output');
    this._addOutput(output);
    this._updateChangeOutput();
    return this;
};


/**
 * Remove all outputs from the transaction.
 *
 * @return {Transaction} this, for chaining
 */
Transaction.prototype.clearOutputs = function ()
{
    this.outputs = [];
    this._clearSignatures();
    this._outputAmount = undefined;
    this._changeIndex = undefined;
    this._updateChangeOutput();
    return this;
};


Transaction.prototype._addOutput = function (output)
{
    this.outputs.push(output);
    this._outputAmount = undefined;
};


/**
 * Calculates or gets the total output amount in satoshis
 *
 * @return {Number} the transaction total output amount
 */
Transaction.prototype._getOutputAmount = function ()
{
    if (_.isUndefined(this._outputAmount))
    {
        var self = this;
        this._outputAmount = 0;
        _.each(this.outputs, function (output)
        {
            self._outputAmount += output.satoshis;
        });
    }
    return this._outputAmount;
};


/**
 * Calculates or gets the total input amount in satoshis
 *
 * @return {Number} the transaction total input amount
 */
Transaction.prototype._getInputAmount = function ()
{
    if (_.isUndefined(this._inputAmount))
    {
        var self = this;
        this._inputAmount = 0;
        _.each(this.inputs, function (input)
        {
            if (_.isUndefined(input.output))
            {
                throw new errors.Transaction.Input.MissingPreviousOutput();
            }
            self._inputAmount += input.output.satoshis;
        });
    }
    return this._inputAmount;
};

Transaction.prototype._updateChangeOutput = function ()
{
    if (!this._changeScript)
    {
        return;
    }
    this._clearSignatures();
    if (!_.isUndefined(this._changeIndex))
    {
        this._removeOutput(this._changeIndex);
    }
    var available = this._getUnspentValue();
    var fee = this.getFee();
    var changeAmount = available - fee;
    if (changeAmount > 0)
    {
        this._changeIndex = this.outputs.length;
        this._addOutput(new Output({
            script: this._changeScript,
            satoshis: changeAmount
        }));
    } else
    {
        this._changeIndex = undefined;
    }
};
/**
 * Calculates the fee of the transaction.
 *
 * If there's a fixed fee set, return that.
 *
 * If there is no change output set, the fee is the
 * total value of the outputs minus inputs. Note that
 * a serialized transaction only specifies the value
 * of its outputs. (The value of inputs are recorded
 * in the previous transaction outputs being spent.)
 * This method therefore raises a "MissingPreviousOutput"
 * error when called on a serialized transaction.
 *
 * If there's no fee set and no change address,
 * estimate the fee based on size.
 *
 * @return {Number} fee of this transaction in satoshis
 */
Transaction.prototype.getFee = function ()
{
    if (this.isCoinbase())
    {
        return 0;
    }
    if (!_.isUndefined(this._fee))
    {
        return this._fee;
    }
    // if no change output is set, fees should equal all the unspent amount
    if (!this._changeScript)
    {
        return this._getUnspentValue();
    }
    return this._estimateFee();
};

/**
 * Estimates fee from serialized transaction size in bytes.
 */
Transaction.prototype._estimateFee = function ()
{
    var estimatedSize = this._estimateSize();
    var available = this._getUnspentValue();
    return Transaction._estimateFee(estimatedSize, available, this._feePerKb);
};

Transaction.prototype._getUnspentValue = function ()
{
    return this._getInputAmount() - this._getOutputAmount();
};

Transaction.prototype._clearSignatures = function ()
{
    _.each(this.inputs, function (input)
    {
        input.clearSignatures();
    });
};

Transaction._estimateFee = function (size, amountAvailable, feePerKb)
{
    var fee = Math.ceil(size / 1000) * (feePerKb || Transaction.FEE_PER_KB);
    if (amountAvailable > fee)
    {
        size += Transaction.CHANGE_OUTPUT_MAX_SIZE;
    }
    return Math.ceil(size / 1000) * (feePerKb || Transaction.FEE_PER_KB);
};

Transaction.prototype._estimateSize = function ()
{
    var result = Transaction.MAXIMUM_EXTRA_SIZE;
    _.each(this.inputs, function (input)
    {
        result += input._estimateSize();
    });
    _.each(this.outputs, function (output)
    {
        result += output.script.toBuffer().length + 9;
    });
    return result;
};

Transaction.prototype._removeOutput = function (index)
{
    var output = this.outputs[index];
    this.outputs = _.without(this.outputs, output);
    this._outputAmount = undefined;
};

Transaction.prototype.removeOutput = function (index)
{
    this._removeOutput(index);
    this._updateChangeOutput();
};

/**
 * Sort a transaction's inputs and outputs according to BIP69
 *
 * @see {https://github.com/bitcoin/bips/blob/master/bip-0069.mediawiki}
 * @return {Transaction} this
 */
Transaction.prototype.sort = function ()
{
    this.sortInputs(function (inputs)
    {
        var copy = Array.prototype.concat.apply([], inputs);
        copy.sort(function (first, second)
        {
            return compare(first.prevTxId, second.prevTxId)
                || first.outputIndex - second.outputIndex;
        });
        return copy;
    });
    this.sortOutputs(function (outputs)
    {
        var copy = Array.prototype.concat.apply([], outputs);
        copy.sort(function (first, second)
        {
            return first.satoshis - second.satoshis
                || compare(first.script.toBuffer(), second.script.toBuffer());
        });
        return copy;
    });
    return this;
};

/**
 * Randomize this transaction's outputs ordering. The shuffling algorithm is a
 * version of the Fisher-Yates shuffle, provided by lodash's _.shuffle().
 *
 * @return {Transaction} this
 */
Transaction.prototype.shuffleOutputs = function ()
{
    return this.sortOutputs(_.shuffle);
};

/**
 * Sort this transaction's outputs, according to a given sorting function that
 * takes an array as argument and returns a new array, with the same elements
 * but with a different order. The argument function MUST NOT modify the order
 * of the original array
 *
 * @param {Function} sortingFunction
 * @return {Transaction} this
 */
Transaction.prototype.sortOutputs = function (sortingFunction)
{
    var outs = sortingFunction(this.outputs);
    return this._newOutputOrder(outs);
};

/**
 * Sort this transaction's inputs, according to a given sorting function that
 * takes an array as argument and returns a new array, with the same elements
 * but with a different order.
 *
 * @param {Function} sortingFunction
 * @return {Transaction} this
 */
Transaction.prototype.sortInputs = function (sortingFunction)
{
    this.inputs = sortingFunction(this.inputs);
    this._clearSignatures();
    return this;
};

Transaction.prototype._newOutputOrder = function (newOutputs)
{
    var isInvalidSorting = (this.outputs.length !== newOutputs.length ||
        _.difference(this.outputs, newOutputs).length !== 0);
    if (isInvalidSorting)
    {
        throw new errors.Transaction.InvalidSorting();
    }

    if (!_.isUndefined(this._changeIndex))
    {
        var changeOutput = this.outputs[this._changeIndex];
        this._changeIndex = _.findIndex(newOutputs, changeOutput);
    }

    this.outputs = newOutputs;
    return this;
};

Transaction.prototype.removeInput = function (txId, outputIndex)
{
    var index;
    if (!outputIndex && _.isNumber(txId))
    {
        index = txId;
    } else
    {
        index = _.findIndex(this.inputs, function (input)
        {
            return input.prevTxId.toString('hex') === txId && input.outputIndex === outputIndex;
        });
    }
    if (index < 0 || index >= this.inputs.length)
    {
        throw new errors.Transaction.InvalidIndex(index, this.inputs.length);
    }
    var input = this.inputs[index];
    this.inputs = _.without(this.inputs, input);
    this._inputAmount = undefined;
    this._updateChangeOutput();
};

//this will be write only
// public void ReadWriteForContractSign(BitcoinStream stream)
// {
//     stream.ReadWrite(ref nVersion);
//     stream.ReadWriteStruct(ref nLockTime);

//     //stream.ReadWrite<TxInList, TxIn>(ref vin);
//     foreach (var v in vin)
//     {
//         stream.ReadWrite(v);
//     }
//     //stream.ReadWrite<TxOutList, TxOut>(ref vout);
//     foreach (var v in vout)
//     {
//         stream.ReadWrite(v);
//     }

//     ReadWriteExtra(stream, true);
// }

Transaction.prototype.WriteForContractSign = function (stream)
{
    stream.writeUInt32LE(this.version);
    stream.writeUInt32LE(this.nLockTime);

    var i;
    var len = this.inputs.length;

    for (i = 0; i < len; i++)
    {
        this.inputs[i].toBufferWriter(stream);
    }

    len = this.outputs.length;
    for (i = 0; i < len; i++)
    {
        this.outputs[i].toBufferWriter(stream);
    }

    this.WriteExtra(stream, true);
}

Transaction.prototype.IsSmartContract = function ()
{
    return this.version == PUBLISH_CONTRACT_VERSION || this.version == CALL_CONTRACT_VERSION;
}

// isContractOut no need to sign
Transaction.prototype.hasAllInputSign = function ()
{
    return _.every(this.inputs.map(function (input)
    {
        return input.output.script.isContractOut() || (input._scriptBuffer && input._scriptBuffer.length > 0);
    }));
};

/* Signature handling */

/**
 * Sign the transaction using one or more private keys.
 *
 * It tries to sign each input, verifying that the signature will be valid
 * (matches a public key).
 *
 * @param {Array|String|PrivateKey} privateKey
 * @param {number} sigtype
 * @return {Transaction} this, for chaining
 */
Transaction.prototype.sign = function (privateKey, sigtype)
{
    $.checkState(this.hasAllUtxoInfo(), 'Not all utxo information is available to sign the transaction.');
    var self = this;
    if (_.isArray(privateKey))
    {
        _.each(privateKey, function (privateKey)
        {
            self.sign(privateKey, sigtype);
        });
        /*
        if (this.IsSmartContract())
        {
            $.checkState(this.hasAllInputSign(), 'Not all input has sign before sgin smcsender, maybe privkey miss');
            _.each(privateKey, function (privateKey)
            {
                self.signSmartContract(privateKey, sigtype);
            });
        }*/
        return this;
    }
    _.each(this.getSignatures(privateKey, sigtype), function (signature)
    {
        self.applySignature(signature);
    });

    // when finish all input sign, then sign smartcontract
    if (this.IsSmartContract() && this.hasAllInputSign())
    {
        self.signSmartContract(privateKey, sigtype);
    }
    return this;
};

Transaction.prototype.signSmartContract = function (privateKey, sigtype)
{
    if (this.IsSmartContract() && this.contractdata.sender != null)
    {
        var hashSign = Script.SignatureHashForContract(this);
        var privKey = new PrivateKey(privateKey);

        sigtype = sigtype || Signature.SIGHASH_ALL;
        if (privKey.publicKey.toString() === this.contractdata.sender.toString())
        {
            var sig = ECDSA.sign(hashSign, privKey, 'little').set({
                nhashtype: sigtype
            });

            //console.log("contract sig: " + sig);
            var script = new Script();
            script.add(BufferUtil.concat([
                sig.toBuffer(),
                BufferUtil.integerAsSingleByteBuffer(sigtype || Signature.SIGHASH_ALL)
            ]));
            this.contractdata.signature = script;//new Script(sig);
        }
    }
}

Transaction.prototype.getSignatures = function (privKey, sigtype)
{
    privKey = new PrivateKey(privKey);
    sigtype = sigtype || Signature.SIGHASH_ALL;
    var transaction = this;
    var results = [];
    var hashData = Hash.sha256ripemd160(privKey.publicKey.toBuffer());
    _.each(this.inputs, function forEachInput(input, index)
    {
        _.each(input.getSignatures(transaction, privKey, index, sigtype, hashData), function (signature)
        {
            results.push(signature);
        });
    });
    return results;
};

/**
 * Add a signature to the transaction
 *
 * @param {Object} signature
 * @param {number} signature.inputIndex
 * @param {number} signature.sigtype
 * @param {PublicKey} signature.publicKey
 * @param {Signature} signature.signature
 * @return {Transaction} this, for chaining
 */
Transaction.prototype.applySignature = function (signature)
{
    this.inputs[signature.inputIndex].addSignature(this, signature);
    return this;
};

Transaction.prototype.isFullySigned = function ()
{
    _.each(this.inputs, function (input)
    {
        if (input.isFullySigned === Input.prototype.isFullySigned)
        {
            throw new errors.Transaction.UnableToVerifySignature(
                'Unrecognized script kind, or not enough information to execute script.' +
                'This usually happens when creating a transaction from a serialized transaction'
            );
        }
    });
    return _.every(_.map(this.inputs, function (input)
    {
        return input.isFullySigned();
    }));
};

Transaction.prototype.isValidSignature = function (signature)
{
    var self = this;
    if (this.inputs[signature.inputIndex].isValidSignature === Input.prototype.isValidSignature)
    {
        throw new errors.Transaction.UnableToVerifySignature(
            'Unrecognized script kind, or not enough information to execute script.' +
            'This usually happens when creating a transaction from a serialized transaction'
        );
    }
    return this.inputs[signature.inputIndex].isValidSignature(self, signature);
};

/**
 * @returns {bool} whether the signature is valid for this transaction input
 */
Transaction.prototype.verifySignature = function (sig, pubkey, nin, subscript)
{
    return Sighash.verify(this, sig, pubkey, nin, subscript);
};

/**
 * Check that a transaction passes basic sanity tests. If not, return a string
 * describing the error. This function contains the same logic as
 * CheckTransaction in bitcoin core.
 */
Transaction.prototype.verify = function ()
{
    // Basic checks that don't depend on any context
    if (this.inputs.length === 0)
    {
        return 'transaction txins empty';
    }

    if (this.outputs.length === 0)
    {
        return 'transaction txouts empty';
    }

    // Check for negative or overflow output values
    var valueoutbn = new BN(0);
    for (var i = 0; i < this.outputs.length; i++)
    {
        var txout = this.outputs[i];

        if (txout.invalidSatoshis())
        {
            return 'transaction txout ' + i + ' satoshis is invalid';
        }
        if (txout._satoshisBN.gt(new BN(Transaction.MAX_MONEY, 10)))
        {
            return 'transaction txout ' + i + ' greater than MAX_MONEY';
        }
        valueoutbn = valueoutbn.add(txout._satoshisBN);
        if (valueoutbn.gt(new BN(Transaction.MAX_MONEY)))
        {
            return 'transaction txout ' + i + ' total output greater than MAX_MONEY';
        }
    }

    // Size limits
    if (this.toBuffer().length > MAX_BLOCK_SIZE)
    {
        return 'transaction over the maximum block size';
    }

    // Check for duplicate inputs
    var txinmap = {};
    for (i = 0; i < this.inputs.length; i++)
    {
        var txin = this.inputs[i];

        var inputid = txin.prevTxId + ':' + txin.outputIndex;
        if (!_.isUndefined(txinmap[inputid]))
        {
            return 'transaction input ' + i + ' duplicate input';
        }
        txinmap[inputid] = true;
    }

    var isCoinbase = this.isCoinbase();
    if (isCoinbase)
    {
        var buf = this.inputs[0]._scriptBuffer;
        if (buf.length < 2 || buf.length > 100)
        {
            return 'coinbase transaction script size invalid';
        }
    } else
    {
        for (i = 0; i < this.inputs.length; i++)
        {
            if (this.inputs[i].isNull())
            {
                return 'transaction input ' + i + ' has null input';
            }
        }
    }
    return true;
};

/**
 * Analogous to bitcoind's IsCoinBase function in transaction.h
 */
Transaction.prototype.isCoinbase = function ()
{
    return (this.inputs.length === 1 && this.inputs[0].isNull());
};

/**
 * Determines if this transaction can be replaced in the mempool with another
 * transaction that provides a sufficiently higher fee (RBF).
 */
Transaction.prototype.isRBF = function ()
{
    for (var i = 0; i < this.inputs.length; i++)
    {
        var input = this.inputs[i];
        if (input.sequenceNumber < Input.MAXINT - 1)
        {
            return true;
        }
    }
    return false;
};

/**
 * Enable this transaction to be replaced in the mempool (RBF) if a transaction
 * includes a sufficiently higher fee. It will set the sequenceNumber to
 * DEFAULT_RBF_SEQNUMBER for all inputs if the sequence number does not
 * already enable RBF.
 */
Transaction.prototype.enableRBF = function ()
{
    for (var i = 0; i < this.inputs.length; i++)
    {
        var input = this.inputs[i];
        if (input.sequenceNumber >= Input.MAXINT - 1)
        {
            input.sequenceNumber = Input.DEFAULT_RBF_SEQNUMBER;
        }
    }
    return this;
};

magnachain.Transaction = Transaction;
// transaction/transaction end----------------------------------------------------------------------

// RPC function begin-------------------------------------------------------------------------------
function getXmlHttpRequest() 
{
    if (window.XMLHttpRequest) 
    {
        //主流浏览器提供了XMLHttpRequest对象
        return new XMLHttpRequest();
    }
    else if (window.ActiveXObject) 
    {
        //低版本的IE浏览器没有提供XMLHttpRequest对象
        //所以必须使用IE浏览器的特定实现ActiveXObject
        return new ActiveXObject("Microsoft.XMLHttpRequest");
    }
}

function isEmpty(obj)
{
    if (typeof obj == "undefined" || obj == null || obj == "")
    {
        return true;
    }
    else
    {
        return false;
    }
}

function RpcClient(opts)
{
    opts = opts || {};
    this.host = opts.host || 'http://127.0.0.1';
    this.rpcport = opts.rpcport || 8201;
    this.rpcuser = opts.rpcuser || 'user';
    this.rpcpassword = opts.rpcpassword || 'pwd';
    //this.protocol = (opts.protocol == 'http') ? http : https;
    this.protocol = getXmlHttpRequest();
    this.protocol.withCredentials = true;
    //this.batchedCalls = null;
    //this.disableAgent = opts.disableAgent || false;

    this.protocol.timeout = 3000;
    this.protocol.ontimeout = function (event)
    {
        console.log("Request timeout!");
    }
}

// arguments: callback, method, args...
// callback(status : int, errors : string, jsonRet : ?)
RpcClient.prototype.sendCommand = function ()
{
    var arrArgsRaw;

    if ( arguments.length == 1 && Array.isArray(arguments[0]))
    {
        arrArgsRaw = arguments[0];
    }
    else
    {
        arrArgsRaw = Array.prototype.slice.apply(arguments);
    }

    var i;

    var arrArgs = new Array();
    for ( i = 0; i < arrArgsRaw.length; i++ )
    {
        if ( arrArgsRaw[i] != null && arrArgsRaw[i] != undefined)
        {
            arrArgs.push(arrArgsRaw[i]);
        }
    }

    if (arrArgs.length < 2)
    {
        console.log("Too less arguments");
        return;
    }

    var req = {};
    
    req['method'] = arrArgs[1];
    if (arrArgs.length > 2)
    {
        //req['params'] = slice(arguments, 2, arguments.length - 1);
        req['params'] = arrArgs.slice(2);
    }

    var jsonReq = JSON.stringify(req);

    //var formData = new FormData();
    //formData.append('tel', '18217767969');
    //formData.append('psw', '111111');
    var url = this.host + ':' + this.rpcport.toString();
    this.protocol.open('POST', url);

    var auth = Buffer(this.rpcuser + ':' + this.rpcpassword).toString('base64');

    this.protocol.setRequestHeader('Content-Length', jsonReq.length);
    this.protocol.setRequestHeader('Content-Type', 'application/json');
    this.protocol.setRequestHeader('Authorization', 'Basic ' + auth);

    this.protocol.send(jsonReq);

    var fnCallback = arrArgs[0];
    this.protocol.onreadystatechange = function ()
    {
        if (this.readyState === 4)  //4表示准备完成
        {
            if (this.status === 200)  //200表示回调成功
            {
                //console.log(this.responseText);

                //返回的数据,这里返回的是json格式数据
                var result = JSON.parse(this.responseText);
                if (fnCallback != null)
                {
                    fnCallback(this.status, result.error, result.result);
                }
            }
            else 
            {
                var error = "Request was failure, status: " + this.status + ", " + this.statusText;
                //console.log(errors);
                if (fnCallback != null)
                {
                    fnCallback(this.status, error, null);
                }
            }
        }
        else
        {

        }
    };
}

var _MCRpcSig = null;

function initializeRpc(strHost, iPort, strRpcUser, strRpcPassword)
{
    if (_MCRpcSig != null)
    {
        return;
    }

    var config =
        {
            rpcuser: strRpcUser,
            rpcpassword: strRpcPassword,
            host: strHost,
            port: iPort
        };

    _MCRpcSig = new RpcClient(config);
}

// arguments: callback, method, args...
// callback(status : int, errors : string, jsonRet : ?)
function sendRpcCommand()
{
    if (_MCRpcSig == null)
    {
        console.log("RpcClient has not initialized!");
        return;
    }

    var arrArgs = Array.prototype.slice.apply(arguments);
    _MCRpcSig.sendCommand(arrArgs);
}

magnachain.RpcClient = RpcClient;
magnachain.initializeRpc = initializeRpc;
magnachain.sendRpcCommand = sendRpcCommand;
// RPC function end---------------------------------------------------------------------------------

// MiscFunc begin-----------------------------------------------------------------------------------
var MiscFunc = {};

// 0. fnCallback                     like OnFinish(bSucc)
// 1. "strFromPriKey"                (string, required) The wif private key for input coins
// 2. "strToAddress"                 (string, required) Send to address
// 3. "fAmount"                      (numeric or string, required) The amount in MGC to send. eg 0.1
// 4. "strChargeAddress"             (string, optional) The address for change coins, empty will use from address
// 5. "fFee"                         (numeric or string, optional) The amount in MGC to for fee eg 0.0001, default 0 and will calc fee by system
var _MCRpcTransfering = false;

function transferByRpc(fnCallback, strFromPriKey, strToAddress, fAmount, strChargeAddress, fFee)
{
    if (_MCRpcSig == null)
    {
        console.log("RpcClient has not initialized, can not transfer by Rpc.");
        return;
    }

    if (isEmpty(strFromPriKey) || isEmpty(strToAddress) || fAmount <= 0.0)
    {
        console.log("Invalid paraments!");
        return;
    }

    if (_MCRpcTransfering)
    {
        console.log("Pre transfer has not done, ignore!");
        return;
    }

    _MCRpcTransfering = true;

    var kPriKey = PrivateKey.fromWIF(strFromPriKey);
    var strFromAddress = kPriKey.toAddress().toString();

    var OnRpcSigned = function (status, error, jsonRet)
    {
        if (status != 200 || (error != null && error != undefined))
        {
            console.log("OnRpcSigned status: " + status + " error: " + error + " msg: " + JSON.stringify(jsonRet));

            _MCRpcTransfering = false;

            if (fnCallback != null)
            {
                fnCallback(false);
            }
            return;
        }

        _MCRpcTransfering = false;
        if (fnCallback != null)
        {
            fnCallback(true);
        }
    }

    var OnRpcPreTransaction = function (status, error, jsonRet)
    {
        if (status != 200 || (error != null && error != undefined))
        {
            console.log("OnRpcPreTransaction status: " + status + " error: " + error + " msg: " + JSON.stringify(jsonRet));

            _MCRpcTransfering = false;

            if (fnCallback != null)
            {
                fnCallback(false);
            }
            return;
        }
        //console.log("OnRpcPreTransaction status: " + status + " error: " + error + " msg: " + JSON.stringify(jsonRet));

        var kTras = new Transaction(jsonRet.txhex);
        kTras.setOutputsFromCoins(jsonRet.coins);

        //kTras.sign(prikeys);
        kTras.sign(kPriKey);

        var txsignedhex = kTras.toString();
        //console.log("signed txhex: " + txsignedhex);

        sendRpcCommand(OnRpcSigned, "sendrawtransaction", txsignedhex);
    }

    if (isEmpty(strChargeAddress))
    {
        strChargeAddress = strFromAddress;
    }

    sendRpcCommand(OnRpcPreTransaction, "premaketransaction", strFromAddress, strToAddress, strChargeAddress, fAmount, fFee);
}
MiscFunc.transferByRpc = transferByRpc;

magnachain.MiscFunc = MiscFunc;
// MiscFunc end-------------------------------------------------------------------------------------