#file to make executables
CC=gcc

all: mem
mem: mem.c
	$(CC) -c -fpic mem.c -Wall
	$(CC) -shared -o libmem.so mem.o

clean:
	rm *.o
	rm libmem.so