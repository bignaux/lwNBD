# memory plugin

## Usage : access memory mapped device

TARGETS : all

STATUS : WIP

You just need to declare a memory slice:

```c
struct memory_config iopram = {
  .base = 0,
  .size = QueryMemSize(),
  .name = "ram",
  .desc = "IOP main RAM",
};
```

Then add it to the memory plugin:

```c
lwnbd_plugin_new(memplg, &iopram);
```

see full example in ports/playstation2/lwnbd_irx.c

a current limitation of some nbd client is their minimal read is 512 bytes (minimal blocksize) despite the protocol allow 1 byte blocksize.
It's not an issue to dump an entire device, but for now,
you need to put a minimal .size of 512 in your structure (and multiple of 512) or you'll get truncated data.

## PS2 note : 

* replacement for [ps2client dumpmem/writemem](https://github.com/ps2dev/ps2link/blob/a14d0b7b9ffb3b319dcfae49834e742b9ed12490/ee/cmdHandler.c#L243) and dump tools

```shell
ps2client -h 192.168.0.10 dumpmem host:bios.bin 0xbfc00000 0x400000
```

eq:

```shell
nbdcopy -p nbd://192.168.0.10/bios bios.bin
```

compare bios md5 sum with http://redump.org/datfile/ps2-bios/

*  [ImHex-PS2](https://github.com/bignaux/ImHex-PS2) is a repository for specific ps2 [ImHex](https://github.com/WerWolv/ImHex) stuff. It will have useful code to work with this plugin to do live RE/debug session. An [ImHex plugin content](https://github.com/WerWolv/ImHex/tree/master/plugins/builtin/source/content/providers) for nbd would be fine and allow 1 byte blocksize (libnbd) ...

### documentation :

 [Playstation 2 Memory Mapping](https://psi-rockin.github.io/ps2tek/#memorymap)
 
## dump tool analysis to review :

* [ImHex](https://github.com/WerWolv/ImHex) 
* http://www.wxhexeditor.org/
* https://github.com/allencch/med
* https://infosecwriteups.com/forensics-memory-analysis-with-volatility-6f2b9e859765