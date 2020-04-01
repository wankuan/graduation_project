#include <pthread.h>

#include "tank_pub.h"
#include "tank_msgq.h"

// tank_status_t tank_msgq_recv(tank_msgq_t *handler, uint8_t *msg, uint16_t len);
// tank_status_t tank_msgq_send(tank_msgq_t *handler, uint8_t *msg, uint16_t len);

char send_buf[6][20] = {
    "test1",
    "test2",
    "test3",
    "test4",
    "test5",
    "test6"
};

char buf[100][20];
int main(int argc, char *argv[])
{
    tank_msgq_t msgq;
    tank_msgq_creat(&msgq, 5);
    for(uint8_t i=0;i<6;i++){
        tank_msgq_send(&msgq, (uint8_t*)send_buf[i], strlen(send_buf[i]));
        printf("write:%s\n", send_buf[i]);
    }

    for(uint8_t i=0;i<6;i++){
        tank_msgq_recv(&msgq, (uint8_t*)buf[i], 20);
        printf("recv:%s\n", buf[i]);
    }

    for(uint8_t i=5;i>0;i--){
        tank_msgq_send(&msgq, (uint8_t*)send_buf[i], strlen(send_buf[i]));
        printf("write:%s\n", send_buf[i]);
    }
    for(uint8_t i=0;i<6;i++){
        tank_msgq_recv(&msgq, (uint8_t*)buf[i], 20);
        printf("recv:%s\n", buf[i]);
    }
    // tank_msgq_delete(&msgq);
    return 0;
}