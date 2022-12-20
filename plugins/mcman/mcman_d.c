// MC plugin for PS2-NBD server
// alexparrado (2021)
// TODO : remove dynamic alloc

#include <intrman.h>
#include <lwnbd-plugin.h>
#include <mcman.h>
#include <stdint.h>
#include <sysmem.h>

typedef struct mcman_plugin
{
    lwnbd_plugin super;
    int device;
} mcman_plugin;

// Buffer to temporary store block data
uint8_t *bbuffer;

// Pages per block
uint16_t pagesPerBlock;

// Functions for memory allocation
void *SysAlloc(u64 size)
{
    int oldstate;
    register void *p;

    CpuSuspendIntr(&oldstate);
    p = AllocSysMemory(ALLOC_FIRST, size, NULL);
    CpuResumeIntr(oldstate);

    return p;
}

int SysFree(void *area)
{
    int oldstate;
    register int r;

    CpuSuspendIntr(&oldstate);
    r = FreeSysMemory(area);
    CpuResumeIntr(oldstate);

    return r;
}

// Function to write a block into flash
int writeBlock(int port, int block, u8 *buffer, int pageSize, int ppb)
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
int readBlock(int port, int block, u8 *buffer, int pageSize, int ppb)
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
int injectPages(int port, int block, int pageSize, int offset, int nPages, int ppb, u8 *buffer)
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
int mcman_read_(lwnbd_plugin const *const me, void *buffer, uint64_t offset, uint32_t length)
{

    int result, i;

    result = sceMcResSucceed;

    uint8_t *aux = (uint8_t *)buffer;

    // Read requested pages
    for (i = 0; i < length; i++) {
        result = McReadPage(((mcman_plugin const *)me)->device, 0, offset + i, aux + i * (me->blocksize));

        if (result != sceMcResSucceed)
            break;
    }

    return result;
}

// NBD write method
int mcman_write_(lwnbd_plugin const *const me, void *buffer, uint64_t offset, uint32_t length)
{

    int result = 0, i;
    uint32_t startBlock, endBlock;
    int nPages, blockOffset;

    int remainingPages = length;
    int writtenPages = 0;

    // Start and end blocks
    startBlock = (offset / pagesPerBlock);
    endBlock = ((offset + length - 1) / pagesPerBlock);

    // Offset in block (measured in pages)
    blockOffset = (offset & (pagesPerBlock - 1));

    // Auxiliary pointer
    uint8_t *aux = (uint8_t *)buffer;

    // Process involved blocks
    for (i = startBlock; i <= endBlock; i++) {

        // Pages to be written into block
        nPages = (remainingPages >= (pagesPerBlock - blockOffset)) ? (pagesPerBlock - blockOffset) : remainingPages;

        // Inject pages to block
        result = injectPages(((mcman_plugin const *)me)->device, i, (me->blocksize), blockOffset, nPages, pagesPerBlock, aux + writtenPages * (me->blocksize));

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
int mcman_flush_(lwnbd_plugin const *const me)
{
    return 0;
}

int mcman_register(lwnbd_plugin lwnbd_plugins, uint16_t eflags)
{
    mcman_plugin mc[2]; // For two MC ports
    int i, j, ret, successed_exported_ctx = 0;
    for (int i = 0; i < 2; i++) {
        ret = mcman_ctor(&mc[i], i);
        if (ret == 0) {
            lwnbd_plugins[successed_exported_ctx] = &mc[i].super;
            successed_exported_ctx++;
        }
    }

    return 0;
}

int mcman_ctor(mcman_plugin *const me, int device)
{

    s16 pageLen;
    // u16 pagesPerCluster;
    // u32 clustersTotal;
    u8 flags;
    int result;
    int cardSize;

    me->device = device;

    static struct lwnbd_plugin_ops const nbdopts = {
        &mcman_read_,
        &mcman_write_,
        &mcman_flush_,
    };

    // Detect MC
    McDetectCard2(device, 0);

    // Retrieve MC info
    result = McGetCardSpec(device, 0, &pageLen, &pagesPerBlock, &cardSize, &flags);


    if (result == sceMcResSucceed) {

        // Make room for block data
        bbuffer = SysAlloc(pageLen * pagesPerBlock);

        // Initializa context
        lwnbd_plugin_ctor(&me->super); /* call the superclass' ctor */
        me->super.vptr = &nbdopts;     /* override the vptr */
        // Description of export
        strcpy(me->super.export_desc, "PlayStation 2 MC via MCMAN");
        sprintf(me->super.export_name, "%s%d", "mc", me->device);
        me->super.blocksize = pageLen;
        me->super.eflags = NBD_FLAG_HAS_FLAGS;

        // Size of export
        me->super.exportsize = cardSize * pageLen;
        return 0;

    } else
        return 1;
}
