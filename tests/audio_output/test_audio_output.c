#include <stddef.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <cmocka.h>

#include "audio_output.h"

static void zero_frame_validity_depends_only_on_payload_length(void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    const uint8_t payload = 0;
    const audio_frame_block null_zero_length = {
        .frame_count = 0,
        .payload = NULL,
        .payload_length = 0,
    };
    const audio_frame_block nonnull_zero_length = {
        .frame_count = 0,
        .payload = &payload,
        .payload_length = 0,
    };
    const audio_frame_block null_nonzero_length = {
        .frame_count = 0,
        .payload = NULL,
        .payload_length = 1,
    };
    const audio_frame_block nonnull_nonzero_length = {
        .frame_count = 0,
        .payload = &payload,
        .payload_length = 1,
    };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &null_zero_length),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(adapter.accepted_block_count, 0);
    assert_int_equal(adapter.accepted_payload_bytes, 0);

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &nonnull_zero_length),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(adapter.accepted_block_count, 0);
    assert_int_equal(adapter.accepted_payload_bytes, 0);

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &null_nonzero_length),
                     AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH);
    assert_int_equal(adapter.accepted_block_count, 0);
    assert_int_equal(adapter.accepted_payload_bytes, 0);

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &nonnull_nonzero_length),
                     AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH);
    assert_int_equal(adapter.accepted_block_count, 0);
    assert_int_equal(adapter.accepted_payload_bytes, 0);
}

static void nonzero_exact_payload_blocks_are_accepted_and_counted(void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    const uint8_t payload[12] = {0};
    const audio_frame_block one_frame = {
        .frame_count = 1,
        .payload = payload,
        .payload_length = 4,
    };
    const audio_frame_block three_frames = {
        .frame_count = 3,
        .payload = payload,
        .payload_length = 12,
    };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &one_frame),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(audio_output_null_adapter_submit(&adapter, &three_frames),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(adapter.accepted_block_count, 2);
    assert_int_equal(adapter.accepted_payload_bytes, 16);
}

static void missing_payload_is_rejected_and_not_counted(void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    const audio_frame_block block = {
        .frame_count = 1,
        .payload = NULL,
        .payload_length = 4,
    };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &block),
                     AUDIO_OUTPUT_SUBMIT_MISSING_PAYLOAD);
    assert_int_equal(adapter.accepted_block_count, 0);
    assert_int_equal(adapter.accepted_payload_bytes, 0);
}

static void wrong_payload_length_is_distinct_from_missing_payload(void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    const uint8_t payload[3] = {0};
    const audio_frame_block wrong_length = {
        .frame_count = 1,
        .payload = payload,
        .payload_length = 3,
    };
    const audio_frame_block missing_payload = {
        .frame_count = 1,
        .payload = NULL,
        .payload_length = 4,
    };

    const audio_output_submit_result wrong_length_result =
        audio_output_null_adapter_submit(&adapter, &wrong_length);
    const audio_output_submit_result missing_payload_result =
        audio_output_null_adapter_submit(&adapter, &missing_payload);

    assert_int_equal(wrong_length_result,
                     AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH);
    assert_int_equal(missing_payload_result,
                     AUDIO_OUTPUT_SUBMIT_MISSING_PAYLOAD);
    assert_int_not_equal(wrong_length_result, missing_payload_result);
    assert_int_equal(adapter.accepted_block_count, 0);
    assert_int_equal(adapter.accepted_payload_bytes, 0);
}

static void accepted_payloads_are_discarded_without_retention(void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    uint8_t first_payload[4] = {1, 2, 3, 4};
    uint8_t second_payload[8] = {5, 6, 7, 8, 9, 10, 11, 12};
    const audio_frame_block first_block = {
        .frame_count = 1,
        .payload = first_payload,
        .payload_length = sizeof(first_payload),
    };
    const audio_frame_block second_block = {
        .frame_count = 2,
        .payload = second_payload,
        .payload_length = sizeof(second_payload),
    };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &first_block),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(adapter.accepted_block_count, 1);
    assert_int_equal(adapter.accepted_payload_bytes, 4);

    for (size_t index = 0; index < sizeof(first_payload); ++index) {
        first_payload[index] = 0;
    }
    assert_int_equal(adapter.accepted_block_count, 1);
    assert_int_equal(adapter.accepted_payload_bytes, 4);

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &second_block),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(adapter.accepted_block_count, 2);
    assert_int_equal(adapter.accepted_payload_bytes, 12);

    for (size_t index = 0; index < sizeof(second_payload); ++index) {
        second_payload[index] = 0;
    }
    assert_int_equal(adapter.accepted_block_count, 2);
    assert_int_equal(adapter.accepted_payload_bytes, 12);
}

static void overflowing_frame_counts_are_rejected_before_payload_classification(void **state)
{
    (void)state;
    audio_output_null_adapter adapter = {0};
    const uint8_t payload = 0;
    const audio_frame_block null_payload = {
        .frame_count = SIZE_MAX / 4 + 1,
        .payload = NULL,
        .payload_length = 0,
    };
    const audio_frame_block nonnull_payload = {
        .frame_count = SIZE_MAX / 4 + 1,
        .payload = &payload,
        .payload_length = 0,
    };

    const audio_output_submit_result null_result =
        audio_output_null_adapter_submit(&adapter, &null_payload);
    const audio_output_submit_result nonnull_result =
        audio_output_null_adapter_submit(&adapter, &nonnull_payload);

    assert_int_equal(null_result, AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH);
    assert_int_equal(nonnull_result, AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH);
    assert_int_equal(adapter.accepted_block_count, 0);
    assert_int_equal(adapter.accepted_payload_bytes, 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(zero_frame_validity_depends_only_on_payload_length),
        cmocka_unit_test(nonzero_exact_payload_blocks_are_accepted_and_counted),
        cmocka_unit_test(missing_payload_is_rejected_and_not_counted),
        cmocka_unit_test(wrong_payload_length_is_distinct_from_missing_payload),
        cmocka_unit_test(accepted_payloads_are_discarded_without_retention),
        cmocka_unit_test(overflowing_frame_counts_are_rejected_before_payload_classification),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
