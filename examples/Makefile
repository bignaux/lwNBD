BIN = \
	local-reader \
	local-shell \
	local-command-wc \
	lwnbd-server

CFLAGS = -I../include $(CFLAGS-$@) -DLWNBD_DEBUG
LWNBD_LIB := ../lwnbd.a
LDLIBS := $(LWNBD_LIB)
LDFLAGS += $(LDFLAGS-$@)
APP_VERSION := $(shell git describe --always --tags)
CC_VERSION := "$(CC) $(shell $(CC) -dumpversion)"

all: $(BIN)

local-reader: local-reader.o $(LWNBD_LIB)
local-shell: local-shell.o $(LWNBD_LIB)
local-command-wc: local-command-wc.o $(LWNBD_LIB)

CFLAGS-lwnbd-server.o += $(shell pkg-config --cflags libuv) \
	-DAPP_VERSION=\"$(APP_VERSION)\" \
	-DCC_VERSION=\"$(CC_VERSION)\" \
	-DAPP_NAME=\"lwnbd-server\"
LDFLAGS-lwnbd-server += $(shell pkg-config --libs libuv)
lwnbd-server: lwnbd-server.o $(LWNBD_LIB)

clean:
	rm -f $(BIN) *.o
