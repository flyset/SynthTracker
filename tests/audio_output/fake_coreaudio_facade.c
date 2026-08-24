#include "fake_coreaudio_facade.h"

static audio_output_submit_result fake_coreaudio_facade_request_internal(
    fake_coreaudio_facade *fake,
    size_t requested_frame_count,
    float *interleaved_samples,
    size_t frame_capacity);

static void record_lifecycle_call(
    fake_coreaudio_facade *fake,
    fake_coreaudio_facade_lifecycle_call call)
{
    fake->lifecycle_trace[fake->lifecycle_trace_count] = call;
    fake->lifecycle_trace_count++;
}

static audio_output_coreaudio_facade_result fake_open(void *context)
{
    fake_coreaudio_facade *fake = context;
    record_lifecycle_call(fake, FAKE_COREAUDIO_FACADE_CALL_OPEN);
    fake->open_call_count++;
    return fake->open_result;
}

static audio_output_coreaudio_facade_result fake_configure(
    void *context,
    const audio_output_coreaudio_format *format,
    audio_output_coreaudio_format *configured_format)
{
    fake_coreaudio_facade *fake = context;
    record_lifecycle_call(fake, FAKE_COREAUDIO_FACADE_CALL_CONFIGURE);
    fake->configure_call_count++;
    if (format != NULL) {
        fake->requested_format = *format;
        fake->requested_format_set = true;
    }
    if (configured_format != NULL) {
        *configured_format = fake->reported_format_set
                                  ? fake->reported_format
                                  : *format;
        fake->configured_format = *configured_format;
        fake->configured_format_set = true;
    }
    return fake->configure_result;
}

static audio_output_coreaudio_facade_result fake_bind_request(
    void *context,
    audio_output_coreaudio_request request,
    void *request_context)
{
    fake_coreaudio_facade *fake = context;
    record_lifecycle_call(fake, FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST);
    fake->bind_request_call_count++;
    if (fake->bind_result == AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        fake->request = request;
        fake->request_context = request_context;
    }
    return fake->bind_result;
}

static audio_output_coreaudio_facade_result fake_start(void *context)
{
    fake_coreaudio_facade *fake = context;
    record_lifecycle_call(fake, FAKE_COREAUDIO_FACADE_CALL_START);
    fake->start_call_count++;
    if (fake->reentrant_start_enabled && !fake->reentrant_start_attempted) {
        fake->reentrant_start_attempted = true;
        fake->reentrant_start_result =
            audio_output_coreaudio_adapter_start_instance(
                fake->reentrant_start_instance,
                fake->reentrant_start_format);
    }
    if (fake->start_result == AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        fake->active = true;
        if (fake->start_request_count > 0) {
            static float output_samples[2 * 1024];
            for (size_t index = 0; index < fake->start_request_count;
                 index++) {
                (void)fake_coreaudio_facade_request_internal(
                    fake, fake->start_request_frame_count, output_samples,
                    sizeof(output_samples) / (2 * sizeof(output_samples[0])));
            }
        }
    }
    return fake->start_result;
}

static audio_output_coreaudio_facade_result fake_quiesce(void *context)
{
    fake_coreaudio_facade *fake = context;
    record_lifecycle_call(fake, FAKE_COREAUDIO_FACADE_CALL_QUIESCE);
    fake->quiesce_call_count++;
    fake->control_quiesce_call_count++;
    if (fake->callback_in_flight) {
        fake->callback_quiesce_call_count++;
    }
    fake->active = false;
    fake->quiescent = true;
    return fake->quiesce_result;
}

static audio_output_coreaudio_facade_result fake_stop(void *context)
{
    fake_coreaudio_facade *fake = context;
    record_lifecycle_call(fake, FAKE_COREAUDIO_FACADE_CALL_STOP);
    fake->control_stop_call_count++;
    if (fake->callback_in_flight) {
        fake->control_stop_called_during_callback = true;
    }
    fake->active = false;
    return fake->stop_result;
}

static audio_output_coreaudio_facade_result fake_dispose(void *context)
{
    fake_coreaudio_facade *fake = context;
    record_lifecycle_call(fake, FAKE_COREAUDIO_FACADE_CALL_DISPOSE);
    fake->control_dispose_call_count++;
    if (fake->callback_in_flight) {
        fake->control_dispose_called_during_callback = true;
    }
    fake->disposed = true;
    return fake->dispose_result;
}

