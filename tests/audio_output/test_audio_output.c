#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#if defined(__APPLE__)
#include <signal.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "audio_output.h"
#include "adapters/coreaudio_adapter.h"
#include "fake_coreaudio_facade.h"

static_assert(offsetof(audio_output_null_adapter, accepted_block_count) == 0);
static_assert(offsetof(audio_output_null_adapter, accepted_frame_count) ==
              sizeof(size_t));
static_assert(sizeof(audio_output_null_adapter) == 2 * sizeof(size_t));

#if defined(__APPLE__)
enum { OBSERVED_SAMPLE_CAPACITY = 8 };

static float observed_samples[OBSERVED_SAMPLE_CAPACITY];
static const float *observed_sample_pointer;
static size_t observed_sample_count;
static size_t observed_submission_count;

static void reset_coreaudio_observation(void)
{
    observed_sample_pointer = NULL;
    observed_sample_count = 0;
    observed_submission_count = 0;
}

static void record_coreaudio_output(const float *samples, size_t sample_count)
{
    assert_true(sample_count <= OBSERVED_SAMPLE_CAPACITY);
    if (sample_count > 0) {
        assert_non_null(samples);
    }

    observed_sample_pointer = samples;
    observed_sample_count = sample_count;
    observed_submission_count++;
    for (size_t index = 0; index < sample_count; index++) {
        observed_samples[index] = samples[index];
    }
}
#endif

static void zero_frame_blocks_are_accepted_with_or_without_frames(void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    const audio_frame_block null_frames = {
        .frame_count = 0,
        .frames = NULL,
    };
    const audio_frame caller_frame = { .left = 1, .right = -1 };
    const audio_frame_block nonnull_frames = {
        .frame_count = 0,
        .frames = &caller_frame,
    };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &null_frames),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(audio_output_null_adapter_submit(&adapter, &nonnull_frames),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(adapter.accepted_block_count, 2);
    assert_int_equal(adapter.accepted_frame_count, 0);
}

static void signed_32_interleaved_frames_are_accepted_and_counted(void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    const audio_frame frames[] = {
        { .left = INT32_MIN, .right = INT32_MAX },
        { .left = -123456789, .right = 987654321 },
        { .left = 0, .right = -1 },
    };
    const audio_frame_block one_frame = {
        .frame_count = 1,
        .frames = frames,
    };
    const audio_frame_block two_frames = {
        .frame_count = 2,
        .frames = frames + 1,
    };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &one_frame),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(audio_output_null_adapter_submit(&adapter, &two_frames),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(adapter.accepted_block_count, 2);
    assert_int_equal(adapter.accepted_frame_count, 3);
}

static void nonzero_frame_blocks_without_frames_are_rejected_and_not_counted(
    void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    const audio_frame_block block = {
        .frame_count = 1,
        .frames = NULL,
    };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &block),
                     AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(adapter.accepted_block_count, 0);
    assert_int_equal(adapter.accepted_frame_count, 0);
}

static void accepted_frames_are_not_retained_after_submit(void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    audio_frame first_frames[] = {
        { .left = 100, .right = -200 },
        { .left = 300, .right = -400 },
    };
    const audio_frame second_frames[] = {
        { .left = INT32_MIN, .right = INT32_MAX },
    };
    const audio_frame_block first_block = {
        .frame_count = 2,
        .frames = first_frames,
    };
    const audio_frame_block second_block = {
        .frame_count = 1,
        .frames = second_frames,
    };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &first_block),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);

    first_frames[0] = (audio_frame){ .left = INT32_MAX, .right = INT32_MIN };
    first_frames[1] = (audio_frame){ .left = 0, .right = 0 };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &second_block),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);

    const audio_output_null_adapter_test_snapshot expected_snapshot = {
        .accepted_block_count = 2,
        .accepted_frame_count = 3,
    };

    const audio_output_null_adapter_test_snapshot actual_snapshot =
        audio_output_null_adapter_test_inspect(&adapter);

    assert_int_equal(actual_snapshot.accepted_block_count,
                     expected_snapshot.accepted_block_count);
    assert_int_equal(actual_snapshot.accepted_frame_count,
                     expected_snapshot.accepted_frame_count);
}

static void private_dispatch_preserves_interleaved_float32_order_without_device(
    void **state)
{
    (void)state;

#if defined(__APPLE__)
    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);
#endif

    audio_output_null_adapter fallback = {0};
    const audio_frame frames[] = {
        { .left = INT32_MIN, .right = 17 },
        { .left = -123456789, .right = 987654321 },
        { .left = 0, .right = INT32_MAX },
    };
    const audio_frame_block block = {
        .frame_count = 3,
        .frames = frames,
    };

    assert_int_equal(audio_output_dispatch_submit(&fallback, &block),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);

#if defined(__APPLE__)
    assert_int_equal(observed_submission_count, 1);
    assert_int_equal(observed_sample_count, 6);
    const float expected_samples[] = {
        -1.0f,
        17.0f / 2147483648.0f,
        -123456789.0f / 2147483648.0f,
        987654321.0f / 2147483648.0f,
        0.0f,
        1.0f,
    };
    assert_memory_equal(observed_samples, expected_samples,
                        sizeof(expected_samples));
    assert_int_equal(fallback.accepted_block_count, 0);
    assert_int_equal(fallback.accepted_frame_count, 0);
    audio_output_coreaudio_adapter_test_set_observer(NULL);
#else
    assert_int_equal(fallback.accepted_block_count, 1);
    assert_int_equal(fallback.accepted_frame_count, 3);
#endif
}

static void private_dispatch_accepts_zero_frame_blocks_without_device(
    void **state)
{
    (void)state;

#if defined(__APPLE__)
    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);
#endif

    audio_output_null_adapter fallback = {0};
    const audio_frame_block block = {
        .frame_count = 0,
        .frames = NULL,
    };

    assert_int_equal(audio_output_dispatch_submit(&fallback, &block),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);

#if defined(__APPLE__)
    assert_int_equal(observed_submission_count, 1);
    assert_int_equal(observed_sample_count, 0);
    assert_int_equal(fallback.accepted_block_count, 0);
    assert_int_equal(fallback.accepted_frame_count, 0);
    audio_output_coreaudio_adapter_test_set_observer(NULL);
#else
    assert_int_equal(fallback.accepted_block_count, 1);
    assert_int_equal(fallback.accepted_frame_count, 0);
#endif
}

static void private_adapter_converts_signed_32_frames_to_interleaved_float32(
    void **state)
{
    (void)state;

#if defined(__APPLE__)
    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);

    const audio_frame frames[] = {
        { .left = INT32_MIN, .right = INT32_MAX },
        { .left = INT32_MIN + 1, .right = INT32_MAX - 1 },
        { .left = -1073741824, .right = 1073741824 },
        { .left = 0, .right = -1 },
    };
    const audio_frame_block block = {
        .frame_count = 4,
        .frames = frames,
    };

    const audio_output_submit_result result =
        audio_output_coreaudio_adapter_submit(&block);
    audio_output_coreaudio_adapter_test_set_observer(NULL);

    assert_int_equal(result, AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(observed_submission_count, 1);
    assert_int_equal(observed_sample_count, 8);

    const float expected_samples[] = {
        -1.0f,
        1.0f,
        -1.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.0f,
        -0x1p-31f,
    };
    assert_memory_equal(observed_samples, expected_samples,
                        sizeof(expected_samples));
    for (size_t index = 0; index < observed_sample_count; index++) {
        assert_true(observed_samples[index] >= -1.0f);
        assert_true(observed_samples[index] <= 1.0f);
    }
#endif
}

