#        _  _  _ __   _ ______  ______
# |      |  |  | | \  | |_____] |     \
# |_____ |__|__| |  \_| |_____] |_____/
#

# TODO: can't use -pedantic-errors yet due to LOG macros
# 
CFLAGS = -Iinclude -std=c99 -Wall $(CFLAGS-$@)
LDFLAGS += $(LDFLAGS-$@)
LDLIBS += $(LDLIBS-$@)

APP_VERSION := $(shell git describe --always --tags)
CC_VERSION := "$(CC) $(shell $(CC) -dumpversion)"
RONN = ronn
MANPAGE = lwnbd.3
LIB := liblwnbd.a
KCONFIG_AUTOHEADER := include/lwnbd/config.h

all: banner $(KCONFIG_AUTOHEADER) $(EXAMPLES)

$(KCONFIG_AUTOHEADER): .config
	$(MAKE) oldconfig

KCONFIG_ENV := \
	KCONFIG_AUTOHEADER=$(KCONFIG_AUTOHEADER) 

.config:
	@kconfig-mconf KConfig
	
# defined menuconfig earlier to avoid break on missing makefile (external $(PS2SDK)/Defs.make for example)
menuconfig:
	@kconfig-mconf KConfig
	
oldconfig:
	@mkdir -p include/config
	@$(KCONFIG_ENV) kconfig-conf --silentoldconfig KConfig

include .config
include .env
include src/Makefile
include servers/Makefile
include plugins/Makefile

#
# Targets
#

ifeq ($(CONFIG_UNIX),y)
    CFLAGS += -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
endif

ifeq ($(CONFIG_IOP),y)
    include ports/playstation2/iop/iop.mk
    include ports/playstation2/iop/iop-lib.mk
endif

ifeq ($(CONFIG_EE),y)
    include ports/playstation2/ee/ee.mk
    include ports/playstation2/ee/ee-lib.mk
endif

ifeq ($(CROSS_COMPILE),)
    CROSS_COMPILE ?= $(CONFIG_CROSS_COMPILER_PREFIX)
#    CROSS_COMPILE	?= $(CONFIG_CROSS_COMPILE:"%"=%)
endif

ifneq ($(CROSS_COMPILE),"")
AS              = $(CROSS_COMPILE)as
CC              = $(CROSS_COMPILE)gcc
LD				= $(CC)
#LD              = $(CROSS_COMPILE)ld
CPP             = $(CROSS_COMPILE)g++
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump
PKG-CONFIG      = $(CROSS_COMPILE)pkg-config
endif

#
# Settings
#

ifeq ($(CONFIG_DEBUGLOG),y)
	CFLAGS += -DCONFIG_DEBUGLOG
endif

ifeq ($(CONFIG_DEBUG),y)
	CFLAGS += -g
endif

ifeq ($(MAKE_SILENT),1)
.SILENT: $(OBJ)
endif

ifeq ($(MAKE_FATAL),1)
	CFLAGS += -Wfatal-errors
endif

#
# Examples
#

ifeq ($(CONFIG_BUILD_EXAMPLES),y)
include examples/Makefile
endif

$(MANPAGE): README.md
	$(RONN) -r --pipe $< > $@

%.a: $(OBJ)
	ar r $@ $^
	ranlib $@

banner:
	@echo "       _  _  _ __   _ ______  ______"
	@echo "|      |  |  | | \\  | |_____] |     \\"
	@echo "|_____ |__|__| |  \\_| |_____] |_____/"
	@echo
	@echo -e "library version $(APP_VERSION) - BSD 2-Clause License\n"

clean:
	rm -f $(BIN) $(LIB) $(CLEAN) $(MANPAGE) $(OBJ) *~ core $(KCONFIG_AUTOHEADER)
#	find -iname "*.o" -exec rm -f {} \;

nbdcleanup:
	sudo lsof -t /dev/nbd* | sudo xargs -r kill -9

# hackish but let you check/format same way both CI and your own env.
# and manage .clang-format-ignore file properly.
cfla = $(WORKSPACE)/tools/clang-format-lint-action
check:
	@python3 $(cfla)/run-clang-format.py --clang-format-executable $(cfla)/clang-format/clang-format14 -r .

format:
	@python3 $(cfla)/run-clang-format.py --clang-format-executable $(cfla)/clang-format/clang-format14 -r . -i true

.PHONY: all clean test menuconfig oldconfig nbdcleanup banner
