# Copyright (c) 2013-2016 The Bitcoin Core developers
# Copyright (c) 2016-2019 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS) $(PIC_FLAGS)
AM_CXXFLAGS = $(HARDENED_CXXFLAGS) $(ERROR_CXXFLAGS) $(PIC_FLAGS)
AM_CPPFLAGS = $(HARDENED_CPPFLAGS) $(PIC_FLAGS)
EXTRA_LIBRARIES =

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

CELLLINK_CONFIG_INCLUDES=-I$(builddir)/config
CELLLINK_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

CELLLINK_INCLUDES += -I$(srcdir)/secp256k1/include
CELLLINK_INCLUDES += $(UNIVALUE_CFLAGS)

LIBCELLLINK_SERVER=libcelllink_server.a
LIBCELLLINK_COMMON=libcelllink_common.a
LIBCELLLINK_CONSENSUS=libcelllink_consensus.a
LIBCELLLINK_CLI=libcelllink_cli.a
LIBCELLLINK_UTIL=libcelllink_util.a
LIBCELLLINK_CRYPTO=crypto/libcelllink_crypto.a
LIBCELLLINKQT=qt/libcelllinkqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

if ENABLE_ZMQ
LIBCELLLINK_ZMQ=libcelllink_zmq.a
endif
if BUILD_CELLLINK_LIBS
LIBCELLLINKCONSENSUS=libcelllinkconsensus.la
endif
if ENABLE_WALLET
LIBCELLLINK_WALLET=libcelllink_wallet.a
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBCELLLINK_CRYPTO) \
  $(LIBCELLLINK_UTIL) \
  $(LIBCELLLINK_COMMON) \
  $(LIBCELLLINK_CONSENSUS) \
  $(LIBCELLLINK_SERVER) \
  $(LIBCELLLINK_CLI) \
  $(LIBCELLLINK_WALLET) \
  $(LIBCELLLINK_ZMQ)