static void private_adapter_rejects_invalid_blocks_before_converted_output(
    void **state)
{
    (void)state;

#if defined(__APPLE__)
    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);

    const audio_frame_block missing_frames = {
        .frame_count = 1,
        .frames = NULL,
    };

    const audio_output_submit_result null_result =
        audio_output_coreaudio_adapter_submit(NULL);
    const audio_output_submit_result missing_frames_result =
        audio_output_coreaudio_adapter_submit(&missing_frames);
    audio_output_coreaudio_adapter_test_set_observer(NULL);

    assert_int_equal(null_result, AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(missing_frames_result, AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(observed_submission_count, 0);
    assert_int_equal(observed_sample_count, 0);
#endif
}

static void private_adapter_rejects_stereo_sample_count_overflow_before_output(
    void **state)
{
    (void)state;

#if defined(__APPLE__)
    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_reset_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);
    audio_output_coreaudio_adapter_test_set_allocation_failure(true);

    const audio_frame frame = { .left = 1, .right = -1 };
    const audio_frame_block overflowing_block = {
        .frame_count = SIZE_MAX / 2 + 1,
        .frames = &frame,
    };

    const audio_output_submit_result result =
        audio_output_coreaudio_adapter_submit(&overflowing_block);
    const size_t allocation_attempt_count =
        audio_output_coreaudio_adapter_test_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_allocation_failure(false);
    audio_output_coreaudio_adapter_test_set_observer(NULL);

    assert_int_equal(result, AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(allocation_attempt_count, 0);
    assert_int_equal(observed_submission_count, 0);
    assert_int_equal(observed_sample_count, 0);
#endif
}

static void private_adapter_rejects_float32_byte_size_overflow_before_output(
    void **state)
{
    (void)state;

#if defined(__APPLE__)
    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_reset_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);
    audio_output_coreaudio_adapter_test_set_allocation_failure(true);

    const size_t bytes_per_frame = 2 * sizeof(float);
    const size_t overflowing_frame_count = SIZE_MAX / bytes_per_frame + 1;
    assert_true(overflowing_frame_count <= SIZE_MAX / 2);

    const audio_frame frame = { .left = 1, .right = -1 };
    const audio_frame_block overflowing_block = {
        .frame_count = overflowing_frame_count,
        .frames = &frame,
    };

    const audio_output_submit_result result =
        audio_output_coreaudio_adapter_submit(&overflowing_block);
    const size_t allocation_attempt_count =
        audio_output_coreaudio_adapter_test_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_allocation_failure(false);
    audio_output_coreaudio_adapter_test_set_observer(NULL);

    assert_int_equal(result, AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(allocation_attempt_count, 0);
    assert_int_equal(observed_submission_count, 0);
    assert_int_equal(observed_sample_count, 0);
#endif
}

static void private_adapter_rejects_deterministic_allocation_failure_before_output(
    void **state)
{
    (void)state;

#if defined(__APPLE__)
    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_reset_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);
    audio_output_coreaudio_adapter_test_set_allocation_failure(true);

    const audio_frame frame = { .left = 123, .right = -456 };
    const audio_frame_block block = {
        .frame_count = 1,
        .frames = &frame,
    };

    const audio_output_submit_result result =
        audio_output_coreaudio_adapter_submit(&block);
    const size_t allocation_attempt_count =
        audio_output_coreaudio_adapter_test_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_allocation_failure(false);
    audio_output_coreaudio_adapter_test_set_observer(NULL);

    assert_int_equal(result, AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(allocation_attempt_count, 1);
    assert_int_equal(observed_submission_count, 0);
    assert_int_equal(observed_sample_count, 0);
#endif
}

static void coreaudio_lifecycle_accepts_only_strict_stereo_float32(
    void **state)
{
    (void)state;

#if defined(__APPLE__)
    struct lifecycle_case {
        const char *name;
        audio_output_coreaudio_format format;
        audio_output_coreaudio_facade_result open_result;
        audio_output_coreaudio_facade_result configure_result;
        audio_output_coreaudio_facade_result start_result;
        audio_output_coreaudio_start_result expected_result;
        fake_coreaudio_facade_lifecycle_call expected_trace[
            FAKE_COREAUDIO_FACADE_LIFECYCLE_TRACE_CAPACITY];
        size_t expected_trace_count;
        bool expected_dispose;
    };

    const struct lifecycle_case cases[] = {
        {
            .name = "stereo interleaved Float32 at 44.1 kHz",
            .format = {
                .sample_rate_hz = 44100,
                .channel_count = 2,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_STARTED,
            .expected_trace = {
                FAKE_COREAUDIO_FACADE_CALL_OPEN,
                FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
                FAKE_COREAUDIO_FACADE_CALL_START,
            },
            .expected_trace_count = 3,
        },
        {
            .name = "stereo interleaved Float32 at 48 kHz",
            .format = {
                .sample_rate_hz = 48000,
                .channel_count = 2,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_STARTED,
            .expected_trace = {
                FAKE_COREAUDIO_FACADE_CALL_OPEN,
                FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
                FAKE_COREAUDIO_FACADE_CALL_START,
            },
            .expected_trace_count = 3,
        },
        {
            .name = "stereo interleaved Float32 at unsupported rate",
            .format = {
                .sample_rate_hz = 32000,
                .channel_count = 2,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_REJECTED,
            .expected_trace_count = 0,
        },
        {
            .name = "mono interleaved Float32",
            .format = {
                .sample_rate_hz = 44100,
                .channel_count = 1,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_REJECTED,
            .expected_trace_count = 0,
        },
        {
            .name = "three-channel interleaved Float32",
            .format = {
                .sample_rate_hz = 44100,
                .channel_count = 3,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_REJECTED,
            .expected_trace_count = 0,
        },
        {
            .name = "stereo planar Float32",
            .format = {
                .sample_rate_hz = 44100,
                .channel_count = 2,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_PLANAR,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_REJECTED,
            .expected_trace_count = 0,
        },
        {
            .name = "stereo interleaved non-Float32",
            .format = {
                .sample_rate_hz = 44100,
                .channel_count = 2,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_INT16,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_REJECTED,
            .expected_trace_count = 0,
        },
        {
            .name = "open failure",
            .format = {
                .sample_rate_hz = 44100,
                .channel_count = 2,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_REJECTED,
            .expected_trace = {
                FAKE_COREAUDIO_FACADE_CALL_OPEN,
                FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
            },
            .expected_trace_count = 2,
            .expected_dispose = true,
        },
        {
            .name = "configure failure",
            .format = {
                .sample_rate_hz = 44100,
                .channel_count = 2,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_REJECTED,
            .expected_trace = {
                FAKE_COREAUDIO_FACADE_CALL_OPEN,
                FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
                FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
            },
            .expected_trace_count = 3,
            .expected_dispose = true,
        },
        {
            .name = "start failure",
            .format = {
                .sample_rate_hz = 44100,
                .channel_count = 2,
                .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
                .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
            },
            .open_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .start_result = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
            .expected_result = AUDIO_OUTPUT_COREAUDIO_START_REJECTED,
            .expected_trace = {
                FAKE_COREAUDIO_FACADE_CALL_OPEN,
                FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
                FAKE_COREAUDIO_FACADE_CALL_START,
                FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
            },
            .expected_trace_count = 4,
            .expected_dispose = true,
        },
    };

    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        fake_coreaudio_facade fake;
        fake_coreaudio_facade_init(&fake, cases[index].open_result,
                                   cases[index].configure_result,
                                   cases[index].start_result);

        const audio_output_coreaudio_start_result actual_result =
            audio_output_coreaudio_adapter_start(&fake.facade,
                                                 &cases[index].format);
        if (actual_result != cases[index].expected_result) {
            fail_msg("lifecycle case '%s': expected %d, got %d",
                     cases[index].name, cases[index].expected_result,
                     actual_result);
        }

        assert_int_equal(fake.lifecycle_trace_count,
                         cases[index].expected_trace_count);
        for (size_t call_index = 0;
             call_index < cases[index].expected_trace_count; call_index++) {
            assert_int_equal(fake.lifecycle_trace[call_index],
                             cases[index].expected_trace[call_index]);
        }
        const size_t expected_stage_trace_count =
            cases[index].expected_trace_count -
            (cases[index].expected_dispose ? 1 : 0);
        assert_int_equal(fake.open_call_count,
                         expected_stage_trace_count >= 1);
        assert_int_equal(fake.configure_call_count,
                         expected_stage_trace_count >= 2);
        assert_int_equal(fake.start_call_count,
                         expected_stage_trace_count >= 3);
        assert_int_equal(fake.control_dispose_call_count,
                         cases[index].expected_dispose);
        assert_int_equal(fake.disposed, cases[index].expected_dispose);
        assert_int_equal(fake.active, cases[index].expected_result ==
                                      AUDIO_OUTPUT_COREAUDIO_START_STARTED);
        assert_int_equal(fake.render_request_count, 0);
    }
#endif
}

enum {
    COORDINATOR_REQUEST_CAPACITY = 3,
    COORDINATOR_EVENT_CAPACITY = 2 * COORDINATOR_REQUEST_CAPACITY,
};

typedef enum {
    COORDINATOR_RENDER_EVENT,
    COORDINATOR_DELIVERY_EVENT,
} coordinator_event;

typedef struct {
    audio_frame_block rendered_blocks[COORDINATOR_REQUEST_CAPACITY];
    size_t render_call_count;
    size_t requested_frame_counts[COORDINATOR_REQUEST_CAPACITY];
    audio_frame_block delivered_blocks[COORDINATOR_REQUEST_CAPACITY];
    size_t delivery_call_count;
    coordinator_event events[COORDINATOR_EVENT_CAPACITY];
    size_t event_count;
} coordinator_fake;

static audio_frame_block coordinator_fake_render(
    void *context,
    size_t requested_frame_count)
{
    coordinator_fake *fake = context;
    const size_t call_index = fake->render_call_count;

    assert_true(call_index < COORDINATOR_REQUEST_CAPACITY);
    assert_true(fake->event_count < COORDINATOR_EVENT_CAPACITY);
    fake->requested_frame_counts[call_index] = requested_frame_count;
    fake->render_call_count++;
    fake->events[fake->event_count] = COORDINATOR_RENDER_EVENT;
    fake->event_count++;
    return fake->rendered_blocks[call_index];
}

static audio_output_submit_result coordinator_fake_deliver(
    void *context,
    const audio_frame_block *block)
{
    coordinator_fake *fake = context;
    const size_t call_index = fake->delivery_call_count;

    assert_true(call_index < COORDINATOR_REQUEST_CAPACITY);
    assert_non_null(block);
    assert_true(fake->event_count < COORDINATOR_EVENT_CAPACITY);
    fake->delivered_blocks[call_index] = *block;
    fake->delivery_call_count++;
    fake->events[fake->event_count] = COORDINATOR_DELIVERY_EVENT;
    fake->event_count++;
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}

static void coordinator_accepts_zero_and_variable_exact_requests_and_rejects(
    void **state)
{
    (void)state;

    const audio_frame one_frames[] = {
        { .left = 11, .right = -11 },
    };
    const audio_frame three_frames[] = {
        { .left = 31, .right = -31 },
        { .left = 32, .right = -32 },
        { .left = 33, .right = -33 },
    };
    const audio_frame two_frames[] = {
        { .left = 21, .right = -21 },
        { .left = 22, .right = -22 },
    };
    const audio_frame short_frames[] = {
        { .left = 41, .right = -41 },
        { .left = 42, .right = -42 },
    };
    const audio_frame long_frames[] = {
        { .left = 51, .right = -51 },
        { .left = 52, .right = -52 },
        { .left = 53, .right = -53 },
        { .left = 54, .right = -54 },
    };

    const audio_frame_block zero_block = {
        .frame_count = 0,
        .frames = NULL,
    };
    const audio_frame_block exact_blocks[] = {
        { .frame_count = 1, .frames = one_frames },
        { .frame_count = 3, .frames = three_frames },
        { .frame_count = 2, .frames = two_frames },
    };

    coordinator_fake zero_fake = {0};
    zero_fake.rendered_blocks[0] = zero_block;
    assert_int_equal(
        audio_output_coordinate_frame_request(
            0, coordinator_fake_render, coordinator_fake_deliver, &zero_fake),
        AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(zero_fake.render_call_count, 1);
    assert_int_equal(zero_fake.requested_frame_counts[0], 0);
    assert_int_equal(zero_fake.delivery_call_count, 1);
    assert_int_equal(zero_fake.delivered_blocks[0].frame_count, 0);
    assert_null(zero_fake.delivered_blocks[0].frames);
    assert_int_equal(zero_fake.event_count, 2);
    assert_int_equal(zero_fake.events[0], COORDINATOR_RENDER_EVENT);
    assert_int_equal(zero_fake.events[1], COORDINATOR_DELIVERY_EVENT);

    coordinator_fake exact_fake = {0};
    for (size_t index = 0; index < COORDINATOR_REQUEST_CAPACITY; index++) {
        exact_fake.rendered_blocks[index] = exact_blocks[index];
    }

    const size_t requested_frame_counts[] = { 1, 3, 2 };
    for (size_t index = 0; index < COORDINATOR_REQUEST_CAPACITY; index++) {
        assert_int_equal(
            audio_output_coordinate_frame_request(
                requested_frame_counts[index], coordinator_fake_render,
                coordinator_fake_deliver, &exact_fake),
            AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    }

    assert_int_equal(exact_fake.render_call_count,
                     COORDINATOR_REQUEST_CAPACITY);
    assert_int_equal(exact_fake.delivery_call_count,
                     COORDINATOR_REQUEST_CAPACITY);
    assert_int_equal(exact_fake.event_count, COORDINATOR_EVENT_CAPACITY);
    for (size_t index = 0; index < COORDINATOR_REQUEST_CAPACITY; index++) {
        assert_int_equal(exact_fake.requested_frame_counts[index],
                         requested_frame_counts[index]);
        assert_int_equal(exact_fake.delivered_blocks[index].frame_count,
                         exact_blocks[index].frame_count);
        assert_true(exact_fake.delivered_blocks[index].frames ==
                    exact_blocks[index].frames);
        assert_int_equal(exact_fake.events[index * 2],
                         COORDINATOR_RENDER_EVENT);
        assert_int_equal(exact_fake.events[index * 2 + 1],
                         COORDINATOR_DELIVERY_EVENT);
    }

    const struct invalid_render_case {
        size_t requested_frame_count;
        audio_frame_block rendered_block;
    } invalid_cases[] = {
        {
            .requested_frame_count = 3,
            .rendered_block = {
                .frame_count = 2,
                .frames = short_frames,
            },
        },
        {
            .requested_frame_count = 3,
            .rendered_block = {
                .frame_count = 4,
                .frames = long_frames,
            },
        },
        {
            .requested_frame_count = 3,
            .rendered_block = {
                .frame_count = 3,
                .frames = NULL,
            },
        },
    };

    for (size_t index = 0; index < sizeof(invalid_cases) /
                                      sizeof(invalid_cases[0]); index++) {
        coordinator_fake fake = {0};
        fake.rendered_blocks[0] = invalid_cases[index].rendered_block;

        assert_int_equal(
            audio_output_coordinate_frame_request(
                invalid_cases[index].requested_frame_count,
                coordinator_fake_render, coordinator_fake_deliver, &fake),
            AUDIO_OUTPUT_SUBMIT_REJECTED);
        assert_int_equal(fake.render_call_count, 1);
        assert_int_equal(fake.requested_frame_counts[0], 3);
        assert_int_equal(fake.delivery_call_count, 0);
        assert_int_equal(fake.event_count, 1);
        assert_int_equal(fake.events[0], COORDINATOR_RENDER_EVENT);
    }
}

#if defined(__APPLE__)
enum {
    BOUND_ROUTE_REQUEST_CAPACITY = 4,
};

enum {
    CORE_AUDIO_ADMISSION_IN_FLIGHT_FOR_TEST = 2,
};

static volatile sig_atomic_t hal_copy_probe_fault_count;
static volatile sig_atomic_t hal_copy_probe_admission_at_fault;
static void *hal_copy_probe_page;
static size_t hal_copy_probe_page_size;
static audio_output_coreaudio_adapter_instance *hal_copy_probe_instance;

static void hal_copy_probe_signal_handler(
    int signal_number,
    siginfo_t *signal_info,
    void *context)
{
    (void)context;
    if (signal_info == NULL || hal_copy_probe_page == NULL ||
        hal_copy_probe_instance == NULL ||
        (signal_number != SIGSEGV && signal_number != SIGBUS)) {
        _exit(128 + signal_number);
    }

    const uintptr_t fault_address = (uintptr_t)signal_info->si_addr;
    const uintptr_t page_start = (uintptr_t)hal_copy_probe_page;
    if (fault_address < page_start ||
        fault_address >= page_start + hal_copy_probe_page_size) {
        _exit(128 + signal_number);
    }

    hal_copy_probe_admission_at_fault = atomic_load_explicit(
        &hal_copy_probe_instance->callback_admission, memory_order_acquire);
    hal_copy_probe_fault_count++;
    if (mprotect(hal_copy_probe_page, hal_copy_probe_page_size,
                 PROT_READ | PROT_WRITE) != 0) {
        _exit(128 + signal_number);
    }
}

typedef struct {
    audio_frame_block rendered_blocks[BOUND_ROUTE_REQUEST_CAPACITY];
    size_t expected_requests[BOUND_ROUTE_REQUEST_CAPACITY];
    size_t render_call_count;
    audio_frame_block delivered_blocks[BOUND_ROUTE_REQUEST_CAPACITY];
    size_t delivery_call_count;
    const void *expected_context;
} bound_route_recording_sink;

static audio_frame_block bound_route_renderer(
    void *context,
    size_t requested_frame_count)
{
    bound_route_recording_sink *sink = context;
    const size_t call_index = sink->render_call_count;

    assert_true(context == sink->expected_context);
    assert_true(call_index < BOUND_ROUTE_REQUEST_CAPACITY);
    assert_int_equal(requested_frame_count, sink->expected_requests[call_index]);
    sink->render_call_count++;
    return sink->rendered_blocks[call_index];
}

static audio_output_submit_result bound_route_recording_sink_deliver(
    void *context,
    const audio_frame_block *block)
{
    bound_route_recording_sink *sink = context;
    const size_t call_index = sink->delivery_call_count;

    assert_true(context == sink->expected_context);
    assert_non_null(block);
    assert_true(call_index < BOUND_ROUTE_REQUEST_CAPACITY);
    sink->delivered_blocks[call_index] = *block;
    sink->delivery_call_count++;
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}

static void coreaudio_bound_instance_hands_off_exact_requests_without_conversion(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_frame one_frames[] = {
        { .left = 11, .right = -11 },
    };
    const audio_frame three_frames[] = {
        { .left = 31, .right = -31 },
        { .left = 32, .right = -32 },
        { .left = 33, .right = -33 },
    };
    const audio_frame two_frames[] = {
        { .left = 21, .right = -21 },
        { .left = 22, .right = -22 },
    };

    bound_route_recording_sink sink = {0};
    sink.expected_requests[0] = 0;
    sink.expected_requests[1] = 1;
    sink.expected_requests[2] = 3;
    sink.expected_requests[3] = 2;
    sink.rendered_blocks[0] = (audio_frame_block){
        .frame_count = 0,
        .frames = NULL,
    };
    sink.rendered_blocks[1] = (audio_frame_block){
        .frame_count = 1,
        .frames = one_frames,
    };
    sink.rendered_blocks[2] = (audio_frame_block){
        .frame_count = 3,
        .frames = three_frames,
    };
    sink.rendered_blocks[3] = (audio_frame_block){
        .frame_count = 2,
        .frames = two_frames,
    };
    sink.expected_context = &sink;

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = bound_route_recording_sink_deliver,
            .context = &sink,
        },
    };

    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_reset_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);

    const audio_output_coreaudio_start_result start_result =
        audio_output_coreaudio_adapter_start_instance(&instance, &format);
    assert_int_equal(start_result, AUDIO_OUTPUT_COREAUDIO_START_STARTED);

    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_START,
    };
    assert_int_equal(fake.lifecycle_trace_count,
                     sizeof(expected_lifecycle) / sizeof(expected_lifecycle[0]));
    for (size_t index = 0; index < sizeof(expected_lifecycle) /
                                      sizeof(expected_lifecycle[0]); index++) {
        assert_int_equal(fake.lifecycle_trace[index], expected_lifecycle[index]);
    }
    assert_int_equal(fake.bind_request_call_count, 1);
    assert_non_null(fake.request);
    assert_true(fake.request_context == &instance);
    assert_true(fake.active);

    const size_t request_counts[] = { 0, 1, 3, 2 };
    for (size_t index = 0; index < BOUND_ROUTE_REQUEST_CAPACITY; index++) {
        assert_int_equal(fake_coreaudio_facade_request(&fake, request_counts[index]),
                         AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    }

    assert_int_equal(fake.request_trace_count, BOUND_ROUTE_REQUEST_CAPACITY);
    for (size_t index = 0; index < BOUND_ROUTE_REQUEST_CAPACITY; index++) {
        assert_int_equal(fake.request_trace[index], request_counts[index]);
    }
    assert_int_equal(sink.render_call_count, BOUND_ROUTE_REQUEST_CAPACITY);
    assert_int_equal(sink.delivery_call_count, BOUND_ROUTE_REQUEST_CAPACITY);

    const audio_frame_block expected_blocks[] = {
        sink.rendered_blocks[0],
        sink.rendered_blocks[1],
        sink.rendered_blocks[2],
        sink.rendered_blocks[3],
    };
    for (size_t index = 0; index < BOUND_ROUTE_REQUEST_CAPACITY; index++) {
        assert_int_equal(sink.delivered_blocks[index].frame_count,
                         expected_blocks[index].frame_count);
        assert_true(sink.delivered_blocks[index].frames ==
                    expected_blocks[index].frames);
        if (expected_blocks[index].frame_count > 0) {
            assert_memory_equal(
                sink.delivered_blocks[index].frames,
                expected_blocks[index].frames,
                expected_blocks[index].frame_count * sizeof(audio_frame));
        }
    }
    assert_int_equal(observed_submission_count, 0);
    assert_int_equal(observed_sample_count, 0);
    assert_int_equal(audio_output_coreaudio_adapter_test_allocation_attempt_count(),
                     0);
    audio_output_coreaudio_adapter_test_set_observer(NULL);

    fake_coreaudio_facade failed_fake;
    fake_coreaudio_facade_init(&failed_fake,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED);
    bound_route_recording_sink failed_sink = {0};
    failed_sink.expected_context = &failed_sink;
    audio_output_coreaudio_adapter_instance failed_instance = {
        .facade = &failed_fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = bound_route_recording_sink_deliver,
            .context = &failed_sink,
        },
    };

    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&failed_instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_false(failed_fake.active);
    assert_int_equal(fake_coreaudio_facade_request(&failed_fake, 3),
                     AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(failed_sink.render_call_count, 0);
    assert_int_equal(failed_sink.delivery_call_count, 0);
}

static void coreaudio_callback_routes_exact_n_into_preallocated_float32_output(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 48000,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_frame frames[] = {
        { .left = INT32_MIN, .right = INT32_MAX },
        { .left = -1073741824, .right = 1073741824 },
        { .left = 0, .right = -1 },
    };
    bound_route_recording_sink route = {
        .expected_requests = { 3 },
        .rendered_blocks = {
            {
                .frame_count = 3,
                .frames = frames,
            },
        },
    };
    route.expected_context = &route;

    static float adapter_workspace[6];
    float callback_output[6] = {
        -99.0f,
        -99.0f,
        -99.0f,
        -99.0f,
        -99.0f,
        -99.0f,
    };
    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .context = &route,
        },
        .delivery_mode = AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE,
        .workspace = {
            .samples = adapter_workspace,
            .frame_capacity = 3,
        },
    };

    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_reset_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);
    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_STARTED);

    assert_int_equal(
        fake_coreaudio_facade_request_with_output(&fake, 3, callback_output, 3),
        AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    const float expected_samples[] = {
        -1.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.0f,
        -0x1p-31f,
    };
    assert_int_equal(fake.request_trace_count, 1);
    assert_int_equal(fake.request_trace[0], 3);
    assert_int_equal(route.render_call_count, 1);
    assert_int_equal(observed_submission_count, 1);
    assert_int_equal(observed_sample_count, 6);
    assert_true(observed_sample_pointer == callback_output);
    assert_memory_equal(callback_output, expected_samples,
                        sizeof(expected_samples));
    assert_memory_equal(observed_samples, expected_samples,
                        sizeof(expected_samples));
    assert_int_equal(audio_output_coreaudio_adapter_test_allocation_attempt_count(),
                     0);
    audio_output_coreaudio_adapter_test_set_observer(NULL);
}

enum {
    STOP_REQUEST_FRAME_COUNT = 3,
};

typedef struct {
    audio_output_coreaudio_adapter_instance *instance;
    audio_frame_block rendered_block;
    audio_frame_block delivered_block;
    size_t render_call_count;
    size_t delivery_call_count;
    bool stop_request_succeeded;
} stop_request_route;

static audio_frame_block stop_request_renderer(
    void *context,
    size_t requested_frame_count)
{
    stop_request_route *route = context;
    assert_int_equal(requested_frame_count, STOP_REQUEST_FRAME_COUNT);
    assert_int_equal(route->render_call_count, 0);

    route->render_call_count++;
    route->stop_request_succeeded =
        audio_output_coreaudio_adapter_request_stop(route->instance);
    return route->rendered_block;
}

static audio_output_submit_result stop_request_sink(
    void *context,
    const audio_frame_block *block)
{
    stop_request_route *route = context;
    assert_non_null(block);
    assert_int_equal(route->delivery_call_count, 0);

    route->delivered_block = *block;
    route->delivery_call_count++;
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}

static void coreaudio_stop_request_completes_current_request_without_quiescing(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_frame frames[] = {
        { .left = 31, .right = -31 },
        { .left = 32, .right = -32 },
        { .left = 33, .right = -33 },
    };

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = stop_request_renderer,
            .delivery = stop_request_sink,
            .context = NULL,
        },
    };
    stop_request_route route = {
        .instance = &instance,
        .rendered_block = {
            .frame_count = STOP_REQUEST_FRAME_COUNT,
            .frames = frames,
        },
    };
    instance.route.context = &route;

    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_STARTED);
    assert_int_equal(instance.state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ACTIVE);

    assert_int_equal(fake_coreaudio_facade_request(&fake,
                                                   STOP_REQUEST_FRAME_COUNT),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_true(route.stop_request_succeeded);
    assert_int_equal(route.render_call_count, 1);
    assert_int_equal(route.delivery_call_count, 1);
    assert_int_equal(route.delivered_block.frame_count,
                     STOP_REQUEST_FRAME_COUNT);
    assert_true(route.delivered_block.frames == frames);

    assert_int_equal(instance.state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_STOP_REQUESTED);
    assert_true(fake.active);
    assert_false(fake.quiescent);
    assert_int_equal(fake.quiesce_call_count, 0);
    assert_int_equal(fake.lifecycle_trace_count, 4);

    assert_int_equal(fake_coreaudio_facade_request(&fake,
                                                   STOP_REQUEST_FRAME_COUNT),
                     AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(fake.request_trace_count, 1);
    assert_int_equal(fake.render_request_count, 1);
    assert_int_equal(route.render_call_count, 1);
    assert_int_equal(route.delivery_call_count, 1);
}

typedef struct {
    fake_coreaudio_facade *fake;
    audio_output_coreaudio_adapter_instance *instance;
    audio_frame_block rendered_block;
    audio_frame_block delivered_block;
    size_t render_call_count;
    size_t delivery_call_count;
    bool stop_request_succeeded;
} control_lifecycle_route;

static audio_frame_block control_lifecycle_renderer(
    void *context,
    size_t requested_frame_count)
{
    control_lifecycle_route *route = context;
    assert_true(route->fake->callback_in_flight);
    assert_int_equal(requested_frame_count, STOP_REQUEST_FRAME_COUNT);
    assert_int_equal(route->render_call_count, 0);

    route->render_call_count++;
    route->stop_request_succeeded =
        fake_coreaudio_facade_control_request_stop(route->fake,
                                                   route->instance);
    assert_int_equal(route->fake->control_stop_call_count, 0);
    assert_int_equal(route->fake->control_dispose_call_count, 0);
    return route->rendered_block;
}

static audio_output_submit_result control_lifecycle_delivery(
    void *context,
    const audio_frame_block *block)
{
    control_lifecycle_route *route = context;
    assert_true(route->fake->callback_in_flight);
    assert_non_null(block);
    assert_int_equal(route->delivery_call_count, 0);
    assert_int_equal(route->fake->control_stop_call_count, 0);
    assert_int_equal(route->fake->control_dispose_call_count, 0);

    route->delivered_block = *block;
    route->delivery_call_count++;
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}

static void coreaudio_control_lifecycle_waits_for_callback_quiescence(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_frame frames[] = {
        { .left = 31, .right = -31 },
        { .left = 32, .right = -32 },
        { .left = 33, .right = -33 },
    };

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = control_lifecycle_renderer,
            .delivery = control_lifecycle_delivery,
            .context = NULL,
        },
    };
    control_lifecycle_route route = {
        .fake = &fake,
        .instance = &instance,
        .rendered_block = {
            .frame_count = STOP_REQUEST_FRAME_COUNT,
            .frames = frames,
        },
    };
    instance.route.context = &route;

    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_STARTED);

    assert_int_equal(fake_coreaudio_facade_request(&fake,
                                                   STOP_REQUEST_FRAME_COUNT),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_true(route.stop_request_succeeded);
    assert_int_equal(fake.control_stop_request_call_count, 1);
    assert_true(fake.control_stop_requested_during_callback);
    assert_int_equal(route.render_call_count, 1);
    assert_int_equal(route.delivery_call_count, 1);
    assert_int_equal(route.delivered_block.frame_count,
                     STOP_REQUEST_FRAME_COUNT);
    assert_true(route.delivered_block.frames == frames);
    assert_false(fake.callback_in_flight);
    assert_int_equal(fake.control_stop_call_count, 0);
    assert_int_equal(fake.control_dispose_call_count, 0);

    assert_int_equal(audio_output_coreaudio_adapter_stop_instance(&instance),
                     AUDIO_OUTPUT_COREAUDIO_FACADE_OK);

    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_START,
        FAKE_COREAUDIO_FACADE_CALL_STOP,
        FAKE_COREAUDIO_FACADE_CALL_QUIESCE,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_int_equal(fake.lifecycle_trace_count,
                     sizeof(expected_lifecycle) / sizeof(expected_lifecycle[0]));
    for (size_t index = 0; index < sizeof(expected_lifecycle) /
                                      sizeof(expected_lifecycle[0]); index++) {
        assert_int_equal(fake.lifecycle_trace[index], expected_lifecycle[index]);
    }
    assert_int_equal(fake.callback_quiesce_call_count, 0);
    assert_int_equal(fake.control_stop_call_count, 1);
    assert_int_equal(fake.control_quiesce_call_count, 1);
    assert_int_equal(fake.control_dispose_call_count, 1);
    assert_false(fake.control_stop_called_during_callback);
    assert_false(fake.control_dispose_called_during_callback);
    assert_true(fake.quiescent);
    assert_true(fake.disposed);
    assert_int_equal(instance.state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_QUIESCENT);

    assert_int_equal(fake_coreaudio_facade_request(&fake,
                                                   STOP_REQUEST_FRAME_COUNT),
                     AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(fake.request_trace_count, 1);
    assert_int_equal(fake.render_request_count, 1);
    assert_int_equal(route.render_call_count, 1);
    assert_int_equal(route.delivery_call_count, 1);
}

static void coreaudio_teardown_returns_first_failure_and_enters_error_without_retry(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const struct teardown_failure_case {
        const char *name;
        audio_output_coreaudio_facade_result stop_result;
        audio_output_coreaudio_facade_result quiesce_result;
        audio_output_coreaudio_facade_result dispose_result;
        audio_output_coreaudio_facade_result expected_first_failure;
    } cases[] = {
        {
            .name = "stop failure",
            .stop_result = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
            .quiesce_result = AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE,
            .dispose_result = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
            .expected_first_failure = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
        },
        {
            .name = "quiesce failure",
            .stop_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .quiesce_result = AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE,
            .dispose_result = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
            .expected_first_failure = AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE,
        },
        {
            .name = "dispose failure",
            .stop_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .quiesce_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .dispose_result = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
            .expected_first_failure = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
        },
    };

    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_START,
        FAKE_COREAUDIO_FACADE_CALL_STOP,
        FAKE_COREAUDIO_FACADE_CALL_QUIESCE,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };

    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        fake_coreaudio_facade fake;
        fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
        fake.stop_result = cases[index].stop_result;
        fake.quiesce_result = cases[index].quiesce_result;
        fake.dispose_result = cases[index].dispose_result;

        bound_route_recording_sink route = {0};
        route.expected_context = &route;
        audio_output_coreaudio_adapter_instance instance = {
            .facade = &fake.facade,
            .route = {
                .renderer = bound_route_renderer,
                .delivery = bound_route_recording_sink_deliver,
                .context = &route,
            },
        };

        assert_int_equal(
            audio_output_coreaudio_adapter_start_instance(&instance, &format),
            AUDIO_OUTPUT_COREAUDIO_START_STARTED);

        const audio_output_coreaudio_facade_result teardown_result =
            audio_output_coreaudio_adapter_stop_instance(&instance);
        if (teardown_result != cases[index].expected_first_failure) {
            fail_msg("teardown case '%s': expected first failure %d, got %d",
                     cases[index].name, cases[index].expected_first_failure,
                     teardown_result);
        }

        if (fake.lifecycle_trace_count !=
            sizeof(expected_lifecycle) / sizeof(expected_lifecycle[0])) {
            fail_msg("teardown case '%s': expected all cleanup calls, got %zu",
                     cases[index].name, fake.lifecycle_trace_count);
        }
        for (size_t call_index = 0; call_index < sizeof(expected_lifecycle) /
                                                   sizeof(expected_lifecycle[0]);
             call_index++) {
            if (fake.lifecycle_trace[call_index] != expected_lifecycle[call_index]) {
                fail_msg("teardown case '%s': cleanup call %zu was %d, expected %d",
                         cases[index].name, call_index,
                         fake.lifecycle_trace[call_index],
                         expected_lifecycle[call_index]);
            }
        }

        assert_int_equal(fake.control_stop_call_count, 1);
        assert_int_equal(fake.control_quiesce_call_count, 1);
        assert_int_equal(fake.control_dispose_call_count, 1);
        assert_true(fake.quiescent);
        assert_true(fake.disposed);
        assert_false(fake.active);
        assert_false(instance.in_request);
        assert_int_equal(instance.state,
                         AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ERROR);

        const size_t lifecycle_trace_count = fake.lifecycle_trace_count;
        const size_t stop_call_count = fake.control_stop_call_count;
        const size_t quiesce_call_count = fake.control_quiesce_call_count;
        const size_t dispose_call_count = fake.control_dispose_call_count;
        const audio_output_coreaudio_facade_result retry_result =
            audio_output_coreaudio_adapter_stop_instance(&instance);
        assert_true(retry_result != AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
        assert_false(audio_output_coreaudio_adapter_request_stop(&instance));
        assert_int_equal(fake.lifecycle_trace_count, lifecycle_trace_count);
        assert_int_equal(fake.control_stop_call_count, stop_call_count);
        assert_int_equal(fake.control_quiesce_call_count, quiesce_call_count);
        assert_int_equal(fake.control_dispose_call_count, dispose_call_count);
    }
}

static void coreaudio_workspace_instance_converts_borrowed_capacity_and_rejects_oversize(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_frame one_frames[] = {
        { .left = INT32_MIN, .right = INT32_MAX },
    };
    const audio_frame three_frames[] = {
        { .left = INT32_MIN, .right = INT32_MAX },
        { .left = -1073741824, .right = 1073741824 },
        { .left = 0, .right = -1 },
    };
    const audio_frame four_frames[] = {
        { .left = 11, .right = -11 },
        { .left = 12, .right = -12 },
        { .left = 13, .right = -13 },
        { .left = 14, .right = -14 },
    };

    bound_route_recording_sink route = {0};
    route.expected_requests[0] = 0;
    route.expected_requests[1] = 1;
    route.expected_requests[2] = 3;
    route.expected_requests[3] = 4;
    route.rendered_blocks[0] = (audio_frame_block){
        .frame_count = 0,
        .frames = NULL,
    };
    route.rendered_blocks[1] = (audio_frame_block){
        .frame_count = 1,
        .frames = one_frames,
    };
    route.rendered_blocks[2] = (audio_frame_block){
        .frame_count = 3,
        .frames = three_frames,
    };
    route.rendered_blocks[3] = (audio_frame_block){
        .frame_count = 4,
        .frames = four_frames,
    };
    route.expected_context = &route;

    enum { WORKSPACE_FRAME_CAPACITY = 3 };
    static float workspace_samples[2 * WORKSPACE_FRAME_CAPACITY];
    for (size_t index = 0; index < 2 * WORKSPACE_FRAME_CAPACITY; index++) {
        workspace_samples[index] = -99.0f;
    }

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = NULL,
            .context = &route,
        },
        .delivery_mode = AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE,
        .workspace = {
            .samples = workspace_samples,
            .frame_capacity = WORKSPACE_FRAME_CAPACITY,
        },
    };

    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_reset_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);

    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_STARTED);
    assert_true(fake.request_context == &instance);

    assert_int_equal(fake_coreaudio_facade_request(&fake, 0),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(observed_submission_count, 0);
    assert_int_equal(observed_sample_count, 0);
    assert_int_equal(audio_output_coreaudio_adapter_test_allocation_attempt_count(),
                     0);

    assert_int_equal(fake_coreaudio_facade_request(&fake, 1),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    const float expected_one_samples[] = { -1.0f, 1.0f };
    assert_int_equal(observed_submission_count, 1);
    assert_int_equal(observed_sample_count, 2);
    assert_true(observed_sample_pointer == workspace_samples);
    assert_memory_equal(observed_samples, expected_one_samples,
                        sizeof(expected_one_samples));
    assert_memory_equal(workspace_samples, expected_one_samples,
                        sizeof(expected_one_samples));
    assert_int_equal(audio_output_coreaudio_adapter_test_allocation_attempt_count(),
                     0);

    assert_int_equal(fake_coreaudio_facade_request(&fake, 3),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    const float expected_three_samples[] = {
        -1.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.0f,
        -0x1p-31f,
    };
    assert_int_equal(observed_submission_count, 2);
    assert_int_equal(observed_sample_count, 6);
    assert_true(observed_sample_pointer == workspace_samples);
    assert_memory_equal(observed_samples, expected_three_samples,
                        sizeof(expected_three_samples));
    assert_memory_equal(workspace_samples, expected_three_samples,
                        sizeof(expected_three_samples));
    assert_int_equal(audio_output_coreaudio_adapter_test_allocation_attempt_count(),
                     0);

    const float workspace_before_oversize[] = {
        -1.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.0f,
        -0x1p-31f,
    };
    assert_int_equal(fake_coreaudio_facade_request(&fake, 4),
                     AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(observed_submission_count, 2);
    assert_int_equal(observed_sample_count, 6);
    assert_memory_equal(workspace_samples, workspace_before_oversize,
                        sizeof(workspace_before_oversize));
    assert_int_equal(audio_output_coreaudio_adapter_test_allocation_attempt_count(),
                     0);
    audio_output_coreaudio_adapter_test_set_observer(NULL);
}

static void coreaudio_workspace_start_rejects_invalid_preparation_before_activation(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    static float dummy_storage;
    const struct invalid_workspace_case {
        const char *name;
        float *samples;
        size_t frame_capacity;
    } cases[] = {
        {
            .name = "non-NULL storage with zero capacity",
            .samples = &dummy_storage,
            .frame_capacity = 0,
        },
        {
            .name = "NULL storage with capacity one",
            .samples = NULL,
            .frame_capacity = 1,
        },
        {
            .name = "non-NULL storage with sample-count overflow capacity",
            .samples = &dummy_storage,
            .frame_capacity = SIZE_MAX / 2 + 1,
        },
    };

    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        fake_coreaudio_facade fake;
        fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK);

        bound_route_recording_sink route = {0};
        route.expected_context = &route;
        audio_output_coreaudio_adapter_instance instance = {
            .facade = &fake.facade,
            .route = {
                .renderer = bound_route_renderer,
                .delivery = NULL,
                .context = &route,
            },
            .delivery_mode = AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE,
            .workspace = {
                .samples = cases[index].samples,
                .frame_capacity = cases[index].frame_capacity,
            },
        };

        reset_coreaudio_observation();
        audio_output_coreaudio_adapter_test_reset_allocation_attempt_count();
        audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);

        const audio_output_coreaudio_start_result result =
            audio_output_coreaudio_adapter_start_instance(&instance, &format);
        if (result != AUDIO_OUTPUT_COREAUDIO_START_REJECTED) {
            fail_msg("workspace case '%s': expected REJECTED, got %d",
                     cases[index].name, result);
        }

        assert_int_equal(instance.state,
                         AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE);
        assert_false(instance.in_request);

        assert_int_equal(fake.lifecycle_trace_count, 0);
        assert_int_equal(fake.open_call_count, 0);
        assert_int_equal(fake.configure_call_count, 0);
        assert_int_equal(fake.bind_request_call_count, 0);
        assert_int_equal(fake.start_call_count, 0);
        assert_int_equal(fake.quiesce_call_count, 0);
        assert_false(fake.active);
        assert_false(fake.quiescent);
        assert_null(fake.request);
        assert_null(fake.request_context);
        assert_int_equal(fake.request_trace_count, 0);
        assert_int_equal(fake.render_request_count, 0);

        assert_int_equal(route.render_call_count, 0);
        assert_int_equal(route.delivery_call_count, 0);
        assert_int_equal(observed_submission_count, 0);
        assert_int_equal(observed_sample_count, 0);
        assert_null(observed_sample_pointer);
        assert_int_equal(audio_output_coreaudio_adapter_test_allocation_attempt_count(),
                         0);

        audio_output_coreaudio_adapter_test_set_observer(NULL);
    }
}

