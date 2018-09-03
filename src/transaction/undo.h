// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_UNDO_H
#define CELLLINK_UNDO_H

#include "transaction/compressor.h" 
#include "consensus/consensus.h"
#include "primitives/transaction.h"
#include "io/serialize.h"

/** Undo information for a CellTxIn
 *
 *  Contains the prevout's CellTxOut being spent, and its metadata as well
 *  (coinbase or not, height). The serialization contains a dummy value of
 *  zero. This is be compatible with older versions which expect to see
 *  the transaction version there.
 */
class TxInUndoSerializer
{
    const Coin* txout;

public:
    template<typename Stream>
    void Serialize(Stream &s) const {
        ::Serialize(s, VARINT(txout->nHeight * 2 + (txout->fCoinBase ? 1 : 0)));
        if (txout->nHeight > 0) {
            // Required to maintain compatibility with older undo format.
            ::Serialize(s, (unsigned char)0);
        }
        ::Serialize(s, CellTxOutCompressor(REF(txout->out)));
    }

    TxInUndoSerializer(const Coin* coin) : txout(coin) {}
};

class TxInUndoDeserializer
{
    Coin* txout;

public:
    template<typename Stream>
    void Unserialize(Stream &s) {
        unsigned int nCode = 0;
        ::Unserialize(s, VARINT(nCode));
        txout->nHeight = nCode / 2;
        txout->fCoinBase = nCode & 1;
        if (txout->nHeight > 0) {
            // Old versions stored the version number for the last spend of
            // a transaction's outputs. Non-final spends were indicated with
            // height = 0.
            int nVersionDummy;
            ::Unserialize(s, VARINT(nVersionDummy));
        }
        ::Unserialize(s, REF(CellTxOutCompressor(REF(txout->out))));
    }

    TxInUndoDeserializer(Coin* coin) : txout(coin) {}
};

static const size_t MIN_TRANSACTION_INPUT_WEIGHT = WITNESS_SCALE_FACTOR * ::GetSerializeSize(CellTxIn(), SER_NETWORK, PROTOCOL_VERSION);
static const size_t MAX_INPUTS_PER_BLOCK = MAX_BLOCK_WEIGHT / MIN_TRANSACTION_INPUT_WEIGHT;

/** Undo information for a CellTransaction */
class CellTxUndo
{
public:
    // undo information for all txins
    std::vector<Coin> vprevout;

    template <typename Stream>
    void Serialize(Stream& s) const {
        // TODO: avoid reimplementing vector serializer
        uint64_t count = vprevout.size();
        ::Serialize(s, COMPACTSIZE(REF(count)));
        for (const auto& prevout : vprevout) {
            ::Serialize(s, REF(TxInUndoSerializer(&prevout)));
        }
    }

    template <typename Stream>
    void Unserialize(Stream& s) {
        // TODO: avoid reimplementing vector deserializer
        uint64_t count = 0;
        ::Unserialize(s, COMPACTSIZE(count));
        if (count > MAX_INPUTS_PER_BLOCK) {
            throw std::ios_base::failure("Too many input undo records");
        }
        vprevout.resize(count);
        for (auto& prevout : vprevout) {
            ::Unserialize(s, REF(TxInUndoDeserializer(&prevout)));
        }
    }
};

/** Undo information for a CellBlock */
class CellBlockUndo
{
public:
    std::vector<CellTxUndo> vtxundo; // for all but the coinbase

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(vtxundo);
    }
};

#endif // CELLLINK_UNDO_H
