#include "audio_output.h"

audio_output_submit_result audio_output_null_adapter_submit(
    audio_output_null_adapter *adapter,
    const audio_frame_block *block)
{
    if (block->frame_count == 0) {
        return block->payload_length == 0
                   ? AUDIO_OUTPUT_SUBMIT_ACCEPTED
                   : AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH;
    }

    if (block->frame_count > SIZE_MAX / 4) {
        return AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH;
    }

    if (block->frame_count > 0 && block->payload == NULL) {
        return AUDIO_OUTPUT_SUBMIT_MISSING_PAYLOAD;
    }

    if (block->frame_count > 0 && block->payload != NULL &&
        block->payload_length == block->frame_count * 4) {
        adapter->accepted_block_count += 1;
        adapter->accepted_payload_bytes += block->payload_length;
        return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
    }

    (void)adapter;
    return AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH;
}
