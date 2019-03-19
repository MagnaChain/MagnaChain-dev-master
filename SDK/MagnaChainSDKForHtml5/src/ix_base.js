'use strict';

function getFunctionName(func)
{
  if (typeof func == 'function' || typeof func == 'object')
  {
    var name = ('' + func).match(/function\s*([\w\$]*)\s*\(/);
  }
  return name && name[1];
}

if (!('console' in window))
{
  window.console = {};
}

if (!console.trace)
{
  console.trace = function()
  {
    var arrStack = [],
      caller = arguments.callee.caller;

    while (caller)
    {
      arrStack.unshift(getFunctionName(caller));
      caller = caller && caller.caller;
    }

    console.log(arrStack.join('\n'));
  }
};

var ix_base = {};

function assert(val, msg)
{
  if (!val) throw new Error(msg || 'Assertion failed');
}
ix_base.assert = assert;

if (typeof Object.create === 'function')
{
  // implementation from standard node.js 'util' module
  ix_base.inherits = function (ctor, superCtor)
  {
    ctor.super_ = superCtor
    ctor.prototype = Object.create(superCtor.prototype, {
      constructor: {
        value: ctor,
        enumerable: false,
        writable: true,
        configurable: true
      }
    });
  };
}
else
{
  // old school shim for old browsers
  ix_base.inherits = function (ctor, superCtor)
  {
    ctor.super_ = superCtor
    var TempCtor = function () { }
    TempCtor.prototype = superCtor.prototype
    ctor.prototype = new TempCtor()
    ctor.prototype.constructor = ctor
  }
}

// minimalistic-crypto-utils--------------------------------
var minic_utils = {};
ix_base.minic_utils = minic_utils;

function toArray(msg, enc)
{
  if (Array.isArray(msg))
    return msg.slice();
  if (!msg)
    return [];
  var res = [];
  if (typeof msg !== 'string')
  {
    for (var i = 0; i < msg.length; i++)
      res[i] = msg[i] | 0;
    return res;
  }
  if (enc === 'hex')
  {
    msg = msg.replace(/[^a-z0-9]+/ig, '');
    if (msg.length % 2 !== 0)
      msg = '0' + msg;
    for (var i = 0; i < msg.length; i += 2)
      res.push(parseInt(msg[i] + msg[i + 1], 16));
  } else
  {
    for (var i = 0; i < msg.length; i++)
    {
      var c = msg.charCodeAt(i);
      var hi = c >> 8;
      var lo = c & 0xff;
      if (hi)
        res.push(hi, lo);
      else
        res.push(lo);
    }
  }
  return res;
}
minic_utils.toArray = toArray;

function zero2(word)
{
  if (word.length === 1)
    return '0' + word;
  else
    return word;
}
minic_utils.zero2 = zero2;

function toHex(msg)
{
  var res = '';
  for (var i = 0; i < msg.length; i++)
    res += zero2(msg[i].toString(16));
  return res;
}
minic_utils.toHex = toHex;

minic_utils.encode = function encode(arr, enc)
{
  if (enc === 'hex')
    return toHex(arr);
  else
    return arr;
};
//----------------------------------------------------------
