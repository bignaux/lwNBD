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
#include <fcntl.h>
#include <lwnbd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "../../plugins/memory/memory.h"
#include "../../servers/nbd/nbd.h"

/* static glue, could be generate :
 * #define LIST_ENTRY(x) x,
 *
 * */

extern lwnbd_plugin_t *memory_plugin_init(void);
extern lwnbd_plugin_t *file_plugin_init(void);
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

int gEnableWrite = 1;
int gHDDStartMode;

void on_close(uv_handle_t *handle)
{
    free(handle);
}

/* see http://docs.libuv.org/en/v1.x/guide/utilities.html#baton */
struct nbd_baton
{
    uv_work_t req;
    uv_tcp_t *client;
    struct nbd_server *s;
    struct nbd_client *c;
};

void on_after_work(uv_work_t *req, int status)
{
    free(req);
}

void on_work(uv_work_t *req)
{
    struct nbd_baton *mybat = (struct nbd_baton *)req->data;
    register err_t r;

    r = client_init(mybat->s, mybat->c);
    if (r)
        return;

    // TODO : move to uv_buf_t
    mybat->c->nbd_buffer = (uint8_t *)calloc(NBD_BUFFER_LEN, sizeof(uint8_t));

    if (mybat->c->nbd_buffer == NULL) {
        perror("calloc:");
        return;
    }

    while (r == 0) {
        switch (mybat->c->state) {
            case HANDSHAKE:
                r = protocol_handshake(mybat->s, mybat->c);
                if (r == -1) {
                    LOG("an error occured during negotiation phase.\n");
                }
                break;
            case TRANSMISSION:
                r = transmission_phase(mybat->c);
                if (r == -1)
                    LOG("an error occured during transmission phase.\n");
                break;
            case ABORT:
                r = -1;
                break;
            default:
                break;
        }
    }

    uv_close((uv_handle_t *)mybat->client, on_close);
    free(mybat->c->nbd_buffer);
    free(mybat->c);
}

void on_new_connection(uv_stream_t *server, int status)
{
    struct nbd_baton *mybat = (struct nbd_baton *)malloc(sizeof(struct nbd_baton));
    mybat->req.data = (void *)mybat;
    mybat->s = server->data;
    mybat->c = (struct nbd_client *)malloc(sizeof(struct nbd_client));

    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    mybat->client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), mybat->client);
    if (uv_accept(server, (uv_stream_t *)mybat->client) == 0) {
        uv_fileno((const uv_handle_t *)mybat->client, &mybat->c->sock);
        LOG("Worker %d: Accepted fd %d\n", getpid(), mybat->c->sock);
        uv_stream_set_blocking((uv_stream_t *)mybat->client, 1);
        uv_queue_work(uv_default_loop(), &mybat->req, on_work, on_after_work);

    } else {
        uv_close((uv_handle_t *)mybat->client, on_close);
    }
}

int main(int argc, const char **argv)
{
    lwnbd_server_t nbdsrv;
    lwnbd_plugin_h fileplg, memplg;

    int i = atexit(coucou);
    if (i != 0) {
        fprintf(stderr, "cannot set exit function\n");
        exit(EXIT_FAILURE);
    }

    //    if (argc < 2) {
    //        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
    //        exit(EXIT_FAILURE);
    //    }

    //    signal(SIGINT, signal_callback_handler);

    /*
     * Register and configure some content plugins ...
     * Plugins can be shared between different servers so they live autonomously.
     *
     */

    /* NBD has no standard to get remote information about the server ?
     * let's use memory plugin to create an export 'motd' with some useful infos.
     */

    char *data = calloc(1, 512);
    sprintf(data, "%s version %s\n compiled on: %s", APP_NAME, APP_VERSION, CC_VERSION);
    struct memory_config memh = {
        .base = (uint64_t)data,
        .name = "motd",
        .size = 512, /* nbdcopy 'nbd://127.0.0.1/motd' - | tr -d '\000' */
        .desc = "plain-text server information",
    };
    memplg = lwnbd_plugin_init(memory_plugin_init);
    lwnbd_plugin_new(memplg, &memh);

    fileplg = lwnbd_plugin_init(file_plugin_init);

    /* assuming no user input error */
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

    //    lwnbd_server_config(nbdsrv, "default-export", "README.md");

    lwnbd_dump_contexts();
    lwnbd_server_dump(nbdsrv);

    /*
     * Main loop with libuv
     */

    uv_loop_t *loop = uv_default_loop();
    struct sockaddr_in addr;

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", mynbd.port, &addr);

    server.data = &mynbd;

    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
    int r = uv_listen((uv_stream_t *)&server, 128, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
