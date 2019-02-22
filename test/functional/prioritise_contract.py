#!/usr/bin/env python3
# Copyright (c) 2015-2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the prioritisetransaction mining RPC."""
"""
该用例测试以下情况：
1.调整无依赖的合约交易的优先级
2.调整有依赖的合约交易的优先级，依赖方式为调用同一合约
3.调整有依赖的合约交易的优先级，依赖方式为输入有依赖
4.调整有依赖的合约交易的优先级，依赖方式为输出有依赖
5.调整有依赖的合约交易的优先级，依赖方式为跨合约调用
"""

from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import assert_equal, generate_contract, gen_lots_of_contracts, sync_blocks
from test_framework.mininode import COIN, MAX_BLOCK_BASE_SIZE

MAX_CONTRACT_NUM = 231  # 一个区块最多可以包含多少个发布合约的交易
MAX_CONTRACT_CALL_NUM = 1700  # 一个区块最多可以包含多少个调用合约的交易


class PrioritiseContractTest(MagnaChainTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 2
        self.extra_args = [["-printpriority=1", "-powtargetspacing=15"], ["-printpriority=1", "-powtargetspacing=15"]]

    def setup_nodes(self):
        self.add_nodes(self.num_nodes, self.extra_args, timewait=9000)
        self.start_nodes()

    def run_test(self):
        self.relayfee = self.nodes[0].getnetworkinfo()['relayfee']
        base_fee = self.relayfee * 1000

        # prepare
        node0, node1 = self.nodes[0:2]
        node0.generate(2)  # make some coins
        self.sync_all()
        contract = generate_contract(self.options.tmpdir)
        # ''''
        # 测试发布合约的交易
        self.log.info("Test contract publish transaction")
        contract = generate_contract(self.options.tmpdir)
        infos = gen_lots_of_contracts(node0, contract, MAX_CONTRACT_NUM)
        node0.prioritisetransaction(txid=infos[-1]['txid'], fee_delta=int(3 * base_fee * COIN))
        mempool_len = len(node0.getrawmempool())
        print(mempool_len)
        print(len(node0.getrawmempool()), len(node1.getrawmempool()))
        self.sync_all()
        print(len(node0.getrawmempool()), len(node1.getrawmempool()))
        node0.generate(1)
        sync_blocks(self.nodes)
        block_txnum = int(node0.getblock(node0.getbestblockhash())['tx_num'])
        mempool = node0.getrawmempool()
        print(block_txnum, len(node0.getrawmempool()), len(node1.getrawmempool()))
        assert infos[-1]['txid'] not in mempool  # should be mined
        assert_equal(len(node0.getrawmempool()),
                     MAX_CONTRACT_NUM - block_txnum + 1)  # can not locate the tx index in infos
        node0.generate(1)  # clear mempool
        sync_blocks(self.nodes, timeout=600)
        assert_equal(len(node0.getrawmempool()), 0)  # make sure mempool is clean

        # 测试调用合约的交易
        self.log.info("Test call contract transaction")
        # node0_call_contract = caller_factory(self, infos[-1]['address'], node0.getnewaddress())
        infos = infos + gen_lots_of_contracts(node0, contract, MAX_CONTRACT_CALL_NUM - MAX_CONTRACT_NUM)
        print(len(node0.getrawmempool()), len(node1.getrawmempool()))
        self.sync_all(timeout=6000, show_max_height=True)
        print(len(node0.getrawmempool()), len(node1.getrawmempool()))
        node0.generate(10)
        print("sync blocks....")
        sync_blocks(self.nodes, timeout=6000)
        assert_equal(len(node0.getrawmempool()), 0)  # make sure mempool is clean
        print("info size: %d"%(len(infos)))
        txids = [node0.callcontract(True, 0, infos[i]['address'], node0.getnewaddress(), 'payable')['txid'] for i in range(MAX_CONTRACT_CALL_NUM)]
        node0.prioritisetransaction(txid=txids[-1], fee_delta=int(3 * base_fee * COIN))
        print("mempool size after change priority: %d"%(len(node0.getrawmempool())))
        node0.generate(1)
        print("mempool size after generate block: %d"%(len(node0.getrawmempool())))
        self.sync_all(timeout=6000, show_max_height=True)
        block_txnum = int(node0.getblock(node0.getbestblockhash())['tx_num'])
        print(block_txnum, len(node0.getrawmempool()))
        mempool = node0.getrawmempool()
        assert txids[-1] not in mempool  # should be mined
        # assert infos[-2]['txid'] in mempool
        assert_equal(len(node0.getrawmempool()), 1)  # can not locate the tx index in infos
        node0.generate(1)  # clear mempool
        sync_blocks(self.nodes, timeout=600)
        assert len(node0.getrawmempool()) == 0  # make sure mempool is clean

        # 测试有依赖的合约交易的优先级，依赖方式为调用同一合约
        publish_transaction = gen_lots_of_contracts(node0, contract, 1)
        print(publish_transaction)
        txid = node0.callcontract(True, 0, publish_transaction[0]['address'], node0.getnewaddress(), 'payable')['txid']
        mempool_old = node0.getrawmempool()
        node0.prioritisetransaction(txid=txid, fee_delta=int(30 * base_fee * COIN))
        mempool_new = node0.getrawmempool()
        # print(mempool_old,mempool_new)
        assert mempool_old == mempool_new  # 因为有依赖关系，所以内存池顺序应该是一致的
        node0.generate(2)
        sync_blocks(self.nodes, timeout=600)

        to = node0.getnewaddress()
        txid1 = node0.callcontract(True, 1, publish_transaction[0]['address'], node0.getnewaddress(), 'payable')['txid']
        print("txid1:", txid1)
        txid2 = node0.callcontract(True, 0, publish_transaction[0]['address'], node0.getnewaddress(), 'sendCoinTest',to,1)['txid']
        print("txid2:", txid2)
        mempool_old = node0.getrawmempool()
        node0.prioritisetransaction(txid=txid2, fee_delta=int(30 * base_fee * COIN))
        mempool_new = node0.getrawmempool()
        assert mempool_old == mempool_new  # 因为有依赖关系，所以内存池顺序应该是一致的
        self.sync_all(timeout=6000, show_max_height=True)
        assert mempool_old == node1.getrawmempool()  # prioritisetransaction不会影响其他节点交易的优先级
        node0.generate(2)
        assert node0.getbalanceof(to) == 1
        # '''
        # 调整有依赖的合约交易的优先级，依赖方式为输入有依赖
        node0.sendtoaddress(node1.getnewaddress(), 100)
        self.sync_all()
        node0.generate(1)
        print(node1.getbalance(), node0.getbalance())
        publish_transaction = gen_lots_of_contracts(node0, contract, 1)
        self.sync_all()
        to = node0.getnewaddress()
        txid3 = node1.callcontract(True, 1, publish_transaction[0]['address'], node1.getnewaddress(), 'payable')['txid']
        txid4 = node1.sendtoaddress(to, 1)
        mempool_old = node1.getrawmempool()
        node1.prioritisetransaction(txid=txid4, fee_delta=int(30 * base_fee * COIN))
        mempool_new = node1.getrawmempool()
        assert mempool_old == mempool_new  # 因为有依赖关系，所以内存池顺序应该是一致的
        self.sync_all(timeout=6000, show_max_height=True)
        assert mempool_old == node0.getrawmempool()  # prioritisetransaction不会影响其他节点交易的优先级
        node0.generate(2)
        assert node0.getbalanceof(to) == 1


if __name__ == '__main__':
    PrioritiseContractTest().main()
