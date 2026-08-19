#include <stdio.h>
#include <unistd.h>

#include "application.h"

int main(int argc, char **argv)
{
    printf("*** main: Starting\n");

    if ((0 == getuid()) || (0 == geteuid()))
    {
        printf("Do not run tfmx-play as 'root'!\n");
        return(-123);
    }

    return application_run(argc, argv);
}
