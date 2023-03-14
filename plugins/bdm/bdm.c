#include <bdm.h>
#include <config.h>
#include <lwnbd-plugin.h>

#define PLUGIN_NAME     bdm
#define MAX_CONNECTIONS 20 // would be in ps2sdk bdm.h

struct handle
{
    struct block_device *device;
};

static struct handle handles[MAX_CONNECTIONS];
// static int handle_in_use[MAX_DEVICES];

static inline int bdm_pread(void *handle, void *buf, uint32_t count,
                            uint64_t offset, uint32_t flags)
{
    struct handle *h = handle;
    struct block_device *device = h->device;
    return device->read(device, (u32)offset, buf, (u16)count);
}

static inline int bdm_pwrite(void *handle, const void *buf, uint32_t count,
                             uint64_t offset, uint32_t flags)
{
    return 0;
}

static inline int bdm_flush(void *handle, uint32_t flags)
{
    return 0;
}

static int bdm_ctor(const void *pconfig, struct lwnbd_export *e)
{
    struct block_device **pbd;
    struct handle *h = &handles[0];

    bdm_get_bd(pbd, 1);

    //    for (uint8_t i = 0; i < MAX_CONNECTIONS; i++) {
    //        //	    if()
    //        //	    	continue;
    //
    //        DEBUGLOG("%s %u %u %c %u\n", pbd[i]->name, pbd[i]->devNr, pbd[i]->parNr, pbd[i]->parId, pbd[i]->sectorSize * pbd[i]->sectorCount);
    //    }


    h->device = pbd[0];
    strcpy(e->name, pbd[0]->name);
    e->handle = h;

    //    bdm_connect_bd(h->device);

    return 0;
}

void unload()
{
    // void bdm_disconnect_bd(struct block_device *bd);
}

static int bdm_block_size(void *handle,
                          uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    struct handle *h = handle;
    struct block_device *device = h->device;
    *minimum = *preferred = *maximum = device->sectorSize;
    return 0;
}

static int64_t bdm_get_size(void *handle)
{
    struct handle *h = handle;
    struct block_device *d = h->device;
    return (int64_t)d->sectorSize * d->sectorCount;
}

static struct lwnbd_plugin plugin = {
    .name = "bdm",
    .longname = "PlayStation 2 BDM plugin",
    .version = PACKAGE_VERSION,
    //    .load = bdm_load,
    .ctor = bdm_ctor,
    .pread = bdm_pread,
    //    .pwrite = bdm_pwrite,
    //    .flush = bdm_flush,
    .get_size = bdm_get_size,
    .block_size = bdm_block_size,
};

NBDKIT_REGISTER_PLUGIN(plugin)
