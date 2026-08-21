#include <motors.h>
#include <time.h>

struct timespec delay = {
    .tv_sec = 0,
    .tv_nsec = 400000
};

int main()
{
    initMotor2();
    while(1)
    {
        motor2ON(1);
        sleep(5);
        motor2OFF();
        sleep(5);
    }
}

