#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "lwnbd.h"
#include "drivers/stdio_d.h"

struct file_driver fakedrive[10];

//TODO; int lwnbdSetDefaultExport(char * export_name)
//TODO: int lookuptable(char * export_name)
//TODO: mask,rw
int lwnbdInitExport(nbd_context **nbd_contexts, int argc, char **argv)
{
    int ret, successed_exported_ctx = 0;
    for (int i = 1; i < argc; i++) {
        ret = file_ctor(&fakedrive[successed_exported_ctx], argv[i]);
        if (ret == 0) {
            nbd_contexts[successed_exported_ctx] = &fakedrive[successed_exported_ctx].super;
            successed_exported_ctx++;
        }
    }
    nbd_contexts[successed_exported_ctx] = NULL;
    return successed_exported_ctx;
}

int main(int argc, char **argv)
{
    int successed_exported_ctx = 0;
    nbd_context *nbd_contexts[10];

    strcpy(gdefaultexport, "README.md");
    nbd_context *dfexp;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    successed_exported_ctx = lwnbdInitExport(nbd_contexts, argc, argv);

    dfexp = nbd_context_getDefaultExportByName(nbd_contexts, gdefaultexport);
    if (dfexp != NULL)
        LOG("default export found : %s\n", dfexp->export_name);

    if (!successed_exported_ctx) {
        LOG("nothing to export.\n");
        exit(EXIT_FAILURE);
    }
    LOG("init %d exports.\n", successed_exported_ctx);
    nbd_init(nbd_contexts);
    exit(EXIT_SUCCESS);
}