static void coreaudio_reentrant_start_is_rejected_without_second_lifecycle(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    bound_route_recording_sink route = {0};
    route.expected_context = &route;

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = bound_route_recording_sink_deliver,
            .context = &route,
        },
    };
    fake_coreaudio_facade_configure_reentrant_start(
        &fake, &instance, &format);

    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_STARTED);
    assert_int_equal(fake.reentrant_start_result,
                     AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_int_equal(fake.lifecycle_trace_count, 4);
    assert_int_equal(fake.open_call_count, 1);
    assert_int_equal(fake.configure_call_count, 1);
    assert_int_equal(fake.bind_request_call_count, 1);
    assert_int_equal(fake.start_call_count, 1);
    assert_true(fake.active);
    assert_int_equal(instance.state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ACTIVE);
}

typedef struct {
    fake_coreaudio_facade *fake;
    audio_output_coreaudio_adapter_instance *instance;
    audio_frame_block rendered_block;
    audio_frame_block delivered_block;
    size_t render_call_count;
    size_t delivery_call_count;
} delayed_stop_route;

static audio_frame_block delayed_stop_renderer(
    void *context,
    size_t requested_frame_count)
{
    delayed_stop_route *route = context;
    assert_true(route->fake->callback_in_flight);
    assert_int_equal(requested_frame_count, STOP_REQUEST_FRAME_COUNT);
    assert_int_equal(route->render_call_count, 0);
    route->render_call_count++;
    fake_coreaudio_facade_defer_control_stop(route->fake, route->instance);
    return route->rendered_block;
}

