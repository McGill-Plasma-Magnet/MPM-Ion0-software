#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <time.h>

//set inital state of the stepper based on array values
// [MS0, MS1, MS2, RST, SLP, EN, DIR]
// as well as define pins in though gpoid.h
int initStp(const int pin[7]);
void driveStp();
void stpSWDir();
void stpSetSlp(int sleep);
void stpSlp(int time);
void extrudeTape(int steps);



