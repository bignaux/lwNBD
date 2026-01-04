# Supported platforms

* GNU/Linux that use *file* plugin to
  serve a list of files as command line parameters. For the time being, the main 
  purpose of the support is to facilitate development. It uses libuv as event loop, and can serve multiple client.

* Playstation 2 IOP via an IRX module for [Open-PS2-Loader](https://github.com/ps2homebrew/Open-PS2-Loader).
  It can export [hdd drive (atad plugin)](./plugins/atad/lwnbd-atad-plugin.md), [MemoryCard (mcman plugin)](./plugins/mcman/lwnbd-mcman-plugin.md), rom0 and IOP ram (*memory* plugin). Read more about this on [my Playstation 2 port notes](./ports/playstation2/lwnbd-playstation2-port.md).
  
## Adding support for a new platform

**IMPORTANT**: Before attempting to add support for a new platform please open
an issue about it for discussion.