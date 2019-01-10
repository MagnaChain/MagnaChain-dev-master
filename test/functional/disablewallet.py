#!/usr/bin/env python3
# Copyright (c) 2015-2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test a node with the -disablewallet option.

- Test that validateaddress RPC works when running with -disablewallet
- Test that it is not possible to mine
"""

from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import *

class DisableWalletTest (MagnaChainTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1


    def run_test (self):
        node = self.nodes[0]
        new_address = node.getnewaddress()
        node.generate(2)
        node.sendtoaddress(new_address,100)
        invalid_address = new_address[:-1] + "P"
        contract = generate_contract(self.options.tmpdir)
        contract_id = node.publishcontract(contract)['contractaddress']
        node.generate(1)
        self.stop_node(0)
        self.start_node(0, extra_args=["-disablewallet"])

        # Make sure wallet is really disabled
        assert_raises_rpc_error(-32601, 'Method not found', node.getwalletinfo)
        x = node.validateaddress(invalid_address)
        assert(x['isvalid'] == False)

        x = node.validateaddress(new_address)
        assert(x['isvalid'] == True)

        x = node.validateaddress(contract_id)
        assert(x['isvalid'] == True)

        # Make sure wallet is really disabled then return "disablewallet option open, no address to mine"
        assert_raises_rpc_error(-25, "disablewallet option open, no address to mine", node.generatetoaddress, 1, new_address)

        # assert_raises_rpc_error(-5, "Invalid address", node.generatetoaddress, 1, invalid_address)
        # assert_raises_rpc_error(-5, "Invalid address", node.generatetoaddress, 1, contract_id)

        # mine test
        assert_raises_rpc_error(-32601, "Method not found (wallet method is disabled because no wallet is loaded)", node.generate, 1)

        # contract test
        assert_raises_rpc_error(-32601, "Method not found", node.publishcontract, 1)
        assert_raises_rpc_error(-32601, "Method not found", node.callcontract, True, 1, contract_id, new_address,"payable")

if __name__ == '__main__':
    DisableWalletTest ().main ()
