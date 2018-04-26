DEBUG=0
CC=g++
OMP=-fopenmp -DOMP
CFLAGS=-g -O3 -Wall -DDEBUG=$(DEBUG) -Wno-write-strings
LDFLAGS= -lm
DDIR = ./data

UTILFILES = uysp.cpp graph.cpp util.cpp cycletimer.c bucket.cpp dijkstra.cpp deltastep.cpp
CFILES = main.cpp $(UTILFILES)
TFILES = test-graph.cpp $(UTILFILES)
HFILES = main.h uysp.h graph.h util.h cycletimer.h bucket.h dijkstra.h deltastep.h

all: crun crun-omp test test-omp

crun: crun-seq
		cp -p crun-seq crun

crun-seq: $(CFILES) $(HFILES)
		$(CC) $(CFLAGS) -o crun-seq $(CFILES) $(LDFLAGS)

crun-omp: $(CFILES) $(HFILES)
		$(CC) $(CFLAGS) $(OMP) -o crun-omp $(CFILES) $(LDFLAGS)

test: $(TFILES) $(HFILES)
	$(CC) $(CFLAGS) -o test-seq $(TFILES) $(LDFLAGS)

test-omp: $(TFILES) $(HFILES)
	$(CC) $(CFLAGS) $(OMP) -o test-omp $(TFILES) $(LDFLAGS)

clean:
	rm -f crun crun-seq crun-omp test-seq test-omp

