#ifndef IOP_IRX_IMPORTS_H
#define IOP_IRX_IMPORTS_H

#include <irx.h>

#define PLUGIN_ATAD   1
//#define PLUGIN_BDM    1
#define PLUGIN_MCMAN  1
#define PLUGIN_MEMORY 1
#define PLUGIN_NBD    1
//#define PLUGIN_TTY    1

#ifdef PLUGIN_ATAD
#include <atad.h>
extern struct lwnbd_plugin *atad_plugin_init(void);
#endif

#ifdef PLUGIN_BDM
#include <bdm.h>
extern struct lwnbd_plugin *bdm_plugin_init(void);
#endif

#ifdef PLUGIN_MCMAN
#include <mcman.h>
extern struct lwnbd_plugin *mcman_plugin_init(void);
#endif

#ifdef PLUGIN_MEMORY
#include <ssbusc.h>
extern struct lwnbd_plugin *memory_plugin_init(void);
#endif

#ifdef PLUGIN_NBD
#include <ps2ip.h>
extern struct lwnbd_server *nbd_server_init(void);
#endif

#ifdef PLUGIN_TTY
#include <iomanX.h>
extern struct lwnbd_plugin *tty_plugin_init(void);
#endif

#include <errno.h>
#include <intrman.h>
#include <loadcore.h>
#include <sifcmd.h>
#include <sifman.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <thevent.h>
#include <thsemap.h>
#endif
