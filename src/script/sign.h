// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_SCRIPT_SIGN_H
#define MAGNACHAIN_SCRIPT_SIGN_H

#include "script/interpreter.h"

class MCKeyID;
class MCKeyStore;
class MCScript;
class MCTransaction;

struct MCMutableTransaction;

/** Virtual base class for signature creators. */
class BaseSignatureCreator {
protected:
    const MCKeyStore* keystore;

public:
    BaseSignatureCreator(const MCKeyStore* keystoreIn) : keystore(keystoreIn) {}
    const MCKeyStore& KeyStore() const { return *keystore; };
    virtual ~BaseSignatureCreator() {}
    virtual const BaseSignatureChecker& Checker() const =0;

    /** Create a singular (non-script) signature. */
    virtual bool CreateSig(std::vector<unsigned char>& vchSig, const MCKeyID& keyid, const MCScript& scriptCode, SigVersion sigversion) const =0;
};

/** A signature creator for transactions. */
class TransactionSignatureCreator : public BaseSignatureCreator {
    const MCTransaction* txTo;
    unsigned int nIn;
    int nHashType;
    MCAmount amount;
    const TransactionSignatureChecker checker;

public:
    TransactionSignatureCreator(const MCKeyStore* keystoreIn, const MCTransaction* txToIn, unsigned int nInIn, const MCAmount& amountIn, int nHashTypeIn=SIGHASH_ALL);
    const BaseSignatureChecker& Checker() const override { return checker; }
    bool CreateSig(std::vector<unsigned char>& vchSig, const MCKeyID& keyid, const MCScript& scriptCode, SigVersion sigversion) const override;
};

class MutableTransactionSignatureCreator : public TransactionSignatureCreator {
    MCTransaction tx;

public:
    MutableTransactionSignatureCreator(const MCKeyStore* keystoreIn, const MCMutableTransaction* txToIn, unsigned int nInIn, const MCAmount& amountIn, int nHashTypeIn) : TransactionSignatureCreator(keystoreIn, &tx, nInIn, amountIn, nHashTypeIn), tx(*txToIn) {}
};

/** A signature creator that just produces 72-byte empty signatures. */
class DummySignatureCreator : public BaseSignatureCreator {
public:
    DummySignatureCreator(const MCKeyStore* keystoreIn) : BaseSignatureCreator(keystoreIn) {}
    const BaseSignatureChecker& Checker() const override;
    bool CreateSig(std::vector<unsigned char>& vchSig, const MCKeyID& keyid, const MCScript& scriptCode, SigVersion sigversion) const override;
};

struct SignatureData {
    MCScript scriptSig;
    CScriptWitness scriptWitness;

    SignatureData() {}
    explicit SignatureData(const MCScript& script) : scriptSig(script) {}
};

/** Produce a script signature using a generic signature creator. */
bool ProduceSignature(const BaseSignatureCreator& creator, const MCScript& scriptPubKey, SignatureData& sigdata);

/** Produce a script signature for a transaction. */
bool SignSignature(const MCKeyStore &keystore, const MCScript& fromPubKey, MCMutableTransaction& txTo, unsigned int nIn, const MCAmount& amount, int nHashType);
bool SignSignature(const MCKeyStore& keystore, const MCTransaction& txFrom, MCMutableTransaction& txTo, unsigned int nIn, int nHashType);

/** Combine two script signatures using a generic signature checker, intelligently, possibly with OP_0 placeholders. */
SignatureData CombineSignatures(const MCScript& scriptPubKey, const BaseSignatureChecker& checker, const SignatureData& scriptSig1, const SignatureData& scriptSig2);

/** Extract signature data from a transaction, and insert it. */
SignatureData DataFromTransaction(const MCMutableTransaction& tx, unsigned int nIn);
void UpdateTransaction(MCMutableTransaction& tx, unsigned int nIn, const SignatureData& data);

bool SignContract(const MCKeyStore* keystoreIn, const MCTransaction* txToIn, MCScript& constractSig);
bool CheckContractSign(const MCTransaction* txToIn, const MCScript& constractSig);

#endif // MAGNACHAIN_SCRIPT_SIGN_H
