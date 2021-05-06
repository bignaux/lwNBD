/****************************************************************/ /**
 *
 * @file nbd_server.h
 *
 * @author   Ronan Bignaux <ronan@aimao.org>
 *
 * @brief    Network Block Device Protocol implementation options
 *
 * Copyright (c) Ronan Bignaux. 2021
 * All rights reserved.
 *
 ********************************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification,are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Ronan Bignaux <ronan@aimao.org>
 *
 */

#ifndef LWIP_HDR_APPS_NBD_SERVER_H
#define LWIP_HDR_APPS_NBD_SERVER_H

#include "nbd-protocol.h"
#include "nbd_opts.h"

//#include "lwip/apps/nbd_opts.h"
//#include "lwip/err.h"
//#include "lwip/pbuf.h"
//#include "lwip/mem.h"

#ifdef PS2SDK
#include <ps2ip.h>
#include <stdio.h>
#include <sysclib.h>
#include <stdint.h>
//#include <errno.h>
//#include <malloc.h>

//TODO: Missing <byteswap.h> in PS2SDK
static inline uint64_t bswap64(uint64_t x)
{
    return (((x & 0xff00000000000000ull) >> 56) | ((x & 0x00ff000000000000ull) >> 40) | ((x & 0x0000ff0000000000ull) >> 24) | ((x & 0x000000ff00000000ull) >> 8) | ((x & 0x00000000ff000000ull) << 8) | ((x & 0x0000000000ff0000ull) << 24) | ((x & 0x000000000000ff00ull) << 40) | ((x & 0x00000000000000ffull) << 56));
}

//TODO: Missing in PS2SK's "common/include/tcpip.h"
#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) bswap64(x)
#define ntohll(x) bswap64(x)
#endif

//TODO: Missing in PS2SK's <stdint.h> , needed for "nbd-protocol.h"
// https://en.cppreference.com/w/c/types/integer
#define UINT64_MAX 0xffffffffffffffff
#define UINT64_C(x) ((x) + (UINT64_MAX - UINT64_MAX))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup nbd
 * NBD context containing callback functions for NBD transfers
 */

//extern uint8_t buffer[];
uint8_t buffer[NBD_BUFFER_LEN] __attribute__((aligned(64)));

struct nbd_context
{

    char export_name[32];
    char export_desc[64];
    uint64_t export_size; /* size of export in byte */
    uint16_t eflags;      /* per-export flags */
    uint16_t blocksize;



    int (*export_init)(struct nbd_context *ctx);

    /**
   * Open file for read/write.
   * @param fname Filename
   * @param mode Mode string from NBD RFC 1350 (netascii, octet, mail)
   * @param write Flag indicating read (0) or write (!= 0) access
   * @returns File handle supplied to other functions
   */
    //  void* (*open)(const char* fname, const char* mode, u8_t write);
    /**
   * Close file handle
   * @param handle File handle returned by open()
   */
    //  void (*close)(void* handle);
    /**
   * Read from file 
   * @param handle File handle returned by open()
   * @param buf Target buffer to copy read data to
   * @param bytes Number of bytes to copy to buf
   * @returns &gt;= 0: Success; &lt; 0: Error
   */
    //  int (*read)(void* handle, void* buf, int bytes);
    int (*read)(void *buffer, uint64_t offset, uint32_t length);
    /**
   * Write to file
   * @param handle File handle returned by open()
   * @param pbuf PBUF adjusted such that payload pointer points
   *             to the beginning of write data. In other words,
   *             NBD headers are stripped off.
   * @returns &gt;= 0: Success; &lt; 0: Error
   */
    //  int (*write)(void* handle, struct pbuf* p);
    int (*write)(void *buffer, uint64_t offset, uint32_t length);
    int (*flush)(void);
};

int nbd_init(struct nbd_context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* LWIP_HDR_APPS_NBD_SERVER_H */
