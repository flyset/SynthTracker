#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "application.h"
#include "audio_output/adapters/coreaudio_adapter.h"
#include "playback_context.h"
#include "playback_legacy_renderer.h"
#include "tfmx.h"

extern int startPat;
extern int gemx;
extern int loops;
extern int dangerFreakHack;
extern int monkeyHack;

enum {
    APPLICATION_AUDIO_BUFFER_CAPACITY = 65536,
};

typedef struct {
    tfmx_playback_context *playback;
    tfmx_playback_legacy_renderer renderer;
    audio_output_coreaudio_adapter_instance output;
    audio_frame output_frames[APPLICATION_AUDIO_BUFFER_CAPACITY];
    audio_frame tick_frames[APPLICATION_AUDIO_BUFFER_CAPACITY];
    float converted_samples[2 * APPLICATION_AUDIO_BUFFER_CAPACITY];
} application_audio_session;

static volatile sig_atomic_t stop_requested;

#if defined(SYNTHTRACKER_APPLICATION_TEST)
static const audio_output_coreaudio_facade *test_facade;
static application_test_preparation_observer test_preparation_observer;

void application_test_set_coreaudio_facade(
    const audio_output_coreaudio_facade *facade)
{
    test_facade = facade;
}

void application_test_set_preparation_observer(
    application_test_preparation_observer observer)
{
    test_preparation_observer = observer;
}
#endif

static void usage(const char *program)
{
    fprintf(stderr,
            "SynthTracker v1.1.7 by Jon Pickard, Neochrome and others.\n\n"
            "Usage: %s [options] mdat-file [smpl-file]\n"
            "where options is one or more of:\n"
            "-p num\\t\\tsubsong to play (default 0)\n"
            "-P num\\t\\tstart at a trackstep\n"
            "-i\\t\\tprint module information\n"
            "-l num\\t\\tset loop mode\n"
            "-D\\t\\tforce the Danger Freak compatibility hack\n"
            "-G\\t\\tforce the GemX compatibility hack\n"
            "-V channels\\tselect active channels\n"
            "-S, -x, -~\\tlegacy control switches\n",
             program);
}

static bool parse_nonnegative_int(const char *text, int *value)
{
    char *end;
    long parsed;

    if (text == NULL || text[0] == '\0') {
        return false;
    }
    errno = 0;
    parsed = strtol(text, &end, 0);
    if (end == text || *end != '\0' || errno == ERANGE || parsed < 0 ||
        parsed > INT_MAX) {
        return false;
    }
    *value = (int)parsed;
    return true;
}

static void handle_interrupt(int signum)
{
    (void)signum;
    stop_requested = 1;
}

static int copy_path(char *destination, const char *source)
{
    const size_t source_length = strlen(source);
    if (source_length >= PATHNAME_LENGTH) {
        return 0;
    }
    memcpy(destination, source, source_length + 1);
    return 1;
}

static int resolve_input_paths(int argc, char **argv, char *mdat_path,
                               char *smpl_path)
{
    const char *mdat_name;
    const char *filename;

    if (optind >= argc || !copy_path(mdat_path, argv[optind++])) {
        return 0;
    }
    if (optind < argc) {
        return copy_path(smpl_path, argv[optind]);
    }

    filename = strrchr(mdat_path, '/');
    mdat_name = filename == NULL ? mdat_path : filename + 1;
    if (strncmp(mdat_name, "mdat.", 5) != 0) {
        return 0;
    }

    if (filename == NULL) {
        smpl_path[0] = 's';
        return copy_path(smpl_path + 1, mdat_path + 1);
    }

    const size_t directory_length = (size_t)(filename - mdat_path) + 1;
    if (directory_length >= PATHNAME_LENGTH) {
        return 0;
    }
    memcpy(smpl_path, mdat_path, directory_length);
    smpl_path[directory_length] = 's';
    return copy_path(smpl_path + directory_length + 1,
                     mdat_path + directory_length + 1);
}

static const audio_output_coreaudio_facade *application_facade(void)
{
#if defined(SYNTHTRACKER_APPLICATION_TEST)
    if (test_facade != NULL) {
        return test_facade;
    }
#endif
    return audio_output_coreaudio_facade_default();
}

static bool application_format_is_supported(
    const audio_output_coreaudio_format *format)
{
    return format != NULL &&
           (format->sample_rate_hz == 44100 ||
            format->sample_rate_hz == 48000) &&
           format->channel_count == 2 &&
           format->sample_format == AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32 &&
           format->layout == AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED;
}

