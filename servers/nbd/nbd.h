#ifndef NBD_SERVER_H
#define NBD_SERVER_H

#include <lwnbd.h>
#include <lwnbd-server.h>
#include <nbd-protocol.h>
#include "config.h"

#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <endian.h>
#include <stddef.h>
#include <string.h>

#ifdef __linux__
// TODO : manage endianess
#define htonll(x) htobe64(x)
#define ntohll(x) be64toh(x)
#endif /* ifdef __linux__ */

#ifndef err_t
typedef signed char err_t;
#endif /* __linux__ */

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

    /* private */
    int socket;
};

/* see https://rwmj.wordpress.com/2019/05/19/nbds-state-machine/ */
enum client_states {
    HANDSHAKE,
    TRANSMISSION,
    ABORT,
};

struct nbd_client
{
    int sock;
    int state;
    lwnbd_context_t *ctx;
    uint8_t *nbd_buffer;
};

/* nbd.c */
uint32_t nbd_server_get_gflags(struct nbd_server *h);
char *nbd_server_get_defaultexport(struct nbd_server *h);
uint16_t nbd_server_get_port(struct nbd_server *h);
int nbd_server_get_preinit(struct nbd_server *h);


int32_t nbd_recv(int s, void *mem, size_t len, int flags);

/* protocol-handshake.c */
err_t protocol_handshake(struct nbd_server *server, struct nbd_client *client);

/* protocol.c */
err_t transmission_phase(struct nbd_client *client);


#ifdef __cplusplus
}
#endif

#endif /* NBD_SERVER_H */
