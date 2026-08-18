#include <stdio.h>
#include <unistd.h>
#include "ms5607.h"

int main(void)
{
    /* Read factory calibration coefficients after reset. */
    if (!readCalibration()) {
        fprintf(stderr, "MS5607 calibration read failed\n");
        return 1;
    }

    /* Highest resolution. Try 256, 512, 1024, 2048, or 4096. */
    setOSR(4096);

    while (1) {
        /* Updates the raw pressure (D1) and temperature (D2) values. */
        if (!readDigitalValue()) {
            fprintf(stderr, "MS5607 measurement failed\n");
            return 1;
        }

        printf("Temperature: %.2f C\n", getTemprature());
        printf("Pressure:    %.2f mbar\n\n", getPressure());

        sleep(1);
    }

    return 0;
}
