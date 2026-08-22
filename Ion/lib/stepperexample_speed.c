#include "stepper.h"
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int initial_state[7] = {0, 0, 0, 1, 1, 0, 0};

    const char speeds[] = {
        0b000, // Full step
        0b001, // Half step
        0b010, // Quarter step
        0b011, // Eighth step
        0b111  // Sixteenth step
    };

    const char *names[] = {
        "Full step",
        "Half step",
        "Quarter step",
        "Eighth step",
        "Sixteenth step"
    };

    if (initStp(initial_state) != 0) {
        fprintf(stderr, "Failed to initialize stepper\n");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        printf("Testing %s\n", names[i]);

        setSpeed(speeds[i]);
        extrudeTape(1000);

        sleep(1);
    }

    printf("Switching direction...\n");
    stpSWDir();

    for (int i = 0; i < 5; i++) {
        printf("Reverse - testing %s\n", names[i]);

        setSpeed(speeds[i]);
        extrudeTape(1000);

        sleep(1);
    }

    endStepper(initial_state);

    return 0;
}
