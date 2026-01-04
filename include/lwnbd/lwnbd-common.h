#ifndef INCLUDE_LWNBD_COMMON_H_
#define INCLUDE_LWNBD_COMMON_H_

#include <lwnbd/config.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>


enum LWNBD_LOG {
    LWNBD_LOG_ERROR = 1,
    LWNBD_LOG_WARN = 2,
    LWNBD_LOG_INFO = 3,
    LWNBD_LOG_DEBUG = 4,
};

int lwnbd_log(int level, int line, const char *func, const char *format, ...);
void lwnbd_set_log_callback(void (*callback)(const char *file, const char *tag, int level, int line, const char *func, const char *message));
void lwnbd_default_log_callback(const char *file, const char *tag, int level, int line, const char *func, const char *message);
void lwnbd_set_log_level(int level);

#define lwnbd_error(fmt, agrs...) lwnbd_log(LWNBD_LOG_ERROR, __LINE__, __func__, fmt, ##agrs)
#define lwnbd_warn(fmt, agrs...)  lwnbd_log(LWNBD_LOG_WARN, __LINE__, __func__, fmt, ##agrs)
#define lwnbd_info(fmt, agrs...)  lwnbd_log(LWNBD_LOG_INFO, __LINE__, __func__, fmt, ##agrs)
#define lwnbd_debug(fmt, agrs...) lwnbd_log(LWNBD_LOG_DEBUG, __LINE__, __func__, fmt, ##agrs)

#define PRI_UINT64_C_Val(value) ((unsigned long)(value >> 32)), ((unsigned long)value)
#define PRI_UINT64              "%lx%lx"

/*
 *
 */

struct nbdsettings
{
    uint16_t port;
    uint16_t max_retry;
    uint32_t gflags;  // nbd proto global flag
    uint8_t readonly; /* all exports would be read only */
    /*
     * Some nbd client has options to use a preinitialized connection, and to specify the device size
     * and skip protocol handshake. (nbd-client -preinit -size <bytes> )
     */
    uint8_t preinit;
};

#ifdef CONFIG_PLUGIN_ATAD
extern struct lwnbd_plugin_t *atad_plugin_init(void);
#endif

#ifdef CONFIG_PLUGIN_BDM

// extern struct lwnbd_plugin_t *bdm_plugin_init(void);
#endif

#ifdef CONFIG_PLUGIN_MCMAN
extern struct lwnbd_plugin_t *mcman_plugin_init(void);
#endif

#ifdef CONFIG_PLUGIN_TTY
extern struct lwnbd_plugin_t *tty_plugin_init(void);
#endif

#ifdef CONFIG_PLUGIN_PCMSTREAM

extern struct lwnbd_plugin_t *pcmstream_plugin_init(void);
#endif

#ifdef CONFIG_PLUGIN_PCMSTREAM
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
#endif

#ifdef CONFIG_PLUGIN_FILE
extern struct lwnbd_plugin_t *file_plugin_init(void);
#endif

#ifdef CONFIG_PLUGIN_COMMAND
typedef enum {
    METHOD_GET,
    METHOD_POST,
} method_type;
extern struct lwnbd_plugin_t *command_plugin_init(void);
struct lwnbd_command
{
    char *name;
    char *desc;
    method_type type;
    int (*cmd)(int argc, char **argv, const void *result, int64_t *size);
};
#endif

#ifdef CONFIG_PLUGIN_MEMORY
extern struct lwnbd_plugin_t *memory_plugin_init(void);
struct memory_config
{
    intptr_t base;
    intptr_t size;
    char name[32];
    char desc[64]; /* export description */
    /* TODO readonly ? */
};
#endif


/*
 * TODO: move next stuff in kconfig
 */


#define PACKAGE_VERSION "TODO"


/* server options */
#define MAX_NUM_SERVERS 2
#define MAX_NUM_CLIENTS 2


/* plugins options - useless here ? */

#define MAX_NUM_PLUGINS 10
#define MAX_CONTEXTS    50


#endif /* INCLUDE_LWNBD_COMMON_H_ */
