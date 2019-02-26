#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""
Smart Contract Testing - softfork test

测试合约相关的分叉情况

开4个节点,节点1,2为1组，3，4为1组
主要测试以下情况：
发布：
    1.同一个utxo，分叉引用发布合约
        a.utxo为普通交易输出
        b.utxo为合约输出
调用:
    1.链高度不同
        a.调用同一合约，不含send操作
        b.调用同一合约，含send操作
        c.切换主链后，前主链产出的块不计入新主链

"""
# Imports should be in PEP8 ordering (std library first, then third party
# libraries then local imports).
from decimal import Decimal

# Avoid wildcard * imports if possible
from test_framework.test_framework import MagnaChainTestFramework
from test_framework.mininode import COIN
from test_framework.script import CScript
from test_framework.util import (
    assert_equal,
    assert_greater_than,
    assert_raises_rpc_error,
    assert_contains,
    generate_contract,
    sync_mempools,
    sync_blocks,
    bytes_to_hex_str,
    hex_str_to_bytes
)
from test_framework.contract import Contract

def get_contract_hex(contract):
    with open(contract) as fh:
        content = "".join(fh.readlines())
    return  bytes_to_hex_str(bytes(content, encoding='utf-8'))

class ContractForkTest(MagnaChainTestFramework):
    def set_test_params(self):
        """Override test parameters for your individual test.

        This method must be overridden and num_nodes must be exlicitly set."""
        self.setup_clean_chain = True
        self.num_nodes = 4

    def run_test(self):
        """Main test logic"""
        # prepare
        for i,node in enumerate(self.nodes):
            # for convenient
            setattr(self,'node' + str(i),node)

        for n in self.nodes:
            n.generate(2)  # make some coins
            self.sync_all()

        # main test
        self.contract_file = generate_contract(self.options.tmpdir)
        self.test_publish_fork_with_utxo()
        # self.test_publish_fork_with_utxo(is_contract_output=True)


    def test_publish_fork_with_utxo(self,is_contract_output = False):
        '''
             ab0[utxo1]
          /         \
        aa1 [ca1]   bb1 [cb1]
         |           |
        aa2         bb2
         |           |
        aa3         bb3
                     |
                    bb4
                     |
                    ...
        :param contract_output:
        :return:
        '''
        hex_content = get_contract_hex(self.contract_file)
        coster = self.node0.getnewaddress()
        if is_contract_output:
            ct = Contract(self.node0)
            ct.call_payable(amount = 2000)
            ct.call_sendCoinTest(coster,1000)
        else:
            self.node0.sendtoaddress(coster, 1000)
        self.node0.generate(2)
        self.sync_all()
        '''
        分割成2个网络，然后各自用同一个utxo发布合约
        '''
        self.split_network()
        sender_pub = self.node0.validateaddress(coster)['pubkey']
        sender_pri = self.node0.dumpprivkey(coster)
        amount = 0
        changeaddress = self.node0.getnewaddress()
        result = self.publish_contract(self.node0,hex_content,coster,sender_pub,sender_pri,amount,changeaddress)
        print(result)
        txid_a1 = result['txid']
        contract_a1 = result['contractaddress']
        # 第一组节点同步,这是该组链高度应该为12
        block_a1,block_a2 = self.node0.generate(2)
        self.sync_all([self.nodes[:2], self.nodes[2:]])
        assert_equal(self.node1.getblockcount(),12)
        assert_equal(self.node1.getbestblockhash(), block_a2)
        last_block_hash =  block_a2

        # 第二组开始发布合约
        amount = 1
        changeaddress = self.node2.getnewaddress()
        result = self.publish_contract(self.node2,hex_content, coster, sender_pub, sender_pri,amount, changeaddress)
        print(result)
        txid_b1 = result['txid']
        contract_b1 = result['contractaddress']
        # 第二组节点同步,这是该组链高度应该为22
        block_b13 = self.node2.generate(12)[11]
        self.sync_all([self.nodes[:2], self.nodes[2:]])
        assert_equal(self.node2.getblockcount(),22)
        assert_equal(self.node2.getbestblockhash(),block_b13)

        # 合并网络
        for i in range(4):
            print("before join:",i,self.nodes[i].getblockcount(),int(self.nodes[i].getchaintipwork(), 16) )
        self.join_network()
        for i in range(4):
            print(i,self.nodes[i].getblockcount(),int(self.nodes[i].getchaintipwork(), 16) )
        # 确认存在分叉存在，并且主链为21个区块的
        tips = self.nodes[0].getchaintips ()
        assert_equal (len (tips), 2)
        assert_equal(self.node2.getblockcount(), 22)
        assert_equal(self.node2.getbestblockhash(), block_b13)
        print(self.node0.gettransaction(txid_a1))
        # 确认分叉上的合约在主链上不能被调用
        assert_raises_rpc_error(-1, "CallContractReal => GetContractInfo fail, contractid is " + contract_a1,self.node0.callcontract, True,
                                1, contract_a1, self.node0.getnewaddress(), "payable")
        # 主链合约是可以调用的
        self.node0.callcontract(True, 1, contract_b1, self.node0.getnewaddress(), "payable")

        # listsinceblock(lastblockhash) should now include txid_a1, as seen from nodes[0]
        lsbres = self.nodes[1].listsinceblock(last_block_hash)
        assert any(tx['txid'] == txid_a1 for tx in lsbres['removed'])

        # but it should not include 'removed' if include_removed=false
        lsbres2 = self.nodes[1].listsinceblock(blockhash=last_block_hash, include_removed=False)
        assert 'removed' not in lsbres2
        '''

        lsbres = self.nodes[0].listsinceblock(last_block_hash,1)
        found_b1 = False
        found_a1 = False
        for tx in lsbres['transactions']:
            if tx['txid'] == txid_b1:
                found_b1 = True
            if tx['txid'] == txid_a1:
                found_a1 = True
        print(found_a1,found_b1)
        assert found_b1 and not found_a1
        '''


    def publish_contract(self,node,hex_content, coster, sender_pub, sender_pri,amount, changeaddress):
        pre_transaction = node.prepublishcode(hex_content, coster, sender_pub, amount, changeaddress)
        txhex = pre_transaction['txhex']
        spent_utxo = pre_transaction['coins']
        print(spent_utxo)
        prevtxs = []
        for ele in spent_utxo:
            info = {}
            info['txid'] = ele['txhash']
            info['vout'] = ele['outn']
            info['amount'] = ele['value']
            info['scriptPubKey'] = ele['script']
            prevtxs.append(info)
        signed_tx = node.signrawtransaction(txhex, prevtxs, [sender_pri, sender_pri])
        return node.sendrawtransaction(signed_tx['hex'])

if __name__ == '__main__':
    ContractForkTest().main()

