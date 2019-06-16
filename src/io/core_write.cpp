// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "io/core_io.h"

#include "coding/base58.h"
#include "consensus/consensus.h"
#include "consensus/validation.h"
#include "script/script.h"
#include "script/standard.h"
#include "io/serialize.h"
#include "io/streams.h"
#include <univalue.h>
#include "utils/util.h"
#include "utils/utilmoneystr.h"
#include "utils/utilstrencodings.h"

UniValue ValueFromAmount(const MCAmount& amount)
{
    bool sign = amount < 0;
    int64_t n_abs = (sign ? -amount : amount);
    int64_t quotient = n_abs / COIN;
    int64_t remainder = n_abs % COIN;
    return UniValue(UniValue::VNUM,
            strprintf("%s%d.%08d", sign ? "-" : "", quotient, remainder));
}

std::string FormatScript(const MCScript& script)
{
    std::string ret;
    MCScript::const_iterator it = script.begin();
    opcodetype op;
    while (it != script.end()) {
        MCScript::const_iterator it2 = it;
        std::vector<unsigned char> vch;
        if (script.GetOp2(it, op, &vch)) {
            if (op == OP_0) {
                ret += "0 ";
                continue;
            } else if ((op >= OP_1 && op <= OP_16) || op == OP_1NEGATE) {
                ret += strprintf("%i ", op - OP_1NEGATE - 1);
                continue;
            } else if (op >= OP_NOP && op <= OP_NOP10) {
                std::string str(GetOpName(op));
                if (str.substr(0, 3) == std::string("OP_")) {
                    ret += str.substr(3, std::string::npos) + " ";
                    continue;
                }
            }
            if (vch.size() > 0) {
                ret += strprintf("0x%x 0x%x ", HexStr(it2, it - vch.size()), HexStr(it - vch.size(), it));
            } else {
                ret += strprintf("0x%x ", HexStr(it2, it));
            }
            continue;
        }
        ret += strprintf("0x%x ", HexStr(it2, script.end()));
        break;
    }
    return ret.substr(0, ret.size() - 1);
}

const std::map<unsigned char, std::string> mapSigHashTypes = {
    {static_cast<unsigned char>(SIGHASH_ALL), std::string("ALL")},
    {static_cast<unsigned char>(SIGHASH_ALL|SIGHASH_ANYONECANPAY), std::string("ALL|ANYONECANPAY")},
    {static_cast<unsigned char>(SIGHASH_NONE), std::string("NONE")},
    {static_cast<unsigned char>(SIGHASH_NONE|SIGHASH_ANYONECANPAY), std::string("NONE|ANYONECANPAY")},
    {static_cast<unsigned char>(SIGHASH_SINGLE), std::string("SINGLE")},
    {static_cast<unsigned char>(SIGHASH_SINGLE|SIGHASH_ANYONECANPAY), std::string("SINGLE|ANYONECANPAY")},
};

/**
 * Create the assembly string representation of a MCScript object.
 * @param[in] script    MCScript object to convert into the asm string representation.
 * @param[in] fAttemptSighashDecode    Whether to attempt to decode sighash types on data within the script that matches the format
 *                                     of a signature. Only pass true for scripts you believe could contain signatures. For example,
 *                                     pass false, or omit the this argument (defaults to false), for scriptPubKeys.
 */
std::string ScriptToAsmStr(const MCScript& script, const bool fAttemptSighashDecode)
{
    std::string str;
    opcodetype opcode;
    std::vector<unsigned char> vch;
    MCScript::const_iterator pc = script.begin();
    while (pc < script.end()) {
        if (!str.empty()) {
            str += " ";
        }
        if (!script.GetOp(pc, opcode, vch)) {
            str += "[error]";
            return str;
        }
        if (0 <= opcode && opcode <= OP_PUSHDATA4) {
            if (vch.size() <= static_cast<std::vector<unsigned char>::size_type>(4)) {
                str += strprintf("%d", CScriptNum(vch, false).getint());
            } else {
                // the IsUnspendable check makes sure not to try to decode OP_RETURN data that may match the format of a signature
                if (fAttemptSighashDecode && !script.IsUnspendable()) {
                    std::string strSigHashDecode;
                    // goal: only attempt to decode a defined sighash type from data that looks like a signature within a scriptSig.
                    // this won't decode correctly formatted public keys in Pubkey or Multisig scripts due to
                    // the restrictions on the pubkey formats (see IsCompressedOrUncompressedPubKey) being incongruous with the
                    // checks in CheckSignatureEncoding.
                    if (CheckSignatureEncoding(vch, SCRIPT_VERIFY_STRICTENC, nullptr)) {
                        const unsigned char chSigHashType = vch.back();
                        if (mapSigHashTypes.count(chSigHashType)) {
                            strSigHashDecode = "[" + mapSigHashTypes.find(chSigHashType)->second + "]";
                            vch.pop_back(); // remove the sighash type byte. it will be replaced by the decode.
                        }
                    }
                    str += HexStr(vch) + strSigHashDecode;
                } else {
                    str += HexStr(vch);
                }
            }
        } else {
            str += GetOpName(opcode);
        }
    }
    return str;
}

