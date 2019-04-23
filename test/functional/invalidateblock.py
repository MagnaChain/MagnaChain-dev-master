#!/usr/bin/env python3
# Copyright (c) 2014-2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the invalidateblock RPC."""
import time

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
        pre_node0_chainwoek = int(self.node0.getchaintipwork(), 16)
        assert_equal(self.nodes[0].getblockcount(), 4)
        besthash = self.nodes[0].getbestblockhash()

        self.log.info("Mine competing 10 blocks on Node 1")
        gen_times = 3
        for i in range(gen_times):
            self.nodes[1].generate(10)
        genblocks = len(self.make_more_work_than(1,0))
        assert_equal(self.nodes[1].getblockcount(),10 * gen_times + genblocks)

        self.log.info("Connect nodes to force a reorg")
        connect_nodes_bi(self.nodes,0,1)
        sync_blocks(self.nodes[0:2])
        assert_equal(self.nodes[0].getblockcount(), 10 * gen_times + genblocks)
        badhash = self.nodes[1].getblockhash(2)

        self.log.info("Invalidate block 2 on node 0 and verify we reorg to node 0's original chain")
        self.nodes[0].invalidateblock(badhash)
        newheight = self.nodes[0].getblockcount()
        newhash = self.nodes[0].getbestblockhash()
        if int(self.node0.getchaintipwork(), 16) > pre_node0_chainwoek:
            # 证明node1的工作量证明大于原来node0的了，newheight应该为1
            self.log.info("node1's chainwork bigger than node0")
            if (newheight != 1):
                raise AssertionError("Wrong tip for node0, hash %s, height %d"%(newhash,newheight))
        else:
            # 相等的情况不处理
            # 证明node1的工作量证明小于原来node0的了，newheight应该为4
            self.log.info("node1's chainwork less than node0")
            if (newheight != 4):
                raise AssertionError("Wrong tip for node0, hash %s, height %d"%(newhash,newheight))

        self.log.info("Make sure we won't reorg to a lower work chain:")
        connect_nodes_bi(self.nodes,1,2)
        self.log.info("Sync node 2 to node 1 so both have 6 blocks")
        sync_blocks(self.nodes[1:3])
        assert_equal(self.nodes[2].getblockcount(), 10 * gen_times + genblocks)
        self.log.info("Invalidate block 20 on node 1 so its tip is now at 19")
        # 最后一个块的工作量有可能好大，使其失效后，会导致节点0的链为最长链
        self.nodes[1].invalidateblock(self.nodes[1].getblockhash(20))
        assert_equal(self.nodes[1].getblockcount(), 19)
        for n in self.nodes:
            print(n.getblockcount(),int(n.getchaintipwork(), 16))
        self.log.info("Invalidate block 3 on node 2, so its tip is now 2")
        self.nodes[2].invalidateblock(self.nodes[2].getblockhash(3))
        time.sleep(10) #wair for sync
        for n in self.nodes:
            print(n.getblockcount(),get_chainwork(n))
        if get_chainwork(self.node0) == get_chainwork(self.node2):
            # 节点0的工作量是最大的，到最后节点0的链为主链
            assert_equal(self.nodes[2].getblockcount(), 4)
        else:
            assert_equal(self.nodes[2].getblockcount(), 2)
        self.log.info("..and then mine a block")
        for n in self.nodes:
            print(n.getblockcount(),int(n.getchaintipwork(), 16))
        node1_chainwork = get_chainwork(self.node1)
        self.nodes[2].generate(1)
        # check the chainwork
        if get_chainwork(self.node2) > node1_chainwork:
            node1_blockcount = self.node2.getblockcount()
            assert_equal(node1_blockcount,3)
        elif get_chainwork(self.node2) == node1_chainwork:
            node1_blockcount = self.node2.getblockcount()
            assert_equal(node1_blockcount,4)
        else:
            node1_blockcount = self.node1.getblockcount()
            assert_equal(node1_blockcount, 19)
        self.log.info("Verify all nodes are at the right height")
        for n in self.nodes:
            print(n.getblockcount(),int(n.getchaintipwork(), 16))
        wait_until(lambda: self.nodes[2].getblockcount() == (5 if get_chainwork(self.node1) == get_chainwork(self.node0) else 3), timeout=30)
        wait_until(lambda: self.nodes[0].getblockcount() == (4 if newheight == 4 else 1) , timeout=30)
        wait_until(lambda: self.nodes[1].getblockcount() == node1_blockcount, timeout=30)
        node1height = self.nodes[1].getblockcount()
        if node1height < node1_blockcount:
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

        #todo: need to add lots of contract transactions to block,then invalidateblock

if __name__ == '__main__':
    InvalidateTest().main()
