#ifndef LWNBD_SERVER_H
#define LWNBD_SERVER_H

#include <stdint.h>
#include <lwnbd-context.h>
// extern struct lwnbd_context contexts;
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int plugin_pread(struct lwnbd_context const *const me, void *buf, uint32_t count, uint64_t offset,
                               uint32_t flags)
{
    return (*me->p->pread)(me->handle, buf, count, offset, flags);
}

static inline int plugin_pwrite(struct lwnbd_context const *const me, const void *buf, uint32_t count,
                                uint64_t offset, uint32_t flags)
{
    return (*me->p->pwrite)(me->handle, buf, count, offset, flags);
}

static inline int plugin_flush(struct lwnbd_context const *const me, uint32_t flags)
{
    return (*me->p->flush)(me->handle, flags);
}

static inline int plugin_trim(struct lwnbd_context const *const me, uint32_t count, uint64_t offset, uint32_t flags)
{
    return (*me->p->trim)(me->handle, count, offset, flags);
}

static inline int plugin_zero(struct lwnbd_context const *const me, uint32_t count, uint64_t offset, uint32_t flags)
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

    void *(*new)(void); /* need since we could run many servers where we run only one instance of a plugin */
    int (*start)(void *handle);
    int (*stop)(void *handle);
    int (*config)(void *handle, const char *key, const char *value);
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
