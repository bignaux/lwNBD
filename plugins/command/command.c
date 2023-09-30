#include <lwnbd-plugin.h>
#include <string.h>

#define PLUGIN_NAME    command
#define CMD_DRIVER_MAX 100

/*
 * Here we don't want an export by command,
 * but one export to be reachable by lwnbd_get_context()
 */

static struct lwnbd_command commands[CMD_DRIVER_MAX];
static int commands_cnt = 0;

struct result_h
{
    char result[512];
    int64_t size;
};

static struct result_h pr;

static inline int command_pread(void *handle, void *buf, uint32_t count,
                                uint64_t offset, uint32_t flags)
{
    memcpy(buf, pr.result, count);
    return 0;
}

static inline int command_pwrite(void *handle, const void *buf, uint32_t count,
                                 uint64_t offset, uint32_t flags)
{
    return 0;
}

static int command_block_size(void *handle,
                              uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = *preferred = *maximum = 1;
    return 0;
}

/* kinda serialiser for commands array of structure */
static int command_list(int argc, char **argv, void *result, int64_t *size)
{
    int cnt = 0;
    /* we could use size as result size*/
    *size = 0;
    while (cnt < commands_cnt) {
        *size += sprintf((char *)result + *size, "%-32s: %s\n", commands[cnt].name, commands[cnt].desc);
        cnt++;
    }
    //    DEBUGLOG("%d : %s\n", *size, result);
    return 0;
}

/*
 * ?name=argv
 */
static int command_query(void *handle, struct query_t *params, int nb_params)
{
    int cnt = 0;
    struct lwnbd_command *pvhandle = NULL;

    if (nb_params == 0) {
        DEBUGLOG("found no command\n");
        return -1;
    }

    while (cnt < commands_cnt) {
        //        DEBUGLOG("commands_cnt=%d\n", commands_cnt);
        if (0 == strcmp(params[0].key, commands[cnt].name)) {
            pvhandle = &commands[cnt];
            DEBUGLOG("found command : %s\n", commands[cnt].name);
            break;
        }
        cnt++;
    }

    memset(pr.result, '\0', 512);

    if (pvhandle == NULL) {
        DEBUGLOG("%s: command not found\n", commands[cnt].name);
        pr.size = sprintf(pr.result, "%s: command not found\n", params[0].key);
        return 0;
    }

    /* TODO: format argv from params */
    pvhandle->cmd(1, &params[0].val, pr.result, &pr.size);

    return 0;
}

static int64_t command_get_size(void *handle)
{
    return pr.size;
}

static int command_add_command(const struct lwnbd_command *h)
{
    if (commands_cnt == CMD_DRIVER_MAX)
        return -1;

    memcpy(&commands[commands_cnt++], h, sizeof(struct lwnbd_command));
    return 0;
}

/*
 * look like register RPC or callback
 */
static int command_ctor(const void *pconfig, lwnbd_export_t *e)
{
    e->handle = NULL;
    return command_add_command((struct lwnbd_command *)pconfig);
}

static void command_load(void)
{
    struct lwnbd_command builtin[] = {
        {.name = "lc", .desc = "list commands", .cmd = command_list},
        {.name = "lce", .desc = "list commands", .cmd = command_list},
        {.name = "lcd", .desc = "list commands", .cmd = command_list},
        NULL,
    };

    command_add_command(&builtin[0]);
}

static lwnbd_plugin_t plugin = {
    .name = "shell",
    .longname = "lwnbd generic command plugin",
    .version = "0",
    .pread = command_pread,
    .pwrite = command_pwrite,
    .get_size = command_get_size,
    .block_size = command_block_size,
    .query = command_query,
    .ctor = command_ctor,
    .export_without_handle = 1,
    .load = command_load,
};

NBDKIT_REGISTER_PLUGIN(plugin)
