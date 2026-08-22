#include "coreaudio_adapter.h"

#include <stdlib.h>

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
