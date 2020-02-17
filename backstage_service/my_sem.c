#include "my_sem.h"

sem_group_t sem_group_s = 0;


static int sem_get_id(void)
{
    int key_id = osal_get_keyid();
    int sem_id = semget(key_id,1,IPC_CREAT|0666);
    printf("sem_id is %d\n",sem_id);
    return sem_id;
}

sem_status_t sem_constuctor(my_sem_t *my_sem)
{
    my_sem->group_id = sem_group_s;
    my_sem->sem_id = sem_get_id();
    sem_set_val(my_sem, my_sem->val);
    sem_group_s += 1;
    if (!IS_SEM_ID_VALID(my_sem->sem_id )){
        printf("[ERROR]code:%d\n",errno);
        goto fail;
    }
    return SUCCESS;
fail:
    return FAIL;
}

sem_status_t sem_destuctor(my_sem_t *my_sem)
{
    int status = 0;
    if(!semctl(my_sem->sem_id, my_sem->group_id, IPC_RMID))
        goto fail;
    my_sem->group_id = 0;
    my_sem->sem_id = 0;
    return SUCCESS;
fail:
    return FAIL;
}


sem_status_t sem_p(my_sem_t *my_sem)
{
  struct sembuf sem_buf;
  sem_buf.sem_num = 0;
  sem_buf.sem_op  = -1;
  sem_buf.sem_flg = SEM_UNDO;
  if (semop(my_sem->sem_id, &sem_buf, 1)==-1)
  {
    printf("p-sem fail\n");
    goto fail;
  }
    return SUCCESS;
fail:
    return FAIL;
}

sem_status_t sem_v(my_sem_t *my_sem)
{
  struct sembuf sem_buf;
  sem_buf.sem_num = 0;
  sem_buf.sem_op  = 1;
  sem_buf.sem_flg = SEM_UNDO;
  if (semop(my_sem->sem_id, &sem_buf, 1)==-1)
  {
    printf("v-sem fail\n");
    goto fail;
  }
    return SUCCESS;
fail:
    return FAIL;
}

sem_status_t sem_get_val(my_sem_t *my_sem, int *val)
{
    *val = semctl(my_sem->sem_id, my_sem->group_id, GETVAL);
    return SUCCESS;
}
sem_status_t sem_set_val(my_sem_t *my_sem, int val)
{
    union semun cmd;
    my_sem->val = val;
    cmd.val = my_sem->val;
    int status = semctl(my_sem->sem_id ,my_sem->group_id,SETVAL,cmd);
        if(status < 0){
            goto fail;
        }
    return SUCCESS;
fail:
    return FAIL;
}

