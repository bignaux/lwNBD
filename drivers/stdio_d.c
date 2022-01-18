#include "stdio_d.h"
#include <string.h>
#include <libgen.h> //basename()

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
    char *filename, *bname;
    static struct lwnbd_operations const nbdopts = {
        &file_read_,
        &file_write_,
        &file_flush_,
    };
    nbd_context_ctor(&me->super); /* call the superclass' ctor */
    me->super.vptr = &nbdopts;    /* override the vptr */
    me->pathname = strdup(pathname);
    if ((me->fp = fopen(me->pathname, "r+")) == NULL) {
        perror(" Error occurred while opening file");
        return 1;
    }

    strcpy(me->super.export_desc, "single file exporter");
    bname = strdup(pathname);
    filename = basename(bname);
    strncpy(me->super.export_name, filename, 31); //troncate name
    DEBUGLOG("bname = %s, filename = %s export = %s\n", bname, filename, me->super.export_name);
    me->super.blocksize = 512;
    me->super.buffer = nbd_buffer;
    me->super.eflags = NBD_FLAG_HAS_FLAGS | NBD_FLAG_SEND_FLUSH;

    fseek(me->fp, 0L, SEEK_END);
    me->super.export_size = ftell(me->fp);

    // 	void setbuf(FILE *stream, char *buffer)
    return 0;
}

//Todo file_close ? free(me->pathname)
