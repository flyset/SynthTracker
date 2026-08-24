#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "../../src/playback/playback_context.h"
#include "../../src/playback/playback_legacy_bridge.h"
#include "../../src/playback/tfmx_loader.h"

extern unsigned int editbuf[];

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
    assert_int_equal(candidate.metadata.macro_count, 2);
    assert_int_equal(candidate.metadata.patterns[0], (0x250 - 0x200) / 4);
    assert_int_equal(candidate.metadata.macros[0], (0x260 - 0x200) / 4);
    assert_int_equal(candidate.metadata.macros[1], (0x250 - 0x200) / 4);
    tfmx_loader_candidate_dispose(&candidate);
}

static void test_loader_resolves_default_header_pointers(void **state)
{
    tfmx_loader_candidate candidate;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.default_pointers",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(candidate.metadata.trackstart, 0x800);
    assert_int_equal(candidate.metadata.pattstart, 0x400);
    assert_int_equal(candidate.metadata.macrostart, 0x600);
    assert_int_equal(candidate.metadata.pattern_count, 1);
    assert_int_equal(candidate.metadata.macro_count, 1);
    assert_int_equal(candidate.metadata.patterns[0], (0x820 - 0x200) / 4);
    assert_int_equal(candidate.metadata.macros[0], (0x830 - 0x200) / 4);
    assert_int_equal(candidate.metadata.first_pattern, 0x820);
    tfmx_loader_candidate_dispose(&candidate);
    assert_null(candidate.mdat);
    assert_null(candidate.smpl);
}

static void test_loader_scans_independent_tables_with_bounded_capacity(void **state)
{
    tfmx_loader_candidate candidate;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.table_scan_2p3m",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(candidate.metadata.pattern_count, 2);
    assert_int_equal(candidate.metadata.macro_count, 3);
    assert_int_equal(candidate.metadata.patterns[0], (0x260 - 0x200) / 4);
    assert_int_equal(candidate.metadata.patterns[1], (0x270 - 0x200) / 4);
    assert_int_equal(candidate.metadata.macros[0], (0x280 - 0x200) / 4);
    assert_int_equal(candidate.metadata.macros[1], (0x290 - 0x200) / 4);
    assert_int_equal(candidate.metadata.macros[2], (0x2a0 - 0x200) / 4);
    assert_int_equal(candidate.metadata.first_pattern, 0x260);
    tfmx_loader_candidate_dispose(&candidate);

    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.table_scan_cap128",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(candidate.metadata.pattern_count, 1);
    assert_int_equal(candidate.metadata.macro_count, 128);
    assert_int_equal(candidate.metadata.patterns[0], (0x260 - 0x200) / 4);
    assert_int_equal(candidate.metadata.macros[0], (0x520 - 0x200) / 4);
    assert_int_equal(candidate.metadata.macros[127], (0x71c - 0x200) / 4);
    tfmx_loader_candidate_dispose(&candidate);
}

static void test_loader_admits_leading_zero_smpl_without_header_rule(void **state)
{
    static const unsigned char expected_smpl[] = { 0x00, 0x00 };
    tfmx_loader_candidate candidate;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.raw_leading_zero",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    assert_int_equal(candidate.smpl_size, sizeof(expected_smpl));
    assert_memory_equal(candidate.smpl, expected_smpl, sizeof(expected_smpl));
    tfmx_loader_candidate_dispose(&candidate);
    assert_null(candidate.mdat);
    assert_null(candidate.smpl);
}

static void test_loader_treats_smpl_as_opaque_without_macro_range_inference(
    void **state)
{
    tfmx_loader_candidate candidate;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.raw_smpl_opaque",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    tfmx_loader_candidate_dispose(&candidate);
    assert_null(candidate.mdat);
    assert_null(candidate.smpl);
}

static void test_loader_rejects_one_byte_smpl(void **state)
{
    tfmx_loader_candidate candidate;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.table_scan_2p3m",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.raw_one_byte",
                         &candidate),
                     TFMX_LOAD_INVALID_FORMAT);
    tfmx_loader_candidate_dispose(&candidate);
    assert_null(candidate.mdat);
    assert_null(candidate.smpl);
}

