# Getting Started

## Download

To post the game on MagnaChain, you need an RPC client, which can be downloaded at:

Https://developers.magnachain.co/article/downloads/

Choose your own version  

## Get the address

Run the following command in the MagnaChain directory

Enter on the Windows platform:

    Magnachain-cli.exe -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port getnewaddress

Enter on the Linux /mac OS platform:

    Chmod +x loadlib.sh
    ./loadlib.sh
    ./magnachain-cli -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port getnewaddress

The command will return a new address like "XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf"

The nodeIp, user, pwd, port and other parameters in the command need to fill in the corresponding real MagnaChain node parameters.

## Writing a contract and publishing

In the current directory, create a new file, name it game.lua, and edit:

```lua
Function init()
PersistentData = {}
PersistentData.killNumTbl = {}
End

Function killMonster()
    --print("GJ!One monster killed by you!")
    PersistentData.killNumTbl[msg.sender] = (PersistentData.killNumTbl[msg.sender] or 0) + 1
    Return true

Function showNum()
    Return PersistentData.killNumTbl[msg.sender]
End
    
```

Enter on the Windows platform:
    
    Magnachain-cli.exe -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port publishcontract game.lua XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf

Enter on linux /mac OS platform:

    ./magnachain-cli -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port publishcontract game.lua XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf

The command returns something like the following:

    {
      "txid": "7501667a85d2c35e57258f7f165a72156f7f8d43e9c03c4fe2e59875abacd3c6",
      "contractaddress": "UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs",
      "senderaddress": "XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf"
    }

This indicates that your contract has been successfully released, and "UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs" is the contract address.

## Calling the contract

After our game contract is released, we can play this game by calling this contract.

Enter on the Windows platform:

    Magnachain-cli.exe -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port callcontract true 0 UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf killMonster

Enter on linux /mac OS platform:

    ./magnachain-cli -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port callcontract true 0 UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf killMonster

Every time we execute a command, we will kill one more monster. Let's take a look at how much we killed in the public.

Enter on the Windows platform:

    Magnachain-cli.exe -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port callcontract true 0 UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf showNum

Enter on linux /mac OS platform:

    ./magnachain-cli -rpcconnect=nodeIp -rpcuser=user -rpcpassword=pwd -rpcport=port callcontract true 0 UpcuLWL6HcSSH5Rc5xQy6e6yEMPQZ9xvWs XQSD7PUPNCf8psJdfFMjfWXKuaHXFwSFRf showNum

This command will return the number of monsters we killed.

So far, we have initially developed a game on MagnaChain. Is it very simple?

For more usage, please refer to the developer website.

https://developers.magnachain.co/

