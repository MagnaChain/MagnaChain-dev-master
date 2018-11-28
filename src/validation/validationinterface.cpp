// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
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
    boost::signals2::signal<void (const CellBlockIndex *, const CellBlockIndex *, bool fInitialDownload)> UpdatedBlockTip;
    boost::signals2::signal<void (const CellTransactionRef &)> TransactionAddedToMempool;
    boost::signals2::signal<void (const std::shared_ptr<const CellBlock> &, const CellBlockIndex *pindex, const std::vector<CellTransactionRef>&)> BlockConnected;
    boost::signals2::signal<void (const std::shared_ptr<const CellBlock> &)> BlockDisconnected;
    boost::signals2::signal<void (const CellBlockLocator &)> SetBestChain;
    boost::signals2::signal<void (const uint256 &)> Inventory;
    boost::signals2::signal<void (int64_t nBestBlockTime, CellConnman* connman)> Broadcast;
    boost::signals2::signal<void (const CellBlock&, const CellValidationState&)> BlockChecked;
    boost::signals2::signal<void (const CellBlockIndex *, const std::shared_ptr<const CellBlock>&)> NewPoWValidBlock;

    // We are not allowed to assume the scheduler only runs in one thread,
    // but must ensure all callbacks happen in-order, so we end up creating
    // our own queue here :(
    SingleThreadedSchedulerClient m_schedulerClient;

    MainSignalsInstance(CellScheduler *pscheduler) : m_schedulerClient(pscheduler) {}
};

static CellMainSignals g_signals;

void CellMainSignals::RegisterBackgroundSignalScheduler(CellScheduler& scheduler) {
    assert(!m_internals);
    m_internals.reset(new MainSignalsInstance(&scheduler));
}

void CellMainSignals::UnregisterBackgroundSignalScheduler() {
    m_internals.reset(nullptr);
}

void CellMainSignals::FlushBackgroundCallbacks() {
    m_internals->m_schedulerClient.EmptyQueue();
}

CellMainSignals& GetMainSignals()
{
    return g_signals;
}

void RegisterValidationInterface(CellValidationInterface* pwalletIn) {
    g_signals.m_internals->UpdatedBlockTip.connect(boost::bind(&CellValidationInterface::UpdatedBlockTip, pwalletIn, _1, _2, _3));
    g_signals.m_internals->TransactionAddedToMempool.connect(boost::bind(&CellValidationInterface::TransactionAddedToMempool, pwalletIn, _1));
    g_signals.m_internals->BlockConnected.connect(boost::bind(&CellValidationInterface::BlockConnected, pwalletIn, _1, _2, _3));
    g_signals.m_internals->BlockDisconnected.connect(boost::bind(&CellValidationInterface::BlockDisconnected, pwalletIn, _1));
    g_signals.m_internals->SetBestChain.connect(boost::bind(&CellValidationInterface::SetBestChain, pwalletIn, _1));
    g_signals.m_internals->Inventory.connect(boost::bind(&CellValidationInterface::Inventory, pwalletIn, _1));
    g_signals.m_internals->Broadcast.connect(boost::bind(&CellValidationInterface::ResendWalletTransactions, pwalletIn, _1, _2));
    g_signals.m_internals->BlockChecked.connect(boost::bind(&CellValidationInterface::BlockChecked, pwalletIn, _1, _2));
    g_signals.m_internals->NewPoWValidBlock.connect(boost::bind(&CellValidationInterface::NewPoWValidBlock, pwalletIn, _1, _2));
}

void UnregisterValidationInterface(CellValidationInterface* pwalletIn) {
    g_signals.m_internals->BlockChecked.disconnect(boost::bind(&CellValidationInterface::BlockChecked, pwalletIn, _1, _2));
    g_signals.m_internals->Broadcast.disconnect(boost::bind(&CellValidationInterface::ResendWalletTransactions, pwalletIn, _1, _2));
    g_signals.m_internals->Inventory.disconnect(boost::bind(&CellValidationInterface::Inventory, pwalletIn, _1));
    g_signals.m_internals->SetBestChain.disconnect(boost::bind(&CellValidationInterface::SetBestChain, pwalletIn, _1));
    g_signals.m_internals->TransactionAddedToMempool.disconnect(boost::bind(&CellValidationInterface::TransactionAddedToMempool, pwalletIn, _1));
    g_signals.m_internals->BlockConnected.disconnect(boost::bind(&CellValidationInterface::BlockConnected, pwalletIn, _1, _2, _3));
    g_signals.m_internals->BlockDisconnected.disconnect(boost::bind(&CellValidationInterface::BlockDisconnected, pwalletIn, _1));
    g_signals.m_internals->UpdatedBlockTip.disconnect(boost::bind(&CellValidationInterface::UpdatedBlockTip, pwalletIn, _1, _2, _3));
    g_signals.m_internals->NewPoWValidBlock.disconnect(boost::bind(&CellValidationInterface::NewPoWValidBlock, pwalletIn, _1, _2));
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

void CellMainSignals::UpdatedBlockTip(const CellBlockIndex *pindexNew, const CellBlockIndex *pindexFork, bool fInitialDownload) {
    m_internals->UpdatedBlockTip(pindexNew, pindexFork, fInitialDownload);
}

void CellMainSignals::TransactionAddedToMempool(const CellTransactionRef &ptx) {
    m_internals->TransactionAddedToMempool(ptx);
}

void CellMainSignals::BlockConnected(const std::shared_ptr<const CellBlock> &pblock, const CellBlockIndex *pindex, const std::vector<CellTransactionRef>& vtxConflicted) {
    m_internals->BlockConnected(pblock, pindex, vtxConflicted);
}

void CellMainSignals::BlockDisconnected(const std::shared_ptr<const CellBlock> &pblock) {
    m_internals->BlockDisconnected(pblock);
}

void CellMainSignals::SetBestChain(const CellBlockLocator &locator) {
    m_internals->SetBestChain(locator);
}

void CellMainSignals::Inventory(const uint256 &hash) {
    m_internals->Inventory(hash);
}

void CellMainSignals::Broadcast(int64_t nBestBlockTime, CellConnman* connman) {
    m_internals->Broadcast(nBestBlockTime, connman);
}

void CellMainSignals::BlockChecked(const CellBlock& block, const CellValidationState& state) {
    m_internals->BlockChecked(block, state);
}

void CellMainSignals::NewPoWValidBlock(const CellBlockIndex *pindex, const std::shared_ptr<const CellBlock> &block) {
    m_internals->NewPoWValidBlock(pindex, block);
}
