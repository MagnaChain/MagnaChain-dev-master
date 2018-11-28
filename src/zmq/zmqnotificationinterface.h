// Copyright (c) 2015-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_ZMQ_ZMQNOTIFICATIONINTERFACE_H
#define MAGNACHAIN_ZMQ_ZMQNOTIFICATIONINTERFACE_H

#include "validation/validationinterface.h"
#include <string>
#include <map>
#include <list>

class MCBlockIndex;
class CZMQAbstractNotifier;

class CZMQNotificationInterface : public MCValidationInterface
{
public:
    virtual ~CZMQNotificationInterface();

    static CZMQNotificationInterface* Create();

protected:
    bool Initialize();
    void Shutdown();

    // MCValidationInterface
    void TransactionAddedToMempool(const MCTransactionRef& tx) override;
    void BlockConnected(const std::shared_ptr<const MCBlock>& pblock, const MCBlockIndex* pindexConnected, const std::vector<MCTransactionRef>& vtxConflicted) override;
    void BlockDisconnected(const std::shared_ptr<const MCBlock>& pblock) override;
    void UpdatedBlockTip(const MCBlockIndex *pindexNew, const MCBlockIndex *pindexFork, bool fInitialDownload) override;

private:
    CZMQNotificationInterface();

    void *pcontext;
    std::list<CZMQAbstractNotifier*> notifiers;
};

#endif // MAGNACHAIN_ZMQ_ZMQNOTIFICATIONINTERFACE_H
