// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_PRIMITIVES_TRANSACTION_H
#define CELLLINK_PRIMITIVES_TRANSACTION_H

#include <stdint.h>
#include "misc/amount.h"
#include "script/script.h"
#include "io/serialize.h"
#include "coding/uint256.h"
#include "key/pubkey.h"
#include "utils/utilstrencodings.h"
#include "transaction/partialmerkletree.h"
#include "chain/chainparamsbase.h"

static const int SERIALIZE_TRANSACTION_NO_WITNESS = 0x40000000;
/** An outpoint - a combination of a transaction hash and an index n into its vout */
class CellOutPoint
{
public:
    uint256 hash;
    uint32_t n;

    CellOutPoint(): n((uint32_t) -1) { }
    CellOutPoint(const uint256& hashIn, uint32_t nIn): hash(hashIn), n(nIn) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(n);
    }

    void SetNull() { hash.SetNull(); n = (uint32_t) -1; }
    bool IsNull() const { return (hash.IsNull() && n == (uint32_t) -1); }

    friend bool operator<(const CellOutPoint& a, const CellOutPoint& b)
    {
        int cmp = a.hash.Compare(b.hash);
        return cmp < 0 || (cmp == 0 && a.n < b.n);
    }

    friend bool operator==(const CellOutPoint& a, const CellOutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const CellOutPoint& a, const CellOutPoint& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CellTxIn
{
public:
    CellOutPoint prevout;
    CellScript scriptSig;
    uint32_t nSequence;
    CScriptWitness scriptWitness; //! Only serialized through CellTransaction

    /* Setting nSequence to this value for every input in a transaction
     * disables nLockTime. */
    static const uint32_t SEQUENCE_FINAL = 0xffffffff;

    /* Below flags apply in the context of BIP 68*/
    /* If this flag set, CellTxIn::nSequence is NOT interpreted as a
     * relative lock-time. */
    static const uint32_t SEQUENCE_LOCKTIME_DISABLE_FLAG = (1 << 31);

    /* If CellTxIn::nSequence encodes a relative lock-time and this flag
     * is set, the relative lock-time has units of 512 seconds,
     * otherwise it specifies blocks with a granularity of 1. */
    static const uint32_t SEQUENCE_LOCKTIME_TYPE_FLAG = (1 << 22);

    /* If CellTxIn::nSequence encodes a relative lock-time, this mask is
     * applied to extract that lock-time from the sequence field. */
    static const uint32_t SEQUENCE_LOCKTIME_MASK = 0x0000ffff;

    /* In order to use the same number of bits to encode roughly the
     * same wall-clock duration, and because blocks are naturally
     * limited to occur every 600s on average, the minimum granularity
     * for time-based relative lock-time is fixed at 512 seconds.
     * Converting from CellTxIn::nSequence to seconds is performed by
     * multiplying by 512 = 2^9, or equivalently shifting up by
     * 9 bits. */
    static const int SEQUENCE_LOCKTIME_GRANULARITY = 9;

    CellTxIn()
    {
        nSequence = SEQUENCE_FINAL;
    }

    explicit CellTxIn(CellOutPoint prevoutIn, CellScript scriptSigIn=CellScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);
    CellTxIn(uint256 hashPrevTx, uint32_t nOut, CellScript scriptSigIn=CellScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(prevout);
        READWRITE(scriptSig);
        READWRITE(nSequence);
    }

    friend bool operator==(const CellTxIn& a, const CellTxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const CellTxIn& a, const CellTxIn& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CellTxOut
{
public:
    CellAmount nValue;
    CellScript scriptPubKey;

    CellTxOut()
    {
        SetNull();
    }

    CellTxOut(const CellAmount& nValueIn, CellScript scriptPubKeyIn );

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(nValue);
        READWRITE(scriptPubKey);
    }

    void SetNull()
    {
        nValue = -1;
        scriptPubKey.clear();
    }

    bool IsNull() const
    {
        return (nValue == -1);
    }

    friend bool operator==(const CellTxOut& a, const CellTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const CellTxOut& a, const CellTxOut& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

struct CellMutableTransaction;

class CellBlockHeader;
class CellBranchBlockInfo
{
public:
    CellBranchBlockInfo();
    CellBranchBlockInfo(const CellBranchBlockInfo&);

    void SetNull();

    //block header data 	//不能直接include block.h，包含CBlockHeader成员，因为会导致头文件循环include error
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce; // this value in bitcion are added for make different hash, we use to indicate the amount of miner's address

    CellOutPoint prevoutStake;
    CellScript vchBlockSig;
    //additional data
    uint256 branchID;
    uint32_t blockHeight;
    std::vector<unsigned char> vchStakeTxData;// block.vtx[1] stake transaction serialize data

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);

        READWRITE(prevoutStake);
        READWRITE(vchBlockSig);
        READWRITE(branchID);
        READWRITE(blockHeight);
        READWRITE(vchStakeTxData);
    }

    void GetBlockHeader(CellBlockHeader& block) const;
    void SetBlockHeader(const CellBlockHeader& block);
};

class CellSpvProof
{
public:
    CellSpvProof() = default;
    CellSpvProof(const CellSpvProof& r) = default;
    CellSpvProof(const std::vector<uint256> &vTxid, const std::vector<bool> &vMatch, const uint256& bh) :blockhash(bh), pmt(vTxid, vMatch) {}

    uint256 blockhash;
    CellPartialMerkleTree pmt;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(blockhash);
        READWRITE(pmt);
    }
};

enum ReportType{
    REPORT_TX = 1,
    REPORT_COINBASE,
    REPORT_MERKLETREE,
};

class ReportData
{
public:
    ReportData() = default;
    int32_t reporttype;
    uint256 reportedBranchId;
    uint256 reportedBlockHash;
    uint256 reportedTxHash;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(reporttype);
        READWRITE(reportedBranchId);
        READWRITE(reportedBlockHash);
        READWRITE(reportedTxHash);
    }
};

