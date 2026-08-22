#ifndef SYNTHTRACKER_AUDIO_OUTPUT_H
#define SYNTHTRACKER_AUDIO_OUTPUT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int32_t left;
    int32_t right;
} audio_frame;

typedef struct {
    size_t frame_count;
    const audio_frame *frames;
} audio_frame_block;

typedef enum {
    AUDIO_OUTPUT_SUBMIT_ACCEPTED,
    AUDIO_OUTPUT_SUBMIT_REJECTED,
} audio_output_submit_result;

typedef struct {
    size_t accepted_block_count;
    size_t accepted_frame_count;
} audio_output_null_adapter;

audio_output_submit_result audio_output_null_adapter_submit(
    audio_output_null_adapter *adapter,
    const audio_frame_block *block);

audio_output_submit_result audio_output_dispatch_submit(
    audio_output_null_adapter *fallback,
    const audio_frame_block *block);

#ifdef SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
typedef struct {
    size_t accepted_block_count;
    size_t accepted_frame_count;
} audio_output_null_adapter_test_snapshot;

audio_output_null_adapter_test_snapshot audio_output_null_adapter_test_inspect(
    const audio_output_null_adapter *adapter);
#endif

#endif
