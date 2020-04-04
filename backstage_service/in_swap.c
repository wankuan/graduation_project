#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"
#include "tank_msgq.h"
#include <pthread.h>



typedef struct{
    char name[8];
    uint32_t len;
}sock_get_info_t;

typedef struct{
    uint16_t id;
    uint32_t shift;
}sock_send_info_t;

typedef struct{
    uint16_t id;
    uint32_t shift;
    char name[8];
}sock_info_t;



sock_info_t msgq_map[256];

uint16_t msgq_id_seq = 0;

uint32_t start_addr = 0;
uint32_t msgq_start_addr = 0;

tank_mm_t in_swap_mm;

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
    printf("map_size:%d\n", TANK_PUB_SIZE);
    ftruncate(fd, TANK_PUB_SIZE);
    void *buf = mmap(NULL, TANK_PUB_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
    if (!buf) {
        printf("mmap failed\n");
        close(fd);
        exit(1);
    }
    // munmap(buf, TANK_PUB_SIZE);
    close(fd);
    return buf;
}

void *app_malloc(void *arg)
{
    tank_mm_register(&in_swap_mm, msgq_start_addr, MSGQ_ALL_SIZE, "app_malloc");

    my_sem_t *sem_malloc = (my_sem_t *)(start_addr + SEM_ADDR);


    my_sem_creat(sem_malloc, 1);
    sock_get_info_t *sock_get_info = (sock_get_info_t *)(start_addr + MALLOC_ADDR);
    sock_send_info_t *sock_send_info = (sock_send_info_t *)(start_addr + MALLOC_ADDR);
    int sem_val = 0;
    while(1){
        do{
            my_sem_get_val(sem_malloc, &sem_val);
            // printf("[app]wait for malloc sem\n");
            // sleep(2);
        }while(sem_val == 1);
        printf("[app]exit, start malloc\n");
        void *addr = tank_mm_alloc(&in_swap_mm, sock_get_info->len * TANK_MSGQ_BUFFER_SIZE + msgq_size);
        strncpy(addr, "a message from backstage!", 100);
        uint32_t shift = (uint32_t)addr - start_addr;

        printf("[app]name:%s, len:%d\n", sock_get_info->name, sock_get_info->len);
        msgq_map[msgq_id_seq].id = msgq_id_seq;
        msgq_map[msgq_id_seq].shift = shift;
        strncpy(msgq_map[msgq_id_seq].name, sock_get_info->name, 8);

        memset(sock_get_info, 0, sizeof(sock_get_info_t));
        sock_send_info->shift = shift;
        sock_send_info->id = msgq_id_seq;

        msgq_id_seq += 1;
        get_socket_info();
        printf("[app]exit, malloc OK\n");
        my_sem_get_val(sem_malloc, &sem_val);
        my_sem_post(sem_malloc);
        my_sem_get_val(sem_malloc, &sem_val);
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

// void *app_socket_test(void *arg)
// {
//     sleep(1);
//     my_sem_t *sem_malloc = (my_sem_t *)(start_addr + SEM_ADDR);
//     printf("[socket]start send malloc request\n");
//     sock_get_info_t *sock_get_info = (sock_get_info_t *)(start_addr + MALLOC_ADDR);
//     memset(sock_get_info, 0, sizeof(sock_get_info_t));
//     strncpy(sock_get_info->name, "sock_1", 8);
//     sock_get_info->len = 10;

//     printf("[socket]name:%s, len:%d\n", sock_get_info->name, sock_get_info->len);
//     int sem_val = 0;

//     my_sem_get_val(sem_malloc, &sem_val);
//     my_sem_wait(sem_malloc);

//     printf("[socket]wait for backstage allocate finished\n");

//     my_sem_get_val(sem_malloc, &sem_val);
//     my_sem_wait(sem_malloc);

//     printf("[socket]backstage allocate OK\n");
//     printf("[socket]get allocate info\n");
//     sock_send_info_t *sock_send_info = (sock_send_info_t *)(start_addr + MALLOC_ADDR);
//     printf("[socket]id:%d, addr_base:%p, shift:%d\n", sock_send_info->id, (void*)start_addr, sock_send_info->shift);
//     my_sem_get_val(sem_malloc, &sem_val);
//     my_sem_post(sem_malloc);
//     return NULL;
// }
// //
// char message[100];
// void* print1(void *arg)
// {
//     printf("fun1 pid is %ld\n",pthread_self());
//     printf("fun1 getpid is %d\n",getpid());
//     printf("wait for get the message\n");
//     int val = 0;
//     my_sem_get_val(&sem1, &val);
//     printf("print1:current: num:%d\n", val);
//     my_sem_wait(&sem1);
//     printf("[fun1]message is %s\n",message);
//     my_sem_post(&sem1);
// }


// void* print2(void *arg)
// {
//     printf("fun2 pid is %ld\n",pthread_self());
//     printf("fun2 getpid is %d\n",getpid());
//     printf("after 5 seconds, will wirte into message\n");
//     sleep(5);
//     int val = 0;
//     my_sem_get_val(&sem1, &val);
//     printf("print2:current: num:%d\n", val);
//     strncpy(message,"huang wan kuan",100);
//     printf("has written into message, check\n");
//     my_sem_post(&sem1);
// }
int main(int argc, char *argv[])
{
    pthread_t p_pid;
    start_addr = (uint32_t)get_mm_start();
    strncpy((char *)start_addr, "OK", 4);
    printf("start addr:%x\n", start_addr);
    msgq_start_addr = start_addr + TANK_MSGQ_BASE;
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
    printf("addr:%x\n", start_addr);
    return 0;
}
