#ifdef _IOP
// no <inttypes.h>
// no <stdlib.h>
#include <sysclib.h>
// Missing in PS2SK's <stdint.h> , needed for "nbd-protocol.h"
// https://en.cppreference.com/w/c/types/integer
#define INT64_MAX   0x7fffffffffffffff
#define UINT64_MAX  0xffffffffffffffff
#define UINT64_C(x) ((x) + (UINT64_MAX - UINT64_MAX))

#endif /* _IOP */

// missing in <unistd.h>
typedef int ssize_t;

// Missing in <stddef.h>
#ifndef offsetof
#define offsetof(st, m) \
    ((size_t)((char *)&((st *)0)->m - (char *)0))
#endif
