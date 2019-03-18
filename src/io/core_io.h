// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_CORE_IO_H
#define MAGNACHAIN_CORE_IO_H

#include "misc/amount.h"

#include <string>
#include <vector>

class MCBlock;
class MCScript;
class MCTransaction;
struct MCMutableTransaction;
class uint256;
class UniValue;
class MCSpvProof;

// core_read.cpp
MCScript ParseScript(const std::string& s);
std::string ScriptToAsmStr(const MCScript& script, const bool fAttemptSighashDecode = false);
bool DecodeHexTx(MCMutableTransaction& tx, const std::string& strHexTx, bool fTryNoWitness = false);
bool DecodeHexSpv(MCSpvProof& spv, const std::string& strHexSpv, bool fTryNoWitness = false);
bool DecodeHexBlk(MCBlock&, const std::string& strHexBlk);
uint256 ParseHashUV(const UniValue& v, const std::string& strName);
uint256 ParseHashStr(const std::string&, const std::string& strName);
std::vector<unsigned char> ParseHexUV(const UniValue& v, const std::string& strName);

// core_write.cpp
UniValue ValueFromAmount(const MCAmount& amount);
std::string FormatScript(const MCScript& script);
std::string EncodeHexTx(const MCTransaction& tx, const int serializeFlags = 0);
std::string EncodeHexSpvProof(const MCSpvProof& spv, const int serializeFlags = 0);
void ScriptPubKeyToUniv(const MCScript& scriptPubKey, UniValue& out, bool fIncludeHex);
void TxToUniv(const MCTransaction& tx, const uint256& hashBlock, UniValue& entry, bool include_hex = true, int serialize_flags = 0);

#endif // MAGNACHAIN_CORE_IO_H
