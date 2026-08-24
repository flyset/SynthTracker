#include "coreaudio_adapter.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdatomic.h>

static_assert(ATOMIC_INT_LOCK_FREE == 2,
              "CoreAudio callback admission must be lock-free");

typedef enum {
    CORE_AUDIO_ADMISSION_CLOSED,
    CORE_AUDIO_ADMISSION_OPEN,
    CORE_AUDIO_ADMISSION_IN_FLIGHT,
    CORE_AUDIO_ADMISSION_CLOSING,
} core_audio_admission_state;

static bool open_callback_admission(
    audio_output_coreaudio_adapter_instance *instance)
{
    int expected = CORE_AUDIO_ADMISSION_CLOSED;
    return atomic_compare_exchange_strong_explicit(
        &instance->callback_admission, &expected,
        CORE_AUDIO_ADMISSION_OPEN, memory_order_acq_rel, memory_order_acquire);
}

static bool admit_callback(
    audio_output_coreaudio_adapter_instance *instance)
{
    int expected = CORE_AUDIO_ADMISSION_OPEN;
    return atomic_compare_exchange_strong_explicit(
        &instance->callback_admission, &expected,
        CORE_AUDIO_ADMISSION_IN_FLIGHT, memory_order_acquire,
        memory_order_relaxed);
}

static void release_callback(
    audio_output_coreaudio_adapter_instance *instance)
{
    int expected = CORE_AUDIO_ADMISSION_IN_FLIGHT;
    if (atomic_compare_exchange_strong_explicit(
        &instance->callback_admission, &expected,
        CORE_AUDIO_ADMISSION_OPEN, memory_order_release, memory_order_relaxed)) {
        return;
    }

    expected = CORE_AUDIO_ADMISSION_CLOSING;
    (void)atomic_compare_exchange_strong_explicit(
        &instance->callback_admission, &expected,
        CORE_AUDIO_ADMISSION_CLOSED, memory_order_release, memory_order_relaxed);
}

static void close_callback_admission_and_drain(
    audio_output_coreaudio_adapter_instance *instance)
{
    for (;;) {
        int current = atomic_load_explicit(&instance->callback_admission,
                                           memory_order_acquire);
        if (current == CORE_AUDIO_ADMISSION_CLOSED) {
            return;
        }
        if (current == CORE_AUDIO_ADMISSION_OPEN) {
            int expected = CORE_AUDIO_ADMISSION_OPEN;
            if (atomic_compare_exchange_weak_explicit(
                    &instance->callback_admission, &expected,
                    CORE_AUDIO_ADMISSION_CLOSED, memory_order_acq_rel,
                    memory_order_acquire)) {
                return;
            }
            continue;
        }
        if (current == CORE_AUDIO_ADMISSION_IN_FLIGHT) {
            int expected = CORE_AUDIO_ADMISSION_IN_FLIGHT;
            if (atomic_compare_exchange_weak_explicit(
                    &instance->callback_admission, &expected,
                    CORE_AUDIO_ADMISSION_CLOSING, memory_order_acq_rel,
                    memory_order_acquire)) {
                current = CORE_AUDIO_ADMISSION_CLOSING;
            }
        }
        if (current == CORE_AUDIO_ADMISSION_CLOSING) {
            while (atomic_load_explicit(&instance->callback_admission,
                                        memory_order_acquire) ==
                   CORE_AUDIO_ADMISSION_CLOSING) {
            }
            return;
        }
    }
}

#ifdef SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
static audio_output_coreaudio_adapter_test_observer test_observer;
static bool test_allocation_failure;
static size_t test_allocation_attempt_count;

void audio_output_coreaudio_adapter_test_set_observer(
    audio_output_coreaudio_adapter_test_observer observer)
{
    test_observer = observer;
}

void audio_output_coreaudio_adapter_test_set_allocation_failure(bool should_fail)
{
    test_allocation_failure = should_fail;
}

