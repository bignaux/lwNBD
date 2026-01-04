/*
 * A demonstration of the methods currently available to access an export locally.
 */

#include <lwnbd/lwnbd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
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
    lwnbd_plugin_new(fileplg, argv[1]);
    ctx = lwnbd_get_context(argv[1]);

    while (r == 0) {
        r = lwnbd_pread(ctx, buf, 1, offset++, 0);
        printf("%c", *buf);
        fflush(stdout);
        usleep(1e4);
    }

    exit(EXIT_SUCCESS);
}
