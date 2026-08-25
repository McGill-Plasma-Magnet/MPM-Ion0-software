#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <errno.h>
#include "ms5607.h"
#include "thermistor.h"
#include "hdc2080.h"

#define DIR_PATH "/home/debian/logs/"
#define DATA_BUF 8192

char setupLogs();
char createFile();
char logData();
char logMessage(const char* message);
char flushBuffer();
void flushClose();
