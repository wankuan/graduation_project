#include "my_shm.h"

shm_group_t shm_group_s = 0;

static int sem_get_id(void)
{
    int key_id = osal_get_keyid();
    int shm_id = shmget(key_id+shm_group_s, SHM_MAX_SIZE,IPC_CREAT|0666);
    printf("shm_id is %d\n",shm_id);
    return shm_id;
}

shm_status_t shm_constuctor(my_shm_t *my_shm)
{
    my_shm->id = sem_get_id();
    my_shm->max_size = SHM_MAX_SIZE;
    my_shm->addr = NULL;
    if(!IS_SHM_ID_VALID(my_shm->id)){
        printf("[ERROR]code:%d\n",errno);
        goto fail;
    }
    return SUCCESS;
fail:
    return FAIL;
}

shm_status_t shm_destuctor(my_shm_t *my_shm)
{
    my_shm->id = 0;
    my_shm->max_size = 0;
    my_shm->addr = NULL;
    return SUCCESS;
}

shm_status_t shm_write(my_shm_t *my_shm, void *addr, int size)
{
    if(size>SHM_MAX_SIZE){
        printf("[ERROR]:shared memory exceed max size\n");
        goto fail;
    }
    void* shm_addr = shmat(my_shm->id, NULL, 0);
    if (shm_addr != (void*)-1) {
        memcpy(shm_addr, addr, size);
        shmdt(shm_addr);
    }else{
        printf("[ERROR]:shmat fail,code:%d\n",errno);
    }
    return SUCCESS;
fail:
    return FAIL;
}

shm_status_t shm_read(my_shm_t *my_shm, void *addr, int size)
{
    if(size > SHM_MAX_SIZE){
        printf("[ERROR]:shared memory exceed max size\n");
        goto fail;
    }
    void* shm_addr = shmat(my_shm->id, NULL, 0);
    if (shm_addr != (void*)-1) {
        memcpy(addr, shm_addr, size);
        shmdt(shm_addr);
    }else{
        printf("[ERROR]:shmat fail,code:%d\n",errno);
    }
    return SUCCESS;
fail:
    return FAIL;
}

shm_status_t shm_delete(my_shm_t *my_shm)
{
    struct shmid_ds buf;
    if (shmctl(my_shm->id, IPC_RMID,&buf)){
        printf("delete shm fail\n");
    }else{
        printf("delete shm success\n");
    }
}