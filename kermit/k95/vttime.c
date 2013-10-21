 

/************************************************************************
 
This program queries the system clock and issues a VT series 
time. 

VT CSI hr ; mm , p

WY ESC c 8 hr mm

TVI ESC SP 1 ampm hr min
************************************************************************/
 

#include <time.h>
#include <stdio.h>
 
int main()
{
    struct tm *newtime;
    time_t ltime;

    printf("Setting VT terminal clock ...\n"); 
    time(&ltime);
    newtime = localtime(&ltime);
    printf("Time now: %d:%.2d:%.2d\n",newtime->tm_hour,         
               newtime->tm_min,newtime->tm_sec);
    if ( newtime->tm_sec != 0 ) {
	sleep( 60 - newtime->tm_sec ) ;
        newtime->tm_min += 1 ;
    }
    printf("%c[%d;%d,p",27,newtime->tm_hour,newtime->tm_min);
    printf("Time set to %d:%.2d:00\n",newtime->tm_hour,newtime->tm_min);
    return 0;
 }
 

