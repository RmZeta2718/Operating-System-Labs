#file to make executables
CC = gcc
CFLAGS = -Wall -std=gnu99

# include directory
CFLAGS += -I.

TARGET = libcounter.so liblist.so libhash.so libhashfl.so

# common lib for all data structures
LIB_OBJ = spinlock.o my_mutex.o change_lock.o my_time.o

COUNTER_OBJ = counter.o

LIST_OBJ = list.o list_node.o

HASH_OBJ = hash.o $(LIST_OBJ)

# another hash table implementation
HASHFL_OBJ = hash_full_lock.o list_node.o

all: $(TARGET)

libcounter.so : $(LIB_OBJ) $(COUNTER_OBJ)
	$(CC) -shared -o $@ $^

liblist.so : $(LIB_OBJ) $(LIST_OBJ)
	$(CC) -shared -o $@ $^

libhash.so : $(LIB_OBJ) $(HASH_OBJ)
	$(CC) -shared -o $@ $^

libhashfl.so : $(LIB_OBJ) $(HASHFL_OBJ)
	$(CC) -shared -o $@ $^

clean:
	rm -rf *.o *.so

%.o:%.c
	$(CC) $(CFLAGS) -fpic -c $<
