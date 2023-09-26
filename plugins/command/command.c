#include <lwnbd-plugin.h>
#include <string.h>
#include "config.h"

#define PLUGIN_NAME    command
#define CMD_DRIVER_MAX 100

/*
 * Here we don't want an export by command,
 * but one export to be reachable by lwnbd_get_context()
 */

static struct lwnbd_command commands[CMD_DRIVER_MAX];
static int commands_cnt = 0;

static inline int command_pread(void *handle, void *buf, uint32_t count,
                                uint64_t offset, uint32_t flags)
{
    //    struct lwnbd_command *h = handle;
    //    h->cmd(h->argc, h->argc, h->result, &h->size);
    //    memcpy(buf, h->result, h->size);
    return 0;
}

static inline int command_pwrite(void *handle, const void *buf, uint32_t count,
                                 uint64_t offset, uint32_t flags)
{
    //    struct memory_config *h = handle;
    //    intptr_t addr = h->base + offset;
    //    memcpy(&addr, buf, count);
    return 0;
}

static int64_t command_get_size(void *handle)
{
    //    struct lwnbd_command *h = handle;
    //    return h->size;
    return 0;
}

static int command_block_size(void *handle,
                              uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = *preferred = *maximum = 1;
    return 0;
}

/*
 * ?name=argv
 */
static int command_query(void *handle, struct query_t *params, int nb_params)
{
    int cnt = 0;
    //    get_commands();
    while (cnt < commands_cnt) {
        if (0 == strcmp(params[0].key, commands[cnt].name)) {
            void *result;
            int64_t size;
            commands[cnt].cmd(1, params[0].val, result, &size);
            break;
        }
        cnt++;
    }
    return 0;
}

/*
 * look like register RPC or callback
 */
static int command_ctor(const void *pconfig, lwnbd_export_t *e)
{
    struct lwnbd_command *h;

    if (commands_cnt == CMD_DRIVER_MAX)
        return -1;

    DEBUGLOG("coucou\n");
    h = &commands[commands_cnt++];
    memcpy(h, pconfig, sizeof(struct lwnbd_command));
    e->handle = NULL;
    return 0;
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
};

NBDKIT_REGISTER_PLUGIN(plugin)
