#include <stddef.h>
#include <string.h>
#include "nbd.h"

/*
 * This could later be independent of transport implementation
 * to be shared between all NBD-like servers.
 *
 */


/*
 * IOP :
 *
 * The IRX loader can only handle max alignment of 16 bytes.
 * Future updates to the toolchain will make this a hard requirement.
 *
 * TODO : check alignment and move per-plateform if specific define ALIGMENT
 */
uint8_t nbd_buffer[NBD_BUFFER_LEN] __attribute__((aligned(16)));

/* temp */
#define BLOCKSIZE 1

err_t negotiation_phase(struct nbd_server *server, const int client_socket, struct lwnbd_context **ctx)
{
    register int size;
    uint32_t cflags;
    struct nbd_new_option new_opt;
    struct nbd_fixed_new_option_reply fixed_new_option_reply;
    struct nbd_new_handshake new_hs;

    /*** handshake ***/

    new_hs.nbdmagic = htonll(NBD_MAGIC);
    new_hs.version = htonll(NBD_NEW_VERSION);
    new_hs.gflags = htons(server->gflags);

    size = send(client_socket, &new_hs, sizeof(struct nbd_new_handshake),
                0);
    if (size < sizeof(struct nbd_new_handshake))
        return -1;

    size = nbd_recv(client_socket, &cflags, sizeof(cflags), 0);
    if (size < sizeof(cflags))
        return -1;
    cflags = htonl(cflags);

    DEBUGLOG("client flags %d, server flags %d\n", cflags, server->gflags);
    if (cflags != server->gflags) {
        LOG("Unsupported client flags %u\n", cflags);
        return -1;
    }

    while (1) {

        /*** options haggling ***/

        size = nbd_recv(client_socket, &new_opt, sizeof(struct nbd_new_option),
                        0);
        if (size < sizeof(struct nbd_new_option))
            return -1;

        new_opt.version = ntohll(new_opt.version);
        if (new_opt.version != NBD_NEW_VERSION)
            return -1;

        new_opt.option = htonl(new_opt.option);
#ifdef DEBUG
        static const char *NBD_OPTIONS[] = {
            NULL,
            "NBD_OPT_EXPORT_NAME",
            "NBD_OPT_ABORT",
            "NBD_OPT_LIST",
            "NBD_OPT_STARTTLS",
            "NBD_OPT_INFO",
            "NBD_OPT_GO",
            "NBD_OPT_STRUCTURED_REPLY",
            "NBD_OPT_LIST_META_CONTEXT",
            "NBD_OPT_SET_META_CONTEXT",
        };
        DEBUGLOG("%s\n", NBD_OPTIONS[new_opt.option]);
#endif
        new_opt.optlen = htonl(new_opt.optlen);

        if (new_opt.optlen > 0) {
            size = nbd_recv(client_socket, &nbd_buffer, new_opt.optlen, 0);
            nbd_buffer[new_opt.optlen] = '\0';
            DEBUGLOG("client option: sz=%d %s.\n", new_opt.optlen, nbd_buffer);
        }

        switch (new_opt.option) {

            case NBD_OPT_EXPORT_NAME: {
                struct nbd_export_name_option_reply handshake_finish;
                // temporary workaround
                if (new_opt.optlen > 0) {
                    *ctx = lwnbd_get_context((const char *)&nbd_buffer);
                } else
                    *ctx = lwnbd_get_context(server->defaultexport);
                // TODO: is that correct ?
                //				if (*ctx == NULL)
                //					*ctx = ctxs[0];

                /* proto.md: If the server is unwilling to allow the export, it MUST terminate the session. */
                if (*ctx == NULL) {
                    DEBUGLOG("find nothing to export.\n");
                    return -1;
                }

                handshake_finish.exportsize = htonll((*ctx)->exportsize);
                handshake_finish.eflags = htons((*ctx)->eflags);
                memset(handshake_finish.zeroes, 0, sizeof(handshake_finish.zeroes));
                // NBD_FLAG_C_NO_ZEROES not defined by nbd-protocol.h, another useless term from proto.md
                size = send(client_socket, &handshake_finish,
                            (cflags & NBD_FLAG_NO_ZEROES) ? offsetof(struct nbd_export_name_option_reply, zeroes) : sizeof handshake_finish, 0);
                return NBD_OPT_EXPORT_NAME;
            }

            case NBD_OPT_ABORT:
                fixed_new_option_reply.magic = htonll(NBD_REP_MAGIC);
                fixed_new_option_reply.option = htonl(new_opt.option);
                fixed_new_option_reply.reply = htonl(NBD_REP_ACK);
                fixed_new_option_reply.replylen = 0;
                size = send(client_socket, &fixed_new_option_reply,
                            sizeof(struct nbd_fixed_new_option_reply), 0);
                return NBD_OPT_ABORT;

            case NBD_OPT_LIST: {

                size_t i, list_len;
                fixed_new_option_reply.magic = htonll(NBD_REP_MAGIC);
                fixed_new_option_reply.option = htonl(new_opt.option);
                fixed_new_option_reply.reply = htonl(NBD_REP_SERVER);
                list_len = lwnbd_contexts_count();

                DEBUGLOG("%d export.\n", list_len);

                for (i = 0; i < list_len; i++) {
                    const struct lwnbd_context *context = lwnbd_get_context_i(i);
                    size_t name_len = strlen(context->name);

                    // TODO : fix there
                    size_t desc_len = context->description ? strlen(context->description) : 0;
                    uint32_t len;

                    DEBUGLOG("%s\n", (context->name));

                    len = htonl(name_len);
                    fixed_new_option_reply.replylen = htonl(
                        name_len + sizeof(len) + desc_len);

                    size = send(client_socket, &fixed_new_option_reply,
                                sizeof(struct nbd_fixed_new_option_reply), MSG_MORE);
                    size = send(client_socket, &len, sizeof len, MSG_MORE);
                    size = send(client_socket, context->name, name_len,
                                MSG_MORE);
                    size = send(client_socket, context->description, desc_len,
                                MSG_MORE);
                }
                fixed_new_option_reply.reply = htonl(NBD_REP_ACK);
                fixed_new_option_reply.replylen = 0;
                size = send(client_socket, &fixed_new_option_reply,
                            sizeof(struct nbd_fixed_new_option_reply), 0);
                break;
            }

            case NBD_OPT_INFO:
            case NBD_OPT_GO:
            case NBD_OPT_STRUCTURED_REPLY:
            case NBD_OPT_LIST_META_CONTEXT:
            case NBD_OPT_SET_META_CONTEXT:
            default:
                // TODO: test
                fixed_new_option_reply.magic = htonll(NBD_REP_MAGIC);
                fixed_new_option_reply.option = htonl(new_opt.option);
                fixed_new_option_reply.reply = htonl(NBD_REP_ERR_UNSUP);
                fixed_new_option_reply.replylen = 0;
                size = send(client_socket, &fixed_new_option_reply,
                            sizeof(struct nbd_fixed_new_option_reply), 0);
                break;
        }
    }
}