static audio_output_submit_result delayed_stop_sink(
    void *context,
    const audio_frame_block *block)
{
    delayed_stop_route *route = context;
    assert_non_null(block);
    assert_int_equal(route->delivery_call_count, 0);
    route->delivered_block = *block;
    route->delivery_call_count++;
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}

static void coreaudio_stop_admission_is_delayed_until_callback_exit(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_frame frames[] = {
        { .left = 41, .right = -41 },
        { .left = 42, .right = -42 },
        { .left = 43, .right = -43 },
    };

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = delayed_stop_renderer,
            .delivery = delayed_stop_sink,
            .context = NULL,
        },
    };
    delayed_stop_route route = {
        .fake = &fake,
        .instance = &instance,
        .rendered_block = {
            .frame_count = STOP_REQUEST_FRAME_COUNT,
            .frames = frames,
        },
    };
    instance.route.context = &route;

    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_STARTED);
    assert_int_equal(fake_coreaudio_facade_request(&fake,
                                                   STOP_REQUEST_FRAME_COUNT),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(fake.control_stop_request_call_count, 1);
    assert_true(fake.control_stop_requested_during_callback);
    assert_false(fake.callback_in_flight);
    assert_true(fake.deferred_stop_admission_result);
    assert_int_equal(instance.state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_STOP_REQUESTED);
    assert_int_equal(audio_output_coreaudio_adapter_stop_instance(&instance),
                     AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    assert_int_equal(instance.state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_QUIESCENT);
}

static void coreaudio_hal_route_rejects_direct_delivery_mode(void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 48000,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    bound_route_recording_sink route = {0};
    route.expected_context = &route;

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    fake_coreaudio_facade_set_workspace_only(&fake, true);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = bound_route_recording_sink_deliver,
            .context = &route,
        },
        .delivery_mode = AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_DIRECT,
    };

    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_int_equal(instance.state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE);
    assert_int_equal(fake.lifecycle_trace_count, 0);
    assert_false(fake.active);
    assert_null(fake.request);
    assert_null(fake.request_context);
    assert_int_equal(route.render_call_count, 0);
    assert_int_equal(route.delivery_call_count, 0);
}