void fake_coreaudio_facade_init(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_facade_result open_result,
    audio_output_coreaudio_facade_result configure_result,
    audio_output_coreaudio_facade_result start_result)
{
    *fake = (fake_coreaudio_facade){
        .facade = {
            .context = fake,
            .open = fake_open,
            .configure = fake_configure,
            .bind_request = fake_bind_request,
            .start = fake_start,
            .stop = fake_stop,
            .quiesce = fake_quiesce,
            .dispose = fake_dispose,
        },
        .open_result = open_result,
        .configure_result = configure_result,
        .bind_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
        .start_result = start_result,
        .stop_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
        .quiesce_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
        .dispose_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
    };
}

void fake_coreaudio_facade_set_workspace_only(
    fake_coreaudio_facade *fake,
    bool workspace_only)
{
    fake->facade.workspace_only = workspace_only;
}

void fake_coreaudio_facade_set_reported_format(
    fake_coreaudio_facade *fake,
    const audio_output_coreaudio_format *format)
{
    fake->reported_format = *format;
    fake->reported_format_set = true;
}

void fake_coreaudio_facade_record_preparation(fake_coreaudio_facade *fake)
{
    record_lifecycle_call(fake, FAKE_COREAUDIO_FACADE_CALL_PREPARE);
    fake->prepare_call_count++;
}

void fake_coreaudio_facade_set_bind_result(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_facade_result bind_result)
{
    fake->bind_result = bind_result;
}

void fake_coreaudio_facade_request_during_start(
    fake_coreaudio_facade *fake,
    size_t requested_frame_count,
    size_t request_count)
{
    fake->start_request_frame_count = requested_frame_count;
    fake->start_request_count = request_count;
}

void fake_coreaudio_facade_configure_reentrant_start(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_adapter_instance *instance,
    const audio_output_coreaudio_format *format)
{
    fake->reentrant_start_enabled = true;
    fake->reentrant_start_instance = instance;
    fake->reentrant_start_format = format;
    fake->reentrant_start_result = AUDIO_OUTPUT_COREAUDIO_START_REJECTED;
}

static audio_output_submit_result fake_coreaudio_facade_request_internal(
    fake_coreaudio_facade *fake,
    size_t requested_frame_count,
    float *interleaved_samples,
    size_t frame_capacity)
{
    const audio_output_coreaudio_adapter_instance *instance =
        fake->request_context;
    if (!fake->active || fake->request == NULL ||
        (instance != NULL &&
         instance->state != AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ACTIVE)) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    fake->request_trace[fake->request_trace_count] = requested_frame_count;
    fake->request_trace_count++;
    fake->render_request_count++;
    fake->callback_in_flight = true;
    const audio_output_submit_result result = fake->request(
        fake->request_context, requested_frame_count, interleaved_samples,
        frame_capacity);
    fake->callback_in_flight = false;
    if (fake->deferred_stop_pending) {
        fake->deferred_stop_pending = false;
        fake->deferred_stop_admission_result =
            audio_output_coreaudio_adapter_request_stop(
                fake->deferred_stop_instance);
    }
    return result;
}

audio_output_submit_result fake_coreaudio_facade_request(
    fake_coreaudio_facade *fake,
    size_t requested_frame_count)
{
    return fake_coreaudio_facade_request_internal(
        fake, requested_frame_count, NULL, 0);
}

audio_output_submit_result fake_coreaudio_facade_request_with_output(
    fake_coreaudio_facade *fake,
    size_t requested_frame_count,
    float *interleaved_samples,
    size_t frame_capacity)
{
    return fake_coreaudio_facade_request_internal(
        fake, requested_frame_count, interleaved_samples, frame_capacity);
}

void fake_coreaudio_facade_defer_control_stop(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_adapter_instance *instance)
{
    fake->control_stop_request_call_count++;
    fake->control_stop_requested_during_callback = fake->callback_in_flight;
    fake->deferred_stop_instance = instance;
    fake->deferred_stop_pending = true;
}

bool fake_coreaudio_facade_control_request_stop(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_adapter_instance *instance)
{
    fake->control_stop_request_call_count++;
    fake->control_stop_requested_during_callback = fake->callback_in_flight;
    return audio_output_coreaudio_adapter_request_stop(instance);
}

void fake_coreaudio_facade_control_stop(fake_coreaudio_facade *fake)
{
    (void)fake_stop(fake);
}

void fake_coreaudio_facade_control_quiesce(fake_coreaudio_facade *fake)
{
    (void)fake_quiesce(fake);
}

void fake_coreaudio_facade_control_dispose(fake_coreaudio_facade *fake)
{
    (void)fake_dispose(fake);
}
