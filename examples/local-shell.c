/*
 * An interactive shell using command plugin
 *
 * [local-shell:/api]$ echo=$test
 * Example of shared memory.
 * [local-shell:/api]$ echo=pouet
 * pouet
 *
 */

#define _GNU_SOURCE
#include <errno.h>
#include <libgen.h>
#include <lwnbd/lwnbd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char *exportname, *prompt, *base;

static int set_prompt(char *base, char *endpoint)
{
    if (0 > asprintf(&prompt, "[%s:/%s]$ ", base, endpoint))
        return -1;
    return 0;
}

static int set_exportname(int argc, char **argv, const void *result, int64_t *size)
{
    if (argc <= 1)
        return -1;

    *size = asprintf(&exportname, "%s", argv[1]);
    if (*size < 0) {
        *size = sprintf(result, "error setting export name\n");
    } else {
        *size = sprintf(result, "export name to %s\n", exportname);
    }
    set_prompt(base, exportname);
    return 0;
}

int main(int argc, char **argv)
{
    lwnbd_plugin_h cmdplg, memplg;
    lwnbd_context_t *ctx;
    char *p, *buf, *memtest;
    int errno, r = 0;
    uint64_t bufsize = 0;

    /*
     * set base URI, default exportname to 'api' and prompt
     *
     */

    if (0 > asprintf(&exportname, "api"))
        exit(EXIT_FAILURE);

    if (0 > asprintf(&base, "%s", basename(argv[0])))
        exit(EXIT_FAILURE);

    if (0 > set_prompt(base, exportname))
        exit(EXIT_FAILURE);

    /*
     * create a slice of memory to test memory query
     *
     */

    memplg = lwnbd_plugin_init(memory_plugin_init);
    memtest = calloc(1, 512);

    struct memory_config memh = {
        .base = (uint64_t)memtest,
        .name = "test",
        .size = 512,
        .desc = "",
    };
    lwnbd_plugin_new(memplg, &memh);

    /*
     * let's use query mechanism to set it, just because we can !
     */

    if (0 > asprintf(&buf, "test?memcpy=Example of shared memory.")) /* create the request */
        exit(EXIT_FAILURE);

    lwnbd_get_context(buf); /* GET */

    /*
     * create a command to change export, to be able to switch to it
     */

    cmdplg = lwnbd_plugin_init(command_plugin_init);

    struct lwnbd_command mycmd = {
        .name = "ce",
        .desc = "Change exportname/endpoint",
        .cmd = set_exportname,
    };

    lwnbd_plugin_new(cmdplg, &mycmd);

    /*
     * main 'interactive' shell loop
     */

    printf("Welcome! Type lc to list commands\n");

    while (1) {
        int n;

        errno = 0;
        printf("%s", prompt);
        n = scanf("%ms", &p);
        if (n == 1) {
            if (0 == strcmp("exit", p)) {
                free(p);
                break;
            }
            bufsize = asprintf(&buf, "%s?%s", exportname, p);
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
        if (ctx == NULL) {
            fprintf(stderr, "lwnbd_get_context\n");
            continue;
        }

        if (bufsize < ctx->exportsize + 1) {
            bufsize = ctx->exportsize + 1;
            buf = (char *)realloc(buf, bufsize);
            if (buf == NULL) {
                fprintf(stderr, "realloc\n");
                goto exit;
            }
        }

        r = lwnbd_pread(ctx, buf, ctx->exportsize, 0, 0);
        if (r) {
            fprintf(stderr, "lwnbd_pread\n");
            break;
        }

        r = strnlen(buf, ctx->exportsize);
        buf[r] = '\0';
        printf("Response: %ld bytes, print %d bytes.\n", ctx->exportsize, r);
        printf("%s\n", buf);
    }

exit:
    free(buf);
    free(exportname);
    free(prompt);
    free(base);
    free(memtest);
    exit(EXIT_SUCCESS);
}
