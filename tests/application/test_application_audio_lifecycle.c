#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <cmocka.h>

#include "SDL.h"

/* SDL_main.h renames main to SDL_main; restore main for the CMocka entry point. */
#undef main

#include "application.h"
#include "audio_output.h"
#include "player.h"
#include "tfmxsong.h"

/* Nonzero ring-buffer sentinel: a compatible live session starts bhead and
 * btail at this same value and must leave both unchanged. */
enum { RING_SENTINEL = 42 };

enum event_kind {
    EVENT_LOAD,
    EVENT_OPEN_LIVE,
    EVENT_OPEN_FILE,
    EVENT_INIT,
    EVENT_START,
    EVENT_IRQ,
    EVENT_SUBMIT,
    EVENT_SYNC_INIT,
    EVENT_SYNC_COND_INIT,
    EVENT_SYNC_PROCESS,
    EVENT_SYNC_FINALIZE,
    EVENT_SYNC_WAIT,
    EVENT_SYNC_MUTEX_DESTROY,
    EVENT_SYNC_COND_DESTROY,
    EVENT_WRITE,
    EVENT_FILE_CLOSE,
    EVENT_FREE,
    EVENT_SDL_CLOSE,
    EVENT_SDL_QUIT,
};

struct harness {
    enum event_kind events[64];
    size_t event_count;
    int file_opens;
    int submissions;
    int writes;
    int file_closes;
    int sdl_open;
    int sdl_pause;
    int sdl_mix;
    int sdl_delay;
    int sdl_close;
    int sdl_quit;
    int mutex_operations;
    int cond_operations;
    int irq_calls;
    int free_calls;
    size_t bytes_written;
};

static struct harness harness;
static S8 sample_storage[0x1C42 + 0x4 + 1];

int singleFile, dosExt, toOutFile, printinfo, songnum, gubed, export;
int startPat, gemx, loops, dangerFreakHack, monkeyHack;
char outf[PATHNAME_LENGTH];
U32 outRate;
extern int force8, blend, filt, over;
extern char act[8];
extern U32 stereo;
struct Header hdr;
struct Audio audioData[8];
struct Channel channelData[8];
struct TrackManager trackManager;
S8 *smplbuf;
int num_ts, num_pat, num_mac;
int jiffies;
U32 eClocks;
int multimode;
extern volatile int bhead, btail;
extern int bytes, bytes2, sndhdl, isfile, eRem;
extern U32 blocksize, multiplier;
extern volatile sig_atomic_t stop;

static void record_event(enum event_kind event)
{
    assert_true(harness.event_count < sizeof(harness.events) / sizeof(harness.events[0]));
    harness.events[harness.event_count++] = event;
}

static void reset_harness(void)
{
    memset(&harness, 0, sizeof(harness));
    memset(outf, 0, sizeof(outf));
    memset(act, 1, sizeof(act));
    memset(&hdr, 0, sizeof(hdr));
    memset(audioData, 0, sizeof(audioData));
    memset(channelData, 0, sizeof(channelData));
    memset(&trackManager, 0, sizeof(trackManager));
    smplbuf = sample_storage;
    memset(sample_storage, 0x12, sizeof(sample_storage));
    singleFile = dosExt = printinfo = songnum = gubed = export = 0;
    startPat = -1;
    toOutFile = 0;
    gemx = dangerFreakHack = force8 = 0;
    loops = 1;
    blend = 1;
    filt = 0;
    over = -1;
    monkeyHack = 0;
    outRate = 44100;
    stereo = 0;
    num_ts = num_pat = num_mac = 0;
    jiffies = 0;
    eClocks = 14318;
    multimode = 0;
    bhead = btail = RING_SENTINEL;
    bytes = bytes2 = sndhdl = isfile = eRem = 0;
    blocksize = 0;
    multiplier = 1;
    stop = 0;
    optind = 1;
}

