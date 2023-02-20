#ifndef LWNBD_PLUGIN_H
#define LWNBD_PLUGIN_H

#include <stdint.h>
//#include <unistd.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lwnbd_export
{
    void *handle; /* Plugin handle. */
                  //    struct lwnbd_plugin *p; /* Plugin that provided handle. */
    char name[32];
    char description[64]; /* optional */
    int64_t exportsize;
    //    uint16_t eflags;
    //    uint16_t blocksize;
};

struct lwnbd_plugin
{
    /* private */
    uint64_t _struct_size;

    /* public */
    const char *name;
    const char *longname;
    const char *version;
    const char *description;

    void (*load)(void);
    void (*unload)(void);

    int (*config)(const char *key, const char *value);

    /* create new export from custom config */
    int (*ctor)(const void *pconfig, struct lwnbd_export *e);

    void (*open)(void *handle, int readonly); /* not nbdkit compatible prototype */
    void (*close)(void *handle);

    int64_t (*get_size)(void *handle);

    int (*pread)(void *handle, void *buf, uint32_t count, uint64_t offset,
                 uint32_t flags);
    int (*pwrite)(void *handle, const void *buf, uint32_t count,
                  uint64_t offset, uint32_t flags);
    int (*flush)(void *handle, uint32_t flags);
    int (*trim)(void *handle, uint32_t count, uint64_t offset, uint32_t flags);
    int (*zero)(void *handle, uint32_t count, uint64_t offset, uint32_t flags);

    const char *magic_config_key;
};

#define M1(x)       x##_##plugin_init
#define FUNCINIT(x) M1(x)
#define NBDKIT_REGISTER_PLUGIN(plugin)          \
    struct lwnbd_plugin *                       \
    FUNCINIT(PLUGIN_NAME)(void)                 \
    {                                           \
        (plugin)._struct_size = sizeof(plugin); \
        return &(plugin);                       \
    }

#ifdef __cplusplus
}
#endif

#endif /* LWNBD_PLUGIN_H */
