#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "audio_output.h"
#include "adapters/coreaudio_adapter.h"
#include "fake_coreaudio_facade.h"
#include "playback_context.h"
#include "playback_legacy_mixer.h"
#include "playback_legacy_renderer.h"

enum {
    LEGACY_TICK_CLOCKS = 14318,
    LEGACY_MIX_CLOCK = 357955,
    MAX_REQUEST_FRAMES = 1024,
    REQUEST_COUNT = 7,
    OBSERVED_SAMPLE_CAPACITY = 2 * MAX_REQUEST_FRAMES,
};

typedef struct {
    const char *name;
    const char *mdat_path;
    const char *smpl_path;
    bool assert_first_historical_pcm_frame;
} fixture_case;

typedef struct {
    tfmx_playback_context *playback;
    fake_coreaudio_facade *fake;
    audio_frame frames[MAX_REQUEST_FRAMES];
    audio_frame tick_frames[MAX_REQUEST_FRAMES];
    tfmx_playback_legacy_renderer renderer;
    bool renderer_prepared;
    size_t preparation_call_count;
    unsigned int prepared_rate_hz;
    size_t tick_advance_count;
    size_t last_tick_frame_count;
    bool first_historical_pcm_frame_found;
    size_t first_historical_pcm_frame_index;
    int32_t first_historical_pcm_left;
    int32_t first_historical_pcm_right;
} legacy_tick_probe;

typedef struct {
    float samples[OBSERVED_SAMPLE_CAPACITY];
    size_t sample_count;
    size_t submission_count;
    bool all_samples_are_float32_range;
    bool non_silent_left;
    bool non_silent_right;
    bool frame_continuity;
    bool expect_frame_continuity;
    bool have_last_frame;
    float last_left;
    float last_right;
    float expected_previous_left;
    float expected_previous_right;
    float expected_next_left;
    float expected_next_right;
    bool first_float_frame_found;
    size_t first_float_submission_index;
    size_t first_float_frame_index;
    float first_float_left;
    float first_float_right;
} float32_observation;

static float32_observation *active_observation;

static size_t expected_tick_frame_count(unsigned int output_rate_hz,
                                        size_t tick_number)
{
    const uint64_t process =
        (uint64_t)LEGACY_TICK_CLOCKS * (output_rate_hz >> 1);
    uint64_t remainder = 0;
    size_t frame_count = 0;

    for (size_t tick = 0; tick < tick_number; tick++) {
        frame_count = (size_t)(process / LEGACY_MIX_CLOCK);
        remainder += process % LEGACY_MIX_CLOCK;
        if (remainder > LEGACY_MIX_CLOCK) {
            frame_count++;
            remainder -= LEGACY_MIX_CLOCK;
        }
    }
    return frame_count;
}

static float audio_frame_sample_to_float(int32_t sample)
{
    return (float)sample / 2147483648.0f;
}

static bool legacy_tick_probe_prepare(
    void *context,
    const audio_output_coreaudio_format *format)
{
    legacy_tick_probe *probe = context;
    if (probe == NULL || probe->fake == NULL || format == NULL ||
        probe->preparation_call_count != 0 ||
        (format->sample_rate_hz != 44100 && format->sample_rate_hz != 48000)) {
        return false;
    }

    fake_coreaudio_facade_record_preparation(probe->fake);
    tfmx_playback_legacy_renderer_init(
        &probe->renderer, probe->playback, format->sample_rate_hz,
        probe->frames, MAX_REQUEST_FRAMES, probe->tick_frames,
        MAX_REQUEST_FRAMES);
    probe->renderer_prepared = true;
    probe->preparation_call_count++;
    probe->prepared_rate_hz = format->sample_rate_hz;
    return true;
}

static bool float_samples_match(float actual, float expected)
{
    return fabsf(actual - expected) <= 0.000001f;
}

static bool float32_bits_match(float actual, float expected)
{
    return memcmp(&actual, &expected, sizeof(actual)) == 0;
}