lib_LTLIBRARIES = $(LIBCELLLINKCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_CELLLINKD
  bin_PROGRAMS += magnachaind
endif

if BUILD_CELLLINK_UTILS
  bin_PROGRAMS += magnachain-cli magnachain-tx
endif

.PHONY: FORCE check-symbols check-security
# celllink core #
CELLLINK_CORE_H = \
  address/addrdb.h \
  address/addrman.h \
  coding/base58.h \
  transaction/bloom.h \
  transaction/blockencodings.h \
  chain/branchchain.h \
  chain/branchdb.h \
  chain/chain.h \
  chain/chainparams.h \
  chain/chainparamsbase.h \
  chain/chainparamsseeds.h \
  validation/checkpoints.h \
  validation/checkqueue.h \
  misc/clientversion.h \
  transaction/coins.h \
  net/compat.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  consensus/tx_verify.h \
  io/core_io.h \
  misc/core_memusage.h \
  misc/cuckoocache.h \
  io/fs.h \
  net/http/httprpc.h \
  net/http/httpserver.h \
  misc/indirectmap.h \
  init.h \
  key/key.h \
  key/keystore.h \
  io/dbwrapper.h \
  misc/limitedmap.h \
  misc/memusage.h \
  transaction/merkleblock.h \
  mining/miner.h \
  net/net.h \
  net/net_processing.h \
  net/netaddress.h \
  net/netbase.h \
  net/netmessagemaker.h \
  ui/noui.h \
  policy/feerate.h \
  policy/fees.h \
  policy/policy.h \
  policy/rbf.h \
  misc/pow.h \
  net/protocol.h \
  misc/random.h \
  misc/reverse_iterator.h \
  misc/reverselock.h \
  rpc/blockchain.h \
  rpc/branchchainrpc.h \
  rpc/client.h \
  mining/mining.h \
  rpc/protocol.h \
  rpc/server.h \
  rpc/register.h \
  thread/scheduler.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  script/ismine.h \
  io/streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/events.h \
  support/lockedpool.h \
  thread/sync.h \
  thread/threadsafety.h \
  thread/threadinterrupt.h \
  misc/timedata.h \
  net/torcontrol.h \
  transaction/txdb.h \
  transaction/txmempool.h \
  ui/ui_interface.h \
  transaction/undo.h \
  utils/util.h \
  utils/utilmoneystr.h \
  utils/utiltime.h \
  validation/validation.h \
  validation/validationinterface.h \
  misc/versionbits.h \
  wallet/coincontrol.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/feebumper.h \
  wallet/rpcwallet.h \
  wallet/wallet.h \
  wallet/walletdb.h \
  misc/warnings.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqconfig.h\
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh "$(abs_top_builddir)/src/obj/build.h" \
	  "$(abs_top_srcdir)"
libcelllink_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between celllinkd and celllink-qt
libcelllink_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libcelllink_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcelllink_server_a_SOURCES = \
  address/addrdb.cpp \
  address/addrman.cpp \
  transaction/bloom.cpp \
  transaction/blockencodings.cpp \
  chain/chain.cpp \
  validation/checkpoints.cpp \
  consensus/tx_verify.cpp \
  net/http/httprpc.cpp \
  net/http/httpserver.cpp \
  init.cpp \
  io/dbwrapper.cpp \
  transaction/merkleblock.cpp \
  mining/miner.cpp \
  net/net.cpp \
  net/net_processing.cpp \
  ui/noui.cpp \
  policy/fees.cpp \
  policy/policy.cpp \
  policy/rbf.cpp \
  misc/pow.cpp \
  misc/rest.cpp \
  rpc/blockchain.cpp \
  rpc/branchchainrpc.cpp \
  mining/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/rawtransaction.cpp \
  rpc/server.cpp \
  script/sigcache.cpp \
  script/ismine.cpp \
  misc/timedata.cpp \
  net/torcontrol.cpp \
  transaction/txdb.cpp \
  transaction/txmempool.cpp \
  ui/ui_interface.cpp \
  validation/validation.cpp \
  validation/validationinterface.cpp \
  misc/versionbits.cpp \
  smartcontract/smartcontract.cpp \
  smartcontract/contractdb.cpp \
  chain/branchchain.cpp \
  chain/branchdb.cpp \
  chain/branchtxdb.cpp \
  $(CELLLINK_CORE_H)

if ENABLE_ZMQ
libcelllink_zmq_a_CPPFLAGS = $(CELLLINK_INCLUDES) $(ZMQ_CFLAGS)
libcelllink_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcelllink_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp
endif


# wallet: shared between celllinkd and celllink-qt, but only linked
# when wallet enabled
libcelllink_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES)
libcelllink_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcelllink_wallet_a_SOURCES = \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/feebumper.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  $(CELLLINK_CORE_H)

# crypto primitives library
crypto_libcelllink_crypto_a_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES) $(CELLLINK_CONFIG_INCLUDES)
crypto_libcelllink_crypto_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libcelllink_crypto_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/common.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.cpp \
  crypto/sha512.h \
  lua/lapi.c \
  lua/lauxlib.c \
  lua/lbaselib.c \
  lua/lcode.c \
  lua/ldblib.c \
  lua/ldebug.c \
  lua/ldo.c \
  lua/ldump.c \
  lua/lfunc.c \
  lua/lgc.c \
  lua/linit.c \
  lua/liolib.c \
  lua/llex.c \
  lua/lmathlib.c \
  lua/lmem.c \
  lua/loadlib.c \
  lua/lobject.c \
  lua/lopcodes.c \
  lua/loslib.c \
  lua/lparser.c \
  lua/lstate.c \
  lua/lstring.c \
  lua/lstrlib.c \
  lua/ltable.c \
  lua/ltablib.c \
  lua/ltm.c \
  lua/lundump.c \
  lua/lvm.c \
  lua/lzio.c \
  lua/print.c \
  lua/cjson/fpconv.c \
  lua/cjson/lua_cjson.c \
  lua/cjson/strbuf.c \
  lua/lua_cmsgpack.c
  

