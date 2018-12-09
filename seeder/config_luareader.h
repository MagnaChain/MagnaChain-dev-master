#ifndef CONFIG_LUAREADER_H_
#define CONFIG_LUAREADER_H_

#include <vector>
#include "mcdnsseedopts.h"

using namespace std;

class ConfigLuaReader
{
public:
    vector<MCDnsSeedOpts> ReadConfig(const char* filename);
};

#endif // !CONFIG-LUAREADER_H_