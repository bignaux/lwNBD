/*
 * RFC 867
 * port = 13
 */

#include <stdio.h>
#include <time.h>

int daytime_cb()
{
    time_t now = time(NULL);
    char buf[128];

    if (now != -1) {
        struct tm *tm_info = localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S\n", tm_info);
        fputs(buf, stdout);
        fflush(stdout);
    }

    return 0;
}
