// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_MONITOR_INIT_H
#define MAGNACHAIN_MONITOR_INIT_H

#include <string>

class MCScheduler;

namespace boost
{
    class thread_group;
} // namespace boost

bool MonitorInitMain(boost::thread_group& threadGroup, MCScheduler& scheduler);

#endif