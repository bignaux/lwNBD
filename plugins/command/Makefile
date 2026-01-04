ifeq ($(CONFIG_BUILTIN_COMMAND),y)
plugins/command/command.o: plugins/command/command.c plugins/command/builtin.c
CFLAGS-plugins/command/command.o += -DCONFIG_BUILTIN_COMMAND
endif

OBJ += plugins/command/command.o
