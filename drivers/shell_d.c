#include "shell_d.h"
#include <string.h>

int shell_read_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length)
{
    size_t ret;
    shell_driver const *const me_ = (shell_driver const *)me;
    fseek(me_->fp, offset, SEEK_SET);
    ret = fread(buffer, me->blocksize, length, me_->fp);
    return ((ret == length) ? 0 : 1);
}

int shell_write_(nbd_context const *const me, void *buffer, uint64_t offset, uint32_t length)
{
    size_t ret;
    shell_driver const *const me_ = (shell_driver const *)me;
    fseek(me_->fp, offset, SEEK_SET);
    ret = fwrite(buffer, me->blocksize, length, me_->fp);
    return ((ret == length) ? 0 : 1);
}

//  flush would require setbuf...
static inline int shell_flush_(nbd_context const *const me)
{
    return -1;
}

int shell_ctor(shell_driver *const me, const char *pathname)
{
    static struct lwnbd_operations const nbdopts = {
        &shell_read_,
        &shell_write_,
        &shell_flush_,
    };
    nbd_context_ctor(&me->super); /* call the superclass' ctor */
    me->super.vptr = &nbdopts;    /* override the vptr */

    strcpy(me->super.export_desc, "simple shell");
    strcpy(me->super.export_name, "shell");
    me->super.blocksize = 1;
    me->super.buffer = nbd_buffer;
    me->super.eflags = NBD_FLAG_HAS_FLAGS; //  | NBD_FLAG_SEND_FLUSH;

    me->super.export_size = ftell(me->fp);

    return 0;
}

//Todo shell_close ?
