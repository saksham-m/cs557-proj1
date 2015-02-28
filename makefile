CC=gcc

all: bt-sim

bt-sim: manager.c manager.h
	$(CC) manager.c -o bt-sim

clean:
	rm -f *.o
	rm -f bt-sim