static bool lifecycle_trace_matches(
    const fake_coreaudio_facade *fake,
    const fake_coreaudio_facade_lifecycle_call *expected,
    size_t expected_count)
{
    if (fake == NULL || fake->lifecycle_trace_count != expected_count) {
        return false;
    }
    for (size_t index = 0; index < expected_count; index++) {
        if (fake->lifecycle_trace[index] != expected[index]) {
            return false;
        }
    }
    return true;
}

static audio_frame_block legacy_tick_probe_render(
    void *context,
    size_t requested_frame_count)
{
    legacy_tick_probe *probe = context;
    if (probe == NULL || !probe->renderer_prepared) {
        return (audio_frame_block){ .frame_count = 0, .frames = NULL };
    }

    const audio_frame_block block =
        tfmx_playback_legacy_renderer_render(&probe->renderer,
                                             requested_frame_count);
    if (!probe->first_historical_pcm_frame_found && block.frames != NULL) {
        for (size_t frame_index = 0; frame_index < block.frame_count;
             frame_index++) {
            if (block.frames[frame_index].left != 0 ||
                block.frames[frame_index].right != 0) {
                probe->first_historical_pcm_frame_found = true;
                probe->first_historical_pcm_frame_index = frame_index;
                probe->first_historical_pcm_left =
                    block.frames[frame_index].left;
                probe->first_historical_pcm_right =
                    block.frames[frame_index].right;
                break;
            }
        }
    }
    probe->tick_advance_count = probe->renderer.tick_advance_count;
    probe->last_tick_frame_count = probe->renderer.last_tick_frame_count;
    return block;
}

static void observe_float32_output(const float *samples, size_t sample_count)
{
    if (active_observation == NULL ||
        sample_count > OBSERVED_SAMPLE_CAPACITY) {
        return;
    }

    active_observation->submission_count++;
    active_observation->sample_count = sample_count;
    for (size_t index = 0; index < sample_count; index++) {
        active_observation->samples[index] = samples[index];
        if (!isfinite(samples[index]) || samples[index] < -1.0f ||
            samples[index] > 1.0f) {
            active_observation->all_samples_are_float32_range = false;
        }
        if (samples[index] != 0.0f) {
            if ((index % 2) == 0) {
                active_observation->non_silent_left = true;
            } else {
                active_observation->non_silent_right = true;
            }
        }
    }
    if (!active_observation->first_float_frame_found &&
        sample_count >= 2) {
        for (size_t frame_index = 0; frame_index < sample_count / 2;
             frame_index++) {
            const float left = samples[frame_index * 2];
            const float right = samples[frame_index * 2 + 1];
            if (left != 0.0f || right != 0.0f) {
                active_observation->first_float_frame_found = true;
                active_observation->first_float_submission_index =
                    active_observation->submission_count;
                active_observation->first_float_frame_index =
                    frame_index;
                active_observation->first_float_left = left;
                active_observation->first_float_right = right;
                break;
            }
        }
    }
    if (sample_count >= 2 && sample_count % 2 == 0) {
        if (active_observation->expect_frame_continuity) {
            if (!active_observation->have_last_frame ||
                !float_samples_match(active_observation->last_left,
                                     active_observation->expected_previous_left) ||
                !float_samples_match(active_observation->last_right,
                                     active_observation->expected_previous_right) ||
                !float_samples_match(samples[0],
                                     active_observation->expected_next_left) ||
                !float_samples_match(samples[1],
                                     active_observation->expected_next_right)) {
                active_observation->frame_continuity = false;
            }
            active_observation->expect_frame_continuity = false;
        }
        active_observation->last_left = samples[sample_count - 2];
        active_observation->last_right = samples[sample_count - 1];
        active_observation->have_last_frame = true;
    }
}

