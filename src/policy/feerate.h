// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_POLICY_FEERATE_H
#define CELLLINK_POLICY_FEERATE_H

#include "misc/amount.h"
#include "io/serialize.h"

#include <string>

extern const std::string CURRENCY_UNIT;

/**
 * Fee rate in satoshis per kilobyte: CellAmount / kB
 */
class CellFeeRate
{
private:
    CellAmount nSatoshisPerK; // unit is satoshis-per-1,000-bytes
public:
    /** Fee rate of 0 satoshis per kB */
    CellFeeRate() : nSatoshisPerK(0) { }
    explicit CellFeeRate(const CellAmount& _nSatoshisPerK): nSatoshisPerK(_nSatoshisPerK) { }
    /** Constructor for a fee rate in satoshis per kB. The size in bytes must not exceed (2^63 - 1)*/
    CellFeeRate(const CellAmount& nFeePaid, size_t nBytes);
    CellFeeRate(const CellFeeRate& other) { nSatoshisPerK = other.nSatoshisPerK; }
    /**
     * Return the fee in satoshis for the given size in bytes.
     */
    CellAmount GetFee(size_t nBytes) const;
    /**
     * Return the fee in satoshis for a size of 1000 bytes
     */
    CellAmount GetFeePerK() const { return GetFee(1000); }
    friend bool operator<(const CellFeeRate& a, const CellFeeRate& b) { return a.nSatoshisPerK < b.nSatoshisPerK; }
    friend bool operator>(const CellFeeRate& a, const CellFeeRate& b) { return a.nSatoshisPerK > b.nSatoshisPerK; }
    friend bool operator==(const CellFeeRate& a, const CellFeeRate& b) { return a.nSatoshisPerK == b.nSatoshisPerK; }
    friend bool operator<=(const CellFeeRate& a, const CellFeeRate& b) { return a.nSatoshisPerK <= b.nSatoshisPerK; }
    friend bool operator>=(const CellFeeRate& a, const CellFeeRate& b) { return a.nSatoshisPerK >= b.nSatoshisPerK; }
    friend bool operator!=(const CellFeeRate& a, const CellFeeRate& b) { return a.nSatoshisPerK != b.nSatoshisPerK; }
    CellFeeRate& operator+=(const CellFeeRate& a) { nSatoshisPerK += a.nSatoshisPerK; return *this; }
    std::string ToString() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(nSatoshisPerK);
    }
};

#endif //  CELLLINK_POLICY_FEERATE_H
