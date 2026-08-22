#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <cmocka.h>

#include "application.h"
#include "player.h"
#include "tfmxsong.h"

struct application_events {
    int load;
    int open_live;
    int open_file;
    int init;
    int start;
    int play;
    int takedown;
};

static struct application_events events;
static S8 sample_storage[0x1C42 + 0x4 + 1];

int singleFile, dosExt, toOutFile, printinfo, songnum, gubed, export;
int startPat, gemx, loops, dangerFreakHack, force8, blend, filt, over;
int monkeyHack;
char outf[PATHNAME_LENGTH], act[8];
U32 outRate, stereo;
struct Header hdr;
struct Audio audioData[8];
S8 *smplbuf;
int num_ts, num_pat, num_mac;

static void reset_application_state(void)
{
    memset(&events, 0, sizeof(events));
    memset(outf, 0, sizeof(outf));
    memset(act, 0, sizeof(act));
    memset(&hdr, 0, sizeof(hdr));
    memset(audioData, 0, sizeof(audioData));
    smplbuf = sample_storage;
    singleFile = 0;
    dosExt = 0;
    toOutFile = 0;
    printinfo = 0;
    songnum = 0;
    gubed = 0;
    export = 0;
    startPat = -1;
    gemx = 0;
    loops = 1;
    dangerFreakHack = 0;
    force8 = 0;
    blend = 1;
    filt = 0;
    over = -1;
    monkeyHack = 0;
    outRate = 44100;
    stereo = 0;
    optind = 1;
}

void open_sndfile(void) { events.open_file++; }
void open_snddev(void) { events.open_live++; }
void TfmxInit(void) { events.init++; }
void StartSong(int song, int mode) { (void)song; (void)mode; events.start++; }
void play_it(void) { events.play++; }
void TfmxTakedown(void) { events.takedown++; }
int load_tfmx(char *mfn, char *sfn)
{
    (void)mfn;
    (void)sfn;
    events.load++;
    return 0;
}
void do_debug(void) {}
int LoopOff(struct Audio *audio) { (void)audio; return 0; }
void inthand(int signum) { (void)signum; }

static int run_application(int argc, char **argv)
{
    reset_application_state();
    return application_run(argc, argv);
}

static void assert_live_lifecycle(void)
{
    assert_int_equal(events.load, 1);
    assert_int_equal(events.open_live, 1);
    assert_int_equal(events.open_file, 0);
    assert_int_equal(events.init, 1);
    assert_int_equal(events.start, 1);
    assert_int_equal(events.play, 1);
    assert_int_equal(events.takedown, 1);
}

static void test_application_accepts_default_live_profile(void **state)
{
    char *argv[] = {"synthtracker", "mdat.test", NULL};
    int status;

    (void)state;
    status = run_application(2, argv);

    assert_int_equal(status, 0);
    assert_int_equal(blend, 1);
    assert_int_equal(stereo, 1);
    assert_live_lifecycle();
}

static void test_application_accepts_explicit_live_profiles(void **state)
{
    char *b1_argv[] = {"synthtracker", "-f", "44100", "-b", "1", "mdat.test", NULL};
    char *b2_argv[] = {"synthtracker", "-f", "44100", "-b", "2", "mdat.test", NULL};

    (void)state;
    assert_int_equal(run_application(6, b1_argv), 0);
    assert_int_equal(outRate, 44100);
    assert_int_equal(blend, 1);
    assert_int_equal(stereo, 1);
    assert_live_lifecycle();

    assert_int_equal(run_application(6, b2_argv), 0);
    assert_int_equal(outRate, 44100);
    assert_int_equal(blend, 0);
    assert_int_equal(stereo, 1);
    assert_live_lifecycle();
}

static void test_application_uses_last_repeated_live_options(void **state)
{
    char *blend_argv[] = {
        "synthtracker", "-b", "0", "-b", "2", "mdat.test", NULL
    };
    char *rate_argv[] = {
        "synthtracker", "-f", "48000", "-f", "44100", "mdat.test", NULL
    };

    (void)state;
    assert_int_equal(run_application(6, blend_argv), 0);
    assert_int_equal(blend, 0);
    assert_int_equal(stereo, 1);
    assert_live_lifecycle();

    assert_int_equal(run_application(6, rate_argv), 0);
    assert_int_equal(outRate, 44100);
    assert_live_lifecycle();
}

static void assert_rejected_live_profile(int argc, char **argv)
{
    int status = run_application(argc, argv);

    assert_true(status != 0);
    assert_int_equal(events.load, 0);
    assert_int_equal(events.open_live, 0);
    assert_int_equal(events.open_file, 0);
    assert_int_equal(events.init, 0);
    assert_int_equal(events.start, 0);
    assert_int_equal(events.play, 0);
    assert_int_equal(events.takedown, 0);
}

static void test_application_rejects_invalid_live_profiles(void **state)
{
    char *b0[] = {"synthtracker", "-b", "0", "mdat.test", NULL};
    char *b3[] = {"synthtracker", "-b", "3", "mdat.test", NULL};
    char *b_negative[] = {"synthtracker", "-b", "-1", "mdat.test", NULL};
    char *eight_bit[] = {"synthtracker", "-8", "mdat.test", NULL};
    char *width_zero[] = {"synthtracker", "-w", "0", "mdat.test", NULL};
    char *width_three[] = {"synthtracker", "-w", "3", "mdat.test", NULL};
    char *rate_zero[] = {"synthtracker", "-f", "0", "mdat.test", NULL};
    char *rate_48000[] = {"synthtracker", "-f", "48000", "mdat.test", NULL};

    (void)state;
    assert_rejected_live_profile(4, b0);
    assert_rejected_live_profile(4, b3);
    assert_rejected_live_profile(4, b_negative);
    assert_rejected_live_profile(3, eight_bit);
    assert_rejected_live_profile(4, width_zero);
    assert_rejected_live_profile(4, width_three);
    assert_rejected_live_profile(4, rate_zero);
    assert_rejected_live_profile(4, rate_48000);
}

static void test_application_file_output_exempts_profile(void **state)
{
    char *argv[] = {
        "synthtracker", "-o", "output.raw", "-b", "99", "-8",
        "-w", "3", "-f", "48000", "mdat.test", NULL
    };

    (void)state;
    assert_int_equal(run_application(11, argv), 0);
    assert_int_equal(events.load, 1);
    assert_int_equal(events.open_file, 1);
    assert_int_equal(events.open_live, 0);
    assert_int_equal(events.init, 1);
    assert_int_equal(events.start, 1);
    assert_int_equal(events.play, 1);
    assert_int_equal(events.takedown, 1);
}

static void test_application_rejects_missing_positional_input_before_profile(void **state)
{
    char *argv[] = {"synthtracker", "-b", "99", "-f", "48000", NULL};
    int status;

    (void)state;
    status = run_application(5, argv);
    assert_int_equal(status, 2);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_application_accepts_default_live_profile),
        cmocka_unit_test(test_application_accepts_explicit_live_profiles),
        cmocka_unit_test(test_application_uses_last_repeated_live_options),
        cmocka_unit_test(test_application_rejects_invalid_live_profiles),
        cmocka_unit_test(test_application_file_output_exempts_profile),
        cmocka_unit_test(test_application_rejects_missing_positional_input_before_profile),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
