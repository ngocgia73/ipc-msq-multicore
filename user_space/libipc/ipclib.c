#include <pthread.h>
#include <string.h>
#include <ipclib.h>
#include <ipc-common.h>
#define DEVICE_NAME 	"/dev/ipc_msgqueue_d0"
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int ipcfd = -1;
static int msqcnt = 0;
int ipc_open(void)
{
	pthread_mutex_lock(&mutex);
	if(ipcfd < 0)
	{
		ipcfd = open(DEVICE_NAME, O_RDWR|SYNC);
		if(ipcfd < 0)
		{
			printf("open device file failed\n");
			pthread_mutex_unlock(&mutex);
			return IPC_ERR_NOT_OPENED;
		}
	}
	pthread_mutex_unlock(&mutex);
	return ipcfd;
		
}

IPC_ERR ipc_close(void)
{
	if(ipcfd >= 0)
	{
		close(ipcfd);
	}
	return IPC_ERR_OK;
}

INT16 ipc_ftok(const char *path)
{
	IPC_IOC_MSGQUE_FTOK msg;
	int ret;
	if(!path)
	{
		printf("path is null\n");
		return -1;
	}
	strncpy(msg.path, path, sizeof(msg.path));
	msg.path[sizeof(msg.path) - 1] = 0; // '\0'
	if(ipc_open() < 0)
	{
		return -2;
	}
	ret = ioctl(ipcfd, IOC_IPC_FTOK, &msg);
	if(ret < 0)
	{
		return -3
	}
	return msg.rtn_key;
}

INT32 ipc_msg_get(INT16 key)
{
	IPC_IOC_MSGQUE_GET msg;
	int ret;
	msg.key = key;
	if(ipc_open() < 0)
	{
		return -1;
	}
	ret = ioctl(ipcfd, IOC_IPC_MSGQUE_GET, &msg);
	if(ret < 0)
	{
		return -2;
	}
	if(msg.rtn >=0)
	{
		msqcnt++;
	}
	return msg.rtn;
}

IPC_ERR ipc_msg_rel(UINT32 msqid)
{
	IPC_IOC_MSGQUE_REL msg;
	int ret;
	msg.msqid = msqid;
	ret = ioctl(ipcfd, IOC_IPC_MSGQUE_REL, &msg);
	if(ret < 0)
	{
		return -1;
	}
	if(msg.rtn == IPC_ERR_OK)
	{
		msqcnt--;
		if(msqcnt == 0)
		{
			close(ipcfd);
			ipcfd = -1;
		}
	}
	return msg.rtn;
}

INT32 ipc_msg_snd(UINT32 msqid, IPC_SENDTO sendTo, void *pmsg_data, UINT32 msg_size)
{
	IPC_IOC_MSGQUE_SND msg;
	int ret;
	if(ipc_open() < 0)
	{
		return IPC_ERR_SND_MSG;
	}
	if(msg_size > IPC_MSG_ELEMENT_SIZE)
	{
		return IPC_ERR_SNDSIZE_OVER_LIMIT;
	}
	if(!pmsg_data)
	{
		printf("pmsg_data is null\n");
		return IPC_ERR_SND_MSG;
	}
	msg.msqid = msqid;
	msg.sendTo = sendTo;
	msg.msg_size = msg_size;
	memcpy(&msg.msg_data, pmsg_data,msg_size);
	ret = ioctl(ipcfd, IOC_IPC_MSGQUE_SND, &msg);
	if(ret < 0)
	{
		return IPC_ERR_SND_MSG;
	}
	return msg.rtn;
}

INT32 ipc_msg_rcv(UINT32 msqid, void *pmsg_data, UINT32 msg_size)
{
	IPC_IOC_MSGQUE_RCV msg;
	int ret;
	if(ipc_open() < 0)
	{
		return IPC_ERR_RCV_MSG;
	}
	if(!pmsg_data)
	{
		printf("pmsg_data is null\n");
		return IPC_ERR_RCV_MSG;
	}
	msg.msqid = msqid;
	msg.msg_size = msg_size;
	ret = ioctl(ipcfd, IOC_IPC_MSGQUE_RCV, &msg);
	if(ret < 0)
	{
		retrun IPC_ERR_RCV_MSG;
	}
	if(msg.rtn > 0)
	{
		memcpy(pmsg_data, &msg.msg_data, msg.rtn);
	}
	return msg.rtn;
}
