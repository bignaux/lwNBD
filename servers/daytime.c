/*
 * RFC 867
 * port = 13
 *
 * test : telnet localhost 1313 (or nc localhost 1313
 */

//int daytime_cb()
//{
//    time_t now = time(NULL);
//    char buf[128];
//
//    if (now != -1) {
//        struct tm *tm_info = localtime(&now);
//        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S\n", tm_info);
//        fputs(buf, stdout);
//        fflush(stdout);
//    }
//
//    return 0;
//}

#include <lwnbd/lwnbd.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

static void daytime_handler(int fd, lwnbd_context_t *ctx)
{
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    write(fd, time_str, strlen(time_str));
}

static lwnbd_server_t daytime_server = {
    .name = "daytime",
    .port = 1313,
    .handler = daytime_handler,
};

NBDKIT_REGISTER_SERVER(daytime_server)
