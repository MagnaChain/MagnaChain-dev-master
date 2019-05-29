#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
测试：
测试移动检查点，主要有以下测试点：
主链：
    1.允许范围内回撤（分叉）
    2.超过最大回撤长度
    3.无效化范围外区块，并尝试重新组合,invalidateblock and reconsiderblock
侧链：
    1.包含主链部分测试点
    2.举报范围内区块
    3.举报范围外区块

"""
# Imports should be in PEP8 ordering (std library first, then third party
# libraries then local imports).
from decimal import Decimal
import sys

# Avoid wildcard * imports if possible
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.mininode import MINER_REWARD, CTransaction
from test_framework.contract import Contract
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
    bytes_to_hex_str,
)

class MovingCheckPointTest(MagnaChainTestFramework):
    # Each functional test is a subclass of the MagnaChainTestFramework class.

    # Override the set_test_params(), add_options(), setup_chain(), setup_network()
    # and setup_nodes() methods to customize the test setup as required.

    def set_test_params(self):
        """Override test parameters for your individual test.

        This method must be overridden and num_nodes must be exlicitly set."""
        self.setup_clean_chain = True
        self.num_nodes = 4
        # self.extra_args = [['-txindex'], ['-txindex']]
        self.rpc_timewait = 120

        '''
        self.num_sidenodes here is setting sidechain nodes num，just like self.num_nodes
        and the self.sidenodes like self.nodes
        '''
        # self.num_sidenodes = 2
        # todo 需要补充侧链的用例

        # self.with_gdb = True

    def run_test(self):
        """Main test logic"""
        self.tips_num = 1
        self.test_long_range_attack()
        self.test_invalidateblock()



    def test_long_range_attack(self):
        self.log.info(sys._getframe().f_code.co_name)
        blockcount = self.node0.getblockcount()
        # 分割网络，制造分叉
        self.split_network()
        for i in range(9):
            for n in range(self.num_nodes):
                self.nodes[n].generate(25)
            self.log.info('mainchain block height {}'.format(self.node2.getblockcount()))

        self.node2.generate(20)
        self.node3.generate(20)
        self.sync_all([self.nodes[:2], self.nodes[2:]])

        # 合并分叉
        self.join_network(timeout=120)

        # assert
        tips = self.node0.getchaintips()
        assert_equal(len(tips), 2)
        tips = self.node3.getchaintips()
        assert_equal(len(tips), 1)
        assert_equal(self.node2.getblockcount(), blockcount + 490)
        assert_equal(self.node0.getblockcount(), blockcount + 490)

    def test_invalidateblock(self):
        bad_hash = self.node0.getblockhash(5)
        best_hash = self.node0.getbestblockhash()
        blockcount = self.node0.getblockcount()
        self.node0.invalidateblock(bad_hash)
        self.node0.generate(5)
        self.node0.reconsiderblock(bad_hash)
        print(self.node0.getchaintips())

        assert_equal(self.node0.getbestblockhash(),best_hash)
        assert_equal(self.node0.getblockcount(), blockcount)




    # def test_invalidateblock(self):
    #     '''
    #     侧链某个区块被标记为失效时，再重新打包
    #     :return:
    #     '''
    #     self.log.info(sys._getframe().f_code.co_name)
    #     self.log.info("snode0 mortgage coins num {}".format(len(self.snode0.listmortgagecoins())))
    #     self.snode0.generate(1)
    #     assert_equal(self.snode0.getrawmempool(), [])
    #     hash = self.snode0.getbestblockhash()
    #     block_height = self.snode0.getblockcount()
    #     # self.log.info("before invalidateblock,besthash {} , heiht {},balance {}".format(hash, block_height,self.snode0.getbalance()))
    #     addr = self.snode0.getnewaddress()
    #     txid1 = self.snode0.sendtoaddress(addr, 10)
    #     txid2 = self.snode0.sendtobranchchain('main', self.node0.getnewaddress(), 10)['txid']
    #     self.snode0.generate(7)
    #     self.node0.generate(1)
    #     self.snode0.generate(3)
    #     bad_hash = self.snode0.getblockhash(block_height + 1)
    #     before_work = int(self.snode0.getchaintipwork(), 16)
    #     before_height = self.snode0.getblockcount()
    #     before_besthash = self.snode0.getbestblockhash()
    #     for n in self.sidenodes:
    #         self.log.info("before invalidateblock {},{}".format(n.getblockcount(), int(n.getchaintipwork(), 16)))
    #     self.snode0.invalidateblock(bad_hash)
    #     for n in self.sidenodes:
    #         self.log.info("after invalidateblock {},{}".format(n.getblockcount(), int(n.getchaintipwork(), 16)))
    #     # self.log.info("after invalidateblock,balance {}".format(self.snode0.getbalance()))
    #     new_height = self.snode0.getblockcount()
    #     new_hash = self.snode0.getbestblockhash()
    #     if (new_height != block_height or new_hash != hash):
    #         raise AssertionError("Wrong tip for snode0, hash %s, height %d" % (new_hash, new_height))
    #     assert txid1 in self.snode0.getrawmempool()
    #     assert txid2 in self.snode0.getrawmempool()
    #     assert len(self.snode0.getrawmempool()) == 2
    #     self.snode0.generate(7)
    #     assert_equal(self.snode0.getblockcount(), new_height + 7)
    #     self.node0.generate(1)
    #     for i in range(3):
    #         # avoid generate timeout
    #         for j in range(8):
    #             self.snode0.generate(1)
    #     self.node0.generate(1)
    #     self.sync_all()
    #     for n in self.sidenodes:
    #         self.log.info(
    #             "before test_getbranchchainheight {},{}".format(n.getblockcount(), int(n.getchaintipwork(), 16)))
    #     gens = self.make_more_work_than(0, 1, True)
    #     self.node0.generate(1)
    #     self.test_getbranchchainheight()
    #     assert_raises_rpc_error(-25, 'txn-already-in-records', self.snode0.rebroadcastchaintransaction, txid2)
    #     self.sync_all()
    #     # self.sync_all([self.sidenodes])
    #     self.test_getbranchchainheight()
    #     self.log.info("node0 mempool size {}".format(self.node0.getmempoolinfo()['size']))
    #     # todo: we need reconsiderblock previous tip
    #     besthash = self.snode0.getbestblockhash()
    #     block_height = self.snode0.getblockcount()
    #     self.snode0.reconsiderblock(bad_hash)
    #     if int(self.snode0.getchaintipwork(), 16) <= before_work:
    #         assert_equal(self.snode0.getblockcount(), before_height)
    #         assert_equal(self.snode0.getbestblockhash(), before_besthash)
    #     else:
    #         assert_equal(self.snode0.getblockcount(), block_height)
    #         assert_equal(self.snode0.getbestblockhash(), besthash)


if __name__ == '__main__':
    MovingCheckPointTest().main()