static void test_legacy_bridge_owns_voices_01_tables_and_rejects_out_of_range_metadata(
    void **state)
{
    tfmx_loader_candidate candidate;
    struct tfmx_loader_metadata metadata;
    tfmx_voice_snapshot snapshots[TFMX_PLAYBACK_SNAPSHOT_VOICE_COUNT];
    size_t copied_words;
    size_t macro_table_slot;
    unsigned int tick;
    int found_jointly_active = 0;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.voices_01",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.voices_01",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    metadata = candidate.metadata;
    copied_words = (candidate.mdat_size - 0x200) / sizeof(editbuf[0]);
    macro_table_slot = (metadata.macrostart - 0x200) / sizeof(editbuf[0]);

    assert_int_equal(tfmx_playback_legacy_bridge_start(
                         candidate.mdat, candidate.mdat_size, candidate.smpl,
                         candidate.smpl_size, &metadata, 0),
                     1);
    /* Mutate the copied on-disk macro table after bridge start. */
    editbuf[macro_table_slot] = (unsigned int)metadata.macros[1];
    for (tick = 0; tick < 8; ++tick) {
        assert_int_equal(tfmx_playback_legacy_bridge_tick(snapshots), 1);
        if (snapshots[0].active && snapshots[1].active) {
            found_jointly_active = 1;
            assert_int_equal(snapshots[0].volume, 18);
            break;
        }
    }
    assert_true(found_jointly_active);
    tfmx_playback_legacy_bridge_reset();

    metadata = candidate.metadata;
    metadata.patterns[0] = (int)copied_words;
    assert_int_equal(tfmx_playback_legacy_bridge_start(
                         candidate.mdat, candidate.mdat_size, candidate.smpl,
                         candidate.smpl_size, &metadata, 0),
                     0);
    tfmx_playback_legacy_bridge_reset();

    metadata = candidate.metadata;
    metadata.macros[0] = (int)copied_words;
    assert_int_equal(tfmx_playback_legacy_bridge_start(
                         candidate.mdat, candidate.mdat_size, candidate.smpl,
                         candidate.smpl_size, &metadata, 0),
                     0);
    tfmx_playback_legacy_bridge_reset();
    tfmx_loader_candidate_dispose(&candidate);
}

static int reduced_voices_01_tables_activate_voice_one(
    const tfmx_loader_candidate *candidate,
    const struct tfmx_loader_metadata *metadata)
{
    tfmx_voice_snapshot snapshots[TFMX_PLAYBACK_SNAPSHOT_VOICE_COUNT];
    int found_voice_zero_active = 0;
    int found_voice_one_active = 0;

    assert_int_equal(tfmx_playback_legacy_bridge_start(
                         candidate->mdat, candidate->mdat_size, candidate->smpl,
                         candidate->smpl_size, metadata, 0),
                     1);
    for (unsigned int tick = 0; tick < 8; ++tick) {
        assert_int_equal(tfmx_playback_legacy_bridge_tick(snapshots), 1);
        if (snapshots[0].active) {
            found_voice_zero_active = 1;
        }
        if (snapshots[1].active) {
            found_voice_one_active = 1;
        }
    }
    assert_true(found_voice_zero_active);
    tfmx_playback_legacy_bridge_reset();
    return found_voice_one_active;
}

