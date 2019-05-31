#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
测试：
创建多侧链
"""
# Imports should be in PEP8 ordering (std library first, then third party
# libraries then local imports).
from decimal import Decimal
import sys

# Avoid wildcard * imports if possible
from test_framework.test_framework import MutiSideChainTestFramework
from test_framework.mininode import MINER_REWARD
from test_framework.util import (
    assert_equal,
    get_mempool_total_fee,
    wait_until,
    get_chainwork,
)


class MutiSideChainTest(MutiSideChainTestFramework):
    # Each functional test is a subclass of the MagnaChainTestFramework class.

    # Override the set_test_params(), add_options(), setup_chain(), setup_network()
    # and setup_nodes() methods to customize the test setup as required.

    def run_test(self):
        """Main test logic"""
        node = self.node0
        for n in self.sidenodes:
            n.generate(2)
        node.generate(2)

        self.log.info('test side to main')
        for i in range(2):
            main_balance = node.getbalance()
            block_height = node.getblockcount()
            start_balance = self.sidenodes[i].getbalance()
            print(start_balance)
            assert start_balance > 20
            addr = node.getnewaddress()
            txid = self.sidenodes[i].sendtobranchchain('main', addr, 1)['txid']
            assert txid in self.sidenodes[i].getrawmempool()
            self.sidenodes[i].generate(7)
            assert txid not in self.sidenodes[i].getrawmempool()
            node.generate(2)
            self.sidenodes[i].generate(1)
            assert_equal(len(node.getrawmempool()), 2)
            total_fee = get_mempool_total_fee(node, only_version=[7])
            node.generate(2)
            print("total fee:", total_fee)
            print("diff:", node.getbalance() - (Decimal(main_balance) + Decimal(
                MINER_REWARD * (node.getblockcount() - block_height)) + 1 + total_fee))
            assert_equal(node.getbalanceof(addr), 1)
            assert_equal(node.getbalance(), Decimal(main_balance) + Decimal(
                MINER_REWARD * (node.getblockcount() - block_height)) + 1 + total_fee)

        assert_equal(self.snode0.getbalance(),self.snode1.getbalance())

        self.log.info('test main to side')
        for i in range(2):
            node.generate(2)
            assert_equal(len(node.getrawmempool()), 0)
            sidechain_id  = self.sidechain_id_one if i == 0 else self.sidechain_id_two
            saddr = self.sidenodes[i].getnewaddress()
            txid = node.sendtobranchchain(sidechain_id, saddr, 100)['txid']
            node.generate(8)  # sync to sidechain
            assert_equal(len(self.sidenodes[i].getrawmempool()), 1)
            print(self.get_txfee(self.sidenodes[i].getrawmempool()[0], self.sidenodes[i]))
            txfee = self.sidenodes[i].getrawmempool(True)[self.sidenodes[i].getrawmempool()[0]]['fee']
            side_balance = self.sidenodes[i].getbalance()
            self.sidenodes[i].generate(2)
            # ensure MGC is reached,include the fee
            assert_equal(self.sidenodes[i].getbalanceof(saddr), 100.0000000)
            assert_equal(self.sidenodes[i].getbalance(),
                         Decimal(side_balance + Decimal(100) + Decimal(txfee)).quantize(Decimal("0.00000000")))
            self.sidenodes[i].generate(2)
            assert_equal(self.sidenodes[i].getrawmempool(), [])

    def get_txfee(self, txid, node=None):
        return Decimal(node.gettransaction(txid)['fee'])

if __name__ == '__main__':
    MutiSideChainTest().main()
