#include <atad.h>
#include <config.h>
#include <lwnbd-plugin.h>

/*
 * TODO
 * - enable/disable booting from MBR ( see https://github.com/parrado/SoftDev2/blob/main/installer/install.c#L52 )
 * - nbdcopy MBR.XIN nbd://192.168.1.45/hdd0/mbr
 */

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

static int atad_ctor(const void *pconfig, lwnbd_export_t *e)
{
    int device = *(int *)pconfig;
    ata_devinfo_t *dev_info = ata_get_devinfo(device);

    if (dev_info != NULL && dev_info->exists) {
        struct handle *h = &handles[device];
        h->device = device;
        h->size = (int64_t)dev_info->total_sectors * 512;
        sprintf(e->name, "hdd%d", device);
        e->handle = h;

        return 0;

    } else
        return 1;
}

/* nbdcopy nbd://192.168.1.45/hdd0/identify - | hdparm --Istdin  */
int identify(int argc, char **argv, void *result, int64_t *size)
{
    return ata_device_sce_identify_drive(argc, result);
}
// int ata_device_idle(int device, int period);
// int ata_device_smart_get_status(int device);
// int ata_device_smart_save_attr(int device);

static int atad_ctrl(void *handle, char *path, struct lwnbd_command *cmd)
{
    struct handle *h = handle;

    if (strcmp("identify", path)) {
        cmd->cmd = identify;
        cmd->argc = h->device;
        cmd->size = 256;
        return 0;
    }

    else
        return -1;
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
    .ctrl = atad_ctrl,
};

NBDKIT_REGISTER_PLUGIN(plugin)
