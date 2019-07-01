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

std::map<uint256, DatabaseBlock> blocks;

sql::Driver* sqlDriver;
std::unique_ptr<sql::Connection> sqlConnection;
std::unique_ptr<sql::Statement> sqlStatement;
std::unique_ptr<sql::PreparedStatement> selectBlockStatement;

std::string sqlBlockHeader;
std::string sqlTransaction;
std::string sqlTxIn;
std::string sqlTxOut;
std::string sqlTxOutPubKey;
std::string sqlContract;
std::string sqlBranchCreate;
std::string sqlBranchTransaction;

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

void AddDatabaseBlock(const uint256& hashBlock, const uint256& hashPrevBlock, const uint256& hashSkipBlock, int height)
{
    blocks[hashBlock] = std::move(DatabaseBlock{ hashBlock, hashPrevBlock, hashSkipBlock, height });

    // clean cache
    int maturityHeight = std::max(height - COINBASE_MATURITY, 0);
    for (auto iter = blocks.begin(); iter != blocks.end();) {
        if (iter->second.height < maturityHeight) {
            iter = blocks.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

const uint256 GetMaxHeightBlock()
{
    char sql[] = "SELECT `blockhash` FROM `block` WHERE (`height`, `time`)"
        "IN (SELECT `height`, MIN(`time`) FROM `block` WHERE `height` = (SELECT MAX(`height`) FROM `block`) GROUP BY `height`);";
    std::unique_ptr<sql::PreparedStatement> getMaxHeightBlockStatement(sqlConnection->prepareStatement(sql));
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

    std::string defaultName("mgc");
    if (Params().IsMainChain())
        defaultName += "_main";
    else
        defaultName += "_" + Params().GetBranchId();
    if (Params().IsRegtest())
        defaultName += "_regtest";
    if (Params().IsTestNet())
        defaultName += "_testnet";
    std::string dbschema = gArgs.GetArg("-dbschema", defaultName);
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
    txnouttype outType;
    std::vector<std::vector<unsigned char>> solutions;
    if (!Solver(txout.scriptPubKey, outType, solutions)) {
        LogPrintf("%s:%d\n", __FUNCTION__, __LINE__);
        return false;
    }

    const char sqlBase[] = "INSERT INTO `txoutpubkey`(`txhash`, `txindex`,`solution`,`solutiontype`) VALUES";

    if (solutions.size() > 0) {
        if (sqlTxOutPubKey.empty())
            sqlTxOutPubKey = sqlBase;

        std::string sql;
        for (const auto& solution : solutions) {
            MagnaChainAddress dest;
            if (outType == txnouttype::TX_PUBKEY || outType == txnouttype::TX_MULTISIG)
                dest = MagnaChainAddress(MCPubKey(solution).GetID());
            else if (outType == txnouttype::TX_PUBKEYHASH)
                dest = MagnaChainAddress(MCKeyID(uint160(solution)));
            else if (outType == txnouttype::TX_SCRIPTHASH)
                dest = MagnaChainAddress(MCScriptID(uint160(solution)));
            else
                LogPrintf("%s:%d %d\n", __FUNCTION__, __LINE__, (int)outType);
            sql = strprintf("('%s', %u, '%s', %u),",
                txHash, index, dest.ToString(), (uint32_t)outType);
            sqlTxOutPubKey += sql;
        }
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

void WriteBranchCreate(const MCTransactionRef tx)
{
    const std::shared_ptr<const BranchCreateData> branchCreateData = tx->pBranchCreateData;
    if (branchCreateData == nullptr) {
        return;
    }

    const char sqlBase[] = "INSERT INTO `branchcreate`(`txhash`, `branchVSeeds`, `branchSeedSpec6`) VALUES";
    if (sqlBranchCreate.empty()) {
        sqlBranchCreate = sqlBase;
    }

    const std::string& txHash = tx->GetHash().ToString();

    std::string sql = strprintf("('%s', '%s', '%s'),",
        txHash, branchCreateData->branchVSeeds, branchCreateData->branchSeedSpec6);
    sqlBranchCreate += sql;
}

void WriteBranchTransaction(const MCTransactionRef tx)
{
    const std::shared_ptr<const BranchTransactionData> branchTransactionData = tx->pBranchTransactionData;
    if (branchTransactionData == nullptr) {
        return;
    }

    const char sqlBase[] = "INSERT INTO `branchtransaction`(`txhash`, `amount`, `branchid`, `txdata`) VALUES";
    if (sqlBranchTransaction.empty()) {
        sqlBranchTransaction = sqlBase;
    }

    const std::string& txHash = tx->GetHash().ToString();
    const std::string& txData = HexStr(branchTransactionData->txData);

    std::string sql = strprintf("('%s', %lld, '%s', '%s'),",
        txHash, branchTransactionData->amount, branchTransactionData->branchId, txData);
    sqlBranchTransaction += sql;
}

void WriteTransaction(const MCBlock& block)
{
    if (block.vtx.size() == 0) {
        return;
    }

    const char sqlBase[] = "INSERT INTO `transaction`"
        "(`txhash`, `blockhash`, `blockindex`, `version`, `locktime`, `txsize`) VALUES";
    if (sqlTransaction.empty()) {
        sqlTransaction = sqlBase;
    }

    const std::string& blockHash = block.GetHash().ToString();
    for (uint32_t i = 0; i < block.vtx.size(); ++i) {
        const MCTransactionRef tx = block.vtx[i];

        const std::string& txHash = tx->GetHash().ToString();
        const uint32_t txSize = tx->GetTotalSize();

        std::string sql = strprintf("('%s', '%s', %u, %d, %u, %d),",
            txHash, blockHash, i, tx->nVersion, tx->nLockTime, txSize);
        sqlTransaction += sql;

        WriteTxIn(tx);
        WriteTxOut(tx);
        WriteContract(tx);
        WriteBranchCreate(tx);
        WriteBranchTransaction(tx);
    }
}

void WriteBlockHeader(const MCBlock& block, const std::shared_ptr<DatabaseBlock> dbBlock, size_t sz)
{
    const char sqlBase[] = "INSERT INTO `block`(`blockhash`, `blocksize`, `height`, `version`, "
        "`hashprevblock`, `hashskipblock`, `hashmerkleroot`, `time`, `bits`, `nonce`) VALUES";
    if (sqlBlockHeader.empty()) {
        sqlBlockHeader = sqlBase;
    }

    const std::string& blockHash = block.GetHash().ToString();
    const std::string& hashPrevBlock = block.hashPrevBlock.ToString();
    const std::string& hashSkipBlock = dbBlock->hashSkipBlock.ToString();
    const std::string& hashMerkleRoot = block.hashMerkleRoot.ToString();

    std::string sql = strprintf("('%s', %u, %d, %d, '%s', '%s', '%s', %u, %u, %u),",
        blockHash, sz, dbBlock->height, block.nVersion, hashPrevBlock, hashSkipBlock, hashMerkleRoot,
        block.nTime, block.nBits, block.nNonce);
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
    sqlTransaction.clear();
    sqlBlockHeader.clear();
}

bool WriteBlockToDatabase(const MCBlock& block, const std::shared_ptr<DatabaseBlock> dbBlock, size_t sz)
{
    try {
        WriteBlockHeader(block, dbBlock, sz);
        WriteTransaction(block);

        Commit(sqlBlockHeader);
        Commit(sqlTransaction);
        Commit(sqlTxIn);
        Commit(sqlTxOut);
        Commit(sqlTxOutPubKey);
        Commit(sqlContract);
        sqlConnection->commit();

        AddDatabaseBlock(block.GetHash(), block.hashPrevBlock, dbBlock->hashSkipBlock, dbBlock->height);
        Clear();
    }
    catch (sql::SQLException e) {
        if (e.getErrorCode() == 1062) {
            Clear();
            return true;
        }
        LogPrintf("%s:%d => %d:%s(%s:%d)\n", __FUNCTION__, __LINE__, e.getErrorCode(), e.what(), block.GetHash().ToString(), dbBlock->height);
        Clear();
        throw e;
    }

    return true;
}
