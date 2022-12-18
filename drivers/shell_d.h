#ifndef SHELL_DRIVERS_NBD_H
#define SHELL_DRIVERS_NBD_H

#include <lwnbd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct shell_driver
{
    nbd_context super;
    char *log;
} shell_driver;

int shell_ctor(file_driver *const me);

#ifdef __cplusplus
}
#endif
#endif /* SHELL_DRIVERS_NBD_H */
