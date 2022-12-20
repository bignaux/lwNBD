lwNBD-mcman-plugin(1) -- Playstation 2 MemoryCard plugin
=============================================

## Usage : use your PS2 as a memory card adapter !

connect with your favorite nbd client :

    $ nbd-client -l 192.168.1.45
    Negotiation: ..
    hdd0: PlayStation 2 HDD via ATAD
    mc0: PlayStation 2 MC via MCMAN
    $ nbd-client -N mc0 192.168.1.45 /dev/nbd2
    Negotiation: ..size = 8MB
    bs=512, sz=8388608 bytes

backup with your favorite raw copy tool :



Use [ps2mcfs](https://github.com/FranciscoDA/ps2mcfs) to mount your memory card : 

    $ ./fuseps2mc /dev/nbd2 test/

    
## SEE ALSO
 
  [PlayStation 2 Memory Card File System](http://www.csclub.uwaterloo.ca:11068/mymc/ps2mcfs.html)