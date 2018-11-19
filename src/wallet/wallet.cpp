// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet/wallet.h"

#include "coding/base58.h"
#include "validation/checkpoints.h"
#include "chain/chain.h"
#include "wallet/coincontrol.h"
#include "consensus/consensus.h"
#include "consensus/validation.h"
#include "io/fs.h"
#include "init.h"
#include "key/key.h"
#include "key/keystore.h"
#include "validation/validation.h"
#include "net/net.h"
#include "policy/fees.h"
#include "policy/policy.h"
#include "policy/rbf.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "script/sign.h"
#include "thread/scheduler.h"
#include "misc/timedata.h"
#include "transaction/txmempool.h"
#include "utils/util.h"
#include "ui/ui_interface.h"
#include "utils/utilmoneystr.h"
#include "smartcontract/smartcontract.h"
#include "rpc/branchchainrpc.h"

#include <assert.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/thread.hpp>
#include <boost/variant/apply_visitor.hpp>

std::vector<CWalletRef> vpwallets;
/** Transaction fee set by the user */
CellFeeRate payTxFee(DEFAULT_TRANSACTION_FEE);
unsigned int nTxConfirmTarget = DEFAULT_TX_CONFIRM_TARGET;
bool bSpendZeroConfChange = DEFAULT_SPEND_ZEROCONF_CHANGE;
bool fWalletRbf = DEFAULT_WALLET_RBF;

const char * DEFAULT_WALLET_DAT = "wallet.dat";
const uint32_t BIP32_HARDENED_KEY_LIMIT = 0x80000000;

#include "chain/branchchain.h"

/**
 * Fees smaller than this (in satoshi) are considered zero fee (for transaction creation)
 * Override with -mintxfee
 */
CellFeeRate CellWallet::minTxFee = CellFeeRate(DEFAULT_TRANSACTION_MINFEE);
/**
 * If fee estimation does not have enough data to provide estimates, use this fee instead.
 * Has no effect if not using fee estimation
 * Override with -fallbackfee
 */
CellFeeRate CellWallet::fallbackFee = CellFeeRate(DEFAULT_FALLBACK_FEE);

CellFeeRate CellWallet::m_discard_rate = CellFeeRate(DEFAULT_DISCARD_FEE);

const uint256 CellMerkleTx::ABANDON_HASH(uint256S("0000000000000000000000000000000000000000000000000000000000000001"));

/** @defgroup mapWallet
 *
 * @{
 */

struct CompareValueOnly
{
    bool operator()(const CellInputCoin& t1,
                    const CellInputCoin& t2) const
    {
        return t1.txout.nValue < t2.txout.nValue;
    }
};

std::string CellOutput::ToString() const
{
    return strprintf("CellOutput(%s, %d, %d) [%s]", tx->GetHash().ToString(), i, nDepth, FormatMoney(tx->tx->vout[i].nValue));
}

class CellAffectedKeysVisitor : public boost::static_visitor<void> {
private:
    const CellKeyStore &keystore;
    std::vector<CellKeyID> &vKeys;

public:
    CellAffectedKeysVisitor(const CellKeyStore &keystoreIn, std::vector<CellKeyID> &vKeysIn) : keystore(keystoreIn), vKeys(vKeysIn) {}

    void Process(const CellScript &script) {
        txnouttype type;
        std::vector<CellTxDestination> vDest;
        int nRequired;
        if (ExtractDestinations(script, type, vDest, nRequired)) {
            for (const CellTxDestination &dest : vDest)
                boost::apply_visitor(*this, dest);
        }
    }

    void operator()(const CellContractID &constractId) {
        // TODO: fill logic
    }

    void operator()(const CellKeyID &keyId) {
        if (keystore.HaveKey(keyId))
            vKeys.push_back(keyId);
    }

    void operator()(const CellScriptID &scriptId) {
        CellScript script;
        if (keystore.GetCScript(scriptId, script))
            Process(script);
    }

    void operator()(const CellNoDestination &none) {}
};

const CellWalletTx* CellWallet::GetWalletTx(const uint256& hash) const
{
    LOCK(cs_wallet);
    std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.find(hash);
    if (it == mapWallet.end())
        return nullptr;
    return &(it->second);
}

CellPubKey CellWallet::GenerateNewKey(CWalletDB &walletdb, bool internal)
{
    AssertLockHeld(cs_wallet); // mapKeyMetadata
    bool fCompressed = CanSupportFeature(FEATURE_COMPRPUBKEY); // default to compressed public keys if we want 0.6.0 wallets

    CellKey secret;

    // Create new metadata
    int64_t nCreationTime = GetTime();
    CKeyMetadata metadata(nCreationTime);

    // use HD key derivation if HD was enabled during wallet creation
    if (IsHDEnabled()) {
        DeriveNewChildKey(walletdb, metadata, secret, (CanSupportFeature(FEATURE_HD_SPLIT) ? internal : false));
    } else {
        secret.MakeNewKey(fCompressed);
    }

    // Compressed public keys were introduced in version 0.6.0
    if (fCompressed) {
        SetMinVersion(FEATURE_COMPRPUBKEY);
    }

    CellPubKey pubkey = secret.GetPubKey();
    assert(secret.VerifyPubKey(pubkey));

    mapKeyMetadata[pubkey.GetID()] = metadata;
    UpdateTimeFirstKey(nCreationTime);

    if (!AddKeyPubKeyWithDB(walletdb, secret, pubkey)) {
        throw std::runtime_error(std::string(__func__) + ": AddKey failed");
    }
    return pubkey;
}

void CellWallet::DeriveNewChildKey(CWalletDB &walletdb, CKeyMetadata& metadata, CellKey& secret, bool internal)
{
    // for now we use a fixed keypath scheme of m/0'/0'/k
    CellKey key;                      //master key seed (256bit)
    CellExtKey masterKey;             //hd master key
    CellExtKey accountKey;            //key at m/0'
    CellExtKey chainChildKey;         //key at m/0'/0' (external) or m/0'/1' (internal)
    CellExtKey childKey;              //key at m/0'/0'/<n>'

    // try to get the master key
    if (!GetKey(hdChain.masterKeyID, key))
        throw std::runtime_error(std::string(__func__) + ": Master key not found");

    masterKey.SetMaster(key.begin(), key.size());

    // derive m/0'
    // use hardened derivation (child keys >= 0x80000000 are hardened after bip32)
    masterKey.Derive(accountKey, BIP32_HARDENED_KEY_LIMIT);

    // derive m/0'/0' (external chain) OR m/0'/1' (internal chain)
    assert(internal ? CanSupportFeature(FEATURE_HD_SPLIT) : true);
    accountKey.Derive(chainChildKey, BIP32_HARDENED_KEY_LIMIT+(internal ? 1 : 0));

    // derive child key at next index, skip keys already known to the wallet
    do {
        // always derive hardened keys
        // childIndex | BIP32_HARDENED_KEY_LIMIT = derive childIndex in hardened child-index-range
        // example: 1 | BIP32_HARDENED_KEY_LIMIT == 0x80000001 == 2147483649
        if (internal) {
            chainChildKey.Derive(childKey, hdChain.nInternalChainCounter | BIP32_HARDENED_KEY_LIMIT);
            metadata.hdKeypath = "m/0'/1'/" + std::to_string(hdChain.nInternalChainCounter) + "'";
            hdChain.nInternalChainCounter++;
        }
        else {
            chainChildKey.Derive(childKey, hdChain.nExternalChainCounter | BIP32_HARDENED_KEY_LIMIT);
            metadata.hdKeypath = "m/0'/0'/" + std::to_string(hdChain.nExternalChainCounter) + "'";
            hdChain.nExternalChainCounter++;
        }
    } while (HaveKey(childKey.key.GetPubKey().GetID()));
    secret = childKey.key;
    metadata.hdMasterKeyID = hdChain.masterKeyID;
    // update the chain model in the database
    if (!walletdb.WriteHDChain(hdChain))
        throw std::runtime_error(std::string(__func__) + ": Writing HD chain model failed");
}

bool CellWallet::AddKeyPubKeyWithDB(CWalletDB &walletdb, const CellKey& secret, const CellPubKey &pubkey)
{
    AssertLockHeld(cs_wallet); // mapKeyMetadata

    // CellCryptoKeyStore has no concept of wallet databases, but calls AddCryptedKey
    // which is overridden below.  To avoid flushes, the database handle is
    // tunneled through to it.
    bool needsDB = !pwalletdbEncryption;
    if (needsDB) {
        pwalletdbEncryption = &walletdb;
    }
    if (!CellCryptoKeyStore::AddKeyPubKey(secret, pubkey)) {
        if (needsDB) pwalletdbEncryption = nullptr;
        return false;
    }
    if (needsDB) pwalletdbEncryption = nullptr;

    // check if we need to remove from watch-only
    CellScript script;
    script = GetScriptForDestination(pubkey.GetID());
    if (HaveWatchOnly(script)) {
        RemoveWatchOnly(script);
    }
    script = GetScriptForRawPubKey(pubkey);
    if (HaveWatchOnly(script)) {
        RemoveWatchOnly(script);
    }

    if (!IsCrypted()) {
        return walletdb.WriteKey(pubkey, secret.GetPrivKey(), mapKeyMetadata[pubkey.GetID()]);
    }
    return true;
}

bool CellWallet::AddKeyPubKey(const CellKey& secret, const CellPubKey &pubkey)
{
    CWalletDB walletdb(*dbw);
    return CellWallet::AddKeyPubKeyWithDB(walletdb, secret, pubkey);
}

bool CellWallet::AddCryptedKey(const CellPubKey &vchPubKey,
                            const std::vector<unsigned char> &vchCryptedSecret)
{
    if (!CellCryptoKeyStore::AddCryptedKey(vchPubKey, vchCryptedSecret))
        return false;
    {
        LOCK(cs_wallet);
        if (pwalletdbEncryption)
            return pwalletdbEncryption->WriteCryptedKey(vchPubKey,
                                                        vchCryptedSecret,
                                                        mapKeyMetadata[vchPubKey.GetID()]);
        else
            return CWalletDB(*dbw).WriteCryptedKey(vchPubKey,
                                                            vchCryptedSecret,
                                                            mapKeyMetadata[vchPubKey.GetID()]);
    }
}

bool CellWallet::LoadKeyMetadata(const CellTxDestination& keyID, const CKeyMetadata &meta)
{
    AssertLockHeld(cs_wallet); // mapKeyMetadata
    UpdateTimeFirstKey(meta.nCreateTime);
    mapKeyMetadata[keyID] = meta;
    return true;
}

bool CellWallet::LoadCryptedKey(const CellPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret)
{
    return CellCryptoKeyStore::AddCryptedKey(vchPubKey, vchCryptedSecret);
}

/**
 * Update wallet first key creation time. This should be called whenever keys
 * are added to the wallet, with the oldest key creation time.
 */
void CellWallet::UpdateTimeFirstKey(int64_t nCreateTime)
{
    AssertLockHeld(cs_wallet);
    if (nCreateTime <= 1) {
        // Cannot determine birthday information, so set the wallet birthday to
        // the beginning of time.
        nTimeFirstKey = 1;
    } else if (!nTimeFirstKey || nCreateTime < nTimeFirstKey) {
        nTimeFirstKey = nCreateTime;
    }
}

bool CellWallet::AddCScript(const CellScript& redeemScript)
{
    if (!CellCryptoKeyStore::AddCScript(redeemScript))
        return false;
    return CWalletDB(*dbw).WriteCScript(Hash160(redeemScript), redeemScript);
}

bool CellWallet::LoadCScript(const CellScript& redeemScript)
{
    /* A sanity check was added in pull #3843 to avoid adding redeemScripts
     * that never can be redeemed. However, old wallets may still contain
     * these. Do not add them to the wallet and warn. */
    if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
    {
        std::string strAddr = CellLinkAddress(CellScriptID(redeemScript)).ToString();
        LogPrintf("%s: Warning: This wallet contains a redeemScript of size %i which exceeds maximum size %i thus can never be redeemed. Do not use address %s.\n",
            __func__, redeemScript.size(), MAX_SCRIPT_ELEMENT_SIZE, strAddr);
        return true;
    }

    return CellCryptoKeyStore::AddCScript(redeemScript);
}

bool CellWallet::AddWatchOnly(const CellScript& dest)
{
    if (!CellCryptoKeyStore::AddWatchOnly(dest))
        return false;
    const CKeyMetadata& meta = mapKeyMetadata[CellScriptID(dest)];
    UpdateTimeFirstKey(meta.nCreateTime);
    NotifyWatchonlyChanged(true);
    return CWalletDB(*dbw).WriteWatchOnly(dest, meta);
}

bool CellWallet::AddWatchOnly(const CellScript& dest, int64_t nCreateTime)
{
    mapKeyMetadata[CellScriptID(dest)].nCreateTime = nCreateTime;
    return AddWatchOnly(dest);
}

bool CellWallet::RemoveWatchOnly(const CellScript &dest)
{
    AssertLockHeld(cs_wallet);
    if (!CellCryptoKeyStore::RemoveWatchOnly(dest))
        return false;
    if (!HaveWatchOnly())
        NotifyWatchonlyChanged(false);
    if (!CWalletDB(*dbw).EraseWatchOnly(dest))
        return false;

    return true;
}

bool CellWallet::LoadWatchOnly(const CellScript &dest)
{
    return CellCryptoKeyStore::AddWatchOnly(dest);
}

bool CellWallet::Unlock(const SecureString& strWalletPassphrase)
{
    CellCrypter crypter;
    CellKeyingMaterial _vMasterKey;

    {
        LOCK(cs_wallet);
        for (const MasterKeyMap::value_type& pMasterKey : mapMasterKeys)
        {
            if(!crypter.SetKeyFromPassphrase(strWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
                return false;
            if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, _vMasterKey))
                continue; // try another master key
            if (CellCryptoKeyStore::Unlock(_vMasterKey))
                return true;
        }
    }
    return false;
}

bool CellWallet::ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase, const SecureString& strNewWalletPassphrase)
{
    bool fWasLocked = IsLocked();

    {
        LOCK(cs_wallet);
        Lock();

        CellCrypter crypter;
        CellKeyingMaterial _vMasterKey;
        for (MasterKeyMap::value_type& pMasterKey : mapMasterKeys)
        {
            if(!crypter.SetKeyFromPassphrase(strOldWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
                return false;
            if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, _vMasterKey))
                return false;
            if (CellCryptoKeyStore::Unlock(_vMasterKey))
            {
                int64_t nStartTime = GetTimeMillis();
                crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
                pMasterKey.second.nDeriveIterations = pMasterKey.second.nDeriveIterations * (100 / ((double)(GetTimeMillis() - nStartTime)));

                nStartTime = GetTimeMillis();
                crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
                pMasterKey.second.nDeriveIterations = (pMasterKey.second.nDeriveIterations + pMasterKey.second.nDeriveIterations * 100 / ((double)(GetTimeMillis() - nStartTime))) / 2;

                if (pMasterKey.second.nDeriveIterations < 25000)
                    pMasterKey.second.nDeriveIterations = 25000;

                LogPrintf("Wallet passphrase changed to an nDeriveIterations of %i\n", pMasterKey.second.nDeriveIterations);

                if (!crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
                    return false;
                if (!crypter.Encrypt(_vMasterKey, pMasterKey.second.vchCryptedKey))
                    return false;
                CWalletDB(*dbw).WriteMasterKey(pMasterKey.first, pMasterKey.second);
                if (fWasLocked)
                    Lock();
                return true;
            }
        }
    }

    return false;
}

void CellWallet::SetBestChain(const CellBlockLocator& loc)
{
    CWalletDB walletdb(*dbw);
    walletdb.WriteBestBlock(loc);
}

bool CellWallet::SetMinVersion(enum WalletFeature nVersion, CWalletDB* pwalletdbIn, bool fExplicit)
{
    LOCK(cs_wallet); // nWalletVersion
    if (nWalletVersion >= nVersion)
        return true;

    // when doing an explicit upgrade, if we pass the max version permitted, upgrade all the way
    if (fExplicit && nVersion > nWalletMaxVersion)
            nVersion = FEATURE_LATEST;

    nWalletVersion = nVersion;

    if (nVersion > nWalletMaxVersion)
        nWalletMaxVersion = nVersion;

    {
        CWalletDB* pwalletdb = pwalletdbIn ? pwalletdbIn : new CWalletDB(*dbw);
        if (nWalletVersion > 40000)
            pwalletdb->WriteMinVersion(nWalletVersion);
        if (!pwalletdbIn)
            delete pwalletdb;
    }

    return true;
}

bool CellWallet::SetMaxVersion(int nVersion)
{
    LOCK(cs_wallet); // nWalletVersion, nWalletMaxVersion
    // cannot downgrade below current version
    if (nWalletVersion > nVersion)
        return false;

    nWalletMaxVersion = nVersion;

    return true;
}

std::set<uint256> CellWallet::GetConflicts(const uint256& txid) const
{
    std::set<uint256> result;
    AssertLockHeld(cs_wallet);

    std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.find(txid);
    if (it == mapWallet.end())
        return result;
    const CellWalletTx& wtx = it->second;

    std::pair<TxSpends::const_iterator, TxSpends::const_iterator> range;

    for (const CellTxIn& txin : wtx.tx->vin)
    {
        if (mapTxSpends.count(txin.prevout) <= 1)
            continue;  // No conflict if zero or one spends
        range = mapTxSpends.equal_range(txin.prevout);
        for (TxSpends::const_iterator _it = range.first; _it != range.second; ++_it)
            result.insert(_it->second);
    }
    return result;
}

bool CellWallet::HasWalletSpend(const uint256& txid) const
{
    AssertLockHeld(cs_wallet);
    auto iter = mapTxSpends.lower_bound(CellOutPoint(txid, 0));
    return (iter != mapTxSpends.end() && iter->first.hash == txid);
}

void CellWallet::Flush(bool shutdown)
{
    dbw->Flush(shutdown);
}

bool CellWallet::Verify()
{
    if (gArgs.GetBoolArg("-disablewallet", DEFAULT_DISABLE_WALLET))
        return true;

    uiInterface.InitMessage(_("Verifying wallet(s)..."));

    // Keep track of each wallet absolute path to detect duplicates.
    std::set<fs::path> wallet_paths;

    for (const std::string& walletFile : gArgs.GetArgs("-wallet")) {
        if (boost::filesystem::path(walletFile).filename() != walletFile) {
            return InitError(strprintf(_("Error loading wallet %s. -wallet parameter must only specify a filename (not a path)."), walletFile));
        }

        if (SanitizeString(walletFile, SAFE_CHARS_FILENAME) != walletFile) {
            return InitError(strprintf(_("Error loading wallet %s. Invalid characters in -wallet filename."), walletFile));
        }

        fs::path wallet_path = fs::absolute(walletFile, GetDataDir());

        if (fs::exists(wallet_path) && (!fs::is_regular_file(wallet_path) || fs::is_symlink(wallet_path))) {
            return InitError(strprintf(_("Error loading wallet %s. -wallet filename must be a regular file."), walletFile));
        }

        if (!wallet_paths.insert(wallet_path).second) {
            return InitError(strprintf(_("Error loading wallet %s. Duplicate -wallet filename specified."), walletFile));
        }

        std::string strError;
        if (!CWalletDB::VerifyEnvironment(walletFile, GetDataDir().string(), strError)) {
            return InitError(strError);
        }

        if (gArgs.GetBoolArg("-salvagewallet", false)) {
            // Recover readable keypairs:
            CellWallet dummyWallet;
            std::string backup_filename;
            if (!CWalletDB::Recover(walletFile, (void *)&dummyWallet, CWalletDB::RecoverKeysOnlyFilter, backup_filename)) {
                return false;
            }
        }

        std::string strWarning;
        bool dbV = CWalletDB::VerifyDatabaseFile(walletFile, GetDataDir().string(), strWarning, strError);
        if (!strWarning.empty()) {
            InitWarning(strWarning);
        }
        if (!dbV) {
            InitError(strError);
            return false;
        }
    }

    return true;
}

void CellWallet::SyncMetaData(std::pair<TxSpends::iterator, TxSpends::iterator> range)
{
    // We want all the wallet transactions in range to have the same metadata as
    // the oldest (smallest nOrderPos).
    // So: find smallest nOrderPos:

    int nMinOrderPos = std::numeric_limits<int>::max();
    const CellWalletTx* copyFrom = nullptr;
    for (TxSpends::iterator it = range.first; it != range.second; ++it)
    {
        const uint256& hash = it->second;
        int n = mapWallet[hash].nOrderPos;
        if (n < nMinOrderPos)
        {
            nMinOrderPos = n;
            copyFrom = &mapWallet[hash];
        }
    }
    // Now copy data from copyFrom to rest:
    for (TxSpends::iterator it = range.first; it != range.second; ++it)
    {
        const uint256& hash = it->second;
        CellWalletTx* copyTo = &mapWallet[hash];
        if (copyFrom == copyTo) continue;
        if (!copyFrom->IsEquivalentTo(*copyTo)) continue;
        copyTo->mapValue = copyFrom->mapValue;
        copyTo->vOrderForm = copyFrom->vOrderForm;
        // fTimeReceivedIsTxTime not copied on purpose
        // nTimeReceived not copied on purpose
        copyTo->nTimeSmart = copyFrom->nTimeSmart;
        copyTo->fFromMe = copyFrom->fFromMe;
        copyTo->strFromAccount = copyFrom->strFromAccount;
        // nOrderPos not copied on purpose
        // cached members not copied on purpose
    }
}

/**
 * Outpoint is spent if any non-conflicted transaction
 * spends it:
 */
bool CellWallet::IsSpent(const uint256& hash, unsigned int n) const
{
    const CellOutPoint outpoint(hash, n);
    std::pair<TxSpends::const_iterator, TxSpends::const_iterator> range;
    range = mapTxSpends.equal_range(outpoint);

    for (TxSpends::const_iterator it = range.first; it != range.second; ++it)
    {
        const uint256& wtxid = it->second;
        std::map<uint256, CellWalletTx>::const_iterator mit = mapWallet.find(wtxid);
        if (mit != mapWallet.end()) {
            int depth = mit->second.GetDepthInMainChain();
            if (depth > 0  || (depth == 0 && !mit->second.isAbandoned()))
                return true; // Spent
        }
    }
    return false;
}

void CellWallet::AddToSpends(const CellOutPoint& outpoint, const uint256& wtxid)
{
    mapTxSpends.insert(std::make_pair(outpoint, wtxid));

    std::pair<TxSpends::iterator, TxSpends::iterator> range;
    range = mapTxSpends.equal_range(outpoint);
    SyncMetaData(range);
}


void CellWallet::AddToSpends(const uint256& wtxid)
{
    assert(mapWallet.count(wtxid));
    CellWalletTx& thisTx = mapWallet[wtxid];
    if (thisTx.IsCoinBase()) // Coinbases don't spend anything!
        return;
	if (thisTx.tx->IsBranchChainTransStep2() && thisTx.tx->fromBranchId == CellBaseChainParams::MAIN)
		return;

    for (const CellTxIn& txin : thisTx.tx->vin)
        AddToSpends(txin.prevout, wtxid);
}

bool CellWallet::EncryptWallet(const SecureString& strWalletPassphrase)
{
    if (IsCrypted())
        return false;

    CellKeyingMaterial _vMasterKey;

    _vMasterKey.resize(WALLET_CRYPTO_KEY_SIZE);
    GetStrongRandBytes(&_vMasterKey[0], WALLET_CRYPTO_KEY_SIZE);

    CellMasterKey kMasterKey;

    kMasterKey.vchSalt.resize(WALLET_CRYPTO_SALT_SIZE);
    GetStrongRandBytes(&kMasterKey.vchSalt[0], WALLET_CRYPTO_SALT_SIZE);

    CellCrypter crypter;
    int64_t nStartTime = GetTimeMillis();
    crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, 25000, kMasterKey.nDerivationMethod);
    kMasterKey.nDeriveIterations = 2500000 / ((double)(GetTimeMillis() - nStartTime));

    nStartTime = GetTimeMillis();
    crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations, kMasterKey.nDerivationMethod);
    kMasterKey.nDeriveIterations = (kMasterKey.nDeriveIterations + kMasterKey.nDeriveIterations * 100 / ((double)(GetTimeMillis() - nStartTime))) / 2;

    if (kMasterKey.nDeriveIterations < 25000)
        kMasterKey.nDeriveIterations = 25000;

    LogPrintf("Encrypting Wallet with an nDeriveIterations of %i\n", kMasterKey.nDeriveIterations);

    if (!crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations, kMasterKey.nDerivationMethod))
        return false;
    if (!crypter.Encrypt(_vMasterKey, kMasterKey.vchCryptedKey))
        return false;

    {
        LOCK(cs_wallet);
        mapMasterKeys[++nMasterKeyMaxID] = kMasterKey;
        assert(!pwalletdbEncryption);
        pwalletdbEncryption = new CWalletDB(*dbw);
        if (!pwalletdbEncryption->TxnBegin()) {
            delete pwalletdbEncryption;
            pwalletdbEncryption = nullptr;
            return false;
        }
        pwalletdbEncryption->WriteMasterKey(nMasterKeyMaxID, kMasterKey);

        if (!EncryptKeys(_vMasterKey))
        {
            pwalletdbEncryption->TxnAbort();
            delete pwalletdbEncryption;
            // We now probably have half of our keys encrypted in memory, and half not...
            // die and let the user reload the unencrypted wallet.
            assert(false);
        }

        // Encryption was introduced in version 0.4.0
        SetMinVersion(FEATURE_WALLETCRYPT, pwalletdbEncryption, true);

        if (!pwalletdbEncryption->TxnCommit()) {
            delete pwalletdbEncryption;
            // We now have keys encrypted in memory, but not on disk...
            // die to avoid confusion and let the user reload the unencrypted wallet.
            assert(false);
        }

        delete pwalletdbEncryption;
        pwalletdbEncryption = nullptr;

        Lock();
        Unlock(strWalletPassphrase);

        // if we are using HD, replace the HD master key (seed) with a new one
        if (IsHDEnabled()) {
            if (!SetHDMasterKey(GenerateNewHDMasterKey())) {
                return false;
            }
        }

        NewKeyPool();
        Lock();

        // Need to completely rewrite the wallet file; if we don't, bdb might keep
        // bits of the unencrypted private key in slack space in the database file.
        dbw->Rewrite();

    }
    NotifyStatusChanged(this);

    return true;
}

