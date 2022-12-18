#include "hdd_d.h"
#include <string.h>
#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <hdd-ioctl.h>

static unsigned char IOBuffer[1024] __attribute__((aligned(64)));

static inline int hdd_read_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length)
{
    hddAtaTransfer_t *args = (hddAtaTransfer_t *)IOBuffer;

    args->lba = (uint32_t) offset;
    args->size = length / 512;

    // if (fileXioDevctl(((hdd_driver const *)me)->device, HDIOC_READSECTOR, args, sizeof(hddAtaTransfer_t), buffer, length) != 0)
    // {
    //   LOG("hdd_read error\n");
    //   return -1;
    // }
    LOG("hdd_read_\n");

    return 0;
}

static inline int hdd_write_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length)
{
    return 0;
}

static inline int hdd_flush_(nbd_context const *const me)
{
    return 0;
}

int hdd_ctor(hdd_driver *const me, int device)
{
    int ret;

    // me->device = device;
    sprintf(me->device, "%s%d:", "hdd", device);

    static struct lwnbd_operations const nbdopts = {
        &hdd_read_,
        &hdd_write_,
        &hdd_flush_,
    };
    nbd_context_ctor(&me->super); /* call the superclass' ctor */
    me->super.vptr = &nbdopts;    /* override the vptr */
    // if (fileXioDevctl(me->device, HDIOC_SCEIDENTIFY, NULL, 0, me->super.export_desc, 0) != 0)
    // {
    //   LOG("HDIOC_SCEIDENTIFY error\n");
    // }
    // else {
      strcpy(me->super.export_desc, "PlayStation 2 HDD via HDD");
    // }

    sprintf(me->super.export_name, "%s%d", "hdd", device);
    me->super.blocksize = 512;
    me->super.buffer = nbd_buffer;
    me->super.eflags = NBD_FLAG_HAS_FLAGS | NBD_FLAG_READ_ONLY;

    ret = fileXioDevctl(me->device, HDIOC_STATUS, NULL, 0, NULL, 0);
    if ((ret >= 3) || (ret < 0)) {
        LOG("%s ret %d.\n", me->super.export_name, ret);
        return -1;
    }

    me->super.export_size = (uint64_t)fileXioDevctl(me->device, HDIOC_TOTALSECTOR, NULL, 0, NULL, 0) * me->super.blocksize;
    return 0;
}
