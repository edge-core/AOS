/* MODULE NAME:  1x_pom.c
 * PURPOSE:
 * This is a sample code for implementation of POM for OM in kernel space.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/3/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_module.h"
#include "sysfun.h"
#include "1x_om.h"
#include "l_mm.h"
#include <string.h>
#include "sys_bld.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLUSTER_POM_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CSCA_POM.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DOT1X_POM_Init(void)
{
    /* Given that DOT1X is run in DOT1X_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Get pe-port MaxReq.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: MaxReq.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortMaxReq(UI32_T lport)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PORTMAXREQ;


    /*assign input
     */
    msg_data_p->data.ui32_v=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_MaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_POM_GetRunning_MaxReq(UI32_T *max_req)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_GETRUNNING_MAXREQ;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *max_req=msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortReAuthMax
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default ReAuth-MAX is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortReAuthMax(UI32_T lport,UI32_T *max_req)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_ui32arg)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_RUNNING_PORT_REAUTH_MAX;


    /*assign input
     */
    msg_data_p->data.lport_ui32arg.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *max_req=msg_data_p->data.lport_ui32arg.ui32arg;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_POM_GetRunning_PortMaxReq(UI32_T lport,UI32_T *max_req)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_ui32arg)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_PORTMAXREQ;


    /*assign input
     */
    msg_data_p->data.lport_ui32arg.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *max_req=msg_data_p->data.lport_ui32arg.ui32arg;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get port control mode of 1X configuration
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: type.
 * NOTES:  Type = VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized for ForceUnauthorized
                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized for ForceAuthorized
                  VAL_dot1xAuthAuthControlledPortControl_auto for Auto
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortControlMode(UI32_T lport)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PORTCONTROLMODE;


    /*assign input
     */
    msg_data_p->data.ui32_v=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: PortControlMode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_POM_GetRunning_PortControlMode(UI32_T lport,UI32_T *mode)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_ui32arg)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_PORTCONTROLMODE;


    /*assign input
     */
    msg_data_p->data.lport_ui32arg.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *mode=msg_data_p->data.lport_ui32arg.ui32arg;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get port period re-authentication status of the client
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: control.
 * NOTES:  control =  VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
                      VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortReAuthEnabled(UI32_T lport)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PORTREAUTHENABLED;


    /*assign input
     */
    msg_data_p->data.ui32_v=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortReAuthEnabled is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: mode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_POM_GetRunning_PortReAuthEnabled(UI32_T lport,UI32_T *mode)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_ui32arg)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_PORTREAUTHENABLED;


    /*assign input
     */
    msg_data_p->data.lport_ui32arg.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *mode=msg_data_p->data.lport_ui32arg.ui32arg;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port QuietPeriod
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: QuietPeriod.
 * NOTES:  0 -- Get error.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortQuietPeriod(UI32_T lport)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PORTQUIETPERIOD;


    /*assign input
     */
    msg_data_p->data.ui32_v=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return 0;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default QuietPeriod is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: QuietPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_QuietPeriod(UI32_T *seconds)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_GETRUNNING_QUIETPERIOD;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *seconds=msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default QuietPeriod is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: QuietPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortQuietPeriod(UI32_T lport,UI32_T *seconds)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_ui32arg)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_PORTQUIETPERIOD;


    /*assign input
     */
    msg_data_p->data.lport_ui32arg.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *seconds=msg_data_p->data.lport_ui32arg.ui32arg;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port ReAuthPeriod
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: ReAuthPeriod.
 * NOTES:  0 -- Get error.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortReAuthPeriod(UI32_T lport)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PORTREAUTHPERIOD;


    /*assign input
     */
    msg_data_p->data.ui32_v=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return 0;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default ReAuthPeriod is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: ReAuthPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_ReAuthPeriod(UI32_T *seconds)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_REAUTHPERIOD;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *seconds=msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default ReAuthPeriod is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: ReAuthPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortReAuthPeriod(UI32_T lport,UI32_T *seconds)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_ui32arg)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_PORTREAUTHPERIOD;


    /*assign input
     */
    msg_data_p->data.lport_ui32arg.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *seconds=msg_data_p->data.lport_ui32arg.ui32arg;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port TxPeriod.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TxPeriod.
 * NOTES:  0 -- Get error.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortTxPeriod(UI32_T lport)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PORTTXPERIODD;


    /*assign input
     */
    msg_data_p->data.ui32_v=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return 0;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TxPeriod is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: TxPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_TxPeriod(UI32_T *seconds)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_TXPERIOD;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *seconds=msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TxPeriod is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: TxPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortTxPeriod(UI32_T lport,UI32_T *seconds)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_ui32arg)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_PORTTXPERIOD;


    /*assign input
     */
    msg_data_p->data.lport_ui32arg.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *seconds=msg_data_p->data.lport_ui32arg.ui32arg;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default supp-timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: AuthSuppTimeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_AuthSuppTimeout(UI32_T lport, UI32_T *seconds)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_ui32arg)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_AUTH_SUPP_TIMEOUT;


    /*assign input
     */
    msg_data_p->data.lport_ui32arg.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *seconds=msg_data_p->data.lport_ui32arg.ui32arg;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Get the administrative enable/disable state for Port Access
            Control in a system.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: SystemAuthControl
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
           VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_SystemAuthControl()
{
    const UI32_T msg_buf_size=DOT1X_OM_DATA_START_OFFSET;
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_SYSTEMAUTHCONTROL;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default SystemAuthControl is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: SystemAuthControl
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_SystemAuthControl(UI32_T *control)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_SYSTEMAUTHCONTROL;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *control=msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetPaePortEntry(UI32_T lport, DOT1X_PaePortEntry_T *entry_p)
{
   const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_paeportentry)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PAE_PORT_ENTRY;


    /*assign input
     */
    msg_data_p->data.lport_paeportentry.lport = lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *entry_p = msg_data_p->data.lport_paeportentry.paeportentry;

    /*return result
     */
    return msg_data_p->type.result_bool;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetNextPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextPaePortEntry(UI32_T *lport_p, DOT1X_PaePortEntry_T *entry_p)
{
   const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_paeportentry)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_NEXT_PAE_PORT_ENTRY;


    /*assign input
     */
    msg_data_p->data.lport_paeportentry.lport = *lport_p;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *lport_p = msg_data_p->data.lport_paeportentry.lport;
    *entry_p = msg_data_p->data.lport_paeportentry.paeportentry;

    /*return result
     */
    return msg_data_p->type.result_bool;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetAuthConfigEntry(UI32_T lport, DOT1X_AuthConfigEntry_T *entry_p)
{
   const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_authconfigentry)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_AUTH_CONFIG_ENTRY;


    /*assign input
     */
    msg_data_p->data.lport_authconfigentry.lport = lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    *entry_p = msg_data_p->data.lport_authconfigentry.authconfigentry;

    /*return result
     */
    return msg_data_p->type.result_bool;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetNextAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextAuthConfigEntry(UI32_T *lport_p, DOT1X_AuthConfigEntry_T *entry_p)
{
   const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_authconfigentry)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_NEXT_AUTH_CONFIG_ENTRY;


    /*assign input
     */
    msg_data_p->data.lport_authconfigentry.lport = *lport_p;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *lport_p = msg_data_p->data.lport_authconfigentry.lport;
    *entry_p = msg_data_p->data.lport_authconfigentry.authconfigentry;

    /*return result
     */
    return msg_data_p->type.result_bool;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetAuthStatsEntry(UI32_T lport, DOT1X_AuthStatsEntry_T *entry_p)
{
   const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_authstateentry)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_AUTH_STATS_ENTRY;


    /*assign input
     */
    msg_data_p->data.lport_authstateentry.lport = lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *entry_p = msg_data_p->data.lport_authstateentry.authstateentry;

    /*return result
     */
    return msg_data_p->type.result_bool;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetNextAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextAuthStatsEntry(UI32_T *lport_p, DOT1X_AuthStatsEntry_T *entry_p)
{
   const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_authstateentry)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_NEXT_AUTH_STATS_ENTRY;


    /*assign input
     */
    msg_data_p->data.lport_authstateentry.lport = *lport_p;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *lport_p = msg_data_p->data.lport_authstateentry.lport;
    *entry_p = msg_data_p->data.lport_authstateentry.authstateentry;

    /*return result
     */
    return msg_data_p->type.result_bool;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetAuthDiagEntry(UI32_T lport, DOT1X_AuthDiagEntry_T*entry_p)
{
   const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_authdiagentry)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_AUTH_DIAG_ENTRY;


    /*assign input
     */
    msg_data_p->data.lport_authdiagentry.lport = lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *entry_p = msg_data_p->data.lport_authdiagentry.authdiagentry;

    /*return result
     */
    return msg_data_p->type.result_bool;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetNextAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextAuthDiagEntry(UI32_T *lport_p, DOT1X_AuthDiagEntry_T*entry_p)
{
   const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_authdiagentry)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_NEXT_AUTH_DIAG_ENTRY;


    /*assign input
     */
    msg_data_p->data.lport_authdiagentry.lport = *lport_p;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *lport_p = msg_data_p->data.lport_authdiagentry.lport;
    *entry_p = msg_data_p->data.lport_authdiagentry.authdiagentry;

    /*return result
     */
    return msg_data_p->type.result_bool;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_Get_Global_Parameters
 *---------------------------------------------------------------------------
 * PURPOSE:  Get dot1x global parameters.
 * INPUT:    global_parameters pointer.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void DOT1X_POM_Get_Global_Parameters(DOT1X_Global_Parameters_T * global_parameters)
{
   const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.global_parameters)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_GLOBAL_PARAMETERS;

    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        memset(global_parameters,0,sizeof(msg_data_p->data.global_parameters));
        return;
    }

    /*assign output
     */
    *global_parameters=msg_data_p->data.global_parameters;

    /*return result
     */
    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetPortAuthorized
 * --------------------------------------------------------------------------
 * PURPOSE : Get the current authorization state of the port
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_AUTHORIZED,
 *           DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_UNAUTHORIZED or
 *           DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_ERR
 * NOTE    : None
 * --------------------------------------------------------------------------
 */