DBErrors CellWallet::ReorderTransactions()
{
    LOCK(cs_wallet);
    CWalletDB walletdb(*dbw);

    // Old wallets didn't have any defined order for transactions
    // Probably a bad idea to change the output of this

    // First: get all CellWalletTx and CellAccountingEntry into a sorted-by-time multimap.
    typedef std::pair<CellWalletTx*, CellAccountingEntry*> TxPair;
    typedef std::multimap<int64_t, TxPair > TxItems;
    TxItems txByTime;

    for (std::map<uint256, CellWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
    {
        CellWalletTx* wtx = &((*it).second);
        txByTime.insert(std::make_pair(wtx->nTimeReceived, TxPair(wtx, (CellAccountingEntry*)0)));
    }
    std::list<CellAccountingEntry> acentries;
    walletdb.ListAccountCreditDebit("", acentries);
    for (CellAccountingEntry& entry : acentries)
    {
        txByTime.insert(std::make_pair(entry.nTime, TxPair((CellWalletTx*)0, &entry)));
    }

    nOrderPosNext = 0;
    std::vector<int64_t> nOrderPosOffsets;
    for (TxItems::iterator it = txByTime.begin(); it != txByTime.end(); ++it)
    {
        CellWalletTx *const pwtx = (*it).second.first;
        CellAccountingEntry *const pacentry = (*it).second.second;
        int64_t& nOrderPos = (pwtx != 0) ? pwtx->nOrderPos : pacentry->nOrderPos;

        if (nOrderPos == -1)
        {
            nOrderPos = nOrderPosNext++;
            nOrderPosOffsets.push_back(nOrderPos);

            if (pwtx)
            {
                if (!walletdb.WriteTx(*pwtx))
                    return DB_LOAD_FAIL;
            }
            else
                if (!walletdb.WriteAccountingEntry(pacentry->nEntryNo, *pacentry))
                    return DB_LOAD_FAIL;
        }
        else
        {
            int64_t nOrderPosOff = 0;
            for (const int64_t& nOffsetStart : nOrderPosOffsets)
            {
                if (nOrderPos >= nOffsetStart)
                    ++nOrderPosOff;
            }
            nOrderPos += nOrderPosOff;
            nOrderPosNext = std::max(nOrderPosNext, nOrderPos + 1);

            if (!nOrderPosOff)
                continue;

            // Since we're changing the order, write it back
            if (pwtx)
            {
                if (!walletdb.WriteTx(*pwtx))
                    return DB_LOAD_FAIL;
            }
            else
                if (!walletdb.WriteAccountingEntry(pacentry->nEntryNo, *pacentry))
                    return DB_LOAD_FAIL;
        }
    }
    walletdb.WriteOrderPosNext(nOrderPosNext);

    return DB_LOAD_OK;
}

int64_t CellWallet::IncOrderPosNext(CWalletDB *pwalletdb)
{
    AssertLockHeld(cs_wallet); // nOrderPosNext
    int64_t nRet = nOrderPosNext++;
    if (pwalletdb) {
        pwalletdb->WriteOrderPosNext(nOrderPosNext);
    } else {
        CWalletDB(*dbw).WriteOrderPosNext(nOrderPosNext);
    }
    return nRet;
}

bool CellWallet::AccountMove(std::string strFrom, std::string strTo, CellAmount nAmount, std::string strComment)
{
    CWalletDB walletdb(*dbw);
    if (!walletdb.TxnBegin())
        return false;

    int64_t nNow = GetAdjustedTime();

    // Debit
    CellAccountingEntry debit;
    debit.nOrderPos = IncOrderPosNext(&walletdb);
    debit.strAccount = strFrom;
    debit.nCreditDebit = -nAmount;
    debit.nTime = nNow;
    debit.strOtherAccount = strTo;
    debit.strComment = strComment;
    AddAccountingEntry(debit, &walletdb);

    // Credit
    CellAccountingEntry credit;
    credit.nOrderPos = IncOrderPosNext(&walletdb);
    credit.strAccount = strTo;
    credit.nCreditDebit = nAmount;
    credit.nTime = nNow;
    credit.strOtherAccount = strFrom;
    credit.strComment = strComment;
    AddAccountingEntry(credit, &walletdb);

    if (!walletdb.TxnCommit())
        return false;

    return true;
}

bool CellWallet::GetAccountPubkey(CellPubKey &pubKey, std::string strAccount, bool bForceNew)
{
    CWalletDB walletdb(*dbw);

    CellAccount account;
    walletdb.ReadAccount(strAccount, account);

    if (!bForceNew) {
        if (!account.vchPubKey.IsValid())
            bForceNew = true;
        else {
            // Check if the current key has been used
            CellScript scriptPubKey = GetScriptForDestination(account.vchPubKey.GetID());
            for (std::map<uint256, CellWalletTx>::iterator it = mapWallet.begin();
                 it != mapWallet.end() && account.vchPubKey.IsValid();
                 ++it)
                for (const CellTxOut& txout : (*it).second.tx->vout)
                    if (txout.scriptPubKey == scriptPubKey) {
                        bForceNew = true;
                        break;
                    }
        }
    }

    // Generate a new key
    if (bForceNew) {
        if (!GetKeyFromPool(account.vchPubKey, false))
            return false;

        SetAddressBook(account.vchPubKey.GetID(), strAccount, "receive");
        walletdb.WriteAccount(strAccount, account);
    }

    pubKey = account.vchPubKey;

    return true;
}

void CellWallet::MarkDirty()
{
    {
        LOCK(cs_wallet);
        for (std::pair<const uint256, CellWalletTx>& item : mapWallet)
            item.second.MarkDirty();
    }
}

bool CellWallet::MarkReplaced(const uint256& originalHash, const uint256& newHash)
{
    LOCK(cs_wallet);

    auto mi = mapWallet.find(originalHash);

    // There is a bug if MarkReplaced is not called on an existing wallet transaction.
    assert(mi != mapWallet.end());

    CellWalletTx& wtx = (*mi).second;

    // Ensure for now that we're not overwriting data
    assert(wtx.mapValue.count("replaced_by_txid") == 0);

    wtx.mapValue["replaced_by_txid"] = newHash.ToString();

    CWalletDB walletdb(*dbw, "r+");

    bool success = true;
    if (!walletdb.WriteTx(wtx)) {
        LogPrintf("%s: Updating walletdb tx %s failed", __func__, wtx.GetHash().ToString());
        success = false;
    }

    NotifyTransactionChanged(this, originalHash, CT_UPDATED);

    return success;
}

bool CellWallet::AddToWallet(const CellWalletTx& wtxIn, bool fFlushOnClose)
{
    LOCK(cs_wallet);

    CWalletDB walletdb(*dbw, "r+", fFlushOnClose);

    uint256 hash = wtxIn.GetHash();

    // Inserts only if not already there, returns tx inserted or tx found
    std::pair<std::map<uint256, CellWalletTx>::iterator, bool> ret = mapWallet.insert(std::make_pair(hash, wtxIn));
    CellWalletTx& wtx = (*ret.first).second;
    wtx.BindWallet(this);
    bool fInsertedNew = ret.second;
    if (fInsertedNew)
    {
        wtx.nTimeReceived = GetAdjustedTime();
        wtx.nOrderPos = IncOrderPosNext(&walletdb);
        wtxOrdered.insert(std::make_pair(wtx.nOrderPos, TxPair(&wtx, (CellAccountingEntry*)0)));
        wtx.nTimeSmart = ComputeTimeSmart(wtx);
        AddToSpends(hash);
    }

    bool fUpdated = false;
    if (!fInsertedNew)
    {
        // Merge
        if (!wtxIn.hashUnset() && wtxIn.hashBlock != wtx.hashBlock)
        {
            wtx.hashBlock = wtxIn.hashBlock;
            fUpdated = true;
        }
        // If no longer abandoned, update
        if (wtxIn.hashBlock.IsNull() && wtx.isAbandoned())
        {
            wtx.hashBlock = wtxIn.hashBlock;
            fUpdated = true;
        }
        if (wtxIn.nIndex != -1 && (wtxIn.nIndex != wtx.nIndex))
        {
            wtx.nIndex = wtxIn.nIndex;
            fUpdated = true;
        }
        if (wtxIn.fFromMe && wtxIn.fFromMe != wtx.fFromMe)
        {
            wtx.fFromMe = wtxIn.fFromMe;
            fUpdated = true;
        }
        // If we have a witness-stripped version of this transaction, and we
        // see a new version with a witness, then we must be upgrading a pre-segwit
        // wallet.  Store the new version of the transaction with the witness,
        // as the stripped-version must be invalid.
        // TODO: Store all versions of the transaction, instead of just one.
        if (wtxIn.tx->HasWitness() && !wtx.tx->HasWitness()) {
            wtx.SetTx(wtxIn.tx);
            fUpdated = true;
        }
    }

    //// debug print
    if (!fFastMode)
        ;// LogPrintf("AddToWallet %s  %s%s\n", wtxIn.GetHash().ToString(), (fInsertedNew ? "new" : ""), (fUpdated ? "update" : ""));

    // Write to disk
    if (fInsertedNew || fUpdated)
        if (!walletdb.WriteTx(wtx))
            return false;

    // Break debit/credit balance caches:
    wtx.MarkDirty();

    // Notify UI of new or updated transaction
    NotifyTransactionChanged(this, hash, fInsertedNew ? CT_NEW : CT_UPDATED);

    // notify an external script when a wallet transaction comes in or is updated
    std::string strCmd = gArgs.GetArg("-walletnotify", "");

    if ( !strCmd.empty())
    {
        boost::replace_all(strCmd, "%s", wtxIn.GetHash().GetHex());
        boost::thread t(runCommand, strCmd); // thread runs free
    }

    return true;
}

bool CellWallet::LoadToWallet(const CellWalletTx& wtxIn)
{
    uint256 hash = wtxIn.GetHash();

    mapWallet[hash] = wtxIn;
    CellWalletTx& wtx = mapWallet[hash];
    wtx.BindWallet(this);
    wtxOrdered.insert(std::make_pair(wtx.nOrderPos, TxPair(&wtx, (CellAccountingEntry*)0)));
    AddToSpends(hash);
    for (const CellTxIn& txin : wtx.tx->vin) {
        if (mapWallet.count(txin.prevout.hash)) {
            CellWalletTx& prevtx = mapWallet[txin.prevout.hash];
            if (prevtx.nIndex == -1 && !prevtx.hashUnset()) {
                MarkConflicted(prevtx.hashBlock, wtx.GetHash());
            }
        }
    }

    return true;
}

/**
 * Add a transaction to the wallet, or update it.  pIndex and posInBlock should
 * be set when the transaction was known to be included in a block.  When
 * pIndex == nullptr, then wallet state is not updated in AddToWallet, but
 * notifications happen and cached balances are marked dirty.
 *
 * If fUpdate is true, existing transactions will be updated.
 * TODO: One exception to this is that the abandoned state is cleared under the
 * assumption that any further notification of a transaction that was considered
 * abandoned is an indication that it is not safe to be considered abandoned.
 * Abandoned state should probably be more carefully tracked via different
 * posInBlock signals or by checking mempool presence when necessary.
 */
bool CellWallet::AddToWalletIfInvolvingMe(const CellTransactionRef& ptx, const CellBlockIndex* pIndex, int posInBlock, bool fUpdate)
{
    const CellTransaction& tx = *ptx;
    {
        AssertLockHeld(cs_wallet);

        if (pIndex != nullptr) {
            for (const CellTxIn& txin : tx.vin) {
                std::pair<TxSpends::const_iterator, TxSpends::const_iterator> range = mapTxSpends.equal_range(txin.prevout);
                while (range.first != range.second) {
                    if (range.first->second != tx.GetHash()) {
                        LogPrintf("Transaction %s (in block %s) conflicts with wallet transaction %s (both spend %s:%i)\n", tx.GetHash().ToString(), pIndex->GetBlockHash().ToString(), range.first->second.ToString(), range.first->first.hash.ToString(), range.first->first.n);
                        MarkConflicted(pIndex->GetBlockHash(), range.first->second);
                    }
                    range.first++;
                }
            }
        }

        bool fExisted = mapWallet.count(tx.GetHash()) != 0;
        if (fExisted && !fUpdate) return false;
        if (fExisted || IsMine(tx) || IsFromMe(tx))
        {
            /* Check if any keys in the wallet keypool that were supposed to be unused
             * have appeared in a new transaction. If so, remove those keys from the keypool.
             * This can happen when restoring an old wallet backup that does not contain
             * the mostly recently created transactions from newer versions of the wallet.
             */

            // loop though all outputs
            for (const CellTxOut& txout: tx.vout) {
                // extract addresses and check if they match with an unused keypool key
                std::vector<CellKeyID> vAffected;
                CellAffectedKeysVisitor(*this, vAffected).Process(txout.scriptPubKey);
                for (const CellKeyID &keyid : vAffected) {
                    std::map<CellKeyID, int64_t>::const_iterator mi = m_pool_key_to_index.find(keyid);
                    if (mi != m_pool_key_to_index.end()) {
                        LogPrintf("%s: Detected a used keypool key, mark all keypool key up to this key as used\n", __func__);
                        MarkReserveKeysAsUsed(mi->second);

                        if (!TopUpKeyPool()) {
                            LogPrintf("%s: Topping up keypool failed (locked wallet)\n", __func__);
                        }
                    }
                }
            }

            CellWalletTx wtx(this, ptx);

            // Get merkle branch if transaction was found in a block
            if (pIndex != nullptr)
                wtx.SetMerkleBranch(pIndex, posInBlock);

            return AddToWallet(wtx, false);
        }
    }
    return false;
}

bool CellWallet::TransactionCanBeAbandoned(const uint256& hashTx) const
{
    LOCK2(cs_main, cs_wallet);
    const CellWalletTx* wtx = GetWalletTx(hashTx);
    return wtx && !wtx->isAbandoned() && wtx->GetDepthInMainChain() <= 0 && !wtx->InMempool();
}

bool CellWallet::AbandonTransaction(const uint256& hashTx)
{
    LOCK2(cs_main, cs_wallet);

    CWalletDB walletdb(*dbw, "r+");

    std::set<uint256> todo;
    std::set<uint256> done;

    // Can't mark abandoned if confirmed or in mempool
    assert(mapWallet.count(hashTx));
    CellWalletTx& origtx = mapWallet[hashTx];
    if (origtx.GetDepthInMainChain() > 0 || origtx.InMempool()) {
        return false;
    }

    todo.insert(hashTx);

    while (!todo.empty()) {
        uint256 now = *todo.begin();
        todo.erase(now);
        done.insert(now);
        assert(mapWallet.count(now));
        CellWalletTx& wtx = mapWallet[now];
        int currentconfirm = wtx.GetDepthInMainChain();
        // If the orig tx was not in block, none of its spends can be
        assert(currentconfirm <= 0);
        // if (currentconfirm < 0) {Tx and spends are already conflicted, no need to abandon}
        if (currentconfirm == 0 && !wtx.isAbandoned()) {
            // If the orig tx was not in block/mempool, none of its spends can be in mempool
            assert(!wtx.InMempool());
            wtx.nIndex = -1;
            wtx.setAbandoned();
            wtx.MarkDirty();
            walletdb.WriteTx(wtx);
            NotifyTransactionChanged(this, wtx.GetHash(), CT_UPDATED);
            // Iterate over all its outputs, and mark transactions in the wallet that spend them abandoned too
            TxSpends::const_iterator iter = mapTxSpends.lower_bound(CellOutPoint(hashTx, 0));
            while (iter != mapTxSpends.end() && iter->first.hash == now) {
                if (!done.count(iter->second)) {
                    todo.insert(iter->second);
                }
                iter++;
            }
            // If a transaction changes 'conflicted' state, that changes the balance
            // available of the outputs it spends. So force those to be recomputed
            for (const CellTxIn& txin : wtx.tx->vin)
            {
                if (mapWallet.count(txin.prevout.hash))
                    mapWallet[txin.prevout.hash].MarkDirty();
            }
        }
    }

    return true;
}

void CellWallet::MarkConflicted(const uint256& hashBlock, const uint256& hashTx)
{
    LOCK2(cs_main, cs_wallet);

    int conflictconfirms = 0;
    if (mapBlockIndex.count(hashBlock)) {
        CellBlockIndex* pindex = mapBlockIndex[hashBlock];
        if (chainActive.Contains(pindex)) {
            conflictconfirms = -(chainActive.Height() - pindex->nHeight + 1);
        }
    }
    // If number of conflict confirms cannot be determined, this means
    // that the block is still unknown or not yet part of the main chain,
    // for example when loading the wallet during a reindex. Do nothing in that
    // case.
    if (conflictconfirms >= 0)
        return;

    // Do not flush the wallet here for performance reasons
    CWalletDB walletdb(*dbw, "r+", false);

    std::set<uint256> todo;
    std::set<uint256> done;

    todo.insert(hashTx);

    while (!todo.empty()) {
        uint256 now = *todo.begin();
        todo.erase(now);
        done.insert(now);
        assert(mapWallet.count(now));
        CellWalletTx& wtx = mapWallet[now];
        int currentconfirm = wtx.GetDepthInMainChain();
        if (conflictconfirms < currentconfirm) {
            // Block is 'more conflicted' than current confirm; update.
            // Mark transaction as conflicted with this block.
            wtx.nIndex = -1;
            wtx.hashBlock = hashBlock;
            wtx.MarkDirty();
            walletdb.WriteTx(wtx);
            // Iterate over all its outputs, and mark transactions in the wallet that spend them conflicted too
            TxSpends::const_iterator iter = mapTxSpends.lower_bound(CellOutPoint(now, 0));
            while (iter != mapTxSpends.end() && iter->first.hash == now) {
                 if (!done.count(iter->second)) {
                     todo.insert(iter->second);
                 }
                 iter++;
            }
            // If a transaction changes 'conflicted' state, that changes the balance
            // available of the outputs it spends. So force those to be recomputed
            for (const CellTxIn& txin : wtx.tx->vin)
            {
                if (mapWallet.count(txin.prevout.hash))
                    mapWallet[txin.prevout.hash].MarkDirty();
            }
        }
    }
}

void CellWallet::SyncTransaction(const CellTransactionRef& ptx, const CellBlockIndex *pindex, int posInBlock) {
    const CellTransaction& tx = *ptx;

    const bool isBranch2ndBlockTx = (posInBlock == 1 && pindex->nHeight == 1 && !Params().IsMainChain());
    if (isBranch2ndBlockTx)
        return;

    if (!AddToWalletIfInvolvingMe(ptx, pindex, posInBlock, true))
        return; // Not one of ours

    // If a transaction changes 'conflicted' state, that changes the balance
    // available of the outputs it spends. So force those to be
    // recomputed, also:
    for (const CellTxIn& txin : tx.vin)
    {
        if (mapWallet.count(txin.prevout.hash))
            mapWallet[txin.prevout.hash].MarkDirty();
    }
}

void CellWallet::TransactionAddedToMempool(const CellTransactionRef& ptx) {
    LOCK2(cs_main, cs_wallet);
    SyncTransaction(ptx);
}

void CellWallet::BlockConnected(const std::shared_ptr<const CellBlock>& pblock, const CellBlockIndex *pindex, const std::vector<CellTransactionRef>& vtxConflicted) {
    LOCK2(cs_main, cs_wallet);
    // TODO: Temporarily ensure that mempool removals are notified before
    // connected transactions.  This shouldn't matter, but the abandoned
    // state of transactions in our wallet is currently cleared when we
    // receive another notification and there is a race condition where
    // notification of a connected conflict might cause an outside process
    // to abandon a transaction and then have it inadvertently cleared by
    // the notification that the conflicted transaction was evicted.

    for (const CellTransactionRef& ptx : vtxConflicted) {
        SyncTransaction(ptx);
    }
    for (size_t i = 0; i < pblock->vtx.size(); i++) {
        SyncTransaction(pblock->vtx[i], pindex, i);
    }
}

void CellWallet::BlockDisconnected(const std::shared_ptr<const CellBlock>& pblock) {
    LOCK2(cs_main, cs_wallet);

    for (const CellTransactionRef& ptx : pblock->vtx) {
        SyncTransaction(ptx);
    }
}



isminetype CellWallet::IsMine(const CellTxIn &txin) const
{
    {
        LOCK(cs_wallet);
        std::map<uint256, CellWalletTx>::const_iterator mi = mapWallet.find(txin.prevout.hash);
        if (mi != mapWallet.end())
        {
            const CellWalletTx& prev = (*mi).second;
            if (txin.prevout.n < prev.tx->vout.size())
                return IsMine(prev.tx->vout[txin.prevout.n]);
        }
    }
    return ISMINE_NO;
}

// Note that this function doesn't distinguish between a 0-valued input,
// and a not-"is mine" (according to the filter) input.
CellAmount CellWallet::GetDebit(const CellTxIn &txin, const isminefilter& filter) const
{
    {
        LOCK(cs_wallet);
        std::map<uint256, CellWalletTx>::const_iterator mi = mapWallet.find(txin.prevout.hash);
        if (mi != mapWallet.end())
        {
            const CellWalletTx& prev = (*mi).second;
            if (txin.prevout.n < prev.tx->vout.size())
                if (IsMine(prev.tx->vout[txin.prevout.n]) & filter)
                    return prev.tx->vout[txin.prevout.n].nValue;
        }
    }
    return 0;
}

isminetype CellWallet::IsMine(const CellTxOut& txout) const
{
    return ::IsMine(*this, txout.scriptPubKey);
}

CellAmount CellWallet::GetCredit(const CellTxOut& txout, const isminefilter& filter) const
{
    if (!MoneyRange(txout.nValue))
        throw std::runtime_error(std::string(__func__) + ": value out of range");
    return ((IsMine(txout) & filter) ? txout.nValue : 0);
}

bool CellWallet::IsChange(const CellTxOut& txout) const
{
    // TODO: fix handling of 'change' outputs. The assumption is that any
    // payment to a script that is ours, but is not in the address book
    // is change. That assumption is likely to break when we implement multisignature
    // wallets that return change back into a multi-signature-protected address;
    // a better way of identifying which outputs are 'the send' and which are
    // 'the change' will need to be implemented (maybe extend CellWalletTx to remember
    // which output, if any, was change).
    if (::IsMine(*this, txout.scriptPubKey))
    {
        CellTxDestination address;
        if (!ExtractDestination(txout.scriptPubKey, address))
            return true;

        LOCK(cs_wallet);
        if (!mapAddressBook.count(address))
            return true;
    }
    return false;
}

CellAmount CellWallet::GetChange(const CellTxOut& txout) const
{
    if (!MoneyRange(txout.nValue))
        throw std::runtime_error(std::string(__func__) + ": value out of range");
    return (IsChange(txout) ? txout.nValue : 0);
}

bool CellWallet::IsMine(const CellTransaction& tx) const
{
    for (const CellTxOut& txout : tx.vout)
        if (IsMine(txout))
            return true;
    return false;
}

bool CellWallet::IsFromMe(const CellTransaction& tx) const
{
    return (GetDebit(tx, ISMINE_ALL) > 0);
}

CellAmount CellWallet::GetDebit(const CellTransaction& tx, const isminefilter& filter) const
{
    CellAmount nDebit = 0;
    for (const CellTxIn& txin : tx.vin)
    {
        nDebit += GetDebit(txin, filter);
        if (!MoneyRange(nDebit))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    return nDebit;
}

bool CellWallet::IsAllFromMe(const CellTransaction& tx, const isminefilter& filter) const
{
    LOCK(cs_wallet);

    for (const CellTxIn& txin : tx.vin)
    {
        auto mi = mapWallet.find(txin.prevout.hash);
        if (mi == mapWallet.end())
            return false; // any unknown inputs can't be from us

        const CellWalletTx& prev = (*mi).second;

        if (txin.prevout.n >= prev.tx->vout.size())
            return false; // invalid input!

        if (!(IsMine(prev.tx->vout[txin.prevout.n]) & filter))
            return false;
    }
    return true;
}

CellAmount CellWallet::GetCredit(const CellTransaction& tx, const isminefilter& filter) const
{
    CellAmount nCredit = 0;
    for (const CellTxOut& txout : tx.vout)
    {
        nCredit += GetCredit(txout, filter);
        if (!MoneyRange(nCredit))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    return nCredit;
}

CellAmount CellWallet::GetChange(const CellTransaction& tx) const
{
    CellAmount nChange = 0;
    for (const CellTxOut& txout : tx.vout)
    {
        nChange += GetChange(txout);
        if (!MoneyRange(nChange))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    return nChange;
}

CellPubKey CellWallet::GenerateNewHDMasterKey()
{
    CellKey key;
    key.MakeNewKey(true);

    int64_t nCreationTime = GetTime();
    CKeyMetadata metadata(nCreationTime);

    // calculate the pubkey
    CellPubKey pubkey = key.GetPubKey();
    assert(key.VerifyPubKey(pubkey));

    // set the hd keypath to "m" -> Master, refers the masterkeyid to itself
    metadata.hdKeypath     = "m";
    metadata.hdMasterKeyID = pubkey.GetID();

    {
        LOCK(cs_wallet);

        // mem store the metadata
        mapKeyMetadata[pubkey.GetID()] = metadata;

        // write the key&metadata to the database
        if (!AddKeyPubKey(key, pubkey))
            throw std::runtime_error(std::string(__func__) + ": AddKeyPubKey failed");
    }

    return pubkey;
}

bool CellWallet::SetHDMasterKey(const CellPubKey& pubkey)
{
    LOCK(cs_wallet);
    // store the keyid (hash160) together with
    // the child index counter in the database
    // as a hdchain object
    CHDChain newHdChain;
    newHdChain.nVersion = CanSupportFeature(FEATURE_HD_SPLIT) ? CHDChain::VERSION_HD_CHAIN_SPLIT : CHDChain::VERSION_HD_BASE;
    newHdChain.masterKeyID = pubkey.GetID();
    SetHDChain(newHdChain, false);

    return true;
}

bool CellWallet::SetHDChain(const CHDChain& chain, bool memonly)
{
    LOCK(cs_wallet);
    if (!memonly && !CWalletDB(*dbw).WriteHDChain(chain))
        throw std::runtime_error(std::string(__func__) + ": writing chain failed");

    hdChain = chain;
    return true;
}

bool CellWallet::IsHDEnabled() const
{
    return !hdChain.masterKeyID.IsNull();
}

int64_t CellWalletTx::GetTxTime() const
{
    int64_t n = nTimeSmart;
    return n ? n : nTimeReceived;
}

int CellWalletTx::GetRequestCount() const
{
    // Returns -1 if it wasn't being tracked
    int nRequests = -1;
    {
        LOCK(pwallet->cs_wallet);
        if (IsCoinBase())
        {
            // Generated block
            if (!hashUnset())
            {
                std::map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(hashBlock);
                if (mi != pwallet->mapRequestCount.end())
                    nRequests = (*mi).second;
            }
        }
        else
        {
            // Did anyone request this transaction?
            std::map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(GetHash());
            if (mi != pwallet->mapRequestCount.end())
            {
                nRequests = (*mi).second;

                // How about the block it's in?
                if (nRequests == 0 && !hashUnset())
                {
                    std::map<uint256, int>::const_iterator _mi = pwallet->mapRequestCount.find(hashBlock);
                    if (_mi != pwallet->mapRequestCount.end())
                        nRequests = (*_mi).second;
                    else
                        nRequests = 1; // If it's in someone else's block it must have got out
                }
            }
        }
    }
    return nRequests;
}

void CellWalletTx::GetAmounts(std::list<CellOutputEntry>& listReceived,
                           std::list<CellOutputEntry>& listSent, CellAmount& nFee, std::string& strSentAccount, const isminefilter& filter) const
{
    nFee = 0;
    listReceived.clear();
    listSent.clear();
    strSentAccount = strFromAccount;

    // Compute fee:
    CellAmount nDebit = GetDebit(filter);
    if (nDebit > 0) // debit>0 means we signed/sent this transaction
    {
        CellAmount nValueOut = tx->GetValueOut();
        nFee = nDebit - nValueOut;
    }

    // Sent/received.
    for (unsigned int i = 0; i < tx->vout.size(); ++i)
    {
        const CellTxOut& txout = tx->vout[i];
        isminetype fIsMine = pwallet->IsMine(txout);
        // Only need to handle txouts if AT LEAST one of these is true:
        //   1) they debit from us (sent)
        //   2) the output is to us (received)
        if (nDebit > 0)
        {
            // Don't report 'change' txouts
            if (pwallet->IsChange(txout))
                continue;
        }
        else if (!(fIsMine & filter))
            continue;

        // In either case, we need to get the destination address
        CellTxDestination address;

        if (!ExtractDestination(txout.scriptPubKey, address) && !txout.scriptPubKey.IsUnspendable())
        {
            LogPrintf("CellWalletTx::GetAmounts: Unknown transaction type found, txid %s\n",
                     this->GetHash().ToString());
            address = CellNoDestination();
        }

        CellOutputEntry output = {address, txout.nValue, (int)i};

        // If we are debited by the transaction, add the output as a "sent" entry
        if (nDebit > 0)
            listSent.push_back(output);

        // If we are receiving the output, add it as a "received" entry
        if (fIsMine & filter)
            listReceived.push_back(output);
    }

}

/**
 * Scan active chain for relevant transactions after importing keys. This should
 * be called whenever new keys are added to the wallet, with the oldest key
 * creation time.
 *
 * @return Earliest timestamp that could be successfully scanned from. Timestamp
 * returned will be higher than startTime if relevant blocks could not be read.
 */
int64_t CellWallet::RescanFromTime(int64_t startTime, bool update)
{
    AssertLockHeld(cs_main);
    AssertLockHeld(cs_wallet);

    // Find starting block. May be null if nCreateTime is greater than the
    // highest blockchain timestamp, in which case there is nothing that needs
    // to be scanned.
    CellBlockIndex* const startBlock = chainActive.FindEarliestAtLeast(startTime - TIMESTAMP_WINDOW);
    LogPrintf("%s: Rescanning last %i blocks\n", __func__, startBlock ? chainActive.Height() - startBlock->nHeight + 1 : 0);

    if (startBlock) {
        const CellBlockIndex* const failedBlock = ScanForWalletTransactions(startBlock, update);
        if (failedBlock) {
            return failedBlock->GetBlockTimeMax() + TIMESTAMP_WINDOW + 1;
        }
    }
    return startTime;
}

/**
 * Scan the block chain (starting in pindexStart) for transactions
 * from or to us. If fUpdate is true, found transactions that already
 * exist in the wallet will be updated.
 *
 * Returns null if scan was successful. Otherwise, if a complete rescan was not
 * possible (due to pruning or corruption), returns pointer to the most recent
 * block that could not be scanned.
 */
CellBlockIndex* CellWallet::ScanForWalletTransactions(CellBlockIndex* pindexStart, bool fUpdate)
{
    int64_t nNow = GetTime();
    const CellChainParams& chainParams = Params();

    CellBlockIndex* pindex = pindexStart;
    CellBlockIndex* ret = nullptr;
    {
        LOCK2(cs_main, cs_wallet);
        fAbortRescan = false;
        fScanningWallet = true;

        ShowProgress(_("Rescanning..."), 0); // show rescan progress in GUI as dialog or on splashscreen, if -rescan on startup
        double dProgressStart = GuessVerificationProgress(chainParams.TxData(), pindex);
        double dProgressTip = GuessVerificationProgress(chainParams.TxData(), chainActive.Tip());
        while (pindex && !fAbortRescan)
        {
            if (pindex->nHeight % 100 == 0 && dProgressTip - dProgressStart > 0.0)
                ShowProgress(_("Rescanning..."), std::max(1, std::min(99, (int)((GuessVerificationProgress(chainParams.TxData(), pindex) - dProgressStart) / (dProgressTip - dProgressStart) * 100))));
            if (GetTime() >= nNow + 60) {
                nNow = GetTime();
                LogPrintf("Still rescanning. At block %d. Progress=%f\n", pindex->nHeight, GuessVerificationProgress(chainParams.TxData(), pindex));
            }

            CellBlock block;
            if (ReadBlockFromDisk(block, pindex, Params().GetConsensus())) {
                for (size_t posInBlock = 0; posInBlock < block.vtx.size(); ++posInBlock) {
                    const bool isBranch2ndBlockTx = (posInBlock == 1 && pindex->nHeight == 1 && !Params().IsMainChain());
                    if (isBranch2ndBlockTx)
                        continue;
                    AddToWalletIfInvolvingMe(block.vtx[posInBlock], pindex, posInBlock, fUpdate);
                }
            } else {
                ret = pindex;
            }
            pindex = chainActive.Next(pindex);
        }
        if (pindex && fAbortRescan) {
            LogPrintf("Rescan aborted at block %d. Progress=%f\n", pindex->nHeight, GuessVerificationProgress(chainParams.TxData(), pindex));
        }
        ShowProgress(_("Rescanning..."), 100); // hide progress dialog in GUI

        fScanningWallet = false;
    }
    return ret;
}

void CellWallet::ReacceptWalletTransactions()
{
    // If transactions aren't being broadcasted, don't let them into local mempool either
    if (!fBroadcastTransactions)
        return;
    LOCK2(cs_main, cs_wallet);
    std::map<int64_t, CellWalletTx*> mapSorted;

    // Sort pending wallet transactions based on their initial wallet insertion order
    for (std::pair<const uint256, CellWalletTx>& item : mapWallet)
    {
        const uint256& wtxid = item.first;
        CellWalletTx& wtx = item.second;
        assert(wtx.GetHash() == wtxid);

        int nDepth = wtx.GetDepthInMainChain();

        if (!wtx.IsCoinBase() && (nDepth == 0 && !wtx.isAbandoned())) {
            mapSorted.insert(std::make_pair(wtx.nOrderPos, &wtx));
        }
    }

    // Try to add wallet transactions to memory pool
    for (std::pair<const int64_t, CellWalletTx*>& item : mapSorted)
    {
        CellWalletTx& wtx = *(item.second);

        LOCK(mempool.cs);
        CellValidationState state;
        wtx.AcceptToMemoryPool(maxTxFee, state, false);
    }
}

bool CellWalletTx::RelayWalletTransaction(CellConnman* connman)
{
    assert(pwallet->GetBroadcastTransactions());
    if (!IsCoinBase() && !isAbandoned() && GetDepthInMainChain() == 0)
    {
        CellValidationState state;
        /* GetDepthInMainChain already catches known conflicts. */
        if (InMempool() || AcceptToMemoryPool(maxTxFee, state, false)) {
            LogPrintf("Relaying wtx %s\n", GetHash().ToString());
            if (connman) {
                CellInv inv(MSG_TX, GetHash());
                connman->ForEachNode([&inv](CellNode* pnode)
                {
                    pnode->PushInventory(inv);
                });
                return true;
            }
        }
    }
    return false;
}

std::set<uint256> CellWalletTx::GetConflicts() const
{
    std::set<uint256> result;
    if (pwallet != nullptr)
    {
        uint256 myHash = GetHash();
        result = pwallet->GetConflicts(myHash);
        result.erase(myHash);
    }
    return result;
}

CellAmount CellWalletTx::GetDebit(const isminefilter& filter) const
{
    if (tx->vin.empty())
        return 0;

    CellAmount debit = 0;
    if(filter & ISMINE_SPENDABLE)
    {
        if (fDebitCached)
            debit += nDebitCached;
        else
        {
            nDebitCached = pwallet->GetDebit(*this, ISMINE_SPENDABLE);
            fDebitCached = true;
            debit += nDebitCached;
        }
    }
    if(filter & ISMINE_WATCH_ONLY)
    {
        if(fWatchDebitCached)
            debit += nWatchDebitCached;
        else
        {
            nWatchDebitCached = pwallet->GetDebit(*this, ISMINE_WATCH_ONLY);
            fWatchDebitCached = true;
            debit += nWatchDebitCached;
        }
    }
    return debit;
}

CellAmount CellWalletTx::GetCredit(const isminefilter& filter) const
{
    // Must wait until coinbase is safely deep enough in the chain before valuing it
    if (IsCoinBase() && GetBlocksToMaturity() > 0)
        return 0;

    CellAmount credit = 0;
    if (filter & ISMINE_SPENDABLE)
    {
        // GetBalance can assume transactions in mapWallet won't change
        if (fCreditCached)
            credit += nCreditCached;
        else
        {
            nCreditCached = pwallet->GetCredit(*this, ISMINE_SPENDABLE);
            fCreditCached = true;
            credit += nCreditCached;
        }
    }
    if (filter & ISMINE_WATCH_ONLY)
    {
        if (fWatchCreditCached)
            credit += nWatchCreditCached;
        else
        {
            nWatchCreditCached = pwallet->GetCredit(*this, ISMINE_WATCH_ONLY);
            fWatchCreditCached = true;
            credit += nWatchCreditCached;
        }
    }
    return credit;
}

CellAmount CellWalletTx::GetImmatureCredit(bool fUseCache) const
{
    if (IsCoinBase() && GetBlocksToMaturity() > 0 && IsInMainChain())
    {
        if (fUseCache && fImmatureCreditCached)
            return nImmatureCreditCached;
        nImmatureCreditCached = pwallet->GetCredit(*this, ISMINE_SPENDABLE);
        fImmatureCreditCached = true;
        return nImmatureCreditCached;
    }

    return 0;
}

CellAmount CellWalletTx::GetAvailableCredit(bool fUseCache) const
{
    if (pwallet == 0)
        return 0;

    // Must wait until coinbase is safely deep enough in the chain before valuing it
    if (IsCoinBase() && GetBlocksToMaturity() > 0)
        return 0;

    if (fUseCache && fAvailableCreditCached)
        return nAvailableCreditCached;

    CellAmount nCredit = 0;
    uint256 hashTx = GetHash();
    for (unsigned int i = 0; i < tx->vout.size(); i++)
    {
        if (!pwallet->IsSpent(hashTx, i))
        {
            const CellTxOut &txout = tx->vout[i];
            if (QuickGetBranchScriptType(txout.scriptPubKey) != BST_INVALID)// 抵押币、挖矿币不能使用
                continue;
            if (tx->IsBranchCreate() && IsCoinCreateBranchScript(txout.scriptPubKey)
                && GetBlocksToMaturityForCoinCreateBranch() > 0)
                continue;
            nCredit += pwallet->GetCredit(txout, ISMINE_SPENDABLE);
            if (!MoneyRange(nCredit))
                throw std::runtime_error(std::string(__func__) + " : value out of range");
        }
    }

    nAvailableCreditCached = nCredit;
    fAvailableCreditCached = true;
    return nCredit;
}

CellAmount CellWalletTx::GetImmatureWatchOnlyCredit(const bool& fUseCache) const
{
    if (IsCoinBase() && GetBlocksToMaturity() > 0 && IsInMainChain())
    {
        if (fUseCache && fImmatureWatchCreditCached)
            return nImmatureWatchCreditCached;
        nImmatureWatchCreditCached = pwallet->GetCredit(*this, ISMINE_WATCH_ONLY);
        fImmatureWatchCreditCached = true;
        return nImmatureWatchCreditCached;
    }

    return 0;
}

CellAmount CellWalletTx::GetAvailableWatchOnlyCredit(const bool& fUseCache) const
{
    if (pwallet == 0)
        return 0;

    // Must wait until coinbase is safely deep enough in the chain before valuing it
    if (IsCoinBase() && GetBlocksToMaturity() > 0)
        return 0;

    if (fUseCache && fAvailableWatchCreditCached)
        return nAvailableWatchCreditCached;

    CellAmount nCredit = 0;
    for (unsigned int i = 0; i < tx->vout.size(); i++)
    {
        if (!pwallet->IsSpent(GetHash(), i))
        {
            const CellTxOut &txout = tx->vout[i];
            nCredit += pwallet->GetCredit(txout, ISMINE_WATCH_ONLY);
            if (!MoneyRange(nCredit))
                throw std::runtime_error(std::string(__func__) + ": value out of range");
        }
    }

    nAvailableWatchCreditCached = nCredit;
    fAvailableWatchCreditCached = true;
    return nCredit;
}

CellAmount CellWalletTx::GetChange() const
{
    if (fChangeCached)
        return nChangeCached;
    nChangeCached = pwallet->GetChange(*this);
    fChangeCached = true;
    return nChangeCached;
}

bool CellWalletTx::InMempool() const
{
    LOCK(mempool.cs);
    return mempool.exists(GetHash());
}

bool CellWalletTx::IsTrusted() const
{
    // Quick answer in most cases
    if (!CheckFinalTx(*this))
        return false;
    int nDepth = GetDepthInMainChain();
    if (nDepth >= 1)
        return true;
    if (nDepth < 0)
        return false;
    if (!bSpendZeroConfChange || !IsFromMe(ISMINE_ALL)) // using wtx's cached debit
        return false;

    // Don't trust unconfirmed transactions from us unless they are in the mempool.
    if (!InMempool())
        return false;

    // Trusted if all inputs are from us and are in the mempool:
    for (const CellTxIn& txin : tx->vin)
    {
        // Transactions not sent by us: not trusted
        const CellWalletTx* parent = pwallet->GetWalletTx(txin.prevout.hash);
        if (parent == nullptr)
            return false;
        const CellTxOut& parentOut = parent->tx->vout[txin.prevout.n];
        if (pwallet->IsMine(parentOut) != ISMINE_SPENDABLE)
            return false;
    }
    return true;
}

bool CellWalletTx::IsEquivalentTo(const CellWalletTx& _tx) const
{
        CellMutableTransaction tx1 = *this->tx;
        CellMutableTransaction tx2 = *_tx.tx;
        for (auto& txin : tx1.vin) txin.scriptSig = CellScript();
        for (auto& txin : tx2.vin) txin.scriptSig = CellScript();
        return CellTransaction(tx1) == CellTransaction(tx2);
}

std::vector<uint256> CellWallet::ResendWalletTransactionsBefore(int64_t nTime, CellConnman* connman)
{
    std::vector<uint256> result;

    LOCK(cs_wallet);

    // Sort them in chronological order
    std::multimap<unsigned int, CellWalletTx*> mapSorted;
    for (std::pair<const uint256, CellWalletTx>& item : mapWallet)
    {
        CellWalletTx& wtx = item.second;
        // Don't rebroadcast if newer than nTime:
        if (wtx.nTimeReceived > nTime)
            continue;
        mapSorted.insert(std::make_pair(wtx.nTimeReceived, &wtx));
    }
    for (std::pair<const unsigned int, CellWalletTx*>& item : mapSorted)
    {
        CellWalletTx& wtx = *item.second;
        if (wtx.RelayWalletTransaction(connman))
            result.push_back(wtx.GetHash());
    }
    return result;
}

void CellWallet::ResendWalletTransactions(int64_t nBestBlockTime, CellConnman* connman)
{
    // Do this infrequently and randomly to avoid giving away
    // that these are our transactions.
    if (GetTime() < nNextResend || !fBroadcastTransactions)
        return;
    bool fFirst = (nNextResend == 0);
    nNextResend = GetTime() + GetRand(30 * 60);
    if (fFirst)
        return;

    // Only do it if there's been a new block since last time
    if (nBestBlockTime < nLastResend)
        return;
    nLastResend = GetTime();

    // Rebroadcast unconfirmed txes older than 5 minutes before the last
    // block was found:
    std::vector<uint256> relayed = ResendWalletTransactionsBefore(nBestBlockTime-5*60, connman);
    if (!relayed.empty())
        LogPrintf("%s: rebroadcast %u unconfirmed transactions\n", __func__, relayed.size());
}

/** @} */ // end of mapWallet




/** @defgroup Actions
 *
 * @{
 */


CellAmount CellWallet::GetBalance() const
{
    CellAmount nTotal = 0;
    {
        LOCK2(cs_main, cs_wallet);
        for (std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CellWalletTx* pcoin = &(*it).second;
            if (pcoin->IsTrusted())
                nTotal += pcoin->GetAvailableCredit();
        }
    }

    return nTotal;
}

CellAmount CellWallet::GetUnconfirmedBalance() const
{
    CellAmount nTotal = 0;
    {
        LOCK2(cs_main, cs_wallet);
        for (std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CellWalletTx* pcoin = &(*it).second;
            if (!pcoin->IsTrusted() && pcoin->GetDepthInMainChain() == 0 && pcoin->InMempool())
                nTotal += pcoin->GetAvailableCredit();
        }
    }
    return nTotal;
}

CellAmount CellWallet::GetImmatureBalance() const
{
    CellAmount nTotal = 0;
    {
        LOCK2(cs_main, cs_wallet);
        for (std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CellWalletTx* pcoin = &(*it).second;
            nTotal += pcoin->GetImmatureCredit();
        }
    }
    return nTotal;
}

CellAmount CellWallet::GetWatchOnlyBalance() const
{
    CellAmount nTotal = 0;
    {
        LOCK2(cs_main, cs_wallet);
        for (std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CellWalletTx* pcoin = &(*it).second;
            if (pcoin->IsTrusted())
                nTotal += pcoin->GetAvailableWatchOnlyCredit();
        }
    }

    return nTotal;
}

CellAmount CellWallet::GetUnconfirmedWatchOnlyBalance() const
{
    CellAmount nTotal = 0;
    {
        LOCK2(cs_main, cs_wallet);
        for (std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CellWalletTx* pcoin = &(*it).second;
            if (!pcoin->IsTrusted() && pcoin->GetDepthInMainChain() == 0 && pcoin->InMempool())
                nTotal += pcoin->GetAvailableWatchOnlyCredit();
        }
    }
    return nTotal;
}

CellAmount CellWallet::GetImmatureWatchOnlyBalance() const
{
    CellAmount nTotal = 0;
    {
        LOCK2(cs_main, cs_wallet);
        for (std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CellWalletTx* pcoin = &(*it).second;
            nTotal += pcoin->GetImmatureWatchOnlyCredit();
        }
    }
    return nTotal;
}

// Calculate total balance in a different way from GetBalance. The biggest
// difference is that GetBalance sums up all unspent TxOuts paying to the
// wallet, while this sums up both spent and unspent TxOuts paying to the
// wallet, and then subtracts the values of TxIns spending from the wallet. This
// also has fewer restrictions on which unconfirmed transactions are considered
// trusted.
CellAmount CellWallet::GetLegacyBalance(const isminefilter& filter, int minDepth, const std::string* account) const
{
    LOCK2(cs_main, cs_wallet);

    CellAmount balance = 0;
    for (const auto& entry : mapWallet) {
        const CellWalletTx& wtx = entry.second;
        const int depth = wtx.GetDepthInMainChain();
        if (depth < 0 || !CheckFinalTx(*wtx.tx) || wtx.GetBlocksToMaturity() > 0) {
            continue;
        }

        // Loop through tx outputs and add incoming payments. For outgoing txs,
        // treat change outputs specially, as part of the amount debited.
        CellAmount debit = wtx.GetDebit(filter);
        const bool outgoing = debit > 0;
        for (const CellTxOut& out : wtx.tx->vout) {
            if (outgoing && IsChange(out)) {
                debit -= out.nValue;
            } else if (IsMine(out) & filter && depth >= minDepth && (!account || *account == GetAccountName(out.scriptPubKey))) {
                balance += out.nValue;
            }
        }

        // For outgoing txs, subtract amount debited.
        if (outgoing && (!account || *account == wtx.strFromAccount)) {
            balance -= debit;
        }
    }

    if (account) {
        balance += CWalletDB(*dbw).GetAccountCreditDebit(*account);
    }

    return balance;
}

CellAmount CellWallet::GetAvailableBalance(const CellCoinControl* coinControl) const
{
    LOCK2(cs_main, cs_wallet);

    CellAmount balance = 0;
    std::vector<CellOutput> vCoins;
    AvailableCoins(vCoins, nullptr, true, coinControl);
    for (const CellOutput& out : vCoins) {
        if (out.fSpendable) {
            balance += out.tx->tx->vout[out.i].nValue;
        }
    }
    return balance;
}

void CellWallet::AvailableCoins(std::vector<CellOutput> &vCoins, const CellTxDestination* dest, bool fOnlySafe, const CellCoinControl *coinControl, const CellAmount &nMinimumAmount, const CellAmount &nMaximumAmount, const CellAmount &nMinimumSumAmount, const uint64_t &nMaximumCount, const int &nMinDepth, const int &nMaxDepth) const
{
    vCoins.clear();

    {
        LOCK2(cs_main, cs_wallet);

        CellAmount nTotal = 0;

        for (std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const uint256& wtxid = it->first;
            const CellWalletTx* pcoin = &(*it).second;

            if (!CheckFinalTx(*pcoin))
                continue;

            if (pcoin->IsCoinBase() && pcoin->GetBlocksToMaturity() > 0)
                continue;

            int nDepth = pcoin->GetDepthInMainChain();
            if (nDepth < 0)
                continue;

            // We should not consider coins which aren't at least in our mempool
            // It's possible for these to be conflicted via ancestors which we may never be able to detect
            if (nDepth == 0 && !pcoin->InMempool())
                continue;

            bool safeTx = pcoin->IsTrusted();

            // We should not consider coins from transactions that are replacing
            // other transactions.
            //
            // Example: There is a transaction A which is replaced by bumpfee
            // transaction B. In this case, we want to prevent creation of
            // a transaction B' which spends an output of B.
            //
            // Reason: If transaction A were initially confirmed, transactions B
            // and B' would no longer be valid, so the user would have to create
            // a new transaction C to replace B'. However, in the case of a
            // one-block reorg, transactions B' and C might BOTH be accepted,
            // when the user only wanted one of them. Specifically, there could
            // be a 1-block reorg away from the chain where transactions A and C
            // were accepted to another chain where B, B', and C were all
            // accepted.
            if (nDepth == 0 && pcoin->mapValue.count("replaces_txid")) {
                safeTx = false;
            }

            // Similarly, we should not consider coins from transactions that
            // have been replaced. In the example above, we would want to prevent
            // creation of a transaction A' spending an output of A, because if
            // transaction B were initially confirmed, conflicting with A and
            // A', we wouldn't want to the user to create a transaction D
            // intending to replace A', but potentially resulting in a scenario
            // where A, A', and D could all be accepted (instead of just B and
            // D, or just A and A' like the user would want).
            if (nDepth == 0 && pcoin->mapValue.count("replaced_by_txid")) {
                safeTx = false;
            }

            if (fOnlySafe && !safeTx) {
                continue;
            }

            if (nDepth < nMinDepth || nDepth > nMaxDepth)
                continue;

            for (unsigned int i = 0; i < pcoin->tx->vout.size(); i++) {
                if (pcoin->tx->vout[i].nValue < nMinimumAmount || pcoin->tx->vout[i].nValue > nMaximumAmount)
                    continue;

                if (coinControl && coinControl->HasSelected() && !coinControl->fAllowOtherInputs && !coinControl->IsSelected(CellOutPoint((*it).first, i)))
                    continue;

                if (IsLockedCoin((*it).first, i))
                    continue;

                if (IsSpent(wtxid, i))
                    continue;

                if (dest != nullptr) {
                    CellTxDestination dest_test;
                    ExtractDestination(pcoin->tx->vout[i].scriptPubKey, dest_test);
                    if (!(dest_test == *dest))
                        continue;
                }

                isminetype mine = IsMine(pcoin->tx->vout[i]);
                if (mine == ISMINE_NO) {
                    continue;
                }

                if (QuickGetBranchScriptType(pcoin->tx->vout[i].scriptPubKey) != BST_INVALID)//抵押币、挖矿币不能普通使用
                {
                    continue;
                }
                if (pcoin->tx->IsBranchCreate() && IsCoinCreateBranchScript(pcoin->tx->vout[i].scriptPubKey) 
                    && pcoin->GetBlocksToMaturityForCoinCreateBranch() > 0)// 支链创建抵押币需要满足一定高度才能使用
                {
                    continue;
                }

                bool fSpendableIn = ((mine & ISMINE_SPENDABLE) != ISMINE_NO) || (coinControl && coinControl->fAllowWatchOnly && (mine & ISMINE_WATCH_SOLVABLE) != ISMINE_NO);
                bool fSolvableIn = (mine & (ISMINE_SPENDABLE | ISMINE_WATCH_SOLVABLE)) != ISMINE_NO;

                vCoins.push_back(CellOutput(pcoin, i, nDepth, fSpendableIn, fSolvableIn, safeTx));

                // Checks the sum amount of all UTXO's.
                if (nMinimumSumAmount != MAX_MONEY) {
                    nTotal += pcoin->tx->vout[i].nValue;

                    if (nTotal >= nMinimumSumAmount) {
                        return;
                    }
                }

                // Checks the maximum number of UTXO's.
                if (nMaximumCount > 0 && vCoins.size() >= nMaximumCount) {
                    return;
                }
            }
        }
    }
}

void CellWallet::AvailableMortgageCoins(std::vector<CellOutput>& vCoins, bool fOnlySafe, branch_script_type bsptype, const CellCoinControl *coinControl, const CellAmount& nMinimumAmount, const CellAmount& nMaximumAmount, const CellAmount& nMinimumSumAmount, const uint64_t& nMaximumCount, const int& nMinDepth, const int& nMaxDepth)
{
    vCoins.clear();

    {
        LOCK2(cs_main, cs_wallet);

        CellAmount nTotal = 0;

        for (std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const uint256& wtxid = it->first;
            const CellWalletTx* pcoin = &(*it).second;

            if (!CheckFinalTx(*pcoin))
                continue;

            if (pcoin->IsCoinBase() && pcoin->GetBlocksToMaturity() > 0)
                continue;

            int nDepth = pcoin->GetDepthInMainChain();
            if (nDepth < 0)
                continue;

            // We should not consider coins which aren't at least in our mempool
            // It's possible for these to be conflicted via ancestors which we may never be able to detect
            if (nDepth == 0 && !pcoin->InMempool())
                continue;

            bool safeTx = pcoin->IsTrusted();

            // We should not consider coins from transactions that are replacing
            // other transactions.
            //
            // Example: There is a transaction A which is replaced by bumpfee
            // transaction B. In this case, we want to prevent creation of
            // a transaction B' which spends an output of B.
            //
            // Reason: If transaction A were initially confirmed, transactions B
            // and B' would no longer be valid, so the user would have to create
            // a new transaction C to replace B'. However, in the case of a
            // one-block reorg, transactions B' and C might BOTH be accepted,
            // when the user only wanted one of them. Specifically, there could
            // be a 1-block reorg away from the chain where transactions A and C
            // were accepted to another chain where B, B', and C were all
            // accepted.
            if (nDepth == 0 && pcoin->mapValue.count("replaces_txid")) {
                safeTx = false;
            }

            // Similarly, we should not consider coins from transactions that
            // have been replaced. In the example above, we would want to prevent
            // creation of a transaction A' spending an output of A, because if
            // transaction B were initially confirmed, conflicting with A and
            // A', we wouldn't want to the user to create a transaction D
            // intending to replace A', but potentially resulting in a scenario
            // where A, A', and D could all be accepted (instead of just B and
            // D, or just A and A' like the user would want).
            if (nDepth == 0 && pcoin->mapValue.count("replaced_by_txid")) {
                safeTx = false;
            }

            if (fOnlySafe && !safeTx) {
                continue;
            }

            if (nDepth < nMinDepth || nDepth > nMaxDepth)
                continue;

            for (unsigned int i = 0; i < pcoin->tx->vout.size(); i++) {
                if (pcoin->tx->vout[i].nValue < nMinimumAmount || pcoin->tx->vout[i].nValue > nMaximumAmount)
                    continue;

                if (coinControl && coinControl->HasSelected() && !coinControl->fAllowOtherInputs && !coinControl->IsSelected(CellOutPoint((*it).first, i)))
                    continue;

                if (IsLockedCoin((*it).first, i))
                    continue;

                if (IsSpent(wtxid, i))
                    continue;

                isminetype mine = IsMine(pcoin->tx->vout[i]);

                if (mine == ISMINE_NO) {
                    continue;
                }

                if (!(QuickGetBranchScriptType(pcoin->tx->vout[i].scriptPubKey) & bsptype))// == BST_INVALID ? 
                    continue;

                bool fSpendableIn = ((mine & ISMINE_SPENDABLE) != ISMINE_NO) || (coinControl && coinControl->fAllowWatchOnly && (mine & ISMINE_WATCH_SOLVABLE) != ISMINE_NO);
                bool fSolvableIn = (mine & (ISMINE_SPENDABLE | ISMINE_WATCH_SOLVABLE)) != ISMINE_NO;

                vCoins.push_back(CellOutput(pcoin, i, nDepth, fSpendableIn, fSolvableIn, safeTx));

                // Checks the sum amount of all UTXO's.
                if (nMinimumSumAmount != MAX_MONEY) {
                    nTotal += pcoin->tx->vout[i].nValue;

                    if (nTotal >= nMinimumSumAmount) {
                        return;
                    }
                }

                // Checks the maximum number of UTXO's.
                if (nMaximumCount > 0 && vCoins.size() >= nMaximumCount) {
                    return;
                }
            }
        }
    }
}

std::map<CellTxDestination, std::vector<CellOutput>> CellWallet::ListCoins() const
{
    // TODO: Add AssertLockHeld(cs_wallet) here.
    //
    // Because the return value from this function contains pointers to
    // CellWalletTx objects, callers to this function really should acquire the
    // cs_wallet lock before calling it. However, the current caller doesn't
    // acquire this lock yet. There was an attempt to add the missing lock in
    // https://github.com/bitcoin/bitcoin/pull/10340, but that change has been
    // postponed until after https://github.com/bitcoin/bitcoin/pull/10244 to
    // avoid adding some extra complexity to the Qt code.

    std::map<CellTxDestination, std::vector<CellOutput>> result;

    std::vector<CellOutput> availableCoins;
    AvailableCoins(availableCoins);

    LOCK2(cs_main, cs_wallet);
    for (auto& coin : availableCoins) {
        CellTxDestination address;
        if (coin.fSpendable &&
            ExtractDestination(FindNonChangeParentOutput(*coin.tx->tx, coin.i).scriptPubKey, address)) {
            result[address].emplace_back(std::move(coin));
        }
    }

    std::vector<CellOutPoint> lockedCoins;
    ListLockedCoins(lockedCoins);
    for (const auto& output : lockedCoins) {
        auto it = mapWallet.find(output.hash);
        if (it != mapWallet.end()) {
            int depth = it->second.GetDepthInMainChain();
            if (depth >= 0 && output.n < it->second.tx->vout.size() &&
                IsMine(it->second.tx->vout[output.n]) == ISMINE_SPENDABLE) {
                CellTxDestination address;
                if (ExtractDestination(FindNonChangeParentOutput(*it->second.tx, output.n).scriptPubKey, address)) {
                    result[address].emplace_back(
                        &it->second, output.n, depth, true /* spendable */, true /* solvable */, false /* safe */);
                }
            }
        }
    }

    return result;
}

const CellTxOut& CellWallet::FindNonChangeParentOutput(const CellTransaction& tx, int output) const
{
    const CellTransaction* ptx = &tx;
    int n = output;
    while (IsChange(ptx->vout[n]) && ptx->vin.size() > 0) {
        const CellOutPoint& prevout = ptx->vin[0].prevout;
        auto it = mapWallet.find(prevout.hash);
        if (it == mapWallet.end() || it->second.tx->vout.size() <= prevout.n ||
            !IsMine(it->second.tx->vout[prevout.n])) {
            break;
        }
        ptx = it->second.tx.get();
        n = prevout.n;
    }
    return ptx->vout[n];
}

static void ApproximateBestSubset(const std::vector<CellInputCoin>& vValue, const CellAmount& nTotalLower, const CellAmount& nTargetValue,
                                  std::vector<char>& vfBest, CellAmount& nBest, int iterations = 1000)
{
    std::vector<char> vfIncluded;

    vfBest.assign(vValue.size(), true);
    nBest = nTotalLower;

    FastRandomContext insecure_rand;

    for (int nRep = 0; nRep < iterations && nBest != nTargetValue; nRep++)
    {
        vfIncluded.assign(vValue.size(), false);
        CellAmount nTotal = 0;
        bool fReachedTarget = false;
        for (int nPass = 0; nPass < 2 && !fReachedTarget; nPass++)
        {
            for (unsigned int i = 0; i < vValue.size(); i++)
            {
                //The solver here uses a randomized algorithm,
                //the randomness serves no real security purpose but is just
                //needed to prevent degenerate behavior and it is important
                //that the rng is fast. We do not use a constant random sequence,
                //because there may be some privacy improvement by making
                //the selection random.
                if (nPass == 0 ? insecure_rand.randbool() : !vfIncluded[i])
                {
                    nTotal += vValue[i].txout.nValue;
                    vfIncluded[i] = true;
                    if (nTotal >= nTargetValue)
                    {
                        fReachedTarget = true;
                        if (nTotal < nBest)
                        {
                            nBest = nTotal;
                            vfBest = vfIncluded;
                        }
                        nTotal -= vValue[i].txout.nValue;
                        vfIncluded[i] = false;
                    }
                }
            }
        }
    }
}

bool CellWallet::SelectCoinsMinConf(const CellAmount& nTargetValue, const int nConfMine, const int nConfTheirs, const uint64_t nMaxAncestors, std::vector<CellOutput> vCoins,
                                 std::set<CellInputCoin>& setCoinsRet, CellAmount& nValueRet) const
{
    setCoinsRet.clear();
    nValueRet = 0;

    // List of values less than target
    boost::optional<CellInputCoin> coinLowestLarger;
    std::vector<CellInputCoin> vValue;
    CellAmount nTotalLower = 0;

    random_shuffle(vCoins.begin(), vCoins.end(), GetRandInt);

    for (const CellOutput &output : vCoins)
    {
        if (!output.fSpendable)
            continue;

        const CellWalletTx *pcoin = output.tx;

        if (output.nDepth < (pcoin->IsFromMe(ISMINE_ALL) ? nConfMine : nConfTheirs))
            continue;

        if (!mempool.TransactionWithinChainLimit(pcoin->GetHash(), nMaxAncestors))
            continue;

        int i = output.i;

        CellInputCoin coin = CellInputCoin(pcoin, i);

        if (coin.txout.nValue == nTargetValue)
        {
            setCoinsRet.insert(coin);
            nValueRet += coin.txout.nValue;
            return true;
        }
        else if (coin.txout.nValue < nTargetValue + MIN_CHANGE)
        {
            vValue.push_back(coin);
            nTotalLower += coin.txout.nValue;
        }
        else if (!coinLowestLarger || coin.txout.nValue < coinLowestLarger->txout.nValue)
        {
            coinLowestLarger = coin;
        }
    }

    if (nTotalLower == nTargetValue)
    {
        for (const auto& input : vValue)
        {
            setCoinsRet.insert(input);
            nValueRet += input.txout.nValue;
        }
        return true;
    }

    if (nTotalLower < nTargetValue)
    {
        if (!coinLowestLarger)
            return false;
        setCoinsRet.insert(coinLowestLarger.get());
        nValueRet += coinLowestLarger->txout.nValue;
        return true;
    }

    // Solve subset sum by stochastic approximation
    std::sort(vValue.begin(), vValue.end(), CompareValueOnly());
    std::reverse(vValue.begin(), vValue.end());
    std::vector<char> vfBest;
    CellAmount nBest;

    ApproximateBestSubset(vValue, nTotalLower, nTargetValue, vfBest, nBest);
    if (nBest != nTargetValue && nTotalLower >= nTargetValue + MIN_CHANGE)
        ApproximateBestSubset(vValue, nTotalLower, nTargetValue + MIN_CHANGE, vfBest, nBest);

    // If we have a bigger coin and (either the stochastic approximation didn't find a good solution,
    //                                   or the next bigger coin is closer), return the bigger coin
    if (coinLowestLarger &&
        ((nBest != nTargetValue && nBest < nTargetValue + MIN_CHANGE) || coinLowestLarger->txout.nValue <= nBest))
    {
        setCoinsRet.insert(coinLowestLarger.get());
        nValueRet += coinLowestLarger->txout.nValue;
    }
    else {
        for (unsigned int i = 0; i < vValue.size(); i++)
            if (vfBest[i])
            {
                setCoinsRet.insert(vValue[i]);
                nValueRet += vValue[i].txout.nValue;
            }

        if (LogAcceptCategory(BCLog::SELECTCOINS)) {
            LogPrint(BCLog::SELECTCOINS, "SelectCoins() best subset: ");
            for (unsigned int i = 0; i < vValue.size(); i++) {
                if (vfBest[i]) {
                    LogPrint(BCLog::SELECTCOINS, "%s ", FormatMoney(vValue[i].txout.nValue));
                }
            }
            LogPrint(BCLog::SELECTCOINS, "total %s\n", FormatMoney(nBest));
        }
    }

    return true;
}

bool CellWallet::SelectCoins(const std::vector<CellOutput>& vAvailableCoins, const CellAmount& nTargetValue, std::set<CellInputCoin>& setCoinsRet, CellAmount& nValueRet, const CellCoinControl* coinControl) const
{
    std::vector<CellOutput> vCoins(vAvailableCoins);

    // coin control -> return all selected outputs (we want all selected to go into the transaction for sure)
    if (coinControl && coinControl->HasSelected() && !coinControl->fAllowOtherInputs)
    {
        for (const CellOutput& out : vCoins)
        {
            if (!out.fSpendable)
                 continue;
            nValueRet += out.tx->tx->vout[out.i].nValue;
            setCoinsRet.insert(CellInputCoin(out.tx, out.i));
        }
        return (nValueRet >= nTargetValue);
    }

    // calculate value from preset inputs and store them
    std::set<CellInputCoin> setPresetCoins;
    CellAmount nValueFromPresetInputs = 0;

    std::vector<CellOutPoint> vPresetInputs;
    if (coinControl)
        coinControl->ListSelected(vPresetInputs);
    for (const CellOutPoint& outpoint : vPresetInputs)
    {
        std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.find(outpoint.hash);
        if (it != mapWallet.end())
        {
            const CellWalletTx* pcoin = &it->second;
            // Clearly invalid input, fail
            if (pcoin->tx->vout.size() <= outpoint.n)
                return false;
            nValueFromPresetInputs += pcoin->tx->vout[outpoint.n].nValue;
            setPresetCoins.insert(CellInputCoin(pcoin, outpoint.n));
        } else
            return false; // TODO: Allow non-wallet inputs
    }

    // remove preset inputs from vCoins
    for (std::vector<CellOutput>::iterator it = vCoins.begin(); it != vCoins.end() && coinControl && coinControl->HasSelected();)
    {
        if (setPresetCoins.count(CellInputCoin(it->tx, it->i)))
            it = vCoins.erase(it);
        else
            ++it;
    }

    size_t nMaxChainLength = std::min(gArgs.GetArg("-limitancestorcount", DEFAULT_ANCESTOR_LIMIT), gArgs.GetArg("-limitdescendantcount", DEFAULT_DESCENDANT_LIMIT));
    bool fRejectLongChains = gArgs.GetBoolArg("-walletrejectlongchains", DEFAULT_WALLET_REJECT_LONG_CHAINS);

    bool res = nTargetValue <= nValueFromPresetInputs ||
        SelectCoinsMinConf(nTargetValue - nValueFromPresetInputs, 1, 6, 0, vCoins, setCoinsRet, nValueRet) ||
        SelectCoinsMinConf(nTargetValue - nValueFromPresetInputs, 1, 1, 0, vCoins, setCoinsRet, nValueRet) ||
        (bSpendZeroConfChange && SelectCoinsMinConf(nTargetValue - nValueFromPresetInputs, 0, 1, 2, vCoins, setCoinsRet, nValueRet)) ||
        (bSpendZeroConfChange && SelectCoinsMinConf(nTargetValue - nValueFromPresetInputs, 0, 1, std::min((size_t)4, nMaxChainLength/3), vCoins, setCoinsRet, nValueRet)) ||
        (bSpendZeroConfChange && SelectCoinsMinConf(nTargetValue - nValueFromPresetInputs, 0, 1, nMaxChainLength/2, vCoins, setCoinsRet, nValueRet)) ||
        (bSpendZeroConfChange && SelectCoinsMinConf(nTargetValue - nValueFromPresetInputs, 0, 1, nMaxChainLength, vCoins, setCoinsRet, nValueRet)) ||
        (bSpendZeroConfChange && !fRejectLongChains && SelectCoinsMinConf(nTargetValue - nValueFromPresetInputs, 0, 1, std::numeric_limits<uint64_t>::max(), vCoins, setCoinsRet, nValueRet));

    // because SelectCoinsMinConf clears the setCoinsRet, we now add the possible inputs to the coinset
    setCoinsRet.insert(setPresetCoins.begin(), setPresetCoins.end());

    // add preset inputs to the total value selected
    nValueRet += nValueFromPresetInputs;

    return res;
}

bool CellWallet::SignTransaction(CellMutableTransaction &tx)
{
    AssertLockHeld(cs_wallet); // mapWallet

    // sign the new tx
    CellTransaction txNewConst(tx);
    int nIn = 0;
    for (const auto& input : tx.vin) {
        std::map<uint256, CellWalletTx>::const_iterator mi = mapWallet.find(input.prevout.hash);
        if(mi == mapWallet.end() || input.prevout.n >= mi->second.tx->vout.size()) {
            return false;
        }
        const CellScript& scriptPubKey = mi->second.tx->vout[input.prevout.n].scriptPubKey;
        const CellAmount& amount = mi->second.tx->vout[input.prevout.n].nValue;
        SignatureData sigdata;
        if (!ProduceSignature(TransactionSignatureCreator(this, &txNewConst, nIn, amount, SIGHASH_ALL), scriptPubKey, sigdata)) {
            return false;
        }
        UpdateTransaction(tx, nIn, sigdata);
        nIn++;
    }
    return true;
}

bool CellWallet::FundTransaction(CellMutableTransaction& tx, CellAmount& nFeeRet, int& nChangePosInOut, std::string& strFailReason, bool lockUnspents, const std::set<int>& setSubtractFeeFromOutputs, CellCoinControl coinControl)
{
    std::vector<CellRecipient> vecSend;

    // Turn the txout set into a CellRecipient vector
    for (size_t idx = 0; idx < tx.vout.size(); idx++)
    {
        const CellTxOut& txOut = tx.vout[idx];
        CellRecipient recipient = {txOut.scriptPubKey, txOut.nValue, setSubtractFeeFromOutputs.count(idx) == 1};
        vecSend.push_back(recipient);
    }

    coinControl.fAllowOtherInputs = true;

    for (const CellTxIn& txin : tx.vin)
        coinControl.Select(txin.prevout);

    CellReserveKey reservekey(this);
    CellWalletTx wtx;
    if (!CreateTransaction(vecSend, wtx, reservekey, nFeeRet, nChangePosInOut, strFailReason, coinControl, false)) {
        return false;
    }

    if (nChangePosInOut != -1) {
        tx.vout.insert(tx.vout.begin() + nChangePosInOut, wtx.tx->vout[nChangePosInOut]);
        // we dont have the normal Create/Commit cycle, and dont want to risk reusing change,
        // so just remove the key from the keypool here.
        reservekey.KeepKey();
    }

    // Copy output sizes from new transaction; they may have had the fee subtracted from them
    for (unsigned int idx = 0; idx < tx.vout.size(); idx++)
        tx.vout[idx].nValue = wtx.tx->vout[idx].nValue;

    // Add new txins (keeping original txin scriptSig/order)
    for (const CellTxIn& txin : wtx.tx->vin)
    {
        if (!coinControl.IsSelected(txin.prevout))
        {
            tx.vin.push_back(txin);

            if (lockUnspents)
            {
              LOCK2(cs_main, cs_wallet);
              LockCoin(txin.prevout);
            }
        }
    }


    return true;
}

CellFeeRate GetDiscardRate(const CellBlockPolicyEstimator& estimator)
{
    unsigned int highest_target = estimator.HighestTargetTracked(FeeEstimateHorizon::LONG_HALFLIFE);
    CellFeeRate discard_rate = estimator.estimateSmartFee(highest_target, nullptr /* FeeCalculation */, false /* conservative */);
    // Don't let discard_rate be greater than longest possible fee estimate if we get a valid fee estimate
    discard_rate = (discard_rate == CellFeeRate(0)) ? CellWallet::m_discard_rate : std::min(discard_rate, CellWallet::m_discard_rate);
    // Discard rate must be at least dustRelayFee
    discard_rate = std::max(discard_rate, ::dustRelayFee);
    return discard_rate;
}

static bool MoveTransactionData(CellWalletTx& fromWtx, CellMutableTransaction& toTx)
{
    if (fromWtx.transaction_version > CellTransaction::CURRENT_VERSION)
    {
        toTx.nVersion = fromWtx.transaction_version;
    }
    if (fromWtx.transaction_version == CellTransaction::PUBLISH_CONTRACT_VERSION)
    {
        toTx.nVersion = CellTransaction::PUBLISH_CONTRACT_VERSION;
        toTx.contractCode = fromWtx.contractCode;
        toTx.contractSender = fromWtx.contractSender;
        toTx.contractAddr = fromWtx.contractAddr;
        toTx.contractOut = fromWtx.contractOut;
    }
    else if (fromWtx.transaction_version == CellTransaction::CALL_CONTRACT_VERSION)
    {
        toTx.nVersion = CellTransaction::CALL_CONTRACT_VERSION;
        toTx.contractFun = fromWtx.contractCode;
        toTx.contractSender = fromWtx.contractSender;
        toTx.contractParams = fromWtx.contractParams;
        toTx.contractAddr = fromWtx.contractAddr;
        toTx.contractOut = fromWtx.contractOut;
    }
    if (fromWtx.transaction_version == CellTransaction::CREATE_BRANCH_VERSION)
    {
        toTx.branchVSeeds = fromWtx.branchVSeeds;
        toTx.branchSeedSpec6 = fromWtx.branchSeedSpec6;
    }
    else if (fromWtx.transaction_version == CellTransaction::TRANS_BRANCH_VERSION_S1)
    {
        toTx.sendToBranchid = fromWtx.sendToBranchid;
        toTx.sendToTxHexData = fromWtx.sendToTxHexData;
        if (toTx.sendToBranchid == CellBaseChainParams::MAIN)
            toTx.pPMT.reset(new CellSpvProof());
    }
    else if (fromWtx.transaction_version == CellTransaction::TRANS_BRANCH_VERSION_S2)//这种类型不是创建出来的，是包含在上面两个类型
    {
    }
    else if (fromWtx.transaction_version == CellTransaction::MINE_BRANCH_MORTGAGE)
    {
        toTx.sendToBranchid = fromWtx.sendToBranchid;
        toTx.sendToTxHexData = fromWtx.sendToTxHexData;
    }
    else if (fromWtx.transaction_version == CellTransaction::REDEEM_MORTGAGE)
    {
        toTx.fromBranchId = fromWtx.fromBranchId;
        toTx.pPMT.reset(new CellSpvProof(*fromWtx.pPMT));
        toTx.fromTx = std::move(fromWtx.fromTx);
    }
    else if (fromWtx.transaction_version == CellTransaction::SYNC_BRANCH_INFO)
    {
        toTx.pBranchBlockData = std::move(fromWtx.pBranchBlockData);
    }
    else if (fromWtx.transaction_version == CellTransaction::REPORT_CHEAT)
    {
        toTx.pPMT.reset(new CellSpvProof(*fromWtx.pPMT));
        toTx.pReportData.reset(new ReportData(*fromWtx.pReportData));
    }
    else if (fromWtx.transaction_version == CellTransaction::PROVE)
    {
        toTx.pProveData.reset(new ProveData(*fromWtx.pProveData));
    }
    else if (fromWtx.transaction_version == CellTransaction::LOCK_MORTGAGE_MINE_COIN)
    {
        toTx.reporttxid = fromWtx.reporttxid;
        toTx.coinpreouthash = fromWtx.coinpreouthash;
    }
    else if (fromWtx.transaction_version == CellTransaction::UNLOCK_MORTGAGE_MINE_COIN)
    {
        toTx.reporttxid = fromWtx.reporttxid;
        toTx.coinpreouthash = fromWtx.coinpreouthash;
        toTx.provetxid = fromWtx.provetxid;
    }
    //fromWtx.ClearTempContractData();
    return true;
}

// 交易至少要有一个输出,当isDataTransaction=true的情况下,至少有个找零输出 
bool CellWallet::CreateTransaction(const std::vector<CellRecipient>& vecSend, CellWalletTx& wtxNew, CellReserveKey& reservekey, CellAmount& nFeeRet,
                                int& nChangePosInOut, std::string& strFailReason, const CellCoinControl& coin_control, bool sign, SmartLuaState* sls)
{
    CellAmount nValue = 0;
    int nChangePosRequest = nChangePosInOut;
    unsigned int nSubtractFeeFromAmount = 0;
    for (const auto& recipient : vecSend)
    {
        if (nValue < 0 || recipient.nAmount < 0)
        {
            strFailReason = _("Transaction amounts must not be negative");
            return false;
        }
        nValue += recipient.nAmount;

        if (recipient.fSubtractFeeFromAmount)
            nSubtractFeeFromAmount++;
    }
    if (vecSend.empty() && !wtxNew.isDataTransaction && !wtxNew.IsSmartContract())
    {
        strFailReason = _("Transaction must have at least one recipient");
        return false;
    }

    wtxNew.fTimeReceivedIsTxTime = true;
    wtxNew.BindWallet(this);
    CellMutableTransaction txNew;
	//other transaction data
	MoveTransactionData(wtxNew, txNew);

    // Discourage fee sniping.
    //
    // For a large miner the value of the transactions in the best block and
    // the mempool can exceed the cost of deliberately attempting to mine two
    // blocks to orphan the current best block. By setting nLockTime such that
    // only the next block can include the transaction, we discourage this
    // practice as the height restricted and limited blocksize gives miners
    // considering fee sniping fewer options for pulling off this attack.
    //
    // A simple way to think about this is from the wallet's point of view we
    // always want the blockchain to move forward. By setting nLockTime this
    // way we're basically making the statement that we only want this
    // transaction to appear in the next block; we don't want to potentially
    // encourage reorgs by allowing transactions to appear at lower heights
    // than the next block in forks of the best chain.
    //
    // Of course, the subsidy is high enough, and transaction volume low
    // enough, that fee sniping isn't a problem yet, but by implementing a fix
    // now we ensure code won't be written that makes assumptions about
    // nLockTime that preclude a fix later.
    txNew.nLockTime = chainActive.Height();

    // Secondly occasionally randomly pick a nLockTime even further back, so
    // that transactions that are delayed after signing for whatever reason,
    // e.g. high-latency mix networks and some CoinJoin implementations, have
    // better privacy.
    if (GetRandInt(10) == 0)
        txNew.nLockTime = std::max(0, (int)txNew.nLockTime - GetRandInt(100));

    assert(txNew.nLockTime <= (unsigned int)chainActive.Height());
    assert(txNew.nLockTime < LOCKTIME_THRESHOLD);
    FeeCalculation feeCalc;
    CellAmount nFeeNeeded = 0;
    unsigned int nBytes;
    {
        std::set<CellInputCoin> setCoins;
        LOCK2(cs_main, cs_wallet);
        {
            std::vector<CellOutput> vAvailableCoins;
            if (!gArgs.GetBoolArg("-quickmode", false)){
                AvailableCoins(vAvailableCoins, nullptr, true, &coin_control);
            }
            else {//找到足够的币提前退出
                CellAmount nMinimumAmount = 1;
                CellAmount nMaximumAmount = MAX_MONEY;
                CellAmount nMinimumSumAmount = nValue * 2 + (nFeeNeeded ? nFeeNeeded : COIN);
                AvailableCoins(vAvailableCoins, nullptr, true, &coin_control, nMinimumAmount, nMaximumAmount, nMinimumSumAmount);
            }

            // Create change script that will be used if we need change
            // TODO: pass in scriptChange instead of reservekey so
            // change transaction isn't always pay-to-celllink-address
            CellScript scriptChange;

			// 获取一个找零地址
            // coin control: send change to custom address
            if (!boost::get<CellNoDestination>(&coin_control.destChange)) {
                scriptChange = GetScriptForDestination(coin_control.destChange);
            } else { // no coin control: send change to newly generated address
                // Note: We use a new key here to keep it from being obvious which side is the change.
                //  The drawback is that by not reusing a previous key, the change may be lost if a
                //  backup is restored, if the backup doesn't have the new private key for the change.
                //  If we reused the old key, it would be possible to add code to look for and
                //  rediscover unknown transactions that were written with keys of ours to recover
                //  post-backup change.

                // Reserve a new key pair from key pool
                CellPubKey vchPubKey;
                bool ret;
                ret = reservekey.GetReservedKey(vchPubKey, true);
                if (!ret)
                {
                    strFailReason = _("Keypool ran out, please call keypoolrefill first");
                    return false;
                }

                scriptChange = GetScriptForDestination(vchPubKey.GetID());
            }
            CellTxOut change_prototype_txout(0, scriptChange);
            size_t change_prototype_size = GetSerializeSize(change_prototype_txout, SER_DISK, 0);

            CellFeeRate discard_rate = GetDiscardRate(::feeEstimator);
            nFeeRet = 0;
            bool pick_new_inputs = true;
            CellAmount nValueIn = 0;
            // Start with no fee and loop until there is enough fee
            while (true)
            {
                nChangePosInOut = nChangePosRequest;
                txNew.vin.clear();
                txNew.vout.clear();
                wtxNew.fFromMe = true;
                bool fFirst = true;

				// 如果是从输入中扣除交易费，则输入要加上交易费用
                CellAmount nValueToSelect = nValue;
                if (nSubtractFeeFromAmount == 0)
                    nValueToSelect += nFeeRet;

                // vouts to the payees
                for (const auto& recipient : vecSend)
                {
                    CellTxOut txout(recipient.nAmount, recipient.scriptPubKey );

					// 从带有"从帐户扣除交易费"标记的输出交易中平均扣除交易费，第一个帐户还要扣除平均交易费后的余数
                    if (recipient.fSubtractFeeFromAmount)
                    {
                        txout.nValue -= nFeeRet / nSubtractFeeFromAmount; // Subtract fee equally from each selected recipient

                        if (fFirst) // first receiver pays the remainder not divisible by output count
                        {
                            fFirst = false;
                            txout.nValue -= nFeeRet % nSubtractFeeFromAmount;
                        }
                    }

					//如果是微额交易，返回错误信息
                    if (IsDust(txout, ::dustRelayFee))
                    {
                        if (recipient.fSubtractFeeFromAmount && nFeeRet > 0)
                        {
                            if (txout.nValue < 0)
                                strFailReason = _("The transaction amount is too small to pay the fee");
                            else
                                strFailReason = _("The transaction amount is too small to send after the fee has been deducted");
                        }
                        else
                            strFailReason = _("Transaction amount too small");
                        return false;
                    }
                    txNew.vout.push_back(txout);
                }

                // Choose coins to use
                if (pick_new_inputs) {
                    nValueIn = 0;
                    setCoins.clear();
                    if (!SelectCoins(vAvailableCoins, nValueToSelect, setCoins, nValueIn, &coin_control))
                    {
                        strFailReason = _("Insufficient funds");
                        return false;
                    }
                }

				// 求找零数额 
                const CellAmount nChange = nValueIn - nValueToSelect;
                if (nChange > 0)
                {
                    // Fill a vout to ourself
                    CellTxOut newTxOut(nChange, scriptChange);

                    // Never create dust outputs; if we would, just
                    // add the dust to the fee.
                    if (IsDust(newTxOut, discard_rate))
                    {
						// 如果是微额交易，则移除找零，同时将找零费用加到交易费用里 
                        nChangePosInOut = -1;
                        nFeeRet += nChange;
                    }
                    else
                    {
						// 在随机位置插入找零输出
                        if (nChangePosInOut == -1)
                        {
                            // Insert change txn at random position:
                            nChangePosInOut = GetRandInt(txNew.vout.size()+1);
                        }
                        else if ((unsigned int)nChangePosInOut > txNew.vout.size())
                        {
                            strFailReason = _("Change index out of range");
                            return false;
                        }

                        std::vector<CellTxOut>::iterator position = txNew.vout.begin()+nChangePosInOut;
                        txNew.vout.insert(position, newTxOut);
                    }
                } else {
                    nChangePosInOut = -1;
                }

                // Fill vin
                //
                // Note how the sequence number is set to non-maxint so that
                // the nLockTime set above actually works.
                //
                // BIP125 defines opt-in RBF as any nSequence < maxint-1, so
                // we use the highest possible value in that range (maxint-2)
                // to avoid conflicting with other possible uses of nSequence,
                // and in the spirit of "smallest possible change from prior
                // behavior."
                const uint32_t nSequence = coin_control.signalRbf ? MAX_BIP125_RBF_SEQUENCE : (CellTxIn::SEQUENCE_FINAL - 1);
                for (const auto& coin : setCoins)
                    txNew.vin.push_back(CellTxIn(coin.outpoint,CellScript(),
                                              nSequence));

				// 虚拟签名 
                // Fill in dummy signatures for fee calculation.
                if (!DummySignTx(txNew, setCoins)) {
                    strFailReason = _("Signing transaction failed");
                    return false;
                }

                if (sls != nullptr && sls->recipients.size() > 0) {
                    txNew.vin.emplace_back(uint256(), txNew.vin.size());
                    txNew.vout.emplace_back(txNew.contractOut, GetScriptForDestination(txNew.contractAddr));
                }

				// 获取交易字节大小 
                nBytes = GetVirtualTransactionSize(txNew, 0, sls);

                if (sls != nullptr && sls->recipients.size() > 0) {
                    txNew.vin.resize(txNew.vin.size() - 1);
                    txNew.vout.resize(txNew.vout.size() - 1);
                }

				// 移除虚拟签名数据
                // Remove scriptSigs to eliminate the fee calculation dummy signatures
                for (auto& vin : txNew.vin) {
                    vin.scriptSig = CellScript();
                    vin.scriptWitness.SetNull();
                }

                // check lsdata
                if (sls != nullptr) {
                    for (size_t i = 0; i < sls->recipients.size(); ++i)
                        txNew.vout.push_back(sls->recipients[i]);
                    txNew.contractAddr = wtxNew.contractAddr;
                }

                nFeeNeeded = GetMinimumFee(nBytes, coin_control, ::mempool, ::feeEstimator, &feeCalc, &txNew, sls);
				
                // If we made it here and we aren't even able to meet the relay fee on the next pass, give up
                // because we must be at the maximum allowed fee.
                if (nFeeNeeded < ::minRelayTxFee.GetFee(nBytes))
                {
                    strFailReason = _("Transaction too large for fee policy");
                    return false;
                }

                if (nFeeRet >= nFeeNeeded) {
					// 预测的费用大于实际需要的费用时 
                    // Reduce fee to only the needed amount if possible. This
                    // prevents potential overpayment in fees if the coins
                    // selected to meet nFeeNeeded result in a transaction that
                    // requires less fee than the prior iteration.

                    // If we have no change and a big enough excess fee, then
                    // try to construct transaction again only without picking
                    // new inputs. We now know we only need the smaller fee
                    // (because of reduced tx size) and so we should add a
                    // change output. Only try this once.
                    if (nChangePosInOut == -1 && nSubtractFeeFromAmount == 0 && pick_new_inputs) {
						// 没有找零输出，则创建找零并计算相关费用 
                        unsigned int tx_size_with_change = nBytes + change_prototype_size + 2; // Add 2 as a buffer in case increasing # of outputs changes compact size
                        CellAmount fee_needed_with_change = GetMinimumFee(tx_size_with_change, coin_control, ::mempool, ::feeEstimator, nullptr, &txNew, sls);
                        CellAmount minimum_value_for_change = GetDustThreshold(change_prototype_txout, discard_rate);
                        if (nFeeRet >= fee_needed_with_change + minimum_value_for_change) {
                            pick_new_inputs = false;
                            nFeeRet = fee_needed_with_change;
                            continue;
                        }
                    }

                    // If we have change output already, just increase it
                    if (nFeeRet > nFeeNeeded && nChangePosInOut != -1 && nSubtractFeeFromAmount == 0) {
                        CellAmount extraFeePaid = nFeeRet - nFeeNeeded;
                        std::vector<CellTxOut>::iterator change_position = txNew.vout.begin()+nChangePosInOut;
                        change_position->nValue += extraFeePaid;
                        nFeeRet -= extraFeePaid;
                    }
                    break; // Done, enough fee included.
                }
                else if (!pick_new_inputs) {
                    // This shouldn't happen, we should have had enough excess
                    // fee to pay for the new output and still meet nFeeNeeded
                    // Or we should have just subtracted fee from recipients and
                    // nFeeNeeded should not have changed
                    strFailReason = _("Transaction fee and change calculation failed");
                    return false;
                }

                // Try to reduce change to include necessary fee
                if (nChangePosInOut != -1 && nSubtractFeeFromAmount == 0) {
                    CellAmount additionalFeeNeeded = nFeeNeeded - nFeeRet;
                    std::vector<CellTxOut>::iterator change_position = txNew.vout.begin()+nChangePosInOut;
                    // Only reduce change if remaining amount is still a large enough output.
                    if (change_position->nValue >= MIN_FINAL_CHANGE + additionalFeeNeeded) {
                        change_position->nValue -= additionalFeeNeeded;
                        nFeeRet += additionalFeeNeeded;
                        break; // Done, able to increase fee from change
                    }
                }

                // If subtracting fee from recipients, we now know what fee we
                // need to subtract, we have no reason to reselect inputs
                if (nSubtractFeeFromAmount > 0) {
                    pick_new_inputs = false;
                }

                // Include more fee and try again.
                nFeeRet = nFeeNeeded;
                continue;
            }
        }

        if (nChangePosInOut == -1) reservekey.ReturnKey(); // Return any reserved key if we don't have change


		//generate contract address
		if (txNew.nVersion == CellTransaction::PUBLISH_CONTRACT_VERSION)
		{
			//replace
			CellContractID oldKey = txNew.contractAddr;
            CellScript oldScript = GetScriptForDestination(CellLinkAddress(oldKey).Get());

			txNew.contractAddr = GenerateContractAddressByTx(txNew);
			//replace vout
            CellScript newScript = GetScriptForDestination(CellLinkAddress(txNew.contractAddr).Get());
			for (auto out : txNew.vout)
			{
				if (out.scriptPubKey == oldScript)
					out.scriptPubKey = newScript;
			}
		}

        if (sign)
        {
            CellTransaction txNewConst(txNew);
            int nIn = 0;
            for (const auto& coin : setCoins)
            {
                const CellScript& scriptPubKey = coin.txout.scriptPubKey;
                SignatureData sigdata;

                if (!ProduceSignature(TransactionSignatureCreator(this, &txNewConst, nIn, coin.txout.nValue, SIGHASH_ALL), scriptPubKey, sigdata))
                {
                    strFailReason = _("Signing transaction failed");
                    return false;
                } else {
                    UpdateTransaction(txNew, nIn, sigdata);
                }

                nIn++;
            }
			// sign with contractSender addr's private key.
			if (txNew.IsSmartContract())
			{
				CellTransaction txNewConst(txNew);
				CellScript constractSig;
				if (!SignContract(this, &txNewConst, constractSig))
				{
					strFailReason = _("Signing contract failed");
					return false;
				}
				else {
					txNew.contractScriptSig = constractSig;
				}
			}
        }

        // Embed the constructed transaction data in wtxNew.
        wtxNew.SetTx(MakeTransactionRef(std::move(txNew)));

        // Limit size
        int64_t txWeight = GetTransactionWeight(wtxNew);
        if (txWeight >= MAX_STANDARD_TX_WEIGHT)
        {
            if (!txNew.IsProve()) {
                strFailReason = strprintf("Transaction too large, weight is %lld", txWeight);
                return false;
            }
        }
    }

    if (gArgs.GetBoolArg("-walletrejectlongchains", DEFAULT_WALLET_REJECT_LONG_CHAINS)) {
        // Lastly, ensure this tx will pass the mempool's chain limits
        LockPoints lp;
        CellTxMemPoolEntry entry(wtxNew.tx, 0, 0, 0, false, 0, lp);
        CellTxMemPool::setEntries setAncestors;
        size_t nLimitAncestors = gArgs.GetArg("-limitancestorcount", DEFAULT_ANCESTOR_LIMIT);
        size_t nLimitAncestorSize = gArgs.GetArg("-limitancestorsize", DEFAULT_ANCESTOR_SIZE_LIMIT)*1000;
        size_t nLimitDescendants = gArgs.GetArg("-limitdescendantcount", DEFAULT_DESCENDANT_LIMIT);
        size_t nLimitDescendantSize = gArgs.GetArg("-limitdescendantsize", DEFAULT_DESCENDANT_SIZE_LIMIT)*1000;
        std::string errString;
        if (!mempool.CalculateMemPoolAncestors(entry, setAncestors, nLimitAncestors, nLimitAncestorSize, nLimitDescendants, nLimitDescendantSize, errString)) {
            strFailReason = _("Transaction has too long of a mempool chain");
            return false;
        }
    }

    LogPrintf("Fee Calculation: Fee:%d Bytes:%u Needed:%d Tgt:%d (requested %d) Reason:\"%s\" Decay %.5f: Estimation: (%g - %g) %.2f%% %.1f/(%.1f %d mem %.1f out) Fail: (%g - %g) %.2f%% %.1f/(%.1f %d mem %.1f out)\n",
              nFeeRet, nBytes, nFeeNeeded, feeCalc.returnedTarget, feeCalc.desiredTarget, StringForFeeReason(feeCalc.reason), feeCalc.est.decay,
              feeCalc.est.pass.start, feeCalc.est.pass.end,
              100 * feeCalc.est.pass.withinTarget / (feeCalc.est.pass.totalConfirmed + feeCalc.est.pass.inMempool + feeCalc.est.pass.leftMempool),
              feeCalc.est.pass.withinTarget, feeCalc.est.pass.totalConfirmed, feeCalc.est.pass.inMempool, feeCalc.est.pass.leftMempool,
              feeCalc.est.fail.start, feeCalc.est.fail.end,
              100 * feeCalc.est.fail.withinTarget / (feeCalc.est.fail.totalConfirmed + feeCalc.est.fail.inMempool + feeCalc.est.fail.leftMempool),
              feeCalc.est.fail.withinTarget, feeCalc.est.fail.totalConfirmed, feeCalc.est.fail.inMempool, feeCalc.est.fail.leftMempool);
    return true;
}

/**
 * Call after CreateTransaction unless you want to abort
 */
bool CellWallet::CommitTransaction(CellWalletTx& wtxNew, CellReserveKey& reservekey, CellConnman* connman, CellValidationState& state)
{
    {
        LOCK2(cs_main, cs_wallet);
        LogPrintf("CommitTransaction:\n%s", wtxNew.tx->ToString());
        {
            // Take key pair from key pool so it won't be used again
            reservekey.KeepKey();

            // Add tx to wallet, because if it has change it's also ours,
            // otherwise just for transaction history.
            AddToWallet(wtxNew);

            // Notify that old coins are spent
            for (const CellTxIn& txin : wtxNew.tx->vin)
            {
                CellWalletTx &coin = mapWallet[txin.prevout.hash];
                coin.BindWallet(this);
                NotifyTransactionChanged(this, coin.GetHash(), CT_UPDATED);
            }
        }

        // Track how many getdata requests our transaction gets
        mapRequestCount[wtxNew.GetHash()] = 0;

        if (fBroadcastTransactions)
        {
            // Broadcast
            bool fMissingInputs = false;
            if (!wtxNew.AcceptToMemoryPool(maxTxFee, state, true, &fMissingInputs)) {
                LogPrintf("CommitTransaction(): Transaction(%s) cannot be broadcast immediately, %s %s\n", wtxNew.GetHash().ToString(), state.GetRejectReason(), fMissingInputs?",MissingInputs":"");
                // TODO: if we expect the failure to be long term or permanent, instead delete wtx from the wallet and return failure.
                return false;// zjh add this return false;
            } else {
                wtxNew.RelayWalletTransaction(connman);
            }
        }
    }
    return true;
}

void CellWallet::ListAccountCreditDebit(const std::string& strAccount, std::list<CellAccountingEntry>& entries) {
    CWalletDB walletdb(*dbw);
    return walletdb.ListAccountCreditDebit(strAccount, entries);
}

bool CellWallet::AddAccountingEntry(const CellAccountingEntry& acentry)
{
    CWalletDB walletdb(*dbw);

    return AddAccountingEntry(acentry, &walletdb);
}

bool CellWallet::AddAccountingEntry(const CellAccountingEntry& acentry, CWalletDB *pwalletdb)
{
    if (!pwalletdb->WriteAccountingEntry(++nAccountingEntryNumber, acentry)) {
        return false;
    }

    laccentries.push_back(acentry);
    CellAccountingEntry & entry = laccentries.back();
    wtxOrdered.insert(std::make_pair(entry.nOrderPos, TxPair((CellWalletTx*)0, &entry)));

    return true;
}

CellAmount CellWallet::GetRequiredFee(unsigned int nTxBytes)
{
    return std::max(minTxFee.GetFee(nTxBytes), ::minRelayTxFee.GetFee(nTxBytes));
}

CellAmount CellWallet::GetMinimumFee(unsigned int nTxBytes, const CellCoinControl& coin_control, const CellTxMemPool& pool, const CellBlockPolicyEstimator& estimator, FeeCalculation *feeCalc, CellMutableTransaction* tx, SmartLuaState* sls)
{
    /* User control of how to calculate fee uses the following parameter precedence:
       1. coin_control.m_feerate
       2. coin_control.m_confirm_target
       3. payTxFee (user-set global variable)
       4. nTxConfirmTarget (user-set global variable)
       The first parameter that is set is used.
    */
    CellAmount fee_needed;
    if (coin_control.m_feerate) { // 1.
		// 根据默认交易费率计算交易费用
        fee_needed = coin_control.m_feerate->GetFee(nTxBytes);
        if (feeCalc) feeCalc->reason = FeeReason::PAYTXFEE;
        // Allow to override automatic min/max check over coin control instance
        if (coin_control.fOverrideFeeRate) return fee_needed;
    }
    else if (!coin_control.m_confirm_target && ::payTxFee != CellFeeRate(0)) { // 3. TODO: remove magic value of 0 for global payTxFee
		// 确认数量大于0且用户配置表设置的费率大于0时，根据用户配置表设置的费率计算费用
        fee_needed = ::payTxFee.GetFee(nTxBytes);
        if (feeCalc) feeCalc->reason = FeeReason::PAYTXFEE;
    }
    else { // 2. or 4.
        // We will use smart fee estimation
		// 需要确认的次数
        unsigned int target = coin_control.m_confirm_target ? *coin_control.m_confirm_target : ::nTxConfirmTarget;
        // By default estimates are economical if we are signaling opt-in-RBF
        bool conservative_estimate = !coin_control.signalRbf;
        // Allow to override the default fee estimate mode over the CoinControl instance
        if (coin_control.m_fee_mode == FeeEstimateMode::CONSERVATIVE) conservative_estimate = true;			// 保守模式
        else if (coin_control.m_fee_mode == FeeEstimateMode::ECONOMICAL) conservative_estimate = false;		// 经济模式

        fee_needed = estimator.estimateSmartFee(target, feeCalc, conservative_estimate).GetFee(nTxBytes);
        if (fee_needed == 0) {
            // if we don't have enough data for estimateSmartFee, then use fallbackFee
            fee_needed = fallbackFee.GetFee(nTxBytes);
            if (feeCalc) feeCalc->reason = FeeReason::FALLBACK;
        }
        // Obey mempool min fee when using smart fee estimation
        CellAmount min_mempool_fee = pool.GetMinFee(gArgs.GetArg("-maxmempool", DEFAULT_MAX_MEMPOOL_SIZE) * 1000000).GetFee(nTxBytes);
        if (fee_needed < min_mempool_fee) {
            fee_needed = min_mempool_fee;
            if (feeCalc) feeCalc->reason = FeeReason::MEMPOOL_MIN;
        }
    }

    // prevent user from paying a fee below minRelayTxFee or minTxFee
    CellAmount required_fee = GetRequiredFee(nTxBytes);
    if (required_fee > fee_needed) {
        fee_needed = required_fee;
        if (feeCalc) feeCalc->reason = FeeReason::REQUIRED;
    }
    // But always obey the maximum
    if (fee_needed > maxTxFee) {
        fee_needed = maxTxFee;
        if (feeCalc) feeCalc->reason = FeeReason::MAXTXFEE;
    }
    /*下面方法废弃的了，直接把fee_needed乘个倍数的话，weight不变，会把手续费直接拉高
      改到GetTransactionWeight
	if (tx != nullptr)
	{
		// fee of branch chain transaction 
        if (tx->IsPregnantTx() || tx->IsBranchCreate() || tx->IsProve() || tx->IsReport()){
            fee_needed *= 10;
        }
        if (tx->IsBranchChainTransStep2()){
            fee_needed *= 20;
        }

		// 根据执行的指令数、代码大小以及存盘数据变化量计算交易费用
		// 相关常量待定，使其与nTxBytes有关
		if (tx->IsSmartContract() && sls != nullptr)
		{
			fee_needed += sls->runningTimes * 0.1 + sls->codeLen * 0.1 + sls->deltaDataLen * 0.1;
		}
	}
    */
    return fee_needed;
}

DBErrors CellWallet::LoadWallet(bool& fFirstRunRet)
{
    LOCK2(cs_main, cs_wallet);

    fFirstRunRet = false;
    DBErrors nLoadWalletRet = CWalletDB(*dbw,"cr+").LoadWallet(this);
    if (nLoadWalletRet == DB_NEED_REWRITE)
    {
        if (dbw->Rewrite("\x04pool"))
        {
            setInternalKeyPool.clear();
            setExternalKeyPool.clear();
            m_pool_key_to_index.clear();
            // Note: can't top-up keypool here, because wallet is locked.
            // User will be prompted to unlock wallet the next operation
            // that requires a new key.
        }
    }

    if (nLoadWalletRet != DB_LOAD_OK)
        return nLoadWalletRet;
    fFirstRunRet = !vchDefaultKey.IsValid();

    uiInterface.LoadWallet(this);

    return DB_LOAD_OK;
}

DBErrors CellWallet::ZapSelectTx(std::vector<uint256>& vHashIn, std::vector<uint256>& vHashOut)
{
    AssertLockHeld(cs_wallet); // mapWallet
    vchDefaultKey = CellPubKey();
    DBErrors nZapSelectTxRet = CWalletDB(*dbw,"cr+").ZapSelectTx(vHashIn, vHashOut);
    for (uint256 hash : vHashOut)
        mapWallet.erase(hash);

    if (nZapSelectTxRet == DB_NEED_REWRITE)
    {
        if (dbw->Rewrite("\x04pool"))
        {
            setInternalKeyPool.clear();
            setExternalKeyPool.clear();
            m_pool_key_to_index.clear();
            // Note: can't top-up keypool here, because wallet is locked.
            // User will be prompted to unlock wallet the next operation
            // that requires a new key.
        }
    }

    if (nZapSelectTxRet != DB_LOAD_OK)
        return nZapSelectTxRet;

    MarkDirty();

    return DB_LOAD_OK;

}

DBErrors CellWallet::ZapWalletTx(std::vector<CellWalletTx>& vWtx)
{
    vchDefaultKey = CellPubKey();
    DBErrors nZapWalletTxRet = CWalletDB(*dbw,"cr+").ZapWalletTx(vWtx);
    if (nZapWalletTxRet == DB_NEED_REWRITE)
    {
        if (dbw->Rewrite("\x04pool"))
        {
            LOCK(cs_wallet);
            setInternalKeyPool.clear();
            setExternalKeyPool.clear();
            m_pool_key_to_index.clear();
            // Note: can't top-up keypool here, because wallet is locked.
            // User will be prompted to unlock wallet the next operation
            // that requires a new key.
        }
    }

    if (nZapWalletTxRet != DB_LOAD_OK)
        return nZapWalletTxRet;

    return DB_LOAD_OK;
}


bool CellWallet::SetAddressBook(const CellTxDestination& address, const std::string& strName, const std::string& strPurpose)
{
    bool fUpdated = false;
    {
        LOCK(cs_wallet); // mapAddressBook
        std::map<CellTxDestination, CellAddressBookData>::iterator mi = mapAddressBook.find(address);
        fUpdated = mi != mapAddressBook.end();
        mapAddressBook[address].name = strName;
        if (!strPurpose.empty()) /* update purpose only if requested */
            mapAddressBook[address].purpose = strPurpose;
    }
    NotifyAddressBookChanged(this, address, strName, ::IsMine(*this, address) != ISMINE_NO,
                             strPurpose, (fUpdated ? CT_UPDATED : CT_NEW) );
    if (!strPurpose.empty() && !CWalletDB(*dbw).WritePurpose(CellLinkAddress(address).ToString(), strPurpose))
        return false;
    return CWalletDB(*dbw).WriteName(CellLinkAddress(address).ToString(), strName);
}

bool CellWallet::DelAddressBook(const CellTxDestination& address)
{
    {
        LOCK(cs_wallet); // mapAddressBook

        // Delete destdata tuples associated with address
        std::string strAddress = CellLinkAddress(address).ToString();
        for (const std::pair<std::string, std::string> &item : mapAddressBook[address].destdata)
        {
            CWalletDB(*dbw).EraseDestData(strAddress, item.first);
        }
        mapAddressBook.erase(address);
    }

    NotifyAddressBookChanged(this, address, "", ::IsMine(*this, address) != ISMINE_NO, "", CT_DELETED);

    CWalletDB(*dbw).ErasePurpose(CellLinkAddress(address).ToString());
    return CWalletDB(*dbw).EraseName(CellLinkAddress(address).ToString());
}

const std::string& CellWallet::GetAccountName(const CellScript& scriptPubKey) const
{
    CellTxDestination address;
    if (ExtractDestination(scriptPubKey, address) && !scriptPubKey.IsUnspendable()) {
        auto mi = mapAddressBook.find(address);
        if (mi != mapAddressBook.end()) {
            return mi->second.name;
        }
    }
    // A scriptPubKey that doesn't have an entry in the address book is
    // associated with the default account ("").
    const static std::string DEFAULT_ACCOUNT_NAME;
    return DEFAULT_ACCOUNT_NAME;
}

bool CellWallet::SetDefaultKey(const CellPubKey &vchPubKey)
{
    if (!CWalletDB(*dbw).WriteDefaultKey(vchPubKey))
        return false;
    vchDefaultKey = vchPubKey;
    return true;
}

/**
 * Mark old keypool keys as used,
 * and generate all new keys
 */
bool CellWallet::NewKeyPool()
{
    {
        LOCK(cs_wallet);
        CWalletDB walletdb(*dbw);

        for (int64_t nIndex : setInternalKeyPool) {
            walletdb.ErasePool(nIndex);
        }
        setInternalKeyPool.clear();

        for (int64_t nIndex : setExternalKeyPool) {
            walletdb.ErasePool(nIndex);
        }
        setExternalKeyPool.clear();

        m_pool_key_to_index.clear();

        if (!TopUpKeyPool()) {
            return false;
        }
        LogPrintf("CellWallet::NewKeyPool rewrote keypool\n");
    }
    return true;
}

size_t CellWallet::KeypoolCountExternalKeys()
{
    AssertLockHeld(cs_wallet); // setExternalKeyPool
    return setExternalKeyPool.size();
}

void CellWallet::LoadKeyPool(int64_t nIndex, const CellKeyPool &keypool)
{
    AssertLockHeld(cs_wallet);
    if (keypool.fInternal) {
        setInternalKeyPool.insert(nIndex);
    } else {
        setExternalKeyPool.insert(nIndex);
    }
    m_max_keypool_index = std::max(m_max_keypool_index, nIndex);
    m_pool_key_to_index[keypool.vchPubKey.GetID()] = nIndex;

    // If no metadata exists yet, create a default with the pool key's
    // creation time. Note that this may be overwritten by actually
    // stored metadata for that key later, which is fine.
    CellKeyID keyid = keypool.vchPubKey.GetID();
    if (mapKeyMetadata.count(keyid) == 0)
        mapKeyMetadata[keyid] = CKeyMetadata(keypool.nTime);
}

bool CellWallet::TopUpKeyPool(unsigned int kpSize)
{
    {
        LOCK(cs_wallet);

        if (IsLocked())
            return false;

        // Top up key pool
        unsigned int nTargetSize;
        if (kpSize > 0)
            nTargetSize = kpSize;
        else
            nTargetSize = std::max(gArgs.GetArg("-keypool", DEFAULT_KEYPOOL_SIZE), (int64_t) 0);

        // count amount of available keys (internal, external)
        // make sure the keypool of external and internal keys fits the user selected target (-keypool)
        int64_t missingExternal = std::max(std::max((int64_t) nTargetSize, (int64_t) 1) - (int64_t)setExternalKeyPool.size(), (int64_t) 0);
        int64_t missingInternal = std::max(std::max((int64_t) nTargetSize, (int64_t) 1) - (int64_t)setInternalKeyPool.size(), (int64_t) 0);

        if (!IsHDEnabled() || !CanSupportFeature(FEATURE_HD_SPLIT))
        {
            // don't create extra internal keys
            missingInternal = 0;
        }
        bool internal = false;
        CWalletDB walletdb(*dbw);
        for (int64_t i = missingInternal + missingExternal; i--;)
        {
            if (i < missingInternal) {
                internal = true;
            }

            assert(m_max_keypool_index < std::numeric_limits<int64_t>::max()); // How in the hell did you use so many keys?
            int64_t index = ++m_max_keypool_index;

            CellPubKey pubkey(GenerateNewKey(walletdb, internal));
            if (!walletdb.WritePool(index, CellKeyPool(pubkey, internal))) {
                throw std::runtime_error(std::string(__func__) + ": writing generated key failed");
            }

            if (internal) {
                setInternalKeyPool.insert(index);
            } else {
                setExternalKeyPool.insert(index);
            }
            m_pool_key_to_index[pubkey.GetID()] = index;
        }
        if (missingInternal + missingExternal > 0) {
            LogPrintf("keypool added %d keys (%d internal), size=%u (%u internal)\n", missingInternal + missingExternal, missingInternal, setInternalKeyPool.size() + setExternalKeyPool.size(), setInternalKeyPool.size());
        }
    }
    return true;
}

void CellWallet::ReserveKeyFromKeyPool(int64_t& nIndex, CellKeyPool& keypool, bool fRequestedInternal)
{
    nIndex = -1;
    keypool.vchPubKey = CellPubKey();
    {
        LOCK(cs_wallet);

        if (!IsLocked())
            TopUpKeyPool();

        bool fReturningInternal = IsHDEnabled() && CanSupportFeature(FEATURE_HD_SPLIT) && fRequestedInternal;
        std::set<int64_t>& setKeyPool = fReturningInternal ? setInternalKeyPool : setExternalKeyPool;

        // Get the oldest key
        if(setKeyPool.empty())
            return;

        CWalletDB walletdb(*dbw);

        auto it = setKeyPool.begin();
        nIndex = *it;
        setKeyPool.erase(it);
        if (!walletdb.ReadPool(nIndex, keypool)) {
            throw std::runtime_error(std::string(__func__) + ": read failed");
        }
        if (!HaveKey(keypool.vchPubKey.GetID())) {
            throw std::runtime_error(std::string(__func__) + ": unknown key in key pool");
        }
        if (keypool.fInternal != fReturningInternal) {
            throw std::runtime_error(std::string(__func__) + ": keypool entry misclassified");
        }

        assert(keypool.vchPubKey.IsValid());
        m_pool_key_to_index.erase(keypool.vchPubKey.GetID());
        LogPrintf("keypool reserve %d\n", nIndex);
    }
}

void CellWallet::KeepKey(int64_t nIndex)
{
    // Remove from key pool
    CWalletDB walletdb(*dbw);
    walletdb.ErasePool(nIndex);
    LogPrintf("keypool keep %d\n", nIndex);
}

void CellWallet::ReturnKey(int64_t nIndex, bool fInternal, const CellPubKey& pubkey)
{
    // Return to key pool
    {
        LOCK(cs_wallet);
        if (fInternal) {
            setInternalKeyPool.insert(nIndex);
        } else {
            setExternalKeyPool.insert(nIndex);
        }
        m_pool_key_to_index[pubkey.GetID()] = nIndex;
    }
    LogPrintf("keypool return %d\n", nIndex);
}

bool CellWallet::GetKeyFromPool(CellPubKey& result, bool internal)
{
    CellKeyPool keypool;
    {
        LOCK(cs_wallet);
        int64_t nIndex = 0;
        ReserveKeyFromKeyPool(nIndex, keypool, internal);
        if (nIndex == -1)
        {
            if (IsLocked()) return false;
            CWalletDB walletdb(*dbw);
            result = GenerateNewKey(walletdb, internal);
            return true;
        }
        KeepKey(nIndex);
        result = keypool.vchPubKey;
    }
    return true;
}

static int64_t GetOldestKeyTimeInPool(const std::set<int64_t>& setKeyPool, CWalletDB& walletdb) {
    if (setKeyPool.empty()) {
        return GetTime();
    }

    CellKeyPool keypool;
    int64_t nIndex = *(setKeyPool.begin());
    if (!walletdb.ReadPool(nIndex, keypool)) {
        throw std::runtime_error(std::string(__func__) + ": read oldest key in keypool failed");
    }
    assert(keypool.vchPubKey.IsValid());
    return keypool.nTime;
}

int64_t CellWallet::GetOldestKeyPoolTime()
{
    LOCK(cs_wallet);

    CWalletDB walletdb(*dbw);

    // load oldest key from keypool, get time and return
    int64_t oldestKey = GetOldestKeyTimeInPool(setExternalKeyPool, walletdb);
    if (IsHDEnabled() && CanSupportFeature(FEATURE_HD_SPLIT)) {
        oldestKey = std::max(GetOldestKeyTimeInPool(setInternalKeyPool, walletdb), oldestKey);
    }

    return oldestKey;
}

std::map<CellTxDestination, CellAmount> CellWallet::GetAddressBalances()
{
    std::map<CellTxDestination, CellAmount> balances;

    {
        LOCK(cs_wallet);
        for (const auto& walletEntry : mapWallet)
        {
            const CellWalletTx *pcoin = &walletEntry.second;

            if (!pcoin->IsTrusted())
                continue;

            if (pcoin->IsCoinBase() && pcoin->GetBlocksToMaturity() > 0)
                continue;

            int nDepth = pcoin->GetDepthInMainChain();
            if (nDepth < (pcoin->IsFromMe(ISMINE_ALL) ? 0 : 1))
                continue;

            for (unsigned int i = 0; i < pcoin->tx->vout.size(); i++)
            {
                CellTxDestination addr;
                if (!IsMine(pcoin->tx->vout[i]))
                    continue;
                if(!ExtractDestination(pcoin->tx->vout[i].scriptPubKey, addr))
                    continue;

                CellAmount n = IsSpent(walletEntry.first, i) ? 0 : pcoin->tx->vout[i].nValue;

                if (!balances.count(addr))
                    balances[addr] = 0;
                balances[addr] += n;
            }
        }
    }

    return balances;
}

std::set< std::set<CellTxDestination> > CellWallet::GetAddressGroupings()
{
    AssertLockHeld(cs_wallet); // mapWallet
    std::set< std::set<CellTxDestination> > groupings;
    std::set<CellTxDestination> grouping;

    for (const auto& walletEntry : mapWallet)
    {
        const CellWalletTx *pcoin = &walletEntry.second;

        if (pcoin->tx->vin.size() > 0)
        {
            bool any_mine = false;
            // group all input addresses with each other
            for (CellTxIn txin : pcoin->tx->vin)
            {
                CellTxDestination address;
                if(!IsMine(txin)) /* If this input isn't mine, ignore it */
                    continue;
                if(!ExtractDestination(mapWallet[txin.prevout.hash].tx->vout[txin.prevout.n].scriptPubKey, address))
                    continue;
                grouping.insert(address);
                any_mine = true;
            }

            // group change with input addresses
            if (any_mine)
            {
               for (CellTxOut txout : pcoin->tx->vout)
                   if (IsChange(txout))
                   {
                       CellTxDestination txoutAddr;
                       if(!ExtractDestination(txout.scriptPubKey, txoutAddr))
                           continue;
                       grouping.insert(txoutAddr);
                   }
            }
            if (grouping.size() > 0)
            {
                groupings.insert(grouping);
                grouping.clear();
            }
        }

        // group lone addrs by themselves
        for (const auto& txout : pcoin->tx->vout)
            if (IsMine(txout))
            {
                CellTxDestination address;
                if(!ExtractDestination(txout.scriptPubKey, address))
                    continue;
                grouping.insert(address);
                groupings.insert(grouping);
                grouping.clear();
            }
    }

    std::set< std::set<CellTxDestination>* > uniqueGroupings; // a set of pointers to groups of addresses
    std::map< CellTxDestination, std::set<CellTxDestination>* > setmap;  // map addresses to the unique group containing it
    for (std::set<CellTxDestination> _grouping : groupings)
    {
        // make a set of all the groups hit by this new group
        std::set< std::set<CellTxDestination>* > hits;
        std::map< CellTxDestination, std::set<CellTxDestination>* >::iterator it;
        for (CellTxDestination address : _grouping)
            if ((it = setmap.find(address)) != setmap.end())
                hits.insert((*it).second);

        // merge all hit groups into a new single group and delete old groups
        std::set<CellTxDestination>* merged = new std::set<CellTxDestination>(_grouping);
        for (std::set<CellTxDestination>* hit : hits)
        {
            merged->insert(hit->begin(), hit->end());
            uniqueGroupings.erase(hit);
            delete hit;
        }
        uniqueGroupings.insert(merged);

        // update setmap
        for (CellTxDestination element : *merged)
            setmap[element] = merged;
    }

    std::set< std::set<CellTxDestination> > ret;
    for (std::set<CellTxDestination>* uniqueGrouping : uniqueGroupings)
    {
        ret.insert(*uniqueGrouping);
        delete uniqueGrouping;
    }

    return ret;
}

std::set<CellTxDestination> CellWallet::GetAccountAddresses(const std::string& strAccount) const
{
    LOCK(cs_wallet);
    std::set<CellTxDestination> result;
    for (const std::pair<CellTxDestination, CellAddressBookData>& item : mapAddressBook)
    {
        const CellTxDestination& address = item.first;
        const std::string& strName = item.second.name;
        if (strName == strAccount)
            result.insert(address);
    }
    return result;
}

bool CellReserveKey::GetReservedKey(CellPubKey& pubkey, bool internal)
{
    if (nIndex == -1)
    {
        CellKeyPool keypool;
        pwallet->ReserveKeyFromKeyPool(nIndex, keypool, internal);
        if (nIndex != -1)
            vchPubKey = keypool.vchPubKey;
        else {
            return false;
        }
        fInternal = keypool.fInternal;
    }
    assert(vchPubKey.IsValid());
    pubkey = vchPubKey;
    return true;
}

void CellReserveKey::KeepKey()
{
    if (nIndex != -1)
        pwallet->KeepKey(nIndex);
    nIndex = -1;
    vchPubKey = CellPubKey();
}

void CellReserveKey::ReturnKey()
{
    if (nIndex != -1) {
        pwallet->ReturnKey(nIndex, fInternal, vchPubKey);
    }
    nIndex = -1;
    vchPubKey = CellPubKey();
}

void CellWallet::MarkReserveKeysAsUsed(int64_t keypool_id)
{
    AssertLockHeld(cs_wallet);
    bool internal = setInternalKeyPool.count(keypool_id);
    if (!internal) assert(setExternalKeyPool.count(keypool_id));
    std::set<int64_t> *setKeyPool = internal ? &setInternalKeyPool : &setExternalKeyPool;
    auto it = setKeyPool->begin();

    CWalletDB walletdb(*dbw);
    while (it != std::end(*setKeyPool)) {
        const int64_t& index = *(it);
        if (index > keypool_id) break; // set*KeyPool is ordered

        CellKeyPool keypool;
        if (walletdb.ReadPool(index, keypool)) { //TODO: This should be unnecessary
            m_pool_key_to_index.erase(keypool.vchPubKey.GetID());
        }
        walletdb.ErasePool(index);
        LogPrintf("keypool index %d removed\n", index);
        it = setKeyPool->erase(it);
    }
}

void CellWallet::GetScriptForMining(std::shared_ptr<CReserveScript> &script)
{
    std::shared_ptr<CellReserveKey> rKey = std::make_shared<CellReserveKey>(this);
    CellPubKey pubkey;
    if (!rKey->GetReservedKey(pubkey))
        return;

    script = rKey;
//    script->reserveScript = CellScript() << ToByteVector(pubkey) << OP_CHECKSIG;
	CellTxDestination kDest(pubkey.GetID());
	script->reserveScript = GetScriptForDestination( kDest );
}

void CellWallet::LockCoin(const CellOutPoint& output)
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    setLockedCoins.insert(output);
}

void CellWallet::UnlockCoin(const CellOutPoint& output)
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    setLockedCoins.erase(output);
}

void CellWallet::UnlockAllCoins()
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    setLockedCoins.clear();
}

bool CellWallet::IsLockedCoin(uint256 hash, unsigned int n) const
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    CellOutPoint outpt(hash, n);

    return (setLockedCoins.count(outpt) > 0);
}

