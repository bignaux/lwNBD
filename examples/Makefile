EXAMPLES = local-reader lwnbd-server
CFLAGS = -I../include
LDFLAGS := ../lwnbd.a
APP_VERSION := $(shell git describe --always --tags)
CC_VERSION := "$(CC) $(shell $(CC) -dumpversion)"
CFLAGS += $(CFLAGS-$@)

all: $(EXAMPLES)

local-reader: local-reader.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

CFLAGS-lwnbd-server.o += $(shell pkg-config --cflags libuv) \
	-DAPP_VERSION=\"$(APP_VERSION)\" \
	-DCC_VERSION=\"$(CC_VERSION)\" \
	-DAPP_NAME=\"lwnbd-server\"

lwnbd-server: lwnbd-server.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(shell pkg-config --libs libuv)

clean:
	rm -f $(EXAMPLES) *.o
