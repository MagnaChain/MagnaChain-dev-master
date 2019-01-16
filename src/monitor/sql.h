#ifndef SQL_H
#define SQL_H

const char* sqls[] = {
    "CREATE TABLE IF NOT EXISTS `block` ("
    "`blockhash` VARCHAR(64) NOT NULL"
    ", `hashprevblock` VARCHAR(64) NOT NULL"
    ", `hashskipblock` VARCHAR(64) NOT NULL"
    ", `hashmerkleroot` VARCHAR(64) NOT NULL"
    ", `height` INT(11) NOT NULL"
    ", `version` INT(11) NOT NULL"
    ", `time` INT(11) NOT NULL"
    ", `bits` INT(11) NOT NULL"
    ", `nonce` INT(11) NOT NULL"
    ", PRIMARY KEY(`blockhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `transaction` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `blockhash` VARCHAR(64) NOT NULL"
    ", `blockindex` INT(11) NOT NULL"
    ", `version` INT(11) NOT NULL"
    ", `locktime` INT(11) NOT NULL"
    ", `branchvseeds` VARCHAR(64) NOT NULL"
    ", `branchseedspec6` VARCHAR(64) NOT NULL"
    ", `sendtobranchid` VARCHAR(64) NOT NULL"
    ", `sendtotxhexdata` BLOB"
    ", `frombranchid` VARCHAR(64) NOT NULL"
    ", `fromtx` BLOB"
    ", `inamount` INT(11) NOT NULL"
    ", `contractdata` INT(11) NOT NULL"
    ", `branchblockdata` INT(11) NOT NULL"
    ", `pmt` INT(11) NOT NULL"
    ", `reportdata` INT(11) NOT NULL"
    ", `provedata` INT(11) NOT NULL"
    ", `reporttxid` VARCHAR(64) NOT NULL"
    ", `coinpreouthash` VARCHAR(64) NOT NULL"
    ", `provetxid` VARCHAR(64) NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `txin` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `txindex` INT(11) NOT NULL"
    ", `outpointhash` VARCHAR(64) NOT NULL"
    ", `outpointindex` INT(11) NOT NULL"
    ", `sequence` INT(11) NOT NULL"
    ", `scriptsig` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`, `txindex`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `txout` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `txindex` INT(11) NOT NULL"
    ", `value` INT(11) NOT NULL"
    ", `scriptpubkey` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`, `txindex`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `contract` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `contractid` VARCHAR(64) NOT NULL"
    ", `sender` VARCHAR(64) NOT NULL"
    ", `codeorfunc` BLOB NOT NULL"
    ", `args` BLOB NOT NULL"
    ", `amountout` INT(11) NOT NULL"
    ", `signature` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `branchblockdata` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `version` INT(11) NOT NULL"
    ", `hashprevblock` VARCHAR(64) NOT NULL"
    ", `hashmerkleroot` VARCHAR(64) NOT NULL"
    ", `hashmerklerootwithdata` VARCHAR(64) NOT NULL"
    ", `hashmerklerootwithprevdata` VARCHAR(64) NOT NULL"
    ", `time` INT(11) NOT NULL"
    ", `bits` INT(11) NOT NULL"
    ", `nonce` INT(11) NOT NULL"
    ", `prevoutstakehash` VARCHAR(64) NOT NULL"
    ", `prevoutstakeindex` INT(11) NOT NULL"
    ", `blocksig` BLOB NOT NULL"
    ", `branchid` VARCHAR(64) NOT NULL"
    ", `blockheight` INT(11) NOT NULL"
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
    ", `reporttype` INT(11) NOT NULL"
    ", `reportedbranchid` VARCHAR(64) NOT NULL"
    ", `reportedblockhash` VARCHAR(64) NOT NULL"
    ", `reportedtxhash` VARCHAR(64) NOT NULL"
    ", `contractcoins` INT(11) NOT NULL"
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
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `contractinfo` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `contractid` VARCHAR(64) NOT NULL"
    ", `txindex` INT(11) NOT NULL"
    ", `blockhash` VARCHAR(64) NOT NULL"
    ", `code` BLOB NOT NULL"
    ", `data` BLOB NOT NULL"
    ", PRIMARY KEY(`txhash`, `contractid`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",
};

#endif