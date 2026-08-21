#include <motors.h>
#include <time.h>

struct timespec delay = {
    .tv_sec = 0,
    .tv_nsec = 400000
};

int main()
{
    if (initMotor2())
    {
        perror("error initialising motor 2");
    }

    while(1)
    {
        motor2ON(1);
        sleep(5);
        motor2OFF();
    }
}

