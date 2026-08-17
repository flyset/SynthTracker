#ifndef TFMX_PLAYBACK_CONTEXT_H
#define TFMX_PLAYBACK_CONTEXT_H

#include <stddef.h>

typedef struct tfmx_playback_context tfmx_playback_context;

/* Result of loading the two required, separate TFMX data files. */
typedef enum tfmx_load_status {
    TFMX_LOAD_SUCCESS = 0,
    TFMX_LOAD_INVALID_ARGUMENT,
    TFMX_LOAD_IO_ERROR,
    TFMX_LOAD_INVALID_FORMAT,
    TFMX_LOAD_OUT_OF_MEMORY
} tfmx_load_status;

typedef enum tfmx_start_status {
    TFMX_START_SUCCESS = 0,
    TFMX_START_INVALID_ARGUMENT,
    TFMX_START_NOT_LOADED,
    TFMX_START_UNSUPPORTED_SUBSONG,
    TFMX_START_LEGACY_FAILURE
} tfmx_start_status;

typedef enum tfmx_tick_status {
    TFMX_TICK_SUCCESS = 0,
    TFMX_TICK_INVALID_ARGUMENT,
    TFMX_TICK_NOT_STARTED
} tfmx_tick_status;

typedef enum tfmx_snapshot_status {
    TFMX_SNAPSHOT_SUCCESS = 0,
    TFMX_SNAPSHOT_INVALID_ARGUMENT,
    TFMX_SNAPSHOT_NOT_STARTED,
    TFMX_SNAPSHOT_UNSUPPORTED_VOICE
} tfmx_snapshot_status;

typedef enum tfmx_render_status {
    TFMX_RENDER_SUCCESS = 0,
    TFMX_RENDER_INVALID_ARGUMENT,
    TFMX_RENDER_INSUFFICIENT_CAPACITY,
    TFMX_RENDER_NOT_STARTED
} tfmx_render_status;

typedef struct tfmx_voice_snapshot {
    int active;
    unsigned short pitch;
    unsigned char volume;
} tfmx_voice_snapshot;

tfmx_playback_context *tfmx_playback_context_create(void);
void tfmx_playback_context_destroy(tfmx_playback_context *context);
int tfmx_playback_context_is_loaded(const tfmx_playback_context *context);

/* Reads and validates MDAT and SMPL, committing them only on full success. */
tfmx_load_status tfmx_playback_context_load(tfmx_playback_context *context,
                                            const char *mdat_path,
                                            const char *smpl_path);

tfmx_start_status tfmx_playback_context_start(tfmx_playback_context *context,
                                              unsigned int subsong);

tfmx_tick_status tfmx_playback_context_tick(tfmx_playback_context *context);
tfmx_render_status tfmx_playback_context_render(tfmx_playback_context *context,
                                                unsigned char *output,
                                                size_t capacity,
                                                size_t *bytes_written);
int tfmx_playback_context_is_complete(const tfmx_playback_context *context);
tfmx_snapshot_status tfmx_playback_context_snapshot(
    const tfmx_playback_context *context, unsigned int voice,
    tfmx_voice_snapshot *snapshot);

#endif
