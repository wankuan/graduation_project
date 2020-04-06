#include "tank_socket.h"
#include "tank_socket_pub.h"
#include "tank_msgq.h"
#include "tank_map.h"

tank_status_t socket_msgq_creat(const char *name, uint16_t len)
{
    my_sem_t *sem_malloc = (my_sem_t *)(shm_start_addr + SEM_ADDR);
    printf("[socket]start send malloc request\n");

    my_sem_wait(sem_malloc);
    printf("[socket]wait for backstage allocate finished\n");

    sock_msgq_request_info_t *sock_get_info = (sock_msgq_request_info_t *)(start_addr + MALLOC_ADDR);
    memset(sock_get_info, 0, sizeof(sock_msgq_request_info_t));
    strncpy(sock_get_info->name, "sock_1", 8);
    sock_get_info->len = 10;
    printf("[socket]name:%s, len:%d\n", sock_get_info->name, sock_get_info->len);

    my_sem_wait(sem_malloc);

    printf("[socket]backstage allocate OK\n");
    printf("[socket]get allocate info\n");
    sock_msgq_get_info_t *sock_send_info = (sock_msgq_get_info_t *)(start_addr + MALLOC_ADDR);
    printf("[socket]id:%d, addr_base:%p, shift:%d\n", sock_send_info->id, (void*)start_addr, sock_send_info->shift);
    printf("%s\n", (char *)(start_addr + sock_send_info->shift));

    my_sem_get_val(sem_malloc, &sem_val);
    my_sem_post(sem_malloc);

}