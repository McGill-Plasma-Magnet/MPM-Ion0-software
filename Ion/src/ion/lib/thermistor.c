#include "thermistor.h"

//Function definitions  
double readTMP(unsigned int pin)  
{  
    int fd;          //file pointer  
    char buf[MAX_BUF];     //file buffer  
    char val[4];     //holds up to 4 digits for ADC value  

    //Create the file path by concatenating the ADC pin number to the end of the string  
    //Stores the file path name string into "buf"  
    snprintf(buf, sizeof(buf), "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw", pin);     //Concatenate ADC file name  

    fd = open(buf, O_RDONLY);     //open ADC as read only  

    //Will trigger if the ADC is not enabled  
    if (fd < 0) {  
        perror("ADC - problem opening ADC");  
    }//end if  

    read(fd, &val, 4);     //read ADC ing val (up to 4 digits 0-4093)  
    close(fd);     //close file and stop reading  

    //calculating temprature
    const float R25 = 10000.0f;
    const float T25 = 298.15f;
    const float B   = 3380.0f;

    int Vraw;
    double Vntc, Rntc, temp;
    Vraw = atoi(val);     //returns an integer value (rather than ascii)  
    Vntc = Vraw * 1.8 / 4094.0;
    Rntc = Vntc * 10000.0f / (1.8f - Vntc);
    temp = (1.0 / ((1.0 / T25) + (log(Rntc / R25) / B)))- 273.15;
    return temp;
}//end read ADC()

