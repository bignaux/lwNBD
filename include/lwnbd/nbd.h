#ifndef NBD_SERVER_H
#define NBD_SERVER_H

#include <lwnbd/lwnbd.h> /* context stuff */
#include <lwnbd/lwnbd-server.h>
#include <lwnbd/piconbd.h>

#ifdef __cplusplus
extern "C" {
#endif


#define NBDDEFAULTPORT 10809
/* nbd.c */
uint32_t nbd_server_get_gflags(struct nbd_server *h);
// char *nbd_server_get_defaultexport(struct nbd_server *h);
uint16_t nbd_server_get_port(struct nbd_server *h);
int nbd_server_get_preinit(struct nbd_server *h);
int nbd_server_create(struct nbd_server *server);

#ifdef __cplusplus
}
#endif

#endif /* NBD_SERVER_H */
