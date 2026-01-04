#include <irx.h>

#include "ioplib.h"

/* clang-format off */

#ifdef CONFIG_PLUGIN_ATAD
#include <atad.h>
atad_IMPORTS_start
I_ata_device_flush_cache
I_ata_device_sce_identify_drive
I_ata_device_sector_io
I_ata_get_devinfo
atad_IMPORTS_end
#endif

#ifdef CONFIG_PLUGIN_BDM
#include <bdm.h>
bdm_IMPORTS_start
I_bdm_connect_bd
I_bdm_disconnect_bd
//I_bdm_connect_fs
//I_bdm_disconnect_fs
I_bdm_get_bd
//I_bdm_RegisterCallback
bdm_IMPORTS_end
#endif

#ifdef CONFIG_PLUGIN_TTY
#include <iomanX.h>
iomanX_IMPORTS_start
I_AddDrv
I_close
I_DelDrv
I_GetDeviceList
I_getstat
I_open
I_write
iomanX_IMPORTS_end
#endif

intrman_IMPORTS_start
I_CpuEnableIntr
I_CpuSuspendIntr
I_CpuResumeIntr
intrman_IMPORTS_end

loadcore_IMPORTS_start
I_RegisterLibraryEntries
I_ReleaseLibraryEntries
loadcore_IMPORTS_end

#ifdef CONFIG_NET
#include <ps2ip.h>
ps2ip_IMPORTS_start
I_lwip_accept
I_lwip_bind
I_lwip_close
I_lwip_listen
I_lwip_recv
I_lwip_send
I_lwip_setsockopt
I_lwip_socket
ps2ip_IMPORTS_end
#endif

thbase_IMPORTS_start
I_CreateThread
I_DeleteThread
I_GetThreadId
I_StartThread
I_TerminateThread
thbase_IMPORTS_end

thevent_IMPORTS_start
I_CreateEventFlag
I_DeleteEventFlag
I_SetEventFlag
I_WaitEventFlag
thevent_IMPORTS_end

thsemap_IMPORTS_start
I_CreateSema
I_DeleteSema
I_SignalSema
I_WaitSema
thsemap_IMPORTS_end

#ifdef CONFIG_PLUGIN_MEMORY
#include <ssbusc.h>
ssbusc_IMPORTS_start
I_GetBaseAddress
I_GetDelay
ssbusc_IMPORTS_end
#endif

#ifdef CONFIG_PLUGIN_PCMSTREAM
#include <audsrv.h>
audsrv_IMPORTS_start
I_audsrv_init
I_audsrv_play_audio
I_audsrv_quit
I_audsrv_set_format
I_audsrv_set_volume
I_audsrv_wait_audio
audsrv_IMPORTS_end
#endif

sifcmd_IMPORTS_start
I_isceSifSendCmd
I_sceSifInitRpc
I_sceSifSetRpcQueue
I_sceSifRegisterRpc
I_sceSifRpcLoop
I_sceSifBindRpc
I_sceSifCallRpc
sifcmd_IMPORTS_end

sysmem_IMPORTS_start
I_AllocSysMemory
I_FreeSysMemory
I_QueryMemSize
sysmem_IMPORTS_end

stdio_IMPORTS_start
I_printf
//#endif
stdio_IMPORTS_end

sysclib_IMPORTS_start
//I_printf
//#endif
I_memcpy
I_memset
I_sprintf
I_strchr
I_strcmp
I_strcpy
I_strlen
I_strncmp
I_strncpy
I_toupper
sysclib_IMPORTS_end

#ifdef CONFIG_PLUGIN_MCMAN
#include <mcman.h>
xmcman_IMPORTS_start
I_McDataChecksum
I_McDetectCard2
I_McEraseBlock2
I_McFlushCache
I_McGetCardSpec
I_McGetMcType
I_McReadCluster
I_McReadPage
I_McWritePage
xmcman_IMPORTS_end
#endif