static void test_legacy_bridge_reset_clears_unused_table_slots(void **state)
{
    tfmx_loader_candidate candidate;
    struct tfmx_loader_metadata metadata;
    int pattern_restart_activated_voice_one;
    int macro_restart_activated_voice_one;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.voices_01",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.voices_01",
                         &candidate),
                     TFMX_LOAD_SUCCESS);

    /* Seed both bridge-owned slots before the reduced pattern-table restart. */
    metadata = candidate.metadata;
    assert_int_equal(tfmx_playback_legacy_bridge_start(
                         candidate.mdat, candidate.mdat_size, candidate.smpl,
                         candidate.smpl_size, &metadata, 0),
                     1);
    tfmx_playback_legacy_bridge_reset();
    metadata = candidate.metadata;
    metadata.pattern_count = 1;
    metadata.macro_count = 2;
    pattern_restart_activated_voice_one =
        reduced_voices_01_tables_activate_voice_one(&candidate, &metadata);

    /* Re-seed both slots before the reduced macro-table restart. */
    metadata = candidate.metadata;
    assert_int_equal(tfmx_playback_legacy_bridge_start(
                         candidate.mdat, candidate.mdat_size, candidate.smpl,
                         candidate.smpl_size, &metadata, 0),
                     1);
    tfmx_playback_legacy_bridge_reset();
    metadata = candidate.metadata;
    metadata.pattern_count = 2;
    metadata.macro_count = 1;
    macro_restart_activated_voice_one =
        reduced_voices_01_tables_activate_voice_one(&candidate, &metadata);

    assert_false(pattern_restart_activated_voice_one);
    assert_false(macro_restart_activated_voice_one);
    tfmx_loader_candidate_dispose(&candidate);
}

static void assert_loader_rejects_malformed_pair_and_disposes(const char *case_name)
{
    char mdat_path[512];
    char smpl_path[512];
    tfmx_loader_candidate candidate;

    assert_true(snprintf(mdat_path, sizeof(mdat_path),
                         "%s/tests/fixtures/mdat.malformed_%s",
                         TFMX_SOURCE_ROOT, case_name) > 0);
    assert_true(snprintf(smpl_path, sizeof(smpl_path),
                         "%s/tests/fixtures/smpl.malformed_%s",
                         TFMX_SOURCE_ROOT, case_name) > 0);
    assert_int_equal(tfmx_loader_read(mdat_path, smpl_path, &candidate),
                     TFMX_LOAD_INVALID_FORMAT);
    tfmx_loader_candidate_dispose(&candidate);
    assert_null(candidate.mdat);
    assert_null(candidate.smpl);
}

static void test_loader_rejects_first_pattern_equal_trackstart(void **state)
{
    (void)state;
    assert_loader_rejects_malformed_pair_and_disposes(
        "first_pattern_equal_trackstart");
}

static void test_loader_rejects_first_pattern_before_trackstart(void **state)
{
    (void)state;
    assert_loader_rejects_malformed_pair_and_disposes(
        "first_pattern_before_trackstart");
}

static void test_loader_rejects_end_span(void **state)
{
    (void)state;
    assert_loader_rejects_malformed_pair_and_disposes("end_span");
}

static void assert_bridge_rejects_and_resets(
    const tfmx_loader_candidate *candidate, const unsigned char *mdat,
    const struct tfmx_loader_metadata *metadata)
{
    assert_int_equal(tfmx_playback_legacy_bridge_start(
                         candidate->mdat, candidate->mdat_size, candidate->smpl,
                         candidate->smpl_size, &candidate->metadata, 0),
                     1);
    assert_int_equal(tfmx_playback_legacy_bridge_start(
                         mdat, candidate->mdat_size, candidate->smpl,
                         candidate->smpl_size, metadata, 0),
                     0);
    assert_true(tfmx_playback_legacy_bridge_is_complete());
    tfmx_playback_legacy_bridge_reset();
}

static void test_legacy_bridge_rejects_first_pattern_equal_trackstart(void **state)
{
    tfmx_loader_candidate candidate;
    struct tfmx_loader_metadata metadata;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    metadata = candidate.metadata;
    metadata.first_pattern = metadata.trackstart;
    assert_bridge_rejects_and_resets(&candidate, candidate.mdat, &metadata);
    tfmx_loader_candidate_dispose(&candidate);
}

static void test_legacy_bridge_rejects_first_pattern_before_trackstart(void **state)
{
    tfmx_loader_candidate candidate;
    struct tfmx_loader_metadata metadata;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    metadata = candidate.metadata;
    metadata.first_pattern = metadata.trackstart - 0x10;
    assert_bridge_rejects_and_resets(&candidate, candidate.mdat, &metadata);
    tfmx_loader_candidate_dispose(&candidate);
}

