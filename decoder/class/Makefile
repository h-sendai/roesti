CC = g++
PROG = main
CXXFLAGS = -g -O2 -Wall

all: $(PROG)
OBJS += main.o

main.o: main.cpp RoestiData.h RoestiDataFile.h

$(PROG): $(OBJS)

clean:
	rm -f *.o $(PROG)
