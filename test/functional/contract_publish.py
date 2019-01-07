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

from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import (
    assert_equal,
    assert_greater_than,
    generate_contract
)

class ContractPublishTest(MagnaChainTestFramework):
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
        node.generate(nblocks=1) # make some coins
        self.sync_all()

        # no coins
        try:
            contract = generate_contract(self.options.tmpdir)
            result = node.publishcontract(contract)
        except Exception as e:
            assert "GetSenderAddr" in repr(e)

        node.generate(nblocks=1)
        # 错误的合约
        contract = generate_contract(self.options.tmpdir,err_type = "syntax_err")
        try:
            result = node.publishcontract(contract)
        except Exception as e:
            assert 'expected near' in repr(e)

        # 超大合约
        contract = generate_contract(self.options.tmpdir,err_type = "bigfile")
        try:
            result = node.publishcontract(contract)
        except Exception as e:
            assert 'Transaction too large' in repr(e)

        # 测试代码压缩
        contract = generate_contract(self.options.tmpdir,err_type = "trim_code")
        try:
            result = node.publishcontract(contract)
        except Exception as e:
            assert 'Transaction too large' not in repr(e)

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
                node.generate(nblocks=1)
                j = min(64,j * 2)

        node.generate(nblocks=1)

if __name__ == '__main__':
    ContractPublishTest().main()
