#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <cmocka.h>

#include "application.h"
#include "fake_coreaudio_facade.h"
#include "playback_legacy_bridge.h"

typedef struct {
    fake_coreaudio_facade *fake;
    size_t preparation_call_count;
    unsigned int prepared_rate_hz;
    unsigned int expected_rate_hz;
    size_t request_count_at_preparation;
} application_preparation_observation;

static application_preparation_observation *active_observation;

static void observe_application_preparation(
    const audio_output_coreaudio_format *format,
    unsigned int renderer_rate_hz)
{
    application_preparation_observation *observation = active_observation;

    assert_non_null(observation);
    assert_non_null(format);
    assert_int_equal(observation->fake->lifecycle_trace_count, 2);
    assert_int_equal(observation->fake->lifecycle_trace[0],
                     FAKE_COREAUDIO_FACADE_CALL_OPEN);
    assert_int_equal(observation->fake->lifecycle_trace[1],
                     FAKE_COREAUDIO_FACADE_CALL_CONFIGURE);
    assert_int_equal(format->sample_rate_hz, observation->expected_rate_hz);
    observation->request_count_at_preparation =
        observation->fake->render_request_count;
    observation->prepared_rate_hz = renderer_rate_hz;
    observation->preparation_call_count++;
    fake_coreaudio_facade_record_preparation(observation->fake);
}

static void reset_application_test_state(void)
{
    application_test_set_coreaudio_facade(NULL);
    application_test_set_preparation_observer(NULL);
    active_observation = NULL;
    tfmx_playback_legacy_bridge_reset();
}

static int application_audio_lifecycle_setup(void **state)
{
    (void)state;
    reset_application_test_state();
    return 0;
}

static int application_audio_lifecycle_teardown(void **state)
{
    (void)state;
    reset_application_test_state();
    return 0;
}

static void assert_lifecycle_trace(
    const fake_coreaudio_facade *fake,
    const fake_coreaudio_facade_lifecycle_call *expected,
    size_t expected_count)
{
    assert_int_equal(fake->lifecycle_trace_count, expected_count);
    for (size_t index = 0; index < expected_count; index++) {
        assert_int_equal(fake->lifecycle_trace[index], expected[index]);
    }
}

static void assert_request_trace_truncated_at_capacity(
    const fake_coreaudio_facade *fake,
    size_t expected_total_request_count,
    size_t requested_frame_count)
{
    /* Distinguish (a) total serviced callbacks (render_request_count),
     * (b) safely retained trace entries (request_trace_count, capped at the
     * trace capacity), and (c) truncation (request_trace_truncated). The fake
     * may service more callbacks than the fixed trace buffer can retain, but
     * must never write past the buffer. */
    assert_int_equal(fake->render_request_count, expected_total_request_count);
    assert_int_equal(fake->request_trace_count,
                     FAKE_COREAUDIO_FACADE_REQUEST_TRACE_CAPACITY);
    assert_true(fake->request_trace_truncated);
    for (size_t index = 0;
         index < FAKE_COREAUDIO_FACADE_REQUEST_TRACE_CAPACITY; index++) {
        assert_int_equal(fake->request_trace[index], requested_frame_count);
    }
}

