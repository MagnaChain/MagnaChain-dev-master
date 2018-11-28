// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_ADDRDB_H
#define CELLLINK_ADDRDB_H

#include "io/fs.h"
#include "io/serialize.h"

#include <string>
#include <map>

class CSubNet;
class CellAddrMan;
class CellDataStream;

typedef enum BanReason
{
    BanReasonUnknown          = 0,
    BanReasonNodeMisbehaving  = 1,
    BanReasonManuallyAdded    = 2
} BanReason;

class CellBanEntry
{
public:
    static const int CURRENT_VERSION=1;
    int nVersion;
    int64_t nCreateTime;
    int64_t nBanUntil;
    uint8_t banReason;

    CellBanEntry()
    {
        SetNull();
    }

    CellBanEntry(int64_t nCreateTimeIn)
    {
        SetNull();
        nCreateTime = nCreateTimeIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(this->nVersion);
        READWRITE(nCreateTime);
        READWRITE(nBanUntil);
        READWRITE(banReason);
    }

    void SetNull()
    {
        nVersion = CellBanEntry::CURRENT_VERSION;
        nCreateTime = 0;
        nBanUntil = 0;
        banReason = BanReasonUnknown;
    }

    std::string banReasonToString()
    {
        switch (banReason) {
        case BanReasonNodeMisbehaving:
            return "node misbehaving";
        case BanReasonManuallyAdded:
            return "manually added";
        default:
            return "unknown";
        }
    }
};

typedef std::map<CSubNet, CellBanEntry> banmap_t;

/** Access to the (IP) address database (peers.dat) */
class CellAddrDB
{
private:
    fs::path pathAddr;
public:
    CellAddrDB();
    bool Write(const CellAddrMan& addr);
    bool Read(CellAddrMan& addr);
    static bool Read(CellAddrMan& addr, CellDataStream& ssPeers);
};

/** Access to the banlist database (banlist.dat) */
class CellBanDB
{
private:
    fs::path pathBanlist;
public:
    CellBanDB();
    bool Write(const banmap_t& banSet);
    bool Read(banmap_t& banSet);
};

#endif // CELLLINK_ADDRDB_H