static bool prepare_application_renderer(
    void *context,
    const audio_output_coreaudio_format *format)
{
    application_audio_session *session = context;
    if (session == NULL || session->playback == NULL ||
        !application_format_is_supported(format)) {
        return false;
    }

    tfmx_playback_legacy_renderer_init(
        &session->renderer, session->playback, format->sample_rate_hz,
        session->output_frames, APPLICATION_AUDIO_BUFFER_CAPACITY,
        session->tick_frames, APPLICATION_AUDIO_BUFFER_CAPACITY);
#if defined(SYNTHTRACKER_APPLICATION_TEST)
    if (test_preparation_observer != NULL) {
        test_preparation_observer(format, session->renderer.output_rate_hz);
    }
#endif
    return true;
}

static audio_frame_block render_application_renderer(
    void *context,
    size_t requested_frame_count)
{
    application_audio_session *session = context;
    if (session == NULL) {
        return (audio_frame_block){ .frame_count = 0, .frames = NULL };
    }
    return tfmx_playback_legacy_renderer_render(
        &session->renderer, requested_frame_count);
}

static int run_live_output(tfmx_playback_context *playback)
{
    application_audio_session session = { .playback = playback };
    const audio_output_coreaudio_format format = {
        .sample_rate_hz = 0,
        .channel_count = 2,
        .sample_format = AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32,
        .layout = AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED,
    };
    const audio_output_coreaudio_facade *facade = application_facade();

    session.output = (audio_output_coreaudio_adapter_instance){
        .facade = facade,
        .route = {
            .renderer = render_application_renderer,
            .prepare = prepare_application_renderer,
            .context = &session,
        },
        .delivery_mode = AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_WORKSPACE,
        .workspace = {
            .samples = session.converted_samples,
            .frame_capacity = APPLICATION_AUDIO_BUFFER_CAPACITY,
        },
    };

    const audio_output_coreaudio_start_result start_result =
        audio_output_coreaudio_adapter_start_instance(&session.output, &format);
    if (start_result != AUDIO_OUTPUT_COREAUDIO_START_STARTED) {
        return 1;
    }

    while (!stop_requested && !tfmx_playback_context_is_complete(playback)) {
        const struct timespec delay = { .tv_sec = 0, .tv_nsec = 1000000 };
        (void)nanosleep(&delay, NULL);
    }

    const audio_output_coreaudio_facade_result stop_result =
        audio_output_coreaudio_adapter_stop_instance(&session.output);
    return stop_result == AUDIO_OUTPUT_COREAUDIO_FACADE_OK ? 0 : 1;
}

int application_run(int argc, char **argv)
{
    char mdat_path[PATHNAME_LENGTH];
    char smpl_path[PATHNAME_LENGTH];
    int song_number = 0;
    int absolute_start = 0;
    bool absolute_start_present = false;
    int option;

    stop_requested = 0;
    opterr = 0;
    startPat = -1;
    while ((option = getopt(argc, argv, "~xGDiSP:V:p:l:")) != -1) {
        switch (option) {
        case 'P':
            if (!parse_nonnegative_int(optarg, &absolute_start)) {
                usage(argv[0]);
                return 2;
            }
            absolute_start_present = true;
            break;
        case 'p':
            if (!parse_nonnegative_int(optarg, &song_number) ||
                song_number > 31) {
                usage(argv[0]);
                return 2;
            }
            break;
        case 'l':
            loops = strtol(optarg, NULL, 0);
            break;
        case 'G':
            gemx = 1;
            break;
        case 'D':
            dangerFreakHack = 1;
            break;
        case 'V':
        case 'S':
        case 'i':
        case 'x':
        case '~':
            break;
        case '?':
        default:
            usage(argv[0]);
            return 2;
        }
    }

    if (!resolve_input_paths(argc, argv, mdat_path, smpl_path)) {
        usage(argv[0]);
        return 2;
    }

    tfmx_playback_context *playback = tfmx_playback_context_create();
    if (playback == NULL ||
        tfmx_playback_context_load(playback, mdat_path, smpl_path) !=
            TFMX_LOAD_SUCCESS) {
        tfmx_playback_context_destroy(playback);
        return 1;
    }
    if (absolute_start_present) {
        startPat = absolute_start;
    }
    if (tfmx_playback_context_start(playback, (unsigned int)song_number) !=
        TFMX_START_SUCCESS) {
        tfmx_playback_context_destroy(playback);
        return 1;
    }

    void (*previous_handler)(int) = signal(SIGINT, handle_interrupt);
    const int status = run_live_output(playback);
    (void)signal(SIGINT, previous_handler);
    tfmx_playback_context_destroy(playback);
    return status;
}
