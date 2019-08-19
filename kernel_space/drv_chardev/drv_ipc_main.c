/*
 * @description : this is main file of ipc driver
 * @author : <ngocgia73@gmail.com>
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#include <ipc_common.h>
#include <drv_ipc_msgque.h>
#include <drv_ipc_api.h>

#define DRV_VERSION "00.00.01"
#define DEVICE_NAME "ipc_msgqueue"

typedef int (*CC_GET_CMD)(IPC_CC_CORE_T core_getfrom, IPC_CC_CMD_T *cmd);
typedef int (*CC_SEND_ACK)(IPC_CC_CORE_T core_sendto);

static dev_t dev_num;
static struct class *cls;
static struct device *dev;
static struct cdev *my_cdev;
// names used to display in /sys/class
const char * class_name = "ipc_msq_sc";


static DEFINE_MUTEX(ipc_mutex);

static struct ipc_device {
	UINT32 storequebit; ///< use to store which msqid is getting 
}

static int ipc_open(struct inode* inode, struct file* file)
{
	struct ipc_device *pdev = NULL;
	pdev = kzalloc(sizeof(struct ipc_device), GFP_KERNEL);
	if(unlikely(!pdev))
	{
		return -ENOMEM;
	}
	// store this pointer into private data field of struct file
	file->private_data = pdev;

	return 0;
}

static int ipc_release(struct inode* inode, struct file* file)
{
	// need release all of msgque
	int i;
	int ret;
	struct ipc_device *pdev = file->private_data;
	if(unlikely(!pdev))
	{
		return -1;
	}
#if 0
	// in case we don't know exactly which queue is geting but not release yet
	for(i = 0; i < IPC_MSG_QUEUE_NUM; i++)
	{
		ret = ipc_msgque_rel(__CORE2, i);
	}
#endif
	for (i = 0; i < IPC_MSG_QUEUE_NUM; i++)
	{
		if(pdev->storequebit & (1 << i))
		{
			// proceed relese 
			ret = ipc_msgque_rel(__CORE2, i);
		}
	}
	if(pdev)
	{
		kfree(pdev);
	}
	return ret;
}

static long ipc_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	int __user *msg_user = (int __user *)arg;
	printk("cmd = 0x%08x", cmd);
	switch(cmd)
	{
		case IOC_IPC_MSGQUE_GET:
			int msqid;
			IPC_IOC_MSGQUE_GET msg_kernel={0};
			struct ipc_device *pdev = file->private_data;
			if(unlikely(!pdev))
			{
				printk(KERN_ERR "pdev is null\n");
				return -EFAULT;
			}
			// get key from user space
			if(unlikely(copy_from_user(&msg_kernel, msg_user, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unable to get key from user space\n");
				return -EFAULT;
			}
			msqid = ipc_msgque_getIDbyKey(__CORE2, msg_kernel.key);
			if(likely(msqid >= 0))
			{
				mutex_lock(&ipc_mutex);
				pdev->storequebit |= (1 << msqid);
				mutex_unlock(&ipc_mutex);
			}
			msg_kernel.rtn = msqid;

			// send back msqid to user space 
			if(unlikely(copy_to_user(msg_user, &msg_kernel, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unablr to send back msqid to user space\n");
				return -EFAULT;
			}
			break;
		case IOC_IPC_MSGQUE_REL:
			IPC_IOC_MSGQUE_REL msg_kernel={0};
			struct ipc_device *pdev = file->private_data;
			if(unlikely(!pdev))
			{
				printk(KERN_ERR "pdev null\n");
				return -EFAULT;
			}
			// get data from user space
			if(unlikely(copy_from_user(&msg_kernel, msg_user, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unable to get msg from user space\n");
				return -EFAULT;
			}
			msg_kernel.rtn = ipc_msgque_rel(__CORE2, msg_kernel.msqid);
			mutex_lock(&ipc_mutex);
			pdev->storequebit &= ~(1 << msg_kernel.msqid); 
			mutex_lock(&ipc_mutex)
			// send back return value to user space
			if(unlikely(copy_to_user(msg_user, &msg_kernel, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unable to send back data to user space\n");
				return -EFAULT;
			}
			break;
		case IOC_IPC_MSGQUE_FTOK:
			IOC_IPC_MSGQUE_FTOK msg_kernel={0};
			// get data from user space
			if(unlikely(copy_from_user(&msg_kernel, msg_user, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unable to get data sent from user space\n");
				return -EFAULT;
			}
			msg_kernel.rtn_key = ipc_msgque_ftok(__CORE2, msg_kernel.path);

			// send back data to user
			if(unlikely(copy_to_user(msg_user, &msg_kernel, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unable to send back data to user space\n");
				return -EFAULT;
			}
			break;
		case IOC_IPC_MSGQUE_SND:
			IPC_IOC_MSGQUE_SND msg_kernel={0};
			// get data from user space
			if(unlikely(copy_from_user(&msg_kernel, msg_user, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unable to get data from user space\n");
				return -EFAULT;
			}
			msg_kernel.rtn = ipc_msg_snd(__CORE2, msg_kernel.msqid, msg_kernel.sendTo, msg_kernel.msg_data , msg_kernel.msg_size, true);
			// send back data to user space
			if(unlikely(copy_to_user(msg_user, &msg_kernel, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unable to send back data to user space\n");
				return -EFAULT;
			}
			break;
		case IOC_IPC_MSGQUE_RCV:
			IPC_IOC_MSGQUE_RCV msg_kernel={0};
			// get data from user space
			if(unlikely(copy_from_user(&msg_kernel, msg_user, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unablr to get data from user space\n");
				return -EFAULT;
			}
			msg_kernel.rtn = ipc_msg_rcv(msg_kernel.msqid, msg_kernel.msg_data, msg_kernel.msg_size, 0); // timeout = 0
			if(msg_kernel.rtn < 0)
			{
				return msg_kernel.rtn;
			}
			// send back data to user space
			if(unlikely(copy_to_user(msg_user, &msg_kernel, sizeof(msg_kernel))))
			{
				printk(KERN_ERR "unable to send back data to user space\n");
				return -EFAULT;
			}
			break;
		
			printk(KERN_ERR "cmd invalid\n");
			break;
	}
}

static const struct file_operation ipc_fops = {
	.owner 		= 	THIS_MODULE,
	.unlocked_ioctl = 	ipc_ioctl,
	.open 		= 	ipc_open,
	.release 	= 	ipc_release,
	.mmap 		= 	ipc_mmap,
};

#ifdef CONFIG_OF
static const struct of_device_id ipc_of_match[] = {
	{.compatible = "ipc_msgqueue"},
	{},
}
#else
#define ipc_of_match NULL
#endif

static UINT32 ipc_get_noncache_addr(UINT32 physical_addr)
{
	UINT32 noncache_addr;

	// check valid
	noncache_addr = (UINT32)physical_addr - (m_ctrl->shm_phy_base - m_ctrl->shm_non_cache_base);
	return noncache_addr;
}	
// function will be called when data is coming
static void ipc_core_handle_func(IPC_CC_CORE_T core_getfrom, CC_GET_CMD get_cmd_func, CC_SEND_ACK send_ack_func)
{
	// cmd used to store data which get from another core
	IPC_CC_CMD_T cmd;
	INT32 msqid;
	UINT32 noncache_addr;
	INT32 ret;
	bool readwait_interruptible = true;
	wait_queue_head_t *read_wq_list;
	get_cmd_func(core_getfrom, &cmd);
	// cmd get from registers include  
	// key -> to know which queue need push data in
	// data_addr -> address of data 
	// data_size -> size of data
	msqid = ipc_msgque_getIDbyKey(__CORE2, (INT32)cmd.opcode);
	if(unlikely(msqid<0))
	{
		printk(KERN_ERR "getIDbyKey failed\n");
		return;
	}

	// need change address to non cache
	// input : physical address
	// output : non cache address (virtual address)	
	noncache_addr = ipc_get_noncache_addr(cmd.data_addr);
	

	// identify which queue need to push data in
	// TODO:	

	// push data to queue 
	// TODO:
	
	ret = ipc_msgque_msgpost(__CORE2, msqid, (void *)noncache_addr, (UINT32)cmd.data_size);
	if(unlikely(ret != 0))
	{
		printk(KERN_ERR "ipc_msgque_msgpost failed\n");
		return;
	}
	
	// send ack to confirm that received already
	send_ack_func(core_getfrom);
	
	// wakeup ipc_msg_rcv to pick up msg from queue
	read_wq_list = &m_ctrl->read_wq_list[msqid];
	readwait_interruptible = m_ctrl->readwait_interruptible[msqid];

	if(readwait_interruptible)
	{
		wake_up_interruptible(read_wq_list);
	}
	else
	{
		wake_up(read_wq_list);
	}
}

static void ipc_core_handler(IPC_CC_EVENT_T evt_comefrom)
{
	if(evt_comefrom == IPC_CC_EVT_FROM_CPU1)
	{
		ipc_core_handle_func(IPC_CC_CORE_CPU1, ipc_cc_get_cmd, ipc_cc_send_ack);
	}
	else if(evt_comefrom == IPC_CC_EVT_FROM_DSP)
	{
		
		ipc_core_handle_func(IPC_CC_CORE_DSP, ipc_cc_get_cmd, ipc_cc_send_ack);
	}
	else
	{
		printk(KERN_ERR "unsupport event come from core %d",evt_comefrom);
	}
}

static int ipc_probe(struct platform_device *pdev)
{
	int ret = 0;
	ret = alloc_chardev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0)
	{
		printk(KERN_ERR "failed to alloc_chrdev_region %d\n",ret);
		ret = -1;
		goto __FAILED_REGISTER_DEVNUM;
	}
	cls = class_create(THIS_MODULE, class_name);
	if(IS_ERR(cls))
	{
		printk(KERN_ERR "failed to create class device\n");
		ret = -2;
		goto __FAILED_CREATE_CLASS_DEVICE;
	}
	dev = device_create(cls, NULL, dev_num, NULL, "ipc_msgqueue_d%d",MINOR(dev_num));
	if(IS_ERR(dev))
	{
		printk(KERN_ERR "failed to create device\n");
		ret = -3;
		goto __FAILED_CREATE_DEVICE;
	}
	my_cdev = cdev_alloc();
	if(IS_ERR(my_cdev))
	{
		printk(KERN_ERR "failed to alloc cdev\n");
		ret = -4;
		goto __FAILED_CDEV_ALLOC;
	}
	cdev_init(my_cdev, &ipc_fops);
	ret = cdev_add(my_cdev, dev_num, 1);
	if(ret < 0)
	{
		ret = -5;
		printk(KERN_ERR "failed to cdev_add\n");
		goto __FAILED_CDEV_ADD;
	}
	m_ctrl = kzalloc(sizeof(IPC_CTRL), GFP_KERNEL);
	if(unlikely(!m_ctrl))
	{
		ret = -6;
		printk(KERN_ERR "m_ctrl is null\n");
		goto __FAILED_CDEV_ADD;
	}
	m_ctrl->shm_phy_base = IPC_SHARED_MEM_ADDR;
	// Remap I/O memory into kernel address space (no cache). 
	m_ctrl->shm_non_cache_base = (UINT32)ioremap_nocache(IPC_SHARED_MEM_ADDR, sizeof(m_ctrl->shm_non_cache_base * MAX_LENGTH));
	if(!m_ctrl->shm_non_cache_base)
	{
		ret = -7;
		printk(KERN_ERR "ioremap_nocache failed\n");
		goto __FAILED_ALLOC_MEM;
	}
	// init wait queue 
	for(i = 0; i < IPC_MSG_QUEUE_NUM; i++)
	{
		init_waitqueue_head(&(m_ctrl->read_wq_list[i]));
	}	

	// register core handler
	ret = ipc_cc_register_handler(IPC_CC_CORE_CPU1, ipc_core_handler);
	if(ret != 0)
	{
		printk(KERN_ERR "register core1 handler failed\n");
		ret = -8;
		goto __FAILED_ALLOC_MEM;
	}
	ret = ipc_cc_register_handler(IPC_CC_CORE_CPU3, ipc_core_handler);
	if(ret != 0)
	{
		printk(KERN_ERR "register core3 handler failed\n");
		ret = -9;
		goto __FAILED_INIT_CORE_HANDLER;
	}
	
	// ipc message queue init
	ret = ipc_msgque_init();
	if(ret != 0)
	{
		ret = -8;
		printk(KERN_ERR "init msg queue failed\n");
		goto __FAILED_INIT_CORE_HANDLER;
	}

	// init task which get data from queue then handle it
	// TODO: for extra feature

	return ret;
__FAILED_INIT_CORE_HANDLER:
	ipc_cc_unregister_handler(IPC_CC_CORE_CPU1);
__FAILED_ALLOC_MEM:
	free(m_ctrl);
__FAILED_CDEV_ADD:
	cdev_del(my_cdev);
__FAILED_CDEV_ALLOC:
	device_destroy(cls, dev_num);
__FAILED_CREATE_CLASS_DEVICE:
	class_destroy(cls);
__FAILED_CREATE_DEVICE:
	unregister_chrdev_region(dev_num, 1);
__FAILED_REGISTER_DEVNUM:
	return ret;
}

static int ipc_remove(struct platform_device *pdev)
{
	int ret;
	cdev_del(my_cdev);
	device_destroy(cls,dev_num);
	class_destroy(cls);
	unregister_chrdev_region(dev_num, 1);
	// iounmap 
	iounmap((void *)m_ctrl->shm_non_cache_base);

	// unregister core handler 
	ret = ipc_cc_unregister_handler(IPC_CC_CORE_CPU1);
	if(ret != 0)
	{
		printk(KERN_ERR "unable to unregister core1 handler");
	}

	ret = ipc_cc_unregister_handler(IPC_CC_CORE_CPU3);
	if(ret != 0)
	{
		printk(KERN_ERR "unable to unregister core3 handler");
	}
	// deinit queue
	ipc_msgque_exit();
	return ret;
}

static int ipc_resume(struct platform_device *pdev)
{

}

static int ipc_suspend(struct platform_device *pdev, pm_message_t state)
{

}

static struct platform_driver ipc_pdriver = {
	.probe 		= 	ipc_probe,
	.remove 	= 	ipc_remove,
	.suspend 	= 	ipc_suspend,
	.resume 	= 	ipc_resume,
	.driver 	= {
		.name 	= DEVICE_NAME,
		.owner 	= THIS_MODULE,
	        .of_match_table = ipc_of_match,	
	},
};

int __init ipc_init(void)
{
	return platform_driver_register(&ipc_pdriver);
}

void __exit ipc_exit(void)
{
	platform_driver_unregister(&ipc_pdriver);
}

arch_initcall(ipc_init);
module_exit(ipc_exit);


MODULE_AUTHOR("giann <ngocgia73@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("drv ipc main");
MODULE_VERSION(DRV_VERSION);
