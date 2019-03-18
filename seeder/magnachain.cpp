#include <algorithm>

#include "db.h"
#include "magnachain.h"
#include "netbase.h"
#include "protocol.h"
#include "serialize.h"
#include "uint256.h"

#define MAGNACHAIN_SEED_NONCE  0x0539a019ca550825ULL

using namespace std;

class MCNode {
  SOCKET sock;
  MCDataStream vSend;
  MCDataStream vRecv;
  unsigned int nHeaderStart;
  unsigned int nMessageStart;
  int nVersion;
  string strSubVer;
  int nStartingHeight;
  vector<MCAddress> *vAddr;
  int ban;
  int64 doneAfter;
  MCAddress you;

  const string branchid;
  unsigned char pchMessageStart[4];

  int GetTimeout() {
      if (you.IsTor())
          return 120;
      else
          return 30;
  }

  void BeginMessage(const char *pszCommand) {
    if (nHeaderStart != -1) AbortMessage();
    nHeaderStart = vSend.size();
    vSend << MCMessageHeader(pszCommand, 0, pchMessageStart);
    nMessageStart = vSend.size();
//    printf("%s: SEND %s\n", ToString(you).c_str(), pszCommand); 
  }
  
  void AbortMessage() {
    if (nHeaderStart == -1) return;
    vSend.resize(nHeaderStart);
    nHeaderStart = -1;
    nMessageStart = -1;
  }
  
  void EndMessage() {
    if (nHeaderStart == -1) return;
    unsigned int nSize = vSend.size() - nMessageStart;
    memcpy((char*)&vSend[nHeaderStart] + offsetof(MCMessageHeader, nMessageSize), &nSize, sizeof(nSize));
    if (vSend.GetVersion() >= 209) {
      uint256 hash = Hash(vSend.begin() + nMessageStart, vSend.end());
      unsigned int nChecksum = 0;
      memcpy(&nChecksum, &hash, sizeof(nChecksum));
      assert(nMessageStart - nHeaderStart >= offsetof(MCMessageHeader, nChecksum) + sizeof(nChecksum));
      memcpy((char*)&vSend[nHeaderStart] + offsetof(MCMessageHeader, nChecksum), &nChecksum, sizeof(nChecksum));
    }
    nHeaderStart = -1;
    nMessageStart = -1;
  }
  
  void Send() {
    if (sock == INVALID_SOCKET) return;
    if (vSend.empty()) return;
    int nBytes = send(sock, &vSend[0], vSend.size(), 0);
    if (nBytes > 0) {
      vSend.erase(vSend.begin(), vSend.begin() + nBytes);
    } else {
      close(sock);
      sock = INVALID_SOCKET;
    }
  }
  
  void PushVersion() {
    int64 nTime = time(NULL);
    uint64 nLocalNonce = MAGNACHAIN_SEED_NONCE;
    int64 nLocalServices = 0;
    MCAddress me(MCService("0.0.0.0"));
    BeginMessage("version");
    int nBestHeight = GetRequireHeight();
    string ver = "/magnachain-seeder:0.01/";
    vSend << PROTOCOL_VERSION << nLocalServices << nTime << this->branchid << you << me << nLocalNonce << ver << nBestHeight;
    EndMessage();
  }
 
  void GotVersion() {
    // printf("\n%s: version %i\n", ToString(you).c_str(), nVersion);
    if (vAddr) {
      BeginMessage("getaddr");
      EndMessage();
      doneAfter = time(NULL) + GetTimeout();
    } else {
      doneAfter = time(NULL) + 1;
    }
  }

