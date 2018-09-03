// Copyright (c) 2017-2017 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_CONSENSUS_TX_VERIFY_H
#define CELLLINK_CONSENSUS_TX_VERIFY_H

#include <stdint.h>
#include <vector>
#include "misc/amount.h"

class CellBlockIndex;
class CellCoinsViewCache;
class CellTransaction;
class CellValidationState;
class CellKeyStore;
class CellMutableTransaction;
class CellScript;
class CellBlock;
class BranchCache;

/** Transaction validation functions */

/** Context-independent validity checks */
bool CheckTransaction(const CellTransaction& tx, CellValidationState& state, bool fCheckDuplicateInputs=true, const CellBlock* pBlock=nullptr, 
    const CellBlockIndex* pBlockIndex=nullptr, const bool fVerifingDB=false, BranchCache *pBranchCache=nullptr);
bool CheckCoinbaseSignature( int nHeight, const CellTransaction& tx);
bool SignatureCoinbaseTransaction( int nHeight, const CellKeyStore* keystoreIn, CellMutableTransaction& tx, CellAmount nValue, const CellScript& scriptPubKey);


namespace Consensus {
/**
 * Check whether all inputs of this transaction are valid (no double spends and amounts)
 * This does not modify the UTXO set. This does not check scripts and sigs.
 * Preconditions: tx.IsCoinBase() is false.
 */
bool CheckTxInputs(const CellTransaction& tx, CellValidationState& state, const CellCoinsViewCache& inputs, int nSpendHeight);
} // namespace Consensus

/** Auxiliary functions for transaction validation (ideally should not be exposed) */

/**
 * Count ECDSA signature operations the old-fashioned (pre-0.6) way
 * @return number of sigops this transaction's outputs will produce when spent
 * @see CellTransaction::FetchInputs
 */
unsigned int GetLegacySigOpCount(const CellTransaction& tx);

/**
 * Count ECDSA signature operations in pay-to-script-hash inputs.
 * 
 * @param[in] mapInputs Map of previous transactions that have outputs we're spending
 * @return maximum number of sigops required to validate this transaction's inputs
 * @see CellTransaction::FetchInputs
 */
unsigned int GetP2SHSigOpCount(const CellTransaction& tx, const CellCoinsViewCache& mapInputs);

/**
 * Compute total signature operation cost of a transaction.
 * @param[in] tx     Transaction for which we are computing the cost
 * @param[in] inputs Map of previous transactions that have outputs we're spending
 * @param[out] flags Script verification flags
 * @return Total signature operation cost of tx
 */
int64_t GetTransactionSigOpCost(const CellTransaction& tx, const CellCoinsViewCache& inputs, int flags);

/**
 * Check if transaction is final and can be included in a block with the
 * specified height and time. Consensus critical.
 */
bool IsFinalTx(const CellTransaction &tx, int nBlockHeight, int64_t nBlockTime);

/**
 * Calculates the block height and previous block's median time past at
 * which the transaction will be considered final in the context of BIP 68.
 * Also removes from the vector of input heights any entries which did not
 * correspond to sequence locked inputs as they do not affect the calculation.
 */
std::pair<int, int64_t> CalculateSequenceLocks(const CellTransaction &tx, int flags, std::vector<int>* prevHeights, const CellBlockIndex& block);

bool EvaluateSequenceLocks(const CellBlockIndex& block, std::pair<int, int64_t> lockPair);
/**
 * Check if transaction is final per BIP 68 sequence numbers and can be included in a block.
 * Consensus critical. Takes as input a list of heights at which tx's inputs (in order) confirmed.
 */
bool SequenceLocks(const CellTransaction &tx, int flags, std::vector<int>* prevHeights, const CellBlockIndex& block);

#endif // CELLLINK_CONSENSUS_TX_VERIFY_H
