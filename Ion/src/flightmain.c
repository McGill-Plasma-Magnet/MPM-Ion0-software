#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <gpiod.h>
#include "stepper.h"
#include "ms5607.h"
#include "current.h"

#define STATE_PATH "/home/debian/logs/state.txt"

struct timespec loopDelay = {
    .tv_sec = 0,
    .tv_nsec = 10000000
};

struct timespec missionDelay = {
    .tv_sec = 1,
    .tv_nsec = 0
};

typedef enum{
    PRELAUNCH,
    ASCENT,
    EXTRUSION_1,
    DELAY,
    EXTRUSION_2,
    INVERTER,
    COMPLETE,
    LANDING,
    LANDED
} FlightState;

static FlightState state;

static int validState(int value)
{
    return value >= PRELAUNCH && value <= LANDED;
}

void getState(void)
{
    FILE *file = fopen(STATE_PATH, "r");

    if (file == NULL)
    {
        fprintf(stderr, "State file missing, using PRELAUNCH\n");
        state = PRELAUNCH;
        return;
    }

    int savedState;

    if (fscanf(file, "%d", &savedState) != 1 ||
            !validState(savedState))
    {
        fprintf(stderr, "Invalid state file, using PRELAUNCH\n");
        state = PRELAUNCH;
    }
    else
    {
        state = (FlightState)savedState;
        fprintf(stdout, "selected state %d\n", savedState);
    }

    fflush(file);
    fsync(fileno(file));
    fclose(file);
}

void changeState(FlightState nextState)
{
    state = nextState;

    FILE *file = fopen(STATE_PATH, "w");
    if (file == NULL)
    {
        fprintf(stderr, "unable to open state file\n");
        fflush(stderr);
        return;
    }
    if (fprintf(file, "%d", (int)nextState) < 0)
    {
        fprintf(stderr, "failed to write state to file\n");
        fflush(stderr);
        fclose(file);
        return;
    }
    fflush(file);
    fsync(fileno(file));
    fclose(file);

}




int main()
{
    getState();

    int counter = 0;
    int invCounter = -1;
    // [MS0, MS1, MS2, RST, SLP, EN, DIR]
    int initial_state[7] = {0, 0, 0, 1, 0, 0, 0};
    float Pressure;
    int pressureValid;
    int thresholdCount = 0;

    //attempt calibration 5 times until succeeds
    int calOk = 0;
    for (int i = 0; i < 5 && !calOk; i++) calOk = readCalibration();

    /* Highest resolution */
    setOSR(4096);

    if (initStp(initial_state) != 0) {
        fprintf(stderr, "Failed to initialize stepper\n");
        return 1;
    }

    float current;
    //initialized GPIO
    static struct gpiod_chip *chipInv;
    static struct gpiod_line *inv;
    static struct gpiod_line *led;

    chipInv = gpiod_chip_open("/dev/gpiochip0");
    if (!chipInv)
    {
        perror("gpiod_chip_open inverter problem");
    }
    inv = gpiod_chip_get_line(chipInv, 9);
    led = gpiod_chip_get_line(chipInv, 26);

    gpiod_line_request_output(inv, "mainflight", 0);
    gpiod_line_request_output(led, "mainflight", 0);

    gpiod_line_set_value(led, 1);

    for (;;)
    {
        pressureValid = 1;
        if (!readDigitalValue())
        {
            fprintf(stderr, "MS5607 measurement failed\n");
            fflush(stderr);
            pressureValid = 0;
        }

        Pressure = getPressure();

        switch (state) 
        {
            case PRELAUNCH:
                if (Pressure <= 1100.0f && Pressure != 0.0f && pressureValid)
                {
                    if (++thresholdCount >= 5)
                    {
                        changeState(ASCENT);
                        printf("Detected Ascent changing state\n");
                        thresholdCount = 0;
                    }
                }else{
                    thresholdCount = 0;
                }
                break;
            case ASCENT:
                nanosleep( &missionDelay, NULL);
                printf("going into delay\n");
                changeState(EXTRUSION_1);
                break;
            case EXTRUSION_1:
                stpSetSlp(1);
                printf("started first extrusion\n");
                //10 000 is 5.55cm of extrusion
                //extrude 133 cm 
                extrudeTape(1);
                printf("completed first extrusion\n");
                stpSetSlp(0);
                changeState(DELAY);
                break;
            case DELAY:
                if (Pressure <= 110.0f && Pressure != 0.0f && pressureValid)
                {
                    if (++thresholdCount >= 5)
                    {
                        changeState(EXTRUSION_2);
                        printf("Initiation second Extrusion\n");
                        thresholdCount = 0;
                    }
                }else{
                    thresholdCount = 0;
                }
                break;
            case EXTRUSION_2:
                stpSetSlp(1);
                printf("started second extrusion\n");
                //10 000 is 5.55cm of extrusion
                // extrude 276 cm
                extrudeTape(497297);
                printf("completed second extrusion\n");
                stpSetSlp(0);
                changeState(INVERTER);
                break;
            case INVERTER:
                if (invCounter <= 0)
                {
                    setOffset();
                    gpiod_line_set_value(inv, 1);
                    invCounter++;
                }else if (invCounter >= 1000){
                    gpiod_line_set_value(inv, 0);
                    changeState(COMPLETE);
                }else{
                    invCounter++;
                    current = readCurrent();
                    printf("current reading: %f\n", current);
                }
                break;
            case COMPLETE:
                if (Pressure >= 200.0f && Pressure != 0.0f && pressureValid)
                {
                    if (++thresholdCount >= 5)
                    {
                        changeState(LANDING);
                        printf("started retrusion of loops\n");
                        thresholdCount = 0;
                    }
                }else{
                    thresholdCount = 0;
                }
                break;
            case LANDING:
                stpSetSlp(1);
                stpSWDir();
                extrudeTape(675675);
                changeState(LANDED);
                printf("Finished landing proceedure waiting for touchdown\n");
                fflush(stdout);
                break;
            case LANDED:
                //do nothing
                break;
        }
        counter++;

        if (counter >= 10)
        {
            changeState(INVERTER);
        }
        if (counter >= 4500000)
        {
            if (state == COMPLETE)
            {
                changeState(LANDING);
                printf("triggered retrusion through timeout\n");
            }
        }
        nanosleep( &loopDelay, NULL);
    }
}
