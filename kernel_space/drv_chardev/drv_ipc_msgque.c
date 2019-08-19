/*
 * this file will handle the ipc message queue operations
 * author <ngocgia73@gmail.com>
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include <drv_ipc_msgque.h>
#include <ipc-common.h>

static IPC_MSQINFO *g_msqinfo = NULL;
static DEFINE_SPINLOCK(lock);

static UINT32 ipc_calcNeddBuffSize(void)
{
	UINT32 buffsize;
	buffsize = (sizeof(IPC_SEND_MSG) * 2) * IPC_SENDTO_MAX;
	return buffsize;
}

// implement init function
INT32 ipc_msgque_init(void)
{
	UINT32 i;
	int ret;
	UINT32 tmpbuff;
	IPC_MSGQUE *p_msgque;

	if(g_msqinfo)
	{
		printk(KERN_INFO "ipc message queue inited already\n");
		return -1;
	}

	g_msqinfo = (IPC_MSQINFO *)kmalloc(sizeof(IPC_MSQINFO), GFP_KERNEL);
	if(!g_msqinfo)
	{
		printk(KERN_ERR "unable allocate memory for IPC_MSQINFO \n");
		return -ENOMEM;
	}
	// this is consistent memory
	g_msqinfo->syscmdbuf_vir = dma_alloc_coherent(NULL, ipc_calcNeddBuffSize(), (dma_addr_t *)&(g_msqinfo->syscmdbuf_dma), GFP_KERNEL);
	if(unlikely(!(g_msqinfo->syscmdbuf_vir)))
	{
		printk(KERN_ERR "unable to allocate mem for syscmdbuf_vir\n");
		ret = -ENOMEM;
		goto __ERR1;
	}
	tmpbuff = (UINT32)g_msqinfo->syscmdbuf_vir;

	for (i = 0; i< IPC_MSG_QUEUE_NUM ; i++)
	{
		p_msgque = &(g_msqinfo->msgQueTbl[i]);
		p_msgque->initkey = 0;

		// reset token string
		g_msqinfo->msgQueTokenStr[i][0] = 0;
	}

	for(i = 0; i < IPC_SENDTO_MAX; i++)
	{
		sema_init(&(g_msqinfo->sndmsg_buffTbl[i].semid), 1); // init semaphore like mutex
		g_msqinfo->sndmsg_buffTbl[i].pingponidx = 0;
		g_msqinfo->sndmsg_buffTbl[i].sndmsg[0] = (IPC_SEND_MSG*)tmpbuff;
		tmpbuff += sizeof(IPC_SEND_MSG);
		g_msqinfo->sndmsg_buffTbl[i].sndmsg[1] = (IPC_SEND_MSG*)tmpbuff;
		tmpbuff += sizeof(IPC_SEND_MSG);
	}
	if(tmpbuff - g_msqinfo->syscmdbuf_vir != ipc_calcNeddBuffSize())
	{
		printk(KERN_ERR "error calc buff\n");
		ret = -EFAULT;
		goto __ERR2;
	}
	return 0;
__ERR2:
	dma_free_coherent(NULL, ipc_calcNeddBuffSize(), g_msqinfo->syscmdbuf_vir, (dma_addr_t)g_msqinfo->syscmdbuf_dma);
__ERR1:
	kfree(g_msqinfo);
	g_msqinfo = NULL;
	return ret;
}

void ipc_msgque_exit(void)
{
	UINT32 i;
	IPC_MSGQUE *p_msgque;
	
	if(unlikely(!g_msqinfo))
	{
		printk(KERN_ERR "g_msqinfo is null\n");
		return;
	}
	for(i = 0; i < IPC_MSG_QUEUE_NUM; i++)
	{
		p_msgque = &(g_msqinfo->msgQueTbl[i]);
		p_msgque->initkey = 0;
	}

	dma_free_coherent(NULL, ipc_calcNeddBuffSize(), g_msqinfo->syscmdbuf_vir, (dma_addr_t)g_msqinfo->syscmdbuf_dma);
	kfree(g_msqinfo);
	g_msqinfo = NULL;
}

// @input : name -> pname
// @output: key  -> hash
// @return: 0 if error otherwise success
static UINT16 ipc_msgque_namehash(const char *pname)
{
	UINT16 hash = 0;
	//TODO: 
	const char *pchar = pname;
	if(!pname)
	{
		printk(KERN_ERR "pname is null");
		return 0;
	}
	// refer to open source 
	while((*pchar) != 0)
	{
		hash = ((hash&1) ? 0x8000 : 0) + (hash>>1) + (*pchar);
		pchar++;
	}
	printk(KERN_INFO "name = %s key = %d",pname, hash);
	return hash;
}
UINT16 ipc_msgque_ftok(unsigned long coreid, const char *path)
{
	unsigned long flags;
	int i;
	char *queuetoken = NULL;

	if(unlikely(!g_msqinfo))
	{
		printk(KERN_ERR "ipc msq not init\n");
		return -1;
	}
	spin_lock_irqsave(&lock, flags);
	for(i = 0; i < IPC_MSG_QUEUE_NUM; i++)
	{
		queuetoken = g_msqinfo->msgQueTokenStr[i];
		if(queuetoken[0] == 0)
		{
			strncpy(queuetoken, path, IPC_MSG_QUEUE_TOKEN_STR_MAXLEN);
			printk(KERN_INFO "tokenstr[%d] = %s\n", i, queuetoken);
			break;
		}

	}
	spin_unlock_irqrestore(&lock, flags);
	return ipc_msgque_namehash(path);
}

// input : key 
// output: msqid
INT32 ipc_msgque_get(unsigned long coreid, INT32 key)
{
	int i = 0;
	IPC_MSGQUE *p_msgque;
	unsigned long flags;
	if(unlikely(!g_msqinfo))
	{
		printk(KERN_ERR "ipc msgque not init yet\n");
		return -1;
	}
	spin_lock_irqsave(&lock, flags);
	for(i = 0 ; i < IPC_MSG_QUEUE_NUM ; i++)
	{
		p_msgque = &(g_msqinfo->msgQueTbl[i]);
		if((p_msgque->initkey == CFG_IPC_INIT_KEY) && (key == p_msgque->sharekey))
		{
			spin_unlock_irqrestore(&lock, flags);
			printk(KERN_ERR "generate key 0x%04x is duplicate\r\n",key);
			return -1;
		}
	}
	if(i = 0 ; i < IPC_MSG_QUEUE_NUM; i++)
	{
		p_msgque = &(g_msqinfo->msgQueTbl[i]);
		if(p_msgque->initkey != CFG_IPC_INIT_KEY)
			break;
	}
	if(unlikely(i>=IPC_MSG_QUEUE_NUM))
	{
		spin_unlock_irqrestore(&lock, flags);
		printk(KERN_ERR "all of queue are used\n");
		return -2;
	}

	p_msgque->initkey = CFG_IPC_INIT_KEY;
	p_msgque->msqid = i;
	p_msgque->sharekey = key;
	p_msgque->indx = IPC_MSG_ELEMENT_NUM - 1;
	p_msgque->outdx = 0;
	p_msgque->count = 0;
	spin_unlock_irqrestore(&lock, flags);

	printk(KERN_INFO "msqid = %d, sharekey = 0x%4x\n",p_msgque->msqid, p_msgque->sharekey);

	return p_msgque->msqid;
}

// get msq id by key
// input -> key
// output -> queid
INT32 ipc_msgque_getIDbyKey(unsigned long coreid, INT32 key)
{
	UINT32 i;
	IPC_MSGQUE *pmsgque = NULL;
	if(unlikely(!g_msqinfo))
	{
		return -1;
	}
	for(i=0; i<IPC_MSG_QUEUE_NUM; i++)
	{
		pmsgque = &(g_msqinfo->msgQueTbl[i]);
		if(!pmsgque)
		{
			return -2
		}
		if(pmsgque->initkey == CFG_IPC_INIT_KEY && (UINT32)key == pmsgque->sharekey)
		{
			return pmsgque->msqid;
		}
	}
	return -3;
}



// implement function get queue by id

IPC_MSGQUE *ipc_msgque_getQuebyID(unsigned long coreid, UINT32 msqid)
{
	int i = 0;
	IPC_MSGQUE *p_msgque = NULL;
	if(unlikely(!g_msqinfo))
	{
		printk(KERN_ERR "msg queue not init yet\n");
		return NULL;
	}
	if(msqid >= IPC_MSG_QUEUE_NUM)
	{
		printk(KERN_ERR "msqid invalid\n");
		return NULL;
	}
	//for(i = 0; i < IPC_MSG_QUEUE_NUM; i++)
	//{
	//	p_msgque = &(g_msqinfo->msgQueTbl[i]);
	//	if(p_msgque->msqid == msqid)
	//		break;
	//}
	p_msgque = &(g_msqinfo->msgQueTbl[msqid]);
	if(p_msgque->initkey != CFG_IPC_INIT_KEY)
	{
		printk("msqid %d is not initial\n",msqid);
		return NULL;
	}
	return p_msgque;
}


// implement function get token by key
char *ipc_msgque_getTokenByKey(unsigned long coreid, UINT32 key)
{
	int i = 0;
	if(unlikely(!g_msqinfo))
	{
		printk("msg not initial yet\n");
		return NULL;
	}

	for(i = 0; i < IPC_MSG_QUEUE_NUM; i++)
	{
		if(ipc_msgque_namehash(g_msqinfo->msgQueTokenStr[i]) == key)
		{
			return g_msqinfo->msgQueTokenStr[i];
		}
	}
	return NULL;
}

INT32 ipc_msgque_rel(unsigned long coreid, UINT32 msqid)
{
	IPC_MSGQUE *p_msgque =NULL;
	unsigned long flags;
	char *token = NULL;
	spin_lock_irqsave(&lock, flags);

	p_msgque = ipc_msgque_getQuebyID(coreid, msqid);

	if(unlikely(!p_msgque))
	{
		spin_unlock_irqrestore(&lock, flags);
		printk(KERN_ERR "uable get queue bu ID\n");
		return -1;
	}
	token = ipc_msgque_getTokenByKey(coreid, p_msgque->sharekey);
	if(token)
	{
		printk(KERN_INFO "release token %s\n",token);
		token[0]=0;
	}
	else
	{
		printk(KERN_ERR "unable to get token by key\n");
		return -2;
	}
	p_msgque->initkey = 0;
	spin_unlock_irqrestore(&lock, flags);
	return 0;
}

bool ipc_msgque_isempty(IPC_MSGQUE *msgque)
{
	if(!msgque)
	{
		printk(KERN_ERR "msgque is null\n");
		return true;
	}
	if(msgque->indx == msgque->outdx)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static UINT32 ipc_msgque_getElementCount(IPC_MSGQUE *msgque)
{
	if(!msgque)
	{
		return -1;
	}
	else
	{
		return msgque->count;	
	}
}

bool ipc_msgque_isfull(IPC_MSGQUE *msgque)
{
	UINT32 count;
	if(!msgque)
	{
		printk(KERN_ERR "msgque is null\n");
		return true;
	}
	count = ipc_msgque_getElementCount(msgque);
	if(count >= IPC_MSG_ELEMENT_NUM)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ipc_msgque_isvalid(IPC_MSGQUE *pmsgque, UINT32 sharekey)
{
	if(!pmsgque)
	{
		printk(KERN_ERR "msgque is null\n");
		return false;
	}
	if(likely(pmsgque->initkey == CFG_IPC_INIT_KEY && sharekey = pmsgque->sharekey))
	{
		return true;
	}
	else
	{
		return false;
	}
}

INT32 ipc_msgque_enqueue(IPC_MSGQUE *msgque, IPC_MSG *msg)
{
	IPC_MSG *Tmpmsgque = NULL;
	unsigned long flags;
	INT32 ret = 0;
	spin_lock_irqsave(&long, flags);
	if(ipc_msgque_isfull(msgque))
	{
		return -1;
	}
	msgque->indx = ((msgque->indx + 1) % IPC_MSG_ELEMENT_NUM);
	Tmpmsgque = &(msgque->element[msgque->indx]);
	if(Tmpmsgque)
	{
		Tmpmsgque->size = msg->size;
		memcpy(Tmpmsgque->data, msg->data, msg->size);
		msgque->count++;
	}
	else
	{
		ret = -2;
	}
	spin_unlock_irqrestore(&long, flags);
	return ret;
}

/*
 * will called as soon as data is coming to get data which store in register then push it into queue
 *
 */
