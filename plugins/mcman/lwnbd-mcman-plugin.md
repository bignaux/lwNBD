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
    $ nbd-client -N mc0 192.168.1.45 /dev/nbd2
    Negotiation: ..size = 8MB
    bs=512, sz=8388608 bytes

Eventually, use [ps2mcfs](https://github.com/FranciscoDA/ps2mcfs) to mount your memory card : 

    $ mkdir -p ps2/mc0
    $ fuseps2mc /dev/nbd2 ps2/mc0
    $ fusermount3 -u ps2/mc0
    $ nbd-client -d /dev/nbd2

Note : ps2mcfs has no write feature at the moment.
    

one-liner backup :

    $ $ nbdcopy -p nbd://192.168.1.45/mc1 ps2-mc1.mc
    100% [****************************************]
    
## SEE ALSO
 
* [PlayStation 2 Memory Card File System](http://www.csclub.uwaterloo.ca:11068/mymc/ps2mcfs.html)
* [windows mymc_2.6.g2.dist.7z](http://sourceforge.net/projects/mymc-opl/files/mymc_2.6.g2.dist.7z/download)
* [ps3-memorycard-adapter](https://github.com/vpelletier/ps3-memorycard-adapter/tree/master/nbd)
* [ps3mca-tool](https://github.com/jimmikaelkael/ps3mca-tool) repository closed by Sony.
* [Card_Adapter on psdevwiki](https://www.psdevwiki.com/ps3/Card_Adapter)
* Memory Card Annihilator (can backup/restore image, no ECC), produce similar dump as this plugin.
