#ifndef LWNBD_H
#define LWNBD_H

#include <lwnbd/lwnbd-common.h>

#ifdef __cplusplus
extern "C" {
#endif

/* contexts
 *
 */

#define LWNBD_CONTEXT_FIELDS                                     \
    void *handle;             /* Plugin handle. */               \
    struct lwnbd_plugin_t *p; /* Plugin that provided handle. */ \
    char name[32];                                               \
    char description[64];                                        \
    int64_t exportsize;

/* The abstract base class of all contexts. */
struct lwnbd_context_s
{
    LWNBD_CONTEXT_FIELDS
};

typedef struct lwnbd_context_s lwnbd_context_t;

size_t lwnbd_contexts_count();
lwnbd_context_t *lwnbd_get_context_string(const char *contextname);
lwnbd_context_t *lwnbd_get_context_i(size_t i);
int lwnbd_dump_contexts(char *s);
lwnbd_context_t *lwnbd_new_context();

#ifndef NBD_URI
lwnbd_context_t *lwnbd_get_context(char *uri);
#else
static inline lwnbd_context_t *lwnbd_get_context(char *uri)
{
    return lwnbd_get_context_string(uri);
};
#endif

int lwnbd_pread(lwnbd_context_t const *const me, void *buf, uint32_t count, uint64_t offset,
                uint32_t flags);
int lwnbd_pwrite(lwnbd_context_t const *const me, const void *buf, uint32_t count,
                 uint64_t offset, uint32_t flags);
int lwnbd_flush(lwnbd_context_t const *const me, uint32_t flags);
int lwnbd_trim(lwnbd_context_t const *const me, uint32_t count, uint64_t offset, uint32_t flags);
int lwnbd_zero(lwnbd_context_t const *const me, uint32_t count, uint64_t offset, uint32_t flags);
int lwnbd_update_size(lwnbd_context_t *me);

/*
 * content plugins
 *
 */

typedef struct lwnbd_plugin_t *(*plugin_init)(void);
typedef uint32_t lwnbd_plugin_h;
int lwnbd_plugin_config(lwnbd_plugin_h const plugin, const char *key, const char *value);
int lwnbd_plugin_new(lwnbd_plugin_h const plugin, const void *pconfig);
int lwnbd_plugin_news(lwnbd_plugin_h const plugin, const void *pconfig[]);
lwnbd_plugin_h lwnbd_plugin_init(plugin_init init);


/*
 * server plugins
 *
 */

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
    int (*run)(void *handle, const void *client);   /* deprecated */
    void (*handler)(int client_fd, lwnbd_context_t *ctx);

    ssize_t (*sync_recv_cb)(int s, void *mem, size_t len, int flags);
    ssize_t (*sync_send_cb)(int sockfd, const void *buf, size_t len, int flags);
};

struct lwnbd_server *serverbis_init(struct lwnbd_server *srv);
int register_server(struct lwnbd_server *srv);
struct lwnbd_server *get_server_by_name(const char *name);
int lwnbd_servers_init();

typedef struct lwnbd_server *(*server_init)(void);
typedef uint32_t lwnbd_server_t;
int lwnbd_server_config(lwnbd_server_t const handle, const char *key, const char *value);
int lwnbd_server_dump(lwnbd_server_t const handle);
int lwnbd_server_new(lwnbd_server_t const handle, const void *pconfig);
int lwnbd_server_stop(lwnbd_server_t const handle);
lwnbd_server_t lwnbd_server_init(server_init init);
void lwnbd_server_run(lwnbd_server_t const handle, void *client);
void lwnbd_server_start(lwnbd_server_t const handle);

/* =============================================================
 *                EVENT LOOP (SELECT-BASED)
 * ============================================================= */

#include <time.h>

#define MAXFD 1024
#define CLIENT_TIMEOUT 60   /* seconds */

typedef void (*event_cb)(int fd, void *userdata);

struct fd_state {
    int want_read;
    int want_write;
    event_cb on_read;
    event_cb on_write;
    void *userdata;
    time_t last_activity;
};

static struct fd_state efds[MAXFD];

void event_watch_readable(int fd, event_cb cb, void *userdata);
void event_watch_writable(int fd, event_cb cb, void *userdata);
void event_remove(int fd);

#ifdef __cplusplus
}
#endif



#endif /* LWNBD_H */
