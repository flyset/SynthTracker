#include "recording_sink.h"

#include <stddef.h>

enum {
    RECORDING_SINK_CAPACITY = 65536,
    RECORDING_SINK_SUBMISSION_CAPACITY = 16,
};

static audio_frame recorded_frames[RECORDING_SINK_CAPACITY];
static size_t recorded_frame_count;
static size_t submission_frame_counts[RECORDING_SINK_SUBMISSION_CAPACITY];
static size_t submission_count;

void recording_sink_reset(void)
{
    recorded_frame_count = 0;
    submission_count = 0;
}

size_t recording_sink_submission_count(void)
{
    return submission_count;
}

size_t recording_sink_submission_frame_count(size_t submission_index)
{
    return submission_frame_counts[submission_index];
}

size_t recording_sink_frame_count(void)
{
    return recorded_frame_count;
}

const audio_frame *recording_sink_frame(size_t index)
{
    return &recorded_frames[index];
}

audio_output_submit_result audio_output_null_adapter_submit(
    audio_output_null_adapter *adapter,
    const audio_frame_block *block)
{
    (void)adapter;

    if (block == NULL || (block->frame_count > 0 && block->frames == NULL) ||
        block->frame_count > RECORDING_SINK_CAPACITY ||
        submission_count >= RECORDING_SINK_SUBMISSION_CAPACITY) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    for (size_t index = 0; index < block->frame_count; index++) {
        recorded_frames[index] = block->frames[index];
    }
    recorded_frame_count = block->frame_count;
    submission_frame_counts[submission_count] = block->frame_count;
    submission_count++;
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}
