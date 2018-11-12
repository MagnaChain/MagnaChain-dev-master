// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_VALIDATIONINTERFACE_H
#define CELLLINK_VALIDATIONINTERFACE_H

#include <memory>

#include "primitives/transaction.h" // CellTransaction(Ref)

class CellBlock;
class CellBlockIndex;
struct CellBlockLocator;
class CellBlockIndex;
class CellConnman;
class CReserveScript;
class CellValidationInterface;
class CellValidationState;
class uint256;
class CellScheduler;

// These functions dispatch to one or all registered wallets

/** Register a wallet to receive updates from core */
void RegisterValidationInterface(CellValidationInterface* pwalletIn);
/** Unregister a wallet from core */
void UnregisterValidationInterface(CellValidationInterface* pwalletIn);
/** Unregister all wallets from core */
void UnregisterAllValidationInterfaces();

class CellValidationInterface {
protected:
    /** Notifies listeners of updated block chain tip */
    virtual void UpdatedBlockTip(const CellBlockIndex *pindexNew, const CellBlockIndex *pindexFork, bool fInitialDownload) {}
    /** Notifies listeners of a transaction having been added to mempool. */
    virtual void TransactionAddedToMempool(const CellTransactionRef &ptxn) {}
    /**
     * Notifies listeners of a block being connected.
     * Provides a vector of transactions evicted from the mempool as a result.
     */
    virtual void BlockConnected(const std::shared_ptr<const CellBlock> &block, const CellBlockIndex *pindex, const std::vector<CellTransactionRef> &txnConflicted) {}
    /** Notifies listeners of a block being disconnected */
    virtual void BlockDisconnected(const std::shared_ptr<const CellBlock> &block) {}
    /** Notifies listeners of the new active block chain on-disk. */
    virtual void SetBestChain(const CellBlockLocator &locator) {}
    /** Notifies listeners about an inventory item being seen on the network. */
    virtual void Inventory(const uint256 &hash) {}
    /** Tells listeners to broadcast their data. */
    virtual void ResendWalletTransactions(int64_t nBestBlockTime, CellConnman* connman) {}
    /**
     * Notifies listeners of a block validation result.
     * If the provided CellValidationState IsValid, the provided block
     * is guaranteed to be the current best block at the time the
     * callback was generated (not necessarily now)
     */
    virtual void BlockChecked(const CellBlock&, const CellValidationState&) {}
    /**
     * Notifies listeners that a block which builds directly on our current tip
     * has been received and connected to the headers tree, though not validated yet */
    virtual void NewPoWValidBlock(const CellBlockIndex *pindex, const std::shared_ptr<const CellBlock>& block) {};
    friend void ::RegisterValidationInterface(CellValidationInterface*);
    friend void ::UnregisterValidationInterface(CellValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();
};

struct MainSignalsInstance;
class CellMainSignals {
private:
    std::unique_ptr<MainSignalsInstance> m_internals;

    friend void ::RegisterValidationInterface(CellValidationInterface*);
    friend void ::UnregisterValidationInterface(CellValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();

public:
    /** Register a CellScheduler to give callbacks which should run in the background (may only be called once) */
    void RegisterBackgroundSignalScheduler(CellScheduler& scheduler);
    /** Unregister a CellScheduler to give callbacks which should run in the background - these callbacks will now be dropped! */
    void UnregisterBackgroundSignalScheduler();
    /** Call any remaining callbacks on the calling thread */
    void FlushBackgroundCallbacks();

    void UpdatedBlockTip(const CellBlockIndex *, const CellBlockIndex *, bool fInitialDownload);
    void TransactionAddedToMempool(const CellTransactionRef &);
    void BlockConnected(const std::shared_ptr<const CellBlock> &, const CellBlockIndex *pindex, const std::vector<CellTransactionRef> &);
    void BlockDisconnected(const std::shared_ptr<const CellBlock> &);
    void SetBestChain(const CellBlockLocator &);
    void Inventory(const uint256 &);
    void Broadcast(int64_t nBestBlockTime, CellConnman* connman);
    void BlockChecked(const CellBlock&, const CellValidationState&);
    void NewPoWValidBlock(const CellBlockIndex *, const std::shared_ptr<const CellBlock>&);
};

CellMainSignals& GetMainSignals();

#endif // CELLLINK_VALIDATIONINTERFACE_H
