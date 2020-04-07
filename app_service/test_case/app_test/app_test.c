#include "tank_socket_pub.h"
#include "tank_socket.h"

ts_info_t app[10];


char name_buf[10][8]={
    "app-0",
    "app-1",
    "app-2",
    "app-3",
    "app-4",
    "app-5",
    "app-6",
    "app-7",
    "app-8",
    "app-9",
};

int main(int argc, char *argv[])
{
    int len = sizeof(name_buf)/sizeof(name_buf[0]);
    for(int i=0;i<len;++i){
        tank_socket_creat(&app[i], name_buf[i], 0, 0);
        sleep(1);
    }
    printf("end running\n");
    return 0;
}
