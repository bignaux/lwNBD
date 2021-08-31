CC=gcc
CFLAGS=-Wall -DDEBUG
OBJ=lwnbd.o nbd_protocol.o nbd_server.o

lwNBD: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
		rm -f $(OBJ) *~ core lwNBD

##################################
#	Linux target
##################################

format:
	find . -type f -a \( -iname \*.h -o -iname \*.c \) | xargs clang-format -i

##################################
#	PS2 OPL target
##################################

DEST=~/devel/Open-PS2-Loader
IP=192.168.1.45
DEV=/dev/nbd4

sync:
	#git -C $(DEST) checkout nbd
	#rm $(DEST)/modules/network/lwnbdsvr/lwNBD/*
	#rm -r $(DEST)/modules/network/lwnbdsvr/obj/
	rsync -avu --files-from=opl.rsync . $(DEST)/modules/network/lwnbdsvr/lwNBD/

softdev2:
	sudo nbd-client -no-optgo $(IP) $(DEV)
	#pfsfuse --partition="+OPL" $(DEV) opl/
	pfsfuse --partition="__sysconf" $(DEV) opl/
	rm -f opl/softdev2/OPNPS2LD.ELF
	cp $(DEST)/opl.elf opl/softdev2/OPNPS2LD.ELF
	diff $(DEST)/opl.elf opl/softdev2/OPNPS2LD.ELF
	umount opl
	sudo nbd-client -d $(DEV)