err_t transmission_phase(const int client_socket, struct lwnbd_context *ctx)
{
    // TODO: fix bug on non 512 blocksize context
    register int r, size, error = -1, retry = MAX_RETRIES, sendflag = 0;
    register uint32_t blkremains = 0, byteread = 0, bufblksz = 0;
    register uint64_t offset = 0;
    struct nbd_simple_reply reply;
    struct nbd_request request;

    if (ctx == NULL) {
        LOG("No context provided.\n");
        return -1;
    }

    while (1) {

        /*** requests handling ***/

        // TODO : blocking here if no proper NBD_CMD_DISC, bad threading design ?
        DEBUGLOG("Wait NBD_CMD ...\n");
        size = nbd_recv(client_socket, &request, sizeof(struct nbd_request), 0);
        if (size < sizeof(struct nbd_request)) {
            LOG("sizeof nbd_request waited %ld receveid %d\n", sizeof(struct nbd_request), size);
            return -1;
        }

        request.magic = ntohl(request.magic);
        if (request.magic != NBD_REQUEST_MAGIC) {
            LOG("wrong NBD_REQUEST_MAGIC\n");
            return -1;
        }

        request.flags = ntohs(request.flags);
        request.type = ntohs(request.type);
        request.offset = ntohll(request.offset);
        request.count = ntohl(request.count);

        reply.magic = htonl(NBD_SIMPLE_REPLY_MAGIC);
        reply.handle = request.handle;

#ifdef DEBUG
        static const char *NBD_CMD[] = {
            "NBD_CMD_READ",
            "NBD_CMD_WRITE",
            "NBD_CMD_DISC",
            "NBD_CMD_FLUSH",
            "NBD_CMD_TRIM",
            "NBD_CMD_CACHE",
            "NBD_CMD_WRITE_ZEROES",
            "NBD_CMD_BLOCK_STATUS",
        };
        LOG("%s\n", NBD_CMD[request.type]);
#define PRI_UINT64_C_Val(value) ((unsigned long)(value >> 32)), ((unsigned long)value)
#define PRI_UINT64              "%lx%lx"
#endif

        switch (request.type) {

            case NBD_CMD_READ:

                DEBUGLOG("request off=0x" PRI_UINT64 ", size %lu\n", PRI_UINT64_C_Val(request.offset), request.count);
                if (request.offset == 0) {
                    DEBUGLOG("request.offset = 0 \n");
                }
                if (request.count == 4096) {
                    DEBUGLOG("request.count = 4096 \n");
                }
                if (request.offset + request.count > ctx->exportsize)
                    error = NBD_EIO;
                else {
                    error = NBD_SUCCESS;
                    sendflag = MSG_MORE;
                    bufblksz = NBD_BUFFER_LEN / BLOCKSIZE;
                    blkremains = request.count / BLOCKSIZE;
                    offset = request.offset / BLOCKSIZE;
                    byteread = bufblksz * BLOCKSIZE;
                }

                reply.error = ntohl(error);
                r = send(client_socket, &reply, sizeof(struct nbd_simple_reply),
                         sendflag);
                if (r != sizeof(struct nbd_simple_reply)) {
                    LOG("send nbd_simple_reply failed\n");
                }

                while (sendflag) {

                    if (blkremains < bufblksz) {
                        bufblksz = blkremains;
                        byteread = bufblksz * BLOCKSIZE;
                    }

                    if (blkremains <= bufblksz)
                        sendflag = 0;

                    r = plugin_pread(ctx, nbd_buffer, bufblksz, offset, 0);
                    //                    DEBUGLOG("offset=%d bufblksz=%d sendflag=%d\n", offset, bufblksz, sendflag);

                    if (r == 0) {
                        DEBUGLOG("send \n");
                        r = send(client_socket, nbd_buffer, byteread, sendflag);
                        if (r != byteread) {
                            LOG("NBD_CMD_READ : send failed r=%d byteread=%d\n", r, byteread);
                            break;
                        } else
                            LOG("NBD_CMD_READ : send OK \n");
                        offset += bufblksz;
                        blkremains -= bufblksz;
                        retry = MAX_RETRIES;
                    } else if (retry) {
                        LOG("NBD_CMD_READ : nbd_pread NOK \n");
                        retry--;
                        sendflag = 1;
                    } else {
                        LOG("NBD_CMD_READ : EIO\n");
                        return -1; // -EIO
                                   //                    	LWIP_DEBUGF(NBD_DEBUG | LWIP_DBG_STATE, ("nbd: error read\n"));
                    }
                }
                break;

            case NBD_CMD_WRITE:

                if (ctx->eflags & NBD_FLAG_READ_ONLY)
                    error = NBD_EPERM;
                else if (request.offset + request.count > ctx->exportsize)
                    error = NBD_ENOSPC;
                else {
                    error = NBD_SUCCESS;
                    sendflag = MSG_MORE;
                    bufblksz = NBD_BUFFER_LEN / BLOCKSIZE;
                    blkremains = request.count / BLOCKSIZE;
                    offset = request.offset / BLOCKSIZE;
                    byteread = bufblksz * BLOCKSIZE;
                }

                while (sendflag) {

                    if (blkremains < bufblksz) {
                        bufblksz = blkremains;
                        byteread = bufblksz * BLOCKSIZE;
                    }

                    if (blkremains <= bufblksz)
                        sendflag = 0;

                    r = nbd_recv(client_socket, nbd_buffer, byteread, 0);

                    if (r == byteread) {
                        r = plugin_pwrite(ctx, nbd_buffer, bufblksz, offset, 0);
                        if (r != 0) {
                            error = NBD_EIO;
                            sendflag = 0;
                            //                    	LWIP_DEBUGF(NBD_DEBUG | LWIP_DBG_STATE, ("nbd: error read\n"));
                        }
                        offset += bufblksz;
                        blkremains -= bufblksz;
                        retry = MAX_RETRIES;
                    } else {
                        error = NBD_EOVERFLOW; // TODO
                        sendflag = 0;
                        //                    	LWIP_DEBUGF(NBD_DEBUG | LWIP_DBG_STATE, ("nbd: error read\n"));
                    }
                }

                reply.error = ntohl(error);
                r = send(client_socket, &reply, sizeof(struct nbd_simple_reply),
                         0);
                break;

            case NBD_CMD_DISC:
                // There is no reply to an NBD_CMD_DISC.
                return 0;

            case NBD_CMD_FLUSH:
                error = (plugin_flush(ctx, 0) == 0) ? NBD_SUCCESS : NBD_EIO;
                reply.error = ntohl(error);
                r = send(client_socket, &reply, sizeof(struct nbd_simple_reply),
                         0);
                break;

            case NBD_CMD_TRIM:
            case NBD_CMD_CACHE:
            case NBD_CMD_WRITE_ZEROES:
            case NBD_CMD_BLOCK_STATUS:
            default:
                /* The server SHOULD return NBD_EINVAL if it receives an unknown command. */
                reply.error = ntohl(NBD_EINVAL);
                r = send(client_socket, &reply, sizeof(struct nbd_simple_reply),
                         0);
                break;
        }
    }
    return -1;
}