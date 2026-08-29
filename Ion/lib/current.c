#include "current.h"

#define ADC_VREF     1.8f
#define ADC_COUNTS   4094.0f   // 12-bit
#define R_SHUNT      0.010f
#define R_IN         120.0f
#define R_FB         2200.0f
#define GAIN         (1.0f + (R_FB / R_IN))   // 19.33


static int zeroOffsetRaw = 0;

double setOffset()  
{  
    int fd;          //file pointer  
    int pin = 6;
    char buf[MAX_BUF];     //file buffer  
    char val[6];     //holds up to 4 digits for ADC value  

    //Create the file path by concatenating the ADC pin number to the end of the string  
    //Stores the file path name string into "buf"  
    snprintf(buf, sizeof(buf), "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw", pin);     //Concatenate ADC file name  

    fd = open(buf, O_RDONLY);     //open ADC as read only  

    //Will trigger if the ADC is not enabled  
    if (fd < 0) {  
        perror("ADC - problem opening ADC");  
    }//end if  

    read(fd, &val, 6);     //read ADC ing val (up to 4 digits 0-4093)  
    close(fd);     //close file and stop reading  

    zeroOffsetRaw = atoi(val);
    return temp;
}//end read ADC()

double readCurrent()  
{  
    int fd;          //file pointer  
    int pin = 6;
    char buf[MAX_BUF];     //file buffer  
    char val[6];     //holds up to 4 digits for ADC value  

    //Create the file path by concatenating the ADC pin number to the end of the string  
    //Stores the file path name string into "buf"  
    snprintf(buf, sizeof(buf), "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw", pin);     //Concatenate ADC file name  

    fd = open(buf, O_RDONLY);     //open ADC as read only  

    //Will trigger if the ADC is not enabled  
    if (fd < 0) {  
        perror("ADC - problem opening ADC");  
    }//end if  

    read(fd, &val, 6);     //read ADC ing val (up to 4 digits 0-4093)  
    close(fd);     //close file and stop reading  

    int corrected = atoi(val) - zero_offset_raw;
    float v_out   = (corrected / ADC_COUNTS) * ADC_VREF;
    float v_shunt = v_out / GAIN;
    return v_shunt / R_SHUNT;   // amps
}//end read ADC()

