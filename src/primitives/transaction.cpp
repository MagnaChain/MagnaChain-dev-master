// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/transaction.h"

#include "coding/hash.h"
#include "misc/tinyformat.h"
#include "utils/utilstrencodings.h"
#include "block.h"

std::string CellOutPoint::ToString() const
{
    return strprintf("CellOutPoint(%s, %u)", hash.ToString().substr(0,10), n);
}

CellTxIn::CellTxIn(CellOutPoint prevoutIn, CellScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

CellTxIn::CellTxIn(uint256 hashPrevTx, uint32_t nOut, CellScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = CellOutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

std::string CellTxIn::ToString() const
{
    std::string str;
    str += "CellTxIn(";
    str += prevout.ToString();
    if (prevout.IsNull())
        str += strprintf(", coinbase %s", HexStr(scriptSig));
    else
        str += strprintf(", scriptSig=%s", HexStr(scriptSig).substr(0, 24));
    if (nSequence != SEQUENCE_FINAL)
        str += strprintf(", nSequence=%u", nSequence);
    str += ")";
    return str;
}

CellTxOut::CellTxOut(const CellAmount& nValueIn, CellScript scriptPubKeyIn)
{
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
}

std::string CellTxOut::ToString() const
{
    return strprintf("CellTxOut(nValue=%d.%08d, scriptPubKey=%s)", nValue / COIN, nValue % COIN, HexStr(scriptPubKey).substr(0, 30));
}

CellMutableTransaction::CellMutableTransaction() : nVersion(CellTransaction::CURRENT_VERSION), nLockTime(0) {}
CellMutableTransaction::CellMutableTransaction(const CellTransaction& tx) : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime) {
	if (nVersion == CellTransaction::PUBLISH_CONTRACT_VERSION || nVersion == CellTransaction::CALL_CONTRACT_VERSION)
        pContractData.reset(tx.pContractData == nullptr ? nullptr : new ContractData(*tx.pContractData));
	else if (nVersion == CellTransaction::CREATE_BRANCH_VERSION)
	{
		branchVSeeds = tx.branchVSeeds;
		branchSeedSpec6 = tx.branchSeedSpec6;
	}
	else if (nVersion == CellTransaction::TRANS_BRANCH_VERSION_S1)
	{
		sendToBranchid = tx.sendToBranchid;
		sendToTxHexData = tx.sendToTxHexData;
        if (sendToBranchid == "main"){
            pPMT.reset(tx.pPMT == nullptr ? nullptr : new CellSpvProof(*tx.pPMT));
        }
	}
	else if (nVersion == CellTransaction::TRANS_BRANCH_VERSION_S2)
	{
		fromBranchId = tx.fromBranchId;
		fromTx = tx.fromTx;
		inAmount = tx.inAmount;
        if (tx.fromBranchId != "main") {
            pPMT.reset(tx.pPMT == nullptr ? nullptr : new CellSpvProof(*tx.pPMT));
        }
	}
    else if (nVersion == CellTransaction::MINE_BRANCH_MORTGAGE)
    {
        sendToBranchid = tx.sendToBranchid;
        sendToTxHexData = tx.sendToTxHexData;
    }
    else if (nVersion == CellTransaction::SYNC_BRANCH_INFO)
    {
        pBranchBlockData.reset((tx.pBranchBlockData == nullptr ? nullptr : new CellBranchBlockInfo(*tx.pBranchBlockData)));
    }
    else if (nVersion == CellTransaction::REPORT_CHEAT)
    {
        pReportData.reset(tx.pReportData == nullptr? nullptr: new ReportData(*tx.pReportData));
        pPMT.reset(tx.pPMT == nullptr ? nullptr : new CellSpvProof(*tx.pPMT));
    }
    else if (nVersion == CellTransaction::PROVE)
    {
        pProveData.reset(tx.pProveData == nullptr ? nullptr : new ProveData(*tx.pProveData));
    }
    else if (nVersion == CellTransaction::REDEEM_MORTGAGE)
    {
        fromBranchId = tx.fromBranchId;
        fromTx = tx.fromTx;
        pPMT.reset(tx.pPMT == nullptr ? nullptr : new CellSpvProof(*tx.pPMT));
    }
    else if (nVersion == CellTransaction::REPORT_REWARD)
    {
        reporttxid = tx.reporttxid;
    }
    else if (nVersion == CellTransaction::LOCK_MORTGAGE_MINE_COIN)
    {
        reporttxid = tx.reporttxid;
        coinpreouthash = tx.coinpreouthash;
    }
    else if (nVersion == CellTransaction::UNLOCK_MORTGAGE_MINE_COIN)
    {
        reporttxid = tx.reporttxid;
        coinpreouthash = tx.coinpreouthash;
        provetxid = tx.provetxid;
    }
}

uint256 CellMutableTransaction::GetHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 CellTransaction::ComputeHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 CellTransaction::GetWitnessHash() const
{
    if (!HasWitness()) {
        return GetHash();
    }
    return SerializeHash(*this, SER_GETHASH, 0);
}

/* For backward compatibility, the hash is initialized to 0. TODO: remove the need for this default constructor entirely. */
//CellTransaction::CellTransaction() : nVersion(CellTransaction::CURRENT_VERSION), vin(), vout(), nLockTime(0), hash() {}
//CellTransaction::CellTransaction(const CellMutableTransaction &tx) : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime), hash(ComputeHash()) {}
//CellTransaction::CellTransaction(CellMutableTransaction &&tx) : nVersion(tx.nVersion), vin(std::move(tx.vin)), vout(std::move(tx.vout)), nLockTime(tx.nLockTime), hash(ComputeHash()) {}



CellAmount CellTransaction::GetValueOut() const
{
    CellAmount nValueOut = 0;
    for (const auto& tx_out : vout) {
        nValueOut += tx_out.nValue;
        if (!MoneyRange(tx_out.nValue) || !MoneyRange(nValueOut))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    return nValueOut;
}

unsigned int CellTransaction::GetTotalSize() const
{
    return ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
}

std::string CellTransaction::ToString() const
{
	std::string str;
	str += strprintf("CellTransaction(hash=%s, ver=%d, vin.size=%u, vout.size=%u, nLockTime=%u)\n",
		GetHash().ToString().substr(0, 10),
		nVersion,
		vin.size(),
		vout.size(),
		nLockTime);
	for (const auto& tx_in : vin)
		str += "    " + tx_in.ToString() + "\n";
	for (const auto& tx_in : vin)
		str += "    " + tx_in.scriptWitness.ToString() + "\n";
	for (const auto& tx_out : vout)
		str += "    " + tx_out.ToString() + "\n";
	return str;
}

CellBranchBlockInfo::CellBranchBlockInfo()
{
    SetNull();
}

CellBranchBlockInfo::CellBranchBlockInfo(const CellBranchBlockInfo& r)
    : nVersion(r.nVersion), hashPrevBlock(r.hashPrevBlock), hashMerkleRoot(r.hashMerkleRoot), hashMerkleRootWithData(r.hashMerkleRootWithData),
      hashMerkleRootWithPrevData(r.hashMerkleRootWithPrevData), nTime(r.nTime), nBits(r.nBits), nNonce(r.nNonce), vchBlockSig(r.vchBlockSig),
      prevoutStake(r.prevoutStake), branchID(r.branchID),blockHeight(r.blockHeight), vchStakeTxData(r.vchStakeTxData)
{
}

void CellBranchBlockInfo::SetNull()
{
    nVersion = 0;
    hashPrevBlock.SetNull();
    hashMerkleRoot.SetNull();
    hashMerkleRootWithData.SetNull();
    hashMerkleRootWithPrevData.SetNull();
    nTime = 0;
    nBits = 0;
    nNonce = 0;

    vchBlockSig.clear();
    prevoutStake.SetNull();

    branchID.SetNull();
	blockHeight = 0;
    vchStakeTxData.clear();
}

void CellBranchBlockInfo::GetBlockHeader(CellBlockHeader& block) const
{
    block.nVersion = nVersion;
    block.hashPrevBlock = hashPrevBlock;
    block.hashMerkleRoot = hashMerkleRoot;
    block.hashMerkleRootWithData = hashMerkleRootWithData;
    block.hashMerkleRootWithPrevData = hashMerkleRootWithPrevData;
    block.nTime = nTime;
    block.nBits = nBits;
    block.nNonce = nNonce;
    block.prevoutStake = prevoutStake;
    block.vchBlockSig = vchBlockSig;
}

void CellBranchBlockInfo::SetBlockHeader(const CellBlockHeader& block)
{
    nVersion = block.nVersion;
    hashPrevBlock = block.hashPrevBlock;
    hashMerkleRoot = block.hashMerkleRoot;
    hashMerkleRootWithData = block.hashMerkleRootWithData;
    hashMerkleRootWithPrevData = block.hashMerkleRootWithPrevData;
    nTime = block.nTime;
    nBits = block.nBits;
    nNonce = block.nNonce;
    prevoutStake = block.prevoutStake;
    vchBlockSig = block.vchBlockSig;
}
