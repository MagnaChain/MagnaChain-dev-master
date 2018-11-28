// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_POLICY_FEERATE_H
#define MAGNACHAIN_POLICY_FEERATE_H

#include "misc/amount.h"
#include "io/serialize.h"

#include <string>

extern const std::string CURRENCY_UNIT;

/**
 * Fee rate in satoshis per kilobyte: MCAmount / kB
 */
class MCFeeRate
{
private:
    MCAmount nSatoshisPerK; // unit is satoshis-per-1,000-bytes
public:
    /** Fee rate of 0 satoshis per kB */
    MCFeeRate() : nSatoshisPerK(0) { }
    explicit MCFeeRate(const MCAmount& _nSatoshisPerK): nSatoshisPerK(_nSatoshisPerK) { }
    /** Constructor for a fee rate in satoshis per kB. The size in bytes must not exceed (2^63 - 1)*/
    MCFeeRate(const MCAmount& nFeePaid, size_t nBytes);
    MCFeeRate(const MCFeeRate& other) { nSatoshisPerK = other.nSatoshisPerK; }
    /**
     * Return the fee in satoshis for the given size in bytes.
     */
    MCAmount GetFee(size_t nBytes) const;
    /**
     * Return the fee in satoshis for a size of 1000 bytes
     */
    MCAmount GetFeePerK() const { return GetFee(1000); }
    friend bool operator<(const MCFeeRate& a, const MCFeeRate& b) { return a.nSatoshisPerK < b.nSatoshisPerK; }
    friend bool operator>(const MCFeeRate& a, const MCFeeRate& b) { return a.nSatoshisPerK > b.nSatoshisPerK; }
    friend bool operator==(const MCFeeRate& a, const MCFeeRate& b) { return a.nSatoshisPerK == b.nSatoshisPerK; }
    friend bool operator<=(const MCFeeRate& a, const MCFeeRate& b) { return a.nSatoshisPerK <= b.nSatoshisPerK; }
    friend bool operator>=(const MCFeeRate& a, const MCFeeRate& b) { return a.nSatoshisPerK >= b.nSatoshisPerK; }
    friend bool operator!=(const MCFeeRate& a, const MCFeeRate& b) { return a.nSatoshisPerK != b.nSatoshisPerK; }
    MCFeeRate& operator+=(const MCFeeRate& a) { nSatoshisPerK += a.nSatoshisPerK; return *this; }
    std::string ToString() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(nSatoshisPerK);
    }
};

#endif //  MAGNACHAIN_POLICY_FEERATE_H
