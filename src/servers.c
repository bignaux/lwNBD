#include "config.h"
#include <lwnbd-server.h>
//#include <stdlib.h>
#include <stdio.h>
// TODO: https://blog.mbedded.ninja/programming/general/control-methodology/a-function-pointer-based-state-machine/

/* equivalent of context for plugins */
struct server_instance
{
    void *handle;
    struct lwnbd_server *s;
};

static struct server_instance servers[MAX_NUM_SERVERS];
static server_state_t servers_status[MAX_NUM_SERVERS];

/* SRV_FREE -> SRV_STOPPED */
// TODO: fix return !!
lwnbd_server_t lwnbd_server_init(server_init init)
{
    struct server_instance *si;
    struct lwnbd_server *s;
    uint32_t i;

    for (i = 0; i < MAX_NUM_SERVERS; i++) {
        if (servers_status[i] == SRV_FREE) {
            break;
        }
    }

    if (i == MAX_NUM_SERVERS) {
        return -1;
    }

    /* Call the initialization function which returns the address of the
     * server own 'struct lwnbd_server'.
     */
    s = init();
    if (!s) {
        // DEBUGLOG(stderr, "server registration function failed\n");
        return -1;
    }

    /* Check for the minimum fields which must exist in the
     * server struct.
     */
    if (s->start == NULL) {
        // DEBUGLOG(stderr, "server must have a .start callback\n");
        return -1;
    }

    if (s->new == NULL) {
        // DEBUGLOG(stderr, "server must have a .new callback\n");
        return -1;
    }

    si = &servers[i];
    si->handle = s->new ();
    si->s = s;

    servers_status[i] = SRV_STOPPED;
    DEBUGLOG("server %s init\n", s->name);
    return i;
}

/*
 * value would be void * so no need conversion from internal ? a bit NIH getopt but pretty good data driven stuff.
 */
int lwnbd_server_config(lwnbd_server_t const handle, const char *key, const char *value)
{
    struct server_instance *si = &servers[handle];
    struct lwnbd_server *s = si->s;

    DEBUGLOG("server %s: %s=%s\n", si->s->name, key, value);
    if (s->config == NULL) {
        DEBUGLOG("this server does not support configuration\n");
        return -1;
    }

    return (s->config(si->handle, key, value));
}


/* on nbdkit, this is done in main */
// int lwnbd_server_plugin_config(lwnbd_server_t const handle, const char *name, const char *key, const char *value)
//{
//	struct lwnbd_plugin *p = nbd_server_get_plugin_by_name(handle, name);
//
//	if (!p)
//	{
//		DEBUGLOG("plugin not found\n");
//		return -1;
//	}
//
//	if (key)
//	{
//		plugin_config(p, key, value);
//		return 0;
//	}
//	else if (p->magic_config_key)
//	{
////		DEBUGLOG("using magic_config_key %s\n",p->magic_config_key);
//		plugin_config(p, p->magic_config_key, value);
//		return 0;
//	}
//	else
//	{
//		DEBUGLOG("you must provide a key\n");
//		return -1;
//	}
//}

int lwnbd_server_dump(lwnbd_server_t const handle)
{
    struct server_instance *si = &servers[handle];
    struct lwnbd_server *s = si->s;

    DEBUGLOG("%s ready...\n", s->name);

    //    for (uint32_t i = 0; i < MAX_NUM_PLUGINS; i++)
    //    {
    //    	if (servers[handle].plugins_status[i] != PLUGIN_FREE) {
    //    		DEBUGLOG("%s", servers[handle].plugins[i]->name);
    //    	    for (uint32_t j = 0; j < servers[handle].plugins[i]->devices_cnt ; j++)
    //    	    {
    ////    	    	DEBUGLOG(" %s ,",  (lwnbd_plugin) servers[handle].plugins[i]->devices[j]->export_name) ;
    //    	    }
    //    	}
    //    }
    return 0;
}

/* SRV_STOPPED -> SRV_STARTED */
int lwnbd_server_start(lwnbd_server_t const handle)
{
    struct server_instance *si = &servers[handle];
    struct lwnbd_server *s = si->s;
    server_state_t st = servers_status[handle];

    if (st != SRV_STOPPED)
        return -1;

    st = SRV_STARTED;
    s->start(si->handle);
    return 0;
}

int lwnbd_server_stop(lwnbd_server_t const handle)
{
    // TODO
    servers_status[handle] = SRV_STOPPED;
    return 0;
}

// TODO
int lwnbd_exit()
{
    // eventually server_stop()
    // cleanup() unload() plugins.
    return 0;
}
