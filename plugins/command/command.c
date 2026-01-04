#include <lwnbd/lwnbd-plugin.h>
#include <string.h>

#define PLUGIN_NAME    command
#define CMD_DRIVER_MAX 100

// temporary big, we should do zero-copy of result using nbd_buffer
#define CMD_BUFFER_SZ 4096

/*
 * Here we don't want an export by command,
 * but one export to be reachable by lwnbd_get_context()
 */

static struct lwnbd_command commands[CMD_DRIVER_MAX];
static int commands_cnt = 0;

struct post_h
{
    struct lwnbd_command *pvhandle;
    struct query_t *params;
    int nb_params;
};

static char result[CMD_BUFFER_SZ];
static int64_t exportsize;
static struct post_h pp;

/* GET */
static inline int command_pread(void *handle, void *buf, uint32_t count,
                                uint64_t offset, uint32_t flags)
{
    lwnbd_debug("count=%lu , strlen=%u\n", count, strlen(result));
    memcpy(buf, result, count);
    return 0;
}

/* POST */
static inline int command_pwrite(void *handle, const void *buf, uint32_t count,
                                 uint64_t offset, uint32_t flags)
{
    //	DEBUGLOG("command_pwrite \n");
    return pp.pvhandle->cmd(pp.nb_params + 1, (char **)&pp.params[0], buf, (int64_t *)&count);
}

static int command_block_size(void *handle,
                              uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = *preferred = *maximum = 1;
    return 0;
}

static int command_query_get(struct lwnbd_command *pvhandle, struct query_t *params, int nb_params)
{
    /* nbd client doesn't like 0 size export :
     * -> nbd.Error: nbd_pread: count cannot be 0: Invalid argument (EINVAL)
     * workaround for cmd that doesn't have result...
     */
    exportsize = 1;
    memset(result, '\0', CMD_BUFFER_SZ);

    /* TODO : disable write */
    return pvhandle->cmd(nb_params, (char **)&params[0], result, &exportsize);
}

static int command_query_post(struct lwnbd_command *pvhandle, struct query_t *params, int nb_params)
{

    /* TODO : disable read */

    /* we donno size of the incoming payload, we should find something here
     * will be size of nbd_buffer
     */
    exportsize = CMD_BUFFER_SZ;

    pp.pvhandle = pvhandle;
    pp.params = params;
    pp.nb_params = nb_params;
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
        lwnbd_debug("found no command\n");
        return -1;
    }

    while (cnt < commands_cnt) {
        //        DEBUGLOG("commands_cnt=%d\n", commands_cnt);
        if (0 == strcmp(params[0].key, commands[cnt].name)) {
            pvhandle = &commands[cnt];
            lwnbd_debug("found command : %s\n", commands[cnt].name);
            break;
        }
        cnt++;
    }

    if (pvhandle == NULL)
        return -1;

    /* dirty workaround */
    if (params[0].val == NULL)
        nb_params = 1;
    else
        nb_params = 2;


    switch (pvhandle->type) {
        case METHOD_GET:
            return command_query_get(pvhandle, params, nb_params);
        case METHOD_POST:
            return command_query_post(pvhandle, params, nb_params);
        default:
            break;
    }
    return -1;
}

static int64_t command_get_size(void *handle)
{
    return exportsize;
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
static int command_ctor(const void *pconfig, lwnbd_context_t *c)
{
    return command_add_command((struct lwnbd_command *)pconfig);
}

#ifdef CONFIG_BUILTIN_COMMAND
#include "builtin.c"

static void command_load(void)
{
    struct lwnbd_command *ptr = &builtin[0];
    lwnbd_debug("Initialize builtin commands\n");
    while (ptr->cmd) {
        //        DEBUGLOG("Initialize builtin commands %s\n", ptr->name);
        command_add_command(ptr++);
    }
}
#else
static void command_load(void) {};
#endif

static lwnbd_plugin_t plugin = {
    .name = "api",
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
