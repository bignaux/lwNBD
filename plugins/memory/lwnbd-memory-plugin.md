# memory plugin

## Usage : memory manipulation using memcpy()

TARGETS : all

STATUS : WIP

## PS2 note : 

* replacement for [ps2client dumpmem/writemem](https://github.com/ps2dev/ps2link/blob/a14d0b7b9ffb3b319dcfae49834e742b9ed12490/ee/cmdHandler.c#L243)

    ps2client -h 192.168.0.10 dumpmem host:bios.bin 0xbfc00000 0x400000

compare bios md5 sum with http://redump.org/datfile/ps2-bios/

*  [ImHex-PS2](https://github.com/bignaux/ImHex-PS2) is a repository for specific ps2 [ImHex](https://github.com/WerWolv/ImHex) stuff. It will have useful code to work with this plugin to do live RE/debug session.

### documentation :

 [Playstation 2 Memory Mapping](https://psi-rockin.github.io/ps2tek/#memorymap)
 
## dump tool analysis to review :

* [ImHex](https://github.com/WerWolv/ImHex) 

https://github.com/WerWolv/ImHex/tree/master/plugins/builtin/source/content/providers

* http://www.wxhexeditor.org/
* https://github.com/allencch/med
* https://infosecwriteups.com/forensics-memory-analysis-with-volatility-6f2b9e859765