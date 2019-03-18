Follow has success in centos 7.4 x64(aliyun server).

yum upgrade  
yum install epel libtool autoconf automake gcc-c++ libdb4-cxx libdb4-cxx-devel boost-devel openssl-devel  
yum install libevent-devel.x86_64  
  
./autogen  
./configure  
make
