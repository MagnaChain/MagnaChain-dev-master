// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_VALIDATIONINTERFACE_H
#define CELLLINK_VALIDATIONINTERFACE_H

#include <memory>

#include "primitives/transaction.h" // MCTransaction(Ref)

class MCBlock;
class MCBlockIndex;
struct MCBlockLocator;
class MCBlockIndex;
class MCConnman;
class CReserveScript;
class MCValidationInterface;
class MCValidationState;
class uint256;
class MCScheduler;

// These functions dispatch to one or all registered wallets

/** Register a wallet to receive updates from core */
void RegisterValidationInterface(MCValidationInterface* pwalletIn);
/** Unregister a wallet from core */
void UnregisterValidationInterface(MCValidationInterface* pwalletIn);
/** Unregister all wallets from core */
void UnregisterAllValidationInterfaces();

class MCValidationInterface {
protected:
    /** Notifies listeners of updated block chain tip */
    virtual void UpdatedBlockTip(const MCBlockIndex *pindexNew, const MCBlockIndex *pindexFork, bool fInitialDownload) {}
    /** Notifies listeners of a transaction having been added to mempool. */
    virtual void TransactionAddedToMempool(const MCTransactionRef &ptxn) {}
    /**
     * Notifies listeners of a block being connected.
     * Provides a vector of transactions evicted from the mempool as a result.
     */
    virtual void BlockConnected(const std::shared_ptr<const MCBlock> &block, const MCBlockIndex *pindex, const std::vector<MCTransactionRef> &txnConflicted) {}
    /** Notifies listeners of a block being disconnected */
    virtual void BlockDisconnected(const std::shared_ptr<const MCBlock> &block) {}
    /** Notifies listeners of the new active block chain on-disk. */
    virtual void SetBestChain(const MCBlockLocator &locator) {}
    /** Notifies listeners about an inventory item being seen on the network. */
    virtual void Inventory(const uint256 &hash) {}
    /** Tells listeners to broadcast their data. */
    virtual void ResendWalletTransactions(int64_t nBestBlockTime, MCConnman* connman) {}
    /**
     * Notifies listeners of a block validation result.
     * If the provided MCValidationState IsValid, the provided block
     * is guaranteed to be the current best block at the time the
     * callback was generated (not necessarily now)
     */
    virtual void BlockChecked(const MCBlock&, const MCValidationState&) {}
    /**
     * Notifies listeners that a block which builds directly on our current tip
     * has been received and connected to the headers tree, though not validated yet */
    virtual void NewPoWValidBlock(const MCBlockIndex *pindex, const std::shared_ptr<const MCBlock>& block) {};
    friend void ::RegisterValidationInterface(MCValidationInterface*);
    friend void ::UnregisterValidationInterface(MCValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();
};

struct MainSignalsInstance;
class MCMainSignals {
private:
    std::unique_ptr<MainSignalsInstance> m_internals;

    friend void ::RegisterValidationInterface(MCValidationInterface*);
    friend void ::UnregisterValidationInterface(MCValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();

public:
    /** Register a MCScheduler to give callbacks which should run in the background (may only be called once) */
    void RegisterBackgroundSignalScheduler(MCScheduler& scheduler);
    /** Unregister a MCScheduler to give callbacks which should run in the background - these callbacks will now be dropped! */
    void UnregisterBackgroundSignalScheduler();
    /** Call any remaining callbacks on the calling thread */
    void FlushBackgroundCallbacks();

    void UpdatedBlockTip(const MCBlockIndex *, const MCBlockIndex *, bool fInitialDownload);
    void TransactionAddedToMempool(const MCTransactionRef &);
    void BlockConnected(const std::shared_ptr<const MCBlock> &, const MCBlockIndex *pindex, const std::vector<MCTransactionRef> &);
    void BlockDisconnected(const std::shared_ptr<const MCBlock> &);
    void SetBestChain(const MCBlockLocator &);
    void Inventory(const uint256 &);
    void Broadcast(int64_t nBestBlockTime, MCConnman* connman);
    void BlockChecked(const MCBlock&, const MCValidationState&);
    void NewPoWValidBlock(const MCBlockIndex *, const std::shared_ptr<const MCBlock>&);
};

MCMainSignals& GetMainSignals();

#endif // CELLLINK_VALIDATIONINTERFACE_H
