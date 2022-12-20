#PLUGINS
include plugins/libhdd/Makefile
include servers/nbd/Makefile

PORT_DIR = ports/playstation2
EE_LIB = lwnbd.a
EE_LIBS = -lc -lps2ips
EE_INCS += -include $(PORT_DIR)/ps2sdk-compat.h -DAPP_NAME=\"lwnbdsvr\"
EE_OBJS += $(OBJ)
EE_CFLAGS += $(CFLAGS)

all: $(EE_LIB)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
