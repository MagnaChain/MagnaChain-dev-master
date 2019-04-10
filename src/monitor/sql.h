#ifndef SQL_H
#define SQL_H

const char* sqls[] = {
    "CREATE TABLE IF NOT EXISTS `block` ("
    "`blockhash` VARCHAR(64) NOT NULL"
    ", `hashprevblock` VARCHAR(64) NOT NULL"
    ", `hashskipblock` VARCHAR(64) NOT NULL"
    ", `hashmerkleroot` VARCHAR(64) NOT NULL"
    ", `height` INT NOT NULL"
    ", `version` INT NOT NULL"
    ", `time` INT NOT NULL"
    ", `bits` INT NOT NULL"
    ", `difficulty` VARCHAR(200) NOT NULL"
    ", `nonce` INT NOT NULL"
    ", `regtest` BOOL NOT NULL"
    ", `branchid` VARCHAR(64) NOT NULL"
    ", `blocksize` INT UNSIGNED NOT NULL"
    ", PRIMARY KEY(`blockhash`)"
    ", INDEX(`hashprevblock`, `height`, `regtest`, `branchid`, `time`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `transaction` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `blockhash` VARCHAR(64) NOT NULL"
    ", `blockindex` INT NOT NULL"
    ", `version` INT NOT NULL"
    ", `locktime` INT NOT NULL"
    ", `branchvseeds` VARCHAR(64) NOT NULL"
    ", `branchseedspec6` VARCHAR(64) NOT NULL"
    ", `sendtobranchid` VARCHAR(64) NOT NULL"
    ", `sendtotxhexdata` BLOB"
    ", `frombranchid` VARCHAR(64) NOT NULL"
    ", `fromtx` BLOB"
    ", `inamount` BIGINT NOT NULL"
    ", `reporttxid` VARCHAR(64) NOT NULL"
    ", `coinpreouthash` VARCHAR(64) NOT NULL"
    ", `provetxid` VARCHAR(64) NOT NULL"
    ", `txsize` INT NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `txin` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `txindex` INT NOT NULL"
    ", `outpointhash` VARCHAR(64) NOT NULL"
    ", `outpointindex` INT NOT NULL"
    ", `sequence` INT UNSIGNED NOT NULL"
    ", `scriptsig` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`, `txindex`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `txout` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `txindex` INT NOT NULL"
    ", `value` BIGINT NOT NULL"
    ", `scriptpubkey` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`, `txindex`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

	"CREATE TABLE IF NOT EXISTS `txoutpubkey` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `txindex` INT NOT NULL"
    ", `solution` VARCHAR(64) NOT NULL"
    ", `solutiontype` INT NOT NULL"
    ", PRIMARY KEY(`txhash`, `txindex`,`solution`)"
    ", INDEX(`solution`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `contract` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `contractid` VARCHAR(64) NOT NULL"
    ", `sender` VARCHAR(64) NOT NULL"
    ", `codeorfunc` BLOB NOT NULL"
    ", `args` BLOB NOT NULL"
    ", `amountout` BIGINT NOT NULL"
    ", `signature` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ", INDEX(`contractid`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `branchblockdata` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `version` INT NOT NULL"
    ", `hashprevblock` VARCHAR(64) NOT NULL"
    ", `hashmerkleroot` VARCHAR(64) NOT NULL"
    ", `hashmerklerootwithdata` VARCHAR(64) NOT NULL"
    ", `hashmerklerootwithprevdata` VARCHAR(64) NOT NULL"
    ", `time` INT NOT NULL"
    ", `bits` INT NOT NULL"
    ", `nonce` INT NOT NULL"
    ", `prevoutstakehash` VARCHAR(64) NOT NULL"
    ", `prevoutstakeindex` INT NOT NULL"
    ", `blocksig` BLOB NOT NULL"
    ", `branchid` VARCHAR(64) NOT NULL"
    ", `blockheight` INT NOT NULL"
    ", `staketxdata` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `pmt` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `blockhash` VARCHAR(64) NOT NULL"
    ", `pmt` VARCHAR(64) NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `reportdata` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `reporttype` INT NOT NULL"
    ", `reportedbranchid` VARCHAR(64) NOT NULL"
    ", `reportedblockhash` VARCHAR(64) NOT NULL"
    ", `reportedtxhash` VARCHAR(64) NOT NULL"
    ", `contractcoins` BIGINT NOT NULL"
    ", `contractreportedspvproof` BLOB NOT NULL"
    ", `contractprovetxhash` VARCHAR(64) NOT NULL"
    ", `contractprovespvproof` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `contractprevdataitem` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `contractid` VARCHAR(64) NOT NULL"
    ", `blockhash` VARCHAR(64) NOT NULL"
    ", `txindex` VARCHAR(64) NOT NULL"
    ", PRIMARY KEY(`txhash`, `contractid`)"
    ", INDEX(`contractid`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `contractinfo` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `contractid` VARCHAR(64) NOT NULL"
    ", `txindex` INT NOT NULL"
    ", `blockhash` VARCHAR(64) NOT NULL"
    ", `code` BLOB NOT NULL"
    ", `data` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`, `contractid`)"
    ", INDEX(`contractid`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",
};

#endif
