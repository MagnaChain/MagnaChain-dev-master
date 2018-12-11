#include "db.h"
#include <stdlib.h>
#include "util.h"
#include <algorithm>
#include <inttypes.h>

using namespace std;

void MCAddrInfo::Update(bool good) {
  uint32_t now = time(NULL);
  if (ourLastTry == 0)
    ourLastTry = now - MIN_RETRY;
  int age = now - ourLastTry;
  lastTry = now;
  ourLastTry = now;
  total++;
  if (good)
  {
    success++;
    ourLastSuccess = now;
  }
  stat2H.Update(good, age, 3600*2);
  stat8H.Update(good, age, 3600*8);
  stat1D.Update(good, age, 3600*24);
  stat1W.Update(good, age, 3600*24*7);
  stat1M.Update(good, age, 3600*24*30);
  int ign = GetIgnoreTime();
  if (ign && (ignoreTill==0 || ignoreTill < ign+now)) ignoreTill = ign+now;
//  printf("%s: got %s result: success=%i/%i; 2H:%.2f%%-%.2f%%(%.2f) 8H:%.2f%%-%.2f%%(%.2f) 1D:%.2f%%-%.2f%%(%.2f) 1W:%.2f%%-%.2f%%(%.2f) \n", ToString(ip).c_str(), good ? "good" : "bad", success, total, 
//  100.0 * stat2H.reliability, 100.0 * (stat2H.reliability + 1.0 - stat2H.weight), stat2H.count,
//  100.0 * stat8H.reliability, 100.0 * (stat8H.reliability + 1.0 - stat8H.weight), stat8H.count,
//  100.0 * stat1D.reliability, 100.0 * (stat1D.reliability + 1.0 - stat1D.weight), stat1D.count,
//  100.0 * stat1W.reliability, 100.0 * (stat1W.reliability + 1.0 - stat1W.weight), stat1W.count);
}

bool MCAddrDB::Get_(MCServiceResult &ip, int &wait) {
  int64 now = time(NULL);
  int cont = 0;
  int tot = unkId.size() + ourId.size();
  if (tot == 0) {
    wait = 5;
    return false;
  }
  do {
    int rnd = rand() % tot;
    int ret;
    if (rnd < unkId.size()) {
      set<int>::iterator it = unkId.end(); it--;
      ret = *it;
      unkId.erase(it);
    } else {
      ret = ourId.front();
      if (time(NULL) - idToInfo[ret].ourLastTry < MIN_RETRY) return false;
      ourId.pop_front();
    }
    if (idToInfo[ret].ignoreTill && idToInfo[ret].ignoreTill < now) {
      ourId.push_back(ret);
      idToInfo[ret].ourLastTry = now;
    } else {
      ip.service = idToInfo[ret].ip;
      ip.ourLastSuccess = idToInfo[ret].ourLastSuccess;
      break;
    }
  } while(1);
  nDirty++;
  return true;
}

int MCAddrDB::Lookup_(const MCService &ip) {
  if (ipToId.count(ip))
    return ipToId[ip];
  return -1;
}

void MCAddrDB::Good_(const MCService &addr, int clientV, std::string clientSV, int blocks) {
  int id = Lookup_(addr);
  if (id == -1) return;
  unkId.erase(id);
  banned.erase(addr);
  MCAddrInfo &info = idToInfo[id];
  info.defaultport = pOpts->defaultport;//first set?
  info.clientVersion = clientV;
  info.clientSubVersion = clientSV;
  info.blocks = blocks;
  info.Update(true);
  if (info.IsGood() && goodId.count(id)==0) {
    goodId.insert(id);
//    printf("%s: good; %i good nodes now\n", ToString(addr).c_str(), (int)goodId.size());
  }
  nDirty++;
  ourId.push_back(id);
}

void MCAddrDB::Bad_(const MCService &addr, int ban)
{
  int id = Lookup_(addr);
  if (id == -1) return;
  unkId.erase(id);
  MCAddrInfo &info = idToInfo[id];
  info.defaultport = pOpts->defaultport;//first set?
  info.Update(false);
  uint32_t now = time(NULL);
  int ter = info.GetBanTime();
  if (ter) {
//    printf("%s: terrible\n", ToString(addr).c_str());
    if (ban < ter) ban = ter;
  }
  if (ban > 0) {
//    printf("%s: ban for %i seconds\n", ToString(addr).c_str(), ban);
    banned[info.ip] = ban + now;
    ipToId.erase(info.ip);
    goodId.erase(id);
    idToInfo.erase(id);
  } else {
    if (/*!info.IsGood() && */ goodId.count(id)==1) {
      goodId.erase(id);
//      printf("%s: not good; %i good nodes left\n", ToString(addr).c_str(), (int)goodId.size());
    }
    ourId.push_back(id);
  }
  nDirty++;
}

void MCAddrDB::Skipped_(const MCService &addr)
{
  int id = Lookup_(addr);
  if (id == -1) return;
  unkId.erase(id);
  ourId.push_back(id);
//  printf("%s: skipped\n", ToString(addr).c_str());
  nDirty++;
}


