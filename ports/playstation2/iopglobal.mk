# stripped version of ps2sdk
# less custom stuff, no useless intermediate files imports.lst and exports.tab
# no IOP_ prefixed rules, use standard var, easier to embed in a port.

CC_VERSION := $(shell $(CC) -dumpversion)

ifeq ($(CC_VERSION),3.2.2)
ASFLAGS_TARGET = -march=r3000
endif

ifeq ($(CC_VERSION),3.2.3)
ASFLAGS_TARGET = -march=r3000
endif

# include dir
INCS := $(INCS) -I$(PS2SDK)/iop/include -I$(PS2SDK)/common/include

# Optimization compiler flags
OPTFLAGS ?= -Os

# C compiler flags
# -fno-builtin is required to prevent the GCC built-in functions from being included,
#   for finer-grained control over what goes into each IRX.
CFLAGS := -D_IOP -fno-builtin -G0 $(OPTFLAGS) $(INCS) $(CFLAGS)
# linker flags
LDFLAGS := -nostdlib -s $(LDFLAGS)

# Additional C compiler flags for GCC >=v5.3.0
# -msoft-float is to "remind" GCC/Binutils that the soft-float ABI is to be used. This is due to a bug, which
#   results in the ABI not being passed correctly to binutils and iop-as defaults to the hard-float ABI instead.
# -mno-explicit-relocs is required to work around the fact that GCC is now known to
#   output multiple LO relocs after one HI reloc (which the IOP kernel cannot deal with).
# -fno-toplevel-reorder (for IOP import and export tables only) disables toplevel reordering by GCC v4.2 and later.
#   Without it, the import and export tables can be broken apart by GCC's optimizations.
ifneq ($(CC_VERSION),3.2.2)
ifneq ($(CC_VERSION),3.2.3)
CFLAGS += -msoft-float -mno-explicit-relocs
IETABLE_CFLAGS := -fno-toplevel-reorder
endif
endif

# If gpopt is requested, use it if the GCC version is compatible
ifneq (x$(PREFER_GPOPT),x)
ifeq ($(CC_VERSION),3.2.2)
CFLAGS += -DUSE_GP_REGISTER=1 -mgpopt -G$(PREFER_GPOPT)
endif
ifeq ($(CC_VERSION),3.2.3)
CFLAGS += -DUSE_GP_REGISTER=1 -mgpopt -G$(PREFER_GPOPT)
endif
endif

# Assembler flags
ASFLAGS := $(ASFLAGS_TARGET) -EL -G0 $(ASFLAGS)

ifneq ($(CC_VERSION),3.2.2)
ifneq ($(CC_VERSION),3.2.3)
# Due to usage of 64 bit integers, support code for __ashldi3 and __lshrdi3 is needed
LIBS += -lgcc
endif
endif

%imports.o: %imports.c
	$(CC) $(CFLAGS) $(IETABLE_CFLAGS) -c $< -o $@

%exports.o: %exports.c
	$(CC) $(CFLAGS) $(IETABLE_CFLAGS) -c $< -o $@
	