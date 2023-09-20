/*
                .-.
 ___  ___  ___ ( __)    .-..
(   )(   )(   )(''")   /    \
 | |  | |  | |  | |   ' .-,  ;
 | |  | |  | |  | |   | |  . |
 | |  | |  | |  | |   | |  | |
 | |  | |  | |  | |   | |  | |
 | |  ; '  | |  | |   | |  ' |
 ' `-'   `-' '  | |   | `-'  '
  '.__.'.__.'  (___)  | \__.'
                      | |
                     (___)
 */

#include <lwnbd-plugin.h>
#include <string.h>
#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <hdd-ioctl.h>
#include <stdio.h>
#include "config.h"

#define PLUGIN_NAME libhdd
#define MAX_DEVICES 2
static unsigned char IOBuffer[1024] __attribute__((aligned(64)));
static lwnbd_plugin_t plugin;

struct handle
{
    char filename[6];
};

struct handle handles[MAX_DEVICES];

static inline int hdd_read(void *handle, void *buf, uint32_t count,
                           uint64_t offset, uint32_t flags)
{
    hddAtaTransfer_t *args = (hddAtaTransfer_t *)IOBuffer;
    struct handle *h = handle;

    args->lba = (uint32_t)offset;
    args->size = count / 512;

    if (fileXioDevctl(h->filename, HDIOC_READSECTOR, args, sizeof(hddAtaTransfer_t), buf, count) != 0)
        return -1;

    return 0;
}

static inline int hdd_write(void *handle, const void *buf, uint32_t count,
                            uint64_t offset, uint32_t flags)
{
    return 0;
}

static inline int hdd_flush(void *handle, uint32_t flags)
{
    return 0;
}

void hdd_load(void)
{
    for (uint8_t i = 0; i < MAX_DEVICES; i++) {
        int ret;
        struct handle *h = &handles[i];
        sprintf(h->filename, "hdd%d:", i & MAX_DEVICES);
        ret = fileXioDevctl(h->filename, HDIOC_STATUS, NULL, 0, NULL, 0);
        if ((ret >= 3) || (ret < 0))
            break;
        // if (fileXioDevctl(me->device, HDIOC_SCEIDENTIFY, NULL, 0, me->super.export_desc, 0) != 0)
        // {
        //   LOG("HDIOC_SCEIDENTIFY error\n");
        // }
        int64_t exportsize = (int64_t)fileXioDevctl(h->filename, HDIOC_TOTALSECTOR,
                                                    NULL, 0, NULL, 0) *
                             512;

        lwnbd_add_context(h, &plugin, h->filename, plugin.longname, exportsize);
    }
}

static lwnbd_plugin_t plugin = {
    .name = "libhdd",
    .longname = "PlayStation 2 HDD via libhdd",
    .version = PACKAGE_VERSION,
    .load = hdd_load,
    .pread = hdd_read,
    //    .pwrite = hdd_pwrite,
    //    .flush = hdd_flush,
};

NBDKIT_REGISTER_PLUGIN(plugin)
