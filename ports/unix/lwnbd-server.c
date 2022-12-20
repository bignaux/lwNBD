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
 *   A simple example of lwnbd library usage.
 */

#include <assert.h>
#include <errno.h>
#include <lwnbd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* static glue, could be generate :
 * #define LIST_ENTRY(x) x,
 *
 * */

extern struct lwnbd_plugin *file_plugin_init(void); // https://blog.the-pans.com/gnu-visibility-attribute/
// plugin_init plugins_table[] = {
//		file_plugin_init,
//		NULL
// };

extern struct lwnbd_server *nbd_server_init(void);

// void signal_callback_handler(int signum)
//{
//     exit(signum);
// }
void coucou()
{
    printf("exit\n");
}

int gEnableWrite = 0;
int gHDDStartMode;

int main(int argc, char **argv)
{
    lwnbd_server_t nbdsrv;
    lwnbd_plugin_t fileplg;

    int i = atexit(coucou);
    if (i != 0) {
        fprintf(stderr, "cannot set exit function\n");
        exit(EXIT_FAILURE);
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //    signal(SIGINT, signal_callback_handler);

    /*
     * Register a content plugin and configure it
     * plugins can be share between different servers so they live autonomously.
     *
     */

    fileplg = lwnbd_plugin_init(file_plugin_init);

    /* create key=value for file plugin assuming no user error */
    for (int i = 1; i < argc; i++) {
        lwnbd_plugin_config(fileplg, NULL, argv[i]);
    }

    /*
     * create a NBD server, and eventually configure it.
     *
     */

    nbdsrv = lwnbd_server_init(nbd_server_init);
    lwnbd_server_config(nbdsrv, "defaultexport", "README.md");
    if (!gEnableWrite)
        lwnbd_server_config(nbdsrv, "readonly", NULL);

    lwnbd_dump_contexts();
    lwnbd_dump_server(nbdsrv);

    // TODO: blocking server, need to go background, with PID file
    lwnbd_server_start(nbdsrv);

    // TODO: install trap signal USR1, make makefile rule to send it
    lwnbd_server_stop(nbdsrv);
    exit(EXIT_SUCCESS);
}
