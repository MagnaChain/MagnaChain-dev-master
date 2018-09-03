// Copyright (c) 2011-2014 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_QT_CELLLINKADDRESSVALIDATOR_H
#define CELLLINK_QT_CELLLINKADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class CellLinkAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit CellLinkAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** CellLink address widget validator, checks for a valid celllink address.
 */
class CellLinkAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit CellLinkAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // CELLLINK_QT_CELLLINKADDRESSVALIDATOR_H
