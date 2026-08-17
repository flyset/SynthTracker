#ifndef TFMX_PLAYBACK_LEGACY_MIXER_H
#define TFMX_PLAYBACK_LEGACY_MIXER_H

#include <stddef.h>

typedef struct tfmx_playback_legacy_mixer {
    unsigned int remainder;
    size_t pending_frames;
} tfmx_playback_legacy_mixer;

void tfmx_playback_legacy_mixer_reset(tfmx_playback_legacy_mixer *mixer);
void tfmx_playback_legacy_mixer_begin_tick(tfmx_playback_legacy_mixer *mixer,
                                           unsigned int eclocks,
                                           unsigned int output_rate);
int tfmx_playback_legacy_mixer_render(tfmx_playback_legacy_mixer *mixer,
                                      unsigned char *output, size_t capacity,
                                      size_t *bytes_written);

#endif
