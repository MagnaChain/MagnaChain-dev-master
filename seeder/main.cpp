#include <algorithm>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <atomic>
#include <map>

#include "magnachain.h"
#include "db.h"

extern "C" {
#include "dns.h"
}

#include "mcdnsseedopts.h"
#include "config_luareader.h"

static const std::string strDefaultOpts = "defaultoptions";

using namespace std;

bool fTestNet = false;

typedef std::map<std::string, MCAddrDB> MAP_BRANCH_DB;
typedef std::map<std::string, MCAddrDB> MAP_HOST_DB;
MAP_BRANCH_DB g_mapBranchDB;
MAP_HOST_DB g_mapHostDB;

const char* g_configgilename = "config.lua";

void ParseCommandLine(int argc, char **argv, MCDnsSeedOpts* opts) {

    static const char *help = "MagnaChain-seeder\n"
        "Usage: %s -h <host> -n <ns> [-m <mbox>] [-t <threads>] [-p <port>]\n"
        "\n"
        "Options:\n"
        "-h <host>       Hostname of the DNS seed\n"
        "-f <filename>   Config file for many branchs's options\n"
        //"-n <ns>         Hostname of the nameserver\n"
        //"-m <mbox>       E-Mail address reported in SOA records\n"
        //"-t <threads>    Number of crawlers to run in parallel (default 96)\n"
        "-d <threads>    Number of DNS server threads (default 4)\n"
        "-p <port>       UDP port to listen on (default 53)\n"
        "-o <ip:port>    Tor proxy IP/Port\n"
        "-i <ip:port>    IPV4 SOCKS5 proxy IP/Port\n"
        "-k <ip:port>    IPV6 SOCKS5 proxy IP/Port\n"
        "-w f1,f2,...    Allow these flag combinations as filters\n"
        //"-b <branchid>   The branch id\n"
        //"-s <seeds>	   The seeds of the chain\n"
        "--testnet       Use testnet\n"
        "--wipeban       Wipe list of banned nodes\n"
        "--wipeignore    Wipe list of ignored nodes\n"
        "-?, --help      Show this text\n"
        "\n";
    bool showHelp = false;
    MCDnsSeedOpts* thisopt = opts;
    while (1) {
        static struct option long_options[] = {
        { "host", required_argument, 0, 'h' },
        { "conffile", required_argument, 0, 'f' },
        { "ns",   required_argument, 0, 'n' },
        { "mbox", required_argument, 0, 'm' },
        { "threads", required_argument, 0, 't' },
        { "dnsthreads", required_argument, 0, 'd' },
        { "port", required_argument, 0, 'p' },
        { "onion", required_argument, 0, 'o' },
        { "proxyipv4", required_argument, 0, 'i' },
        { "proxyipv6", required_argument, 0, 'k' },
        { "filter", required_argument, 0, 'w' },
        { "branchid", required_argument, 0, 'b' },
        { "seeds", required_argument, 0, 's' },
        { "testnet", no_argument, &(thisopt->fUseTestNet), 1 },
        { "wipeban", no_argument, &(thisopt->fWipeBan), 1 },
        { "wipeignore", no_argument, &(thisopt->fWipeBan), 1 },
        { "help", no_argument, 0, 'h' },
        { 0, 0, 0, 0 }
        };
        int option_index = 0;
        int c = getopt_long(argc, argv, "h:f:n:m:t:p:d:o:i:k:w:b:s:", long_options, &option_index);
        if (c == -1) break;
        switch (c) {
        case 'h': {
            thisopt->host = optarg;
            break;
        }
        case 'f': {
            g_configgilename = optarg;
            break;
        }

        case 'm': {
            thisopt->mbox = optarg;
            break;
        }

        case 'n': {
            thisopt->ns = optarg;
            break;
        }

        case 't': {
            int n = strtol(optarg, NULL, 10);
            if (n > 0 && n < 1000) thisopt->nThreads = n;
            break;
        }

        case 'd': {
            int n = strtol(optarg, NULL, 10);
            if (n > 0 && n < 1000) thisopt->nDnsThreads = n;
            break;
        }

        case 'p': {
            int p = strtol(optarg, NULL, 10);
            if (p > 0 && p < 65536) thisopt->nPort = p;
            break;
        }

        case 'o': {
            thisopt->tor = optarg;
            break;
        }

        case 'i': {
            thisopt->ipv4_proxy = optarg;
            break;
        }

        case 'k': {
            thisopt->ipv6_proxy = optarg;
            break;
        }

        case 'w': {
            char* ptr = optarg;
            while (*ptr != 0) {
                unsigned long l = strtoul(ptr, &ptr, 0);
                if (*ptr == ',') {
                    ptr++;
                }
                else if (*ptr != 0) {
                    break;
                }
                thisopt->filter_whitelist.insert(l);
            }
            break;
        }

        case 'b': {
            thisopt->branchid = optarg;
            break;
        }

        case 's': {
            char* p = nullptr;
            char* ptr = optarg;
#ifdef _WIN32
            char* seed = strtok_s(ptr, ",", &p);
#else
            char* seed = strtok_r(ptr, ",", &p);
#endif
            while (seed != nullptr) {
                thisopt->seeds.emplace_back(seed);
#ifdef _WIN32
                seed = strtok_s(nullptr, ",", &p);
#else
                seed = strtok_r(nullptr, ",", &p);
#endif
            }
            break;
        }

        case '?': {
            showHelp = true;
            break;
        }
        }
    }
    if (thisopt->filter_whitelist.empty()) {
        thisopt->filter_whitelist.insert(1);
        thisopt->filter_whitelist.insert(5);
        thisopt->filter_whitelist.insert(9);
        thisopt->filter_whitelist.insert(13);
    }
    if (thisopt->host != NULL && thisopt->ns == NULL) showHelp = true;
    if (showHelp) fprintf(stderr, help, argv[0]);
}

