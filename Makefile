CC=g++
CFLAGS = -lX11 -pthread -Wall -O3

comp: cpp_img.o dirutil.o
	$(CC) -o imgtile cpp_img.o dirutil.o $(CFLAGS)