// Copyright (c) 2015 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Functionality for communicating with Tor.
 */
#ifndef CELLLINK_TORCONTROL_H
#define CELLLINK_TORCONTROL_H

#include "thread/scheduler.h"

extern const std::string DEFAULT_TOR_CONTROL;
static const bool DEFAULT_LISTEN_ONION = true;

void StartTorControl(boost::thread_group& threadGroup, MCScheduler& scheduler);
void InterruptTorControl();
void StopTorControl();

#endif /* CELLLINK_TORCONTROL_H */
