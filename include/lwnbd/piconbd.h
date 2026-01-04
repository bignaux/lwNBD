/*
 * piconbd.h
 *
 */

#ifndef SERVERS_NBD_PICONBD_H_
#define SERVERS_NBD_PICONBD_H_

#include <endian.h>
#include <lwnbd/lwnbd.h> // lwnbd_context_t
#include <lwnbd/nbd-protocol.h>
#include <netinet/in.h> // sockaddr_in
#include <stddef.h>     // offsetof()

/*
 * nbdkit alloc this size ...
 * #define MAX_REQUEST_SIZE (64 * 1024 * 1024)
 */
#define MAX_REQUEST_SIZE NBD_BUFFER_LEN

#ifndef MSG_MORE
#define MSG_MORE 0x8000 /* latest thing missing to skip tcp */
#endif

struct lwnbd_context_nbd_s
{
    uint16_t eflags;
    uint32_t minimum_block_size;
    uint32_t preferred_block_size;
    uint32_t maximum_block_size;

    uint16_t port;
    uint16_t max_retry;
    uint32_t gflags; // nbd proto global flag
    uint8_t readonly;
    char defaultexport[32];

    /*
     * Some nbd client has options to use a preinitialized connection, and to specify the device size
     * and skip protocol handshake. (nbd-client -preinit -size <bytes> )
     */
    uint8_t preinit;

    int sock;
    int state;
};

struct nbd_client
{
    int sock;
    int state;
    lwnbd_context_t *ctx;
    uint8_t *nbd_buffer;
};

typedef struct lwnbd_context_nbd_s lwnbd_context_nbd_t;

/* see https://rwmj.wordpress.com/2019/05/19/nbds-state-machine/ */
enum client_states {
    HANDSHAKE,
    TRANSMISSION,
    ABORT,
};

/* protocol-handshake.c */
int protocol_handshake(lwnbd_context_nbd_t *ctx, struct nbd_client *c);

/* protocol.c */
int transmission_phase(lwnbd_context_nbd_t *ctx, struct nbd_client *c);

#endif /* SERVERS_NBD_PICONBD_H_ */
