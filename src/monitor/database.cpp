#include "chain/chain.h"
#include "chain/chainparams.h"
#include "consensus/consensus.h"
#include "monitor/database.h"
#include "monitor/sql.h"
#include "utils/util.h"
#include "validation/validation.h"
#include "rpc/blockchain.h"
#include "key/key.h"
#include "coding/base58.h"

#include <cppconn/connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/metadata.h>
#include <cppconn/parameter_metadata.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <strstream>

std::map<uint256, DatabaseBlock> blocks;

sql::Driver* sqlDriver;
std::unique_ptr<sql::Connection> sqlConnection;
std::unique_ptr<sql::Statement> sqlStatement;
std::unique_ptr<sql::PreparedStatement> selectBlockStatement;

std::string sqlTxIn;
std::string sqlTxOutPubKey;
std::string sqlTxOut;
std::string sqlContract;
std::string sqlBranchBlockData;
std::string sqlPMT;
std::string sqlContractPrevDataItem;
std::string sqlContractInfo;
std::string sqlReportData;
std::string sqlTransaction;
std::string sqlBlockHeader;

std::shared_ptr<DatabaseBlock> GetDatabaseBlock(const uint256& hashBlock)
{
    std::map<uint256, DatabaseBlock>::iterator iter = blocks.find(hashBlock);
    if (iter != blocks.end()) {
        std::shared_ptr<DatabaseBlock> block = std::make_shared<DatabaseBlock>();
        block->hashBlock = hashBlock;
        block->hashPrevBlock = iter->second.hashPrevBlock;
        block->hashSkipBlock = iter->second.hashSkipBlock;
        block->height = iter->second.height;
        return block;
    }
    else {
        selectBlockStatement->setString(1, hashBlock.ToString());
        try {
            std::unique_ptr<sql::ResultSet> resultSet(selectBlockStatement->executeQuery());
            if (resultSet == nullptr || !resultSet->next()) {
                LogPrintf("%s:%d => resultSet == nullptr, hash is %s\n", __FUNCTION__, __LINE__, hashBlock.ToString());
                return nullptr;
            }

            std::shared_ptr<DatabaseBlock> block = std::make_shared<DatabaseBlock>();
            block->hashBlock = hashBlock;
            block->hashPrevBlock.SetHex(resultSet->getString(1));
            block->hashSkipBlock.SetHex(resultSet->getString(2));
            block->height = resultSet->getInt(3);
            return block;
        }
        catch (std::exception e) {
            LogPrintf("%s:%d %s\n", __FUNCTION__, __LINE__, e.what());
            return nullptr;
        }
    }
}

void AddDatabaseBlock(const uint256& hashBlock, const uint256& hashPrevBlock, const uint256& hashSkipBlock, const int height)
{
    blocks[hashBlock] = std::move(DatabaseBlock{ hashBlock, hashPrevBlock, hashSkipBlock, height });

    // clean cache
    std::vector<std::map<uint256, DatabaseBlock>::iterator> iters;
    int maturityHeight = std::max(height - COINBASE_MATURITY, 0);
    for (auto iter = blocks.begin(); iter != blocks.end();) {
        if (iter->second.height < maturityHeight) {
            iter = blocks.erase(iter);
        }
        else {
            if (iter->second.height == height) {
                iters.emplace_back(iter);
            }
            ++iter;
        }
    }
}

const uint256 GetMaxHeightBlock()
{
    char sql[] = "SELECT `blockhash` FROM `block` WHERE (`height`, `time`)"
        "IN (SELECT `height`, MIN(`time`) FROM `block` WHERE `height` = (SELECT MAX(`height`) FROM `block` WHERE `regtest` = ? AND `branchid` = ?) GROUP BY `height`);";
    std::unique_ptr<sql::PreparedStatement> getMaxHeightBlockStatement(sqlConnection->prepareStatement(sql));
    getMaxHeightBlockStatement->setBoolean(1, gArgs.GetBoolArg("-regtest", false));
    getMaxHeightBlockStatement->setString(2, gArgs.GetArg("-branchid", ""));
    std::unique_ptr<sql::ResultSet> resultSet(getMaxHeightBlockStatement->executeQuery());
    if (resultSet == nullptr || !resultSet->next()) {
        return uint256();
    }

    uint256 hash;
    hash.SetHex(resultSet->getString(1));
    return hash;
}

