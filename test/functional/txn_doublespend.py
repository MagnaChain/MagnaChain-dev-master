#!/usr/bin/env python3
# Copyright (c) 2014-2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the wallet accounts properly when there is a double-spend conflict."""

from decimal import Decimal

from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import (
    assert_equal,
    connect_nodes,
    disconnect_nodes,
    find_output,
    sync_blocks,
)

class TxnMallTest(MagnaChainTestFramework):
    def set_test_params(self):
        self.num_nodes = 4
        self.setup_clean_chain = True


    def add_options(self, parser):
        parser.add_option("--mineblock", dest="mine_block", default=False, action="store_true",
                            help="Test double-spend of 1-confirmed transaction")

    def setup_network(self):
        # Start with split network:
        super().setup_network()

    def run_test(self):
        # All nodes should start with 1,250 BTC:
        for i in range(1):
            for i in range(4):
                pergennum = 1
                genblocks = self.nodes[i].generate(pergennum)
                assert_equal(len(genblocks), pergennum)
                sync_blocks(self.nodes)
        self.nodes[i].generate(1)
        sync_blocks(self.nodes)

        disconnect_nodes(self.nodes[1], 2)
        disconnect_nodes(self.nodes[2], 1)
        
        fp_blockreward = 2600085 #mine block reward
        starting_balance = 2600085 # min unspent value is 10000 in mgc.
        for i in range(4):
            nodebalance=self.nodes[i].getbalance()
            assert_equal(nodebalance, starting_balance)
            self.nodes[i].getnewaddress("")  # bug workaround, coins generated assigned to first getnewaddress!

        nfoo_spend = 2591085 # 1219
        nbar_spend = 5999 # 29
        # Assign coins to foo and bar addresses:
        node0_address_foo = self.nodes[0].getnewaddress()
        fund_foo_txid = self.nodes[0].sendtoaddress(node0_address_foo, nfoo_spend)
        fund_foo_tx = self.nodes[0].gettransaction(fund_foo_txid)

        node0_address_bar = self.nodes[0].getnewaddress()
        fund_bar_txid = self.nodes[0].sendtoaddress(node0_address_bar, nbar_spend)
        fund_bar_tx = self.nodes[0].gettransaction(fund_bar_txid)

        assert_equal(self.nodes[0].getbalance(),
                     starting_balance + fund_foo_tx["fee"] + fund_bar_tx["fee"])

        # Coins are sent to node1_address
        node1_address = self.nodes[1].getnewaddress()

        ndb_spend = 2597000 # 1240
        # First: use raw transaction API to send $ndb_spend BTC to node1_address,
        # but don't broadcast:
        doublespend_fee = Decimal('-.02')
        rawtx_input_0 = {}
        rawtx_input_0["txid"] = fund_foo_txid
        rawtx_input_0["vout"] = find_output(self.nodes[0], fund_foo_txid, nfoo_spend)
        rawtx_input_1 = {}
        rawtx_input_1["txid"] = fund_bar_txid
        rawtx_input_1["vout"] = find_output(self.nodes[0], fund_bar_txid, nbar_spend)
        inputs = [rawtx_input_0, rawtx_input_1]
        change_address = self.nodes[0].getnewaddress()
        outputs = {}
        outputs[node1_address] = ndb_spend
        outputs[change_address] = (nfoo_spend + nbar_spend) - ndb_spend + doublespend_fee
        rawtx = self.nodes[0].createrawtransaction(inputs, outputs)
        doublespend = self.nodes[0].signrawtransaction(rawtx)
        assert_equal(doublespend["complete"], True)

        # Create two spends using 1 50 BTC coin each
        txid1 = self.nodes[0].sendtoaddress(node1_address, 9000) # 40
        txid2 = self.nodes[0].sendtoaddress(node1_address, 8000) # 20

        # Have node0 mine a block:
        if (self.options.mine_block):
            self.nodes[0].generate(1)
            sync_blocks(self.nodes[0:2])

        tx1 = self.nodes[0].gettransaction(txid1)
        tx2 = self.nodes[0].gettransaction(txid2)

        # Node0's balance should be starting balance, plus 50BTC for another
        # matured block, minus 40, minus 20, and minus transaction fees:
        expected = starting_balance + fund_foo_tx["fee"] + fund_bar_tx["fee"]
        if self.options.mine_block:
            expected += 0 # node 0 is the last one in init test data.no more coinbase mature
        expected += tx1["amount"] + tx1["fee"]
        expected += tx2["amount"] + tx2["fee"]
        assert_equal(self.nodes[0].getbalance(), expected)

        if self.options.mine_block:
            assert_equal(tx1["confirmations"], 1)
            assert_equal(tx2["confirmations"], 1)
            # Node1's balance should be both transaction amounts:
            assert_equal(self.nodes[1].getbalance(), starting_balance - tx1["amount"] - tx2["amount"])
        else:
            assert_equal(tx1["confirmations"], 0)
            assert_equal(tx2["confirmations"], 0)

        # Now give doublespend and its parents to miner:
        self.nodes[2].sendrawtransaction(fund_foo_tx["hex"])
        self.nodes[2].sendrawtransaction(fund_bar_tx["hex"])
        doublespend_txid = self.nodes[2].sendrawtransaction(doublespend["hex"])
        # ... mine a block...
        self.nodes[2].generate(1)
        blocks = [self.nodes[i].getblockcount() for i in range(4)]
        self.log.info('before join the nodes,blocks are:{},{},{},{}'.format(*blocks))
        self.log.info('before join the nodes,bestblockhash are:{},{},{},{}'.format(*[self.nodes[i].getbestblockhash() for i in range(4)]))

        # Reconnect the split network, and sync chain:
        connect_nodes(self.nodes[1], 2)
        self.nodes[2].generate(1)  # Mine another block to make sure we sync
        sync_blocks(self.nodes)
        blocks = [self.nodes[i].getblockcount() for i in range(4)]
        self.log.info('after join the nodes,blocks are:{},{},{},{}'.format(*blocks))
        self.log.info('after join the nodes,bestblockhash are:{},{},{},{}'.format(
            *[self.nodes[i].getbestblockhash() for i in range(4)]))
        assert_equal(self.nodes[0].gettransaction(doublespend_txid)["confirmations"], 2)

        # Re-fetch transaction info:
        tx1 = self.nodes[0].gettransaction(txid1)
        tx2 = self.nodes[0].gettransaction(txid2)

        # Both transactions should be conflicted
        assert_equal(tx1["confirmations"], -2)
        assert_equal(tx2["confirmations"], -2)

        # Node0's total balance should be starting balance, plus 2*$fp_blockreward MGC for
        # two more matured blocks, minus $ndb_spend for the double-spend, plus fees (which are
        # negative):
        expected = starting_balance - ndb_spend + fund_foo_tx["fee"] + fund_bar_tx["fee"] + doublespend_fee # + fp_blockreward*2
        assert_equal(self.nodes[0].getbalance(), expected)

        # Node1's balance should be its initial balance ($starting_balance for 25 block rewards) plus the doublespend:
        assert_equal(self.nodes[1].getbalance(), starting_balance + ndb_spend)

if __name__ == '__main__':
    TxnMallTest().main()

