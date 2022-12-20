#include <atad.h>
#include <config.h>
#include <lwnbd-plugin.h>

#define PLUGIN_NAME atad
#define MAX_DEVICES 2
static struct lwnbd_plugin plugin;
struct handle
{
    int device;
};

struct handle handles[MAX_DEVICES];

static inline int atad_pread(void *handle, void *buf, uint32_t count,
                             uint64_t offset, uint32_t flags)
{
    struct handle *h = handle;
    return ata_device_sector_io(h->device, buf, (uint32_t)offset, count,
                                ATA_DIR_READ);
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

void atad_load(void)
{
    for (uint8_t i = 0; i < MAX_DEVICES; i++) {
        ata_devinfo_t *dev_info = ata_get_devinfo(i);
        char name[5];
        if (dev_info != NULL && dev_info->exists) {
            struct handle *h = &handles[i];
            h->device = i;
            int64_t exportsize = (int64_t)dev_info->total_sectors * 512;
            sprintf(name, "hdd%d", i & MAX_DEVICES);
            lwnbd_add_context(h, &plugin, name, plugin.longname, exportsize);
        }
    }
}

static struct lwnbd_plugin plugin = {
    .name = "atad",
    .longname = "PlayStation 2 HDD via ATAD",
    .version = PACKAGE_VERSION,
    .load = atad_load,
    .pread = atad_pread,
    .pwrite = atad_pwrite,
    .flush = atad_flush,
};

NBDKIT_REGISTER_PLUGIN(plugin)