static void assert_start_rollback_disposes(
    fake_coreaudio_facade *fake,
    audio_output_coreaudio_adapter_instance *instance,
    const fake_coreaudio_facade_lifecycle_call *expected_lifecycle,
    size_t expected_lifecycle_count)
{
    assert_int_equal(fake->lifecycle_trace_count, expected_lifecycle_count);
    for (size_t index = 0; index < expected_lifecycle_count; index++) {
        assert_int_equal(fake->lifecycle_trace[index], expected_lifecycle[index]);
    }
    assert_int_equal(fake->control_dispose_call_count, 1);
    assert_true(fake->disposed);
    assert_false(fake->active);
    assert_int_equal(instance->state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE);
}

static void assert_stateless_start_rollback_disposes(
    fake_coreaudio_facade *fake,
    const fake_coreaudio_facade_lifecycle_call *expected_lifecycle,
    size_t expected_lifecycle_count)
{
    assert_int_equal(fake->lifecycle_trace_count, expected_lifecycle_count);
    for (size_t index = 0; index < expected_lifecycle_count; index++) {
        assert_int_equal(fake->lifecycle_trace[index], expected_lifecycle[index]);
    }
    assert_int_equal(fake->control_dispose_call_count, 1);
    assert_true(fake->disposed);
    assert_false(fake->active);
}

