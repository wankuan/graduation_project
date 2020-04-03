#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"




void *get_mm_start(void)
{
    const char *name = TANK_PUB_NAME;
    int fd = shm_open(name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    printf("fd:%d\n", fd);
    printf("map_size:%d\n", TANK_PUB_SIZE);
    ftruncate(fd, TANK_PUB_SIZE);
    void *buf = mmap(NULL, TANK_PUB_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
    if (!buf) {
        printf("mmap failed\n");
        close(fd);
        exit(1);
    }
    // munmap(buf, TANK_PUB_SIZE);
    close(fd);
    return buf;
}


// //
// char message[100];
// void* print1(void *arg)
// {
//     printf("fun1 pid is %ld\n",pthread_self());
//     printf("fun1 getpid is %d\n",getpid());
//     printf("wait for get the message\n");
//     int val = 0;
//     my_sem_get_val(&sem1, &val);
//     printf("print1:current: num:%d\n", val);
//     my_sem_wait(&sem1);
//     printf("[fun1]message is %s\n",message);
//     my_sem_post(&sem1);
// }


// void* print2(void *arg)
// {
//     printf("fun2 pid is %ld\n",pthread_self());
//     printf("fun2 getpid is %d\n",getpid());
//     printf("after 5 seconds, will wirte into message\n");
//     sleep(5);
//     int val = 0;
//     my_sem_get_val(&sem1, &val);
//     printf("print2:current: num:%d\n", val);
//     strncpy(message,"huang wan kuan",100);
//     printf("has written into message, check\n");
//     my_sem_post(&sem1);
// }
int main(int argc, char *argv[])
{
    // pthread_t p_pid;
    // printf("process pid is %d\n",getpid());
    // my_sem_creat(&sem1, 1);
    // uint8_t val;
    // my_sem_get_val(&sem1, &val);
    // printf("current: num:%d\n", val);
    // my_sem_wait(&sem1);
    // my_sem_get_val(&sem1, &val);
    // printf("current: num:%d\n", val);
    // pthread_create(&p_pid,NULL,&print1,NULL);
    // pthread_create(&p_pid,NULL,&print2,NULL);

    // pthread_join(p_pid,NULL);
    // my_sem_destroy(&sem1);
    void *p = get_mm_start();
    printf("addr:%p\n", p);
    return 0;
}
