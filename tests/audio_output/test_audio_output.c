#include <stddef.h>
#include <stdint.h>

#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "audio_output.h"
#include "adapters/coreaudio_adapter.h"

static_assert(offsetof(audio_output_null_adapter, accepted_block_count) == 0);
static_assert(offsetof(audio_output_null_adapter, accepted_frame_count) ==
              sizeof(size_t));
static_assert(sizeof(audio_output_null_adapter) == 2 * sizeof(size_t));

#if defined(__APPLE__)
enum { OBSERVED_SAMPLE_CAPACITY = 8 };

static float observed_samples[OBSERVED_SAMPLE_CAPACITY];
static size_t observed_sample_count;
static size_t observed_submission_count;

static void reset_coreaudio_observation(void)
{
    observed_sample_count = 0;
    observed_submission_count = 0;
}

static void record_coreaudio_output(const float *samples, size_t sample_count)
{
    assert_true(sample_count <= OBSERVED_SAMPLE_CAPACITY);
    if (sample_count > 0) {
        assert_non_null(samples);
    }

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
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
