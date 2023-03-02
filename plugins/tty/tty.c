#include <config.h>
#include <errno.h>

#include <lwnbd-plugin.h>
#include <string.h>
#include <thsemap.h>
#include <thevent.h>

// workaround
#undef close
#include <iomanX.h>

#define PLUGIN_NAME tty
#define DEVNAME     "tty"
#define TTY_SIZE    4096

/* poor man ring buffer */
char ringbuffer[TTY_SIZE];
static int pos = 0;
static int64_t rwhence = 0; // consumer
static int64_t wwhence = 0; // producer
static int ev = -1;

#define EF_TTY_TRANSFER_IDLE 0
#define EF_TTY_TRANSFER_BUSY (1 << 0)
#define EF_TTY_TRANSFER_DATA (1 << 1) // data to read

static int tty_init(iop_device_t *device);
static int tty_deinit(iop_device_t *device);
static int tty_stdout_fd(void);
static int tty_write(iop_file_t *file, void *buf, size_t size);
static int tty_error(void);

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

/* Ioman TTY driver.  */

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
    int res = 0;

    WaitEventFlag(ev, EF_TTY_TRANSFER_IDLE, 0, NULL);
    SetEventFlag(ev, EF_TTY_TRANSFER_BUSY);
    if (pos + size > TTY_SIZE)
        pos = 0;

    memcpy(ringbuffer + pos, buf, size);
    pos += size;
    wwhence += size;

    SetEventFlag(ev, EF_TTY_TRANSFER_DATA);
    return res;
}

static int tty_error(void)
{
    return -EIO;
}


/******************* lwNBD side *******************/

static inline int nbdtty_pread(void *handle, void *buf, uint32_t count,
                               uint64_t offset, uint32_t flags)
{
    int wait = 1;

    /*
     * TODO : hack for nbd client that try to scan data over the export like nbd-client
     */


    /*
     * hack to block here if no new data available to simulate the stream
     */
    WaitEventFlag(ev, EF_TTY_TRANSFER_DATA, 0, NULL);
    SetEventFlag(ev, EF_TTY_TRANSFER_BUSY);

    memcpy(buf, ringbuffer, count);
    rwhence += count;

    SetEventFlag(ev, EF_TTY_TRANSFER_IDLE);
    return 0;
}

/*
 * pconfig could be the .buffer + .size
 */
static int nbdtty_ctor(const void *pconfig, struct lwnbd_export *e)
{
    // TODO : move to right place
    iop_event_t event;

    event.attr = EA_MULTI;
    event.option = 0;
    event.bits = EF_TTY_TRANSFER_IDLE;

    ev = CreateEventFlag(&event);
    if (ev < 0) {
        DEBUGLOG("error creating event flag for tty: %\n", ev);
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

    strcpy(e->name, "tty");

    /**
     * since NBD doesn't support stream, we need to lie about size
     * to have enough for our session
     */
    e->exportsize = UINT64_MAX;
    return 0;
}

static int nbdtty_block_size(void *handle,
                             uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = *preferred = *maximum = 1;
    return 0;
}

// DeleteEventFlag(ev);

static struct lwnbd_plugin plugin = {
    .name = "tty",
    .longname = "lwnbd generic tty plugin",
    .version = PACKAGE_VERSION,
    .ctor = nbdtty_ctor,
    .pread = nbdtty_pread,
    //    .pwrite = tty_pwrite,
    //    .flush = tty_flush,
    .block_size = nbdtty_block_size,
};

NBDKIT_REGISTER_PLUGIN(plugin)
