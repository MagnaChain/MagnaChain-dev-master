// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
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
std::string FormatMoney(const CellAmount& n);
bool ParseMoney(const std::string& str, CellAmount& nRet);
bool ParseMoney(const char* pszIn, CellAmount& nRet);

#endif // CELLLINK_UTILMONEYSTR_H
