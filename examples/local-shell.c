/*
 * An interactive shell using command plugin
 *
 */

#define _GNU_SOURCE
#include <libgen.h>
#include <lwnbd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lwnbd-plugin.h>

extern struct lwnbd_plugin_t *command_plugin_init(void);

char *exportname, *prompt, *base;

static int set_prompt(char *base, char *endpoint)
{
    if (-1 == asprintf(&prompt, "[%s:/%s]$ ", base, endpoint))
        return -1;
    return 0;
}

static int set_exportname(int argc, char **argv, void *result, int64_t *size)
{
    *size = asprintf(&exportname, "%s", argv[0]);
    if (*size == -1) {
        *size = sprintf(result, "error setting export name\n");
    } else {
        *size = sprintf(result, "export name to %s\n", exportname);
    }
    set_prompt(base, exportname);
    return 0;
}

int main(int argc, char **argv)
{
    lwnbd_plugin_h cmdplg;
    lwnbd_context_t *ctx;
    char *p, *buf;
    int errno, r = 0;
    ;
    uint64_t size = 0;

    if (-1 == asprintf(&exportname, "shell"))
        exit(EXIT_FAILURE);

    if (-1 == asprintf(&base, "%s", basename(argv[0])))
        exit(EXIT_FAILURE);

    if (-1 == set_prompt(base, exportname))
        exit(EXIT_FAILURE);

    cmdplg = lwnbd_plugin_init(command_plugin_init);

    struct lwnbd_command mycmd = {
        .name = "ce",
        .desc = "change exportname",
        .cmd = set_exportname,
    };

    lwnbd_plugin_new(cmdplg, &mycmd);

    while (r == 0) {
        int n;

        errno = 0;
        printf("%s", prompt);
        n = scanf("%ms", &p);
        if (n == 1) {
            if (0 == strcmp("exit", p))
                break;
            r = asprintf(&buf, "%s?%s", exportname, p);
            printf("Request: %s\n", buf);
            free(p);
        } else if (errno != 0) {
            perror("scanf");
            break;
        } else {
            fprintf(stderr, "No matching characters\n");
            break;
        }

        ctx = lwnbd_get_context(buf);

        if (size < ctx->exportsize) {
            size = ctx->exportsize;
            buf = realloc(buf, size);
        }
        printf("Response: %ld\n", ctx->exportsize);
        r = lwnbd_pread(ctx, buf, ctx->exportsize, 0, 0);
        printf("%s", buf);
    }

    exit(EXIT_SUCCESS);
}
