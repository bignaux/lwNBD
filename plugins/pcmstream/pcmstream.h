#ifndef PLUGINS_PCMSTREAM_H_
#define PLUGINS_PCMSTREAM_H_

#include <lwnbd-plugin.h>

struct pcmstream_config
{
    char name[32];
    char desc[64]; /* export description */
                   //    char format[32];
    int rate;      /** output frequency in hz */
    int bits;      /** bits per sample (8, 16) */
    int channels;  /** output channels (1, 2) */
    char volume;
    /* input => readonly or output => writeonly */
};

#endif /* PLUGINS_PCMSTREAM_H_ */