void CellWallet::ListLockedCoins(std::vector<CellOutPoint>& vOutpts) const
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    for (std::set<CellOutPoint>::iterator it = setLockedCoins.begin();
         it != setLockedCoins.end(); it++) {
        CellOutPoint outpt = (*it);
        vOutpts.push_back(outpt);
    }
}

/** @} */ // end of Actions

void CellWallet::GetKeyBirthTimes(std::map<CellTxDestination, int64_t> &mapKeyBirth) const {
    AssertLockHeld(cs_wallet); // mapKeyMetadata
    mapKeyBirth.clear();

    // get birth times for keys with metadata
    for (const auto& entry : mapKeyMetadata) {
        if (entry.second.nCreateTime) {
            mapKeyBirth[entry.first] = entry.second.nCreateTime;
        }
    }

    // map in which we'll infer heights of other keys
    CellBlockIndex *pindexMax = chainActive[std::max(0, chainActive.Height() - 144)]; // the tip can be reorganized; use a 144-block safety margin
    std::map<CellKeyID, CellBlockIndex*> mapKeyFirstBlock;
    std::set<CellKeyID> setKeys;
    GetKeys(setKeys);
    for (const CellKeyID &keyid : setKeys) {
        if (mapKeyBirth.count(keyid) == 0)
            mapKeyFirstBlock[keyid] = pindexMax;
    }
    setKeys.clear();

    // if there are no such keys, we're done
    if (mapKeyFirstBlock.empty())
        return;

    // find first block that affects those keys, if there are any left
    std::vector<CellKeyID> vAffected;
    for (std::map<uint256, CellWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); it++) {
        // iterate over all wallet transactions...
        const CellWalletTx &wtx = (*it).second;
        BlockMap::const_iterator blit = mapBlockIndex.find(wtx.hashBlock);
        if (blit != mapBlockIndex.end() && chainActive.Contains(blit->second)) {
            // ... which are already in a block
            int nHeight = blit->second->nHeight;
            for (const CellTxOut &txout : wtx.tx->vout) {
                // iterate over all their outputs
                CellAffectedKeysVisitor(*this, vAffected).Process(txout.scriptPubKey);
                for (const CellKeyID &keyid : vAffected) {
                    // ... and all their affected keys
                    std::map<CellKeyID, CellBlockIndex*>::iterator rit = mapKeyFirstBlock.find(keyid);
                    if (rit != mapKeyFirstBlock.end() && nHeight < rit->second->nHeight)
                        rit->second = blit->second;
                }
                vAffected.clear();
            }
        }
    }

    // Extract block timestamps for those keys
    for (std::map<CellKeyID, CellBlockIndex*>::const_iterator it = mapKeyFirstBlock.begin(); it != mapKeyFirstBlock.end(); it++)
        mapKeyBirth[it->first] = it->second->GetBlockTime() - TIMESTAMP_WINDOW; // block times can be 2h off
}

