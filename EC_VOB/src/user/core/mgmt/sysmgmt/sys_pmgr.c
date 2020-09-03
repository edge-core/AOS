/*-----------------------------------------------------------------------------
 * Module   : sysmgmt_pmgr.c
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
#include "sys_bld.h"
#include "string.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SYS_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);
#if 0
static void SYS_PMGR_SendMsgWithoutWaittingResponse(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size);
#endif
/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;




/*-------------------------------------------------------------------------
 * FUNCTION NAME - SYS_PMGR_InitiateProcessResource
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME	 - SYS_MGR_GetConsoleCfg
 * ---------------------------------------------------------------------
 * PURPOSE:	This function will get whole record	data of	console	setting
 * INPUT	: None
 * OUTPUT	: console setting(by record)
 * RETURN	: TRUE/FALSE
 * NOTES	: None
 * ---------------------------------------------------------------------
 */

BOOL_T SYS_PMGR_GetConsoleCfg(SYS_MGR_Console_T *consolecfg)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_ConsoleCfg_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPCMsg_ConsoleCfg_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETCONSOLECFG,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_ConsoleCfg_T),
                         (UI32_T)FALSE);

    *consolecfg = data_p->consolecfg;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);



}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default password threshold is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: passwordThreshold
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningPasswordThreshold(UI32_T *passwordThreshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPCMsg_PasswordThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGPASSWORDTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T),
                         (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

   *passwordThreshold = data_p->passwordthreshold ;

    return SYS_MGR_MSG_RETVAL(msg_p);

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : UI32_T    password threshold
 * OUTPUT   : None
 * NOTES    : None
 * RETURN   : TRUE/FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetPasswordThreshold(UI32_T passwordThreshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPCMsg_PasswordThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->passwordthreshold  = passwordThreshold;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETPASSWORDTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console exec time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value   - nactive time out
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningConsoleExecTimeOut(UI32_T *time_out_value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ExecTimeOut_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGCONSOLEEXECTIMEOUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T),
                         (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

   *time_out_value = data_p->time_out_value ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : time_out_value
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetConsoleExecTimeOut(UI32_T time_out_value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ExecTimeOut_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->time_out_value  = time_out_value;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETCONSOLEEXECTIMEOUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console silent time is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: silent_time
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningConsoleSilentTime(UI32_T *silent_time)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ConsoleSilentTime_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGCONSOLESILENTTIME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T),
                         (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

   *silent_time = data_p->silent_time ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : silent time value
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetConsoleSilentTime(UI32_T silent_time)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ConsoleSilentTime_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->silent_time  = silent_time;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETCONSOLESILENTTIME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetSilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console silent time is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: silent_time
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningTelnetSilentTime(UI32_T *silent_time)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ConsoleSilentTime_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GET_RUNNING_TELNET_SILENT_TIME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T),
                         (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

   *silent_time = data_p->silent_time ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetVtySilentTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : silent time value
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetTelnetSilentTime(UI32_T silent_time)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ConsoleSilentTime_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->silent_time  = silent_time;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SET_TELNET_SILENT_TIME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ConsoleSilentTime_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetTelnetCfg
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get whole record data of Telnet setting
 * INPUT    : None
 * OUTPUT   : console setting(by record)
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetTelnetCfg(SYS_MGR_Telnet_T *telnetCfg)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_TelnetCfg_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_TelnetCfg_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETTELNETCFG,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_TelnetCfg_T),
                         (UI32_T)FALSE);
    *telnetCfg = data_p->telnetcfg ;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default password threshold is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: passwordThreshold
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningTelnetPasswordThreshold(UI32_T *passwordThreshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPCMsg_PasswordThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGTELNETPASSWORDTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T),
                         (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

   *passwordThreshold = data_p->passwordthreshold ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetPasswordThreshold
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set Telnet password retry count
 * INPUT    : UI32_T    password threshold
 * OUTPUT   : None
 * NOTES    : None
 * RETURN   : TRUE/FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetTelnetPasswordThreshold(UI32_T passwordThreshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPCMsg_PasswordThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->passwordthreshold  = passwordThreshold;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETTELNETPASSWORDTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_PasswordThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default telnet console exec time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningTelnetExecTimeOut(UI32_T *time_out_value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ExecTimeOut_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGTELNETEXECTIMEOUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T),
                         (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

   *time_out_value = data_p->time_out_value ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetExecTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set password retry count
 * INPUT    : time_out_value
 * OUTPUT  : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetTelnetExecTimeOut(UI32_T time_out_value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ExecTimeOut_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->time_out_value  = time_out_value;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETTELNETEXECTIMEOUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ExecTimeOut_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetUartParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns the parameters of local console UART.
 * INPUT: None
 * OUTPUT: uart_cfg including baudrate, parity, data_length and stop bits
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetUartParameters(SYS_MGR_Uart_Cfg_T *uartCfg)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartCfg_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_UartCfg_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETUARTPARAMETERS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartCfg_T),
                         (UI32_T)FALSE);
    *uartCfg = data_p->uartcfg ;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}
#if (SYS_CPNT_AUTOBAUDRATE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetUartOperBaudrate
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns the operating baudrate of local console UART .
 * INPUT: None
 * OUTPUT: uart_cfg including baudrate, parity, data_length and stop bits
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetUartOperBaudrate(SYS_MGR_Uart_BaudRate_T *uart_operbaudrate)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartBaudRate_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_UartBaudRate_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETUARTOPERBAUDRATE,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartBaudRate_T),
                         (UI32_T)FALSE);
    *uart_operbaudrate = data_p->uart_operbaudrate ;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_Get_Autobaudrate_Switch
 * ---------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   : [none]
 * RETURN   :
 *
 *
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_Get_Autobaudrate_Switch()
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_AutoBaudRate_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_AutoBaudRate_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETAUTOBAUDRATESWITCH,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningUartParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default uart setting is successfully retrieved.
 *          For UART parameters, any one is changed will be thought
 *          that the setting is different.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: uart_cfg including baudrate, parity, data_length and stop bits
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *        4. We have a rule that getrunning should be getrunning by field,
 *           but for CLI there is a command "terminal" which can set
 *           all the uart config by one command, so we can getrunning
 *           by record for this uart parameter.
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningUartParameters(SYS_MGR_Uart_RunningCfg_T *uart_cfg)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartRunningCfg_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_UartRunningCfg_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGUARTPARAMETERS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartRunningCfg_T),
                         (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *uart_cfg = data_p->uart_runningcfg;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartBaudrate
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set baudrate of serial port
 * INPUT    : SYS_MGR_Uart_BaudRate_T Baudrate
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetUartBaudrate(SYS_MGR_Uart_BaudRate_T baudrate)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartBaudRate_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_UartBaudRate_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->uart_operbaudrate  = baudrate;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETUARTBAUDRATE,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartBaudRate_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartParity
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set Parity of serial port
 * INPUT    : SYS_MGR_Console_Parity_T Parity
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetUartParity(SYS_MGR_Uart_Parity_T parity)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartParity_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_UartParity_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->parity  = parity;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETUARTPARITY,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartParity_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartDataBits
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set DataLength of serial port
 * INPUT    : SYS_MGR_Uart_Parity_T DataLength
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetUartDataBits(SYS_MGR_Uart_Data_Length_T data_length)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartDataBits_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_UartDataBits_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->data_length =  data_length;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETUARTDATABITS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartDataBits_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetUartStopBits
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set StopBits of serial port
 * INPUT    : SYS_MGR_Uart_Stop_Bits_T stop_bits
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetUartStopBits(SYS_MGR_Uart_Stop_Bits_T stop_bits)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartStopBits_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_UartStopBits_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->stop_bits=  stop_bits;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETUARTSTOPBITS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_UartStopBits_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetPromptString
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system prompt string
 * INPUT:   : *prompt_string -- prompt string address
 * OUTPUT:  : *prompt_string -- prompt string with max length 32
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES:   :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetPromptString(UI8_T *prompt_string)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PromptString_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_PromptString_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETPROMPTSTRING,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PromptString_T),
                         (UI32_T)FALSE);

    memcpy(prompt_string, data_p->prompt_string, SYS_ADPT_MAX_PROMPT_STRING_LEN);
    prompt_string[SYS_ADPT_MAX_PROMPT_STRING_LEN] =	0;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetPromptString
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system prompt string
 * INPUT:   : *prompt_string -- prompt string address
 * OUTPUT:  : None
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES:   :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetPromptString(UI8_T *prompt_string)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PromptString_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_PromptString_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    memcpy(data_p->prompt_string, prompt_string,  SYS_ADPT_MAX_PROMPT_STRING_LEN);
    data_p->prompt_string[SYS_ADPT_MAX_PROMPT_STRING_LEN] =	0;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETPROMPTSTRING,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PromptString_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningPromptString
 * ---------------------------------------------------------------------
 * PURPOSE  : Get running system prompt string
 * INPUT    : *prompt_string -- prompt string address
 * OUTPUT   : *prompt_string -- prompt string with max length 32
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningPromptString(UI8_T *prompt_string)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PromptString_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_PromptString_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGPROMPTSTRING,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PromptString_T),
                         (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

    memcpy(prompt_string, data_p->prompt_string, SYS_ADPT_MAX_PROMPT_STRING_LEN);
    prompt_string[SYS_ADPT_MAX_PROMPT_STRING_LEN] =	0;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME	 - SYS_PMGR_LogWatchDogExceptionInfo
 * ---------------------------------------------------------------------
 * PURPOSE:	This function will set watch dog timer exception information
 *			to SYS_MGR_WATCHDOG_TIMER_FILE
 * INPUT	: SYS_MGR_WatchDogExceptionInfo_T *wd_own
 * OUTPUT	: None
 * RETURN	: TRUE/FALSE
 * NOTES	: Call by NMI exception	0x500 handler rtcInt() in usrconfig.c
 *			  and backdoor only!!
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_LogWatchDogExceptionInfo(SYS_MGR_WatchDogExceptionInfo_T	*wd_own)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LogWatchDogExceptionInfo_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_LogWatchDogExceptionInfo_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->wd_own = *wd_own;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_LOGWATCHDOGEXCEPTIONINFO,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LogWatchDogExceptionInfo_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetPowerStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status of specific unit.
 * INPUT  : power_status->sw_unit_index   --- Which unit.
 * OUTPUT : power_status->sw_power_status --- VAL_swPowerStatus_internalPower
 *                                            VAL_swPowerStatus_redundantPower
 *                                            VAL_swPowerStatus_internalAndRedundantPower
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetPowerStatus(SYS_MGR_PowerStatus_T *power_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PowerStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_PowerStatus_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->power_status = *power_status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETPOWERSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PowerStatus_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PowerStatus_T),
                         (UI32_T)FALSE);

    *power_status =data_p->power_status ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetNextPowerStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status next to some unit.
 * INPUT  : power_status->sw_unit_index   --- Next to which unit.
 * OUTPUT : power_status->sw_power_status --- VAL_swPowerStatus_internalPower
 *                                            VAL_swPowerStatus_redundantPower
 *                                            VAL_swPowerStatus_internalAndRedundantPower
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextPowerStatus(SYS_MGR_PowerStatus_T *power_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PowerStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_PowerStatus_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->power_status= *power_status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETNEXTPOWERSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PowerStatus_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_PowerStatus_T),
                         (UI32_T)FALSE);

    *power_status =data_p->power_status ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetSwitchIndivPower
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status of specific power.
 * INPUT  : indiv_power->sw_indiv_power_unit_index --- Which unit.
 *          indiv_power->sw_indiv_power_index      --- Which power.
 * OUTPUT : indiv_power->sw_indiv_power_status     --- VAL_swIndivPowerStatus_notPresent
 *                                                     VAL_swIndivPowerStatus_green
 *                                                     VAL_swIndivPowerStatus_red
 *          indiv_power->sw_indiv_power_type       --- VAL_swIndivPowerType_DC_N48
 *                                                     VAL_swIndivPowerType_DC_P24
 *                                                     VAL_swIndivPowerType_AC
 *                                                     VAL_swIndivPowerType_DC_N48_Wrong
 *                                                     VAL_swIndivPowerType_DC_P24_Wrong
 *                                                     VAL_swIndivPowerType_none
 *                                                     VAL_swIndivPowerType_AC_Wrong
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwitchIndivPower(SYS_MGR_IndivPower_T *indiv_power)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchIndivPower_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchIndivPower_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->indiv_power = *indiv_power;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETSWITCHINDIVPOWER,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchIndivPower_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchIndivPower_T),
                         (UI32_T)FALSE);

    *indiv_power =data_p->indiv_power ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_MGR_GetNextSwitchIndivPower
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get power status next to some power.
 * INPUT  : indiv_power->sw_indiv_power_unit_index --- Next to which unit.
 *          indiv_power->sw_indiv_power_index      --- Next to which power.
 * OUTPUT : indiv_power->sw_indiv_power_status     --- VAL_swIndivPowerStatus_notPresent
 *                                                     VAL_swIndivPowerStatus_green
 *                                                     VAL_swIndivPowerStatus_red
 *          indiv_power->sw_indiv_power_type       --- VAL_swIndivPowerType_DC_N48
 *                                                     VAL_swIndivPowerType_DC_P24
 *                                                     VAL_swIndivPowerType_AC
 *                                                     VAL_swIndivPowerType_DC_N48_Wrong
 *                                                     VAL_swIndivPowerType_DC_P24_Wrong
 *                                                     VAL_swIndivPowerType_none
 *                                                     VAL_swIndivPowerType_AC_Wrong
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextSwitchIndivPower(SYS_MGR_IndivPower_T *indiv_power)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchIndivPower_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchIndivPower_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->indiv_power = *indiv_power;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETNEXTSWITCHINDIVPOWER,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchIndivPower_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchIndivPower_T),
                         (UI32_T)FALSE);

    *indiv_power =data_p->indiv_power ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetSwAlarmInput
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get name and status of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index        --- Which unit.
 *          sw_alarm->sw_alarm_unit_input_index  --- Which index.
 * OUTPUT : sw_alarm->sw_alarm_input_name        --- description of alarm input
 *          sw_alarm->sw_alarm_status            --- status of four alarm input(bit mapped)
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwAlarmInput(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETSWALARMINPUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         (UI32_T)FALSE);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetNextSwAlarmInput
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get name and status of next alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 * OUTPUT : sw_alarm->sw_alarm_input_name  --- description of alarm input
 *          sw_alarm->sw_alarm_status      --- status of four alarm input(bit mapped)
 * RETURN : TRUE/FALSE
 * NOTES  : Used for SNMP.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextSwAlarmInput(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETNEXTSWALARMINPUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         (UI32_T)FALSE);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetSwAlarmInputStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get status of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index        --- Which unit.
 * OUTPUT : sw_alarm->sw_alarm_status            --- status of four alarm input(bit mapped)
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwAlarmInputStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETSWALARMINPUTSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         (UI32_T)FALSE);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetSwAlarmInputName
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to get name of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 * OUTPUT : sw_alarm->sw_alarm_input_name  --- description of alarm input
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETSWALARMINPUTNAME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         (UI32_T)FALSE);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_SetSwAlarmInputName
 * ---------------------------------------------------------------------
 * PURPOSE: Use this routine to set name of alarm input.
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 *          sw_alarm->sw_alarm_input_name  --- description of alarm input
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETSWALARMINPUTNAME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         (UI32_T)FALSE);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}
/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_PMGR_GetRunningAlarmInputName
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to get name of alarm input if it's different
 *          from default value
 * INPUT  : sw_alarm->sw_alarm_unit_index  --- Which unit.
 *          sw_alarm->sw_alarm_input_index --- Which index.
 * OUTPUT:  sw_alarm->sw_alarm_input_name  --- description of alarm input
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_FAIL      -- error (system is not in MASTER mode)
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- different from default value
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningAlarmInputName(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGALARMINPUTNAME,
			 msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
			 SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME	- SYS_PMGR_GetMajorAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE:	Use this routine to get major alarm output status
 * INPUT  :	sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT :	sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * RETURN :	TRUE if get successfully
 * NOTES  :	None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMajorAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETMAJORALARMOUTPUTSTATUS,
                     msg_p,
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                     (UI32_T)FALSE);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME	- SYS_PMGR_GetMinorAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE:	Use this routine to get minor alarm output status
 * INPUT  :	sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT :	sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN :	TRUE if get successfully
 * NOTES  :	None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMinorAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETMINORALARMOUTPUTSTATUS,
                     msg_p,
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                     (UI32_T)FALSE);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME	- SYS_PMGR_GetAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE:	Use this routine to get major and minor alarm output status
 * INPUT  :	sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT :	sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 *         	sw_alarm->sw_indiv_alarm_status_2     --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN :	TRUE if get successfully
 * NOTES  :	Used for SNMP
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETALARMOUTPUTSTATUS,
                     msg_p,
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                     (UI32_T)FALSE);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME	- SYS_PMGR_GetNextAlarmOutputCurrentStatus
 * ---------------------------------------------------------------------
 * PURPOSE:	Use this routine to get next major and minor alarm output status
 * INPUT  :	sw_alarm->sw_indiv_alarm_unit_index   --- Which unit.
 * OUTPUT :	sw_alarm->sw_indiv_alarm_status       --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 *         	sw_alarm->sw_indiv_alarm_status_2     --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN :	TRUE if get successfully
 * NOTES  :	Used for SNMP
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextAlarmOutputCurrentStatus(SYS_MGR_SwAlarmEntry_T *sw_alarm)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwAlarm_T *data_p;

    data_p = (SYS_MGR_IPC_SwAlarm_T*)SYS_MGR_MSG_DATA(msg_p);
    data_p->sw_alarm = *sw_alarm;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETNEXTALARMOUTPUTSTATUS,
                     msg_p,
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwAlarm_T),
                     (UI32_T)FALSE);

    *sw_alarm = data_p->sw_alarm;

    return SYS_MGR_MSG_RETVAL(msg_p);
}
#endif /* #if (SYS_CPNT_ALARM_DETECT == TRUE) */

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetFanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get exact fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Which fan
 *            switch_fan_status->switch_fan_index  --- Which fan
 * OUTPUT   : switch_fan_status->switch_fan_status --- The status of the fan
 *                               VAL_switchFanStatus_ok/VAL_switchFanStatus_failure
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetFanStatus(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_FanStatus_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->switch_fan_status = *switch_fan_status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETFANSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         (UI32_T)FALSE);

    *switch_fan_status =data_p->switch_fan_status ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextFanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the next fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Next to which fan
 *            switch_fan_status->switch_fan_index  --- Next to which fan
 * OUTPUT   : switch_fan_status->switch_fan_status --- The status of the fan
 *                               VAL_switchFanStatus_ok/VAL_switchFanStatus_failure

 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextFanStatus(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_FanStatus_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->switch_fan_status = *switch_fan_status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETNEXTFANSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         (UI32_T)FALSE);

    *switch_fan_status =data_p->switch_fan_status ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To get exact fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Which fan
 *            switch_fan_status->switch_fan_index  --- Which fan
 * OUTPUT   : switch_fan_status->switch_fan_speed --- The speed of the
 *            given fan
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_FanStatus_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->switch_fan_status = *switch_fan_status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETFANSPEED,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         (UI32_T)FALSE);

    *switch_fan_status =data_p->switch_fan_status ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the next fan status.
 * INPUT    : switch_fan_status->switch_unit_index --- Next to which fan
 *            switch_fan_status->switch_fan_index  --- Next to which fan
 * OUTPUT   : switch_fan_status->switch_fan_speed --- The speed of the
 *            given fan
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_FanStatus_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->switch_fan_status = *switch_fan_status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETNEXTFANSPEED,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         (UI32_T)FALSE);

    *switch_fan_status =data_p->switch_fan_status ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetFanSpeed
 * ---------------------------------------------------------------------
 * PURPOSE  : To set the next fan speed.
 * INPUT    : switch_fan_status->switch_unit_index --- Next to which fan
 *            switch_fan_status->switch_fan_index  --- Next to which fan
 * OUTPUT   : switch_fan_status->switch_fan_speed --- The speed of the
 *            given fan
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetFanSpeed(SYS_MGR_SwitchFanEntry_T *switch_fan_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_FanStatus_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->switch_fan_status  =  *switch_fan_status ;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETFANSPEED,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return SYS_MGR_MSG_RETVAL(msg_p);
}

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_PMGR_SetFanSpeedForceFull
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to set force fan speed full
 * INPUT:   mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * OUTPUT:  None.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetFanSpeedForceFull(BOOL_T mode)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanSpeedForceFull_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_FanSpeedForceFull_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->mode  =  mode;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETFANSPEEDFORCEFULL,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanSpeedForceFull_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_PMGR_GetFanSpeedForceFull
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to get force fan speed full
 * INPUT:   None.
 * OUTPUT:  mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetFanSpeedForceFull(BOOL_T *mode)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanSpeedForceFull_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_FanSpeedForceFull_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETFANSPEEDFORCEFULL,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanSpeedForceFull_T),
                         (UI32_T)FALSE);

    *mode = data_p->mode ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: SYS_PMGR_GetRunningFanSpeedForceFull
 *------------------------------------------------------------------------
 * PURPOSE: This routine is used to get running force fan speed full
 * INPUT:   None.
 * OUTPUT:  mode  --- TRUE/FALSE.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_FAIL      -- error (system is not in MASTER mode)
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- different from default value
 * NOTES:   None.
 *------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningFanSpeedForceFull(BOOL_T *mode)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanSpeedForceFull_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_FanSpeedForceFull_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGFANSPEEDFORCEFULL,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanSpeedForceFull_T),
                         SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *mode = data_p->mode ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}