static void run_fixture_at_rate(const fixture_case *fixture,
                                unsigned int output_rate_hz,
                                size_t *failed_case_count,
                                char *first_failure,
    size_t first_failure_capacity)
{
    const size_t first_tick_frames =
        expected_tick_frame_count(output_rate_hz, 1);
    const size_t second_tick_frames =
        expected_tick_frame_count(output_rate_hz, 2);
    const size_t requests[REQUEST_COUNT] = {
        0,
        7,
        3,
        first_tick_frames + 5,
        1,
        second_tick_frames - 16,
        1,
    };
    const size_t expected_tick_counts[REQUEST_COUNT] = { 0, 1, 1, 2, 2, 2, 3 };
    const audio_output_coreaudio_format format = {
        .sample_rate_hz = output_rate_hz,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    tfmx_playback_context *playback = tfmx_playback_context_create();
    legacy_tick_probe probe = {
        .playback = playback,
    };
    fake_coreaudio_facade fake;
    float32_observation observation = {
        .all_samples_are_float32_range = true,
        .frame_continuity = true,
    };
    float workspace[OBSERVED_SAMPLE_CAPACITY];
    bool case_failed = false;
    audio_output_coreaudio_adapter_instance instance;

    assert_non_null(playback);
    assert_int_equal(tfmx_playback_context_load(
                         playback, fixture->mdat_path, fixture->smpl_path),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(playback, 0),
                     TFMX_START_SUCCESS);
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    fake_coreaudio_facade_set_reported_format(&fake, &format);
    probe.fake = &fake;
    instance = (audio_output_coreaudio_adapter_instance){
        .facade = &fake.facade,
        .route = {
            .renderer = legacy_tick_probe_render,
            .prepare = legacy_tick_probe_prepare,
            .context = &probe,
        },
        .delivery_mode = AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE,
        .workspace = {
            .samples = workspace,
            .frame_capacity = MAX_REQUEST_FRAMES,
        },
    };

    active_observation = &observation;
    audio_output_coreaudio_adapter_test_reset_allocation_attempt_count();
    audio_output_coreaudio_adapter_test_set_observer(
        observe_float32_output);
    assert_int_equal(
        audio_output_coreaudio_adapter_start_instance(&instance, &format),
        AUDIO_OUTPUT_COREAUDIO_START_STARTED);
    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_PREPARE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_START,
    };
    if (!probe.renderer_prepared || probe.preparation_call_count != 1 ||
        probe.prepared_rate_hz != output_rate_hz ||
        probe.prepared_rate_hz != fake.reported_format.sample_rate_hz ||
        probe.renderer.output_rate_hz != fake.reported_format.sample_rate_hz ||
        fake.prepare_call_count != 1 ||
        fake.render_request_count != 0 ||
        !lifecycle_trace_matches(
            &fake, expected_lifecycle,
            sizeof(expected_lifecycle) / sizeof(expected_lifecycle[0]))) {
        case_failed = true;
    }

    for (size_t request_index = 0; request_index < REQUEST_COUNT;
         request_index++) {
        if (request_index == 3 || request_index == 5) {
            const size_t retained_frame_offset =
                probe.renderer.tick_frame_offset;
            if (retained_frame_offset == 0 ||
                retained_frame_offset >= probe.renderer.tick_frame_count) {
                observation.frame_continuity = false;
            } else {
                /* The next request must begin with the frame retained after
                 * the previous partial request, not a restarted tick. */
                observation.expected_previous_left =
                    audio_frame_sample_to_float(
                        probe.tick_frames[retained_frame_offset - 1].left);
                observation.expected_previous_right =
                    audio_frame_sample_to_float(
                        probe.tick_frames[retained_frame_offset - 1].right);
                observation.expected_next_left = audio_frame_sample_to_float(
                    probe.tick_frames[retained_frame_offset].left);
                observation.expected_next_right = audio_frame_sample_to_float(
                    probe.tick_frames[retained_frame_offset].right);
                observation.expect_frame_continuity = true;
            }
        }
        const audio_output_submit_result result =
            fake_coreaudio_facade_request(&fake, requests[request_index]);
        if (result != AUDIO_OUTPUT_SUBMIT_ACCEPTED ||
            probe.tick_advance_count != expected_tick_counts[request_index]) {
            case_failed = true;
        }
        if (request_index == 3 &&
            probe.last_tick_frame_count != second_tick_frames) {
            case_failed = true;
        }
    }

    if (fake.request_trace_count != REQUEST_COUNT ||
        observation.submission_count != 6 ||
        observation.sample_count != 2 ||
        !observation.all_samples_are_float32_range ||
        !observation.non_silent_left || !observation.non_silent_right ||
        !observation.frame_continuity || observation.expect_frame_continuity ||
        audio_output_coreaudio_adapter_test_allocation_attempt_count() != 0) {
        case_failed = true;
    }

    if (fixture->assert_first_historical_pcm_frame) {
        const float expected_left = 1680.0f / 32768.0f;
        const float expected_right = 1392.0f / 32768.0f;
        const int32_t expected_frame_left = INT32_C(1680) * INT32_C(65536);
        const int32_t expected_frame_right = INT32_C(1392) * INT32_C(65536);
        if (!probe.first_historical_pcm_frame_found ||
            probe.first_historical_pcm_left != expected_frame_left ||
            probe.first_historical_pcm_right != expected_frame_right ||
            !observation.first_float_frame_found ||
            !float32_bits_match(observation.first_float_left,
                                expected_left) ||
            !float32_bits_match(observation.first_float_right,
                                expected_right)) {
            case_failed = true;
        }
    }

    audio_output_coreaudio_adapter_test_set_observer(NULL);
    active_observation = NULL;
    tfmx_playback_context_destroy(playback);

    if (case_failed) {
        if (*failed_case_count == 0) {
            (void)snprintf(
                first_failure, first_failure_capacity,
                "%s at %u Hz (ticks=%zu, last_tick_frames=%zu, expected=%zu)",
                fixture->name, output_rate_hz, probe.tick_advance_count,
                probe.last_tick_frame_count, second_tick_frames);
        }
        *failed_case_count += 1;
    }
}