  bool ProcessMessage(string strCommand, MCDataStream& vRecv) {
//    printf("%s: RECV %s\n", ToString(you).c_str(), strCommand.c_str());
    if (strCommand == "version") {
      int64 nTime;
      string strBranchId;
      MCAddress addrMe;
      MCAddress addrFrom;
      uint64 nNonce = 1;
      vRecv >> nVersion >> you.nServices >> nTime >> strBranchId >> addrMe;

      if (nVersion == 10300) nVersion = 300;
      if (nVersion >= 106 && !vRecv.empty())
        vRecv >> addrFrom >> nNonce;
      if (nVersion >= 106 && !vRecv.empty())
        vRecv >> strSubVer;
      if (nVersion >= 209 && !vRecv.empty())
		vRecv >> nStartingHeight;

	  if (strBranchId == this->branchid)
	  {
		  if (nVersion >= 209) {
			  BeginMessage("verack");
			  EndMessage();
		  }
		  vSend.SetVersion(min(nVersion, PROTOCOL_VERSION));
		  if (nVersion < 209) {
			  this->vRecv.SetVersion(min(nVersion, PROTOCOL_VERSION));
			  GotVersion();
		  }
	  }
      
      return false;
    }
    
    if (strCommand == "verack") {
      this->vRecv.SetVersion(min(nVersion, PROTOCOL_VERSION));
      GotVersion();
      return false;
    }
    
    if (strCommand == "addr" && vAddr) {
      vector<MCAddress> vAddrNew;
      vRecv >> vAddrNew;
      // printf("%s: got %i addresses\n", ToString(you).c_str(), (int)vAddrNew.size());
      int64 now = time(NULL);
      vector<MCAddress>::iterator it = vAddrNew.begin();
      if (vAddrNew.size() > 1) {
        if (doneAfter == 0 || doneAfter > now + 1) doneAfter = now + 1;
      }
      while (it != vAddrNew.end()) {
        MCAddress &addr = *it;
//        printf("%s: got address %s\n", ToString(you).c_str(), addr.ToString().c_str(), (int)(vAddr->size()));
        it++;
        if (addr.nTime <= 100000000 || addr.nTime > now + 600)
          addr.nTime = now - 5 * 86400;
        if (addr.nTime > now - 604800)
          vAddr->push_back(addr);
//        printf("%s: added address %s (#%i)\n", ToString(you).c_str(), addr.ToString().c_str(), (int)(vAddr->size()));
        if (vAddr->size() > 1000) {doneAfter = 1; return true; }
      }
      return false;
    }
    
    return false;
  }
  
