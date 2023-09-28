#include "sifrpc.h"
#include <lwnbd.h>
#include <lwnbd-server.h>
#include <thbase.h>

#define NAME sifrpc

struct sifrpc_server
{
    /* user config */
    int sid;
    SifRpcFunc_t sifrpc_handler;

    /* private */
    int RpcThreadID;
    SifRpcDataQueue_t SifQueueData;
    SifRpcServerData_t SifServerData;
    unsigned char SifServerRxBuffer[64];
    //    unsigned char SifServerTxBuffer[32];
};

/* for now, only one server per apps */
static struct sifrpc_server sifrpc_servers;

/* ctypes config */
static int sifrpc_ctor(void *handle, const void *pconfig)
{
    const struct sifrpc_handler *conf = pconfig;
    struct sifrpc_server *h = handle;

    h->sid = conf->sid;
    h->sifrpc_handler = conf->sifrpc_handler;
    return 0;
}


// void sceSifRegisterRpc(SifRpcServerData_t *sd, int sid, SifRpcFunc_t func, void *buf,
//     SifRpcFunc_t cfunc, void *cbuf, SifRpcDataQueue_t *qd);
//
//
// static int *sifrpc_service_handler(int fno, void *buffer, int length)
//{
//	if(!handler[fno])
//		return NULL;
//
//	/* this pattern would force to have same prototype, as sceSifAddCmdHandler() */
//	return handler[fno](buffer, length);
// }


/*  */
static void sifrpc_thread(void *handle)
{
    struct sifrpc_server *h = handle;
    printf("lwNBD service.(27/03/23)\n");               // would be configurable
    sceSifInitRpc(0);                                   // really usefull ?
    sceSifSetRpcQueue(&h->SifQueueData, GetThreadId()); // h->RpcThreadID
    sceSifRegisterRpc(&h->SifServerData, h->sid, h->sifrpc_handler, &h->SifServerRxBuffer, NULL, NULL, &h->SifQueueData);
    sceSifRpcLoop(&h->SifQueueData);
}

static int sifrpc_start(void *handle)
{
    int result;
    iop_thread_t thread;
    struct sifrpc_server *h = handle;

    thread.attr = TH_C;
    //    thread.option = h->sid;
    thread.thread = &sifrpc_thread;
    thread.priority = 0x20;
    thread.stacksize = 0x800;

    h->RpcThreadID = CreateThread(&thread);
    if (h->RpcThreadID > 0) {
        StartThread(h->RpcThreadID, handle);
        result = 0;
    } else {
        result = h->RpcThreadID;
    }

    return result;
}

static int sifrpc_stop(void *handle)
{
    struct sifrpc_server *h = handle;
    TerminateThread(h->RpcThreadID);
    DeleteThread(h->RpcThreadID);
    return 0;
}

static void *sifrpc_new(void)
{
    return &sifrpc_servers;
}

static struct lwnbd_server server = {
    .name = "sifrpc",
    .new = sifrpc_new,
    .start = sifrpc_start,
    .stop = sifrpc_stop,
    .ctor = sifrpc_ctor,
};

NBDKIT_REGISTER_SERVER(server)
