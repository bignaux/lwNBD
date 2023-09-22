#ifndef IOP_IRX_IMPORTS_H
#define IOP_IRX_IMPORTS_H

#include <irx.h>

#define PLUGIN_ATAD      1
//#define PLUGIN_BDM    1
#define PLUGIN_MCMAN     1
#define PLUGIN_MEMORY    1
#define PLUGIN_NBD       1
//#define PLUGIN_TTY    1
#define PLUGIN_PCMSTREAM 1
#define PLUGIN_SIFRPC    1

#ifdef PLUGIN_ATAD
#include <atad.h>
extern struct lwnbd_plugin_t *atad_plugin_init(void);
#endif

#ifdef PLUGIN_BDM
#include <bdm.h>
extern struct lwnbd_plugin_t *bdm_plugin_init(void);
#endif

#ifdef PLUGIN_MCMAN
#include <mcman.h>
extern struct lwnbd_plugin_t *mcman_plugin_init(void);
#endif

#ifdef PLUGIN_MEMORY
#include <ssbusc.h>
extern struct lwnbd_plugin_t *memory_plugin_init(void);
#endif

#ifdef PLUGIN_NBD
#include <ps2ip.h>
extern struct lwnbd_server *nbd_server_init(void);
#endif

#ifdef PLUGIN_TTY
#include <iomanX.h>
extern struct lwnbd_plugin_t *tty_plugin_init(void);
#endif

#ifdef PLUGIN_PCMSTREAM
#include <audsrv.h>
extern struct lwnbd_plugin_t *pcmstream_plugin_init(void);
#endif

#ifdef PLUGIN_SIFRPC
#include <sifcmd.h>
#include <sifman.h>
#include "../../servers/sifrpc/sifrpc.h"
extern struct lwnbd_server *sifrpc_server_init(void);
#endif

#include <errno.h>
#include <intrman.h>
#include <loadcore.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <thevent.h>
#include <thsemap.h>
#endif
