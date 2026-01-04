/*
 * TODO
 * - enable/disable booting from MBR ( see https://github.com/parrado/SoftDev2/blob/main/installer/install.c#L52 )
 * - nbdcopy MBR.XIN nbd://192.168.1.45/hdd0/mbr
 */

/* nbdcopy nbd://192.168.1.45/hdd0/identify - | hdparm --Istdin  */
int identify(int argc, char **argv, void *result, int64_t *size)
{
    return ata_device_sce_identify_drive(argc, result);
}
int ata_device_idle(int device, int period);
int ata_device_smart_get_status(int device);
int ata_device_smart_save_attr(int device);

static int atad_ctrl(void *handle, char *path, struct lwnbd_command *cmd)
{
    struct handle *h = handle;

    if (strcmp("identify", path)) {
        cmd->cmd = identify;
        cmd->argc = h->device;
        cmd->size = 256;
        return 0;
    }

    else
        return -1;
}
