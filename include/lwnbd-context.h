#ifndef LWNBD_CONTEXT_H
#define LWNBD_CONTEXT_H

#include "config.h"

#include <lwnbd.h>
#include <lwnbd-plugin.h>
//#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CONTEXT_FREE,
    CONTEXT_CREATED,
    CONTEXT_FILLED,
    //	CONTEXT_INUSE,
} context_state_t;

typedef struct
{
    void *handle;      /* Plugin handle. */
    lwnbd_plugin_t *p; /* Plugin that provided handle. */
    char name[32];
    char description[64];
    int64_t exportsize;
    uint16_t eflags;
    uint32_t minimum_block_size;
    uint32_t preferred_block_size;
    uint32_t maximum_block_size;
} lwnbd_context_t;

/* contexts.c */
size_t lwnbd_contexts_count();
int lwnbd_add_context(lwnbd_plugin_t *p, lwnbd_export_t *e);
lwnbd_context_t *lwnbd_get_context(const char *contextname);

lwnbd_context_t *lwnbd_get_context_i(size_t i);
void lwnbd_dump_contexts();

#ifndef NBD_URI
lwnbd_context_t *lwnbd_get_context_uri(char *uri);
#else
static inline lwnbd_context_t *lwnbd_get_context_uri(char *uri)
{
    return lwnbd_get_context(uri);
};
#endif

#ifdef __cplusplus
}
#endif

#endif /* LWNBD_CONTEXT_H */