static int event_index(enum event_kind event)
{
    for (size_t index = 0; index < harness.event_count; ++index) {
        if (harness.events[index] == event) {
            return (int)index;
        }
    }
    return -1;
}

void TfmxInit(void) { record_event(EVENT_INIT); }
void StartSong(int song, int mode) { (void)song; (void)mode; record_event(EVENT_START); trackManager.PlayerEnable = 1; }
void do_debug(void) {}
int LoopOff(struct Audio *audio) { (void)audio; return 0; }

int load_tfmx(char *mfn, char *sfn)
{
    (void)mfn;
    (void)sfn;
    record_event(EVENT_LOAD);
    return 0;
}

void tfmxIrqIn(void)
{
    harness.irq_calls++;
    record_event(EVENT_IRQ);
    trackManager.PlayerEnable = 0;
}

audio_output_submit_result audio_output_null_adapter_submit(
    audio_output_null_adapter *adapter, const audio_frame_block *block)
{
    (void)adapter;
    assert_non_null(block);
    assert_true(block->frame_count > 0);
    harness.submissions++;
    record_event(EVENT_SUBMIT);
    return AUDIO_OUTPUT_SUBMIT_ACCEPTED;
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    (void)desired; (void)obtained;
    harness.sdl_open++;
    return 0;
}
void SDL_PauseAudio(int pause_on) { (void)pause_on; harness.sdl_pause++; }
char *SDL_GetError(void) { return "fake SDL"; }
void SDL_CloseAudio(void) { harness.sdl_close++; record_event(EVENT_SDL_CLOSE); }
void SDL_Quit(void) { harness.sdl_quit++; record_event(EVENT_SDL_QUIT); }
void SDL_Delay(Uint32 ms)
{
    (void)ms;
    harness.sdl_delay++;
    btail = bhead;
}
void SDL_MixAudio(Uint8 *dst, const Uint8 *src, Uint32 len, int volume)
{ (void)dst; (void)src; (void)len; (void)volume; harness.sdl_mix++; }

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{ (void)mutex; (void)attr; record_event(EVENT_SYNC_INIT); return 0; }
int pthread_mutex_lock(pthread_mutex_t *mutex)
{ (void)mutex; harness.mutex_operations++; return 0; }
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{ (void)mutex; harness.mutex_operations++; return 0; }
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{ (void)mutex; record_event(EVENT_SYNC_MUTEX_DESTROY); return 0; }
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{ (void)cond; (void)attr; record_event(EVENT_SYNC_COND_INIT); return 0; }
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{ (void)cond; (void)mutex; harness.cond_operations++; return 0; }
int pthread_cond_signal(pthread_cond_t *cond)
{ (void)cond; harness.cond_operations++; return 0; }
int pthread_cond_destroy(pthread_cond_t *cond)
{ (void)cond; record_event(EVENT_SYNC_COND_DESTROY); return 0; }

int open(const char *path, int flags, ...)
{ (void)path; (void)flags; harness.file_opens++; record_event(EVENT_OPEN_FILE); return 17; }
ssize_t write(int fd, const void *buffer, size_t count)
{ (void)fd; (void)buffer; harness.writes++; harness.bytes_written += count; record_event(EVENT_WRITE); return (ssize_t)count; }
int close(int fd)
{ (void)fd; harness.file_closes++; record_event(EVENT_FILE_CLOSE); return 0; }
void free(void *pointer)
{ (void)pointer; harness.free_calls++; record_event(EVENT_FREE); }

static int run_application(int argc, char **argv)
{
    reset_harness();
    return application_run(argc, argv);
}