void audio_output_coreaudio_adapter_test_reset_allocation_attempt_count(void)
{
    test_allocation_attempt_count = 0;
}

size_t audio_output_coreaudio_adapter_test_allocation_attempt_count(void)
{
    return test_allocation_attempt_count;
}
#endif

static float *allocate_converted_samples(size_t byte_count)
{
#ifdef SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
    test_allocation_attempt_count++;
    if (test_allocation_failure) {
        return NULL;
    }
#endif

    return malloc(byte_count);
}

static bool is_supported_rate(uint32_t sample_rate_hz)
{
    return sample_rate_hz == 44100 || sample_rate_hz == 48000;
}

static bool is_strict_static_format(
    const audio_output_coreaudio_format *format)
{
    return format != NULL && format->channel_count == 2 &&
           format->sample_format == AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32 &&
           format->layout == AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED;
}

static bool is_supported_requested_format(
    const audio_output_coreaudio_format *format)
{
    return is_strict_static_format(format) &&
           (format->sample_rate_hz == 0 ||
            is_supported_rate(format->sample_rate_hz));
}

static bool is_supported_configured_format(
    const audio_output_coreaudio_format *format)
{
    return is_strict_static_format(format) &&
           is_supported_rate(format->sample_rate_hz);
}

static bool configured_rate_matches_request(
    const audio_output_coreaudio_format *requested_format,
    const audio_output_coreaudio_format *configured_format)
{
    return requested_format != NULL && configured_format != NULL &&
           (requested_format->sample_rate_hz == 0 ||
            requested_format->sample_rate_hz == configured_format->sample_rate_hz);
}

static bool is_compatible_configured_format(
    const audio_output_coreaudio_format *requested_format,
    const audio_output_coreaudio_format *configured_format)
{
    return is_supported_configured_format(configured_format) &&
           configured_rate_matches_request(requested_format,
                                           configured_format);
}

static bool prepare_route_without_callback(
    void *context,
    const audio_output_coreaudio_format *format)
{
    (void)context;
    (void)format;
    return true;
}

static bool is_valid_workspace(
    const audio_output_coreaudio_float32_workspace *workspace)
{
    return workspace != NULL && workspace->samples != NULL &&
           workspace->frame_capacity > 0 &&
           workspace->frame_capacity <= SIZE_MAX / 2;
}

static audio_output_coreaudio_start_result map_facade_result(
    audio_output_coreaudio_facade_result result)
{
    if (result == AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE) {
        return AUDIO_OUTPUT_COREAUDIO_START_UNAVAILABLE;
    }
    return AUDIO_OUTPUT_COREAUDIO_START_REJECTED;
}

static audio_output_coreaudio_start_result dispose_failed_start(
    const audio_output_coreaudio_facade *facade,
    audio_output_coreaudio_facade_result result)
{
    (void)facade->dispose(facade->context);
    return map_facade_result(result);
}

static audio_output_coreaudio_start_result rollback_instance_start(
    audio_output_coreaudio_adapter_instance *instance,
    audio_output_coreaudio_facade_result result)
{
    close_callback_admission_and_drain(instance);
    instance->in_request = false;
    instance->state = AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE;
    return dispose_failed_start(instance->facade, result);
}

typedef struct {
    audio_output_coreaudio_adapter_instance *instance;
    audio_output_coreaudio_float32_workspace workspace;
} audio_output_coreaudio_workspace_delivery_context;

static audio_frame_block audio_output_coreaudio_adapter_workspace_renderer(
    void *context,
    size_t requested_frame_count)
{
    audio_output_coreaudio_workspace_delivery_context *delivery_context =
        context;
    audio_output_coreaudio_adapter_instance *instance =
        delivery_context == NULL ? NULL : delivery_context->instance;
    if (instance == NULL || instance->route.renderer == NULL) {
        return (audio_frame_block){ .frame_count = 0, .frames = NULL };
    }
    return instance->route.renderer(instance->route.context,
                                    requested_frame_count);
}

