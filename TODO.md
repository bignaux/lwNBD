## TODO

* fix NBD_FLAG_FIXED_NEWSTYLE negotiation
* NBD_OPT_STARTTLS : TLS support with wolfssl ( see https://www.wolfssl.com/wolfssl-lwip-support/ ) 
@uyjulian "choose wolfssl because it support TLS 1.3" . In the other side, latest lwip has mbed support.
* event loop based on select() then NBD_FLAG_CAN_MULTI_CONN, but playstation2 has no asynchronous API
* provide a clean API
* NBD_OPT_INFO/NBD_OPT_GO
