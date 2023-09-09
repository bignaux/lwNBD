#include <config.h>
#include <string.h>
#include "pcmstream.h"

#include <audsrv.h>

#define PLUGIN_NAME            pcmstream
#define PCM_DRIVER_MAX_DEVICES 1

typedef enum {
    HANDLE_FREE,
    HANDLE_CREATED,
    //	HANDLE_INUSE,
} handle_state_t;

/* specific plugin private data */
static struct pcmstream_config handles[PCM_DRIVER_MAX_DEVICES];
static int handle_in_use[PCM_DRIVER_MAX_DEVICES];

static inline int pcmstream_pread(void *handle, void *buf, uint32_t count,
                                  uint64_t offset, uint32_t flags)
{
    return 0;
}

static inline int pcmstream_pwrite(void *handle, const void *buf, uint32_t count,
                                   uint64_t offset, uint32_t flags)
{
    //    struct pcmstream_config *h = handle;

    audsrv_wait_audio(count); /* blocking !! */
    audsrv_play_audio(buf, count);

    return 0;
}

static inline int pcmstream_flush(void *handle, uint32_t flags)
{
    return 0;
}

static int pcmstream_ctor(const void *pconfig, struct lwnbd_export *e)
{
    uint32_t i;
    struct pcmstream_config *h;

    for (i = 0; i < PCM_DRIVER_MAX_DEVICES; i++) {
        if (handle_in_use[i] == HANDLE_FREE) {
            handle_in_use[i] = HANDLE_CREATED;
            break;
        }
    }

    h = &handles[i];
    memcpy(h, pconfig, sizeof(struct pcmstream_config));

    e->handle = h;
    strcpy(e->name, h->name);
    strcpy(e->description, h->desc);

    if (audsrv_init() != 0) {
        LOG("Failed to initialize audsrv: %s\n", audsrv_get_error_string());
        return -1;
    }

    audsrv_set_format(h->rate, h->bits, h->channels);
    audsrv_set_volume(h->volume);

    return 0;
}

static int64_t pcmstream_get_size(void *handle)
{
    /**
     * since NBD doesn't support stream, we need to lie about size
     * to have enough for our session
     */
    return 0x7FFFFFFFFFFFE00; // %512 = 0 for nbd-client compat
}

static int pcmstream_block_size(void *handle,
                                uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = *preferred = *maximum = 1;
    return 0;
}

static struct lwnbd_plugin plugin = {
    .name = "pcmstream",
    .longname = "lwnbd generic pcmstream plugin",
    .version = PACKAGE_VERSION,
    .ctor = pcmstream_ctor,
    .pread = pcmstream_pread,
    .pwrite = pcmstream_pwrite,
    .flush = pcmstream_flush,
    .get_size = pcmstream_get_size,
    .block_size = pcmstream_block_size,
};

// audsrv_quit();

NBDKIT_REGISTER_PLUGIN(plugin)
