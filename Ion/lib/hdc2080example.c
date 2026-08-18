#include <stdio.h>
#include <unistd.h>

#include "hdc2080.h"

int main(void)
{
    if (!initHDC()) {
        fprintf(stderr, "Failed to initialize HDC2080\n");
        return 1;
    }

    /* Manual measurement mode */
    if (!setMODE(0x00)) {
        fprintf(stderr, "Failed to set measurement mode\n");
        return 1;
    }

    /* 14-bit temperature + humidity */
    if (!setResolution(RESOLUTION)) {
        fprintf(stderr, "Failed to set resolution\n");
        return 1;
    }

    for (int i = 0; i < 10; i++) {

        /* Must happen BEFORE readTempHDC()/readHumidity() */
        if (!startMeasurementHDC()) {
            fprintf(stderr, "Measurement failed\n");
            sleep(1);
            continue;
        }

        float temperature = readTempHDC();
        float humidity = readHumidity();

        printf("Temperature: %.2f C | Humidity: %.2f %%RH\n",
               temperature, humidity);

    }
    printf("testind heater\n");
    heaterEN(1000);

    return 0;
}
