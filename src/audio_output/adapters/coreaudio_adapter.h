#ifndef SYNTHTRACKER_COREAUDIO_ADAPTER_H
#define SYNTHTRACKER_COREAUDIO_ADAPTER_H

#include "audio_output.h"

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
