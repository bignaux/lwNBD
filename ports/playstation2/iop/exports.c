#include "irx.h"
void _retonly(void) {}

DECLARE_EXPORT_TABLE(lwnbd, 1, 1)
DECLARE_EXPORT(_start)
DECLARE_EXPORT(_retonly)
DECLARE_EXPORT(_shutdown)
DECLARE_EXPORT(_retonly)
DECLARE_EXPORT(_retonly)
END_EXPORT_TABLE
