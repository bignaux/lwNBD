ifeq ($(IOPCORE_DEBUG),1)
	IOP_CFLAGS := -DDEBUG
endif

IOP_BIN = lwnbdsvr.irx

PORT_DIR = ports/playstation2
IOP_OBJS := $(PORT_DIR)/lwnbd_irx.o $(PORT_DIR)/exports.o $(PORT_DIR)/imports.o
IOP_INCS := -I$(PORT_DIR) -include $(PORT_DIR)/ps2sdk-compat.h -DAPP_NAME=\"lwnbdsvr\"

# for __udivdi3
IOP_LIBS += -lgcc

all: $(IOP_BIN)

#PLUGINS
include servers/nbd/Makefile
include plugins/atad/Makefile
include plugins/memory/Makefile
include plugins/mcman/Makefile
IOP_OBJS += $(OBJ)

# only suitable for https://github.com/ps2dev/ps2sdk-ports/
#install: all
#	mkdir -p $(DESTDIR)$(PS2SDK)/ports/include
#	mkdir -p $(DESTDIR)$(PS2SDK)/ports/lib
#	mkdir -p $(DESTDIR)$(PS2SDK)/ports/share/man/man3
#	cp -f $(IOP_BIN) $(DESTDIR)$(PS2SDK)/ports/lib
#	cp -f include/lwnbd.h $(DESTDIR)$(PS2SDK)/ports/include
#	cp -f lwnbd.3 $(DESTDIR)$(PS2SDK)/ports/share/man/man3/

include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.iopglobal

DEST ?= ~/devel/Open-PS2-Loader
TARGET_IP ?= 192.168.1.45
DEV ?= /dev/nbd2

softdev2:
	sudo nbd-client -N hdd0 $(TARGET_IP) $(DEV)
	#pfsfuse --partition="+OPL" $(DEV) opl/
	pfsfuse --partition="__sysconf" $(DEV) opl/
	rm -f opl/softdev2/OPNPS2LD.ELF
	cp $(DEST)/opl.elf opl/softdev2/OPNPS2LD.ELF
	diff $(DEST)/opl.elf opl/softdev2/OPNPS2LD.ELF
	#sync --file-system opl
	umount opl
	sleep 5
	sudo nbd-client -d $(DEV)
