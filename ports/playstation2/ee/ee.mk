#PLUGINS
#include plugins/libhdd/Makefile
include plugins/memory/Makefile
include servers/nbd/Makefile

PORT_DIR = ports/playstation2

EE_LIB = lwnbd.a
EE_LIBS = -lc -lps2ips
EE_INCS += -I$(PORT_DIR)/include -DAPP_NAME=\"lwnbdsvr\"
EE_OBJS += $(OBJ)
EE_CFLAGS += $(CFLAGS)

all: $(EE_LIB)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

.PHONY: all clean test