#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "../../src/playback/playback_context.h"
#include "../../src/playback/tfmx_loader.h"

static void test_loader_normalizes_fixture_tables(void **state)
{
    tfmx_loader_candidate candidate;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(candidate.metadata.trackstart, 0x230);
    assert_int_equal(candidate.metadata.first_pattern, 0x250);
    assert_int_equal(candidate.metadata.pattern_count, 1);
    assert_int_equal(candidate.metadata.macro_count, 1);
    assert_int_equal(candidate.metadata.patterns[0], (0x250 - 0x200) / 4);
    assert_int_equal(candidate.metadata.macros[0], (0x260 - 0x200) / 4);
    tfmx_loader_candidate_dispose(&candidate);
}

static void test_playback_context_create_destroy(void **state)
{
    (void)state;

    tfmx_playback_context *context = tfmx_playback_context_create();

    assert_non_null(context);
    assert_false(tfmx_playback_context_is_loaded(context));
    tfmx_playback_context_destroy(context);
    tfmx_playback_context_destroy(NULL);
}

static void test_playback_context_loads_separate_fixture_files(void **state)
{
    (void)state;

    tfmx_playback_context *context = tfmx_playback_context_create();
    char mdat_path[512];
    char smpl_path[512];

    assert_non_null(context);
    assert_true(snprintf(mdat_path, sizeof(mdat_path), "%s/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT) > 0);
    assert_true(snprintf(smpl_path, sizeof(smpl_path), "%s/tests/fixtures/smpl.step8",
                         TFMX_SOURCE_ROOT) > 0);
    assert_int_equal(tfmx_playback_context_load(context, mdat_path, smpl_path),
                     TFMX_LOAD_SUCCESS);

    tfmx_playback_context_destroy(context);
}

static void test_playback_context_rejects_invalid_or_missing_paths(void **state)
{
    (void)state;

    tfmx_playback_context *context = tfmx_playback_context_create();

    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_load(NULL, "mdat", "smpl"),
                     TFMX_LOAD_INVALID_ARGUMENT);
    assert_int_equal(tfmx_playback_context_load(context, NULL, "smpl"),
                     TFMX_LOAD_INVALID_ARGUMENT);
    assert_int_equal(tfmx_playback_context_load(context, "missing-mdat", "missing-smpl"),
                     TFMX_LOAD_IO_ERROR);

    tfmx_playback_context_destroy(context);
}

static void test_playback_context_rejects_malformed_data_transactionally(void **state)
{
    (void)state;

    tfmx_playback_context *context = tfmx_playback_context_create();
    char mdat_path[512];
    char smpl_path[512];

    assert_non_null(context);
    assert_true(snprintf(mdat_path, sizeof(mdat_path), "%s/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT) > 0);
    assert_true(snprintf(smpl_path, sizeof(smpl_path), "%s/tests/fixtures/smpl.step8",
                         TFMX_SOURCE_ROOT) > 0);
    assert_int_equal(tfmx_playback_context_load(context, mdat_path, smpl_path),
                     TFMX_LOAD_SUCCESS);
    assert_true(tfmx_playback_context_is_loaded(context));

    assert_int_equal(tfmx_playback_context_load(context, "/dev/null", smpl_path),
                     TFMX_LOAD_INVALID_FORMAT);
    assert_true(tfmx_playback_context_is_loaded(context));

    tfmx_playback_context_destroy(context);
}

static int copy_fixture_with_byte_changed(const char *source_path,
                                          unsigned int offset,
                                          unsigned char value,
                                          char *output_path,
                                          size_t output_path_size)
{
    FILE *source;
    FILE *output;
    long length;
    unsigned char *data;
    int descriptor;
    int result = -1;

    source = fopen(source_path, "rb");
    if (source == NULL || fseek(source, 0, SEEK_END) != 0 ||
        (length = ftell(source)) < 0 || fseek(source, 0, SEEK_SET) != 0) {
        if (source != NULL) {
            fclose(source);
        }
        return -1;
    }
    data = malloc((size_t)length);
    if (data == NULL || fread(data, 1, (size_t)length, source) != (size_t)length ||
        offset >= (unsigned int)length) {
        free(data);
        fclose(source);
        return -1;
    }
    fclose(source);
    data[offset] = value;

    assert_true(snprintf(output_path, output_path_size, "/tmp/tfmx-malformed-XXXXXX") > 0);
    descriptor = mkstemp(output_path);
    if (descriptor < 0) {
        free(data);
        return -1;
    }
    output = fdopen(descriptor, "wb");
    if (output != NULL && fwrite(data, 1, (size_t)length, output) == (size_t)length &&
        fclose(output) == 0) {
        result = 0;
    } else {
        if (output != NULL) {
            fclose(output);
        } else {
            close(descriptor);
        }
        unlink(output_path);
    }
    free(data);
    return result;
}

static void test_playback_context_rejects_malformed_documented_structure(void **state)
{
    const char *fixture_path = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8";
    const char *smpl_path = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8";
    tfmx_playback_context *context = tfmx_playback_context_create();
    char malformed_path[128];

    (void)state;
    assert_non_null(context);

    assert_int_equal(copy_fixture_with_byte_changed(fixture_path, 0x231, 0x01,
                                                    malformed_path, sizeof(malformed_path)),
                     0);
    assert_int_equal(tfmx_playback_context_load(context, malformed_path, smpl_path),
                     TFMX_LOAD_INVALID_FORMAT);
    unlink(malformed_path);

    assert_int_equal(copy_fixture_with_byte_changed(fixture_path, 0x250, 0x81,
                                                    malformed_path, sizeof(malformed_path)),
                     0);
    assert_int_equal(tfmx_playback_context_load(context, malformed_path, smpl_path),
                     TFMX_LOAD_INVALID_FORMAT);
    unlink(malformed_path);

    tfmx_playback_context_destroy(context);
}

static void test_playback_context_rejects_invalid_fixture_semantics(void **state)
{
    const char *fixture_path = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8";
    const char *smpl_path = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8";
    tfmx_playback_context *context = tfmx_playback_context_create();
    char mutated_path[128];
    char reordered_path[128];
    unsigned int inactive_channel;

    (void)state;
    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_load(context, fixture_path, smpl_path),
                     TFMX_LOAD_SUCCESS);

    /* Every inactive voice must retain its distinct disabled-channel binding. */
    for (inactive_channel = 1; inactive_channel < 8; ++inactive_channel) {
        assert_int_equal(copy_fixture_with_byte_changed(
                             fixture_path, 0x230 + inactive_channel * 2 + 1,
                             0x00, mutated_path, sizeof(mutated_path)),
                         0);
        assert_int_equal(tfmx_playback_context_load(context, mutated_path, smpl_path),
                         TFMX_LOAD_INVALID_FORMAT);
        assert_true(tfmx_playback_context_is_loaded(context));
        unlink(mutated_path);
    }

    /* The pitch command must precede sample setup in the macro. */
    assert_int_equal(copy_fixture_with_byte_changed(fixture_path, 0x260, 0x02,
                                                    mutated_path, sizeof(mutated_path)),
                     0);
    assert_int_equal(tfmx_playback_context_load(context, mutated_path, smpl_path),
                     TFMX_LOAD_INVALID_FORMAT);
    assert_true(tfmx_playback_context_is_loaded(context));
    unlink(mutated_path);

    assert_int_equal(copy_fixture_with_byte_changed(fixture_path, 0x260, 0x02,
                                                    mutated_path, sizeof(mutated_path)),
                     0);
    assert_int_equal(copy_fixture_with_byte_changed(mutated_path, 0x264, 0x09,
                                                    reordered_path, sizeof(reordered_path)),
                     0);
    assert_int_equal(tfmx_playback_context_load(context, reordered_path, smpl_path),
                     TFMX_LOAD_INVALID_FORMAT);
    assert_true(tfmx_playback_context_is_loaded(context));
    unlink(mutated_path);
    unlink(reordered_path);

    /* Reject a sample length that directly exceeds the two-byte SMPL payload. */
    assert_int_equal(copy_fixture_with_byte_changed(fixture_path, 0x26b, 0x03,
                                                    mutated_path, sizeof(mutated_path)),
                     0);
    assert_int_equal(tfmx_playback_context_load(context, mutated_path, smpl_path),
                     TFMX_LOAD_INVALID_FORMAT);
    assert_true(tfmx_playback_context_is_loaded(context));
    unlink(mutated_path);

    /* The pattern wait must cover the complete seven-word macro. */
    assert_int_equal(copy_fixture_with_byte_changed(fixture_path, 0x257, 0x01,
                                                    mutated_path, sizeof(mutated_path)),
                     0);
    assert_int_equal(tfmx_playback_context_load(context, mutated_path, smpl_path),
                     TFMX_LOAD_INVALID_FORMAT);
    assert_true(tfmx_playback_context_is_loaded(context));
    unlink(mutated_path);

    tfmx_playback_context_destroy(context);
}

static void test_playback_context_starts_loaded_subsong_zero(void **state)
{
    tfmx_playback_context *context = tfmx_playback_context_create();
    const char *mdat_path = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8";
    const char *smpl_path = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8";

    (void)state;
    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_load(context, mdat_path, smpl_path),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_start_rejects_invalid_state_or_subsong(void **state)
{
    tfmx_playback_context *context = tfmx_playback_context_create();
    const char *mdat_path = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8";
    const char *smpl_path = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8";

    (void)state;
    assert_int_equal(tfmx_playback_context_start(NULL, 0),
                     TFMX_START_INVALID_ARGUMENT);
    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_start(context, 0),
                     TFMX_START_NOT_LOADED);
    assert_int_equal(tfmx_playback_context_load(context, mdat_path, smpl_path),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(context, 1),
                     TFMX_START_UNSUPPORTED_SUBSONG);
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_tick_and_snapshot_trace(void **state)
{
    static const tfmx_voice_snapshot expected[] = {
        { 0, 0, 0 },
        { 0, 0x06AE, 0 },
        { 1, 0x06AE, 15 }, { 1, 0x06AE, 15 },
        { 0, 0x06AE, 15 }
    };
    tfmx_playback_context *context = tfmx_playback_context_create();
    tfmx_voice_snapshot snapshot;
    unsigned int tick;

    (void)state;
    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_load(
                         context, TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8"),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);
    for (tick = 0; tick < sizeof(expected) / sizeof(expected[0]); ++tick) {
        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
        assert_int_equal(tfmx_playback_context_snapshot(context, 0, &snapshot),
                         TFMX_SNAPSHOT_SUCCESS);
        assert_int_equal(snapshot.active, expected[tick].active);
        assert_int_equal(snapshot.pitch, expected[tick].pitch);
        assert_int_equal(snapshot.volume, expected[tick].volume);
    }
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_tick_and_snapshot_reject_invalid_state(void **state)
{
    tfmx_playback_context *context = tfmx_playback_context_create();
    tfmx_voice_snapshot snapshot;

    (void)state;
    assert_int_equal(tfmx_playback_context_tick(NULL), TFMX_TICK_INVALID_ARGUMENT);
    assert_int_equal(tfmx_playback_context_snapshot(NULL, 0, &snapshot),
                     TFMX_SNAPSHOT_INVALID_ARGUMENT);
    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_NOT_STARTED);
    assert_int_equal(tfmx_playback_context_snapshot(context, 0, &snapshot),
                     TFMX_SNAPSHOT_NOT_STARTED);
    assert_int_equal(tfmx_playback_context_snapshot(context, 1, &snapshot),
                     TFMX_SNAPSHOT_UNSUPPORTED_VOICE);
    assert_int_equal(tfmx_playback_context_snapshot(context, 0, NULL),
                     TFMX_SNAPSHOT_INVALID_ARGUMENT);
    tfmx_playback_context_destroy(context);
}

static tfmx_playback_context *start_step8_fixture(void);

static void assert_snapshot_set_zero(const tfmx_voice_snapshot_set *snapshot)
{
    for (unsigned int voice = 0; voice < TFMX_PLAYBACK_SNAPSHOT_VOICE_COUNT; ++voice) {
        assert_int_equal(snapshot->voice[voice].active, 0);
        assert_int_equal(snapshot->voice[voice].pitch, 0);
        assert_int_equal(snapshot->voice[voice].volume, 0);
    }
}

static void test_playback_context_snapshot_all_rejects_invalid_or_unstarted_without_writes(void **state)
{
    tfmx_playback_context *context = tfmx_playback_context_create();
    tfmx_voice_snapshot_set snapshot;
    tfmx_voice_snapshot_set sentinel;

    (void)state;
    for (unsigned int voice = 0; voice < TFMX_PLAYBACK_SNAPSHOT_VOICE_COUNT; ++voice) {
        sentinel.voice[voice] = (tfmx_voice_snapshot){ 7, 0x1234, 9 };
    }
    snapshot = sentinel;
    assert_int_equal(tfmx_playback_context_snapshot_all(NULL, &snapshot),
                     TFMX_SNAPSHOT_INVALID_ARGUMENT);
    assert_memory_equal(&snapshot, &sentinel, sizeof(snapshot));
    assert_int_equal(tfmx_playback_context_snapshot_all(context, NULL),
                     TFMX_SNAPSHOT_INVALID_ARGUMENT);
    snapshot = sentinel;
    assert_int_equal(tfmx_playback_context_snapshot_all(context, &snapshot),
                     TFMX_SNAPSHOT_NOT_STARTED);
    assert_memory_equal(&snapshot, &sentinel, sizeof(snapshot));
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_snapshot_all_caches_same_tick_values(void **state)
{
    tfmx_playback_context *context = start_step8_fixture();
    tfmx_voice_snapshot_set all_snapshot;
    tfmx_voice_snapshot voice_zero;
    tfmx_voice_snapshot_set stable_snapshot;

    (void)state;
    assert_int_equal(tfmx_playback_context_snapshot_all(context, &all_snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_snapshot_set_zero(&all_snapshot);
    assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    assert_int_equal(tfmx_playback_context_snapshot(context, 0, &voice_zero),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_int_equal(tfmx_playback_context_snapshot_all(context, &all_snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_memory_equal(&all_snapshot.voice[0], &voice_zero, sizeof(voice_zero));
    stable_snapshot = all_snapshot;
    assert_int_equal(tfmx_playback_context_snapshot_all(context, &all_snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_memory_equal(&all_snapshot, &stable_snapshot, sizeof(all_snapshot));
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_snapshot_all_resets_on_start_and_reload(void **state)
{
    const char *mdat_path = TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8";
    const char *smpl_path = TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8";
    tfmx_playback_context *context = start_step8_fixture();
    tfmx_voice_snapshot_set snapshot;

    (void)state;
    for (unsigned int tick = 0; tick < 3; ++tick) {
        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    }
    assert_int_equal(tfmx_playback_context_snapshot_all(context, &snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_true(snapshot.voice[0].active != 0);
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);
    assert_int_equal(tfmx_playback_context_snapshot_all(context, &snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_snapshot_set_zero(&snapshot);
    assert_int_equal(tfmx_playback_context_load(context, mdat_path, smpl_path),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);
    assert_int_equal(tfmx_playback_context_snapshot_all(context, &snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_snapshot_set_zero(&snapshot);
    tfmx_playback_context_destroy(context);
}

static tfmx_playback_context *start_step8_fixture(void)
{
    tfmx_playback_context *context = tfmx_playback_context_create();

    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_load(
                         context, TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8"),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);
    return context;
}

static void test_playback_context_renders_silent_first_two_ticks(void **state)
{
    unsigned char output[3528];
    size_t bytes;
    unsigned int tick;
    tfmx_playback_context *context;

    (void)state;
    context = start_step8_fixture();
    for (tick = 0; tick < 2; ++tick) {
        memset(output, 0xA5, sizeof(output));
        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
        assert_int_equal(tfmx_playback_context_render(context, output,
                                                       sizeof(output), &bytes),
                         TFMX_RENDER_SUCCESS);
        assert_true(bytes == 881 * 4 || bytes == 882 * 4);
        for (size_t index = 0; index < bytes; ++index) {
            assert_int_equal(output[index], 0);
        }
    }
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_renders_completed_tick_as_canonical_pcm(void **state)
{
    unsigned char output[3528];
    size_t bytes = 0;
    int nonzero = 0;
    tfmx_playback_context *context;

    (void)state;
    context = start_step8_fixture();
    for (unsigned int tick = 0; tick < 3; ++tick) {
        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    }
    memset(output, 0, sizeof(output));
    assert_int_equal(tfmx_playback_context_render(context, output, sizeof(output),
                                                   &bytes),
                     TFMX_RENDER_SUCCESS);
    assert_int_equal(bytes, 882 * 4);
    for (size_t index = 0; index < bytes; index += 2) {
        short sample = (short)(output[index] | ((unsigned short)output[index + 1] << 8));
        if (sample != 0) {
            nonzero = 1;
        }
    }
    assert_true(nonzero);
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_render_rejects_invalid_arguments_and_capacity(void **state)
{
    unsigned char output[3528];
    size_t bytes;
    tfmx_playback_context *context;

    (void)state;
    assert_int_equal(tfmx_playback_context_render(NULL, output, sizeof(output), &bytes),
                     TFMX_RENDER_INVALID_ARGUMENT);
    context = start_step8_fixture();
    assert_int_equal(tfmx_playback_context_render(context, NULL, sizeof(output), &bytes),
                     TFMX_RENDER_INVALID_ARGUMENT);
    assert_int_equal(tfmx_playback_context_render(context, output, 0, &bytes),
                     TFMX_RENDER_NOT_STARTED);
    assert_int_equal(tfmx_playback_context_render(context, output, sizeof(output), NULL),
                     TFMX_RENDER_INVALID_ARGUMENT);
    assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    assert_int_equal(tfmx_playback_context_render(context, output, 4, &bytes),
                     TFMX_RENDER_INSUFFICIENT_CAPACITY);
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_reports_engine_completion(void **state)
{
    tfmx_playback_context *context;
    tfmx_voice_snapshot snapshot;

    (void)state;
    context = start_step8_fixture();
    for (unsigned int tick = 0; tick < 29; ++tick) {
        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    }
    assert_true(tfmx_playback_context_is_complete(context));
    assert_int_equal(tfmx_playback_context_snapshot(context, 0, &snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_false(snapshot.active);
    assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    assert_int_equal(tfmx_playback_context_snapshot(context, 0, &snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_false(snapshot.active);
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_plays_finite_pattern_loop_to_completion(void **state)
{
    unsigned char output[3528];
    size_t bytes;
    tfmx_playback_context *context;
    tfmx_voice_snapshot snapshot;
    unsigned int repeated_events = 0;
    unsigned int non_silent_renders = 0;
    unsigned int tick;

    (void)state;
    context = tfmx_playback_context_create();
    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_load(
                         context, TFMX_SOURCE_ROOT "/tests/fixtures/mdat.loop_f1",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.loop_f1"),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);

    for (tick = 0; tick < 128 && !tfmx_playback_context_is_complete(context);
         ++tick) {
        int nonzero = 0;

        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
        assert_int_equal(tfmx_playback_context_snapshot(context, 0, &snapshot),
                         TFMX_SNAPSHOT_SUCCESS);
        if (snapshot.active != 0 && snapshot.pitch != 0 && snapshot.volume != 0) {
            ++repeated_events;
        }
        memset(output, 0, sizeof(output));
        assert_int_equal(tfmx_playback_context_render(context, output, sizeof(output),
                                                       &bytes),
                         TFMX_RENDER_SUCCESS);
        for (size_t index = 0; index < bytes; ++index) {
            if (output[index] != 0) {
                nonzero = 1;
                break;
            }
        }
        if (nonzero) {
            ++non_silent_renders;
        }
    }

    assert_int_equal(repeated_events, 6);
    assert_true(non_silent_renders > 0);
    assert_true(tfmx_playback_context_is_complete(context));
    assert_true(tick < 128);
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_applies_envelope_on_engine_ticks_with_tempo_prescale(void **state)
{
    static const unsigned char expected_volume[] = {
        0, 0, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 12, 12, 9, 9, 6
    };
    tfmx_playback_context *context;
    tfmx_voice_snapshot snapshot;
    unsigned char output[3528];
    size_t bytes = 0;
    unsigned int tick;
    unsigned int non_silent_renders = 0;

    (void)state;
    context = tfmx_playback_context_create();
    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_load(
                         context, TFMX_SOURCE_ROOT "/tests/fixtures/mdat.envelope_tempo",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.envelope_tempo"),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);

    for (tick = 0; tick < 64 && !tfmx_playback_context_is_complete(context); ++tick) {
        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
        assert_int_equal(tfmx_playback_context_snapshot(context, 0, &snapshot),
                         TFMX_SNAPSHOT_SUCCESS);
        assert_int_equal(snapshot.active, tick < 2 ? 0 : 1);
        assert_int_equal(snapshot.pitch, tick == 0 ? 0 : 0x06AE);
        assert_int_equal(snapshot.volume, expected_volume[tick]);
        memset(output, 0, sizeof(output));
        assert_int_equal(tfmx_playback_context_render(context, output, sizeof(output), &bytes),
                         TFMX_RENDER_SUCCESS);
        for (size_t index = 0; index < bytes; ++index) {
            if (output[index] != 0) {
                ++non_silent_renders;
                break;
            }
        }
    }

    assert_true(non_silent_renders > 0);
    assert_true(tfmx_playback_context_is_complete(context));
    assert_true(tick < 64);
    assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    assert_true(tfmx_playback_context_is_complete(context));
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_clean_start_clears_prior_pitch_state(void **state)
{
    tfmx_playback_context *context;
    tfmx_voice_snapshot snapshot;

    (void)state;
    context = tfmx_playback_context_create();
    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_load(
                         context, TFMX_SOURCE_ROOT "/tests/fixtures/mdat.envelope_tempo",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.envelope_tempo"),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);
    for (unsigned int tick = 0; tick < 14; ++tick) {
        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    }
    assert_int_equal(tfmx_playback_context_snapshot(context, 0, &snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_int_equal(snapshot.active, 1);
    assert_int_equal(snapshot.pitch, 0x06AE);
    assert_int_equal(snapshot.volume, 15);
    tfmx_playback_context_destroy(context);

    context = start_step8_fixture();
    assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    assert_int_equal(tfmx_playback_context_snapshot(context, 0, &snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_int_equal(snapshot.active, 0);
    assert_int_equal(snapshot.pitch, 0);
    assert_int_equal(snapshot.volume, 0);
    tfmx_playback_context_destroy(context);
}

static void test_playback_context_plays_independent_voice_zero_and_one_fixture(void **state)
{
    unsigned char output[3528];
    size_t bytes = 0;
    tfmx_playback_context *context;
    tfmx_voice_snapshot_set snapshot;
    unsigned int tick;
    int found_jointly_active = 0;

    (void)state;
    context = tfmx_playback_context_create();
    assert_non_null(context);
    assert_int_equal(tfmx_playback_context_load(
                         context, TFMX_SOURCE_ROOT "/tests/fixtures/mdat.voices_01",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.voices_01"),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);

    for (tick = 0; tick < 128 && !tfmx_playback_context_is_complete(context); ++tick) {
        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
        assert_int_equal(tfmx_playback_context_snapshot_all(context, &snapshot),
                         TFMX_SNAPSHOT_SUCCESS);
        if (snapshot.voice[0].active && snapshot.voice[1].active) {
            found_jointly_active = 1;
            assert_int_equal(snapshot.voice[0].pitch, 0x06AE);
            assert_int_equal(snapshot.voice[0].volume, 18);
            assert_int_equal(snapshot.voice[1].pitch, 0x064E);
            assert_int_equal(snapshot.voice[1].volume, 30);
            for (unsigned int voice = 2; voice < 8; ++voice) {
                assert_false(snapshot.voice[voice].active);
            }
            memset(output, 0, sizeof(output));
            assert_int_equal(tfmx_playback_context_render(context, output, sizeof(output), &bytes),
                             TFMX_RENDER_SUCCESS);
            assert_true(bytes == 881 * 4 || bytes == 882 * 4);
            {
                short left = (short)(output[0] | ((unsigned short)output[1] << 8));
                short right = (short)(output[2] | ((unsigned short)output[3] << 8));
                assert_true(left != 0 || right != 0);
                assert_true(left > right);
            }
            break;
        }
    }
    assert_true(found_jointly_active);

    while (tick < 128 && !tfmx_playback_context_is_complete(context)) {
        assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
        ++tick;
    }
    assert_true(tick < 128);
    assert_int_equal(tfmx_playback_context_snapshot_all(context, &snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_false(snapshot.voice[0].active);
    assert_false(snapshot.voice[1].active);
    tfmx_playback_context_destroy(context);
}

static void assert_malformed_pair_preserves_step8(const char *case_name,
                                                  const char *smpl_name)
{
    char mdat_path[512];
    char smpl_path[512];
    tfmx_voice_snapshot snapshot;
    tfmx_playback_context *context = tfmx_playback_context_create();

    assert_non_null(context);
    assert_true(snprintf(mdat_path, sizeof(mdat_path),
                         "%s/tests/fixtures/mdat.malformed_%s",
                         TFMX_SOURCE_ROOT, case_name) > 0);
    assert_true(snprintf(smpl_path, sizeof(smpl_path),
                         "%s/tests/fixtures/smpl.malformed_%s",
                         TFMX_SOURCE_ROOT, smpl_name) > 0);
    assert_int_equal(tfmx_playback_context_load(
                         context, TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8"),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(tfmx_playback_context_load(context, mdat_path, smpl_path),
                     TFMX_LOAD_INVALID_FORMAT);
    assert_true(tfmx_playback_context_is_loaded(context));
    assert_int_equal(tfmx_playback_context_start(context, 0), TFMX_START_SUCCESS);
    assert_int_equal(tfmx_playback_context_tick(context), TFMX_TICK_SUCCESS);
    assert_int_equal(tfmx_playback_context_snapshot(context, 0, &snapshot),
                     TFMX_SNAPSHOT_SUCCESS);
    assert_int_equal(snapshot.active, 0);
    assert_int_equal(snapshot.pitch, 0);
    assert_int_equal(snapshot.volume, 0);
    tfmx_playback_context_destroy(context);
}

#define MALFORMED_CASE_TEST(function_name, case_name)                         \
    static void function_name(void **state)                                    \
    {                                                                           \
        (void)state;                                                           \
        assert_malformed_pair_preserves_step8(case_name, case_name);           \
    }

MALFORMED_CASE_TEST(test_malformed_truncated_mdat, "truncated_mdat")
MALFORMED_CASE_TEST(test_malformed_unaligned_track, "unaligned_track")
MALFORMED_CASE_TEST(test_malformed_out_of_range_pattern, "out_of_range_pattern")
MALFORMED_CASE_TEST(test_malformed_invalid_active_binding, "invalid_active_binding")
MALFORMED_CASE_TEST(test_malformed_invalid_inactive_binding, "invalid_inactive_binding")
MALFORMED_CASE_TEST(test_malformed_invalid_stop_step, "invalid_stop_step")
MALFORMED_CASE_TEST(test_malformed_invalid_pattern_contract, "invalid_pattern_contract")
MALFORMED_CASE_TEST(test_malformed_invalid_macro_ordering, "invalid_macro_ordering")
MALFORMED_CASE_TEST(test_malformed_sample_range_overflow, "sample_range_overflow")
MALFORMED_CASE_TEST(test_malformed_silent_sample_payload, "silent_sample_payload")

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_playback_context_create_destroy),
        cmocka_unit_test(test_loader_normalizes_fixture_tables),
        cmocka_unit_test(test_playback_context_loads_separate_fixture_files),
        cmocka_unit_test(test_playback_context_rejects_invalid_or_missing_paths),
        cmocka_unit_test(test_playback_context_rejects_malformed_data_transactionally),
        cmocka_unit_test(test_playback_context_rejects_malformed_documented_structure),
        cmocka_unit_test(test_playback_context_rejects_invalid_fixture_semantics),
        cmocka_unit_test(test_playback_context_starts_loaded_subsong_zero),
        cmocka_unit_test(test_playback_context_start_rejects_invalid_state_or_subsong),
        cmocka_unit_test(test_playback_context_tick_and_snapshot_trace),
        cmocka_unit_test(test_playback_context_tick_and_snapshot_reject_invalid_state),
        cmocka_unit_test(test_playback_context_snapshot_all_rejects_invalid_or_unstarted_without_writes),
        cmocka_unit_test(test_playback_context_snapshot_all_caches_same_tick_values),
        cmocka_unit_test(test_playback_context_snapshot_all_resets_on_start_and_reload),
        cmocka_unit_test(test_playback_context_renders_silent_first_two_ticks),
        cmocka_unit_test(test_playback_context_renders_completed_tick_as_canonical_pcm),
        cmocka_unit_test(test_playback_context_render_rejects_invalid_arguments_and_capacity),
        cmocka_unit_test(test_playback_context_reports_engine_completion),
        cmocka_unit_test(test_playback_context_plays_finite_pattern_loop_to_completion),
        cmocka_unit_test(test_playback_context_applies_envelope_on_engine_ticks_with_tempo_prescale),
        cmocka_unit_test(test_playback_context_clean_start_clears_prior_pitch_state),
        cmocka_unit_test(test_playback_context_plays_independent_voice_zero_and_one_fixture),
        cmocka_unit_test(test_malformed_truncated_mdat),
        cmocka_unit_test(test_malformed_unaligned_track),
        cmocka_unit_test(test_malformed_out_of_range_pattern),
        cmocka_unit_test(test_malformed_invalid_active_binding),
        cmocka_unit_test(test_malformed_invalid_inactive_binding),
        cmocka_unit_test(test_malformed_invalid_stop_step),
        cmocka_unit_test(test_malformed_invalid_pattern_contract),
        cmocka_unit_test(test_malformed_invalid_macro_ordering),
        cmocka_unit_test(test_malformed_sample_range_overflow),
        cmocka_unit_test(test_malformed_silent_sample_payload),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
