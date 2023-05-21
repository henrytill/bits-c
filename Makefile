.POSIX:
.SUFFIXES:

CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -Wconversion -Wsign-conversion -g
LDFLAGS =

SOURCES =\
	base64.c \
	threadtest.c \
	window.c

BIN =\
	base64 \
	threadtest \
	window

.PHONY: all
all: $(BIN)

base64: LDFLAGS += -lcrypto
base64: base64.c

threadtest: CFLAGS += -pthread
threadtest: threadtest.c

window: LDFLAGS += -lX11 -lGL -lGLU -lGLEW
window: window.c

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
	rm -f $(BIN)

# Local Variables:
# mode: makefile-bsdmake
# End:
