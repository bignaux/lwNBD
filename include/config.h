#ifndef CONFIG_H
#define CONFIG_H

#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PACKAGE_VERSION "TODO"

#define LOG(format, args...) \
    printf(APP_NAME ": " format, ##args)

#ifdef DEBUG
#define DEBUGLOG LOG
#else
#define DEBUGLOG(args...) \
    do {                  \
    } while (0)
#endif

/* server options */
#define MAX_NUM_SERVERS 2

/* nbd_protocol options */

#define MAX_RETRIES    5
#define NBD_BUFFER_LEN 512

/* plugins options - useless here ? */

#define MAX_NUM_PLUGINS 5
#define MAX_CONTEXTS    10

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
