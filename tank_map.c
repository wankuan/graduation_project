#include "tank_map.h"


#include "tank_log_api.h"
#define FILE_NAME "tank_map"

volatile uint32_t g_shm_base = 0u;



tank_status_t tank_creat_shm(void)
{
    const char *name = TANK_PUB_NAME;
    int fd = shm_open(name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    log_info("map_size:%d\n", SHM_SIZE);
    ftruncate(fd, SHM_SIZE);
    void *buf = mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
    if (!buf) {
        log_error("mmap failed\n");
        close(fd);
        exit(1);
    }
    memset(buf, 0, SHM_SIZE);
    g_shm_base = (uint32_t)buf;
    log_info("shm_base:0x%x  addr:%p\n", g_shm_base, &g_shm_base);
    close(fd);
    return TANK_SUCCESS;
}


tank_status_t get_shm_base_addr(void)
{
    const char *name = TANK_PUB_NAME;
    int fd = shm_open(name, O_RDWR, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    void *buf = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (!buf) {
        log_info("mmap failed\n");
        close(fd);
        exit(1);
    }
    g_shm_base = (uint32_t)buf;
    log_info("shm_base:0x%x  addr:%p\n", g_shm_base, &g_shm_base);
    close(fd);
    return TANK_SUCCESS;
}

tank_status_t get_service_msgq_addr(tank_msgq_t **addr)
{
    *addr = (tank_msgq_t *)(*(uint32_t *)MSGQ_MAP_ADDR + g_shm_base);
    log_info("service msgq addr:%p\n", *addr);
    return TANK_SUCCESS;
}