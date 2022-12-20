## PS2SDK Toolchain

EE => gcc 10.2.0, newlib
IOP => gcc 3.2.3, no newlib -nostdlib
IOP modules use -fno-toplevel-reorder 

EE : uint32_t => long unsigned int _> %ld



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
It'd reduce driver depend on ioman the wrapping (ie: udptty)

## TCP/IP stack

```
█▀ ▀█▀ ▄▀█ ▀█▀ █▀▀   █▀█ █▀▀   ▀█▀ █░█ █▀▀   █▀ ▀█▀ ▄▀█ █▀▀ █▄▀
▄█ ░█░ █▀█ ░█░ ██▄   █▄█ █▀░   ░█░ █▀█ ██▄   ▄█ ░█░ █▀█ █▄▄ █░█

Updated : 5/10/2023
Author  : Ronan Bignaux

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