bool DBCreateTable()
{
    int size = sizeof(sqls) / sizeof(char*);
    for (int i = 0; i < size; ++i) {
        if (!sqlStatement->execute(sqls[i])) {
            const sql::SQLWarning* warnings = sqlStatement->getWarnings();
            if (warnings != nullptr && warnings->getErrorCode() != 1050) {
                LogPrintf("%s:%d => %s\n", __FUNCTION__, __LINE__, warnings->getMessage().c_str());
                return false;
            }
        }
    }

    sqlConnection->setAutoCommit(false);

    {
        char sql[] = "SELECT `hashprevblock`, `hashskipblock`, `height` FROM `block` WHERE `blockhash` = ?;";
        selectBlockStatement.reset(sqlConnection->prepareStatement(sql));
    }

    return true;
}

bool DBInitialize()
{
    sqlDriver = get_driver_instance();
    if (sqlDriver == nullptr) {
        printf("%s:%d => Get driver instance fail\n", __FUNCTION__, __LINE__);
        return false;
    }

    std::string dbhost = gArgs.GetArg("-dbhost", "localhost:3306");
    std::string dbuser = gArgs.GetArg("-dbuser", "root");
    std::string dbpassword = gArgs.GetArg("-dbpassword", "");
    sqlConnection.reset(sqlDriver->connect(dbhost, dbuser, dbpassword));
    if (sqlConnection == nullptr) {
        const sql::SQLWarning* warnings = sqlConnection->getWarnings();
        if (warnings != nullptr) {
            printf("%s:%d => %s\n", __FUNCTION__, __LINE__, warnings->getMessage().c_str());
            return false;
        }
    }

    sqlStatement.reset(sqlConnection->createStatement());
    if (sqlStatement == nullptr) {
        const sql::SQLWarning* warnings = sqlConnection->getWarnings();
        if (warnings != nullptr) {
            printf("%s:%d => %s\n", __FUNCTION__, __LINE__, warnings->getMessage().c_str());
            return false;
        }
    }

    std::string dbschema = gArgs.GetArg("-dbschema", "magnachain");
    sqlStatement->execute(std::string("CREATE DATABASE IF NOT EXISTS `") + dbschema + "`;");
    sqlConnection->setSchema(dbschema);

    return DBCreateTable();
}

void WriteTxIn(const MCTransactionRef tx)
{
    if (tx->vin.size() == 0) {
        return;
    }

    const char sqlBase[] = "INSERT INTO `txin`(`txhash`, `txindex`, `outpointhash`, `outpointindex`, `sequence`, `scriptsig`) VALUES";

    std::string sql;
    const std::string& txHash(tx->GetHash().ToString());
    for (uint32_t i = 0; i < tx->vin.size(); ++i) {
        const MCTxIn& txin = tx->vin[i];
        if (txin.prevout.IsNull()) {
            continue;
        }

        if (sqlTxIn.empty()) {
            sqlTxIn = sqlBase;
        }

        const std::string& scriptSig = HexStr(txin.scriptSig);

        sql = strprintf("('%s', %u, '%s', %u, %u, '%s'),", 
            txHash, i, txin.prevout.hash.ToString(), txin.prevout.n, txin.nSequence, scriptSig);
        sqlTxIn += sql;
    }
}

