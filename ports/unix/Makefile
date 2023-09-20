BIN = lwnbd-server
SERVER = ports/unix/lwnbd-server.o

CFLAGS-$(SERVER) += $(shell pkg-config --cflags libuv) \
	-DAPP_VERSION=\"$(APP_VERSION)\" \
	-DCC_VERSION=\"$(CC_VERSION)\"


OBJ += $(SERVER)
CFLAGS += $(CFLAGS-$@) -DAPP_NAME=\"lwNBD\" -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
LDFLAGS += $(shell pkg-config --libs libuv)

#PLUGINS
include servers/nbd/Makefile
include plugins/file/Makefile
include plugins/memory/Makefile

all: $(BIN)