void AddNewDB(MCAddrDB &db)
{
    g_mapBranchDB[db.pOpts->branchid] = db;
    g_mapHostDB[db.pOpts->host] = db;
}

extern "C" void* ThreadCrawler(void* data) {
  MCAddrDB* pDB = (MCAddrDB*)data;
  RenameThread(strprintf("Crawler%s", pDB->pOpts->branchid.substr(0, 8).c_str()).c_str());
  MCDnsSeedOpts* pOpts = pDB->pOpts;
  int nThreads = pOpts->nThreads;
  do {
    std::vector<MCServiceResult> ips;
    int wait = 5;
    pDB->GetMany(ips, 16, wait);
    int64 now = time(NULL);
    if (ips.empty()) {
      wait *= 1000;
      wait += rand() % (500 * nThreads);
      Sleep(wait);
      continue;
    }
    vector<MCAddress> addr;
    for (int i=0; i<ips.size(); i++) {
      MCServiceResult &res = ips[i];
      res.nBanTime = 0;
      res.nClientV = 0;
      res.nHeight = 0;
      res.strClientV = "";
      bool getaddr = res.ourLastSuccess + 86400 < now;
      res.fGood = TestNode(res.service, res.nBanTime, res.nClientV, res.strClientV, res.nHeight, getaddr ? &addr : NULL, pOpts->branchid, pOpts->pchMessageStart);
    }
    pDB->ResultMany(ips);
    pDB->Add(addr);
  } while(1);
  return nullptr;
}

extern "C" int GetIPList(void *thread, char *requestedHostname, addr_t *addr, int max, int ipv4, int ipv6);

void SetDNS_opt_by(dns_opt_t &dns_opt, MCDnsSeedOpts* opts)
{
    dns_opt.host = opts->host;
    dns_opt.ns = opts->ns;
    dns_opt.mbox = opts->mbox;
    dns_opt.datattl = 3600;
    dns_opt.nsttl = 40000;
    dns_opt.cb = GetIPList;
    dns_opt.port = opts->nPort;
    dns_opt.nRequests = 0;
}

extern "C" bool CheckRequestName(char *name, dns_opt_t *opt)
{
    for (auto& mi : g_mapHostDB)
    {
        const char* host = mi.first.c_str();
        int namel = strlen(name), hostl = strlen(host);
        if (strcasecmp(name, host) && (namel < hostl + 2 || name[namel - hostl - 1] != '.' || strcasecmp(name + namel - hostl, host)))
        {
            // fail one
        }
        else {
            SetDNS_opt_by(*opt, mi.second.pOpts);
            return true;
        }
    }

    return false;//all fail
}

