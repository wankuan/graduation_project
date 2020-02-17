#ifndef __MY_SEM_H__
#define __MY_SEM_H___

#include <sys/ipc.h>
#include <sys/sem.h>
#include "lib_pub.h"
#include "osal_pub.h"
#include <errno.h>

#define IS_SEM_ID_VALID(id) (id<0?false:true)


union semun
{
    int val;				//<= value for SETVAL
    struct semid_ds *buf;		//<= buffer for IPC_STAT & IPC_SET
    unsigned short int *array;		//<= array for GETALL & SETALL
    struct seminfo *__buf;		//<= buffer for IPC_INFO
};

typedef int sem_group_t;

extern sem_group_t sem_group_s;

typedef enum{
    SEM_SUCCESS = 0,
    SEM_FAIL = 1
}sem_status_t;

typedef struct _my_sem_t{
    int group_id;
    int sem_id;
    int val;
}my_sem_t;



static int sem_get_id(void);
sem_status_t sem_constuctor(my_sem_t *my_sem);
sem_status_t sem_destuctor(my_sem_t *my_sem);
sem_status_t sem_p(my_sem_t *my_sem);
sem_status_t sem_v(my_sem_t *my_sem);
sem_status_t sem_get_val(my_sem_t *my_sem, int *val);
sem_status_t sem_set_val(my_sem_t *my_sem, int val);
#endif


