lwNBD-NBD-server(1) -- A NBD server
=============================================

TARGETS : ALL

STATUS : DONE

# TODO

* fix NBD_FLAG_FIXED_NEWSTYLE negotiation
* NBD_OPT_STARTTLS : TLS support with wolfssl ( see https://www.wolfssl.com/wolfssl-lwip-support/ ) 
@uyjulian "choose wolfssl because it support TLS 1.3" . In the other side, latest lwip has mbed support.
see https://openziti.io/tlsuv/
* turn to async/nonblock with event loop example based on select() then NBD_FLAG_CAN_MULTI_CONN, but playstation2 has no asynchronous API
* NBD_OPT_INFO/NBD_OPT_GO

NBD proto doesn't support streams, we need to discuss it @
https://lists.debian.org/nbd/ is the central mailing list for the NBD protocol
On stream, exportsize doesn't matter ... stream_get_size()
