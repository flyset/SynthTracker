#ifndef SYNTHTRACKER_APPLICATION_H
#define SYNTHTRACKER_APPLICATION_H

int application_run(int argc, char **argv);

#if defined(SYNTHTRACKER_APPLICATION_TEST)
#include "audio_output/adapters/coreaudio_facade.h"

void application_test_set_coreaudio_facade(
    const audio_output_coreaudio_facade *facade);

typedef void (*application_test_preparation_observer)(
    const audio_output_coreaudio_format *format,
    unsigned int renderer_rate_hz);

void application_test_set_preparation_observer(
    application_test_preparation_observer observer);
#endif

#endif
