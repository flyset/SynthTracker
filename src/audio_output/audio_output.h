#ifndef SYNTHTRACKER_AUDIO_OUTPUT_H
#define SYNTHTRACKER_AUDIO_OUTPUT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t frame_count;
    const uint8_t *payload;
    size_t payload_length;
} audio_frame_block;

typedef enum {
    AUDIO_OUTPUT_SUBMIT_ACCEPTED,
    AUDIO_OUTPUT_SUBMIT_MISSING_PAYLOAD,
    AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH,
} audio_output_submit_result;

typedef struct {
    size_t accepted_block_count;
    size_t accepted_payload_bytes;
} audio_output_null_adapter;

audio_output_submit_result audio_output_null_adapter_submit(
    audio_output_null_adapter *adapter,
    const audio_frame_block *block);

#endif
