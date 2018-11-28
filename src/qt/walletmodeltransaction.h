// Copyright (c) 2011-2014 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_QT_WALLETMODELTRANSACTION_H
#define CELLLINK_QT_WALLETMODELTRANSACTION_H

#include "walletmodel.h"

#include <QObject>

class SendCoinsRecipient;

class MCReserveKey;
class MCWallet;
class MCWalletTx;

/** Data model for a walletmodel transaction. */
class WalletModelTransaction
{
public:
    explicit WalletModelTransaction(const QList<SendCoinsRecipient> &recipients);
    ~WalletModelTransaction();

    QList<SendCoinsRecipient> getRecipients();

    MCWalletTx *getTransaction();
    unsigned int getTransactionSize();

    void setTransactionFee(const MCAmount& newFee);
    MCAmount getTransactionFee();

    MCAmount getTotalTransactionAmount();

    void newPossibleKeyChange(MCWallet *wallet);
    MCReserveKey *getPossibleKeyChange();

    void reassignAmounts(int nChangePosRet); // needed for the subtract-fee-from-amount feature

private:
    QList<SendCoinsRecipient> recipients;
    MCWalletTx *walletTransaction;
    MCReserveKey *keyChange;
    MCAmount fee;
};

#endif // CELLLINK_QT_WALLETMODELTRANSACTION_H
