#include "tank_delay.h"
#include <sys/select.h>

void sleep_ms(unsigned int secs)
{
    struct timeval tval;
    tval.tv_sec=secs/1000;
    tval.tv_usec=(secs*1000)%1000000;
    while(select(1,NULL,NULL,NULL,&tval)!=0){
        // printf("select to be interrupt\n");
    }
}
