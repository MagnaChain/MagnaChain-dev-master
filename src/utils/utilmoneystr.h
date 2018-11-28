// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Money parsing/formatting utilities.
 */
#ifndef CELLLINK_UTILMONEYSTR_H
#define CELLLINK_UTILMONEYSTR_H

#include <stdint.h>
#include <string>

#include "misc/amount.h"

/* Do not use these functions to represent or parse monetary amounts to or from
 * JSON but use AmountFromValue and ValueFromAmount for that.
 */
std::string FormatMoney(const MCAmount& n);
bool ParseMoney(const std::string& str, MCAmount& nRet);
bool ParseMoney(const char* pszIn, MCAmount& nRet);

#endif // CELLLINK_UTILMONEYSTR_H
