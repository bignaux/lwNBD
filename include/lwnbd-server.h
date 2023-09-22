#ifndef LWNBD_SERVER_H
#define LWNBD_SERVER_H

#include <stdint.h>
#include <lwnbd-context.h>
// extern lwnbd_context_t contexts;
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int plugin_pread(lwnbd_context_t const *const me, void *buf, uint32_t count, uint64_t offset,
                               uint32_t flags)
{
    return (*me->p->pread)(me->handle, buf, count, offset, flags);
}

static inline int plugin_pwrite(lwnbd_context_t const *const me, const void *buf, uint32_t count,
                                uint64_t offset, uint32_t flags)
{
    return (*me->p->pwrite)(me->handle, buf, count, offset, flags);
}

static inline int plugin_flush(lwnbd_context_t const *const me, uint32_t flags)
{
    return (*me->p->flush)(me->handle, flags);
}

static inline int plugin_trim(lwnbd_context_t const *const me, uint32_t count, uint64_t offset, uint32_t flags)
{
    return (*me->p->trim)(me->handle, count, offset, flags);
}

static inline int plugin_zero(lwnbd_context_t const *const me, uint32_t count, uint64_t offset, uint32_t flags)
{
    return (*me->p->zero)(me->handle, count, offset, flags);
}

typedef enum {
    SRV_FREE,
    SRV_STOPPED,
    SRV_STARTED,
} server_state_t;

struct lwnbd_server
{
    /* private */
    uint64_t _struct_size;

    /* public */
    const char *name;

    void *(*new)(void); /* need since we could run many servers where we run only one instance of a plugin, anyway, why not (*open) ?
    plus "new" is reserved C++ keyword ... */
    int (*start)(void *handle);
    int (*stop)(void *handle);
    int (*config)(void *handle, const char *key, const char *value);
    int (*ctor)(void *handle, const void *pconfig); /* config with custom structure */
    int (*run)(void *handle, void *client);
};

#define SERVERINIT(x)  x##_##server_init
#define SERVERINIT2(x) SERVERINIT(x)
#define NBDKIT_REGISTER_SERVER(server)          \
    struct lwnbd_server *                       \
    SERVERINIT2(NAME)(void)                     \
    {                                           \
        (server)._struct_size = sizeof(server); \
        return &(server);                       \
    }

#ifdef __cplusplus
}
#endif

#endif /* LWNBD_SERVER_H */
