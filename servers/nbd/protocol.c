#include <lwnbd/piconbd.h>

const char *nbd_commands_to_string(uint16_t f)
{
    static const char *const nbd_commands[] = {
        "NBD_CMD_READ",
        "NBD_CMD_WRITE",
        "NBD_CMD_DISC",
        "NBD_CMD_FLUSH",
        "NBD_CMD_TRIM",
        "NBD_CMD_CACHE",
        "NBD_CMD_WRITE_ZEROES",
        "NBD_CMD_BLOCK_STATUS",
    };

    return nbd_commands[f];
}

int transmission_phase(lwnbd_context_nbd_t *ctx, struct nbd_client *c)
{
    // TODO: fix bug on non 512 blocksize context (should be now)
    register int r, error, retry = CONFIG_MAX_RETRIES, sendflag = 0;
    register uint32_t blocksize, blkremains = 0, byteread = 0, bufblksz = 0;
    register uint64_t offset = 0;
    struct nbd_simple_reply reply;
    struct nbd_request request;
    uint8_t nbd_buffer[MAX_REQUEST_SIZE];

    if (ctx == NULL) {
        lwnbd_info("No context provided.\n");
        return -1;
    }

    blocksize = ctx->minimum_block_size;

    while (1) {

        /*** requests handling ***/

        // TODO : blocking here if no proper NBD_CMD_DISC, bad threading design ?
        lwnbd_debug("Wait NBD_CMD ...\n");
        r = ctx->sync_recv_cb(ctx->sock, &request, sizeof(struct nbd_request), 0);
        if (r < sizeof(struct nbd_request)) {
            lwnbd_info("sizeof nbd_request waited %ld receveid %d\n", sizeof(struct nbd_request), r);
            return -1;
        }

        request.magic = ntohl(request.magic);
        if (request.magic != NBD_REQUEST_MAGIC) {
            lwnbd_info("wrong NBD_REQUEST_MAGIC\n");
            return -1;
        }

        request.flags = ntohs(request.flags);
        request.type = ntohs(request.type);
        request.offset = be64toh(request.offset);
        request.count = ntohl(request.count);

        reply.magic = htonl(NBD_SIMPLE_REPLY_MAGIC);
        reply.handle = request.handle;

        lwnbd_debug("%s\n", nbd_commands_to_string(request.type));

        switch (request.type) {

            case NBD_CMD_READ:

                lwnbd_debug("request off=0x" PRI_UINT64 ", size %lu\n", PRI_UINT64_C_Val(request.offset), request.count);
                if (request.offset + request.count > ctx->exportsize)
                    error = NBD_EIO;
                else {
                    error = NBD_SUCCESS;
                    sendflag = MSG_MORE;
                    bufblksz = CONFIG_NBD_BUFFER_LEN / blocksize;
                    blkremains = request.count / blocksize;
                    offset = request.offset / blocksize;
                    byteread = bufblksz * blocksize;
                }

                reply.error = ntohl(error);
                r = ctx->sync_send_cb(ctx->sock, &reply, sizeof(struct nbd_simple_reply),
                                      sendflag);
                if (r != sizeof(struct nbd_simple_reply)) {
                    lwnbd_info("send nbd_simple_reply failed\n");
                }

                while (sendflag) {

                    if (blkremains < bufblksz) {
                        bufblksz = blkremains;
                        byteread = bufblksz * blocksize;
                    }

                    if (blkremains <= bufblksz)
                        sendflag = 0;

                    // in zero-copy , we could skip lwnbd_pread
                    r = lwnbd_pread((lwnbd_context_t *)ctx, nbd_buffer, bufblksz, offset, 0);
                    // DEBUGLOG("offset=%d bufblksz=%d sendflag=%d\n", offset, bufblksz, sendflag);

                    if (r == 0) {
                        r = ctx->sync_send_cb(ctx->sock, nbd_buffer, byteread, sendflag);
                        if (r != byteread) {
                            lwnbd_info("NBD_CMD_READ : send failed r=%d byteread=%d\n", r, byteread);
                            break;
                        } else
                            lwnbd_debug("NBD_CMD_READ : send OK \n");
                        offset += bufblksz;
                        blkremains -= bufblksz;
                        retry = CONFIG_MAX_RETRIES;
                    } else if (retry) {
                        lwnbd_info("NBD_CMD_READ : NOK : %d\n", r);
                        retry--;
                        sendflag = 1;
                    } else {
                        lwnbd_info("NBD_CMD_READ : EIO\n");
                        return -1; // -EIO
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
                    bufblksz = CONFIG_NBD_BUFFER_LEN / blocksize;
                    blkremains = request.count / blocksize;
                    offset = request.offset / blocksize;
                    byteread = bufblksz * blocksize;
                }

                while (sendflag) {

                    if (blkremains < bufblksz) {
                        bufblksz = blkremains;
                        byteread = bufblksz * blocksize;
                    }

                    if (blkremains <= bufblksz)
                        sendflag = 0;

                    r = ctx->sync_recv_cb(ctx->sock, nbd_buffer, byteread, 0);

                    // in zero-copy , we could skip lwnbd_pwrite
                    if (r == byteread) {
                        r = lwnbd_pwrite((lwnbd_context_t *)ctx, nbd_buffer, bufblksz, offset, 0);
                        if (r != 0) {
                            error = NBD_EIO;
                            sendflag = 0;
                        }
                        offset += bufblksz;
                        blkremains -= bufblksz;
                        retry = CONFIG_MAX_RETRIES;
                    } else {
                        error = NBD_EOVERFLOW; // TODO
                        sendflag = 0;
                    }
                }

                reply.error = ntohl(error);
                r = ctx->sync_send_cb(ctx->sock, &reply, sizeof(struct nbd_simple_reply),
                                      0);
                break;

            case NBD_CMD_DISC:
                // There is no reply to an NBD_CMD_DISC.
                ctx->state = ABORT;
                return 0;

            case NBD_CMD_FLUSH:
                error = (lwnbd_flush((lwnbd_context_t *)ctx, 0) == 0) ? NBD_SUCCESS : NBD_EIO;
                reply.error = ntohl(error);
                r = ctx->sync_send_cb(ctx->sock, &reply, sizeof(struct nbd_simple_reply),
                                      0);
                break;

            case NBD_CMD_TRIM:
            case NBD_CMD_CACHE:
            case NBD_CMD_WRITE_ZEROES:
            case NBD_CMD_BLOCK_STATUS:
            default:
                /* The server SHOULD return NBD_EINVAL if it receives an unknown command. */
                reply.error = ntohl(NBD_EINVAL);
                r = ctx->sync_send_cb(ctx->sock, &reply, sizeof(struct nbd_simple_reply),
                                      0);
                break;
        }
    }
    return -1;
}
