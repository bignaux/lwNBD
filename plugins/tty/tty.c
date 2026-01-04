#include <errno.h>
#include <iomanX.h>
#include <lwnbd/lwnbd-plugin.h>
#include <string.h>
#include <thevent.h>

#define PLUGIN_NAME tty
#define DEVNAME     "tty"
#define TTY_SIZE    4096

/* poor man ring buffer */
char ringbuffer[TTY_SIZE + 1];
static int64_t rwhence = 0; // consumer
static int64_t wwhence = 0; // producer
static int ev = -1;

#define EF_TTY_TRANSFER_IDLE 0
#define EF_TTY_TRANSFER_BUSY (1 << 0)
#define EF_TTY_TRANSFER_DATA (1 << 1) // data to read

/* Ioman TTY driver part */

static int tty_init(iop_device_t *device)
{
    return 0;
}

static int tty_deinit(iop_device_t *device)
{
    return 0;
}

static int tty_stdout_fd(void)
{
    return 1;
}

static int tty_write(iop_file_t *file, void *buf, size_t size)
{
    int pos = wwhence % TTY_SIZE;

    WaitEventFlag(ev, EF_TTY_TRANSFER_IDLE, 0, NULL);
    SetEventFlag(ev, EF_TTY_TRANSFER_BUSY);
    // very stupid rb , verify at least rwhence !
    if (pos + size > TTY_SIZE) {
        int sz = TTY_SIZE - pos;
        memcpy(ringbuffer + pos, buf, sz);

        buf += sz;
        sz = size - sz;
        memcpy(ringbuffer, buf, sz);
    } else
        memcpy(ringbuffer + pos, buf, size);

    wwhence += size;

    SetEventFlag(ev, EF_TTY_TRANSFER_DATA);
    return 0;
}

static int tty_error(void)
{
    return -EIO;
}

/* device ops */
static iop_device_ops_t tty_ops = {
    tty_init,
    tty_deinit,
    (void *)tty_error,
    (void *)tty_stdout_fd,
    (void *)tty_stdout_fd,
    (void *)tty_error,
    (void *)tty_write,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
};

/* device descriptor */
static iop_device_t tty_device = {
    DEVNAME,
    IOP_DT_CHAR | IOP_DT_CONS,
    1,
    "TTY via NBD",
    &tty_ops,
};


/* lwNBD TTY driver part */

static inline int nbdtty_pread(void *handle, void *buf, uint32_t count,
                               uint64_t offset, uint32_t flags)
{
    int pos = rwhence % TTY_SIZE;

    /*
     * hack for nbd clients that try to scan data over the export like nbd-client
     */
    //    if (offset == 0)           // offset reseted
    //    	rwhence = 0;
    //    else if (offset > rwhence) // scan data detected
    //    {
    //    	memset(buf, 0, count);
    //    	return 0;
    //    }
    //    else if (offset < rwhence) // would not happend ?
    //    	rwhence = offset;

    /*
     * hack to block here if not enough new data available to simulate the stream
     */
    while (1) {
        WaitEventFlag(ev, EF_TTY_TRANSFER_DATA, 0, NULL);
        SetEventFlag(ev, EF_TTY_TRANSFER_BUSY);

        // not enough data to read
        if (rwhence + count > wwhence) {
            SetEventFlag(ev, EF_TTY_TRANSFER_IDLE);
            continue;
        } else
            break;
    }

    if (pos + count > TTY_SIZE) {
        int sz = TTY_SIZE - pos;
        memcpy(buf, ringbuffer + pos, sz);

        buf += sz;
        sz = count - sz;
        memcpy(buf, ringbuffer, sz);
    } else
        memcpy(buf, ringbuffer + pos, count);

    rwhence += count;

    if (rwhence < wwhence) // still data to read
        SetEventFlag(ev, EF_TTY_TRANSFER_DATA);
    else
        SetEventFlag(ev, EF_TTY_TRANSFER_IDLE);

    return 0;
}

/*
 * pconfig could be the .buffer + .size, a path ...
 */
static int nbdtty_ctor(const void *pconfig, lwnbd_context_t *c)
{
    // TODO : move to right place
    iop_event_t event;

    event.attr = EA_MULTI;
    event.option = 0;
    event.bits = EF_TTY_TRANSFER_IDLE;

    ev = CreateEventFlag(&event);
    if (ev < 0) {
        lwnbd_debug("error creating event flag for tty: %\n", ev);
        return -1;
    }

    close(0);
    close(1);
    DelDrv(tty_device.name);

    if (AddDrv(&tty_device) < 0)
        return -1;

    open(DEVNAME "00:", 0x1000 | O_RDWR);
    open(DEVNAME "00:", O_WRONLY);

    printf("NBDTTY loaded! 0x%p\n", ringbuffer);

    strcpy(c->name, "tty");

    return 0;
}

static int nbdtty_block_size(void *handle,
                             uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = *preferred = *maximum = 1;
    return 0;
}

// DeleteEventFlag(ev);

static lwnbd_plugin_t plugin = {
    .name = "tty",
    .longname = "lwnbd generic tty plugin",
    .version = PACKAGE_VERSION,
    .ctor = nbdtty_ctor,
    .pread = nbdtty_pread,
    //    .pwrite = tty_pwrite,
    //    .flush = tty_flush,
    .get_size = stream_get_size,
    .block_size = nbdtty_block_size,
};

NBDKIT_REGISTER_PLUGIN(plugin)
