#ifndef TFMX_PLAYBACK_LEGACY_BRIDGE_H
#define TFMX_PLAYBACK_LEGACY_BRIDGE_H

#include <stddef.h>

#include "tfmx_loader.h"
#include "playback_context.h"

int tfmx_playback_legacy_bridge_start(const unsigned char *mdat,
                                      size_t mdat_size,
                                       const unsigned char *smpl,
                                       size_t smpl_size,
                                       const struct tfmx_loader_metadata *metadata,
                                       unsigned int subsong);
void tfmx_playback_legacy_bridge_reset(void);
void tfmx_playback_legacy_bridge_set_output_rate(unsigned int output_rate_hz);
int tfmx_playback_legacy_bridge_tick(tfmx_voice_snapshot *snapshots);
int tfmx_playback_legacy_bridge_is_complete(void);

#endif
