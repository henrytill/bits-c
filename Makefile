.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -std=gnu11 -Wall -Wextra -Wconversion -Wsign-conversion -g
LDFLAGS =

WINDOW_CFLAGS = $(CFLAGS)
WINDOW_LDFLAGS = $(LDFLAGS)

-include config.mk

SOURCES =\
	alloc.c \
	base64.c \
	demo_oop.c \
	hashtable.c \
	overflow.c \
	poll.c \
	threadtest.c \
	window.c

BIN =\
	base64 \
	demo_oop \
	hashtable \
	overflow \
	poll \
	threadtest \
	window

OBJ =\
	alloc.o \
	fnv.o

.PHONY: all
all: $(BIN)

fnv.o: fnv.c fnv.h

alloc.o: alloc.c alloc.h

base64: base64.c alloc.o
	$(CC) $(CFLAGS) $(.ALLSRC) $(LDFLAGS) -lcrypto -o $@

hashtable: hashtable.c alloc.o fnv.o

overflow: overflow.c
	$(CC) $(CFLAGS) -ftrapv $(.ALLSRC) $(LDFLAGS) -o $@

threadtest: threadtest.c alloc.o
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -pthread $(.ALLSRC) $(LDFLAGS) -o $@

window: window.c
	$(CC) $(WINDOW_CFLAGS) $(.ALLSRC) $(WINDOW_LDFLAGS) -lX11 -lGL -lGLU -lGLEW -o $@

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.SUFFIXES: .c
.c:
	$(CC) $(CFLAGS) $(.ALLSRC) $(LDFLAGS) -o $@

.PHONY: check test
check test: $(BIN)
	./base64 >/dev/null
	./hashtable >/dev/null
	./threadtest foo bar baz >/dev/null

.PHONY: lint
lint:
	clang-tidy --quiet -p compile_commands.json $(SOURCES)

.PHONY: clean
clean:
	rm -f $(BIN) $(OBJ)

# Local Variables:
# mode: makefile-bsdmake
# End:
