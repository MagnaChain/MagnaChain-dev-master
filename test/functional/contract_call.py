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
import random
import re
from functools import wraps
from decimal import Decimal

# Avoid wildcard * imports if possible
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import (
    assert_equal,
    assert_greater_than,
    assert_contains,
    generate_contract,
    caller_factory
)

# TODO SKIP should be set False
SKIP = False
PAYABLE = "payable"
CYCLE_CALL = "callOtherContractTest"


class ContractCallTest(MagnaChainTestFramework):
    # Each functional test is a subclass of the MagnaChainTestFramework class.

    # Override the set_test_params(), add_options(), setup_chain(), setup_network()
    # and setup_nodes() methods to customize the test setup as required.

    def set_test_params(self):
        """Override test parameters for your individual test.

        This method must be overridden and num_nodes must be exlicitly set."""
        self.setup_clean_chain = True
        self.num_nodes = 1

    def run_test(self):
        """Main test logic"""
        # prepare
        node = self.nodes[0]
        node.generate(nblocks=2)  # make some coins
        self.sync_all()

        # publish
        contract = generate_contract(self.options.tmpdir)
        result = node.publishcontract(contract)
        contract_id = result['contractaddress']
        sender = result['senderaddress']

        call_contract = caller_factory(self, contract_id, sender)
        # call
        # no coins
        assert_contains(call_contract(PAYABLE, amount=10000000), "Insufficient funds")

        # # 非法的入参
        for amount in [10000000000, -1, 0, Decimal("0.0009").quantize(Decimal("0.0000"))]:
            call_contract(PAYABLE, amount=amount)

        # 非法sender
        for sender_addr in [sender + "x", contract_id]:
            assert_contains(caller_factory(self, contract_id, sender_addr)(PAYABLE), "Invalid sender address")

        # # 合约不存在
        assert_contains(caller_factory(self, "2PPWpHssgXjA8yEgfd3Vo36Hhx1eimbGCcP", sender)(PAYABLE),
                        "GetContractInfo fail")

        # # 地址错误
        for addr in [contract_id + 'x', sender]:
            caller_factory(self, addr, sender)(PAYABLE)

        # 函数不存在
        if not SKIP:
            assert_contains(call_contract("payable2"), "can not find function")

        # recharge to contract
        call_contract(PAYABLE)
        node.generate(nblocks=1)

        # # tailLoopTest
        call_contract("tailLoopTest", 594)  # v452,594 is the limit
        assert_contains(call_contract("tailLoopTest", 595), "run out of limit instruction")

        # # tailLoopTest2
        call_contract("tailLoopTest2", 11)
        assert_contains(call_contract("tailLoopTest2", 12), "Too many args in lua function")

        # # unpackTest
        assert_contains(call_contract("unpackTest"), "too many results to unpack")

        # localFuncTest
        if not SKIP:
            assert_contains(call_contract("localFuncTest"), "can not find function")

        # contractDataTest
        call_contract("contractDataTest")
        assert_equal(call_contract("get", "size")['return'][0], 127)

        # sendCoinTest
        # send to mgc address
        new_address = node.getnewaddress()
        call_contract("sendCoinTest", new_address, 1)
        call_contract("sendCoinTest", new_address, "1")
        assert_contains(call_contract("sendCoinTest", new_address, 2 ** 31 - 1), "not enough amount ")
        assert_contains(call_contract("sendCoinTest", new_address, 0.1), "JSON integer out of range")
        assert_contains(call_contract("sendCoinTest", new_address, 0), "Dust amount")
        assert_contains(call_contract("sendCoinTest", new_address, -1), "Dust amount")
        # send all balance
        tmp_id = node.publishcontract(contract)["contractaddress"]
        tmp_caller = caller_factory(self, tmp_id, sender)
        tmp_caller(PAYABLE, amount=Decimal("1"))
        tmp_caller("sendCoinTest", new_address, 1, amount=Decimal("0"))
        node.generate(nblocks=2)  # 这里需要挖2个，因为send的输出需要达到成熟度才可以使用
        assert_equal(node.getbalanceof(new_address), 3)
        assert_contains(tmp_caller("sendCoinTest", new_address, 1), "not enough amount ")

        # send to contract
        tmp_id = node.publishcontract(contract)["contractaddress"]
        assert_contains(call_contract("sendCoinTest", tmp_id, 100), "Invalid destination address")
        node.generate(nblocks=1)

        # batchSendTest
        # 12个参数是上限，除去内部调用之后，实际能用的就只有7个参数位，并且不支持数组
        for to in range(35):
            to_list = [node.getnewaddress() for i in range(7)]
            call_contract("addWithdrawList", *to_list)
        call_contract("batchSendTest")
        node.generate(nblocks=2)
        for addr in to_list:
            assert_equal(node.getbalanceof(addr), 1)

        # updateContractTest
        call_contract("updateContract", "self", "weigun")
        node.generate(nblocks=1)
        assert_equal(call_contract("get", "self")['return'][0], "weigun")

        # cycleSelfTest
        if not SKIP:
            for i in range(100):
                call_contract("cycleSelf")
                call_contract("updateContract", "this", "")
                if i % 10 == 0:
                    node.generate(nblocks=1)
            node.generate(nblocks=1)

        # maxContractCallTest
        call_contract("maxContractCallTest", 11)  # 11 is the limit
        assert_contains(call_contract("maxContractCallTest", 12), "run out of limit instruction")

        # callOtherContractTest
        # cycle call
        # step1 create contracts
        ca_id = contract_id
        cb_id = node.publishcontract(contract)["contractaddress"]
        cc_id = node.publishcontract(contract)["contractaddress"]
        caller_b = caller_factory(self, cb_id, sender)
        node.generate(nblocks=1)

        # step2  a->b->c->a(send will be call in last a)
        new_address = node.getnewaddress()
        if not SKIP:
            call_contract(CYCLE_CALL, cb_id, CYCLE_CALL, cc_id, CYCLE_CALL, ca_id, "sendCoinTest", new_address)
            node.generate(nblocks=1)
            assert_equal(node.getbalanceof(new_address), 1)

        # step3 a->b->c->b,modify PersistentData
        if not SKIP:
            caller_b("contractDataTest")  # after called,size should be 127
            assert_equal(caller_b("get", "size")['return'][0], 127)
            call_contract(CYCLE_CALL, cb_id, CYCLE_CALL, cc_id, CYCLE_CALL, cb_id, "contractDataTest",
                          new_address)  # after called,size should be 126
            node.generate(nblocks=1)
            assert_equal(caller_b("get", "size")['return'][0], 126)

        # lots of dust vin in contract's send transaction
        # TODO:maybe  need to set payfee param in magnachaind
        if not SKIP:
            cd_id = node.publishcontract(contract)["contractaddress"]
            caller_d = caller_factory(self, cd_id, sender)
            for i in range(2000):
                caller_d(PAYABLE, amount=Decimal("1"))
                if i % 50 == 0:
                    node.generate(nblocks=1)
            new_address = node.getnewaddress()
            caller_d("sendCoinTest", new_address, 1900, amount=Decimal("0"))
            node.generate(nblocks=2)
            assert_equal(node.getbalanceof(new_address), 1900)

        # dust change vout in send
        # node.sendtoaddress(new_sender,2)
        if not SKIP:
            tmp_id = node.publishcontract(contract)['contractaddress']
            caller_tmp = caller_factory(self, tmp_id, sender)
            senders = [node.getnewaddress() for i in range(11)]
            list(map(lambda x: caller_tmp(PAYABLE, amount=1), senders))  # 充值11次，每次1个MGC
            node.generate(nblocks=2)
            list(map(
                lambda to: assert_equal(isinstance(caller_tmp("dustChangeTest", to, amount=Decimal("0")), dict), True),
                senders))  # 向每个地址发送(cell / 11) * 10(最小单位)，cell = 100000000。这里应该有11个微交易的找零
            node.generate(nblocks=2)
            tmp_sender = node.getnewaddress()
            assert_equal(isinstance(caller_tmp("sendCoinTest", tmp_sender, 1, amount=Decimal("0")), dict),
                         True)  # 组合所有微交易的找零，应该足够1个MGC的
            node.generate(nblocks=2)
            assert_equal(node.getbalanceof(tmp_sender), 1)

        # reentrancyTest
        last_id = node.publishcontract(contract)['contractaddress']
        caller_last = caller_factory(self, last_id, sender)
        if not SKIP:
            caller_last("reentrancyTest")
            node.generate(nblocks=1)
            assert_equal(caller_last("get", "this")['return'][0], None)

        # 疲劳测试
        if not SKIP:
            to_list = [node.getnewaddress() for i in range(1000)]
            for to in to_list:
                caller_last(PAYABLE, amount=2)
                caller_last(CYCLE_CALL, last_id, "contractDataTest",amount=0)
                caller_last(CYCLE_CALL, last_id, "dustChangeTest",to,amount=0)
                caller_last(CYCLE_CALL, last_id, "addWithdrawList",to,amount=0)
                caller_last(CYCLE_CALL, last_id, "batchSendTest",amount=0)
                if random.randint(1,100) > 80:
                    node.generate(nblocks=1)


if __name__ == '__main__':
    ContractCallTest().main()
