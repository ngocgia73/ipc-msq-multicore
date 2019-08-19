#ifndef __IPCLIB_H__
#define __IPCLIB_H__
#include "ipc-common.h"
typedef enum ipc_err
{
	IPC_ERR_OK = 0, 		///< return ok
	IPC_ERR_MSGQUE_ID = -1, 	///< there is error on message queue ID
	IPC_ERR_MSGQUE_FULL = -2, 	///< the message queue is full
	IPC_ERR_SND_MSG = -3, 		///< has err when send msg
	IPC_ERR_RCV_MSG = -4, 		///< has err when receive msg
	IPC_ERR_INPUT_PARAM = -5, 	///< has some error of input parameter
	IPC_ERR_NO_MORE_QUEUE = -6, 	///< no more message queue to get
	IPC_ERR_KEY_DUPLICATE = -7, 	///< the ipc_key is duplicate  with other msg queue
	IPC_ERR_RCVSIZE_OVER_LIMIT = -8,///< the rcv msg size over limit
	IPC_ERR_SNDSIZE_OVER_LIMIT = -9,///< the snd msg size over limit
	IPC_ERR_MSGQUE_RELEASED = -10, 	///< the msg queue is released
	IPC_ERR_NOT_OPENED = -11, 	///< the ipc is not open
	ENUM_DUMMY4WORD(IPC_ERR)
} IPC_ERR;


/*********************************************************************************************************
 * define prototype function 
 ********************************************************************************************************/
/*
 * @des : open ipc device file 
 * @param[in]: 	none
 * @param[out]: none
 * @return : 	follow return from IPC_ERR 
 */
int ipc_open(void);

/*
 * @des : 	close device file
 * @param[in]: 	none
 * @param[out]: none
 * @return: 	follow return value from IPC_ERR.
 */
IPC_ERR ipc_close(void);

/*
 * @des: 	ipc_ftok function implement
 * @param[in]: 	path
 * @return: 	key if success otherwise return error
 */
INT16 	ipc_ftok(const char *path);

/*
 * @des: 	ipc_msg_get function implement
 * @param[in]: 	key
 * @param[out]: none
 * @return: 	msqid if success otherwise return err
 */
INT32 	ipc_msg_get(INT16 key);

/*
 * @des: 	ipc_msg_rel function implement
 * @param[in]: 	msqid
 * @param[out]: none
 * @return : 	follow return value from IPC_ERR 
 */
IPC_ERR ipc_msg_rel(UINT32 msqid);

/*
 * @des: 	ipc_msg_snd function implement
 * @param[in]: 	msqid, sendTo, pmsg_data, msg_size 
 * @param[out]: none
 * @return: 	follow return value from IPC_ERR
 */
INT32 ipc_msg_snd(UINT32 msqid, IPC_SENDTO sendTo, void *pmsg_data, UINT32 msg_size);

/*
 * @des: 	ipc_msg_rcv function implement
 * @param[in]: 	msqid,msg_size
 * @param[out]: pmsg_data
 * @return: 	follow return value from IPC_ERR
 */
INT32 ipc_msg_rcv(UINT32 msqid, void *pmsg_data, UINT32 msg_size);

#endif // __IPCLIB_H__
