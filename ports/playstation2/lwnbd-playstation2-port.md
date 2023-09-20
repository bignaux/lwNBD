## Asynchronous API ?


* https://eli.thegreenplace.net/2017/concurrent-servers-part-1-introduction/
* https://github.com/codepr/ev/tree/master

## PS2SDK Toolchain

EE => [gcc 11.3.0](https://github.com/ps2dev/ps2toolchain-ee/blob/main/scripts/002-gcc-stage1.sh), newlib

IOP => [gcc 11.3.0](https://github.com/ps2dev/ps2toolchain-iop/blob/main/scripts/002-gcc-stage1.sh), no newlib -nostdlib
  
IOP modules use -fno-toplevel-reorder for export/import .

EE : uint32_t => long unsigned int _> %ld


## workflow (see helloworld - wip)

TODO : auto from the lwnbd build system.
could add echo "https://dl-cdn.alpinelinux.org/alpine/edge/testing/" >> /etc/apk/repositories && apk update



    PS2_WORKSPACE=~/devel/ps2
    cd $PS2_WORKSPACE

    rm -fr Open-PS2-Loader/modules/network/lwNBD/
    ln -rs lwNBD Open-PS2-Loader/modules/network/lwNBD

    docker pull ghcr.io/ps2homebrew/ps2homebrew:main 
    docker run -it -w /app -v "$(pwd)":/app ghcr.io/ps2homebrew/ps2homebrew:main
    
    export PS2_WORKSPACE=/app (should be in a docker script)
    cd Open-PS2-Loader
    
    #git config --global --add safe.directory /app
    git config --system --add safe.directory "*"
    
    rm -f ./modules/network/lwNBD/lwnbdsvr.irx 
    make -C modules/network/lwNBD/ TARGET=iop clean && make LWNBD_DEBUG=1    
   
### commit on OPL
   
    dont forget to update lwNBD sha in download_lwNBD.sh and remove experimentale plugins.

## IOP debug

### udptty

  udptty.irx send stdout to udp broadcast.
  
  Open log
  
     nc -ukl 18194
    with promiscous : tshark -o data.show_as_text:TRUE -f "dst port 18194" -Tfields -e data.text
    #issue is filtering on dst only make cmd send to same port
    
    socat - UDP-LISTEN:18194
    socat - udp-recv:18194
    socat - udp:$PS2HOSTNAME:18194
    
    tshark -o data.show_as_text:TRUE -f "dst port 18194" -Tjson -e data.text | jq
    (todo: add filter from ps2, alias ps2listen )
    snffing : sudo tcpdump udp port 18194 -vv -A
  
### iopdebug

  iopdebug library to manage breakpoint. see in action : https://github.com/asmblur/pshell
  
  ps2rd https://github.com/mlafeldt/ps2rd/blob/master/Documentation/user-manual.txt
  
  https://www.psx-place.com/threads/retail-debugging-startup-card.14027/

## IOP Exception handling

On PS2SDK, you could use `ioptrap.irx` as exception handler : 


    IOP Exception : Data Bus Error
    EPC=00009d04 CAUSE=3000001c SR=00000404 BADVADDR=2d8559c9 DCIC=00000000
    module System_C_lib at unreloc offset 000017D4
    ra module System_C_lib at unreloc offset 00000ADC
    r[00]=00000000 r[01]=00160000 r[02]=00108bb0 r[03]=00000000r[04]=00108bb0 r[05]=003d0000 r[06]=00000080 r[07]=00000003r[08]=00000000 r[09]=0000000e r[10]=00000000 r[11]=00000000r[12]=00000000 r[13]=00001100 r[14]=00000000 r[15]=00000000r[16]=00000200 r[17]=00108bb0 r[18]=00000010 r[19]=00001000r[20]=003d0000 r[21]=00000004 r[22]=00000000 r[23]=00000005r[24]=00000001 r[25]=000f09b0 r[26]=0005e784 r[27]=00432a0dr[28]=00110670 r[29]=001dc0b0 r[30]=00108bb0 r[31]=0000900c

TODO reformat : 
00 00000000 00160000 00108bb0 00000000
04
08
0c
..
1c 

    EPC : Return address from trap
    CAUSE : Describes the most recently recognized exception
    SR : (status register) CPU mode flags
    BADVADDR : Note in particular that it is not set after a bus error. => ioptrap doesn't make difference between exception ...
    DCIC : 

read [EXCEPTION MANAGEMENT CHAPTER 4](https://psx.arthus.net/docs/R3000.pdf)


## standard I/O on PS2

from iop/kernel/include/ioman.h :

```C
typedef struct _iop_device {
        const char *name;
        unsigned int type;
        /** Not so sure about this one.  */
        unsigned int version;
        const char *desc;
        struct _iop_device_ops *ops;
} iop_device_t;
```

int AddDrv(iop_device_t *device);
int DelDrv(const char *name);

could lwnbd automatically wrap content plugin to ioman in a transport plugin and offer local connection on a file I/O API ?
It'd reduce driver depend on ioman wrapping (ie: udptty) . 

BDM is another tentative to make a common interface for PS2 block device, acting same like lwnbd. It's a shame to miss the opportunity to have a functional and universal interface regardless of the type of device, but that's the social situation of this project at this moment. Bringing out the possibilities of being able to share a common software interface from an input device like a gamepad to a storage device is not easy without having to show all the possibilities that this kind of thing offers.

## idea, c'mon pick one (or more!)

make oneliner like nbdcopy nbd://.../cmd/poweroff available like a bus , see @vates/nbd-client node stuff for bundle apps on phone ?
mem/spu2 ... mc/1/filename.
close Remotely Launch games https://github.com/ps2homebrew/Open-PS2-Loader/issues/418

custom build (like nbdtty ...)

Loader to replace ps2link

enable OPL in-game

screenshot plugin (see ee_core/src/igs_api.c or ps2sdk one)
GS plugin [see](https://github.com/F0bes/gs4ps2)

OPL UI "service manager" to configure plugin/server start opl{x} in-game{x} 

HDL plugin to provide easy drop of iso, copy entire device (ftp or http)

PADMAN plugin for TAS/HID... :

* https://github.com/Snugggles/PCSX2-AHK-TAS-Tools/tree/v1.0
* https://github.com/nmelihsensoy/virtual-hid-tcp
* https://dzone.com/articles/build-your-own-usb-hid-joystick-device-and-game-co
* https://www.youtube.com/watch?v=QpP6gfQVXgs

## TCP/IP stack

```
█▀ ▀█▀ ▄▀█ ▀█▀ █▀▀   █▀█ █▀▀   ▀█▀ █░█ █▀▀   █▀ ▀█▀ ▄▀█ █▀▀ █▄▀
▄█ ░█░ █▀█ ░█░ ██▄   █▄█ █▀░   ░█░ █▀█ ██▄   ▄█ ░█░ █▀█ █▄▄ █░█

Updated : 5/10/2023

  ┌───────────────────────────────────┐
  │           APPLICATION             │  We only have access to a subset of lwip
  │                                   │  basicly <lwip/socket.h>
  │ #include <ps2ip.h>                │
  │                                   │  Custom header makes portability hard.
  │                                   │  We don't benefit other lwip "apps" that
  │     /*  BSD-style socket */       │  could be available on ps2sdk.
  │                                   │
  └─────────────────┬─────────────────┘
                    │
                    │
  ┌─────────────────▼─────────────────┐  ┌────────┐
  │          Network stack            │  │  LWIP  │ Cherry-picking compilation in ps2sdk of forked
  │                                   ├──►        │ lwip @
  │ IOP: PS2IP.IRX                    │  │        │ https://github.com/ps2dev/lwip/tree/ps2-v2.0.3
  │ EE:  -lps2ip -lps2ips             │  └────────┘
  │                                   │
  │ #include "lwip/netif.h"           │  We have to manually wrap lwip function
  │                                   │  in the headers (ee/iop ps2ip.h, RPC ps2ips.h).
  │                                   │
  │                                   │  A lot of code duplication there!
  │ #include <netman.h>               │
  │                                   │
  │ NetManRegisterNetworkStack()      │
  │ NetManUnregisterNetworkStack()    │
  │                                   │
  └─────────────────┬─────────────────┘
                    │1
                    │
  ┌─────────────────▼─────────────────┐
  │       Network Manager             │
  │                                   │
  │ IOP: NETMAN.IRX                   │
  │ EE: -lnetman                      │
  │                                   │
  └─────────────────▲─────────────────┘
                    │
                    │*
  ┌─────────────────┴─────────────────┐
  │           Network driver          │
  │                                   │
  │ IOP: SMAP.IRX                     │
  │                                   │
  │                                   │  More than one network adaptor driver can be registered, 
  │ #include <netman.h>               │  but only the first one that has a link up state will be used.
  │                                   │  (NETMAN.txt) -> no route table mecanism ?
  │ NetManRegisterNetIF()             │
  │ NetManUnregisterNetIF()           │
  │                                   │
  └───────────────────────────────────┘
  
  ( https://asciiflow.com )
```
add SMAP use of lwip too (at least with BUILDING_SMAP_PS2IP)
localhost ?
iperf ? https://gitlab.com/ps2max/ps2sdk/-/commit/c4b71b54b83964dacb95ca66782f33100d98507e

in-game OPL uses another stack forked from SMS :

* SMSTCPIP is old stripped lwip port (2003) with option to use less memory (#define INGAME_DRIVER). 
* SMSUTILS is asm memcpy
* SMSMAP is outdated SMAP comparing to PS2SDK but get more delta.