bool WriteTxOutPubkey(const std::string& txHash, uint32_t index, const MCTxOut& txout)
{
    txnouttype typeRet;
    std::vector<std::vector<unsigned char>> vSolutionsRet;
    if (!Solver(txout.scriptPubKey, typeRet, vSolutionsRet)) {
        LogPrintf("%s:%d\n", __FUNCTION__, __LINE__);
        return false;
    }

    const char sqlBase[] = "INSERT INTO `txoutpubkey`(`txhash`, `txindex`,`solution`,`solutiontype`) VALUES";

    std::string sql;
    MagnaChainAddress address;
    if (typeRet == TX_MULTISIG) {
        if (vSolutionsRet.size() == 0) {
            LogPrintf("%s:%d\n", __FUNCTION__, __LINE__);
            return false;
        }

        if (sqlTxOutPubKey.empty()) {
            sqlTxOutPubKey = sqlBase;
        }

        for (const auto& pubkey : vSolutionsRet) {
            address = MagnaChainAddress(MCPubKey(pubkey).GetID());

            sql = strprintf("('%s', %u, '%s', %u),", 
                txHash, index, address.ToString(), (uint32_t)typeRet);
            sqlTxOutPubKey += sql;
        }
    }
    else {
        if (typeRet == TX_PUBKEY) {
            address = MagnaChainAddress(MCPubKey(vSolutionsRet[0]).GetID());
        }
        else if (typeRet == TX_PUBKEYHASH) {
            address = MagnaChainAddress(MCKeyID(uint160(vSolutionsRet[0])));
        }
        else if (typeRet == TX_SCRIPTHASH) {
            address = MagnaChainAddress(MCScriptID(uint160(vSolutionsRet[0])));
        }
        else if (typeRet == TX_NULL_DATA) {
            return true;
        }
        else {
            LogPrintf("%s:%d\n", __FUNCTION__, __LINE__);
            return false;
        }

        if (sqlTxOutPubKey.empty()) {
            sqlTxOutPubKey = sqlBase;
        }

        sql = strprintf("('%s', %u, '%s', %u),", 
            txHash, index, address.ToString(), (uint32_t)typeRet);
        sqlTxOutPubKey += sql;
    }

    return true;
}

void WriteTxOut(const MCTransactionRef tx)
{
    if (tx->vout.size() == 0) {
        return;
    }

    const char sqlBase[] = "INSERT INTO `txout`(`txhash`, `txindex`, `value`, `scriptpubkey`) VALUES";
    if (sqlTxOut.empty()) {
        sqlTxOut = sqlBase;
    }

    std::string sql;
    const std::string& txHash(tx->GetHash().ToString());
    for (uint32_t i = 0; i < tx->vout.size(); ++i) {
        const MCTxOut& txout = tx->vout[i];

        const std::string& scriptPubKey = HexStr(txout.scriptPubKey);
        if (!WriteTxOutPubkey(txHash, i, txout)) {
            LogPrintf("%s:%d => transaction %s txout %d has not been process\n", __FUNCTION__, __LINE__, txHash, i);
            assert(false);
        }

        sql = strprintf("('%s', %u, %llu, '%s'),", 
            txHash, i, txout.nValue, scriptPubKey);
        sqlTxOut += sql;
    }
}

void WriteContract(const MCTransactionRef tx)
{
    const std::shared_ptr<const ContractData> contractData = tx->pContractData;
    if (contractData == nullptr) {
        return;
    }

    const char sqlBase[] = "INSERT INTO `contract`(`txhash`, `contractid`, `sender`, `codeorfunc`, `args`, `amountout`, `signature`) VALUES";
    if (sqlContract.empty()) {
        sqlContract = sqlBase;
    }

    const std::string& txHash = tx->GetHash().ToString();
    const std::string& contractId = contractData->address.ToString();
    std::string codeOrFunc(contractData->codeOrFunc);
    if (tx->nVersion == MCTransaction::PUBLISH_CONTRACT_VERSION) {
        codeOrFunc = HexStr(contractData->codeOrFunc.begin(), contractData->codeOrFunc.end());
    }
    const std::string args(contractData->args);
    const std::string& signature = HexStr(contractData->signature);
    const std::string& sender = HexStr(contractData->sender);

    std::string sql = strprintf("('%s', '%s', '%s', '%s', '%s', %lld, '%s'),", 
        txHash, contractId, sender, codeOrFunc, args, 0, signature);
    sqlContract += sql;
}