class ProveDataItem
{
public:
    uint256 blockHash;
    std::vector<unsigned char> tx;
    CellSpvProof pCSP;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(blockHash);
        READWRITE(tx);
        READWRITE(pCSP);
    }
};

class ProveData
{
public:
    int32_t provetype;
    uint256 branchId;
    uint256 blockHash;
    uint256 txHash;// to prove txid
    std::vector<ProveDataItem> vectProveData;

    std::vector<unsigned char> vtxData; // block vtx serialize data, service for coinbase prove n merkle prove
    std::vector<std::vector<ProveDataItem>> vecBlockTxProve;// all block tx(exclude coinbase)'s prove data, use to prove each input of each tx in vtx is exist.  

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action){
        READWRITE(provetype);
        READWRITE(branchId);
        READWRITE(blockHash);
        READWRITE(txHash);
        if (provetype == ReportType::REPORT_TX){
            READWRITE(vectProveData);
        }
        else if (provetype == ReportType::REPORT_COINBASE) {
            READWRITE(vtxData);
            READWRITE(vecBlockTxProve);
        }
        else if (provetype == ReportType::REPORT_MERKLETREE) {
            READWRITE(vtxData);
            READWRITE(vecBlockTxProve);
        }
    }
};

/**
 * Basic transaction serialization format:
 * - int32_t nVersion
 * - std::vector<CellTxIn> vin
 * - std::vector<CellTxOut> vout
 * - uint32_t nLockTime
 *
 * Extended transaction serialization format:
 * - int32_t nVersion
 * - unsigned char dummy = 0x00
 * - unsigned char flags (!= 0)
 * - std::vector<CellTxIn> vin
 * - std::vector<CellTxOut> vout
 * - if (flags & 1):
 *   - CTxWitness wit;
 * - uint32_t nLockTime
 */
