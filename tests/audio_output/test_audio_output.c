#include <stddef.h>
#include <stdint.h>

#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "audio_output.h"

static_assert(offsetof(audio_output_null_adapter, accepted_block_count) == 0);
static_assert(offsetof(audio_output_null_adapter, accepted_frame_count) ==
              sizeof(size_t));
static_assert(sizeof(audio_output_null_adapter) == 2 * sizeof(size_t));

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

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(zero_frame_blocks_are_accepted_with_or_without_frames),
        cmocka_unit_test(signed_32_interleaved_frames_are_accepted_and_counted),
        cmocka_unit_test(nonzero_frame_blocks_without_frames_are_rejected_and_not_counted),
        cmocka_unit_test(accepted_frames_are_not_retained_after_submit),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