/**
 * Compute smart timestamp for a transaction being added to the wallet.
 *
 * Logic:
 * - If sending a transaction, assign its timestamp to the current time.
 * - If receiving a transaction outside a block, assign its timestamp to the
 *   current time.
 * - If receiving a block with a future timestamp, assign all its (not already
 *   known) transactions' timestamps to the current time.
 * - If receiving a block with a past timestamp, before the most recent known
 *   transaction (that we care about), assign all its (not already known)
 *   transactions' timestamps to the same timestamp as that most-recent-known
 *   transaction.
 * - If receiving a block with a past timestamp, but after the most recent known
 *   transaction, assign all its (not already known) transactions' timestamps to
 *   the block time.
 *
 * For more information see CellWalletTx::nTimeSmart,
 * https://bitcointalk.org/?topic=54527, or
 * https://github.com/bitcoin/bitcoin/pull/1393.
 */
unsigned int CellWallet::ComputeTimeSmart(const CellWalletTx& wtx) const
{
    unsigned int nTimeSmart = wtx.nTimeReceived;
    if (!wtx.hashUnset()) {
        if (mapBlockIndex.count(wtx.hashBlock)) {
            int64_t latestNow = wtx.nTimeReceived;
            int64_t latestEntry = 0;

            // Tolerate times up to the last timestamp in the wallet not more than 5 minutes into the future
            int64_t latestTolerated = latestNow + 300;
            const TxItems& txOrdered = wtxOrdered;
            for (auto it = txOrdered.rbegin(); it != txOrdered.rend(); ++it) {
                CellWalletTx* const pwtx = it->second.first;
                if (pwtx == &wtx) {
                    continue;
                }
                CellAccountingEntry* const pacentry = it->second.second;
                int64_t nSmartTime;
                if (pwtx) {
                    nSmartTime = pwtx->nTimeSmart;
                    if (!nSmartTime) {
                        nSmartTime = pwtx->nTimeReceived;
                    }
                } else {
                    nSmartTime = pacentry->nTime;
                }
                if (nSmartTime <= latestTolerated) {
                    latestEntry = nSmartTime;
                    if (nSmartTime > latestNow) {
                        latestNow = nSmartTime;
                    }
                    break;
                }
            }

            int64_t blocktime = mapBlockIndex[wtx.hashBlock]->GetBlockTime();
            nTimeSmart = std::max(latestEntry, std::min(blocktime, latestNow));
        } else {
            LogPrintf("%s: found %s in block %s not in index\n", __func__, wtx.GetHash().ToString(), wtx.hashBlock.ToString());
        }
    }
    return nTimeSmart;
}

