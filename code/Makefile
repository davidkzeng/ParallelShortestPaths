DEBUG=0
CC=g++
OMP=-fopenmp -DOMP
CFLAGS=-g -O3 -Wall -DDEBUG=$(DEBUG)
LDFLAGS= -lm
DDIR = ./data

CFILES = main.c
HFILES = main.h

all: crun crun-omp

crun: crun-seq
		cp -p crun-seq crun

crun-seq: $(CFILES) $(HFILES)
		$(CC) $(CFLAGS) -o crun-seq $(CFILES) $(LDFLAGS)

crun-omp: $(CFILES) $(HFILES)
		$(CC) $(CFLAGS) $(OMP) -o crun-omp $(CFILES) $(LDFLAGS)

clean:
	rm -f crun crun-seq crun-omp

