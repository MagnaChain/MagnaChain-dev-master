// Copyright (c) 2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_QT_TEST_RPC_NESTED_TESTS_H
#define CELLLINK_QT_TEST_RPC_NESTED_TESTS_H

#include <QObject>
#include <QTest>

#include "transaction/txdb.h"
#include "transaction/txmempool.h"

class RPCNestedTests : public QObject
{
    Q_OBJECT

    private Q_SLOTS:
    void rpcNestedTests();

private:
    CellCoinsViewDB *pcoinsdbview;
};

#endif // CELLLINK_QT_TEST_RPC_NESTED_TESTS_H
