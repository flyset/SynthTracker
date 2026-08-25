#ifndef SYNTHTRACKER_TEST_FAKE_COREAUDIO_FACADE_H
#define SYNTHTRACKER_TEST_FAKE_COREAUDIO_FACADE_H

#include <stdbool.h>
#include <stddef.h>

#include "adapters/coreaudio_adapter.h"

typedef enum {
    FAKE_COREAUDIO_FACADE_CALL_OPEN,
    FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
    FAKE_COREAUDIO_FACADE_CALL_PREPARE,
    FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
    FAKE_COREAUDIO_FACADE_CALL_START,
    FAKE_COREAUDIO_FACADE_CALL_QUIESCE,
    FAKE_COREAUDIO_FACADE_CALL_STOP,
    FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
} fake_coreaudio_facade_lifecycle_call;

enum {
    FAKE_COREAUDIO_FACADE_LIFECYCLE_TRACE_CAPACITY = 8,
    FAKE_COREAUDIO_FACADE_REQUEST_TRACE_CAPACITY = 8,
};

typedef struct {
    audio_output_coreaudio_facade facade;
    audio_output_coreaudio_facade_result open_result;
    audio_output_coreaudio_facade_result configure_result;
    audio_output_coreaudio_format requested_format;
    bool requested_format_set;
    audio_output_coreaudio_format reported_format;
    bool reported_format_set;
    audio_output_coreaudio_format configured_format;
    bool configured_format_set;
    audio_output_coreaudio_facade_result bind_result;
    audio_output_coreaudio_facade_result start_result;
    audio_output_coreaudio_facade_result stop_result;
    audio_output_coreaudio_facade_result quiesce_result;
    audio_output_coreaudio_facade_result dispose_result;
    size_t open_call_count;
    size_t configure_call_count;
    size_t prepare_call_count;
    size_t start_call_count;
    fake_coreaudio_facade_lifecycle_call lifecycle_trace[
        FAKE_COREAUDIO_FACADE_LIFECYCLE_TRACE_CAPACITY];
    size_t lifecycle_trace_count;
    size_t bind_request_call_count;
    size_t quiesce_call_count;
    size_t callback_quiesce_call_count;
    size_t control_stop_request_call_count;
    size_t control_stop_call_count;
    size_t control_quiesce_call_count;
    size_t control_dispose_call_count;
    audio_output_coreaudio_request request;
    void *request_context;
    /* request_trace retains only the first
     * FAKE_COREAUDIO_FACADE_REQUEST_TRACE_CAPACITY request frame counts, so
     * the fake facade may service more callbacks than the fixed trace buffer
     * can hold without ever writing past it. request_trace_count is the number
     * of safely retained entries and never exceeds the trace capacity;
     * request_trace_truncated records that additional callbacks were serviced
     * beyond the retained prefix. render_request_count (below) remains the
     * separate total callback/request count used by lifecycle/completion
     * assertions. */
    size_t request_trace[FAKE_COREAUDIO_FACADE_REQUEST_TRACE_CAPACITY];
    size_t request_trace_count;
    bool request_trace_truncated;
    bool active;
    bool quiescent;
    bool disposed;
    bool callback_in_flight;
    bool control_stop_requested_during_callback;
    bool control_stop_called_during_callback;
    bool control_dispose_called_during_callback;
    size_t render_request_count;
    bool reentrant_start_enabled;
    bool reentrant_start_attempted;
    audio_output_coreaudio_adapter_instance *reentrant_start_instance;
    const audio_output_coreaudio_format *reentrant_start_format;
    audio_output_coreaudio_start_result reentrant_start_result;
    audio_output_coreaudio_adapter_instance *deferred_stop_instance;
    bool deferred_stop_pending;
    bool deferred_stop_admission_result;
    size_t start_request_frame_count;
    size_t start_request_count;
} fake_coreaudio_facade;

void fake_coreaudio_facade_init(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_facade_result open_result,
    audio_output_coreaudio_facade_result configure_result,
    audio_output_coreaudio_facade_result start_result);

void fake_coreaudio_facade_set_workspace_only(
    fake_coreaudio_facade *fake,
    bool workspace_only);

void fake_coreaudio_facade_set_reported_format(
    fake_coreaudio_facade *fake,
    const audio_output_coreaudio_format *format);

void fake_coreaudio_facade_record_preparation(fake_coreaudio_facade *fake);

void fake_coreaudio_facade_set_bind_result(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_facade_result bind_result);

void fake_coreaudio_facade_request_during_start(
    fake_coreaudio_facade *fake,
    size_t requested_frame_count,
    size_t request_count);

void fake_coreaudio_facade_configure_reentrant_start(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_adapter_instance *instance,
    const audio_output_coreaudio_format *format);

audio_output_submit_result fake_coreaudio_facade_request(
    fake_coreaudio_facade *fake,
    size_t requested_frame_count);

audio_output_submit_result fake_coreaudio_facade_request_with_output(
    fake_coreaudio_facade *fake,
    size_t requested_frame_count,
    float *interleaved_samples,
    size_t frame_capacity);

void fake_coreaudio_facade_defer_control_stop(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_adapter_instance *instance);

bool fake_coreaudio_facade_control_request_stop(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_adapter_instance *instance);

void fake_coreaudio_facade_control_stop(fake_coreaudio_facade *fake);

void fake_coreaudio_facade_control_quiesce(fake_coreaudio_facade *fake);

void fake_coreaudio_facade_control_dispose(fake_coreaudio_facade *fake);

#endif
