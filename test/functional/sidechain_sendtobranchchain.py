#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
测试：
创建侧链
"""
# Imports should be in PEP8 ordering (std library first, then third party
# libraries then local imports).
from decimal import Decimal

# Avoid wildcard * imports if possible
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.contract import Contract
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
)


class SendToBranchchainTest(MagnaChainTestFramework):
    # Each functional test is a subclass of the MagnaChainTestFramework class.

    # Override the set_test_params(), add_options(), setup_chain(), setup_network()
    # and setup_nodes() methods to customize the test setup as required.

    def set_test_params(self):
        """Override test parameters for your individual test.

        This method must be overridden and num_nodes must be exlicitly set."""
        self.setup_clean_chain = True
        self.num_nodes = 2
        self.mapped = [[0],[1]]

        '''
        self.num_sidenodes here is setting sidechain nodes num，just like self.num_nodes
        and the self.sidenodes like self.nodes
        '''
        self.num_sidenodes = 2

    def run_test(self):
        """Main test logic"""
        for i in range(2):
            print(self.nodes[i].rpcport)
        print(self.sidenodes[0].rpcport,self.sidenodes[0].getpeerinfo())
        print(self.sidenodes[1].rpcport,self.sidenodes[1].getpeerinfo())
        print(len(self.sidenodes[0].getrawmempool()),self.sidenodes[0].getrawmempool())
        print(len(self.sidenodes[1].getrawmempool()),self.sidenodes[1].getrawmempool())
        # self.sync_all([self.sidenodes])
        node = self.nodes[0]
        sidechain_id = self.sidechain_id
        saddr = self.sidenodes[0].getnewaddress()
        assert_raises_rpc_error(-3, "Invalid amount", node.sendtobranchchain, sidechain_id, saddr, 10000000000)
        assert_raises_rpc_error(-3, "Amount out of range", node.sendtobranchchain, sidechain_id, saddr, -1)
        txid = node.sendtobranchchain(sidechain_id, saddr, 0)['txid']
        txid1 = node.sendtobranchchain(sidechain_id, saddr, Decimal("0.00000009").quantize(Decimal("0.000000")))['txid']
        self.sync_all()
        assert txid in node.getrawmempool()
        assert txid1 in node.getrawmempool()
        node.generate(8)
        self.sidenodes[0].generate(2)
        assert_equal(self.sidenodes[0].getrawmempool(), [])
        txid = node.sendtobranchchain(sidechain_id, saddr, 100)['txid']
        node.generate(8)
        assert_equal(len(self.sidenodes[0].getrawmempool()), 1)
        # print(self.get_txfee(self.sidenodes[0].getrawmempool()[0],self.sidenodes[0]))
        txfee = self.sidenodes[0].getrawmempool(True)[self.sidenodes[0].getrawmempool()[0]]['fee']
        side_balance = self.sidenodes[0].getbalance()
        side_balance1 = self.sidenodes[1].getbalance()
        self.sync_all([self.sidenodes])
        assert_equal(self.sidenodes[0].getrawmempool(), self.sidenodes[1].getrawmempool())
        self.sidenodes[0].generate(2)
        self.sync_all([self.sidenodes])
        assert_equal(self.sidenodes[0].getbalanceof(saddr), 100.0000000)
        assert_equal(self.sidenodes[1].getbalanceof(saddr), 100.0000000)
        assert_equal(self.sidenodes[0].getbalance(),
                     Decimal(side_balance + Decimal(100) + Decimal(txfee)).quantize(Decimal("0.000000")))
        assert_equal(self.sidenodes[1].getbalance(),side_balance1)
        assert_equal(self.sidenodes[0].getbestblockhash(),self.sidenodes[1].getbestblockhash())

        txid = node.sendtobranchchain(sidechain_id, saddr, 100)['txid']
        node.generate(8)
        assert_equal(len(self.sidenodes[0].getrawmempool()), 1)
        txfee = self.sidenodes[0].getrawmempool(True)[self.sidenodes[0].getrawmempool()[0]]['fee']
        side_balance = self.sidenodes[0].getbalance()
        side_balance1 = self.sidenodes[1].getbalance()
        self.sync_all([self.sidenodes])
        assert_equal(self.sidenodes[0].getrawmempool(), self.sidenodes[1].getrawmempool())
        self.sidenodes[1].generate(2) #ensure another node generate blocks is work
        self.sync_all([self.sidenodes])
        assert_equal(self.sidenodes[0].getbalanceof(saddr), 200.0000000)
        assert_equal(self.sidenodes[1].getbalanceof(saddr), 200.0000000)
        assert_equal(self.sidenodes[0].getbalance(),side_balance)
        assert_equal(self.sidenodes[1].getbalance(),Decimal(side_balance1 + Decimal(100) + Decimal(txfee)).quantize(Decimal("0.000000")))
        assert_equal(self.sidenodes[0].getbestblockhash(),self.sidenodes[1].getbestblockhash())

        # self.sync_all(self.sidenodes)

    def get_txfee(self, txid, node = None):
        return Decimal(node.gettransaction(txid)['fee'])


if __name__ == '__main__':
    SendToBranchchainTest().main()
