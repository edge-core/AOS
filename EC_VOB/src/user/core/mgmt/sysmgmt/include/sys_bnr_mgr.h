#ifndef SYS_BNR_MGR_H
#define SYS_BNR_MGR_H


/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_BNR_MGR.H		           						
 * ------------------------------------------------------------------------
 * PURPOSE: This package provides the services to handle the system banner message
 *          functions
 *
 * Notes: This module will support all the system banner message functions includes
 *        1) MOTD (Message of the day)
 *        2) Enable (after enter priviledge) --> not implement
 *        3) Incoming (when telnet connect, show message in console) --> not implement
 *
 *  History
 *
 *   BHU     11/07/2004      new created
 * 
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2004      
 * ------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

/* TYPE DECLARATIONS
 */

/*
 *  1) MOTD (Message of the day)
 *  2) Enable (after enter priviledge) --> not implement
 *  3) Incoming (when telnet connect, show message in console) --> not implement
*/

/*Add it for Build2*/
#define SYS_ADPT_MAX_MOTD_LENGTH                       2047
/*-------------*/

typedef enum 
{
	SYS_BNR_MGR_MOTD_TYPE   = 0,
    /*SYS_BNR_MGR_ENABLE_TYPE,*/
    /*SYS_BNR_MGR_INCOMING_TYPE,*/
    SYS_BNR_MGR_MAX_TYPE    
} SYS_BNR_MGR_TYPE_T;

/* Add funtion typedef macro for pmgr function*/

/* MACRO FUNCTION DECLARATIONS
 */

/*-------------------------------------------------------------------------
 * MACRO NAME - SYS_BNR_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SYS message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of SYS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SYS_BNR_MGR_GET_MSGBUFSIZE(datatype_name) \
        ( sizeof(SYS_BNR_MGR_IPCMsg_Header_T) + sizeof (datatype_name) ) 
        
/*-------------------------------------------------------------------------
 * MACRO NAME - SYS_BNR_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SYS message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of SYS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SYS_BNR_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_EmptyData_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - SYS_BNR_MGR_MSG_CMD
 *              SYS_BNR_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the SYS command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The SYS command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SYS_BNR_MGR_MSG_CMD(msg_p)    (((SYS_BNR_MGR_IPCMsg_T *)msg_p->msg_buf)->header.cmd)
#define SYS_BNR_MGR_MSG_RETVAL(msg_p) (((SYS_BNR_MGR_IPCMsg_T *)msg_p->msg_buf)->header.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - SYS_BNR_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SYS_BNR_MGR_MSG_DATA(msg_p)   ((void *)&((SYS_BNR_MGR_IPCMsg_T *)msg_p->msg_buf)->data)

/* MGR handler will use this when it can't handle the message.
 *                                  (is in transition mode)
 */
#define SYS_BNR_MGR_IPC_RESULT_FAIL  (-1)

/* definitions of command in sys_mgr which will be used in ipc message
 */
enum
{
    SYS_BNR_MGR_IPC_CMD_SETSYSBNRMSG,
    SYS_BNR_MGR_IPC_CMD_SETSYSBNRMSGDELIMITINGCHAR,
    SYS_BNR_MGR_IPC_CMD_GETSYSBNRMSGDELIMITINGCHAR,
    SYS_BNR_MGR_IPC_CMD_GETNEXTSYSBNRMSGBYID,
};
/* DATA TYPE DECLARATIONS
 */
/* structure for the request/response ipc message in sys pmgr and mgr
 */

/* Message declarations for IPC.
 */
typedef struct
{
    SYS_BNR_MGR_TYPE_T sys_bnr_type; 
    UI8_T msg[ SYS_ADPT_MAX_MOTD_LENGTH +1 ];
} SYS_BNR_MGR_IPCMsg_SysBnrMsg_T;

typedef struct
{
    SYS_BNR_MGR_TYPE_T sys_bnr_type;
    UI8_T delimitingChar;
} SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T;

typedef struct
{
    SYS_BNR_MGR_TYPE_T sys_bnr_type;
    UI8_T msg[ SYS_ADPT_MAX_MOTD_LENGTH +1 ]; 
    UI32_T section_id; 
    UI32_T buffer_size;
} SYS_BNR_MGR_IPCMsg_NextSysBnrMsgByID_T;

