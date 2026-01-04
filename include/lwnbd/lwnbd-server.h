/*
 * Servers
 *
 */

#ifndef LWNBD_SERVER_H
#define LWNBD_SERVER_H

#include <lwnbd/config.h>
#include <lwnbd/lwnbd-common.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SRV_FREE,
    SRV_STOPPED,
    SRV_STARTED,
} server_state_t;

#ifdef CONFIG_SIF_SERVER
//#include <lwnbd/sifrpc.h>
#include <sifcmd.h>
#include <sifman.h>
struct sifrpc_handler
{
    int sid;
    SifRpcFunc_t sifrpc_handler;
};
struct sifrpc_server
{
    /* user config */
    int sid;
    SifRpcFunc_t sifrpc_handler;

    /* private */
    int RpcThreadID;
    SifRpcDataQueue_t SifQueueData __attribute__((aligned(16)));
    ;
    SifRpcServerData_t SifServerData __attribute__((aligned(16)));
    ;
    unsigned char SifServerRxBuffer[64] __attribute__((__aligned__(4)));
    ;
    //    unsigned char SifServerTxBuffer[32];
};
extern struct lwnbd_server *sifrpc_server_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LWNBD_SERVER_H */
