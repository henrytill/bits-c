.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -std=gnu11 -Wall -Wextra -Wconversion -Wsign-conversion -g
LDFLAGS =

CURLING_CFLAGS = $(CFLAGS)
CURLING_LDFLAGS = $(LDFLAGS)

WINDOW_CFLAGS = $(CFLAGS)
WINDOW_LDFLAGS = $(LDFLAGS)

PYTHON3 = python3

-include config.mk

SOURCES =\
	alloc.c \
	base64.c \
	curling.c \
	demo_oop.c \
	fnv.c \
	fnv_test.c \
	hashtable.c \
	hashtable_test.c \
	overflow.c \
	poll.c \
	threadtest.c \
	window.c

BIN =\
	base64 \
	curling \
	demo_oop \
	fnv_test \
	hashtable_test \
	overflow \
	poll \
	threadtest \
	window

OBJ =\
	alloc.o \
	fnv.o \
	hashtable.o

.PHONY: all
all: $(BIN) py

alloc.o: alloc.c alloc.h

fnv.o: fnv.c fnv.h

fnv_test: fnv_test.c fnv.o

hashtable.o: hashtable.c hashtable.h

hashtable_test: hashtable_test.c hashtable.o alloc.o fnv.o

base64: base64.c
	$(CC) $(CFLAGS) $(.ALLSRC) $(LDFLAGS) -lcrypto -o $@

curling: curling.c
	$(CC) $(CURLING_CFLAGS) $(.ALLSRC) $(CURLING_LDFLAGS) -lcurl -o $@

overflow: overflow.c
	$(CC) $(CFLAGS) -ftrapv $(.ALLSRC) $(LDFLAGS) -o $@

threadtest: threadtest.c
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -pthread $(.ALLSRC) $(LDFLAGS) -o $@

window: window.c
	$(CC) $(WINDOW_CFLAGS) $(.ALLSRC) $(WINDOW_LDFLAGS) -lX11 -lGL -lGLU -lGLEW -o $@

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.SUFFIXES: .c
.c:
	$(CC) $(CFLAGS) $(.ALLSRC) $(LDFLAGS) -o $@

.PHONY: py
py:
	$(PYTHON3) setup.py build_ext --inplace

.PHONY: check test
check test: $(BIN) py
	./base64
	./curling
	./fnv_test
	./threadtest foo bar baz
	./hashtable_test
	$(PYTHON3) hashtable_test.py

.PHONY: lint
lint:
	clang-tidy --quiet -p compile_commands.json $(SOURCES)

.PHONY: clean
clean:
	rm -f $(BIN) $(OBJ)
	rm -rf build
	rm -rf zig-out

# Local Variables:
# mode: makefile-bsdmake
# End:
