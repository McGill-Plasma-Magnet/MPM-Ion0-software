#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <unistd.h>     //close()  
#include <fcntl.h>     //define O_WONLY and O_RDONLY  
#include <math.h>
#define MAX_BUF 64     //This is plenty large  

double readTMP(unsigned int pin);
