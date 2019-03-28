#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
test cpp code for RevertTransaction
1. 支链上有币
2. 支链转主链
3. test remove from mempool
4. fork re-add to mempool

"""
# Imports should be in PEP8 ordering (std library first, then third party
# libraries then local imports).
from decimal import Decimal
import sys

# Avoid wildcard * imports if possible
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.mininode import MINER_REWARD,CTransaction
from test_framework.contract import Contract
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
    bytes_to_hex_str,
)


class RevertTransactionTest(MagnaChainTestFramework):
    # Each functional test is a subclass of the MagnaChainTestFramework class.

    # Override the set_test_params(), add_options(), setup_chain(), setup_network()
    # and setup_nodes() methods to customize the test setup as required.

    def set_test_params(self):
        """Override test parameters for your individual test.

        This method must be overridden and num_nodes must be exlicitly set."""
        self.setup_clean_chain = True
        self.num_nodes = 2
        self.extra_args = [['-txindex','-grouping=0'],['-txindex','-grouping=0']]
        # self.side_extra_args = [['-txindex']]
        self.side_extra_args = [['-disablesafemode=1'],['-disablesafemode=1']]

        '''
        self.num_sidenodes here is setting sidechain nodes num，just like self.num_nodes
        and the self.sidenodes like self.nodes
        '''
        self.num_sidenodes = 2
        # self.rpc_timewait = 900

    def run_test(self):
        """Main test logic"""
        self.sync_all([self.sidenodes])

        for i in range(2):
            self.sidenodes[i].generate(2)
            self.sync_all([self.sidenodes])
            print(self.node0.getrawmempool(True),self.node1.getrawmempool())
            self.nodes[i].generate(2)
            print(self.node0.getrawmempool(), self.node1.getrawmempool(True))
            self.sync_all()  #放开注释，这里会主链的内存池会同步失败，因为branch block info duplicate

        self.test_getblockchaininfo()
        self.test_getallbranchinfo()
        self.test_getbranchchainheight()
        self.test_getbranchchaininfo()
        self.test_reverttransaction()
        
    def test_getblockchaininfo(self):
        self.log.info(sys._getframe().f_code.co_name)
        ret = self.snode0.getblockchaininfo()
        assert_equal(ret['chain'], 'branch')
        assert_equal(ret['branchid'], self.sidechain_id)

    def test_getallbranchinfo(self):
        self.log.info(sys._getframe().f_code.co_name)
        ret = self.node0.getallbranchinfo()[0]
        print(ret)
        assert_equal(ret['ismaturity'], True)
        assert_equal(ret['confirmations'], 29)

    def test_getbranchchainheight(self):
        self.log.info(sys._getframe().f_code.co_name)
        ret = self.node0.getbranchchainheight(self.sidechain_id)
        print(ret)
        assert_equal(ret['height'], self.snode0.getblockcount())
        assert_equal(ret['blockhash'], self.snode0.getbestblockhash())

    def test_getbranchchaininfo(self):
        self.log.info(sys._getframe().f_code.co_name)
        tmp = self.node0.getallbranchinfo()[0]
        ret = self.node0.getbranchchaininfo(self.sidechain_id)
        print(ret)
        assert_equal(tmp['txid'], ret['txid'])
        assert_equal(tmp['vseeds'], ret['vseeds'])
        assert_equal(tmp['seedspec6'], ret['seedspec6'])

    def test_reverttransaction(self):
        self.node0.sendtobranchchain(self.sidechain_id, self.snode0.getnewaddress(), 20)
        self.node0.sendtobranchchain(self.sidechain_id, self.snode0.getnewaddress(), 20)
        self.node0.sendtobranchchain(self.sidechain_id, self.snode0.getnewaddress(), 20)
        self.node0.sendtobranchchain(self.sidechain_id, self.snode0.getnewaddress(), 20)
        self.node0.sendtobranchchain(self.sidechain_id, self.snode0.getnewaddress(), 20)
        self.node0.generate(8)
        self.snode0.generate(1)
        self.log.info("snode0 balance".format(self.snode0.getbalance()))
        self.node0.generate(1)
        node0addr = self.node0.getnewaddress()
        self.snode0.sendtobranchchain("main", node0addr, 1)
        self.snode0.generate(7)
        self.node0.generate(1)
        assert_equal(0, len(self.node0.getrawmempool()))
        self.snode0.generate(1)
        txids = self.node0.getrawmempool()
        assert_equal(2, len(txids))
        self.node0.generate(1)
        assert_equal(0, len(self.node0.getrawmempool()))
        self.node0.invalidateblock(self.node0.getbestblockhash())
        txids2 = self.node0.getrawmempool()
        assert_equal(2, len(txids2))
        assert_equal(txids, txids2)
        self.node0.generate(1)
        assert_equal(0, len(self.node0.getrawmempool()))
    

if __name__ == '__main__':
    RevertTransactionTest().main()
