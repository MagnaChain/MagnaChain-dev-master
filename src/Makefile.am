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

MAGNACHAIN_CONFIG_INCLUDES=-I$(builddir)/config
MAGNACHAIN_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

MAGNACHAIN_INCLUDES += -I$(srcdir)/secp256k1/include -I$(srcdir)/lua
MAGNACHAIN_INCLUDES += $(UNIVALUE_CFLAGS)

MAGNACHAIN_MONITOR_INCLUDES = -I$(srcdir)/monitor/mysqlcppconn

LIBMAGNACHAIN_LUA=libmagnachain_lua.a
LIBMAGNACHAIN_SERVER=libmagnachain_server.a
LIBMAGNACHAIN_COMMON=libmagnachain_common.a
LIBMAGNACHAIN_CONSENSUS=libmagnachain_consensus.a
LIBMAGNACHAIN_CLI=libmagnachain_cli.a
LIBMAGNACHAIN_UTIL=libmagnachain_util.a
LIBMAGNACHAIN_CRYPTO=crypto/libmagnachain_crypto.a
LIBMAGNACHAINQT=qt/libmagnachainqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

if ENABLE_ZMQ
LIBMAGNACHAIN_ZMQ=libmagnachain_zmq.a
endif
if BUILD_MAGNACHAIN_LIBS
LIBMAGNACHAINCONSENSUS=libmagnachainconsensus.la
endif
if BUILD_MAGNACHAIN_MONITOR
LIBMAGNACHAIN_MYSQLCPPCONN=libmagnachain_mysqlcppconn.a
endif
if ENABLE_WALLET
LIBMAGNACHAIN_WALLET=libmagnachain_wallet.a
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBMAGNACHAIN_CRYPTO) \
  $(LIBMAGNACHAIN_UTIL) \
  $(LIBMAGNACHAIN_COMMON) \
  $(LIBMAGNACHAIN_CONSENSUS) \
  $(LIBMAGNACHAIN_SERVER) \
  $(LIBMAGNACHAIN_LUA) \
  $(LIBMAGNACHAIN_CLI) \
  $(LIBMAGNACHAIN_WALLET) \
  $(LIBMAGNACHAIN_ZMQ) \
  $(LIBMAGNACHAIN_MYSQLCPPCONN)

lib_LTLIBRARIES = $(LIBMAGNACHAINCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_MAGNACHAIND
  bin_PROGRAMS += magnachaind
endif

if BUILD_MAGNACHAIN_MONITOR
  bin_PROGRAMS += magnachain-monitor
endif

if BUILD_MAGNACHAIN_UTILS
  bin_PROGRAMS += magnachain-cli magnachain-tx
endif

.PHONY: FORCE check-symbols check-security
# magnachain core #
MAGNACHAIN_CORE_H = \
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
  zmq/zmqpublishnotifier.h \
  smartcontract/smartcontract.h \
  smartcontract/contractdb.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh "$(abs_top_builddir)/src/obj/build.h" \
	  "$(abs_top_srcdir)"
libmagnachain_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between magnachaind and magnachain-qt
libmagnachain_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libmagnachain_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmagnachain_server_a_SOURCES = \
  address/addrdb.cpp \
  address/addrman.cpp \
  transaction/bloom.cpp \
  transaction/blockencodings.cpp \
  chain/chain.cpp \
  validation/checkpoints.cpp \
  consensus/consensus.cpp \
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
  $(MAGNACHAIN_CORE_H)

if ENABLE_ZMQ
libmagnachain_zmq_a_CPPFLAGS = $(MAGNACHAIN_INCLUDES) $(ZMQ_CFLAGS)
libmagnachain_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmagnachain_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp
endif


# wallet: shared between magnachaind and magnachain-qt, but only linked
# when wallet enabled
libmagnachain_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES)
libmagnachain_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmagnachain_wallet_a_SOURCES = \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/feebumper.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  $(MAGNACHAIN_CORE_H)

