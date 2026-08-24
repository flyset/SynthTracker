#include <setjmp.h>
#include <stdarg.h>
#include <unistd.h>

#include <cmocka.h>

#include "application.h"
#include "fake_coreaudio_facade.h"

typedef struct {
    fake_coreaudio_facade *fake;
    size_t preparation_call_count;
    unsigned int prepared_rate_hz;
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
    assert_int_equal(format->sample_rate_hz, 48000);
    observation->request_count_at_preparation =
        observation->fake->render_request_count;
    observation->prepared_rate_hz = renderer_rate_hz;
    observation->preparation_call_count++;
    fake_coreaudio_facade_record_preparation(observation->fake);
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
    observation = (application_preparation_observation){ .fake = &fake };
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
    assert_int_equal(fake.render_request_count, 64);
    assert_int_equal(fake.control_stop_call_count, 1);
    assert_int_equal(fake.control_quiesce_call_count, 1);
    assert_int_equal(fake.control_dispose_call_count, 1);
    assert_false(fake.active);
    assert_true(fake.quiescent);
    assert_true(fake.disposed);

    application_test_set_preparation_observer(NULL);
    application_test_set_coreaudio_facade(NULL);
    active_observation = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(
            test_application_prepares_renderer_at_negotiated_rate),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
