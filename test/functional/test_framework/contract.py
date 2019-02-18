#!/usr/bin/env python3
# Copyright (c) 2016 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Smart contract object"""
import os
import tempfile
import random


from test_framework.util import (
    assert_equal,
    assert_greater_than,
    assert_contains,
    generate_contract,
    caller_factory,
    sync_mempools,
    sync_blocks,
)

class Caller(object):
    """
    合约的实际调用类
    """
    def __init__(self,node,func,contract_id, sender):
        self.node = node
        self.func = func
        self.contract_id = contract_id
        self.sender = sender

    def __call__(self, *args,amount = random.randint(1,10000),throw_exception = True,broadcasting = True):
        try:
            print("%s,%s,%s,%s,%s"%(self.contract_id,self.func,self.sender,amount,args))
            return self.node.callcontract(broadcasting, amount, self.contract_id, self.sender, self.func, *args)
        except Exception as e:
            if throw_exception:
                raise
            print(repr(e))




class Contract(object):
    """
    合约类：
    封装合约的发布与调用，一个合约对象代表一份合约
    合约实例化时，需要绑定对应的节点实例，当调用时，默认是在绑定的节点调用，或者指定node参数，在某个节点执行档次调用
    """
    def __init__(self,node,contract_path = None,sender = None,immediate = True):
        '''

        :param node:
        :param contract_path:
        :param sender:
        :param immediate:
        '''
        self.bind_node = node
        if contract_path is None:
            contract_path = tempfile.mkdtemp(prefix="contract_")
        self.contract_path = generate_contract(contract_path)
        if sender is not None:
            self._sender = sender

        self.has_publish = False
        if immediate:
            # if true than publish it right now
            self.publish()


    def publish(self):
        '''
        publish the contract
        :return:
        '''
        if not self.has_publish:
            result = self.bind_node.publishcontract(self.contract_path)
            self.contract_id = result['contractaddress']
            self.sender = result['senderaddress']
            self.has_publish = True


    @property
    def sender(self):
        return self._sender

    @sender.setter
    def sender(self, value):
        self._sender = value

    def __getattr__(self, item):
        '''
        通过类似call_xxxxx的方式来调用合约的接口
        :param item:
        :return:
        '''
        if not self.has_publish:
            raise Exception("contract not publish,can not be call")
        if item.startswith('call_'):
            item = item.replace('call_','',1)
            return Caller(self.bind_node,item,self.contract_id, self.sender)
        raise AttributeError




