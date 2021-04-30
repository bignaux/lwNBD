/****************************************************************/ /**
 *
 * @file nbd_server.c
 *
 * @author   Ronan Bignaux <ronan@aimao.org>
 *
 * @brief    Network Block Device Protocol server
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

/**
 * @defgroup nbd NBD server
 * @ingroup apps
 *
 * This is simple NBD server for the lwIP raw API.
 */

#include "nbd_server.h"

//NBD_MAX_STRING is minimal size for the buffer
static uint8_t buffer[256 * 512] __attribute__((aligned(64)));

/** @ingroup nbd
 * Fixed newstyle negotiation.
 * @param tcp_client_socket
 * @param ctx NBD callback struct
 */
static int negotiate_handshake_newstyle(int tcp_client_socket, struct nbd_context *ctx)
{
    register int size;
    uint32_t cflags, name_len, desc_len, len;
    struct nbd_new_option new_opt;
    struct nbd_export_name_option_reply handshake_finish;
    struct nbd_fixed_new_option_reply fixed_new_option_reply;
    struct nbd_new_handshake new_hs;

    new_hs.nbdmagic = htonll(NBD_MAGIC);
    new_hs.version = htonll(NBD_NEW_VERSION);
    new_hs.gflags = 0;
    size = send(tcp_client_socket, &new_hs, sizeof(struct nbd_new_handshake),
                0);
    if (size < sizeof(struct nbd_new_handshake))
        goto error;

    size = recv(tcp_client_socket, &cflags, sizeof(cflags), 0);
    if (size < sizeof(cflags))
        goto error;
    cflags = htonl(cflags);
    //TODO: manage cflags

    ctx->eflags = NBD_FLAG_HAS_FLAGS | NBD_FLAG_FIXED_NEWSTYLE; // | NBD_FLAG_READ_ONLY

    while (1) {

        /*
		 * options haggling
		 *
		 */
        size = recv(tcp_client_socket, &new_opt, sizeof(struct nbd_new_option),
                    0);
        if (size < sizeof(struct nbd_new_option))
            goto error;

        new_opt.version = ntohll(new_opt.version);
        if (new_opt.version != NBD_NEW_VERSION)
            goto error;

        new_opt.option = htonl(new_opt.option);
        new_opt.optlen = htonl(new_opt.optlen);

        if (new_opt.optlen > 0) {
            size = recv(tcp_client_socket, &buffer, new_opt.optlen, 0);
            buffer[new_opt.optlen] = '\0';
        }

        switch (new_opt.option) {

            case NBD_OPT_EXPORT_NAME:

                memset(&handshake_finish, 0,
                       sizeof(struct nbd_export_name_option_reply));
                handshake_finish.exportsize = htonll(ctx->export_size);
                handshake_finish.eflags = htons(ctx->eflags);
                size = send(tcp_client_socket, &handshake_finish,
                            sizeof(struct nbd_export_name_option_reply), 0);

                goto abort;

            case NBD_OPT_ABORT:
                //TODO : test
                fixed_new_option_reply.magic = htonll(NBD_REP_MAGIC);
                fixed_new_option_reply.option = htonl(new_opt.option);
                fixed_new_option_reply.reply = htonl(NBD_REP_ACK);
                fixed_new_option_reply.replylen = 0;
                size = send(tcp_client_socket, &fixed_new_option_reply,
                            sizeof(struct nbd_fixed_new_option_reply), 0);
                goto soft_disconnect;
                break;
            // see nbdkit send_newstyle_option_reply_exportnames()
            case NBD_OPT_LIST:

                name_len = strlen(ctx->export_name);
                desc_len = ctx->export_desc ? strlen(ctx->export_desc) : 0;
                len = htonl(name_len);

                //TODO : many export in a loop
                fixed_new_option_reply.magic = htonll(NBD_REP_MAGIC);
                fixed_new_option_reply.option = htonl(new_opt.option);
                fixed_new_option_reply.reply = htonl(NBD_REP_SERVER);
                fixed_new_option_reply.replylen = htonl(name_len + sizeof(len) +
                                                        desc_len);

                size = send(tcp_client_socket, &fixed_new_option_reply,
                            sizeof(struct nbd_fixed_new_option_reply), MSG_MORE);
                size = send(tcp_client_socket, &len, sizeof len, MSG_MORE);
                size = send(tcp_client_socket, ctx->export_name, name_len, MSG_MORE);
                size = send(tcp_client_socket, ctx->export_desc, desc_len, 0);
                break;
                //TODO
                //                break;
                //            case NBD_OPT_STARTTLS:
                //                break;
                // see nbdkit send_newstyle_option_reply_info_export()
            case NBD_OPT_INFO:
            case NBD_OPT_GO:


                //            	if (new_opt.option == NBD_OPT_GO)
                //            		goto abort;
                //            	break;
            case NBD_OPT_STRUCTURED_REPLY:
            case NBD_OPT_LIST_META_CONTEXT:
            case NBD_OPT_SET_META_CONTEXT:
            default:
                //TODO: test
                fixed_new_option_reply.magic = htonll(NBD_REP_MAGIC);
                fixed_new_option_reply.option = htonl(new_opt.option);
                fixed_new_option_reply.reply = htonl(NBD_REP_ERR_UNSUP);
                fixed_new_option_reply.replylen = 0;
                size = send(tcp_client_socket, &fixed_new_option_reply,
                            sizeof(struct nbd_fixed_new_option_reply), 0);
                break;
        }
    }

abort:
    return 0;
soft_disconnect:
error:
    return -1;
}

