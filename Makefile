CC=gcc
CFLAGS = -ltiff -lpng -w -O3
DFLAGS = -g -Wall -ltiff -lpng -O0

comp: imgtile.o dirutil.o
	$(CC) -o imgtile imgtile.o dirutil.o $(CFLAGS)


debug: imgtile.o dirutil.o
	$(CC) -o imgtile_dbg imgtile.o dirutil.o $(DFLAGS)