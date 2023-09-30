//#include "../../servers/nbd/nbd.h"

#include <errno.h>
#include <lwnbd-server.h>
#include <lwnbd/nbd.h>
#include <sys/socket.h>

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


int nbd_close(int socket)
{
    DEBUGLOG("close server socket\n");
    return close(socket);
}

// get initialized socket
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

void listener(lwnbd_server_t const *handle)
{
    struct sockaddr peer;
    socklen_t addrlen = sizeof(struct sockaddr);
    int sock;
    struct nbd_server *s = handle; // poor man ...

    while (1) {
        // blocking
        sock = accept(s->socket, &peer, &addrlen);
        LOG("a client connected.\n");
        // the abstracted blocking loop
        lwnbd_server_run(handle, &sock);
        close(sock);
        LOG("a client disconnected.\n\n\n");
    }
}
