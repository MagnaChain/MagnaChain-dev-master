// Copyright (c) 2011-2014 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_QT_WALLETMODELTRANSACTION_H
#define CELLLINK_QT_WALLETMODELTRANSACTION_H

#include "walletmodel.h"

#include <QObject>

class SendCoinsRecipient;

class CellReserveKey;
class CellWallet;
class CellWalletTx;

/** Data model for a walletmodel transaction. */
class WalletModelTransaction
{
public:
    explicit WalletModelTransaction(const QList<SendCoinsRecipient> &recipients);
    ~WalletModelTransaction();

    QList<SendCoinsRecipient> getRecipients();

    CellWalletTx *getTransaction();
    unsigned int getTransactionSize();

    void setTransactionFee(const CellAmount& newFee);
    CellAmount getTransactionFee();

    CellAmount getTotalTransactionAmount();

    void newPossibleKeyChange(CellWallet *wallet);
    CellReserveKey *getPossibleKeyChange();

    void reassignAmounts(int nChangePosRet); // needed for the subtract-fee-from-amount feature

private:
    QList<SendCoinsRecipient> recipients;
    CellWalletTx *walletTransaction;
    CellReserveKey *keyChange;
    CellAmount fee;
};

#endif // CELLLINK_QT_WALLETMODELTRANSACTION_H