bool CellWallet::AddDestData(const CellTxDestination &dest, const std::string &key, const std::string &value)
{
    if (boost::get<CellNoDestination>(&dest))
        return false;

    mapAddressBook[dest].destdata.insert(std::make_pair(key, value));
    return CWalletDB(*dbw).WriteDestData(CellLinkAddress(dest).ToString(), key, value);
}

bool CellWallet::EraseDestData(const CellTxDestination &dest, const std::string &key)
{
    if (!mapAddressBook[dest].destdata.erase(key))
        return false;
    return CWalletDB(*dbw).EraseDestData(CellLinkAddress(dest).ToString(), key);
}

bool CellWallet::LoadDestData(const CellTxDestination &dest, const std::string &key, const std::string &value)
{
    mapAddressBook[dest].destdata.insert(std::make_pair(key, value));
    return true;
}

bool CellWallet::GetDestData(const CellTxDestination &dest, const std::string &key, std::string *value) const
{
    std::map<CellTxDestination, CellAddressBookData>::const_iterator i = mapAddressBook.find(dest);
    if(i != mapAddressBook.end())
    {
        CellAddressBookData::StringMap::const_iterator j = i->second.destdata.find(key);
        if(j != i->second.destdata.end())
        {
            if(value)
                *value = j->second;
            return true;
        }
    }
    return false;
}

