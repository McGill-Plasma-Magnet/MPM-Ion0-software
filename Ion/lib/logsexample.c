/*
 * logsexample.c
 *
 * Minimal driver program to exercise the logs.c/logs.h API:
 *   createFile() -> setupLogs() -> logData()/logMessage() in a loop -> flushClose()
 *
 * NOTE: logs.c currently has several bugs that will stop it from compiling
 * (typos, invalid C++-style string ops on a plain char[], a missing
 * #include <errno.h>, etc). This example assumes those get fixed first --
 * it's here to test behavior once logs.c actually builds, not to work
 * around the current bugs.
 *
 * Build (adjust the source list to match your actual driver filenames):
 *   gcc logsexample.c logs.c ms5607.c hdc2080.c thermistor.c -o logsexample
 *
 * Run (Ctrl+C stops early; it also stops after NUM_READINGS readings):
 *   ./logsexample
 */

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
        fprintf(stderr, "createFile() failed, aborting test\n");
        return EXIT_FAILURE;
    }

    /* setupLogs(), like the rest of logs.c, returns 0 on success. */
    if (setupLogs() != 0)
    {
        fprintf(stderr, "setupLogs() failed, aborting test\n");
        return EXIT_FAILURE;
    }

    logMessage("logsexample: starting test run");

    int i;
    for (i = 0; i < NUM_READINGS && keepRunning; i++)
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
    }

    logMessage("logsexample: ending test run");
    flushClose();

    printf("Done. Attempted %d readings, files written under %s\n", i, DIR_PATH);
    return EXIT_SUCCESS;
}
