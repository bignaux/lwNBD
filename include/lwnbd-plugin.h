#ifndef LWNBD_PLUGIN_H
#define LWNBD_PLUGIN_H

#include <stdint.h>
//#include <unistd.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char name[32];
    char description[64]; /* optional */

    /* lwnbd specific */
    void *handle; /* Plugin handle. */
} lwnbd_export_t;

/* experimental */
struct lwnbd_command
{
    int (*cmd)(int argc, char **argv, void *result, int64_t *size);
    int argc;
    char **argv;
    void *result; /* return of cmd() */
    int64_t size; /* size of result */
};

/* A struct to hold the query string parameter values. */
struct query_param
{
    char *key;
    char *val;
};

typedef struct
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

    int (*ctor)(const void *pconfig, lwnbd_export_t *e);           /* create new export from custom config */
    int (*ctrl)(void *handle, char *cmd, struct lwnbd_command *c); /* experimental */
    int (*query)(void *handle, struct query_param *params,
                 int nb_params); /* experimental */

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
