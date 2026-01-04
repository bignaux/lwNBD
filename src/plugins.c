/*
 * Should not be aware of nbd protocol or transport layer
 */

#include <lwnbd/lwnbd.h>
#include <lwnbd/lwnbd-plugin.h>

typedef enum {
    PLUGIN_FREE,
    PLUGIN_CREATED,
    //	PLUGIN_INUSE,
} plugin_state_t;

static lwnbd_plugin_t *plugins[MAX_NUM_PLUGINS];
static plugin_state_t plugins_status[MAX_NUM_PLUGINS];

int lwnbd_plugin_new(lwnbd_plugin_h const plugin, const void *pconfig)
{
    lwnbd_plugin_t *p = plugins[plugin];
    lwnbd_context_t *c;

    if (p->ctor == NULL) {
        return -1;
    }

    if (p->export_without_handle == 1) {
        if (p->ctor(pconfig, NULL) != 0) {
            return -1;
        }
        return 0;
    }

    c = lwnbd_new_context();

    c->description[0] = '\0';
    if (p->ctor(pconfig, c) != 0) {
        return -1;
    }

    c->p = p;

    if (!strlen(c->description))
        strcpy(c->description, p->longname);

    lwnbd_debug("Add context %s: %s 0x" PRI_UINT64 " %p\n", c->name, c->description, PRI_UINT64_C_Val(c->exportsize), c);
    return 0;
}

int lwnbd_plugin_news(lwnbd_plugin_h const plugin, const void *pconfig[])
{
    int i = 0;
    while (pconfig != NULL) {
        lwnbd_plugin_new(plugin, pconfig[i++]);
    }
    return 0;
}

int lwnbd_plugin_config(lwnbd_plugin_h const plugin, const char *key, const char *value)
{
    lwnbd_plugin_t *p = plugins[plugin];
    const char *lkey;

    if (p->config == NULL) {
        // DEBUGLOG(stderr, "this plugin does not support configuration\n");
        return -1;
    }

    if (key) {
        lkey = key;
    } else if (p->magic_config_key) {
        lkey = p->magic_config_key;
        lwnbd_debug("using magic_config_key %s\n", lkey);
    } else {
        lwnbd_debug("you must provide a key\n");
        return -1;
    }

    lwnbd_debug("plugin %s: %s=%s\n", p->name, lkey, value);
    if (p->config(lkey, value) == -1)
        return -1;

    return 0;
}


/* Register and load a plugin.
 * TODO : fix return err !!
 *
 * */
lwnbd_plugin_h lwnbd_plugin_init(plugin_init init)
{
    lwnbd_plugin_t *p;
    uint32_t i;

    for (i = 0; i < MAX_NUM_PLUGINS; i++) {
        if (plugins_status[i] == PLUGIN_FREE) {
            break;
        }
    }

    if (i == MAX_NUM_PLUGINS) {
        return -1;
    }

    /* Call the initialization function which returns the address of the
     * plugin's own 'lwnbd_plugin_t'.
     */
    p = init();
    if (!p) {
        // DEBUGLOG(stderr, "plugin registration function failed\n");
        return -1;
    }

    /* Check for the minimum fields which must exist in the
     * plugin struct.
     */
    //    if (p->open == NULL) {
    //        // DEBUGLOG(stderr, "plugin must have a .open callback\n");
    //        return -1;
    //    }

    if (p->pread == NULL) {
        // DEBUGLOG(stderr, "plugin must have a .pread callback\n");
        return -1;
    }

    if (p->load)
        p->load();

    plugins[i] = p;
    plugins_status[i] = PLUGIN_CREATED;

    if (p->export_without_handle == 1) {

        lwnbd_context_t *c = lwnbd_new_context();
        if (c == NULL)
            lwnbd_info("lwnbd_new_context!\n");
        c->handle = NULL;
        strcpy(c->description, p->longname);
        strcpy(c->name, "api"); /* TODO: configurable endpoint */
        c->p = p;
    }

    lwnbd_debug("plugin %s registered\n", p->name);
    return i;
}

/*lwnbd_plugin_t *get_plugin_by_name(lwnbd_server_t const handle, const char *name)
{
    for (uint32_t i = 0; i < MAX_NUM_PLUGINS; i++) {
        if (plugins_status[i] == PLUGIN_CREATED) {
            if ( ! strcmp(plugins[i]->name, name) )
            {
//				DEBUGLOG("found plugin at index %d\n",i);
                return plugins[i];
            }
        }
    }
    return NULL;
}*/
