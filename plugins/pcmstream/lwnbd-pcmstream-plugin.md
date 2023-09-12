# PCM stream plugin

## Usage : access PCM stream

TARGETS : PlayStation 2

STATUS : WIP (speaker only)

generally, a RTP (or icecast and so) is used. Let's try NBD as alternative.

### need more explanation ...

### Pulseaudio

1/ A solution can be create a sink with [module-null-sink](https://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/User/Modules/#module-null-sink), then relay it

```shell
# defaultsink=$(pactl get-default-sink)
# Creates a virtual sink that silently drops all data. Together with its monitor, may be used as a source-output-to-sink-input adapter.
# we can fix format here (ie format=s16le channels=1 rate=16000), default is pcm_s16le 2ch 44100Hz for me.
pacmd load-module module-null-sink sink_name=playstation2 \
    sink_properties="'device.description=\"Playstation 2 (analog-stereo)\"'"
pacmd update-sink-proplist playstation2 device.icon_name="audio-speakers"
```

You can now set this new sink to stream you want to bridge or set as default sink in pavucontrol,
(pavucontrol-qt label default as fallback, fix that) or use cli :

```shell
pacmd set-default-sink playstation2
```

then pipe it to nbd

```shell
parec -d playstation2.monitor | nbdcopy nbd://192.168.1.45/pcmstream
```

when finished, you can unload module.

```shell
pacmd unload-module module-null-sink
```

2/ another solution can be to bridge the source directly with  
pacat --record or parecord . it looks like a lot of possibilities here , see 

* https://gavv.net/articles/pulseaudio-under-the-hood/ for more information
* https://wiki.archlinux.org/title/PulseAudio/Examples#PulseAudio_over_network

## PS2 note:

It seems to be no code support for USB audio class in PS2SDK, so no microphone support (eyetoy, Singstar or headset microphone).
No Sony secret here, just generic USB Audio PCM stream, Sony SDK gave example of USB out (usbspkr sample).

Optical output (S/PDIF) would be interesting too, i've not the hardware yet. There are possibilities to bypass (passthrough) the data for both
Optical and analog output. Sony SDK provide some stuff about that too, and at the time, a complete library was available to ease usage of the SPU2 : Multistream by [Jason Page](https://www.lemonamiga.com/interviews/jason_page/). 

PCM vs Adpcm ? 

encoding ? https://github.com/vgmstream/vgmstream/blob/master/doc/FORMATS.md
vgmstream-cli -m boot.raw (look at extention, should be .raw . seems not working anyway)
ps2sdk contains ps2adpcm and adpenc

from ps2sdk/tools/adpenc/src/main.c :

```c
struct AdpcmHeader{
        char id[4];     //"APCM"
        unsigned char version;
        unsigned char channels;
        unsigned char loop;
        unsigned char reserved;
        unsigned int pitch;
        unsigned int samples;
};
```
```shell
Open-PS2-Loader/audio]$ hexdump -C -n 50 boot.adp 
00000000  41 50 43 4d 01 02 00 00  b3 0e 00 00 e3 eb 01 00  |APCM............|
00000010  0c 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
```

from PS2, anyway, we don't need to open a file but a (raw) stream of PCM(?) with right format, PA should be able to do that
