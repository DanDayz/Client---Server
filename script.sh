#!/bin/bash
wget http://download.zeromq.org/zeromq-4.0.5.tar.gz
tar xzvf zeromq-4.0.5.tar.gz
mkdir zmq
cd zeromq-4.0.5
./configure --prefix=/home/ubuntu/workspace/zmq
make -j4 
make install
echo export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ubuntu/workspace/zmq/lib >> /home/ubuntu/workspace/.bashrc 
source /home/ubuntu/workspace/.bashrc
cd ..
wget http://download.zeromq.org/czmq-3.0.0-rc1.tar.gz
tar xzvf czmq-3.0.0-rc1.tar.gz
cd czmq-3.0.0
./configure --with-libzmq=/home/ubuntu/workspace/zmq --prefix=/home/ubuntu/workspace/zmq/
make install