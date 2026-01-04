#include "ioplib.h"


/*
 * see https://github.com/AKuHAK/fps2bios/blob/master/kernel/iopload/ssbusc/ssbusc.c#L58
 *
 */
static const char *SSBUSC_NAMES[] = {
    "SSBUSC_DEV0",
    "SSBUSC_DEV_DVDROM",
    "SSBUSC_DEV_BOOTROM",
    "SSBUSC_DEV3",
    "SSBUSC_DEV_SPU",
    "SSBUSC_DEV_CDVD",
    "SSBUSC_DEV6",
    "SSBUSC_DEV7",
    "SSBUSC_DEV8",
    "SSBUSC_DEV_SPU2",
    "SSBUSC_DEV_DEV9I",
    "SSBUSC_DEV_DEV9M",
    "SSBUSC_DEV_DEV9C",
};

/* from sysman/rom.c
 * seems not reliable, biosdrain has same bug on that stuff.
 *
 * */
int GetSizeFromDelay(int device)
{
    int size = (GetDelay(device) >> 16) & 0x1F;
    return (1 << size);
}

void print_memorymap()
{
    struct memory_config mem;
    for (int i = 0; i <= 12; i++) {
        strcpy(mem.name, SSBUSC_NAMES[i]);
        mem.base = GetBaseAddress(i);
        mem.size = GetDelay(i);
        printf("%d\t\t%s\t\t\tOx%llx\t\t0x%llx\n", i, mem.name, mem.base, mem.size);
    }
}
