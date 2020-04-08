#include "tank_socket_pub.h"
#include "tank_socket.h"
#include "pthread.h"
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

int len = sizeof(name_buf)/sizeof(name_buf[0]);

void *test1(void *arg)
{
    while(1){
        for(int i=0;i<len;++i){
        tank_socket_creat(&app[i], name_buf[i], 0, 0);
        printf("\n\n\n");
        }
        sleep(5);
    }
}
void *test2(void *arg)
{
    while(1){
        for(int i=0;i<len;++i){
        tank_socket_creat(&app[i], name_buf[i], 0, 0);
        }
        sleep(1);
    }
}

void *test3(void *arg)
{
    while(1){
        for(int i=0;i<len;++i){
        tank_socket_creat(&app[i], name_buf[i], 0, 0);
        }
        sleep(1);
    }
}


int main(int argc, char *argv[])
{


    pthread_t my_pid_t;
    pthread_create(&my_pid_t,NULL,&test1,NULL);
    // pthread_create(&my_pid_t,NULL,&test2,NULL);
    // pthread_create(&my_pid_t,NULL,&test3,NULL);

    pthread_join(my_pid_t,NULL);
    printf("end running\n");
    return 0;
}
