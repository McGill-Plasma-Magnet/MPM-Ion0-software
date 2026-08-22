#include <stepper.h>

struct timespec delay = {
    .tv_sec = 0,
    .tv_nsec = 400000
};

static struct gpiod_chip *chip;
static struct gpiod_line *ms0;
static struct gpiod_line *ms1;
static struct gpiod_line *ms2;
static struct gpiod_line *stpRST;
static struct gpiod_line *stpSLP;
static struct gpiod_line *stpEN;
static struct gpiod_line *stpSTEP;
static struct gpiod_line *stpDIR;

//gpiochip2 
// [MS0, MS1, MS2, RST, SLP, EN, DIR]
const int statepin[7] = {0, 0, 0, 1, 1, 0, 0};
int initStp(const int pin[7]) {

    chip = gpiod_chip_open("/dev/gpiochip2");
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    ms0 = gpiod_chip_get_line(chip, 14);
    ms1 = gpiod_chip_get_line(chip, 15);
    ms2 = gpiod_chip_get_line(chip, 13);
    stpRST = gpiod_chip_get_line(chip, 12);
    stpSLP = gpiod_chip_get_line(chip, 9);
    stpEN = gpiod_chip_get_line(chip, 11);
    stpSTEP = gpiod_chip_get_line(chip, 10);
    stpDIR = gpiod_chip_get_line(chip, 8);

    gpiod_line_request_output(ms0, "stepper", pin[0]);
    gpiod_line_request_output(ms1, "stepper", pin[1]);
    gpiod_line_request_output(ms2, "stepper", pin[2]);
    gpiod_line_request_output(stpRST, "stepper", pin[3]);
    gpiod_line_request_output(stpSLP, "stepper", pin[4]);
    gpiod_line_request_output(stpEN, "stepper", pin[5]);
    gpiod_line_request_output(stpSTEP, "stepper", 0);
    gpiod_line_request_output(stpDIR, "stepper", pin[6]);
    return 0;
}

void driveStp() {
    gpiod_line_set_value(stpSTEP, 1);
    nanosleep(&delay, NULL);
    gpiod_line_set_value(stpSTEP, 0);
    nanosleep(&delay, NULL);
}
void stpSWDir() {
    int dir = gpiod_line_get_value(stpDIR);
    gpiod_line_set_value(stpDIR, !dir);
}

void stpSetSlp(int sleep) {
    gpiod_line_set_value(stpSLP, sleep);
}
void stpSlp(int time) {
    gpiod_line_set_value(stpSLP, 0);
    sleep(time);
    gpiod_line_set_value(stpSLP, 1);
}

void extrudeTape(int steps) {
    while (steps > 0) {
        driveStp();
        --steps;
    }
}
void setSpeed(char speed)
{
    gpiod_line_set_value(ms0, (speed >> 2) & 1);
    gpiod_line_set_value(ms1, (speed >> 1) & 1);
    gpiod_line_set_value(ms2, speed & 1);
}

void endStepper()
{
    if (ms0)
    {
        gpiod_line_set_value(ms0, pin[0]);
        gpiod_line_release(ms0);
        ms0 = NULL;
    }
    if (ms1)
    {
        gpiod_line_set_value(ms1, pin[1]);
        gpiod_line_release(ms1);
        ms1 = NULL;
    }
    if (ms2)
    {
        gpiod_line_set_value(ms2, pin[0]);
        gpiod_line_release(ms2);
        ms2 = NULL;
    }

    if (stpRST)
    {
        gpiod_line_set_value(stpRST,pin[3]);
        gpiod_line_release(stpRST);
        stpRST = NULL;
    }
    if (stpSLP)
    {
        gpiod_line_set_value(stpSLP,pin[4]);
        gpiod_line_release(stpRST);
        stpSLP = NULL;
    }
    if (stpEN)
    {
        gpiod_line_set_value(stpEN,pin[5]);
        gpiod_line_release(stpEN);
        stpEN = NULL;
    }
    if (stpDIR)
    {
        gpiod_line_set_value(stpDIR,pin[6]);
        gpiod_line_release(stpDIR);
        stpDIR = NULL;
    }
    if (stpSTEP)
    {
        gpiod_line_set_value(stpSTEP,pin[6]);
        gpiod_line_release(stpSTEP);
        stpSTEP = NULL;
    }
    if (chip2)
    {
        gpiod_chip_close(chip2);
        chip2 = NULL;
    }
}
