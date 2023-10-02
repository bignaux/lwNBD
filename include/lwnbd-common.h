/*
 *
 */

#ifndef INCLUDE_LWNBD_COMMON_H_
#define INCLUDE_LWNBD_COMMON_H_

#include "config.h"
#include <stdint.h>

/*
 *
 */

/* experimental */
//#ifdef PLUGIN_COMMAND

typedef enum {
    METHOD_GET,
    METHOD_POST,
} method_type;

struct lwnbd_command
{
    char *name;
    char *desc;
    method_type type;
    int (*cmd)(int argc, char **argv, void *data, int64_t *size);
};

extern struct lwnbd_plugin_t *command_plugin_init(void);
//#endif

//#ifdef PLUGIN_MEMORY
extern struct lwnbd_plugin_t *memory_plugin_init(void);
struct memory_config
{
    intptr_t base;
    intptr_t size;
    char name[32];
    char desc[64]; /* export description */
    /* TODO readonly ? */
};
//#endif

//#ifdef PLUGIN_PCMSTREAM

struct pcmstream_config
{
    char name[32];
    char desc[64]; /* export description */
                   //    char format[32];
    int rate;      /** output frequency in hz */
    int bits;      /** bits per sample (8, 16) */
    int channels;  /** output channels (1, 2) */
    char volume;
    /* input => readonly or output => writeonly */
};
//#endif

extern struct lwnbd_plugin_t *file_plugin_init(void);
extern struct lwnbd_server *nbd_server_init(void);

#endif /* INCLUDE_LWNBD_COMMON_H_ */
