#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "thermistor.h"

#define PROB 1 //define probe used to measure battery temp.
#define EXPORT "/sys/class/pwm/pwmchip4/export"
#define PERIOD "/sys/class/pwm/pwmchip4/pwm1/period"
#define DUTY_CYCLE "/sys/class/pwm/pwmchip4/pwm1/duty_cycle"
#define ENABLE "/sys/class/pwm/pwmchip4/pwm1/enable"

static FILE *fpwm, *fperiod, *fduty, *fen;

int initBAT() {
    //open and enable pwm if it is not already
    fpwm = fopen(EXPORT, "w");
    fprintf(fpwm, "1");
    fclose(fpwm);

    fperiod = fopen(PERIOD, "w"); //set pwm period
    fduty = fopen(DUTY_CYCLE, "w"); //set pwm duty cycle by writing to this file
    fen = fopen(ENABLE, "w"); //enable the PWM by writting 1 to the file

    fprintf(fperiod, "1000000"); //set frequency to 1kHz
    fprintf(fduty, "0"); //set frequency to 1kHz
    fprintf(fen, "1"); //set frequency to 1kHz
    fclose(fperiod);
    fclose(fen);
    return 0;
}


int main() {
    if (initBAT() != 0) {
        fprintf(stderr, "can't initialize PWM\n");
        return 1;
    }

    double batTMP;

    while(1) {
        batTMP = readTMP(PROB);
        if (batTMP < -80.0) {
            perror("prob is disconnected");
            return -1;
        }
        else if ( batTMP >= 25.0) {
            fprintf(fduty, "0");
            fflush(fduty);
        }
        else if ( batTMP >= 5.0) {
            fprintf(fduty, "500000");
            fflush(fduty);
        }
        else if ( batTMP < 5.0) {
            fprintf(fduty, "1000000");
            fflush(fduty);
        }
    }

    fclose(fduty);
    return 0;
}

