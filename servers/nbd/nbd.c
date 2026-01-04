#include <errno.h>
#include <lwnbd/nbd.h>
#include <lwnbd/tcp.h> // need tcp_recv_block() until we get async complete on nbd
#include <lwnbd/lwnbd-plugin.h>
#include <lwnbd/nbd-protocol.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

typedef enum {
    HANDLE_FREE,
    HANDLE_CREATED,
    //	HANDLE_INUSE,
} handle_state_t;

#define NBD_SERVER_MAX_DEVICES 3

/* specific plugin private data */
static struct lwnbd_context_nbd_t handles[NBD_SERVER_MAX_DEVICES];
static int handle_in_use[NBD_SERVER_MAX_DEVICES];

void nbdify_context(lwnbd_context_nbd_t *ctx)
{
    uint16_t eflags = NBD_FLAG_HAS_FLAGS;
    uint32_t minimum, preferred, maximum;
    struct lwnbd_plugin_t *p = ctx->p;


    if (!p->pwrite)
        eflags |= NBD_FLAG_READ_ONLY;

    if (p->flush)
        eflags |= NBD_FLAG_SEND_FLUSH;

    ctx->eflags = eflags;

    if (p->block_size) {
        p->block_size(ctx->handle, &minimum, &preferred, &maximum);
    } else
        minimum = preferred = maximum = 512;

    ctx->minimum_block_size = minimum;
    ctx->preferred_block_size = preferred;
    ctx->maximum_block_size = maximum;

    //    ctx->sock = *(int *)data;
    //    ctx->nbd_buffer = malloc(NBD_BUFFER_LEN); // __attribute__((aligned(16)));
}

// lwnbd_context_t *lwnbd_get_defaultexport()
//{
//     return defaultcontexts;
// }
//
// lwnbd_context_t *lwnbd_set_defaultexport(const char *name)
//{
//     defaultcontexts = lwnbd_get_context_string(name);
//     return defaultcontexts;
// }

// get initialized socket
int nbd_server_create(struct nbd_server *server)
{
    struct sockaddr_in peer;
    register int r;
    int sock;

    peer.sin_family = AF_INET;
    peer.sin_port = htons(10809);
    peer.sin_addr.s_addr = htonl(INADDR_ANY);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        lwnbd_debug("socket failed\n");
        goto error_trap;
    }

    //        if (setsockopt(server->tcp_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    //        {
    //        	DEBUGLOG("setsockopt(SO_REUSEADDR) failed\n");
    //            goto error_trap;
    //        }

    r = bind(sock, (struct sockaddr *)&peer, sizeof(peer));
    if (r < 0) {
        lwnbd_debug("bind failed\n");
        goto error_trap;
    }

    r = listen(sock, MAX_NUM_CLIENTS);
    if (r < 0) {
        lwnbd_debug("listen failed\n");
        goto error_trap;
    }

    lwnbd_info("nbd server created.\n");
    return 0;

error_trap:
    lwnbd_info("failed to init nbd server.\n");
    close(sock);
    return -1;
}

/* Transitional workaround
 *
 *
 * actually, void *client is just the socket client
 *
 * */

static int nbd_synchronous_client_thread_cb(void *handle, const void *client)
{
    register int r = 0;
    lwnbd_context_nbd_t *ctx = handle;

    struct nbd_client temp_client;
    struct nbd_client *c = &temp_client;

    c->sock = *(int *)client;

    //    c.nbd_buffer = malloc(NBD_BUFFER_LEN); // __attribute__((aligned(16)));

    if (!s->preinit) {
        c->ctx = lwnbd_new_context(); /* create ctx for handshake */
        if (!c->ctx)
            return -1;
        c->state = HANDSHAKE;
    } else {
        c->ctx = lwnbd_get_defaultexport();
        if (c->ctx != NULL) {
            c->state = TRANSMISSION;
            lwnbd_debug("export context %s.\n", c->ctx->name);
        } else {
            lwnbd_info("You need to provide a default export to use preinit.\n");
            return -1;
        }
    }

    //    nbdify_context(ctx);
    lwnbd_debug("coucou = %p.\n", handle);

    while (r == 0) {
        lwnbd_debug("coucou = %d.\n", ctx->state);
        switch (c->state) {
            case HANDSHAKE:
                r = protocol_handshake(ctx, c);
                if (r == -1) {
                    lwnbd_info("an error occured during negotiation phase.\n");
                }
                break;
            case TRANSMISSION:
                r = transmission_phase(ctx, c);
                if (r == -1)
                    lwnbd_info("an error occured during transmission phase.\n");
                break;
            case ABORT:
                r = -1;
                break;
            default:
                break;
        }
    }
    return r;
}

void nbd_server_set_preinit(struct nbd_server *h, int preinit)
{
    h->preinit = preinit;
}

int nbd_server_get_preinit(struct nbd_server *h)
{
    return h->preinit;
}

uint32_t nbd_server_get_gflags(struct nbd_server *h)
{
    return h->gflags;
}

static int nbd_config(void *handle, const char *key, const char *value)
{
    struct nbd_server *s = handle;
    if (strcmp(key, "readonly") == 0) {
        s->readonly = 1;
    } else if (strcmp(key, "preinit") == 0) {
        s->preinit = 1;
    } else {
        printf("config key %s unknown.\n", key);
        return -1;
    }

    //	else if (strcmp(key, "retry") == 0) {
    //		if (nbdkit_parse_unsigned("retry", value, &retry) == -1)
    //			return -1;
    //	}
    return 0;
}

static int nbd_ctor(void *handle, const void *settings)
{
    struct lwnbd_context_nbd_t *h = handle;
    static const struct nbdsettings default_nbd_server = {
        .port = NBDDEFAULTPORT,
        .max_retry = CONFIG_MAX_RETRIES,
        .gflags = (NBD_FLAG_FIXED_NEWSTYLE | NBD_FLAG_NO_ZEROES),
        .preinit = 0,
        .readonly = 0,
    };

    //    memcpy(h, &default_nbd_server, sizeof(struct nbdsettings));
    return 0;
}

static void *nbd_new(void)
{
    uint32_t i;

    for (i = 0; i < NBD_SERVER_MAX_DEVICES; i++) {
        if (handle_in_use[i] == HANDLE_FREE) {
            handle_in_use[i] = HANDLE_CREATED;
            break;
        }
    }

    return &handles[i];
}
