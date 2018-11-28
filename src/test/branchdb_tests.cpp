// Copyright (c) 2012-2016 The Bitcoin Core developers
// Copyright (c) 2016-2019 The MagnaChain Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain/branchdb.h"
#include "primitives/transaction.h"
#include "coding/uint256.h"
#include "coding/arith_uint256.h"
#include "chain/chainparams.h"

#include "test/test_magnachain.h"

#include <vector>

#include <boost/test/unit_test.hpp>
#include "validation/validation.h"

class BranchDbTest : public BranchDb
{
public:
    BranchDbTest(const fs::path& path, size_t nCacheSize, bool fMemory, bool fWipe) :BranchDb(path, nCacheSize, fMemory, fWipe){}
    void AddBlockInfoTxData(CellTransactionRef &transaction, const uint256 &mainBlockHash, const size_t iTxVtxIndex, std::set<uint256>& modifyBranch)
    {
        BranchDb::AddBlockInfoTxData(transaction, mainBlockHash, iTxVtxIndex, modifyBranch);
    }
};

void AddBlockInfoTx(CellMutableTransaction &mtx, const uint256 &branchid, CellBlockHeader &header, const uint32_t &nbits, uint32_t &preblockH, uint32_t &t, CellBranchBlockInfo &firstBlock, BranchDbTest &branchdb, uint256 &temphash, const size_t &txindex, std::set<uint256> &modifyBranch)
{
    mtx.pBranchBlockData.reset(new CellBranchBlockInfo());
    mtx.pBranchBlockData->branchID = branchid;

    mtx.pBranchBlockData->hashPrevBlock = header.GetHash();
    mtx.pBranchBlockData->nBits = nbits;
    mtx.pBranchBlockData->blockHeight = ++preblockH;
    mtx.pBranchBlockData->nTime = t++;
    mtx.pBranchBlockData->vchStakeTxData = firstBlock.vchStakeTxData;
    CellTransactionRef ptx = MakeTransactionRef(mtx);
    branchdb.AddBlockInfoTxData(ptx, temphash, txindex, modifyBranch);
}

BOOST_FIXTURE_TEST_SUITE(branchdb_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(branchdb_chainactive)
{
    BranchDbTest branchdb(fs::temp_directory_path() / fs::unique_path(), 8<<20, false, false);

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
    CellTransactionRef ptx = MakeTransactionRef(mtx);
    branchdb.AddBlockInfoTxData(ptx, temphash, txindex, modifyBranch);
    
    CellBlockHeader lastchainheader;
    uint32_t lastchaintipheight;
    // 普通没发生分叉
    {//chain 1
        uint32_t preblockH = firstBlock.blockHeight;
        CellBlockHeader header;
        firstBlock.GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());

        //-----------------------------------
        AddBlockInfoTx(mtx, branchid, header, nbits, preblockH, t, firstBlock, branchdb, temphash, txindex, modifyBranch);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());

        //-----------------------------------
        AddBlockInfoTx(mtx, branchid, header, nbits, preblockH, t, firstBlock, branchdb, temphash, txindex, modifyBranch);
        
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
        AddBlockInfoTx(mtx, branchid, header, nbits, preblockH, t, firstBlock, branchdb, temphash, txindex, modifyBranch);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) != header.GetHash());
        //-----------------------------------
        AddBlockInfoTx(mtx, branchid, header, nbits, preblockH, t, firstBlock, branchdb, temphash, txindex, modifyBranch);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) != header.GetHash());
        //-----------------------------------
        AddBlockInfoTx(mtx, branchid, header, nbits, preblockH, t, firstBlock, branchdb, temphash, txindex, modifyBranch);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());
        //-----------------------------------
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == preblockH);
    }
    {// chain 1 back
        uint32_t preblockH = lastchaintipheight;
        CellBlockHeader header = lastchainheader;

        //-----------------------------------
        AddBlockInfoTx(mtx, branchid, header, nbits, preblockH, t, firstBlock, branchdb, temphash, txindex, modifyBranch);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) != header.GetHash());
        //-----------------------------------
        AddBlockInfoTx(mtx, branchid, header, nbits, preblockH, t, firstBlock, branchdb, temphash, txindex, modifyBranch);
        
        mtx.pBranchBlockData->GetBlockHeader(header);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == header.GetHash());
        //-----------------------------------
        AddBlockInfoTx(mtx, branchid, header, nbits, preblockH, t, firstBlock, branchdb, temphash, txindex, modifyBranch);

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
    BranchDbTest branchdb(fs::temp_directory_path() / fs::unique_path(), 8 << 20, false, false);

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
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size()-1);
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
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size()-1);

        BranchData blockdata = branchdb.GetBranchData(branchid);
        BOOST_CHECK(expertBranchChain.size() == blockdata.vecChainActive.size());
        for (int i = 0; i < expertBranchChain.size(); i++)
        {
            BOOST_CHECK(expertBranchChain[i] == blockdata.vecChainActive[i]);
        }

        branchdb.Flush(pblockNew, false);
        expertBranchChain = preBlockChain;
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size()-1);
        blockdata = branchdb.GetBranchData(branchid);
        BOOST_CHECK(expertBranchChain.size() == blockdata.vecChainActive.size());
        for (int i = 0; i < expertBranchChain.size(); i++)
        {
            BOOST_CHECK(expertBranchChain[i] == blockdata.vecChainActive[i]);
        }
    }
}

