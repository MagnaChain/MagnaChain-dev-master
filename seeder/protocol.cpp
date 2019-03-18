// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 The Bitcoin developers
// Copyright (c) 2016-2018 The MagnaChain developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <stdexcept>

#include "protocol.h"
#include "util.h"
#include "netbase.h"


#ifndef WIN32
# include <arpa/inet.h>
#endif

static const char* ppszTypeName[] =
{
    "ERROR",
    "tx",
    "block",
};

MCMessageHeader::MCMessageHeader(unsigned char* pchMsgStart)
{
    //memcpy(pchMessageStart, pchMessageStart, sizeof(pchMessageStart));
    pchMessageStart[0] = pchMsgStart[0];
    pchMessageStart[1] = pchMsgStart[1];
    pchMessageStart[2] = pchMsgStart[2];
    pchMessageStart[3] = pchMsgStart[3];

    memset(pchCommand, 0, sizeof(pchCommand));
    pchCommand[1] = 1;
    nMessageSize = -1;
    nChecksum = 0;
}

MCMessageHeader::MCMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn, unsigned char* pchMsgStart)
{
    //memcpy(pchMessageStart, pchMessageStart, sizeof(pchMessageStart));
    pchMessageStart[0] = pchMsgStart[0];
    pchMessageStart[1] = pchMsgStart[1];
    pchMessageStart[2] = pchMsgStart[2];
    pchMessageStart[3] = pchMsgStart[3];

    strncpy(pchCommand, pszCommand, COMMAND_SIZE);
    nMessageSize = nMessageSizeIn;
    nChecksum = 0;
}

std::string MCMessageHeader::GetCommand() const
{
    if (pchCommand[COMMAND_SIZE-1] == 0)
        return std::string(pchCommand, pchCommand + strlen(pchCommand));
    else
        return std::string(pchCommand, pchCommand + COMMAND_SIZE);
}

bool MCMessageHeader::IsValid() const
{
    // Check start string
    if (memcmp(pchMessageStart, pchMessageStart, sizeof(pchMessageStart)) != 0)
        return false;

    // Check the command string for errors
    for (const char* p1 = pchCommand; p1 < pchCommand + COMMAND_SIZE; p1++)
    {
        if (*p1 == 0)
        {
            // Must be all zeros after the first zero
            for (; p1 < pchCommand + COMMAND_SIZE; p1++)
                if (*p1 != 0)
                    return false;
        }
        else if (*p1 < ' ' || *p1 > 0x7E)
            return false;
    }

    // Message size
    if (nMessageSize > MAX_SIZE)
    {
        printf("MCMessageHeader::IsValid() : (%s, %u bytes) nMessageSize > MAX_SIZE\n", GetCommand().c_str(), nMessageSize);
        return false;
    }

    return true;
}



MCAddress::MCAddress() : MCService()
{
    Init();
}

MCAddress::MCAddress(MCService ipIn, uint64 nServicesIn) : MCService(ipIn)
{
    Init();
    nServices = nServicesIn;
}

void MCAddress::Init()
{
    nServices = NODE_NETWORK;
    nTime = 100000000;
}

void MCAddress::print() const
{
    printf("MCAddress(%s)\n", ToString().c_str());
}

MCInv::MCInv()
{
    type = 0;
    hash = 0;
}

MCInv::MCInv(int typeIn, const uint256& hashIn)
{
    type = typeIn;
    hash = hashIn;
}

MCInv::MCInv(const std::string& strType, const uint256& hashIn)
{
    int i;
    for (i = 1; i < ARRAYLEN(ppszTypeName); i++)
    {
        if (strType == ppszTypeName[i])
        {
            type = i;
            break;
        }
    }
    if (i == ARRAYLEN(ppszTypeName))
        throw std::out_of_range("MCInv::MCInv(string, uint256) : unknown type");
    hash = hashIn;
}

bool operator<(const MCInv& a, const MCInv& b)
{
    return (a.type < b.type || (a.type == b.type && a.hash < b.hash));
}

bool MCInv::IsKnownType() const
{
    return (type >= 1 && type < ARRAYLEN(ppszTypeName));
}

const char* MCInv::GetCommand() const
{
    if (!IsKnownType())
        throw std::out_of_range("MCInv::GetCommand() : unknown type");
    return ppszTypeName[type];
}

std::string MCInv::ToString() const
{
    return "MCInv()";
}

void MCInv::print() const
{
    printf("MCInv\n");
}
