// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BRANCHCHAIN_RPC_H
#define BRANCHCHAIN_PRC_H

#include "misc/amount.h"
#include "coding/uint256.h"

class CellBlock;
class CellBlockIndex;
class CellTransaction;

CellAmount GetCreateBranchMortgage(const CellBlock* pBlock=nullptr, const CellBlockIndex* pBlockIndex=nullptr);

uint256 GetBranchTxHash(const CellTransaction& tx);

#endif //  BRANCHCHAIN_PRC_H
