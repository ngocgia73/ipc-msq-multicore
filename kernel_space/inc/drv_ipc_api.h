#ifndef __DRV_IPC_API_H__
#define __DRV_IPC_API_H__

#include "/linux/types.h"

struct ipc_ctrl
{
	bool readwait_interruptible[IPC_MSG_QUEUE_NUM];
	wait_queue_head_t read_wq_list[IPC_MSG_QUEUE_NUM];
	UINT32 shm_phy_base; 		// share mem phy addr
	UINT32 shm_non_cache_base; 	// share mem vir addr
} IPC_CTRL;

extern IPC_CTRL *m_ctrl;

INT32 ipc_msg_snd(unsigned long coreid, UINT32 msqid, IPC_SENTO sendTo, void *msg_data, UINT32 msg_size, bool via_ioctl);

INT32 ipc_msg_rcv(unsigned long coreid, UINT32 msqid, void *msg_data, UINT32 msg_size, UINT32 timeout_ms);

#endif //__DRV_IPC_API_H__
