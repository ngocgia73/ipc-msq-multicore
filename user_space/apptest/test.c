/*
 * des:     this is a example for testing ipc device driver which help to communicate between multi core
 * author : <ngocgia73@gmail.com>
 */
#include <ipclib.h>
#include <stdbool.h>
#include <ipc-common.h>
static INT16 key;
static INT32 msqid;

#define MAX_LENGTH 	8
#define IPC_GET_JPG 		0
#define IPC_GET_SOMETHING	1

UINT32 msg[MAX_LENGTH];
UINT32 msgsize;
INT32  sndsize;

bool bcontinue = true;

void main()
{
	int fd;
	fd = ipc_open();
	if(fd < 0)
	{
		printf("unable to open device file");
		return -1;
	}
	key = ipc_ftok("demoipc");
	msqid = ipc_msg_get(key);

	while(bcontinue)
	{
		msg[0] = IPC_GET_JPG;
		msgsize = 4;
		sndsize  = ipc_msg_snd(msqid, IPC_SENDTO_CORE1, &msg, msgsize);

		// get ack
		ret = ipc_msg_rcv(msqid, &msg, sizeof(msg));
		if(ret < 0)
		{
			printf("error to receive ack\n");
			bcontinue = false;
			break;
		}

		// do something when receive msg from another core
	}
	ipc_msg_rel(msqid);
	ipc_close();
}


