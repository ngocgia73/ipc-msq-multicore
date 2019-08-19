/*
 * @description : this driver provide mechanism to communicate between multicore
 * @author : <ngocgia73@gmail.com>
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>


#include <cc-reg.h>
#include <ipc-cc.h>

#define DRV_VERSION "00.00.01"

#define IPC_CC_DEFAULT_RUN_CORE IPC_CC_CORE_CPU2

typedef struct ipc_cc_dev_t 
{
	struct device *dev;
	void __iomem *iobase;
	int irq;
	spinlock_t lock;
	IPC_CC_CORE_T running_core;

	void (*cpu1_handler)(IPC_CC_EVENT_T event);
	void (*cpu2_handler)(IPC_CC_EVENT_T event);
	void (*dsp_handler)(IPC_CC_EVENT_T event);
} IPC_CC_DEV_T;

static IPC_CC_DEV_T *ipc_cc_dev = NULL;
static IPC_CC_CORE_T ipc_cc_run_core = IPC_CC_DEFAULT_RUN_CORE;

static inline u32 ipc_cc_read(IPC_CC_DEV_T *dd, u32 offset)
{
	return readl(dd->iobase + offset);
}

static inline void ipc_cc_write(IPC_CC_DEV_T *dd, u32 offset, u32 data)
{
	writel(data, dd->iobase + offset);
}

int ipc_cc_register_handler(IPC_CC_CORE_T core, void (*hdl_func)(IPC_CC_EVENT_T))
{
	int ret = 0;
	IPC_CC_DEV_T *dd = ipc_cc_dev;
	unsigned long flags;

	if(!dd)
	{
		return -1;
	}
	if(core >= IPC_CC_CORE_MAX)
	{
		return -2;
	}
	spin_lock_irqsave(&dd->lock, flags);

	switch(core) {
		case IPC_CC_CORE_CPU1:
			dd->cpu1_handler = hdl_func;
			break;
		case IPC_CC_CORE_CPU2:
			dd->cpu2_handler = hdl_func;
			break;
		case IPC_CC_CORE_DSP:
			dd->dsp_handler = hdl_func;
			break;
		default:
			ret = -1;
			break;
	}
	spin_unlock_irqrestore(&dd->lock, flags);
	return ret;
}
EXPORT_SYMBOL(ipc_cc_register_handler);

int ipc_cc_unregister_handler(IPC_CC_CORE_T core)
{
	int ret = 0;
	IPC_CC_DEV_T *dd = ipc_cc_dev;
	unsigned long flags;

	if(!dd)
	{
		return -1;
	}

	if(core > IPC_CC_CORE_MAX)
	{
		return -2;
	}
	spin_lock_irqsave(&dd->lock, flags);
	switch(dd->running_core) {
		case IPC_CC_CORE_CPU1:
			dd->cpu1_handler = NULL;
			break;
		case IPC_CC_CORE_CPU2:
			dd->cpu2_handler = NULL;
			break;
		case IPC_CC_CORE_DSP:
			dd->dsp_handler = NULL;
			break;
		default:
			ret = -1;
			break;
	}
	spin_unlock_irqrestore(&dd->lock, flags);
	return ret;
}
EXPORT_SYMBOL(ipc_cc_unregister_handler);

// send data to anothor core
int ipc_cc_send_cmd(IPC_CC_CORE_T core_sendto, IPC_CC_CMD_T *cmd)
{
	int ret = 0;
	IPC_CC_DEV_T *dd = ipc_cc_dev;
	unsigned long flags;

	u32 sts_reg_ofs;
	u32 act_reg_ofs;

	// ofs for data
	u32 cmdbuf1_reg_ofs;
	u32 cmdbuf2_reg_ofs;
	u32 cmdbuf3_reg_ofs;

	// 
	CC_CMDBUF1_REG cmdbuf1;
	CC_CMDBUF2_REG cmdbuf2;
	CC_CMDBUF3_REG cmdbuf3;

	if(!dd)
	{
		return -1;
	}

	spin_lock_irqsave(&dd->lock, flags);
	switch(dd->running_core) {
		case IPC_CC_CORE_CPU1:
			break;
		case IPC_CC_CORE_CPU2:
			sts_reg_ofs = CC_CPU2_STS_REG_OFS;
			act_reg_ofs = CC_CPU2_ACT_REG_OFS;
			if(core_sendto == IPC_CC_CORE_CPU1)
			{
				cmdbuf1_reg_ofs = CC_CPU2_CPU1_CMDBUF1_REG_OFS;
				cmdbuf2_reg_ofs = CC_CPU2_CPU1_CMDBUF2_REG_OFS;
				cmdbuf2_reg_ofs = CC_CPU2_CPU1_CMDBUF3_REG_OFS;
			}
			else if(core_sendto == IPC_CC_CORE_DSP)
			{
				cmdbuf1_reg_ofs = CC_CPU2_DSP_CMDBUF1_REG_OFS;
				cmdbuf2_reg_ofs = CC_CPU2_DSP_CMDBUF2_REG_OFS;
				cmdbuf2_reg_ofs = CC_CPU2_DSP_CMDBUF3_REG_OFS;
			}
			else
			{
				printk(KERN_ERR "not support core_sendto %s\n",core_sendto);
				ret = -1;
				goto __EXIT;
			}
			break;
		case IPC_CC_CORE_DSP:
			break;
	}

	// need wait for previous tx ack ready
	// TODO: 

	// clear ack
	ipc_cc_write(dd,sts_reg_ofs, CLEAR_ACK);

	// data which need to send
	cmdbuf1.bit.opcode = cmd->opcode;
	cmdbuf2.bit.data_addr = cmd->data_addr;
	cmdbuf3.bit.data_size = cmd->data_size;

	ipc_cc_write(dd, cmdbuf1_reg_ofs, cmdbuf1.reg);
	ipc_cc_write(dd, cmdbuf2_reg_ofs, cmdbuf2.reg);
	ipc_cc_write(dd, cmdbuf3_reg_ofs, cmdbuf3.reg);
	// need trigger to inform that data is comming 
	ipc_cc_write(dd, act_reg_ofs, DATA_TRIGGER);
__EXIT:
	spin_unlock_irqrestore(&dd->lock, flags);
	return ret;
}
EXPORT_SYMBOL(ipc_cc_send_cmd);

// receive data which sent from another core
int ipc_cc_get_cmd(IPC_CC_CORE_T core_getfrom, IPC_CC_CMD_T *cmd)
{
	int ret = 0;
	unsigned long flags;
	IPC_CC_DEV_T *dd = ipc_cc_get_cmd;
	// 
	u32 cmdbuf1_reg_ofs;
	u32 cmdbuf2_reg_ofs;
	u32 cmdbuf3_reg_ofs;

	//
	CC_CMDBUF1_REG cmdbuf1;

	if(!dd || !cmd)
	{
		return -1;
	}

	if(core_getfrom >= IPC_CC_CORE_MAX)
	{
		printf(KERN_ERR "invalid core %d\n",core_getfrom);
		return -2;
	}
	spin_lock_irqsave(&dd->lock, flags);
	switch(dd->running_core) {
		case IPC_CC_CORE_CPU1:
			if(core_getfrom == IPC_CC_CORE_CPU2)
			{
				cmdbuf1_reg_ofs = CC_CPU2_CPU1_CMDBUF1_REG_OFS;
				cmdbuf2_reg_ofs = CC_CPU2_CPU1_CMDBUF2_REG_OFS;
				cmdbuf3_reg_ofs = CC_CPU2_CPU1_CMDBUF3_REG_OFS;

			}
			else if(core_getfrom == IPC_CC_CORE_DSP)
			{
				cmdbuf1_reg_ofs = CC_DSP_CPU1_CMDBUF1_REG_OFS;
				cmdbuf2_reg_ofs = CC_DSP_CPU1_CMDBUF2_REG_OFS;
				cmdbuf3_reg_ofs = CC_DSP_CPU1_CMDBUF3_REG_OFS;
			}
			else
			{
				ret = -1;
				printk(KERN_ERR "cpu1 is running .not support get cmd from cpu1\n");
				goto __EXIT;
			}
		case IPC_CC_CORE_CPU2:
			if(core_getfrom == IPC_CC_CORE_CPU1)
			{
				cmdbuf1_reg_ofs = CC_CPU1_CPU2_CMDBUF1_REG_OFS;
				cmdbuf2_reg_ofs = CC_CPU1_CPU2_CMDBUF2_REG_OFS;
				cmdbuf3_reg_ofs = CC_CPU1_CPU2_CMDBUF3_REG_OFS;
			}
			else if(core_getfrom == IPC_CC_CORE_DSP)
			{
				cmdbuf1_reg_ofs = CC_DSP_CPU2_CMDBUF1_REG_OFS;
				cmdbuf2_reg_ofs = CC_DSP_CPU2_CMDBUF2_REG_OFS;
				cmdbuf3_reg_ofs = CC_DSP_CPU2_CMDBUF3_REG_OFS;
			}
			else
			{
				ret = -1;
				printk(KERN_ERR "cpu2 is running. not support get cmd from cpu2\n");
				goto __EXIT;
			}
		case IPC_CC_CORE_DSP:
			if(core_getfrom == IPC_CC_CORE_CPU1)
			{
				cmdbuf1_reg_ofs = CC_CPU1_DSP_CMDBUF1_REG_OFS;
				cmdbuf2_reg_ofs = CC_CPU1_DSP_CMDBUF2_REG_OFS;
				cmdbuf3_reg_ofs = CC_CPU1_DSP_CMDBUF3_REG_OFS;
			}
			else if(core_getfrom == IPC_CC_CORE_CPU2)
			{
				cmdbuf1_reg_ofs = CPU2_DSP_CMDBUF1_REG_OFS;
				cmdbuf2_reg_ofs = CPU2_DSP_CMDBUF2_REG_OFS;
				cmdbuf3_reg_ofs = CPU2_DSP_CMDBUF3_REG_OFS;
			}
			else
			{
				ret = -1;
				printk(KERN_ERR "coredsp is running . not support get cmd form dsp core\n ");
				goto __EXIT;
			}
		default:
			printk("not support core %d\n",dd->running_core);
			ret = -1;
			goto __EXIT:
	}
	// get data from register then pass to cmd
	cmdbuf1.reg = ipc_cc_read(dd, cmdbuf1_reg_ofs);
	cmd.opcode = cmdbuf1_reg_ofs.bit.opcode;
	cmd.data_addr = ipc_cc_read(dd, cmdbuf2_reg_ofs);
	cmd.data_size = ipc_cc_read(dd, cmdbuf3_reg_ofs);
__EXIT:
	spin_unlock_irqrestore(&dd->lock, flags);
	return ret;
}
EXPORT_SYMBOL(ipc_cc_get_cmd);


// send ack after receive data
int ipc_cc_send_ack(IPC_CC_CORE_T core_sendto)
{
	int ret = 0;
	IPC_CC_DEV_T *dd = ipc_cc_dev;
	unsigned long flags;

	if(!dd)
	{
		return -1;
	}
	if(core >= IPC_CC_CORE_MAX)
	{
		return -2;
	}
	spin_lock_irqsave(dd->lock, flags);
	if(core_sendto == dd->running_core)
	{
		printk(KERN_ERR "not support ack to same core\n");
		ret = -3;
		goto __EXIT;
	}
	switch(dd->running_core) {
		case IPC_CC_CORE_CPU1:
			ipc_cc_write(dd, CC_CPU1_ACK_REG_OFS, (0x1<<(core_sendto+16)));
			break;
		case IPC_CC_CORE_CPU2:
			ipc_cc_write(dd, CC_CPU2_ACK_REG_OFS, (0x1<<(core_sendto+16)));
			break;
		case IPC_CC_CORE_DSP:
			ipc_cc_write(dd, CC_DSP_ACK_REG_OFS, (0x1<<(core_sendto+16)));
			break;
		default:
			ret = -4;
			goto __EXIT;
	}

__EXIT:
	spin_unlock_irqrestore(dd->lock, flags);
	return ret;
}	
EXPORT_SYMBOL(ipc_cc_send_ack);
// enable interupt
void ipc_cc_resume(void)
{
	IPC_CC_DEV_T *dd = ipc_cc_dev;
	unsigned long flags;
	if(!dd)
	{
		return -1;
	}

	spin_lock_irqsave(&dd->lock, flags);
	if(dd->irq > 0)
	{
		enable_irq(dd->irq);
	}
	spin_unlock_irqrestore(&dd->lock, flags);
}
EXPORT_SYMBOL(ipc_cc_resume);

// disable interupt
void ipc_cc_suspend(void)
{
	IPC_CC_DEV_T *dd = ipc_cc_dev;
	unsigned long flags;
	if(!dd)
	{
		return -1;
	}

	spin_lock_irqsave(&dd->lock, flags);
	if(dd->irq > 0)
	{
		disable_irq(dd->irq);
	}
	spin_unlock_irqrestore(&dd->lock, flags);

}
EXPORT_SYMBOL(ipc_cc_suspend);

static int ipc_cc_open(IPC_CC_DEV_T *dd)
{
	int ret = 0;
	CC_CPUX_STS_REG cpux_sts;
	CC_CPUX_INTEN_REG cpux_inten;

	spin_lock(&dd->lock);
	switch(dd->running_core) {
		case IPC_CC_CORE_CPU1:
			break;
		case IPC_CC_CORE_CPU2:
			// we are running on cpu2
			// clear ack
			// read status register value first 
			cpux_sts.reg = ipc_cc_read(dd, CC_CPU2_STS_REG_OFS);
			cpux_sts.bit.ack_from_cpu1 = 0;
			cpux_sts.bit.ack_from_cpu2 = 0;
			cpux_sts.bit.ack_from_dsp  = 0;
			// write back 
			ipc_cc_write(dd, CC_CPU2_STS_REG_OFS, cpux_sts.reg);

			// enable event interupt
			cpux_inten.reg = ipc_cc_read(dd, CC_CPU2_INTEN_REG_OFS);
			cpux_inten.bit.event_from_cpu1_inten = 1;
			cpux_inten.bit.event_from_cpu2_inten = 0;
			cpux_inten.bit.event_from_dsp_inten = 1;
			ipc_cc_write(dd, CC_CPU2_INTEN_REG_OFS, cpux_inten.reg);
			break;
		case IPC_CC_CORE_DSP:
			break;
		default:
			ret = -1;
			goto __EXIT;
	}
__EXIT:
	spin_unlock(&dd->lock);
	return ret;
}

static void ipc_cc_close(IPC_CC_DEV_T *dd)
{
	int ret = 0;
	CC_CPUX_INTEN_REG cpux_inten;

	spin_lock(&dd->lock);

	switch(dd->running_core) {
		case IPC_CC_CORE_CPU1:
			break;
		case IPC_CC_CORE_CPU2:
			// disable event interupt
			cpux_inten.reg = ipc_cc_read(dd, CC_CPU2_INTEN_REG_OFS);
			cpux_inten.bit.event_from_cpu1_inten = 0;
			cpux_inten.bit.event_from_dsp_inten = 0;
			ipc_cc_write(dd, CC_CPU2_INTEN_REG_OFS, cpux_inten.reg);
		case IPC_CC_CORE_CPU2:
			break;
		default:
			ret = -1;
			goto __EXIT;
__EXIT:
	spin_unlock(&dd->lock);
	}
}

// function interupt handler
static irqreturn_t ipc_cc_irq_handler(int irq, void *dev_id)
{
	IPC_CC_DEV_T *dd = (IPC_CC_DEV_T *)dev_id;
	unsigned long flags;
	CC_CPUX_STS_REG cpux_sts;
	CC_CPUX_INTEN_REG cpux_inten;


	spin_lock_irqsave(&dd->lock, flags);

	switch(dd->running_core) {
		case IPC_CC_CORE_CPU1:
			break;
		case IPC_CC_CORE_CPU2:
			cpux_sts = ipc_cc_read(dd, CC_CPU2_STS_REG_OFS);
			cpux_inten = ipc_cc_read(dd, CC_CPU2_INTEN_REG_OFS);
			break;
		case IPC_CC_CORE_DSP:
			break;
		default:
			spin_unlock_irqrestore(&dd->lock, flags);
			goto __EXIT;
	}

	spin_unlock_irqrestore(&dd->lock,flags);

	// handle event come from cpu1
	if(cpux_sts.bit.event_from_cpu1 && cpux_inten.bit.event_from_cpu1_inten)
	{
		if(dd->running_core == IPC_CC_CORE_CPU1)
		{
			printk(KERN_ERR "not support cpu1\n");
		}
		else if(dd->cpu1_handler)
		{
			dd->cpu1_handler(IPC_CC_EVT_FROM_CPU1);
		}
	}
	// handle event come from cpu2
	if(cpux_sts.bit.event_from_cpu2 && cpux_inten.bit.event_from_cpu2_inten)
	{
		if(dd->running_core == IPC_CC_CORE_CPU2)
		{
			printk(KERN_ERR "not support cpu2\n");
		}
		else if(dd->cpu2_handler)
		{
			dd->cpu2_handler(IPC_CC_EVT_FROM_CPU2);
		}
	}
	// handle event come from dsp
	if(cpux_sts.bit.event_from_dsp && cpux_inten.bit.event_from_dsp_inten)
	{
		if(dd->running_core == IPC_CC_CORE_DSP)
		{
			printk(KERN_ERR "not support dsp\n");
		}
		else if(dd->dsp_handler)
		{
			dd->dsp_handler(IPC_CC_EVT_FROM_DSP);
		}
	}
__EXIT:
	return IRQ_HANDLED;
}

#ifdef CONFIG_OF
static int ipc_cc_of_probe(struct platform_device *pdev, IPC_CC_DEV_T *cdev)
{
	struct device_node *pdn = pdev->dev.of_node;
        u32 core_id ;
	if(of_property_read_u32(pdn, "run-core", &core_id))
	{
		printk(KERN_ERR " missing required parameter run-core\n");
		return -ENODEV;
	}
	if(core_id < IPC_CC_CORE_MAX)
	{
		ipc_cc_run_core = core_id;
	}
	else
	{
		ipc_cc_run_core = IPC_CC_DEFAULT_RUN_CORE;
	}
	return 0;
}
#else
#define ipc_cc_of_probe(pdev, cdev) -ENODEV
#endif
static int ipc_cc_probe(struct platform_device *pdev)
{
	int ret = -1;
	struct resource *res = NULL;
	IPC_CC_DEV_T *dd = NULL;
	dd = ipc_cc_dev = devm_kzalloc(&pdev->dev, sizeof(IPC_CC_DEV_T), GFP_KERNEL);
	if(!dd)
	{
		printk(KERN_ERR "can not alloc mem for private data struct\n");
		return -ENOMEM;
	}
	// get running core
	if(ipc_cc_of_probe(pdev, dd) != 0)
	{
		// use default running core
		ipc_cc_run_core = IPC_CC_DEFAULT_RUN_CORE;
	}

	dd->dev = &pdev->dev;
	dd->irq = -1;
	dd->running_core = ipc_cc_run_core;
	platform_set_drvdata(pdev, dd);
	
	// init spin lock
	spin_lock_init(&dd->lock);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res)
	{
		printk(KERN_ERR "no mem resource info\n");
		return -ENODEV;
	}
	dd->irq = platform_get_irq(pdev, 0);
	if(dd->irq < 0)
	{
		printk(KERN_ERR "no irq resource info\n");
		return dd->irq;
	}

	// input : physical address
	// output: virtual address
	dd->iobase = devm_ioremap_resource(&pdev->dev,res);
	if(IS_ERR(dd->iobase))
	{
		printk(KERN_ERR "can not ioremap\n");
		return PTR_ERR(dd->ioremap);
	}
	// request irq
	ret = devm_request_irq(&pdev->dev, dd->irq, ipc_cc_irq_handler , 0 , dev_name(&pdev->dev,dd));
	if(ret)
	{
		printk(KERN_ERR "unable to request IRQ\n");
		return ret;
	}
	ret = ipc_cc_open(dd);
	if(ret)
	{
		printk(KERN_ERR "ipc_cc_open failed\n");
		return ret;
	}
	printk(KERN_INFO "ipc_cc_open registered \n");
	return 0;

}

static int ipc_cc_remove(struct platform_device *pdev)
{
	IPC_CC_DEV_T *dd = platform_get_drvdata(pdev);
	ipc_cc_close(dd);
	return 0;
}


#ifdef CONFIG_OF
static const struct  of_device_id ipc_cc_ids[] = {
	{
		.compatible = 	"ipc,ipc_cc"
	},
	{},
};
#else
#define ipc_cc_ids = NULL
#endif

static struct platform_driver ipc_cc_platform_driver = {
	.driver = {
		.name  			= 	"ipc_cc",
		.owner 			= 	THIS_MODULE,
		.of_match_table  	= 	of_match_ptr(ipc_cc_ids),
	},
	.probe 		= 	ipc_cc_probe,
	.remove 	= 	ipc_cc_remove,
};

static int __init ipc_cc_init(void)
{
	return platform_driver_register(&ipc_cc_platform_driver);
}

static void __exit ipc_cc_exit(void)
{
	platform_driver_unregister(&ipc_cc_platform_driver);
}
core_initcall(ipc_cc_init);
module_exit(ipc_cc_exit);

MODULE_AUTHOR("giann");
MODULE_DESCRIPTION("IPC-CC driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
