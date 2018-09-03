// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BRANCHCHAIN_RPC_H
#define BRANCHCHAIN_PRC_H

#include "misc/amount.h"

class CellBlock;
class CellBlockIndex;

CellAmount GetCreateBranchMortgage(const CellBlock* pBlock=nullptr, const CellBlockIndex* pBlockIndex=nullptr);

extern const CellAmount CreateBranchChainMortgage;
extern const uint32_t BRANCH_CHAIN_CREATE_COIN_MATURITY;
extern const uint32_t BRANCH_CHAIN_MATURITY;
extern const CellAmount MIN_MINE_BRANCH_MORTGAGE;
extern const uint32_t REDEEM_SAFE_HEIGHT;
extern const uint32_t REPORT_OUTOF_HEIGHT;
extern const uint32_t REPORT_LOCK_COIN_HEIGHT;

#endif //  BRANCHCHAIN_PRC_H
