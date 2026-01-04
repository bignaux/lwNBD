#include <atad.h>
#include <lwnbd/lwnbd-plugin.h>

struct handle
{
    int device;
    int64_t size;
};

#define PLUGIN_NAME atad
#define MAX_DEVICES 2

static struct handle handles[MAX_DEVICES];

static inline int atad_pread(void *handle, void *buf, uint32_t count,
                             uint64_t offset, uint32_t flags)
{
    struct handle *h = handle;
    //    printf("atad_pread d = %d, off = %u count %d \n", h->device,(uint32_t)offset,count);
    // ata_device_sector_io64
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

static int atad_ctor(const void *pconfig, lwnbd_context_t *c)
{
    int device = *(int *)pconfig;
    ata_devinfo_t *dev_info = ata_get_devinfo(device);

    if (dev_info != NULL && dev_info->exists) {
        struct handle *h = &handles[device];
        h->device = device;
        h->size = (int64_t)dev_info->total_sectors * 512;
        sprintf(c->name, "hdd%d", device);
        c->handle = h;

        return 0;

    } else
        return 1;
}

static int64_t atad_get_size(void *handle)
{
    struct handle *h = handle;
    return h->size;
}

static int atad_block_size(void *handle,
                           uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = 512;
    *preferred = 4096;
    *maximum = 4096;
    return 0;
}

static lwnbd_plugin_t plugin = {
    .name = "atad",
    .longname = "PlayStation 2 HDD via ATAD",
    .version = PACKAGE_VERSION,
    //    .load = atad_load,
    .ctor = atad_ctor,
    .pread = atad_pread,
    .pwrite = atad_pwrite,
    .flush = atad_flush,
    .get_size = atad_get_size,
    .block_size = atad_block_size,
    //    .ctrl = atad_ctrl,
};

NBDKIT_REGISTER_PLUGIN(plugin)
