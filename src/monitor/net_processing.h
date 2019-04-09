// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_MONITOR_NET_PROCESSING_H
#define MAGNACHAIN_MONITOR_NET_PROCESSING_H

#include "net/net_processing.h"

bool MonitorProcessMessage(MCNode* pfrom, const std::string& strCommand, MCDataStream& vRecv, int64_t nTimeReceived, const MCChainParams& chainparams, MCConnman* connman, const std::atomic<bool>& interruptMsgProc);

class MonitorPeerLogicValidation : public PeerLogicValidation
{
public:
    explicit MonitorPeerLogicValidation(MCConnman* connman, MCScheduler &scheduler, ProcessMessageFunc processMessageFunc, GetLocatorFunc getLocatorFunc);

    void GetBlockData(MCNode* pto, MCNodeState& state, bool fFetch, std::vector<MCInv>& vGetData) override;
};

#endif // MAGNACHAIN_NET_PROCESSING_H
