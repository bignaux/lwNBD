/*
 * A demonstration of setting POST method
 *
 *
 *
 */

#define _GNU_SOURCE
#include <lwnbd/lwnbd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int64_t count;

static int byte_count(int argc, char **argv, const void *data, int64_t *size)
{
    count++;
    return 0;
}

int main(int argc, char **argv)
{
    lwnbd_plugin_h fileplg, cmdplg;
    lwnbd_context_t *filectx, *wcctx;
    char *buf;
    int r = 0;
    uint64_t offset = 0;
    count = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fileplg = lwnbd_plugin_init(file_plugin_init);
    lwnbd_plugin_new(fileplg, argv[1]);
    filectx = lwnbd_get_context(argv[1]);

    cmdplg = lwnbd_plugin_init(command_plugin_init);

    struct lwnbd_command mycmd = {
        .name = "wc",
        .desc = "byte count",
        .cmd = byte_count,
        .type = METHOD_POST,
    };

    lwnbd_plugin_new(cmdplg, &mycmd);

    if (-1 == asprintf(&buf, "api?wc")) /* create the request */
        exit(EXIT_FAILURE);

    wcctx = lwnbd_get_context(buf);

    while (r == 0) {
        r = lwnbd_pread(filectx, buf, 1, offset++, 0);
        if (r)
            break;
        r = lwnbd_pwrite(wcctx, buf, 1, 0, 0);
    }

    printf("%ld %s\n", count, argv[1]);
    free(buf);
    exit(EXIT_SUCCESS);
}
