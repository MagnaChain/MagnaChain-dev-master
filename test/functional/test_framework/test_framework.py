#!/usr/bin/env python3
# Copyright (c) 2014-2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Base class for RPC testing."""

from collections import deque
from enum import Enum
import logging
import optparse
import os
import pdb
import shutil
import sys
import tempfile
import time
import traceback

from .authproxy import JSONRPCException
from . import coverage
from .test_node import TestNode
from .util import (
    MAX_NODES,
    PortSeed,
    assert_equal,
    check_json_precision,
    connect_nodes_bi,
    disconnect_nodes,
    initialize_datadir,
    log_filename,
    p2p_port,
    set_node_times,
    sync_blocks,
    sync_mempools,
)


class TestStatus(Enum):
    PASSED = 1
    FAILED = 2
    SKIPPED = 3


TEST_EXIT_PASSED = 0
TEST_EXIT_FAILED = 1
TEST_EXIT_SKIPPED = 77


class MagnaChainTestFramework(object):
    """Base class for a magnachain test script.

    Individual magnachain test scripts should subclass this class and override the set_test_params() and run_test() methods.

    Individual tests can also override the following methods to customize the test setup:

    - add_options()
    - setup_chain()
    - setup_network()
    - setup_nodes()

    The __init__() and main() methods should not be overridden.

    This class also contains various public and private helper methods."""

    def __init__(self):
        """Sets test framework defaults. Do not override this method. Instead, override the set_test_params() method"""
        self.setup_clean_chain = False
        self.nodes = []
        self.sidenodes = []
        self.mocktime = 0
        # mapped，侧链对主链的映射，eg.[[0],[],[],[1]],表示侧链的0节点attach在主链的节点0；侧链1节点attach在主链的节点3
        self.mapped = []
        # self.mortgage_coins = [] #抵押币的txid，赎回抵押币时会用到
        self.set_test_params()

        assert hasattr(self, "num_nodes"), "Test must set self.num_nodes in set_test_params()"

    def main(self):
        """Main function. This should not be overridden by the subclass test scripts."""

        parser = optparse.OptionParser(usage="%prog [options]")
        parser.add_option("--nocleanup", dest="nocleanup", default=False, action="store_true",
                          help="Leave magnachainds and test.* datadir on exit or error")
        parser.add_option("--noshutdown", dest="noshutdown", default=False, action="store_true",
                          help="Don't stop magnachainds after the test execution")
        parser.add_option("--srcdir", dest="srcdir",
                          default=os.path.normpath(os.path.dirname(os.path.realpath(__file__)) + "/../../../src"),
                          help="Source directory containing magnachaind/magnachain-cli (default: %default)")
        parser.add_option("--cachedir", dest="cachedir",
                          default=os.path.normpath(os.path.dirname(os.path.realpath(__file__)) + "/../../cache"),
                          help="Directory for caching pregenerated datadirs")
        parser.add_option("--tmpdir", dest="tmpdir", help="Root directory for datadirs")
        parser.add_option("-l", "--loglevel", dest="loglevel", default="INFO",
                          help="log events at this level and higher to the console. Can be set to DEBUG, INFO, WARNING, ERROR or CRITICAL. Passing --loglevel DEBUG will output all logs to console. Note that logs at all levels are always written to the test_framework.log file in the temporary test directory.")
        parser.add_option("--tracerpc", dest="trace_rpc", default=False, action="store_true",
                          help="Print out all RPC calls as they are made")
        parser.add_option("--portseed", dest="port_seed", default=os.getpid(), type='int',
                          help="The seed to use for assigning port numbers (default: current process id)")
        parser.add_option("--coveragedir", dest="coveragedir",
                          help="Write tested RPC commands into this directory")
        parser.add_option("--configfile", dest="configfile",
                          help="Location of the test framework config file")
        parser.add_option("--pdbonfailure", dest="pdbonfailure", default=False, action="store_true",
                          help="Attach a python debugger if test fails")
        self.add_options(parser)
        (self.options, self.args) = parser.parse_args()

        PortSeed.n = self.options.port_seed

        os.environ['PATH'] = self.options.srcdir + ":" + self.options.srcdir + "/qt:" + os.environ['PATH']

        check_json_precision()

        self.options.cachedir = os.path.abspath(self.options.cachedir)

        # Set up temp directory and start logging
        if self.options.tmpdir:
            self.options.tmpdir = os.path.abspath(self.options.tmpdir)
            os.makedirs(self.options.tmpdir, exist_ok=False)
        else:
            self.options.tmpdir = tempfile.mkdtemp(prefix="test")
        self._start_logging()

        success = TestStatus.FAILED

        try:
            self.setup_chain()
            self.setup_network()
            if getattr(self, "num_sidenodes", 0) > 0:
                self.setup_sidechain()
            self.__for_convenient()
            self.run_test()
            success = TestStatus.PASSED
        except JSONRPCException as e:
            self.log.exception("JSONRPC error")
        except SkipTest as e:
            self.log.warning("Test Skipped: %s" % e.message)
            success = TestStatus.SKIPPED
        except AssertionError as e:
            self.log.exception("Assertion failed")
        except KeyError as e:
            self.log.exception("Key error")
        except Exception as e:
            self.log.exception("Unexpected exception caught during testing")
        except KeyboardInterrupt as e:
            self.log.warning("Exiting after keyboard interrupt")

        if success == TestStatus.FAILED and self.options.pdbonfailure:
            print("Testcase failed. Attaching python debugger. Enter ? for help")
            pdb.set_trace()

        if not self.options.noshutdown:
            self.log.info("Stopping nodes")
            if self.nodes:
                self.stop_nodes()
        else:
            self.log.info("Note: magnachainds were not stopped and may still be running")

        if not self.options.nocleanup and not self.options.noshutdown and success != TestStatus.FAILED:
            self.log.info("Cleaning up")
            shutil.rmtree(self.options.tmpdir)
        else:
            self.log.warning("Not cleaning up dir %s" % self.options.tmpdir)
            if os.getenv("PYTHON_DEBUG", ""):
                # Dump the end of the debug logs, to aid in debugging rare
                # travis failures.
                import glob
                filenames = [self.options.tmpdir + "/test_framework.log"]
                filenames += glob.glob(self.options.tmpdir + "/node*/regtest/debug.log")
                MAX_LINES_TO_PRINT = 1000
                for fn in filenames:
                    try:
                        with open(fn, 'r') as f:
                            print("From", fn, ":")
                            print("".join(deque(f, MAX_LINES_TO_PRINT)))
                    except OSError:
                        print("Opening file %s failed." % fn)
                        traceback.print_exc()

        if success == TestStatus.PASSED:
            self.log.info("Tests successful")
            sys.exit(TEST_EXIT_PASSED)
        elif success == TestStatus.SKIPPED:
            self.log.info("Test skipped")
            sys.exit(TEST_EXIT_SKIPPED)
        else:
            self.log.error("Test failed. Test logging available at %s/test_framework.log", self.options.tmpdir)
            logging.shutdown()
            sys.exit(TEST_EXIT_FAILED)

    # Methods to override in subclass test scripts.
    def set_test_params(self):
        """Tests must this method to change default values for number of nodes, topology, etc"""
        raise NotImplementedError

    def add_options(self, parser):
        """Override this method to add command-line options to the test"""
        pass

    def setup_chain(self):
        """Override this method to customize blockchain setup"""
        self.log.info("Initializing test directory " + self.options.tmpdir)
        if self.setup_clean_chain:
            self._initialize_chain_clean()
        else:
            self._initialize_chain()

    def setup_network(self, sidechain=False):
        """Override this method to customize test network topology"""
        self.setup_nodes(sidechain=sidechain)

        # Connect the nodes as a "chain".  This allows us
        # to split the network between nodes 1 and 2 to get
        # two halves that can work on competing chains.
        node_num = (self.num_nodes if not sidechain else self.num_sidenodes)
        nodes = self.nodes if not sidechain else self.sidenodes
        for i in range(node_num - 1):
            connect_nodes_bi(nodes, i, i + 1, sidechain=sidechain)
        self.sync_all([nodes])

    def setup_nodes(self, sidechain=False):
        """Override this method to customize test node setup"""
        extra_args = None
        if not sidechain:
            if hasattr(self, "extra_args"):
                extra_args = self.extra_args
            if not getattr(self, 'rpc_timewait', 0):
                self.add_nodes(self.num_nodes, extra_args)
            else:
                self.add_nodes(self.num_nodes, extra_args, timewait=self.rpc_timewait)
        else:
            if hasattr(self, "side_extra_args"):
                extra_args = self.side_extra_args
            if not getattr(self, 'rpc_timewait', 0):
                self.add_nodes(self.num_sidenodes, extra_args, sidechain=True)
            else:
                self.add_nodes(self.num_sidenodes, extra_args, sidechain=True, timewait=self.rpc_timewait)
        self.start_nodes(sidechain=sidechain)

    # 支链相关
    def setup_sidechain(self):
        """Override this method to customize test sidenode setup"""
        # todo 多侧链支持
        # 目前主节点与侧节点只能是1对1关系，不支持1对多
        assert self.num_nodes >= self.num_sidenodes
        self.log.info("setup sidechain")
        # 创建抵押币
        # for convince
        node = self.nodes[0]
        logger = self.log.info
        node.generate(2)
        sidechain_id = node.createbranchchain("clvseeds.com", "00:00:00:00:00:00:00:00:00:00:ff:ff:c0:a8:3b:80:8333",
                                              node.getnewaddress())['branchid']
        self.sidechain_id = sidechain_id
        node.generate(1)
        self.sync_all()
        logger("sidechain id is {}".format(sidechain_id))
        # 创建magnachaind的软链接，为了区分主链和侧链
        if not os.path.exists(os.path.join(self.options.srcdir, 'magnachaind-side')):
            try:
                os.system("ln -s {} {}".format(os.path.join(self.options.srcdir, 'magnachaind'),
                                               os.path.join(self.options.srcdir, 'magnachaind-side')))
            except Exception as e:
                pass

        # Set env vars
        if "MAGNACHAIND_SIDE" not in os.environ:
            os.environ["MAGNACHAIND_SIDE"] = os.path.join(self.options.srcdir, 'magnachaind-side')

        # 初始化侧链目录
        logger("create sidechain datadir")
        side_datadirs = []
        if not self.mapped:
            self.mapped = [[i] for i in range(self.num_sidenodes)]
        for i in range(self.num_sidenodes):
            attach_index = None
            # 处理特定的节点映射
            # 最多只能是1对1
            all([len(m) == 1 for m in self.mapped])
            for index, m in enumerate(self.mapped):
                if i in m:
                    attach_index = index
                    break
            if not attach_index:
                attach_index = i
            logger("sidenode{} attach to mainnode{}".format(i, attach_index))
            side_datadirs.append(
                initialize_datadir(self.options.tmpdir, i, sidechain_id=sidechain_id,
                                   mainport=self.nodes[attach_index].rpcport,
                                   main_datadir=os.path.join(self.options.tmpdir, 'node{}'.format(attach_index))))
        logger("setup sidechain network and start side nodes")
        self.setup_network(sidechain=True)
        logger("sidechain attach to mainchains")
        for index, m in enumerate(self.mapped):
            if m:
                # 只有主节点有被挂载时才处理
                self.nodes[index].generate(2)  # make some coins
                self.sync_all()
                # addbranchnode接口会覆盖旧的配置。目前主节点与侧节点只能是1对1关系，不支持1对多
                ret = self.nodes[index].addbranchnode(sidechain_id, '127.0.0.1', self.sidenodes[m[0]].rpcport, '', '',
                                                      '', side_datadirs[m[0]])
                if ret != 'ok':
                    raise Exception(ret)
        for index, m in enumerate(self.mapped):
            if m:
                logger("mortgage coins to sidenode{}".format(m[0]))
                for j in range(10):
                    addr = self.sidenodes[m[0]].getnewaddress()
                    txid = self.nodes[index].mortgageminebranch(sidechain_id, 5000, addr)['txid']  # 抵押挖矿币
                self.nodes[index].generate(10)
                self.sync_all()
                assert self.sidenodes[m[0]].getmempoolinfo()['size'] > 0
        self.sync_all()
        logger("sidechains setup done")

    def run_test(self):
        """Tests must override this method to define test logic"""
        raise NotImplementedError

    # Public helper methods. These can be accessed by the subclass test scripts.

    def add_nodes(self, num_nodes, extra_args=None, rpchost=None, timewait=None, binary=None, sidechain=False):
        """Instantiate TestNode objects"""

        if extra_args is None:
            extra_args = [[]] * num_nodes
        if binary is None:
            binary = [None] * num_nodes
        assert_equal(len(extra_args), num_nodes)
        assert_equal(len(binary), num_nodes)
        for i in range(num_nodes):
            n = TestNode(i, self.options.tmpdir, extra_args[i], rpchost, timewait=timewait, binary=binary[i],
                         stderr=None, mocktime=self.mocktime, coverage_dir=self.options.coveragedir,
                         sidechain=sidechain)
            if not sidechain:
                self.nodes.append(n)
            else:
                self.sidenodes.append(n)

    def start_node(self, i, extra_args=None, stderr=None):
        """Start a magnachaind"""

        node = self.nodes[i]

        node.start(extra_args, stderr)
        node.wait_for_rpc_connection()

        if self.options.coveragedir is not None:
            coverage.write_all_rpc_commands(self.options.coveragedir, node.rpc)

    def start_nodes(self, extra_args=None, sidechain=False):
        """Start multiple magnachainds"""
        nodes = []
        if extra_args is None:
            if not sidechain:
                extra_args = [None] * self.num_nodes
                assert_equal(len(extra_args), self.num_nodes)
                nodes = self.nodes
            else:
                extra_args = [None] * self.num_sidenodes
                assert_equal(len(extra_args), self.num_sidenodes)
                nodes = self.sidenodes
        try:
            if not nodes:
                if sidechain:
                    nodes = self.sidenodes
                else:
                    nodes = self.nodes
            for i, node in enumerate(nodes):
                node.start(extra_args[i])
            for node in nodes:
                node.wait_for_rpc_connection()
        except:
            # If one node failed to start, stop the others
            self.stop_nodes()
            raise

        if self.options.coveragedir is not None:
            for node in nodes:
                coverage.write_all_rpc_commands(self.options.coveragedir, node.rpc)

    def stop_node(self, i):
        """Stop a magnachaind test node"""
        self.nodes[i].stop_node()
        self.nodes[i].wait_until_stopped()

    def stop_nodes(self):
        """Stop multiple magnachaind test nodes"""
        all_nodes = self.nodes + self.sidenodes
        for node in all_nodes:
            # Issue RPC to stop nodes
            node.stop_node()

        for node in all_nodes:
            # Wait for nodes to stop
            node.wait_until_stopped()

    def assert_start_raises_init_error(self, i, extra_args=None, expected_msg=None):
        with tempfile.SpooledTemporaryFile(max_size=2 ** 16) as log_stderr:
            try:
                self.start_node(i, extra_args, stderr=log_stderr)
                self.stop_node(i)
            except Exception as e:
                assert 'magnachaind exited' in str(e)  # node must have shutdown
                self.nodes[i].running = False
                self.nodes[i].process = None
                if expected_msg is not None:
                    log_stderr.seek(0)
                    stderr = log_stderr.read().decode('utf-8')
                    if expected_msg not in stderr:
                        raise AssertionError("Expected error \"" + expected_msg + "\" not found in:\n" + stderr)
            else:
                if expected_msg is None:
                    assert_msg = "magnachaind should have exited with an error"
                else:
                    assert_msg = "magnachaind should have exited with expected error " + expected_msg
                raise AssertionError(assert_msg)

    def wait_for_node_exit(self, i, timeout):
        self.nodes[i].process.wait(timeout)

    def split_network(self):
        """
        Split the network of four nodes into nodes 0/1 and 2/3.
        """
        disconnect_nodes(self.nodes[1], 2)
        disconnect_nodes(self.nodes[2], 1)
        self.sync_all([self.nodes[:2], self.nodes[2:]])

    def join_network(self, timeout=60):
        """
        Join the (previously split) network halves together.
        """
        connect_nodes_bi(self.nodes, 1, 2)
        self.sync_all(timeout=timeout)

    """make a chain have more work than b"""

    def make_more_work_than(self, a, b):
        bwork = int(self.nodes[b].getchaintipwork(), 16)
        genblocks = []
        while int(self.nodes[a].getchaintipwork(), 16) <= bwork:
            genblocks.append(self.nodes[a].generate(1)[0])
        if bwork == int(self.nodes[a].getchaintipwork(), 16):
            genblocks.append(self.nodes[a].generate(1)[0])
        if len(genblocks) > 0:
            self.log.info("make more work by gen %d" % (len(genblocks)))
        return genblocks

    def sync_all(self, node_groups=None, show_max_height=False, timeout=60):
        if not node_groups:
            node_groups = [self.nodes]

        if show_max_height:
            self.log.info("syncall group : %s" % (str([len(g) for g in node_groups])))
        for group in node_groups:
            logger = self.log if show_max_height else None
            sync_blocks(group, logger=logger, timeout=timeout)
            sync_mempools(group, timeout=timeout)

    def enable_mocktime(self):
        """Enable mocktime for the script.

        mocktime may be needed for scripts that use the cached version of the
        blockchain.  If the cached version of the blockchain is used without
        mocktime then the mempools will not sync due to IBD.

        For backwared compatibility of the python scripts with previous
        versions of the cache, this helper function sets mocktime to Jan 1,
        2014 + (201 * 10 * 60)"""
        self.mocktime = 1388534400 + (201 * 10 * 60)

    def disable_mocktime(self):
        self.mocktime = 0

    # Private helper methods. These should not be accessed by the subclass test scripts.

    def _start_logging(self):
        # Add logger and logging handlers
        self.log = logging.getLogger('TestFramework')
        self.log.setLevel(logging.DEBUG)
        # Create file handler to log all messages
        fh = logging.FileHandler(self.options.tmpdir + '/test_framework.log')
        fh.setLevel(logging.DEBUG)
        # Create console handler to log messages to stderr. By default this logs only error messages, but can be configured with --loglevel.
        ch = logging.StreamHandler(sys.stdout)
        # User can provide log level as a number or string (eg DEBUG). loglevel was caught as a string, so try to convert it to an int
        ll = int(self.options.loglevel) if self.options.loglevel.isdigit() else self.options.loglevel.upper()
        ch.setLevel(ll)
        # Format logs the same as magnachaind's debug.log with microprecision (so log files can be concatenated and sorted)
        formatter = logging.Formatter(fmt='%(asctime)s.%(msecs)03d000 %(name)s (%(levelname)s): %(message)s',
                                      datefmt='%Y-%m-%d %H:%M:%S')
        formatter.converter = time.gmtime
        fh.setFormatter(formatter)
        ch.setFormatter(formatter)
        # add the handlers to the logger
        self.log.addHandler(fh)
        self.log.addHandler(ch)

        if self.options.trace_rpc:
            rpc_logger = logging.getLogger("MagnaChainRPC")
            rpc_logger.setLevel(logging.DEBUG)
            rpc_handler = logging.StreamHandler(sys.stdout)
            rpc_handler.setLevel(logging.DEBUG)
            rpc_logger.addHandler(rpc_handler)

    def _initialize_chain(self):
        """Initialize a pre-mined blockchain for use by the test.

        Create a cache of a 200-block-long chain (with wallet) for MAX_NODES
        Afterward, create num_nodes copies from the cache."""

        assert self.num_nodes <= MAX_NODES
        create_cache = False
        for i in range(MAX_NODES):
            if not os.path.isdir(os.path.join(self.options.cachedir, 'node' + str(i))):
                create_cache = True
                break

        if create_cache:
            self.log.debug("Creating data directories from cached datadir")

            # find and delete old cache directories if any exist
            for i in range(MAX_NODES):
                if os.path.isdir(os.path.join(self.options.cachedir, "node" + str(i))):
                    shutil.rmtree(os.path.join(self.options.cachedir, "node" + str(i)))

            # Create cache directories, run magnachainds:
            for i in range(MAX_NODES):
                datadir = initialize_datadir(self.options.cachedir, i)
                args = [os.getenv("MAGNACHAIND", "magnachaind"), "-server", "-keypool=1", "-datadir=" + datadir,
                        "-discover=0"]
                if i > 0:
                    args.append("-connect=127.0.0.1:" + str(p2p_port(0)))
                self.nodes.append(
                    TestNode(i, self.options.cachedir, extra_args=[], rpchost=None, timewait=None, binary=None,
                             stderr=None, mocktime=self.mocktime, coverage_dir=None))
                self.nodes[i].args = args
                self.start_node(i)

            # Wait for RPC connections to be ready
            for node in self.nodes:
                node.wait_for_rpc_connection()

            # Create a 200-block-long chain; each of the 4 first nodes
            # gets 25 mature blocks and 25 immature.
            # Note: To preserve compatibility with older versions of
            # initialize_chain, only 4 nodes will generate coins.
            #
            # blocks are created with timestamps 10 minutes apart
            # starting from 2010 minutes in the past
            self.enable_mocktime()
            block_time = self.mocktime - (201 * 10 * 60)
            for i in range(2):
                for peer in range(4):
                    for j in range(25):
                        set_node_times(self.nodes, block_time)
                        self.nodes[peer].generate(1)
                        block_time += 10 * 60
                    # Must sync before next peer starts generating blocks
                    sync_blocks(self.nodes)

            # Shut them down, and clean up cache directories:
            self.stop_nodes()
            self.nodes = []
            self.disable_mocktime()
            for i in range(MAX_NODES):
                os.remove(log_filename(self.options.cachedir, i, "debug.log"))
                os.remove(log_filename(self.options.cachedir, i, "db.log"))
                os.remove(log_filename(self.options.cachedir, i, "peers.dat"))
                os.remove(log_filename(self.options.cachedir, i, "fee_estimates.dat"))

        for i in range(self.num_nodes):
            from_dir = os.path.join(self.options.cachedir, "node" + str(i))
            to_dir = os.path.join(self.options.tmpdir, "node" + str(i))
            shutil.copytree(from_dir, to_dir)
            initialize_datadir(self.options.tmpdir, i)  # Overwrite port/rpcport in magnachain.conf

    def _initialize_chain_clean(self):
        """Initialize empty blockchain for use by the test.

        Create an empty blockchain and num_nodes wallets.
        Useful if a test case wants complete control over initialization."""
        for i in range(self.num_nodes):
            initialize_datadir(self.options.tmpdir, i)

    def __for_convenient(self):
        '''
        be convenient for self.node0 to call
        :return:
        '''
        for i, node in enumerate(self.nodes):
            # for convenient
            setattr(self, 'node' + str(i), node)

        for i, node in enumerate(self.sidenodes):
            # for convenient
            setattr(self, 'snode' + str(i), node)


class ComparisonTestFramework(MagnaChainTestFramework):
    """Test framework for doing p2p comparison testing

    Sets up some magnachaind binaries:
    - 1 binary: test binary
    - 2 binaries: 1 test binary, 1 ref binary
    - n>2 binaries: 1 test binary, n-1 ref binaries"""

    def set_test_params(self):
        self.num_nodes = 2
        self.setup_clean_chain = True

    def add_options(self, parser):
        parser.add_option("--testbinary", dest="testbinary",
                          default=os.getenv("MAGNACHAIND", "magnachaind"),
                          help="magnachaind binary to test")
        parser.add_option("--refbinary", dest="refbinary",
                          default=os.getenv("MAGNACHAIND", "magnachaind"),
                          help="magnachaind binary to use for reference nodes (if any)")

    def setup_network(self):
        extra_args = [['-whitelist=127.0.0.1']] * self.num_nodes
        if hasattr(self, "extra_args"):
            extra_args = self.extra_args
        self.add_nodes(self.num_nodes, extra_args,
                       binary=[self.options.testbinary] +
                              [self.options.refbinary] * (self.num_nodes - 1))
        self.start_nodes()


class SkipTest(Exception):
    """This exception is raised to skip a test"""

    def __init__(self, message):
        self.message = message
