#ifndef TFMX_PLAYBACK_LEGACY_MIXER_H
#define TFMX_PLAYBACK_LEGACY_MIXER_H

#include <stddef.h>
#include <stdint.h>

#include "audio_output.h"

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
int tfmx_playback_legacy_mixer_render_frames(
    tfmx_playback_legacy_mixer *mixer, audio_frame *output, size_t capacity,
    size_t *frames_written);

#ifdef SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
int32_t tfmx_playback_legacy_mixer_test_normalize_pcm_sample(
    int32_t mixed_sample);
#endif

#endif
