BIN = lwnbd-server
CFLAGS += -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -DAPP_NAME=\"lwNBD\"
OBJ += ports/unix/lwnbd-server.o

#PLUGINS
include servers/nbd/Makefile
include plugins/file/Makefile
include plugins/memory/Makefile

all: $(BIN)