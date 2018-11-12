// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CELLLINK_WALLET_CRYPTER_H
#define CELLLINK_WALLET_CRYPTER_H

#include "key/keystore.h"
#include "io/serialize.h"
#include "support/allocators/secure.h"

const unsigned int WALLET_CRYPTO_KEY_SIZE = 32;
const unsigned int WALLET_CRYPTO_SALT_SIZE = 8;
const unsigned int WALLET_CRYPTO_IV_SIZE = 16;

/**
 * Private key encryption is done based on a CellMasterKey,
 * which holds a salt and random encryption key.
 * 
 * CMasterKeys are encrypted using AES-256-CBC using a key
 * derived using derivation method nDerivationMethod
 * (0 == EVP_sha512()) and derivation iterations nDeriveIterations.
 * vchOtherDerivationParameters is provided for alternative algorithms
 * which may require more parameters (such as scrypt).
 * 
 * Wallet Private Keys are then encrypted using AES-256-CBC
 * with the double-sha256 of the public key as the IV, and the
 * master key's key as the encryption key (see keystore.[ch]).
 */

/** Master key for wallet encryption */
class CellMasterKey
{
public:
    std::vector<unsigned char> vchCryptedKey;
    std::vector<unsigned char> vchSalt;
    //! 0 = EVP_sha512()
    //! 1 = scrypt()
    unsigned int nDerivationMethod;
    unsigned int nDeriveIterations;
    //! Use this for more parameters to key derivation,
    //! such as the various parameters to scrypt
    std::vector<unsigned char> vchOtherDerivationParameters;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(vchCryptedKey);
        READWRITE(vchSalt);
        READWRITE(nDerivationMethod);
        READWRITE(nDeriveIterations);
        READWRITE(vchOtherDerivationParameters);
    }

    CellMasterKey()
    {
        // 25000 rounds is just under 0.1 seconds on a 1.86 GHz Pentium M
        // ie slightly lower than the lowest hardware we need bother supporting
        nDeriveIterations = 25000;
        nDerivationMethod = 0;
        vchOtherDerivationParameters = std::vector<unsigned char>(0);
    }
};

typedef std::vector<unsigned char, secure_allocator<unsigned char> > CellKeyingMaterial;

namespace wallet_crypto
{
    class TestCrypter;
}

/** Encryption/decryption context with key information */
class CellCrypter
{
friend class wallet_crypto::TestCrypter; // for test access to chKey/chIV
private:
    std::vector<unsigned char, secure_allocator<unsigned char>> vchKey;
    std::vector<unsigned char, secure_allocator<unsigned char>> vchIV;
    bool fKeySet;

    int BytesToKeySHA512AES(const std::vector<unsigned char>& chSalt, const SecureString& strKeyData, int count, unsigned char *key,unsigned char *iv) const;

public:
    bool SetKeyFromPassphrase(const SecureString &strKeyData, const std::vector<unsigned char>& chSalt, const unsigned int nRounds, const unsigned int nDerivationMethod);
    bool Encrypt(const CellKeyingMaterial& vchPlaintext, std::vector<unsigned char> &vchCiphertext) const;
    bool Decrypt(const std::vector<unsigned char>& vchCiphertext, CellKeyingMaterial& vchPlaintext) const;
    bool SetKey(const CellKeyingMaterial& chNewKey, const std::vector<unsigned char>& chNewIV);

    void CleanKey()
    {
        memory_cleanse(vchKey.data(), vchKey.size());
        memory_cleanse(vchIV.data(), vchIV.size());
        fKeySet = false;
    }

    CellCrypter()
    {
        fKeySet = false;
        vchKey.resize(WALLET_CRYPTO_KEY_SIZE);
        vchIV.resize(WALLET_CRYPTO_IV_SIZE);
    }

    ~CellCrypter()
    {
        CleanKey();
    }
};

/** Keystore which keeps the private keys encrypted.
 * It derives from the basic key store, which is used if no encryption is active.
 */
class CellCryptoKeyStore : public CellBasicKeyStore
{
private:
    CryptedKeyMap mapCryptedKeys;

    CellKeyingMaterial vMasterKey;

    //! if fUseCrypto is true, mapKeys must be empty
    //! if fUseCrypto is false, vMasterKey must be empty
    bool fUseCrypto;

    //! keeps track of whether Unlock has run a thorough check before
    bool fDecryptionThoroughlyChecked;

protected:
    bool SetCrypted();

    //! will encrypt previously unencrypted keys
    bool EncryptKeys(CellKeyingMaterial& vMasterKeyIn);

    bool Unlock(const CellKeyingMaterial& vMasterKeyIn);

public:
    CellCryptoKeyStore() : fUseCrypto(false), fDecryptionThoroughlyChecked(false)
    {
    }

    bool IsCrypted() const
    {
        return fUseCrypto;
    }

    bool IsLocked() const
    {
        if (!IsCrypted())
            return false;
        bool result;
        {
            LOCK(cs_KeyStore);
            result = vMasterKey.empty();
        }
        return result;
    }

    bool Lock();

    virtual bool AddCryptedKey(const CellPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret);
    bool AddKeyPubKey(const CellKey& key, const CellPubKey &pubkey) override;
    bool HaveKey(const CellKeyID &address) const override
    {
        {
            LOCK(cs_KeyStore);
            if (!IsCrypted())
                return CellBasicKeyStore::HaveKey(address);
            return mapCryptedKeys.count(address) > 0;
        }
        return false;
    }
    bool GetKey(const CellKeyID &address, CellKey& keyOut) const override;
    bool GetPubKey(const CellKeyID &address, CellPubKey& vchPubKeyOut) const override;
    void GetKeys(std::set<CellKeyID> &setAddress) const override
    {
        if (!IsCrypted())
        {
            CellBasicKeyStore::GetKeys(setAddress);
            return;
        }
        setAddress.clear();
        CryptedKeyMap::const_iterator mi = mapCryptedKeys.begin();
        while (mi != mapCryptedKeys.end())
        {
            setAddress.insert((*mi).first);
            mi++;
        }
    }

    /**
     * Wallet status (encrypted, locked) changed.
     * Note: Called without locks held.
     */
    boost::signals2::signal<void (CellCryptoKeyStore* wallet)> NotifyStatusChanged;
};

#endif // CELLLINK_WALLET_CRYPTER_H
