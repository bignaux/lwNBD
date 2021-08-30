#include <stdio.h>
#include <stdlib.h>
#include "nbd_server.h"

#include <string.h>
#include <errno.h>

typedef struct file_driver
{
    nbd_context super;
    //TODO: char *pathname;
    FILE *fp;
} file_driver;

int file_read_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length)
{
    size_t ret;
    file_driver const *const me_ = (file_driver const *)me;
    fseek(me_->fp, offset, SEEK_SET);
    ret = fread(buffer, me->blocksize, length, me_->fp);
    return ((ret == length) ? 0 : 1);
}

int file_write_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length)
{
    size_t ret;
    file_driver const *const me_ = (file_driver const *)me;
    fseek(me_->fp, offset, SEEK_SET);
    ret = fwrite(buffer, me->blocksize, length, me_->fp);
    return ((ret == length) ? 0 : 1);
}

static inline int file_flush_(nbd_context const *const me)
{
    return fflush(((file_driver const *)me)->fp);
}

int file_ctor(file_driver *const me, const char *pathname)
{
    static struct nbd_context_Vtbl const vtbl = {
        &file_read_,
        &file_write_,
        &file_flush_,
    };

    me->super.vptr = &vtbl; /* override the vptr */
    // strcpy(me->pathname, pathname);
    if ((me->fp = fopen(pathname, "r+")) == NULL) {
        perror("lwNBD: Error occurred while opening file");
        return 1;
    }

    strcpy(me->super.export_desc, "single file exporter");
    strcpy(me->super.export_name, pathname);
    me->super.blocksize = 512;
    me->super.buffer = nbd_buffer;
    me->super.eflags = NBD_FLAG_HAS_FLAGS;

    fseek(me->fp, 0L, SEEK_END);
    me->super.export_size = ftell(me->fp);

    // 	void setbuf(FILE *stream, char *buffer)
    return 0;
}

//Todo file_close ?

int main(int argc, char **argv)
{
    int ret;
    int successed_exported_ctx = 0;
    struct file_driver fakedrive[10];
    nbd_context *nbd_contexts[10];
    // nbd_context **ptr_ctx = nbd_contexts;

    if (argc <= 2) {
        printf("Usage ./lwNDB <files>\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        ret = file_ctor(&fakedrive[successed_exported_ctx], argv[i]);
        if (ret == 0) {
            nbd_contexts[successed_exported_ctx] = &fakedrive[successed_exported_ctx].super;
            successed_exported_ctx++;
        }
    }
    nbd_contexts[successed_exported_ctx] = NULL;

    //Platform specific block device detection then nbd_context initialization go here
    // while (*ptr_ctx) {
    //     if ((*ptr_ctx)->export_init(*ptr_ctx) != 0) {
    //         printf("lwnbdsvr: failed to init %s driver!\n", (*ptr_ctx)->export_name);
    //     } else {
    //         printf("lwnbdsvr: export %s\n", (*ptr_ctx)->export_desc);
    //         successed_exported_ctx++;
    //     }
    //     ptr_ctx++;
    // }
    //

    if (!successed_exported_ctx) {
        printf("lwNBD: nothing to export.\n");
        exit(EXIT_FAILURE);
    }
    printf("lwNBD: init %d exports.\n", successed_exported_ctx);

    nbd_init(nbd_contexts);
    exit(EXIT_SUCCESS);
}