static void negotiated_rate_prepares_legacy_renderer_for_exact_requests(
    void **state)
{
    (void)state;

    const fixture_case fixtures[] = {
        {
            .name = "step8",
            .mdat_path = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
            .smpl_path = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
        },
        {
            .name = "loop_f1",
            .mdat_path = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.loop_f1",
            .smpl_path = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.loop_f1",
        },
        {
            .name = "envelope_tempo",
            .mdat_path = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.envelope_tempo",
            .smpl_path = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.envelope_tempo",
        },
        {
            .name = "voices_01",
            .mdat_path = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.voices_01",
            .smpl_path = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.voices_01",
            .assert_first_historical_pcm_frame = true,
        },
    };
    const unsigned int output_rates[] = { 44100, 48000 };
    size_t failed_case_count = 0;
    char first_failure[160] = { 0 };

    for (size_t fixture_index = 0;
         fixture_index < sizeof(fixtures) / sizeof(fixtures[0]);
         fixture_index++) {
        for (size_t rate_index = 0;
             rate_index < sizeof(output_rates) / sizeof(output_rates[0]);
             rate_index++) {
            run_fixture_at_rate(&fixtures[fixture_index],
                                output_rates[rate_index], &failed_case_count,
                                first_failure, sizeof(first_failure));
        }
    }

    if (failed_case_count != 0) {
        fail_msg("%zu fixture/rate cases are still missing exact-N legacy "
                 "rendering; first failure: %s",
                 failed_case_count, first_failure);
    }
}

static void legacy_mixer_normalizes_signed16_pcm_values_safely(void **state)
{
    (void)state;

    const struct {
        int32_t mixed_sample;
        int32_t expected_frame_sample;
    } cases[] = {
        { INT32_C(-32768), INT32_MIN },
        { INT32_C(-1), INT32_C(-65536) },
        { INT32_C(0), INT32_C(0) },
        { INT32_C(1), INT32_C(65536) },
        { INT32_C(32767), INT32_C(2147418112) },
        /* The historical low-16-bit wrap is -32768 before expansion. */
        { INT32_C(32768), INT32_MIN },
        { INT32_C(65535), INT32_C(-65536) },
    };

    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        assert_int_equal(
            tfmx_playback_legacy_mixer_test_normalize_pcm_sample(
                cases[index].mixed_sample),
            cases[index].expected_frame_sample);
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(legacy_mixer_normalizes_signed16_pcm_values_safely),
        cmocka_unit_test(
            negotiated_rate_prepares_legacy_renderer_for_exact_requests),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
