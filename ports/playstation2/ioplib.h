#ifndef IOPLIB_H
#define IOPLIB_H

#include "irx_imports.h"
#include "../../plugins/memory/memory.h"

int GetSizeFromDelay(int device);
void print_memorymap();



/* TODO: manage existence */
// struct memory_config bios = {
//     .base = 0x1FC00000,
//     .size = GetSizeFromDelay(SSBUSC_DEV_BOOTROM), // 0x400000
//     .name = "bios",
//     .desc = "BIOS (rom0)",
// };

// struct memory_config iopram = {
//     .base = 0,
//     .size = QueryMemSize(),
//     .name = "ram",
//     .desc = "IOP main RAM",
// };

//    struct memory_config dvdrom = {
//        .base = GetBaseAddress(SSBUSC_DEV_DVDROM),
//        .size = GetSizeFromDelay(SSBUSC_DEV_DVDROM),
//        .name = "dvdrom",
//        .desc = "DVD-ROM rom",
//    };

// struct memory_config sif = {
//     .base = 0x1D000000,
//     .size = QueryMemSize(),
//     .name = "sif",
//     .desc = "SIF registers",
// };
//
// struct memory_config io = {
//     .base = 0x1F800000,
//     .size = QueryMemSize(),
//     .name = "io",
//     .desc = "Various I/O registers",
// };
//
// struct memory_config iopram = {
//     .base = 0x1F900000,
//     .size = QueryMemSize(),
//     .name = "spu2",
//     .desc = "SPU2 registers",
// };

#endif /* IOPLIB_H */
