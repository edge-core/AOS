/*-----------------------------------------------------------------------------
 * FILE NAME: poe_pom.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for POE OM IPC.
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
#include "l_mm.h"
#include "sysfun.h"
#include "poe_om.h"
#include "poe_pom.h"
#include "poe_type.h"


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
 * ROUTINE NAME : POE_POM_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for POE_POM in the calling process.
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
BOOL_T POE_POM_InitiateProcessResources(void)
{
    /* get the handle of ipc message queues for LLDP OM
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_POE_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of POE_POM_InitiateProcessResources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortAdmin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port admin status
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSEPORTADMIN;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPsePortAdmin */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortPowerPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port power priority
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSEPORTPOWERPRIORITY;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPsePortPowerPriority */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortDetectionStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port detection status
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortDetectionStatus(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSEPORTDECTECTIONSTATUS;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPsePortDetectionStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortPowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE port power maximum allocation
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPORTPOWERMAXIMUMALLOCATION;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPortPowerMaximumAllocation */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortPowerConsumption
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE port power consumption
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortPowerConsumption(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPORTPOWERCONSUMPTION;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPortPowerConsumption */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortManualHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE port manual high power mode
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPORTMANUALHIGHPOWERMODE;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPortManualHighPowerMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetMainpowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE main power maximum allocation
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetMainpowerMaximumAllocation(UI32_T group_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETMAINPOWERMAXIMUMALLOCATION;
	msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetMainpowerMaximumAllocation */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPoeSoftwareVersion
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE software version
 * INPUT    : group_index
 * OUTPUT   : version1, version2, build
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPoeSoftwareVersion(UI32_T group_index, UI8_T *version1, UI8_T *version2, UI8_T *build)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui8_ui8_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPOESOFTWAREVERSION;
	msg_p->data.arg_grp_ui32_ui8_ui8_ui8.arg_ui32 = group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *version1 = msg_p->data.arg_grp_ui32_ui8_ui8_ui8.arg_ui8_1;
    *version2 = msg_p->data.arg_grp_ui32_ui8_ui8_ui8.arg_ui8_2;
    *build =    msg_p->data.arg_grp_ui32_ui8_ui8_ui8.arg_ui8_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPoeSoftwareVersion */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextLegacyDetection
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE next legacy dectection
 * INPUT    : group_index
 * OUTPUT   : group_index, value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextLegacyDetection(UI32_T *group_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETNEXTLEGACYDETECTION;
	msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = *group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *group_index = msg_p->data.arg_grp_ui32_ui32.arg_ui32_1;
    *value = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetNextLegacyDetection */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextPsePortAdmin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE next PSE port admin status
 * INPUT    : group_index, port_idex
 * OUTPUT   : group_index, port_idex, value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextPsePortAdmin(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETNEXTPSEPORTADMIN;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = *group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = *port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *group_index = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1;
    *port_index = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2;
    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetNextPsePortAdmin */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextMainPseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE next main PSE entry
 * INPUT    : group_index
 * OUTPUT   : group_index, entry
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextMainPseEntry(UI32_T *group_index, POE_OM_MainPse_T *entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_mainpse);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETNEXTMAINPSEENTRY;
	msg_p->data.arg_grp_ui32_mainpse.arg_ui32 = *group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *group_index = msg_p->data.arg_grp_ui32_mainpse.arg_ui32;
    *entry = msg_p->data.arg_grp_ui32_mainpse.arg_entry;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetNextMainPseEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPethMainPseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE peth main PSE Entry
 * INPUT    : group_index
 * OUTPUT   : entry
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPethMainPseEntry(UI32_T group_index, POE_OM_MainPse_T *entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_mainpse);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPETHMAINPSEENTRY;
	msg_p->data.arg_grp_ui32_mainpse.arg_ui32 = group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *entry = msg_p->data.arg_grp_ui32_mainpse.arg_entry;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPethMainPseEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port entry
 * INPUT    : group_index, port_idex
 * OUTPUT   : entry
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortEntry(UI32_T group_index, UI32_T port_index, POE_OM_PsePort_T *entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_pseport);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSEPORTENTRY;
	msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *entry = msg_p->data.arg_grp_ui32_ui32_pseport.arg_entry;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPsePortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningPsePortAdmin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running PSE port admin
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETRUNNINGPSEPORTADMIN;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetRunningPsePortAdmin */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningPsePortPowerPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running PSE port power priority
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETRUNNINGPSEPORTPOWERPRIORITY;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetRunningPsePortPowerPriority */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningPortPowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running port power maximum allocation
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETRUNNINGPORTPOWERMAXIMUMALLOCATION;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetRunningPortPowerMaximumAllocation */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningMainPowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running main power maximum allocation
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningMainPowerMaximumAllocation(UI32_T group_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETRUNNINGMAINPOWERMAXIMUMALLOCATION;
	msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetRunningMainPowerMaximumAllocation */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningLegacyDetection
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running legacy detection
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningLegacyDetection(UI32_T group_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETRUNNINGLEGACYDETECTION;
	msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetRunningLegacyDetection */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetMainPseOperStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE main PSE operation status
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetMainPseOperStatus(UI32_T group_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETMAINPSEOPERSTATUS;
	msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetMainPseOperStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortPowerPairsCtrlAbility
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port power pairs control ability
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortPowerPairsCtrlAbility(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSEPORTPOWERPAIRSCTRLABILITY;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPsePortPowerPairsCtrlAbility */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortPowerPairs
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port power pairs
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSEPORTPOWERPAIRS;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPsePortPowerPairs */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortPowerClassifications
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port power pairs
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortPowerClassifications(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSEPORTPOWERCLASSIFICATIONS;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPsePortPowerClassifications */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortPowerCurrent
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE port power current
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortPowerCurrent(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPORTPOWERCURRENT;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPortPowerCurrent */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortPowerVoltage
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE port power voltage
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortPowerVoltage(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPORTPOWERVOLTAGE;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPortPowerVoltage */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextPsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port entry
 * INPUT    : group_index, port_idex
 * OUTPUT   : group_index, port_idex, entry
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextPsePortEntry(UI32_T *group_index, UI32_T *port_index, POE_OM_PsePort_T *entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_pseport);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETNEXTPSEPORTENTRY;
	msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_1 = *group_index;
	msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_2 = *port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *group_index = msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_1;
	*port_index = msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_2;
    *entry = msg_p->data.arg_grp_ui32_ui32_pseport.arg_entry;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetNextPsePortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPseNotificationCtrl
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE main PSE operation status
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPseNotificationCtrl(UI32_T group_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSENOTIFICATIONCONTROL;
	msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPseNotificationCtrl */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextNotificationCtrl
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE main PSE operation status
 * INPUT    : group_index
 * OUTPUT   : group_index, value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextNotificationCtrl(UI32_T *group_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETNEXTNOTIFICATIONCONTROL;
	msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = *group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *group_index = msg_p->data.arg_grp_ui32_ui32.arg_ui32_1;
    *value = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetNextNotificationCtrl */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortDot3atPowerInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the poe infomation for LLDP to transmition frame
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortDot3atPowerInfo(UI32_T group_index, UI32_T port_index, POE_TYPE_Dot3atPowerInfo_T *info)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_3atInfo);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETDOT3ATPORTPOWERINFO;
	msg_p->data.arg_grp_ui32_ui32_3atInfo.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_3atInfo.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *info = msg_p->data.arg_grp_ui32_ui32_3atInfo.arg_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningPortManualHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running port power maximum allocation
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETRUNNINGPORTMANUALHIGHPOWERMODE;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetRunningPortManualHighPowerMode */

#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetUseLocalPower                               
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection        
 * INPUT   : unit
 *           value (TRUE => Local power, FALSE => RPS)
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetUseLocalPower(UI32_T group_index, BOOL_T *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETUSELOCALPOWER;
	msg_p->data.arg_grp_ui32_bool.arg_ui32 = group_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *value = msg_p->data.arg_grp_ui32_bool.arg_bool;

    return msg_p->type.ret_ui32;
}
#endif

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : time_range - time range name
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T *time_range)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSEPORTTIMERANGENAME;
	msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

	strcpy((char*)time_range, (char*)msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPsePortTimeRangeName */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetNextPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range name on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           time_range - time range name
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetNextPsePortTimeRangeName(UI32_T *group_index, UI32_T *port_index, UI8_T *time_range)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETNEXTPSEPORTTIMERANGENAME;
	msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1 = *group_index;
	msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2 = *port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *group_index = msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1;
    *port_index  = msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2;
	strcpy((char*)time_range, (char*)msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetNextPsePortTimeRangeName */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetRunningPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get running POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : time_range
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetRunningPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T *time_range)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETRUNNINGPSEPORTTIMERANGENAME;
	msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

	strcpy((char*)time_range, (char*)msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetRunningPsePortTimeRangeName */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetNextRunningPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get running POE port binding time range name on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           time_range
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetNextRunningPsePortTimeRangeName(UI32_T *group_index, UI32_T *port_index, UI8_T *time_range)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETNEXTRUNNINGPSEPORTTIMERANGENAME;
	msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1 = *group_index;
	msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2 = *port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *group_index = msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1;
    *port_index  = msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2;
	strcpy((char*)time_range, (char*)msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetNextRunningPsePortTimeRangeName */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetPsePortTimeRangeStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range status on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : status - time range status
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetPsePortTimeRangeStatus(UI32_T group_index, UI32_T port_index, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETPSEPORTTIMERANGESTATUS;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *status = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetPsePortTimeRangeStatus */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetNextPsePortTimeRangeStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range status on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           status - time range status
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetNextPsePortTimeRangeStatus(UI32_T *group_index, UI32_T *port_index, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    POE_OM_IpcMsg_T *msg_p;

DBG_PRINT();
    msgbuf_p->cmd = SYS_MODULE_POE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = POE_OM_IPC_GETNEXTPSEPORTTIMERANGESTATUS;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = *group_index;
	msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = *port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return POE_TYPE_RETURN_ERROR;
    }

    *group_index = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1;
    *port_index  = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2;
    *status      = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;

    return msg_p->type.ret_ui32;
} /* End of POE_POM_GetNextPsePortTimeRangeStatus */
#endif