template<typename Stream, typename TxType>
inline void UnserializeTransaction(TxType& tx, Stream& s) {
    const bool fAllowWitness = !(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

    s >> tx.nVersion;
    unsigned char flags = 0;
    tx.vin.clear();
    tx.vout.clear();
    /* Try to read the vin. In case the dummy is there, this will be read as an empty vector. */
    s >> tx.vin;
    if (tx.vin.size() == 0 && fAllowWitness) {
        /* We read a dummy or an empty vin. */
        s >> flags;
        if (flags != 0) {
            s >> tx.vin;
            s >> tx.vout;
        }
    } else {
        /* We read a non-empty vin. Assume a normal vout follows. */
        s >> tx.vout;
    }
    if ((flags & 1) && fAllowWitness) {
        /* The witness flag is present, and we support witnesses. */
        flags ^= 1;
        for (size_t i = 0; i < tx.vin.size(); i++) {
            s >> tx.vin[i].scriptWitness.stack;
        }
    }
    if (flags) {
        /* Unknown flag in the serialization */
        throw std::ios_base::failure("Unknown transaction optional data");
    }
    s >> tx.nLockTime;

    if (tx.nVersion == 3) { //CellTransaction::PUBLISH_CONTRACT_VERSION
        s >> tx.contractAddrs;
		s >> tx.contractCode;
		s >> tx.contractSender;
		s >> tx.scontractScriptSig;
	}
    else if (tx.nVersion == 4) {//CellTransaction::CALL_CONTRACT_VERSION
        s >> tx.contractAddrs;
		s >> tx.contractSender;
		s >> tx.contractFun;
		s >> tx.contractParams;
		s >> tx.scontractScriptSig;
	}
	else if (tx.nVersion == 5) {//CellTransaction::CREATE_BRANCH_VERSION
		s >> tx.branchVSeeds;
		s >> tx.branchSeedSpec6;
	}
	else if (tx.nVersion == 6) {//CellTransaction::TRANS_BRANCH_VERSION_S1
		s >> tx.sendToBranchid;
		s >> tx.sendToTxHexData;
        if (tx.sendToBranchid == "main") {
            tx.pPMT.reset(new CellSpvProof());
            s >> *tx.pPMT;
        }
	}
	else if (tx.nVersion == 7){//CellTransaction::TRANS_BRANCH_VERSION_S2
		s >> tx.fromBranchId;
		s >> tx.fromTx;
		s >> tx.inAmount;
        if (tx.fromBranchId != "main") {
            tx.pPMT.reset(new CellSpvProof());
            s >> *tx.pPMT;
        }
	}
	else if (tx.nVersion == 8){ //CellTransaction::MINE_BRANCH_MORTGAGE
        s >> tx.sendToBranchid;
        s >> tx.sendToTxHexData;
    }
    else if (tx.nVersion == 9) {//CellTransaction::SYNC_BRANCH_INFO
        tx.pBranchBlockData.reset(new CellBranchBlockInfo);
        s >> *tx.pBranchBlockData;
    }
    else if (tx.nVersion == 10){//CellTransaction::REPORT_CHEAT
        tx.pReportData.reset(new ReportData);
        s >> *tx.pReportData;
        tx.pPMT.reset(new CellSpvProof());
        s >> *tx.pPMT;
    }
    else if (tx.nVersion == 11) {//CellTransaction::PROVE
        tx.pProveData.reset(new ProveData);
        s >> *tx.pProveData;
    }
    else if (tx.nVersion == 13) {//CellTransaction::REDEEM_MORTGAGE
        s >> tx.fromBranchId;
        s >> tx.fromTx;
        tx.pPMT.reset(new CellSpvProof());
        s >> *tx.pPMT;
    }
    else if (tx.nVersion == 15) {//CellTransaction::REPORT_REWARD
        s >> tx.reporttxid;
    }
    else if (tx.nVersion == 16) {//CellTransaction::LOCK_MORTGAGE_MINE_COIN
        s >> tx.reporttxid;
        s >> tx.coinpreouthash;
    }
    else if (tx.nVersion == 17) {//CellTransaction::UNLOCK_MORTGAGE_MINE_COIN
        s >> tx.reporttxid;
        s >> tx.coinpreouthash;
        s >> tx.provetxid;
    }
}

template<typename Stream, typename TxType>
inline void SerializeTransaction(const TxType& tx, Stream& s) {
    const bool fAllowWitness = !(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

    s << tx.nVersion;
    unsigned char flags = 0;
    // Consistency check
    if (fAllowWitness) {
        /* Check whether witnesses need to be serialized. */
        if (tx.HasWitness()) {
            flags |= 1;
        }
    }
    if (flags) {
        /* Use extended format in case witnesses are to be serialized. */
        std::vector<CellTxIn> vinDummy;
        s << vinDummy;
        s << flags;
    }
    s << tx.vin;
    s << tx.vout;
    if (flags & 1) {
        for (size_t i = 0; i < tx.vin.size(); i++) {
            s << tx.vin[i].scriptWitness.stack;
        }
    }
    s << tx.nLockTime;

	if (tx.nVersion == 3) {//CellTransaction::PUBLISH_CONTRACT_VERSION
		s << tx.contractAddrs;
		s << tx.contractCode;
		s << tx.contractSender;
		s << tx.scontractScriptSig;
	}
	else if (tx.nVersion == 4) {//CellTransaction::CALL_CONTRACT_VERSION
		s << tx.contractAddrs;
		s << tx.contractSender;
		s << tx.contractFun;
		s << tx.contractParams;
		s << tx.scontractScriptSig;
	}
	else if (tx.nVersion == 5) {//CellTransaction::CREATE_BRANCH_VERSION
		s << tx.branchVSeeds;
		s << tx.branchSeedSpec6;
	}
	else if (tx.nVersion == 6) {//CellTransaction::TRANS_BRANCH_VERSION_S1
		s << tx.sendToBranchid;
		s << tx.sendToTxHexData;
        if (tx.sendToBranchid == "main") {
            s << *tx.pPMT;
        }
	}
	else if (tx.nVersion == 7) {//CellTransaction::TRANS_BRANCH_VERSION_S2
		s << tx.fromBranchId;
		s << tx.fromTx;
		s << tx.inAmount;
        if (tx.fromBranchId != "main") {
            s << *tx.pPMT;
        }
	}
	else if (tx.nVersion == 8) { //CellTransaction::MINE_BRANCH_MORTGAGE
        s << tx.sendToBranchid;
        s << tx.sendToTxHexData;
    }
    else if (tx.nVersion == 9) {//CellTransaction::SYNC_BRANCH_INFO
        s << *tx.pBranchBlockData;
    }
    else if (tx.nVersion == 10) {//CellTransaction::REPORT_CHEAT
        s << *tx.pReportData;
        s << *tx.pPMT;
    }
    else if (tx.nVersion == 11) {//CellTransaction::PROVE
        s << *tx.pProveData;
    }
    else if (tx.nVersion == 13) {//CellTransaction::REDEEM_MORTGAGE
        s << tx.fromBranchId;
        s << tx.fromTx;
        s << *tx.pPMT;
    }
    else if (tx.nVersion == 15) {//CellTransaction::REPORT_REWARD
        s << tx.reporttxid;
    }
    else if (tx.nVersion == 16) {//CellTransaction::LOCK_MORTGAGE_MINE_COIN
        s << tx.reporttxid;
        s << tx.coinpreouthash;
    }
    else if (tx.nVersion == 17) {//CellTransaction::UNLOCK_MORTGAGE_MINE_COIN
        s << tx.reporttxid;
        s << tx.coinpreouthash;
        s << tx.provetxid;
    }
}

/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
class CellTransaction
{
public:
    // Default transaction version.
    static const int32_t CURRENT_VERSION=2;
	static const int32_t PUBLISH_CONTRACT_VERSION = 3;
	static const int32_t CALL_CONTRACT_VERSION = 4; 
    static const int32_t CREATE_BRANCH_VERSION = 5;// 创建支链 
	static const int32_t TRANS_BRANCH_VERSION_S1 = 6;// 跨链交易的发起链方 
	static const int32_t TRANS_BRANCH_VERSION_S2 = 7;// 跨链交易的接收链方 

    static const int32_t MINE_BRANCH_MORTGAGE = 8; // 挖矿抵押,like TRANS_BRANCH_VERSION_S1
    static const int32_t SYNC_BRANCH_INFO = 9;     // 同步侧链数据,给主链
    static const int32_t REPORT_CHEAT = 10;        // 举报
    static const int32_t PROVE = 11;               // 证明
    static const int32_t REDEEM_MORTGAGE_STATEMENT = 12;     // 赎回挖矿抵押(在侧链声明阶段,把挖矿币销毁)
    static const int32_t REDEEM_MORTGAGE = 13;               // 赎回挖矿抵押币(把主链上的抵押拿回来)

    static const int32_t STAKE = 14; // mining stake transaction
    static const int32_t REPORT_REWARD = 15; // 举报奖励
    static const int32_t LOCK_MORTGAGE_MINE_COIN = 16; // 锁定挖矿币
    static const int32_t UNLOCK_MORTGAGE_MINE_COIN = 17; // 解锁挖矿币

    // Changing the default transaction version requires a two step process: first
    // adapting relay policy by bumping MAX_STANDARD_VERSION, and then later date
    // bumping the default CURRENT_VERSION at which point both CURRENT_VERSION and
    // MAX_STANDARD_VERSION will be equal.
    static const int32_t MAX_STANDARD_VERSION=17;

    // The local variables are made const to prevent unintended modification
    // without updating the cached hash value. However, CellTransaction is not
    // actually immutable; deserialization and assignment are implemented,
    // and bypass the constness. This is safe, as they update the entire
    // structure, including the hash.
    const int32_t nVersion;
    const std::vector<CellTxIn> vin;
    const std::vector<CellTxOut> vout;
    const uint32_t nLockTime;

	//contract data
	const std::string contractCode;
	const CellPubKey contractSender;// sender pub addr
	const std::string contractFun;
	const std::string contractParams;
    const CellScript scontractScriptSig;
    const std::vector<CellContractID> contractAddrs;
    const CellAmount contractAmountIn;    // 向合约转入
    const CellAmount contractAmountOut;   // 向合约转出

	//branch create data
	const std::string branchVSeeds;
	const std::string branchSeedSpec6;
	//branch trans start (step 1)
	const std::string sendToBranchid;
	const std::string sendToTxHexData;	//在跨链交易中有用到 目标链交易的数据
	//branch trans end (step 2)
	const std::string fromBranchId;
	const std::vector<unsigned char> fromTx;
	const uint64_t inAmount;
	
	const std::shared_ptr<const CellBranchBlockInfo> pBranchBlockData;
    const std::shared_ptr<const CellSpvProof> pPMT;
    const std::shared_ptr<const ReportData> pReportData;
    const std::shared_ptr<const ProveData> pProveData;

    const uint256 reporttxid;// 从reported tx获取block coin，是否被证明等信息
    const uint256 coinpreouthash; // coin preout hash
    const uint256 provetxid; // 证明txid

	bool IsExistVin(const CellTxIn &txIn) const {
		auto it = find(vin.begin(), vin.end(), txIn);
		if (it == vin.end())
		{
			return false;
		}
		return true;
	}
	bool IsExistVout(const CellTxOut &txOut) const {
		auto it = find(vout.begin(), vout.end(), txOut);
		if (it == vout.end())
		{
			return false;
		}
		return true;
	}
private:
    /** Memory only. */
    const uint256 hash;

    uint256 ComputeHash() const;

public:
    /** Construct a CellTransaction that qualifies as IsNull() */
    CellTransaction();
	CellTransaction(const CellTransaction& tx);
    /** Convert a CellMutableTransaction into a CellTransaction. */
    CellTransaction(const CellMutableTransaction &tx);
    CellTransaction(CellMutableTransaction &&tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeTransaction(*this, s);
    }

    /** This deserializing constructor is provided instead of an Unserialize method.
     *  Unserialize is not possible, since it would require overwriting const fields. */
    template <typename Stream>
    CellTransaction(deserialize_type, Stream& s) : CellTransaction(CellMutableTransaction(deserialize, s)) {}

    bool IsNull() const {
        return vin.empty() && vout.empty();
    }

    const uint256& GetHash() const {
        return hash;
    }

    // Compute a hash that includes both transaction and witness data
    uint256 GetWitnessHash() const;

    // Return sum of txouts.
    CellAmount GetValueOut() const;
    // GetValueIn() is a method on CellCoinsViewCache, because
    // inputs must be known to compute value in.

    /**
     * Get the total transaction size in bytes, including witness data.
     * "Total Size" defined in BIP141 and BIP144.
     * @return Total transaction size in bytes
     */
    unsigned int GetTotalSize() const;

    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull() && (nVersion <= CURRENT_VERSION));
    }
	
    bool IsSmartContract() const {
        return nVersion == PUBLISH_CONTRACT_VERSION || nVersion == CALL_CONTRACT_VERSION;
    }

    bool IsSyncBranchInfo() const
    {
        return nVersion == SYNC_BRANCH_INFO;
    }

    //dose it have a child transaction in sendToTxHexData
    bool IsPregnantTx() const
    {
        return IsBranchChainTransStep1() || IsMortgage();
    }

	bool IsBranchCreate() const
	{
		return nVersion == CREATE_BRANCH_VERSION && !branchVSeeds.empty();
	}

	bool IsBranchChainTransStep1() const
	{
		return nVersion == TRANS_BRANCH_VERSION_S1 && !sendToBranchid.empty() && !sendToTxHexData.empty();
	}

	bool IsBranchChainTransStep2() const
	{
		return nVersion == TRANS_BRANCH_VERSION_S2 && inAmount > 0;
	}

	bool IsMortgage() const
	{
		return nVersion == MINE_BRANCH_MORTGAGE;
	}

    bool IsRedeemMortgageStatement() const
    {
        return nVersion == REDEEM_MORTGAGE_STATEMENT;
    }
    bool IsRedeemMortgage() const
    {
        return nVersion == REDEEM_MORTGAGE;
    }

    bool IsReport() const
    {
        return nVersion == CellTransaction::REPORT_CHEAT;
    }

    bool IsProve() const
    {
        return nVersion == CellTransaction::PROVE;
    }

    bool IsStake() const {
        return nVersion == STAKE;
    }

    bool IsReportReward() const {
        return nVersion == REPORT_REWARD;
    }

    bool IsLockMortgageMineCoin() const {
        return nVersion == LOCK_MORTGAGE_MINE_COIN;
    }
    bool IsUnLockMortgageMineCoin() const {
        return nVersion == UNLOCK_MORTGAGE_MINE_COIN;
    }

    friend bool operator==(const CellTransaction& a, const CellTransaction& b)
    {
        return a.hash == b.hash;
    }

    friend bool operator!=(const CellTransaction& a, const CellTransaction& b)
    {
        return a.hash != b.hash;
    }

    std::string ToString() const;

    bool HasWitness() const
    {
        for (size_t i = 0; i < vin.size(); i++) {
            if (!vin[i].scriptWitness.IsNull()) {
                return true;
            }
        }
        return false;
    }
};