void WriteBranchBlockData(const MCTransactionRef tx)
{
    const std::shared_ptr<const MCBranchBlockInfo> branchBlockData = tx->pBranchBlockData;
    if (branchBlockData == nullptr) {
        return;
    }

    const char sqlBase[] = "INSERT INTO `branchblockdata`(`txhash`, `version`, `hashprevblock`, `hashmerkleroot`"
            ", `hashmerklerootwithdata`, `hashmerklerootwithprevdata`, `time`, `bits`, `nonce`"
            ", `prevoutstakehash`, `prevoutstakeindex`, `blocksig`, `branchid`, `blockheight`, `staketxdata`) VALUES";
    if (sqlBranchBlockData.empty()) {
        sqlBranchBlockData = sqlBase;
    }

    const std::string& txHash = tx->GetHash().ToString();
    const std::string& hashPrevBlock = branchBlockData->hashPrevBlock.ToString();
    const std::string& hashMerkleRoot = branchBlockData->hashMerkleRoot.ToString();
    const std::string& hashMerkleRootWithData = branchBlockData->hashMerkleRootWithData.ToString();
    const std::string& hashMerkleRootWithPrevData = branchBlockData->hashMerkleRootWithPrevData.ToString();
    const std::string& hashPrevoutStake = branchBlockData->prevoutStake.hash.ToString();
    const std::string& blockSig = HexStr(branchBlockData->vchBlockSig);
    const std::string& branchId = branchBlockData->branchID.ToString();
    const std::string& stakeTxData = HexStr(branchBlockData->vchStakeTxData.begin(), branchBlockData->vchStakeTxData.end());

    std::string sql = strprintf("('%s', %d, '%s', '%s', '%s', '%s', %u, %u, %u, '%s', %u, '%s', '%s', %d, '%s'),",
        txHash, branchBlockData->nVersion, hashPrevBlock, hashMerkleRoot, hashMerkleRootWithData, hashMerkleRootWithPrevData,
        branchBlockData->nTime, branchBlockData->nBits, branchBlockData->nNonce, hashPrevoutStake, branchBlockData->prevoutStake.n,
        blockSig, branchId, branchBlockData->blockHeight, stakeTxData);
    sqlBranchBlockData += sql;
}

void WritePMT(const MCTransactionRef tx)
{
    const std::shared_ptr<const MCSpvProof> spvProof = tx->pPMT;
    if (spvProof == nullptr) {
        return;
    }

    const char sqlBase[] = "INSERT INTO `pmt`(`txhash`, `blockhash`, `pmt`) VALUES";
    if (sqlPMT.empty()) {
        sqlPMT = sqlBase;
    }

    MCDataStream pmtData(SER_DISK, CLIENT_VERSION);
    spvProof->pmt.Serialize(pmtData);

    const std::string& txHash = tx->GetHash().ToString();
    const std::string& blockHash = spvProof->blockhash.ToString();
    const std::string& pmt = HexStr(pmtData.begin(), pmtData.end());

    std::string sql = strprintf("('%s', '%s', '%s'),", 
        txHash, blockHash, pmt);
    sqlPMT += sql;
}

void WriteContractPrevDataItem(const std::string& txHash, const std::string& contractId, const ContractPrevDataItem& item)
{
    const char sqlBase[] = "INSERT INTO `contractprevdataitem`(`txhash`, `contractid`, `blockhash`, `txindex`) VALUES";
    if (sqlContractPrevDataItem.empty()) {
        sqlContractPrevDataItem = sqlBase;
    }

    const std::string& blockHash = item.blockHash.ToString();

    std::string sql = strprintf("('%s', '%s', '%s', %d),", 
        txHash, contractId, blockHash, item.txIndex);
    sqlContractPrevDataItem += sql;
}