static void test_compatible_live_has_finite_direct_lifecycle(void **state)
{
    char *argv[] = {"synthtracker", "mdat.test", NULL};

    (void)state;
    assert_int_equal(run_application(2, argv), 0);
    assert_int_equal(harness.irq_calls, 1);
    assert_int_equal(harness.submissions, 1);
    assert_int_equal(bhead, RING_SENTINEL);
    assert_int_equal(btail, RING_SENTINEL);
    assert_int_equal(harness.sdl_open, 0);
    assert_int_equal(harness.sdl_pause, 0);
    assert_int_equal(harness.sdl_mix, 0);
    assert_int_equal(harness.sdl_delay, 0);
    assert_int_equal(harness.sdl_close, 0);
    assert_int_equal(harness.sdl_quit, 0);
    assert_int_equal(harness.mutex_operations, 0);
    assert_int_equal(harness.cond_operations, 0);
    assert_int_equal(harness.file_closes, 0);
    assert_int_equal(harness.file_opens, 0);
    assert_int_equal(harness.writes, 0);
    assert_int_equal(harness.bytes_written, 0);
    assert_int_equal(multiplier, 4);
    assert_int_equal(blocksize, 32768);
    assert_true(event_index(EVENT_SYNC_INIT) >= 0);
    assert_true(event_index(EVENT_SYNC_COND_INIT) >= 0);
    assert_true(event_index(EVENT_SUBMIT) >= 0);
    assert_true(event_index(EVENT_SYNC_MUTEX_DESTROY) >= 0);
    assert_true(event_index(EVENT_SYNC_COND_DESTROY) >= 0);
    assert_true(event_index(EVENT_FREE) >= 0);
    assert_int_equal(event_index(EVENT_SYNC_INIT) < event_index(EVENT_SYNC_COND_INIT), 1);
    assert_int_equal(event_index(EVENT_SYNC_COND_INIT) < event_index(EVENT_SUBMIT), 1);
    assert_int_equal(event_index(EVENT_SUBMIT) < event_index(EVENT_SYNC_MUTEX_DESTROY), 1);
    assert_int_equal(event_index(EVENT_SYNC_MUTEX_DESTROY) < event_index(EVENT_SYNC_COND_DESTROY), 1);
    assert_int_equal(event_index(EVENT_SYNC_COND_DESTROY) < event_index(EVENT_FREE), 1);
}

static void test_incompatible_option_uses_isolated_file_route(void **state)
{
    char *argv[] = {"synthtracker", "-o", "output.raw", "-b", "99", "-8",
                    "-w", "3", "-f", "48000", "mdat.test", NULL};

    (void)state;
    assert_int_equal(run_application(11, argv), 0);
    assert_int_equal(harness.file_opens, 1);
    assert_true(harness.writes > 0);
    assert_true(harness.bytes_written > 0);
    assert_true(bytes2 > 0);
    assert_true(bhead > 0);
    assert_int_equal(btail, bhead);
    assert_int_equal(harness.file_closes, 1);
    assert_int_equal(harness.submissions, 0);
    assert_int_equal(harness.sdl_open, 0);
    assert_int_equal(harness.sdl_pause, 0);
    assert_int_equal(harness.sdl_mix, 0);
    assert_int_equal(harness.sdl_delay, 0);
    assert_int_equal(harness.sdl_close, 0);
    assert_int_equal(harness.sdl_quit, 0);
    assert_int_equal(harness.free_calls, 1);
    assert_true(event_index(EVENT_OPEN_FILE) >= 0);
    assert_true(event_index(EVENT_WRITE) >= 0);
    assert_true(event_index(EVENT_FILE_CLOSE) >= 0);
    assert_true(event_index(EVENT_FREE) >= 0);
    assert_int_equal(event_index(EVENT_OPEN_FILE) < event_index(EVENT_WRITE), 1);
    assert_int_equal(event_index(EVENT_WRITE) < event_index(EVENT_FILE_CLOSE), 1);
    assert_int_equal(event_index(EVENT_FILE_CLOSE) < event_index(EVENT_FREE), 1);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_compatible_live_has_finite_direct_lifecycle),
        cmocka_unit_test(test_incompatible_option_uses_isolated_file_route),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
