.POSIX:
.SUFFIXES:

CC = cc
CXX = c++
CFLAGS =  -Wall -Wextra -Wconversion -Wsign-conversion -Wpointer-arith -g
CXXFLAGS = -Wall -Wextra -Wno-exceptions -fno-rtti -g
LDFLAGS =

INCLUDES = -Iinclude
ALL_CFLAGS = $(INCLUDES) -std=gnu11 $(CFLAGS)
ALL_CXXFLAGS = $(INCLUDES) -std=c++20 $(CXXFLAGS)

CURLING_CFLAGS = $(ALL_CFLAGS)
CURLING_LDFLAGS = $(LDFLAGS)

WINDOW_CFLAGS = $(ALL_CFLAGS)
WINDOW_LDFLAGS = $(LDFLAGS)

PYTHON3 = python3
PYTHON3_CFLAGS =
PYTHON3_LDFLAGS =

OBJS =
OBJS += lib/fnv.o
OBJS += lib/hashtable.o
OBJS += lib/hashtable_py.o
OBJS += lib/hashtable.so
OBJS += lib/message_queue.o

BINS =
BINS += bin/base64
BINS += bin/curling
BINS += bin/demo_oop
BINS += bin/overflow
BINS += bin/poll
BINS += bin/threadtest
BINS += bin/window

TESTS =
TESTS += test/arena
TESTS += test/fnv_test
TESTS += test/hashtable_test
TESTS += test/message_queue_basic
TESTS += test/message_queue_block
TESTS += test/notramp
TESTS += test/sum_tree

-include config.mk

.PHONY: all
all: $(OBJS) $(BINS) $(TESTS)

lib/fnv.o: lib/fnv.c include/fnv.h
	$(CC) $(ALL_CFLAGS) -DNDEBUG -o $@ -c $<

lib/hashtable.o: lib/hashtable.c include/hashtable.h
	$(CC) $(ALL_CFLAGS) -DNDEBUG -o $@ -c $<

lib/hashtable_py.o: lib/hashtable_py.c lib/hashtable.o include/hashtable.h
	$(CC) $(INCLUDES) $(PYTHON3_CFLAGS) -fPIC -DNDEBUG -o $@ -c $<

lib/hashtable.so: lib/fnv.o lib/hashtable.o lib/hashtable_py.o
	$(CC) -shared $(PYTHON3_LDFLAGS) -o $@ $(.ALLSRC)

lib/message_queue.o: lib/message_queue.c include/message_queue.h

test/message_queue_expected.o: test/message_queue_expected.c

bin/base64: bin/base64.c
	$(CC) $(ALL_CFLAGS) $(.ALLSRC) $(LDFLAGS) -lcrypto -o $@

bin/curling: bin/curling.c
	$(CC) $(CURLING_CFLAGS) $(.ALLSRC) $(CURLING_LDFLAGS) -lcurl -o $@

bin/demo_oop: bin/demo_oop.c
	$(CC) $(ALL_CFLAGS) $(.ALLSRC) $(LDFLAGS) -o $@

bin/greet: bin/greet.m
	$(CC) $(GREET_CFLAGS) $(.ALLSRC) $(GREET_LDFLAGS) -o $@

bin/hello: bin/hello.cpp
	$(CXX) $(ALL_CXXFLAGS) -DSAY_GOODBYE $(.ALLSRC) -o $@

bin/overflow: bin/overflow.c
	$(CC) $(ALL_CFLAGS) -ftrapv $(.ALLSRC) $(LDFLAGS) -o $@

bin/poll: bin/poll.c
	$(CC) $(ALL_CFLAGS) $(.ALLSRC) $(LDFLAGS) -o $@

bin/threadtest: bin/threadtest.c
	$(CC) $(ALL_CFLAGS) -D_POSIX_C_SOURCE=200809L -pthread $(.ALLSRC) $(LDFLAGS) -o $@

bin/window: bin/window.c
	$(CC) $(WINDOW_CFLAGS) $(.ALLSRC) $(WINDOW_LDFLAGS) -lX11 -lGL -lGLU -lGLEW -o $@

test/arena: test/arena.c
	$(CC) $(ALL_CFLAGS) $(ARENA_CFLAGS) $(.ALLSRC) $(LDFLAGS) $(ARENA_LDFLAGS) -o $@

test/fnv_test: test/fnv_test.c lib/fnv.o
	$(CC) $(ALL_CFLAGS) $(.ALLSRC) $(LDFLAGS) -o $@

test/hashtable_test: test/hashtable_test.c lib/hashtable.o lib/fnv.o
	$(CC) $(ALL_CFLAGS) $(.ALLSRC) $(LDFLAGS) -o $@

test/message_queue_basic: test/message_queue_basic.c lib/message_queue.o
	$(CC) $(ALL_CFLAGS) -pthread $(.ALLSRC) $(LDFLAGS) -o $@

test/message_queue_block: test/message_queue_block.c lib/message_queue.o test/message_queue_expected.o
	$(CC) $(ALL_CFLAGS) -pthread $(.ALLSRC) $(LDFLAGS) -o $@

test/notramp: test/notramp.c
	$(CC) $(ALL_CFLAGS) $(.ALLSRC) $(LDFLAGS) -o $@

test/sum_tree: test/sum_tree.c
	$(CC) $(ALL_CFLAGS) $(SUM_TREE_CFLAGS) $(.ALLSRC) $(LDFLAGS) $(SUM_TREE_LDFLAGS) -o $@

.SUFFIXES: .c .o
.c.o:
	$(CC) $(ALL_CFLAGS) -o $@ -c $<

.PHONY: check test
check test: $(OBJS) $(BINS) $(TESTS)
	./bin/base64
	./bin/curling
	./bin/demo_oop
	./bin/threadtest foo bar baz
	./test/fnv_test
	./test/hashtable_test
	./test/message_queue_basic
	./test/message_queue_block
	env -i PYTHONPATH=lib $(PYTHON3) test/hashtable_test.py

.PHONY: lint
lint:
	clang-tidy --quiet -p compile_commands.json bin/*.c lib/*.c test/*.c

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINS) $(TESTS)

.PHONY: distclean
distclean: clean
	rm -f config.mk

# Local Variables:
# mode: makefile-bsdmake
# End:
