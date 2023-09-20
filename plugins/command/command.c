#include <lwnbd-plugin.h>

#define PLUGIN_NAME command

static inline int command_pread(void *handle, void *buf, uint32_t count,
                                uint64_t offset, uint32_t flags)
{
    struct lwnbd_command *h = handle;
    h->cmd(h->argc, h->argc, h->result, &h->size);
    memcpy(buf, h->result, h->size);
    return 0;
}

static int64_t command_get_size(void *handle)
{
    struct lwnbd_command *h = handle;
    return h->size;
}

static int command_block_size(void *handle,
                              uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = *preferred = *maximum = 1;
    return 0;
}

static lwnbd_plugin_t plugin = {
    .name = "command",
    .version = PACKAGE_VERSION,
    .pread = command_pread,
    .get_size = command_get_size,
    .block_size = command_block_size,
};

NBDKIT_REGISTER_PLUGIN(plugin)
