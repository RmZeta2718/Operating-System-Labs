# file to make executables
CC = LD_LIBRARY_PATH=../sources gcc
# CC = gcc
CFLAGS = -Wall -std=gnu99
# include directory
CFLAGS += -I../sources/
# link directory
CFLAGS += -L../sources/
# gdb
# CFLAGS += -g

# testers
TCOUNTER = ct
TLIST = lti ltd ltr ltdon
THASH = hti htr hts
THASHFL = hflti hfltr

TARGET = $(TCOUNTER) $(TLIST) $(THASH) $(THASHFL)

# link library
LCOUNTER = -lpthread -lcounter
LLIST = -lpthread -llist
LHASH = -lpthread -lhash
LHASHFL = -lpthread -lhashfl

all : $(TARGET)

# TODO: how to do this correctly?
# $(TCOUNTER) : $(addsuffix .c,$@)
# 	$(CC) $^ -o $@ $(LCOUNTER) $(CFLAGS)
# $(TLIST) : $(addsuffix .c,$@)
# 	$(CC) $^ -o $@ $(LLIST) $(CFLAGS)
# $(THASH) : $(addsuffix .c,$@)
# 	$(CC) $^ -o $@ $(LHASH) $(CFLAGS)
# $(THASHFL) : $(addsuffix .c,$@)
# 	$(CC) $^ -o $@ $(LHASHFL) $(CFLAGS)

ct : ct.c
	$(CC) $^ -o $@ $(LCOUNTER) $(CFLAGS)

lti : lti.c
	$(CC) $^ -o $@ $(LLIST) $(CFLAGS)

ltd : ltd.c
	$(CC) $^ -o $@ $(LLIST) $(CFLAGS)

ltdon : ltdon.c
	$(CC) $^ -o $@ $(LLIST) $(CFLAGS)

ltr : ltr.c
	$(CC) $^ -o $@ $(LLIST) $(CFLAGS)

hti : hti.c
	$(CC) $^ -o $@ $(LHASH) $(CFLAGS)

htr : htr.c
	$(CC) $^ -o $@ $(LHASH) $(CFLAGS)

hts : hts.c
	$(CC) $^ -o $@ $(LHASH) $(CFLAGS)

hflti : hflti.c
	$(CC) $^ -o $@ $(LHASHFL) $(CFLAGS)

hfltr : hfltr.c
	$(CC) $^ -o $@ $(LHASHFL) $(CFLAGS)

clean:
	rm -rf $(TARGET)
