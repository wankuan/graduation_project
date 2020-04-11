#include <pthread.h>

#include "tank_pub.h"
#include "tank_msgq.h"
#include "tank_mm.h"
#include "tank_map.h"
#include <pthread.h>
// tank_status_t tank_msgq_recv(tank_msgq_t *handler, uint8_t *msg, uint16_t len);
// tank_status_t tank_msgq_send(tank_msgq_t *handler, uint8_t *msg, uint16_t len);

#include "tank_log_api.h"
#define FILE_NAME "msgq_test"

uint8_t send_buf[20][20];
// GDB查看內存方法 -exec x/20xw 0xf7db4000
void *get_mm_start(void)
{
    const char *name = "masg";
    int fd = shm_open(name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    printf("fd:%d\n", fd);

    printf("map_size:%d\n", SHM_SIZE);
    ftruncate(fd, SHM_SIZE);
    void *buf = mmap(NULL, SHM_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
    if (!buf) {
        printf("mmap failed\n");
        close(fd);
        exit(1);
    }
    printf("addr:%p\n", buf);
    // munmap(buf, SHM_SIZE);
    close(fd);
    return buf;
}
tank_mm_t mm;

tank_msgq_t *msgq;

void *send(void *arg)
{
    printf("send running!\n");
    while(1){
        for(int i=0;i<10;i++){
            printf("SEND ID:%d\n", i);
            tank_msgq_send(msgq, (uint8_t*)send_buf[i], 20);
        }
        sleep(1);
    }
}


void *recv(void *arg)
{
    printf("recv running!\n");
    uint8_t buf[20];
    while(1){
        tank_msgq_recv_wait(msgq, buf, 20);
    }
}

int main(int argc, char *argv[])
{

    tank_log_init(&mylog, "msgq",2048, LEVEL_DEBUG,
                LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                PORT_FILE
                );
    pthread_t p_pid;
    for(int i=0;i<20;i++){
        send_buf[i][0]=i;
        for(int j=1;j<20;j++){
            send_buf[i][j]=0x11;
        }
    }
    uint16_t len = 5;
    uint16_t size = 20;
    uint32_t p = (uint32_t)get_mm_start();
    tank_mm_register(&mm, p, 1024, "test_mm");

    msgq = (tank_msgq_t*)tank_mm_malloc(&mm, sizeof(tank_msgq_t)+len*size);
    tank_msgq_creat(msgq, size, len);

    pthread_create(&p_pid,NULL,&send,NULL);
    pthread_create(&p_pid,NULL,&recv,NULL);
    // char buf[20];
    // sleep(2);
    // while(1){
    //     tank_msgq_recv_wait(msgq, buf, 20);
    //     printf("recv:%s\n", buf);
    // }
    pthread_join(p_pid,NULL);

    // for(uint8_t i=5;i>0;i--){
    //     tank_msgq_send(msgq, (uint8_t*)send_buf[i], strlen(send_buf[i])+1);
    //     printf("write:%s\n", send_buf[i]);
    // }
    // for(uint8_t i=0;i<6;i++){
    //     tank_msgq_recv(msgq, (uint8_t*)buf[i], 20);
    //     printf("recv:%s\n", buf[i]);
    // }
    // tank_msgq_delete(msgq);
    // munmap(buf, SHM_SIZE);
    return 0;
}