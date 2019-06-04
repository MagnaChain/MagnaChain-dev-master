// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/transaction.h"

#include "coding/hash.h"
#include "misc/tinyformat.h"
#include "utils/utilstrencodings.h"
#include "block.h"
#include "chain/chainparamsbase.h"

std::string MCOutPoint::ToString() const
{
    return strprintf("MCOutPoint(%s, %u)", hash.ToString().substr(0,10), n);
}

MCTxIn::MCTxIn(MCOutPoint prevoutIn, MCScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

MCTxIn::MCTxIn(uint256 hashPrevTx, uint32_t nOut, MCScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = MCOutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

std::string MCTxIn::ToString() const
{
    std::string str;
    str += "MCTxIn(";
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

MCTxOut::MCTxOut(const MCAmount& nValueIn, MCScript scriptPubKeyIn)
{
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
}

std::string MCTxOut::ToString() const
{
    return strprintf("MCTxOut(nValue=%d.%08d, scriptPubKey=%s)", nValue / COIN, nValue % COIN, HexStr(scriptPubKey).substr(0, 30));
}

MCMutableTransaction::MCMutableTransaction()
    : nVersion(MCTransaction::CURRENT_VERSION), nLockTime(0), inAmount(0)
{
}

MCMutableTransaction::MCMutableTransaction(const MCTransaction& tx)
    : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime), inAmount(tx.inAmount)
{
	if (nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION || nVersion == MCTransaction::CALL_CONTRACT_VERSION)
        pContractData.reset(tx.pContractData == nullptr ? nullptr : new ContractData(*tx.pContractData));
	else if (nVersion == MCTransaction::CREATE_BRANCH_VERSION)
	{
		branchVSeeds = tx.branchVSeeds;
		branchSeedSpec6 = tx.branchSeedSpec6;
	}
	else if (nVersion == MCTransaction::TRANS_BRANCH_VERSION_S1)
	{
		sendToBranchid = tx.sendToBranchid;
		sendToTxHexData = tx.sendToTxHexData;
        //if (sendToBranchid == "main"){
        //    pPMT.reset(tx.pPMT == nullptr ? nullptr : new MCSpvProof(*tx.pPMT));
        //}
	}
	else if (nVersion == MCTransaction::TRANS_BRANCH_VERSION_S2)
	{
		fromBranchId = tx.fromBranchId;
		fromTx = tx.fromTx;
		inAmount = tx.inAmount;
        //if (tx.fromBranchId != "main") {
        //    pPMT.reset(tx.pPMT == nullptr ? nullptr : new MCSpvProof(*tx.pPMT));
        //}
	}
    else if (nVersion == MCTransaction::MINE_BRANCH_MORTGAGE)
    {
        sendToBranchid = tx.sendToBranchid;
        sendToTxHexData = tx.sendToTxHexData;
    }
    //else if (nVersion == MCTransaction::SYNC_BRANCH_INFO)
    //{
    //    pBranchBlockData.reset((tx.pBranchBlockData == nullptr ? nullptr : new MCBranchBlockInfo(*tx.pBranchBlockData)));
    //}
    //else if (nVersion == MCTransaction::REPORT_CHEAT)
    //{
    //    pReportData.reset(tx.pReportData == nullptr? nullptr: new ReportData(*tx.pReportData));
    //    pPMT.reset(tx.pPMT == nullptr ? nullptr : new MCSpvProof(*tx.pPMT));
    //}
    //else if (nVersion == MCTransaction::PROVE)
    //{
    //    pProveData.reset(tx.pProveData == nullptr ? nullptr : new ProveData(*tx.pProveData));
    //}
    else if (nVersion == MCTransaction::REDEEM_MORTGAGE)
    {
        fromBranchId = tx.fromBranchId;
        fromTx = tx.fromTx;
        //pPMT.reset(tx.pPMT == nullptr ? nullptr : new MCSpvProof(*tx.pPMT));
    }
    //else if (nVersion == MCTransaction::REPORT_REWARD)
    //{
    //    reporttxid = tx.reporttxid;
    //}
    //else if (nVersion == MCTransaction::LOCK_MORTGAGE_MINE_COIN)
    //{
    //    reporttxid = tx.reporttxid;
    //    coinpreouthash = tx.coinpreouthash;
    //}
    //else if (nVersion == MCTransaction::UNLOCK_MORTGAGE_MINE_COIN)
    //{
    //    reporttxid = tx.reporttxid;
    //    coinpreouthash = tx.coinpreouthash;
    //    provetxid = tx.provetxid;
    //}
}

uint256 MCMutableTransaction::GetHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 MCTransaction::ComputeHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 MCTransaction::GetWitnessHash() const
{
    if (!HasWitness()) {
        return GetHash();
    }
    return SerializeHash(*this, SER_GETHASH, 0);
}

/* For backward compatibility, the hash is initialized to 0. TODO: remove the need for this default constructor entirely. */
//MCTransaction::MCTransaction() : nVersion(MCTransaction::CURRENT_VERSION), vin(), vout(), nLockTime(0), hash() {}
//MCTransaction::MCTransaction(const MCMutableTransaction &tx) : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime), hash(ComputeHash()) {}
//MCTransaction::MCTransaction(MCMutableTransaction &&tx) : nVersion(tx.nVersion), vin(std::move(tx.vin)), vout(std::move(tx.vout)), nLockTime(tx.nLockTime), hash(ComputeHash()) {}



MCAmount MCTransaction::GetValueOut() const
{
    MCAmount nValueOut = 0;
    for (const auto& tx_out : vout) {
        nValueOut += tx_out.nValue;
        if (!MoneyRange(tx_out.nValue) || !MoneyRange(nValueOut))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    if (IsSmartContract()) {
        nValueOut += pContractData->contractCoinsIn;
        if (!MoneyRange(pContractData->contractCoinsIn) || !MoneyRange(nValueOut))
            throw std::runtime_error(std::string(__func__) + ": value out of range");

    }
    return nValueOut;
}

unsigned int MCTransaction::GetTotalSize() const
{
    return ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
}

std::string MCTransaction::ToString() const
{
	std::string str;
	str += strprintf("MCTransaction(hash=%s, ver=%d, vin.size=%u, vout.size=%u, nLockTime=%u)\n",
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

MCBranchBlockInfo::MCBranchBlockInfo()
{
    SetNull();
}

MCBranchBlockInfo::MCBranchBlockInfo(const MCBranchBlockInfo& r)
    : nVersion(r.nVersion), hashPrevBlock(r.hashPrevBlock), hashMerkleRoot(r.hashMerkleRoot),
    hashMerkleRootWithPrevData(r.hashMerkleRootWithPrevData), hashMerkleRootWithData(r.hashMerkleRootWithData),
    nTime(r.nTime), nBits(r.nBits), nNonce(r.nNonce),
    prevoutStake(r.prevoutStake), vchBlockSig(r.vchBlockSig),
    branchID(r.branchID), blockHeight(r.blockHeight), vchStakeTxData(r.vchStakeTxData)
{
}

void MCBranchBlockInfo::SetNull()
{
    nVersion = 0;
    hashPrevBlock.SetNull();
    hashMerkleRoot.SetNull();
    hashMerkleRootWithPrevData.SetNull();
    hashMerkleRootWithData.SetNull();
    nTime = 0;
    nBits = 0;
    nNonce = 0;

    vchBlockSig.clear();
    prevoutStake.SetNull();

    branchID.SetNull();
	blockHeight = 0;
    vchStakeTxData.clear();
}

void MCBranchBlockInfo::GetBlockHeader(MCBlockHeader& block) const
{
    block.nVersion = nVersion;
    block.hashPrevBlock = hashPrevBlock;
    block.hashMerkleRoot = hashMerkleRoot;
    block.hashMerkleRootWithPrevData = hashMerkleRootWithPrevData;
    block.hashMerkleRootWithData = hashMerkleRootWithData;
    block.nTime = nTime;
    block.nBits = nBits;
    block.nNonce = nNonce;
    block.prevoutStake = prevoutStake;
    block.vchBlockSig = vchBlockSig;
}

void MCBranchBlockInfo::SetBlockHeader(const MCBlockHeader& block)
{
    nVersion = block.nVersion;
    hashPrevBlock = block.hashPrevBlock;
    hashMerkleRoot = block.hashMerkleRoot;
    hashMerkleRootWithPrevData = block.hashMerkleRootWithPrevData;
    hashMerkleRootWithData = block.hashMerkleRootWithData;
    nTime = block.nTime;
    nBits = block.nBits;
    nNonce = block.nNonce;
    prevoutStake = block.prevoutStake;
    vchBlockSig = block.vchBlockSig;
}
