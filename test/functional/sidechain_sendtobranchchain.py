#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
测试：
testcase for sendtobranchchain rpc
"""
# Imports should be in PEP8 ordering (std library first, then third party
# libraries then local imports).
from decimal import Decimal

# Avoid wildcard * imports if possible
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.mininode import MINER_REWARD
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

        '''
        self.num_sidenodes here is setting sidechain nodes num，just like self.num_nodes
        and the self.sidenodes like self.nodes
        '''
        self.num_sidenodes = 2
        self.rpc_timewait = 900

    def run_test(self):
        """Main test logic"""
        self.sync_all([self.sidenodes])
        node = self.nodes[0]
        sidechain_id = self.sidechain_id
        saddr = self.sidenodes[0].getnewaddress()
        assert_raises_rpc_error(-25, "Load transaction sendinfo fail", node.sendtobranchchain,
                                sidechain_id[:len(sidechain_id) - 5] + 'abcde', saddr, 1)
        assert_raises_rpc_error(-4, "can not send to this chain", node.sendtobranchchain,
                                "main", node.getnewaddress(), 1)
        assert_raises_rpc_error(-4, "can not send to this chain", self.sidenodes[0].sendtobranchchain,
                                sidechain_id, saddr, 0)
        assert_raises_rpc_error(-3, "Invalid amount", node.sendtobranchchain, sidechain_id, saddr, 10000000000)
        assert_raises_rpc_error(-6, "Insufficient funds", node.sendtobranchchain, sidechain_id, saddr, 100000000)
        assert_raises_rpc_error(-3, "Amount out of range", node.sendtobranchchain, sidechain_id, saddr, -1)
        txid = node.sendtobranchchain(sidechain_id, saddr, 0)['txid']
        txid1 = node.sendtobranchchain(sidechain_id, saddr, Decimal("0.00000009").quantize(Decimal("0.000000")))['txid']
        self.sync_all()
        assert txid in node.getrawmempool()
        assert txid1 in node.getrawmempool()
        node.generate(8)
        self.sidenodes[0].generate(2)
        assert_equal(self.sidenodes[0].getrawmempool(), []) #ensure mempool is empty
        self.sync_all([self.sidenodes])
        assert_equal(self.sidenodes[0].getbestblockhash(), self.sidenodes[1].getbestblockhash())
        self.sync_all()
        assert_equal(self.nodes[0].getbestblockhash(), self.nodes[1].getbestblockhash())

        # main to side
        txid = node.sendtobranchchain(sidechain_id, saddr, 100)['txid']
        node.generate(8) #sync to sidechain
        self.sync_all()
        assert_equal(len(self.sidenodes[0].getrawmempool()), 1)
        self.sync_all([self.sidenodes])
        print(self.get_txfee(self.sidenodes[0].getrawmempool()[0], self.sidenodes[0]))
        txfee = self.sidenodes[0].getrawmempool(True)[self.sidenodes[0].getrawmempool()[0]]['fee']
        side_balance = self.sidenodes[0].getbalance()
        side_balance1 = self.sidenodes[1].getbalance()
        self.sync_all([self.sidenodes])
        assert_equal(self.sidenodes[0].getrawmempool(), self.sidenodes[1].getrawmempool())#ensure transactions are synced
        self.sidenodes[0].generate(2)
        self.sync_all([self.sidenodes])
        # ensure MGC is reached,include the fee
        assert_equal(self.sidenodes[0].getbalanceof(saddr), 100.0000000)
        assert_equal(self.sidenodes[1].getbalanceof(saddr), 100.0000000)
        assert_equal(self.sidenodes[0].getbalance(),
                     Decimal(side_balance + Decimal(100) + Decimal(txfee)).quantize(Decimal("0.000000")))
        # make sure blocks are synced
        assert_equal(self.sidenodes[1].getbalance(), side_balance1)
        assert_equal(self.sidenodes[0].getbestblockhash(), self.sidenodes[1].getbestblockhash())
        self.sidenodes[0].generate(2)
        assert_equal(self.sidenodes[0].getrawmempool(), [])
        self.sync_all([self.sidenodes])
        assert_equal(self.sidenodes[0].getbestblockhash(), self.sidenodes[1].getbestblockhash())
        self.sync_all()
        assert_equal(self.nodes[0].getbestblockhash(), self.nodes[1].getbestblockhash())

        # confirm fee in another node with different generate
        txid = node.sendtobranchchain(sidechain_id, saddr, 100)['txid']
        node.generate(8)
        self.sync_all()
        assert_equal(len(self.sidenodes[0].getrawmempool()), 1)
        print(len(self.sidenodes[0].getrawmempool()), self.sidenodes[0].getrawmempool())
        print(len(self.sidenodes[1].getrawmempool()), self.sidenodes[1].getrawmempool())
        self.sync_all([self.sidenodes])
        txfee = self.sidenodes[0].getrawmempool(True)[self.sidenodes[0].getrawmempool()[0]]['fee']
        side_balance = self.sidenodes[0].getbalance()
        side_balance1 = self.sidenodes[1].getbalance()
        self.sync_all([self.sidenodes])
        assert_equal(self.sidenodes[0].getrawmempool(), self.sidenodes[1].getrawmempool())
        self.sidenodes[1].generate(2)  # ensure another node generate blocks is work
        self.sync_all([self.sidenodes])
        assert_equal(self.sidenodes[0].getbalanceof(saddr), 200.0000000)
        assert_equal(self.sidenodes[1].getbalanceof(saddr), 200.0000000)
        assert_equal(self.sidenodes[0].getbalance(), Decimal(side_balance + Decimal(100)).quantize(Decimal("0.000000")))
        assert_equal(self.sidenodes[1].getbalance(), side_balance1 + Decimal(txfee))
        assert_equal(self.sidenodes[0].getbestblockhash(), self.sidenodes[1].getbestblockhash())

        # side to main
        self.sync_all()
        node.generate(2)
        self.sync_all()
        side_balance = self.sidenodes[0].getbalance()
        balance = node.getbalance()
        addr = node.getnewaddress()
        txid = self.sidenodes[0].sendtobranchchain("main", addr, side_balance - 30)['txid']
        # ensure sendtobranchchain tracsaction sync to mainchain
        assert txid in self.sidenodes[0].getrawmempool()
        self.sidenodes[0].generate(7)
        assert txid not in self.sidenodes[0].getrawmempool()
        node.generate(2)
        self.sync_all()
        total_fee = Decimal('0')
        self.sidenodes[0].generate(1)
        # 侧链的区块头是一个交易，侧转主的是一个交易，所以主链的内存池会有2个交易
        assert len(node.getrawmempool()) == 2
        txs = node.getrawmempool(True)
        for txid in txs:
            print(txs[txid]['version'],txs[txid]['fee'])
            if txs[txid]['version'] == 7:
                # static const int32_t TRANS_BRANCH_VERSION_S2 = 7;// 跨链交易的接收链方
                print("Decimal(txs[txid]['fee']):",Decimal(txs[txid]['fee']))
                total_fee += Decimal(txs[txid]['fee'])
        node.generate(2)
        self.sync_all()
        print("total fee:",total_fee)
        print("diff:",node.getbalance() - (Decimal(balance) + Decimal(MINER_REWARD * 4 + Decimal(side_balance - 30) + total_fee)))
        assert_equal(node.getbalanceof(addr), side_balance - 30)
        assert_equal(node.getbalance(), Decimal(balance) + Decimal(MINER_REWARD * 4) + Decimal(side_balance - 30) + total_fee)

        # batch test
        transaction_num = 5000
        side_balance = self.sidenodes[0].getbalance()
        saddrs = [self.sidenodes[0].getnewaddress() for i in range(transaction_num)]
        for addr in saddrs:
            node.sendtobranchchain(sidechain_id, addr, 2)
        assert_equal(node.getmempoolinfo()['size'], transaction_num)
        while node.getmempoolinfo()['size'] > 0:
            node.generate(1)
            print('node mempool size left', node.getmempoolinfo()['size'])
        node.generate(8)
        self.sync_all()
        assert_equal(len(self.sidenodes[0].getrawmempool()), transaction_num)
        total_fee = 0
        txs = self.sidenodes[0].getrawmempool(True)
        for txid in txs:
            total_fee += Decimal(txs[txid]['fee'])
        while self.sidenodes[0].getmempoolinfo()['size'] > 0:
            self.sidenodes[0].generate(1)
            print('sidenode0 mempool left', self.sidenodes[0].getmempoolinfo()['size'])
        self.sidenodes[0].generate(1)
        assert_equal(len(self.sidenodes[0].getrawmempool()), 0)
        for addr in saddrs:
            assert_equal(self.sidenodes[0].getbalanceof(addr), 2)
        assert_equal(self.sidenodes[0].getbalance(), side_balance + transaction_num*2 + total_fee)

        balance = node.getbalance()
        side_balance = self.sidenodes[0].getbalance()
        saddrs = [node.getnewaddress() for i in range(transaction_num)]
        for addr in saddrs:
            self.sidenodes[0].sendtobranchchain("main", addr, 1)
        self.sidenodes[0].generate(10)
        assert_equal(len(node.getrawmempool()), transaction_num + 10)
        total_fee = 0
        txs = node.getrawmempool(True)
        for txid in txs:
            if txs[txid]['version'] == 7:
                total_fee += Decimal(txs[txid]['fee'])
        node.generate(2)
        self.sync_all()
        assert_equal(len(node.getrawmempool()), 0)
        for addr in saddrs:
            assert_equal(node.getbalanceof(addr), 1)
        assert_equal(node.getbalance(), balance + transaction_num + total_fee)
        assert_equal(self.sidenodes[0].getbalance(), side_balance - transaction_num)

        # delay generate test
        node.generate(2)
        self.sync_all()
        self.sidenodes[0].generate(2)
        self.sync_all([self.sidenodes])
        [node.sendtobranchchain(self.sidechain_id, self.sidenodes[0].getnewaddress(), 10) for i in range(1000)]
        self.sync_all()
        node.generate(8)
        self.sync_all()
        [self.sidenodes[0].sendtobranchchain('main', node.getnewaddress(), 1) for i in range(1000)]
        self.sync_all([self.sidenodes])
        self.sidenodes[0].generate(7)
        self.sync_all([self.sidenodes])
        node.generate(1)
        self.sync_all()
        self.sidenodes[0].generate(1)
        self.sync_all([self.sidenodes])
        self.sync_all()
        assert_equal(len(node.getrawmempool()), len(self.nodes[1].getrawmempool()))
        assert_equal(len(self.sidenodes[0].getrawmempool()), len(self.sidenodes[1].getrawmempool()))
        assert_equal(self.sidenodes[0].getbestblockhash(), self.sidenodes[1].getbestblockhash())
        assert_equal(self.nodes[0].getbestblockhash(), self.nodes[1].getbestblockhash())

        '''
        todo: 
        need to add lots of contract transactions to block,then sendtobranch
        because contract's vin is dynamic
        '''
        for ct in (Contract(self.sidenodes[0], self.options.tmpdir) for i in range(1000)):
            ct.call_payable(amount=2)
            ct.call_callOtherContractTest(ct.contract_id, 'callOtherContractTest', ct.contract_id, "contractDataTest",
                                          amount=0)
        self.sync_all([self.sidenodes])
        self.sidenodes[0].sendtobranchchain('main',node.getnewaddress(),self.sidenodes[0].getbalance() - 233)
        self.sync_all([self.sidenodes])
        self.sidenodes[0].generate(7)
        self.sync_all([self.sidenodes])
        assert_equal(len(self.sidenodes[0].getrawmempool()),0)
        assert_equal(len(self.sidenodes[1].getrawmempool()), 0)
        node.generate(2)
        self.sidenodes[0].generate(7)
        self.sync_all([self.sidenodes])
        node.generate(8)

        # self.sync_all(self.sidenodes)

    def get_txfee(self, txid, node=None):
        return Decimal(node.gettransaction(txid)['fee'])


if __name__ == '__main__':
    SendToBranchchainTest().main()
