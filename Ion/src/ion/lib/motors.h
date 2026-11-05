#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>

int initMotor1();
int initMotor2();
void motor1ON(int dir);
void motor2ON(int dir);
void motor1OFF();
void motor2OFF();

