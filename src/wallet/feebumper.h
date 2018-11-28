// Copyright (c) 2017 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_WALLET_FEEBUMPER_H
#define CELLLINK_WALLET_FEEBUMPER_H

#include "primitives/transaction.h"

class CellWallet;
class CellWalletTx;
class uint256;
class CellCoinControl;
enum class FeeEstimateMode;

enum class BumpFeeResult
{
    OK,
    INVALID_ADDRESS_OR_KEY,
    INVALID_REQUEST,
    INVALID_PARAMETER,
    WALLET_ERROR,
    MISC_ERROR,
};

class CFeeBumper
{
public:
    CFeeBumper(const CellWallet *pWalletIn, const uint256 txidIn, const CellCoinControl& coin_control, CellAmount totalFee);
    BumpFeeResult getResult() const { return currentResult; }
    const std::vector<std::string>& getErrors() const { return vErrors; }
    CellAmount getOldFee() const { return nOldFee; }
    CellAmount getNewFee() const { return nNewFee; }
    uint256 getBumpedTxId() const { return bumpedTxid; }

    /* signs the new transaction,
     * returns false if the tx couldn't be found or if it was
     * impossible to create the signature(s)
     */
    bool signTransaction(CellWallet *pWallet);

    /* commits the fee bump,
     * returns true, in case of CellWallet::CommitTransaction was successful
     * but, eventually sets vErrors if the tx could not be added to the mempool (will try later)
     * or if the old transaction could not be marked as replaced
     */
    bool commit(CellWallet *pWalletNonConst);

private:
    bool preconditionChecks(const CellWallet *pWallet, const CellWalletTx& wtx);

    const uint256 txid;
    uint256 bumpedTxid;
    CellMutableTransaction mtx;
    std::vector<std::string> vErrors;
    BumpFeeResult currentResult;
    CellAmount nOldFee;
    CellAmount nNewFee;
};

#endif // CELLLINK_WALLET_FEEBUMPER_H
