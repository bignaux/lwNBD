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
    /* move in key/value store */
    uint16_t port;
    uint16_t max_retry;
    uint32_t gflags; // nbd proto global flag
    char defaultexport[32];
    uint8_t readonly; /* all exports would be read only */
    /*
     * Some nbd client has options to use a preinitialized connection, and to specify the device size
     * and skip protocol handshake. (nbd-client -preinit -size <bytes> )
     */
    uint8_t preinit;
};

/* tcp.c */
uint32_t nbd_recv(int s, void *mem, size_t len, int flags);
int tcp_loop(void *handle);

/* nbd.c */
uint32_t nbd_server_get_gflags(struct nbd_server *h);
char *nbd_server_get_defaultexport(struct nbd_server *h);
uint16_t nbd_server_get_port(struct nbd_server *h);
int nbd_server_get_preinit(struct nbd_server *h);

/* protocol-handshake.c */
err_t protocol_handshake(struct nbd_server *server, const int client_socket, struct lwnbd_context **ctx);

/* protocol.c */
err_t transmission_phase(const int client_socket, struct lwnbd_context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* NBD_SERVER_H */
