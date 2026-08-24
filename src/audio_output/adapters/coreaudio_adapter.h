#ifndef SYNTHTRACKER_COREAUDIO_ADAPTER_H
#define SYNTHTRACKER_COREAUDIO_ADAPTER_H

#include <stdbool.h>
#include <stdatomic.h>

#include "audio_output.h"
#include "coreaudio_facade.h"

typedef enum {
    AUDIO_OUTPUT_COREAUDIO_START_REJECTED,
    AUDIO_OUTPUT_COREAUDIO_START_STARTED,
    AUDIO_OUTPUT_COREAUDIO_START_UNAVAILABLE,
} audio_output_coreaudio_start_result;

typedef bool (*audio_output_coreaudio_route_prepare)(
    void *context,
    const audio_output_coreaudio_format *format);

audio_output_coreaudio_start_result audio_output_coreaudio_adapter_start(
    const audio_output_coreaudio_facade *facade,
    const audio_output_coreaudio_format *format);

audio_output_coreaudio_start_result
audio_output_coreaudio_adapter_start_default(
    const audio_output_coreaudio_format *format);

typedef struct {
    audio_output_frame_renderer renderer;
    audio_output_frame_delivery delivery;
    audio_output_coreaudio_route_prepare prepare;
    void *context;
} audio_output_coreaudio_bound_route;

typedef enum {
    AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_DIRECT = 0,
    AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE,
} audio_output_coreaudio_adapter_delivery_mode;

typedef struct {
    float *samples;
    size_t frame_capacity;
} audio_output_coreaudio_float32_workspace;

typedef enum {
    AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE,
    AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ACTIVE,
    AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_STOP_REQUESTED,
    AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_QUIESCENT,
    AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ERROR,
} audio_output_coreaudio_adapter_instance_state;

typedef struct {
    const audio_output_coreaudio_facade *facade;
    audio_output_coreaudio_bound_route route;
    audio_output_coreaudio_adapter_delivery_mode delivery_mode;
    audio_output_coreaudio_float32_workspace workspace;
    audio_output_coreaudio_adapter_instance_state state;
    bool in_request;
    atomic_int callback_admission;
} audio_output_coreaudio_adapter_instance;

audio_output_coreaudio_start_result
audio_output_coreaudio_adapter_start_instance(
    audio_output_coreaudio_adapter_instance *instance,
    const audio_output_coreaudio_format *format);

bool audio_output_coreaudio_adapter_request_stop(
    audio_output_coreaudio_adapter_instance *instance);

audio_output_coreaudio_facade_result
audio_output_coreaudio_adapter_stop_instance(
    audio_output_coreaudio_adapter_instance *instance);

audio_output_submit_result audio_output_coreaudio_adapter_submit(
    const audio_frame_block *block);

#ifdef SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
#include <stdbool.h>

typedef void (*audio_output_coreaudio_adapter_test_observer)(
    const float *interleaved_samples,
    size_t sample_count);

void audio_output_coreaudio_adapter_test_set_observer(
    audio_output_coreaudio_adapter_test_observer observer);

void audio_output_coreaudio_adapter_test_set_allocation_failure(
    bool should_fail);

void audio_output_coreaudio_adapter_test_reset_allocation_attempt_count(void);

size_t audio_output_coreaudio_adapter_test_allocation_attempt_count(void);
#endif

#endif
