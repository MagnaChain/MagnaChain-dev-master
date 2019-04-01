// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "magnachain-config.h"
#endif

#include "consensus/merkle.h"
#include "primitives/block.h"
#include "script/script.h"
#include "address/addrman.h"
#include "chain/chain.h"
#include "transaction/coins.h"
#include "transaction/compressor.h"
#include "net/net.h"
#include "net/protocol.h"
#include "io/streams.h"
#include "transaction/undo.h"
#include "misc/version.h"
#include "key/pubkey.h"

#include <stdint.h>
#include <unistd.h>

#include <algorithm>
#include <vector>

enum TEST_ID {
    CBLOCK_DESERIALIZE=0,
    CTRANSACTION_DESERIALIZE,
    CBLOCKLOCATOR_DESERIALIZE,
    CBLOCKMERKLEROOT,
    CADDRMAN_DESERIALIZE,
    CBLOCKHEADER_DESERIALIZE,
    CBANENTRY_DESERIALIZE,
    CTXUNDO_DESERIALIZE,
    CBLOCKUNDO_DESERIALIZE,
    CCOINS_DESERIALIZE,
    CNETADDR_DESERIALIZE,
    CSERVICE_DESERIALIZE,
    CMESSAGEHEADER_DESERIALIZE,
    CADDRESS_DESERIALIZE,
    CINV_DESERIALIZE,
    CBLOOMFILTER_DESERIALIZE,
    CDISKBLOCKINDEX_DESERIALIZE,
    CTXOUTCOMPRESSOR_DESERIALIZE,
    TEST_ID_END
};

bool read_stdin(std::vector<char> &data) {
    char buffer[1024];
    ssize_t length=0;
    while((length = read(STDIN_FILENO, buffer, 1024)) > 0) {
        data.insert(data.end(), buffer, buffer+length);

        if (data.size() > (1<<20)) return false;
    }
    return length==0;
}

int do_fuzz()
{
    std::vector<char> buffer;
    if (!read_stdin(buffer)) return 0;

    if (buffer.size() < sizeof(uint32_t)) return 0;

    uint32_t test_id = 0xffffffff;
    memcpy(&test_id, &buffer[0], sizeof(uint32_t));
    buffer.erase(buffer.begin(), buffer.begin() + sizeof(uint32_t));

    if (test_id >= TEST_ID_END) return 0;

    MCDataStream ds(buffer, SER_NETWORK, INIT_PROTO_VERSION);
    try {
        int nVersion;
        ds >> nVersion;
        ds.SetVersion(nVersion);
    } catch (const std::ios_base::failure& e) {
        return 0;
    }

    switch(test_id) {
        case CBLOCK_DESERIALIZE:
        {
            try
            {
                MCBlock block;
                ds >> block;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CTRANSACTION_DESERIALIZE:
        {
            try
            {
                MCTransaction tx(deserialize, ds);
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOCKLOCATOR_DESERIALIZE:
        {
            try
            {
                MCBlockLocator bl;
                ds >> bl;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOCKMERKLEROOT:
        {
            try
            {
                MCBlock block;
                ds >> block;
                bool mutated;
                BlockMerkleRoot(block, &mutated);
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CADDRMAN_DESERIALIZE:
        {
            try
            {
                MCAddrMan am;
                ds >> am;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOCKHEADER_DESERIALIZE:
        {
            try
            {
                MCBlockHeader bh;
                ds >> bh;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBANENTRY_DESERIALIZE:
        {
            try
            {
                MCBanEntry be;
                ds >> be;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CTXUNDO_DESERIALIZE:
        {
            try
            {
                MCTxUndo tu;
                ds >> tu;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOCKUNDO_DESERIALIZE:
        {
            try
            {
                MCBlockUndo bu;
                ds >> bu;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CCOINS_DESERIALIZE:
        {
            try
            {
                Coin coin;
                ds >> coin;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CNETADDR_DESERIALIZE:
        {
            try
            {
                MCNetAddr na;
                ds >> na;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CSERVICE_DESERIALIZE:
        {
            try
            {
                MCService s;
                ds >> s;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CMESSAGEHEADER_DESERIALIZE:
        {
            MCMessageHeader::MessageStartChars pchMessageStart = {0x00, 0x00, 0x00, 0x00};
            try
            {
                MCMessageHeader mh(pchMessageStart);
                ds >> mh;
                if (!mh.IsValid(pchMessageStart)) {return 0;}
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CADDRESS_DESERIALIZE:
        {
            try
            {
                MCAddress a;
                ds >> a;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CINV_DESERIALIZE:
        {
            try
            {
                MCInv i;
                ds >> i;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CBLOOMFILTER_DESERIALIZE:
        {
            try
            {
                MCBloomFilter bf;
                ds >> bf;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CDISKBLOCKINDEX_DESERIALIZE:
        {
            try
            {
                MCDiskBlockIndex dbi;
                ds >> dbi;
            } catch (const std::ios_base::failure& e) {return 0;}
            break;
        }
        case CTXOUTCOMPRESSOR_DESERIALIZE:
        {
            MCTxOut to;
            MCTxOutCompressor toc(to);
            try
            {
                ds >> toc;
            } catch (const std::ios_base::failure& e) {return 0;}

            break;
        }
        default:
            return 0;
    }
    return 0;
}

int main(int argc, char **argv)
{
    ECCVerifyHandle globalVerifyHandle;
#ifdef __AFL_INIT
    // Enable AFL deferred forkserver mode. Requires compilation using
    // afl-clang-fast++. See fuzzing.md for details.
    __AFL_INIT();
#endif

#ifdef __AFL_LOOP
    // Enable AFL persistent mode. Requires compilation using afl-clang-fast++.
    // See fuzzing.md for details.
    while (__AFL_LOOP(1000)) {
        do_fuzz();
    }
    return 0;
#else
    return do_fuzz();
#endif
}
