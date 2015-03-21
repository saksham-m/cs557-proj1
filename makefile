CC=gcc
CXX       =      g++
CPPFLAGS  = 	-I. -g -Wall -fno-inline
CFLAGS    = 	-I. -g -Wall -fno-inline

FLAGS     = 	${CPPFLAGS} ${AC_DEFS}

TIMERSC_LIB_OBJS = timers.o timers-c.o tools.o

all: tools.o timers.o timers-c.o bt-sim tracker client.o client

tools.o: tools.cc tools.hh
	$(CXX) $(FLAGS) -c tools.cc

timers.o: timers.cc timers.hh
	$(CXX) $(FLAGS) -c timers.cc

timers-c.o: timers-c.cc timers-c.h
	$(CXX) $(FLAGS) -c timers-c.cc

client.o: client.c
	$(CC) -c client.c 

client: $(TIMERSC_LIB_OBJS) client.o
	$(CXX) $(FLAGS) $(TIMERSC_LIB_OBJS) client.o -o client

tracker: tracker.c
	$(CC) tracker.c -o tracker

bt-sim: manager.c manager.h
	$(CC) manager.c -o bt-sim

clean:
	rm -f *.o
	rm -f bt-sim
	rm -f tracker
	rm -f client
