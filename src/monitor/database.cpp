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
std::unique_ptr<sql::PreparedStatement> insertBlockStatement;
std::unique_ptr<sql::PreparedStatement> insertTransactionStatement;
std::unique_ptr<sql::PreparedStatement> insertTxInStatement;
std::unique_ptr<sql::PreparedStatement> insertTxOutStatement;
std::unique_ptr<sql::PreparedStatement> insertTxOutPubkeyStatement;
std::unique_ptr<sql::PreparedStatement> insertContractStatement;
std::unique_ptr<sql::PreparedStatement> insertBranchBlockDataStatement;
std::unique_ptr<sql::PreparedStatement> insertPMTStatement;
std::unique_ptr<sql::PreparedStatement> insertReportDataStatement;
std::unique_ptr<sql::PreparedStatement> insertContractPrevDataItemStatement;
std::unique_ptr<sql::PreparedStatement> insertContractInfoStatement;

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
                //LogPrintf("%s:%d => resultSet == nullptr, hash is %s\n", __FUNCTION__, __LINE__, hashBlock.ToString());
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

    {
        char sql[] = "INSERT INTO `block`(`blockhash`, `hashprevblock`, `hashskipblock`, `hashmerkleroot`"
            ", `height`, `version`, `time`, `bits`, `difficulty` , `nonce`, `regtest`, `branchid`, `blocksize`)"
            " VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
        insertBlockStatement.reset(sqlConnection->prepareStatement(sql));
    }

    {
        char sql[] = "INSERT INTO `transaction`(`txhash`, `blockhash`, `blockindex`, `version`, `locktime`"
            ", `branchvseeds`, `branchseedspec6`, `sendtobranchid`, `sendtotxhexdata`, `frombranchid`, `fromtx`"
            ", `inamount`, `reporttxid`, `coinpreouthash`, `provetxid`, `txsize`)"
            " VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
        insertTransactionStatement.reset(sqlConnection->prepareStatement(sql));
    }

    {
        char sql[] = "INSERT INTO `txin`(`txhash`, `txindex`, `outpointhash`, `outpointindex`"
            ", `sequence`, `scriptsig`) VALUES(?, ?, ?, ?, ?, ?);";
        insertTxInStatement.reset(sqlConnection->prepareStatement(sql));
    }

    {
        char sql[] = "INSERT INTO `txout`(`txhash`, `txindex`, `value`, `scriptpubkey`) VALUES(?, ?, ?, ?);";
        insertTxOutStatement.reset(sqlConnection->prepareStatement(sql));
    }

	{
        char sql[] = "INSERT INTO `txoutpubkey`(`txhash`, `txindex`,`solution`,`solutiontype`) VALUES(?, ?, ?, ?);";
        insertTxOutPubkeyStatement.reset(sqlConnection->prepareStatement(sql));
    }

    {
        char sql[] = "INSERT INTO `contract`(`txhash`, `contractid`, `sender`, `codeorfunc`, `args`"
            ", `amountout`, `signature`) VALUES(?, ?, ?, ?, ?, ?, ?);";
        insertContractStatement.reset(sqlConnection->prepareStatement(sql));
    }

    {
        char sql[] = "INSERT INTO `branchblockdata`(`txhash`, `version`, `hashprevblock`, `hashmerkleroot`"
            ", `hashmerklerootwithdata`, `hashmerklerootwithprevdata`, `time`, `bits`, `nonce`"
            ", `prevoutstakehash`, `prevoutstakeindex`, `blocksig`, `branchid`, `blockheight`, `staketxdata`)"
            " VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
        insertBranchBlockDataStatement.reset(sqlConnection->prepareStatement(sql));
    }

    {
        char sql[] = "INSERT INTO `pmt`(`txhash`, `blockhash`, `pmt`) VALUES(?, ?, ?);";
        insertPMTStatement.reset(sqlConnection->prepareStatement(sql));
    }

    {
        char sql[] = "INSERT INTO `reportdata`(`txhash`, `reporttype`, `reportedbranchid`, `reportedblockhash`"
            ", `reportedtxhash`, `contractcoins`, `contractreportedspvproof`, `contractprovetxhash`"
            ", `contractprovespvproof`) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);";
        insertReportDataStatement.reset(sqlConnection->prepareStatement(sql));
    }

    {
        char sql[] = "INSERT INTO `contractprevdataitem`(`txhash`, `contractid`, `blockhash`"
            ", `txindex`) VALUES(?, ?, ?, ?);";
        insertContractPrevDataItemStatement.reset(sqlConnection->prepareStatement(sql));
    }

    {
        char sql[] = "INSERT INTO `contractinfo`(`txhash`, `contractid`, `txindex`, `blockhash`"
            ", `code`, `data`) VALUES(?, ?, ?, ?, ?, ?);";
        insertContractInfoStatement.reset(sqlConnection->prepareStatement(sql));
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

bool WriteBlockHeader(const MCBlock& block, const std::shared_ptr<DatabaseBlock> dbBlock , size_t sz)
{
    bool isGenesisBlock = block.hashPrevBlock.IsNull();

    // update
    insertBlockStatement->setString(1, block.GetHash().ToString());
    if (!block.hashPrevBlock.IsNull()) {
        insertBlockStatement->setString(2, block.hashPrevBlock.ToString());
    }
    else {
        insertBlockStatement->setString(2, std::string());
    }

    if (!isGenesisBlock) {
        insertBlockStatement->setString(3, dbBlock->hashSkipBlock.ToString());
    }
    else {
        insertBlockStatement->setString(3, std::string());
    }

    insertBlockStatement->setString(4, block.hashMerkleRoot.ToString());
    insertBlockStatement->setInt(5, dbBlock->height);
    insertBlockStatement->setInt(6, block.nVersion);
    insertBlockStatement->setUInt(7, block.nTime);
    insertBlockStatement->setUInt(8, block.nBits);
    std::strstream team;
    std::string difficulty;
    team << GetDifficulty(block.nBits);
    team >> difficulty;
    team.clear();
    insertBlockStatement->setString(9, difficulty);
    insertBlockStatement->setUInt(10, block.nNonce);
    insertBlockStatement->setBoolean(11, gArgs.GetBoolArg("-regtest", false));
    insertBlockStatement->setString(12, gArgs.GetArg("-branchid", ""));
    insertBlockStatement->setUInt(13, sz);

    if (!insertBlockStatement->executeUpdate()) {
        const sql::SQLWarning* warnings = insertBlockStatement->getWarnings();
        if (warnings != nullptr) {
            LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
            return false;
        }
    }

    return true;
}

bool WriteTxIn(const MCTransactionRef tx)
{
    const std::string& txHash(tx->GetHash().ToString());
    for (uint32_t i = 0; i < tx->vin.size(); ++i) {
        const MCTxIn& txin = tx->vin[i];
        if (txin.prevout.IsNull()) {
            continue;
        }

        const std::string scriptSig(txin.scriptSig.begin(), txin.scriptSig.end());
        std::istringstream scriptSigStream(scriptSig);

        insertTxInStatement->setString(1, txHash);
        insertTxInStatement->setUInt(2, i);
        insertTxInStatement->setString(3, txin.prevout.hash.ToString());
        insertTxInStatement->setUInt(4, txin.prevout.n);
        insertTxInStatement->setUInt(5, txin.nSequence);
        insertTxInStatement->setBlob(6, &scriptSigStream);

        if (!insertTxInStatement->executeUpdate()) {
            const sql::SQLWarning* warnings = insertTxInStatement->getWarnings();
            if (warnings != nullptr) {
                LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
                return false;
            }
        }
    }

    return true;
}

bool WriteTxOut(const MCTransactionRef tx)
{
    const std::string& txHash(tx->GetHash().ToString());
    for (uint32_t i = 0; i < tx->vout.size(); ++i) {
        const MCTxOut& txout = tx->vout[i];

        const std::string scriptPubKey(txout.scriptPubKey.begin(), txout.scriptPubKey.end());
        std::istringstream scriptPubKeyStream(scriptPubKey);

        insertTxOutStatement->setString(1, txHash);
        insertTxOutStatement->setUInt(2, i);
        insertTxOutStatement->setInt64(3, txout.nValue);
        insertTxOutStatement->setBlob(4, &scriptPubKeyStream);

        if (!insertTxOutStatement->executeUpdate()) {
            const sql::SQLWarning* warnings = insertTxOutStatement->getWarnings();
            if (warnings != nullptr) {
                LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
                return false;
            }
        }
    }

    return true;
}

bool WriteTxOutPubkey(const MCTransactionRef tx)
{
    typedef std::vector<unsigned char> valtype;
    int num = 0;
    std::string str = " ";
    const std::string& txHash(tx->GetHash().ToString());
    
    for (uint32_t i = 0; i < tx->vout.size(); ++i) {
        const MCTxOut& txout = tx->vout[i];

        std::vector<std::vector<unsigned char>> vSolutionsRet;
        txnouttype typeRet;
        bool type = Solver(txout.scriptPubKey, typeRet, vSolutionsRet);

		if (typeRet == TX_MULTISIG) {
            for (const valtype& pubkey : vSolutionsRet) {
                str = MagnaChainAddress(MCPubKey(pubkey).GetID()).ToString();
                insertTxOutPubkeyStatement->setString(1, txHash);
                insertTxOutPubkeyStatement->setUInt(2, i);
                insertTxOutPubkeyStatement->setString(3, str);
                insertTxOutPubkeyStatement->setUInt(4, uint32_t(typeRet));

				 if (!insertTxOutPubkeyStatement->executeUpdate()) {
                    const sql::SQLWarning* warnings = insertTxOutPubkeyStatement->getWarnings();
                    if (warnings != nullptr) {
                        LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
                        return false;
                    }
                }
            }
        }else{
            if (typeRet == TX_PUBKEY ) {
                str = MagnaChainAddress(MCPubKey(vSolutionsRet[0]).GetID()).ToString();
            } else if (typeRet == TX_PUBKEYHASH) {
                str = MagnaChainAddress(MCKeyID(uint160(vSolutionsRet[0]))).ToString();
            } else if (typeRet == TX_SCRIPTHASH) {
                str = MagnaChainAddress(MCScriptID(uint160(vSolutionsRet[0]))).ToString();
            }else if (typeRet== TX_NULL_DATA){
                continue;
            }else {
                num++;
                LogPrintf("%s:%d => This type has not been processed => num=%d\n", __FUNCTION__, __LINE__, num);
                assert(false);
            }
            insertTxOutPubkeyStatement->setString(1, txHash);
            insertTxOutPubkeyStatement->setUInt(2, i);
            insertTxOutPubkeyStatement->setString(3, str);
            insertTxOutPubkeyStatement->setUInt(4, uint32_t(typeRet));

            if (!insertTxOutPubkeyStatement->executeUpdate()) {
                const sql::SQLWarning* warnings = insertTxOutPubkeyStatement->getWarnings();
                if (warnings != nullptr) {
                    LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
                    return false;
                }
            }
        }
    }
    return true;
}

bool WriteContract(const MCTransactionRef tx)
{
    const std::shared_ptr<const ContractData> contractData = tx->pContractData;
    if (contractData == nullptr) {
        return true;
    }

    const std::string codeOrFunc(contractData->codeOrFunc);
    std::istringstream codeOrFuncStream(codeOrFunc);
    const std::string args(contractData->args);
    std::istringstream argsStream(args);
    const std::string signature(contractData->signature.begin(), contractData->signature.end());
    std::istringstream signatureStream(signature);

    insertContractStatement->setString(1, tx->GetHash().ToString());
    insertContractStatement->setString(2, contractData->address.ToString());
    insertContractStatement->setString(3, HexStr(contractData->sender));
    insertContractStatement->setBlob(4, &codeOrFuncStream);
    insertContractStatement->setBlob(5, &argsStream);

    // TODO: change to multi contract coins out
    insertContractStatement->setInt64(6, 0);
    //insertContractStatement->setInt64(6, contractData->amountOut);

    insertContractStatement->setBlob(7, &signatureStream);

    if (!insertContractStatement->executeUpdate()) {
        const sql::SQLWarning* warnings = insertContractStatement->getWarnings();
        if (warnings != nullptr) {
            LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
            return false;
        }
    }

    return true;
}

bool WriteBranchBlockData(const MCTransactionRef tx)
{
    const std::shared_ptr<const MCBranchBlockInfo> branchBlockData = tx->pBranchBlockData;
    if (branchBlockData == nullptr) {
        return true;
    }

    const std::string blockSig(branchBlockData->vchBlockSig.begin(), branchBlockData->vchBlockSig.end());
    std::istringstream blockSigStream(blockSig);
    const std::string stakeTxData(branchBlockData->vchStakeTxData.begin(), branchBlockData->vchStakeTxData.end());
    std::istringstream stakeTxDataStream(stakeTxData);

    insertBranchBlockDataStatement->setString(1, tx->GetHash().ToString());
    insertBranchBlockDataStatement->setInt(2, branchBlockData->nVersion);
    insertBranchBlockDataStatement->setString(3, branchBlockData->hashPrevBlock.ToString());
    insertBranchBlockDataStatement->setString(4, branchBlockData->hashMerkleRoot.ToString());
    insertBranchBlockDataStatement->setString(5, branchBlockData->hashMerkleRootWithData.ToString());
    insertBranchBlockDataStatement->setString(6, branchBlockData->hashMerkleRootWithPrevData.ToString());
    insertBranchBlockDataStatement->setUInt(7, branchBlockData->nTime);
    insertBranchBlockDataStatement->setUInt(8, branchBlockData->nBits);
    insertBranchBlockDataStatement->setUInt(9, branchBlockData->nNonce);
    insertBranchBlockDataStatement->setString(10, branchBlockData->prevoutStake.hash.ToString());
    insertBranchBlockDataStatement->setUInt(11, branchBlockData->prevoutStake.n);
    insertBranchBlockDataStatement->setBlob(12, &blockSigStream);
    insertBranchBlockDataStatement->setString(13, branchBlockData->branchID.ToString());
    insertBranchBlockDataStatement->setInt(14, branchBlockData->blockHeight);
    insertBranchBlockDataStatement->setBlob(15, &stakeTxDataStream);

    if (!insertBranchBlockDataStatement->executeUpdate()) {
        const sql::SQLWarning* warnings = insertBranchBlockDataStatement->getWarnings();
        if (warnings != nullptr) {
            LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
            return false;
        }
    }

    return true;
}

bool WritePMT(const MCTransactionRef tx)
{
    const std::shared_ptr<const MCSpvProof> spvProof = tx->pPMT;
    if (spvProof == nullptr) {
        return true;
    }

    MCDataStream pmt(SER_DISK, CLIENT_VERSION);
    spvProof->pmt.Serialize(pmt);
    std::istringstream pmtStream(pmt.str());

    insertPMTStatement->setString(1, tx->GetHash().ToString());
    insertPMTStatement->setString(2, spvProof->blockhash.ToString());
    insertPMTStatement->setBlob(3, &pmtStream);

    if (!insertPMTStatement->executeUpdate()) {
        const sql::SQLWarning* warnings = insertPMTStatement->getWarnings();
        if (warnings != nullptr) {
            LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
            return false;
        }
    }

    return true;
}

bool WriteContractPrevDataItem(const uint256& txHash, const MCContractID& contractId, const ContractPrevDataItem& item)
{
    insertContractPrevDataItemStatement->setString(1, txHash.ToString());
    insertContractPrevDataItemStatement->setString(2, contractId.ToString());
    insertContractPrevDataItemStatement->setString(3, item.blockHash.ToString());
    insertContractPrevDataItemStatement->setInt(4, item.txIndex);

    if (!insertContractPrevDataItemStatement->executeUpdate()) {
        const sql::SQLWarning* warnings = insertContractPrevDataItemStatement->getWarnings();
        if (warnings != nullptr) {
            LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
            return false;
        }
    }

    return true;
}

bool WriteContractInfo(const uint256& txHash, const MCContractID& contractId, const ContractInfo& info)
{
    const std::string& code = info.code;
    std::istringstream codeStream(code);
    const std::string& data = info.data;
    std::istringstream dataStream(data);

    insertContractInfoStatement->setString(1, txHash.ToString());
    insertContractInfoStatement->setString(2, contractId.ToString());
    insertContractInfoStatement->setInt(3, info.txIndex);
    insertContractInfoStatement->setString(4, info.blockHash.ToString());
    insertContractInfoStatement->setBlob(5, &codeStream);
    insertContractInfoStatement->setBlob(6, &dataStream);

    if (!insertContractInfoStatement->executeUpdate()) {
        const sql::SQLWarning* warnings = insertContractInfoStatement->getWarnings();
        if (warnings != nullptr) {
            LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
            return false;
        }
    }

    return true;
}

bool WriteReportData(const MCTransactionRef tx)
{
    const std::shared_ptr<const ReportData> reportData = tx->pReportData;
    if (reportData == nullptr) {
        return true;
    }

    MCDataStream contractReportedSpvProof(SER_DISK, CLIENT_VERSION);
    MCDataStream contractProveSpvProof(SER_DISK, CLIENT_VERSION);
    if (reportData->contractData != nullptr) {
        reportData->contractData->reportedSpvProof.Serialize(contractReportedSpvProof);
        reportData->contractData->proveSpvProof.Serialize(contractProveSpvProof);
    }
    std::istringstream contractReportedSpvProofStream(contractReportedSpvProof.str());
    std::istringstream contractProveSpvProofStream(contractProveSpvProof.str());

    insertReportDataStatement->setString(1, tx->GetHash().ToString());
    insertReportDataStatement->setInt(2, reportData->reporttype);
    insertReportDataStatement->setString(3, reportData->reportedBranchId.ToString());
    insertReportDataStatement->setString(4, reportData->reportedBlockHash.ToString());
    insertReportDataStatement->setString(5, reportData->reportedTxHash.ToString());
    insertReportDataStatement->setInt64(6, reportData->contractData->reportedContractPrevData.coins);
    insertReportDataStatement->setBlob(7, &contractReportedSpvProofStream);
    insertReportDataStatement->setString(8, reportData->contractData->proveTxHash.ToString());
    insertReportDataStatement->setBlob(9, &contractProveSpvProofStream);

    if (!insertReportDataStatement->executeUpdate()) {
        const sql::SQLWarning* warnings = insertReportDataStatement->getWarnings();
        if (warnings != nullptr) {
            LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
            return false;
        }
    }

    if (reportData->contractData != nullptr) {
        const uint256& txHash = tx->GetHash();
        for (auto item : reportData->contractData->reportedContractPrevData.items) {
            if (!WriteContractPrevDataItem(txHash, item.first, item.second)) {
                return false;
            }
        }
        for (auto item : reportData->contractData->proveContractData) {
            if (!WriteContractInfo(txHash, item.first, item.second)) {
                return false;
            }
        }
    }

    return true;
}

bool WriteTransaction(const MCBlock& block)
{
    const std::string& blockHash(block.GetHash().ToString());
    for (uint32_t i = 0; i < block.vtx.size(); ++i) {
        MCTransactionRef tx = block.vtx[i];

        std::string sendToTxHexData(tx->sendToTxHexData);
        std::istringstream sendToTxHexDataStream(sendToTxHexData);

        std::string fromTx(tx->fromTx.begin(), tx->fromTx.end());
        std::istringstream fromTxStream(fromTx);

        std::string reporttxid(tx->reporttxid.IsNull() ? std::string() : tx->reporttxid.ToString());
        std::string coinpreouthash(tx->coinpreouthash.IsNull() ? std::string() : tx->coinpreouthash.ToString());
        std::string provetxid(tx->provetxid.IsNull() ? std::string() : tx->provetxid.ToString());

        insertTransactionStatement->setString(1, tx->GetHash().ToString());
        insertTransactionStatement->setString(2, blockHash);
        insertTransactionStatement->setUInt(3, i);
        insertTransactionStatement->setInt(4, tx->nVersion);
        insertTransactionStatement->setUInt(5, tx->nLockTime);
        insertTransactionStatement->setString(6, tx->branchVSeeds);
        insertTransactionStatement->setString(7, tx->branchSeedSpec6);
        insertTransactionStatement->setString(8, tx->sendToBranchid);
        insertTransactionStatement->setBlob(9, &sendToTxHexDataStream);
        insertTransactionStatement->setString(10, tx->fromBranchId);
        insertTransactionStatement->setBlob(11, &fromTxStream);
        insertTransactionStatement->setInt64(12, tx->inAmount);
        insertTransactionStatement->setString(13, reporttxid);
        insertTransactionStatement->setString(14, coinpreouthash);
        insertTransactionStatement->setString(15, provetxid);
        insertTransactionStatement->setInt(16,tx->GetTotalSize());

        if (!insertTransactionStatement->executeUpdate()) {
            const sql::SQLWarning* warnings = insertTransactionStatement->getWarnings();
            if (warnings != nullptr) {
                LogPrintf("%s:%d => %d:%s\n", __FUNCTION__, __LINE__, warnings->getErrorCode(), warnings->getMessage().c_str());
                return false;
            }
        }

        if (!WriteTxIn(tx)) {
            return false;
        }
        if (!WriteTxOut(tx)) {
            return false;
        }
        if (!WriteTxOutPubkey(tx)) {
            return false;
        }
        if (!WriteContract(tx)) {
            return false;
        }
        if (!WriteBranchBlockData(tx)) {
            return false;
        }
        if (!WritePMT(tx)) {
            return false;
        }
        if (!WriteReportData(tx)) {
            return false;
        }
    }
    return true;
}

bool WriteBlockToDatabase(const MCBlock& block, const std::shared_ptr<DatabaseBlock> dbBlock, size_t sz)
{
    try {
        if (!WriteBlockHeader(block, dbBlock, sz)) {
            return false;
        }
        if (!WriteTransaction(block)) {
            return false;
        }

        sqlConnection->commit();
        AddDatabaseBlock(block.GetHash(), block.hashPrevBlock, dbBlock->hashSkipBlock, dbBlock->height);
    }
    catch (sql::SQLException e) {
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
