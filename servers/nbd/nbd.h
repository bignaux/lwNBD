#ifndef NBD_SERVER_H
#define NBD_SERVER_H

#include <lwnbd.h>
#include <lwnbd-server.h>
#include <nbd-protocol.h>
#include "config.h"

#ifdef __linux__
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <endian.h>
#include <unistd.h>

// TODO : manage endianess
#define htonll(x) htobe64(x)
#define ntohll(x) be64toh(x)
#endif /* ifdef __linux__ */

#ifndef err_t
typedef signed char err_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct nbd_server
{
    //	struct server_ops const *vptr;
    /* move in key/value store */
    uint16_t port;
    uint16_t max_retry;
    uint32_t gflags; // nbd proto global flag
    char defaultexport[32];
    int readonly; /* TODO: all exports would be readonly */
};

/* tcp.c */
uint32_t nbd_recv(int s, void *mem, size_t len, int flags);
int nbd_start(void *handle);

/* nbd.c */
uint32_t nbd_server_get_gflags(void *handle);
char *nbd_server_get_defaultexport(void *handle);
uint16_t nbd_server_get_port(void *handle);

/* nbd_protocol.c */
err_t negotiation_phase(struct nbd_server *server, const int client_socket, struct lwnbd_context **ctx);
err_t transmission_phase(const int client_socket, struct lwnbd_context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* NBD_SERVER_H */
