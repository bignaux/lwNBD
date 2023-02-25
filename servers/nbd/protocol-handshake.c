#include <stddef.h>
#include <string.h>
#include "nbd.h"


/*
 * nbdkit alloc this size ...
 * #define MAX_REQUEST_SIZE (64 * 1024 * 1024)
 */
#define MAX_REQUEST_SIZE NBD_BUFFER_LEN

/* ... so we shared the big buffer */
extern uint8_t nbd_buffer[];


err_t protocol_handshake(struct nbd_server *server, const int client_socket, struct lwnbd_context **ctx)
{
    register int size;
    uint32_t cflags;
    struct nbd_new_option new_opt;
    struct nbd_fixed_new_option_reply fixed_new_option_reply;
    struct nbd_new_handshake new_hs;

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

    DEBUGLOG("client flags %d, %d server flags %d\n", cflags, server->port, server->gflags);
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

        if (new_opt.optlen > MAX_REQUEST_SIZE) {
            DEBUGLOG("client option data too long (%u)", new_opt.optlen);
            return -1;
        }

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
                    struct lwnbd_context *context = lwnbd_get_context_i(i);
                    size_t name_len = strlen(context->name);
                    size_t desc_len = strlen(context->description);
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
