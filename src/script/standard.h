// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_SCRIPT_STANDARD_H
#define CELLLINK_SCRIPT_STANDARD_H

#include "script/interpreter.h"
#include "coding/uint256.h"

#include <boost/variant.hpp>

#include <stdint.h>

static const bool DEFAULT_ACCEPT_DATACARRIER = true;

class CellKeyID;
class CellScript;

/** A reference to a CellScript: the Hash160 of its serialization (see script.h) */
class CellScriptID : public uint160
{
public:
    CellScriptID() : uint160() {}
    CellScriptID(const CellScript& in);
    CellScriptID(const uint160& in) : uint160(in) {}
};

static const unsigned int MAX_OP_RETURN_RELAY = 83; //!< bytes (+1 for OP_RETURN, +2 for the pushdata opcodes)
extern bool fAcceptDatacarrier;
extern unsigned nMaxDatacarrierBytes;

/**
 * Mandatory script verification flags that all new blocks must comply with for
 * them to be valid. (but old blocks may not comply with) Currently just P2SH,
 * but in the future other flags may be added, such as a soft-fork to enforce
 * strict DER encoding.
 * 
 * Failing one of these tests may trigger a DoS ban - see CheckInputs() for
 * details.
 */
static const unsigned int MANDATORY_SCRIPT_VERIFY_FLAGS = SCRIPT_VERIFY_P2SH;

enum txnouttype
{
    TX_NONSTANDARD,
    // 'standard' transaction types:
    TX_PUBKEY,
    TX_PUBKEYHASH,
    TX_SCRIPTHASH,
    TX_MULTISIG,
    TX_NULL_DATA,
    TX_WITNESS_V0_SCRIPTHASH,
    TX_WITNESS_V0_KEYHASH,
	TX_CREATE_BRANCH,
	TX_TRANS_BRANCH,
    TX_SEND_BRANCH,
    TX_MINE_MORTGAGE,
    TX_MORTGAGE_COIN,
    TX_REDEEM_MORTGAGE,
};

class CellNoDestination {
public:
    friend bool operator==(const CellNoDestination &a, const CellNoDestination &b) { return true; }
    friend bool operator<(const CellNoDestination &a, const CellNoDestination &b) { return true; }
};

/** 
 * A txout script template with a specific destination. It is either:
 *  * CellNoDestination: no destination set
 *  * CellKeyID: TX_PUBKEYHASH destination
 *  * CellScriptID: TX_SCRIPTHASH destination
 *  A CellTxDestination is the internal data type encoded in a CellLinkAddress
 */
typedef boost::variant<CellNoDestination, CellKeyID, CellScriptID, CellContractID> CellTxDestination;

const char* GetTxnOutputType(txnouttype t);

bool Solver(const CellScript& scriptPubKey, txnouttype& typeRet, std::vector<std::vector<unsigned char> >& vSolutionsRet);
bool ExtractDestination(const CellScript& scriptPubKey, CellTxDestination& addressRet);
bool ExtractDestinations(const CellScript& scriptPubKey, txnouttype& typeRet, std::vector<CellTxDestination>& addressRet, int& nRequiredRet);

CellScript GetScriptForDestination(const CellTxDestination& dest);
CellScript GetScriptForRawPubKey(const CellPubKey& pubkey);
CellScript GetScriptForMultisig(int nRequired, const std::vector<CellPubKey>& keys);
CellScript GetScriptForWitness(const CellScript& redeemscript);

#endif // CELLLINK_SCRIPT_STANDARD_H
