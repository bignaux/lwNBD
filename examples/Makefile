BIN = local-reader lwnbd-server
CFLAGS = -I../include $(CFLAGS-$@)
LDLIBS := ../lwnbd.a
LDFLAGS += $(LDFLAGS-$@)
APP_VERSION := $(shell git describe --always --tags)
CC_VERSION := "$(CC) $(shell $(CC) -dumpversion)"

all: $(BIN)

local-reader: local-reader.o

CFLAGS-lwnbd-server.o += $(shell pkg-config --cflags libuv) \
	-DAPP_VERSION=\"$(APP_VERSION)\" \
	-DCC_VERSION=\"$(CC_VERSION)\" \
	-DAPP_NAME=\"lwnbd-server\"
LDFLAGS-lwnbd-server += $(shell pkg-config --libs libuv)
lwnbd-server: lwnbd-server.o

clean:
	rm -f $(BIN) *.o
