#include "coding/base58.h"
#include "consensus/consensus.h"
#include "utils/util.h"
#include "vm/contract.h"
#include "wallet/wallet.h"
#include "validation/validation.h"

uint256 GetContextHash(const ContractContext& context)
{
    MCHashWriter ss(SER_GETHASH, 0);
    ss << context.coins << context.code << context.data;
    return ss.GetHash();
}

uint256 GetContractContextHash(const MCContractID& contractId, const uint256& blockHash, int txIndex, const uint256& contextHash)
{
    MCHashWriter ss(SER_GETHASH, 0);
    ss << contractId << blockHash << txIndex << contextHash;
    return ss.GetHash();
}

bool GetPubKey(const MCWallet* pWallet, const MagnaChainAddress& addr, MCPubKey& pubKey)
{
    MCKeyID key;
    if (!addr.GetKeyID(key))
        return false;

    return pWallet->GetPubKey(key, pubKey);
}

bool GenerateContractSender(MCWallet* pWallet, MagnaChainAddress& sendAddr)
{
    MCPubKey newKey;
    if (!pWallet->GetKeyFromPool(newKey)) {
        return false;
    }
    MCKeyID keyID = newKey.GetID();
    sendAddr.Set(keyID);
    return true;
}

bool GetSenderAddr(MCWallet* pWallet, const std::string& strSenderAddr, MagnaChainAddress& senderAddr)
{
    MCKeyID key;
    bool ret = false;
    if (!strSenderAddr.empty()) {
        senderAddr.SetString(strSenderAddr);
        senderAddr.GetKeyID(key);
        ret = pWallet->HaveKey(key);
    }
    else {
        if (pWallet->_senderAddr.IsValid()) {
            senderAddr = pWallet->_senderAddr;
            senderAddr.GetKeyID(key);
            ret = pWallet->HaveKey(key);
        }

        if (!ret && GenerateContractSender(pWallet, senderAddr)) {
            senderAddr.GetKeyID(key);
            ret = pWallet->HaveKey(key);
        }
    }

    if (ret) {
        pWallet->_senderAddr = senderAddr;
    }

    return ret;
}

// generate contract address
// format: sender address keyid + block address + new magnachain address + contract script file hash
MCContractID GenerateContractAddress(MCWallet* pWallet, const MagnaChainAddress& senderAddr, const std::string& code)
{
    MCHashWriter ss(SER_GETHASH, 0);

    // sender address keyid
    MCKeyID senderId;
    senderAddr.GetKeyID(senderId);
    ss << senderId;

    // block address
    std::string blockAddress;
    if (chainActive.Height() < COINBASE_MATURITY)
        blockAddress = chainActive.Tip()->GetBlockHash().GetHex();
    else
        blockAddress = chainActive[chainActive.Height() - COINBASE_MATURITY]->GetBlockHash().GetHex();
    ss << blockAddress;

    // new magnachain address
    if (pWallet != nullptr) {
        MCPubKey newKey;
        if (!pWallet->GetKeyFromPool(newKey))
            throw std::runtime_error(strprintf("%s:%d Keypool ran out, please call keypoolrefill first", __FILE__, __LINE__));
        ss << newKey.GetID();
    }
    else {
        // sdk用户没有钱包在本地
        ss << GetTimeMillis();
        ss << (int64_t)(&senderAddr); // get random value by address point
    }

    // contract script file hash
    ss << Hash(code.begin(), code.end()).GetHex();
    return MCContractID(Hash160(ParseHex(ss.GetHash().ToString())));
}

uint256 GetHashWithMapContractContext(const MapContractContext& contractData)
{
    MCHashWriter ss(SER_GETHASH, 0);
    for (const auto& item : contractData) {
        ss << GetContractContextHash(item.first, item.second.blockHash, item.second.txIndex, GetContextHash(item.second));
    }
    return ss.GetHash();
}