static void configure_fake_workspace_lifecycle(fake_coreaudio_facade *fake)
{
    const audio_output_coreaudio_format reported_format = {
        .sample_rate_hz = 48000,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };

    fake_coreaudio_facade_init(fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    fake_coreaudio_facade_set_workspace_only(fake, true);
    fake_coreaudio_facade_set_reported_format(fake, &reported_format);
    fake_coreaudio_facade_request_during_start(fake, 1024, 64);
}

static void capture_run_stderr(int argc, char **argv, int *status,
                               char *output, size_t output_capacity)
{
    int pipe_fds[2];
    int saved_stderr;
    ssize_t output_size;

    assert_int_equal(pipe(pipe_fds), 0);
    saved_stderr = dup(STDERR_FILENO);
    assert_true(saved_stderr >= 0);
    assert_int_equal(dup2(pipe_fds[1], STDERR_FILENO), STDERR_FILENO);
    close(pipe_fds[1]);

    optind = 1;
    *status = application_run(argc, argv);
    fflush(stderr);

    assert_int_equal(dup2(saved_stderr, STDERR_FILENO), STDERR_FILENO);
    close(saved_stderr);
    output_size = read(pipe_fds[0], output, output_capacity - 1);
    close(pipe_fds[0]);
    assert_true(output_size >= 0);
    output[output_size] = '\0';
}

static void test_application_prepares_renderer_at_negotiated_rate(void **state)
{
    fake_coreaudio_facade fake;
    application_preparation_observation observation;
    const audio_output_coreaudio_format reported_format = {
        .sample_rate_hz = 48000,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    char *argv[] = {
        "synthtracker",
        TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
        TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
        NULL,
    };

    (void)state;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    fake_coreaudio_facade_set_workspace_only(&fake, true);
    fake_coreaudio_facade_set_reported_format(&fake, &reported_format);
    fake_coreaudio_facade_request_during_start(&fake, 1024, 64);
    observation = (application_preparation_observation){
        .fake = &fake,
        .expected_rate_hz = 48000,
    };
    active_observation = &observation;
    application_test_set_coreaudio_facade(&fake.facade);
    application_test_set_preparation_observer(
        observe_application_preparation);

    optind = 1;
    assert_int_equal(application_run(3, argv), 0);
    assert_true(fake.requested_format_set);
    assert_int_equal(fake.requested_format.sample_rate_hz, 0);
    assert_int_equal(fake.open_call_count, 1);
    assert_int_equal(fake.configure_call_count, 1);
    assert_int_equal(observation.preparation_call_count, 1);
    assert_int_equal(observation.prepared_rate_hz, 48000);
    assert_int_equal(observation.request_count_at_preparation, 0);
    assert_int_equal(fake.prepare_call_count, 1);
    assert_int_equal(fake.bind_request_call_count, 1);
    assert_int_equal(fake.start_call_count, 1);
    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_PREPARE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_START,
        FAKE_COREAUDIO_FACADE_CALL_STOP,
        FAKE_COREAUDIO_FACADE_CALL_QUIESCE,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_lifecycle_trace(
        &fake, expected_lifecycle,
        sizeof(expected_lifecycle) / sizeof(expected_lifecycle[0]));
    assert_request_trace_truncated_at_capacity(&fake, 64, 1024);
    assert_int_equal(fake.control_stop_call_count, 1);
    assert_int_equal(fake.control_quiesce_call_count, 1);
    assert_int_equal(fake.control_dispose_call_count, 1);
    assert_false(fake.active);
    assert_true(fake.quiescent);
    assert_true(fake.disposed);
}

static void test_application_header_speed_finite_lifecycle(void **state)
{
    fake_coreaudio_facade fake;
    application_preparation_observation observation;
    const audio_output_coreaudio_format reported_format = {
        .sample_rate_hz = 44100,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    char *argv[] = {
        "synthtracker",
        TFMX_SOURCE_ROOT "/tests/fixtures/mdat.header_speed_finite",
        TFMX_SOURCE_ROOT "/tests/fixtures/smpl.header_speed_finite",
        NULL,
    };

    (void)state;
    fake_coreaudio_facade_init(&fake, AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK,
                               AUDIO_OUTPUT_COREAUDIO_FACADE_OK);
    fake_coreaudio_facade_set_workspace_only(&fake, true);
    fake_coreaudio_facade_set_reported_format(&fake, &reported_format);
    fake_coreaudio_facade_request_during_start(&fake, 1024, 64);
    observation = (application_preparation_observation){
        .fake = &fake,
        .expected_rate_hz = 44100,
    };
    active_observation = &observation;
    application_test_set_coreaudio_facade(&fake.facade);
    application_test_set_preparation_observer(
        observe_application_preparation);

    optind = 1;
    assert_int_equal(application_run(3, argv), 0);
    assert_true(fake.requested_format_set);
    assert_int_equal(fake.requested_format.sample_rate_hz, 0);
    assert_int_equal(fake.open_call_count, 1);
    assert_int_equal(fake.configure_call_count, 1);
    assert_int_equal(observation.preparation_call_count, 1);
    assert_int_equal(observation.prepared_rate_hz, 44100);
    assert_int_equal(observation.request_count_at_preparation, 0);
    assert_int_equal(fake.prepare_call_count, 1);
    assert_int_equal(fake.bind_request_call_count, 1);
    assert_int_equal(fake.start_call_count, 1);
    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_PREPARE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_START,
        FAKE_COREAUDIO_FACADE_CALL_STOP,
        FAKE_COREAUDIO_FACADE_CALL_QUIESCE,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_lifecycle_trace(
        &fake, expected_lifecycle,
        sizeof(expected_lifecycle) / sizeof(expected_lifecycle[0]));
    assert_request_trace_truncated_at_capacity(&fake, 64, 1024);
    assert_int_equal(fake.control_stop_call_count, 1);
    assert_int_equal(fake.control_quiesce_call_count, 1);
    assert_int_equal(fake.control_dispose_call_count, 1);
    assert_false(fake.active);
    assert_true(fake.quiescent);
    assert_true(fake.disposed);
}

static void test_application_selected_subsong_absolute_lifecycle(void **state)
{
    fake_coreaudio_facade fake;
    application_preparation_observation observation;
    char *argv[] = {
        "synthtracker",
        "-p", "1",
        "-P", "2",
        TFMX_SOURCE_ROOT "/tests/fixtures/mdat.selected_01",
        TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
        NULL,
    };

    (void)state;
    configure_fake_workspace_lifecycle(&fake);
    observation = (application_preparation_observation){
        .fake = &fake,
        .expected_rate_hz = 48000,
    };
    active_observation = &observation;
    application_test_set_coreaudio_facade(&fake.facade);
    application_test_set_preparation_observer(
        observe_application_preparation);

    optind = 1;
    assert_int_equal(application_run(7, argv), 0);
    assert_true(fake.requested_format_set);
    assert_int_equal(fake.requested_format.sample_rate_hz, 0);
    assert_int_equal(fake.open_call_count, 1);
    assert_int_equal(fake.configure_call_count, 1);
    assert_int_equal(observation.preparation_call_count, 1);
    assert_int_equal(observation.prepared_rate_hz, 48000);
    assert_int_equal(observation.request_count_at_preparation, 0);
    assert_int_equal(fake.prepare_call_count, 1);
    assert_int_equal(fake.bind_request_call_count, 1);
    assert_int_equal(fake.start_call_count, 1);
    const fake_coreaudio_facade_lifecycle_call expected_lifecycle[] = {
        FAKE_COREAUDIO_FACADE_CALL_OPEN,
        FAKE_COREAUDIO_FACADE_CALL_CONFIGURE,
        FAKE_COREAUDIO_FACADE_CALL_PREPARE,
        FAKE_COREAUDIO_FACADE_CALL_BIND_REQUEST,
        FAKE_COREAUDIO_FACADE_CALL_START,
        FAKE_COREAUDIO_FACADE_CALL_STOP,
        FAKE_COREAUDIO_FACADE_CALL_QUIESCE,
        FAKE_COREAUDIO_FACADE_CALL_DISPOSE,
    };
    assert_lifecycle_trace(
        &fake, expected_lifecycle,
        sizeof(expected_lifecycle) / sizeof(expected_lifecycle[0]));
    assert_request_trace_truncated_at_capacity(&fake, 64, 1024);
    assert_int_equal(fake.control_stop_call_count, 1);
    assert_int_equal(fake.control_quiesce_call_count, 1);
    assert_int_equal(fake.control_dispose_call_count, 1);
    assert_false(fake.active);
    assert_true(fake.quiescent);
    assert_true(fake.disposed);
}

static void test_application_selected_position_outside_range_returns_one(
    void **state)
{
    fake_coreaudio_facade fake;
    char output[256] = {0};
    char *argv[] = {
        "synthtracker",
        "-p", "1",
        "-P", "4",
        TFMX_SOURCE_ROOT "/tests/fixtures/mdat.selected_01",
        TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
        NULL,
    };
    int status;

    (void)state;
    configure_fake_workspace_lifecycle(&fake);
    active_observation = NULL;
    application_test_set_preparation_observer(NULL);
    application_test_set_coreaudio_facade(&fake.facade);

    capture_run_stderr(7, argv, &status, output, sizeof(output));

    assert_int_equal(status, 1);
    assert_null(strstr(output, "Usage:"));
    assert_true(output[0] == '\0');
    assert_int_equal(fake.lifecycle_trace_count, 0);
    assert_int_equal(fake.open_call_count, 0);
    assert_int_equal(fake.start_call_count, 0);
    assert_int_equal(fake.render_request_count, 0);
}

static void test_application_malformed_selected_slot_returns_one(void **state)
{
    fake_coreaudio_facade fake;
    char output[256] = {0};
    char *argv[] = {
        "synthtracker",
        "-p", "1",
        TFMX_SOURCE_ROOT "/tests/fixtures/mdat.malformed_selected_slot_end_span",
        TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
        NULL,
    };
    int status;

    (void)state;
    configure_fake_workspace_lifecycle(&fake);
    application_test_set_coreaudio_facade(&fake.facade);

    capture_run_stderr(5, argv, &status, output, sizeof(output));

    assert_int_equal(status, 1);
    assert_null(strstr(output, "Usage:"));
    assert_true(output[0] == '\0');
    assert_int_equal(fake.lifecycle_trace_count, 0);
    assert_int_equal(fake.open_call_count, 0);
    assert_int_equal(fake.start_call_count, 0);
    assert_int_equal(fake.render_request_count, 0);
}

typedef struct {
    const char *option;
    const char *value;
} selection_argument;

static void test_application_rejects_invalid_selection_arguments(void **state)
{
    static const selection_argument cases[] = {
        { "-p", "bad" },
        { "-P", "bad" },
        { "-p", "-1" },
        { "-P", "-1" },
        { "-p", "999999999999999999999" },
        { "-P", "999999999999999999999" },
        { "-p", "32" },
        { "-p", "1x" },
        { "-P", "2x" },
        { "-p", "2147483648" },
        { "-P", "2147483648" },
    };
    const char *mdat = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8";
    const char *smpl = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8";

    (void)state;
    for (size_t index = 0;
         index < sizeof(cases) / sizeof(cases[0]); index++) {
        fake_coreaudio_facade fake;
        char output[256] = {0};
        char *argv[] = {
            "synthtracker",
            (char *)cases[index].option,
            (char *)cases[index].value,
            (char *)mdat,
            (char *)smpl,
            NULL,
        };
        int status;

        print_message("selection case %zu: %s %s\n", index,
                      cases[index].option, cases[index].value);
        reset_application_test_state();
        configure_fake_workspace_lifecycle(&fake);
        application_test_set_coreaudio_facade(&fake.facade);

        capture_run_stderr(5, argv, &status, output, sizeof(output));

        assert_int_equal(status, 2);
        assert_non_null(strstr(output, "Usage:"));
        assert_int_equal(fake.lifecycle_trace_count, 0);
        assert_int_equal(fake.open_call_count, 0);
        assert_int_equal(fake.start_call_count, 0);
        assert_int_equal(fake.render_request_count, 0);
        reset_application_test_state();
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_application_prepares_renderer_at_negotiated_rate,
            application_audio_lifecycle_setup,
            application_audio_lifecycle_teardown),
        cmocka_unit_test_setup_teardown(
            test_application_header_speed_finite_lifecycle,
            application_audio_lifecycle_setup,
            application_audio_lifecycle_teardown),
        cmocka_unit_test_setup_teardown(
            test_application_selected_subsong_absolute_lifecycle,
            application_audio_lifecycle_setup,
            application_audio_lifecycle_teardown),
        cmocka_unit_test_setup_teardown(
            test_application_selected_position_outside_range_returns_one,
            application_audio_lifecycle_setup,
            application_audio_lifecycle_teardown),
        cmocka_unit_test_setup_teardown(
            test_application_malformed_selected_slot_returns_one,
            application_audio_lifecycle_setup,
            application_audio_lifecycle_teardown),
        cmocka_unit_test_setup_teardown(
            test_application_rejects_invalid_selection_arguments,
            application_audio_lifecycle_setup,
            application_audio_lifecycle_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
