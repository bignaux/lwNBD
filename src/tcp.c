
#include <errno.h>
#include <lwnbd/lwnbd-server.h>
#include <sys/socket.h>
#include <unistd.h>

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


/*
 * https://lwip.fandom.com/wiki/Receiving_data_with_LWIP
 * pread() eq for nbd server.
 */
ssize_t tcp_recv_block(int s, void *mem, size_t len, int flags)
{
    ssize_t left = len;
    ssize_t totalRead = 0;

    do {
        ssize_t bytesRead = recv(s, (void *)((uint8_t *)mem + totalRead), left, flags);
        lwnbd_debug("bytesRead = %zd/%lu\n", bytesRead, left); // %u
        if (bytesRead <= 0) {
            //            perror("server->sync_recv_cb:");
            return bytesRead;
        }
        left -= bytesRead;
        totalRead += bytesRead;

    } while (left);
    return totalRead;
}

int tcp_close(int socket)
{
    lwnbd_debug("close server socket\n");
    return close(socket);
}

void listener(int socket)
{
    struct sockaddr peer;
    socklen_t addrlen = sizeof(struct sockaddr);
    int sock;

    while (1) {
        // blocking
        sock = accept(socket, &peer, &addrlen);
        lwnbd_info("a client connected.\n");
        // the abstracted blocking loop
        //        lwnbd_server_run(handle, &sock);

        close(sock);
        lwnbd_info("a client disconnected.\n\n\n");
    }
}
