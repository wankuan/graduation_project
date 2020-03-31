#include "my_sem.h"



sem_status_t my_sem_creat(my_sem_t *sem, uint8_t value)
{
    uint8_t flag = 0;
#ifdef __LINUX__
    flag = sem_init(sem, 0, value);
#endif
    if(flag == 0){
        return SEM_SUCCESS;
    }else{
        return SEM_FAIL;
    }
}
sem_status_t my_sem_destroy(my_sem_t *sem)
{
    uint8_t flag = 0;
#ifdef __LINUX__
    flag = sem_destroy(sem);
#endif
    if(flag == 0){
        return SEM_SUCCESS;
    }else{
        return SEM_FAIL;
    }
}
sem_status_t my_sem_wait(my_sem_t *sem)
{
    if(!sem_wait(sem)){
        return SEM_SUCCESS;
    }else{
        return SEM_FAIL;
    }
}
sem_status_t my_sem_trywait(my_sem_t *sem)
{

}
sem_status_t my_sem_post(my_sem_t *sem)
{
    if(!sem_post(sem)){
        return SEM_SUCCESS;
    }else{
        return SEM_FAIL;
    }
}
sem_status_t my_sem_get_val(my_sem_t *sem, int *val)
{
    if(!sem_getvalue(sem, val)){
        return SEM_SUCCESS;
    }else{
        return SEM_FAIL;
    }
}
sem_status_t my_sem_set_val(my_sem_t *sem, int val)
{

}