#include <stdio.h>
#include "thermistor.h"

int main(void) {

    while (1) {
        double t1 = readTMP(0);
        double t2 = readTMP(1);
        double t3 = readTMP(2);

        printf("t1 = %d\n", t1);
        printf("t2 = %d\n", t2);
        printf("t3 = %d\n", t3);
        
        sleep(1);

    }

}

