#ifndef LWNBD_H
#define LWNBD_H

#include <stdint.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* contexts.c
 *
 */

typedef struct
{
    void *handle;             /* Plugin handle. */
    struct lwnbd_plugin_t *p; /* Plugin that provided handle. */
    char name[32];
    char description[64];
    int64_t exportsize;
    uint16_t eflags;
    uint32_t minimum_block_size;
    uint32_t preferred_block_size;
    uint32_t maximum_block_size;
} lwnbd_context_t;

size_t lwnbd_contexts_count();
lwnbd_context_t *lwnbd_get_context_string(const char *contextname);
lwnbd_context_t *lwnbd_get_context_i(size_t i);
int lwnbd_dump_contexts(char *s);

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

/* content plugins */

typedef uint32_t lwnbd_plugin_h;
typedef struct lwnbd_plugin_t *(*plugin_init)(void);
int lwnbd_plugin_config(lwnbd_plugin_h const plugin, const char *key, const char *value);
int lwnbd_plugin_new(lwnbd_plugin_h const plugin, const void *pconfig);
int lwnbd_plugin_news(lwnbd_plugin_h const plugin, const void *pconfig[]);
lwnbd_plugin_h lwnbd_plugin_init(plugin_init init);

/* server plugins
 *
 */

typedef uint32_t lwnbd_server_t;
typedef struct lwnbd_server *(*server_init)(void);
int lwnbd_server_config(lwnbd_server_t const handle, const char *key, const char *value);
int lwnbd_server_new(lwnbd_server_t const handle, const void *pconfig);
void lwnbd_server_start(lwnbd_server_t const handle);
int lwnbd_server_stop(lwnbd_server_t const handle);
void lwnbd_server_run(lwnbd_server_t const handle, void *client);
int lwnbd_server_dump(lwnbd_server_t const handle);
lwnbd_server_t lwnbd_server_init(server_init init);

#ifdef __cplusplus
}
#endif

#endif /* LWNBD_H */
