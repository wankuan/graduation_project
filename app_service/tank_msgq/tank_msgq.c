#include "tank_msgq.h"
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

tank_msgq_map_t *msgq_map_s;


tank_status_t tank_msgq_map_creat(tank_msgq_t* handler)
{
    const char *name = tank_msgq_map_name;
    int fd = shm_open(name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    /* 更改文件长度，要比共享内存写入区域大 */
    uint16_t map_size = (TANK_TANK_MSGQ_NUM+1)*4;
    printf("map_size:%d\n", map_size);
    ftruncate(fd, map_size);

    //TODO:
    char *buf = (char*)mmap(NULL, 1024, PROT_WRITE, MAP_SHARED, fd, 0);
    if (!buf) {
        printf("mmap failed\n");
        close(fd);
        exit(1);
    }
    printf("mmap address:%p\n", buf);
    snprintf(buf, 1024, "This is a share memory!");
    printf("message:%s\n", buf);

    munmap(buf, 1024);
    close(fd);
    return TANK_SUCCESS;
}



tank_status_t tank_msgq_constructor(void)
{
    msgq_map_s = (tank_msgq_map_t*)TANK_MSGQ_MAP_ADDR_START;
    return TANK_SUCCESS;
}
tank_msgq_t *tank_get_msgq_addr(tank_queue_id_t id)
{
    if(id > TANK_MAX_SIZE){
        return NULL;
    }
    return msgq_map_s->addr[id];
}

tank_status_t tank_msgq_creat(tank_msgq_t* handler, tank_msgq_len_t len)
{
    handler->len = len;
    handler->cur_len = 0;
    handler->buf = NULL;
    handler->head = 0;
    handler->tail = 0;
    handler->buf = (msgq_t)malloc(len*TANK_MSGQ_BUFFER_SIZE);
    my_sem_creat(&handler->sem, 1);
    if(handler->buf == NULL){
        tank_msgq_delete(handler);
        return TANK_FAIL;
    }
    return TANK_SUCCESS;
}

tank_status_t tank_msgq_delete(tank_msgq_t* handler)
{
    free(handler->buf);
    free(handler);
    return TANK_SUCCESS;
}


tank_status_t tank_msgq_recv(tank_msgq_t* handler, uint8_t *msg, uint16_t len)
{
    if(tank_msgq_is_empty(handler)){
        printf("[ERROR]msgq is empty\n");
        return TANK_FAIL;
    }
    my_sem_wait(&handler->sem);
    printf("\ntank_msgq_recv  tail:%d cur_len:%d\n", handler->tail, handler->cur_len);
    memcpy(msg, (uint8_t*)handler->buf[handler->tail], len);
    handler->cur_len -= 1;
    if(!tank_msgq_is_empty(handler)){
        handler->tail += 1;
        handler->tail %= handler->len;
    }
    my_sem_post(&handler->sem);
    return TANK_SUCCESS;
}
tank_status_t tank_msgq_recv_wait(tank_msgq_t* handler, uint8_t *msg, uint16_t len,uint16_t timeout)
{
    return TANK_SUCCESS;
}
tank_status_t tank_msgq_send(tank_msgq_t* handler, uint8_t *msg, uint16_t len)
{
    if(tank_msgq_is_full(handler)){
        printf("[ERROR]msgq is full\n");
        return TANK_FAIL;
    }
    my_sem_wait(&handler->sem);
    if(!tank_msgq_is_empty(handler)){
        handler->head += 1;
        handler->head %= handler->len;
    }
    handler->cur_len += 1;
    printf("\ntank_msgq_send  head:%d cur_len:%d\n", handler->head, handler->cur_len);
    memcpy((uint8_t*)handler->buf[handler->head], msg, len);
    my_sem_post(&handler->sem);
    return TANK_SUCCESS;
}

uint8_t tank_msgq_is_full(tank_msgq_t *handler)
{
    if(handler->cur_len == handler->len){
        return 1;
    }else{
        return 0;
    }
}
uint8_t tank_msgq_is_empty(tank_msgq_t *handler)
{
    if(handler->cur_len == 0){
        return 1;
    }else{
        return 0;
    }
}