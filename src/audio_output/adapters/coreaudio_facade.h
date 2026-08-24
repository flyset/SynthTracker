#ifndef SYNTHTRACKER_AUDIO_OUTPUT_COREAUDIO_FACADE_H
#define SYNTHTRACKER_AUDIO_OUTPUT_COREAUDIO_FACADE_H

#include <stdbool.h>
#include <stdint.h>

#include "audio_output.h"

typedef enum {
    AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
    AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
    AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE,
} audio_output_coreaudio_facade_result;

typedef enum {
    AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
    AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_INT16,
} audio_output_coreaudio_sample_format;

typedef enum {
    AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    AUDIO_OUTPUT_COREAUDIO_LAYOUT_PLANAR,
} audio_output_coreaudio_layout;

typedef struct {
    uint32_t sample_rate_hz;
    uint32_t channel_count;
    audio_output_coreaudio_sample_format sample_format;
    audio_output_coreaudio_layout layout;
} audio_output_coreaudio_format;

typedef audio_output_coreaudio_facade_result
(*audio_output_coreaudio_facade_open)(void *context);

typedef audio_output_coreaudio_facade_result
(*audio_output_coreaudio_facade_configure)(
    void *context,
    const audio_output_coreaudio_format *format,
    audio_output_coreaudio_format *configured_format);

typedef audio_output_submit_result (*audio_output_coreaudio_request)(
    void *context,
    size_t requested_frame_count,
    float *interleaved_samples,
    size_t frame_capacity);

typedef audio_output_coreaudio_facade_result
(*audio_output_coreaudio_facade_bind_request)(
    void *context,
    audio_output_coreaudio_request request,
    void *request_context);

typedef audio_output_coreaudio_facade_result
(*audio_output_coreaudio_facade_start)(void *context);

typedef audio_output_coreaudio_facade_result
(*audio_output_coreaudio_facade_quiesce)(void *context);

typedef audio_output_coreaudio_facade_result
(*audio_output_coreaudio_facade_stop)(void *context);

typedef audio_output_coreaudio_facade_result
(*audio_output_coreaudio_facade_dispose)(void *context);

typedef struct {
    void *context;
    bool workspace_only;
    audio_output_coreaudio_facade_open open;
    audio_output_coreaudio_facade_configure configure;
    audio_output_coreaudio_facade_bind_request bind_request;
    audio_output_coreaudio_facade_start start;
    audio_output_coreaudio_facade_quiesce quiesce;
    audio_output_coreaudio_facade_stop stop;
    audio_output_coreaudio_facade_dispose dispose;
} audio_output_coreaudio_facade;

const audio_output_coreaudio_facade *
audio_output_coreaudio_facade_default(void);

#endif
