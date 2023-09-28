#include <libgen.h> // TODO: remove basename() usage
#include <lwnbd-plugin.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define PLUGIN_NAME             file
#define FILE_DRIVER_MAX_DEVICES 10

/* The per-connection handle. */
struct handle
{
    int fd;
    int flags;
    char *filename;
};

typedef enum {
    HANDLE_FREE,
    HANDLE_CREATED,
    //	HANDLE_INUSE,
} handle_state_t;

/* specific plugin private data */
static struct handle handles[FILE_DRIVER_MAX_DEVICES];
static int handle_in_use[FILE_DRIVER_MAX_DEVICES];

int file_pread(void *handle, void *buf, uint32_t count, uint64_t offset,
               uint32_t flags)
{
    struct handle *h = handle;

    //    printf("file_pread, count=%d offset=%ld \n", count, offset);

    while (count > 0) {
        ssize_t r = pread(h->fd, buf, count, offset);
        if (r == -1) {
            //			nbdkit_error("pread: %m");
            return -1;
        }
        if (r == 0) { // end-of-file
            //			nbdkit_error("pread: unexpected end of file");
            return -1;
        }
        buf += r;
        count -= r;
        offset += r;

        //        printf("buffer : %s\n", (char *)buf);
    }
    return 0;
}

int file_pwrite(void *handle, const void *buf, uint32_t count, uint64_t offset,
                uint32_t flags)
{
    struct handle *h = handle;

    while (count > 0) {
        ssize_t r = pwrite(h->fd, buf, count, offset);
        if (r == -1) {
            //	    	nbdkit_error ("pwrite: %m");
            return -1;
        }
        buf += r;
        count -= r;
        offset += r;
    }
    return 0;
}

/* Flush the file to disk. */
static int file_flush(void *handle, uint32_t flags)
{
    struct handle *h = handle;

    if (fdatasync(h->fd) == -1) {
        //    nbdkit_error ("fdatasync: %m");
        return -1;
    }

    return 0;
}


/* Create the per-connection handle. */
static void file_open(void *handle, int readonly)
{
    struct handle *h = handle;

    if (readonly)
        h->flags = O_RDONLY | O_CLOEXEC;
    else
        h->flags = O_RDWR | O_CLOEXEC;

    h->fd = open(h->filename, h->flags);
    //    if (h->fd <= 0) {
    //        return -1;
    //    }
    //    return 0;
}

static int file_ctor(const void *pconfig, lwnbd_export_t *e)
{
    struct handle *h;
    char *bname;
    uint32_t i;
    const char *filename = pconfig;

    for (i = 0; i < FILE_DRIVER_MAX_DEVICES; i++) {
        if (handle_in_use[i] == HANDLE_FREE) {
            handle_in_use[i] = HANDLE_CREATED;
            break;
        }
    }

    h = &handles[i];
    bname = strdup(filename);
    h->filename = basename(bname);

    e->handle = h;
    strcpy(e->name, h->filename);
    return 0;
}

static void file_close(void *handle)
{
    struct handle *h = handle;
    close(h->fd);
}

/*
 * if no newlib, no fstat :
 fseek(h->fp, 0L, SEEK_END);
 me->super.export_size = ftell(h->fp);
 */
static int64_t file_get_size(void *handle)
{
    struct handle *h = handle;
    struct stat statbuf;

    file_open(h, 0);
    if (fstat(h->fd, &statbuf) == -1) {
        //		nbdkit_error("fstat: %m");
        return -1;
    }
    return statbuf.st_size;
}

static int file_block_size(void *handle,
                           uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = 1;
    *preferred = 1;
    *maximum = 1;
    return 0;
}

static lwnbd_plugin_t plugin = {
    .name = "file",
    .longname =
        "lwnbd file plugin",
    .version = PACKAGE_VERSION,
    .ctor = file_ctor,
    .open = file_open,
    .close = file_close,
    .pread = file_pread,
    .pwrite = file_pwrite,
    .flush = file_flush,
    .get_size = file_get_size,
    .block_size = file_block_size,
};

NBDKIT_REGISTER_PLUGIN(plugin)
