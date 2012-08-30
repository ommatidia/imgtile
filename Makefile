CC=g++
CFLAGS = -lX11 -pthread -Wall -O3

comp: cpp_img.o dirutil.o
	$(CC) -o imgproc cpp_img.o dirutil.o $(CFLAGS)