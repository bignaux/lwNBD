#include "stdio_d.h"
#include <string.h>

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
    nbd_context_ctor(&me->super); /* call the superclass' ctor */
    me->super.vptr = &vtbl;       /* override the vptr */
    // strcpy(me->pathname, pathname);
    if ((me->fp = fopen(pathname, "r+")) == NULL) {
        perror("lwNBD: Error occurred while opening file");
        return 1;
    }

    strcpy(me->super.export_desc, "single file exporter");
    strcpy(me->super.export_name, pathname); //todo basename()
    me->super.blocksize = 512;
    me->super.buffer = nbd_buffer;
    me->super.eflags = NBD_FLAG_HAS_FLAGS | NBD_FLAG_SEND_FLUSH;

    fseek(me->fp, 0L, SEEK_END);
    me->super.export_size = ftell(me->fp);

    // 	void setbuf(FILE *stream, char *buffer)
    return 0;
}

//Todo file_close ?