static void coreaudio_open_failure_rolls_back_with_dispose(void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    bound_route_recording_sink route = {0};
    route.expected_context = &route;

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = bound_route_recording_sink_deliver,
            .context = &route,
        },
    };

    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_start_rollback_disposes(&fake, &instance, expected_lifecycle,
                                   sizeof(expected_lifecycle) /
                                       sizeof(expected_lifecycle[0]));
    assert_int_equal(fake.configure_call_count, 0);
    assert_int_equal(fake.bind_request_call_count, 0);
    assert_int_equal(fake.start_call_count, 0);
}

static void coreaudio_stateless_configure_failure_rolls_back_with_dispose(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);

    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_int_equal(
        audio_output_coreaudio_adapter_start(&fake.facade, &format),
        AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_stateless_start_rollback_disposes(
        &fake, expected_lifecycle,
        sizeof(expected_lifecycle) / sizeof(expected_lifecycle[0]));
    assert_int_equal(fake.start_call_count, 0);
}

static void coreaudio_stateless_start_failure_rolls_back_with_dispose(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED);

    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_START,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_int_equal(
        audio_output_coreaudio_adapter_start(&fake.facade, &format),
        AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_stateless_start_rollback_disposes(
        &fake, expected_lifecycle,
        sizeof(expected_lifecycle) / sizeof(expected_lifecycle[0]));
}

static void coreaudio_configure_failure_rolls_back_with_dispose(void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    bound_route_recording_sink route = {0};
    route.expected_context = &route;

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = bound_route_recording_sink_deliver,
            .context = &route,
        },
    };

    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_start_rollback_disposes(&fake, &instance, expected_lifecycle,
                                   sizeof(expected_lifecycle) /
                                       sizeof(expected_lifecycle[0]));
}

static void coreaudio_bind_failure_rolls_back_with_dispose(void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    bound_route_recording_sink route = {0};
    route.expected_context = &route;

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    fake_coreaudio_facade_set_bind_result(
        &fake, AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = bound_route_recording_sink_deliver,
            .context = &route,
        },
    };

    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_start_rollback_disposes(&fake, &instance, expected_lifecycle,
                                   sizeof(expected_lifecycle) /
                                       sizeof(expected_lifecycle[0]));
}

static void coreaudio_start_failure_rolls_back_with_dispose(void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    bound_route_recording_sink route = {0};
    route.expected_context = &route;

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = bound_route_recording_sink_deliver,
            .context = &route,
        },
    };

    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_START,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_start_rollback_disposes(&fake, &instance, expected_lifecycle,
                                   sizeof(expected_lifecycle) /
                                       sizeof(expected_lifecycle[0]));
}