std::vector<std::string> CellWallet::GetDestValues(const std::string& prefix) const
{
    LOCK(cs_wallet);
    std::vector<std::string> values;
    for (const auto& address : mapAddressBook) {
        for (const auto& data : address.second.destdata) {
            if (!data.first.compare(0, prefix.size(), prefix)) {
                values.emplace_back(data.second);
            }
        }
    }
    return values;
}

std::string CellWallet::GetWalletHelpString(bool showDebug)
{
    std::string strUsage = HelpMessageGroup(_("Wallet options:"));
    strUsage += HelpMessageOpt("-disablewallet", _("Do not load the wallet and disable wallet RPC calls"));
    strUsage += HelpMessageOpt("-keypool=<n>", strprintf(_("Set key pool size to <n> (default: %u)"), DEFAULT_KEYPOOL_SIZE));
    strUsage += HelpMessageOpt("-fallbackfee=<amt>", strprintf(_("A fee rate (in %s/kB) that will be used when fee estimation has insufficient data (default: %s)"),
                                                               CURRENCY_UNIT, FormatMoney(DEFAULT_FALLBACK_FEE)));
    strUsage += HelpMessageOpt("-discardfee=<amt>", strprintf(_("The fee rate (in %s/kB) that indicates your tolerance for discarding change by adding it to the fee (default: %s). "
                                                                "Note: An output is discarded if it is dust at this rate, but we will always discard up to the dust relay fee and a discard fee above that is limited by the fee estimate for the longest target"),
                                                              CURRENCY_UNIT, FormatMoney(DEFAULT_DISCARD_FEE)));
    strUsage += HelpMessageOpt("-mintxfee=<amt>", strprintf(_("Fees (in %s/kB) smaller than this are considered zero fee for transaction creation (default: %s)"),
                                                            CURRENCY_UNIT, FormatMoney(DEFAULT_TRANSACTION_MINFEE)));
    strUsage += HelpMessageOpt("-paytxfee=<amt>", strprintf(_("Fee (in %s/kB) to add to transactions you send (default: %s)"),
                                                            CURRENCY_UNIT, FormatMoney(payTxFee.GetFeePerK())));
    strUsage += HelpMessageOpt("-rescan", _("Rescan the block chain for missing wallet transactions on startup"));
    strUsage += HelpMessageOpt("-salvagewallet", _("Attempt to recover private keys from a corrupt wallet on startup"));
    strUsage += HelpMessageOpt("-spendzeroconfchange", strprintf(_("Spend unconfirmed change when sending transactions (default: %u)"), DEFAULT_SPEND_ZEROCONF_CHANGE));
    strUsage += HelpMessageOpt("-txconfirmtarget=<n>", strprintf(_("If paytxfee is not set, include enough fee so transactions begin confirmation on average within n blocks (default: %u)"), DEFAULT_TX_CONFIRM_TARGET));
    strUsage += HelpMessageOpt("-usehd", _("Use hierarchical deterministic key generation (HD) after BIP32. Only has effect during wallet creation/first start") + " " + strprintf(_("(default: %u)"), DEFAULT_USE_HD_WALLET));
    strUsage += HelpMessageOpt("-walletrbf", strprintf(_("Send transactions with full-RBF opt-in enabled (default: %u)"), DEFAULT_WALLET_RBF));
    strUsage += HelpMessageOpt("-upgradewallet", _("Upgrade wallet to latest format on startup"));
    strUsage += HelpMessageOpt("-wallet=<file>", _("Specify wallet file (within data directory)") + " " + strprintf(_("(default: %s)"), DEFAULT_WALLET_DAT));
    strUsage += HelpMessageOpt("-walletbroadcast", _("Make the wallet broadcast transactions") + " " + strprintf(_("(default: %u)"), DEFAULT_WALLETBROADCAST));
    strUsage += HelpMessageOpt("-walletnotify=<cmd>", _("Execute command when a wallet transaction changes (%s in cmd is replaced by TxID)"));
    strUsage += HelpMessageOpt("-zapwallettxes=<mode>", _("Delete all wallet transactions and only recover those parts of the blockchain through -rescan on startup") +
                               " " + _("(1 = keep tx meta data e.g. account owner and payment request information, 2 = drop tx meta data)"));

    if (showDebug)
    {
        strUsage += HelpMessageGroup(_("Wallet debugging/testing options:"));

        strUsage += HelpMessageOpt("-dblogsize=<n>", strprintf("Flush wallet database activity from memory to disk log every <n> megabytes (default: %u)", DEFAULT_WALLET_DBLOGSIZE));
        strUsage += HelpMessageOpt("-flushwallet", strprintf("Run a thread to flush wallet periodically (default: %u)", DEFAULT_FLUSHWALLET));
        strUsage += HelpMessageOpt("-privdb", strprintf("Sets the DB_PRIVATE flag in the wallet db environment (default: %u)", DEFAULT_WALLET_PRIVDB));
        strUsage += HelpMessageOpt("-walletrejectlongchains", strprintf(_("Wallet will not create transactions that violate mempool chain limits (default: %u)"), DEFAULT_WALLET_REJECT_LONG_CHAINS));
    }

    return strUsage;
}

