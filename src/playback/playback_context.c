#include <stdlib.h>

#include "playback_context.h"
#include "playback_legacy_bridge.h"
#include "playback_legacy_mixer.h"
#include "tfmx_loader.h"

struct tfmx_playback_context {
    unsigned char *mdat;
    size_t mdat_size;
    unsigned char *smpl;
    size_t smpl_size;
    struct tfmx_loader_metadata metadata;
    int started;
    int render_ready;
    tfmx_playback_legacy_mixer mixer;
    tfmx_voice_snapshot voice_zero;
    tfmx_voice_snapshot_set snapshot_cache;
};

tfmx_playback_context *tfmx_playback_context_create(void)
{
    return calloc(1, sizeof(tfmx_playback_context));
}

int tfmx_playback_context_is_loaded(const tfmx_playback_context *context)
{
    return context != NULL && context->mdat != NULL && context->smpl != NULL;
}

void tfmx_playback_context_destroy(tfmx_playback_context *context)
{
    if (context == NULL) {
        return;
    }
    free(context->mdat);
    free(context->smpl);
    tfmx_playback_legacy_bridge_reset();
    free(context);
}

tfmx_load_status tfmx_playback_context_load(tfmx_playback_context *context,
                                            const char *mdat_path,
                                            const char *smpl_path)
{
    tfmx_loader_candidate candidate;
    tfmx_load_status status;

    if (context == NULL || mdat_path == NULL || smpl_path == NULL) {
        return TFMX_LOAD_INVALID_ARGUMENT;
    }
    status = tfmx_loader_read(mdat_path, smpl_path, &candidate);
    if (status != TFMX_LOAD_SUCCESS) {
        return status;
    }
    tfmx_playback_legacy_bridge_reset();
    free(context->mdat);
    free(context->smpl);
    context->mdat = candidate.mdat;
    context->mdat_size = candidate.mdat_size;
    context->smpl = candidate.smpl;
    context->smpl_size = candidate.smpl_size;
    context->metadata = candidate.metadata;
    context->started = 0;
    context->render_ready = 0;
    tfmx_playback_legacy_mixer_reset(&context->mixer);
    context->voice_zero = (tfmx_voice_snapshot){ 0, 0, 0 };
    context->snapshot_cache = (tfmx_voice_snapshot_set){ 0 };
    candidate.mdat = NULL;
    candidate.smpl = NULL;
    tfmx_loader_candidate_dispose(&candidate);
    return TFMX_LOAD_SUCCESS;
}

tfmx_start_status tfmx_playback_context_start(tfmx_playback_context *context,
                                              unsigned int subsong)
{
    if (context == NULL) {
        return TFMX_START_INVALID_ARGUMENT;
    }
    if (!tfmx_playback_context_is_loaded(context)) {
        tfmx_playback_legacy_bridge_reset();
        return TFMX_START_NOT_LOADED;
    }
    if (subsong != 0) {
        tfmx_playback_legacy_bridge_reset();
        return TFMX_START_UNSUPPORTED_SUBSONG;
    }
    if (!tfmx_playback_legacy_bridge_start(context->mdat, context->mdat_size,
                                           context->smpl, context->smpl_size,
                                           &context->metadata,
                                           subsong)) {
        return TFMX_START_LEGACY_FAILURE;
    }
    context->started = 1;
    context->render_ready = 0;
    tfmx_playback_legacy_mixer_reset(&context->mixer);
    context->snapshot_cache = (tfmx_voice_snapshot_set){ 0 };
    context->voice_zero = context->snapshot_cache.voice[0];
    return TFMX_START_SUCCESS;
}

tfmx_tick_status tfmx_playback_context_tick(tfmx_playback_context *context)
{
    tfmx_voice_snapshot_set tick_snapshot;

    if (context == NULL) {
        return TFMX_TICK_INVALID_ARGUMENT;
    }
    if (!context->started) {
        return TFMX_TICK_NOT_STARTED;
    }
    if (!tfmx_playback_legacy_bridge_tick(tick_snapshot.voice)) {
        return TFMX_TICK_NOT_STARTED;
    }
    context->snapshot_cache = tick_snapshot;
    context->voice_zero = context->snapshot_cache.voice[0];
    tfmx_playback_legacy_mixer_begin_tick(&context->mixer, 14318, 44100);
    context->render_ready = 1;
    return TFMX_TICK_SUCCESS;
}

tfmx_render_status tfmx_playback_context_render(tfmx_playback_context *context,
                                                unsigned char *output,
                                                size_t capacity,
                                                size_t *bytes_written)
{
    size_t required;

    if (context == NULL || output == NULL || bytes_written == NULL) {
        return TFMX_RENDER_INVALID_ARGUMENT;
    }
    if (!context->started || !context->render_ready) {
        return TFMX_RENDER_NOT_STARTED;
    }
    required = context->mixer.pending_frames * 4U;
    if (capacity < required) {
        return TFMX_RENDER_INSUFFICIENT_CAPACITY;
    }
    if (!tfmx_playback_legacy_mixer_render(&context->mixer, output, capacity,
                                           bytes_written)) {
        return TFMX_RENDER_INVALID_ARGUMENT;
    }
    context->render_ready = 0;
    return TFMX_RENDER_SUCCESS;
}

int tfmx_playback_context_is_complete(const tfmx_playback_context *context)
{
    return context != NULL && context->started &&
           tfmx_playback_legacy_bridge_is_complete();
}

tfmx_snapshot_status tfmx_playback_context_snapshot(
    const tfmx_playback_context *context, unsigned int voice,
    tfmx_voice_snapshot *snapshot)
{
    if (context == NULL || snapshot == NULL) {
        return TFMX_SNAPSHOT_INVALID_ARGUMENT;
    }
    if (voice != 0) {
        return TFMX_SNAPSHOT_UNSUPPORTED_VOICE;
    }
    if (!context->started) {
        return TFMX_SNAPSHOT_NOT_STARTED;
    }
    *snapshot = context->voice_zero;
    return TFMX_SNAPSHOT_SUCCESS;
}

tfmx_snapshot_status tfmx_playback_context_snapshot_all(
    const tfmx_playback_context *context, tfmx_voice_snapshot_set *snapshot)
{
    if (context == NULL || snapshot == NULL) {
        return TFMX_SNAPSHOT_INVALID_ARGUMENT;
    }
    if (!context->started) {
        return TFMX_SNAPSHOT_NOT_STARTED;
    }
    *snapshot = context->snapshot_cache;
    return TFMX_SNAPSHOT_SUCCESS;
}
