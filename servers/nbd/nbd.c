#include "nbd.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h> // temporary

#define NAME                   nbd
#define NBD_SERVER_MAX_DEVICES 1 /* TODO handles are bugged, fix it */

typedef enum {
    HANDLE_FREE,
    HANDLE_CREATED,
    //	HANDLE_INUSE,
} handle_state_t;

/* specific plugin private data */
static struct nbd_server handles[NBD_SERVER_MAX_DEVICES];
static int handle_in_use[NBD_SERVER_MAX_DEVICES];

/*
 * https://lwip.fandom.com/wiki/Receiving_data_with_LWIP
 * pread() eq for nbd server.
 */
int32_t nbd_recv(int s, void *mem, size_t len, int flags)
{
    uint32_t left = len;
    uint32_t totalRead = 0;

    do {
        ssize_t bytesRead = recv(s, (void *)((uint8_t *)mem + totalRead), left, flags);
        DEBUGLOG("bytesRead = %zd/%u\n", bytesRead, left); // %u
        if (bytesRead <= 0) {
            //            perror("nbd_recv:");
            return bytesRead;
        }
        left -= bytesRead;
        totalRead += bytesRead;

    } while (left);
    return totalRead;
}

static int client_init(struct nbd_server *s, struct nbd_client *c)
{
    if (c->sock < 0)
        return -1;

    if (!s->preinit) {
        c->state = HANDSHAKE;
    } else {
        c->ctx = lwnbd_get_context(s->defaultexport);
        if (c->ctx != NULL) {
            c->state = TRANSMISSION;
            DEBUGLOG("export context %s.\n", c->ctx->name);
        } else {
            LOG("You need to provide a default export to use preinit.\n");
            return -1;
        }
    }
    return 0;
}

/* Transitional workaround */
static int nbd_synchronous_client_thread_cb(void *handle, void *data)
{
    struct nbd_server *s = handle;
    struct nbd_client c;

    c.sock = *(int *)data;
    c.nbd_buffer = malloc(NBD_BUFFER_LEN); // __attribute__((aligned(16)));

    register err_t r = client_init(s, &c);
    if (r)
        return -1;

    while (r == 0) {
        switch (c.state) {
            case HANDSHAKE:
                r = protocol_handshake(s, &c);
                if (r == -1) {
                    LOG("an error occured during negotiation phase.\n");
                }
                break;
            case TRANSMISSION:
                r = transmission_phase(&c);
                if (r == -1)
                    LOG("an error occured during transmission phase.\n");
                break;
            case ABORT:
                r = -1;
                break;
            default:
                break;
        }
    }
    free(c.nbd_buffer);
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

char *nbd_server_get_defaultexport(struct nbd_server *h)
{
    return h->defaultexport;
}

uint16_t nbd_server_get_port(struct nbd_server *h)
{
    return h->port;
}

static int nbd_config(void *handle, const char *key, const char *value)
{
    struct nbd_server *s = handle;
    if (strcmp(key, "default-export") == 0) {
        strncpy(s->defaultexport, value, 31);
    } else if (strcmp(key, "readonly") == 0) {
        s->readonly = 1;
    } else if (strcmp(key, "preinit") == 0) {
        s->preinit = 1;
    } else {
        printf("config key %s unknown.\n", key);
        return -1;
    }
    //	else if (strcmp(key, "port") == 0)
    //		s.port = value;

    //	else if (strcmp(key, "retry") == 0) {
    //		if (nbdkit_parse_unsigned("retry", value, &retry) == -1)
    //			return -1;
    //	}
    return 0;
}

static int nbd_ctor(void *handle, const void *pconfig)
{
    struct nbd_server *h = handle;
    memcpy(h, pconfig, sizeof(struct nbd_server));
    return 0;
}

/* to have some default setting ? */
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

static struct lwnbd_server server = {
    .name = "nbd",
    .new = nbd_new,
    .config = nbd_config,
    .ctor = nbd_ctor,
    .run = nbd_synchronous_client_thread_cb,
};

NBDKIT_REGISTER_SERVER(server)
