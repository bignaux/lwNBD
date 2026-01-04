# Servers

ifeq ($(CONFIG_NBD_SERVER),y)
include servers/nbd/Makefile
endif

ifeq ($(CONFIG_SIF_SERVER),y)
include servers/sifrpc/Makefile
endif

ifeq ($(CONFIG_SSDP_SERVER),y)
OBJ += servers/ssdp.o
endif

ifeq ($(CONFIG_CHARGEN_SERVER),y)
OBJ += servers/chargen.o
endif

ifeq ($(CONFIG_DAYTIME_SERVER),y)
OBJ += servers/daytime.o
endif
