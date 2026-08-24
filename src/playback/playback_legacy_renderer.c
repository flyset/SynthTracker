#include <stdbool.h>
#include <string.h>

#include "playback_legacy_renderer.h"

static bool supported_output_rate(unsigned int output_rate_hz)
{
    return output_rate_hz == 44100 || output_rate_hz == 48000;
}

static bool prepare_next_tick(tfmx_playback_legacy_renderer *renderer)
{
    size_t frame_count = 0;

    if (tfmx_playback_context_tick_at_rate(renderer->playback,
                                           renderer->output_rate_hz) !=
            TFMX_TICK_SUCCESS ||
        tfmx_playback_context_render_frames(
            renderer->playback, renderer->tick_frames, renderer->tick_capacity,
            &frame_count) != TFMX_RENDER_SUCCESS ||
        frame_count == 0) {
        return false;
    }

    renderer->tick_frame_count = frame_count;
    renderer->tick_frame_offset = 0;
    renderer->tick_advance_count++;
    renderer->last_tick_frame_count = frame_count;
    return true;
}

void tfmx_playback_legacy_renderer_init(
    tfmx_playback_legacy_renderer *renderer,
    tfmx_playback_context *playback,
    unsigned int output_rate_hz,
    audio_frame *output_frames,
    size_t output_capacity,
    audio_frame *tick_frames,
    size_t tick_capacity)
{
    if (renderer == NULL) {
        return;
    }
    *renderer = (tfmx_playback_legacy_renderer){
        .playback = playback,
        .output_rate_hz = output_rate_hz,
        .output_frames = output_frames,
        .output_capacity = output_capacity,
        .tick_frames = tick_frames,
        .tick_capacity = tick_capacity,
    };
}

audio_frame_block tfmx_playback_legacy_renderer_render(
    void *context, size_t requested_frame_count)
{
    tfmx_playback_legacy_renderer *renderer = context;
    size_t produced = 0;

    if (renderer == NULL || requested_frame_count == 0) {
        return (audio_frame_block){ .frame_count = 0, .frames = NULL };
    }
    if (!supported_output_rate(renderer->output_rate_hz) ||
        renderer->playback == NULL || renderer->output_frames == NULL ||
        renderer->tick_frames == NULL ||
        requested_frame_count > renderer->output_capacity ||
        renderer->tick_capacity == 0) {
        return (audio_frame_block){ .frame_count = 0, .frames = NULL };
    }

    while (produced < requested_frame_count) {
        if (renderer->tick_frame_offset == renderer->tick_frame_count &&
            !prepare_next_tick(renderer)) {
            return (audio_frame_block){ .frame_count = 0, .frames = NULL };
        }

        const size_t available =
            renderer->tick_frame_count - renderer->tick_frame_offset;
        const size_t needed = requested_frame_count - produced;
        const size_t copied = available < needed ? available : needed;
        memcpy(&renderer->output_frames[produced],
               &renderer->tick_frames[renderer->tick_frame_offset],
               copied * sizeof(*renderer->output_frames));
        produced += copied;
        renderer->tick_frame_offset += copied;
    }

    return (audio_frame_block){
        .frame_count = requested_frame_count,
        .frames = renderer->output_frames,
    };
}
