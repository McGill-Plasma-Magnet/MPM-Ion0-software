#include <motors.h>

static struct gpiod_chip *chip1;
static struct gpiod_chip *chip2;
static struct gpiod_line *motor1a;
static struct gpiod_line *motor1b;
static struct gpiod_line *motor2a;
static struct gpiod_line *motor2b;

int initMotor1()
{
    chip1 = gpiod_chip_open("/dev/gpiochip1");

    motor1a = gpiod_chip_get_line(chip1, 0);
    motor1b = gpiod_chip_get_line(chip1, 4);

    gpiod_line_request_output(motor1a, "motor", 0);
    gpiod_line_request_output(motor1b, "motor", 0);
    return 0;
}

int initMotor2()
{
    chip2 = gpiod_chip_open("/dev/gpiochip2");

    motor2a = gpiod_chip_get_line(chip2, 22);
    motor2b = gpiod_chip_get_line(chip2, 23);

    gpiod_line_request_output(motor2a, "motor", 0);
    gpiod_line_request_output(motor2b, "motor", 0);
    return 0;
}

void motor1ON(int dir)
{
    switch (dir)
    {
        case 0:
            gpiod_line_set_value(motor1a, 1);
            gpiod_line_set_value(motor1b, 0);
            break;
        case 1:
            gpiod_line_set_value(motor1a, 0);
            gpiod_line_set_value(motor1b, 1);
            break;
        default:
            perror("Please input a valid direction");
            break;
    }
}

void motor2ON(int dir)
{
    switch (dir)
    {
        case 0:
            gpiod_line_set_value(motor2a, 1);
            gpiod_line_set_value(motor2b, 0);
            break;
        case 1:
            gpiod_line_set_value(motor2a, 0);
            gpiod_line_set_value(motor2b, 1);
            break;
        default:
            perror("Please input a valid direction");
            break;
    }
}

void motor1OFF()
{
    gpiod_line_set_value(motor1a, 0);
    gpiod_line_set_value(motor1b, 0);
}

void motor2OFF()
{
    gpiod_line_set_value(motor2a, 0);
    gpiod_line_set_value(motor2b, 0);
}
