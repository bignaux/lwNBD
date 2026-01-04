#include <lwnbd/piconbd.h>

const char *nbd_option_to_string(uint32_t f)
{
    static const char *const nbd_options[] = {
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

    return nbd_options[f];
}

int protocol_handshake(lwnbd_context_nbd_t *ctx, struct nbd_client *c)
{
    register int size;
    uint32_t cflags;
    struct nbd_new_option new_opt;
    struct nbd_fixed_new_option_reply fixed_new_option_reply;
    struct nbd_new_handshake new_hs;
    uint8_t nbd_buffer[MAX_REQUEST_SIZE];

    new_hs.nbdmagic = htobe64(NBD_MAGIC);
    new_hs.version = htobe64(NBD_NEW_VERSION);
    new_hs.gflags = htons(ctx->gflags);

    size = ctx->sync_send_cb(c->sock, &new_hs, sizeof(struct nbd_new_handshake),
                             0);
    if (size < sizeof(struct nbd_new_handshake))
        return -1;

    size = ctx->sync_recv_cb(c->sock, &cflags, sizeof(cflags), 0);
    if (size < sizeof(cflags))
        return -1;
    cflags = htonl(cflags);

    lwnbd_debug("ctx flags %d, server flags %d\n", cflags, ctx->gflags);
    if (cflags != ctx->gflags) {
        lwnbd_info("Unsupported ctx flags %u\n", cflags);
        return -1;
    }

    while (1) {

        /*** options haggling ***/

        size = ctx->sync_recv_cb(c->sock, &new_opt, sizeof(struct nbd_new_option),
                                 0);
        if (size < sizeof(struct nbd_new_option))
            return -1;

        new_opt.version = be64toh(new_opt.version);
        if (new_opt.version != NBD_NEW_VERSION)
            return -1;

        new_opt.option = htonl(new_opt.option);
        lwnbd_debug("%s\n", nbd_option_to_string(new_opt.option));

        new_opt.optlen = htonl(new_opt.optlen);

        if (new_opt.optlen > MAX_REQUEST_SIZE) {
            lwnbd_debug("ctx option data too long (%u)", new_opt.optlen);
            return -1;
        }

        if (new_opt.optlen > 0) {
            size = ctx->sync_recv_cb(c->sock, nbd_buffer, new_opt.optlen, 0);
            if (size < new_opt.optlen)
                return -1;
            nbd_buffer[new_opt.optlen] = '\0';
            lwnbd_debug("ctx option: sz=%d %s.\n", new_opt.optlen, nbd_buffer);
        }

        switch (new_opt.option) {

            case NBD_OPT_EXPORT_NAME: {
                struct nbd_export_name_option_reply handshake_finish;

                if (new_opt.optlen > 0) {
                    c->ctx = lwnbd_get_context((char *)nbd_buffer);
                }

                /* proto.md: If the server is unwilling to allow the export, it MUST terminate the session. */
                if (c->ctx == NULL) {
                    lwnbd_info("find nothing to export.\n");
                    return -1;
                }

                memset(&handshake_finish, 0, sizeof handshake_finish);
                handshake_finish.exportsize = htobe64(ctx->exportsize);

                /* could be hardened, here we trust client */
                if (ctx->readonly)
                    handshake_finish.eflags = htons(ctx->eflags | NBD_FLAG_READ_ONLY);
                else
                    handshake_finish.eflags = htons(ctx->eflags);

                // NBD_FLAG_C_NO_ZEROES not defined by nbd-protocol.h, another useless term from proto.md
                size = ctx->sync_send_cb(ctx->sock, &handshake_finish,
                                         (cflags & NBD_FLAG_NO_ZEROES) ? offsetof(struct nbd_export_name_option_reply, zeroes) : sizeof handshake_finish, 0);
                if (size < ((cflags & NBD_FLAG_NO_ZEROES) ? offsetof(struct nbd_export_name_option_reply, zeroes) : sizeof handshake_finish))
                    return -1;

                ctx->state = TRANSMISSION;
                return 0;
            }

            case NBD_OPT_ABORT:
                fixed_new_option_reply.magic = htobe64(NBD_REP_MAGIC);
                fixed_new_option_reply.option = htonl(new_opt.option);
                fixed_new_option_reply.reply = htonl(NBD_REP_ACK);
                fixed_new_option_reply.replylen = 0;
                size = ctx->sync_send_cb(ctx->sock, &fixed_new_option_reply,
                                         sizeof(struct nbd_fixed_new_option_reply), 0);
                if (size < sizeof(struct nbd_fixed_new_option_reply))
                    return -1;

                ctx->state = ABORT;
                return 0;

            case NBD_OPT_LIST: {

                fixed_new_option_reply.magic = htobe64(NBD_REP_MAGIC);
                fixed_new_option_reply.option = htonl(new_opt.option);
                fixed_new_option_reply.reply = htonl(NBD_REP_SERVER);

                for (size_t i = 0; i < lwnbd_contexts_count(); i++) {
                    lwnbd_context_t *context = lwnbd_get_context_i(i);
                    size_t name_len = strlen(context->name);
                    size_t desc_len = strlen(context->description);
                    uint32_t len;

                    lwnbd_debug("%s\n", (context->name));

                    len = htonl(name_len);
                    fixed_new_option_reply.replylen = htonl(
                        name_len + sizeof(len) + desc_len);

                    size = ctx->sync_send_cb(ctx->sock, &fixed_new_option_reply,
                                             sizeof(struct nbd_fixed_new_option_reply), MSG_MORE);
                    if (size < (sizeof(struct nbd_fixed_new_option_reply)))
                        return -1;
                    size = ctx->sync_send_cb(ctx->sock, &len, sizeof len, MSG_MORE);
                    if (size < (sizeof len))
                        return -1;
                    size = ctx->sync_send_cb(ctx->sock, context->name, name_len,
                                             MSG_MORE);
                    if (size < name_len)
                        return -1;
                    size = ctx->sync_send_cb(ctx->sock, context->description, desc_len,
                                             MSG_MORE);
                    if (size < desc_len)
                        return -1;
                }
                fixed_new_option_reply.reply = htonl(NBD_REP_ACK);
                fixed_new_option_reply.replylen = 0;
                size = ctx->sync_send_cb(ctx->sock, &fixed_new_option_reply,
                                         sizeof(struct nbd_fixed_new_option_reply), 0);
                if (size < sizeof(struct nbd_fixed_new_option_reply))
                    return -1;
                break;
            }

            case NBD_OPT_INFO:
            case NBD_OPT_GO:
            case NBD_OPT_STRUCTURED_REPLY:
            case NBD_OPT_LIST_META_CONTEXT:
            case NBD_OPT_SET_META_CONTEXT:
            default:
                // TODO: test
                fixed_new_option_reply.magic = htobe64(NBD_REP_MAGIC);
                fixed_new_option_reply.option = htonl(new_opt.option);
                fixed_new_option_reply.reply = htonl(NBD_REP_ERR_UNSUP);
                fixed_new_option_reply.replylen = 0;
                size = ctx->sync_send_cb(ctx->sock, &fixed_new_option_reply,
                                         sizeof(struct nbd_fixed_new_option_reply), 0);
                if (size < sizeof(struct nbd_fixed_new_option_reply))
                    return -1;
                break;
        }
    }
}