if EXPERIMENTAL_ASM
crypto_libcelllink_crypto_a_SOURCES += crypto/sha256_sse4.cpp
endif

# consensus: shared between all executables that validate any consensus rules.
libcelllink_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES)
libcelllink_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcelllink_consensus_a_SOURCES = \
  misc/amount.h \
  coding/arith_uint256.cpp \
  coding/arith_uint256.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.h \
  consensus/validation.h \
  coding/hash.cpp \
  coding/hash.h \
  misc/prevector.h \
  primitives/block.cpp \
  primitives/block.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  transaction/partialmerkletree.cpp \
  transaction/partialmerkletree.h \
  key/pubkey.cpp \
  key/pubkey.h \
  script/celllinkconsensus.cpp \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  io/serialize.h \
  misc/tinyformat.h \
  coding/uint256.cpp \
  coding/uint256.h \
  utils/utilstrencodings.cpp \
  utils/utilstrencodings.h \
  misc/version.h

# common: shared between celllinkd, and celllink-qt and non-server tools
libcelllink_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES)
libcelllink_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcelllink_common_a_SOURCES = \
  coding/base58.cpp \
  chain/chainparams.cpp \
  transaction/coins.cpp \
  transaction/compressor.cpp \
  io/core_read.cpp \
  io/core_write.cpp \
  key/key.cpp \
  key/keystore.cpp \
  net/netaddress.cpp \
  net/netbase.cpp \
  policy/feerate.cpp \
  net/protocol.cpp \
  thread/scheduler.cpp \
  script/sign.cpp \
  script/standard.cpp \
  misc/warnings.cpp \
  $(CELLLINK_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libcelllink_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES)
libcelllink_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcelllink_util_a_SOURCES = \
  support/lockedpool.cpp \
  chain/chainparamsbase.cpp \
  misc/clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  io/fs.cpp \
  misc/random.cpp \
  rpc/protocol.cpp \
  support/cleanse.cpp \
  thread/sync.cpp \
  thread/threadinterrupt.cpp \
  utils/util.cpp \
  utils/utilmoneystr.cpp \
  utils/utilstrencodings.cpp \
  utils/utiltime.cpp \
  $(CELLLINK_CORE_H)

if GLIBC_BACK_COMPAT
libcelllink_util_a_SOURCES += compat/glibc_compat.cpp
endif

# cli: shared between celllink-cli and celllink-qt
libcelllink_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES)
libcelllink_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcelllink_cli_a_SOURCES = \
  rpc/client.cpp \
  $(CELLLINK_CORE_H)

nodist_libcelllink_util_a_SOURCES = $(srcdir)/obj/build.h
#

# magnachaind binary #
magnachaind_SOURCES = celllinkd.cpp
magnachaind_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES)
magnachaind_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
magnachaind_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
magnachaind_SOURCES += celllinkd-res.rc
endif

magnachaind_LDADD = \
  $(LIBCELLLINK_SERVER) \
  $(LIBCELLLINK_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBCELLLINK_UTIL) \
  $(LIBCELLLINK_WALLET) \
  $(LIBCELLLINK_ZMQ) \
  $(LIBCELLLINK_CONSENSUS) \
  $(LIBCELLLINK_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

magnachaind_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)

# magnachain-cli binary #
magnachain_cli_SOURCES = celllink-cli.cpp
magnachain_cli_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES) $(EVENT_CFLAGS)
magnachain_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
magnachain_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
magnachain_cli_SOURCES += celllink-cli-res.rc
endif

magnachain_cli_LDADD = \
  $(LIBCELLLINK_CLI) \
  $(LIBUNIVALUE) \
  $(LIBCELLLINK_UTIL) \
  $(LIBCELLLINK_CRYPTO)

