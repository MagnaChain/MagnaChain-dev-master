#yum install lua-devel.x86_64 lua-devel.i686

CXXFLAGS = -O3 -g0 -march=native -I/usr/include/ 
LDFLAGS = $(CXXFLAGS)

dnsseed: mcdnsseedopts.o dns.o magnachain.o netbase.o protocol.o db.o main.o util.o config_luareader.o
	g++ -pthread $(LDFLAGS) -o dnsseed mcdnsseedopts.o dns.o magnachain.o netbase.o protocol.o db.o main.o util.o config_luareader.o -lcrypto -llua

%.o: %.cpp magnachain.h netbase.h protocol.h db.h serialize.h uint256.h util.h mcdnsseedopts.h config_luareader.h
	g++ -std=c++11 -pthread $(CXXFLAGS) -Wall -Wno-unused -Wno-sign-compare -Wno-reorder -Wno-comment -c -o $@ $<

dns.o: dns.c
	gcc -pthread -std=c99 $(CXXFLAGS) dns.c -Wall -c -o dns.o

%.o: %.cpp
