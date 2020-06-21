#ifndef __MY_SEM_H__
<<<<<<< HEAD
#define __MY_SEM_H___
=======
#define __MY_SEM_H__
>>>>>>> dev

#include "tank_pub.h"

#ifdef __LINUX__
#include <semaphore.h>
#endif

typedef enum{
    SEM_SUCCESS = 0,
    SEM_FAIL = 1
}sem_status_t;

#ifdef __LINUX__
<<<<<<< HEAD
#define my_sem_t sem_t
#endif

static int sem_get_id(void);
=======
typedef sem_t my_sem_t;
#endif

>>>>>>> dev

sem_status_t my_sem_creat(my_sem_t *sem, uint8_t value);
sem_status_t my_sem_destroy(my_sem_t *sem);
sem_status_t my_sem_wait(my_sem_t *sem);
sem_status_t my_sem_trywait(my_sem_t *sem);
sem_status_t my_sem_post(my_sem_t *sem);
sem_status_t my_sem_get_val(my_sem_t *sem, int *val);
sem_status_t my_sem_set_val(my_sem_t *sem, int val);


#endif