/** A mutable version of CellTransaction. */
struct CellMutableTransaction
{
    int32_t nVersion;
    std::vector<CellTxIn> vin;
    std::vector<CellTxOut> vout;
    uint32_t nLockTime;

	//contract data
	std::string contractCode;
	CellPubKey contractSender;
	std::string contractFun;
	std::string contractParams;
	CellScript scontractScriptSig;
    std::vector<CellContractID> contractAddrs;
    CellAmount contractAmountIn = 0;    // 向合约转入
    CellAmount contractAmountOut = 0;   // 向合约转出

	//branch create data
	std::string branchVSeeds;
	std::string branchSeedSpec6;
	//branch trans start (step 1)
	std::string sendToBranchid;
	std::string sendToTxHexData;
	//branch trans end (step 2)
	std::string fromBranchId;
    std::vector<unsigned char> fromTx;
	uint64_t inAmount;

	std::shared_ptr<CellBranchBlockInfo> pBranchBlockData;
    std::shared_ptr<CellSpvProof> pPMT;

    std::shared_ptr<ReportData> pReportData;
    std::shared_ptr<ProveData> pProveData;

    uint256 reporttxid;
    uint256 coinpreouthash;
    uint256 provetxid;

    CellMutableTransaction();
	CellMutableTransaction(const CellMutableTransaction&);
    CellMutableTransaction(const CellTransaction& tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeTransaction(*this, s);
    }


