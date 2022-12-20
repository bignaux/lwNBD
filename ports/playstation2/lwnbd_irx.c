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

IRX_ID(APP_NAME, 1, 1);
static int nbd_tid;
extern struct irx_export_table _exp_lwnbd;
extern struct lwnbd_server *nbd_server_init(void);
extern struct lwnbd_plugin *atad_plugin_init(void);

lwnbd_server_t nbdsrv;
//    lwnbd_plugin_t atadplg;
//    lwnbd_plugin_t mcmanplg;

int _start(int argc, char **argv)
{
    iop_thread_t nbd_thread;


    if (argc > 1) {
        //        strcpy(gdefaultexport, argv[1]);
        //        LOG("default export : %s\n", gdefaultexport);
    }

    // register exports
    RegisterLibraryEntries(&_exp_lwnbd);


    lwnbd_plugin_init(atad_plugin_init);
    //    mcmanplg = lwnbd_plugin_init();

    nbdsrv = lwnbd_server_init(nbd_server_init);

    nbd_thread.attr = TH_C;
    nbd_thread.option = 0;
    nbd_thread.thread = (void *)lwnbd_server_start;
    nbd_thread.stacksize = 0x800;
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
