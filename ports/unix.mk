CFLAGS += -DAPP_NAME=\"lwNBD\" -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L

#PLUGINS
include servers/nbd/Makefile
include plugins/file/Makefile
include plugins/memory/Makefile
include plugins/command/Makefile

%.a: $(OBJ)
	ar r $@ $^
	ranlib $@

all: lwnbd.a
