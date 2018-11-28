// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "addrdb.h"

#include "addrman.h"
#include "chain/chainparams.h"
#include "misc/clientversion.h"
#include "io/fs.h"
#include "coding/hash.h"
#include "misc/random.h"
#include "io/streams.h"
#include "misc/tinyformat.h"
#include "utils/util.h"

namespace {

template <typename Stream, typename Data>
bool SerializeDB(Stream& stream, const Data& data)
{
    // Write and commit header, data
    try {
        MCHashWriter hasher(SER_DISK, CLIENT_VERSION);
        stream << FLATDATA(Params().MessageStart()) << data;
        hasher << FLATDATA(Params().MessageStart()) << data;
        stream << hasher.GetHash();
    } catch (const std::exception& e) {
        return error("%s: Serialize or I/O error - %s", __func__, e.what());
    }

    return true;
}

template <typename Data>
bool SerializeFileDB(const std::string& prefix, const fs::path& path, const Data& data)
{
    // Generate random temporary filename
    unsigned short randv = 0;
    GetRandBytes((unsigned char*)&randv, sizeof(randv));
    std::string tmpfn = strprintf("%s.%04x", prefix, randv);

    // open temp output file, and associate with MCAutoFile
    fs::path pathTmp = GetDataDir() / tmpfn;
    FILE *file = fsbridge::fopen(pathTmp, "wb");
    MCAutoFile fileout(file, SER_DISK, CLIENT_VERSION);
    if (fileout.IsNull())
        return error("%s: Failed to open file %s", __func__, pathTmp.string());

    // Serialize
    if (!SerializeDB(fileout, data)) return false;
    FileCommit(fileout.Get());
    fileout.fclose();

    // replace existing file, if any, with new file
    if (!RenameOver(pathTmp, path))
        return error("%s: Rename-into-place failed", __func__);

    return true;
}

template <typename Stream, typename Data>
bool DeserializeDB(Stream& stream, Data& data, bool fCheckSum = true)
{
    try {
        CHashVerifier<Stream> verifier(&stream);
        // de-serialize file header (network specific magic number) and ..
        unsigned char pchMsgTmp[4];
        verifier >> FLATDATA(pchMsgTmp);
        // ... verify the network matches ours
        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp)))
            return error("%s: Invalid network magic number", __func__);

        // de-serialize data
        verifier >> data;

        // verify checksum
        if (fCheckSum) {
            uint256 hashTmp;
            stream >> hashTmp;
            if (hashTmp != verifier.GetHash()) {
                return error("%s: Checksum mismatch, data corrupted", __func__);
            }
        }
    }
    catch (const std::exception& e) {
        return error("%s: Deserialize or I/O error - %s", __func__, e.what());
    }

    return true;
}

template <typename Data>
bool DeserializeFileDB(const fs::path& path, Data& data)
{
    // open input file, and associate with MCAutoFile
    FILE *file = fsbridge::fopen(path, "rb");
    MCAutoFile filein(file, SER_DISK, CLIENT_VERSION);
    if (filein.IsNull())
        return error("%s: Failed to open file %s", __func__, path.string());

    return DeserializeDB(filein, data);
}

}

MCBanDB::MCBanDB()
{
    pathBanlist = GetDataDir() / "banlist.dat";
}

bool MCBanDB::Write(const banmap_t& banSet)
{
    return SerializeFileDB("banlist", pathBanlist, banSet);
}

bool MCBanDB::Read(banmap_t& banSet)
{
    return DeserializeFileDB(pathBanlist, banSet);
}

MCAddrDB::MCAddrDB()
{
    pathAddr = GetDataDir() / "peers.dat";
}

bool MCAddrDB::Write(const MCAddrMan& addr)
{
    return SerializeFileDB("peers", pathAddr, addr);
}

bool MCAddrDB::Read(MCAddrMan& addr)
{
    return DeserializeFileDB(pathAddr, addr);
}

bool MCAddrDB::Read(MCAddrMan& addr, MCDataStream& ssPeers)
{
    bool ret = DeserializeDB(ssPeers, addr, false);
    if (!ret) {
        // Ensure addrman is left in a clean state
        addr.Clear();
    }
    return ret;
}
