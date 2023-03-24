/***
 *     ___           ___       __       ________       ________      ________
 *    |\  \         |\  \     |\  \    |\   ___  \    |\   __  \    |\   ___ \
 *    \ \  \        \ \  \    \ \  \   \ \  \\ \  \   \ \  \|\ /_   \ \  \_|\ \
 *     \ \  \        \ \  \  __\ \  \   \ \  \\ \  \   \ \   __  \   \ \  \ \\ \
 *      \ \  \____    \ \  \|\__\_\  \   \ \  \\ \  \   \ \  \|\  \   \ \  \_\\ \
 *       \ \_______\   \ \____________\   \ \__\\ \__\   \ \_______\   \ \_______\
 *        \|_______|    \|____________|    \|__| \|__|    \|_______|    \|_______|
 *
 *
 *
 */

#include <lwnbd.h>
#include "irx_imports.h"
#include "ioplib.h"
#include <config.h>

IRX_ID(APP_NAME, 1, 1);
extern struct irx_export_table _exp_lwnbd;

static SifRpcDataQueue_t SifQueueData;
static SifRpcServerData_t SifServerData;
static int RpcThreadID, nbdThreadID;
static unsigned char SifServerRxBuffer[64];
static unsigned char SifServerTxBuffer[32];

static lwnbd_server_t nbdsrv;

enum LWNBD_SERVER_CMD {
    LWNBD_SERVER_CMD_START,
    LWNBD_SERVER_CMD_STOP,
};
static int sid = 0x2A39;

static int enable = 0;

struct lwnbd_config
{
    char defaultexport[32];
    uint8_t readonly;
};

static int config(struct lwnbd_config *config)
{


    lwnbd_server_config(nbdsrv, "default-export", config->defaultexport);
    if (config->readonly)
        lwnbd_server_config(nbdsrv, "readonly", NULL);


        //    lwnbd_server_config(nbdsrv, "preinit", NULL);
        // print twice doesn't give same result ...
        //    print_memorymap();
        //    print_memorymap();

#ifdef PLUGIN_ATAD
    lwnbd_plugin_t atadplg = lwnbd_plugin_init(atad_plugin_init);
    for (int i = 0; i < 2; i++) {
        lwnbd_plugin_new(atadplg, &i);
    }
#endif

#ifdef PLUGIN_MCMAN
    lwnbd_plugin_t mcmanplg = lwnbd_plugin_init(mcman_plugin_init);
    for (int i = 0; i < 2; i++) {
        lwnbd_plugin_new(mcmanplg, &i);
    }
#endif

#ifdef PLUGIN_MEMORY
    lwnbd_plugin_t memplg = lwnbd_plugin_init(memory_plugin_init);
    struct memory_config bios = {
        .base = 0x1FC00000,
        .size = 0x400000, // GetSizeFromDelay(SSBUSC_DEV_BOOTROM)
        .name = "bios",
        .desc = "BIOS (rom0)",
    };

    struct memory_config iopram = {
        .base = 0,
        .size = QueryMemSize(),
        .name = "ram",
        .desc = "IOP main RAM",
    };

    lwnbd_plugin_new(memplg, &iopram);
    lwnbd_plugin_new(memplg, &bios);
    //    lwnbd_plugin_new(memplg, &dvdrom);
#endif

    //#ifdef PLUGIN_TTY
    //    lwnbd_plugin_t ttyplg = lwnbd_plugin_init(tty_plugin_init);
    //    lwnbd_plugin_new(ttyplg, NULL);
    //#endif

#ifdef PLUGIN_BDM
    lwnbd_plugin_t bdmplg = lwnbd_plugin_init(bdm_plugin_init);
    lwnbd_plugin_new(bdmplg, NULL);
#endif

    return 0;
}

static void *SifRpc_handler(int fno, void *buffer, int nbytes)
{
    iop_thread_t nbd_thread;

    switch (fno) {
        case LWNBD_SERVER_CMD_START:

            LOG("LWNBD_SERVER_CMD_START.\n");
            nbdsrv = lwnbd_server_init(nbd_server_init);
            if (nbdsrv < 0) {
                LOG("LWNBD_SERVER_CMD_START failed init server.\n");
                break;
            }

            if (enable == 0) {
                config((struct lwnbd_config *)buffer);
                enable = 1;
            }
            LOG("LWNBD_SERVER_CMD_START 2.\n");

            nbd_thread.attr = TH_C;
            nbd_thread.option = 0;
            nbd_thread.thread = (void *)lwnbd_server_start;
            nbd_thread.stacksize = 0x800;
            nbd_thread.priority = 0x10;

            nbdThreadID = CreateThread(&nbd_thread);
            if (nbdThreadID > 0) {
                LOG("LWNBD_SERVER_CMD_START StartThread.\n");
                *(int *)SifServerTxBuffer = StartThread(nbdThreadID, (struct lwnbd_server_t *)nbdsrv);
            } else {
                LOG("LWNBD_SERVER_CMD_START FAILED CreateThread.\n");
                *(int *)SifServerTxBuffer = nbdThreadID;
            }

            break;
        case LWNBD_SERVER_CMD_STOP:
            LOG("LWNBD_SERVER_CMD_STOP.\n");
            // TODO
            TerminateThread(nbdThreadID);
            DeleteThread(nbdThreadID);
            lwnbd_server_stop(nbdsrv);
            break;
        default:
            *(int *)SifServerTxBuffer = -ENXIO;
    }
    return SifServerTxBuffer;
}

/*  */
static void RpcThread(void *arg)
{
    sceSifSetRpcQueue(&SifQueueData, GetThreadId());
    sceSifRegisterRpc(&SifServerData, sid, &SifRpc_handler, SifServerRxBuffer, NULL, NULL, &SifQueueData);
    sceSifRpcLoop(&SifQueueData);
}

int _start(int argc, char **argv)
{
    int result;
    iop_thread_t thread;

    if (RegisterLibraryEntries(&_exp_lwnbd) == 0) {
        thread.attr = TH_C;
        thread.option = sid;
        thread.thread = &RpcThread;
        thread.priority = 0x20;
        thread.stacksize = 0x800;
        if ((RpcThreadID = CreateThread(&thread)) > 0) {
            StartThread(RpcThreadID, NULL);
            result = 0;
        } else {
            result = RpcThreadID;
            ReleaseLibraryEntries(&_exp_lwnbd);
        }
    } else {
        result = -1;
    }

    return (result == 0 ? MODULE_RESIDENT_END : MODULE_NO_RESIDENT_END);
}

// in don't think we go there with current module handling.
// TODO complete with nbd_server_stop
// int SifStopModule(int id, int arg_len, const char *args, int *mod_res);
// int SifUnloadModule(int id);
int _shutdown(void)
{
    ReleaseLibraryEntries(&_exp_lwnbd);
    TerminateThread(RpcThreadID);
    DeleteThread(RpcThreadID);
    return MODULE_NO_RESIDENT_END;
}
