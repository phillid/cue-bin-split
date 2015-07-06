# Makefile for cue-bin-split

.POSIX:
include config.mk

OBJECTS = \
	cue-bin-split.o \
	misc.o

CFLAGS += -Wall -Werror


all: cue-bin-split

cue-bin-split: $(OBJECTS)
	$(CC) -o $(EXEC_NAME) $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)


clean:
	rm -f cue-bin-split

distclean: clean
	rm -f *.o

.PHONY: all clean
