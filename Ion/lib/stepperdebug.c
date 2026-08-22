#include "stepper.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#define BURST_STEPS 100

static struct termios old_term;

static void enableRawMode(void)
{
    struct termios new_term;

    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;

    new_term.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

static void disableRawMode(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

int main(void)
{
    /* [MS0, MS1, MS2, RST, SLP, EN, DIR] */
    int initial_state[7] = {0, 0, 0, 1, 1, 0, 0};

    /*
     * Ordered from slowest to fastest at a fixed STEP pulse rate.
     *
     * MS0 MS1 MS2
     *  1   1   1   = 1/16 step
     *  1   1   0   = 1/8 step
     *  0   1   0   = 1/4 step
     *  1   0   0   = 1/2 step
     *  0   0   0   = full step
     */
    const char speeds[5] = {
        0b111,
        0b110,
        0b010,
        0b100,
        0b000
    };

    const char *speed_names[5] = {
        "1/16 step",
        "1/8 step",
        "1/4 step",
        "1/2 step",
        "Full step"
    };

    int speed_index = 2;      /* Start at quarter step */
    int direction = 0;
    int ch;

    if (initStp(initial_state) != 0) {
        fprintf(stderr, "Failed to initialize stepper.\n");
        return 1;
    }

    setSpeed(speeds[speed_index]);

    enableRawMode();

    printf("\nStepper debug controls:\n");
    printf("  W = extrude forward\n");
    printf("  S = extrude reverse\n");
    printf("  D = increase speed\n");
    printf("  A = decrease speed\n");
    printf("  Q = quit\n\n");

    printf("Current speed: %s\n", speed_names[speed_index]);

    while (1) {
        ch = getchar();

        switch (ch) {
        case 'w':
        case 'W':
            if (direction != 0) {
                stpSWDir();
                direction = 0;
            }

            extrudeTape(BURST_STEPS);
            break;

        case 's':
        case 'S':
            if (direction != 1) {
                stpSWDir();
                direction = 1;
            }

            extrudeTape(BURST_STEPS);
            break;

        case 'd':
        case 'D':
            if (speed_index < 4) {
                speed_index++;
                setSpeed(speeds[speed_index]);
                printf("\rSpeed: %-12s\n", speed_names[speed_index]);
            } else {
                printf("\rAlready at maximum speed.\n");
            }
            break;

        case 'a':
        case 'A':
            if (speed_index > 0) {
                speed_index--;
                setSpeed(speeds[speed_index]);
                printf("\rSpeed: %-12s\n", speed_names[speed_index]);
            } else {
                printf("\rAlready at minimum speed.\n");
            }
            break;

        case 'q':
        case 'Q':
            disableRawMode();
            endStepper(initial_state);
            printf("\nStepper debug finished.\n");
            return 0;

        default:
            break;
        }
    }
}
