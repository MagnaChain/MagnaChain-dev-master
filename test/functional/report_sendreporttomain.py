#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
测试：
举报机制。在支链发起举报某个交易，或者举报coinbase交易
测试点：
1.主链举报：无效
2.侧链举报：
    a.最大举报高度限制
    b.非法的块与交易
    块类型：
        c.正常块
        d.分叉上的块
        e.被无效化的块
        f.孤儿块
    交易类型：
        g.正常交易
        h.合约交易
        i.抵押交易
        j.coinbase交易
        k.权益交易
        l.跨链交易
    m.举报与赎回
"""
# Avoid wildcard * imports if possible
from decimal import Decimal

from test_framework.test_framework import MagnaChainTestFramework
from test_framework.mininode import MINER_REWARD
from test_framework.contract import Contract
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
    wait_until,
)

REDEEM_SAFE_HEIGHT = 20
REPORT_LOCK_COIN_HEIGHT = 10


def get_mempool_size(n):
    return len(n.getrawmempool())


class SendReportTest(MagnaChainTestFramework):
    # Each functional test is a subclass of the MagnaChainTestFramework class.

    # Override the set_test_params(), add_options(), setup_chain(), setup_network()
    # and setup_nodes() methods to customize the test setup as required.

    def set_test_params(self):
        """Override test parameters for your individual test.

        This method must be overridden and num_nodes must be exlicitly set."""
        self.setup_clean_chain = True
        self.num_nodes = 2
        self.extra_args = [
            ['-regtestrsheight={}'.format(REDEEM_SAFE_HEIGHT), '-regtestrooheight={}'.format(REPORT_LOCK_COIN_HEIGHT)],
            ['-regtestrsheight={}'.format(REDEEM_SAFE_HEIGHT), '-regtestrooheight={}'.format(REPORT_LOCK_COIN_HEIGHT)]]

        '''
        self.num_sidenodes here is setting sidechain nodes num，just like self.num_nodes
        and the self.sidenodes like self.nodes
        '''
        self.num_sidenodes = 2
        self.side_extra_args = [
            ['-regtestrsheight={}'.format(REDEEM_SAFE_HEIGHT), '-regtestrooheight={}'.format(REPORT_LOCK_COIN_HEIGHT)],
            ['-regtestrsheight={}'.format(REDEEM_SAFE_HEIGHT), '-regtestrooheight={}'.format(REPORT_LOCK_COIN_HEIGHT)]]

    def run_test(self):
        """Main test logic"""
        self.sync_all([self.sidenodes])
        for n in self.sidenodes:
            n.generate(1)
            self.sync_all([self.sidenodes])
            self.sync_all()
        assert_equal(get_mempool_size(self.node0), 2)
        self.node0.generate(2)
        self.sync_all()
        self.import_prikey()
        origin_mortgage = 50000
        s0_besthash = self.snode0.getbestblockhash()

        self.log.info('start test')
        self.log.info('run illegal test ')
        addr = self.node0.getnewaddress()
        txid = self.node0.sendtoaddress(addr, 100)
        blockhash = self.node0.generate(2)[1]
        assert_raises_rpc_error(-32603, "Can not call this RPC in main chain", self.node0.sendreporttomain, blockhash,
                                txid)
        # self.node0.sendreporttomain(blockhash,txid)
        assert_raises_rpc_error(-5, "Block not found", self.snode0.sendreporttomain, blockhash, txid)
        assert_raises_rpc_error(-32603, "block did not contain the reported transaction", self.snode0.sendreporttomain,
                                s0_besthash, txid)
        self.node0.generate(2)
        for i in range(10):
            self.snode0.generate(2)
        self.snode0.generate(1)
        self.sync_all([self.sidenodes])
        self.sync_all()
        assert_raises_rpc_error(-32603, "Report block is too old", self.snode0.sendreporttomain, s0_besthash, txid)
        self.log.info('illegal test passed')

        # 举报某个交易
        s0addr = self.snode0.getnewaddress()
        txid = self.snode0.sendtoaddress(s0addr, 1)
        mempool_size = get_mempool_size(self.node0)
        blockhash, = self.snode0.generate(1)
        self.sync_all([self.sidenodes])
        wait_until(lambda: get_mempool_size(self.node0) > mempool_size, timeout=10)
        self.sync_all()
        assert_equal(mempool_size + 1, get_mempool_size(self.node0))
        self.node0.generate(2)
        self.sync_all()
        report_txid = self.snode0.sendreporttomain(blockhash, txid)['txid']
        '''
        举报交易需要打包进主链的区块后，才会生效
        生效之后的变化：
        1.侧链对应的挖矿币会被锁定，不能用于被挖矿，赎回(目前逻辑是要举报用户自己手动锁定，是一个非必要条件，故本用例中都不进行锁定)
        2.主链的侧链记录会disconnect掉被举报的区块
        3.再被证明之前，不接受该侧链的任何交易，包括区块头
        '''
        s0_blockcount = self.node0.getbranchchainheight(self.sidechain_id)['height']
        self.node0.generate(2)
        self.sync_all()
        assert_equal(self.node0.getbranchchainheight(self.sidechain_id)['height'], s0_blockcount - 1)
        self.generate_but_not_accept()
        # 尝试赎回
        self.clear_mempool(self.node0)
        balance = self.node0.getbalance()
        results = []
        self.sync_all()  # TODO: make sure node1 have all branch header tx in mempool.(to be optimize)
        self.snode1.generate(20)
        self.sync_all([self.sidenodes])
        print(get_mempool_size(self.node0))
        for i in range(10):
            result = self.snode0.redeemmortgagecoinstatement(self.mortgage_coin())
            results.append(result['txid'])
            print("result:", result)
            assert_equal(len(self.snode0.listmortgagecoins()), 10 - i - 1)
        assert_raises_rpc_error(-32603, 'no address with enough coins', self.snode0.generate, 1)
        self.sync_all([self.sidenodes]) # sync mempool to snode1
        besthash = self.snode1.getbestblockhash()
        best_height = self.snode1.getblockcount()
        badhash = self.snode1.generate(7)[0]
        print(get_mempool_size(self.node0))
        self.sync_all([self.sidenodes])
        self.sync_all()
        # assert_equal(24, len(self.node0.getrawmempool()))  # side chain gen block count
        fees = 0
        txs = self.node0.getrawmempool(True)
        for txid in txs:
            fees += Decimal(txs[txid]['fee'])
        self.node0.generate(1)
        self.sync_all()
        self.snode1.generate(1)  #主链的balance变少了
        self.log.info("after node0 gen blocks,mortgage coins should be redeemed")
        self.sync_all([self.sidenodes])
        self.sync_all()
        txs = self.node0.getrawmempool(True)
        for txid in txs:
            fees += Decimal(txs[txid]['fee'])
        self.node0.generate(2)
        self.sync_all()
        self.node0.generate(2)
        print("self.node0.getbalance", self.node0.getbalance(), "balance", balance,
            "self.node0.getbalance() - balance", self.node0.getbalance() - balance,
            "origin_mortgage", origin_mortgage, "fees", fees)
        print(self.node0.getbalance() - balance - 4* MINER_REWARD, fees)
        '''
        这里有些问题：
        1.举报完成后，按理主链是不会更新支链的高度信息的，所有，后面提交的区块头是否应该接受到主链的内存池里边？
        2.同理，由于高度不更新，赎回交易到主链上不会被接受，因为高度不一致，所以到最后，主链的balance基本只是挖矿奖励与手续费，不应该有其他
        '''
        assert_equal(self.node0.getbalance() - balance - 4* MINER_REWARD, fees)

    def mortgage_coin(self, spentable=True):
        '''
        获取有一个已成熟的抵押币
        :param spentable:
        :return:
        '''
        for tx in self.snode0.listmortgagecoins():
            if tx['confirmations'] >= 20:
                if spentable:
                    return tx['txid']
            if spentable:
                continue
            return tx['txid']
        else:
            self.log.info("mortgage_coin not found,generate")
            self.snode1.generate(20)
            return self.mortgage_coin()

    def generate_but_not_accept(self):
        s0_blockcount = self.node0.getbranchchainheight(self.sidechain_id)
        self.snode0.generate(1)
        self.sync_all([self.sidenodes])
        self.sync_all()
        self.node0.generate(1)
        self.sync_all()
        assert_equal(self.node0.getbranchchainheight(self.sidechain_id), s0_blockcount)

    def import_prikey(self):
        for item in self.snode0.listmortgagecoins():
            prikey = self.snode0.dumpprivkey(item['address'])
            self.node0.importprivkey(prikey)

    def clear_mempool(self,n):
        n.generate(2)
        assert_equal(get_mempool_size(n),0)

if __name__ == '__main__':
    SendReportTest().main()
