#include "mcdnsseedopts.h"

MCDnsSeedOpts::MCDnsSeedOpts() :
    nThreads(96), nDnsThreads(4), nPort(53), mbox(NULL), ns(NULL), host(NULL), tor(NULL)
    ,fUseTestNet(false), fWipeBan(false), fWipeIgnore(false), ipv4_proxy(NULL), ipv6_proxy(NULL)
    ,defaultport(8833)
{
}