#endif

void	SYS_PMGR_FanStatusChanged_CallBack(UI32_T unit, UI32_T fan, UI32_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_FanStatus_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    data_p->switch_fan_status.switch_unit_index = unit;
    data_p->switch_fan_status.switch_fan_index = fan;
    data_p->switch_fan_status.switch_fan_status = status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_FANSTATUSCHANGED,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_FanStatus_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

   /* return SYS_MGR_MSG_RETVAL(msg_p);  */
}
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetThermalStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get exact fan status.
 * INPUT    : switch_thermal_status->switch_unit_index     --- Which unit
 *            switch_thermal_status->switch_thermal_index  --- Which thermal
 * OUTPUT   : switch_thermal_status->switch_thermal_status --- The temperature of
 *            the given thermal
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetThermalStatus(SYS_MGR_SwitchThermalEntry_T *switch_thermal_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalEntry_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->switch_thermal_status = *switch_thermal_status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETTHERMALSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T),
                         (UI32_T)FALSE);

    *switch_thermal_status =data_p->switch_thermal_status ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextThermalStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the next fan status.
 * INPUT    : switch_thermal_status->switch_unit_index --- Next to which unit
 *            switch_thermal_status->switch_thermal_index  --- Next to which thermal
 * OUTPUT   : switch_thermal_status->switch_thermal_status --- The temperature of
 *            the given thermal
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextThermalStatus(SYS_MGR_SwitchThermalEntry_T *switch_thermal_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalEntry_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->switch_thermal_status = *switch_thermal_status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETNEXTTHERMALSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T),
                         (UI32_T)FALSE);

    *switch_thermal_status =data_p->switch_thermal_status ;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetDefaultSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the default switch thermal action entry.
 * INPUT    : *entry --- default entry pointer
 * OUTPUT   : *entry --- default entry pointer
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetDefaultSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalActionEntry_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->action_entry = *entry;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETDEFAULTSWITCHTHERMALACTIONENTRY,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T),
                         (UI32_T)FALSE);

    *entry =data_p->action_entry;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get the switch thermal action entry.
 * INPUT    : *entry --- entry pointer
 * OUTPUT   : *entry --- entry pointer
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalActionEntry_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->action_entry = *entry;
    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETSWITCHTHERMALACTIONENTRY,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T),
                         (UI32_T)FALSE);

    *entry =data_p->action_entry;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetNextSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To get next switch thermal action entry.
 * INPUT    : *entry --- entry pointer
 * OUTPUT   : *entry --- entry pointer
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetNextSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalActionEntry_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->action_entry = *entry;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETNEXTSWITCHTHERMALACTIONENTRY,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T),
                         (UI32_T)FALSE);

    *entry =data_p->action_entry;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action entry.
 * INPUT    : *entry --- entry pointer
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionEntry(SYS_MGR_SwitchThermalActionEntry_T *entry)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalActionEntry_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->action_entry = *entry;
    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONENTRY,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionEntry_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action rising threshold.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            rising_threshold -- rising threshold
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionRisingThreshold(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T rising_threshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalActionThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_index = unit_index;
    data_p->thermal_index =  thermal_index;
    data_p->index = index;
    data_p->threshold_action = rising_threshold;
    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONRISINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action falling threshold.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            falling_threshold -- falling threshold
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionFallingThreshold(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T falling_threshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalActionThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_index = unit_index;
    data_p->thermal_index =  thermal_index;
    data_p->index = index;
    data_p->threshold_action = falling_threshold;
    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONFALLINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionAction
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action entry action.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            action -- entry action
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionAction(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T action)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalActionThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_index = unit_index;
    data_p->thermal_index =  thermal_index;
    data_p->index = index;
    data_p->threshold_action = action;
    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONACTION,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetSwitchThermalActionStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : To set switch thermal action entry status.
 * INPUT    : unit_index --- unit index
 *            thermal_index -- thermal index
 *            index -- index
 *            status -- entry status
 * OUTPUT   : None
 *
 * RETURN   : TRUE/FALSE
 * NOTES    : from proprietary MIB
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetSwitchThermalActionStatus(UI32_T unit_index, UI32_T thermal_index, UI32_T index, UI32_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalActionThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_index = unit_index;
    data_p->thermal_index =  thermal_index;
    data_p->index = index;
    data_p->threshold_action = status;
    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETSWITCHTHERMALACTIONSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalActionThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return SYS_MGR_MSG_RETVAL(msg_p);
}

void	SYS_PMGR_ThermalStatusChanged_CallBack(UI32_T unit, UI32_T thermal, UI32_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SwitchThermalEntry_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    data_p->switch_thermal_status.switch_unit_index = unit;
    data_p->switch_thermal_status.switch_thermal_index = thermal;
    data_p->switch_thermal_status.switch_thermal_temp_value = status;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_THERMALSTATUSCHANGED,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SwitchThermalEntry_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

/*    return SYS_MGR_MSG_RETVAL(msg_p);  */
}

#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetConsoleLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set console login timeout seconds
 * INPUT    : time_out_value
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetConsoleLoginTimeOut(UI32_T time_out_value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_LoginTimeOut_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->time_out_value  = time_out_value;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETCONSOLELOGINTIMEOUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_SetTelnetLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set telnet login timeout seconds
 * INPUT    : time_out_value
 * OUTPUT  : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetTelnetLoginTimeOut(UI32_T time_out_value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_LoginTimeOut_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->time_out_value  = time_out_value;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETTELNETLOGINTIMEOUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningConsoleLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default console login time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value   - nactive time out
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningConsoleLoginTimeOut(UI32_T *time_out_value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_LoginTimeOut_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGCONSOLELOGINTIMEOUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T),
                         (UI32_T)FALSE);
    *time_out_value = data_p->time_out_value ;
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetRunningTelnetLoginTimeOut
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default telnet console login time out  is successfully
 *          retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: time_out_value
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetRunningTelnetLoginTimeOut(UI32_T *time_out_value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_LoginTimeOut_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGTELNETLOGINTIMEOUT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_LoginTimeOut_T),
                         (UI32_T)FALSE);
    *time_out_value = data_p->time_out_value ;
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSysDescr
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system description.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : sys_descrption -- system description.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_sysDescr+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSysDescr(UI32_T unit_id, UI8_T *sys_descrption)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SysDescr_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SysDescr_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_id = unit_id;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETSYSDESCR,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SysDescr_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SysDescr_T),
                         (UI32_T)FALSE);
    strncpy((char *)sys_descrption,	(char *)data_p->sys_descrption ,MAXSIZE_sysDescr);
    sys_descrption[ MAXSIZE_sysDescr ] = 0 ;
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetSysObjectID
 * ---------------------------------------------------------------------
 * PURPOSE  : Get system object ID string.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : sys_oid -- system OID string.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_OID_STRING_LEN+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetSysObjectID(UI32_T unit_id, UI8_T *sys_oid)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SysObjectID_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_SysObjectID_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_id = unit_id;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETSYSOBJECTID,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SysObjectID_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_SysObjectID_T),
                         (UI32_T)FALSE);
    strncpy((char *)sys_oid , (char *)data_p->sys_oid ,SYS_ADPT_MAX_OID_STRING_LEN);
    sys_oid[ SYS_ADPT_MAX_OID_STRING_LEN] = 0 ;
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetProductName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get product name.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : sys_descrption -- product name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_swProdName+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetProductName(UI32_T unit_id, UI8_T *product_name)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductName_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ProductName_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_id = unit_id;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETPRODUCTNAME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductName_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductName_T),
                         (UI32_T)FALSE);
    strncpy((char *)product_name,	(char *)data_p->product_name ,MAXSIZE_swProdName);
    product_name[MAXSIZE_swProdName ] = 0 ;
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetDeviceName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get device name.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : device_name    -- device name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_DEVICE_NAME_STRING_LEN+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetDeviceName(UI32_T unit_id, UI8_T *device_name)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_DeviceName_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_DeviceName_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_id = unit_id;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETDEVICENAME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_DeviceName_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_DeviceName_T),
                         (UI32_T)FALSE);
    strncpy((char *)device_name , (char *)data_p->device_name ,SYS_ADPT_DEVICE_NAME_STRING_LEN);
    device_name[ SYS_ADPT_DEVICE_NAME_STRING_LEN ] = 0 ;
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetPrivateMibRoot
 * ---------------------------------------------------------------------
 * PURPOSE  : Get MIB root OID string.
 * INPUT    : unit_id        -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                              means local unit.
 * OUTPUT   : mib_root -- MIB root OID string.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_OID_STRING_LEN+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetPrivateMibRoot(UI32_T unit_id, UI8_T *mib_root)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MibRoot_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MibRoot_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_id = unit_id;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETPRIVATEMIBROOT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MibRoot_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MibRoot_T),
                         (UI32_T)FALSE);
    strncpy((char *)mib_root , (char *)data_p->mib_root ,SYS_ADPT_MAX_OID_STRING_LEN);
    mib_root[ SYS_ADPT_MAX_OID_STRING_LEN ] = 0 ;
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_MGR_GetModelName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get model name by board ID.
 * INPUT    : unit_id     -- unit ID; using SYS_VAL_LOCAL_UNIT_ID
 *                           means local unit.
 * OUTPUT   : model_name  -- model name.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (SYS_ADPT_MAX_MODEL_NAME_SIZE + 1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetModelName(UI32_T unit_id, UI8_T *model_name)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ModelName_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ModelName_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->unit_id = unit_id;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETMODELNAME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ModelName_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ModelName_T),
                         (UI32_T)FALSE);
    strncpy((char *)model_name , (char *)data_p->model_name ,SYS_ADPT_MAX_MODEL_NAME_SIZE);
    model_name[ SYS_ADPT_MAX_MODEL_NAME_SIZE ] = 0 ;
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetProductManufacturer
 * ---------------------------------------------------------------------
 * PURPOSE  : Get product manufacturer.
 * INPUT    : none.
 * OUTPUT   : prod_manufacturer -- product manufacturer.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_swProdManufacturer+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetProductManufacturer(char *prod_manufacturer)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductManufacturer_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ProductManufacturer_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GET_PRODUCT_MANUFACTURER,
                     msg_p,
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductManufacturer_T),
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductManufacturer_T),
                     (UI32_T)FALSE);
    strncpy(prod_manufacturer, data_p->product_manufacturer, MAXSIZE_swProdManufacturer);
    prod_manufacturer[MAXSIZE_swProdManufacturer] = '\0';
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetProductDescription
 * ---------------------------------------------------------------------
 * PURPOSE  : Get product description.
 * INPUT    : none.
 * OUTPUT   : prod_descrption -- product description.
 * RETURN   : TRUE/FALSE
 * NOTES    : Allocated size shall be (MAXSIZE_swProdDescription+1) bytes.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetProductDescription(char *prod_description)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductDescription_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ProductDescription_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GET_PRODUCT_DESCRIPTION,
                     msg_p,
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductDescription_T),
                     SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ProductDescription_T),
                     (UI32_T)FALSE);
    strncpy(prod_description, data_p->product_description, MAXSIZE_swProdDescription);
    prod_description[MAXSIZE_swProdDescription] = '\0';
    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}


