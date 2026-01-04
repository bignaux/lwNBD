lwNBD-NBD-server(1) -- A NBD server
=============================================

TARGETS : ALL

STATUS : DONE

## Tested clients

Although this server is not yet complete in respect of the minimal requirements
defined by the [NBD protocol](https://github.com/NetworkBlockDevice/nbd/blob/master/doc/proto.md#baseline),
it is nevertheless usable with certain clients. In a [RERO spirit](https://en.wikipedia.org/wiki/Release_early,_release_often)
i publish this "AS-IS".

Known supported clients :

* nbdcopy (provided by libnbd)
* nbdfuse (provided by libnbd), works on windows with WSL2.
* nbd-client
* Ceph for Windows (wnbd-client.exe)

# *piconbd*

The NBD protocol itself is implemented in this 2 files :

* protocol-handshake.c
* protocol.c

'piconbd' is a name code for this files. Except from endianess operations, they are free from transport layer, we could use it on another transport than TCP? think UDP or local IPC. They still rely on context method, but it could be easily factored to be a separate library, letting people using it without all the mechanism of lwnbd. I'll fork lwnbd and let only this files to provide protocol implementation only.

# TODO

* allow zero-copy for memory mapped context, instead of recv/write/memcpy and memcpy/read/send scheme (wip)
* fix NBD_FLAG_FIXED_NEWSTYLE negotiation
* NBD_OPT_STARTTLS : TLS support with wolfssl ( see https://www.wolfssl.com/wolfssl-lwip-support/ ) 
@uyjulian "choose wolfssl because it support TLS 1.3" . In the other side, latest lwip has mbed support.
see https://openziti.io/tlsuv/
* turn to async/nonblock with event loop example based on select() then NBD_FLAG_CAN_MULTI_CONN, but playstation2 has no asynchronous API
* NBD_OPT_INFO/NBD_OPT_GO

NBD proto doesn't support streams, we need to discuss it @
https://lists.debian.org/nbd/ is the central mailing list for the NBD protocol
On stream, exportsize doesn't matter ... stream_get_size()
