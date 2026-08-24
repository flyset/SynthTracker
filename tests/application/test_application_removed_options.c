#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <cmocka.h>

#include "application.h"

struct removed_option {
    const char *option;
    const char *argument;
};

static ssize_t run_and_capture_usage(int argc, char **argv, char *output,
                                     size_t output_capacity, int *status)
{
    int pipe_fds[2];
    int saved_stderr;
    ssize_t output_size;

    assert_int_equal(pipe(pipe_fds), 0);
    saved_stderr = dup(STDERR_FILENO);
    assert_true(saved_stderr >= 0);
    assert_int_equal(dup2(pipe_fds[1], STDERR_FILENO), STDERR_FILENO);
    close(pipe_fds[1]);

    optind = 1;
    opterr = 0;
    *status = application_run(argc, argv);
    fflush(stderr);

    assert_int_equal(dup2(saved_stderr, STDERR_FILENO), STDERR_FILENO);
    close(saved_stderr);
    output_size = read(pipe_fds[0], output, output_capacity - 1);
    close(pipe_fds[0]);
    assert_true(output_size >= 0);
    output[output_size] = '\0';
    return output_size;
}

static void test_removed_audio_options_are_unknown(void **state)
{
    const struct removed_option options[] = {
        { "-b", "1" },
        { "-8", NULL },
        { "-f", "44100" },
        { "-o", "/tmp/synthtracker-removed-o-output" },
        { "-w", "0" },
        { "-v", NULL },
    };

    (void)state;
    for (size_t index = 0; index < sizeof(options) / sizeof(options[0]);
         index++) {
        char output[512] = {0};
        char *argv[] = {
            "synthtracker",
            (char *)options[index].option,
            (char *)options[index].argument,
            "mdat.removed-option-test",
            NULL,
        };
        const int argc = options[index].argument == NULL ? 3 : 4;
        int status = 0;

        if (options[index].argument != NULL &&
            strcmp(options[index].option, "-o") == 0) {
            unlink(options[index].argument);
        }

        run_and_capture_usage(argc, argv, output, sizeof(output), &status);

        assert_int_equal(status, 2);
        assert_non_null(strstr(output, "Usage:"));
        if (strcmp(options[index].option, "-o") == 0) {
            assert_int_equal(access(options[index].argument, F_OK), -1);
        }
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_removed_audio_options_are_unknown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
