#ifndef CELLLINK_H
#define CELLLINK_H

#include "protocol.h"

bool TestNode(const CellService &cip, int &ban, int &client, std::string &clientSV, int &blocks, std::vector<CellAddress>* vAddr);

#endif
