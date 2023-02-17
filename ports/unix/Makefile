BIN = lwnbd-server
CFLAGS += -DAPP_NAME=\"lwNBD\"
OBJ += ports/unix/lwnbd-server.o

#PLUGINS
include servers/nbd/Makefile
include plugins/file/Makefile
include plugins/memory/Makefile

all: $(BIN)