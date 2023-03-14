#include "memory.h"
#include <config.h>
#include <string.h>

#define PLUGIN_NAME            memory
#define MEM_DRIVER_MAX_DEVICES 10

typedef enum {
    HANDLE_FREE,
    HANDLE_CREATED,
    //	HANDLE_INUSE,
} handle_state_t;

/* specific plugin private data */
static struct memory_config handles[MEM_DRIVER_MAX_DEVICES];
static int handle_in_use[MEM_DRIVER_MAX_DEVICES];

static inline int memory_pread(void *handle, void *buf, uint32_t count,
                               uint64_t offset, uint32_t flags)
{
    struct memory_config *h = handle;
    intptr_t addr = h->base + offset;
    memcpy(buf, (void *)addr, count);
    return 0;
}

static inline int memory_pwrite(void *handle, const void *buf, uint32_t count,
                                uint64_t offset, uint32_t flags)
{
    struct memory_config *h = handle;
    intptr_t addr = h->base + offset;
    memcpy(&addr, buf, count);
    return 0;
}

static inline int memory_flush(void *handle, uint32_t flags)
{
    return 0;
}

static int memory_ctor(const void *pconfig, struct lwnbd_export *e)
{
    uint32_t i;
    struct memory_config *h;

    for (i = 0; i < MEM_DRIVER_MAX_DEVICES; i++) {
        if (handle_in_use[i] == HANDLE_FREE) {
            handle_in_use[i] = HANDLE_CREATED;
            break;
        }
    }

    h = &handles[i];
    memcpy(h, pconfig, sizeof(struct memory_config));

    e->handle = h;
    strcpy(e->name, h->name);
    strcpy(e->description, h->desc);

    return 0;
}

static int64_t memory_get_size(void *handle)
{
    struct memory_config *h = handle;
    return h->size;
}

static int memory_block_size(void *handle,
                             uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = *preferred = *maximum = 1;
    return 0;
}

static struct lwnbd_plugin plugin = {
    .name = "memory",
    .longname = "lwnbd generic memory plugin",
    .version = PACKAGE_VERSION,
    .ctor = memory_ctor,
    .pread = memory_pread,
    .pwrite = memory_pwrite,
    .flush = memory_flush,
    .get_size = memory_get_size,
    .block_size = memory_block_size,
};

NBDKIT_REGISTER_PLUGIN(plugin)