void ScriptPubKeyToUniv(const MCScript& scriptPubKey,
                        UniValue& out, bool fIncludeHex)
{
    txnouttype type;
    std::vector<MCTxDestination> addresses;
    int nRequired;

    out.pushKV("asm", ScriptToAsmStr(scriptPubKey));
    if (fIncludeHex)
        out.pushKV("hex", HexStr(scriptPubKey.begin(), scriptPubKey.end()));

    if (!ExtractDestinations(scriptPubKey, type, addresses, nRequired)) {
        out.pushKV("type", GetTxnOutputType(type));
        return;
    }

    out.pushKV("reqSigs", nRequired);
    out.pushKV("type", GetTxnOutputType(type));

    UniValue a(UniValue::VARR);
    for (const MCTxDestination& addr : addresses)
        a.push_back(MagnaChainAddress(addr).ToString());
    out.pushKV("addresses", a);
}

// desc transaction functional
void TxVersionToString(const int32_t nVersion, UniValue& entry)
{
    switch (nVersion)
    {
    case MCTransaction::PUBLISH_CONTRACT_VERSION:
        entry.pushKV("versionType", "publishContract");
        break;
    case MCTransaction::CALL_CONTRACT_VERSION:
        entry.pushKV("versionType", "callContract");
        break;
    case MCTransaction::CREATE_BRANCH_VERSION:
        entry.pushKV("versionType", "createBranch");
        break;
    case MCTransaction::TRANS_BRANCH_VERSION_S1:
        entry.pushKV("versionType", "transBranchStep1");
        break;
    case MCTransaction::TRANS_BRANCH_VERSION_S2:
        entry.pushKV("versionType", "transBranchStep2");
        break;
    case MCTransaction::MINE_BRANCH_MORTGAGE:
        entry.pushKV("versionType", "mineBranchMortgage");
        break;
    case MCTransaction::SYNC_BRANCH_INFO:
        entry.pushKV("versionType", "syncBranchInfo");
        break;
    case MCTransaction::REPORT_CHEAT:
        entry.pushKV("versionType", "reportCheat");
        break;
    case MCTransaction::PROVE:
        entry.pushKV("versionType", "prove");
        break;
    case MCTransaction::REDEEM_MORTGAGE_STATEMENT:
        entry.pushKV("versionType", "redeemMortgageStatement");
        break;
    case MCTransaction::REDEEM_MORTGAGE:
        entry.pushKV("versionType", "redeemMortgage");
        break;
    case MCTransaction::STAKE:
        entry.pushKV("versionType", "stake");
        break;
    case MCTransaction::REPORT_REWARD:
        entry.pushKV("versionType", "reportReward");
        break;
    case MCTransaction::LOCK_MORTGAGE_MINE_COIN:
        entry.pushKV("versionType", "lockMortgageMineCoin");
        break;
    case MCTransaction::UNLOCK_MORTGAGE_MINE_COIN:
        entry.pushKV("versionType", "unlockMortgageMineCoin");
        break;
    default:
        break;
    }
}

