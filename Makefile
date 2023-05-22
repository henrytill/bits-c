.POSIX:
.SUFFIXES:

CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -Wconversion -Wsign-conversion -g
LDFLAGS =

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

base64: LDFLAGS += -lcrypto
base64: base64.c prelude.o

threadtest: CFLAGS += -pthread
threadtest: threadtest.c prelude.o

window: LDFLAGS += -lX11 -lGL -lGLU -lGLEW
window: window.c

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
