#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "logs.h"

#define NUM_READINGS 20
#define READING_INTERVAL_SEC 1

struct timespec logDelay = {
    .tv_sec = 0,
    .tv_nsec = 200000000
};
int main(void)
{
    if (!createFile())
    {
        fprintf(stderr, "createFile() failed, exiting\n");
        return EXIT_FAILURE;
    }

    /* setupLogs(), like the rest of logs.c, returns 0 on success. */
    if (setupLogs() != 0)
    {
        fprintf(stderr, "setupLogs() failed, exiting\n");
        return EXIT_FAILURE;
    }

    logMessage("Starting logging");

    int i = 0;
    for (;;)
    {
        if (logData() != 0)
        {
            fprintf(stderr, "logData() reported an error on reading %d\n", i);
        }
        else
        {
            printf("Logged reading %d\n", i);
        }
        sleep(READING_INTERVAL_SEC);
                nanosleep( &logDelay, NULL);
        i++;
    }

    //dead code but why not if something bad happens
    logMessage("something bad happened");
    flushClose();
    return EXIT_FAILURE;
}
