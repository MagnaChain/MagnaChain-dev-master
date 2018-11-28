// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "validation/validationinterface.h"

#include "init.h"
#include "primitives/block.h"
#include "thread/scheduler.h"
#include "thread/sync.h"
#include "utils/util.h"

#include <list>
#include <atomic>

#include <boost/signals2/signal.hpp>

struct MainSignalsInstance {
    boost::signals2::signal<void (const MCBlockIndex *, const MCBlockIndex *, bool fInitialDownload)> UpdatedBlockTip;
    boost::signals2::signal<void (const MCTransactionRef &)> TransactionAddedToMempool;
    boost::signals2::signal<void (const std::shared_ptr<const MCBlock> &, const MCBlockIndex *pindex, const std::vector<MCTransactionRef>&)> BlockConnected;
    boost::signals2::signal<void (const std::shared_ptr<const MCBlock> &)> BlockDisconnected;
    boost::signals2::signal<void (const MCBlockLocator &)> SetBestChain;
    boost::signals2::signal<void (const uint256 &)> Inventory;
    boost::signals2::signal<void (int64_t nBestBlockTime, MCConnman* connman)> Broadcast;
    boost::signals2::signal<void (const MCBlock&, const MCValidationState&)> BlockChecked;
    boost::signals2::signal<void (const MCBlockIndex *, const std::shared_ptr<const MCBlock>&)> NewPoWValidBlock;

    // We are not allowed to assume the scheduler only runs in one thread,
    // but must ensure all callbacks happen in-order, so we end up creating
    // our own queue here :(
    SingleThreadedSchedulerClient m_schedulerClient;

    MainSignalsInstance(MCScheduler *pscheduler) : m_schedulerClient(pscheduler) {}
};

static MCMainSignals g_signals;

void MCMainSignals::RegisterBackgroundSignalScheduler(MCScheduler& scheduler) {
    assert(!m_internals);
    m_internals.reset(new MainSignalsInstance(&scheduler));
}

void MCMainSignals::UnregisterBackgroundSignalScheduler() {
    m_internals.reset(nullptr);
}

void MCMainSignals::FlushBackgroundCallbacks() {
    m_internals->m_schedulerClient.EmptyQueue();
}

MCMainSignals& GetMainSignals()
{
    return g_signals;
}

void RegisterValidationInterface(MCValidationInterface* pwalletIn) {
    g_signals.m_internals->UpdatedBlockTip.connect(boost::bind(&MCValidationInterface::UpdatedBlockTip, pwalletIn, _1, _2, _3));
    g_signals.m_internals->TransactionAddedToMempool.connect(boost::bind(&MCValidationInterface::TransactionAddedToMempool, pwalletIn, _1));
    g_signals.m_internals->BlockConnected.connect(boost::bind(&MCValidationInterface::BlockConnected, pwalletIn, _1, _2, _3));
    g_signals.m_internals->BlockDisconnected.connect(boost::bind(&MCValidationInterface::BlockDisconnected, pwalletIn, _1));
    g_signals.m_internals->SetBestChain.connect(boost::bind(&MCValidationInterface::SetBestChain, pwalletIn, _1));
    g_signals.m_internals->Inventory.connect(boost::bind(&MCValidationInterface::Inventory, pwalletIn, _1));
    g_signals.m_internals->Broadcast.connect(boost::bind(&MCValidationInterface::ResendWalletTransactions, pwalletIn, _1, _2));
    g_signals.m_internals->BlockChecked.connect(boost::bind(&MCValidationInterface::BlockChecked, pwalletIn, _1, _2));
    g_signals.m_internals->NewPoWValidBlock.connect(boost::bind(&MCValidationInterface::NewPoWValidBlock, pwalletIn, _1, _2));
}

void UnregisterValidationInterface(MCValidationInterface* pwalletIn) {
    g_signals.m_internals->BlockChecked.disconnect(boost::bind(&MCValidationInterface::BlockChecked, pwalletIn, _1, _2));
    g_signals.m_internals->Broadcast.disconnect(boost::bind(&MCValidationInterface::ResendWalletTransactions, pwalletIn, _1, _2));
    g_signals.m_internals->Inventory.disconnect(boost::bind(&MCValidationInterface::Inventory, pwalletIn, _1));
    g_signals.m_internals->SetBestChain.disconnect(boost::bind(&MCValidationInterface::SetBestChain, pwalletIn, _1));
    g_signals.m_internals->TransactionAddedToMempool.disconnect(boost::bind(&MCValidationInterface::TransactionAddedToMempool, pwalletIn, _1));
    g_signals.m_internals->BlockConnected.disconnect(boost::bind(&MCValidationInterface::BlockConnected, pwalletIn, _1, _2, _3));
    g_signals.m_internals->BlockDisconnected.disconnect(boost::bind(&MCValidationInterface::BlockDisconnected, pwalletIn, _1));
    g_signals.m_internals->UpdatedBlockTip.disconnect(boost::bind(&MCValidationInterface::UpdatedBlockTip, pwalletIn, _1, _2, _3));
    g_signals.m_internals->NewPoWValidBlock.disconnect(boost::bind(&MCValidationInterface::NewPoWValidBlock, pwalletIn, _1, _2));
}

void UnregisterAllValidationInterfaces() {
    g_signals.m_internals->BlockChecked.disconnect_all_slots();
    g_signals.m_internals->Broadcast.disconnect_all_slots();
    g_signals.m_internals->Inventory.disconnect_all_slots();
    g_signals.m_internals->SetBestChain.disconnect_all_slots();
    g_signals.m_internals->TransactionAddedToMempool.disconnect_all_slots();
    g_signals.m_internals->BlockConnected.disconnect_all_slots();
    g_signals.m_internals->BlockDisconnected.disconnect_all_slots();
    g_signals.m_internals->UpdatedBlockTip.disconnect_all_slots();
    g_signals.m_internals->NewPoWValidBlock.disconnect_all_slots();
}

void MCMainSignals::UpdatedBlockTip(const MCBlockIndex *pindexNew, const MCBlockIndex *pindexFork, bool fInitialDownload) {
    m_internals->UpdatedBlockTip(pindexNew, pindexFork, fInitialDownload);
}

void MCMainSignals::TransactionAddedToMempool(const MCTransactionRef &ptx) {
    m_internals->TransactionAddedToMempool(ptx);
}

void MCMainSignals::BlockConnected(const std::shared_ptr<const MCBlock> &pblock, const MCBlockIndex *pindex, const std::vector<MCTransactionRef>& vtxConflicted) {
    m_internals->BlockConnected(pblock, pindex, vtxConflicted);
}

void MCMainSignals::BlockDisconnected(const std::shared_ptr<const MCBlock> &pblock) {
    m_internals->BlockDisconnected(pblock);
}

void MCMainSignals::SetBestChain(const MCBlockLocator &locator) {
    m_internals->SetBestChain(locator);
}

void MCMainSignals::Inventory(const uint256 &hash) {
    m_internals->Inventory(hash);
}

void MCMainSignals::Broadcast(int64_t nBestBlockTime, MCConnman* connman) {
    m_internals->Broadcast(nBestBlockTime, connman);
}

void MCMainSignals::BlockChecked(const MCBlock& block, const MCValidationState& state) {
    m_internals->BlockChecked(block, state);
}

void MCMainSignals::NewPoWValidBlock(const MCBlockIndex *pindex, const std::shared_ptr<const MCBlock> &block) {
    m_internals->NewPoWValidBlock(pindex, block);
}
