#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"
#include "tank_msgq.h"
#include <pthread.h>
#include "tank_request.h"

uint32_t shm_base_s = 0;
tank_msgq_t *backstage_msgq;
uint16_t sem_size = sizeof(my_sem_t);
uint16_t msgq_size = sizeof(tank_msgq_t);

void print_all_info(void)
{
    printf("sem:%d, msgq:%d \n", sem_size, msgq_size);
}



void *get_mm_start(void)
{
    const char *name = TANK_PUB_NAME;
    int fd = shm_open(name, O_RDWR, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    printf("fd:%d\n", fd);
    printf("map_size:%d\n", SHM_SIZE);
    void *buf = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (!buf) {
        printf("mmap failed\n");
        close(fd);
        exit(1);
    }
    // munmap(buf, SHM_SIZE);
    close(fd);
    return buf;
}

void *app_socket_test(void *arg)
{
    int sem_val = 0;
    app_heap_get_t *app_get = (app_heap_get_t *)(SEM_ADDR);

    printf("[socket]start send malloc request\n");

    my_sem_get_val(&app_get->sem, &sem_val);
    printf("[socket]wait for backstage allocate finished\n");
    app_info_t app_info;
    app_info.type = MM_ALLOCATE;
    memset(&app_info, 0, sizeof(app_info_t));
    strncpy(app_info.request.name, "app_1", 8);
    app_info.request.size = 256;
    printf("[socket]name:%s, len:%d\n", app_info.request.name, app_info.request.size);
    tank_msgq_send(backstage_msgq, &app_info, sizeof(app_info_t));

    my_sem_wait(&app_get->sem);
    printf("[socket]backstage allocate OK\n");
    printf("[socket]get allocate info\n");

    printf("[socket]id:%d, addr_base:%p, shift:%d\n", app_get->id, (void*)shm_base_s, app_get->shift);

    my_sem_get_val(&app_get->sem, &sem_val);



    return NULL;
}



int main(int argc, char *argv[])
{
    pthread_t p_pid;
    shm_base_s = (uint32_t)get_mm_start();
    printf("str:%s\n", (char *)shm_base_s);
    printf("start addr:%x\n", shm_base_s);
    uint32_t backstage_msgq_addr = *(uint32_t *)MSGQ_MAP_ADDR + shm_base_s;
    backstage_msgq = (tank_msgq_t *)backstage_msgq_addr;
    printf("backstage msgq addr:%p\n", backstage_msgq);
    print_all_info();
    pthread_create(&p_pid,NULL,&app_socket_test,NULL);
    // // pthread_create(&p_pid,NULL,&print2,NULL);

    pthread_join(p_pid,NULL);
    // my_sem_destroy(&sem1);

    printf("addr:%x\n", shm_base_s);
    return 0;
}
