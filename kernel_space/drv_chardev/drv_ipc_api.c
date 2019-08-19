#include "drv_ipc_api.h"
#include "drv_ipc_msgque.h"

/*
 * get data from user space then pass to drv-cc to tranfer to other core
 */
INT32 ipc_msg_snd(unsigned long coreid, UINT32 msqid, IPC_SENTO sendTo, void *msg_data, UINT32 msg_size, bool via_ioctl)
{
	IPC_MSGQUE *pmsgque = NULL;
	IPC_SEND_MSG *p_sendmsg = NULL;
	IPC_CC_CMD_T ccCmd;
	pmsgque = ipc_msgque_getQuebyID(coreid, msqid);
	if(unlikely(!pmsgque))
	{
		printk(KERN_ERR "pmsgque is null\n");
		return -1;
	}
	if(unlikely(!msg_data))
	{
		printk(KERN_ERR "msg_data is null\n");
		return -2;
	}
	if(unlikely(sendTo > IPC_SENDTO_MAX))
	{
		printk(KERN_ERR "senTo is invalid\n");
		return -3;
	}
	if(unlikely(msg_size > IPC_MSG_ELEMENT_SIZE))
	{
		printk(KERN_ERR "size need to send is limit\n");
		return -4;
	}

	// protect sent data and get back pingpong buff
	p_sendmsg = ipc_msgque_lockSndMsgBuff(coreid, sendTo);
	if(unlikely(!p_sendmsg))
	{
		printk(KERN_ERR "p_sendmsg is null\n");
		return -5;
	}
	// pass data which need to send into pingpong buff
	p_sendmsg->sendTo = sendTo;
	memcpy(&(p_sendmsg->msg_data), msg_data, msg_size);
	if(likely(p_sendmsg->sendTo == IPC_SENDTO_CORE1))
	{
		// TODO:
		ccCmd.opcode = pmsgque->sharekey;
		// now data placed in RAM . but we need identify physical address of data 
		// then transfer phyaddr to others core
		ccCmd.data_addr = ipc_msgque_getPhyAddr((UINT32)&(p_sendmsg->msg_data));
		ccCmd.data_size = msg_size;
		ret = ipc_cc_send_cmd(IPC_CC_CORE_CPU1, &ccCmd);
	}
	else if(likely(p_sendmsg == IPC_SENDTO_CORE3))
	{
		// TODO:
		ccCmd.opcode = pmsgque->sharekey;
		// now data placed in RAM . but we need identify physical address of data 
		// then transfer phyaddr to others core
		ccCmd.data_addr = ipc_msgque_getPhyAddr((UINT32)&(p_sendmsg->msg_data));
		ccCmd.data_size = msg_size;
		ret = ipc_cc_send_cmd(IPC_CC_CORE_DSP, &ccCmd);
	}
	else
	{
		printk(KERN_ERR "err : send data to same core\n");
		return -6;
	}

	ipc_msgque_unlockSndMsgBuff(coreid, sendTo);
}

/*
 * get data from drv-cc then push back to user space everytime user request
 */
INT32 ipc_msg_rcv(unsigned long coreid, UINT32 msqid, void *msg_data, UINT32 msg_size, UINT32 timeout_ms)
{
	int ret;
	IPC_MSGQUE *pmsgque = NULL;
	// msg_drv is msg share bw drv_cc and drv_chardev
	IPC_MSG msg_drv;
	wait_queue_head_t *pread_wq_list = NULL;
	bool *p_interruptible;
	UINT32 sharekey;
	if(unlikely(!msg_data))
	{
		printk(KERN_ERR "msg_data is null\n");
		return -EFAULT;
	}

	pread_wq_list = &(m_ctrl->read_wq_list[msqid]);
	if(unlikely(!pread_wq_list))
	{
		printk(KERN_ERR "pread_wq_list is null\n");
		retrun -EFAULT;
	}
	p_interruptible = &(m_ctrl->readwait_interruptible[msqid]);
	if(unlikely(!p_interruptible))
	{
		printk(KERN_ERR "p_interruptible is null \n");
		return -EFAULT;
	}

	// identify which queue we're getting data 
	pmsgque = ipc_msgque_getQuebyID(msqid);
	if(unlikely(!pmsgque))
	{
		printk(KERN_ERR "ipc_msgque_getQuebyID failed\n");
		return -EFAULT;
	}
	sharekey = pmsgque->sharekey;
	while(ipc_msgque_isempty(pmsgque))
	{
		// TODO:
		// need sleep waiting until out of timeout value in case of we pass timeout val
		if(timeout_ms)
		{
			ret = wait_event_timeout(*pread_wq_list, (!ipc_msgque_isempty(pmsgque)) || (!ipc_msgque_isvalid(pmsgque, sharekey)), msecs_to_jiffies(timeout_ms));
			*p_interruptible = false;
		}
		// otherwise sleep waiting here until have interupted event
		else
		{
			ret = wait_event_interruptible(*pread_wq_list, (!ipc_msgque_isempty(pmsgque)) || (!ipc_msgque_isvalid(pmsgque, sharekey)));
			*p_interruptible = true;
		}
		// TODO : need check somes case before proceed handle message
		// check msq is released or not
		if(!ipc_msgque_isvalid(pmsgque, sharekey))
		{
			printk(KERN_ERR "msq is invalid\n");
			return -1;
		}
		// check timeout
		if(ret == 0 && timeout_ms)
		{
			printk(KERN_ERR "wait for msg timeout\n");
			return -2;
		}

		// check queue is empty or not
		if(ipc_msgque_isempty(pmsgque))
		{
			printk(KERN_ERR "queue is empty");
			return -3;
		}
	}
	// have data in queue . proceed to handle data
	// TODO : get data from queue then pass it to msg_data pointer
	if(ipc_msgque_dequeue(pmsgque, &msg_drv) != 0)
	{
		printk(KERN_ERR "dequeue failed\n");
		return -4;
	}
	if(unlikely(msg_size < msg_drv.size))
	{
		printk(KERN_ERR "user request size smaller than size of data sent from another core");
		return -5;
	}
	memcpy(msg_data, msg_drv.data, sizeof(msg_drv.size));
	return msg_drv.size;
}
