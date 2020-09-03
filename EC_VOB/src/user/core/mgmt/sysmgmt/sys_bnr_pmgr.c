/*-----------------------------------------------------------------------------
 * Module   : sys_bnr_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access SYSMGMT.
 *-----------------------------------------------------------------------------
 * NOTES    : 
 *    
 *-----------------------------------------------------------------------------
 * HISTORY  : 07/10/2007 - Echo Chen, Create
 *    
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sys_mgr.h"
#include "sys_bnr_mgr.h"
#include "string.h"


/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static void SYS_BNR_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);
/*static void SYS_BNR_PMGR_SendMsgWithoutWaittingResponse(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size);*/

static SYSFUN_MsgQ_T ipcmsgq_handle;


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
BOOL_T SYS_BNR_PMGR_SetSysBnrMsg(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_SysBnrMsg_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_BNR_MGR_IPCMsg_SysBnrMsg_T *data_p;

    data_p = SYS_BNR_MGR_MSG_DATA(msg_p);
    data_p->sys_bnr_type = sys_bnr_type;
    
    strncpy((char *)data_p->msg,	(char *)msg ,  SYS_ADPT_MAX_MOTD_LENGTH  );
    msg[  SYS_ADPT_MAX_MOTD_LENGTH ] = 0 ;     

    SYS_BNR_PMGR_SendMsg(SYS_BNR_MGR_IPC_CMD_SETSYSBNRMSG ,
                         msg_p,
                         SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_SysBnrMsg_T),
                         SYS_BNR_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);     

    return (BOOL_T)SYS_BNR_MGR_MSG_RETVAL(msg_p);
}

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
 BOOL_T SYS_BNR_PMGR_SetSysBnrMsgDelimitingChar(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T delimitingChar)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T *data_p;

    data_p = SYS_BNR_MGR_MSG_DATA(msg_p);
    data_p->sys_bnr_type = sys_bnr_type;
    data_p-> delimitingChar =  delimitingChar;

    SYS_BNR_PMGR_SendMsg(SYS_BNR_MGR_IPC_CMD_SETSYSBNRMSGDELIMITINGCHAR ,
                         msg_p,
                         SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T),
                         SYS_BNR_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);     

    return (BOOL_T)SYS_BNR_MGR_MSG_RETVAL(msg_p);
}

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
BOOL_T SYS_BNR_PMGR_GetSysBnrMsgDelimitingChar(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *delimitingChar)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T *data_p;

    data_p = SYS_BNR_MGR_MSG_DATA(msg_p);
    data_p->sys_bnr_type = sys_bnr_type;
    data_p-> delimitingChar =  *delimitingChar;

    SYS_BNR_PMGR_SendMsg(SYS_BNR_MGR_IPC_CMD_GETSYSBNRMSGDELIMITINGCHAR ,
                         msg_p,
                         SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T),
                         SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T),
                         (UI32_T)FALSE);     
    
    *delimitingChar = data_p-> delimitingChar;
    return (BOOL_T)SYS_BNR_MGR_MSG_RETVAL(msg_p);
}

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
BOOL_T SYS_BNR_PMGR_GetNextSysBnrMsgByID(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg, UI32_T *section_id, UI32_T buffer_size)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_NextSysBnrMsgByID_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_BNR_MGR_IPCMsg_NextSysBnrMsgByID_T *data_p;

    data_p = SYS_BNR_MGR_MSG_DATA(msg_p);
    data_p->sys_bnr_type = sys_bnr_type;

    SYS_BNR_PMGR_SendMsg( SYS_BNR_MGR_IPC_CMD_GETNEXTSYSBNRMSGBYID,
                         msg_p,
                         SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_NextSysBnrMsgByID_T),
                         SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_NextSysBnrMsgByID_T),
                         (UI32_T)FALSE);     
    
    strncpy( (char *)msg ,(char *)data_p->msg,  buffer_size  );
    msg[  buffer_size ] = 0 ;       
   *section_id = data_p->section_id;
   
    return (BOOL_T)SYS_BNR_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES 
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SYS_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the SYS message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of SYS request message.
 *           res_size  - the size of SYS response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : If the message is for a MGR API and has neigher output para. or
 *           return value, use SYS_PMGR_SendMsgWithoutWaittingResponse()
 *           instead.
 *-------------------------------------------------------------------------
 */
static void SYS_BNR_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_SYSMGMT;
    msg_p->msg_size = req_size;

    SYS_BNR_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK ||
        SYS_BNR_MGR_MSG_RETVAL(msg_p) ==  SYS_BNR_MGR_IPC_RESULT_FAIL)
        SYS_BNR_MGR_MSG_RETVAL(msg_p) = ret_val;
}

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SYS_PMGR_SendMsgWithoutWaittingResponse
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message without waitting response.
 * INPUT   : cmd       - the SYS message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of SYSrequest message.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This is only used for the message that is for a MGR API and
 *           has neigher output para. or return value.
 *-------------------------------------------------------------------------
 */
static void SYS_BNR_PMGR_SendMsgWithoutWaittingResponse(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size)
{
    msg_p->cmd = SYS_MODULE_SYSMGMT;
    msg_p->msg_size = req_size;

    SYS_BNR_MGR_MSG_CMD(msg_p) = cmd;

    SYSFUN_SendRequestMsg(ipcmsgq_handle,
                          msg_p,
                          SYSFUN_TIMEOUT_WAIT_FOREVER,
                          SYSFUN_SYSTEM_EVENT_IPCMSG,
                          0,
                          NULL);
}
#endif
/* End of this file */

 


