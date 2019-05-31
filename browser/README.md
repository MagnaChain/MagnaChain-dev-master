# MagnaChain区块链浏览器介绍 Introduction of Magnachain BlockChain Browser 

为了使用户更方便、快捷的使用MGC公链，我们开发了一款MGC公链专用的区块链浏览器，该区块链浏览器具更快速、更友好、更准确的特性

In order to make users use the MGC public chain more conveniently and quickly, we have developed a special block chain browser for MGC public chain. This block chain browser has faster, friendlier and more accurate characteristics.

## 功能 Function

* 区块查询 Block search
* 块高查询 The height of block search
* 所有块查询 All block search
* 账户查询 Account search
* 交易查询 Transaction search
* 多语言（已添加中文、英文）Multi-language supported（Chinese and English）

## 数据库 Database

MGC区块链浏览器主要使用October CMS框架开发，整个项目拥有两个数据库，数据库文件在项目根目录下

MGC Block Chain Browser is mainly developed using the October CMS framework. The whole project has two databases. The database files are in the project root directory.

* mgc: October CMS框架的数据库 Database of October CMS Framework
* magnachain: 存放MagnaChain公链链上数据的数据库 A database for storing data which from MagnaChain public chain

## 环境依赖 System requirement

* PHP7.0版本或者更高 （PHP version 7.0 or higher）
* PHP的PDO扩展 （PDO PHP Extension）
* PHP的cURL扩展 （cURL PHP Extension）
* PHP的OpenSSL扩展 （OpenSSL PHP Extension）
* PHP Mbstring 库 （Mbstring PHP Library）
* PHP ZipArchive 库 （ZipArchive PHP Library）
* PHP GD 库 （GD PHP Library）