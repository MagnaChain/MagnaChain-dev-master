#!/usr/bin/env python3
# Copyright (c) 2014-2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the abandontransaction RPC.

 The abandontransaction RPC marks a transaction and all its in-wallet
 descendants as abandoned which allows their inputs to be respent. It can be
 used to replace "stuck" or evicted transactions. It only works on transactions
 which are not included in a block and are not currently in the mempool. It has
 no effect on transactions which are already conflicted or abandoned.
"""
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import *
from  test_framework.contract import Contract



class AbandonConflictTest(MagnaChainTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.extra_args = [["-minrelaytxfee=0.00001", "-disablesafemode"], ["-disablesafemode"]]

    def get_txfee(self, txid, node_index=0):
        return Decimal(self.nodes[node_index].gettransaction(txid)['fee'])

    def run_test(self):
        self.nodes[1].generate(2)
        sync_blocks(self.nodes)
        balance = self.nodes[0].getbalance()
        contract_obj = Contract(self.nodes[0],self.options.tmpdir) #构造合约
        txA = contract_obj.publish_txid #发布合约的交易ID
        txB = contract_obj.call_payable(amount = 1)['txid']
        txC = contract_obj.call_payable(amount = 1)['txid']
        sync_mempools(self.nodes)
        self.nodes[1].generate(1)

        sync_blocks(self.nodes)
        newbalance = self.nodes[0].getbalance()
        print(balance - newbalance)
        assert (balance - newbalance < Decimal("19.67900001"))  # no more than fees lost
        balance = newbalance

        # Disconnect nodes so node0's transactions don't get into node1's mempool
        disconnect_nodes(self.nodes[0], 1)

        # Identify the contract outputs
        # 分别获取发布合约与调用合约的交易输出
        nA,vA = next((i,vout["value"]) for i, vout in enumerate(self.nodes[0].getrawtransaction(txA, 1)["vout"]) if
                  vout["value"] != Decimal("1"))
        nB,vB = next((i,vout["value"]) for i, vout in enumerate(self.nodes[0].getrawtransaction(txB, 1)["vout"]) if
                  vout["value"] != Decimal("1"))
        nC,vC = next((i,vout["value"]) for i, vout in enumerate(self.nodes[0].getrawtransaction(txC, 1)["vout"]) if
                  vout["value"] != Decimal("1"))

        # self.stop_node(0)
        # self.start_node(0, extra_args=["-minrelaytxfee=0.001"])
        assert_equal(len(self.nodes[0].getrawmempool()), 0)
        assert_equal(self.nodes[0].getbalance(), balance)
        inputs = []
        # spend 9985.026 + 9997.646 mgc outputs from txD and txE
        # 花费掉vA和vB
        inputs.append({"txid": txA, "vout": nA})
        inputs.append({"txid": txB, "vout": nB})
        outputs = {}
        outputs[self.nodes[0].getnewaddress()] = Decimal(int(vA))
        outputs[self.nodes[1].getnewaddress()] = Decimal(int(vB))
        signed = self.nodes[0].signrawtransaction(self.nodes[0].createrawtransaction(inputs, outputs))
        txAB1 = self.nodes[0].sendrawtransaction(signed["hex"])

        # Identify the 9985mgc output
        # 找到va的输出
        nAB,vAB = next((i,vout["value"]) for i, vout in enumerate(self.nodes[0].getrawtransaction(txAB1, 1)["vout"]) if
                   vout["value"] == Decimal(int(vA)))

        # Create a child tx spending AB1 and C
        inputs = []
        inputs.append({"txid": txAB1, "vout": nAB})
        inputs.append({"txid": txC, "vout": nC})
        outputs = {}
        addrABC2 = self.nodes[0].getnewaddress()
        outputs[addrABC2] = Decimal("19977")
        signed2 = self.nodes[0].signrawtransaction(self.nodes[0].createrawtransaction(inputs, outputs))
        txABC2 = self.nodes[0].sendrawtransaction(signed2["hex"])

        # In mempool txs from self should increase balance from change
        newbalance = self.nodes[0].getbalance()
        print(newbalance,balance,balance - newbalance,newbalance - (balance - Decimal(str(vA + vB + vC)) + Decimal("19977")))
        assert_equal(newbalance, balance - Decimal(str(vA + vB + vC)) + Decimal("19977"))
        balance = newbalance

        # Restart the node with a higher min relay fee so the parent tx is no longer in mempool
        # TODO: redo with eviction
        self.stop_node(0)
        self.start_node(0, extra_args=["-minrelaytxfee=10.00000"])

        # Verify txs no longer in either node's mempool
        assert_equal(len(self.nodes[0].getrawmempool()), 0)
        assert_equal(len(self.nodes[1].getrawmempool()), 0)

        # Not in mempool txs from self should only reduce balance
        # inputs are still spent, but change not received
        newbalance = self.nodes[0].getbalance()
        assert_equal(newbalance, balance - Decimal("19977"))
        # Unconfirmed received funds that are not in mempool, also shouldn't show
        # up in unconfirmed balance
        unconfbalance = self.nodes[0].getunconfirmedbalance() + self.nodes[0].getbalance()
        assert_equal(unconfbalance, newbalance)
        # Also shouldn't show up in listunspent
        x = self.nodes[0].listunspent(1, 9999999, [addrABC2])
        assert(not txABC2 in [utxo["txid"] for utxo in x])
        balance = newbalance

        # Abandon original transaction and verify inputs are available again
        # including that the child tx was also abandoned
        self.nodes[0].abandontransaction(txAB1)
        newbalance = self.nodes[0].getbalance()
        assert_equal(newbalance, balance + Decimal(str(vA + vB + vC)))
        balance = newbalance

        # Verify that even with a low min relay fee, the tx is not reaccepted from wallet on startup once abandoned
        self.stop_node(0)
        self.start_node(0, extra_args=["-minrelaytxfee=0.00001"])
        assert_equal(len(self.nodes[0].getrawmempool()), 0)
        assert_equal(self.nodes[0].getbalance(), balance)

        # But if its received again then it is unabandoned
        # And since now in mempool, the change is available
        # But its child tx remains abandoned
        # There is currently a bug around this and so this test doesn't work.  See Issue #0060 in testin
        self.nodes[0].sendrawtransaction(signed["hex"])
        newbalance = self.nodes[0].getbalance()
        assert_equal(newbalance, balance - Decimal(str(vA + vB)) + Decimal(int(vA)))
        balance = newbalance

        # Send child tx again so its unabandoned
        self.nodes[0].sendrawtransaction(signed2["hex"])
        newbalance = self.nodes[0].getbalance()
        assert_equal(newbalance, balance - Decimal(int(vAB)) - Decimal(vC) + Decimal("19977"))
        balance = newbalance

        # Remove using high relay fee again
        self.stop_node(0)
        self.start_node(0, extra_args=["-minrelaytxfee=10.00000"])
        assert_equal(len(self.nodes[0].getrawmempool()), 0)
        newbalance = self.nodes[0].getbalance()
        assert_equal(newbalance, balance - Decimal("19977"))
        balance = newbalance

        # Create a double spend of AB1 by spending again from only A's 10 output
        # Mine double spend from node 1
        inputs = []
        inputs.append({"txid": txA, "vout": nA})
        inputs.append({"txid": txC, "vout": nC})
        outputs = {}
        outputs[self.nodes[1].getnewaddress()] = Decimal("19950")
        tx = self.nodes[0].createrawtransaction(inputs, outputs)
        signed = self.nodes[0].signrawtransaction(tx)
        self.nodes[1].sendrawtransaction(signed["hex"])
        self.nodes[1].generate(1)

        connect_nodes(self.nodes[0], 1)
        sync_blocks(self.nodes)

        # Verify that B and C's 10 MGC outputs are available for spending again because AB1 is now conflicted
        newbalance = self.nodes[0].getbalance()
        assert_equal(newbalance, balance + Decimal(vC))
        balance = newbalance

        # There is currently a minor bug around this and so this test doesn't work.  See Issue #7315
        # Invalidate the block with the double spend and B's 10 MGC output should no longer be available
        # Don't think C's should either
        self.nodes[0].invalidateblock(self.nodes[0].getbestblockhash())
        newbalance = self.nodes[0].getbalance()
        # assert_equal(newbalance, balance - Decimal("10"))
        self.log.info(
            "If balance has not declined after invalidateblock then out of mempool wallet tx which is no longer")
        self.log.info("conflicted has not resumed causing its inputs to be seen as spent.  See Issue #7315")
        self.log.info(str(balance) + " -> " + str(newbalance) + " ?")

        self.stop_node(0)
        self.start_node(0, extra_args=["-minrelaytxfee=0.00001"])
        disconnect_nodes(self.nodes[0], 1)
        node = self.nodes[0]
        node.generate(2)
        assert_equal([], node.getrawmempool())  # make sure mempool is empty
        new_address = node.getnewaddress()
        balance = node.getbalance()
        contract_balance = node.getbalanceof(contract_obj.contract_id)
        amount = 1
        txD = contract_obj.call_sendCoinTest(new_address,amount,amount=0)['txid']
        txE = contract_obj.call_sendCoinTest(new_address, amount, amount=0)['txid']
        txD_fee = self.get_txfee(txD)
        txE_fee = self.get_txfee(txE)
        newbalance = node.getbalance()
        assert_equal(contract_balance, node.getbalanceof(contract_obj.contract_id))

        # 放弃合约的sendcoin交易
        self.stop_node(0)
        self.start_node(0, extra_args=["-minrelaytxfee=10.0000"])
        assert_equal([], node.getrawmempool())  # make sure mempool is empty
        assert_equal(newbalance, balance + txD_fee + txE_fee)
        node.abandontransaction(txD)
        node.abandontransaction(txE)
        assert_equal(node.getbalance(), balance)
        node.generate(2)
        assert_equal(contract_balance, node.getbalanceof(contract_obj.contract_id))


if __name__ == '__main__':
    AbandonConflictTest().main()