    template <typename Stream>
    inline void Unserialize(Stream& s) {
        UnserializeTransaction(*this, s);
    }

    template <typename Stream>
    CellMutableTransaction(deserialize_type, Stream& s) {
        Unserialize(s);
    }

    /** Compute the hash of this CellMutableTransaction. This is computed on the
     * fly, as opposed to GetHash() in CellTransaction, which uses a cached result.
     */
    uint256 GetHash() const;

    bool IsSmartContract() const {
        return nVersion == CellTransaction::PUBLISH_CONTRACT_VERSION || nVersion == CellTransaction::CALL_CONTRACT_VERSION;
    }

	//dose it have a child transaction in sendToTxHexData
    bool IsPregnantTx() const
    {
        return IsBranchChainTransStep1() || IsMortgage();
    }

	bool IsBranchCreate() const
	{
		return nVersion == CellTransaction::CREATE_BRANCH_VERSION && !branchVSeeds.empty();
	}

	bool IsBranchChainTransStep1() const
	{
		return nVersion == CellTransaction::TRANS_BRANCH_VERSION_S1 && !sendToBranchid.empty() && !sendToTxHexData.empty();
	}

	bool IsBranchChainTransStep2() const
	{
		return nVersion == CellTransaction::TRANS_BRANCH_VERSION_S2 && inAmount > 0;
	}