#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsagePercentage
 * ---------------------------------------------------------------------
 * PURPOSE  : The current CPU utilization in percent in the past 5 seconds.
 * INPUT    : None.
 * OUTPUT   : cpu_util_p       -- cpu usage (X%)
 *            cpu_util_float_p -- fractional part of cpu usage (X * 0.001%)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsagePercentage(UI32_T *cpu_util_p, UI32_T *cpu_util_float_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsagePercentage_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsagePercentage_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETCPUUSAGEPERCENTAGE,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsagePercentage_T),
                         (UI32_T)FALSE);

    *cpu_util_p = data_p->cpu_util;
    *cpu_util_float_p = data_p->cpu_util_float;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsageMaximum
 * ---------------------------------------------------------------------
 * PURPOSE  : The maximum CPU utilization in percent in the past 60 seconds.
 * INPUT    : None.
 * OUTPUT   : cpu_util_max_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsageMaximum(UI32_T *cpu_util_max_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageStat_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETCPUUSAGEMAXIMUM,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T),
                         (UI32_T)FALSE);

    *cpu_util_max_p = data_p->cpu_stat;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsageAverage
 * ---------------------------------------------------------------------
 * PURPOSE  : The average CPU utilization in percent in the past 60 seconds.
 * INPUT    : None.
 * OUTPUT   : cpu_util_avg_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsageAverage(UI32_T *cpu_util_avg_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageStat_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETCPUUSAGEAVERAGE,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T),
                         (UI32_T)FALSE);

    *cpu_util_avg_p = data_p->cpu_stat;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsagePeakTime
 * ---------------------------------------------------------------------
 * PURPOSE  : The start time of peak CPU usage of the latest alarm.
 * INPUT    : None.
 * OUTPUT   : cpu_util_peak_time -- peak start time (seconds)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsagePeakTime(UI32_T *cpu_util_peak_time)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageStat_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETCPUUSAGEPEAKTIME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T),
                         (UI32_T)FALSE);

    *cpu_util_peak_time = data_p->cpu_stat;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUsagePeakDuration
 * ---------------------------------------------------------------------
 * PURPOSE  : The duration time of peak CPU usage of the latest alarm.
 * INPUT    : None.
 * OUTPUT   : cpu_util_peak_duration -- peak duration time (seconds)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUsagePeakDuration(UI32_T *cpu_util_peak_duration)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageStat_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETCPUUSAGEPEAKDURATION,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T),
                         (UI32_T)FALSE);

    *cpu_util_peak_duration = data_p->cpu_stat;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUtilAlarmStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : The alarm status of CPU utilization.
 * INPUT    : None.
 * OUTPUT   : alarm_status_p -- alarm status
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUtilAlarmStatus(BOOL_T *alarm_status_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageStat_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETCPUUTILALARMSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageStat_T),
                         (UI32_T)FALSE);

    *alarm_status_p = data_p->cpu_stat;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set rising threshold of CPU utilization
 * INPUT    : rising_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuUtilRisingThreshold(UI32_T rising_threshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsagePercentage_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->threshold = rising_threshold;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETCPUUTILRISINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUtilRisingThreshold(UI32_T *rising_threshold_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETCPUUTILRISINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T),
                         (UI32_T)FALSE);

    *rising_threshold_p = data_p->threshold;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningCpuUtilRisingThreshold(UI32_T *rising_threshold_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGCPUUTILRISINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T),
                         SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *rising_threshold_p = data_p->threshold;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set falling threshold of CPU utilization
 * INPUT    : falling_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuUtilFallingThreshold(UI32_T falling_threshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->threshold = falling_threshold;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETCPUUTILFALLINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get falling threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUtilFallingThreshold(UI32_T *falling_threshold_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETCPUUTILFALLINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T),
                         (UI32_T)FALSE);

    *falling_threshold_p = data_p->threshold;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get falling threshold of CPU utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningCpuUtilFallingThreshold(UI32_T *falling_threshold_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGCPUUTILFALLINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuUsageThreshold_T),
                         SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *falling_threshold_p = data_p->threshold;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardHighWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set high watermark.
 * INPUT    : watermark.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardHighWatermark(UI32_T watermark)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->value = watermark;

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_SETCPUGUARDHIGHWATERMARK,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardLowWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set low watermark.
 * INPUT    : watermark.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardLowWatermark(UI32_T watermark)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->value = watermark;

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_SETCPUGUARDLOWWATERMARK,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardMaxThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set max threshold.
 * INPUT    : threshold.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardMaxThreshold(UI32_T threshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->value = threshold;

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_SETCPUGUARDMAXTHRESHOLD,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardMinThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set min threshold.
 * INPUT    : threshold.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardMinThreshold(UI32_T threshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->value = threshold;

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_SETCPUGUARDMINTHRESHOLD,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set cpu guard status.
 * INPUT    : status - TRUE/FALSE.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardStatus(BOOL_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->status = status;

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_SETCPUGUARDSTATUS,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetCpuGuardTrapStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Set cpu guard trap status.
 * INPUT    : status - TRUE/FALSE.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetCpuGuardTrapStatus(BOOL_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->status = status;

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_SETCPUGUARDTRAPSTATUS,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuGuardInfo
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get cpu guard information.
 * INPUT    : None.
 * OUTPUT   : info - cpu guard information.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuGuardInfo(SYS_MGR_CpuGuardInfo_T *info)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuardInfo_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuardInfo_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    if (info == NULL)
    {
        return FALSE;
    }

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_GETCPUGUARDINFO,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuardInfo_T),
            (UI32_T)FALSE);

    *info = data_p->info;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardHighWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get high watermark.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardHighWatermark(UI32_T *value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    if (value == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDHIGHWATERMARK,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *value = data_p->value;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardLowWatermark
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get low watermark.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardLowWatermark(UI32_T *value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    if (value == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDLOWWATERMARK,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *value = data_p->value;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardMaxThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get max threshold.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardMaxThreshold(UI32_T *value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    if (value == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDMAXTHRESHOLD,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *value = data_p->value;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardMinThreshold
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get min threshold.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardMinThreshold(UI32_T *value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    if (value == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDMINTHRESHOLD,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *value = data_p->value;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get status.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardStatus(BOOL_T *value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    if (value == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDSTATUS,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *value = data_p->status;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningCpuGuardTrapStatus
 * -----------------------------------------------------------------------------
 * PURPOSE  : Get status.
 * INPUT    : None.
 * OUTPUT   : value.
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T.
 * NOTES    : None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
SYS_PMGR_GetRunningCpuGuardTrapStatus(BOOL_T *value)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_CpuGuard_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    if (value == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYS_PMGR_SendMsg(
            SYS_MGR_IPC_CMD_GETRUNNINGCPUGUARDTRAPSTATUS,
            msg_p,
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
            SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_CpuGuard_T),
            SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *value = data_p->status;

    return SYS_MGR_MSG_RETVAL(msg_p);
}
#endif


#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetCpuUtilByName
 * ---------------------------------------------------------------------
 * PURPOSE  : Get task CPU util info
 * INPUT    : cpu_util_info_p->task_name
 *            get_next
 * OUTPUT   : cpu_util_info_p
 * RETURN   : TRUE/FALSE
 * NOTES    : To get the first entry by task_name[0] = 0 and get_next = TRUE
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetCpuUtilByName(
    SYS_MGR_TaskCpuUtilInfo_T *cpu_util_info_p,
    BOOL_T get_next)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_TaskCpuUtil_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_TaskCpuUtil_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->cpu_util_info = *cpu_util_info_p;
    data_p->get_next = get_next;

    SYS_PMGR_SendMsg(
        SYS_MGR_IPC_CMD_GETCPUUTILBYNAME,
        msg_p,
        SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_TaskCpuUtil_T),
        SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_TaskCpuUtil_T),
        (UI32_T)FALSE);

    if ((BOOL_T)SYS_MGR_MSG_RETVAL(msg_p))
    {
        *cpu_util_info_p = data_p->cpu_util_info;
    }

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE) */
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) */

#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
/*------------------------------------------------------------------------------
 * Function : SYS_PMGR_GetMemoryUtilizationBrief
 *------------------------------------------------------------------------------
 * Purpose  : This function will get memory utilization
 * INPUT    : none
 * OUTPUT   : sys_mem_brief_p
 * RETURN   : TRUE/FALSE
 * NOTES    : shall update each 5 seconds.
 *-----------------------------------------------------------------------------*/
BOOL_T  SYS_PMGR_GetMemoryUtilizationBrief(SYS_MGR_MemoryUtilBrief_T *sys_mem_brief_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageBrief_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemoryUsageBrief_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETMEMORYUTILIZATIONBRIEF,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageBrief_T),
                         (UI32_T)FALSE);

    *sys_mem_brief_p = data_p->sys_mem_brief;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilTotalMemory
 * ---------------------------------------------------------------------
 * PURPOSE  : Get total memory capability
 * INPUT    : None.
 * OUTPUT   : total_memory_p -- total size of usable memory (bytes)
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilTotalMemory(MEM_SIZE_T *total_memory_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemorySize_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemorySize_T*data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETMEMORYUTILTOTALMEMORY,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemorySize_T),
                         (UI32_T)FALSE);

    *total_memory_p = data_p->mem_size;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilFreePercentage
 * ---------------------------------------------------------------------
 * PURPOSE  : Get free memory percentage
 * INPUT    : None.
 * OUTPUT   : free_percentage_p
 * RETURN   : TRUE/FALSE
 * NOTES    : shall update each 5 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilFreePercentage(UI32_T *free_percentage_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryPercentage_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemoryPercentage_T*data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETMEMORYUTILFREEPERCENTAGE,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryPercentage_T),
                         (UI32_T)FALSE);

    *free_percentage_p = data_p->percentage;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilAlarmStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : The alarm status of memory utilization.
 * INPUT    : None.
 * OUTPUT   : alarm_status_p -- alarm status
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilAlarmStatus(BOOL_T *alarm_status_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageStat_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemoryUsageStat_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETMEMORYUTILALARMSTATUS,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageStat_T),
                         (UI32_T)FALSE);

    *alarm_status_p = data_p->status;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetMemoryUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set rising threshold of memory utilization
 * INPUT    : rising_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetMemoryUtilRisingThreshold(UI32_T rising_threshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemoryUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->threshold = rising_threshold;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETMEMORYUTILRISINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilRisingThreshold(UI32_T *rising_threshold_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemoryUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETMEMORYUTILRISINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T),
                         (UI32_T)FALSE);

    *rising_threshold_p = data_p->threshold;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningMemoryUtilRisingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : rising_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningMemoryUtilRisingThreshold(UI32_T *rising_threshold_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemoryUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGMEMORYUTILRISINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T),
                         SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *rising_threshold_p = data_p->threshold;

    return SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetMemoryUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Set rising threshold of memory utilization
 * INPUT    : falling_threshold -- (0..100)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetMemoryUtilFallingThreshold(UI32_T falling_threshold)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemoryUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);
    data_p->threshold = falling_threshold;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SETMEMORYUTILFALLINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetMemoryUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get rising threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetMemoryUtilFallingThreshold(UI32_T *falling_threshold_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemoryUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETMEMORYUTILFALLINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T),
                         (UI32_T)FALSE);

    *falling_threshold_p = data_p->threshold;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetRunningMemoryUtilFallingThreshold
 * ---------------------------------------------------------------------
 * PURPOSE  : Get falling threshold of memory utilization
 * INPUT    : None.
 * OUTPUT   : falling_threshold_p
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningMemoryUtilFallingThreshold(UI32_T *falling_threshold_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_MemoryUsageThreshold_T *data_p;

    data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GETRUNNINGMEMORYUTILFALLINGTHRESHOLD,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPCMsg_EmptyData_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_MemoryUsageThreshold_T),
                         SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *falling_threshold_p = data_p->threshold;

    return SYS_MGR_MSG_RETVAL(msg_p);
}
#endif /* (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE) */

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_QueryNextReloadInTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload in date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : UI32_T remain_seconds
 * OUTPUT   : SYS_TIME_DST time
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_QueryNextReloadInTime(I32_T remain_seconds, SYS_TIME_DST *next_reload_time)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadInTime_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_NextReloadInTime_T* data_p = SYS_MGR_MSG_DATA(msg_p);

    data_p->remain_seconds = remain_seconds;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_QUERY_NEXT_RELOAD_IN_TIME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadInTime_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadInTime_T),
                         (UI32_T)FALSE);

    *next_reload_time = data_p->next_reload_time;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_QueryNextReloadAtTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload in date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST reload_at
 * OUTPUT   : SYS_TIME_DST time
 *            BOOL_T        function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_QueryNextReloadAtTime(
    SYS_RELOAD_OM_RELOADAT_DST reload_at,
    SYS_TIME_DST *next_reload_time,
    BOOL_T *function_active
    )
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadAtTime_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_NextReloadAtTime_T* data_p = SYS_MGR_MSG_DATA(msg_p);

    data_p->reload_at = reload_at;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_QUERY_NEXT_RELOAD_AT_TIME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadAtTime_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadAtTime_T),
                         (UI32_T)FALSE);

    *next_reload_time = data_p->next_reload_time;
    *function_active = data_p->function_active;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_QueryNextReloadRegularityTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to query next reload reload_regularity date, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity
 *            ( 1<= reload_regularity.day_of_month <= 31)
 *            ( 1<= reload_regularity.day_of_week <= 7)
 * OUTPUT   : SYS_TIME_DST time
 *            BOOL_T        function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_QueryNextReloadRegularityTime(
    SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity,
    SYS_TIME_DST *next_reload_time,
    BOOL_T *function_active
    )
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadRegularityTime_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_NextReloadRegularityTime_T* data_p = SYS_MGR_MSG_DATA(msg_p);

    data_p->reload_regularity = reload_regularity;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_QUERY_NEXT_RELOAD_REGULARITY_TIME,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadAtTime_T),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_NextReloadAtTime_T),
                         (UI32_T)FALSE);

    *next_reload_time = data_p->next_reload_time;
    *function_active = data_p->function_active;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetReloadTimeInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get remain time and reload time.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 *            SYS_TIME_DST  *next_reload_time
 *            I32_T         *remain_seconds
 * RETURN   : UI32_T
 * NOTES    : return value
 *              0 : Get value success. (remaining_reload_time is continue countdown)
 *              1 : Get value fail
 *              2 : No reload function on. (reload-in, reload-at, reload-regularity are all off)
 *              3 : At least one reload function is on, but remaining_reload_time is nerver countdown
 *                  (The System time is nerver changed by user or sntp)
 *              4 : At least one reload function is on, but remaining_reload_time is nerver countdown
 *                  (Reload at time passed)
 *              "3" and "4" may happen together, but we return "3" advanced.
 * ---------------------------------------------------------------------
 */
