/* Function : URI dispatcher
 *
 * Proof-of concept
 * https://github.com/NetworkBlockDevice/nbd/blob/master/doc/uri.md
 *
 * client support :
 *  NetworkBlockDevice/nbd pass the full URI
 *  libnbd need to URI encoding in query string ie :
 *  'nbd://127.0.0.1/some.pcm%3fvolume=70%231m30' AND compiled with libxml2 support
 *  https://0mg.github.io/tools/uri/ doesn't validate this kind of mixed URI
 *
 * main interest passing "data" through URI query is that it doesn't need custom client to handle a
 * custom protocol.
 *
 *
 */

#include <lwnbd/lwnbd.h>
#include <lwnbd/lwnbd-plugin.h>
#include "yuarel.h"
#define MAX_QUERY 3

lwnbd_context_t *lwnbd_get_context(char *uri)
{
    lwnbd_context_t *c;
    lwnbd_plugin_t *p;

    //    if (!uri) {
    //        return lwnbd_get_defaultexport();
    //    }

    lwnbd_debug("searched handler for %s.\n", uri);

    struct yuarel url;
    struct yuarel_param params[MAX_QUERY];

    if (-1 == yuarel_parse(&url, uri)) {
        lwnbd_info("Could not parse url!\n");
        return NULL;
    }

    lwnbd_debug("Struct values:\n");
    lwnbd_debug("\tpath:\t\t%s\n", url.path);
    lwnbd_debug("\tquery:\t\t%s\n", url.query);
    lwnbd_debug("\tfragment:\t%s\n", url.fragment);

    int pa = yuarel_parse_query(url.query, '&', params, MAX_QUERY);
    int pa2 = pa;
    while (pa-- > 0) {
        lwnbd_debug("\t%s: %s\n", params[pa].key, params[pa].val);
    }

    /* we should rely on some kind of get_plugin_by_namespace()
     * plugin visibility without export ?
     * scheme support ? file:// http:// rpc://
     *
     */
    c = lwnbd_get_context_string(url.path);

    if (!c)
        return NULL;

    p = c->p;

    /* let plugin manage query if it can
     * we'd like to get eventually a new temporary context from them, but need further change
     *
     */
    if (p->query) {
        int ret = p->query(c->handle, (struct query_t *)params, pa2);
        if (ret) {
            lwnbd_info("query failed\n");
            return NULL;
        }
    }

    /*
     * last minute lwnbd_update_size() allows to get size on freshly created context
     * according request
     */
    lwnbd_update_size(c);

    return c;
}
