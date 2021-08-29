CC=gcc
CFLAGS=-Wall
OBJ=lwnbd.o nbd_protocol.o nbd_server.o

lwNBD: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
		rm -f $(OBJ) *~ core lwNBD

#################
#	PS2 OPL target
#################

DEST=~/devel/Open-PS2-Loader

sync:
	#git -C $(DEST) checkout nbd
	#rm $(DEST)/modules/network/lwnbdsvr/lwNBD/*
	#rm -r $(DEST)/modules/network/lwnbdsvr/obj/
	rsync -avu --files-from=opl.rsync . $(DEST)/modules/network/lwnbdsvr/lwNBD/

softdev2:
	sudo nbd-client -no-optgo 192.168.1.45 /dev/nbd8
	#pfsfuse --partition="+OPL" /dev/nbd8 opl/
	pfsfuse --partition="__sysconf" /dev/nbd8 opl/
	rm opl/softdev2/OPNPS2LD.ELF
	cp $(DEST)/opl.elf opl/softdev2/OPNPS2LD.ELF
	umount opl
	sudo nbd-client -d /dev/nbd8