typedef struct
{
    /* empty struct.
     */
} SYS_BNR_MGR_IPCMsg_EmptyData_T;

 typedef union 
{
        UI32_T cmd;    /* for sending IPC request. SYS_BNR_MGR_IPC_CMD1 ... */
        UI32_T result; /* for response */
}  SYS_BNR_MGR_IPCMsg_Header_T;

typedef union 
{
    SYS_BNR_MGR_IPCMsg_SysBnrMsg_T sysbnrmsg_data;
    SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T sysbnrmsgdelimitingchar_data;
    SYS_BNR_MGR_IPCMsg_NextSysBnrMsgByID_T nextsysbnrmsgbyid_data;
    SYS_BNR_MGR_IPCMsg_EmptyData_T empty_data;
} SYS_BNR_MGR_IPCMsg_Data_T;

typedef struct
{
    SYS_BNR_MGR_IPCMsg_Header_T header;
    SYS_BNR_MGR_IPCMsg_Data_T data;
} SYS_BNR_MGR_IPCMsg_T;

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_Init
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system resource
 * 																		
 * INPUT : None                                     				
 * OUTPUT: None                          					
 * RETURN: TRUE/FALSE                                               		
 * NOTES: 1. This routine will initialize the All Banner Message
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_Init(void);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter master mode																		
 * INPUT:   None                                     				
 * OUTPUT:  None                          					
 * RETURN:  None                                               		
 * NOTES:   None  
 * ---------------------------------------------------------------------*/
void SYS_BNR_MGR_EnterMasterMode(void);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter slave mode 																		
 * INPUT:   None                                     				
 * OUTPUT:  None                          					
 * RETURN:  None                                               		
 * NOTES:   None  
 * ---------------------------------------------------------------------
 */
void SYS_BNR_MGR_EnterSlaveMode(void);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_BNR_MGR_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the SYS_BNR into the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SYS_BNR_MGR_SetTransitionMode();


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter transition mode																	
 * INPUT:   None                                     				
 * OUTPUT:  None                          					
 * RETURN:  None                                               		
 * NOTES:   None  
 * ---------------------------------------------------------------------
 */
void SYS_BNR_MGR_EnterTransitionMode(void);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetSysBnrMsg
 * ---------------------------------------------------------------------
 * PURPOSE: get system banner message for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
 * OUTPUT	: *msg  - the banner message of specified type
 * RETURN	: TRUE
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_GetSysBnrMsg(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_SetSysBnrMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Set system banner message for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *msg  - the banner message of specified type
 * OUTPUT	: None
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_SetSysBnrMsg(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetRunningSysBnrMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Get running system banner message for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *msg  - the banner message of specified type
 * OUTPUT	: *msg  - the banner message of specified type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
UI32_T SYS_BNR_MGR_GetRunningSysBnrMsg(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg);



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_SetSysBnrMsgDelimitingChar
 * ---------------------------------------------------------------------
 * PURPOSE: Set system banner message delimiting character for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              delimitingChar  - the delimiting character of specified type
 * OUTPUT	: None
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
 BOOL_T SYS_BNR_MGR_SetSysBnrMsgDelimitingChar(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T delimitingChar);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetSysBnrMsgDelimitingChar
 * ---------------------------------------------------------------------
 * PURPOSE: Set system banner message delimiting character for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *delimitingChar  - the delimiting character of specified type
 * OUTPUT	: *delimitingChar  - the delimiting character of specified type
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_GetSysBnrMsgDelimitingChar(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *delimitingChar);



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetNextSysBnrMsgByID
 * ---------------------------------------------------------------------
 * PURPOSE: get system banner message for specified type, section ID, and buffer 
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *section_id   - key to specifiy getting which section 
              buffer_size  - key to specifiy size to split data 
 * OUTPUT	: *msg  - the banner message of specified type
 * OUTPUT	: *section_id  -  current section_id
 * RETURN	: TRUE
 * NOTES	: If section ID is zero, it means to get first section data. 
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_GetNextSysBnrMsgByID(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg, UI32_T *section_id, UI32_T buffer_size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SYS_BNR_MGR_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for sys_bnr mgr.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);


#endif /* End of SYS_BNR_MGR_H */
