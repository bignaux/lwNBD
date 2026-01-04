#include <lwnbd/lwnbd-server.h>
#include <lwnbd/lwnbd.h>
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


static int server_count = 0;
static struct lwnbd_server *servers_bis[MAX_NUM_SERVERS]; // replace with server_instance ?

#ifdef CONFIG_CHARGEN_SERVER
//extern int chargen_cb();
//struct lwnbd_server chargen = {
//    .name = "chargen",
//    //    .config = nbd_config,
//    //    .ctor = nbd_ctor,
//    .run = chargen_cb,
//    //    .new = nbd_new,
//};
#endif

#ifdef CONFIG_DAYTIME_SERVER
//extern int daytime_cb();
//struct lwnbd_server daytime = {
//    .name = "daytime",
//    //    .config = nbd_config,
//    //    .ctor = nbd_ctor,
//    .run = daytime_cb,
//    //    .new = nbd_new,
//};
#endif

#ifdef CONFIG_NBD_SERVER
extern int nbd_synchronous_client_thread_cb(void *handle, const void *client) static struct lwnbd_server server = {
    .name = "nbd",
    .config = nbd_config,
    .ctor = nbd_ctor,
    .run = nbd_synchronous_client_thread_cb,
    .new = nbd_new,

    .sync_recv_cb = tcp_recv_block,
    .sync_send_cb = send,
};
#endif

int lwnbd_servers_init()
{
#ifdef CONFIG_CHARGEN_SERVER
    serverbis_init(&chargen);
    register_server(&chargen);
    lwnbd_debug("chargen ready\n");
#endif

#ifdef CONFIG_DAYTIME_SERVER
    serverbis_init(&daytime);
    register_server(&daytime);
    lwnbd_debug("daytime ready\n");
#endif


    return 0;
}

int register_server(struct lwnbd_server *srv)
{
    if (server_count >= MAX_NUM_SERVERS) {
        lwnbd_debug("Error: too many servers\n");
        return -1;
    }

    servers_bis[server_count++] = srv;
    return 0;
}

struct lwnbd_server *serverbis_init(struct lwnbd_server *srv)
{
    srv->_struct_size = sizeof(*srv);
    return srv;
}
struct lwnbd_server *get_server_by_name(const char *name)
{

    for (int i = 0; i < server_count; i++) {
        if (strcmp(servers_bis[i]->name, name) == 0) {
            //            servers_bis[i]->_struct_size = sizeof(*servers_bis[i]);
            lwnbd_debug("server  found\n");
            return servers_bis[i];
        }
    }
    lwnbd_debug("server not found\n");
    return NULL;
}

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
        lwnbd_debug("MAX_NUM_SERVERS limit\n");
        return -1;
    }

    /* Call the initialization function which returns the address of the
     * server own 'struct lwnbd_server'.
     */
    s = init();
    if (!s) {
        lwnbd_debug("server registration function failed\n");
        return -1;
    }

    /* Check for the minimum fields which must exist in the
     * server struct.
     */
    //    if (s->start == NULL) {
    //        // DEBUGLOG(stderr, "server must have a .start callback\n");
    //        return -1;
    //    }

    if (s->new == NULL) {
        lwnbd_debug("server must have a .new callback\n");
        return -1;
    }

    si = &servers[i];
    si->handle = s->new ();
    si->s = s;

    servers_status[i] = SRV_STOPPED;
    lwnbd_debug("server %s init id=%lu\n", s->name, i);
    return i;
}

/*
 * value would be void * so no need conversion from internal ? a bit NIH getopt but pretty good data driven stuff.
 */
int lwnbd_server_config(lwnbd_server_t const handle, const char *key, const char *value)
{
    struct server_instance *si = &servers[handle];
    struct lwnbd_server *s = si->s;

    lwnbd_debug("server %s: %s=%s\n", si->s->name, key, value);
    if (s->config == NULL) {
        lwnbd_debug("this server does not support configuration\n");
        return -1;
    }

    return (s->config(si->handle, key, value));
}

int lwnbd_server_new(lwnbd_server_t const handle, const void *pconfig)
{
    struct server_instance *si = &servers[handle];
    struct lwnbd_server *s = si->s;

    return (s->ctor(si->handle, pconfig));
}

/* on nbdkit, this is done in main */
// int lwnbd_server_plugin_config(lwnbd_server_t const handle, const char *name, const char *key, const char *value)
//{
//	lwnbd_plugin_t *p = nbd_server_get_plugin_by_name(handle, name);
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

    (void)s->name;
    lwnbd_debug("%s ready...\n", s->name);

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

void lwnbd_server_run(lwnbd_server_t const handle, void *client)
{
    struct server_instance *si = &servers[handle];
    struct lwnbd_server *s = si->s;
    server_state_t st = servers_status[handle];

    if (st != SRV_STOPPED)
        return;

    st = SRV_STARTED;
    s->run(si->handle, client);
}

/* SRV_STOPPED -> SRV_STARTED */
// void lwnbd_server_start(lwnbd_server_t const handle)
//{
//     struct server_instance *si = &servers[handle];
//     struct lwnbd_server *s = si->s;
//     server_state_t st = servers_status[handle];
//
//     if (st != SRV_STOPPED)
//         return;
//
//     st = SRV_STARTED;
//     s->start(si->handle);
// }

int lwnbd_server_stop(lwnbd_server_t const handle)
{
    // TODO SRV_STOPPED
    struct server_instance *si = &servers[handle];
    struct lwnbd_server *s = si->s;
    s->stop(si->handle);

    servers_status[handle] = SRV_FREE;
    return 0;
}

// TODO
int lwnbd_exit()
{
    // eventually server_stop()
    // cleanup() unload() plugins.
    return 0;
}


/* =============================================================
 *                EVENT LOOP (SELECT-BASED)
 * ============================================================= */

#include <time.h>

void event_watch_readable(int fd, event_cb cb, void *userdata) {
    efds[fd].want_read = 1;
    efds[fd].on_read   = cb;
    efds[fd].userdata  = userdata;
    efds[fd].last_activity = time(NULL);
}

void event_watch_writable(int fd, event_cb cb, void *userdata) {
    efds[fd].want_write = 1;
    efds[fd].on_write   = cb;
    efds[fd].userdata   = userdata;
    efds[fd].last_activity = time(NULL);
}

void event_remove(int fd) {
    efds[fd].want_read = 0;
    efds[fd].want_write = 0;
    efds[fd].on_read = NULL;
    efds[fd].on_write = NULL;
    efds[fd].userdata = NULL;
    efds[fd].last_activity = 0;
}



