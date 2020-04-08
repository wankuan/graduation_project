#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"
#include "tank_msgq.h"
#include <pthread.h>
#include "tank_request.h"

app_allocate_info_t      g_msgq_map[256];
uint16_t                 g_app_id_seq = 0;
tank_mm_t                g_in_swap_mm;

pthread_t                g_service_pid;
tank_msgq_t             *g_service_request_msgq;
app_heap_get_t          *g_app_get;
app_info_t               g_app_request;


tank_status_t app_malloc_init(void);

void get_socket_info(void)
{
    for(int i=0;i<g_app_id_seq;i++)
    {
        printf("id:%d, name:%s, shift:%d\n", g_msgq_map[i].id, g_msgq_map[i].name, g_msgq_map[i].shift);
    }
}

tank_status_t inner_service_init(void)
{
    get_service_base_addr();
    printf("start addr:0x%x\n", shm_base_s);
    printf("inner swap addr:0x%x\n", INNER_SWAP_ADDR);
    tank_mm_register(&g_in_swap_mm, INNER_SWAP_ADDR, INNER_SWAP_SIZE, "inner_service_mm");
    app_malloc_init();
    return TANK_SUCCESS;

}
tank_status_t app_malloc_init(void)
{
    g_app_get = (app_heap_get_t *)SEM_ADDR;
    printf("sem addr:%p  size:%d\n", &g_app_get->sem, sizeof(my_sem_t));

    sem_destroy(&g_app_get->sem);
    my_sem_creat(&g_app_get->sem, 0);

    g_service_request_msgq = (tank_msgq_t*)tank_mm_malloc(&g_in_swap_mm, sizeof(tank_msgq_t)+20*20);
    printf("msgq addr:%p\n", g_service_request_msgq);

    *(uint32_t*)MSGQ_MAP_ADDR = (uint32_t)g_service_request_msgq - shm_base_s;
    tank_msgq_creat(g_service_request_msgq, 20, 20);
    return TANK_SUCCESS;
}


void *app_malloca_thread(void *arg)
{
    while(1){
        tank_msgq_recv_wait(g_service_request_msgq, &g_app_request, 20);
        if(g_app_request.type == MM_ALLOCATE){
            printf("\n[app]start malloc\n");
            void *addr = tank_mm_alloc(&g_in_swap_mm, g_app_request.request.size);
            printf("[%s]remain:%d\n", g_in_swap_mm.name, g_in_swap_mm.heap.xFreeBytesRemaining);
            uint32_t shift = (uint32_t)addr - shm_base_s;
            printf("[app]name:%s, size:%d\n", g_app_request.request.name, g_app_request.request.size);

            g_msgq_map[g_app_id_seq].id = g_app_id_seq;
            g_msgq_map[g_app_id_seq].shift = shift;
            strncpy(g_msgq_map[g_app_id_seq].name, g_app_request.request.name, 8);
            printf("[app]id:%d, name:%s, shift:%d\n", g_msgq_map[g_app_id_seq].id, g_msgq_map[g_app_id_seq].name, g_msgq_map[g_app_id_seq].shift);
            // memset(g_app_get, 0, sizeof(app_heap_get_t));
            g_app_get->shift = shift;
            g_app_get->id = g_app_id_seq;
            g_app_id_seq += 1;

            printf("[app]exit, malloc OK\n");
            my_sem_post(&g_app_get->sem);
        }
    }
    return NULL;
}





int main(int argc, char *argv[])
{
    inner_service_init();
    pthread_create(&g_service_pid,NULL,&app_malloca_thread,NULL);
    pthread_join(g_service_pid,NULL);
    printf("[inner_service]:ending!\n");
    return 0;
}