static void coreaudio_hal_route_converts_into_borrowed_workspace_then_copies_to_native_buffer(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 48000,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    enum { HAL_WORKSPACE_FRAME_CAPACITY = 3 };
    static float hal_workspace_samples[2 * HAL_WORKSPACE_FRAME_CAPACITY];
    float native_buffer[2 * HAL_WORKSPACE_FRAME_CAPACITY];
    const float untouched_fill[2 * HAL_WORKSPACE_FRAME_CAPACITY] = {
        -99.0f, -99.0f, -99.0f, -99.0f, -99.0f, -99.0f,
    };
    memcpy(hal_workspace_samples, untouched_fill, sizeof(untouched_fill));
    memcpy(native_buffer, untouched_fill, sizeof(untouched_fill));

    const audio_frame three_frames[] = {
        { .left = INT32_MIN, .right = INT32_MAX },
        { .left = -1073741824, .right = 1073741824 },
        { .left = 0, .right = -1 },
    };

    bound_route_recording_sink route = {0};
    route.expected_requests[0] = 3;
    route.rendered_blocks[0] = (audio_frame_block){
        .frame_count = 3,
        .frames = three_frames,
    };
    route.expected_context = &route;

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    fake_coreaudio_facade_set_workspace_only(&fake, true);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = NULL,
            .context = &route,
        },
        .delivery_mode = AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE,
        .workspace = {
            .samples = hal_workspace_samples,
            .frame_capacity = HAL_WORKSPACE_FRAME_CAPACITY,
        },
    };

    reset_coreaudio_observation();
    audio_output_coreaudio_adapter_test_reset_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_observer(record_coreaudio_output);

    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_STARTED);

    assert_int_equal(fake_coreaudio_facade_request(&fake, 3),
                     AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(fake_coreaudio_facade_request_with_output(
                         &fake, 3, native_buffer,
                         HAL_WORKSPACE_FRAME_CAPACITY - 1),
                     AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(fake_coreaudio_facade_request_with_output(
                         &fake, 3, native_buffer, SIZE_MAX / 2 + 1),
                     AUDIO_OUTPUT_SUBMIT_REJECTED);
    assert_int_equal(route.render_call_count, 0);
    assert_int_equal(route.delivery_call_count, 0);
    assert_int_equal(observed_submission_count, 0);
    assert_int_equal(observed_sample_count, 0);
    assert_memory_equal(hal_workspace_samples, untouched_fill,
                        sizeof(untouched_fill));
    assert_memory_equal(native_buffer, untouched_fill, sizeof(untouched_fill));

    assert_int_equal(fake_coreaudio_facade_request(&fake, 0),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(route.render_call_count, 0);
    assert_int_equal(observed_submission_count, 0);

    assert_int_equal(fake_coreaudio_facade_request_with_output(
                         &fake, 3, native_buffer,
                         HAL_WORKSPACE_FRAME_CAPACITY),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    const float expected_samples[] = {
        -1.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.0f,
        -0x1p-31f,
    };
    assert_int_equal(route.render_call_count, 1);
    assert_int_equal(route.delivery_call_count, 0);
    assert_int_equal(observed_submission_count, 1);
    assert_int_equal(observed_sample_count, 6);
    assert_true(observed_sample_pointer == hal_workspace_samples);
    assert_memory_equal(observed_samples, expected_samples,
                        sizeof(expected_samples));
    assert_memory_equal(hal_workspace_samples, expected_samples,
                        sizeof(expected_samples));
    assert_memory_equal(native_buffer, expected_samples,
                        sizeof(expected_samples));
    assert_int_equal(
        audio_output_coreaudio_adapter_test_allocation_attempt_count(), 0);

    const size_t expected_request_trace[] = { 3, 3, 3, 0, 3 };
    assert_int_equal(fake.request_trace_count,
                     sizeof(expected_request_trace) /
                         sizeof(expected_request_trace[0]));
    for (size_t index = 0;
         index < sizeof(expected_request_trace) / sizeof(expected_request_trace[0]);
         index++) {
        assert_int_equal(fake.request_trace[index],
                         expected_request_trace[index]);
    }
    assert_int_equal(fake.render_request_count,
                     sizeof(expected_request_trace) /
                         sizeof(expected_request_trace[0]));
    audio_output_coreaudio_adapter_test_set_observer(NULL);
}

static void coreaudio_hal_copies_native_output_before_releasing_admission(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 48000,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_frame frames[] = {
        { .left = INT32_MIN, .right = INT32_MAX },
        { .left = -1073741824, .right = 1073741824 },
        { .left = 0, .right = -1 },
    };
    static float workspace_samples[6];
    const float untouched_fill[] = {
        -99.0f, -99.0f, -99.0f, -99.0f, -99.0f, -99.0f,
    };

    bound_route_recording_sink route = {0};
    route.expected_requests[0] = 3;
    route.rendered_blocks[0] = (audio_frame_block){
        .frame_count = 3,
        .frames = frames,
    };
    route.expected_context = &route;

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    fake_coreaudio_facade_set_workspace_only(&fake, true);
    audio_output_coreaudio_adapter_instance instance = {
        .facade = &fake.facade,
        .route = {
            .renderer = bound_route_renderer,
            .delivery = NULL,
            .context = &route,
        },
        .delivery_mode = AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE,
        .workspace = {
            .samples = workspace_samples,
            .frame_capacity = 3,
        },
    };

    const long page_size_result = sysconf(_SC_PAGESIZE);
    assert_true(page_size_result > 0);
    const size_t page_size = (size_t)page_size_result;
    void *page = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANON, -1, 0);
    assert_true(page != MAP_FAILED);
    float *native_buffer = page;
    memcpy(native_buffer, untouched_fill, sizeof(untouched_fill));

    struct sigaction probe_action = {0};
    struct sigaction previous_sigsegv = {0};
    struct sigaction previous_sigbus = {0};
    probe_action.sa_sigaction = hal_copy_probe_signal_handler;
    probe_action.sa_flags = SA_SIGINFO;
    assert_int_equal(sigemptyset(&probe_action.sa_mask), 0);
    assert_int_equal(sigaction(SIGSEGV, &probe_action, &previous_sigsegv), 0);
    assert_int_equal(sigaction(SIGBUS, &probe_action, &previous_sigbus), 0);

    hal_copy_probe_fault_count = 0;
    hal_copy_probe_admission_at_fault = -1;
    hal_copy_probe_page = page;
    hal_copy_probe_page_size = page_size;
    hal_copy_probe_instance = &instance;
    assert_int_equal(mprotect(page, page_size, PROT_NONE), 0);

    const audio_output_coreaudio_start_result start_result =
        audio_output_coreaudio_adapter_start_instance(&instance, &format);
    const audio_output_submit_result request_result =
        fake_coreaudio_facade_request_with_output(&fake, 3, native_buffer, 3);
    const size_t fault_count = (size_t)hal_copy_probe_fault_count;
    const int admission_at_fault = hal_copy_probe_admission_at_fault;
    const float expected_samples[] = {
        -1.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.0f,
        -0x1p-31f,
    };
    const bool output_matches =
        memcmp(native_buffer, expected_samples, sizeof(expected_samples)) == 0;

    const int restore_sigsegv = sigaction(
        SIGSEGV, &previous_sigsegv, NULL);
    const int restore_sigbus = sigaction(SIGBUS, &previous_sigbus, NULL);
    hal_copy_probe_instance = NULL;
    hal_copy_probe_page = NULL;
    const int unmap_result = munmap(page, page_size);

    assert_int_equal(start_result, AUDIO_OUTPUT_COREAUDIO_START_STARTED);
    assert_int_equal(request_result, AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(fault_count, 1);
    assert_int_equal(admission_at_fault,
                     CORE_AUDIO_ADMISSION_IN_FLIGHT_FOR_TEST);
    assert_true(output_matches);
    assert_int_equal(restore_sigsegv, 0);
    assert_int_equal(restore_sigbus, 0);
    assert_int_equal(unmap_result, 0);
}

typedef struct {
    fake_coreaudio_facade *fake;
    bool preparation_succeeds;
    bool prepared;
    size_t prepare_call_count;
    audio_output_coreaudio_format prepared_format;
    audio_frame rendered_frame;
    size_t render_call_count;
    size_t delivery_call_count;
} negotiated_preparation_route;

static bool negotiated_preparation_prepare(
    void *context,
    const audio_output_coreaudio_format *format)
{
    negotiated_preparation_route *route = context;
    assert_non_null(route);
    assert_non_null(format);
    assert_int_equal(route->fake->lifecycle_trace_count, 2);
    assert_int_equal(route->fake->lifecycle_trace[0],
                     FAKE_COREAUDIO_FACADE_CALL_OPEN);
    assert_int_equal(route->fake->lifecycle_trace[1],
                     FAKE_COREAUDIO_FACADE_CALL_CONFIGURE);

    route->prepare_call_count++;
    route->prepared_format = *format;
    fake_coreaudio_facade_record_preparation(route->fake);
    route->prepared = route->preparation_succeeds;
    return route->preparation_succeeds;
}

static audio_frame_block negotiated_preparation_renderer(
    void *context,
    size_t requested_frame_count)
{
    negotiated_preparation_route *route = context;
    assert_true(route->prepared);
    assert_int_equal(requested_frame_count, 1);
    route->render_call_count++;
    return (audio_frame_block){
        .frame_count = 1,
        .frames = &route->rendered_frame,
    };
}

static audio_output_submit_result negotiated_preparation_delivery(
    void *context,
    const audio_frame_block *block)
{
    negotiated_preparation_route *route = context;
    assert_true(route->prepared);
    assert_non_null(block);
    assert_int_equal(block->frame_count, 1);
    route->delivery_call_count++;
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}

static audio_output_coreaudio_adapter_instance
negotiated_preparation_instance(
    fake_coreaudio_facade *fake,
    negotiated_preparation_route *route)
{
    return (audio_output_coreaudio_adapter_instance){
        .facade = &fake->facade,
        .route = {
            .renderer = negotiated_preparation_renderer,
            .delivery = negotiated_preparation_delivery,
            .prepare = negotiated_preparation_prepare,
            .context = route,
        },
    };
}

static void assert_lifecycle_trace(
    const fake_coreaudio_facade *fake,
    const fake_coreaudio_facade_lifecycle_call *expected,
    size_t expected_count)
{
    assert_int_equal(fake->lifecycle_trace_count, expected_count);
    for (size_t index = 0; index < expected_count; index++) {
        assert_int_equal(fake->lifecycle_trace[index], expected[index]);
    }
}

static void coreaudio_negotiated_format_preparation_precedes_activation(
    void **state)
{
    (void)state;

    const uint32_t sample_rates[] = { 44100, 48000 };
    for (size_t index = 0; index < sizeof(sample_rates) / sizeof(sample_rates[0]);
         index++) {
        const audio_output_coreaudio_format format = {
            .sample_rate_hz = sample_rates[index],
            .channel_count = 2,
            .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
            .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
        };
        fake_coreaudio_facade fake;
        fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
        fake_coreaudio_facade_set_reported_format(&fake, &format);

        negotiated_preparation_route route = {
            .fake = &fake,
            .preparation_succeeds = true,
            .rendered_frame = { .left = 17, .right = -17 },
        };
        audio_output_coreaudio_adapter_instance instance =
            negotiated_preparation_instance(&fake, &route);
        fake_coreaudio_facade_request_during_start(&fake, 1, 1);

        assert_int_equal(
            audio_output_coreaudio_adapter_start_instance(&instance, &format),
            AUDIO_OUTPUT_COREAUDIO_START_STARTED);
        const fake_coreaudio_facade_lifecycle_call expected[] = {
            FAKE_COREAUDIO_FACADE_CALL_OPEN,
            FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
            FAKE_COREAUDIO_FACADE_CALL_PREPARE,
            FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
            FAKE_COREAUDIO_FACADE_CALL_START,
        };
        assert_lifecycle_trace(&fake, expected,
                               sizeof(expected) / sizeof(expected[0]));
        assert_int_equal(route.prepare_call_count, 1);
        assert_int_equal(fake.prepare_call_count, 1);
        assert_int_equal(route.prepared_format.sample_rate_hz,
                         sample_rates[index]);
        assert_int_equal(route.render_call_count, 1);
        assert_int_equal(route.delivery_call_count, 1);
        assert_int_equal(fake.render_request_count, 1);
        assert_true(fake.active);
        assert_int_equal(instance.state,
                         AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ACTIVE);
    }
}

static void coreaudio_negotiated_format_failures_roll_back_before_activation(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format valid_format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_output_coreaudio_format unspecified_requested_format = {
        .sample_rate_hz = 0,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_output_coreaudio_format invalid_requested_format = {
        .sample_rate_hz = 32000,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_output_coreaudio_format invalid_reported_format = {
        .sample_rate_hz = 48000,
        .channel_count = 1,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_output_coreaudio_format invalid_reported_rate_format = {
        .sample_rate_hz = 32000,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };

    fake_coreaudio_facade preflight_fake;
    fake_coreaudio_facade_init(&preflight_fake,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    negotiated_preparation_route preflight_route = {
        .fake = &preflight_fake,
        .preparation_succeeds = true,
    };
    audio_output_coreaudio_adapter_instance preflight_instance =
        negotiated_preparation_instance(&preflight_fake, &preflight_route);
    assert_int_equal(audio_output_coreaudio_adapter_start_instance(
                         &preflight_instance, &invalid_requested_format),
                     AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
    assert_int_equal(preflight_instance.state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE);
    assert_lifecycle_trace(&preflight_fake, NULL, 0);
    assert_false(preflight_fake.disposed);
    assert_int_equal(preflight_route.prepare_call_count, 0);
    assert_int_equal(preflight_route.render_call_count, 0);
    assert_int_equal(preflight_route.delivery_call_count, 0);

    const struct failure_case {
        const char *name;
        const audio_output_coreaudio_format *requested_format;
        audio_output_coreaudio_facade_result configure_result;
        const audio_output_coreaudio_format *reported_format;
        bool preparation_succeeds;
        const fake_coreaudio_facade_lifecycle_call *expected_trace;
        size_t expected_trace_count;
        size_t expected_prepare_call_count;
    } cases[] = {
        {
            .name = "configure failure",
            .requested_format = &valid_format,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED,
            .reported_format = &valid_format,
            .preparation_succeeds = true,
            .expected_trace = (const fake_coreaudio_facade_lifecycle_call[]){
                FAKE_COREAUDIO_FACADE_CALL_OPEN,
                FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
                FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
            },
            .expected_trace_count = 3,
            .expected_prepare_call_count = 0,
        },
        {
            .name = "unsupported reported format",
            .requested_format = &valid_format,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .reported_format = &invalid_reported_format,
            .preparation_succeeds = true,
            .expected_trace = (const fake_coreaudio_facade_lifecycle_call[]){
                FAKE_COREAUDIO_FACADE_CALL_OPEN,
                FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
                FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
            },
            .expected_trace_count = 3,
            .expected_prepare_call_count = 0,
        },
        {
            .name = "preparation failure",
            .requested_format = &valid_format,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .reported_format = &valid_format,
            .preparation_succeeds = false,
            .expected_trace = (const fake_coreaudio_facade_lifecycle_call[]){
                FAKE_COREAUDIO_FACADE_CALL_OPEN,
                FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
                FAKE_COREAUDIO_FACADE_CALL_PREPARE,
                FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
            },
            .expected_trace_count = 4,
            .expected_prepare_call_count = 1,
        },
        {
            .name = "unsupported reported rate for unspecified request",
            .requested_format = &unspecified_requested_format,
            .configure_result = AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
            .reported_format = &invalid_reported_rate_format,
            .preparation_succeeds = true,
            .expected_trace = (const fake_coreaudio_facade_lifecycle_call[]){
                FAKE_COREAUDIO_FACADE_CALL_OPEN,
                FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
                FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
            },
            .expected_trace_count = 3,
            .expected_prepare_call_count = 0,
        },
    };

    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        fake_coreaudio_facade fake;
        fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   cases[index].configure_result,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
        fake_coreaudio_facade_set_reported_format(&fake,
                                                  cases[index].reported_format);
        negotiated_preparation_route route = {
            .fake = &fake,
            .preparation_succeeds = cases[index].preparation_succeeds,
        };
        audio_output_coreaudio_adapter_instance instance =
            negotiated_preparation_instance(&fake, &route);
        fake_coreaudio_facade_request_during_start(&fake, 1, 1);

        const audio_output_coreaudio_start_result result =
            audio_output_coreaudio_adapter_start_instance(&instance,
                                                          cases[index].requested_format);
        if (result != AUDIO_OUTPUT_COREAUDIO_START_REJECTED) {
            fail_msg("failure case '%s': expected REJECTED, got %d",
                     cases[index].name, result);
        }
        assert_lifecycle_trace(&fake, cases[index].expected_trace,
                               cases[index].expected_trace_count);
        assert_int_equal(route.prepare_call_count,
                         cases[index].expected_prepare_call_count);
        assert_int_equal(fake.bind_request_call_count, 0);
        assert_int_equal(fake.start_call_count, 0);
        assert_int_equal(fake.render_request_count, 0);
        assert_int_equal(route.render_call_count, 0);
        assert_int_equal(route.delivery_call_count, 0);
        assert_false(fake.active);
        assert_true(fake.disposed);
        assert_int_equal(instance.state,
                         AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE);
        assert_false(instance.in_request);
    }
}

static void coreaudio_unspecified_startup_rate_uses_reported_rate_before_activation(
    void **state)
{
    (void)state;

    const audio_output_coreaudio_format requested_format = {
        .sample_rate_hz = 0,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_output_coreaudio_format reported_format = {
        .sample_rate_hz = 48000,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };

    fake_coreaudio_facade fake;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    fake_coreaudio_facade_set_reported_format(&fake, &reported_format);

    negotiated_preparation_route route = {
        .fake = &fake,
        .preparation_succeeds = true,
        .rendered_frame = { .left = 23, .right = -23 },
    };
    audio_output_coreaudio_adapter_instance instance =
        negotiated_preparation_instance(&fake, &route);
    fake_coreaudio_facade_request_during_start(&fake, 1, 1);

    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(
            &instance, &requested_format),
        AUDIO_OUTPUT_COREAUDIO_START_STARTED);
    assert_true(fake.requested_format_set);
    assert_memory_equal(&fake.requested_format, &requested_format,
                        sizeof(requested_format));
    assert_true(fake.configured_format_set);
    assert_memory_equal(&fake.configured_format, &reported_format,
                        sizeof(reported_format));
    assert_int_equal(route.prepare_call_count, 1);
    assert_int_equal(route.prepared_format.sample_rate_hz, 48000);
    assert_int_equal(route.render_call_count, 1);
    assert_int_equal(route.delivery_call_count, 1);
    assert_int_equal(fake.render_request_count, 1);
    assert_true(fake.active);
    assert_int_equal(instance.state,
                     AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_ACTIVE);

    const fake_coreaudio_facade_lifecycle_call expected[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_PREPARE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_START,
    };
    assert_lifecycle_trace(&fake, expected,
                           sizeof(expected) / sizeof(expected[0]));
}

static void coreaudio_explicit_startup_rate_must_match_reported_rate(
    void **state)
{
    (void)state;

    const struct mismatch_case {
        uint32_t requested_rate_hz;
        uint32_t reported_rate_hz;
    } cases[] = {
        { .requested_rate_hz = 44100, .reported_rate_hz = 48000 },
        { .requested_rate_hz = 48000, .reported_rate_hz = 44100 },
    };

    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        const audio_output_coreaudio_format requested_format = {
            .sample_rate_hz = cases[index].requested_rate_hz,
            .channel_count = 2,
            .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
            .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
        };
        const audio_output_coreaudio_format reported_format = {
            .sample_rate_hz = cases[index].reported_rate_hz,
            .channel_count = 2,
            .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
            .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
        };

        fake_coreaudio_facade fake;
        fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
        fake_coreaudio_facade_set_reported_format(&fake, &reported_format);
        negotiated_preparation_route route = {
            .fake = &fake,
            .preparation_succeeds = true,
        };
        audio_output_coreaudio_adapter_instance instance =
            negotiated_preparation_instance(&fake, &route);
        fake_coreaudio_facade_request_during_start(&fake, 1, 1);

        assert_int_equal(
            audio_output_coreaudio_adapter_start_instance(
                &instance, &requested_format),
            AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
        assert_true(fake.requested_format_set);
        assert_memory_equal(&fake.requested_format, &requested_format,
                            sizeof(requested_format));
        assert_true(fake.configured_format_set);
        assert_memory_equal(&fake.configured_format, &reported_format,
                            sizeof(reported_format));
        const fake_coreaudio_facade_lifecycle_call expected[] = {
            FAKE_COREAUDIO_FACADE_CALL_OPEN,
            FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
            FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
        };
        assert_lifecycle_trace(&fake, expected,
                               sizeof(expected) / sizeof(expected[0]));
        assert_int_equal(fake.prepare_call_count, 0);
        assert_int_equal(fake.bind_request_call_count, 0);
        assert_int_equal(fake.start_call_count, 0);
        assert_int_equal(fake.render_request_count, 0);
        assert_false(fake.active);
        assert_true(fake.disposed);
        assert_int_equal(instance.state,
                         AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE);
    }
}

static void coreaudio_startup_rate_request_validation_stays_preflight_only(
    void **state)
{
    (void)state;

    const struct invalid_request_case {
        uint32_t sample_rate_hz;
        uint32_t channel_count;
        audio_output_coreaudio_sample_format sample_format;
        audio_output_coreaudio_layout layout;
    } cases[] = {
        {
            .sample_rate_hz = 32000,
            .channel_count = 2,
            .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
            .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
        },
        {
            .sample_rate_hz = 0,
            .channel_count = 1,
            .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
            .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
        },
        {
            .sample_rate_hz = 0,
            .channel_count = 2,
            .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_INT16,
            .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
        },
        {
            .sample_rate_hz = 0,
            .channel_count = 2,
            .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
            .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_PLANAR,
        },
    };

    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        const audio_output_coreaudio_format format = {
            .sample_rate_hz = cases[index].sample_rate_hz,
            .channel_count = cases[index].channel_count,
            .sample_format = cases[index].sample_format,
            .layout = cases[index].layout,
        };
        fake_coreaudio_facade fake;
        fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                                   AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
        negotiated_preparation_route route = {
            .fake = &fake,
            .preparation_succeeds = true,
        };
        audio_output_coreaudio_adapter_instance instance =
            negotiated_preparation_instance(&fake, &route);

        assert_int_equal(
            audio_output_coreaudio_adapter_start_instance(&instance, &format),
            AUDIO_OUTPUT_COREAUDIO_START_REJECTED);
        assert_int_equal(instance.state,
                         AUDIO_OUTPUT_COREAUDIO_ADAPTER_INSTANCE_INACTIVE);
        assert_lifecycle_trace(&fake, NULL, 0);
        assert_false(fake.disposed);
        assert_int_equal(route.prepare_call_count, 0);
        assert_int_equal(route.render_call_count, 0);
        assert_int_equal(route.delivery_call_count, 0);
    }
}
#endif

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(zero_frame_blocks_are_accepted_with_or_without_frames),
        cmocka_unit_test(signed_32_interleaved_frames_are_accepted_and_counted),
        cmocka_unit_test(nonzero_frame_blocks_without_frames_are_rejected_and_not_counted),
        cmocka_unit_test(accepted_frames_are_not_retained_after_submit),
        cmocka_unit_test(private_dispatch_preserves_interleaved_float32_order_without_device),
        cmocka_unit_test(private_dispatch_accepts_zero_frame_blocks_without_device),
        cmocka_unit_test(private_adapter_converts_signed_32_frames_to_interleaved_float32),
        cmocka_unit_test(private_adapter_rejects_invalid_blocks_before_converted_output),
        cmocka_unit_test(private_adapter_rejects_stereo_sample_count_overflow_before_output),
        cmocka_unit_test(private_adapter_rejects_float32_byte_size_overflow_before_output),
        cmocka_unit_test(private_adapter_rejects_deterministic_allocation_failure_before_output),
        cmocka_unit_test(coreaudio_lifecycle_accepts_only_strict_stereo_float32),
        cmocka_unit_test(
            coordinator_accepts_zero_and_variable_exact_requests_and_rejects),
        cmocka_unit_test(
            coreaudio_bound_instance_hands_off_exact_requests_without_conversion),
        cmocka_unit_test(
            coreaudio_callback_routes_exact_n_into_preallocated_float32_output),
        cmocka_unit_test(
            coreaudio_stop_request_completes_current_request_without_quiescing),
        cmocka_unit_test(
            coreaudio_control_lifecycle_waits_for_callback_quiescence),
        cmocka_unit_test(
            coreaudio_teardown_returns_first_failure_and_enters_error_without_retry),
        cmocka_unit_test(
            coreaudio_workspace_instance_converts_borrowed_capacity_and_rejects_oversize),
        cmocka_unit_test(
            coreaudio_workspace_start_rejects_invalid_preparation_before_activation),
        cmocka_unit_test(
            coreaudio_reentrant_start_is_rejected_without_second_lifecycle),
        cmocka_unit_test(
            coreaudio_stop_admission_is_delayed_until_callback_exit),
        cmocka_unit_test(coreaudio_hal_route_rejects_direct_delivery_mode),
        cmocka_unit_test(coreaudio_open_failure_rolls_back_with_dispose),
        cmocka_unit_test(
            coreaudio_stateless_configure_failure_rolls_back_with_dispose),
        cmocka_unit_test(
            coreaudio_stateless_start_failure_rolls_back_with_dispose),
        cmocka_unit_test(
            coreaudio_configure_failure_rolls_back_with_dispose),
        cmocka_unit_test(
            coreaudio_bind_failure_rolls_back_with_dispose),
        cmocka_unit_test(
            coreaudio_start_failure_rolls_back_with_dispose),
        cmocka_unit_test(
            coreaudio_hal_route_converts_into_borrowed_workspace_then_copies_to_native_buffer),
        cmocka_unit_test(
            coreaudio_hal_copies_native_output_before_releasing_admission),
        cmocka_unit_test(
            coreaudio_negotiated_format_preparation_precedes_activation),
        cmocka_unit_test(
            coreaudio_negotiated_format_failures_roll_back_before_activation),
        cmocka_unit_test(
            coreaudio_unspecified_startup_rate_uses_reported_rate_before_activation),
        cmocka_unit_test(
            coreaudio_explicit_startup_rate_must_match_reported_rate),
        cmocka_unit_test(
            coreaudio_startup_rate_request_validation_stays_preflight_only),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