DOT1X_TYPE_AuthControlledPortStatus_T DOT1X_POM_GetPortAuthorized(UI32_T lport)
 {
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PORT_AUTHORIZED;

    /*assign input
     */
    msg_data_p->data.ui32_v=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME: DOT1X_POM_Get_Port_Details
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_Get_Port_Details(UI32_T lport, DOT1X_PortDetails_T *port_details_p)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_portdetails)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PORT_DETAILS;

    /*assign input
     */
    msg_data_p->data.lport_portdetails.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *port_details_p = msg_data_p->data.lport_portdetails.port_details;
    /*return result
     */
    return msg_data_p->type.result_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME: DOT1X_POM_GetNextPortDetails
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextPortDetails(UI32_T lport, UI32_T *index_p, DOT1X_PortDetails_T *port_details_p)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_macidx_portdetails)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_NEXT_PORT_DETAILS;

    /*assign input
     */
    msg_data_p->data.lport_macidx_portdetails.lport=lport;
    msg_data_p->data.lport_macidx_portdetails.mac_index = *index_p;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *index_p = msg_data_p->data.lport_macidx_portdetails.mac_index;
    *port_details_p = msg_data_p->data.lport_macidx_portdetails.port_details;
    /*return result
     */
    return msg_data_p->type.result_bool;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortMultiHostMacCount is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: PortMultiHostMacCount
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_POM_GetRunning_PortMultiHostMacCount(UI32_T lport,UI32_T *count)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_ui32arg)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_PORTMULTIHOSTMACCOUNT;

    /*assign input
     */
    msg_data_p->data.lport_ui32arg.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *count=msg_data_p->data.lport_ui32arg.ui32arg;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortReAuthMax
 * ---------------------------------------------------------------------
 * PURPOSE: Get the port's re-auth max
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortReAuthMax.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortReAuthMax(UI32_T lport)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.ui32_v)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GET_PORTREAUTHMAX;

    /*assign input
     */
    msg_data_p->data.ui32_v=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the authenticator's operation mode
 * INPUT:  lport.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortOperationMode(UI32_T lport,UI32_T *mode)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_mode)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETRUNNING_PORTOPERATIONMODE;

    /*assign input
     */
    msg_data_p->data.lport_mode.lport=lport;


    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_buf_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *mode=msg_data_p->data.lport_mode.mode;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetNext_MAC_Auth_Stats_Table
 * ---------------------------------------------------------------------
 * PURPOSE: A table that contains the statistics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each supplicant on the port
 *          that may authenticate access to itself.
 * INPUT:  lport.
 * OUTPUT: AuthStateEntry,supplicant_mac.
 * RETURN: TRUE/FALSE
 * NOTES:  index = 0xFFFF --- get the first entry on the port.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNext_MAC_Auth_Stats_Table(UI32_T lport,UI32_T *mac_index,DOT1X_AuthStatsEntry_T *AuthStateEntry,UI8_T *supplicant_mac)
{
    const UI32_T msg_buf_size=(sizeof(((DOT1X_OM_IPCMsg_T *)0)->data.lport_mac_authstate_mac)
        +DOT1X_OM_DATA_START_OFFSET);
    SYSFUN_Msg_T *msg_p;
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DOT1X;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DOT1X_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DOT1X_OM_IPCCMD_GETNEXT_MAC_AUTH_STATS_TABLE;

    /*assign input
     */
    msg_data_p->data.lport_mac_authstate_mac.lport=lport;
    msg_data_p->data.lport_mac_authstate_mac.mac_index=*mac_index;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *mac_index=msg_data_p->data.lport_mac_authstate_mac.mac_index;
    *AuthStateEntry=msg_data_p->data.lport_mac_authstate_mac.AuthStateEntry;
    memcpy(supplicant_mac,
        msg_data_p->data.lport_mac_authstate_mac.supplicant_mac,sizeof(msg_data_p->data.lport_mac_authstate_mac.supplicant_mac));
    /*return result
     */
    return msg_data_p->type.result_bool;
}

