# This file is target specific configuration to build the library

PORT_DIR = ports/playstation2/ee

CFLAGS += -DAPP_NAME=\"lwNBD\" \
	-I./ports/playstation2/common/include

ifeq ($(CONFIG_NET),y)
LDLIBS += -lps2ips -lnetman
endif

OBJ += $(PORT_DIR)/ee-helper.o $(PORT_DIR)/connman.o

EE_IRX_FILES = poweroff.irx mcman.irx ps2dev9.irx ps2atad.irx usbd.irx usbhdfsd.irx \
	sio2man.irx udptty.irx ps2ip.irx audsrv.irx netman.irx smap.irx libsd.irx ps2ip_nm.irx

# The 'minus' sign is not handled well...
ps2ip_nm.irx: ps2ip-nm.irx
	cp $< $@
CLEAN += ps2ip_nm.irx

vpath %.irx $(PS2SDK)/iop/irx/

OBJ += $(addsuffix _irx.o, $(basename $(EE_IRX_FILES)))
