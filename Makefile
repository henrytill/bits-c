.POSIX:
.SUFFIXES:

CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -Wconversion -Wsign-conversion -g
LDFLAGS =

WINDOW_CFLAGS  = $(CFLAGS)
WINDOW_LDFLAGS = $(LDFLAGS)

-include local.mk

SOURCES =\
	base64.c \
	prelude.c \
	threadtest.c \
	window.c

BIN =\
	base64 \
	threadtest \
	window

OBJ =\
	prelude.o

.PHONY: all
all: $(BIN)

prelude.o: prelude.c prelude.h

base64: base64.c prelude.o
	$(CC) $(CFLAGS) $(.ALLSRC) $(LDFLAGS) -lcrypto -o $@

threadtest: threadtest.c prelude.o
	$(CC) $(CFLAGS) -pthread $(.ALLSRC) $(LDFLAGS) -o $@

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
	./threadtest foo bar baz

.PHONY: lint
lint:
	clang-tidy --quiet -p compile_commands.json $(SOURCES)

.PHONY: clean
clean:
	rm -f $(BIN) $(OBJ)

# Local Variables:
# mode: makefile-bsdmake
# End:
