/*

Could be in iop/dev9/atad/src/ps2atad.c

TODO: _ata_identify_offsets

ref: https://github.com/ps2homebrew/HDDChecker/blob/main/hdst.c#L44
https://salsa.debian.org/debian/hdparm/-/blob/master/identify.c

 * ata_devinfo_t could be extended ?
 */

typedef struct
{
    char unknown[10];
    char serial[22];    // 10
    char FWVersion[10]; // 23
    char model[42];     // 27
} ata_devinfoplus_t;

static int ata_parameter_parser(const void *ata_param,
                                ata_devinfoplus_t *sparam)
{


    for (i = 0; i < 20; i++)
        ((u16 *)AtadDeviceData[unit].model)[i] = BSWAP16(
            AtadDeviceData[unit].IdentificationData[27 + i]);
    TrimWhitespacing(AtadDeviceData[unit].model,
                     sizeof(AtadDeviceData[unit].model) - 1);
    for (i = 0; i < 10; i++)
        ((u16 *)AtadDeviceData[unit].serial)[i] = BSWAP16(
            AtadDeviceData[unit].IdentificationData[10 + i]);
    TrimWhitespacing(AtadDeviceData[unit].serial,
                     sizeof(AtadDeviceData[unit].serial) - 1);
    for (i = 0; i < 4; i++)
        ((u16 *)AtadDeviceData[unit].FWVersion)[i] = BSWAP16(
            AtadDeviceData[unit].IdentificationData[23 + i]);
    TrimWhitespacing(AtadDeviceData[unit].FWVersion,
                     sizeof(AtadDeviceData[unit].FWVersion) - 1);

    return res;
}

/*
 * linux equivalent of ata_device_identify
 * int ata_dev_read_id(struct ata_device * dev, unsigned int * p_class, unsigned int flags, u16 * id)
 * should be some ata_getters
 */

static u16 ata_param[256];
ata_devinfoplus_t sparam;
res = ata_device_identify(device, ata_param);
res = ata_parameter_parser(ata_param, sparam);

    then use model for me->super.export_desc
