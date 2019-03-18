#!/usr/bin/env python3
# Copyright (c) 2014-2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test running magnachaind with -reindex and -reindex-chainstate options.

- Start a single node and generate 3 blocks.
- Stop the node and restart it with -reindex. Verify that the node has reindexed up to block 3.
- Stop the node and restart it with -reindex-chainstate. Verify that the node has reindexed up to block 3.
"""

from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import assert_equal,generate_contract
import time

class ReindexTest(MagnaChainTestFramework):

    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1

    def reindex(self, justchainstate=False,with_contract = False,with_sendcoin = False):
        self.nodes[0].generate(3)

        if with_contract:
            # make some contract transactions
            cid = self.nodes[0].publishcontract(generate_contract(self.options.tmpdir))['contractaddress']
            self.nodes[0].callcontract(True, 100, cid, self.nodes[0].getnewaddress(), 'payable')
            for i in range(10):
                if not with_sendcoin:
                    # first it works!
                    self.nodes[0].callcontract(True, 10, cid, self.nodes[0].getnewaddress(), 'payable')
                else:
                    # next it failed!
                    self.nodes[0].callcontract(True,0,cid,self.nodes[0].getnewaddress(),'sendCoinTest',self.nodes[0].getnewaddress(),1)
        self.nodes[0].sendtoaddress(self.nodes[0].getnewaddress(),1)
        self.nodes[0].generate(2)

        # TODO 还需要测试跨链交易的情况

        blockcount = self.nodes[0].getblockcount()
        self.stop_nodes()
        extra_args = [["-reindex-chainstate" if justchainstate else "-reindex", "-checkblockindex=1"]]
        self.start_nodes(extra_args)

        while self.nodes[0].getblockcount() < blockcount:
            print(self.nodes[0].getblockcount() , blockcount)
            time.sleep(0.1)
        assert_equal(self.nodes[0].getblockcount(), blockcount)
        self.log.info("Success")

    def run_test(self):
        self.log.info("without contract test")
        self.reindex(False)
        self.reindex(True)
        self.reindex(False)
        self.reindex(True)

        self.log.info("with contract,but without sendcoin test")
        self.reindex(False,with_contract = True)
        self.reindex(True,with_contract = True)
        self.reindex(False,with_contract = True)
        self.reindex(True,with_contract = True)

        self.log.info("with contract, with sendcoin test")
        self.reindex(False,with_contract = True,with_sendcoin = True)
        self.reindex(True,with_contract = True,with_sendcoin = True)
        self.reindex(False,with_contract = True,with_sendcoin = True)
        self.reindex(True,with_contract = True,with_sendcoin = True)

if __name__ == '__main__':
    ReindexTest().main()
