// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key/keystore.h"

#include "key/key.h"
#include "key/pubkey.h"
#include "utils/util.h"

bool CellKeyStore::AddKey(const CellKey &key) {
    return AddKeyPubKey(key, key.GetPubKey());
}

bool CellBasicKeyStore::GetPubKey(const CellKeyID &address, CellPubKey &vchPubKeyOut) const
{
    CellKey key;
    if (!GetKey(address, key)) {
        LOCK(cs_KeyStore);
        WatchKeyMap::const_iterator it = mapWatchKeys.find(address);
        if (it != mapWatchKeys.end()) {
            vchPubKeyOut = it->second;
            return true;
        }
        return false;
    }
    vchPubKeyOut = key.GetPubKey();
    return true;
}

bool CellBasicKeyStore::AddKeyPubKey(const CellKey& key, const CellPubKey &pubkey)
{
    LOCK(cs_KeyStore);
    mapKeys[pubkey.GetID()] = key;
    return true;
}

bool CellBasicKeyStore::AddCScript(const CellScript& redeemScript)
{
    if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
        return error("CellBasicKeyStore::AddCScript(): redeemScripts > %i bytes are invalid", MAX_SCRIPT_ELEMENT_SIZE);

    LOCK(cs_KeyStore);
    mapScripts[CellScriptID(redeemScript)] = redeemScript;
    return true;
}

bool CellBasicKeyStore::HaveCScript(const CellScriptID& hash) const
{
    LOCK(cs_KeyStore);
    return mapScripts.count(hash) > 0;
}

bool CellBasicKeyStore::GetCScript(const CellScriptID &hash, CellScript& redeemScriptOut) const
{
    LOCK(cs_KeyStore);
    ScriptMap::const_iterator mi = mapScripts.find(hash);
    if (mi != mapScripts.end())
    {
        redeemScriptOut = (*mi).second;
        return true;
    }
    return false;
}

static bool ExtractPubKey(const CellScript &dest, CellPubKey& pubKeyOut)
{
    //TODO: Use Solver to extract this?
    CellScript::const_iterator pc = dest.begin();
    opcodetype opcode;
    std::vector<unsigned char> vch;
    if (!dest.GetOp(pc, opcode, vch) || vch.size() < 33 || vch.size() > 65)
        return false;
    pubKeyOut = CellPubKey(vch);
    if (!pubKeyOut.IsFullyValid())
        return false;
    if (!dest.GetOp(pc, opcode, vch) || opcode != OP_CHECKSIG || dest.GetOp(pc, opcode, vch))
        return false;
    return true;
}

bool CellBasicKeyStore::AddWatchOnly(const CellScript &dest)
{
    LOCK(cs_KeyStore);
    setWatchOnly.insert(dest);
    CellPubKey pubKey;
    if (ExtractPubKey(dest, pubKey))
        mapWatchKeys[pubKey.GetID()] = pubKey;
    return true;
}

bool CellBasicKeyStore::RemoveWatchOnly(const CellScript &dest)
{
    LOCK(cs_KeyStore);
    setWatchOnly.erase(dest);
    CellPubKey pubKey;
    if (ExtractPubKey(dest, pubKey))
        mapWatchKeys.erase(pubKey.GetID());
    return true;
}

bool CellBasicKeyStore::HaveWatchOnly(const CellScript &dest) const
{
    LOCK(cs_KeyStore);
    return setWatchOnly.count(dest) > 0;
}

bool CellBasicKeyStore::HaveWatchOnly() const
{
    LOCK(cs_KeyStore);
    return (!setWatchOnly.empty());
}
