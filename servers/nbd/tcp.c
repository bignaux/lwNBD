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
 * https://lwip.fandom.com/wiki/Receiving_data_with_LWIP
 * pread() eq for nbd server.
 */
int32_t nbd_recv(int s, void *mem, size_t len, int flags)
{
    uint32_t left = len;
    uint32_t totalRead = 0;

    do {
        ssize_t bytesRead = recv(s, (void *)((uint8_t *)mem + totalRead), left, flags);
        DEBUGLOG("bytesRead = %u\n", bytesRead);
        if (bytesRead <= 0)
            return bytesRead;

        left -= bytesRead;
        totalRead += bytesRead;

    } while (left);
    return totalRead;
}

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

int client_init(struct nbd_server *s, struct nbd_client *c)
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
