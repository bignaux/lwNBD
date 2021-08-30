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

int file_read_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length);
int file_write_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length);
int file_flush_(nbd_context const *const me);

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
    me->super.blockshift = 9;
    me->super.buffer = nbd_buffer;

    fseek(me->fp, 0L, SEEK_END);
    me->super.export_size = ftell(me->fp);

    // 	void setbuf(FILE *stream, char *buffer)
    return 0;
}

int file_read_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length)
{
    size_t ret;
    file_driver const *const me_ = (file_driver const *)me;
    fseek(me_->fp, offset, SEEK_SET);
    ret = fread(buffer, 512, length, me_->fp);
    return ((ret == length) ? 0 : 1);
}

int file_write_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length)
{
    size_t ret;
    file_driver const *const me_ = (file_driver const *)me;
    fseek(me_->fp, offset, SEEK_SET);
    ret = fwrite(buffer, 512, length, me_->fp);
    return ((ret == length) ? 0 : 1);
}

int file_flush_(nbd_context const *const me)
{
    file_driver const *const me_ = (file_driver const *)me;
    return fflush(me_->fp);
}

//Todo file_close ?

int main(int argc, char **argv)
{
    int ret;
    int successed_exported_ctx = 0;
    struct file_driver fakedrive;
    nbd_context *nbd_contexts[] = {
        &fakedrive.super,
        NULL,
    };

    if (argc != 2) {
        printf("Usage ./lwNDB <file>\n");
        exit(EXIT_FAILURE);
    }

    ret = file_ctor(&fakedrive, argv[1]);
    if (ret == 0)
        successed_exported_ctx = 1;

    /*
    nbd_context **ptr_ctx = nbd_contexts;

    //Platform specific block device detection then nbd_context initialization go here
    while (*ptr_ctx) {
        if ((*ptr_ctx)->export_init(*ptr_ctx) != 0) {
            printf("lwnbdsvr: failed to init %s driver!\n", (*ptr_ctx)->export_name);
        } else {
            printf("lwnbdsvr: export %s\n", (*ptr_ctx)->export_desc);
            successed_exported_ctx++;
        }
        ptr_ctx++;
    }
    */

    if (!successed_exported_ctx) {
        printf("lwNBD: nothing to export.\n");
        exit(EXIT_FAILURE);
    }
    printf("lwNBD: init nbd_contexts ok.\n");

    nbd_init(nbd_contexts);
    exit(EXIT_SUCCESS);
}
