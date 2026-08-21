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
    if (!chip1)
    {
        perror("initMotor1: failed to open /dev/gpiochip1");
        return -1;
    }

    motor1a = gpiod_chip_get_line(chip1, 0);
    motor1b = gpiod_chip_get_line(chip1, 4);
    if (!motor1a || !motor1b)
    {
        fprintf(stderr, "initMotor1: failed to get GPIO lines\n");
        gpiod_chip_close(chip1);
        chip1 = NULL;
        return -1;
    }

    if (gpiod_line_request_output(motor1a, "motor", 0) < 0)
    {
        perror("initMotor1: failed to request motor1a as output");
        gpiod_chip_close(chip1);
        chip1 = NULL;
        return -1;
    }

    if (gpiod_line_request_output(motor1b, "motor", 0) < 0)
    {
        perror("initMotor1: failed to request motor1b as output");
        gpiod_line_release(motor1a);
        gpiod_chip_close(chip1);
        chip1 = NULL;
        return -1;
    }

    return 0;
}

int initMotor2()
{
    chip2 = gpiod_chip_open("/dev/gpiochip2");
    if (!chip2)
    {
        perror("initMotor2: failed to open /dev/gpiochip2");
        return -1;
    }

    motor2a = gpiod_chip_get_line(chip2, 22);
    motor2b = gpiod_chip_get_line(chip2, 23);
    if (!motor2a || !motor2b)
    {
        fprintf(stderr, "initMotor2: failed to get GPIO lines\n");
        gpiod_chip_close(chip2);
        chip2 = NULL;
        return -1;
    }

    if (gpiod_line_request_output(motor2a, "motor", 0) < 0)
    {
        perror("initMotor2: failed to request motor2a as output");
        gpiod_chip_close(chip2);
        chip2 = NULL;
        return -1;
    }

    if (gpiod_line_request_output(motor2b, "motor", 0) < 0)
    {
        perror("initMotor2: failed to request motor2b as output");
        gpiod_line_release(motor2a);
        gpiod_chip_close(chip2);
        chip2 = NULL;
        return -1;
    }

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

void endMotor1()
{
    if (motor1a)
    {
        gpiod_line_set_value(motor1a, 0);
        gpiod_line_release(motor1a);
        motor1a = NULL;
    }
    if (motor1b)
    {
        gpiod_line_set_value(motor1b, 0);
        gpiod_line_release(motor1b);
        motor1b = NULL;
    }
    if (chip1)
    {
        gpiod_chip_close(chip1);
        chip1 = NULL;
    }
}

void endMotor2()
{
    if (motor2a)
    {
        gpiod_line_set_value(motor2a, 0);
        gpiod_line_release(motor2a);
        motor2a = NULL;
    }
    if (motor2b)
    {
        gpiod_line_set_value(motor2b, 0);
        gpiod_line_release(motor2b);
        motor2b = NULL;
    }
    if (chip2)
    {
        gpiod_chip_close(chip2);
        chip2 = NULL;
    }
}
