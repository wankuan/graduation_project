#ifndef __MY_SHM_H__
#define __MY_SHM_H__

#include <sys/ipc.h>
#include <errno.h>
#include <sys/shm.h>

#include "lib_pub.h"
#include "osal_pub.h"

#define IS_SHM_ID_VALID(id) (id<0?false:true)
#define SHM_MAX_SIZE 16384


typedef int shm_group_t;

extern shm_group_t shm_group_s;

typedef enum{
    SHM_SUCCESS = 0,
    SHM_FAIL = 1
}shm_status_t;

typedef struct _my_shm_t{
    void *addr;
    int max_size;
    int id;
}my_shm_t;

typedef struct _shm_handle_t{
    my_shm_t *next_shm;
    int shm_id;
}shm_handle_t;




static int shm_get_id(void);
shm_status_t shm_constuctor(my_shm_t *my_shm);
shm_status_t shm_destuctor(my_shm_t *my_shm);

shm_status_t shm_write(my_shm_t *my_shm, void *addr, int size);
shm_status_t shm_read(my_shm_t *my_shm, void *addr, int size);

shm_status_t shm_delete(my_shm_t *my_shm);



int msg_send(void);
int msg_rcv(void);


#endif