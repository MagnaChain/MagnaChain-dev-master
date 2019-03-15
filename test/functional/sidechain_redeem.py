#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
测试：
赎回挖矿币
"""
# Imports should be in PEP8 ordering (std library first, then third party
# libraries then local imports).
from decimal import Decimal

# Avoid wildcard * imports if possible
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.mininode import MINER_REWARD,CTransaction
from test_framework.contract import Contract
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
    bytes_to_hex_str,
)


class RedeemMortgageTest(MagnaChainTestFramework):
    # Each functional test is a subclass of the MagnaChainTestFramework class.

    # Override the set_test_params(), add_options(), setup_chain(), setup_network()
    # and setup_nodes() methods to customize the test setup as required.

    def set_test_params(self):
        """Override test parameters for your individual test.

        This method must be overridden and num_nodes must be exlicitly set."""
        self.setup_clean_chain = True
        self.num_nodes = 1  #todo should be 2 nodes
        self.extra_args = [['-txindex']]
        self.side_extra_args = [['-txindex']]

        '''
        self.num_sidenodes here is setting sidechain nodes num，just like self.num_nodes
        and the self.sidenodes like self.nodes
        '''
        self.num_sidenodes = 1
        # self.rpc_timewait = 900

    def run_test(self):
        """Main test logic"""
        self.sync_all([self.sidenodes])
        for i, node in enumerate(self.nodes):
            # for convenient
            setattr(self, 'node' + str(i), node)

        for i, node in enumerate(self.sidenodes):
            # for convenient
            setattr(self, 'snode' + str(i), node)

        for i in range(1):
            self.sidenodes[i].generate(2)
            # self.sync_all([self.sidenodes])
            # print(self.node0.getrawmempool(True),self.node1.getrawmempool())
            self.nodes[i].generate(2)
            # print(self.node0.getrawmempool(), self.node1.getrawmempool(True))
            # self.sync_all()  #放开注释，这里会主链的内存池会同步失败，因为branch block info duplicate

if __name__ == '__main__':
    RedeemMortgageTest().main()