void NewMainBlockHead(std::shared_ptr<CellBlock> &pblockNew, const uint32_t &nbits, uint32_t &mainblocktime, uint256 &mainpreblockhash, int &mainblockheight, CellBlockIndex * &pprev)
{
    pblockNew->nBits = nbits;
    pblockNew->nTime = mainblocktime++;
    pblockNew->hashPrevBlock = mainpreblockhash;
    CellBlockIndex* pindexNew = new CellBlockIndex(*pblockNew);
    pindexNew->nHeight = mainblockheight++;
    pindexNew->pprev = pprev; pprev = pindexNew;
    mainpreblockhash = pblockNew->GetHash();
    BlockMap::iterator mi = mapBlockIndex.insert(std::make_pair(pblockNew->GetHash(), pindexNew)).first;
    pindexNew->phashBlock = &((*mi).first);
}

BOOST_AUTO_TEST_CASE(branchdb_flushreportprove)
{
    BranchDbTest branchdb(fs::temp_directory_path() / fs::unique_path(), 8 << 20, false, false);
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
    CellBlockHeader forkHeader;
    uint32_t forkHeight;
    uint256 mainpreblockhash = genesisblock.GetHash();
    int mainblockheight = 1;
    CellBlockIndex* pprev = nullptr;
    std::vector<uint256> expertBranchChain;
    expertBranchChain.emplace_back(preHeader.GetHash());

    std::shared_ptr<CellBlock> pblock2;
    {// block 1
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        NewMainBlockHead(pblockNew, nbits, mainblocktime, mainpreblockhash, mainblockheight, pprev);

        for (int i=0; i<10; i++)
        {
            CreateTestTxToBlock(pblockNew, branchid, preHeader, nbits, preblockH, branchblocktime, vchStakeTxData);
            pblockNew->vtx.back()->pBranchBlockData->GetBlockHeader(preHeader);
            expertBranchChain.emplace_back(preHeader.GetHash());
        }
 
        //connect block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 1]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 1);
    }
    //--------------------
    {// block 2 举报 -3
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        NewMainBlockHead(pblockNew, nbits, mainblocktime, mainpreblockhash, mainblockheight, pprev);

        //add report tx---------
        CellMutableTransaction mtx;
        mtx.nVersion = CellTransaction::REPORT_CHEAT;
        mtx.pPMT = std::make_shared<CellSpvProof>();
        mtx.pReportData = std::make_shared<ReportData>();
        mtx.pReportData->reporttype = ReportType::REPORT_TX;
        mtx.pReportData->reportedBranchId = branchid;
        mtx.pReportData->reportedBlockHash = expertBranchChain[expertBranchChain.size() - 3];
        mtx.pReportData->reportedTxHash = uint256S("tx00000000000000000000000000000000000000000000000000000000000001");

        pblockNew->vtx.push_back(MakeTransactionRef(mtx));
        //------------

        //connect block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 4]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 4);

        pblock2 = pblockNew;
    }
    {// block 3 举报 -4
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        NewMainBlockHead(pblockNew, nbits, mainblocktime, mainpreblockhash, mainblockheight, pprev);

        //add report tx---------
        CellMutableTransaction mtx;
        mtx.nVersion = CellTransaction::REPORT_CHEAT;
        mtx.pPMT = std::make_shared<CellSpvProof>();
        mtx.pReportData = std::make_shared<ReportData>();
        mtx.pReportData->reporttype = ReportType::REPORT_TX;
        mtx.pReportData->reportedBranchId = branchid;
        mtx.pReportData->reportedBlockHash = expertBranchChain[expertBranchChain.size() - 4];
        mtx.pReportData->reportedTxHash = uint256S("tx00000000000000000000000000000000000000000000000000000000000001");

        pblockNew->vtx.push_back(MakeTransactionRef(mtx));
        //------------

        //connect block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 5]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 5);

        //disconnect block
        branchdb.Flush(pblockNew, false);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 4]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 4);

        //disconnect
        branchdb.Flush(pblock2, false);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == preHeader.GetHash());
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 1);

        //reconnect
        branchdb.Flush(pblock2, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 4]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 4);

        //reconnect
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 5]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 5);
    }
    {// block 4 举报 -2
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        NewMainBlockHead(pblockNew, nbits, mainblocktime, mainpreblockhash, mainblockheight, pprev);

        //add report tx---------
        CellMutableTransaction mtx;
        mtx.nVersion = CellTransaction::REPORT_CHEAT;
        mtx.pPMT = std::make_shared<CellSpvProof>();
        mtx.pReportData = std::make_shared<ReportData>();
        mtx.pReportData->reporttype = ReportType::REPORT_TX;
        mtx.pReportData->reportedBranchId = branchid;
        mtx.pReportData->reportedBlockHash = expertBranchChain[expertBranchChain.size() - 2];
        mtx.pReportData->reportedTxHash = uint256S("tx00000000000000000000000000000000000000000000000000000000000001");

        pblockNew->vtx.push_back(MakeTransactionRef(mtx));
        //------------

        //connect block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 5]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 5);
    }
    {//block 5 开始证明啦 -3
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        NewMainBlockHead(pblockNew, nbits, mainblocktime, mainpreblockhash, mainblockheight, pprev);

        //add report tx---------
        CellMutableTransaction mtx;
        mtx.nVersion = CellTransaction::PROVE;
        mtx.pProveData = std::make_shared<ProveData>();
        mtx.pProveData->provetype = ReportType::REPORT_TX;
        mtx.pProveData->branchId = branchid;
        mtx.pProveData->blockHash = expertBranchChain[expertBranchChain.size() - 3];
        mtx.pProveData->txHash = uint256S("tx00000000000000000000000000000000000000000000000000000000000001");
        mtx.pProveData->contractData = std::make_shared<ContractProveData>();

        pblockNew->vtx.push_back(MakeTransactionRef(mtx));
        //------------

        //connect block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 5]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 5);
    }
    {//block 6 开始证明啦 -4
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        NewMainBlockHead(pblockNew, nbits, mainblocktime, mainpreblockhash, mainblockheight, pprev);

        //add report tx---------
        CellMutableTransaction mtx;
        mtx.nVersion = CellTransaction::PROVE;
        mtx.pProveData = std::make_shared<ProveData>();
        mtx.pProveData->provetype = ReportType::REPORT_TX;
        mtx.pProveData->branchId = branchid;
        mtx.pProveData->blockHash = expertBranchChain[expertBranchChain.size() - 4];
        mtx.pProveData->txHash = uint256S("tx00000000000000000000000000000000000000000000000000000000000001");
        mtx.pProveData->contractData = std::make_shared<ContractProveData>();

        pblockNew->vtx.push_back(MakeTransactionRef(mtx));
        //------------

        //connect block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 3]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 3);
    }
    {//block 7 开始证明啦 -2
        std::shared_ptr<CellBlock> pblockNew = std::make_shared<CellBlock>();
        NewMainBlockHead(pblockNew, nbits, mainblocktime, mainpreblockhash, mainblockheight, pprev);

        //add report tx---------
        CellMutableTransaction mtx;
        mtx.nVersion = CellTransaction::PROVE;
        mtx.pProveData = std::make_shared<ProveData>();
        mtx.pProveData->provetype = ReportType::REPORT_TX;
        mtx.pProveData->branchId = branchid;
        mtx.pProveData->blockHash = expertBranchChain[expertBranchChain.size() - 2];
        mtx.pProveData->txHash = uint256S("tx00000000000000000000000000000000000000000000000000000000000001");
        mtx.pProveData->contractData = std::make_shared<ContractProveData>();

        pblockNew->vtx.push_back(MakeTransactionRef(mtx));
        //------------

        //connect block
        branchdb.Flush(pblockNew, true);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 1]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 1);

        //disconnect block
        branchdb.Flush(pblockNew, false);
        BOOST_CHECK(branchdb.GetBranchTipHash(branchid) == expertBranchChain[expertBranchChain.size() - 3]);
        BOOST_CHECK(branchdb.GetBranchHeight(branchid) == expertBranchChain.size() - 3);
    }
}

BOOST_AUTO_TEST_SUITE_END()
