#ifndef TFMX_PLAYBACK_LEGACY_RENDERER_H
#define TFMX_PLAYBACK_LEGACY_RENDERER_H

#include <stddef.h>

#include "audio_output.h"
#include "playback_context.h"

typedef struct {
    tfmx_playback_context *playback;
    unsigned int output_rate_hz;
    audio_frame *output_frames;
    size_t output_capacity;
    audio_frame *tick_frames;
    size_t tick_capacity;
    size_t tick_frame_count;
    size_t tick_frame_offset;
    size_t tick_advance_count;
    size_t last_tick_frame_count;
} tfmx_playback_legacy_renderer;

void tfmx_playback_legacy_renderer_init(
    tfmx_playback_legacy_renderer *renderer,
    tfmx_playback_context *playback,
    unsigned int output_rate_hz,
    audio_frame *output_frames,
    size_t output_capacity,
    audio_frame *tick_frames,
    size_t tick_capacity);

audio_frame_block tfmx_playback_legacy_renderer_render(
    void *context, size_t requested_frame_count);

#endif
