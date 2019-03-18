#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
Smart Contract Testing - Published

Major test publish smart contract
"""
# Imports should be in PEP8 ordering (std library first, then third party
# libraries then local imports).
from collections import defaultdict
from decimal import Decimal

import time
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import (
    assert_equal,
    assert_greater_than,
    generate_contract,
    assert_contains,
    bytes_to_hex_str,
    hex_str_to_bytes,
    sync_blocks,
    count_bytes,
    assert_raises_rpc_error,
    connect_nodes_bi,
)

class ContractPublishTest(MagnaChainTestFramework):
    # Each functional test is a subclass of the MagnaChainTestFramework class.

    # Override the set_test_params(), add_options(), setup_chain(), setup_network()
    # and setup_nodes() methods to customize the test setup as required.

    def set_test_params(self):
        """Override test parameters for your individual test.

        This method must be overridden and num_nodes must be exlicitly set."""
        self.setup_clean_chain = True
        self.num_nodes = 2


    def run_test(self):
        """Main test logic"""
        # prepare
        node = self.nodes[0]
        node2 = self.nodes[1]
        node2.generate(2) # make some coins
        self.sync_all()

        # no coins
        try:
            contract = generate_contract(self.options.tmpdir)
            result = node.publishcontract(contract)
        except Exception as e:
            assert "GetSenderAddr" in repr(e)

        # make sure not in mempool when the tx failed
        assert_equal([],node.getrawmempool())

        node.generate(2) # make some coins
        self.sync_all()
        # 错误的合约
        contract = generate_contract(self.options.tmpdir,err_type = "syntax_err")
        try:
            result = node.publishcontract(contract)
        except Exception as e:
            assert 'expected near' in repr(e)

        # 超大合约
        contract = generate_contract(self.options.tmpdir,err_type = "bigfile") # should be bigfile
        try:
            result = node.publishcontract(contract)
        except Exception as e:
            assert 'code is too large' in repr(e)

        # 测试sdk发布合约的接口
        # TODO 需要单独测试prepublishcode接口
        # prepublishcodeTest
        contract = generate_contract(self.options.tmpdir)
        with open(contract) as fh:
            content = "".join(fh.readlines())
        hex_content = bytes_to_hex_str(bytes(content,encoding='utf-8'))
        coster = node.getnewaddress()
        for i in range(10):
            node.sendtoaddress(coster, 1000)
        node.generate(2)
        sender = coster
        amount = 1
        changeaddress = node.getnewaddress()
        result = node.prepublishcode(hex_content,coster,sender,amount,changeaddress)
        print(result)

        # syntax_err hex content
        contract = generate_contract(self.options.tmpdir,err_type = "syntax_err")
        with open(contract) as fh:
            content = "".join(fh.readlines())
        hex_content_tmp = bytes_to_hex_str(bytes(content,encoding='utf-8'))
        try:
            result = node.prepublishcode(hex_content_tmp, coster, sender, amount, changeaddress)
        except Exception as e:
            assert 'expected near' in repr(e)

        # empty content
        hex_content_tmp = bytes_to_hex_str(bytes("",encoding='utf-8'))
        try:
            result = node.prepublishcode(hex_content_tmp, coster, sender, amount, changeaddress)
        except Exception as e:
            assert 'code data can not empty' in repr(e)

        # not hex
        hex_content_tmp = "not hex data"
        try:
            result = node.prepublishcode(hex_content_tmp, coster, sender, amount, changeaddress)
        except Exception as e:
            assert 'code data must hex data' in repr(e)

        # coster test
        # Invalid address
        contract = generate_contract(self.options.tmpdir)
        contract_id = node.publishcontract(contract)["contractaddress"]
        node1_newaddress = self.nodes[1].getnewaddress()
        self.nodes[1].sendtoaddress(node1_newaddress, 100)
        self.nodes[1].generate(2)
        sync_blocks(self.nodes)
        for coster_tmp in ["","DFGHJK12316547645",contract_id,node1_newaddress]:
            try:
                result = node.prepublishcode(hex_content, coster_tmp, sender, amount, changeaddress)
            except Exception as e:
                assert 'Invalid MagnaChain public key address' in repr(e) or "Invalid MagnaChain fund address" in repr(e)
                continue
        node.generate(1)

        # sender test
        # Invalid address
        for sender_tmp in ["", "DFGHJK12316547645", contract_id, node1_newaddress]:
            try:
                result = node.prepublishcode(hex_content, coster, sender_tmp, amount, changeaddress)
            except Exception as e:
                assert 'Invalid MagnaChain public key address' in repr(e) or "Invalid MagnaChain sender address" in repr(e)
                continue
        node.generate(1)

        # amount test
        for amount_tmp in [10000000000, "10",-1, 0, Decimal("0.0009").quantize(Decimal("0.0000"))]:
            try:
                result = node.prepublishcode(hex_content, coster, sender, amount_tmp, changeaddress)
            except Exception as e:
                assert 'Invalid amount for send' in repr(e) or "Amount out of range" in repr(e) or "Invalid amount" in repr(e)
                continue
        node.generate(1)

        # changeaddress test
        for changeaddress_tmp in ["", "DFGHJK12316547645", contract_id, node1_newaddress]:
            try:
                result = node.prepublishcode(hex_content, coster, sender, amount_tmp, changeaddress)
            except Exception as e:
                assert 'Invalid MagnaChain public key address' in repr(e) or "Invalid MagnaChain change address" in repr(e)
                continue
        node.generate(1)

        # test fee
        # encrypt wallet test with contract
        node.node_encrypt_wallet('test')
        self.stop_node(1)
        self.start_nodes()
        connect_nodes_bi(self.nodes, 0, 1)
        node.walletpassphrase("test", 1)
        time.sleep(2) # wait for timeout
        assert_raises_rpc_error(-13,'Please enter the wallet passphrase with walletpassphrase first',node.publishcontract,contract)
        node.walletpassphrase("test", 300)
        payfee = node.getinfo()['paytxfee']
        relayfee = node.getinfo()['relayfee']
        txid = node.publishcontract(contract)['txid']
        txfee = node.gettransaction(txid)['fee']
        tx_size = count_bytes(node.getrawtransaction(txid))
        node.settxfee(20)
        txid = node.publishcontract(contract)['txid']
        print(txfee,node.gettransaction(txid)['fee'])
        assert abs(node.gettransaction(txid)['fee']) > abs(txfee) and abs(node.gettransaction(txid)['fee']) == 100
        assert_equal(node.gettransaction(txid)['confirmations'],0)
        self.sync_all()
        node2.generate(1)
        self.sync_all()
        assert_equal(node.gettransaction(txid)['confirmations'], 1)
        node.settxfee(payfee)

        # 正确的合约，并且进行重复测试
        j = 2
        contract = generate_contract(self.options.tmpdir)
        for i in range(200):
            balance = node.getbalance()
            result = node.publishcontract(contract)
            diff = balance - node.getbalance()
            assert diff > 0 and diff < 13 ,"publish fee too much:%s"%(diff)  # 该合约的费用基本是固定的，避免修改数值出现比较大的偏差
            self.log.info("publish cost:%s"%(balance - node.getbalance()))
            if i % j == 0:
                # 每个多少个交易后才打一次包
                self.sync_all()
                node.generate(1)
                j = min(64,j * 2)

        node.generate(1)

if __name__ == '__main__':
    ContractPublishTest().main()