CellWallet* CellWallet::CreateWalletFromFile(const std::string walletFile)
{
    // needed to restore wallet transaction meta data after -zapwallettxes
    std::vector<CellWalletTx> vWtx;

    if (gArgs.GetBoolArg("-zapwallettxes", false)) {
        uiInterface.InitMessage(_("Zapping all transactions from wallet..."));

        std::unique_ptr<CellWalletDBWrapper> dbw(new CellWalletDBWrapper(&bitdb, walletFile));
        CellWallet *tempWallet = new CellWallet(std::move(dbw));
        DBErrors nZapWalletRet = tempWallet->ZapWalletTx(vWtx);
        if (nZapWalletRet != DB_LOAD_OK) {
            InitError(strprintf(_("Error loading %s: Wallet corrupted"), walletFile));
            return nullptr;
        }

        delete tempWallet;
        tempWallet = nullptr;
    }

    uiInterface.InitMessage(_("Loading wallet..."));

    int64_t nStart = GetTimeMillis();
    bool fFirstRun = true;
    std::unique_ptr<CellWalletDBWrapper> dbw(new CellWalletDBWrapper(&bitdb, walletFile));
    CellWallet *walletInstance = new CellWallet(std::move(dbw));
    DBErrors nLoadWalletRet = walletInstance->LoadWallet(fFirstRun);
    if (nLoadWalletRet != DB_LOAD_OK)
    {
        if (nLoadWalletRet == DB_CORRUPT) {
            InitError(strprintf(_("Error loading %s: Wallet corrupted"), walletFile));
            return nullptr;
        }
        else if (nLoadWalletRet == DB_NONCRITICAL_ERROR)
        {
            InitWarning(strprintf(_("Error reading %s! All keys read correctly, but transaction data"
                                         " or address book entries might be missing or incorrect."),
                walletFile));
        }
        else if (nLoadWalletRet == DB_TOO_NEW) {
            InitError(strprintf(_("Error loading %s: Wallet requires newer version of %s"), walletFile, _(PACKAGE_NAME)));
            return nullptr;
        }
        else if (nLoadWalletRet == DB_NEED_REWRITE)
        {
            InitError(strprintf(_("Wallet needed to be rewritten: restart %s to complete"), _(PACKAGE_NAME)));
            return nullptr;
        }
        else {
            InitError(strprintf(_("Error loading %s"), walletFile));
            return nullptr;
        }
    }

    if (gArgs.GetBoolArg("-upgradewallet", fFirstRun))
    {
        int nMaxVersion = gArgs.GetArg("-upgradewallet", 0);
        if (nMaxVersion == 0) // the -upgradewallet without argument case
        {
            LogPrintf("Performing wallet upgrade to %i\n", FEATURE_LATEST);
            nMaxVersion = CLIENT_VERSION;
            walletInstance->SetMinVersion(FEATURE_LATEST); // permanently upgrade the wallet immediately
        }
        else
            LogPrintf("Allowing wallet upgrade up to %i\n", nMaxVersion);
        if (nMaxVersion < walletInstance->GetVersion())
        {
            InitError(_("Cannot downgrade wallet"));
            return nullptr;
        }
        walletInstance->SetMaxVersion(nMaxVersion);
    }

    if (fFirstRun)
    {
        // Create new keyUser and set as default key
        if (gArgs.GetBoolArg("-usehd", DEFAULT_USE_HD_WALLET) && !walletInstance->IsHDEnabled()) {

            // ensure this wallet.dat can only be opened by clients supporting HD with chain split
            walletInstance->SetMinVersion(FEATURE_HD_SPLIT);

            // generate a new master key
            CellPubKey masterPubKey = walletInstance->GenerateNewHDMasterKey();
            if (!walletInstance->SetHDMasterKey(masterPubKey))
                throw std::runtime_error(std::string(__func__) + ": Storing master key failed");
        }
        CellPubKey newDefaultKey;
        if (walletInstance->GetKeyFromPool(newDefaultKey, false)) {
            walletInstance->SetDefaultKey(newDefaultKey);
            if (!walletInstance->SetAddressBook(walletInstance->vchDefaultKey.GetID(), "", "receive")) {
                InitError(_("Cannot write default address") += "\n");
                return nullptr;
            }
        }

        walletInstance->SetBestChain(chainActive.GetLocator());
    }
    else if (gArgs.IsArgSet("-usehd")) {
        bool useHD = gArgs.GetBoolArg("-usehd", DEFAULT_USE_HD_WALLET);
        if (walletInstance->IsHDEnabled() && !useHD) {
            InitError(strprintf(_("Error loading %s: You can't disable HD on an already existing HD wallet"), walletFile));
            return nullptr;
        }
        if (!walletInstance->IsHDEnabled() && useHD) {
            InitError(strprintf(_("Error loading %s: You can't enable HD on an already existing non-HD wallet"), walletFile));
            return nullptr;
        }
    }

    LogPrintf("Load %s      %15dms\n", walletFile.c_str(), GetTimeMillis() - nStart);

    RegisterValidationInterface(walletInstance);

    // Try to top up keypool. No-op if the wallet is locked.
    walletInstance->TopUpKeyPool();

    CellBlockIndex *pindexRescan = chainActive.Genesis();
    if (!gArgs.GetBoolArg("-rescan", false))
    {
        CWalletDB walletdb(*walletInstance->dbw);
        CellBlockLocator locator;
        if (walletdb.ReadBestBlock(locator))
            pindexRescan = FindForkInGlobalIndex(chainActive, locator);
    }
    if (chainActive.Tip() && chainActive.Tip() != pindexRescan)
    {
        //We can't rescan beyond non-pruned blocks, stop and throw an error
        //this might happen if a user uses an old wallet within a pruned node
        // or if he ran -disablewallet for a longer time, then decided to re-enable
        if (fPruneMode)
        {
            CellBlockIndex *block = chainActive.Tip();
            while (block && block->pprev && (block->pprev->nStatus & BLOCK_HAVE_DATA) && block->pprev->nTx > 0 && pindexRescan != block)
                block = block->pprev;

            if (pindexRescan != block) {
                InitError(_("Prune: last wallet synchronisation goes beyond pruned data. You need to -reindex (download the whole blockchain again in case of pruned node)"));
                return nullptr;
            }
        }

        uiInterface.InitMessage(_("Rescanning..."));
        LogPrintf("Rescanning last %i blocks (from block %i)...\n", chainActive.Height() - pindexRescan->nHeight, pindexRescan->nHeight);

        // No need to read and scan block if block was created before
        // our wallet birthday (as adjusted for block time variability)
        while (pindexRescan && walletInstance->nTimeFirstKey && (pindexRescan->GetBlockTime() < (walletInstance->nTimeFirstKey - TIMESTAMP_WINDOW))) {
            pindexRescan = chainActive.Next(pindexRescan);
        }

        nStart = GetTimeMillis();
        walletInstance->ScanForWalletTransactions(pindexRescan, true);
        LogPrintf(" rescan      %15dms\n", GetTimeMillis() - nStart);
        walletInstance->SetBestChain(chainActive.GetLocator());
        walletInstance->dbw->IncrementUpdateCounter();

        // Restore wallet transaction metadata after -zapwallettxes=1
        if (gArgs.GetBoolArg("-zapwallettxes", false) && gArgs.GetArg("-zapwallettxes", "1") != "2")
        {
            CWalletDB walletdb(*walletInstance->dbw);

            for (const CellWalletTx& wtxOld : vWtx)
            {
                uint256 hash = wtxOld.GetHash();
                std::map<uint256, CellWalletTx>::iterator mi = walletInstance->mapWallet.find(hash);
                if (mi != walletInstance->mapWallet.end())
                {
                    const CellWalletTx* copyFrom = &wtxOld;
                    CellWalletTx* copyTo = &mi->second;
                    copyTo->mapValue = copyFrom->mapValue;
                    copyTo->vOrderForm = copyFrom->vOrderForm;
                    copyTo->nTimeReceived = copyFrom->nTimeReceived;
                    copyTo->nTimeSmart = copyFrom->nTimeSmart;
                    copyTo->fFromMe = copyFrom->fFromMe;
                    copyTo->strFromAccount = copyFrom->strFromAccount;
                    copyTo->nOrderPos = copyFrom->nOrderPos;
                    walletdb.WriteTx(*copyTo);
                }
            }
        }
    }
    walletInstance->SetBroadcastTransactions(gArgs.GetBoolArg("-walletbroadcast", DEFAULT_WALLETBROADCAST));

    {
        LOCK(walletInstance->cs_wallet);
        LogPrintf("setKeyPool.size() = %u\n",      walletInstance->GetKeyPoolSize());
        LogPrintf("mapWallet.size() = %u\n",       walletInstance->mapWallet.size());
        LogPrintf("mapAddressBook.size() = %u\n",  walletInstance->mapAddressBook.size());
    }

    return walletInstance;
}