void TxToUniv(const MCTransaction& tx, const uint256& hashBlock, UniValue& entry, bool include_hex, int serialize_flags)
{
    entry.pushKV("txid", tx.GetHash().GetHex());
    entry.pushKV("hash", tx.GetWitnessHash().GetHex());
    entry.pushKV("version", tx.nVersion);
    TxVersionToString(tx.nVersion, entry);
    entry.pushKV("size", (int)::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION));
    entry.pushKV("vsize", (GetTransactionWeight(tx) + WITNESS_SCALE_FACTOR - 1) / WITNESS_SCALE_FACTOR);
    entry.pushKV("locktime", (int64_t)tx.nLockTime);

    UniValue vin(UniValue::VARR);
    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        const MCTxIn& txin = tx.vin[i];
        UniValue in(UniValue::VOBJ);
        if (tx.IsCoinBase())
            in.pushKV("coinbase", HexStr(txin.scriptSig.begin(), txin.scriptSig.end()));
        else {
            in.pushKV("txid", txin.prevout.hash.GetHex());
            in.pushKV("vout", (int64_t)txin.prevout.n);
            UniValue o(UniValue::VOBJ);
            o.pushKV("asm", ScriptToAsmStr(txin.scriptSig, true));
            o.pushKV("hex", HexStr(txin.scriptSig.begin(), txin.scriptSig.end()));
            in.pushKV("scriptSig", o);
            if (!tx.vin[i].scriptWitness.IsNull()) {
                UniValue txinwitness(UniValue::VARR);
                for (const auto& item : tx.vin[i].scriptWitness.stack) {
                    txinwitness.push_back(HexStr(item.begin(), item.end()));
                }
                in.pushKV("txinwitness", txinwitness);
            }
        }
        in.pushKV("sequence", (int64_t)txin.nSequence);
        vin.push_back(in);
    }
    entry.pushKV("vin", vin);

    UniValue vout(UniValue::VARR);
    for (unsigned int i = 0; i < tx.vout.size(); i++) {
        const MCTxOut& txout = tx.vout[i];

        UniValue out(UniValue::VOBJ);

        out.pushKV("value", ValueFromAmount(txout.nValue));
        out.pushKV("n", (int64_t)i);

        UniValue o(UniValue::VOBJ);
        ScriptPubKeyToUniv(txout.scriptPubKey, o, true);
        out.pushKV("scriptPubKey", o);
        vout.push_back(out);
    }
    entry.pushKV("vout", vout);

    if (!hashBlock.IsNull()) {
        entry.pushKV("blockhash", hashBlock.GetHex());
    }

    if (tx.nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION || tx.nVersion == MCTransaction::CALL_CONTRACT_VERSION) {
        UniValue contractdata(UniValue::VOBJ);
        contractdata.pushKV("contractaddress", MagnaChainAddress(tx.pContractData->address).ToString());
        contractdata.pushKV("senderpubkey", tx.pContractData->sender.GetID().ToString());//HexStr(tx.pContractData->sender.begin(), tx.pContractData->sender.end())

        if (tx.nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION)
            contractdata.pushKV("codeOrFunc", HexStr(tx.pContractData->codeOrFunc.begin(), tx.pContractData->codeOrFunc.end()));
        else if (tx.nVersion == MCTransaction::CALL_CONTRACT_VERSION)
            contractdata.pushKV("codeOrFunc", tx.pContractData->codeOrFunc);
        contractdata.pushKV("args", tx.pContractData->args);

        contractdata.pushKV("contractcoinsin", tx.pContractData->contractCoinsIn);
        UniValue contractCoinsOut(UniValue::VOBJ);
        for (auto it : tx.pContractData->contractCoinsOut) {
            contractCoinsOut.pushKV(it.first.ToString(), it.second);
        }
        contractdata.pushKV("contractcoinsout", contractCoinsOut);

        UniValue o(UniValue::VOBJ);
        o.pushKV("asm", ScriptToAsmStr(tx.pContractData->signature, true));
        o.pushKV("hex", HexStr(tx.pContractData->signature.begin(), tx.pContractData->signature.end()));
        contractdata.pushKV("signature", o);

        entry.pushKV("contractdata", contractdata);
    }
    if (tx.nVersion == MCTransaction::CREATE_BRANCH_VERSION) {
        entry.pushKV("branchVSeeds", tx.branchVSeeds);
        entry.pushKV("branchSeedSpec6", tx.branchSeedSpec6);
    }
    if (tx.nVersion == MCTransaction::TRANS_BRANCH_VERSION_S1) {
        entry.pushKV("sendToBranchid", tx.pBranchTransactionData->branchId);
        entry.pushKV("sendToTxHexData", HexStr(tx.pBranchTransactionData->txData));
    }
    if (tx.nVersion == MCTransaction::TRANS_BRANCH_VERSION_S2) {
        entry.pushKV("fromBranchId", tx.pBranchTransactionData->branchId);
        entry.pushKV("inAmount", ValueFromAmount(tx.pBranchTransactionData->inAmount));
        MCTransactionRef pfromtx;
        MCDataStream cds(tx.pBranchTransactionData->txData, SER_NETWORK, INIT_PROTO_VERSION);
        cds >> (pfromtx);
        entry.pushKV("fromTxid", pfromtx->GetHash().GetHex());
    }
    if (tx.nVersion == MCTransaction::SYNC_BRANCH_INFO) {
        entry.pushKV("branchid", tx.pBranchBlockData->branchID.GetHex());
        entry.pushKV("branchblockheight", tx.pBranchBlockData->blockHeight);
        MCBlockHeader block;
        tx.pBranchBlockData->GetBlockHeader(block);
        entry.pushKV("branchblockhash", block.GetHash().GetHex());
    }
    if (tx.IsRedeemMortgage()) {
        entry.pushKV("fromBranchId", tx.pBranchTransactionData->branchId);
        entry.pushKV("inAmount", ValueFromAmount(tx.pBranchTransactionData->inAmount));
        MCTransactionRef pfromtx;
        MCDataStream cds(tx.pBranchTransactionData->txData, SER_NETWORK, INIT_PROTO_VERSION);
        cds >> (pfromtx);
        entry.pushKV("fromTxid", pfromtx->GetHash().GetHex());
    }

    if (include_hex) {
        entry.pushKV("hex", EncodeHexTx(tx, serialize_flags)); // the hex-encoded transaction. used the name "hex" to be consistent with the verbose output of "getrawtransaction".
    }
}
