# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utp/zmq/lib
#export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/Users/utp/zmq/lib

##CC=g++ -std=c++11 -Wall -L/home/utp/zmq/lib -I/home/utp/zmq/include

ZMQDIR =/usr/local/zmq

CC=clang++ -std=c++11 -Wall

LDFLAGS= -lpthread -lzmqpp -lsfml-audio -lzmq -I$(ZMQDIR)/include -L$(ZMQDIR)/lib

all: client worker server

client: client.cc
	$(CC) -o client client.cc $(LDFLAGS)
	
worker: worker.cc
	$(CC) -o worker worker.cc $(LDFLAGS)

server: server.cc
	$(CC) -o server server.cc $(LDFLAGS)	
clean: 
	rm -f client worker server
