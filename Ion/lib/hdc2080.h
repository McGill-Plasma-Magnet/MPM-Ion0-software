#include <stdio.h>
#include <stdint.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<unistd.h>
#include<time.h>

#define I2C_DEV "/dev/i2c-1"
#define HDC2080_ADDR 0x41

//define register addresses
#define TEMP_LSB 0x00
#define TEMP_MSB 0x01
#define HUMIDITY_LSB 0x02
#define HUMIDITY_MSB 0x03
#define DRDY 0x04 //read interrupts states
#define INT_EN 0x07 //set to 0 because I don
#define CONFIG 0x0E
#define MEASURE 0x0F

#define MODE 0x50 //set measure frequency to 1Hz
#define RESOLUTION 0x00 //14 bit res and measure temp + humidity
#define NUM_OF_POLLING_LOOPS 10

char hdcBegin();
int initHDC(); //write wanted default configuration to register
int hdcRST();
int setMODE(char byte);
int setResolution(char byte);
int startMeasurementHDC(); //do not use when in auto mode
float readTempHDC(); //only gets temp 
float readHumidity(); //only gets humidity
int heaterEN(int time);
