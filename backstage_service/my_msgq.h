#ifndef __MY_MSGQ_H__
#define __MY_MSGQ_H__

#include <sys/ipc.h>
#include <sys/msg.h>
#include "lib_pub.h"
#include "osal_pub.h"
#include <errno.h>


#define MSGQ_MAX_SIZE 1024

#define IS_MSGQ_ID_VALID(id) (id<0?false:true)

typedef int msgq_group_t;

extern msgq_group_t msgq_group_s;

typedef enum{
    MSGQ_SUCCESS = 0,
    MSGQ_FAIL = 1
}msgq_status_t;

typedef struct _my_msgq_t{
    int id;
}my_msgq_t;


typedef struct _msgq_message_t{
    long type;
    char message[MSGQ_MAX_SIZE];
}msgq_message_t;



static int msgq_get_id(void);
msgq_status_t msgq_constuctor_id(my_msgq_t *my_msgq, int id);
msgq_status_t msgq_constuctor(my_msgq_t *my_msgq);
msgq_status_t msgq_destuctor(my_msgq_t *my_msgq);

msgq_status_t msgq_send(my_msgq_t *my_msgq, void *addr, int size, long type);
msgq_status_t msgq_rcvall_wait(my_msgq_t *my_msgq, void *addr, int size, long *type);
msgq_status_t msgq_rcv_wait(my_msgq_t *my_msgq, void *addr, int size, long type);
msgq_status_t msgq_rcv_nowait(my_msgq_t *my_msgq, void *addr, int size, long type);
msgq_status_t msgq_delete(my_msgq_t *my_msgq);

#endif