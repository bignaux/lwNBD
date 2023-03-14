#ifndef PLUGINS_MEMORY_H_
#define PLUGINS_MEMORY_H_

#include <lwnbd-plugin.h>

struct memory_config
{
    intptr_t base;
    intptr_t size;
    char name[32];
    char desc[64]; /* export description */
};

#endif /* PLUGINS_MEMORY_H_ */
