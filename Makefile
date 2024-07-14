.POSIX:
.SUFFIXES:

CC = cc
CXX = c++
CFLAGS = -std=gnu11 -Wall -Wextra -Wconversion -Wsign-conversion -g
CXXFLAGS = -std=c++20 -Wall -Wextra -Wno-exceptions -fno-rtti -g
LDFLAGS =

CURLING_CFLAGS = $(CFLAGS)
CURLING_LDFLAGS = $(LDFLAGS)

WINDOW_CFLAGS = $(CFLAGS)
WINDOW_LDFLAGS = $(LDFLAGS)

PYTHON3 = python3
PYTHON3_CFLAGS =
PYTHON3_LDFLAGS =

SOURCES =
SOURCES += base64.c
SOURCES += curling.c
SOURCES += demo_oop.c
SOURCES += fnv.c
SOURCES += fnv_test.c
SOURCES += hashtable.c
SOURCES += hashtable_test.c
SOURCES += hashtable_py.c
SOURCES += hello.cpp
SOURCES += overflow.c
SOURCES += poll.c
SOURCES += threadtest.c
SOURCES += window.c

BIN =
BIN += base64
BIN += curling
BIN += demo_oop
BIN += fnv_test
BIN += hashtable_test
BIN += overflow
BIN += poll
BIN += threadtest
BIN += window

OBJ =
OBJ += fnv.o
OBJ += hashtable.o
OBJ += hashtable_py.o
OBJ += hashtable.so
OBJ += message_queue.o

-include config.mk

.PHONY: all
all: $(OBJ) $(BIN)

base64: base64.c
	$(CC) $(CFLAGS) $(.ALLSRC) $(LDFLAGS) -lcrypto -o $@

curling: curling.c
	$(CC) $(CURLING_CFLAGS) $(.ALLSRC) $(CURLING_LDFLAGS) -lcurl -o $@

fnv.o: fnv.c fnv.h
	$(CC) $(CFLAGS) -DNDEBUG -o $@ -c $<

fnv_test: fnv_test.c fnv.o

hashtable.o: hashtable.c hashtable.h
	$(CC) $(CFLAGS) -DNDEBUG -o $@ -c $<

hashtable_py.o: hashtable_py.c
	$(CC) $(PYTHON3_CFLAGS) -fPIC -DNDEBUG -o $@ -c $<

hashtable.so: fnv.o hashtable.o hashtable_py.o
	$(CC) -shared $(PYTHON3_LDFLAGS) -o $@ $(.ALLSRC)

hashtable_test: hashtable_test.c hashtable.o fnv.o

hello: hello.cpp
	$(CXX) $(CXXFLAGS) -DSAY_GOODBYE $(.ALLSRC) -o $@

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

.PHONY: check test
check test: $(OBJ) $(BIN)
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
	rm -f $(OBJ) $(BIN)
	rm -rf build
	rm -rf zig-out

.PHONY: distclean
distclean: clean
	rm -rf zig-cache
	rm -f config.mk

# Local Variables:
# mode: makefile-bsdmake
# End:
