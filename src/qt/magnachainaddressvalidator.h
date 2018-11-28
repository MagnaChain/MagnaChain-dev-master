// Copyright (c) 2011-2014 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_QT_MAGNACHAINADDRESSVALIDATOR_H
#define MAGNACHAIN_QT_MAGNACHAINADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class MagnaChainAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit MagnaChainAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** MagnaChain address widget validator, checks for a valid magnachain address.
 */
class MagnaChainAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit MagnaChainAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // MAGNACHAIN_QT_MAGNACHAINADDRESSVALIDATOR_H
