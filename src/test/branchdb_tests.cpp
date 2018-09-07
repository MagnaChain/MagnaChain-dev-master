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
#include "validation/validation.h"

BOOST_FIXTURE_TEST_SUITE(branchdb_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(branchdb_chainactive)
{
    BranchDb branchdb(fs::temp_directory_path() / fs::unique_path(), 8<<20, false, false);

    uint256 branchid = uint256S("8af97c9b85ebf8b0f16b4c50cd1fa72c50dfa5d1bec93625c1dde7a4f211b65e");
    const CellChainParams& bparams = BranchParams(branchid);
    const CellBlock& genesisblock = bparams.GenesisBlock();
    
    uint256 temphash;
    uint32_t t = 0;
    std::set<uint256> modifyBranch;

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
    branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex, modifyBranch);
    
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
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex, modifyBranch);
        
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
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex, modifyBranch);
        
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
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex, modifyBranch);
        
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
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex, modifyBranch);
        
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
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex, modifyBranch);
        
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
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex, modifyBranch);
        
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
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex, modifyBranch);
        
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
        branchdb.AddBlockInfoTxData(MakeTransactionRef(mtx), temphash, txindex, modifyBranch);

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
    
    BOOST_CHECK(branchid == uint256S("8af97c9b85ebf8b0f16b4c50cd1fa72c50dfa5d1bec93625c1dde7a4f211b65e"));
    BOOST_CHECK(branchid != uint256S("9af97c9b85ebf8b0f16b4c50cd1fa72c50dfa5d1bec93625c1dde7a4f211b65e"));
    
    const CellChainParams& bparams = BranchParams(branchid);
    const CellBlock& genesisblock = bparams.GenesisBlock();

    uint32_t nbits = genesisblock.nBits;

    std::vector<unsigned char> vchStakeTxData;
    CellVectorWriter cvw{ SER_NETWORK, INIT_PROTO_VERSION, vchStakeTxData, 0, MakeTransactionRef() };

    uint32_t mainblocktime = 0;
    uint32_t branchblocktime = 0;
    uint32_t preblockH = 0;
    CellBlockHeader preHeader = genesisblock;
    CellBlockHeader forkHeader;
    uint32_t forkHeight;
    uint256 mainpreblockhash;
    int mainblockheight = 1;
    CellBlockIndex* pprev = nullptr;
    std::vector<uint256> expertBranchChain;
    expertBranchChain.emplace_back(preHeader.GetHash());
    {// block 1
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        pblockNew->nBits = nbits;
        pblockNew->nTime = mainblocktime++;
        //pblockNew->hashPrevBlock;
        CellBlockIndex* pindexNew = new CellBlockIndex(*pblockNew);
        pindexNew->nHeight = mainblockheight++;
        pindexNew->pprev = pprev; pprev = pindexNew;
        mainpreblockhash = pblockNew->GetHash();
        BlockMap::iterator mi = mapBlockIndex.insert(std::make_pair(pblockNew->GetHash() , pindexNew)).first;
        pindexNew->phashBlock = &((*mi).first);

        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        expertBranchChain.emplace_back(preHeader.GetHash());
        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        expertBranchChain.emplace_back(preHeader.GetHash());

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

    std::vector<uint256> preBlockChain;
    {//another new block
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        pblockNew->nBits = nbits;
        pblockNew->nTime = mainblocktime++;
        pblockNew->hashPrevBlock = mainpreblockhash;
        CellBlockIndex* pindexNew = new CellBlockIndex(*pblockNew);
        pindexNew->nHeight = mainblockheight++;
        pindexNew->pprev = pprev; pprev = pindexNew;
        mainpreblockhash = pblockNew->GetHash();
        BlockMap::iterator mi = mapBlockIndex.insert(std::make_pair(pblockNew->GetHash(), pindexNew)).first;
        pindexNew->phashBlock = &((*mi).first);

        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        expertBranchChain.emplace_back(preHeader.GetHash());
        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);   forkHeader = preHeader; forkHeight = preblockH;
        expertBranchChain.emplace_back(preHeader.GetHash());
        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        expertBranchChain.emplace_back(preHeader.GetHash());
        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        expertBranchChain.emplace_back(preHeader.GetHash());

        //connect a new block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == preHeader.GetHash());
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size()-1);
        BranchData blockdata = branchdb.GetBranchData(branchid);
        BOOST_CHECK(expertBranchChain.size() == blockdata.vecChainActive.size());
        for (int i = 0; i < expertBranchChain.size(); i++)
        {
            BOOST_CHECK(expertBranchChain[i] == blockdata.vecChainActive[i]);
        }
        preBlockChain = expertBranchChain;
    }
    {//add empty block
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        pblockNew->nBits = nbits;
        pblockNew->nTime = mainblocktime++;
        pblockNew->hashPrevBlock = mainpreblockhash;
        CellBlockIndex* pindexNew = new CellBlockIndex(*pblockNew);
        pindexNew->nHeight = mainblockheight++;
        pindexNew->pprev = pprev; pprev = pindexNew;
        mainpreblockhash = pblockNew->GetHash();
        BlockMap::iterator mi = mapBlockIndex.insert(std::make_pair(pblockNew->GetHash(), pindexNew)).first;
        pindexNew->phashBlock = &((*mi).first);

        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == preHeader.GetHash());
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == 6);
    }
    {//add block with branch fork 
        preHeader = forkHeader;
        preblockH = forkHeight;

        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        pblockNew->nBits = nbits;
        pblockNew->nTime = mainblocktime++;
        pblockNew->hashPrevBlock = mainpreblockhash;
        CellBlockIndex* pindexNew = new CellBlockIndex(*pblockNew);
        pindexNew->nHeight = mainblockheight++;
        pindexNew->pprev = pprev; pprev = pindexNew;
        mainpreblockhash = pblockNew->GetHash();
        BlockMap::iterator mi = mapBlockIndex.insert(std::make_pair(pblockNew->GetHash(), pindexNew)).first;
        pindexNew->phashBlock = &((*mi).first);

        expertBranchChain.pop_back(); expertBranchChain.pop_back();// pop number base fork node

        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        expertBranchChain.emplace_back(preHeader.GetHash());
        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        expertBranchChain.emplace_back(preHeader.GetHash());
        CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
        pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
        expertBranchChain.emplace_back(preHeader.GetHash());

        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == preHeader.GetHash());
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == 7);

        BranchData blockdata = branchdb.GetBranchData(branchid);
        BOOST_CHECK(expertBranchChain.size() == blockdata.vecChainActive.size());
        for (int i = 0; i < expertBranchChain.size(); i++)
        {
            BOOST_CHECK(expertBranchChain[i] == blockdata.vecChainActive[i]);
        }

        branchdb.Flush(pblockNew, false);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == 6);
        expertBranchChain = preBlockChain;
        blockdata = branchdb.GetBranchData(branchid);
        BOOST_CHECK(expertBranchChain.size() == blockdata.vecChainActive.size());
        for (int i = 0; i < expertBranchChain.size(); i++)
        {
            BOOST_CHECK(expertBranchChain[i] == blockdata.vecChainActive[i]);
        }
    }
}



BOOST_AUTO_TEST_SUITE_END()