INT32 ipc_msgque_msgpost(unsigned long coreid, UINT32 msqid, void *pmsgaddr, UINT32 msgsize)
{
	IPC_MSGQUE *pmsgque = NULL;
	IPC_MSG rcvmsg;
	INT32 ret;
	unsigned long flags;
	printk(KERN_DEBUG "msqid = %d msg addr = 0x%08x msg size = %d",msqid, noncache_addr, cmd.data_size);

	if(unlikely(!pmsgaddr))
	{
		printk(KERN_ERR "msgaddr is null\n");
		return -1;
	}
	if(msgsize > IPC_MSG_ELEMENT_SIZE)
	{
		printk(KERN_ERR "msgsize is over limit\n");
		return -2;
	}
	spin_lock_irqsave(&lock, flags);
	pmsgque = ipc_msgque_getQuebyID(coreid, msqid);
	if(unlikely(!pmsgque))
	{
		printk(KERN_ERR "ipc_msgque_getQuebyID failed\n");
		spin_unlock_irqrestore(&lock, flags);
		return -3;
	}
	memcpy(&rcvmsg.data, pmsgaddr, msgsize);
	rcvmsg.size = msgsize;

	ret = ipc_msgque_enqueue(pmsgque, &rcvmsg);
	if(unlikely(ret != 0))
	{
		spin_unlock_irqrestore(&lock, flags);
		return -4;
	}

	spin_unlock_irqrestore(&lock, flags);
	return ret;
}