/** @ingroup nbd
 * Transmission phase.
 * @param tcp_client_socket
 * @param ctx NBD callback struct
 */
int transmission_phase(int tcp_client_socket, struct nbd_context *ctx)
{
    register int i, r, size, error, retry = NBD_MAX_RETRIES, sendflag = 0;
    register uint32_t blkremains, byteread, bufbklsz;
    register uint64_t offset;
    struct nbd_simple_reply reply;
    struct nbd_request request;

    //    static uint8_t buffer[512];
    //    buffer = malloc(ctx->blocksize);

    reply.magic = htonl(NBD_SIMPLE_REPLY_MAGIC);

    while (1) {

        /*
		 * requests handling
		 *
		 */

        // TODO : blocking here if no proper NBD_CMD_DISC, bad threading design ?
        size = recv(tcp_client_socket, &request, sizeof(struct nbd_request), 0);
        if (size < sizeof(struct nbd_request))
            goto error;

        request.magic = ntohl(request.magic);
        if (request.magic != NBD_REQUEST_MAGIC)
            goto error;

        request.flags = ntohs(request.flags);
        request.type = ntohs(request.type);
        request.offset = ntohll(request.offset);
        request.count = ntohl(request.count);

        reply.handle = request.handle;

        switch (request.type) {

            case NBD_CMD_READ:

                if (request.offset + request.count > ctx->export_size)
                    error = NBD_EIO;
                else {
                    error = NBD_SUCCESS;
                    sendflag = MSG_MORE;
                }

                reply.error = ntohl(error);

                r = send(tcp_client_socket, &reply, sizeof(struct nbd_simple_reply),
                		sendflag);

                if (!sendflag)
                    break;

                bufbklsz = 256; // NBD_MAX_STRING / ctx->blocksize;
                blkremains = request.count / ctx->blocksize;
                offset = request.offset;
                byteread = bufbklsz * ctx->blocksize;

                while (sendflag && retry) {

                    if (blkremains < bufbklsz) {
                        bufbklsz = blkremains;
                        byteread = bufbklsz * ctx->blocksize;
                    }

                    if (blkremains <= bufbklsz)
                        sendflag = 0;

                    r = ctx->read(buffer, offset, bufbklsz);

                    if (r == 0) {
                        r = send(tcp_client_socket, buffer, byteread, sendflag);
                        if (r != byteread)
                            break;
                        offset += byteread;
                        blkremains -= bufbklsz;
                        retry = NBD_MAX_RETRIES;
                    }
                    else {
//                    	LWIP_DEBUGF(NBD_DEBUG | LWIP_DBG_STATE, ("nbd: error read\n"));
                    	retry--;
                    }
                }
                break;

            case NBD_CMD_WRITE:
                //TODO: test

                if (ctx->eflags & NBD_FLAG_READ_ONLY)
                    error = NBD_EPERM;
                else if (request.offset + request.count > ctx->export_size)
                    error = NBD_ENOSPC;
                else
                    error = NBD_SUCCESS;

                reply.error = ntohl(error);
                r = send(tcp_client_socket, &reply, sizeof(struct nbd_simple_reply),
                         0);

                if (error != NBD_SUCCESS)
                    break;

                //Stupid recv one block write one block
                for (i = 0; i < request.count / ctx->blocksize; i++) {

                    r = recv(tcp_client_socket, buffer, ctx->blocksize, 0);
                    if (r == -1)
                        break;

                    r = ctx->write(buffer, request.offset + i * ctx->blocksize, 1);
                }

                break;

            case NBD_CMD_DISC:
                //TODO
                goto soft_disconnect;
                break;

            case NBD_CMD_FLUSH:
                ctx->flush();
                break;

            default:
                /* NBD_REP_ERR_INVALID */
                goto error;
        }
    }

soft_disconnect:
    return 0;
error:
    return -1;
}

/** @ingroup nbd
 * Initialize NBD server.
 * @param ctx NBD callback struct
 */
int nbd_init(struct nbd_context *ctx)
{
    int tcp_socket, client_socket = -1;
    struct sockaddr_in peer;
    socklen_t addrlen;
    register int r;

    peer.sin_family = AF_INET;
    peer.sin_port = htons(NBD_SERVER_PORT);
    peer.sin_addr.s_addr = htonl(INADDR_ANY);

    while (1) {

        tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (tcp_socket < 0)
            goto error;

        r = bind(tcp_socket, (struct sockaddr *)&peer, sizeof(peer));
        if (r < 0)
            goto error;

        r = listen(tcp_socket, 3);
        if (r < 0)
            goto error;

        while (1) {

            addrlen = sizeof(peer);
            client_socket = accept(tcp_socket, (struct sockaddr *)&peer,
                                   &addrlen);
            if (client_socket < 0)
                goto error;

            r = negotiate_handshake_newstyle(client_socket, ctx);
            if (r == 0)
                transmission_phase(client_socket, ctx);

            closesocket(client_socket);
        }

    error:
        closesocket(tcp_socket);
    }
}
