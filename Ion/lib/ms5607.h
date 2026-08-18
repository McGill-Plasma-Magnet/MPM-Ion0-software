#include <stdio.h>
#include <stdint.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<unistd.h>
#include<time.h>

#define MS5607_ADDR 0x77
#define I2C_DEV "/dev/i2c-1"
#define R_ADC 0x00 //read adc command
#define R_PROM 0xA0
#define RST 0x1E


//function used to interface with sensor
char msBegin();
void setOSR(short OSR_U);
float getTemprature();
float getPressure();
char readDigitalValue();

//Internel functions
char resetDevice(void);
char readCalibration();
char readUInt_16(uint8_t address, uint16_t *value);
char readBytes(unsigned char *values, int length);
char startConversion(char CMD);
char startMeasurment(void);
char getDigitalValue(uint32_t *value);
static void calculateCompensation(void);