libmagnachain_mysqlcppconn_a_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS) $(MYSQL_CLIENT_CFLAGS) $(MAGNACHAIN_MONITOR_INCLUDES)
libmagnachain_mysqlcppconn_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmagnachain_mysqlcppconn_a_SOURCES = \
  monitor/mysqlcppconn/driver/nativeapi/library_loader.cpp \
  monitor/mysqlcppconn/driver/nativeapi/mysql_client_api.cpp \
  monitor/mysqlcppconn/driver/nativeapi/mysql_native_connection_wrapper.cpp \
  monitor/mysqlcppconn/driver/nativeapi/mysql_native_driver_wrapper.cpp \
  monitor/mysqlcppconn/driver/nativeapi/mysql_native_resultset_wrapper.cpp \
  monitor/mysqlcppconn/driver/nativeapi/mysql_native_statement_wrapper.cpp \
  monitor/mysqlcppconn/driver/mysql_art_resultset.cpp \
  monitor/mysqlcppconn/driver/mysql_art_rset_metadata.cpp \
  monitor/mysqlcppconn/driver/mysql_connection.cpp \
  monitor/mysqlcppconn/driver/mysql_debug.cpp \
  monitor/mysqlcppconn/driver/mysql_driver.cpp \
  monitor/mysqlcppconn/driver/mysql_metadata.cpp \
  monitor/mysqlcppconn/driver/mysql_parameter_metadata.cpp \
  monitor/mysqlcppconn/driver/mysql_prepared_statement.cpp \
  monitor/mysqlcppconn/driver/mysql_ps_resultset.cpp \
  monitor/mysqlcppconn/driver/mysql_ps_resultset_metadata.cpp \
  monitor/mysqlcppconn/driver/mysql_resultbind.cpp \
  monitor/mysqlcppconn/driver/mysql_resultset.cpp \
  monitor/mysqlcppconn/driver/mysql_resultset_metadata.cpp \
  monitor/mysqlcppconn/driver/mysql_statement.cpp \
  monitor/mysqlcppconn/driver/mysql_uri.cpp \
  monitor/mysqlcppconn/driver/mysql_util.cpp \
  monitor/mysqlcppconn/driver/mysql_warning.cpp

# crypto primitives library
crypto_libmagnachain_crypto_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES) $(MAGNACHAIN_CONFIG_INCLUDES)
crypto_libmagnachain_crypto_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libmagnachain_crypto_a_SOURCES = \
  crypto/ctaes/ctaes.c \
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
  crypto/sha512.h
  

if EXPERIMENTAL_ASM
crypto_libmagnachain_crypto_a_SOURCES += crypto/sha256_sse4.cpp
endif

libmagnachain_lua_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES)
libmagnachain_lua_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmagnachain_lua_a_SOURCES = \
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
  lua/lua_cmsgpack.c

# consensus: shared between all executables that validate any consensus rules.
libmagnachain_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES)
libmagnachain_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmagnachain_consensus_a_SOURCES = \
  misc/amount.h \
  coding/arith_uint256.cpp \
  coding/arith_uint256.h \
  consensus/consensus.cpp \
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
  script/magnachainconsensus.cpp \
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

# common: shared between magnachaind, and magnachain-qt and non-server tools
libmagnachain_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES)
libmagnachain_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmagnachain_common_a_SOURCES = \
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
  $(MAGNACHAIN_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libmagnachain_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES)
libmagnachain_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmagnachain_util_a_SOURCES = \
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
  $(MAGNACHAIN_CORE_H)

if GLIBC_BACK_COMPAT
libmagnachain_util_a_SOURCES += compat/glibc_compat.cpp
endif

# cli: shared between magnachain-cli and magnachain-qt
libmagnachain_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES)
libmagnachain_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libmagnachain_cli_a_SOURCES = \
  rpc/client.cpp \
  $(MAGNACHAIN_CORE_H)

nodist_libmagnachain_util_a_SOURCES = $(srcdir)/obj/build.h
#

# magnachaind binary #
magnachaind_SOURCES = magnachaind.cpp
magnachaind_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES)
magnachaind_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
magnachaind_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
magnachaind_SOURCES += magnachaind-res.rc
endif

