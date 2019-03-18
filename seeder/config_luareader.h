#ifndef CONFIG_LUAREADER_H_
#define CONFIG_LUAREADER_H_

#include <vector>
#include "mcdnsseedopts.h"

using namespace std;

class ConfigLuaReader
{
public:
    bool ReadConfig(const char* filename, vector<MCDnsSeedOpts*> &vecOpts);
};

#endif // !CONFIG-LUAREADER_H_