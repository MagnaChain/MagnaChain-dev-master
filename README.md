[![Build Status](https://travis-ci.org/MagnaChain/MagnaChain-dev-master.svg?branch=master)](https://travis-ci.org/MagnaChain/MagnaChain-dev-master)  
# Getting Started

## How to build
Detail see doc directory, there are many doc files for build different OS.
Build on windows, see build-windows.md, another choise use msvc, location to directory build-msvc.
There are many other build manuals, as build-unix.md、build-osx.md、build-centos.md...

## Download

To publish the game on MagnaChain, you need an RPC client, which can be downloaded at:

Https://developers.magnachain.co/article/downloads/

Choose your own version according to your system environment

## Get the address

Run the following command in the MagnaChain directory

For Windows platform, enter:

    Magnachain-cli.exe -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port getnewaddress

For Linux / Mac OS platform, enter:

    Chmod +x loadlib.sh
    ./loadlib.sh
    ./magnachain-cli -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port getnewaddress

The command will return a new address like "XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf"

The nodeIp, user, pwd, port and other parameters in the command need to fill in the corresponding real MagnaChain node parameters.

## Write a contract and publish it

In the current directory, create a new file, name it game.lua, and edit:

```lua
function init()
    PersistentData = {}
    PersistentData.killNumTbl = {}
end

function killMonster()
    --print("GJ!One monster killed by you!")
    PersistentData.killNumTbl[msg.sender] = (PersistentData.killNumTbl[msg.sender] or 0) + 1
    return true
end

function showNum()
    return PersistentData.killNumTbl[msg.sender]
end
    
```

For Windows platform, enter:
    
    Magnachain-cli.exe -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port publishcontract game.lua XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf

For Linux / Mac OS platform, enter:

    ./magnachain-cli -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port publishcontract game.lua XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf

The command returns something like the following:

    {
      "txid": "7501667a85d2c35e57258f7f165a72156f7f8d43e9c03c4fe2e59875abacd3c6",
      "contractaddress": "UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs",
      "senderaddress": "XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf"
    }

This indicates that your contract has been successfully released, and "UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs" is the contract address.

## Call the contract

After our game contract is released, we can play this game by calling this contract.

For Windows platform, enter:

    Magnachain-cli.exe -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port callcontract true 0 UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf killMonster

For Linux / Mac OS platform, enter:

    ./magnachain-cli -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port callcontract true 0 UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf killMonster

Every time we execute a command, we will kill one more monster. Let's take a look at how many we killed.

For Windows platform, enter:

    Magnachain-cli.exe -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port callcontract true 0 UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf showNum

For Linux / Mac OS platform, enter:

    ./magnachain-cli -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port callcontract true 0 UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf showNum

This command will return the number of monsters we killed.

So far, we have initially developed a game on MagnaChain. Isn’t  it very easy?   
For more usage, please refer to the developer website.

https://developers.magnachain.co/

