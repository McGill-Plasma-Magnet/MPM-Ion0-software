#include "hdc2080.h"

struct timespec poll_delay = {
    .tv_sec = 0,
    .tv_nsec = 450000
};

struct timespec heat_delay = {
    .tv_sec = 0,
    .tv_nsec = 1
};

static int file;

//define default state
static char config[2] = {CONFIG, 0x00};
static char measure[2] = {MEASURE, 0x00};

char hdcBegin ()
{
    if ((file=open(I2C_DEV, O_RDWR)) < 0) {
        perror("failed to open bus\n");
        return 0;
    }
    if (ioctl(file, I2C_SLAVE, HDC2080_ADDR) < 0)
    {
        perror("Failed to connect to the sensor\n");
        close(file);
        return 0;
    }
    return 1;
}

int initHDC ()
{
    if (!hdcBegin()) return 0;

    if (write(file, config, 2) != 1)
    {
        perror("failed to write config register");
        close(file);
        return 0;
    }

    if (write(file, &measure, 2) != 1)
    {
        perror("failed to write measure register");
        close(file);
        return 0;
    }

    close(file);
    return 1;
}

int hdcRST ()
{
    if (!hdcBegin()) return 0;

    config[1] = 0x80;
    if (write(file, config, 2) != 1)
    {
        perror("failed to soft reset sensor");
        close(file);
        return 0;
    }
    close(file);
    return 1;
}

int setMODE(char byte)
{
    if (!hdcBegin()) return 0;
    
    config[1] = config[1] & 0x0F; //reset mode only 
    config[1] = config[1] | byte; //this assumes the user puts a valid mode in format 0b0xxx0000

    if (write(file, config, 2) != 1)
    {
        perror("failed to set mode");
        close(file);
        return 0;
    }
    close(file);
    return 1;
}

int setResolution(char byte)
{
    if (!hdcBegin()) return 0;
    
    measure[1] = byte;
    if (write(file, measure, 2) != 1)
    {
        perror("failed to set resolution");
        close(file);
        return 0;
    }
    
    close(file);
    return 1;
}

int startMeasurementHDC ()
{
    if (!hdcBegin()) return 0;

    measure[1] = measure[1] | 0x01;
    if (write(file, measure, 2) != 1)
    {
        perror("failed to trigger measurement");
        close(file);
        return 0;
    }
    uint8_t drdy = DRDY;
    if (write(file, drdy, 1) != 1)
    {
        perror("failed to write DRDY address");
        close(file);
        return 0;
    }
    char buf;
    int i;
    for (i = 0; i < NUM_OF_POLLING_LOOPS; i++)
    {
        if (read(file, &buf, 1) != 1){
            perror("failed to read DRDY");
            close(file);
            return 0;
        }
        if (buf & 0x80) //check if data is ready 
        {
            close(file);
            return 1;
        }
        nanosleep( &poll_delay, NULL);
    }
    perror("Conversion was not completed");
    close(file);
    return 0;
}

float readTempHDC()
{

    if (!hdcBegin()) return 0;

    //variables will hold the temperature data, also used as temporary holders of their respective regisiter address 
    char templsb = TEMP_LSB;
    char tempmsb = TEMP_MSB;

    if (write(file, templsb, 1) != 1)
    {
        perror("failed to write temp lsb register");
        close(file);
        return 0;
    }
    if (read(file, templsb, 1) != 1)
    {
        perror("failed to read temp lsb");
        close(file);
        return 0;
    }
    if (write(file, tempmsb, 1) != 1)
    {
        perror("failed to write temp msb register");
        close(file);
        return 0;
    }
    if (read(file, tempmsb, 1) != 1)
    {
        perror("failed to read temp msb");
        close(file);
        return 0;
    }
    close(file);
    uint16_t rawTemp = ((uint16_t)tempmsb << 8) | templsb;
    float temp = ((rawTemp/65536)*165 - 40.62f);
    return temp;
}

float readHumidity ()
{

    if (!hdcBegin()) return 0;

    //variables will hold the humidity data, also used as temporary holders of their respective regisiter address 
    char humiditylsb = HUMIDITY_LSB;
    char humiditymsb = HUMIDITY_MSB;

    if (write(file, humiditylsb, 1) != 1)
    {
        perror("failed to write humidity lsb register");
        close(file);
        return 0;
    }
    if (read(file, humiditylsb, 1) != 1)
    {
        perror("failed to read humidity lsb");
        close(file);
        return 0;
    }
    if (write(file, humiditymsb, 1) != 1)
    {
        perror("failed to write humidity msb register");
        close(file);
        return 0;
    }
    if (read(file, humiditymsb, 1) != 1 )
    {
        perror("failed to read humidity msb");
        close(file);
        return 0;
    }
    close(file);
    uint16_t rawHumidity = ((uint16_t)humiditymsb << 8) | humiditylsb;
    float humidity = ((rawHumidity/65536)*100.0f);
    return humidity;
}
int heaterEN (int time )
{
    heat_delay.tv_nsec = time * 1000000L;
    if (!hdcBegin()) return 0;
    
    config[1] = config[1] | 0x08;
    if (write(file, config, 2) != 1)
    {
        perror("failed to enable heater");
        close(file);
        return 0;
    }
    close(file);

    int startMeasurementHDC();
    float temp = readTempHDC();
    printf("temprature is %f\n", temp);
    nanosleep(&heat_delay, NULL);
    int startMeasurementHDC();
    temp = readTempHDC();
    printf("temprature is %f\n", temp);

    config[1] = config[1] ^ 0x08;
    if (write(file, config, 2) != 1)
    {
        perror("failed to disable heater");
        close(file);
        return 0;
    }

    close(file);
    return 1;
}
