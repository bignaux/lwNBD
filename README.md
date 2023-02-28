lwNBD(3) -- A Lightweight NBD server library
=============================================

## SYNOPSIS

    #include <lwnbd.h>
    #include <lwnbd-server.h>
    #include <lwnbd-plugin.h>

## DESCRIPTION

* Official repository : <https://github.com/bignaux/lwNBD>
* BSD-style socket API: lwIP 2.0.0 and Linux supported.

Targeting first the use on Playstation 2 IOP, a 37.5 MHz MIPS processor
and 2 MB of RAM, lwNBD is designed to run on bare metal or OS embedded system.
With modulararity and portability in mind, it is developed according to several
code standards, including :

* no dynamically allocate memory
* static-linking
* C standard library usage

It has 3 API :

* an API to manage server and content to be embbed in apps. The idea is to be able to manage servers as an xinetd would.

* a server API to create protocol and transport plugins. Currently, only support NBD protocol, but could extend to Zmodem, AoE ... You then benefit from the mechanisms put in place for NBD such as content and management plugins.

* a plugin API to create content plugins. The plugin API has been entirely rewritten to be closer to the [nbdkit-plugin](https://libguestfs.org/nbdkit-plugin.3.html) one in order to benefit from the experience of their software architecture, and to facilitate the porting of code from one or the other library. An obstacle to this convergence is that nbdkit does not support the use of multiple plugins in a single instance.

There are 2 targets supported :

* GNU/Linux that use *file* plugin to
  serve a list of files as command line parameters. For the time being, the main 
  purpose of the support is to facilitate development.

* Playstation 2 IOP via an IRX module for [Open-PS2-Loader](https://github.com/ps2homebrew/Open-PS2-Loader).
  It can export hdd drive (*atad* plugin), MemoryCard (*mcman* plugin), rom0 and IOP ram (*memory* plugin). PS2SDK use lwip [v2.0.3](https://github.com/ps2dev/lwip/tree/ps2-v2.0.3).

## HISTORY

On Playstation 2, there is no standardized central partition table like GPT for
hard disk partitioning, nor is there a standard file system but PFS and
HDLoader. In fact, there are few tools capable of handling hard disks,
especially under Linux, and the servers developed in the past to handle these
disks via the network did not use a standard protocol, which required each
software wishing to handle the disks to include a specific client part,
which were broken when the toolchain was updated. The same goes for the memory
cards and other block devices on this console, which is why I decided to
implement NBD on this target first.

## STATUS

Although this server is not yet complete in respect of the minimal requirements
defined by the [NBD protocol](https://github.com/NetworkBlockDevice/nbd/blob/master/doc/proto.md#baseline),
it is nevertheless usable with certain clients. In a [RERO spirit](https://en.wikipedia.org/wiki/Release_early,_release_often)
i publish this "AS-IS".

Known supported clients :

* nbdcopy (provided by libnbd)
* nbdfuse (provided by libnbd), works on windows with WSL2.
* nbd-client
* Ceph for Windows (wnbd-client.exe)

## AUTHOR

Bignaux Ronan &lt;ronan at aimao.org&gt;

## LICENSE

BSD
