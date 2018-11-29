// Copyright (c) 2015 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_ZMQ_ZMQABSTRACTNOTIFIER_H
#define MAGNACHAIN_ZMQ_ZMQABSTRACTNOTIFIER_H

#include "zmq/zmqconfig.h"

class MCBlockIndex;
class CZMQAbstractNotifier;

typedef CZMQAbstractNotifier* (*CZMQNotifierFactory)();

class CZMQAbstractNotifier
{
public:
    CZMQAbstractNotifier() : psocket(0) { }
    virtual ~CZMQAbstractNotifier();

    template <typename T>
    static CZMQAbstractNotifier* Create()
    {
        return new T();
    }

    std::string GetType() const { return type; }
    void SetType(const std::string &t) { type = t; }
    std::string GetAddress() const { return address; }
    void SetAddress(const std::string &a) { address = a; }

    virtual bool Initialize(void *pcontext) = 0;
    virtual void Shutdown() = 0;

    virtual bool NotifyBlock(const MCBlockIndex *pindex);
    virtual bool NotifyTransaction(const MCTransaction &transaction);

protected:
    void *psocket;
    std::string type;
    std::string address;
};

#endif // MAGNACHAIN_ZMQ_ZMQABSTRACTNOTIFIER_H
