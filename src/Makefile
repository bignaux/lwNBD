OBJ += src/servers.o src/plugins.o src/contexts.o 

ifeq ($(NBD_URI),1)
    OBJ += src/nbd_uri.o src/yuarel.o
    CFLAGS += -D_GNU_SOURCE
endif