CC=gcc

all: bt-sim tracker client

clien: client.c
	$(CC) client.c -o client

tracker: tracker.c
	$(CC) tracker.c -o tracker

bt-sim: manager.c manager.h
	$(CC) manager.c -o bt-sim

clean:
	rm -f *.o
	rm -f bt-sim
	rm -f tracker
	rm -f client
