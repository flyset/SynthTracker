#ifndef TFMX_LOADER_H
#define TFMX_LOADER_H

#include <stddef.h>

#include "playback_context.h"

typedef struct tfmx_loader_candidate {
    unsigned char *mdat;
    size_t mdat_size;
    unsigned char *smpl;
    size_t smpl_size;
    struct tfmx_loader_metadata {
        unsigned int trackstart;
        unsigned int first_pattern;
        unsigned int pattstart;
        unsigned int macrostart;
        unsigned int pattern_count;
        unsigned int macro_count;
        int patterns[128];
        int macros[128];
    } metadata;
} tfmx_loader_candidate;

tfmx_load_status tfmx_loader_read(const char *mdat_path,
                                  const char *smpl_path,
                                  tfmx_loader_candidate *candidate);
void tfmx_loader_candidate_dispose(tfmx_loader_candidate *candidate);

#endif
