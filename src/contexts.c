
#include <lwnbd/lwnbd.h>
#include <lwnbd/lwnbd-plugin.h>
#include <lwnbd/nbd-protocol.h>
//#include <stdlib.h>
#include <string.h>

typedef enum {
    CONTEXT_FREE,
    CONTEXT_CREATED,
    //    CONTEXT_FILLED,
    //	CONTEXT_INUSE,
} context_state_t;


static lwnbd_context_t contexts[MAX_CONTEXTS];
static context_state_t contexts_status[MAX_CONTEXTS];

inline int lwnbd_pread(lwnbd_context_t const *const me, void *buf, uint32_t count, uint64_t offset,
                       uint32_t flags)
{
    return (*me->p->pread)(me->handle, buf, count, offset, flags);
}

inline int lwnbd_pwrite(lwnbd_context_t const *const me, const void *buf, uint32_t count,
                        uint64_t offset, uint32_t flags)
{
    return (*me->p->pwrite)(me->handle, buf, count, offset, flags);
}

inline int lwnbd_flush(lwnbd_context_t const *const me, uint32_t flags)
{
    return (*me->p->flush)(me->handle, flags);
}

inline int lwnbd_trim(lwnbd_context_t const *const me, uint32_t count, uint64_t offset, uint32_t flags)
{
    return (*me->p->trim)(me->handle, count, offset, flags);
}

inline int lwnbd_zero(lwnbd_context_t const *const me, uint32_t count, uint64_t offset, uint32_t flags)
{
    return (*me->p->zero)(me->handle, count, offset, flags);
}

inline int lwnbd_update_size(lwnbd_context_t *me)
{
    me->exportsize = (*me->p->get_size)(me->handle);
    return (me->exportsize);
}

/*
 * Get a new context from the pool
 * return NULL on error
 */
lwnbd_context_t *lwnbd_new_context()
{
    lwnbd_context_t *c;
    uint32_t i;

    for (i = 0; i < MAX_CONTEXTS; i++) {
        if (contexts_status[i] == CONTEXT_FREE) {
            break;
        }
    }

    if (i == MAX_CONTEXTS) {
        lwnbd_info("no free context available\n");
        return NULL;
    }

    c = &contexts[i];
    contexts_status[i] = CONTEXT_CREATED;
    return c;
}

int lwnbd_dump_contexts(char *s)
{
    size_t i;
    int cnt = 0;

    for (i = 0; i < MAX_CONTEXTS; i++) {
        if (contexts_status[i] != CONTEXT_FREE) {
            cnt += sprintf(s + cnt, "%-32s: %s\n", contexts[i].name, contexts[i].description);
        }
    }
    return cnt + 1;
}

size_t lwnbd_contexts_count()
{
    size_t i;

    for (i = 0; i < MAX_CONTEXTS; i++) {
        if (contexts_status[i] == CONTEXT_FREE) {
            break;
        }
    }
    return i;
}

lwnbd_context_t *lwnbd_get_context_i(size_t i)
{
    return &contexts[i];
}

/*
 * like open() syscall
 * search for context by name
 * return NULL if not found any.
 */
lwnbd_context_t *lwnbd_get_context_string(const char *contextname)
{

    //    if ( strlen(contextname) == 0)
    //    	return NULL;
    lwnbd_debug("searched for \"%s\".\n", contextname);
    /*  a bit noob but since we don't have hole ... */
    for (uint8_t i = 0; i < lwnbd_contexts_count(); i++) {
        lwnbd_context_t *ptr_ctx = lwnbd_get_context_i(i);
        if (strncmp((ptr_ctx)->name, contextname, 32) == 0) {
            lwnbd_debug("searched for \"%s\" ... found.\n", contextname);
            return ptr_ctx;
        }
    }
    //    DEBUGLOG("searched for \"%s\" ... not found.\n", contextname);
    return NULL;
}
