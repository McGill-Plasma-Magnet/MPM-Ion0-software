#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "logs.h"

#define NUM_READINGS 20
#define READING_INTERVAL_SEC 1

static volatile sig_atomic_t keepRunning = 1;

static void handleSigint(int sig)
{
    (void)sig;
    keepRunning = 0;
}

int main(void)
{
    signal(SIGINT, handleSigint);

    /* NOTE: createFile() currently returns 1 on success / 0 on failure --
     * the opposite convention from every other function in logs.c. Worth
     * making consistent, but this example calls it the way it's written
     * today. */
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
        i++;
    }

    //dead code but why not if something bad happens
    logMessage("something bad happened");
    flushClose();
    return EXIT_SUCCESS;
}
