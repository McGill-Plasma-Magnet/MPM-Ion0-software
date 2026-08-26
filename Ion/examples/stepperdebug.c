#include "stepper.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#define BURST_STEPS 30

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

/* Returns 1 if a key is waiting, 0 otherwise. */
static int keyAvailable(void)
{
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) > 0;
}

static void setDirection(int *direction, int new_direction)
{
    if (*direction != new_direction) {
        stpSWDir();
        *direction = new_direction;
    }
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

    int speed_index = 2;   /* Start at quarter step */
    int direction = 0;
    int continuous = 0;    /* 0 = stopped, 1 = continuous */
    int ch;

    if (initStp(initial_state) != 0) {
        fprintf(stderr, "Failed to initialize stepper.\n");
        return 1;
    }

    setSpeed(speeds[speed_index]);
    enableRawMode();

    printf("\nStepper debug controls:\n");
    printf("  w       = 35-step forward burst\n");
    printf("  s       = 35-step reverse burst\n");
    printf("  f       = continuous forward\n");
    printf("  b       = continuous reverse\n");
    printf("  d       = increase speed\n");
    printf("  a       = decrease speed\n");
    printf("  q       = quit\n\n");

    printf("Current speed: %s\n", speed_names[speed_index]);

    while (1) {

        /*
         * In continuous mode, don't block waiting for a key.
         * Keep sending 35-step bursts until a key arrives.
         */
        if (continuous) {
            if (!keyAvailable()) {
                extrudeTape(BURST_STEPS);
                continue;
            }
        }

        ch = getchar();

        /*
         * Any normal valid command stops continuous movement first.
         * The command is then handled normally below.
         */
        if (ch == 'a' || ch == 'd' ||
            ch == 'w' || ch == 's' ||
            ch == 'q') {
            if (continuous) {
                continuous = 0;
                printf("\rContinuous movement stopped.       \n");
            }
        }

        switch (ch) {

        case 'w':
            setDirection(&direction, 0);
            extrudeTape(BURST_STEPS);
            break;

        case 's':
            setDirection(&direction, 1);
            extrudeTape(BURST_STEPS);
            break;

        case 'W':
            setDirection(&direction, 0);
            continuous = 1;
            printf("\rContinuous forward...               \n");
            break;

        case 'S':
            setDirection(&direction, 1);
            continuous = 1;
            printf("\rContinuous reverse...               \n");
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