void WriteContractInfo(const std::string& txHash, const std::string& contractId, const ContractInfo& info)
{
    char sqlBase[] = "INSERT INTO `contractinfo`(`txhash`, `contractid`, `txindex`, `blockhash`, `code`, `data`) VALUES";
    if (sqlContractInfo.empty()) {
        sqlContractInfo = sqlBase;
    }

    const std::string& blockHash = info.blockHash.ToString();
    const std::string& code = HexStr(info.code.begin(), info.code.end());
    const std::string& data = HexStr(info.data.begin(), info.data.end());

    std::string sql = strprintf("('%s', '%s', %d, '%s', '%s', '%s'),", 
        txHash, contractId, info.txIndex, blockHash, code, data);
    sqlContractInfo += sql;
}

bool WriteReportData(const MCTransactionRef tx)
{
    const std::shared_ptr<const ReportData> reportData = tx->pReportData;
    if (reportData == nullptr) {
        return true;
    }

    const char sqlBase[] = "INSERT INTO `reportdata`(`txhash`, `reporttype`, `reportedbranchid`, `reportedblockhash`, "
        "`reportedtxhash`, `contractcoins`, `contractreportedspvproof`, `contractprovetxhash`, `contractprovespvproof`) VALUES";
    if (sqlReportData.empty()) {
        sqlReportData = sqlBase;
    }

    MCDataStream contractReportedSpvProofData(SER_DISK, CLIENT_VERSION);
    MCDataStream contractProveSpvProofData(SER_DISK, CLIENT_VERSION);
    if (reportData->contractData != nullptr) {
        reportData->contractData->reportedSpvProof.Serialize(contractReportedSpvProofData);
        reportData->contractData->proveSpvProof.Serialize(contractProveSpvProofData);
    }

    const std::string& txHash = tx->GetHash().ToString();
    const std::string& reportedbranchId = reportData->reportedBranchId.ToString();
    const std::string& reportedBlockHash = reportData->reportedBlockHash.ToString();
    const std::string& reportedTxHash = reportData->reportedTxHash.ToString();
    const std::string& contractReportedSpvProof = HexStr(contractReportedSpvProofData.begin(), contractReportedSpvProofData.end());
    const std::string& proveTxHash = reportData->contractData->proveTxHash.ToString();
    const std::string& contractProveSpvProof = HexStr(contractProveSpvProofData.begin(), contractProveSpvProofData.end());

    std::string sql = strprintf("('%s', %d, '%s', '%s', '%s', %lld, '%s', '%s', '%s'),",
        txHash, reportData->reporttype, reportedbranchId, reportedBlockHash, reportedTxHash,
        reportData->contractData->reportedContractPrevData.coins, contractReportedSpvProof, proveTxHash, contractProveSpvProof);
    sqlReportData += sql;
}

void WriteTransaction(const MCBlock& block)
{
    if (block.vtx.size() == 0) {
        return;
    }

    const char sqlBase[] = "INSERT INTO `transaction`(`txhash`, `blockhash`, `blockindex`, `version`, `locktime`"
            ", `branchvseeds`, `branchseedspec6`, `sendtobranchid`, `sendtotxhexdata`, `frombranchid`, `fromtx`"
            ", `inamount`, `reporttxid`, `coinpreouthash`, `provetxid`, `txsize`) VALUES";
    if (sqlTransaction.empty()) {
        sqlTransaction = sqlBase;
    }

    const std::string& blockHash = block.GetHash().ToString();
    for (uint32_t i = 0; i < block.vtx.size(); ++i) {
        MCTransactionRef tx = block.vtx[i];

        const std::string& txHash = tx->GetHash().ToString();
        const std::string& branchVSeeds = tx->branchVSeeds;
        const std::string& branchSeedSpec6 = tx->branchSeedSpec6;
        const std::string& sendToBranchId = tx->sendToBranchid;
        const std::string& sendToTxHexData = tx->sendToTxHexData;
        const std::string& fromBranchId = tx->fromBranchId;
        const std::string& fromTx = HexStr(tx->fromTx.begin(), tx->fromTx.end());
        const std::string reportTxid(tx->reporttxid.IsNull() ? std::string() : tx->reporttxid.ToString());
        const std::string coinPreoutHash(tx->coinpreouthash.IsNull() ? std::string() : tx->coinpreouthash.ToString());
        const std::string proveTxid(tx->provetxid.IsNull() ? std::string() : tx->provetxid.ToString());
        const uint32_t txSize = tx->GetTotalSize();

        std::string sql = strprintf("('%s', '%s', %u, %d, %u, '%s', '%s', '%s', '%s', '%s', '%s', %lld, '%s', '%s', '%s', %d),",
            txHash, blockHash, i, tx->nVersion, tx->nLockTime, branchVSeeds, branchSeedSpec6, sendToBranchId, sendToTxHexData,
            fromBranchId, fromTx, tx->inAmount, reportTxid, coinPreoutHash, proveTxid, txSize);
        sqlTransaction += sql;

        WriteTxIn(tx);
        WriteTxOut(tx);
        WriteContract(tx);
        WriteBranchBlockData(tx);
        WritePMT(tx);
        WriteReportData(tx);
    }
}

