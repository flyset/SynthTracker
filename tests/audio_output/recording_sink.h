#ifndef SYNTHTRACKER_TEST_RECORDING_SINK_H
#define SYNTHTRACKER_TEST_RECORDING_SINK_H

#include <stddef.h>

#include "audio_output.h"

void recording_sink_reset(void);
size_t recording_sink_submission_count(void);
size_t recording_sink_submission_frame_count(size_t submission_index);
size_t recording_sink_frame_count(void);
const audio_frame *recording_sink_frame(size_t index);

#endif