INT32 ipc_msgque_dequeue(IPC_MSGQUE *msgque, IPC_MSG *msg)
{
	IPC_MSG *Tmpmsgque = NULL;
	unsigned long flags;
	spin_lock_irqsave(&lock, flags);
	if(ipc_msgque_isempty(msgque))
	{
		return -1;
	}
	Tmpmsgque = &(msgque->element[msgque->outdx]);
	if(Tmpmsgque)
	{
		msg->size= Tmpmsgque->size; 
		msgque->outdx = (msgque->outdx+1) % IPC_MSG_ELEMENT_NUM;
		memcpy(msg->data, Tmpmsgque->data, Tmpmsgque->size);
		msgque->count--;
	}
	else
	{
		ret = -2;
	}
	spin_unlock_irqrestore(&long, flags);
	return 0;

}

IPC_SEND_MSG* ipc_msgque_lockSndMsgBuff(unsigned long coreid, IPC_SENTO sendTo)
{
	IPC_SNDMSG_BUFF *psndmsgbuf = NULL;
	if(unlikely(!g_msqinfo))
	{
		printk(KERN_ERR "g_msqinfo is null\n");
		return NULL;
	}
	psndmsgbuf = g_msqinfo->sndmsg_buffTbl[sendTo - 1];
	if(unlikely(!psndmsgbuf))
	{
		printk(KERN_ERR "psndmsgbuf is null\n");
		return NULL;
	}
	down_interruptible(&psndmsgbuf.semid);

	if(psndmsgbuf.pingponidx == 0)
	{
		psndmsgbuf.pingponidx = 1;
		return psndmsgbuf.sndmsg[0];
	}
	else
	{
		psndmsgbuf.pingponidx = 0
		return psndmsgbuf.sndmsg[1];
	}
}

void ipc_msgque_unlockSndMsgBuff(unsigned long coreid, IPC_SENTO sendTo)
{
	IPC_SNDMSG_BUFF *psndmsgbuf = NULL;
	if(unlikely(!g_msqinfo))
	{
		printk(KERN_ERR "g_msqinfo is null\n");
		return;
	}
	psndmsgbuf = g_msqinfo->sndmsg_buffTbl[sendTo -1];
	up(&psndmsgbuf.semid);
}


UINT32 ipc_msgque_getPhyAddr(UINT32 virt_addr)
{
	if(unlikely(!g_msqinfo))
	{
		printk(KERN_ERR "g_msqinfo is null\n");
		return virt_addr;
	}
	else
	{
		// (g_msqinfo->syscmdbuf_vir - g_msqinfo->syscmdbuf_dma)
		// size bw virtual addr and physical address
		return virt_addr - (g_msqinfo->syscmdbuf_vir - g_msqinfo->syscmdbuf_dma);
	}
}
