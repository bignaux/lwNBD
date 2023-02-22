CC ?= gcc
CFLAGS = -Wfatal-errors -Wall -Iinclude
OBJ = src/servers.o src/plugins.o src/contexts.o
DEBUG ?= 0

APP_VERSION := $(shell git describe --always --tags)
TARGET ?= unix
RONN = ronn
MANPAGE = lwnbd.3

include .env

ifeq ($(TARGET),unix)
include ports/unix/Makefile
endif

ifeq ($(TARGET),iop)
include ports/playstation2/iop.mk
endif

ifeq ($(TARGET),ee)
include ports/playstation2/ee.mk
endif

ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG
endif

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS) $(LIBS)

$(MANPAGE): README.md
	$(RONN) -r --pipe $< > $@

clean:
	rm -f $(BIN) $(MANPAGE) $(OBJ) *~ core 

nbdcleanup:
	sudo lsof -t /dev/nbd* | sudo xargs -r kill -9

# hackish but let you check/format same way both CI and your own env.
# and manage .clang-format-ignore file properly.
cfla = $(WORKSPACE)/tools/clang-format-lint-action
check:
	@python3 $(cfla)/run-clang-format.py --clang-format-executable $(cfla)/clang-format/clang-format14 -r .

format:
	@python3 $(cfla)/run-clang-format.py --clang-format-executable $(cfla)/clang-format/clang-format14 -r . -i true

.PHONY: all clean