/*
 * tcp.h
 *
 */

#ifndef INCLUDE_LWNBD_TCP_H_
#define INCLUDE_LWNBD_TCP_H_

// void listener(struct nbd_server *s);

int tcp_close(int socket);
ssize_t tcp_recv_block(int s, void *mem, size_t len, int flags);
void listener(int socket);

#endif /* INCLUDE_LWNBD_TCP_H_ */