  bool ProcessMessages() {
    if (vRecv.empty()) return false;
    do {
      MCDataStream::iterator pstart = search(vRecv.begin(), vRecv.end(), BEGIN(pchMessageStart), END(pchMessageStart));
      int nHeaderSize = vRecv.GetSerializeSize(MCMessageHeader(pchMessageStart));
      if (vRecv.end() - pstart < nHeaderSize) {
        if (vRecv.size() > nHeaderSize) {
          vRecv.erase(vRecv.begin(), vRecv.end() - nHeaderSize);
        }
        break;
      }
      vRecv.erase(vRecv.begin(), pstart);
      vector<char> vHeaderSave(vRecv.begin(), vRecv.begin() + nHeaderSize);
      MCMessageHeader hdr(pchMessageStart);
      vRecv >> hdr;
      if (!hdr.IsValid()) { 
        // printf("%s: BAD (invalid header)\n", ToString(you).c_str());
        ban = 100000; return true;
      }
      string strCommand = hdr.GetCommand();
      unsigned int nMessageSize = hdr.nMessageSize;
      if (nMessageSize > MAX_SIZE) { 
        // printf("%s: BAD (message too large)\n", ToString(you).c_str());
        ban = 100000;
        return true; 
      }
      if (nMessageSize > vRecv.size()) {
        vRecv.insert(vRecv.begin(), vHeaderSave.begin(), vHeaderSave.end());
        break;
      }
      if (vRecv.GetVersion() >= 209) {
        uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
        unsigned int nChecksum = 0;
        memcpy(&nChecksum, &hash, sizeof(nChecksum));
        if (nChecksum != hdr.nChecksum) continue;
      }
      MCDataStream vMsg(vRecv.begin(), vRecv.begin() + nMessageSize, vRecv.nType, vRecv.nVersion);
      vRecv.ignore(nMessageSize);
      if (ProcessMessage(strCommand, vMsg))
        return true;
//      printf("%s: done processing %s\n", ToString(you).c_str(), strCommand.c_str());
    } while(1);
    return false;
  }
  
public:
  MCNode(const MCService& ip, vector<MCAddress>* vAddrIn, const string &strBranchId, unsigned char* pchMsgStart)
      : you(ip), nHeaderStart(-1), nMessageStart(-1), vAddr(vAddrIn), ban(0), doneAfter(0), nVersion(0), branchid(strBranchId){
    vSend.SetType(SER_NETWORK);
    vSend.SetVersion(0);
    vRecv.SetType(SER_NETWORK);
    vRecv.SetVersion(0);
    if (time(NULL) > 1329696000) {
      vSend.SetVersion(209);
      vRecv.SetVersion(209);
    }
    //copy
    pchMessageStart[0] = pchMsgStart[0];
    pchMessageStart[1] = pchMsgStart[1];
    pchMessageStart[2] = pchMsgStart[2];
    pchMessageStart[3] = pchMsgStart[3];
  }
  bool Run() {
    bool res = true;
    if (!ConnectSocket(you, sock)) return false;
    PushVersion();
    Send();
    int64 now;
    while (now = time(NULL), ban == 0 && (doneAfter == 0 || doneAfter > now) && sock != INVALID_SOCKET) {
      char pchBuf[0x10000];
      fd_set set;
      FD_ZERO(&set);
      FD_SET(sock,&set);
      struct timeval wa;
      if (doneAfter) {
        wa.tv_sec = doneAfter - now;
        wa.tv_usec = 0;
      } else {
        wa.tv_sec = GetTimeout();
        wa.tv_usec = 0;
      }
      int ret = select(sock+1, &set, NULL, &set, &wa);
      if (ret != 1) {
        if (!doneAfter) res = false;
        break;
      }
      int nBytes = recv(sock, pchBuf, sizeof(pchBuf), 0);
      int nPos = vRecv.size();
      if (nBytes > 0) {
        vRecv.resize(nPos + nBytes);
        memcpy(&vRecv[nPos], pchBuf, nBytes);
      } else if (nBytes == 0) {
        // printf("%s: BAD (connection closed prematurely)\n", ToString(you).c_str());
        res = false;
        break;
      } else {
        // printf("%s: BAD (connection error)\n", ToString(you).c_str());
        res = false;
        break;
      }
      ProcessMessages();
      Send();
    }
    if (sock == INVALID_SOCKET) res = false;
    close(sock);
    sock = INVALID_SOCKET;
    return (ban == 0) && res;
  }
  
  int GetBan() {
    return ban;
  }
  
  int GetClientVersion() {
    return nVersion;
  }
  
  std::string GetClientSubVersion() {
    return strSubVer;
  }
  
  int GetStartingHeight() {
    return nStartingHeight;
  }
};

bool TestNode(const MCService &cip, int &ban, int &clientV, std::string &clientSV, int &blocks, vector<MCAddress>* vAddr, const std::string &strBranchId, unsigned char* pchMessageStart) {
  try {
    MCNode node(cip, vAddr, strBranchId, pchMessageStart);
    bool ret = node.Run();
    if (!ret) {
      ban = node.GetBan();
    } else {
      ban = 0;
    }
    clientV = node.GetClientVersion();
    clientSV = node.GetClientSubVersion();
    blocks = node.GetStartingHeight();
//  printf("%s: %s!!!\n", cip.ToString().c_str(), ret ? "GOOD" : "BAD");
    return ret;
  } catch(std::ios_base::failure& e) {
    ban = 0;
    return false;
  }
}

/*
int main(void) {
  MCService ip("magnachain.sipa.be", 8333, true);
  vector<MCAddress> vAddr;
  vAddr.clear();
  int ban = 0;
  bool ret = TestNode(ip, ban, vAddr);
  printf("ret=%s ban=%i vAddr.size()=%i\n", ret ? "good" : "bad", ban, (int)vAddr.size());
}
*/

