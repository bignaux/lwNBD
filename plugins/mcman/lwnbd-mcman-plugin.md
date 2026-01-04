lwNBD-mcman-plugin(1) -- Playstation 2 MemoryCard plugin
=============================================

TARGETS : PlayStation 2 IOP

STATUS : WIP

## Usage : use your PS2 as a memory card adapter !

connect with your favorite nbd client :

    $ nbd-client -l 192.168.1.45
    Negotiation: ..
    hdd0: PlayStation 2 HDD via ATAD
    mc0: PlayStation 2 MC via MCMAN
    $ nbd-client -N mc0 192.168.1.45 /dev/nbd1
    Negotiation: ..size = 8MB
    bs=512, sz=8388608 bytes

# [ps2mcfs](https://github.com/FranciscoDA/ps2mcfs)

Eventually, use [ps2mcfs](https://github.com/FranciscoDA/ps2mcfs) to mount your memory card : 

    $ mkdir -p ps2/mc0
    $ fuseps2mc /dev/nbd1 ps2/mc0
    

When finished, umount & disconnect :

    $ fusermount3 -u ps2/mc0
    $ nbd-client -d /dev/nbd1

# nbdcopy

one-liner backup :

    $ nbdcopy -p nbd://192.168.1.45/mc0 ps2-mc1.mc
    100% [****************************************]

(wait ~ 23 sec, progressbar seems not to work ?)

one-liner restore :

    $ nbdcopy -p mc.ps2 nbd://192.168.1.45/mc0
    100% [****************************************]

(wait ~ 84 sec, progressbar seems not to work ?)

## Note OSDSYS

you can't make a bootable card with such simple copy as you can with ps3mca-tool and the Sony USB adapter. To be able to do that, we have to bind the kelf binaries dialoging with a MechaCon to have proper signature (see ps3mca-tool --sign-kelf option or ps2sdk secrman library, same stuff). Since binding depend more on the hardware, i'd keep that independant, with a shell command for example something like a ps2sdk implementation of kelfbind(slot, port, path, ...) who do sign ELF in place.

## SEE ALSO

* [PlayStation 2 Memory Card File System](http://www.csclub.uwaterloo.ca:11068/mymc/ps2mcfs.html)
* [windows mymc_2.6.g2.dist.7z](http://sourceforge.net/projects/mymc-opl/files/mymc_2.6.g2.dist.7z/download)
* [ps3-memorycard-adapter](https://github.com/vpelletier/ps3-memorycard-adapter/tree/master/nbd)
* [ps3mca-tool](https://github.com/jimmikaelkael/ps3mca-tool) repository closed by Sony.
* [Card_Adapter on psdevwiki](https://www.psdevwiki.com/ps3/Card_Adapter)
* [Memory Card Annihilator](https://github.com/ffgriever-pl/Memory-Card-Annihilator)(can backup/restore image, no ECC), produce similar dump as this plugin.
