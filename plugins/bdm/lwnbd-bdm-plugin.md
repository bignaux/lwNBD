lwNBD-bdm-plugin(1) -- Playstation 2 HDD plugin
=============================================

TARGETS : PlayStation 2

STATUS : WIP

## Usage with nbd server : 


    $ nbd-client -l 192.168.1.45
    Negotiation: ..
    hdd0: PlayStation 2 HDD via ATAD
    mc0: PlayStation 2 MC via MCMAN

connect with your favorite nbd client :
    
    $ nbd-client -N hdd0 192.168.1.45 /dev/nbd1
    Negotiation: ..size = 114473MB
    bs=512, sz=120034123776 bytes

on Linux, you can auto-mount partitions with [hdl-dump](https://github.com/ps2homebrew/hdl-dump) :

    $ hdl_dump toc /dev/nbd1 --dm | sudo dmsetup create --concise
    
## SEE ALSO
 
 [pfsshell](https://github.com/ps2homebrew/pfsshell)
 
 [nixos wiki](https://nixos.wiki/wiki/Playstation2#HDD_Partitioning)
