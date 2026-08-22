#include <stdint.h>
#include <string.h>

#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "player.h"
#include "recording_sink.h"

enum { HALFBUFSIZE = 65536 * 4 };

extern struct Audio audioData[8];
extern struct Channel channelData[8];
extern struct TrackManager trackManager;
extern S32 tbuf[HALFBUFSIZE * 2];
extern char act[8];
extern int multimode;
extern U32 blocksize, multiplier, stereo;
extern int bytes, bytes2, force8, blend, filt, over;
extern int eRem, toOutFile;
void processAudioData(S32 *num_samples_to_process, S32 *buf_position,
                      int *buf_proc_counter, int *audio_samples);

static int controlled_voice_does_not_loop(struct Audio *audio)
{
    (void)audio;
    return 0;
}

static void reset_audio_globals(void)
{
    memset(audioData, 0, sizeof(struct Audio) * 8);
    memset(channelData, 0, sizeof(struct Channel) * 8);
    memset(tbuf, 0, sizeof(S32) * HALFBUFSIZE * 2);
    for (size_t index = 0; index < 8; index++) {
        act[index] = 1;
    }

    trackManager = (struct TrackManager){ .PlayerEnable = 1 };
    multimode = 1;
    blocksize = HALFBUFSIZE;
    multiplier = 1;
    stereo = 2;
    force8 = 1;
    blend = 1;
    filt = 3;
    over = -1;
    bytes = 0;
    bytes2 = 0;
    eRem = 0;
    toOutFile = 0;
}

static void routes_mixed_multimode_lanes_to_one_recorded_block(void **state)
{
    (void)state;
    reset_audio_globals();
    recording_sink_reset();

    const S32 position = 7;
    tbuf[HALFBUFSIZE + position] = 16384;
    tbuf[HALFBUFSIZE + position + 1] = -16384;
    tbuf[HALFBUFSIZE + position + 2] = 123;

    static S8 right_lane_samples[] = { 10, 20, 30, 40 };
    audioData[4] = (struct Audio){
        .delta = 0x4000,
        .slen = 4,
        .SampleLength = 4,
        .sbeg = right_lane_samples,
        .SampleStart = right_lane_samples,
        .vol = 2,
        .mode = 1,
        .loop = controlled_voice_does_not_loop,
    };
    for (size_t index = 0; index < 8; index++) {
        act[index] = index == 4;
    }

    S32 samples_to_process = 3;
    S32 buffer_position = position;
    int block_counter = 0;
    int audio_samples = 0;
    processAudioData(&samples_to_process, &buffer_position, &block_counter,
                     &audio_samples);

    assert_int_equal(recording_sink_frame_count(), 3);
    const audio_frame expected_frames[] = {
        { .left = 16383, .right = 20 },
        { .left = -16383, .right = 40 },
        { .left = 123, .right = 60 },
    };
    assert_memory_equal(recording_sink_frame(0), &expected_frames[0],
                        sizeof(audio_frame));
    assert_memory_equal(recording_sink_frame(1), &expected_frames[1],
                        sizeof(audio_frame));
    assert_memory_equal(recording_sink_frame(2), &expected_frames[2],
                        sizeof(audio_frame));

    for (size_t index = 0; index < 3; index++) {
        assert_int_equal(tbuf[position + index], 0);
        assert_int_equal(tbuf[HALFBUFSIZE + position + index], 0);
    }
}

