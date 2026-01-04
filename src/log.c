#include <lwnbd/lwnbd-common.h>
#include <stdarg.h>

/* cherry picked from lssdp */
#define LWNBD_BUFFER_LEN 2048

void lwnbd_default_log_callback(const char *file, const char *tag, int level, int line, const char *func, const char *message)
{
    char *level_name = "DEBUG";
    if (level == LWNBD_LOG_INFO)
        level_name = "INFO";
    if (level == LWNBD_LOG_WARN)
        level_name = "WARN";
    if (level == LWNBD_LOG_ERROR)
        level_name = "ERROR";

    printf("[%-5s][%s] %s", level_name, tag, message);
}

static struct
{
    int level;
    void (*log_callback)(const char *file, const char *tag, int level, int line, const char *func, const char *message);

} Global = {
    .level = CONFIG_LOG_DEFAULT_LEVEL,
    .log_callback = lwnbd_default_log_callback,
};

int lwnbd_log(int level, int line, const char *func, const char *format, ...)
{
    if (Global.log_callback == NULL) {
        return -1;
    }

    if (level > Global.level) {
        return -1;
    }

    char message[LWNBD_BUFFER_LEN] = {};

    // create message by va_list
    va_list args;
    va_start(args, format);
    vsnprintf(message, LWNBD_BUFFER_LEN, format, args);
    va_end(args);

    // invoke log callback function
    Global.log_callback(__FILE__, "LWNBD", level, line, func, message);

    return 0;
}

void lwnbd_set_log_callback(void (*callback)(const char *file, const char *tag, int level, int line, const char *func, const char *message))
{
    Global.log_callback = callback;
}

void lwnbd_set_log_level(int level)
{
    Global.level = level;
}