	bool IsMortgage() const
	{
		return nVersion == CellTransaction::MINE_BRANCH_MORTGAGE;
	}

    bool IsRedeemMortgageStatement() const
    {
        return nVersion == CellTransaction::REDEEM_MORTGAGE_STATEMENT;
    }
    bool IsRedeemMortgage() const
    {
        return nVersion == CellTransaction::REDEEM_MORTGAGE;
    }

    bool IsSyncBranchInfo() const
    {
        return nVersion == CellTransaction::SYNC_BRANCH_INFO;
    }

    bool IsReport() const
    {
        return nVersion == CellTransaction::REPORT_CHEAT;
    }

    bool IsProve() const
    {
        return nVersion == CellTransaction::PROVE;
    }

    bool IsReportReward() const {
        return nVersion == CellTransaction::REPORT_REWARD;
    }

    bool IsLockMortgageMineCoin() const {
        return nVersion == CellTransaction::LOCK_MORTGAGE_MINE_COIN;
    }
    bool IsUnLockMortgageMineCoin() const {
        return nVersion == CellTransaction::UNLOCK_MORTGAGE_MINE_COIN;
    }
    friend bool operator==(const CellMutableTransaction& a, const CellMutableTransaction& b)
    {
        return a.GetHash() == b.GetHash();
    }

