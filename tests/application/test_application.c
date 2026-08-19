#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "../../src/application.h"

static void test_application_requires_program_arguments(void **state)
{
    int pipe_fds[2];
    int saved_stderr;
    char output[256] = {0};
    char *argv[] = {"synthtracker", NULL};
    ssize_t output_size;
    int status;

    (void)state;
    assert_int_equal(pipe(pipe_fds), 0);
    saved_stderr = dup(STDERR_FILENO);
    assert_true(saved_stderr >= 0);
    assert_int_equal(dup2(pipe_fds[1], STDERR_FILENO), STDERR_FILENO);
    close(pipe_fds[1]);

    optind = 1;
    status = application_run(1, argv);
    fflush(stderr);

    assert_int_equal(dup2(saved_stderr, STDERR_FILENO), STDERR_FILENO);
    close(saved_stderr);
    output_size = read(pipe_fds[0], output, sizeof(output) - 1);
    close(pipe_fds[0]);
    assert_true(output_size >= 0);
    output[output_size] = '\0';

    assert_int_equal(status, 2);
    assert_non_null(strstr(output, "Usage:"));
    assert_non_null(strstr(output, "SynthTracker v1.1.7/SDL"));
    assert_null(strstr(output, "tfmxplay"));
}

static void test_application_rejects_invalid_option(void **state)
{
    int pipe_fds[2];
    int saved_stderr;
    char output[256] = {0};
    char *argv[] = {"synthtracker", "-z", NULL};
    ssize_t output_size;
    int status;

    (void)state;
    assert_int_equal(pipe(pipe_fds), 0);
    saved_stderr = dup(STDERR_FILENO);
    assert_true(saved_stderr >= 0);
    assert_int_equal(dup2(pipe_fds[1], STDERR_FILENO), STDERR_FILENO);
    close(pipe_fds[1]);

    optind = 1;
    status = application_run(2, argv);
    fflush(stderr);

    assert_int_equal(dup2(saved_stderr, STDERR_FILENO), STDERR_FILENO);
    close(saved_stderr);
    output_size = read(pipe_fds[0], output, sizeof(output) - 1);
    close(pipe_fds[0]);
    assert_true(output_size >= 0);
    output[output_size] = '\0';

    assert_int_equal(status, 2);
    assert_non_null(strstr(output, "Usage:"));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_application_requires_program_arguments),
        cmocka_unit_test(test_application_rejects_invalid_option),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
