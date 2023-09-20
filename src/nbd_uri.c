/* Function : URI dispatcher
 *
 * Proof-of concept
 * https://github.com/NetworkBlockDevice/nbd/blob/master/doc/uri.md
 *
 * client support :
 *  NetworkBlockDevice/nbd pass the full URI
 *  libnbd need to URI encoding in query string ie :
 *  'nbd://127.0.0.1/some.pcm%3fvolume=70%231m30' AND compiled with libxml2 support (not the case in alpine for example)
 *
 * main interest passing "data" through URI query is that it doesn't need custom client to handle a
 * custom protocol.
 *
 *
 */

#include <lwnbd-context.h>
#include "yuarel.h"
#define MAX_QUERY 3

lwnbd_context_t *lwnbd_get_context_uri(const char *uri)
{
    lwnbd_context_t *c;
    lwnbd_export_t *e;
    lwnbd_plugin_t *p;

    DEBUGLOG("searched handler for %s.\n", uri);

    struct yuarel url;
    struct yuarel_param params[MAX_QUERY];
    char *url_str;

    /* export name is not a valid URI ? , need to start by '/'
     *
     * TODO : move to buf+1 in protocol_hs, remove -D_GNU_SOURCE
     * or strip yuarel.c ...
     *
     */
    asprintf(&url_str, "/%s", uri);

    if (-1 == yuarel_parse(&url, url_str)) {
        fprintf(stderr, "Could not parse url!\n");
        return NULL;
    }

    DEBUGLOG("Struct values:\n");
    DEBUGLOG("\tpath:\t\t%s\n", url.path);
    DEBUGLOG("\tquery:\t\t%s\n", url.query);
    DEBUGLOG("\tfragment:\t%s\n", url.fragment);

    int pa = yuarel_parse_query(url.query, '&', params, MAX_QUERY);
    int pa2 = pa;
    while (pa-- > 0) {
        DEBUGLOG("\t%s: %s\n", params[pa].key, params[pa].val);
    }

    /* we should rely on some kind of get_plugin_by_namespace()
     *
     */
    c = lwnbd_get_context(url.path);

    if (c == NULL)
        return NULL;

    p = c->p;

    /* let plugin manage query if it can
     * we'd like to get eventually a new temporary context from them, but need further change
     *
     */
    if (p->query) {
        p->query(c->handle, params, pa2);
    }

    return c;
}