    bool HasWitness() const
    {
        for (size_t i = 0; i < vin.size(); i++) {
            if (!vin[i].scriptWitness.IsNull()) {
                return true;
            }
        }
        return false;
    }
	CellMutableTransaction& operator=(const CellMutableTransaction& tx);
};

typedef std::shared_ptr<const CellTransaction> CellTransactionRef;
static inline CellTransactionRef MakeTransactionRef() { return std::make_shared<const CellTransaction>(); }
template <typename Tx> static inline CellTransactionRef MakeTransactionRef(Tx&& txIn) { return std::make_shared<const CellTransaction>(std::forward<Tx>(txIn)); }

inline CellTransaction::CellTransaction() : nVersion(CellTransaction::CURRENT_VERSION), vin(), vout(), nLockTime(0),
    contractAddrs(), contractCode(), contractSender(), contractFun(), contractParams(), scontractScriptSig(), contractAmountIn(0), contractAmountOut(0),
    branchVSeeds(), branchSeedSpec6(), sendToBranchid(), sendToTxHexData(),
    fromBranchId(), fromTx(), inAmount(0), pBranchBlockData(), pPMT(), pReportData(), pProveData(), reporttxid(), coinpreouthash(), provetxid(), hash(){}
inline CellTransaction::CellTransaction(const CellMutableTransaction& tx) : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime),
    contractAddrs(tx.contractAddrs), contractCode(tx.contractCode), contractSender(tx.contractSender), contractFun(tx.contractFun), contractParams(tx.contractParams),
    scontractScriptSig(tx.scontractScriptSig), contractAmountIn(tx.contractAmountIn), contractAmountOut(tx.contractAmountOut),
    branchVSeeds(tx.branchVSeeds), branchSeedSpec6(tx.branchSeedSpec6), sendToBranchid(tx.sendToBranchid), sendToTxHexData(tx.sendToTxHexData),
    fromBranchId(tx.fromBranchId), fromTx(tx.fromTx), inAmount(tx.inAmount),
    pBranchBlockData(tx.pBranchBlockData == nullptr ? nullptr : new CellBranchBlockInfo(*tx.pBranchBlockData)), 
    pPMT(tx.pPMT == nullptr? nullptr: new CellSpvProof(*tx.pPMT)),
    pReportData(tx.pReportData == nullptr ? nullptr : new ReportData(*tx.pReportData)),
    pProveData(tx.pProveData == nullptr?nullptr:new ProveData(*tx.pProveData)), reporttxid(tx.reporttxid), coinpreouthash(tx.coinpreouthash), provetxid(tx.provetxid), hash(ComputeHash()) {}
inline CellTransaction::CellTransaction(CellMutableTransaction&& tx) : nVersion(tx.nVersion), vin(std::move(tx.vin)), vout(std::move(tx.vout)), nLockTime(tx.nLockTime),
    contractAddrs(std::move(tx.contractAddrs)), contractCode(std::move(tx.contractCode)), contractSender(std::move(tx.contractSender)), contractFun(std::move(tx.contractFun)), contractParams(std::move(tx.contractParams)),
    scontractScriptSig(tx.scontractScriptSig), contractAmountIn(tx.contractAmountIn), contractAmountOut(tx.contractAmountOut),
    branchVSeeds(std::move(tx.branchVSeeds)), branchSeedSpec6(std::move(tx.branchSeedSpec6)), sendToBranchid(std::move(tx.sendToBranchid)), sendToTxHexData(tx.sendToTxHexData),
    fromBranchId(std::move(tx.fromBranchId)), fromTx(std::move(tx.fromTx)), inAmount(tx.inAmount),
    pBranchBlockData(std::move(tx.pBranchBlockData)), pPMT(std::move(tx.pPMT)), 
    pReportData(std::move(tx.pReportData)), pProveData(std::move(tx.pProveData)),
    reporttxid(std::move(tx.reporttxid)), coinpreouthash(std::move(tx.coinpreouthash)), provetxid(std::move(tx.provetxid)), hash(ComputeHash()) {}

