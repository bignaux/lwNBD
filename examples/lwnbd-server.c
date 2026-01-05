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
#include <lwnbd/lwnbd.h>
#include <lwnbd/nbd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-journal.h>
#include <unistd.h>
#include <uv.h>

typedef enum {
    CLIENT_FREE,
    CLIENT_INUSE,
} worker_state_t;

struct nbd_worker
{
    uv_work_t req;
    int client_id;
    uv_stream_t *server;
};

#define MAX_CLIENTS 3
int gEnableWrite = 1;
int gHDDStartMode;

static worker_state_t client_states[MAX_CLIENTS];
struct nbd_worker *client_reqs[MAX_CLIENTS]; /* need to be accessible to signal handler */

// void journaldlog(const char *file, const char *tag, int level, int line,
//		const char *func, const char *message) {
//
// printf("[%-5s][%s] %s", level_name, tag, message);
//	int sd_journal_print(int level,);
//
// }

static void
usage(void)
{
    fprintf(stderr, "usage: %s [-v verboselevel] [-i iface] [-p port] [-f file]\n", argv0);
    exit(1);
}

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

/* man 7 signal */
int signal_cb(int argc, char **argv, void *result, int64_t *size)
{
    int signum = SIGINT;
    if (argc > 1) {
        signum = atoi(argv[1]);
    }
    *size = sprintf(result, "%s with signal %s\n", argv[0], strsignal(signum));
    uv_kill(uv_os_getpid(), signum);
    return 0;
}

void on_after_work(uv_work_t *req, int status)
{
    struct nbd_worker *client_reqs = (struct nbd_worker *)req;
    client_states[client_reqs->client_id] = CLIENT_FREE;
    free(req);
}

void on_close(uv_handle_t *handle)
{
    lwnbd_debug("on_close\n");
    free(handle);
}

/*
 *  client thread
 *
 */
void on_work(uv_work_t *req)
{
    /* a bit of headache but avoid a baton */
    struct nbd_worker *client_reqs = (struct nbd_worker *)req;
    uv_stream_t *server = (uv_stream_t *)client_reqs->server;
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
    lwnbd_debug("/++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    lwnbd_debug("Worker %d: Accepted fd %d\n", getpid(), sock);
    lwnbd_server_run(*s, &sock); // the abstracted blocking loop
    lwnbd_debug("+++++++++++++++++++++++++++++++++++++++++++++++++++/\n");
    uv_close((uv_handle_t *)client, on_close);
}

void on_new_connection(uv_stream_t *server, int status)
{
    int i;

    if (status < 0) {
        lwnbd_info("New connection error %s\n", uv_strerror(status));
        return;
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        lwnbd_debug("client_reqs[%d] = %p\n", i, client_reqs[i]);
        if (client_states[i] == CLIENT_FREE) {
            client_reqs[i] = (struct nbd_worker *)malloc(sizeof(struct nbd_worker));
            client_states[i] = CLIENT_INUSE;
            client_reqs[i]->client_id = i;
            client_reqs[i]->server = server;
            uv_queue_work(uv_default_loop(), (uv_work_t *)client_reqs[i], on_work, on_after_work);
            return;
        }
    }

    lwnbd_info("New connection error : no slot available\n");
    return;
}

int main(int argc, const char **argv)
{
    lwnbd_server_t httpsrv;
    lwnbd_plugin_h fileplg, memplg, cmdplg;
    struct sockaddr_in addr;
    int port = NBDDEFAULTPORT;
    int level = LWNBD_LOG_DEBUG;

    /*
     *
     */

    while ((opt = getopt(argc, argv, "v:i:p:f:")) != -1) {
        switch (opt) {
            case 'v':
                level = strtol(optarg, NULL, 10);
                lwnbd_set_log_level(level);
                break;
            case 'i':
                //			iface = optarg;
                break;
            case 'p':
                port = strtol(optarg, NULL, 10);
                break;
            case 'f':
                if (fileplg == NULL)
                    fileplg = lwnbd_plugin_init(file_plugin_init);
                lwnbd_plugin_new(fileplg, optarg);
                break;
            case '?':
                usage();
                break;
        }
    }

    //    lwnbd_set_log_callback(journaldlog);

    /*******************************************************************************
     * Register and configure some content plugins ...
     * Plugins can be shared between different servers so they live autonomously.
     *
     *******************************************************************************/

    /*
     * NBD has no standard to get remote information about the server ?
     * Let's use memory plugin to create an export 'motd' with some useful infos.
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

    lwnbd_set_defaultexport("motd");

    /*
     * Register few useful callback to control the server over the network
     *
     */

    cmdplg = lwnbd_plugin_init(command_plugin_init);
    struct lwnbd_command mycmd[] = {
        {.name = "shutdown", .desc = "turn off the application", .cmd = signal_cb},
        {NULL},
    };

    lwnbd_plugin_new(cmdplg, &mycmd[0]);

    /*
     * Create a NBD transport, and eventually configure it.
     *
     */

    httpsrv = lwnbd_server_init(nbd_server_init);
    struct nbdsettings mynbd;

    /* Initialize user callbacks and settings */
    lwnbd_server_new(httpsrv, &mynbd);
    //    mynbd.readonly = !gEnableWrite;


    /*
     * Main loop with libuv
     */

    uv_loop_t *loop = uv_default_loop();
    memset(client_states, CLIENT_FREE, sizeof(client_states));
    uv_tcp_t server;

    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", port, &addr);

    server.data = &httpsrv;

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
