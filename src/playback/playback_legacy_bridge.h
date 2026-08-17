#ifndef TFMX_PLAYBACK_LEGACY_BRIDGE_H
#define TFMX_PLAYBACK_LEGACY_BRIDGE_H

#include <stddef.h>

#include "tfmx_loader.h"

int tfmx_playback_legacy_bridge_start(const unsigned char *mdat,
                                      size_t mdat_size,
                                       const unsigned char *smpl,
                                       size_t smpl_size,
                                       const struct tfmx_loader_metadata *metadata,
                                       unsigned int subsong);
void tfmx_playback_legacy_bridge_reset(void);
int tfmx_playback_legacy_bridge_tick(int *active, unsigned short *pitch,
                                     unsigned char *volume);
int tfmx_playback_legacy_bridge_is_complete(void);

#endif
