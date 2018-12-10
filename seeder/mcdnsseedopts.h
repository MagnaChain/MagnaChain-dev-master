#ifndef MCDNSSEEDOPTS_H_
#define MCDNSSEEDOPTS_H_

#include <set>
#include <vector>
#include <string>

using namespace std;

class MCDnsSeedOpts {
public:
    MCDnsSeedOpts();
    MCDnsSeedOpts(const MCDnsSeedOpts& left);

    int nThreads;//Number of crawlers to run in parallel (default 96)
    int nPort;// dns UDP port 
    int nDnsThreads;// dns
    int fUseTestNet;
    int fWipeBan;
    int fWipeIgnore;
    char *mbox; // TODO: if have time, these three char* member need to change imp
    char *ns;
    char *host;
    string tor;
    string ipv4_proxy;
    string ipv6_proxy;
    std::set<uint64_t> filter_whitelist;
    std::vector<string> seeds;

    string branchid;
    unsigned short defaultport;
    unsigned char pchMessageStart[4];

    void InitMessageStart();
    void PrintWhitelistFilter();
};

#endif