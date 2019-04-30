#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
测试：
侧链分叉下的情况，主要包括以下方面:
1.合并后，确认侧链的主链、主链的侧链信息是否随着合并而更新
"""
# Imports should be in PEP8 ordering (std library first, then third party
# libraries then local imports).
import sys

# Avoid wildcard * imports if possible
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.contract import Contract
from test_framework.mininode import MINER_REWARD
from test_framework.util import (
    assert_equal,
    get_chainwork,
    get_mempool_total_fee,
)

class SideChainForkTest(MagnaChainTestFramework):
    # Each functional test is a subclass of the MagnaChainTestFramework class.

    # Override the set_test_params(), add_options(), setup_chain(), setup_network()
    # and setup_nodes() methods to customize the test setup as required.

    def set_test_params(self):
        """Override test parameters for your individual test.

        This method must be overridden and num_nodes must be exlicitly set."""
        self.setup_clean_chain = True
        self.num_nodes = 4

        '''
        self.num_sidenodes here is setting sidechain nodes num，just like self.num_nodes
        and the self.sidenodes like self.nodes
        '''
        self.num_sidenodes = 4
        # mapped，侧链对主链的映射，eg.[[0],[],[],[1]],表示侧链的0节点attach在主链的节点0；侧链1节点attach在主链的节点3
        # self.mapped = [[0,1],[2,3]]

    def run_test(self):
        """Main test logic"""

        # prepare
        for i in range(self.num_sidenodes):
            n = self.sidenodes[i]
            print(len(n.getpeerinfo()),n.getpeerinfo())
            print('mainchain',len(self.nodes[i].getpeerinfo()))
            self.sync_all(self.nodes)
            n.generate(2)  # make some coins
            self.sync_all([self.sidenodes])
            self.log.info("start balance {}".format(n.getbalance()))
        self.sync_all()
        for i in range(self.num_sidenodes):
            n = self.sidenodes[i]
            print(len(n.getpeerinfo()),n.getpeerinfo())
            print('mainchain',len(self.nodes[i].getpeerinfo()))
        print([len(n.getrawmempool()) for n in self.nodes])
        assert_equal(len(self.node0.getrawmempool()),8)
        self.node0.generate(1)
        self.sync_all()

        self.test_sidechain_info()


    def test_sidechain_info(self):
        '''
        确认侧链的主链、主链的侧链信息是否随着合并而更新
        node0会产生一条转账到主链的交易，node2只出空块，并且保证工作量高于node0
        合并后，node0的转账交易失效
        :return:
        '''
        # make sure data is correct before fork
        self.log.info(sys._getframe().f_code.co_name)
        ret = self.node0.getbranchchainheight(self.sidechain_id)
        assert_equal(ret['height'], self.snode0.getblockcount())
        assert_equal(ret['blockhash'], self.snode0.getbestblockhash())
        before_balance = self.node0.getbalance()
        before_snode0_bal = self.snode0.getbalance()

        # split fork
        self.split_sidechain_network()

        maddr = self.node0.getnewaddress()
        txid = self.snode0.sendtobranchchain('main', maddr, 1)['txid']
        self.sync_sidechain()
        self.snode0.generate(7)
        self.sync_sidechain()
        self.sync_all()
        self.node0.generate(1)
        self.sync_all()
        self.snode0.generate(1)
        self.sync_sidechain()
        self.node0.generate(1)
        self.sync_all()

        # assert the balance
        assert_equal(self.node0.getbalanceof(maddr),1)
        assert_equal(self.node0.getbalance(), before_balance + 1 + MINER_REWARD + get_mempool_total_fee(self.node0))
        assert_equal(self.snode0.getbalance(), before_snode0_bal - 1 + get_mempool_total_fee(self.node0,onle_version = [7]))
        pass



    def sync_sidechain(self):
        '''
        侧链同步节点
        :return:
        '''
        if self.is_fork:
            self.sync_all([self.sidenodes[:2], self.sidenodes[2:]])
        else:
            self.sync_all([self.sidenodes])

    def split_sidechain_network(self):
        self.split_network(sidechain=True)
        self.is_fork = True

    def join_sidechain_network(self):
        self.join_network(sidechain = True)
        self.is_fork = False

if __name__ == '__main__':
    SideChainForkTest().main()
