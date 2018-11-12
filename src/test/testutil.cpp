// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "test/testutil.h"

#ifdef _WIN32
#include <shlobj.h>
#endif

fs::path GetTempPath() {
    return fs::temp_directory_path();
}
