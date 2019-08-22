#ifndef MAGNACHAIN_SQL_H
#define MAGNACHAIN_SQL_H

const char* sqls[] = {
    "CREATE TABLE IF NOT EXISTS `block` ("
    "`blockhash` VARCHAR(64) NOT NULL"
    ", `blocksize` INT UNSIGNED NOT NULL"
    ", `height` INT NOT NULL"
    ", `version` INT NOT NULL"
    ", `hashprevblock` VARCHAR(64) NOT NULL"
    ", `hashskipblock` VARCHAR(64) NOT NULL"
    ", `hashmerkleroot` VARCHAR(64) NOT NULL"
    ", `time` INT UNSIGNED NOT NULL"
    ", `bits` INT UNSIGNED NOT NULL"
    ", `nonce` INT UNSIGNED NOT NULL"
    ", PRIMARY KEY(`blockhash`)"
    ", INDEX(`hashprevblock`)"
    ", INDEX(`height`)"
    ", INDEX(`time`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `transaction` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `blockhash` VARCHAR(64) NOT NULL"
    ", `blockindex` INT NOT NULL"
    ", `version` INT NOT NULL"
    ", `locktime` INT UNSIGNED NOT NULL"
    ", `txsize` INT UNSIGNED NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ", INDEX(`blockhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `txin` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `txindex` INT NOT NULL"
    ", `outpointhash` VARCHAR(64) NOT NULL"
    ", `outpointindex` INT NOT NULL"
    ", `sequence` INT UNSIGNED NOT NULL"
    ", `scriptsig` VARCHAR(1024) NOT NULL"
    ", PRIMARY KEY(`txhash`, `txindex`)"
    ", INDEX(`txhash`)"
    ", INDEX(`txindex`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `txout` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `txindex` INT NOT NULL"
    ", `value` BIGINT NOT NULL"
    ", `scriptpubkey` VARCHAR(1024) NOT NULL"
    ", PRIMARY KEY(`txhash`, `txindex`)"
    ", INDEX(`txhash`)"
    ", INDEX(`txindex`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

	"CREATE TABLE IF NOT EXISTS `txoutpubkey` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `txindex` INT NOT NULL"
    ", `solution` VARCHAR(64) NOT NULL"
    ", `solutiontype` INT NOT NULL"
    ", PRIMARY KEY(`txhash`, `txindex`,`solution`)"
    ", INDEX(`solution`)"
    ", INDEX(`txhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `contract` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `contractid` VARCHAR(64) NOT NULL"
    ", `sender` VARCHAR(128) NOT NULL"
    ", `codeorfunc` TEXT NOT NULL"
    ", `args` VARCHAR(512) NOT NULL"
    ", `amountout` BIGINT NOT NULL"
    ", `signature` VARCHAR(1024) NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ", INDEX(`contractid`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `branchcreate` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `branchVSeeds` VARCHAR(256) NOT NULL"
    ", `branchSeedSpec6` VARCHAR(256) NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",

    "CREATE TABLE IF NOT EXISTS `branchtransaction` ("
    "`txhash` VARCHAR(64) NOT NULL"
    ", `amount` VARCHAR(256) NOT NULL"
    ", `branchid` VARCHAR(256) NOT NULL"
    ", `txdata` VARCHAR(256) NOT NULL"
    ", PRIMARY KEY(`txhash`)"
    ") ENGINE=InnoDB DEFAULT CHARSET = utf8mb4;",
};

#endif
