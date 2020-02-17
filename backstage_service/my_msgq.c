#include "my_msgq.h"

static int msgq_get_id(void)
{
    int key_id = osal_get_keyid();
    int msgq_id = msgget(key_id+2, IPC_CREAT|0666);
    printf("msgq_id is %d\n",msgq_id);
    return msgq_id;
}

static int msgq_set_id(int id)
{
    int key_id = osal_get_keyid();
    int msgq_id = msgget(key_id+id, IPC_CREAT|0666);
    printf("msgq_id is %d\n",msgq_id);
    return msgq_id;
}
msgq_status_t msgq_constuctor_id(my_msgq_t *my_msgq, int id)
{
    my_msgq->id = msgq_set_id(id);
    if (!IS_MSGQ_ID_VALID(my_msgq->id )){
        printf("[ERROR]code:%d\n",errno);
        goto MSGQ_FAIL;
    }
    return MSGQ_SUCCESS;
MSGQ_FAIL:
    return MSGQ_FAIL;
}
msgq_status_t msgq_constuctor(my_msgq_t *my_msgq)
{
    my_msgq->id = msgq_get_id();
    if (!IS_MSGQ_ID_VALID(my_msgq->id )){
        printf("[ERROR]code:%d\n",errno);
        goto MSGQ_FAIL;
    }
    return MSGQ_SUCCESS;
MSGQ_FAIL:
    return MSGQ_FAIL;
}
msgq_status_t msgq_destuctor(my_msgq_t *my_msgq)
{
    my_msgq->id = 0;
    return MSGQ_SUCCESS;
}

msgq_status_t msgq_send(my_msgq_t *my_msgq, void *addr, int size, long type)
{
    msgq_message_t buf;
    buf.type = type;
    memcpy(buf.message, addr, size);
    if(msgsnd(my_msgq->id, &buf, size, IPC_NOWAIT)){
        printf("[sender]MSGQ_FAIL\n");
        goto MSGQ_FAIL;
    }else{
        printf("[sender]MSGQ_SUCCESS\n");
    }
    return MSGQ_SUCCESS;
MSGQ_FAIL:
    return MSGQ_FAIL;
}

msgq_status_t msgq_rcvall_wait(my_msgq_t *my_msgq, void *addr, int size, long *type)
{
    msgq_message_t buf;
    int status = msgrcv(my_msgq->id, &buf, size, 0, MSG_NOERROR);
    if(status==-1){
        if (errno == ENOMSG) {
            printf("[receiver]No message!\n");
        }else{
            printf("[ERROR]code:errno\n");
        }
        goto MSGQ_FAIL;
    }else{
        *type = buf.type;
        memcpy(addr, buf.message, size);
        printf("[receiver]bytes:%d\n",status);
        printf("[receiver]messgae type:%ld\n",buf.type);
    }
    return MSGQ_SUCCESS;
MSGQ_FAIL:
    return MSGQ_FAIL;
}

msgq_status_t msgq_rcv_wait(my_msgq_t *my_msgq, void *addr, int size, long type)
{
    msgq_message_t buf;
    int status = msgrcv(my_msgq->id, &buf, size, type, MSG_NOERROR);
    if(status==-1){
        if (errno == ENOMSG) {
            printf("[receiver]No message!\n");
        }else{
            printf("[ERROR]code:errno\n");
        }
        goto MSGQ_FAIL;
    }else{
        memcpy(addr, buf.message, size);
        printf("[receiver]bytes:%d\n",status);
        printf("[receiver]messgae type:%ld\n",buf.type);
    }
    return MSGQ_SUCCESS;
MSGQ_FAIL:
    return MSGQ_FAIL;
}



msgq_status_t msgq_rcv_nowait(my_msgq_t *my_msgq, void *addr, int size, long type)
{
    msgq_message_t buf;
    int status = msgrcv(my_msgq->id, &buf, size, type, MSG_NOERROR|IPC_NOWAIT);
    if(status==-1){
        if (errno == ENOMSG) {
            printf("[receiver]No message!\n");
        }else{
            printf("[ERROR]code:errno\n");
        }
        goto MSGQ_FAIL;
    }else{
        memcpy(addr, buf.message, size);
        printf("[receiver]bytes:%d\n",status);
        printf("[receiver]messgae type:%ld\n",buf.type);
    }
    return MSGQ_SUCCESS;
MSGQ_FAIL:
    return MSGQ_FAIL;
}

msgq_status_t msgq_delete(my_msgq_t *my_msgq)
{
    struct msqid_ds buf;
    if(msgctl(my_msgq->id,IPC_RMID,&buf)==0){
        printf("delete msgq MSGQ_SUCCESS id:%d\n",my_msgq->id);
    }else{
        printf("delete error\n");
        goto MSGQ_FAIL;
    }
    return MSGQ_SUCCESS;
MSGQ_FAIL:
    return MSGQ_FAIL;
}

