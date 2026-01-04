/* =============================================================
 *                SERVICES: CHARGEN ASYNC (RFC 864)
 * ============================================================= */

#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <lwnbd/lwnbd.h>

static const char chargen_line[] =
"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\n";

struct chargen_state {
    const char *ptr;
    size_t len;
};

void chargen_on_writable(int fd, void *userdata) {
    struct chargen_state *st = userdata;

    ssize_t n = write(fd, st->ptr, st->len);

    if (n <= 0) {
        close(fd);
        event_remove(fd);
        free(st);
        return;
    }

    if ((size_t)n < st->len) {
        st->ptr += n;
        st->len -= n;
    } else {
        st->ptr = chargen_line;
        st->len = sizeof(chargen_line) - 1;
    }

    efds[fd].last_activity = time(NULL);
    event_watch_writable(fd, chargen_on_writable, st);
}

void service_chargen_new(int fd) {
    struct chargen_state *st = malloc(sizeof(*st));
    st->ptr = chargen_line;
    st->len = sizeof(chargen_line) - 1;
    event_watch_writable(fd, chargen_on_writable, st);
}
