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
#include "../../plugins/memory/memory.h"

IRX_ID(APP_NAME, 1, 1);
static int nbd_tid;
extern struct irx_export_table _exp_lwnbd;
extern struct lwnbd_server *nbd_server_init(void);
extern struct lwnbd_plugin *atad_plugin_init(void);
extern struct lwnbd_plugin *mcman_plugin_init(void);
extern struct lwnbd_plugin *memory_plugin_init(void);

lwnbd_server_t nbdsrv;

/* from sysman */
static int GetSizeFromDelay(int device)
{
    int size = (GetDelay(device) >> 16) & 0x1F;
    return (1 << size);
}

int _start(int argc, char **argv)
{
    iop_thread_t nbd_thread;
    lwnbd_plugin_t atadplg, memplg, mcmanplg;

    /* TODO: manage existence */
    struct memory_config bios = {
        .base = 0x1FC00000,
        .name = "bios",
        .size = GetSizeFromDelay(SSBUSC_DEV_BOOTROM), // 0x400000
        .desc = "BIOS (rom0)",
    };

    struct memory_config iopram = {
        .base = 0,
        .size = QueryMemSize(),
        .name = "ram",
        .desc = "IOP main RAM",
    };

    //    struct memory_config dvdrom = {
    //        .base = GetBaseAddress(SSBUSC_DEV_DVDROM),
    //        .size = GetSizeFromDelay(SSBUSC_DEV_DVDROM),
    //        .name = "dvdrom",
    //        .desc = "DVD-ROM rom",
    //    };

    if (argc > 1) {
        //		strcpy(gdefaultexport, argv[1]);
        //		LOG("default export : %s\n", gdefaultexport);
        //		lwnbd_server_config(nbdsrv, "defaultexport", gdefaultexport);
    }

    RegisterLibraryEntries(&_exp_lwnbd);

    atadplg = lwnbd_plugin_init(atad_plugin_init);
    for (int i = 0; i < 2; i++) {
        lwnbd_plugin_new(atadplg, &i);
    }

    mcmanplg = lwnbd_plugin_init(mcman_plugin_init);
    for (int i = 0; i < 2; i++) {
        lwnbd_plugin_new(mcmanplg, &i);
    }

    memplg = lwnbd_plugin_init(memory_plugin_init);
    lwnbd_plugin_new(memplg, &iopram);
    lwnbd_plugin_new(memplg, &bios);
    //    lwnbd_plugin_new(memplg, &dvdrom);

    nbdsrv = lwnbd_server_init(nbd_server_init);

    nbd_thread.attr = TH_C;
    nbd_thread.option = 0;
    nbd_thread.thread = (void *)lwnbd_server_start;
    nbd_thread.stacksize = 0x2000; // 0x800;
    nbd_thread.priority = 0x10;

    nbd_tid = CreateThread(&nbd_thread);

    // int StartThreadArgs(int thid, int args, void *argp);
    //    StartThread(nbd_tid, (struct lwnbd_plugin *)lwnbd_plugins);
    StartThread(nbd_tid, (struct lwnbd_server_t *)nbdsrv);
    return MODULE_RESIDENT_END;
}

// TODO complete with nbd_server_stop
int _shutdown(void)
{
    DeleteThread(nbd_tid);
    return 0;
}
