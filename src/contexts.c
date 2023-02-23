#include "config.h"

#include <lwnbd-context.h>
#include <nbd-protocol.h>
//#include <stdlib.h>
#include <string.h>

static struct lwnbd_context contexts[MAX_CONTEXTS];
static context_state_t contexts_status[MAX_CONTEXTS];

int lwnbd_add_context(struct lwnbd_plugin *p, struct lwnbd_export *e)
{
    struct lwnbd_context *c;
    uint32_t i;
    uint16_t eflags = NBD_FLAG_HAS_FLAGS;

    for (i = 0; i < MAX_CONTEXTS; i++) {
        if (contexts_status[i] == CONTEXT_FREE) {
            break;
        }
    }

    if (i == MAX_CONTEXTS) {
        return -1;
    }
    DEBUGLOG("lwnbd_add_context %s\n", e->name);

    c = &contexts[i];

    c->handle = e->handle;
    c->p = p;
    strcpy(c->name, e->name);

    //    printf("len %ld\n", strlen(e->description));

    if (strlen(e->description))
        strcpy(c->description, e->description);
    else
        strcpy(c->description, p->longname);

    c->exportsize = e->exportsize;

    if (!p->pwrite)
        eflags |= NBD_FLAG_READ_ONLY;

    if (p->flush)
        eflags |= NBD_FLAG_SEND_FLUSH;

    c->eflags = eflags;
    contexts_status[i] = CONTEXT_CREATED;
    return 0;
}

// TODO: workaround
// struct lwnbd_context *lwnbd_contexts_return()
//{
//	return contexts;
//}

void lwnbd_dump_contexts()
{
    size_t i;

    for (i = 0; i < MAX_CONTEXTS; i++) {
        if (contexts_status[i] != CONTEXT_FREE) {
            printf("%s : %s\n", contexts[i].name, contexts[i].description);
        }
    }
}


size_t
lwnbd_contexts_count()
{
    size_t i;

    for (i = 0; i < MAX_CONTEXTS; i++) {
        if (contexts_status[i] == CONTEXT_FREE) {
            break;
        }
    }
    return i;
}

struct lwnbd_context
    *
    lwnbd_get_context_i(size_t i)
{
    return &contexts[i];
}

/* search for context by name
 * (TODO: if contextname is NULL, search for default one if set.)
 * return NULL if not found any.
 */
struct lwnbd_context *lwnbd_get_context(const char *contextname)
{
    struct lwnbd_context *ptr_ctx = contexts;

    while (ptr_ctx) {
        if (strncmp((ptr_ctx)->name, contextname, 32) == 0) {
            //             DEBUGLOG("searched for \"%s\" ... found.\n", contextname);
            return ptr_ctx;
        }
        ptr_ctx++;
    }
    DEBUGLOG("searched for \"%s\" ... not found.\n", contextname);
    return NULL;
}

// const struct nbdkit_context context = nbdkit_get_context (exps, i);
