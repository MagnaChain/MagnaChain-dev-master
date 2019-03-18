--[[
  "Options:\n"
  "-h <host>    host        Hostname of the DNS seed\n"
  "-n <ns>      ns          Hostname of the nameserver\n"
  "-m <mbox>    mbox        E-Mail address reported in SOA records\n"
  "-t <threads> nThreads    Number of crawlers to run in parallel (default 96)\n"
  "-b <branchid>branchid    The branch id\n"
  "-s <seeds>	seeds       The seeds of the chain, array of domain name or IP\n"
  "--wipeban                Wipe list of banned nodes\n"
  "--wipeignore             Wipe list of ignored nodes\n"
  "             defaultport Node default port, dns only return IP, no port.\n"
  
  "command params for all"
  "-d <threads> dns_t       Number of DNS server threads (default 4)\n"
  "--testnet                Use testnet\n"
  "-p <port>    upd_p       UDP port to listen on (default 53)\n"
  "-o <ip:port> tor         Tor proxy IP/Port\n"
  "-i <ip:port> IPV4proxy   IPV4 SOCKS5 proxy IP/Port\n"
  "-k <ip:port> IPV6proxy   IPV6 SOCKS5 proxy IP/Port\n"
  "-w f1,f2,... whitefilter Allow these flag combinations as filters\n"
]]

--,pchMessageStart={}
--mainchain

--testnet

seederconfig={
--main
{ branchid="main",
  defaultport=8833,
  nThreads=4,
  host="seed.celllinkseed.io",
  ns="dns.celllinkseed.io",
  mbox="alibuybuy@yandex.com",
  seeds={"main.celllinkseed.io","seed.magnachainseed.io"}, --"120.92.85.97", do not hard code the ip
},
--branch
--[[
{ branchid="9aa3965c779b2611c7ffd43d7c85a9a06bd811f11a45eb6c35f71c2bfe36a99c",
  defaultport=28834,
  nThreads=4,
  host="seedb1.celllinkseed.io",
  ns="dnsb1.celllinkseed.io",
  mbox="alibuybuy@yandex.com",
  seeds={"120.92.85.97", seedb1.magnachainseed.io},
},
]]
}