UI32_T SYS_PMGR_GetReloadTimeInfo(I32_T *remain_seconds, SYS_TIME_DST *reload_time)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadTimeInfo_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ReloadTimeInfo_T* data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GET_RELOAD_TIME_INFOR,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadTimeInfo_T),
                         (UI32_T)FALSE);

    *remain_seconds = data_p->remain_seconds;
    *reload_time = data_p->reload_time;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetReloadIn
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-in time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : UI32_T minute
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetReloadIn(UI32_T minute)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadIn_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ReloadIn_T* data_p = SYS_MGR_MSG_DATA(msg_p);

    data_p->minute  = minute;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SET_RELOAD_IN,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadIn_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_SetReloadAt
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetReloadAt(SYS_RELOAD_OM_RELOADAT_DST *reload_at_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadAt_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ReloadAt_T* data_p = SYS_MGR_MSG_DATA(msg_p);

    data_p->reload_at = *reload_at_p;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SET_RELOAD_AT,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadAt_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_SetReloadRegularity
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to set reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_SetReloadRegularity(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadRegularity_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ReloadRegularity_T* data_p = SYS_MGR_MSG_DATA(msg_p);

    data_p->reload_regularity = *reload_regularity_p;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_SET_RELOAD_REGULARITY,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadRegularity_T),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetReloadInInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-in time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : UI32_T        *remain_seconds
 *            SYS_TIME_DST  *next_reload_time
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetReloadInInfo(I32_T *remain_seconds, SYS_TIME_DST *next_reload_time)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadInInfo_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ReloadInInfo_T *data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GET_RELOAD_IN_INFO,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadInInfo_T),
                         (UI32_T)FALSE);

    *remain_seconds = data_p->remain_seconds;
    *next_reload_time = data_p->next_reload_time;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetReloadAtInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 *            SYS_TIME_DST  *next_reload_time
 *            I32_T         *remain_seconds
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetReloadAtInfo(SYS_RELOAD_OM_RELOADAT_DST *reload_at, SYS_TIME_DST *next_reload_time, I32_T *remain_seconds, BOOL_T *function_active)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadAtInfo_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ReloadAtInfo_T *data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GET_RELOAD_AT_INFO,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadAtInfo_T),
                         (UI32_T)FALSE);

    *reload_at = data_p->reload_at;
    *next_reload_time = data_p->next_reload_time;
    *remain_seconds = data_p->remain_seconds;
    *function_active = data_p->function_active;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_PMGR_GetRunningReloadAtInfo
 * -------------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-at time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 * OUTPUT   : value
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningReloadAtInfo(
    SYS_RELOAD_OM_RELOADAT_DST *reload_at
    )
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadAt_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ReloadAt_T* data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(
        SYS_MGR_IPC_CMD_GET_RUNN_RELOAD_AT_INFO,
        msg_p,
        SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadAt_T),
        (SYS_TYPE_Get_Running_Cfg_T)SYS_TYPE_GET_RUNNING_CFG_FAIL
        );

    *reload_at = data_p->reload_at;

    return (SYS_TYPE_Get_Running_Cfg_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_GetReloadRegularityInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 *            SYS_TIME_DST  *next_reload_time
 *            I32_T         *remain_seconds
 *            BOOL_T        *function_active (if system time doesnot be changed, function_active =FALSE)
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_GetReloadRegularityInfo(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity, SYS_TIME_DST *next_reload_time, I32_T *remain_seconds, BOOL_T *function_active)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadRegularityInfo_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ReloadRegularityInfo_T *data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_GET_RELOAD_REGULARITY_INFO,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadRegularityInfo_T),
                         (UI32_T)FALSE);

    *reload_regularity = data_p->reload_regularity;
    *next_reload_time = data_p->next_reload_time;
    *remain_seconds = data_p->remain_seconds;
    *function_active = data_p->function_active;

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_RELOAD_MGR_GetRunningReloadRegularityInfo
 * -------------------------------------------------------------------------
 * PURPOSE  : This function is used to get reload-regularity time, available for
 *            Management on SYS_RELOAD.
 * INPUT    : None
 * OUTPUT   : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_PMGR_GetRunningReloadRegularityInfo(
    SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
    )
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadRegularity_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SYS_MGR_IPC_ReloadRegularity_T* data_p = SYS_MGR_MSG_DATA(msg_p);

    SYS_PMGR_SendMsg(
        SYS_MGR_IPC_CMD_GET_RUNN_RELOAD_REGULARITY_INFO,
        msg_p,
        SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        SYS_MGR_GET_MSGBUFSIZE(SYS_MGR_IPC_ReloadRegularity_T),
        (SYS_TYPE_Get_Running_Cfg_T)SYS_TYPE_GET_RUNNING_CFG_FAIL
        );

    *reload_regularity = data_p->reload_regularity;

    return (SYS_TYPE_Get_Running_Cfg_T)SYS_MGR_MSG_RETVAL(msg_p);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_MGR_ReloadInCancel
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to cancel reload-in function and clear database
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_ReloadInCancel()
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_RELOAD_IN_CANCEL,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_ReloadAtCancel
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to cancel reload-at function and clear database
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_ReloadAtCancel()
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_RELOAD_AT_CANCEL,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_PMGR_ReloadRegularityCancel
 * ---------------------------------------------------------------------
 * PURPOSE  : This function is used to cancel reload-regularity function and clear database
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_PMGR_ReloadRegularityCancel()
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    SYS_PMGR_SendMsg(SYS_MGR_IPC_CMD_RELOAD_REGULARITY_CANCEL,
                         msg_p,
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         SYS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                         (UI32_T)FALSE);

    return (BOOL_T)SYS_MGR_MSG_RETVAL(msg_p);
}

#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */

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
static void SYS_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_SYSMGMT;
    msg_p->msg_size = req_size;

    SYS_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK ||
        SYS_MGR_MSG_RETVAL(msg_p) == SYS_MGR_IPC_RESULT_FAIL)
        SYS_MGR_MSG_RETVAL(msg_p) = ret_val;
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
static void SYS_PMGR_SendMsgWithoutWaittingResponse(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size)
{
    msg_p->cmd = SYS_MODULE_SYSMGMT;
    msg_p->msg_size = req_size;

    SYS_MGR_MSG_CMD(msg_p) = cmd;

    SYSFUN_SendRequestMsg(ipcmsgq_handle,
                          msg_p,
                          SYSFUN_TIMEOUT_WAIT_FOREVER,
                          SYSFUN_SYSTEM_EVENT_IPCMSG,
                          0,
                          NULL);
}
#endif
/* End of this file */

