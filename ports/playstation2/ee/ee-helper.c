/*
 * WIP
 * ee-helper.c
 *
 * add automagically right modules according to IOP configuration ?
 *
 *
 */
#include <lwnbd/ee_lib.h>


/*
 * int 	ps2_screenshot (void *pTemp, unsigned int VramAdress, unsigned int x, unsigned int y, unsigned int Width, unsigned int Height, unsigned int Psm)
 */



int init_platform()
{

    int ret;

    SifInitRpc(0);
#if !defined(DEBUG) || defined(BUILD_FOR_PCSX2)
    /* Comment this line if you don't wanna debug the output */
    while (!SifIopReset(NULL, 0)) {
    };
#endif

    while (!SifIopSync()) {
    };
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

#ifdef CONFIG_PLUGIN_ATAD
    extern unsigned char ps2atad_irx[];
    extern unsigned int size_ps2atad_irx;
#endif

    //    required by mcman
    ret = SifExecModuleBuffer(&sio2man_irx, size_sio2man_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: sio2man\n");
        return -1;
    }

    //    required by audsrv
    ret = SifExecModuleBuffer(&libsd_irx, size_libsd_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: libsd\n");
        return -1;
    }

    ret = SifExecModuleBuffer(&audsrv_irx, size_audsrv_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: audsrv\n");
        return -1;
    }

    ret = SifExecModuleBuffer(&ps2ip_irx, size_ps2ip_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: ps2ip_irx\n");
        return -1;
    }

    ret = SifExecModuleBuffer(&udptty_irx, size_udptty_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: UDPTTY\n");
        return -1;
    }

    ret = SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: poweroff\n");
        return -1;
    }

    ret = SifExecModuleBuffer(&mcman_irx, size_mcman_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: mcman\n");
        return -1;
    }

    ret = SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: ps2dev9\n");
        return -1;
    }

    ret = SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: ps2atad\n");
        return -1;
    }

    ret = SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: usbd\n");
        return -1;
    }

    //    ret = SifExecModuleBuffer(&usbhdfsd_irx, size_usbhdfsd_irx, 0, NULL, NULL);
    //    if (ret < 0) {
    //        scr_printf("Failed to load module: usbhdfsd\n");
    //        return -1;
    //    }

    ret = SifExecModuleBuffer(&netman_irx, size_netman_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: netman\n");
        return -1;
    }

    ret = SifExecModuleBuffer(&smap_irx, size_smap_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: smap\n");
        return -1;
    }

    /* IOP-side stack : LWIP v2.0.0 stack */

    //    ret = SifExecModuleBuffer(&ps2ip_nm_irx, size_ps2ip_nm_irx, if_conf_len, &if_conf[0], NULL);
    ret = SifExecModuleBuffer(&ps2ip_nm_irx, size_ps2ip_nm_irx, 0, NULL, NULL);
    if (ret < 0) {
        scr_printf("Failed to load module: ps2ip_nm\n");
        return -1;
    }

    ret = set_ipconfig();
    if (ret < 0) {
        scr_printf("Failed to set network configuration.\n");
        return -1;
    }

    //#ifdef CONFIG_PLUGIN_BDM
    //
    //#endif
    //
    //#ifdef CONFIG_PLUGIN_TTY
    // iomanX_IMPORTS_start
    //
    // iomanX_IMPORTS_end
    //#endif
    //
    //#ifdef CONFIG_PLUGIN_NBD
    // ps2ip_IMPORTS_start
    //
    // ps2ip_IMPORTS_end
    //#endif
    //
    //
    //
    //#ifdef CONFIG_PLUGIN_MEMORY
    // ssbusc_IMPORTS_start
    // I_GetBaseAddress
    // I_GetDelay
    // ssbusc_IMPORTS_end
    //#endif
    //
    //#ifdef CONFIG_PLUGIN_PCMSTREAM
    // audsrv_IMPORTS_start
    //
    // audsrv_IMPORTS_end
    //#endif
    //
    //#ifdef CONFIG_PLUGIN_MCMAN
    // xmcman_IMPORTS_start
    //
    // xmcman_IMPORTS_end
    //#endif

    return 0;
}
