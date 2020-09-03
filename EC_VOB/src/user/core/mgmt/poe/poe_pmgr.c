/*-----------------------------------------------------------------------------
 * FILE NAME: poe_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for POE MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "poe_mgr.h"
#include "poe_pmgr.h"
#include "poe_type.h"
#include "leaf_3621.h"

/* NAMING CONSTANT DECLARATIONS
 */
#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */

static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : POE_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for POE_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T POE_PMGR_InitiateProcessResources(void)
{
    /* get the handle of ipc message queues for POE MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
                SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): SYSFUN_GetMsgQ failed.\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
 } /* End of POE_PMGR_InitiateProcessResources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetMainpowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE mainpower maxmum allocation
 * INPUT    : group_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetMainpowerMaximumAllocation(UI32_T group_index, UI32_T value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETMAINPOWERMAXIMUMALLOCATION;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetMainpowerMaximumAllocation */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPsePortAdmin
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETPSEPORTADMIN;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetPsePortAdmin */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPortPowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETPORTPOWERMAXIMUMALLOCATION;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetPortPowerMaximumAllocation */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPsePortPowerPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETPSEPORTPOWERPRIORITY;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetPsePortPowerPriority */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetLegacyDetection
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetLegacyDetection(UI32_T group_index, UI32_T value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETLEGACYDECTECTION;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetLegacyDetection */

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPortManualHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETPORTMANUALHIGHPOWERMODE;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetPortManualHighPowerMode */
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPsePortPowerPairs
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETPSEPORTPOWERPAIRS;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetPsePortPowerPairs */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPsePortType
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value1, value2
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPsePortType(UI32_T group_index, UI32_T port_index, UI8_T* value1, UI32_T value2)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETPSEPORTPOWERTYPE;
    msg_p->data.arg_grp_ui32_ui32_ui32_ui8.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32_ui32_ui8.arg_ui32_2 = port_index;
    memcpy(msg_p->data.arg_grp_ui32_ui32_ui32_ui8.arg_ui8,value1,sizeof(UI8_T)*(MAXSIZE_pethPsePortType));
    msg_p->data.arg_grp_ui32_ui32_ui32_ui8.arg_ui32_3 = value2;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetPsePortType */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetMainPseUsageThreshold
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetMainPseUsageThreshold(UI32_T group_index, UI32_T value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETMAINPSEUSAGETHRESHOLD;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetMainPseUsageThreshold */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetNotificationCtrl
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetNotificationCtrl(UI32_T group_index, UI32_T value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_SETNOTIFICATIONCTRL;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_SetNotificationCtrl */

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_BindTimeRangeToPsePort
 *-------------------------------------------------------------------------
 * PURPOSE  : Bind POE PSE port to a specified time range with name
 * INPUT    : group_index, port_index, time_range
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_BindTimeRangeToPsePort(UI32_T group_index, UI32_T port_index, UI8_T* time_range)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_BINDTIMERANGETOPSEPORT;
    msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2 = port_index;
    strcpy((char*)msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8,(char*)time_range);
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_BindTimeRangeToPsePort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_UnbindTimeRangeToPsePort
 *-------------------------------------------------------------------------
 * PURPOSE  : Unbind POE PSE port to a specified time range with name
 * INPUT    : group_index, port_index
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_UnbindTimeRangeToPsePort(UI32_T group_index, UI32_T port_index)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_MGR_IpcMsg_T *msg_p;

    DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_MGR_IPC_UNBINDTIMERANGETOPSEPORT;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                POE_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }
    return msg_p->type.ret_ui32;
} /* End of POE_PMGR_UnbindTimeRangeToPsePort */
#endif


