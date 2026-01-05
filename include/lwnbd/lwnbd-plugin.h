#ifndef LWNBD_PLUGIN_H
#define LWNBD_PLUGIN_H

#include <lwnbd/lwnbd-common.h>
#include <lwnbd/lwnbd.h> //workaround
//#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A struct to hold the query string parameter values. */
struct query_t
{
    char *key;
    char *val;
};

typedef struct lwnbd_plugin_t
{
    /* private */
    uint64_t _struct_size;

    /* public */
    const char *name;
    const char *longname;
    const char *version;
    const char *description;

    void (*load)(void);
    void (*unload)(void); /* not implemented */

    void (*open)(void *handle, int readonly); /* not nbdkit compatible prototype */
    void (*close)(void *handle);

    int64_t (*get_size)(void *handle);

    /*
     * NOTE: unistd.h pread/pwrite return value read/written. Here, we follow compatibility with nbdkit
     * returning 0 on success
     * ( not sure it's a great idea, that could confound contributors )
     */
    int (*pread)(void *handle, void *buf, uint32_t count, uint64_t offset,
                 uint32_t flags);
    int (*pwrite)(void *handle, const void *buf, uint32_t count,
                  uint64_t offset, uint32_t flags);

    int (*flush)(void *handle, uint32_t flags);
    int (*trim)(void *handle, uint32_t count, uint64_t offset, uint32_t flags); /* not implemented */
    int (*zero)(void *handle, uint32_t count, uint64_t offset, uint32_t flags); /* not implemented */

    /* currently only use minimum */
    int (*block_size)(void *handle,
                      uint32_t *minimum, uint32_t *preferred, uint32_t *maximum);

    int (*config)(const char *key, const char *value);
    const char *magic_config_key;

    /* lwnbd specific after nbdkit compat */

    int (*ctor)(const void *pconfig, lwnbd_context_t *c);          /* create new context from custom config */
    int (*ctrl)(void *handle, char *cmd, struct lwnbd_command *c); /* experimental */
    int (*query)(void *handle, struct query_t *params,
                 int nb_params); /* experimental */

    int export_without_handle; /* workaround */

} lwnbd_plugin_t;

#define M1(x)       x##_##plugin_init
#define FUNCINIT(x) M1(x)
#define NBDKIT_REGISTER_PLUGIN(plugin)          \
    lwnbd_plugin_t *                            \
    FUNCINIT(PLUGIN_NAME)(void)                 \
    {                                           \
        (plugin)._struct_size = sizeof(plugin); \
        return &(plugin);                       \
    }

/*
 * Default callback
 *
 */

static inline int func_no_error(void *handle, uint32_t flags)
{
    return 0;
}

static inline int char_block_size(void *handle,
                                  uint32_t *minimum, uint32_t *preferred, uint32_t *maximum)
{
    *minimum = *preferred = *maximum = 1;
    return 0;
}

static inline int64_t stream_get_size(void *handle)
{
    /**
     * since NBD doesn't support stream, we need to lie about size
     * to have enough for our session
     */
    return 0x7FFFFFFFFFFFE00; // %512 = 0 for nbd-client compat
}

#ifdef __cplusplus
}
#endif

#endif /* LWNBD_PLUGIN_H */
