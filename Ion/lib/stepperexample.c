#include "stepper.h"
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    /* [MS0, MS1, MS2, RST, SLP, EN, DIR]
       Uses the same initial values currently shown in stepper.c. */
    int initial_state[7] = {0, 0, 0, 1, 0, 1, 0};

    printf("Initializing stepper...\n");
    initStp(initial_state);

    printf("Testing one step...\n");
    driveStp();

    printf("Testing 5 steps...\n");
    extrudeTape(5);

    printf("Switching direction...\n");
    stpSWDir();

    printf("Testing 5 steps in the other direction...\n");
    extrudeTape(5);

    printf("Testing sleep pin control...\n");
    stpSetSlp(0);
    sleep(1);
    stpSetSlp(1);

    printf("Testing timed sleep function...\n");
    stpSlp(1);

    printf("Stepper library test finished.\n");
    return 0;
}
