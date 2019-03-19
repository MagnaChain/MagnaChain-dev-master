#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
测试：
赎回挖矿币
//赎回挖矿币, 步骤
// 1).侧链提起赎回请求.(侧链先销毁挖矿币,防止继续挖矿)
// 2).主链收到,创造新的交易,抵押币作为输入,赎回到正常地址,需要指定来自那个侧链请求
// 如果是主链先发起请求的,而且是先拿回抵押币的话,可能侧链还在继续挖矿.
// 这个交易和前面跨链交易不一样,原先"转到"侧链成为挖矿币的输入并没有销毁,可以作为转入s2时的输入.
// 赎回挖矿币, 步骤1
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
        self.num_nodes = 2  #todo should be 2 nodes

        '''
        self.num_sidenodes here is setting sidechain nodes num，just like self.num_nodes
        and the self.sidenodes like self.nodes
        '''
        self.num_sidenodes = 2
        self.side_extra_args = [["-regtestrsheight=10"],["-regtestrsheight=10"]]
        self.rpc_timewait = 900

    def run_test(self):
        """Main test logic"""
        self.sync_all([self.sidenodes])
        for i, node in enumerate(self.nodes):
            # for convenient
            setattr(self, 'node' + str(i), node)

        for i, node in enumerate(self.sidenodes):
            # for convenient
            setattr(self, 'snode' + str(i), node)

        for i in range(self.num_nodes):
            self.sidenodes[i].generate(2)
            assert_equal(len(self.sidenodes[i].getrawmempool()),0)
            self.sync_all([self.sidenodes])
            self.nodes[i].generate(2)
            self.sync_all()
            # // 赎回挖矿币, 步骤
            # // 1).侧链提起赎回请求.(侧链先销毁挖矿币, 防止继续挖矿)
            #       // 2).主链收到, 创造新的交易, 抵押币作为输入, 赎回到正常地址, 需要指定来自那个侧链请求
            #             // 如果是主链先发起请求的, 而且是先拿回抵押币的话, 可能侧链还在继续挖矿.
            #             // 这个交易和前面跨链交易不一样, 原先
            # "转到"
            # 侧链成为挖矿币的输入并没有销毁, 可以作为转入s2时的输入.
            # // 赎回挖矿币, 步骤1

        # 原来抵押的总额
        origin_mortgage = 50000
        start_balance = self.snode0.getbalance()
        start_balance1 = self.snode1.getbalance()
        mortgage_txs = self.snode0.listmortgagecoins()
        assert_raises_rpc_error(-32600,'Coin need 10 confirmation',self.snode0.redeemmortgagecoinstatement,mortgage_txs[0]['txid'])
        self.snode0.generate(7)  #使REDEEM_SAFE_HEIGHT满足
        # 确认抵押交易的确认数正确,这里的所有确认数应该为4 + 7 = 11
        assert all([ item['confirmations'] == 11 for item in mortgage_txs])
        print(self.snode0.listmortgagecoins())
        print(self.snode0.redeemmortgagecoinstatement(mortgage_txs[0]['txid']))
        assert_equal(self.snode0.listmortgagecoins(),[])
        assert_raises_rpc_error(-32603,'no address with enough coins',self.snode0.generate,1)



if __name__ == '__main__':
    RedeemMortgageTest().main()
