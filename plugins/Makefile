# Plugins

COUNT := 0

ifeq ($(CONFIG_PLUGIN_FILE),y)
include plugins/file/Makefile
COUNT += 1
endif

ifeq ($(CONFIG_PLUGIN_ATAD),y)
include plugins/atad/Makefile
COUNT += 1
endif

ifeq ($(CONFIG_PLUGIN_MCMAN),y)
include plugins/mcman/Makefile
COUNT += 1
endif

ifeq ($(CONFIG_PLUGIN_MEMORY),y)
include plugins/memory/Makefile
COUNT += 1
endif

ifeq ($(CONFIG_PLUGIN_TTY),y)
include plugins/tty/Makefile
COUNT += 1
endif

#ifeq ($(CONFIG_EE),y)
#include plugins/bdm/Makefile
#COUNT += 1
#endif

ifeq ($(CONFIG_PLUGIN_COMMAND),y)
include plugins/command/Makefile
COUNT += 1
endif

ifeq ($(CONFIG_PLUGIN_PCMSTREAM),y)
include plugins/pcmstream/Makefile
COUNT += 1
endif
