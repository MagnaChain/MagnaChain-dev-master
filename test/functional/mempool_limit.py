#!/usr/bin/env python3
# Copyright (c) 2014-2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test mempool limiting together/eviction with the wallet."""

from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import *
from test_framework.contract import Contract


class MempoolLimitTest(MagnaChainTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1
        self.extra_args = [["-maxmempool=5", "-spendzeroconfchange=0"]]

    def run_test(self):
        txouts = gen_return_txouts()
        relayfee = self.nodes[0].getnetworkinfo()['relayfee']

        txids = []
        utxos = create_confirmed_utxos(relayfee, self.nodes[0], 91)

        # create a mempool tx that will be evicted
        us0 = utxos.pop()
        inputs = [{"txid": us0["txid"], "vout": us0["vout"]}]
        outputs = {self.nodes[0].getnewaddress(): 0.1}
        tx = self.nodes[0].createrawtransaction(inputs, outputs)
        self.nodes[0].settxfee(relayfee)  # specifically fund this tx with low fee
        txF = self.nodes[0].fundrawtransaction(tx)
        self.nodes[0].settxfee(0)  # return to automatic fee selection
        txFS = self.nodes[0].signrawtransaction(txF['hex'])
        txid = self.nodes[0].sendrawtransaction(txFS['hex'])

        relayfee = self.nodes[0].getnetworkinfo()['relayfee']
        base_fee = relayfee * 100
        for i in range(3):
            txids.append([])
            txids[i] = create_lots_of_big_transactions(self.nodes[0], txouts, utxos[30 * i:30 * i + 30], 30,
                                                       (i + 1) * base_fee)

        # by now, the tx should be evicted, check confirmation state
        assert (txid not in self.nodes[0].getrawmempool())
        txdata = self.nodes[0].gettransaction(txid)
        assert (txdata['confirmations'] == 0)  # confirmation should still be 0

        # 存在依赖关系的合约交易，内存池满时的情况
        self.nodes[0].generate(5)
        assert_equal(self.nodes[0].getrawmempool(), [])

        cts = []
        for i in range(1600):
            ct = Contract(self.nodes[0], self.options.tmpdir, debug=False)
            cts.append(ct)
        for ct in cts:
            ct.call_payable(amount=100)

        for ct in cts:
            # print(self.nodes[0].getmempoolinfo())
            reason = ct.call_doubleSpendTest(self.nodes[0].getnewaddress(), amount=0,
                                             throw_exception=False).reason()
            if reason:
                # 这里表示mempool满了，把payable的交易都移除了，call_doubleSpendTest里边有send调用，所以没钱
                assert_contains(reason, "mempool full")


if __name__ == '__main__':
    MempoolLimitTest().main()
