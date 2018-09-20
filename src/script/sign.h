// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_SCRIPT_SIGN_H
#define CELLLINK_SCRIPT_SIGN_H

#include "script/interpreter.h"

class CellKeyID;
class CellKeyStore;
class CellScript;
class CellTransaction;

struct CellMutableTransaction;

/** Virtual base class for signature creators. */
class BaseSignatureCreator {
protected:
    const CellKeyStore* keystore;

public:
    BaseSignatureCreator(const CellKeyStore* keystoreIn) : keystore(keystoreIn) {}
    const CellKeyStore& KeyStore() const { return *keystore; };
    virtual ~BaseSignatureCreator() {}
    virtual const BaseSignatureChecker& Checker() const =0;

    /** Create a singular (non-script) signature. */
    virtual bool CreateSig(std::vector<unsigned char>& vchSig, const CellKeyID& keyid, const CellScript& scriptCode, SigVersion sigversion) const =0;
};

/** A signature creator for transactions. */
class TransactionSignatureCreator : public BaseSignatureCreator {
    const CellTransaction* txTo;
    unsigned int nIn;
    int nHashType;
    CellAmount amount;
    const TransactionSignatureChecker checker;

public:
    TransactionSignatureCreator(const CellKeyStore* keystoreIn, const CellTransaction* txToIn, unsigned int nInIn, const CellAmount& amountIn, int nHashTypeIn=SIGHASH_ALL);
    const BaseSignatureChecker& Checker() const override { return checker; }
    bool CreateSig(std::vector<unsigned char>& vchSig, const CellKeyID& keyid, const CellScript& scriptCode, SigVersion sigversion) const override;
};

class MutableTransactionSignatureCreator : public TransactionSignatureCreator {
    CellTransaction tx;

public:
    MutableTransactionSignatureCreator(const CellKeyStore* keystoreIn, const CellMutableTransaction* txToIn, unsigned int nInIn, const CellAmount& amountIn, int nHashTypeIn) : TransactionSignatureCreator(keystoreIn, &tx, nInIn, amountIn, nHashTypeIn), tx(*txToIn) {}
};

/** A signature creator that just produces 72-byte empty signatures. */
class DummySignatureCreator : public BaseSignatureCreator {
public:
    DummySignatureCreator(const CellKeyStore* keystoreIn) : BaseSignatureCreator(keystoreIn) {}
    const BaseSignatureChecker& Checker() const override;
    bool CreateSig(std::vector<unsigned char>& vchSig, const CellKeyID& keyid, const CellScript& scriptCode, SigVersion sigversion) const override;
};

struct SignatureData {
    CellScript scriptSig;
    CScriptWitness scriptWitness;

    SignatureData() {}
    explicit SignatureData(const CellScript& script) : scriptSig(script) {}
};

/** Produce a script signature using a generic signature creator. */
bool ProduceSignature(const BaseSignatureCreator& creator, const CellScript& scriptPubKey, SignatureData& sigdata);

/** Produce a script signature for a transaction. */
bool SignSignature(const CellKeyStore &keystore, const CellScript& fromPubKey, CellMutableTransaction& txTo, unsigned int nIn, const CellAmount& amount, int nHashType);
bool SignSignature(const CellKeyStore& keystore, const CellTransaction& txFrom, CellMutableTransaction& txTo, unsigned int nIn, int nHashType);

/** Combine two script signatures using a generic signature checker, intelligently, possibly with OP_0 placeholders. */
SignatureData CombineSignatures(const CellScript& scriptPubKey, const BaseSignatureChecker& checker, const SignatureData& scriptSig1, const SignatureData& scriptSig2);

/** Extract signature data from a transaction, and insert it. */
SignatureData DataFromTransaction(const CellMutableTransaction& tx, unsigned int nIn);
void UpdateTransaction(CellMutableTransaction& tx, unsigned int nIn, const SignatureData& data);

bool SignContract(const CellKeyStore* keystoreIn, const CellTransaction* txToIn, CellScript& constractSig);
bool CheckContractSign(const CellTransaction* txToIn, const CellScript& constractSig);

#endif // CELLLINK_SCRIPT_SIGN_H
