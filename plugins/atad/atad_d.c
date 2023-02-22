#include <atad.h>
#include <config.h>
#include <lwnbd-plugin.h>

#define PLUGIN_NAME atad
#define MAX_DEVICES 2

/* Error definitions.
#define ATA_RES_ERR_NOTREADY -501
#define ATA_RES_ERR_TIMEOUT  -502
#define ATA_RES_ERR_IO       -503
#define ATA_RES_ERR_NODATA   -504
#define ATA_RES_ERR_NODEV    -505
#define ATA_RES_ERR_CMD      -506
#define ATA_RES_ERR_LOCKED   -509
#define ATA_RES_ERR_ICRC     -510
*/

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
    printf("h->device = %d\n",h->device);
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

static int atad_ctor(const void *pconfig, struct lwnbd_export *e)
{
    uint8_t device = *(uint8_t *)pconfig;
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

static struct lwnbd_plugin plugin = {
    .name = "atad",
    .longname = "PlayStation 2 HDD via ATAD",
    .version = PACKAGE_VERSION,
    //    .load = atad_load,
    .ctor = atad_ctor,
    .pread = atad_pread,
    .pwrite = atad_pwrite,
    .flush = atad_flush,
};

NBDKIT_REGISTER_PLUGIN(plugin)
