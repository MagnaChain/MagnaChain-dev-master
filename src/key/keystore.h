// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_KEYSTORE_H
#define CELLLINK_KEYSTORE_H

#include "key/key.h"
#include "key/pubkey.h"
#include "script/script.h"
#include "script/standard.h"
#include "thread/sync.h"

#include <boost/signals2/signal.hpp>

/** A virtual base class for key stores */
class CellKeyStore
{
protected:
    mutable CellCriticalSection cs_KeyStore;

public:
    virtual ~CellKeyStore() {}

    //! Add a key to the store.
    virtual bool AddKeyPubKey(const CellKey &key, const CellPubKey &pubkey) =0;
    virtual bool AddKey(const CellKey &key);

    //! Check whether a key corresponding to a given address is present in the store.
    virtual bool HaveKey(const CellKeyID &address) const =0;
    virtual bool GetKey(const CellKeyID &address, CellKey& keyOut) const =0;
    virtual void GetKeys(std::set<CellKeyID> &setAddress) const =0;
    virtual bool GetPubKey(const CellKeyID &address, CellPubKey& vchPubKeyOut) const =0;

    //! Support for BIP 0013 : see https://github.com/bitcoin/bips/blob/master/bip-0013.mediawiki
    virtual bool AddCScript(const CellScript& redeemScript) =0;
    virtual bool HaveCScript(const CellScriptID &hash) const =0;
    virtual bool GetCScript(const CellScriptID &hash, CellScript& redeemScriptOut) const =0;

    //! Support for Watch-only addresses
    virtual bool AddWatchOnly(const CellScript &dest) =0;
    virtual bool RemoveWatchOnly(const CellScript &dest) =0;
    virtual bool HaveWatchOnly(const CellScript &dest) const =0;
    virtual bool HaveWatchOnly() const =0;

public:
	virtual bool GetKeyFromPool(CellPubKey &key, bool internal = false) { return false;  }
	virtual bool SetAddressBook(const CellTxDestination& address, const std::string& strName, const std::string& strPurpose) { return false;  }
};

typedef std::map<CellKeyID, CellKey> KeyMap;
typedef std::map<CellKeyID, CellPubKey> WatchKeyMap;
typedef std::map<CellScriptID, CellScript > ScriptMap;
typedef std::set<CellScript> WatchOnlySet;

/** Basic key store, that keeps keys in an address->secret map */
class CellBasicKeyStore : public CellKeyStore
{
protected:
    KeyMap mapKeys;
    WatchKeyMap mapWatchKeys;
    ScriptMap mapScripts;
    WatchOnlySet setWatchOnly;

public:
    bool AddKeyPubKey(const CellKey& key, const CellPubKey &pubkey) override;
    bool GetPubKey(const CellKeyID &address, CellPubKey& vchPubKeyOut) const override;
    bool HaveKey(const CellKeyID &address) const override
    {
        bool result;
        {
            LOCK(cs_KeyStore);
            result = (mapKeys.count(address) > 0);
        }
        return result;
    }
    void GetKeys(std::set<CellKeyID> &setAddress) const override
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
    bool GetKey(const CellKeyID &address, CellKey &keyOut) const override
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
    virtual bool AddCScript(const CellScript& redeemScript) override;
    virtual bool HaveCScript(const CellScriptID &hash) const override;
    virtual bool GetCScript(const CellScriptID &hash, CellScript& redeemScriptOut) const override;

    virtual bool AddWatchOnly(const CellScript &dest) override;
    virtual bool RemoveWatchOnly(const CellScript &dest) override;
    virtual bool HaveWatchOnly(const CellScript &dest) const override;
    virtual bool HaveWatchOnly() const override;
};

typedef std::vector<unsigned char, secure_allocator<unsigned char> > CellKeyingMaterial;
typedef std::map<CellKeyID, std::pair<CellPubKey, std::vector<unsigned char> > > CryptedKeyMap;

#endif // CELLLINK_KEYSTORE_H
