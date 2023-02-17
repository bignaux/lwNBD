/*
 * Should not be aware of nbd protocol or transport layer
 */

#include "config.h"
#include <lwnbd.h>
#include <lwnbd-plugin.h>
//#include <stdlib.h>

typedef enum {
    PLUGIN_FREE,
    PLUGIN_CREATED,
    //	PLUGIN_INUSE,
} plugin_state_t;

static struct lwnbd_plugin *plugins[MAX_NUM_PLUGINS];
static plugin_state_t plugins_status[MAX_NUM_PLUGINS];

/*struct lwnbd_plugin *get_plugin_by_name(lwnbd_server_t const handle, const char *name)
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

int lwnbd_plugin_new(lwnbd_plugin_t const plugin, const void *pconfig)
{
    struct lwnbd_plugin *p = plugins[plugin];
    struct lwnbd_export e;

    if (p->ctor == NULL) {
        // fDEBUGLOG(stderr, "this plugin does not support configuration\n");
        return -1;
    }

    if (p->ctor(pconfig, &e) == -1) {
        return -1;
    }

    return lwnbd_add_context2(p, &e);
}

int lwnbd_plugin_config(lwnbd_plugin_t const plugin, const char *key, const char *value)
{
    struct lwnbd_plugin *p = plugins[plugin];
    const char *lkey;

    if (p->config == NULL) {
        // fDEBUGLOG(stderr, "this plugin does not support configuration\n");
        return -1;
    }

    if (key) {
        lkey = key;
    } else if (p->magic_config_key) {
        lkey = p->magic_config_key;
        DEBUGLOG("using magic_config_key %s\n", lkey);
    } else {
        DEBUGLOG("you must provide a key\n");
        return -1;
    }

    DEBUGLOG("plugin %s: %s=%s\n", p->name, lkey, value);
    if (p->config(lkey, value) == -1)
        return -1;

    return 0;
}


/* Register and load a plugin.
 * TODO : fix return err !!
 *
 * */
lwnbd_plugin_t lwnbd_plugin_init(plugin_init init)
{
    struct lwnbd_plugin *p;
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
     * plugin's own 'struct lwnbd_plugin'.
     */
    p = init();
    if (!p) {
        // fDEBUGLOG(stderr, "plugin registration function failed\n");
        return -1;
    }

    /* Check for the minimum fields which must exist in the
     * plugin struct.
     */
    //    if (p->open == NULL) {
    //        // fDEBUGLOG(stderr, "plugin must have a .open callback\n");
    //        return -1;
    //    }

    if (p->pread == NULL) {
        // fDEBUGLOG(stderr, "plugin must have a .pread callback\n");
        return -1;
    }

    if (p->load)
        p->load();

    plugins[i] = p;
    plugins_status[i] = PLUGIN_CREATED;
    DEBUGLOG("plugin %s registered\n", p->name);
    return i;
}
