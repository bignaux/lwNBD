#include "nbd.h"
#include <errno.h>

#ifdef __PS2SDK_SYS_SOCKET_H__
#define close(x) lwip_close(x)
#endif

/*
 * This could later be independent of protocol implementation
 * to be shared between all TCP servers.
 *
 */

/*
 * IOP :
 *
 * The IRX loader can only handle max alignment of 16 bytes.
 * Future updates to the toolchain will make this a hard requirement.
 *
 * TODO : check alignment and move per-plateform if specific define ALIGMENT
 */
// uint8_t nbd_buffer[NBD_BUFFER_LEN] __attribute__((aligned(16)));


int nbd_close(struct nbd_server *server)
{
    DEBUGLOG("close server socket\n");
    return close(server->socket);
}

int nbd_server_create(struct nbd_server *server)
{
    struct sockaddr_in peer;
    register err_t r;


    peer.sin_family = AF_INET;
    peer.sin_port = htons(nbd_server_get_port(server));
    peer.sin_addr.s_addr = htonl(INADDR_ANY);

    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket < 0) {
        DEBUGLOG("socket failed\n");
        goto error_trap;
    }

    //        if (setsockopt(server->tcp_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    //        {
    //        	DEBUGLOG("setsockopt(SO_REUSEADDR) failed\n");
    //            goto error_trap;
    //        }

    r = bind(server->socket, (struct sockaddr *)&peer, sizeof(peer));
    if (r < 0) {
        DEBUGLOG("bind failed\n");
        goto error_trap;
    }

    r = listen(server->socket, MAX_NUM_SERVERS);
    if (r < 0) {
        DEBUGLOG("listen failed\n");
        goto error_trap;
    }

    LOG("nbd server created.\n");
    return 0;

error_trap:
    LOG("failed to init nbd server.\n");
    close(server->socket);
    return -1;
}

void listener(struct nbd_server *s)
{
    register err_t r;
    struct nbd_client c;
    struct sockaddr peer;
    socklen_t addrlen = sizeof(struct sockaddr);

    while (1) {

        // blocking
        c.sock = accept(s->socket, &peer, &addrlen);
        r = client_init(s, &c);
        if (r)
            break;

        LOG("a client connected.\n");

        while (r == 0) {
            switch (c.state) {
                case HANDSHAKE:
                    r = protocol_handshake(s, &c);
                    if (r == -1) {
                        LOG("an error occured during negotiation_phase phase.\n");
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
        close(c.sock);
        LOG("a client disconnected.\n\n\n");
    }
}

/*
static int nbd_start(void *handle)
{
    struct nbd_server *h = handle;
    nbd_server_create(h);
    listener(h);
    return 0;
}

static int nbd_stop(void *handle)
{
    struct nbd_server *h = handle;
    return nbd_close(h);
}
*/