bool CellWallet::InitLoadWallet()
{
    if (gArgs.GetBoolArg("-disablewallet", DEFAULT_DISABLE_WALLET)) {
        LogPrintf("Wallet disabled!\n");
        return true;
    }

    for (const std::string& walletFile : gArgs.GetArgs("-wallet")) {
        CellWallet * const pwallet = CreateWalletFromFile(walletFile);
        if (!pwallet) {
            return false;
        }
        vpwallets.push_back(pwallet);
    }

    return true;
}

std::atomic<bool> CellWallet::fFlushScheduled(false);

void CellWallet::postInitProcess(CellScheduler& scheduler)
{
    // Add wallet transactions that aren't already in a block to mempool
    // Do this here as mempool requires genesis block to be loaded
    ReacceptWalletTransactions();

    // Run a thread to flush wallet periodically
    if (!CellWallet::fFlushScheduled.exchange(true)) {
        scheduler.scheduleEvery(MaybeCompactWalletDB, 500);
    }
}

bool CellWallet::ParameterInteraction()
{
    gArgs.SoftSetArg("-wallet", DEFAULT_WALLET_DAT);
    const bool is_multiwallet = gArgs.GetArgs("-wallet").size() > 1;

    if (gArgs.GetBoolArg("-disablewallet", DEFAULT_DISABLE_WALLET))
        return true;

    if (gArgs.GetBoolArg("-blocksonly", DEFAULT_BLOCKSONLY) && gArgs.SoftSetBoolArg("-walletbroadcast", false)) {
        LogPrintf("%s: parameter interaction: -blocksonly=1 -> setting -walletbroadcast=0\n", __func__);
    }

    if (gArgs.GetBoolArg("-salvagewallet", false)) {
        if (is_multiwallet) {
            return InitError(strprintf("%s is only allowed with a single wallet file", "-salvagewallet"));
        }
        // Rewrite just private keys: rescan to find transactions
        if (gArgs.SoftSetBoolArg("-rescan", true)) {
            LogPrintf("%s: parameter interaction: -salvagewallet=1 -> setting -rescan=1\n", __func__);
        }
    }

    int zapwallettxes = gArgs.GetArg("-zapwallettxes", 0);
    // -zapwallettxes implies dropping the mempool on startup
    if (zapwallettxes != 0 && gArgs.SoftSetBoolArg("-persistmempool", false)) {
        LogPrintf("%s: parameter interaction: -zapwallettxes=%s -> setting -persistmempool=0\n", __func__, zapwallettxes);
    }

    // -zapwallettxes implies a rescan
    if (zapwallettxes != 0) {
        if (is_multiwallet) {
            return InitError(strprintf("%s is only allowed with a single wallet file", "-zapwallettxes"));
        }
        if (gArgs.SoftSetBoolArg("-rescan", true)) {
            LogPrintf("%s: parameter interaction: -zapwallettxes=%s -> setting -rescan=1\n", __func__, zapwallettxes);
        }
    }

    if (is_multiwallet) {
        if (gArgs.GetBoolArg("-upgradewallet", false)) {
            return InitError(strprintf("%s is only allowed with a single wallet file", "-upgradewallet"));
        }
    }

    if (gArgs.GetBoolArg("-sysperms", false))
        return InitError("-sysperms is not allowed in combination with enabled wallet functionality");
    if (gArgs.GetArg("-prune", 0) && gArgs.GetBoolArg("-rescan", false))
        return InitError(_("Rescans are not possible in pruned mode. You will need to use -reindex which will download the whole blockchain again."));

    if (::minRelayTxFee.GetFeePerK() > HIGH_TX_FEE_PER_KB)
        InitWarning(AmountHighWarn("-minrelaytxfee") + " " +
                    _("The wallet will avoid paying less than the minimum relay fee."));

    if (gArgs.IsArgSet("-mintxfee"))
    {
        CellAmount n = 0;
        if (!ParseMoney(gArgs.GetArg("-mintxfee", ""), n) || 0 == n)
            return InitError(AmountErrMsg("mintxfee", gArgs.GetArg("-mintxfee", "")));
        if (n > HIGH_TX_FEE_PER_KB)
            InitWarning(AmountHighWarn("-mintxfee") + " " +
                        _("This is the minimum transaction fee you pay on every transaction."));
        CellWallet::minTxFee = CellFeeRate(n);
    }
    if (gArgs.IsArgSet("-fallbackfee"))
    {
        CellAmount nFeePerK = 0;
        if (!ParseMoney(gArgs.GetArg("-fallbackfee", ""), nFeePerK))
            return InitError(strprintf(_("Invalid amount for -fallbackfee=<amount>: '%s'"), gArgs.GetArg("-fallbackfee", "")));
        if (nFeePerK > HIGH_TX_FEE_PER_KB)
            InitWarning(AmountHighWarn("-fallbackfee") + " " +
                        _("This is the transaction fee you may pay when fee estimates are not available."));
        CellWallet::fallbackFee = CellFeeRate(nFeePerK);
    }
    if (gArgs.IsArgSet("-discardfee"))
    {
        CellAmount nFeePerK = 0;
        if (!ParseMoney(gArgs.GetArg("-discardfee", ""), nFeePerK))
            return InitError(strprintf(_("Invalid amount for -discardfee=<amount>: '%s'"), gArgs.GetArg("-discardfee", "")));
        if (nFeePerK > HIGH_TX_FEE_PER_KB)
            InitWarning(AmountHighWarn("-discardfee") + " " +
                        _("This is the transaction fee you may discard if change is smaller than dust at this level"));
        CellWallet::m_discard_rate = CellFeeRate(nFeePerK);
    }
    if (gArgs.IsArgSet("-paytxfee"))
    {
        CellAmount nFeePerK = 0;
        if (!ParseMoney(gArgs.GetArg("-paytxfee", ""), nFeePerK))
            return InitError(AmountErrMsg("paytxfee", gArgs.GetArg("-paytxfee", "")));
        if (nFeePerK > HIGH_TX_FEE_PER_KB)
            InitWarning(AmountHighWarn("-paytxfee") + " " +
                        _("This is the transaction fee you will pay if you send a transaction."));

        payTxFee = CellFeeRate(nFeePerK, 1000);
        if (payTxFee < ::minRelayTxFee)
        {
            return InitError(strprintf(_("Invalid amount for -paytxfee=<amount>: '%s' (must be at least %s)"),
                                       gArgs.GetArg("-paytxfee", ""), ::minRelayTxFee.ToString()));
        }
    }
    if (gArgs.IsArgSet("-maxtxfee"))
    {
        CellAmount nMaxFee = 0;
        if (!ParseMoney(gArgs.GetArg("-maxtxfee", ""), nMaxFee))
            return InitError(AmountErrMsg("maxtxfee", gArgs.GetArg("-maxtxfee", "")));
        if (nMaxFee > HIGH_MAX_TX_FEE)
            InitWarning(_("-maxtxfee is set very high! Fees this large could be paid on a single transaction."));
        maxTxFee = nMaxFee;
        if (CellFeeRate(maxTxFee, 1000) < ::minRelayTxFee)
        {
            return InitError(strprintf(_("Invalid amount for -maxtxfee=<amount>: '%s' (must be at least the minrelay fee of %s to prevent stuck transactions)"),
                                       gArgs.GetArg("-maxtxfee", ""), ::minRelayTxFee.ToString()));
        }
    }
    nTxConfirmTarget = gArgs.GetArg("-txconfirmtarget", DEFAULT_TX_CONFIRM_TARGET);
    bSpendZeroConfChange = gArgs.GetBoolArg("-spendzeroconfchange", DEFAULT_SPEND_ZEROCONF_CHANGE);
    fWalletRbf = gArgs.GetBoolArg("-walletrbf", DEFAULT_WALLET_RBF);

    return true;
}

bool CellWallet::BackupWallet(const std::string& strDest)
{
    return dbw->Backup(strDest);
}

CellKeyPool::CellKeyPool()
{
    nTime = GetTime();
    fInternal = false;
}

CellKeyPool::CellKeyPool(const CellPubKey& vchPubKeyIn, bool internalIn)
{
    nTime = GetTime();
    vchPubKey = vchPubKeyIn;
    fInternal = internalIn;
}

CWalletKey::CWalletKey(int64_t nExpires)
{
    nTimeCreated = (nExpires ? GetTime() : 0);
    nTimeExpires = nExpires;
}

void CellMerkleTx::SetMerkleBranch(const CellBlockIndex* pindex, int posInBlock)
{
    // Update the tx's hashBlock
    hashBlock = pindex->GetBlockHash();

    // set the position of the transaction in the block
    nIndex = posInBlock;
}

int CellMerkleTx::GetDepthInMainChain(const CellBlockIndex* &pindexRet) const
{
    if (hashUnset())
        return 0;

    AssertLockHeld(cs_main);

    // Find the block it claims to be in
    BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
    if (mi == mapBlockIndex.end())
        return 0;
    CellBlockIndex* pindex = (*mi).second;
    if (!pindex || !chainActive.Contains(pindex))
        return 0;

    pindexRet = pindex;
    return ((nIndex == -1) ? (-1) : 1) * (chainActive.Height() - pindex->nHeight + 1);
}

int CellMerkleTx::GetBlocksToMaturity() const
{
    if (!IsCoinBase())
        return 0;
    return std::max(0, (COINBASE_MATURITY+1) - GetDepthInMainChain());
}

int CellMerkleTx::GetBlocksToMaturityForCoinCreateBranch() const
{
    if (!tx->IsBranchCreate())
        return 0;
    return std::max(0, (BRANCH_CHAIN_CREATE_COIN_MATURITY + 1) - GetDepthInMainChain());
}

bool CellMerkleTx::AcceptToMemoryPool(const CellAmount& nAbsurdFee, CellValidationState& state, bool executeSmartContract, bool* pfMissingInputs)
{
    return ::AcceptToMemoryPool(mempool, state, tx, true, pfMissingInputs, nullptr, false, nAbsurdFee, executeSmartContract);
}

void GetAvailableMortgageCoinsInMemPool(const CellKeyStore& keystore, std::vector<CellOutput>& vecOutput, std::map<uint256, CellWalletTx> &mapTempWallet, CellCoinsViewCache &view)
{
    vecOutput.clear();
    mapTempWallet.clear();
    for (const CellTxMemPoolEntry& e : mempool.mapTx)
    {
        const CellTransaction& tx = e.GetTx();
        if (tx.IsBranchChainTransStep2())
        {
            uint256 txid;
            CellKeyID keyid;
            if (GetMortgageCoinData(tx.vout[0].scriptPubKey, &txid, &keyid))//Is mortgage coin
            {
                //IsMine(my) mortgage
                if (keystore.HaveKey(keyid))
                {
                    CellWalletTx &wtx = mapTempWallet[tx.GetHash()];
                    wtx.SetTx(e.GetSharedTx());
                    vecOutput.push_back(CellOutput(&wtx, 0, 1, true, true, false));

                    view.AddCoin(CellOutPoint(tx.GetHash(), 0), Coin(tx.vout[0], 1, false), true);
                }
            }
        }
    }
}
