#ifndef _IPC_COMMON_H_
#define _IPC_COMMON_H_
/*
 * this header file used by both user space and kernel space
 */
#define __CORE1 	1
#define __CORE2 	2
#define __CORE3 	3

#define MAX_LENGTH 	3
#define IPC_MSG_QUEUE_TOKEN_STR_MAXLEN 32 // bytes

#define IPC_MSG_QUEUW_NUM 	12
#define IPC_MSG_ELEMENT_NUM 	16
#define IPC_MSG_ELEMENT_SIZE 	24 // queue element size


#define IPC_SHARED_MEM_ADDR          0x00100000  // ipc shared memory on RAM
#define IPC_SHARED_MEM_SIZE          (1024*4)    // size of shared memory

typedef enum ipc_sendto
{
	IPC_SENDTO_CORE1 = 1,
	IPC_SENDTO_CORE2,
	IPC_SENDTO_CORE3,
	IPC_SENDTO_MAX = IPC_SENDTO_CORE3
} IPC_SENDTO;

// this data structure share bw drv_cc and drv_chardev 
// for receive data
typedef struct ipc_msg
{
	UINT32 size;
	UINT32 data[MAX_LENGTH];
} IPC_MSG;

typedef struct ipc_send_msg
{
	UINT32 sendTo;
	UINT32 msg_data[MAX_LENGTH];
} IPC_SEND_MSG;

typedef struct ipc_ioc_msgque_get
{
	UINT16 key;
	INT32 rtn;
} IPC_IOC_MSGQUE_GET;

typedef struct ipc_ioc_msgque_rel
{
	UINT32 msqid;
	INT32 rtn;
} IPC_IOC_MSGQUE_REL;

typedef struct ipc_ioc_msgque_ftok
{
	char path[IPC_MSG_QUEUE_TOKEN_STR_MAXLEN + 1];
	UINT16 rtn_key;
} IPC_IOC_MSGQUE_FTOK;

typedef struct ipc_ioc_msgque_snd
{
	UINT32 msqid;
	IPC_SENDTO sendTo;
	char msg_data[IPC_MSG_ELEMENT_SIZE]; // MAX 24 BYTE FOR EVERY TRANFER DATA
	UINT32 msg_size;
	INT32 rtn;
} IPC_IOC_MSGQUE_SND;

typedef struct ipc_ioc_msgque_rcv
{
	UINT32 msqid;
	char msg_data[IPC_MSG_ELEMENT_SIZE]; // MAX 24 BYTE FOR EVERY TRANFER DATA
	UINT32 msg_size;
	INT32 rtn;
} IPC_IOC_MSGQUE_RCV;

#define IPC_IOC_MAGIC 		'C'
#define IOC_IPC_MSGQUE_FTOK 	 _IOWR(IPC_IOC_MAGIC, 0, IPC_IOC_MSGQUE_FTOK)
#define IOC_IPC_MSGQUE_GET 	 _IOWR(IPC_IOC_MAGIC, 1, IPC_IOC_MSGQUE_GET)
#define IOC_IPC_MSGQUE_REL 	 _IOWR(IPC_IOC_MAGIC, 2, IPC_IOC_MSGQUE_REL)
#define IOC_IPC_MSGQUE_SND 	 _IOWR(IPC_IOC_MAGIC, 3, IPC_IOC_MSGQUE_SND)
#define IOC_IPC_MSGQUE_RCV 	 _IOWR(IPC_IOC_MAGIC, 4, IPC_IOC_MSGQUE_RCV)

#endif // _IPC_COMMON_H_
