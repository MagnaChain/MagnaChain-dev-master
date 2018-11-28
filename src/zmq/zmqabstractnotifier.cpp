// Copyright (c) 2015 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "zmq/zmqabstractnotifier.h"
#include "utils/util.h"


CZMQAbstractNotifier::~CZMQAbstractNotifier()
{
    assert(!psocket);
}

bool CZMQAbstractNotifier::NotifyBlock(const MCBlockIndex * /*MCBlockIndex*/)
{
    return true;
}

bool CZMQAbstractNotifier::NotifyTransaction(const MCTransaction &/*transaction*/)
{
    return true;
}
