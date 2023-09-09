#ifdef _IOP
// no <inttypes.h>
// no <stdlib.h>
#include <sysclib.h>
// Missing in PS2SK's <stdint.h> , needed for "nbd-protocol.h"
// https://en.cppreference.com/w/c/types/integer
#define INT64_MAX   0x7fffffffffffffff
#define UINT64_MAX  0xffffffffffffffff
#define UINT64_C(x) ((x) + (UINT64_MAX - UINT64_MAX))


typedef int ssize_t;

#endif /* _IOP */
