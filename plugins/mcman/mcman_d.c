// MC plugin for PS2-NBD server
// alexparrado (2021) & ronan bignaux (2023)
// TODO : remove dynamic alloc
// TODO : add slot to pconfig ?
// TODO : format e->description according to Memory Card device types and port/slot

#include <intrman.h>
#include <lwnbd-plugin.h>
#include <mcman.h>
#include <stdint.h>
#include <sysmem.h>

#define PLUGIN_NAME mcman
#define MAX_DEVICES 2

struct handle
{
    int device;
    int64_t size;
    uint16_t blocksize;
};

static struct handle handles[MAX_DEVICES];
// static int handle_in_use[MAX_DEVICES];

// Buffer to temporary store block data
uint8_t *bbuffer;

// Pages per block
uint16_t pagesPerBlock;

// Functions for memory allocation
static void *SysAlloc(u64 size)
{
    int oldstate;
    register void *p;

    CpuSuspendIntr(&oldstate);
    p = AllocSysMemory(ALLOC_FIRST, size, NULL);
    CpuResumeIntr(oldstate);

    return p;
}

/*
static int SysFree(void *area)
{
    int oldstate;
    register int r;

    CpuSuspendIntr(&oldstate);
    r = FreeSysMemory(area);
    CpuResumeIntr(oldstate);

    return r;
}
*/

// Function to write a block into flash
static int writeBlock(int port, int block, u8 *buffer, int pageSize, int ppb)
{
    int j;
    uint8_t ecc[16];
    int result;
    memset(ecc, 0, 16);

    // Block erasing before writing (NAND flash technology)
    result = McEraseBlock2(port, 0, block, NULL, NULL);

    if (result != sceMcResSucceed)
        return result;

    // For each page in block
    for (j = 0; j < ppb; j++) {
        // Compute checksum
        McDataChecksum(buffer + j * pageSize, &ecc[0]);
        McDataChecksum(buffer + j * pageSize + 128, &ecc[3]);
        McDataChecksum(buffer + j * pageSize + 256, &ecc[6]);
        McDataChecksum(buffer + j * pageSize + 384, &ecc[9]);

        // Write page
        result = McWritePage(port, 0, block * ppb + j, buffer + j * pageSize, ecc);

        if (result != sceMcResSucceed)
            return result;
    }

    return result;
}

// Function to read a block from flash
static int readBlock(int port, int block, u8 *buffer, int pageSize, int ppb)
{
    int j;
    int result;

    for (j = 0; j < ppb; j++) {
        result = McReadPage(port, 0, block * ppb + j, buffer + j * pageSize);
        if (result != sceMcResSucceed)
            return result;
    }

    return result;
}

// Function to inject pages to flash
static int injectPages(int port, int block, int pageSize, int offset, int nPages, int ppb, u8 *buffer)
{
    int result;

    // If whole block is not gonna be written
    if ((offset != 0) || (nPages != ppb)) {
        result = readBlock(port, block, bbuffer, pageSize, ppb);
        if (result != sceMcResSucceed)
            return result;
    }

    // Inject pages to buffer
    memcpy(bbuffer + offset * pageSize, buffer, nPages * pageSize);

    // Write buffer to block
    result = writeBlock(port, block, bbuffer, pageSize, ppb);

    return result;
}

// NBD read method
static int mcman_pread(void *handle, void *buf, uint32_t count, uint64_t offset, uint32_t flags)
{
    struct handle *h = handle;
    int result, i;

    result = sceMcResSucceed; // = 0

    uint8_t *aux = (uint8_t *)buf;

    // Read requested pages
    for (i = 0; i < count; i++) {
        result = McReadPage(h->device, 0, offset + i, aux + i * (h->blocksize));

        if (result != sceMcResSucceed)
            break;
    }

    return result;
}

// NBD write method
static int mcman_pwrite(void *handle, const void *buf, uint32_t count,
                        uint64_t offset, uint32_t flags)
{
    struct handle *h = handle;
    int result = 0, i;
    uint32_t startBlock, endBlock;
    int nPages, blockOffset;

    int remainingPages = count;
    int writtenPages = 0;

    // Start and end blocks
    startBlock = (offset / pagesPerBlock);
    endBlock = ((offset + count - 1) / pagesPerBlock);

    // Offset in block (measured in pages)
    blockOffset = (offset & (pagesPerBlock - 1));

    // Auxiliary pointer
    uint8_t *aux = (uint8_t *)buf;

    // Process involved blocks
    for (i = startBlock; i <= endBlock; i++) {

        // Pages to be written into block
        nPages = (remainingPages >= (pagesPerBlock - blockOffset)) ? (pagesPerBlock - blockOffset) : remainingPages;

        // Inject pages to block
        result = injectPages(h->device, i, (h->blocksize), blockOffset, nPages, pagesPerBlock, aux + writtenPages * (h->blocksize));

        if (result != sceMcResSucceed)
            break;
        // For next block
        blockOffset = 0;
        remainingPages -= nPages;
        writtenPages += nPages;
    }

    return result;
}

// TODO
static int mcman_flush(void *handle, uint32_t flags)
{
    return 0;
}

static int mcman_ctor(const void *pconfig, lwnbd_export_t *e)
{
    uint8_t device = *(uint8_t *)pconfig;

    s16 pageLen;
    // u16 pagesPerCluster;
    // u32 clustersTotal;
    u8 flags;
    int result;
    int cardSize;

    // Detect MC
    McDetectCard2(device, 0);

    // Retrieve MC info
    result = McGetCardSpec(device, 0, &pageLen, &pagesPerBlock, &cardSize, &flags);

    if (result == sceMcResSucceed) {

        struct handle *h = &handles[device];
        e->handle = h;
        h->device = device;

        // Make room for block data
        bbuffer = SysAlloc(pageLen * pagesPerBlock); // TODO: not here

        // Description of export
        sprintf(e->name, "mc%d", device);
        h->blocksize = pageLen;

        // Size of export
        h->size = (int64_t)cardSize * pageLen;

        return 0;

    } else
        return 1;
}

static int64_t mcman_get_size(void *handle)
{
    struct handle *h = handle;
    return h->size;
}

static int mcman_block_size(void *handle,
                            uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    //    struct handle *h = handle;
    *minimum = 512;   // h->blocksize;
    *preferred = 512; // h->blocksize
    *maximum = 512;   // h->blocksize;
    return 0;
}

static lwnbd_plugin_t plugin = {
    .name = "mcman",
    .longname = "PlayStation 2 MemoryCard via MCMAN",
    .version = PACKAGE_VERSION,
    //    .load = mcman_load,
    .ctor = mcman_ctor,
    .pread = mcman_pread,
    .pwrite = mcman_pwrite,
    .flush = mcman_flush,
    .get_size = mcman_get_size,
    .block_size = mcman_block_size,
};

NBDKIT_REGISTER_PLUGIN(plugin)