void WriteBlockHeader(const MCBlock& block, const std::shared_ptr<DatabaseBlock> dbBlock, size_t sz)
{
    const char sqlBase[] = "INSERT INTO `block`(`blockhash`, `hashprevblock`, `hashskipblock`, `hashmerkleroot`, "
        "`height`, `version`, `time`, `bits`, `nonce`, `regtest`, `branchid`, `blocksize`) VALUES";
    if (sqlBlockHeader.empty()) {
        sqlBlockHeader = sqlBase;
    }

    const std::string& blockHash = block.GetHash().ToString();
    const std::string& hashPrevBlock = block.hashPrevBlock.ToString();
    const std::string& hashSkipBlock = dbBlock->hashSkipBlock.ToString();
    const std::string& hashMerkleRoot = block.hashMerkleRoot.ToString();
    const bool regtest = gArgs.GetBoolArg("-regtest", false);
    const std::string& branchId = gArgs.GetArg("-branchid", "");

    std::string sql = strprintf("('%s', '%s', '%s', '%s', %d, %d, %u, %u, %u, %d, '%s', %u),",
        blockHash, hashPrevBlock, hashSkipBlock, hashMerkleRoot, dbBlock->height, block.nVersion, block.nTime,
        block.nBits, block.nNonce, regtest, branchId, sz);
    sqlBlockHeader += sql;
}

void Commit(std::string& sql)
{
    if (!sql.empty()) {
        sql.resize(sql.size() - 1);
        //LogPrintf("%s\n", sql);
        if (!sqlStatement->execute(sql)) {
            const sql::SQLWarning* warnings = sqlStatement->getWarnings();
            if (warnings != nullptr) {
                LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
            }
        }
    }
}

void Clear()
{
    sqlTxIn.clear();
    sqlTxOutPubKey.clear();
    sqlTxOut.clear();
    sqlContract.clear();
    sqlBranchBlockData.clear();
    sqlPMT.clear();
    sqlContractPrevDataItem.clear();
    sqlContractInfo.clear();
    sqlReportData.clear();
    sqlTransaction.clear();
    sqlBlockHeader.clear();
}

bool WriteBlockToDatabase(const MCBlock& block, const std::shared_ptr<DatabaseBlock> dbBlock, size_t sz)
{
    try {
        WriteBlockHeader(block, dbBlock, sz);
        WriteTransaction(block);

        Commit(sqlTxIn);
        Commit(sqlTxOutPubKey);
        Commit(sqlTxOut);
        Commit(sqlContract);
        Commit(sqlBranchBlockData);
        Commit(sqlPMT);
        Commit(sqlContractPrevDataItem);
        Commit(sqlContractInfo);
        Commit(sqlReportData);
        Commit(sqlTransaction);
        Commit(sqlBlockHeader);
        sqlConnection->commit();

        AddDatabaseBlock(block.GetHash(), block.hashPrevBlock, dbBlock->hashSkipBlock, dbBlock->height);
        Clear();
    }
    catch (sql::SQLException e) {
        Clear();
        LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, e.getErrorCode(), e.what());
        if (e.getErrorCode() != 1062) {
            throw e;
        }
        else {
            return false;
        }
    }

    return true;
}