class MCDnsThread {
public:
  struct FlagSpecificData {
      int nIPv4, nIPv6;
      std::vector<addr_t> cache;
      time_t cacheTime;
      unsigned int cacheHits;
      FlagSpecificData() : nIPv4(0), nIPv6(0), cacheTime(0), cacheHits(0) {}
  };

  dns_opt_t dns_opt; // must be first, GetIPList will trans dns_opt* to MCDnsThread*
  const int id;
  std::map<uint64_t, FlagSpecificData> perflag;
  std::atomic<uint64_t> dbQueries;
  std::set<uint64_t> filterWhitelist;

  void cacheHit(uint64_t requestedFlags, MCAddrDB* pDB, bool force = false) {
    static bool nets[NET_MAX] = {};
    if (!nets[NET_IPV4]) {
        nets[NET_IPV4] = true;
        nets[NET_IPV6] = true;
    }
    time_t now = time(NULL);
    FlagSpecificData& thisflag = perflag[requestedFlags];
    thisflag.cacheHits++;
    if (force || thisflag.cacheHits * 400 > (thisflag.cache.size()*thisflag.cache.size()) || (thisflag.cacheHits*thisflag.cacheHits * 20 > thisflag.cache.size() && (now - thisflag.cacheTime > 5))) {
      set<MCNetAddr> ips;
      pDB->GetIPs(ips, requestedFlags, 1000, nets);
      dbQueries++;
      thisflag.cache.clear();
      thisflag.nIPv4 = 0;
      thisflag.nIPv6 = 0;
      thisflag.cache.reserve(ips.size());
      for (set<MCNetAddr>::iterator it = ips.begin(); it != ips.end(); it++) {
        struct in_addr addr;
        struct in6_addr addr6;
        if ((*it).GetInAddr(&addr)) {
          addr_t a;
          a.v = 4;
          memcpy(&a.data.v4, &addr, 4);
          thisflag.cache.push_back(a);
          thisflag.nIPv4++;
        } else if ((*it).GetIn6Addr(&addr6)) {
          addr_t a;
          a.v = 6;
          memcpy(&a.data.v6, &addr6, 16);
          thisflag.cache.push_back(a);
          thisflag.nIPv6++;
        }
      }
      thisflag.cacheHits = 0;
      thisflag.cacheTime = now;
    }
  }

  MCDnsThread(MCAddrDB* pdb, int idIn) : id(idIn) { 
    MCDnsSeedOpts* opts = pdb->pOpts;
    SetDNS_opt_by(dns_opt, opts);
    dbQueries = 0;
    perflag.clear();
    filterWhitelist = opts->filter_whitelist;
  }

  void run() {
    dnsserver(&dns_opt);//TODO:这里必须是第一个变量dns_opt，后面有代码将该指针转换成MCDnsThread
  }
};


bool MCNetAddr2addr_t(MCNetAddr &na, addr_t& a)
{
    a.v = 0;// ?
    struct in_addr addr;
    struct in6_addr addr6;
    if (na.GetInAddr(&addr)) {
        a.v = 4;
        memcpy(&a.data.v4, &addr, 4);
        return true;
    }
    else if (na.GetIn6Addr(&addr6)) {
        a.v = 6;
        memcpy(&a.data.v6, &addr6, 16);
        return true;
    }
    return false;
}

