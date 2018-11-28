// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key/keystore.h"

#include "key/key.h"
#include "key/pubkey.h"
#include "utils/util.h"

bool MCKeyStore::AddKey(const MCKey &key) {
    return AddKeyPubKey(key, key.GetPubKey());
}

bool MCBasicKeyStore::GetPubKey(const MCKeyID &address, MCPubKey &vchPubKeyOut) const
{
    MCKey key;
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

bool MCBasicKeyStore::AddKeyPubKey(const MCKey& key, const MCPubKey &pubkey)
{
    LOCK(cs_KeyStore);
    mapKeys[pubkey.GetID()] = key;
    return true;
}

bool MCBasicKeyStore::AddCScript(const MCScript& redeemScript)
{
    if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
        return error("MCBasicKeyStore::AddCScript(): redeemScripts > %i bytes are invalid", MAX_SCRIPT_ELEMENT_SIZE);

    LOCK(cs_KeyStore);
    mapScripts[MCScriptID(redeemScript)] = redeemScript;
    return true;
}

bool MCBasicKeyStore::HaveCScript(const MCScriptID& hash) const
{
    LOCK(cs_KeyStore);
    return mapScripts.count(hash) > 0;
}

bool MCBasicKeyStore::GetCScript(const MCScriptID &hash, MCScript& redeemScriptOut) const
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

static bool ExtractPubKey(const MCScript &dest, MCPubKey& pubKeyOut)
{
    //TODO: Use Solver to extract this?
    MCScript::const_iterator pc = dest.begin();
    opcodetype opcode;
    std::vector<unsigned char> vch;
    if (!dest.GetOp(pc, opcode, vch) || vch.size() < 33 || vch.size() > 65)
        return false;
    pubKeyOut = MCPubKey(vch);
    if (!pubKeyOut.IsFullyValid())
        return false;
    if (!dest.GetOp(pc, opcode, vch) || opcode != OP_CHECKSIG || dest.GetOp(pc, opcode, vch))
        return false;
    return true;
}

bool MCBasicKeyStore::AddWatchOnly(const MCScript &dest)
{
    LOCK(cs_KeyStore);
    setWatchOnly.insert(dest);
    MCPubKey pubKey;
    if (ExtractPubKey(dest, pubKey))
        mapWatchKeys[pubKey.GetID()] = pubKey;
    return true;
}

bool MCBasicKeyStore::RemoveWatchOnly(const MCScript &dest)
{
    LOCK(cs_KeyStore);
    setWatchOnly.erase(dest);
    MCPubKey pubKey;
    if (ExtractPubKey(dest, pubKey))
        mapWatchKeys.erase(pubKey.GetID());
    return true;
}

bool MCBasicKeyStore::HaveWatchOnly(const MCScript &dest) const
{
    LOCK(cs_KeyStore);
    return setWatchOnly.count(dest) > 0;
}

bool MCBasicKeyStore::HaveWatchOnly() const
{
    LOCK(cs_KeyStore);
    return (!setWatchOnly.empty());
}
