// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The MagnaChain Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAGNACHAIN_KEYSTORE_H
#define MAGNACHAIN_KEYSTORE_H

#include "key/key.h"
#include "key/pubkey.h"
#include "script/script.h"
#include "script/standard.h"
#include "thread/sync.h"

#include <boost/signals2/signal.hpp>

/** A virtual base class for key stores */
class MCKeyStore
{
protected:
    mutable MCCriticalSection cs_KeyStore;

public:
    virtual ~MCKeyStore() {}

    //! Add a key to the store.
    virtual bool AddKeyPubKey(const MCKey &key, const MCPubKey &pubkey) =0;
    virtual bool AddKey(const MCKey &key);

    //! Check whether a key corresponding to a given address is present in the store.
    virtual bool HaveKey(const MCKeyID &address) const =0;
    virtual bool GetKey(const MCKeyID &address, MCKey& keyOut) const =0;
    virtual void GetKeys(std::set<MCKeyID> &setAddress) const =0;
    virtual bool GetPubKey(const MCKeyID &address, MCPubKey& vchPubKeyOut) const =0;

    //! Support for BIP 0013 : see https://github.com/magnachain/bips/blob/master/bip-0013.mediawiki
    virtual bool AddCScript(const MCScript& redeemScript) =0;
    virtual bool HaveCScript(const MCScriptID &hash) const =0;
    virtual bool GetCScript(const MCScriptID &hash, MCScript& redeemScriptOut) const =0;

    //! Support for Watch-only addresses
    virtual bool AddWatchOnly(const MCScript &dest) =0;
    virtual bool RemoveWatchOnly(const MCScript &dest) =0;
    virtual bool HaveWatchOnly(const MCScript &dest) const =0;
    virtual bool HaveWatchOnly() const =0;

public:
	virtual bool GetKeyFromPool(MCPubKey &key, bool internal = false) { return false;  }
	virtual bool SetAddressBook(const MCTxDestination& address, const std::string& strName, const std::string& strPurpose) { return false;  }
};

typedef std::map<MCKeyID, MCKey> KeyMap;
typedef std::map<MCKeyID, MCPubKey> WatchKeyMap;
typedef std::map<MCScriptID, MCScript > ScriptMap;
typedef std::set<MCScript> WatchOnlySet;

/** Basic key store, that keeps keys in an address->secret map */
class MCBasicKeyStore : public MCKeyStore
{
protected:
    KeyMap mapKeys;
    WatchKeyMap mapWatchKeys;
    ScriptMap mapScripts;
    WatchOnlySet setWatchOnly;

public:
    bool AddKeyPubKey(const MCKey& key, const MCPubKey &pubkey) override;
    bool GetPubKey(const MCKeyID &address, MCPubKey& vchPubKeyOut) const override;
    bool HaveKey(const MCKeyID &address) const override
    {
        bool result;
        {
            LOCK(cs_KeyStore);
            result = (mapKeys.count(address) > 0);
        }
        return result;
    }
    void GetKeys(std::set<MCKeyID> &setAddress) const override
    {
        setAddress.clear();
        {
            LOCK(cs_KeyStore);
            KeyMap::const_iterator mi = mapKeys.begin();
            while (mi != mapKeys.end())
            {
                setAddress.insert((*mi).first);
                mi++;
            }
        }
    }
    bool GetKey(const MCKeyID &address, MCKey &keyOut) const override
    {
        {
            LOCK(cs_KeyStore);
            KeyMap::const_iterator mi = mapKeys.find(address);
            if (mi != mapKeys.end())
            {
                keyOut = mi->second;
                return true;
            }
        }
        return false;
    }
    virtual bool AddCScript(const MCScript& redeemScript) override;
    virtual bool HaveCScript(const MCScriptID &hash) const override;
    virtual bool GetCScript(const MCScriptID &hash, MCScript& redeemScriptOut) const override;

    virtual bool AddWatchOnly(const MCScript &dest) override;
    virtual bool RemoveWatchOnly(const MCScript &dest) override;
    virtual bool HaveWatchOnly(const MCScript &dest) const override;
    virtual bool HaveWatchOnly() const override;
};

typedef std::vector<unsigned char, secure_allocator<unsigned char> > MCKeyingMaterial;
typedef std::map<MCKeyID, std::pair<MCPubKey, std::vector<unsigned char> > > CryptedKeyMap;

#endif // MAGNACHAIN_KEYSTORE_H
