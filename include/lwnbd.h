#ifndef LWNBD_H
#define LWNBD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* content plugins */

typedef uint32_t lwnbd_plugin_t;
typedef struct lwnbd_plugin *(*plugin_init)(void);
int lwnbd_plugin_config(lwnbd_plugin_t const plugin, const char *key, const char *value);
int lwnbd_plugin_new(lwnbd_plugin_t const plugin, const void *pconfig);
lwnbd_plugin_t lwnbd_plugin_init(plugin_init init);

/* server plugins */

typedef uint32_t lwnbd_server_t;
typedef struct lwnbd_server *(*server_init)(void);
int lwnbd_server_config(lwnbd_server_t const handle, const char *key, const char *value);
int lwnbd_server_new(lwnbd_server_t const handle, const void *pconfig);
void lwnbd_server_start(lwnbd_server_t const handle);
int lwnbd_server_stop(lwnbd_server_t const handle);
int lwnbd_server_dump(lwnbd_server_t const handle);
lwnbd_server_t lwnbd_server_init(server_init init);

extern void lwnbd_dump_contexts();

#ifdef __cplusplus
}
#endif

#endif /* LWNBD_H */
