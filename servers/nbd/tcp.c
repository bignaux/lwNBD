#include "nbd.h"

/*
 * This could later be independent of protocol implementation
 * to be shared between all TCP servers.
 *
 */


/*
 * https://lwip.fandom.com/wiki/Receiving_data_with_LWIP
 */
uint32_t nbd_recv(int s, void *mem, size_t len, int flags)
{
    ssize_t bytesRead;
    uint32_t left = len;
    uint32_t totalRead = 0;

    //        LWIP_DEBUGF(NBD_DEBUG | LWIP_DBG_STATE("nbd_recv(-, 0x%X, %d)\n", (int)mem, size);
    // dbgLOG("left = %u\n", left);
    do {
        bytesRead = recv(s, (void *)((uint8_t *)mem + totalRead), left, flags);
        //        DEBUGLOG("bytesRead = %u\n", bytesRead);
        if (bytesRead <= 0) // if (bytesRead == -1) failed for nbdfuse, seems it not send NBD_CMD_DISC
            break;

        left -= bytesRead;
        totalRead += bytesRead;

    } while (left);
    return totalRead;
}

int nbd_start(void *handle)
{
    int tcp_socket, client_socket = -1;
    struct sockaddr_in peer;
    socklen_t addrlen;
    register err_t r;
    struct lwnbd_context *nego_ctx = NULL;

    peer.sin_family = AF_INET;
    peer.sin_port = htons(nbd_server_get_port(handle));
    peer.sin_addr.s_addr = htonl(INADDR_ANY);

    while (1) {

        tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (tcp_socket < 0)
            goto error;

        r = bind(tcp_socket, (struct sockaddr *)&peer, sizeof(peer));
        if (r < 0)
            goto error;

        r = listen(tcp_socket, MAX_NUM_SERVERS);
        if (r < 0)
            goto error;

        while (1) {

            addrlen = sizeof(peer);

            // blocking
            client_socket = accept(tcp_socket, (struct sockaddr *)&peer,
                                   &addrlen);
            if (client_socket < 0)
                goto error;

            LOG("a client connected.\n");
            r = negotiation_phase(handle, client_socket, &nego_ctx);
            if (r == NBD_OPT_EXPORT_NAME) {
                // TODO : make other ctx available for other connection
                r = transmission_phase(client_socket, nego_ctx);
                if (r == -1)
                    LOG("an error occured during transmission phase.\n");
            } else if (r == -1) {
                LOG("an error occured during negotiation_phase phase.\n");
            }
            close(client_socket);
            LOG("a client disconnected.\n\n\n");
        }
    }
error:
    LOG("failed to init server.");
    close(tcp_socket);
    return 0;
}