static void routes_normalized_stereo_profile_without_blending(void **state)
{
    (void)state;
    reset_audio_globals();
    blend = 0;
    stereo = 1;
    force8 = 0;
    filt = 0;
    recording_sink_reset();

    const S32 position = 7;
    tbuf[HALFBUFSIZE + position] = 16384;
    tbuf[HALFBUFSIZE + position + 1] = -16384;
    tbuf[HALFBUFSIZE + position + 2] = 123;

    static S8 right_lane_samples[] = { 10, 20, 30, 40 };
    audioData[4] = (struct Audio){
        .delta = 0x4000,
        .slen = 4,
        .SampleLength = 4,
        .sbeg = right_lane_samples,
        .SampleStart = right_lane_samples,
        .vol = 2,
        .mode = 1,
        .loop = controlled_voice_does_not_loop,
    };
    for (size_t index = 0; index < 8; index++) {
        act[index] = index == 4;
    }

    S32 samples_to_process = 3;
    S32 buffer_position = position;
    int block_counter = 0;
    int audio_samples = 0;
    processAudioData(&samples_to_process, &buffer_position, &block_counter,
                     &audio_samples);

    const audio_frame expected_frames[] = {
        { .left = 16383, .right = 20 },
        { .left = -16383, .right = 40 },
        { .left = 123, .right = 60 },
    };
    assert_int_equal(recording_sink_frame_count(), 3);
    for (size_t index = 0; index < 3; index++) {
        assert_memory_equal(recording_sink_frame(index), &expected_frames[index],
                            sizeof(audio_frame));
    }
}

static void accepts_declared_bridge_maximum(void **state)
{
    (void)state;
    recording_sink_reset();

    static audio_frame frames[65536];
    const audio_frame_block block = {
        .frame_count = 65536,
        .frames = frames,
    };
    audio_output_null_adapter adapter = { 0 };

    assert_int_equal(audio_output_null_adapter_submit(&adapter, &block),
                     AUDIO_OUTPUT_SUBMIT_ACCEPTED);
    assert_int_equal(recording_sink_submission_count(), 1);
    assert_int_equal(recording_sink_frame_count(), 65536);
}

static void submits_one_declared_bridge_maximum_mixem_block(void **state)
{
    (void)state;
    reset_audio_globals();
    recording_sink_reset();

    blocksize = 65536;
    S32 samples_to_process = 65536;
    S32 buffer_position = 0;
    int block_counter = 0;
    int audio_samples = 0;

    processAudioData(&samples_to_process, &buffer_position, &block_counter,
                     &audio_samples);

    assert_int_equal(recording_sink_submission_count(), 1);
    assert_int_equal(recording_sink_submission_frame_count(0), 65536);
}

static void submits_full_then_partial_blocks(void **state)
{
    (void)state;
    reset_audio_globals();
    recording_sink_reset();

    blocksize = 4;
    tbuf[HALFBUFSIZE + 0] = 100;
    tbuf[HALFBUFSIZE + 1] = 200;
    tbuf[HALFBUFSIZE + 2] = 300;
    tbuf[HALFBUFSIZE + 3] = 400;
    tbuf[0] = 10;
    tbuf[1] = 20;
    tbuf[2] = 30;
    tbuf[3] = 40;

    S32 samples_to_process = 4;
    S32 buffer_position = 0;
    int block_counter = 0;
    int audio_samples = 0;
    processAudioData(&samples_to_process, &buffer_position, &block_counter,
                     &audio_samples);

    tbuf[HALFBUFSIZE + 0] = 500;
    tbuf[HALFBUFSIZE + 1] = 600;
    tbuf[0] = 50;
    tbuf[1] = 60;
    samples_to_process = 2;
    processAudioData(&samples_to_process, &buffer_position, &block_counter,
                     &audio_samples);

    assert_int_equal(recording_sink_submission_count(), 2);
    assert_int_equal(recording_sink_submission_frame_count(0), 4);
    assert_int_equal(recording_sink_submission_frame_count(1), 2);
    assert_int_equal(recording_sink_frame_count(), 2);
    const audio_frame expected_partial[] = {
        { .left = 500, .right = 50 },
        { .left = 600, .right = 60 },
    };
    assert_memory_equal(recording_sink_frame(0), &expected_partial[0],
                        sizeof(audio_frame));
    assert_memory_equal(recording_sink_frame(1), &expected_partial[1],
                        sizeof(audio_frame));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(routes_mixed_multimode_lanes_to_one_recorded_block),
        cmocka_unit_test(routes_normalized_stereo_profile_without_blending),
        cmocka_unit_test(accepts_declared_bridge_maximum),
        cmocka_unit_test(submits_one_declared_bridge_maximum_mixem_block),
        cmocka_unit_test(submits_full_then_partial_blocks),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
