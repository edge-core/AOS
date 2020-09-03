/* static char SccsId[] = "+-<>?!NTP_TASK.H   22.1  24/04/02  14:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _NTP_TASK.H
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *		This module handles all events and signal handling functions, one callback
 *		handling packet received, and timeout of protocol.
 *
 *  Notes:
 *		None.
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *   HardSun, 2005 02 17 10:59     Created	  Created
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 2005
 * ------------------------------------------------------------------------
 */


#ifndef		_NTP_TASK_H
#define		_NTP_TASK_H


/* INCLUDE FILE	DECLARATIONS
 */


/* NAME	CONSTANT DECLARATIONS
 */



/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */


/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : SInit system resource required by NTP_TASK; including message queue,
 *			  memory.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void	NTP_TASK_Init (void);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_CreateTask
 *------------------------------------------------------------------------------
 * PURPOSE  : Create all tasks in NTP, client, server, and relay.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. All tasks created, then call MGR, perform proper function.
 *------------------------------------------------------------------------------*/
void	NTP_TASK_CreateTask (void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will tell the SSHD module to start.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : This function is invoked in NTP_INIT_ProvisionComplete().
 *------------------------------------------------------------------------------*/
void NTP_TASK_ProvisionComplete(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NTP_TASK_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the COS subsystem will enter the
 *          Master Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. All the other subsystems must enter the master mode before this function
 *          can be invoked.
 *       2. If "System Boot Configruation File" does not exist, the database and
 *          switch will be initiated to the factory default value.
 *-------------------------------------------------------------------------
 */
void NTP_TASK_EnterMasterMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NTP_TASK_EnterTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. Reallocate all resource for this subsystem
 *       2. Will be called when operation mode change between Master and Slave mode
 *-------------------------------------------------------------------------
 */
void NTP_TASK_EnterTransitionMode(void);
/*-------------------------------------------------------------------------
 * ROUTINE NAME - NTP_TASK_Enter_Slave_Mode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */
void NTP_TASK_EnterSlaveMode(void);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void NTP_TASK_SetTransitionMode(void);

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_Get_SocketId
 * ------------------------------------------------------------------------
 *  FUNCTION : get socket id for dbg use
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
BOOL_T NTP_TASK_Get_SocketId(I32_T *sockid);

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_CreateSocket
 * ------------------------------------------------------------------------
 *  FUNCTION : create ntp socket (create and bind socket)
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : TRUE is successful and FALSE is failure.
 *  NOTE     : None.
 * ------------------------------------------------------------------------
 */
BOOL_T NTP_TASK_CreateSocket(void);

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_CloseSocket
 * ------------------------------------------------------------------------
 *  FUNCTION : close ntp socket
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------
 */
void NTP_TASK_CloseSocket(void);

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_Get_TaskId
 * ------------------------------------------------------------------------
 *  FUNCTION : get task id for dbg use
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
UI32_T NTP_TASK_Get_TaskId(void);
/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_AddPkts_ToRecvbuff
 * ------------------------------------------------------------------------
 *  FUNCTION : add incoming packets to the recvbuff.
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
void NTP_TASK_AddPkts_ToRecvbuff(void);

#endif	 /*	_NTP_TASK_H */