void MCAddrDB::Add_(const MCAddress &addr, bool force) {
  if (!force && !addr.IsRoutable())
    return;
  MCService ipp(addr);
  if (banned.count(ipp)) {
    time_t bantime = banned[ipp];
    if (force || (bantime < time(NULL) && addr.nTime > bantime))
      banned.erase(ipp);
    else
      return;
  }
  if (ipToId.count(ipp)) {
    MCAddrInfo &ai = idToInfo[ipToId[ipp]];
    if (addr.nTime > ai.lastTry || ai.services != addr.nServices)
    {
      ai.lastTry = addr.nTime;
      ai.services |= addr.nServices;
//      printf("%s: updated\n", ToString(addr).c_str());
    }
    if (force) {
      ai.ignoreTill = 0;
    }
    return;
  }
  MCAddrInfo ai;
  ai.ip = ipp;
  ai.services = addr.nServices;
  ai.lastTry = addr.nTime;
  ai.ourLastTry = 0;
  ai.total = 0;
  ai.success = 0;
  ai.defaultport = pOpts->defaultport;
  int id = nId++;
  idToInfo[id] = ai;
  ipToId[ipp] = id;
  printf("%s: added %d\n", ToString(ipp).c_str(), ipToId[ipp]);
  unkId.insert(id);
  nDirty++;
}

void MCAddrDB::GetIPs_(set<MCNetAddr>& ips, uint64_t requestedFlags, int max, const bool* nets) {
  if (goodId.size() == 0) {
    int id = -1;
    if (ourId.size() == 0) {
      if (unkId.size() == 0) return;
      id = *unkId.begin();
    } else {
      id = *ourId.begin();
    }
    if (id >= 0 && (idToInfo[id].services & requestedFlags) == requestedFlags) {
      ips.insert(idToInfo[id].ip);
    }
    return;
  }
  std::vector<int> goodIdFiltered;
  for (std::set<int>::const_iterator it = goodId.begin(); it != goodId.end(); it++) {
    if ((idToInfo[*it].services & requestedFlags) == requestedFlags)
      goodIdFiltered.push_back(*it);
  }

  if (!goodIdFiltered.size())
    return;

  if (max > goodIdFiltered.size() / 2)
    max = goodIdFiltered.size() / 2;
  if (max < 1)
    max = 1;

  set<int> ids;
  while (ids.size() < max) {
    ids.insert(goodIdFiltered[rand() % goodIdFiltered.size()]);
  }
  for (set<int>::const_iterator it = ids.begin(); it != ids.end(); it++) {
    MCService &ip = idToInfo[*it].ip;
    if (nets[ip.GetNetwork()])
      ips.insert(ip);
  }
}

void MCAddrDB::LoadDBData()
{
    std::string dbfilename = strprintf("dnsseed_%s.dat", pOpts->branchid.c_str());
    FILE *f = fopen(dbfilename.c_str(), "r");
    if (f) {
        printf("Loading %s...", dbfilename.c_str());
        MCAutoFile cf(f);
        cf >> *this;
        if (pOpts->fWipeBan)
            this->banned.clear();
        if (pOpts->fWipeIgnore)
            this->ResetIgnores();
        printf("done\n");
    }
}

int StatCompare(const MCAddrReport& a, const MCAddrReport& b) {
    if (a.uptime[4] == b.uptime[4]) {
        if (a.uptime[3] == b.uptime[3]) {
            return a.clientVersion > b.clientVersion;
        }
        else {
            return a.uptime[3] > b.uptime[3];
        }
    }
    else {
        return a.uptime[4] > b.uptime[4];
    }
}

void MCAddrDB::SaveDBData()
{
    std::string dbfilename = strprintf("dnsseed_%s.dat", pOpts->branchid.c_str());
    std::string dbfilenamenew = strprintf("dnsseed_%s.dat.new", pOpts->branchid.c_str());
    std::string dbfilenamedump = strprintf("dnsseed_%s.dump", pOpts->branchid.c_str());
    std::string dblogfile = strprintf("dnsstats_%s.log", pOpts->branchid.c_str());

    vector<MCAddrReport> v = this->GetAll();
    std::sort(v.begin(), v.end(), StatCompare);
    FILE *f = fopen(dbfilenamenew.c_str(), "w+");
    if (f) {
        {
            MCAutoFile cf(f);
            cf << *this;
        }
        rename(dbfilenamenew.c_str(), dbfilename.c_str());
    }
    FILE *d = fopen(dbfilenamedump.c_str(), "w");
    fprintf(d, "# address                                        good  lastSuccess    %%(2h)   %%(8h)   %%(1d)   %%(7d)  %%(30d)  blocks      svcs  version\n");
    double stat[5] = { 0,0,0,0,0 };
    for (vector<MCAddrReport>::const_iterator it = v.begin(); it < v.end(); it++) {
        MCAddrReport rep = *it;
        fprintf(d, "%-47s  %4d  %11" PRId64 "  %6.2f%% %6.2f%% %6.2f%% %6.2f%% %6.2f%%  %6i  %08" PRIx64 "  %5i \"%s\"\n", rep.ip.ToString().c_str(), (int)rep.fGood, rep.lastSuccess, 100.0*rep.uptime[0], 100.0*rep.uptime[1], 100.0*rep.uptime[2], 100.0*rep.uptime[3], 100.0*rep.uptime[4], rep.blocks, rep.services, rep.clientVersion, rep.clientSubVersion.c_str());
        stat[0] += rep.uptime[0];
        stat[1] += rep.uptime[1];
        stat[2] += rep.uptime[2];
        stat[3] += rep.uptime[3];
        stat[4] += rep.uptime[4];
    }
    fclose(d);
    FILE *ff = fopen(dblogfile.c_str(), "a");
    fprintf(ff, "%llu %g %g %g %g %g\n", (unsigned long long)(time(NULL)), stat[0], stat[1], stat[2], stat[3], stat[4]);
    fclose(ff);
}