
#include <lwnbd-context.h>
#include <nbd-protocol.h>
//#include <stdlib.h>
#include <string.h>

static lwnbd_context_t contexts[MAX_CONTEXTS];
static context_state_t contexts_status[MAX_CONTEXTS];

int lwnbd_add_context(lwnbd_plugin_t *p, lwnbd_export_t *e)
{
    lwnbd_context_t *c;
    uint32_t i;
    uint16_t eflags = NBD_FLAG_HAS_FLAGS;
    uint32_t minimum, preferred, maximum;

    for (i = 0; i < MAX_CONTEXTS; i++) {
        if (contexts_status[i] == CONTEXT_FREE) {
            break;
        }
    }

    if (i == MAX_CONTEXTS) {
        return -1;
    }

    c = &contexts[i];

    c->handle = e->handle;
    c->p = p;
    strcpy(c->name, e->name);

    //        printf("len %ld\n", strlen(e->description));

    if (strlen(e->description))
        strcpy(c->description, e->description);
    else
        strcpy(c->description, p->longname);

    if (p->get_size)
        c->exportsize = p->get_size(c->handle);

    if (!p->pwrite)
        eflags |= NBD_FLAG_READ_ONLY;

    if (p->flush)
        eflags |= NBD_FLAG_SEND_FLUSH;
    c->eflags = eflags;

    if (p->block_size) {
        p->block_size(c->handle, &minimum, &preferred, &maximum);
    } else
        minimum = preferred = maximum = 512;

    c->minimum_block_size = minimum;
    c->preferred_block_size = preferred;
    c->maximum_block_size = maximum;

    //	printf("preferred_block_size %u\n", c->preferred_block_size);

    contexts_status[i] = CONTEXT_CREATED;
    DEBUGLOG("Add context %s: %s 0x" PRI_UINT64 " %p\n", c->name, c->description, PRI_UINT64_C_Val(c->exportsize), c);
    return 0;
}

// TODO: workaround
// lwnbd_context_t *lwnbd_contexts_return()
//{
//	return contexts;
//}

void lwnbd_dump_contexts()
{
    size_t i;

    for (i = 0; i < MAX_CONTEXTS; i++) {
        if (contexts_status[i] != CONTEXT_FREE) {
            LOG("%-32s: %s\n", contexts[i].name, contexts[i].description);
        }
    }
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

/* search for context by name
 * return NULL if not found any.
 */
lwnbd_context_t *lwnbd_get_context(const char *contextname)
{

    //    if ( strlen(contextname) == 0)
    //    	return NULL;
    DEBUGLOG("searched for \"%s\".\n", contextname);
    /*  a bit noob but since we don't have hole ... */
    for (uint8_t i = 0; i < lwnbd_contexts_count(); i++) {
        lwnbd_context_t *ptr_ctx = lwnbd_get_context_i(i);
        if (strncmp((ptr_ctx)->name, contextname, 32) == 0) {
            DEBUGLOG("searched for \"%s\" ... found.\n", contextname);
            return ptr_ctx;
        }
    }
    //    DEBUGLOG("searched for \"%s\" ... not found.\n", contextname);
    return NULL;
}
