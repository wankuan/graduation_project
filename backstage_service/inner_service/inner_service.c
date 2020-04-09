#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"
#include "tank_msgq.h"
#include <pthread.h>
#include "tank_request.h"
#include "inner_service.h"

app_info_t               app_info_table[256];
tank_id_t                app_lut[256];

uint16_t                 g_app_id_seq = 0;
tank_mm_t                g_in_swap_mm;

pthread_t                g_service_pid;
tank_msgq_t             *g_service_request_msgq;
app_heap_get_t          *g_app_get;
app_request_info_t               g_app_request;


tank_status_t app_malloc_init(void);



void print_app_info_table(void)
{
    printf("=========app_info_table============\n");
    for(int i=0;i<g_app_id_seq;++i){
        printf("[service]id:%d, mm:%p, msgq_r:%p, msgq_s:%p\n",
        app_info_table[i].id, app_info_table[i].heap_addr,
        app_info_table[i].msgq_recv_addr, app_info_table[i].msgq_send_addr
        );
    }
    printf("=========app_info_table============\n");
}


tank_status_t inner_service_init(void)
{
    get_service_base_addr();
    printf("start addr:0x%x\n", g_shm_base);
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

    *(uint32_t*)MSGQ_MAP_ADDR = (uint32_t)g_service_request_msgq - g_shm_base;
    tank_msgq_creat(g_service_request_msgq, 20, 20);
    return TANK_SUCCESS;
}


int find_id_index(tank_id_t id)
{
    tank_id_t index = 0;
    for(int i=0;i<g_app_id_seq;++i){
        if(app_info_table[i].id == id){
            printf("[service]find id index:%d\n", i);
            return i;
        }
    }
    printf("[service]ERROR, can not find id:%d\n", id);
    return -1;
}

void *main_thread(void *arg)
{
    while(1){
        memset(&g_app_request, 0, TANK_MSGQ_NORMAL_SIZE);
        tank_msgq_recv_wait(g_service_request_msgq, &g_app_request, TANK_MSGQ_NORMAL_SIZE);
        if(g_app_request.type == MM_ALLOCATE){
            int index = find_id_index(g_app_request.heap.id);
            if(index >= 0){
                printf("[ERROR]id:%d has been malloc\n", g_app_request.heap.id);
                continue;
            }
            printf("\n[app]start malloc\n");
            printf("[app]id:%d, size:%d\n", g_app_request.heap.id, g_app_request.heap.size);
            void *addr = tank_mm_alloc(&g_in_swap_mm, g_app_request.heap.size);
            printf("[%s]remain:%d\n", g_in_swap_mm.name, g_in_swap_mm.heap.xFreeBytesRemaining);
            uint32_t shift = (uint32_t)addr - g_shm_base;

            app_info_table[g_app_id_seq].id = g_app_request.heap.id;
            app_info_table[g_app_id_seq].heap_addr = (void*)addr;

            printf("[app]id:%d, shift:%d\n", g_app_request.heap.id, shift);

            // memset(g_app_get, 0, sizeof(app_heap_get_t));
            g_app_get->shift = shift;

            g_app_id_seq += 1;
            printf("[app]exit, malloc OK\n");
            my_sem_post(&g_app_get->sem);
        }else if(g_app_request.type == PUSH_MSGQ_ADDR){
            printf("[service]id:%d, type:%d, recv_shift:%d, send_shift:%d\n", g_app_request.msgq.id, g_app_request.type, g_app_request.msgq.recv_shift, g_app_request.msgq.send_shift);
            int index = find_id_index(g_app_request.msgq.id);
            app_info_table[index].msgq_recv_addr = (void*)g_app_request.msgq.recv_shift + g_shm_base;
            app_info_table[index].msgq_send_addr = (void*)g_app_request.msgq.send_shift + g_shm_base;

            // memset(&g_app_request, 0, TANK_MSGQ_NORMAL_SIZE);
            // g_app_request.type = SEND_MSG;
            // g_app_request.msg.src_id = 10;
            // g_app_request.msg.dst_id = 1;
            // g_app_request.msg.state = 5;

            // tank_msgq_send((tank_msgq_t*)app_info_table[index].msgq_recv_addr, &g_app_request, TANK_MSGQ_NORMAL_SIZE);

            print_app_info_table();
        }else if(g_app_request.type == SEND_MSG){
            int index = find_id_index(g_app_request.msg.dst_id);
            if(index >= 0){
                tank_msgq_send((tank_msgq_t*)app_info_table[index].msgq_recv_addr, &g_app_request, APP_MSG_SIZE);
            }
            printf("[service]recv a msg, src_id:%d, dst_id:%d, state:%d\n",
                    g_app_request.msg.src_id, g_app_request.msg.dst_id, g_app_request.msg.state
                    );
        }

    }
    return NULL;
}





int main(int argc, char *argv[])
{
    inner_service_init();
    pthread_create(&g_service_pid,NULL,&main_thread,NULL);
    pthread_join(g_service_pid,NULL);
    printf("[inner_service]:ending!\n");
    return 0;
}
