// Copyright (c) 2010-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ui/ui_interface.h"
#include "utils/util.h"

CellClientUIInterface uiInterface;

bool InitError(const std::string& str)
{
    uiInterface.ThreadSafeMessageBox(str, "", CellClientUIInterface::MSG_ERROR);
    return false;
}

void InitWarning(const std::string& str)
{
    uiInterface.ThreadSafeMessageBox(str, "", CellClientUIInterface::MSG_WARNING);
}

std::string AmountHighWarn(const std::string& optname)
{
    return strprintf(_("%s is set very high!"), optname);
}

std::string AmountErrMsg(const char* const optname, const std::string& strValue)
{
    return strprintf(_("Invalid amount for -%s=<amount>: '%s'"), optname, strValue);
}