magnachain_cli_LDADD += $(BOOST_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(EVENT_LIBS)
#

# magnachain-tx binary #
magnachain_tx_SOURCES = celllink-tx.cpp
magnachain_tx_CPPFLAGS = $(AM_CPPFLAGS) $(CELLLINK_INCLUDES)
magnachain_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
magnachain_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
magnachain_tx_SOURCES += celllink-tx-res.rc
endif

magnachain_tx_LDADD = \
  $(LIBCELLLINK_SERVER) \
  $(LIBCELLLINK_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBCELLLINK_UTIL) \
  $(LIBCELLLINK_WALLET) \
  $(LIBCELLLINK_ZMQ) \
  $(LIBCELLLINK_CONSENSUS) \
  $(LIBCELLLINK_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

magnachain_tx_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)
#

# celllinkconsensus library #
if BUILD_CELLLINK_LIBS
include_HEADERS = script/celllinkconsensus.h
libcelllinkconsensus_la_SOURCES = $(crypto_libcelllink_crypto_a_SOURCES) $(libcelllink_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libcelllinkconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libcelllinkconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libcelllinkconsensus_la_LIBADD = $(LIBSECP256K1)
libcelllinkconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_CELLLINK_INTERNAL
libcelllinkconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += address/*.gcda address/*.gcno
CLEANFILES += bench/*.gcda bench/*.gcno
CLEANFILES += boost/*.gcda boost/*.gcno
CLEANFILES += chain/*.gcda chain/*.gcno
CLEANFILES += coding/*.gcda coding/*.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += config/*.gcda config/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += io/*.gcda io/*.gcno
CLEANFILES += key/*.gcda key/*.gcno
CLEANFILES += leveldb/*.gcda leveldb/*.gcno
CLEANFILES += lua/*.gcda lua/*.gcno
CLEANFILES += main/*.gcda main/*.gcno
CLEANFILES += mining/*.gcda mining/*.gcno
CLEANFILES += misc/*.gcda misc/*.gcno
CLEANFILES += net/*.gcda net/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += qt/*.gcda qt/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += secp256k1/*.gcda secp256k1/*.gcno
CLEANFILES += smartcontract/*.gcda smartcontract/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += test/*.gcda test/*.gcno
CLEANFILES += thread/*.gcda thread/*.gcno
CLEANFILES += transaction/*.gcda transaction/*.gcno
CLEANFILES += ui/*.gcda ui/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += utils/*.gcda utils/*.gcno
CLEANFILES += validation/*.gcda validation/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno

DISTCLEANFILES = obj/build.h

EXTRA_DIST = $(CTAES_DIST)


config/celllink-config.h: config/stamp-h1
	@$(MAKE) -C $(top_builddir) $(subdir)/$(@)
config/stamp-h1: $(top_srcdir)/$(subdir)/config/celllink-config.h.in $(top_builddir)/config.status
	$(AM_V_at)$(MAKE) -C $(top_builddir) $(subdir)/$(@)
$(top_srcdir)/$(subdir)/config/celllink-config.h.in:  $(am__configure_deps)
	$(AM_V_at)$(MAKE) -C $(top_srcdir) $(subdir)/config/celllink-config.h.in

clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -f config.h
	-rm -rf test/__pycache__

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

.mm.o:
	$(AM_V_CXX) $(OBJCXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	  $(CPPFLAGS) $(AM_CXXFLAGS) $(QT_INCLUDES) $(AM_CXXFLAGS) $(PIE_FLAGS) $(CXXFLAGS) -c -o $@ $<

check-symbols: $(bin_PROGRAMS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(top_srcdir)/contrib/devtools/symbol-check.py < $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py < $(bin_PROGRAMS)
endif

%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(<D) $<

if EMBEDDED_LEVELDB
include Makefile.leveldb.include
endif

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_BENCH
include Makefile.bench.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

if ENABLE_QT_TESTS
include Makefile.qttest.include
endif
