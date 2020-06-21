#ifndef __EXTERNAL_PROCESS_H__
#define __EXTERNAL_PROCESS_H__
#include "tank_pub.h"
#include "tank_request.h"
#include "tank_map.h"
#include <time.h>

#include "tank_msgq.h"
#include "tank_mm.h"
#include "my_list.h"

typedef uint16_t port_t;
typedef uint32_t addr_t;
typedef uint32_t addr_shift_t;
typedef uint16_t ta_id_t;
typedef uint16_t ta_msg_len_t;
typedef uint16_t system_id_t;

#define TA_HOST_MAX 20
#define TA_NAME_SIZE_MAX 32
#define TA_PACKAGE_MAX 20


typedef struct{
    pthread_t           pid;
    pthread_mutex_t     thread_mutex;
    my_sem_t            thread_sem;
    void*               (*recv_thread)(void*);
    void*               (*send_thread)(void*);
    ta_id_t             id;
    char                name[32];
    list_head_t         send_package_list;
    list_head_t         recv_package_list;
    tank_mm_t           mm_handler;
    tank_msgq_t         *sender;
    tank_msgq_t         *receiver;
    tank_status_t      (*recv_package_cb)(app_package_info_t* info);
}external_process_info_t;



tank_status_t external_service_init(external_process_info_t *handler);

tank_status_t external_service_deinit(void);

#define __weak __attribute__((weak))

void *external_process_recv_thread(void *arg);
void *external_process_send_thread(void *arg);

tank_status_t external_process_send_msgq(system_id_t system_id, void *buf, uint32_t len);





#endif
