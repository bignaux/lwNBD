OBJ += src/servers.o src/plugins.o src/contexts.o

ifeq ($(CONFIG_URI),y)
    OBJ += src/yuarel.o src/contexts_uri.o
endif

ifeq ($(CONFIG_NET),y)
    OBJ += src/tcp.o
endif

ifeq ($(CONFIG_LOG),y)
    OBJ += src/log.o
endif
