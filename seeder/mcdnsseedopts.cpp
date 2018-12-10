#include "mcdnsseedopts.h"

MCDnsSeedOpts::MCDnsSeedOpts() :
    nThreads(96), nDnsThreads(4), nPort(53), mbox(NULL), ns(NULL), host(NULL)
    ,fUseTestNet(false), fWipeBan(false), fWipeIgnore(false)
    ,defaultport(8833)
{
    this->pchMessageStart[0] = 0;
    this->pchMessageStart[1] = 0;
    this->pchMessageStart[2] = 0;
    this->pchMessageStart[3] = 0;
}

MCDnsSeedOpts::MCDnsSeedOpts(const MCDnsSeedOpts& left)
    :nThreads(left.nThreads),nPort(left.nPort),nDnsThreads(left.nDnsThreads),
    fUseTestNet(left.fUseTestNet),fWipeBan(left.fWipeBan),fWipeIgnore(left.fWipeIgnore),
    filter_whitelist(left.filter_whitelist), seeds(left.seeds), branchid(left.branchid),
    defaultport(left.defaultport), tor(left.tor), ipv4_proxy(left.ipv4_proxy), ipv6_proxy(left.ipv6_proxy)
{
    //char pointer
    mbox = left.mbox;
    ns = left.ns;
    host = left.host;

    this->pchMessageStart[0] = left.pchMessageStart[0];
    this->pchMessageStart[1] = left.pchMessageStart[1];
    this->pchMessageStart[2] = left.pchMessageStart[2];
    this->pchMessageStart[3] = left.pchMessageStart[3];
}

void MCDnsSeedOpts::InitMessageStart()
{
    if (this->branchid == "main" && !this->fUseTestNet) {//main
        this->pchMessageStart[0] = 0xce;
        this->pchMessageStart[1] = 0x11;
        this->pchMessageStart[2] = 0x16;
        this->pchMessageStart[3] = 0x89;
    }
    if (this->branchid == "main" && this->fUseTestNet)
    {
        printf("Using testnet.\n");//testnet
        this->pchMessageStart[0] = 0xce;
        this->pchMessageStart[1] = 0x11;
        this->pchMessageStart[2] = 0x09;
        this->pchMessageStart[3] = 0x07;
    }

    if (this->branchid != "main") {//branch main net and test net use same header.
        this->pchMessageStart[0] = 0xce;
        this->pchMessageStart[1] = 0x11;
        this->pchMessageStart[2] = 0x68;
        this->pchMessageStart[3] = 0x99;
    }
}

void MCDnsSeedOpts::PrintWhitelistFilter()
{
    printf("Supporting whitelisted filters: ");
    for (std::set<uint64_t>::const_iterator it = this->filter_whitelist.begin(); it != this->filter_whitelist.end(); it++) {
        if (it != this->filter_whitelist.begin()) {
            printf(",");
        }
        printf("0x%lx", (unsigned long)*it);
    }
    printf("\n");
}