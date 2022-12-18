#ifndef HDD_DRIVERS_NBD_H
#define HDD_DRIVERS_NBD_H

#include <lwnbd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hdd_driver
{
    nbd_context super;
    char device[6];
} hdd_driver;

int hdd_ctor(hdd_driver *const me, int device);

#ifdef __cplusplus
}
#endif
#endif /* hdd_DRIVERS_NBD_H */