extern "C" int GetIPList(void *data, char *requestedHostname, addr_t* addr, int max, int ipv4, int ipv6) {
  MCDnsThread *thread = (MCDnsThread*)data;

  std::string strReqHostName = requestedHostname;
  MCAddrDB* pDB = nullptr;
  if (g_mapHostDB.count(strReqHostName))
  {
      pDB = &g_mapHostDB[strReqHostName];
  }

  uint64_t requestedFlags = 0;
  int hostlen = strlen(requestedHostname);
  if (hostlen > 1 && requestedHostname[0] == 'x' && requestedHostname[1] != '0') {//TODO: if true ,pDB is nullpr. Zhe li zai zuo shen me a
    char *pEnd;
    uint64_t flags = (uint64_t)strtoull(requestedHostname+1, &pEnd, 16);
    if (*pEnd == '.' && pEnd <= requestedHostname+17 && std::find(thread->filterWhitelist.begin(), thread->filterWhitelist.end(), flags) != thread->filterWhitelist.end()){
      requestedFlags = flags;
    }
    else
      return 0;
  }
  else if (pDB== nullptr)//(strcasecmp(requestedHostname, thread->dns_opt.host))// HasModify
    return 0;

  //TODO: 把上面的if（334行）改好后,把这里if return 去掉
  if (pDB == nullptr)
  {
      return 0;
  }

  thread->cacheHit(requestedFlags, pDB, false);

  auto& thisflag = thread->perflag[requestedFlags];
  unsigned int size = thisflag.cache.size();
  unsigned int maxmax = (ipv4 ? thisflag.nIPv4 : 0) + (ipv6 ? thisflag.nIPv6 : 0);
  if (max > size)
    max = size;
  if (max > maxmax)
    max = maxmax;
  int i=0;
  while (i<max) {
    int j = i + (rand() % (size - i));
    do {
        bool ok = (ipv4 && thisflag.cache[j].v == 4) ||
                  (ipv6 && thisflag.cache[j].v == 6);
        if (ok) break;
        j++;
        if (j==size)
            j=i;
    } while(1);
    addr[i] = thisflag.cache[j];
    thisflag.cache[j] = thisflag.cache[i];
    thisflag.cache[i] = addr[i];
    i++;
  }
  //force add
  if (size == 0)
  {
      for (const std::string& seed : pDB->pOpts->seeds /*int i = 0; i < pDB->pOpts->seeds.size(); i++*/) {
          vector<MCNetAddr> ips;
          LookupHost(seed.c_str(), ips);
          for (vector<MCNetAddr>::iterator it = ips.begin(); it != ips.end(); it++) {
              if (MCNetAddr2addr_t(*it, addr[0]))
              {
                  printf("force add ip,ignore isgood.%s\n", seed.c_str());
                  return 1;
              }
              
          }
      }
  }
  return max;
}

vector<MCDnsThread*> dnsThread;

extern "C" void* ThreadDNS(void* arg) {
  RenameThread("ThreadDNS");
  MCDnsThread *thread = (MCDnsThread*)arg;
  thread->run();
  return nullptr;
}

extern "C" void* ThreadDumper(void*pData) {
  MCAddrDB* pDB = (MCAddrDB*)pData;
  RenameThread(strprintf("Dumper_%s", pDB->pOpts->branchid.substr(0, 8).c_str()).c_str());
  int count = 0;
  do {
    Sleep(100000 << count); // First 100s, than 200s, 400s, 800s, 1600s, and then 3200s forever
    if (count < 5)
        count++;
    {
        pDB->SaveDBData();
    }
  } while(1);
  return nullptr;
}

extern "C" void* ThreadStats(void*pData) {
  MCAddrDB* pDB = (MCAddrDB*)pData;
  RenameThread(strprintf("Stats_%s", pDB->pOpts->branchid.substr(0, 8).c_str()).c_str());
  bool first = true;
  std::string strShortName = pDB->pOpts->branchid.substr(0, 8).c_str();
  do {
    char c[256];
    time_t tim = time(NULL);
    struct tm *tmp = localtime(&tim);
    strftime(c, 256, "[%y-%m-%d %H:%M:%S]", tmp);
    MCAddrDBStats stats;
    pDB->GetStats(stats);
    if (first)
    {
        first = false;
        //printf("\n\n\n\x1b[3A");//this is not work in secureCRT, 
        printf("\n\n\n");
    }
    else{
        //printf("\x1b[2K\x1b[u");
    }
    //printf("\x1b[s");
    uint64_t requests = 0;
    uint64_t queries = 0;
    for (unsigned int i=0; i<dnsThread.size(); i++) {
      requests += dnsThread[i]->dns_opt.nRequests;//OP: this may be can keep this
      queries += dnsThread[i]->dbQueries;
    }
    printf("%s %i/%i available (%i tried in %is, %i new, %i active), %i banned; %llu DNS requests, %llu db queries, branchid %s\n", 
        c, stats.nGood, stats.nAvail, stats.nTracked, stats.nAge, stats.nNew, stats.nAvail - stats.nTracked - stats.nNew, 
        stats.nBanned, (unsigned long long)requests, (unsigned long long)queries, strShortName.c_str());
    Sleep(5000);
  } while(1);
  return nullptr;
}

