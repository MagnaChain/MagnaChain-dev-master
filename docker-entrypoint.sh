#!/bin/bash
set -Eeuo pipefail
if [ -e  ${CHAIN_DATA}/main/magnachain.conf ];then
    echo "ignore config"
else
    echo "rpcuser=user" >> ${CHAIN_DATA}/main/magnachain.conf 
    echo "rpcpassword=pwd" >> ${CHAIN_DATA}/main/magnachain.conf 
    echo "rpcworkqueue=1000" >> ${CHAIN_DATA}/main/magnachain.conf 
    echo "rpcallowip=0.0.0.0/0" >> ${CHAIN_DATA}/main/magnachain.conf
fi
echo "--------------------------"
exec "$@"