/*
 * WIP
 * loader.c
 *
 * idea of implementation, for doc purpose
 * zero-copy, difficulty is to make loadelf reentrant.
 *
 */

#include <string.h>
#include <sifrpc.h>
#include <kernel.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>

#include "elf.h"

// Loader ELF variables
extern u8 loader_elf[];
extern int size_loader_elf;

// ELF-loading stuff
#define ELF_MAGIC   0x464c457f
#define ELF_PT_LOAD 1

static void wipe_bramMem(void)
{
    int i;
    for (i = 0x00084000; i < 0x100000; i += 64) {
        asm volatile(
            "\tsq $0, 0(%0) \n"
            "\tsq $0, 16(%0) \n"
            "\tsq $0, 32(%0) \n"
            "\tsq $0, 48(%0) \n" ::"r"(i));
    }
}

static int loadelf(int argc, char **argv, void *data, int64_t *size)
{

    u8 *boot_elf;
    elf_header_t *eh;
    elf_pheader_t *eph;
    void *pdata;
    int i;
    int new_argc = argc + 2;

    /* NB: LOADER.ELF is embedded  */
    boot_elf = (u8 *)loader_elf;
    eh = (elf_header_t *)boot_elf;
    if (_lw((u32)&eh->ident) != ELF_MAGIC)
        asm volatile("break\n");

    eph = (elf_pheader_t *)(boot_elf + eh->phoff);

    /* Scan through the ELF's program headers and copy them into RAM, then zero out any non-loaded regions.  */
    for (i = 0; i < eh->phnum; i++) {
        if (eph[i].type != ELF_PT_LOAD)
            continue;

        pdata = (void *)(boot_elf + eph[i].offset);
        memcpy(eph[i].vaddr, pdata, eph[i].filesz);

        if (eph[i].memsz > eph[i].filesz)
            memset((void *)((u8 *)(eph[i].vaddr) + eph[i].filesz), 0,
                   eph[i].memsz - eph[i].filesz);
    }

    /* Let's go.  */
    SifExitRpc();
    FlushCache(0);
    FlushCache(2);

    return ExecPS2((void *)eh->entry, NULL, new_argc, new_argv);
}