//static const string mainnet_seeds[] = {"120.92.85.97", ""};//domain name or IP
//static const string testnet_seeds[] = {"120.92.85.97", ""};
//static const string *seeds = mainnet_seeds;

extern "C" void* ThreadSeeder(void*pData) {
  MCAddrDB* pDB = (MCAddrDB*)pData;
  RenameThread(strprintf("Seeder_%s", pDB->pOpts->branchid.substr(0, 8).c_str()).c_str());
  if (!fTestNet){
    //db.Add(MCService("kjy2eqzk4zwi5zd3.onion", 8333), true);
  }
  do {
    for (int i=0; i < pDB->pOpts->seeds.size(); i++) {
      vector<MCNetAddr> ips;
      LookupHost(pDB->pOpts->seeds[i].c_str(), ips);
      for (vector<MCNetAddr>::iterator it = ips.begin(); it != ips.end(); it++) {
          pDB->Add(MCService(*it, pDB->pOpts->defaultport), true);
      }
    }
    Sleep(1800000);
  } while(1);
  return nullptr;
}

MCDnsSeedOpts g_defaultOpts;// common options

void InitCommonOptions(int argc, char **argv)
{
    ParseCommandLine(argc, argv, &g_defaultOpts);
    g_defaultOpts.PrintWhitelistFilter();
    g_defaultOpts.branchid = strDefaultOpts;
    g_defaultOpts.defaultport = 0;
    
    if (!g_defaultOpts.tor.empty()) {
        MCService service(g_defaultOpts.tor, 9050);
        if (service.IsValid()) {
            printf("Using Tor proxy at %s\n", service.ToStringIPPort().c_str());
            SetProxy(NET_TOR, service);
        }
    }
    if (!g_defaultOpts.ipv4_proxy.empty()) {
        MCService service(g_defaultOpts.ipv4_proxy, 9050);
        if (service.IsValid()) {
            printf("Using IPv4 proxy at %s\n", service.ToStringIPPort().c_str());
            SetProxy(NET_IPV4, service);
        }
    }
    if (!g_defaultOpts.ipv6_proxy.empty()) {
        MCService service(g_defaultOpts.ipv6_proxy, 9050);
        if (service.IsValid()) {
            printf("Using IPv6 proxy at %s\n", service.ToStringIPPort().c_str());
            SetProxy(NET_IPV6, service);
        }
    }
    //defaultOpts.InitMessageStart();
    if (g_defaultOpts.fUseTestNet)
    {
        printf("Using testnet.\n");
        fTestNet = true;
    }
}

//注意 最后调用的fend 传 true
void StartSeederThread(MCAddrDB &db, bool fend)
{
    pthread_t threadSeed, threadDump, threadStats;
    printf("Starting seeder...");
    pthread_create(&threadSeed, NULL, ThreadSeeder, &db);
    printf("done\n");
    printf("Starting %i crawler threads...", db.pOpts->nThreads);
    pthread_attr_t attr_crawler;
    pthread_attr_init(&attr_crawler);
    pthread_attr_setstacksize(&attr_crawler, 0x20000);
    for (int i = 0; i < db.pOpts->nThreads; i++) {
        pthread_t thread;
        pthread_create(&thread, &attr_crawler, ThreadCrawler, &db);
    }
    pthread_attr_destroy(&attr_crawler);
    printf("done\n");
    pthread_create(&threadStats, NULL, ThreadStats, &db);
    pthread_create(&threadDump, NULL, ThreadDumper, &db);
    
    if (fend)
    {
        void* res;
        pthread_join(threadDump, &res);//
    }
}

void StartDNS(MCAddrDB& defaultdb)
{
    pthread_t threadDns;
    printf("Starting %i DNS threads (port %i)...\n", g_defaultOpts.nDnsThreads, g_defaultOpts.nPort);
    dnsThread.clear();
    for (int i = 0; i < g_defaultOpts.nDnsThreads; i++) {
        dnsThread.push_back(new MCDnsThread(&defaultdb, i));
        pthread_create(&threadDns, NULL, ThreadDNS, dnsThread[i]);
        printf(".");
        Sleep(20);
    }
    printf("dns thread done\n");
}

