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

# Avoid wildcard * imports if possible
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import (
    assert_equal,
    assert_greater_than,
    generate_contract
)

def get_amount():
    return random.randint(1, 10000)

def caller_factory(mgr,contract_id,sender):
    node = mgr.nodes[0]
    contract_id = contract_id
    sender = sender

    def _call_contract(func,*args,amount = get_amount()):
        mgr.log.info("%s,%s,%s,%s,%s"%(contract_id,func,sender,amount,args))
        balance = node.getbalance()
        try:
            result = node.callcontract(True, amount, contract_id, sender, func,*args)
            mgr.log.info("total cost :%s"%(balance - node.getbalance() - amount))
            return result
        except Exception as e:
            print(e)
            assert all(re.findall('-\d+\)$', repr(e)))
            return repr(e)
    return _call_contract

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
        node.generate(nblocks=2) # make some coins
        self.sync_all()

        # publish
        contract = generate_contract(self.options.tmpdir)
        result = node.publishcontract(contract)
        contract_id = result['contractaddress']
        sender = result['senderaddress']

        call_contract = caller_factory(self,contract_id,sender)
        # call
        # no coins
        # call_contract("payable",amount = 10000000)
        #
        # # 非法的入参
        # for amount in [10000000000, -1, 0, 0.00009,0.01]:
        #     call_contract("payable", amount=amount)
        #
        # # 合约不存在
        # caller_factory(self, "2PPWpHssgXjA8yEgfd3Vo36Hhx1eimbGCcP", sender)("payable")
        #
        # # 地址错误
        # for addr in [contract_id + 'x',sender]:
        #     caller_factory(self, addr, sender)("payable")

        # 函数不存在
        # call_contract("payable2")

        # recharge to contract
        call_contract("payable")
        node.generate(nblocks = 1)

        # # tailLoopTest
        # call_contract("tailLoopTest",594) # v452,594 is the limit
        # assert "run out of limit instruction" in call_contract("tailLoopTest", 595)
        #
        # # tailLoopTest2
        # call_contract("tailLoopTest2", 11)
        # assert "Too many args in lua function" in call_contract("tailLoopTest2", 12)
        #
        # # unpackTest
        # assert "too many results to unpack" in call_contract("unpackTest")

        # localFuncTest
        # call_contract("localFuncTest")

        # contractDataTest
        # call_contract("contractDataTest")

        # sendCoinTest
        new_address = node.getnewaddress()
        call_contract("sendCoinTest",new_address,1)
        node.generate(nblocks = 1)
        print(node.getbalanceof(new_address))
        assert node.getbalanceof(new_address) > 0







if __name__ == '__main__':
    ContractCallTest().main()
