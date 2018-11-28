// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_ADDRDB_H
#define MAGNACHAIN_ADDRDB_H

#include "io/fs.h"
#include "io/serialize.h"

#include <string>
#include <map>

class CSubNet;
class MCAddrMan;
class MCDataStream;

typedef enum BanReason
{
    BanReasonUnknown          = 0,
    BanReasonNodeMisbehaving  = 1,
    BanReasonManuallyAdded    = 2
} BanReason;

class MCBanEntry
{
public:
    static const int CURRENT_VERSION=1;
    int nVersion;
    int64_t nCreateTime;
    int64_t nBanUntil;
    uint8_t banReason;

    MCBanEntry()
    {
        SetNull();
    }

    MCBanEntry(int64_t nCreateTimeIn)
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
        nVersion = MCBanEntry::CURRENT_VERSION;
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

typedef std::map<CSubNet, MCBanEntry> banmap_t;

/** Access to the (IP) address database (peers.dat) */
class MCAddrDB
{
private:
    fs::path pathAddr;
public:
    MCAddrDB();
    bool Write(const MCAddrMan& addr);
    bool Read(MCAddrMan& addr);
    static bool Read(MCAddrMan& addr, MCDataStream& ssPeers);
};

/** Access to the banlist database (banlist.dat) */
class MCBanDB
{
private:
    fs::path pathBanlist;
public:
    MCBanDB();
    bool Write(const banmap_t& banSet);
    bool Read(banmap_t& banSet);
};

#endif // MAGNACHAIN_ADDRDB_H
