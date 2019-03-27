#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
测试：
测试涉及到侧链的rpc接口
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


class SendToBranchchainTest(MagnaChainTestFramework):
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
        self.test_getbranchchaintransaction()
        self.test_makebranchtransaction()
        self.test_mortgageminebranch()
        self.test_rebroadcastchaintransaction()
        self.test_resendbranchchainblockinfo()
        self.test_submitbranchblockinfo()
        self.test_invalidateblock()

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

    def test_getbranchchaintransaction(self):
        self.log.info(sys._getframe().f_code.co_name)
        txid = self.node0.getbranchchaininfo(self.sidechain_id)['txid']
        assert_raises_rpc_error(-25, "Invalid branch transaction.", self.node0.getbranchchaintransaction, txid)
        txid = self.snode0.sendtoaddress(self.snode0.getnewaddress(), 1)
        assert_raises_rpc_error(-25, "Load transaction sendinfo fail.", self.node0.getbranchchaintransaction, txid)
        assert_raises_rpc_error(-25, "Load transaction sendinfo fail.", self.snode0.getbranchchaintransaction, txid)
        txid = self.node0.sendtobranchchain(self.sidechain_id, self.snode0.getnewaddress(), 1000)['txid']
        self.node0.generate(8)
        ret = self.node0.getbranchchaintransaction(txid)
        print(ret)
        assert_equal(ret['txid'], txid)
        assert_equal(ret['confirmations'], 8)
        txid = self.snode0.sendtobranchchain('main', self.node0.getnewaddress(), 1)['txid']
        self.snode0.generate(7)
        self.node0.generate(1)
        self.snode0.generate(1)
        ret = self.snode0.getbranchchaintransaction(txid)
        print(ret)
        assert_equal(ret['txid'], txid)
        assert_equal(ret['confirmations'], 8)

    def test_makebranchtransaction(self):
        self.log.info(sys._getframe().f_code.co_name)
        fake_hexdata = "718af6082213655da6910d7272afd1d87755c66b155729491d6f5cb79ddee612"
        assert_raises_rpc_error(-4, "DecodeHexTx tx hex fail.", self.node0.makebranchtransaction, fake_hexdata)
        txid = self.node0.sendtoaddress(self.node0.getnewaddress(), 1)
        hex_data = self.node0.gettransaction(txid)['hex']
        assert_raises_rpc_error(-4, "Transaction is not a valid chain trans step1", self.node0.makebranchtransaction,
                                hex_data)

        txid = self.node0.sendtobranchchain(self.sidechain_id, self.snode0.getnewaddress(), 1)['txid']
        hex_data = self.node0.gettransaction(txid)['hex']
        assert_raises_rpc_error(-4, "Target branch id is not valid", self.node0.makebranchtransaction,
                                hex_data)
        assert_raises_rpc_error(-4, "accept to memory pool fail: ", self.snode0.makebranchtransaction,
                                hex_data)
        txid = self.snode0.sendtobranchchain('main', self.node0.getnewaddress(), 1)['txid']
        hex_data = self.snode0.gettransaction(txid)['hex']
        assert_raises_rpc_error(-4, "Target branch id is not valid", self.snode0.makebranchtransaction,
                                hex_data)
        assert_raises_rpc_error(-4, "accept to memory pool fail: Get transstep2 blockdata fail",
                                self.node0.makebranchtransaction,
                                hex_data)
        self.snode0.generate(7)
        self.node0.generate(1)
        self.snode0.generate(1)
        print(self.node0.getrawmempool(True))
        txs = self.nodes[0].getrawmempool(True)
        for txid in txs:
            if txs[txid]['version'] == 7:
                print(txid)
                break
        hex_data = self.node0.gettransaction(txid)['hex']
        assert_raises_rpc_error(-4, "Transaction is not a valid chain trans step1", self.node0.makebranchtransaction,
                                hex_data)

    def test_mortgageminebranch(self):
        self.log.info(sys._getframe().f_code.co_name)
        ct = Contract(self.snode0, self.options.tmpdir)
        assert_raises_rpc_error(-25, 'Only in main chain can mortgage coin for mining branch!',
                                self.snode0.mortgageminebranch, self.sidechain_id, 10000, self.snode0.getnewaddress())
        assert_raises_rpc_error(-5, 'Invalid magnachain keyid',
                                self.node0.mortgageminebranch, self.sidechain_id, 10000, ct.contract_id)
        self.node0.mortgageminebranch(self.sidechain_id, 10000, self.snode0.getnewaddress())
        

    def test_rebroadcastchaintransaction(self):
        # to sidechain
        self.log.info(sys._getframe().f_code.co_name)
        self.node0.generate(8) #todo 需要注释看一下,这个应该没什么关系的
        self.snode0.generate(2)
        assert_equal(len(self.snode0.getrawmempool()), 0)  # ensure mempool is empty
        txid = self.node0.sendtoaddress(self.node0.getnewaddress(), 1)
        assert_raises_rpc_error(-25, 'Load transaction sendinfo fail.',
                                self.node0.rebroadcastchaintransaction, txid)
        txid = self.snode0.sendtoaddress(self.snode0.getnewaddress(), 1)
        assert_raises_rpc_error(-25, 'Load transaction sendinfo fail.',
                                self.snode0.rebroadcastchaintransaction, txid)
        self.snode0.generate(1)
        assert_equal(len(self.snode0.getrawmempool()), 0)  # ensure mempool is empty
        txid = self.node0.sendtobranchchain(self.sidechain_id, self.snode0.getnewaddress(), 1000)['txid']
        self.node0.generate(1)
        assert_raises_rpc_error(-25, 'can not broadcast because no enough confirmations',
                                self.node0.rebroadcastchaintransaction, txid)
        self.node0.generate(7)
        assert_equal(len(self.snode0.getrawmempool()), 1)  # here we are
        txs = self.snode0.getrawmempool(True)
        for tid in txs:
            assert_equal(txs[tid]['version'], 7)  # here we are
        assert_raises_rpc_error(-25, 'txn-already-in-mempool', self.node0.rebroadcastchaintransaction, txid)
        self.snode0.generate(2)  # 注释后，会导致后面主链的某个generate报错，Branch contextual check block header fail

        # to mainchain
        self.node0.generate(2)
        assert_equal(len(self.node0.getrawmempool()), 0)  # ensure mempool is empty
        txid = self.snode0.sendtobranchchain('main', self.node0.getnewaddress(), 1)['txid']
        self.snode0.generate(1)
        assert_raises_rpc_error(-25, 'can not broadcast because no enough confirmations',
                                self.snode0.rebroadcastchaintransaction, txid)
        self.snode0.generate(6)
        self.node0.generate(1)
        self.snode0.generate(1)
        assert_equal(len(self.node0.getrawmempool()), 2)  # here we are,one for header,one for transaction
        assert_raises_rpc_error(-25, 'txn-already-in-mempool', self.snode0.rebroadcastchaintransaction, txid)
        self.node0.generate(1)
        assert_raises_rpc_error(-25, 'txn-already-in-records', self.snode0.rebroadcastchaintransaction, txid)

    def test_resendbranchchainblockinfo(self):
        '''
        Resend branch chain block info by height
        :return:
        '''
        self.log.info(sys._getframe().f_code.co_name)
        self.node0.generate(1)
        assert_equal(self.node0.getrawmempool(), [])
        assert_raises_rpc_error(-32603, 'Can not call this RPC in main chain', self.node0.resendbranchchainblockinfo,
                                1)
        assert_raises_rpc_error(-32603, 'Params[0] is a invalid number', self.snode0.resendbranchchainblockinfo,
                                "number")
        assert_raises_rpc_error(-32603, 'Invalid block height', self.snode0.resendbranchchainblockinfo,
                                -1)
        assert_raises_rpc_error(-32603, 'Invalid block height',
                                self.snode0.resendbranchchainblockinfo,
                                self.snode0.getblockcount() + 1)
        assert 'blockheader info has include before' in self.snode0.resendbranchchainblockinfo(
            self.snode0.getblockcount())
        assert_equal(len(self.node0.getrawmempool()), 0)  # 因为之前已经提交过了，所以节点会拒绝该侧链头

    def test_submitbranchblockinfo(self):
        '''
        submitbranchblockinfo "CTransaction hex data"
        Include branch block data to a transaction, then send to main chain
        :return:
        '''
        self.log.info(sys._getframe().f_code.co_name)
        txid = self.node0.sendtoaddress(self.node0.getnewaddress(),1)
        tx_hex = self.node0.getrawtransaction(txid)
        assert_raises_rpc_error(-32602, 'Invalid transaction data',self.node0.submitbranchblockinfo,tx_hex)
        assert_raises_rpc_error(-32602, 'This rpc api can not be called in branch chain',
                                self.snode0.submitbranchblockinfo, tx_hex)
        tx = CTransaction()
        tx.nVersion = 9
        tx.rehash()
        assert_raises_rpc_error(-4, 'DecodeHexTx tx hex fail',
                                self.node0.submitbranchblockinfo, tx.hash)

    def test_invalidateblock(self):
        '''
        侧链某个区块被标记为失效时，再重新打包
        :return:
        '''
        self.log.info(sys._getframe().f_code.co_name)
        self.log.info("snode0 mortgage coins num {}".format(len(self.snode0.listmortgagecoins())))
        self.snode0.generate(1)
        assert_equal(self.snode0.getrawmempool(),[])
        hash = self.snode0.getbestblockhash()
        block_height = self.snode0.getblockcount()
        self.log.info("before invalidateblock,besthash {} , heiht {},balance {}".format(hash, block_height,self.snode0.getbalance()))
        addr = self.snode0.getnewaddress()
        txid1 = self.snode0.sendtoaddress(addr,10)
        txid2 = self.snode0.sendtobranchchain('main',self.node0.getnewaddress(),10)['txid']
        self.snode0.generate(7)
        self.node0.generate(1)
        self.snode0.generate(3)
        bad_hash = self.snode0.getblockhash(block_height + 1)
        self.snode0.invalidateblock(bad_hash)
        self.log.info("after invalidateblock,balance {}".format(self.snode0.getbalance()))
        new_height = self.snode0.getblockcount()
        new_hash = self.snode0.getbestblockhash()
        if (new_height != block_height or new_hash != hash):
            raise AssertionError("Wrong tip for snode0, hash %s, height %d"%(new_hash,new_height))
        assert txid1 in self.snode0.getrawmempool()
        assert txid2 in self.snode0.getrawmempool()
        assert len(self.snode0.getrawmempool()) == 2
        self.snode0.generate(7)
        assert_equal(self.snode0.getblockcount(),new_height + 7)
        self.node0.generate(1)
        txids = self.node0.getrawmempool()
        self.log.info("node0 mempool size {}".format(len(txids)))
        print(txids)
        # self.snode0.rebroadcastchaintransaction(txid2)
        self.snode0.generate(8)
        self.node0.generate(1)
        self.test_getbranchchainheight()
        self.snode0.rebroadcastchaintransaction(txid2)
        self.test_getbranchchainheight()
        self.log.info("node0 mempool size {}".format(self.node0.getmempoolinfo()['size']))
        # todo: we need reconsiderblock previous tip



if __name__ == '__main__':
    SendToBranchchainTest().main()