int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);
  setbuf(stdout, NULL);
  
  InitCommonOptions(argc, argv);

  bool fDNS = true;
  //TODO: Check if no ns option then will not open dns thread 
  //if (!g_defaultOpts.ns) {
  //  printf("No nameserver set. Not starting DNS server.\n");
  //  fDNS = false;
  //}
  //if (fDNS && !g_defaultOpts.host) {
  //  fprintf(stderr, "No hostname set. Please use -h.\n");
  //  exit(1);
  //}
  //if (fDNS && !g_defaultOpts.mbox) {
  //  fprintf(stderr, "No e-mail address set. Please use -m.\n");
  //  exit(1);
  //}

  MCAddrDB defaultdb(&g_defaultOpts);
  //defaultdb.LoadDBData();
  //AddNewDB(defaultdb);


  //多个dnsseed
  // main branch
  /*

  // dns threads share by all branch.
  if (fDNS) {
      StartDNS(defaultdb);
  }
  {
      MCDnsSeedOpts* pOpts = new MCDnsSeedOpts();
      pOpts->branchid = "main";
      pOpts->defaultport = !fTestNet ? 8833 : 18833;
      pOpts->nThreads = 33; //default 96
      pOpts->fUseTestNet = fTestNet;

      pOpts->host = "seed.celllinkseed.io";// -h
      pOpts->ns = "dns.celllinkseed.io";// -n
      pOpts->mbox = "alibuybuy@yandex.com"; // -m

      pOpts->seeds.push_back("120.92.85.97");
      //pOpts->seeds.push_back("1.2.3.4");// test data

      pOpts->InitMessageStart();

      MCAddrDB* pdb = new MCAddrDB(pOpts);
      pdb->LoadDBData();
      AddNewDB(*pdb);
      StartSeederThread(*pdb, false);
  }
  // branch 1
  {
      MCDnsSeedOpts* pOpts = new MCDnsSeedOpts();
      pOpts->branchid = "9aa3965c779b2611c7ffd43d7c85a9a06bd811f11a45eb6c35f71c2bfe36a99c";
      pOpts->defaultport = 28833;// TODO: 
      pOpts->nThreads = 33; //default 96
      pOpts->fUseTestNet = fTestNet;

      pOpts->host = "seedb1.celllinkseed.io";// -h
      pOpts->ns = "dnsb1.celllinkseed.io";// -n
      pOpts->mbox = "alibuybuy@yandex.com"; // -m

      pOpts->seeds.push_back("120.92.85.97");
      //pOpts->seeds.push_back("11.22.33.44");// test data

      pOpts->InitMessageStart();

      MCAddrDB* pdb = new MCAddrDB(pOpts);
      pdb->LoadDBData();
      AddNewDB(*pdb);
      StartSeederThread(*pdb, true);
  }
  */
  if (g_configgilename == nullptr)
  {
      printf("g_configgilename is null");
      return 1;
  }

  ConfigLuaReader luaConfReader;
  vector<MCDnsSeedOpts*> vectDNSSeeds;
  bool ret = luaConfReader.ReadConfig(g_configgilename, vectDNSSeeds);
  if (!ret)
  {
      printf("reading config error!!\n");
      return 1;
  }

  if (fDNS) {
      StartDNS(defaultdb);
  }

  const size_t vecSize = vectDNSSeeds.size();
  for (int i=0; i < vecSize; i++)
  {
      MCDnsSeedOpts* opts = vectDNSSeeds[i];
      if (g_mapBranchDB.count(opts->branchid))
      {
          printf("duplicate branch id %s\n", opts->branchid.c_str());
          continue;
      }
      printf("running branch %s\n", opts->branchid.c_str());

      opts->fUseTestNet = fTestNet;
      opts->InitMessageStart();

      MCAddrDB* pdb = new MCAddrDB(opts);
      pdb->LoadDBData();
      AddNewDB(*pdb);
      StartSeederThread(*pdb, i == (vecSize -1));
  }

  return 0;
}
