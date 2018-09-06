// Copyright (c) 2012-2016 The Bitcoin Core developers
// Copyright (c) 2016-2018 The CellLink Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain/branchdb.h"
#include "primitives/transaction.h"
#include "coding/uint256.h"
#include "coding/arith_uint256.h"
#include "chain/chainparams.h"

#include "test/test_celllink.h"

#include <vector>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(branchdb_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(branchdb_chainactive)
{
    BranchDb branchdb(fs::temp_directory_path() / fs::unique_path(), 8<<20, false, false);

    uint256 branchid = uint256S("8af97c9b85ebf8b0f16b4c50cd1fa72c50dfa5d1bec93625c1dde7a4f211b65e");
    const CellChainParams& bparams = BranchParams(branchid);
    const CellBlock& genesisblock = bparams.GenesisBlock();
    
    uint256 temphash;
    uint32_t t = 0;

    uint32_t nbits = genesisblock.nBits;

    CellBranchBlockInfo* pFirstBlock = new CellBranchBlockInfo();
    pFirstBlock->branchID = branchid;

    pFirstBlock->hashPrevBlock = genesisblock.GetHash();
    pFirstBlock->nBits = nbits;
    pFirstBlock->blockHeight = 1;
    pFirstBlock->nTime = t++;
    CellVectorWriter cvw{ SER_NETWORK, INIT_PROTO_VERSION, pFirstBlock->vchStakeTxData, 0, MakeTransactionRef() };
    CellBranchBlockInfo firstBlock(*pFirstBlock);// copy 

    CellMutableTransaction mtx;
    mtx.pBranchBlockData.reset(pFirstBlock);
    
    size_t txindex = 2;
    branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex);
    
    CellBlockHeader lastchainheader;
    uint32_t lastchaintipheight;
    // 普通没发生分叉
    {//chain 1
        uint32_t preblockH = firstBlock.blockHeight;
        CellBlockHeader header;
        firstBlock.GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());

        //-----------------------------------
        mtx.pBranchBlockData.reset(new CellBranchBlockInfo());
        mtx.pBranchBlockData->branchID = branchid;

        mtx.pBranchBlockData->hashPrevBlock = header.GetHash();
        mtx.pBranchBlockData->nBits = nbits;
        mtx.pBranchBlockData->blockHeight = ++preblockH;
        mtx.pBranchBlockData->nTime = t++;
        mtx.pBranchBlockData->vchStakeTxData = firstBlock.vchStakeTxData;
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());

        //-----------------------------------
        mtx.pBranchBlockData.reset(new CellBranchBlockInfo());
        mtx.pBranchBlockData->branchID = branchid;

        mtx.pBranchBlockData->hashPrevBlock = header.GetHash();
        mtx.pBranchBlockData->nBits = nbits;
        mtx.pBranchBlockData->blockHeight = ++preblockH;
        mtx.pBranchBlockData->nTime = t++;
        mtx.pBranchBlockData->vchStakeTxData = firstBlock.vchStakeTxData;
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());
        //-----------------------------------

        lastchainheader = header;
        lastchaintipheight = preblockH;
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == preblockH);
    }

    {//chain 2 分叉来了，比前面多一个块
        uint32_t preblockH = firstBlock.blockHeight;
        CellBlockHeader header;
        firstBlock.GetBlockHeader(header);

        //-----------------------------------
        mtx.pBranchBlockData.reset(new CellBranchBlockInfo());
        mtx.pBranchBlockData->branchID = branchid;

        mtx.pBranchBlockData->hashPrevBlock = header.GetHash();
        mtx.pBranchBlockData->nBits = nbits;
        mtx.pBranchBlockData->blockHeight = ++preblockH;
        mtx.pBranchBlockData->nTime = t++;
        mtx.pBranchBlockData->vchStakeTxData = firstBlock.vchStakeTxData;
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) != header.GetHash());
        //-----------------------------------
        mtx.pBranchBlockData.reset(new CellBranchBlockInfo());
        mtx.pBranchBlockData->branchID = branchid;

        mtx.pBranchBlockData->hashPrevBlock = header.GetHash();
        mtx.pBranchBlockData->nBits = nbits;
        mtx.pBranchBlockData->blockHeight = ++preblockH;
        mtx.pBranchBlockData->nTime = t++;
        mtx.pBranchBlockData->vchStakeTxData = firstBlock.vchStakeTxData;
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) != header.GetHash());
        //-----------------------------------
        mtx.pBranchBlockData.reset(new CellBranchBlockInfo());
        mtx.pBranchBlockData->branchID = branchid;

        mtx.pBranchBlockData->hashPrevBlock = header.GetHash();
        mtx.pBranchBlockData->nBits = nbits;
        mtx.pBranchBlockData->blockHeight = ++preblockH;
        mtx.pBranchBlockData->nTime = t++;
        mtx.pBranchBlockData->vchStakeTxData = firstBlock.vchStakeTxData;
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());
        //-----------------------------------
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == preblockH);
    }
    {// chain 1 back
        uint32_t preblockH = lastchaintipheight;
        CellBlockHeader header = lastchainheader;

        //-----------------------------------
        mtx.pBranchBlockData.reset(new CellBranchBlockInfo());
        mtx.pBranchBlockData->branchID = branchid;

        mtx.pBranchBlockData->hashPrevBlock = header.GetHash();
        mtx.pBranchBlockData->nBits = nbits;
        mtx.pBranchBlockData->blockHeight = ++preblockH;
        mtx.pBranchBlockData->nTime = t++;
        mtx.pBranchBlockData->vchStakeTxData = firstBlock.vchStakeTxData;
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) != header.GetHash());
        //-----------------------------------
        mtx.pBranchBlockData.reset(new CellBranchBlockInfo());
        mtx.pBranchBlockData->branchID = branchid;

        mtx.pBranchBlockData->hashPrevBlock = header.GetHash();
        mtx.pBranchBlockData->nBits = nbits;
        mtx.pBranchBlockData->blockHeight = ++preblockH;
        mtx.pBranchBlockData->nTime = t++;
        mtx.pBranchBlockData->vchStakeTxData = firstBlock.vchStakeTxData;
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());
        //-----------------------------------
        mtx.pBranchBlockData.reset(new CellBranchBlockInfo());
        mtx.pBranchBlockData->branchID = branchid;

        mtx.pBranchBlockData->hashPrevBlock = header.GetHash();
        mtx.pBranchBlockData->nBits = nbits;
        mtx.pBranchBlockData->blockHeight = ++preblockH;
        mtx.pBranchBlockData->nTime = t++;
        mtx.pBranchBlockData->vchStakeTxData = firstBlock.vchStakeTxData;
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex);

        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());
        //-----------------------------------
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == preblockH);
    }

}

