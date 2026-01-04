/*
 * builtin.c
 *
 * A great sandbox :D
 */

static int echo(int argc, char **argv, const void *result, int64_t *size)
{
    lwnbd_context_t *ctx;
    const char *delimiters = "\n\0 ";

    char *temp, *pargs, *presult;
    int cnt = 0;
    char req[30];

    if (argc == 1) {
        return 0;
    }

    *size = 0;
    temp = argv[1];

    while (1) {
        pargs = strchr(temp, '$');
        if (pargs) {
            cnt = pargs - temp - 1;
            memcpy(result, temp, cnt);
            *size += cnt;
            pargs++;

            temp = strtok(pargs, delimiters);
            cnt = temp - pargs - 1;
            strncpy(req, pargs, cnt);

            ctx = lwnbd_get_context(req);
            if (ctx) {
                lwnbd_pread(ctx, result + *size, ctx->exportsize, 0, 0);
                *size += ctx->exportsize;
            } else {
            }

        } else {
            *size += strlen(temp);
            memcpy(result, temp, *size);
            break;
        }
    }

    return 0;
}

static int export(int argc, char **argv, const void *result, int64_t *size)
{
    if (argc == 1) {
        *size = lwnbd_dump_contexts(result);
        return 0;
    }

    /* need mapper to finish */

    return 0;
}

/* kinda serialiser for commands array of structure */
static int list_command(int argc, char **argv, const void *result, int64_t *size)
{
    int cnt = 0;
    /* we could use size as result size*/
    *size = 0;
    while (cnt < commands_cnt) {
        *size += sprintf((char *)result + *size, "%-32s: %s\n", commands[cnt].name, commands[cnt].desc);
        cnt++;
    }
    //    (*size)++;
    //    DEBUGLOG("%d : %s\n", *size, result);
    return 0;
}

static struct lwnbd_command builtin[] = {
    {.name = "echo", .desc = "Print on the standard output.", .cmd = echo},
    {.name = "export", .desc = "Set export attribute for shell variables.", .cmd = export},
    {.name = "lc", .desc = "List shell commands.", .cmd = list_command},
    {NULL},
};