static audio_output_submit_result
audio_output_coreaudio_adapter_workspace_delivery(
    void *context,
    const audio_frame_block *block)
{
    audio_output_coreaudio_workspace_delivery_context *delivery_context =
        context;
    audio_output_coreaudio_adapter_instance *instance =
        delivery_context == NULL ? NULL : delivery_context->instance;
    if (block == NULL) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }
    if (block->frame_count == 0) {
        return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
    }
    if (instance == NULL ||
        block->frame_count > delivery_context->workspace.frame_capacity ||
        delivery_context->workspace.samples == NULL ||
        block->frame_count > SIZE_MAX / 2) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    const size_t sample_count = block->frame_count * 2;
    for (size_t frame_index = 0; frame_index < block->frame_count;
         frame_index++) {
        const size_t sample_index = frame_index * 2;
        delivery_context->workspace.samples[sample_index] =
            (float)block->frames[frame_index].left / 2147483648.0f;
        delivery_context->workspace.samples[sample_index + 1] =
            (float)block->frames[frame_index].right / 2147483648.0f;
    }

#ifdef SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
    if (test_observer != NULL) {
        test_observer(delivery_context->workspace.samples, sample_count);
    }
#endif

    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}

static audio_output_submit_result
audio_output_coreaudio_adapter_request(void *context,
                                       size_t requested_frame_count,
                                       float *interleaved_samples,
                                       size_t frame_capacity)
{
    audio_output_coreaudio_adapter_instance *instance = context;
    if (instance == NULL || !admit_callback(instance)) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    const bool hal_route =
        instance->facade != NULL && instance->facade->workspace_only;
    if (hal_route) {
        if (requested_frame_count == 0) {
            release_callback(instance);
            return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
        }
        if (interleaved_samples == NULL ||
            frame_capacity < requested_frame_count ||
            frame_capacity > SIZE_MAX / 2) {
            release_callback(instance);
            return AUDIO_OUTPUT_SUBMIT_REJECTED;
        }
    }

    audio_output_frame_renderer renderer = instance->route.renderer;
    audio_output_frame_delivery delivery = instance->route.delivery;
    void *route_context = instance->route.context;
    audio_output_coreaudio_workspace_delivery_context workspace_context = {
        .instance = instance,
        .workspace = instance->workspace,
    };
    if (instance->delivery_mode ==
        AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE) {
        if (!hal_route && interleaved_samples != NULL) {
            workspace_context.workspace = (audio_output_coreaudio_float32_workspace){
                .samples = interleaved_samples,
                .frame_capacity = frame_capacity,
            };
        }
        renderer = audio_output_coreaudio_adapter_workspace_renderer;
        delivery = audio_output_coreaudio_adapter_workspace_delivery;
        route_context = &workspace_context;
    } else if (instance->delivery_mode !=
               AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_DIRECT) {
        release_callback(instance);
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    instance->in_request = true;
    const audio_output_submit_result result =
        audio_output_coordinate_frame_request(
            requested_frame_count, renderer, delivery, route_context);
    instance->in_request = false;

    if (hal_route && result == AUDIO_OUTPUT_SUBMIT_ACCEPTED) {
        const size_t sample_count = requested_frame_count * 2;
        for (size_t sample_index = 0; sample_index < sample_count;
             sample_index++) {
            interleaved_samples[sample_index] =
                workspace_context.workspace.samples[sample_index];
        }
    }
    release_callback(instance);

    return result == AUDIO_OUTPUT_SUBMIT_ACCEPTED
               ? AUDIO_OUTPUT_SUBMIT_ACCEPTED
               : AUDIO_OUTPUT_SUBMIT_REJECTED;
}

audio_output_coreaudio_start_result audio_output_coreaudio_adapter_start(
    const audio_output_coreaudio_facade *facade,
    const audio_output_coreaudio_format *format)
{
    if (!is_supported_requested_format(format) || facade == NULL ||
        facade->open == NULL ||
        facade->configure == NULL || facade->start == NULL ||
        facade->dispose == NULL) {
        return AUDIO_OUTPUT_COREAUDIO_START_REJECTED;
    }

    audio_output_coreaudio_facade_result result =
        facade->open(facade->context);
    if (result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        return dispose_failed_start(facade, result);
    }

    audio_output_coreaudio_format configured_format = {0};
    result = facade->configure(facade->context, format, &configured_format);
    if (result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        return dispose_failed_start(facade, result);
    }
    if (!is_compatible_configured_format(format, &configured_format)) {
        return dispose_failed_start(
            facade, AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED);
    }

    result = facade->start(facade->context);
    if (result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        return dispose_failed_start(facade, result);
    }

    return AUDIO_OUTPUT_COREAUDIO_START_STARTED;
}

audio_output_coreaudio_start_result
audio_output_coreaudio_adapter_start_default(
    const audio_output_coreaudio_format *format)
{
    return audio_output_coreaudio_adapter_start(
        audio_output_coreaudio_facade_default(), format);
}

audio_output_coreaudio_start_result
audio_output_coreaudio_adapter_start_instance(
    audio_output_coreaudio_adapter_instance *instance,
    const audio_output_coreaudio_format *format)
{
    if (instance == NULL ||
        instance->state != AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE ||
        !is_supported_requested_format(format) ||
        instance->facade == NULL || instance->facade->open == NULL ||
        instance->facade->configure == NULL ||
        instance->facade->bind_request == NULL ||
        instance->facade->start == NULL || instance->facade->quiesce == NULL ||
        instance->facade->stop == NULL || instance->facade->dispose == NULL ||
        instance->route.renderer == NULL ||
        (instance->delivery_mode !=
             AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_DIRECT &&
         instance->delivery_mode !=
             AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE) ||
        (instance->delivery_mode ==
             AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_DIRECT &&
         instance->route.delivery == NULL) ||
        (instance->facade->workspace_only &&
         instance->delivery_mode !=
             AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE)) {
        return AUDIO_OUTPUT_COREAUDIO_START_REJECTED;
    }
    if (instance->delivery_mode ==
            AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE &&
        !is_valid_workspace(&instance->workspace)) {
        return AUDIO_OUTPUT_COREAUDIO_START_REJECTED;
    }

    instance->in_request = false;
    atomic_init(&instance->callback_admission, CORE_AUDIO_ADMISSION_CLOSED);

    const audio_output_coreaudio_facade *facade = instance->facade;
    audio_output_coreaudio_facade_result result =
        facade->open(facade->context);
    if (result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        return rollback_instance_start(instance, result);
    }

    audio_output_coreaudio_format configured_format = {0};
    result = facade->configure(facade->context, format, &configured_format);
    if (result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        return rollback_instance_start(instance, result);
    }

    if (!is_compatible_configured_format(format, &configured_format)) {
        return rollback_instance_start(
            instance, AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED);
    }

    const audio_output_coreaudio_route_prepare prepare =
        instance->route.prepare == NULL ? prepare_route_without_callback
                                         : instance->route.prepare;
    if (!prepare(instance->route.context, &configured_format)) {
        return rollback_instance_start(
            instance, AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED);
    }

    result = facade->bind_request(facade->context,
                                  audio_output_coreaudio_adapter_request,
                                  instance);
    if (result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        return rollback_instance_start(instance, result);
    }

    instance->state = AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ACTIVE;
    if (!open_callback_admission(instance)) {
        return rollback_instance_start(
            instance, AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED);
    }

    result = facade->start(facade->context);
    if (result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        return rollback_instance_start(instance, result);
    }

    return AUDIO_OUTPUT_COREAUDIO_START_STARTED;
}

bool audio_output_coreaudio_adapter_request_stop(
    audio_output_coreaudio_adapter_instance *instance)
{
    if (instance == NULL ||
        instance->state != AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ACTIVE) {
        return false;
    }

    for (;;) {
        int current = atomic_load_explicit(&instance->callback_admission,
                                           memory_order_acquire);
        if (current == CORE_AUDIO_ADMISSION_OPEN) {
            int expected = CORE_AUDIO_ADMISSION_OPEN;
            if (atomic_compare_exchange_weak_explicit(
                    &instance->callback_admission, &expected,
                    CORE_AUDIO_ADMISSION_CLOSED, memory_order_acq_rel,
                    memory_order_acquire)) {
                instance->state =
                    AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_STOP_REQUESTED;
                return true;
            }
            continue;
        }
        if (current == CORE_AUDIO_ADMISSION_IN_FLIGHT) {
            int expected = CORE_AUDIO_ADMISSION_IN_FLIGHT;
            if (atomic_compare_exchange_weak_explicit(
                    &instance->callback_admission, &expected,
                    CORE_AUDIO_ADMISSION_CLOSING, memory_order_acq_rel,
                    memory_order_acquire)) {
                instance->state =
                    AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_STOP_REQUESTED;
                return true;
            }
            continue;
        }
        return false;
    }
}

audio_output_coreaudio_facade_result
audio_output_coreaudio_adapter_stop_instance(
    audio_output_coreaudio_adapter_instance *instance)
{
    if (instance == NULL ||
        (instance->state != AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ACTIVE &&
         instance->state !=
             AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_STOP_REQUESTED) ||
         instance->facade == NULL || instance->facade->stop == NULL ||
         instance->facade->quiesce == NULL || instance->facade->dispose == NULL) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    close_callback_admission_and_drain(instance);
    instance->state = AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_STOP_REQUESTED;

    audio_output_coreaudio_facade_result first_failure =
        AUDIO_OUTPUT_COREAUDIO_FACADE_OK;
    const audio_output_coreaudio_facade_result stop_result =
        instance->facade->stop(instance->facade->context);
    if (stop_result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        first_failure = stop_result;
    }

    const audio_output_coreaudio_facade_result quiesce_result =
        instance->facade->quiesce(instance->facade->context);
    if (first_failure == AUDIO_OUTPUT_COREAUDIO_FACADE_OK &&
        quiesce_result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        first_failure = quiesce_result;
    }

    const audio_output_coreaudio_facade_result dispose_result =
        instance->facade->dispose(instance->facade->context);
    if (first_failure == AUDIO_OUTPUT_COREAUDIO_FACADE_OK &&
        dispose_result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        first_failure = dispose_result;
    }

    instance->in_request = false;
    if (first_failure != AUDIO_OUTPUT_COREAUDIO_FACADE_OK) {
        instance->state = AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ERROR;
        return first_failure;
    }

    instance->state = AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_QUIESCENT;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_OK;
}

audio_output_submit_result audio_output_coreaudio_adapter_submit(
    const audio_frame_block *block)
{
    if (block == NULL || (block->frame_count > 0 && block->frames == NULL)) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    if (block->frame_count == 0) {
#ifdef SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
        if (test_observer != NULL) {
            test_observer(NULL, 0);
        }
#endif
        return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
    }

    if (block->frame_count > SIZE_MAX / 2) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    const size_t sample_count = block->frame_count * 2;
    if (sample_count > SIZE_MAX / sizeof(float)) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    const size_t byte_count = sample_count * sizeof(float);
    float *converted_samples = allocate_converted_samples(byte_count);
    if (converted_samples == NULL) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    for (size_t frame_index = 0; frame_index < block->frame_count;
         frame_index++) {
        const size_t sample_index = frame_index * 2;
        converted_samples[sample_index] =
            (float)block->frames[frame_index].left / 2147483648.0f;
        converted_samples[sample_index + 1] =
            (float)block->frames[frame_index].right / 2147483648.0f;
    }

#ifdef SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
    if (test_observer != NULL) {
        test_observer(converted_samples, sample_count);
    }
#endif

    free(converted_samples);
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}
