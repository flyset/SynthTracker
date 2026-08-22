#include "audio_output.h"

#if defined(__APPLE__)
#include "adapters/coreaudio_adapter.h"
#endif

audio_output_submit_result audio_output_null_adapter_submit(
    audio_output_null_adapter *adapter,
    const audio_frame_block *block)
{
    if (block->frame_count > 0 && block->frames == NULL) {
        return AUDIO_OUTPUT_SUBMIT_REJECTED;
    }

    adapter->accepted_block_count += 1;
    adapter->accepted_frame_count += block->frame_count;
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}

audio_output_submit_result audio_output_dispatch_submit(
    audio_output_null_adapter *fallback,
    const audio_frame_block *block)
{
#if defined(__APPLE__)
    (void)fallback;
    return audio_output_coreaudio_adapter_submit(block);
#else
    return audio_output_null_adapter_submit(fallback, block);
#endif
}

#ifdef SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
audio_output_null_adapter_test_snapshot audio_output_null_adapter_test_inspect(
    const audio_output_null_adapter *adapter)
{
    return (audio_output_null_adapter_test_snapshot){
        .accepted_block_count = adapter->accepted_block_count,
        .accepted_frame_count = adapter->accepted_frame_count,
    };
}
#endif
