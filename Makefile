CC=g++ -std=c++11
ZMQ=/home/ubuntu/workspace/zmq
#ZMQ=/usr/local
ZMQ_LIBS=$(ZMQ)/lib
ZMQ_HDRS=$(ZMQ)/include


all: nodo

nodo: node.cc
	$(CC) -I$(ZMQ_HDRS) -c node.cc
	$(CC) -L$(ZMQ_LIBS) -o node node.o -lzmq -lczmq -lpthread

clean:
	rm -rf node node.o *~


#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ubuntu/workspace/zmq
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utp/zmq/lib
# LD_LIBRARY_PATH=/usr/local/lib
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH=/usr/local/lib
#killall -9 *