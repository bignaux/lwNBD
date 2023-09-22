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
 *
 *
 */

#include "ioplib.h"
#include "irx_imports.h"
#include <config.h>
#include <lwnbd.h>

/* tcp.c */
extern void listener(struct nbd_server *s);
extern int nbd_close(int socket);
extern int nbd_server_create(struct nbd_server *server);

IRX_ID(APP_NAME, 1, 1);
extern struct irx_export_table _exp_lwnbd;

static lwnbd_server_t sifrpcsrv, nbdsrv;
static int nbdThreadID;

enum LWNBD_SERVER_CMD {
    LWNBD_SERVER_CMD_START,
    LWNBD_SERVER_CMD_STOP,
};

static int configured = 0;

struct lwnbd_config
{
    char defaultexport[32];
    uint8_t readonly;
};


// TODO : move config in OPL, EE/apps side
// new bug : issue when config already done, write permission can't be enable back.
static int config(struct lwnbd_config *config)
{
    /*
     * create a NBD server, and eventually configure it.
     *
     */
    nbdsrv = lwnbd_server_init(nbd_server_init);

    struct nbd_server mynbd = {
        .port = 10809,
        .max_retry = MAX_RETRIES,
        .gflags = (NBD_FLAG_FIXED_NEWSTYLE | NBD_FLAG_NO_ZEROES),
        .preinit = 0,
        //		.readonly = config->readonly ? ,
    };
    lwnbd_server_new(nbdsrv, &mynbd);
    lwnbd_server_config(nbdsrv, "default-export", config->defaultexport);

    nbd_server_create(&mynbd);

    if (config->readonly)
        lwnbd_server_config(nbdsrv, "readonly", NULL);


        //    lwnbd_server_config(nbdsrv, "preinit", NULL);
        // print twice doesn't give same result ...
        //    print_memorymap();
        //    print_memorymap();

#ifdef PLUGIN_ATAD
    lwnbd_plugin_h atadplg = lwnbd_plugin_init(atad_plugin_init);
    for (int i = 0; i < 2; i++) {
        lwnbd_plugin_new(atadplg, &i);
    }
#endif

#ifdef PLUGIN_MCMAN
    lwnbd_plugin_h mcmanplg = lwnbd_plugin_init(mcman_plugin_init);
    for (int i = 0; i < 2; i++) {
        lwnbd_plugin_new(mcmanplg, &i);
    }
#endif

#ifdef PLUGIN_MEMORY
    lwnbd_plugin_h memplg = lwnbd_plugin_init(memory_plugin_init);
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
    lwnbd_plugin_h bdmplg = lwnbd_plugin_init(bdm_plugin_init);
    lwnbd_plugin_new(bdmplg, NULL);
#endif

#ifdef PLUGIN_PCMSTREAM
    lwnbd_plugin_h pcmplg = lwnbd_plugin_init(pcmstream_plugin_init);
    struct pcmstream_config pcmc = {
        .name = "speakers",
        .desc = "stereo speaker",
        //        .format = "s16le",
        .bits = 16,
        .rate = 44100,
        .channels = 2,
        .volume = 70,
    };

    lwnbd_plugin_new(pcmplg, &pcmc);
#endif

    return 0;
}

static int *lwnbd_server_cmd_start(struct lwnbd_config *conf, int length, int *ret)
{
    iop_thread_t nbd_thread;

    if (configured == 0) {
        config(conf);
        configured = 1;
    }

    DEBUGLOG("LWNBD_SERVER_CMD_START 2.\n");

    nbd_thread.attr = TH_C;
    nbd_thread.option = 0;
    nbd_thread.thread = (void *)listener;
    nbd_thread.stacksize = 0x800;
    nbd_thread.priority = 0x10;

    nbdThreadID = CreateThread(&nbd_thread);
    if (nbdThreadID > 0) {
        LOG("LWNBD_SERVER_CMD_START StartThread.\n");
        *ret = StartThread(nbdThreadID, (struct lwnbd_server_t *)nbdsrv);
    } else {
        LOG("LWNBD_SERVER_CMD_START FAILED CreateThread.\n");
        *ret = nbdThreadID;
    }
    return ret;
}

/* sifrpc server custom app handler
 *
 * ideally <lwnbd.h> completed so we can remove config() hack
 *
 * typedef void * (*SifRpcFunc_t)(int fno, void *buffer, int length);
 * */
static int *lwnbd_rpc_handler(int fno, void *buffer, int length)
{
    int ret;
    switch (fno) {
        case LWNBD_SERVER_CMD_START:
            return lwnbd_server_cmd_start(buffer, length, &ret);
        case LWNBD_SERVER_CMD_STOP:
            LOG("LWNBD_SERVER_CMD_STOP.\n");
            // TODO
            TerminateThread(nbdThreadID);
            DeleteThread(nbdThreadID);
            //            nbd_close(int socket)
            //            lwnbd_server_stop(nbdsrv);

            break;
        default:
            ret = -ENXIO;
    }
    return &ret;
}


/* create a classic RPC listener thread *again* */
int _start(int argc, char **argv)
{
    struct sifrpc_handler conf = {
        .sid = 0x2A39,
        .sifrpc_handler = (void *)&lwnbd_rpc_handler,
    };

    sifrpcsrv = lwnbd_server_init(sifrpc_server_init);
    if (sifrpcsrv < 0) {
        LOG("failed init sifrpc server.\n");
        return MODULE_NO_RESIDENT_END;
    }

    lwnbd_server_new(sifrpcsrv, &conf);

    if (!RegisterLibraryEntries(&_exp_lwnbd)) {
        return MODULE_NO_RESIDENT_END;
    }

    lwnbd_server_start(sifrpcsrv);

    return MODULE_RESIDENT_END;
}

// in don't think we go there with current module handling.
// int SifStopModule(int id, int arg_len, const char *args, int *mod_res);
// int SifUnloadModule(int id);
int _shutdown(void)
{
    lwnbd_server_stop(sifrpcsrv);
    ReleaseLibraryEntries(&_exp_lwnbd);
    return MODULE_NO_RESIDENT_END;
}