magnachaind_LDADD = \
  $(LIBMAGNACHAIN_SERVER) \
  $(LIBMAGNACHAIN_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBMAGNACHAIN_UTIL) \
  $(LIBMAGNACHAIN_WALLET) \
  $(LIBMAGNACHAIN_ZMQ) \
  $(LIBMAGNACHAIN_CONSENSUS) \
  $(LIBMAGNACHAIN_CRYPTO) \
  $(LIBMAGNACHAIN_LUA) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

magnachaind_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)

# magnachain monitor binary #
magnachain_monitor_SOURCES = \
	magnachain-monitor.cpp \
	monitor/database.cpp \
	monitor/monitorinit.cpp \
	monitor/net_processing.cpp

magnachain_monitor_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES) $(MYSQL_CLIENT_CFLAGS) $(MAGNACHAIN_MONITOR_INCLUDES)
magnachain_monitor_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
magnachain_monitor_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

magnachain_monitor_LDADD = \
  $(LIBMAGNACHAIN_SERVER) \
  $(LIBMAGNACHAIN_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBMAGNACHAIN_UTIL) \
  $(LIBMAGNACHAIN_WALLET) \
  $(LIBMAGNACHAIN_ZMQ) \
  $(LIBMAGNACHAIN_CONSENSUS) \
  $(LIBMAGNACHAIN_CRYPTO) \
  $(LIBMAGNACHAIN_LUA) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1) \
  $(LIBMAGNACHAIN_MYSQLCPPCONN)

magnachain_monitor_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS) $(MYSQL_CLIENT_LIBS)

# magnachain-cli binary #
magnachain_cli_SOURCES = magnachain-cli.cpp
magnachain_cli_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES) $(EVENT_CFLAGS)
magnachain_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
magnachain_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
magnachain_cli_SOURCES += magnachain-cli-res.rc
endif

magnachain_cli_LDADD = \
  $(LIBMAGNACHAIN_CLI) \
  $(LIBUNIVALUE) \
  $(LIBMAGNACHAIN_UTIL) \
  $(LIBMAGNACHAIN_CRYPTO)

magnachain_cli_LDADD += $(BOOST_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(EVENT_LIBS)
#

# magnachain-tx binary #
magnachain_tx_SOURCES = magnachain-tx.cpp
magnachain_tx_CPPFLAGS = $(AM_CPPFLAGS) $(MAGNACHAIN_INCLUDES)
magnachain_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
magnachain_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
magnachain_tx_SOURCES += magnachain-tx-res.rc
endif

magnachain_tx_LDADD = \
  $(LIBMAGNACHAIN_SERVER) \
  $(LIBMAGNACHAIN_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBMAGNACHAIN_UTIL) \
  $(LIBMAGNACHAIN_WALLET) \
  $(LIBMAGNACHAIN_ZMQ) \
  $(LIBMAGNACHAIN_CONSENSUS) \
  $(LIBMAGNACHAIN_CRYPTO) \
  $(LIBMAGNACHAIN_LUA) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

magnachain_tx_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)
#

# magnachainconsensus library #
if BUILD_MAGNACHAIN_LIBS
include_HEADERS = script/magnachainconsensus.h
libmagnachainconsensus_la_SOURCES = $(crypto_libmagnachain_crypto_a_SOURCES) $(libmagnachain_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libmagnachainconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libmagnachainconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libmagnachainconsensus_la_LIBADD = $(LIBSECP256K1)
libmagnachainconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_MAGNACHAIN_INTERNAL
libmagnachainconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

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
CLEANFILES += monitor/*.gcda monitor/*.gcno
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


config/magnachain-config.h: config/stamp-h1
	@$(MAKE) -C $(top_builddir) $(subdir)/$(@)
config/stamp-h1: $(top_srcdir)/$(subdir)/config/magnachain-config.h.in $(top_builddir)/config.status
	$(AM_V_at)$(MAKE) -C $(top_builddir) $(subdir)/$(@)
$(top_srcdir)/$(subdir)/config/magnachain-config.h.in:  $(am__configure_deps)
	$(AM_V_at)$(MAKE) -C $(top_srcdir) $(subdir)/config/magnachain-config.h.in

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
