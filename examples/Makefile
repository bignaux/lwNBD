SUFFIX = ".elf"

ifeq ($(CONFIG_LOCAL_READER),y)
EXAMPLES += local-reader
local-reader: examples/local-reader.o $(LIB)
endif

ifeq ($(CONFIG_LOCAL_SHELL),y)
EXAMPLES += local-shell
local-shell: examples/local-shell.o $(LIB)
endif

ifeq ($(CONFIG_LOCAL_COMMAND_WC),y)
EXAMPLES += local-command-wc
local-command-wc: examples/local-command-wc.o $(LIB)
endif

ifeq ($(CONFIG_SDL_TEXT),y)
ifeq ($(CONFIG_UNIX),y)
EXAMPLES += sdl_text
sdl_text: examples/Vera.ttf.o examples/sdl_text.o $(LIB)
CFLAGS-examples/sdl_text.o += $(shell pkg-config --cflags SDL2_ttf sdl2)
LDFLAGS-sdl_text += $(shell pkg-config --libs SDL2_ttf sdl2)
endif
ifeq ($(CONFIG_EE),y)
EXAMPLES += sdl_text_unpack.elf
ALL += sdl_text.elf
CFLAGS += -I$(PS2SDK)/ports/include/ -I$(PS2SDK)/ports/include/freetype2/
LDFLAGS += -L$(PS2DEV)/gsKit/lib/ -L$(PS2SDK)/ee/lib/
sdl_text_unpack.elf: examples/sdl_text.o examples/Vera.ttf.o $(LIB)
CFLAGS-examples/sdl_text.o += $(shell pkg-config --cflags sdl2)
# cp /usr/local/ps2dev/ps2sdk/ports/share/pkgconfig/zlib.pc /usr/local/ps2dev/ps2sdk/ports/lib/pkgconfig/
# finally sdl2 comes with ps2_drivers, not an option here
# and this is very huge !!
# ar tv /usr/local/ps2dev/ps2sdk/ports/lib/libps2_drivers.a
# we could extract it and keep only what needed by lwnbd
LDLIBS-sdl_text_unpack.elf += $(shell pkg-config --libs sdl2) -lSDL2_ttf -lfreetype -lpng16 -lz -ldebug
endif
endif

ifeq ($(CONFIG_LWNBD_SERVER),y)
EXAMPLES += lwnbd-server
CFLAGS-examples/lwnbd-server.o += $(shell pkg-config --cflags libuv) \
	-DAPP_VERSION=\"$(APP_VERSION)\" \
	-DCC_VERSION=\"$(CC_VERSION)\" \
	-DAPP_NAME=\"lwnbd-server\"
LDFLAGS-lwnbd-server += $(shell pkg-config --libs libuv)
lwnbd-server: examples/lwnbd-server.o $(LIB)
endif

ifeq ($(CONFIG_DAYTIMED),y)
EXAMPLES += daytimed
CFLAGS-examples/daytimed.o += $(shell pkg-config --cflags libuv) \
	-DAPP_VERSION=\"$(APP_VERSION)\" \
	-DCC_VERSION=\"$(CC_VERSION)\" \
	-DAPP_NAME=\"daytimed\"
LDFLAGS-lwnbd-server += $(shell pkg-config --libs libuv)
daytimed: examples/daytimed.o $(LIB)
endif

ifeq ($(CONFIG_MINI_XINETD),y)
EXAMPLES += mini-xinetd
CFLAGS-examples/lwnbd-server.o += \
	-DAPP_VERSION=\"$(APP_VERSION)\" \
	-DCC_VERSION=\"$(CC_VERSION)\" \
	-DAPP_NAME=\"mini-xinetd\"
mini-xinetd: examples/mini-xinetd.o $(LIB)
endif

all: $(EXAMPLES) $(ALL)

# donno why i've to force implicit rule here
$(foreach)$(EXAMPLES):
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

CLEAN += $(EXAMPLES) $(ALL) examples/*.o
