#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "stepper.h"
#include "ms5607.h"

#define STATE_PATH "/home/debian/logs/state.txt"

struct timespec loopDelay = {
    .tv_sec = 0,
    .tv_nsec = 100000000
};

struct timespec missionDelay = {
    .tv_sec = 300,
    .tv_nsec = 0
};

typedef enum{
    PRELAUNCH,
    ASCENT,
    EXTRUSION_1,
    DELAY,
    EXTRUSION_2,
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
                if (Pressure <= 1000.0f && Pressure != 0.0f && pressureValid)
                {
                    if (++thresholdCount >= 5)
                    {
                        changeState(ASCENT);
                        printf("Detected Ascent changing state");
                        thresholdCount = 0;
                    }else{
                       thresholdCount = 0;
                    }
                }
                break;
            case ASCENT:
                    if (Pressure <= 110.0f && Pressure != 0.0f && pressureValid)
                    {
                        if (++thresholdCount >= 5)
                        {
                            changeState(EXTRUSION_1);
                            printf("Initiation first Extrusion\n");
                            thresholdCount = 0;
                        }else{
                            thresholdCount = 0;
                        }
                    }
                    break;
            case EXTRUSION_1:
                    stpSetSlp(1);
                    printf("started first extrusion\n");
                    extrudeTape(10000);
                    printf("completed first extrusion\n");
                    stpSetSlp(0);
                    changeState(DELAY);
                    break;
            case DELAY:
                    nanosleep( &missionDelay, NULL);
                    printf("going into delay\n");
                    changeState(EXTRUSION_2);
                    break;
            case EXTRUSION_2:
                    stpSetSlp(1);
                    printf("started second extrusion\n");
                    extrudeTape(10000);
                    printf("completed second extrusion\n");
                    stpSetSlp(0);
                    changeState(COMPLETE);
                    break;
            case COMPLETE:
                    if (Pressure >= 600.0f && Pressure != 0.0f && pressureValid)
                    {
                        if (++thresholdCount >= 5)
                        {
                            changeState(LANDING);
                            printf("started retrusion of loops\n");
                            thresholdCount = 0;
                        }else{
                            thresholdCount = 0;
                        }
                    }
                    break;
            case LANDING:
                    stpSetSlp(1);
                    for (int i = 0; i < 100; i++)
                    {
                        extrudeTape(200);
                        stpSWDir();
                        extrudeTape(100);
                        stpSWDir();
                    }
                    stpSetSlp(0);
                    changeState(LANDED);
                    printf("Finished landing proceedure waiting for touchdown\n");
                    fflush(stdout);
                    endStepper(initial_state);
                    break;
            case LANDED:
                    //do nothing
                    break;
        }
        counter++;
        if (counter >= 24000)
        {
            if (state == PRELAUNCH || state == ASCENT)
            {
                changeState(EXTRUSION_1);
                printf("triggered extrusion 1 through timeout\n");
                    fflush(stdout);
            }
            if (counter >= 57000)
            {
                if (state == COMPLETE)
                {
                    changeState(LANDING);
                    printf("triggered retrusion through timeout\n");
                    fflush(stdout);
                }
            }
        }
        nanosleep( &loopDelay, NULL);
    }
}
