#include "ms5607.h"

static const float P0 = 1011.25;
static short OSR = 4096; //default and highest resolution config
static short CONV_D1 = 0x48; //CONV for pressure at max resolution
static short CONV_D2 = 0x58; //conversion for temprature at max resolution
static char Conv_Delay = 10; //give time for conversion

static int file;
static uint16_t C1, C2, C3, C4, C5, C6;
static uint32_t DP, DT;
static float dT, TEMP, P;
static int64_t OFF, SENS;


struct timespec delay = {
    .tv_sec = 0,
    .tv_nsec = 500000000
};

struct timespec conv_delay = {
    .tv_sec = 0,
    .tv_nsec = 10000000L
};

char msBegin ()
{
    if ((file=open(I2C_DEV, O_RDWR)) < 0) {
        perror("failed to open bus\n");
        return 0;
    }
    if (ioctl(file, I2C_SLAVE, MS5607_ADDR) < 0)
    {
        perror("Failed to connect to the sensor\n");
        close(file);
        return 0;
    }
    return 1;
}

char resetDevice()
{
    if (!msBegin()) return 0;

    uint8_t cmd = RST;
    if (write(file, &cmd, 1)!=1)
    {
        perror("failed to resest sensor\n");
        return 0; //error
    }
    nanosleep(&delay, NULL); //delay of 1s to give time for the sensor to restart
    close(file);
    return(1); //success
}

char readCalibration()
{
    if (resetDevice() && 
            readUInt_16(R_PROM+2, &C1) &&
            readUInt_16(R_PROM+4, &C2) &&
            readUInt_16(R_PROM+6, &C3) &&
            readUInt_16(R_PROM+8, &C4) &&
            readUInt_16(R_PROM+10, &C5) &&
            readUInt_16(R_PROM+12, &C6)
       ){
        return 1;
    }else {return 0;}
}

char readUInt_16(uint8_t address, uint16_t *value)
{
    unsigned char data[2];
    data[0] = address;
    if (readBytes(data,2))
    {
        *value = (((uint16_t)data[0]*(1<<8))|(uint16_t)data[1]);
		return(1);
    }
    *value = 0;
    return 0;
}

char readBytes( unsigned char *values, int length)
{
    if (!msBegin()) return 0;
    
    if (write(file, values[0], 1) != 1) 
    {
        perror("failled to send command");
        return 0;
    }
    if (read(file, &values, length) != length)
    {
        perror("could not read data");
        return 0;
    }
    close(file);
    return 1;
}

char startConversion(char CMD)
{
    if (!msBegin()) return 0;
    if (write(file, &CMD, 1) != 1)
    {
        perror("failed to send command");
        return 0;
    }
    nanosleep(&conv_delay, NULL);
    close(file);
    return 1;
}

char startMeasurment(void)
{
    if (!msBegin()) return 0;
    uint8_t cmd = R_ADC;
    if (write(file, &cmd, 1) != 1){
        perror("failed to read ADC");
        return 0;
    }
    return 1;
}

char readDigitalValue()
{
    if (!startConversion(CONV_D1))
        return 0;
    if (!startMeasurment())
        return 0;
    if (!getDigitalValue(&DP))
        return 0;

    if (!startConversion(CONV_D2))
        return 0;
    if (!startMeasurment())
        return 0;
    if (!getDigitalValue(&DT))
        return 0;

    return 1;
}
char getDigitalValue(uint32_t *value)
{
    char length =3;
    unsigned char data[3];
    if (read(file, data, length) != length)
    {
        perror("failed to read data from file");
        return 0;
    }
    *value = (uint64_t)data[0]*1<<16|(uint64_t)data[1]*1<<8|(uint64_t)data[2];
    close(file);
    return 1;
}

static void calculateCompensation(void)
{
    int64_t dt;
    int64_t temp;
    int64_t off;
    int64_t sens;

    int64_t T2 = 0;
    int64_t OFF2 = 0;
    int64_t SENS2 = 0;

    dt = (int64_t)DT - ((int64_t)C5 << 8);

    temp = 2000 + (dt * (int64_t)C6) / (1LL << 23);

    off = ((int64_t)C2 << 17)
        + ((int64_t)C4 * dt) / (1LL << 6);

    sens = ((int64_t)C1 << 16)
         + ((int64_t)C3 * dt) / (1LL << 7);

    /* Second-order compensation below 20 C */
    if (temp < 2000)
    {
        int64_t t;

        T2 = (dt * dt) / (1LL << 31);

        t = temp - 2000;

        OFF2  = (61 * t * t) / (1LL << 4);
        SENS2 = 2 * t * t;

        /* Additional compensation below -15 C */
        if (temp < -1500)
        {
            t = temp + 1500;

            OFF2  += 15 * t * t;
            SENS2 += 8 * t * t;
        }
    }

    temp -= T2;
    off  -= OFF2;
    sens -= SENS2;

    TEMP = (float)temp;

    P = (float)(
        ((((int64_t)DP * sens) / (1LL << 21)) - off)
        / (1LL << 15)
    );
}

float getTemprature()
{
    calculateCompensation();
    return TEMP/100 ;
}
    

float getPressure(void)
{
    calculateCompensation();
    return P/100;
}


// set OSR and select corresponding values for conversion commands & delay
void setOSR(short OSR_U){
    OSR = OSR_U;
    switch (OSR) {
        case 256:
            CONV_D1 = 0x40;
          CONV_D2 = 0x50;
          Conv_Delay = 1;
          conv_delay.tv_nsec = Conv_Delay * 1000000L;
          break;
      case 512:
          CONV_D1 = 0x42;
          CONV_D2 = 0x52;
          Conv_Delay = 2;
          conv_delay.tv_nsec = Conv_Delay * 1000000L;
          break;
      case 1024:
          CONV_D1 = 0x44;
          CONV_D2 = 0x54;
          Conv_Delay = 3;
          conv_delay.tv_nsec = Conv_Delay * 1000000L;
          break;
      case 2048:
          CONV_D1 = 0x46;
          CONV_D2 = 0x56;
          Conv_Delay = 5;
          conv_delay.tv_nsec = Conv_Delay * 1000000L;
          break;
      case 4096:
          CONV_D1 = 0x48;
          CONV_D2 = 0x58;
          Conv_Delay = 10;
          conv_delay.tv_nsec = Conv_Delay * 1000000L;
          break;
      default:
          CONV_D1 = 0x40;
          CONV_D2 = 0x50;
          Conv_Delay = 1;
          conv_delay.tv_nsec = Conv_Delay * 1000000L;
          break;
    }
  }
