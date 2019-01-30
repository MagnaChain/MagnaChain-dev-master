#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
Smart Contract Testing - call

Major test call smart contract

开2个节点：
节点1作为主测试节点
节点2作为辅助验证
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
from test_framework.mininode import COIN
from test_framework.util import (
    assert_equal,
    assert_greater_than,
    assert_contains,
    generate_contract,
    caller_factory,
    sync_mempools,
    sync_blocks,
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
        self.num_nodes = 2

    def run_test(self):
        """Main test logic"""
        # prepare
        node = self.nodes[0]
        node2 = self.nodes[1]
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
        for amount in [10000000000, -1, 0, Decimal("0.000009").quantize(Decimal("0.000000"))]:
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

        # send不能被裸调用
        # bug被修复前，暂时跳过
        # if not SKIP:
        #     assert_contains(call_contract("send",sender,0),'can not find function')
        #     assert_contains(call_contract("rpcSendTest"), 'can not find function')

        # when not commit transaction, make sure contract tx is not in mempool
        if not SKIP:
            mempool = node.getrawmempool()
            node.callcontract(False, 1, contract_id, sender, PAYABLE)
            assert mempool == node.getrawmempool()

        # recharge to contract
        txid = call_contract(PAYABLE,amount = 1000)['txid']
        sync_mempools(self.nodes)
        assert txid in node.getrawmempool()
        assert txid in node2.getrawmempool()
        assert_equal(node.getbalanceof(contract_id), 0)  #合约的余额只会在一定的确认数之后才可以查看，在内存池中是不生效的
        node.generate(nblocks=1)
        self.sync_all()
        assert txid not in node.getrawmempool()
        assert txid not in node2.getrawmempool()
        assert_equal(node.getbalanceof(contract_id), 1000)  # 确认合约余额

        # doubleSpendTest
        call_contract("doubleSpendTest", node.getnewaddress(),throw_exception = True)
        self.sync_all()

        # # tailLoopTest
        call_contract("tailLoopTest", 896)  # v452,594 is the limit
        assert_contains(call_contract("tailLoopTest", 897), "run out of limit instruction")


        # # tailLoopTest2
        call_contract("tailLoopTest2", 11)
        assert_contains(call_contract("tailLoopTest2", 12), "Too many args in lua function")

        # # unpackTest
        assert_contains(call_contract("unpackTest"), "too many results to unpack")

        # localFuncTest
        if not SKIP:
            assert_contains(call_contract("localFuncTest"), "can not find function")

        # longReturnTest
        if not SKIP:
            c = generate_contract(self.options.tmpdir, err_type="long_string_return")
            addre = node.publishcontract(c)['contractaddress']
            node.callcontract(True,1,addre,sender,'longReturnTest')

        # contractDataTest
        call_contract("contractDataTest")
        assert_equal(call_contract("get", "size")['return'][0], 127)

        # sendCoinTest
        # send to mgc address
        new_address = node.getnewaddress()
        txid = call_contract("sendCoinTest", new_address, 1)['txid']
        assert txid in node.getrawmempool()
        call_contract("sendCoinTest", new_address, "1e-3")
        assert_contains(call_contract("sendCoinTest", new_address, 2 ** 31 - 1), "not enough amount ")
        assert_contains(call_contract("sendCoinTest", new_address, 0.1), "JSON integer out of range")
        assert_contains(call_contract("sendCoinTest", new_address, 0), "Dust amount")
        assert_contains(call_contract("sendCoinTest", new_address, -1), "Dust amount")
        # send all balance
        tmp_id = node.publishcontract(contract)["contractaddress"]
        tmp_caller = caller_factory(self, tmp_id, sender)
        tmp_caller(PAYABLE, amount=Decimal("1"))
        tmp_caller("sendCoinTest", new_address, 1, amount=Decimal("0"))
        # 利用节点2挖矿，确保节点1的交易可以打包的块
        node2.generate(nblocks=2)  # 这里需要挖2个，因为send的输出需要达到成熟度才可以使用
        self.sync_all()
        assert_equal(node.getbalanceof(new_address), 2)
        assert_equal(node.getbalanceof(tmp_id), 0)
        assert_equal(node2.getbalanceof(new_address), 2)
        assert_equal(node2.getbalanceof(tmp_id), 0)
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
        assert_equal(call_contract("get", "self")['return'][0], "weigun")
        node.generate(1)

        assert_equal([],node.getrawmempool()) # make sure mempool is empty
        # cycleSelfTest
        self.log.info("begin cycleSelfTest")
        if not SKIP:
            for i in range(100):
                call_contract("cycleSelf",throw_exception = False)
                call_contract("updateContract", "this", "",throw_exception = False)
                if i % 5 == 0:
                    # print("generate block at :",i)
                    node.generate(1)
            node.generate(1)

        # assert_equal([], node.getrawmempool())  # make sure mempool is empty
        # maxContractCallTest
        call_contract("maxContractCallTest", 15)  # 15 is the limit
        assert_contains(call_contract("maxContractCallTest", 16), "run out of limit instruction")

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
        if  SKIP:
            call_contract(CYCLE_CALL, cb_id, CYCLE_CALL, cc_id, CYCLE_CALL, ca_id, "sendCoinTest", new_address,throw_exception = True)
            node.generate(nblocks=1)
            assert_equal(node.getbalanceof(new_address), 1)

        # step3 a->b->c->b,modify PersistentData
        if  SKIP:
            caller_b("contractDataTest")  # after called,size should be 127
            assert_equal(caller_b("get", "size")['return'][0], 127)
            call_contract(CYCLE_CALL, cb_id, CYCLE_CALL, cc_id, CYCLE_CALL, cb_id, "contractDataTest",
                          new_address,throw_exception = True)  # after called,size should be 127,because of replace dump
            node.generate(nblocks=1)
            assert_equal(caller_b("get", "size")['return'][0], 127)

        # lots of dust vin in contract's send transaction
        # TODO:maybe  need to set payfee param in magnachaind
        if  SKIP:
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
            senders = [node.getnewaddress() for i in range(101)]
            i = 0
            for _ in senders:
                # 充值101次，每次1个MGC
                caller_tmp(PAYABLE, amount=1)
                if i % 50 == 0:
                    node.generate(1)
                i += 1
            node.generate(2)
            assert_equal(node.getbalanceof(tmp_id), 101)
            i = 0
            for to in senders:
                # 向每个地址发送cell - 999(最小单位)，cell = 100000000。这里应该有101个微交易的找零
                assert_equal(isinstance(caller_tmp("dustChangeTest", to, amount=Decimal("0")), dict), True)
                if i % 50 == 0:
                    node.generate(1)
                i += 1
            node.generate(nblocks=2)
            assert_equal(node.getbalanceof(tmp_id) * COIN, 101 * COIN - (COIN - 999 ) * 101) #因为lua没有浮点数，所以小数部分会截断掉
            bal = node.getbalanceof(tmp_id)
            print(bal)
            tmp_sender = node.getnewaddress()
            assert_equal(isinstance(caller_tmp("sendCoinTest", tmp_sender, 1, amount=Decimal("0.001"),throw_exception = True), dict),
                         True)  # 组合所有微交易的找零，应该足够0.001个MGC的
            node.generate(nblocks=2)
            assert_equal(node.getbalanceof(tmp_sender), Decimal("0.001"))
            assert_equal(node.getbalanceof(tmp_id), bal - Decimal("0.001"))

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
