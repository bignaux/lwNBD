/*
 * A demonstration of the methods currently available to access an export locally.
 */

#include <lwnbd.h>
#include <lwnbd-context.h>
#include <lwnbd-server.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern lwnbd_plugin_t *file_plugin_init(void);

int main(int argc, const char **argv)
{
    lwnbd_plugin_h fileplg;
    lwnbd_context_t *ctx;
    char buf[80];
    int r = 0;
    uint64_t offset = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fileplg = lwnbd_plugin_init(file_plugin_init);
    lwnbd_plugin_new(fileplg, argv[1]); /* could return context */

    /*
     * using lwnbd_context_t like a server plugin would.
     */

    ctx = lwnbd_get_context(argv[1]);

    //    while (r == 0) {
    //        r = plugin_pread(ctx, buf, 1, offset++, 0); /* we are limited to wrapped func in server.h */
    //        printf("%c", *buf);
    //        fflush(stdout);
    //        usleep(1e4);
    //    }

    /*
     * using lwnbd_plugin_t
     */

    lwnbd_plugin_t *p = ctx->p;
    void *h = ctx->handle;

    while (r == 0) {
        r = p->pread(h, buf, 1, offset++, 0); /* full access but to a generic lwnbd_plugin_t interface */
        printf("%c", *buf);
        fflush(stdout);
        usleep(1e4);
    }

    exit(EXIT_SUCCESS);
}
