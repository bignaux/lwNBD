#include <atad.h>
#include <config.h>
#include <lwnbd-plugin.h>

#define PLUGIN_NAME atad
#define MAX_DEVICES 2

struct handle
{
    int device;
};

static struct handle handles[MAX_DEVICES];
// static int handle_in_use[MAX_DEVICES];

static inline int atad_pread(void *handle, void *buf, uint32_t count,
                             uint64_t offset, uint32_t flags)
{
    struct handle *h = handle;
    //    printf("atad_pread d = %d, off = %u count %d \n", h->device,(uint32_t)offset,count);
    return ata_device_sector_io(h->device, buf, (uint32_t)offset, count, ATA_DIR_READ);
}

static inline int atad_pwrite(void *handle, const void *buf, uint32_t count,
                              uint64_t offset, uint32_t flags)
{
    struct handle *h = handle;
    return ata_device_sector_io(h->device, (void *)buf, (uint32_t)offset, count,
                                ATA_DIR_WRITE);
}

static inline int atad_flush(void *handle, uint32_t flags)
{
    struct handle *h = handle;
    return ata_device_flush_cache(h->device);
}

static int atad_ctor(const void *pconfig, struct lwnbd_export *e)
{
    int device = *(int *)pconfig;
    ata_devinfo_t *dev_info = ata_get_devinfo(device);

    if (dev_info != NULL && dev_info->exists) {
        struct handle *h = &handles[device];
        h->device = device;
        e->exportsize = (int64_t)dev_info->total_sectors * 512;
        sprintf(e->name, "hdd%d", device);
        e->handle = h;

        return 0;

    } else
        return 1;
}

static int atad_block_size(void *handle,
                           uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = 512;
    *preferred = 4096;
    *maximum = 4096;
    return 0;
}

static struct lwnbd_plugin plugin = {
    .name = "atad",
    .longname = "PlayStation 2 HDD via ATAD",
    .version = PACKAGE_VERSION,
    //    .load = atad_load,
    .ctor = atad_ctor,
    .pread = atad_pread,
    .pwrite = atad_pwrite,
    .flush = atad_flush,
    .block_size = atad_block_size,
};

NBDKIT_REGISTER_PLUGIN(plugin)
