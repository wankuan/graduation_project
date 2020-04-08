#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"
#include "tank_msgq.h"
#include <pthread.h>
#include "tank_request.h"


msgq_allocate_info_t msgq_map[256];

uint16_t msgq_id_seq = 0;

uint32_t shm_base_s = 0;
tank_mm_t g_in_swap_mm;

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
    my_sem_t *sem_malloc = (my_sem_t *)(SEM_ADDR);
    printf("[socket]start send malloc request\n");

    my_sem_get_val(sem_malloc, &sem_val);
    my_sem_wait(sem_malloc);
    printf("[socket]wait for backstage allocate finished\n");

    sock_msgq_request_info_t *sock_get_info = (sock_msgq_request_info_t *)(MSGQ_REQUEST_ADDR);
    memset(sock_get_info, 0, sizeof(sock_msgq_request_info_t));
    strncpy(sock_get_info->name, "sock_1", 8);
    sock_get_info->msgq_len = 10;
    printf("[socket]name:%s, len:%d\n", sock_get_info->name, sock_get_info->msgq_len);

    my_sem_get_val(sem_malloc, &sem_val);
    my_sem_wait(sem_malloc);

    printf("[socket]backstage allocate OK\n");
    printf("[socket]get allocate info\n");
    sock_msgq_get_info_t *sock_send_info = (sock_msgq_get_info_t *)(MSGQ_REQUEST_ADDR);
    printf("[socket]id:%d, addr_base:%p, shift:%d\n", sock_send_info->id, (void*)shm_base_s, sock_send_info->shift);
    printf("%s\n", (char *)(shm_base_s + sock_send_info->shift));

    my_sem_get_val(sem_malloc, &sem_val);
    my_sem_post(sem_malloc);
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t p_pid;
    shm_base_s = (uint32_t)get_mm_start();
    printf("str:%s\n", (char *)shm_base_s);
    printf("start addr:%x\n", shm_base_s);
    print_all_info();
    pthread_create(&p_pid,NULL,&app_socket_test,NULL);
    // pthread_create(&p_pid,NULL,&print2,NULL);

    pthread_join(p_pid,NULL);
    // my_sem_destroy(&sem1);

    printf("addr:%x\n", shm_base_s);
    return 0;
}
