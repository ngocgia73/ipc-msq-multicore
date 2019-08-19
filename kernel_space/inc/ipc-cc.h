#ifndef _IPC_CC_H_
#define _IPC_CC_H_

/************************************************************************************************
 *  CC Core Index Definition
 *  ********************************************************************************************/

typedef enum ipc_cc_core_t
{
	IPC_CC_CORE_CPU1 = 0,
	IPC_CC_CORE_CPU2,
	IPC_CC_CORE_CPU3,
	IPC_CC_CORE_MAX
} IPC_CC_CORE_T;

/************************************************************************************************
 * CC Event Definition
 ***********************************************************************************************/

typedef enum ipc_cc_event_t
{
	IPC_CC_EVT_FROM_CPU1 = 0,
	IPC_CC_EVT_FROM_CPU2,
	IPC_CC_EVT_FROM_DSP,

	IPC_CC_ACK_FROM_CPU1,
	IPC_CC_ACK_FROM_CPU2,
	IPC_CC_ACK_FROM_DSP
} IPC_CC_EVENT_T;


/************************************************************************************************
 *  CC Command Buffer Definition
 ***********************************************************************************************/
typedef struct ipc_cc_cmd_t
{
	unsigned int opcode; // key
	unsigned int data_addr;
	unsigned int data_size;
} IPC_CC_CMD_T;

/************************************************************************************************
 * Public Function Prototype
 ***********************************************************************************************/

int ipc_cc_register_handler(IPC_CC_CORE_T core, void (*hdl_func)(IPC_CC_EVENT_T));
int ipc_cc_unregister_handler(IPC_CC_CORE_T core);
int ipc_cc_send_cmd(IPC_CC_CORE_T core_sendto, IPC_CC_CMD_T *cmd);
int ipc_cc_get_cmd(IPC_CC_CORE_T core_getfrom, IPC_CC_CMD_T *cmd);
int ipc_cc_send_ack(IPC_CC_CORE_T core_sendto);
void ipc_cc_resume(void);
void ipc_cc_suspend(void);

#endif //_IPC_CC_H_
