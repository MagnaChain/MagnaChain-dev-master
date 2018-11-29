// Copyright (c) 2017 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_WALLET_FEEBUMPER_H
#define MAGNACHAIN_WALLET_FEEBUMPER_H

#include "primitives/transaction.h"

class MCWallet;
class MCWalletTx;
class uint256;
class MCCoinControl;
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
    CFeeBumper(const MCWallet *pWalletIn, const uint256 txidIn, const MCCoinControl& coin_control, MCAmount totalFee);
    BumpFeeResult getResult() const { return currentResult; }
    const std::vector<std::string>& getErrors() const { return vErrors; }
    MCAmount getOldFee() const { return nOldFee; }
    MCAmount getNewFee() const { return nNewFee; }
    uint256 getBumpedTxId() const { return bumpedTxid; }

    /* signs the new transaction,
     * returns false if the tx couldn't be found or if it was
     * impossible to create the signature(s)
     */
    bool signTransaction(MCWallet *pWallet);

    /* commits the fee bump,
     * returns true, in case of MCWallet::CommitTransaction was successful
     * but, eventually sets vErrors if the tx could not be added to the mempool (will try later)
     * or if the old transaction could not be marked as replaced
     */
    bool commit(MCWallet *pWalletNonConst);

private:
    bool preconditionChecks(const MCWallet *pWallet, const MCWalletTx& wtx);

    const uint256 txid;
    uint256 bumpedTxid;
    MCMutableTransaction mtx;
    std::vector<std::string> vErrors;
    BumpFeeResult currentResult;
    MCAmount nOldFee;
    MCAmount nNewFee;
};

#endif // MAGNACHAIN_WALLET_FEEBUMPER_H
