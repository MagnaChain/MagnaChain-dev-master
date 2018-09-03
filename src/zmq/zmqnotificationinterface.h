// Copyright (c) 2015-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_ZMQ_ZMQNOTIFICATIONINTERFACE_H
#define CELLLINK_ZMQ_ZMQNOTIFICATIONINTERFACE_H

#include "validation/validationinterface.h"
#include <string>
#include <map>
#include <list>

class CellBlockIndex;
class CZMQAbstractNotifier;

class CZMQNotificationInterface : public CellValidationInterface
{
public:
    virtual ~CZMQNotificationInterface();

    static CZMQNotificationInterface* Create();

protected:
    bool Initialize();
    void Shutdown();

    // CellValidationInterface
    void TransactionAddedToMempool(const CellTransactionRef& tx) override;
    void BlockConnected(const std::shared_ptr<const CellBlock>& pblock, const CellBlockIndex* pindexConnected, const std::vector<CellTransactionRef>& vtxConflicted) override;
    void BlockDisconnected(const std::shared_ptr<const CellBlock>& pblock) override;
    void UpdatedBlockTip(const CellBlockIndex *pindexNew, const CellBlockIndex *pindexFork, bool fInitialDownload) override;

private:
    CZMQNotificationInterface();

    void *pcontext;
    std::list<CZMQAbstractNotifier*> notifiers;
};

#endif // CELLLINK_ZMQ_ZMQNOTIFICATIONINTERFACE_H
