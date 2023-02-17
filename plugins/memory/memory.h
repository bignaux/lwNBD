#ifndef PLUGINS_MEMORY_H_
#define PLUGINS_MEMORY_H_

#include <lwnbd-plugin.h>

struct memory_config
{
    char name[32];
    uint64_t base;
    uint64_t size;
};

#endif /* PLUGINS_MEMORY_H_ */
