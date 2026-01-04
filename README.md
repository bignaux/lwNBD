lwNBD(3) -- A Lightweight software component framework
=============================================

## 👇 SYNOPSIS

    #include <lwnbd/lwnbd.h>
    #include <lwnbd/lwnbd-server.h>
    #include <lwnbd/lwnbd-plugin.h>

## ✨ DESCRIPTION

* Official repository : <https://github.com/bignaux/lwNBD>
* BSD-style socket API: lwIP 2.0.0 and Linux supported.

Targeting first the use on Playstation 2 IOP, a 37.5 MHz MIPS processor
and 2 MB of RAM, lwNBD is designed to run on bare metal or OS embedded system.
With modulararity and portability in mind, it is developed according to several
code standards, including :

* no dynamically allocate memory
* static-linking
* written in C99 (using -std=c99 compile-time flag), use standard library usage
* thread-safe synchronous NBD protocol implementation
* optional and experimental query support 

The lwNBD API is broken down into 3:

* an API to manage server and content to be embbed in apps. The idea is to be able to manage servers as xinetd would.

* a server API to create protocol and transport plugins. Currently, only support [NBD protocol](./servers/nbd/lwnbd-nbd-server.md), but could extend to Zmodem, AoE ... You then benefit from the mechanisms put in place for NBD such as content and management plugins.

* a plugin API to create content plugins. The plugin API has been entirely rewritten to be closer to the [nbdkit-plugin](https://libguestfs.org/nbdkit-plugin.3.html) one in order to benefit from the experience of their software architecture, and to facilitate the porting of code from one or the other library. An obstacle to this convergence is that nbdkit does not support the use of multiple plugins in a single instance.

## 📜 HISTORY

On Playstation 2, there is no standardized central partition table like GPT for
hard disk partitioning, nor is there a standard file system but PFS and
HDLoader. In fact, there are few tools capable of handling hard disks,
especially under Linux, and the servers developed in the past to handle these
disks via the network did not use a standard protocol, which required each
software wishing to handle the disks to include a specific client part,
which were broken when the toolchain was updated. The same goes for the memory
cards and other block devices on this console, which is why I decided to
implement NBD on this target first.

## 🪶 AUTHOR

Bignaux Ronan &lt;rbignaux at free.fr&gt;

## ©️ LICENSE

BSD
