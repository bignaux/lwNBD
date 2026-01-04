File to record change that would be copy to OPL PR... and general journal about OPL integration.


* Kconfig support to easily configure library for your need
* query support enable writing command on specific device
* pcmstream support to enable pcm stream




* API to be able to write custom API around your software

modules/network/lwNBD/.config:
    cp modules/network/lwNBD/configs/config.open-ps2-loader-irx modules/network/lwNBD/.config


PR #887
lwnbd => 9777a10f840679ef89b1ec6a588e2d93803d7c37


https://github.com/ps2homebrew/Open-PS2-Loader/pull/904
You can get it https://mega.nz/folder/Ndwi1bAK#oLWNhH_g-h0p4BoT4c556A/folder/hQxTTIgR

Revert few weeks later in
https://github.com/ps2homebrew/Open-PS2-Loader/pull/919

it looks like it was a classical configuration file issue ...
this would be boring to try to fix this dead cow, so avoiding to use any stuff like this seems to be best way. Strange this was introduice the previous PR without any issue. Btw gExportName seems useless variable now. struct lwnbd_config should not be defined here. Novalty here is NBDInit() and why i 
get this worked. sid useless var, all others define it on 0xXXXXXXX base. i started to put some SIF to be able to control the server , mainly to finish it gracefully.

https://github.com/ps2homebrew/Open-PS2-Loader/issues/917#issuecomment-1501090033

NEXT
    * remove unmaintened labs/lwnbdsvr/
    * don't depend on hdd to have nbd menu
    * move to ata_device_sector_io64 (can't test myself, need tester)
    * atad SMART report and /boot
    * API plugin
    * pcmstream new plugin

COMMIT="a6e3a1feb3b8a80e53ef3040b50789469ef59f0b"