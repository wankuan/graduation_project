#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"
#include "tank_msgq.h"
#include <pthread.h>
#include "tank_request.h"

app_allocate_info_t msgq_map_s[256];
uint16_t app_id_seq_s = 0;
tank_mm_t in_swap_mm_s;

void *app_malloc(void *arg)
{
    app_info_t request;
    tank_msgq_t *request_msgq;

    app_heap_get_t *app_get = (app_heap_get_t *)SEM_ADDR;

    printf("sem addr:%p  size:%d\n", &app_get->sem, sizeof(my_sem_t));

    sem_destroy(&app_get->sem);
    my_sem_creat(&app_get->sem, 0);

    request_msgq = (tank_msgq_t*)tank_mm_malloc(&in_swap_mm_s, sizeof(tank_msgq_t)+20*20);
    printf("msgq addr:%p\n", request_msgq);

    *(uint32_t*)MSGQ_MAP_ADDR = (uint32_t)request_msgq - shm_base_s;

    tank_msgq_creat(request_msgq, 20, 20);


    while(1){
        // while(tank_msgq_recv(request_msgq, &request, 20)==TANK_FAIL){
        //     sleep(1);
        //     printf("wait for meessage from app\n");
        // }
        tank_msgq_recv_wait(request_msgq, &request, 20);
        if(request.type == MM_ALLOCATE){
            printf("[app]start malloc\n");
            void *addr = tank_mm_alloc(&in_swap_mm_s, request.request.size);

            uint32_t shift = (uint32_t)addr - shm_base_s;

            printf("[app]name:%s, size:%d\n", request.request.name, request.request.size);

            msgq_map[msgq_id_seq].id = msgq_id_seq;
            msgq_map[msgq_id_seq].shift = shift;
            strncpy(msgq_map[msgq_id_seq].name, request.request.name, 8);

            // memset(app_get, 0, sizeof(app_heap_get_t));
            app_get->shift = shift;
            app_get->id = msgq_id_seq;
            msgq_id_seq += 1;
            get_socket_info();
            printf("[app]exit, malloc OK\n");
            int sem_val;
            my_sem_get_val(&app_get->sem, &sem_val);
            my_sem_post(&app_get->sem);
            my_sem_get_val(&app_get->sem, &sem_val);
        }
    }
    return NULL;
}

void get_socket_info(void)
{
    for(int i=0;i<msgq_id_seq;i++)
    {
        printf("id:%d, name:%s, shift:%d\n", msgq_map[i].id, msgq_map[i].name, msgq_map[i].shift);
    }
}



int main(int argc, char *argv[])
{
    pthread_t p_pid;
    shm_base_s = (uint32_t)get_mm_start();
    printf("start addr:%x\n", shm_base_s);
    printf("inner swap addr:%x\n", INNER_SWAP_ADDR);
    tank_mm_register(&in_swap_mm_s, INNER_SWAP_ADDR, INNER_SWAP_SIZE, "inner_swap_malloc");

    strncpy((char *)shm_base_s, "OK", 4);

    print_all_info();
    // printf("process pid is %d\n",getpid());
    // my_sem_creat(&sem1, 1);
    // uint8_t val;
    // my_sem_get_val(&sem1, &val);
    // printf("current: num:%d\n", val);
    // my_sem_wait(&sem1);
    // my_sem_get_val(&sem1, &val);
    // printf("current: num:%d\n", val);
    pthread_create(&p_pid,NULL,&app_malloc,NULL);
    // pthread_create(&p_pid,NULL,&get_socket_info,NULL);

    // pthread_create(&p_pid,NULL,&app_socket_test,NULL);
    // pthread_create(&p_pid,NULL,&print2,NULL);

    pthread_join(p_pid,NULL);
    printf("addr:%x\n", shm_base_s);
    return 0;
}
