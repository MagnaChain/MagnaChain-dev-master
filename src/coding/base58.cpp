// Copyright (c) 2014-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"

#include "hash.h"
#include "uint256.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <string>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

/** All alphanumeric characters except for "0", "I", "O", and "l" */
static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

bool DecodeBase58(const char* psz, std::vector<unsigned char>& vch)
{
    // Skip leading spaces.
    while (*psz && isspace(*psz))
        psz++;
    // Skip and count leading '1's.
    int zeroes = 0;
    int length = 0;
    while (*psz == '1') {
        zeroes++;
        psz++;
    }
    // Allocate enough space in big-endian base256 representation.
    int size = strlen(psz) * 733 /1000 + 1; // log(58) / log(256), rounded up.
    std::vector<unsigned char> b256(size);
    // Process the characters.
    while (*psz && !isspace(*psz)) {
        // Decode base58 character
        const char* ch = strchr(pszBase58, *psz);
        if (ch == nullptr)
            return false;
        // Apply "b256 = b256 * 58 + ch".
        int carry = ch - pszBase58;
        int i = 0;
        for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        assert(carry == 0);
        length = i;
        psz++;
    }
    // Skip trailing spaces.
    while (isspace(*psz))
        psz++;
    if (*psz != 0)
        return false;
    // Skip leading zeroes in b256.
    std::vector<unsigned char>::iterator it = b256.begin() + (size - length);
    while (it != b256.end() && *it == 0)
        it++;
    // Copy result into output vector.
    vch.reserve(zeroes + (b256.end() - it));
    vch.assign(zeroes, 0x00);
    while (it != b256.end())
        vch.push_back(*(it++));
    return true;
}

std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
{
    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    while (pbegin != pend && *pbegin == 0) {
        pbegin++;
        zeroes++;
    }
    // Allocate enough space in big-endian base58 representation.
    int size = (pend - pbegin) * 138 / 100 + 1; // log(256) / log(58), rounded up.
    std::vector<unsigned char> b58(size);
    // Process the bytes.
    while (pbegin != pend) {
        int carry = *pbegin;
        int i = 0;
        // Apply "b58 = b58 * 256 + ch".
        for (std::vector<unsigned char>::reverse_iterator it = b58.rbegin(); (carry != 0 || i < length) && (it != b58.rend()); it++, i++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }

        assert(carry == 0);
        length = i;
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    std::vector<unsigned char>::iterator it = b58.begin() + (size - length);
    while (it != b58.end() && *it == 0)
        it++;
    // Translate the result into a string.
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, '1');
    while (it != b58.end())
        str += pszBase58[*(it++)];
    return str;
}

std::string EncodeBase58(const std::vector<unsigned char>& vch)
{
    return EncodeBase58(vch.data(), vch.data() + vch.size());
}

bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet)
{
    return DecodeBase58(str.c_str(), vchRet);
}

std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn)
{
    // add 4-byte hash check to the end
    std::vector<unsigned char> vch(vchIn);
    uint256 hash = Hash(vch.begin(), vch.end());
    vch.insert(vch.end(), (unsigned char*)&hash, (unsigned char*)&hash + 4);
    return EncodeBase58(vch);
}

bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet)
{
    if (!DecodeBase58(psz, vchRet) ||
        (vchRet.size() < 4)) {
        vchRet.clear();
        return false;
    }
    // re-calculate the checksum, ensure it matches the included 4-byte checksum
    uint256 hash = Hash(vchRet.begin(), vchRet.end() - 4);
    if (memcmp(&hash, &vchRet.end()[-4], 4) != 0) {
        vchRet.clear();
        return false;
    }
    vchRet.resize(vchRet.size() - 4);
    return true;
}

bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet)
{
    return DecodeBase58Check(str.c_str(), vchRet);
}

CellBase58Data::CellBase58Data()
{
    vchVersion.clear();
    vchData.clear();
}

void CellBase58Data::SetData(const std::vector<unsigned char>& vchVersionIn, const void* pdata, size_t nSize)
{
    vchVersion = vchVersionIn;
    vchData.resize(nSize);
    if (!vchData.empty())
        memcpy(vchData.data(), pdata, nSize);
}

void CellBase58Data::SetData(const std::vector<unsigned char>& vchVersionIn, const unsigned char* pbegin, const unsigned char* pend)
{
    SetData(vchVersionIn, (void*)pbegin, pend - pbegin);
}

bool CellBase58Data::SetString(const char* psz, unsigned int nVersionBytes)
{
    std::vector<unsigned char> vchTemp;
    bool rc58 = DecodeBase58Check(psz, vchTemp);
    if ((!rc58) || (vchTemp.size() < nVersionBytes)) {
        vchData.clear();
        vchVersion.clear();
        return false;
    }
    vchVersion.assign(vchTemp.begin(), vchTemp.begin() + nVersionBytes);
    vchData.resize(vchTemp.size() - nVersionBytes);
    if (!vchData.empty())
        memcpy(vchData.data(), vchTemp.data() + nVersionBytes, vchData.size());
    memory_cleanse(vchTemp.data(), vchTemp.size());
    return true;
}

bool CellBase58Data::SetString(const std::string& str)
{
    return SetString(str.c_str());
}

