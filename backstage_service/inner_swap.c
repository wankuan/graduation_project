#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"
#include "tank_msgq.h"
#include <pthread.h>
#include "tank_request.h"

app_allocate_info_t msgq_map[256];

uint16_t msgq_id_seq = 0;
uint32_t shm_base = 0;

tank_mm_t in_swap_mm_s;

uint16_t sem_size = sizeof(my_sem_t);
uint16_t msgq_size = sizeof(tank_msgq_t);


void get_socket_info(void);

void print_all_info(void)
{
    printf("sem:%d, msgq:%d \n", sem_size, msgq_size);
}

void *get_mm_start(void)
{
    const char *name = TANK_PUB_NAME;
    int fd = shm_open(name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    printf("fd:%d\n", fd);
    printf("map_size:%d\n", SHM_SIZE);
    ftruncate(fd, SHM_SIZE);
    void *buf = mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
    if (!buf) {
        printf("mmap failed\n");
        close(fd);
        exit(1);
    }
    memset(buf, 0, SHM_SIZE);
    // munmap(buf, SHM_SIZE);
    close(fd);
    return buf;
}

void *app_malloc(void *arg)
{
    app_info_t request;
    tank_msgq_t *request_msgq;

    app_heap_get_t *app_get = (app_heap_get_t *)SEM_ADDR;

    my_sem_t *sem_alloc = (my_sem_t *)TEMP_ADDR;
    printf("sem addr:%p  size:%d\n", sem_alloc, sizeof(my_sem_t));

    sem_destroy(sem_alloc);
    my_sem_creat(sem_alloc, 0);

    request_msgq = (tank_msgq_t*)tank_mm_malloc(&in_swap_mm_s, sizeof(tank_msgq_t)+20*20);
    printf("msgq addr:%p\n", request_msgq);

    *(uint32_t*)MSGQ_MAP_ADDR = (uint32_t)request_msgq - shm_base;

    tank_msgq_creat(request_msgq, 20, 20);


    while(1){
        while(tank_msgq_recv(request_msgq, &request, 20)==TANK_FAIL){
            sleep(1);
            printf("wait for meessage from app\n");
        }
        if(request.type == MM_ALLOCATE){
            printf("[app]start malloc\n");
            void *addr = tank_mm_alloc(&in_swap_mm_s, request.request.size);

            uint32_t shift = (uint32_t)addr - shm_base;

            printf("[app]name:%s, size:%d\n", request.request.name, request.request.size);

            msgq_map[msgq_id_seq].id = msgq_id_seq;
            msgq_map[msgq_id_seq].shift = shift;
            strncpy(msgq_map[msgq_id_seq].name, request.request.name, 8);

            memset(app_get, 0, sizeof(app_heap_get_t));
            app_get->shift = shift;
            app_get->id = msgq_id_seq;
            msgq_id_seq += 1;
            get_socket_info();
            printf("[app]exit, malloc OK\n");
            int sem_val;
            my_sem_get_val(sem_alloc, &sem_val);
            my_sem_post(sem_alloc);
            my_sem_get_val(sem_alloc, &sem_val);
        }
    }
    return NULL;
    // my_sem_t *sem_malloc = (my_sem_t *)(SEM_ADDR);
    // my_sem_creat(sem_malloc, 1);
    // // sock_msgq_request_info_t *sock_get_info = (sock_msgq_request_info_t *)(MSGQ_REQUEST_ADDR);
    // // sock_msgq_get_info_t *sock_send_info = (sock_msgq_get_info_t *)(MSGQ_REQUEST_ADDR);
    // // int sem_val = 0;
    // // printf("[app]wait for malloc sem\n");
    // while(1){
    //     tank_msgq_recv()
    //     do{
    //         my_sem_get_val(sem_malloc, &sem_val);
    //         // printf("[app]wait for malloc sem\n");
    //         // sleep(2);
    //     }while(sem_val == 1);
    //     printf("[app]start malloc\n");
    //     void *addr = tank_mm_alloc(&in_swap_mm_s, sock_get_info->msgq_len * TANK_MSGQ_BUFFER_SIZE + msgq_size);
    //     strncpy(addr, "a message from backstage!", 100);
    //     uint32_t shift = (uint32_t)addr - shm_base;

    //     printf("[app]name:%s, len:%d\n", sock_get_info->name, sock_get_info->msgq_len);
    //     msgq_map[msgq_id_seq].id = msgq_id_seq;
    //     msgq_map[msgq_id_seq].shift = shift;
    //     strncpy(msgq_map[msgq_id_seq].name, sock_get_info->name, 8);

    //     memset(sock_get_info, 0, sizeof(sock_msgq_request_info_t));
    //     sock_send_info->shift = shift;
    //     sock_send_info->id = msgq_id_seq;

    //     msgq_id_seq += 1;
    //     get_socket_info();
    //     printf("[app]exit, malloc OK\n");
    //     my_sem_get_val(sem_malloc, &sem_val);
    //     my_sem_post(sem_malloc);
    //     my_sem_get_val(sem_malloc, &sem_val);
    // }

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
    shm_base = (uint32_t)get_mm_start();
    printf("start addr:%x\n", shm_base);
    printf("inner swap addr:%x\n", INNER_SWAP_ADDR);
    tank_mm_register(&in_swap_mm_s, INNER_SWAP_ADDR, INNER_SWAP_SIZE, "inner_swap_malloc");


    strncpy((char *)shm_base, "OK", 4);

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
    printf("addr:%x\n", shm_base);
    return 0;
}
