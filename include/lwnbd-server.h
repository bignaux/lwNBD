#ifndef LWNBD_SERVER_H
#define LWNBD_SERVER_H

#include <lwnbd-common.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

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
