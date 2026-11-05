#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <unistd.h>     //close()  
#include <fcntl.h>     //define O_WONLY and O_RDONLY  

#define MAX_BUF 64     //This is plenty large  

void setOffset();
double readCurrent();
