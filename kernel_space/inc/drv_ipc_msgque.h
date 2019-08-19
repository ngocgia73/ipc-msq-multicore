#ifndef _DRV_IPC_MSGQUE_H_
#define _DRV_IPC_MSGQUE_H_


#define IPC_MSG_QUEUE_NUM  	12
#define IPC_MSG_ELEMENT_NUM  	16

#define CFG_IPC_INIT_KEY 	0x12345678

typedef struct ipc_msgque {
	UINT32 initkey;
	UNT32 sharekey;
	UINT32 msqid;
	UINT32 indx;
	UINT32 outdx;
	UINT32 count;
	IPC_MSG element[IPC_MSG_ELEMENT_NUM];
} IPC_MSGQUE;

typedef struct ipc_sndmsgbuff
{
	UINT32 pingponidx;
	IPC_SEND_MSG *sndmsg[2]; // maximum 2 msg in buff
	struct semaphore semid;
} IPC_SNDMSG_BUFF;

typedef struct ipc_msqinfo {
	char *syscmdbuf_vir; 	// buff in virtual addr . return from dma_alloc_coherent
	char *syscmdbuf_dma;
	IPC_MSGQUE msgQueTbl[IPC_MSG_QUEUE_NUM];
	char 	   msgQueTokenStr[IPC_MSG_QUEUE_NUM][IPC_MSG_QUEUE_TOKEN_STR_MAXLEN + 1];
	IPC_SNDMSG_BUFF  sndmsg_buffTbl[IPC_SENDTO_MAX];
} IPC_MSQINFO;


// function defination 
INT32 ipc_msgque_init(void);
void ipc_msgque_exit(void);

IPC_MSGQUE *ipc_msgque_getQuebyID(unsigned long coreid, UINT32 msqid);
INT32 ipc_msgque_getIDbyKey(unsigned long coreid, INT32 key);

INT32 ipc_msgque_get(unsigned long coreid, INT32 key);
INT32 ipc_msgque_rel(unsigned long coreid, UINT32 msqid);

INT32 ipc_msgque_dequeue(IPC_MSGQUE *msgque, IPC_MSG *msg);
INT32 ipc_msgque_enqueue(IPC_MSGQUE *msgque, IPC_MSG *msg);

bool ipc_msgque_isempty(IPC_MSGQUE *msgque);
bool ipc_msgque_isfull(IPC_MSGQUE *msgque);
bool ipc_msgque_isvalid(IPC_MSGQUE *msgque, UINT32 sharekey);
UINT32 ipc_msgque_getElementCount(IPC_MSGQUE *p_msgque);

UINT16 ipc_msgque_ftok(unsigned long coreid, const char *PATH);
char *ipc_msgque_getTokenByKey(unsigned long coreid, UINT32 key);

IPC_SEND_MSG* ipc_msgque_lockSndMsgBuff(unsigned long coreid, IPC_SENTO sendTo)IPC_SEND_MSG* ipc_msgque_lockSndMsgBuff(unsigned long coreid, IPC_SENTO sendTo);
IPC_SEND_MSG* ipc_msgque_lockSndMsgBuff(unsigned long coreid, IPC_SENTO sendTo);
void ipc_msgque_unlockSndMsgBuff(unsigned long coreid, IPC_SENTO sendTo);
#endif //_DRV_IPC_MSGQUE_H_
