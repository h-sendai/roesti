CC = g++
PROG = read_roesti
CXXFLAGS = -g -O2 -Wall
# LDLIBS += -L/usr/local/lib/paprica -lpaprica

all: $(PROG)

$(PROG): $(PROG).o

$(PROG).o: $(PROG.c) roesti_decoder.h

clean:
	rm -f *.o $(PROG)
