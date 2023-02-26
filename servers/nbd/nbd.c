#include "nbd.h"
#include <string.h>

#define NAME nbd

/* for now, only one server */
static struct nbd_server nbd_servers = {
    .port = 10809,
    .max_retry = MAX_RETRIES,
    .gflags = (NBD_FLAG_FIXED_NEWSTYLE | NBD_FLAG_NO_ZEROES),
    .preinit = 0,
    .readonly = 0,
};

void nbd_server_set_preinit(void *handle, int preinit)
{
    struct nbd_server *h = handle;
    h->preinit = preinit;
}

int nbd_server_get_preinit(void *handle)
{
    struct nbd_server *h = handle;
    return h->preinit;
}

uint32_t nbd_server_get_gflags(void *handle)
{
    struct nbd_server *h = handle;
    return h->gflags;
}

char *nbd_server_get_defaultexport(void *handle)
{
    struct nbd_server *h = handle;
    return h->defaultexport;
}

uint16_t nbd_server_get_port(void *handle)
{
    struct nbd_server *h = handle;
    return h->port;
}

int nbd_config(void *handle, const char *key, const char *value)
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

void *nbd_new(void)
{
    return &nbd_servers;
}

static struct lwnbd_server server = {
    .name = "nbd",
    .new = nbd_new,
    .start = nbd_start,
    .config = nbd_config,
};

NBDKIT_REGISTER_SERVER(server)
