#!/usr/bin/env python3
# Copyright (c) 2014-2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the invalidateblock RPC."""

from test_framework.test_framework import MagnaChainTestFramework
from test_framework.util import *

class InvalidateTest(MagnaChainTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 3

    def setup_network(self):
        self.setup_nodes()

    def run_test(self):
        self.log.info("Make sure we repopulate setBlockIndexCandidates after InvalidateBlock:")
        self.log.info("Mine 4 blocks on Node 0")
        self.nodes[0].generate(4)
        assert_equal(self.nodes[0].getblockcount(), 4)
        besthash = self.nodes[0].getbestblockhash()

        self.log.info("Mine competing 10 blocks on Node 1")
        self.nodes[1].generate(10)
        genblocks = len(self.make_more_work_than(1,0))
        assert_equal(self.nodes[1].getblockcount(),10 + genblocks)

        self.log.info("Connect nodes to force a reorg")
        connect_nodes_bi(self.nodes,0,1)
        sync_blocks(self.nodes[0:2])
        assert_equal(self.nodes[0].getblockcount(), 10 + genblocks)
        badhash = self.nodes[1].getblockhash(2)

        self.log.info("Invalidate block 2 on node 0 and verify we reorg to node 0's original chain")
        self.nodes[0].invalidateblock(badhash)
        newheight = self.nodes[0].getblockcount()
        newhash = self.nodes[0].getbestblockhash()
        if (newheight != 4 or newhash != besthash):
            raise AssertionError("Wrong tip for node0, hash %s, height %d"%(newhash,newheight))

        self.log.info("Make sure we won't reorg to a lower work chain:")
        connect_nodes_bi(self.nodes,1,2)
        self.log.info("Sync node 2 to node 1 so both have 6 blocks")
        sync_blocks(self.nodes[1:3])
        assert_equal(self.nodes[2].getblockcount(), 10 + genblocks)
        self.log.info("Invalidate block 5 on node 1 so its tip is now at 4")
        self.nodes[1].invalidateblock(self.nodes[1].getblockhash(5))
        assert_equal(self.nodes[1].getblockcount(), 4)
        self.log.info("Invalidate block 3 on node 2, so its tip is now 2")
        self.nodes[2].invalidateblock(self.nodes[2].getblockhash(3))
        assert_equal(self.nodes[2].getblockcount(), 2)
        self.log.info("..and then mine a block")
        for n in self.nodes:
            print(n.getblockcount())
        self.nodes[2].generate(1)
        self.log.info("Verify all nodes are at the right height")
        for n in self.nodes:
            print(n.getblockcount())
        wait_until(lambda: self.nodes[2].getblockcount() == 3, timeout=30)
        wait_until(lambda: self.nodes[0].getblockcount() == 4, timeout=30)
        wait_until(lambda: self.nodes[1].getblockcount() == 4, timeout=30)
        node1height = self.nodes[1].getblockcount()
        if node1height < 4:
            raise AssertionError("Node 1 reorged to a lower height: %d"%node1height)

        self.log.info("Verify that we reconsider all ancestors as well")
        blocks = self.nodes[1].generate(10)
        assert_equal(self.nodes[1].getbestblockhash(), blocks[-1])
        # Invalidate the two blocks at the tip
        self.nodes[1].invalidateblock(blocks[-1])
        self.nodes[1].invalidateblock(blocks[-2])
        assert_equal(self.nodes[1].getbestblockhash(), blocks[-3])
        # Reconsider only the previous tip
        self.nodes[1].reconsiderblock(blocks[-1])
        # Should be back at the tip by now
        assert_equal(self.nodes[1].getbestblockhash(), blocks[-1])

        self.log.info("Verify that we reconsider all descendants")
        blocks = self.nodes[1].generate(10)
        assert_equal(self.nodes[1].getbestblockhash(), blocks[-1])
        # Invalidate the two blocks at the tip
        self.nodes[1].invalidateblock(blocks[-2])
        self.nodes[1].invalidateblock(blocks[-4])
        assert_equal(self.nodes[1].getbestblockhash(), blocks[-5])
        # Reconsider only the previous tip
        self.nodes[1].reconsiderblock(blocks[-4])
        # Should be back at the tip by now
        assert_equal(self.nodes[1].getbestblockhash(), blocks[-1])

if __name__ == '__main__':
    InvalidateTest().main()
