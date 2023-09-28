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
 *
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <lwnbd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "../plugins/memory/memory.h"
#include "../servers/nbd/nbd.h"

/* static glue, could be generate :
 * #define LIST_ENTRY(x) x,
 *
 * */

extern lwnbd_plugin_t *command_plugin_init(void);
extern lwnbd_plugin_t *memory_plugin_init(void);
extern lwnbd_plugin_t *file_plugin_init(void);
extern struct lwnbd_server *nbd_server_init(void);

#define MAX_CLIENTS 10
uv_work_t *client_reqs[MAX_CLIENTS];

// plugin_init plugins_table[] = {
//		file_plugin_init,
//		NULL
// };

void signal_handler(uv_signal_t *req, int signum)
{
    printf("Signal received!\n");
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        uv_cancel((uv_req_t *)&client_reqs[i]);
    }
    uv_signal_stop(req);
    uv_stop(uv_default_loop());
}

int greeter(int argc, char **argv, void *result, int64_t *size)
{
    *size = lwnbd_dump_contexts(result);
    return 0;
}

int server_shutdown(int argc, char **argv, void *result, int64_t *size)
{
    uv_kill(uv_os_getpid(), SIGINT);
    return 0;
}

int gEnableWrite = 1;
int gHDDStartMode;

void on_close(uv_handle_t *handle)
{
    free(handle);
}

void on_after_work(uv_work_t *req, int status)
{
    free(req);
}

/*
 *  client thread
 *
 */
void on_work(uv_work_t *req)
{
    /* a bit of headache but avoid a baton */
    uv_stream_t *server = (uv_stream_t *)req->data;
    lwnbd_server_t *s = (lwnbd_server_t *)server->data;

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), client);

    if (uv_accept(server, (uv_stream_t *)client) == 0) {
        uv_stream_set_blocking((uv_stream_t *)client, 1);
    } else {
        uv_close((uv_handle_t *)client, on_close);
        return;
    }

    int sock;
    uv_fileno((const uv_handle_t *)client, &sock);
    DEBUGLOG("Worker %d: Accepted fd %d\n", getpid(), sock);
    lwnbd_server_run(*s, &sock); // the abstracted blocking loop
    uv_close((uv_handle_t *)client, on_close);
}

void on_new_connection(uv_stream_t *server, int status)
{
    int i;

    if (status < 0) {
        LOG("New connection error %s\n", uv_strerror(status));
        return;
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (client_reqs[i] == NULL) {
            client_reqs[i] = (uv_work_t *)malloc(sizeof(uv_work_t));
            break;
        }
    }

    if (client_reqs[i] == NULL) {
        LOG("New connection error : no slot available\n");
        return;
    }

    client_reqs[i]->data = server;
    uv_queue_work(uv_default_loop(), client_reqs[i], on_work, on_after_work);
}

int main(int argc, const char **argv)
{
    lwnbd_server_t nbdsrv;
    lwnbd_plugin_h fileplg, memplg, cmdplg;

    //    int i = atexit(coucou);
    //    if (i != 0) {
    //        fprintf(stderr, "cannot set exit function\n");
    //        exit(EXIT_FAILURE);
    //    }

    //    if (argc < 2) {
    //        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
    //        exit(EXIT_FAILURE);
    //    }

    /*
     * Register and configure some content plugins ...
     * Plugins can be shared between different servers so they live autonomously.
     *
     */

    /* NBD has no standard to get remote information about the server ?
     * let's use memory plugin to create an export 'motd' with some useful infos.
     */

    memplg = lwnbd_plugin_init(memory_plugin_init);
    char *data = calloc(1, 512);
    sprintf(data, "%s version %s\n compiled on: %s", APP_NAME, APP_VERSION, CC_VERSION);
    struct memory_config memh = {
        .base = (uint64_t)data,
        .name = "motd",
        .size = 512, /* nbdcopy 'nbd://127.0.0.1/motd' - | tr -d '\000' */
        .desc = "plain-text server information",
    };
    lwnbd_plugin_new(memplg, &memh);


    /*
     *
     */
    cmdplg = lwnbd_plugin_init(command_plugin_init);
    struct lwnbd_command mycmd[] = {
        {.name = "le", .desc = "list export", .cmd = greeter},
        {.name = "shutdown", .desc = "turn off the application", .cmd = server_shutdown},
        NULL,
    };

    lwnbd_plugin_new(cmdplg, &mycmd[0]);
    lwnbd_plugin_new(cmdplg, &mycmd[1]);


    fileplg = lwnbd_plugin_init(file_plugin_init);
    /* assuming no user input error */
    /* todo : lwnbd_plugin_news() */
    for (int i = 1; i < argc; i++) {
        lwnbd_plugin_new(fileplg, argv[i]);
    }

    /*
     * create a NBD server, and eventually configure it.
     *
     */

    struct nbd_server mynbd = {
        .port = 10809,
        .max_retry = MAX_RETRIES,
        .gflags = (NBD_FLAG_FIXED_NEWSTYLE | NBD_FLAG_NO_ZEROES),
        .preinit = 0,
        .readonly = !gEnableWrite,
    };
    nbdsrv = lwnbd_server_init(nbd_server_init);
    lwnbd_server_new(nbdsrv, &mynbd);

    lwnbd_server_config(nbdsrv, "default-export", "motd");

    lwnbd_server_dump(nbdsrv);

    /*
     * Main loop with libuv
     */

    uv_loop_t *loop = uv_default_loop();
    struct sockaddr_in addr;

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", mynbd.port, &addr);

    server.data = &nbdsrv;

    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
    int r = uv_listen((uv_stream_t *)&server, 128, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);

    return uv_run(loop, UV_RUN_DEFAULT);
}
