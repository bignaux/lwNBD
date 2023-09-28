/*
 * Should not be aware of nbd protocol or transport layer
 */

#include <lwnbd.h>
#include <lwnbd-plugin.h>
#include <stdlib.h>

// workaround
extern int lwnbd_add_context(lwnbd_plugin_t *p, lwnbd_export_t *e);

typedef enum {
    PLUGIN_FREE,
    PLUGIN_CREATED,
    //	PLUGIN_INUSE,
} plugin_state_t;

static lwnbd_plugin_t *plugins[MAX_NUM_PLUGINS];
static plugin_state_t plugins_status[MAX_NUM_PLUGINS];


// int lwnbd_plugin_export(lwnbd_export_t *e)
//{
//	lwnbd_plugin_t *p = plugins[plugin];
//	return lwnbd_add_context(p, e);
// }

int lwnbd_plugin_new(lwnbd_plugin_h const plugin, const void *pconfig)
{
    lwnbd_plugin_t *p = plugins[plugin];
    lwnbd_export_t e;

    if (p->ctor == NULL) {
        return -1;
    }

    e.description[0] = '\0';
    if (p->ctor(pconfig, &e) != 0) {
        return -1;
    }

    if (e.handle == NULL) /* case for command for example */
    {
        return 0;
    }


    lwnbd_add_context(p, &e);
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
    DEBUGLOG("plugin %s registered\n", p->name);

    if (p->export_without_handle == 1) {

        lwnbd_export_t *e = malloc(sizeof(lwnbd_export_t));
        if (e == NULL)
            LOG("malloc!\n");
        e->handle = NULL;
        //		strcpy(e->description , p->description);
        strcpy(e->name, "shell");
        lwnbd_add_context(p, e);
        free(e);
    }

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
