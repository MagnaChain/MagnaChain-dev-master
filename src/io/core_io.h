// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_CORE_IO_H
#define CELLLINK_CORE_IO_H

#include "misc/amount.h"

#include <string>
#include <vector>

class CellBlock;
class CellScript;
class CellTransaction;
struct CellMutableTransaction;
class uint256;
class UniValue;
class CellSpvProof;

// core_read.cpp
CellScript ParseScript(const std::string& s);
std::string ScriptToAsmStr(const CellScript& script, const bool fAttemptSighashDecode = false);
bool DecodeHexTx(CellMutableTransaction& tx, const std::string& strHexTx, bool fTryNoWitness = false);
bool DecodeHexSpv(CellSpvProof& spv, const std::string& strHexSpv, bool fTryNoWitness = false);
bool DecodeHexBlk(CellBlock&, const std::string& strHexBlk);
uint256 ParseHashUV(const UniValue& v, const std::string& strName);
uint256 ParseHashStr(const std::string&, const std::string& strName);
std::vector<unsigned char> ParseHexUV(const UniValue& v, const std::string& strName);

// core_write.cpp
UniValue ValueFromAmount(const CellAmount& amount);
std::string FormatScript(const CellScript& script);
std::string EncodeHexTx(const CellTransaction& tx, const int serializeFlags = 0);
std::string EncodeHexSpvProof(const CellSpvProof& spv, const int serializeFlags = 0);
void ScriptPubKeyToUniv(const CellScript& scriptPubKey, UniValue& out, bool fIncludeHex);
void TxToUniv(const CellTransaction& tx, const uint256& hashBlock, UniValue& entry, bool include_hex = true, int serialize_flags = 0);

#endif // CELLLINK_CORE_IO_H
