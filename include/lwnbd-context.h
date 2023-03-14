#ifndef LWNBD_CONTEXT_H
#define LWNBD_CONTEXT_H

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

struct lwnbd_context
{
    void *handle;           /* Plugin handle. */
    struct lwnbd_plugin *p; /* Plugin that provided handle. */
    char name[32];
    char description[64];
    int64_t exportsize;
    uint16_t eflags;
    uint32_t minimum_block_size;
    uint32_t preferred_block_size;
    uint32_t maximum_block_size;
};

/* contexts.c */
size_t lwnbd_contexts_count();
int lwnbd_add_context(struct lwnbd_plugin *p, struct lwnbd_export *e);
struct lwnbd_context *lwnbd_get_context(const char *contextname);
struct lwnbd_context *lwnbd_get_context_uri(const char *uri, struct lwnbd_context *ctx);
struct lwnbd_context *lwnbd_get_context_i(size_t i);
void lwnbd_dump_contexts();

#ifdef __cplusplus
}
#endif

#endif /* LWNBD_CONTEXT_H */
