#ifndef MAGNACHAIN_H
#define MAGNACHAIN_H

#include "protocol.h"

bool TestNode(const MCService &cip, int &ban, int &client, std::string &clientSV, int &blocks, std::vector<MCAddress>* vAddr, const std::string &strBranchId, unsigned char* pchMessageStart);

#endif