void CreateTestTxToBlock(std::shared_ptr<CellBlock> &pblockNew, const uint256 &branchid, CellBlockHeader &preHeader, 
    const uint32_t &nbits, uint32_t &preblockH, uint32_t &branchblocktime, std::vector<unsigned char> &vchStakeTxData)
{
    CellMutableTransaction mtx;
    mtx.nVersion = CellTransaction::SYNC_BRANCH_INFO;
    mtx.pBranchBlockData = std::make_shared<CellBranchBlockInfo>();
    mtx.pBranchBlockData->branchID = branchid;

    mtx.pBranchBlockData->hashPrevBlock = preHeader.GetHash();
    mtx.pBranchBlockData->nBits = nbits;
    mtx.pBranchBlockData->blockHeight = ++preblockH;
    mtx.pBranchBlockData->nTime = branchblocktime++;
    mtx.pBranchBlockData->vchStakeTxData = vchStakeTxData;

    pblockNew->vtx.push_back(MakeTransactionRef(mtx));
}

BOOST_AUTO_TEST_CASE(branchdb_flush)
{
    BranchDb branchdb(fs::temp_directory_path() / fs::unique_path(), 8 << 20, false, false);

    uint256 branchid = uint256S("8af97c9b85ebf8b0f16b4c50cd1fa72c50dfa5d1bec93625c1dde7a4f211b65e");
    const CellChainParams& bparams = BranchParams(branchid);
    const CellBlock& genesisblock = bparams.GenesisBlock();

    uint32_t nbits = genesisblock.nBits;

    std::vector<unsigned char> vchStakeTxData;
    CellVectorWriter cvw{ SER_NETWORK, INIT_PROTO_VERSION, vchStakeTxData, 0, MakeTransactionRef() };

    uint32_t mainblocktime = 0;
    uint32_t branchblocktime = 0;
    uint32_t preblockH = 0;
    CellBlockHeader preHeader = genesisblock;
    {
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        pblockNew->nBits = nbits;
        pblockNew->nTime = mainblocktime++;

        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);

        //connect block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == preHeader.GetHash());
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == 2);

        //disconnect block
        branchdb.Flush(pblockNew, false);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == genesisblock.GetHash());
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == 0);

        //re-connnect block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == preHeader.GetHash());
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == 2);
    }
    {//another new block
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        pblockNew->nBits = nbits;
        pblockNew->nTime = mainblocktime++;

        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);

        //connect a new block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == preHeader.GetHash());
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == 5);
    }
}



BOOST_AUTO_TEST_SUITE_END()
