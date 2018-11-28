// Copyright (c) 2011-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_TEST_MODULE MagnaChain Test Suite

#include "net/net.h"

#include <boost/test/unit_test.hpp>

std::unique_ptr<MCConnman> g_connman;

void Shutdown(void* parg)
{
  exit(EXIT_SUCCESS);
}

void StartShutdown()
{
  exit(EXIT_SUCCESS);
}

bool ShutdownRequested()
{
  return false;
}
