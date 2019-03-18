// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BRANCHCHAIN_RPC_H
#define BRANCHCHAIN_PRC_H

#include "misc/amount.h"
#include "coding/uint256.h"

class MCBlock;
class MCBlockIndex;
class MCTransaction;

MCAmount GetCreateBranchMortgage(const MCBlock* pBlock=nullptr, const MCBlockIndex* pBlockIndex=nullptr);

#endif //  BRANCHCHAIN_PRC_H
