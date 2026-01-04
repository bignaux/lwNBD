# Clean and standard compliant replacement for $PS2SDK/samples/Makefile.eeglobal
# Use correctly LDLIBS / LDFLAGS / CFLAGS, and let you use implicit rules, pkg-config ...

CONFIG_CROSS_COMPILER_PREFIX := mips64r5900el-ps2-elf-

# Helpers to make easy the use of newlib-nano
NODEFAULTLIBS = 0

export PKG_CONFIG_PATH = $(PS2SDK)/ports/lib/pkgconfig/

LIBC = -lc
LIBM = -lm
ifeq ($(NEWLIB_NANO), 1)
   NODEFAULTLIBS = 1
   LIBC = -lc_nano
   LIBM = -lm_nano
endif

CFLAGS += -O2 -D_EE -G0 \
	-I$(PS2SDK)/ee/include \
	-I$(PS2SDK)/common/include
	
# untested
ifeq ($(NODEFAULTLIBS), 1)
   LDLIBS += -nodefaultlibs $(LIBM) -lgcc \
   -Wl,--start-group -lcdvd -lcglue -lpthread -lpthreadglue -lkernel -Wl,--end-group
endif

LDFLAGS += -T$(PS2SDK)/ee/startup/linkfile \
	-L$(PS2SDK)/ports/lib -L$(PS2SDK)/ee/lib

%.elf: %_unpack.elf
	ps2-packer $< $@

%_irx.c: %.irx
	bin2c $< $*_irx.c $*_irx
	