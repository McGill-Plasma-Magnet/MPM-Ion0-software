#include "stepper.h"
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    // [MS0, MS1, MS2, RST, SLP, EN, DIR]
    int initial_state[7] = {0, 0, 0, 1, 1, 0, 0};

    printf("Initializing stepper...\n");

    if (initStp(initial_state) != 0) {
        fprintf(stderr, "Failed to initialize stepper.\n");
        return 1;
    }

    // Test all MS0/MS1/MS2 combinations: 000 through 111
    for (int speed = 0; speed < 8; speed++) {

        printf("Testing setting %d: MS0=%d MS1=%d MS2=%d\n",
               speed,
               (speed >> 2) & 1,
               (speed >> 1) & 1,
               speed & 1);

        setSpeed((char)speed);

        extrudeTape(1000);

        sleep(1);
    }

    printf("Switching direction...\n");
    stpSWDir();

    // Test all settings again in reverse direction
    for (int speed = 0; speed < 8; speed++) {

        printf("Reverse - testing setting %d\n", speed);

        setSpeed((char)speed);
        extrudeTape(1000);

        sleep(1);
    }

    printf("Stepper test finished.\n");

    endStepper(initial_state);

    return 0;
}