std::string CellBase58Data::ToString() const
{
    std::vector<unsigned char> vch = vchVersion;
    vch.insert(vch.end(), vchData.begin(), vchData.end());
    return EncodeBase58Check(vch);
}

int CellBase58Data::CompareTo(const CellBase58Data& b58) const
{
    if (vchVersion < b58.vchVersion)
        return -1;
    if (vchVersion > b58.vchVersion)
        return 1;
    if (vchData < b58.vchData)
        return -1;
    if (vchData > b58.vchData)
        return 1;
    return 0;
}

namespace
{
class MagnaChainAddressVisitor : public boost::static_visitor<bool>
{
private:
    MagnaChainAddress* addr;

public:
    MagnaChainAddressVisitor(MagnaChainAddress* addrIn) : addr(addrIn) {}

    bool operator()(const CellContractID& id) const { return addr->Set(id); }
    bool operator()(const CellKeyID& id) const { return addr->Set(id); }
    bool operator()(const CellScriptID& id) const { return addr->Set(id); }
    bool operator()(const CellNoDestination& no) const { return false; }
};

} // namespace

bool MagnaChainAddress::Set(const CellContractID& id)
{
    SetData(Params().Base58Prefix(CellChainParams::CONTRACT_ADDRESS), &id, 20);
    return true;
}

bool MagnaChainAddress::Set(const CellKeyID& id)
{
    SetData(Params().Base58Prefix(CellChainParams::PUBKEY_ADDRESS), &id, 20);
    return true;
}

bool MagnaChainAddress::Set(const CellScriptID& id)
{
    SetData(Params().Base58Prefix(CellChainParams::SCRIPT_ADDRESS), &id, 20);
    return true;
}

bool MagnaChainAddress::Set(const CellTxDestination& dest)
{
    return boost::apply_visitor(MagnaChainAddressVisitor(this), dest);
}

bool MagnaChainAddress::IsValid() const
{
    return IsValid(Params());
}

bool MagnaChainAddress::IsValid(const CellChainParams& params) const
{
    bool fCorrectSize = vchData.size() == 20;
    bool fKnownVersion = vchVersion == params.Base58Prefix(CellChainParams::PUBKEY_ADDRESS) ||
        vchVersion == params.Base58Prefix(CellChainParams::SCRIPT_ADDRESS) ||
        vchVersion == params.Base58Prefix(CellChainParams::CONTRACT_ADDRESS);
    return fCorrectSize && fKnownVersion;
}

CellTxDestination MagnaChainAddress::Get() const
{
    if (!IsValid())
        return CellNoDestination();
    uint160 id;
    memcpy(&id, vchData.data(), 20);
    if (vchVersion == Params().Base58Prefix(CellChainParams::PUBKEY_ADDRESS))
        return CellKeyID(id);
    else if (vchVersion == Params().Base58Prefix(CellChainParams::SCRIPT_ADDRESS))
        return CellScriptID(id);
    else if (vchVersion == Params().Base58Prefix(CellChainParams::CONTRACT_ADDRESS))
        return CellContractID(id);
    else
        return CellNoDestination();
}

bool MagnaChainAddress::GetContractID(CellContractID& contractID) const
{
    if (!IsValid() || vchVersion != Params().Base58Prefix(CellChainParams::CONTRACT_ADDRESS))
        return false;
    uint160 id;
    memcpy(&id, vchData.data(), 20);
    contractID = CellContractID(id);
    return true;
}

bool MagnaChainAddress::GetKeyID(CellKeyID& keyID) const
{
    if (!IsValid() || vchVersion != Params().Base58Prefix(CellChainParams::PUBKEY_ADDRESS))
        return false;
    uint160 id;
    memcpy(&id, vchData.data(), 20);
    keyID = CellKeyID(id);
    return true;
}

bool MagnaChainAddress::IsContractID() const
{
    return IsValid() && vchVersion == Params().Base58Prefix(CellChainParams::CONTRACT_ADDRESS);
}

bool MagnaChainAddress::IsScript() const
{
    return IsValid() && vchVersion == Params().Base58Prefix(CellChainParams::SCRIPT_ADDRESS);
}

void MagnaChainSecret::SetKey(const CellKey& vchSecret)
{
    assert(vchSecret.IsValid());
    SetData(Params().Base58Prefix(CellChainParams::SECRET_KEY), vchSecret.begin(), vchSecret.size());
    if (vchSecret.IsCompressed())
        vchData.push_back(1);
}

CellKey MagnaChainSecret::GetKey()
{
    CellKey ret;
    assert(vchData.size() >= 32);
    ret.Set(vchData.begin(), vchData.begin() + 32, vchData.size() > 32 && vchData[32] == 1);
    return ret;
}

bool MagnaChainSecret::IsValid() const
{
    bool fExpectedFormat = vchData.size() == 32 || (vchData.size() == 33 && vchData[32] == 1);
    bool fCorrectVersion = vchVersion == Params().Base58Prefix(CellChainParams::SECRET_KEY);
    return fExpectedFormat && fCorrectVersion;
}

bool MagnaChainSecret::SetString(const char* pszSecret)
{
    return CellBase58Data::SetString(pszSecret) && IsValid();
}

bool MagnaChainSecret::SetString(const std::string& strSecret)
{
    return SetString(strSecret.c_str());
}