// add copy constructor, 添加了不可复制成员变量pBranchBlockData后，默认复制构造函数被删除了
inline CellTransaction::CellTransaction(const CellTransaction& tx)
    : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime),
      contractAddrs(tx.contractAddrs), contractCode(tx.contractCode), contractSender(tx.contractSender), contractFun(tx.contractFun), contractParams(tx.contractParams),
      scontractScriptSig(tx.scontractScriptSig), contractAmountIn(tx.contractAmountIn), contractAmountOut(tx.contractAmountOut),
      branchVSeeds(tx.branchVSeeds), branchSeedSpec6(tx.branchSeedSpec6), sendToBranchid(tx.sendToBranchid), sendToTxHexData(tx.sendToTxHexData),
      fromBranchId(tx.fromBranchId), fromTx(tx.fromTx), inAmount(tx.inAmount),
      pBranchBlockData(tx.pBranchBlockData == nullptr ? nullptr : new CellBranchBlockInfo(*tx.pBranchBlockData)), 
      pPMT(tx.pPMT == nullptr ? nullptr : new CellSpvProof(*tx.pPMT)), 
      pReportData(tx.pReportData == nullptr ? nullptr : new ReportData(*tx.pReportData)),
    pProveData(tx.pProveData == nullptr?nullptr:new ProveData(*tx.pProveData)), reporttxid(tx.reporttxid), coinpreouthash(tx.coinpreouthash), provetxid(tx.provetxid), hash(ComputeHash()) {}

inline CellMutableTransaction::CellMutableTransaction(const CellMutableTransaction& tx)
    : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime),
    contractAddrs(tx.contractAddrs), contractCode(tx.contractCode), contractSender(tx.contractSender), contractFun(tx.contractFun), contractParams(tx.contractParams),
      scontractScriptSig(tx.scontractScriptSig), contractAmountIn(tx.contractAmountIn), contractAmountOut(tx.contractAmountOut),
      branchVSeeds(tx.branchVSeeds), branchSeedSpec6(tx.branchSeedSpec6), sendToBranchid(tx.sendToBranchid), sendToTxHexData(tx.sendToTxHexData),
      fromBranchId(tx.fromBranchId), fromTx(tx.fromTx), inAmount(tx.inAmount),
      pBranchBlockData(tx.pBranchBlockData == nullptr ? nullptr : new CellBranchBlockInfo(*tx.pBranchBlockData)),
      pPMT(tx.pPMT == nullptr ? nullptr : new CellSpvProof(*tx.pPMT)), 
      pReportData(tx.pReportData == nullptr ? nullptr : new ReportData(*tx.pReportData)),
    pProveData(tx.pProveData==nullptr?nullptr:new ProveData(*tx.pProveData)), reporttxid(tx.reporttxid), coinpreouthash(tx.coinpreouthash), provetxid(tx.provetxid){}

inline CellMutableTransaction& CellMutableTransaction::operator=(const CellMutableTransaction& tx)
{
    if (&tx == this)
        return *this;
    
    nVersion = (tx.nVersion);
    vin = (tx.vin);
    vout = (tx.vout);
    nLockTime = (tx.nLockTime);
    contractAddrs = (tx.contractAddrs);
    contractCode = (tx.contractCode);
    contractSender = (tx.contractSender);
    contractFun = (tx.contractFun);
    contractParams = (tx.contractParams);
    contractAmountIn = (tx.contractAmountIn);
    contractAmountOut = (tx.contractAmountOut);
    scontractScriptSig = (tx.scontractScriptSig);
    branchVSeeds = (tx.branchVSeeds);
    branchSeedSpec6 = (tx.branchSeedSpec6);
    sendToBranchid = (tx.sendToBranchid);
    sendToTxHexData = (tx.sendToTxHexData);
    fromBranchId = (tx.fromBranchId);
    fromTx = (tx.fromTx);
    inAmount = (tx.inAmount);
    pBranchBlockData.reset(tx.pBranchBlockData == nullptr ? nullptr : new CellBranchBlockInfo(*tx.pBranchBlockData));
    pPMT.reset(tx.pPMT == nullptr ? nullptr : new CellSpvProof(*tx.pPMT));
    pReportData.reset(tx.pReportData == nullptr ? nullptr : new ReportData(*tx.pReportData));
    pProveData.reset(tx.pProveData == nullptr ? nullptr : new ProveData(*tx.pProveData));
    reporttxid = tx.reporttxid;
    coinpreouthash = tx.coinpreouthash;
    provetxid = tx.provetxid;
    return *this;
}

#endif // CELLLINK_PRIMITIVES_TRANSACTION_H