static void test_legacy_bridge_rejects_end_span(void **state)
{
    tfmx_loader_candidate candidate;
    struct tfmx_loader_metadata metadata;
    unsigned char *copied_mdat;

    (void)state;
    assert_int_equal(tfmx_loader_read(
                         TFMX_SOURCE_ROOT "/tests/fixtures/mdat.step8",
                         TFMX_SOURCE_ROOT "/tests/fixtures/smpl.step8",
                         &candidate),
                     TFMX_LOAD_SUCCESS);
    copied_mdat = malloc(candidate.mdat_size);
    assert_non_null(copied_mdat);
    memcpy(copied_mdat, candidate.mdat, candidate.mdat_size);
    assert_int_equal(copied_mdat[0x141], 0x01);
    copied_mdat[0x141] = 0x02;
    metadata = candidate.metadata;
    assert_bridge_rejects_and_resets(&candidate, copied_mdat, &metadata);
    free(copied_mdat);
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
MALFORMED_CASE_TEST(test_malformed_empty_pattern_table, "empty_pattern_table")
MALFORMED_CASE_TEST(test_malformed_empty_macro_table, "empty_macro_table")
MALFORMED_CASE_TEST(test_malformed_unaligned_pattern_entry, "unaligned_pattern_entry")
MALFORMED_CASE_TEST(test_malformed_unaligned_macro_entry, "unaligned_macro_entry")
MALFORMED_CASE_TEST(test_malformed_out_of_range_macro, "out_of_range_macro")
MALFORMED_CASE_TEST(test_malformed_below_note_data_pattern, "below_note_data_pattern")
MALFORMED_CASE_TEST(test_malformed_truncated_macro_table, "truncated_macro_table")
MALFORMED_CASE_TEST(test_malformed_first_pattern_equal_trackstart,
                    "first_pattern_equal_trackstart")
MALFORMED_CASE_TEST(test_malformed_first_pattern_before_trackstart,
                    "first_pattern_before_trackstart")
MALFORMED_CASE_TEST(test_malformed_end_span, "end_span")

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_playback_context_create_destroy),
        cmocka_unit_test(test_loader_normalizes_fixture_tables),
        cmocka_unit_test(test_loader_resolves_default_header_pointers),
        cmocka_unit_test(test_loader_scans_independent_tables_with_bounded_capacity),
        cmocka_unit_test(test_loader_admits_leading_zero_smpl_without_header_rule),
        cmocka_unit_test(test_loader_treats_smpl_as_opaque_without_macro_range_inference),
        cmocka_unit_test(test_loader_rejects_one_byte_smpl),
        cmocka_unit_test(test_loader_rejects_first_pattern_equal_trackstart),
        cmocka_unit_test(test_loader_rejects_first_pattern_before_trackstart),
        cmocka_unit_test(test_loader_rejects_end_span),
        cmocka_unit_test(
            test_legacy_bridge_owns_voices_01_tables_and_rejects_out_of_range_metadata),
        cmocka_unit_test(test_legacy_bridge_reset_clears_unused_table_slots),
        cmocka_unit_test(test_legacy_bridge_rejects_first_pattern_equal_trackstart),
        cmocka_unit_test(test_legacy_bridge_rejects_first_pattern_before_trackstart),
        cmocka_unit_test(test_legacy_bridge_rejects_end_span),
        cmocka_unit_test(test_playback_context_loads_separate_fixture_files),
        cmocka_unit_test(test_playback_context_rejects_invalid_or_missing_paths),
        cmocka_unit_test(test_playback_context_rejects_malformed_data_transactionally),
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
        cmocka_unit_test(test_malformed_empty_pattern_table),
        cmocka_unit_test(test_malformed_empty_macro_table),
        cmocka_unit_test(test_malformed_unaligned_pattern_entry),
        cmocka_unit_test(test_malformed_unaligned_macro_entry),
        cmocka_unit_test(test_malformed_out_of_range_macro),
        cmocka_unit_test(test_malformed_below_note_data_pattern),
        cmocka_unit_test(test_malformed_truncated_macro_table),
        cmocka_unit_test(test_malformed_first_pattern_equal_trackstart),
        cmocka_unit_test(test_malformed_first_pattern_before_trackstart),
        cmocka_unit_test(test_malformed_end_span),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
