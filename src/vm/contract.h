// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef CONTRACT_H
#define CONTRACT_H

#include "key/pubkey.h"

class MCWallet;
class MagnaChainAddress;

class ContractContext
{
public:
    uint256 blockHash;
    int txIndex;
    MCAmount coins;
    std::string code;
    std::string data;

    ContractContext() : txIndex(-1), coins(0)
    {
    }

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(blockHash);
        READWRITE(txIndex);
        READWRITE(coins);
        READWRITE(code);
        READWRITE(data);
    }
};
typedef std::map<MCContractID, ContractContext> MapContractContext;

class SimpleContractContext
{
public:
    uint256 blockHash;
    int txIndex;
    uint256 prevDataHash;
    std::vector<uint256> prevDataHashes;
    MCPartialMerkleTree prevDataPmt;
    uint256 finalDataHash;

    SimpleContractContext() : txIndex(-1)
    {
    }

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(blockHash);
        READWRITE(txIndex);
        READWRITE(prevDataHash);
        READWRITE(prevDataHashes);
        READWRITE(prevDataPmt);
        READWRITE(finalDataHash);
    }
};
typedef std::map<MCContractID, SimpleContractContext> MapSimpleContractContext;

class ReportedContractData
{
public:
    std::vector<MapSimpleContractContext> prevData;
    std::vector<uint256> finalData;

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(prevData);
        READWRITE(finalData);
    }
};

//temp contract address for publish
template<typename T>
MCContractID GenerateContractAddressByTx(T& tx)
{
    MCHashWriter ss(SER_GETHASH, 0);
    for (const auto& v : tx.vin) {
        ss << v.prevout << v.nSequence << v.scriptWitness.ToString();
    }
    for (const auto& v : tx.vout) {
        ss << v.nValue << v.scriptPubKey;
    }

    ss << tx.pContractData->codeOrFunc << tx.pContractData->sender;
    return MCContractID(Hash160(ParseHex(ss.GetHash().ToString())));
}

uint256 GetContextHash(const ContractContext& context);
uint256 GetContractContextHash(const MCContractID& contractId, const uint256& blockHash, int txIndex, const uint256& contextHash);
bool GetSenderAddr(MCWallet* pWallet, const std::string& strSenderAddr, MagnaChainAddress& senderAddr);
MCContractID GenerateContractAddress(MCWallet* pWallet, const MagnaChainAddress& senderAddr, const std::string& code);
uint256 GetHashWithMapContractContext(const MapContractContext& contractData);

#endif
