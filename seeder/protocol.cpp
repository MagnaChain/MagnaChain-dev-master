// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 The Bitcoin developers
// Copyright (c) 2016-2018 The CellLink developers
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

std::string gBranchId;

static const char* ppszTypeName[] =
{
    "ERROR",
    "tx",
    "block",
};

unsigned char pchMessageStart[4] = { 0xce, 0x11, 0x16, 0x89 };

CellMessageHeader::CellMessageHeader()
{
    memcpy(pchMessageStart, ::pchMessageStart, sizeof(pchMessageStart));
    memset(pchCommand, 0, sizeof(pchCommand));
    pchCommand[1] = 1;
    nMessageSize = -1;
    nChecksum = 0;
}

CellMessageHeader::CellMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn)
{
    memcpy(pchMessageStart, ::pchMessageStart, sizeof(pchMessageStart));
    strncpy(pchCommand, pszCommand, COMMAND_SIZE);
    nMessageSize = nMessageSizeIn;
    nChecksum = 0;
}

std::string CellMessageHeader::GetCommand() const
{
    if (pchCommand[COMMAND_SIZE-1] == 0)
        return std::string(pchCommand, pchCommand + strlen(pchCommand));
    else
        return std::string(pchCommand, pchCommand + COMMAND_SIZE);
}

bool CellMessageHeader::IsValid() const
{
    // Check start string
    if (memcmp(pchMessageStart, ::pchMessageStart, sizeof(pchMessageStart)) != 0)
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
        printf("CellMessageHeader::IsValid() : (%s, %u bytes) nMessageSize > MAX_SIZE\n", GetCommand().c_str(), nMessageSize);
        return false;
    }

    return true;
}



CellAddress::CellAddress() : CellService()
{
    Init();
}

CellAddress::CellAddress(CellService ipIn, uint64 nServicesIn) : CellService(ipIn)
{
    Init();
    nServices = nServicesIn;
}

void CellAddress::Init()
{
    nServices = NODE_NETWORK;
    nTime = 100000000;
}

void CellAddress::print() const
{
    printf("CellAddress(%s)\n", ToString().c_str());
}

CellInv::CellInv()
{
    type = 0;
    hash = 0;
}

CellInv::CellInv(int typeIn, const uint256& hashIn)
{
    type = typeIn;
    hash = hashIn;
}

CellInv::CellInv(const std::string& strType, const uint256& hashIn)
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
        throw std::out_of_range("CellInv::CellInv(string, uint256) : unknown type");
    hash = hashIn;
}

bool operator<(const CellInv& a, const CellInv& b)
{
    return (a.type < b.type || (a.type == b.type && a.hash < b.hash));
}

bool CellInv::IsKnownType() const
{
    return (type >= 1 && type < ARRAYLEN(ppszTypeName));
}

const char* CellInv::GetCommand() const
{
    if (!IsKnownType())
        throw std::out_of_range("CellInv::GetCommand() : unknown type");
    return ppszTypeName[type];
}

std::string CellInv::ToString() const
{
    return "CellInv()";
}

void CellInv::print() const
{
    printf("CellInv\n");
}
