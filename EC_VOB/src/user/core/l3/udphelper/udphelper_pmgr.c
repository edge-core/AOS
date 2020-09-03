/* MODULE NAME:  udphelper_pmgr.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access UDPHELPER_MGR and UDPHELPER_OM service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call UDPHELPER_PMGR_XXX for APIs UDPHELPER_MGR_XXX provided by UDPHELPER, and same as UDPHELPER_POM for
 *    UDPHELPER_OM APIs
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    03/31/2009 - LinLi, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_bld.h"
#include "udphelper_pmgr.h"
#include "sys_module.h"
#include "l_mm.h"
#include "udphelper_type.h"
#include "udphelper_mgr.h"

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
static SYSFUN_MsgQ_T udphelper_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : UDPHELPER_PMGR_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource used in the calling process.
 *
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use UDPHELPER_PMGR, it should initiate the resource (get the message queue handler internally)
 *
 */
BOOL_T UDPHELPER_PMGR_InitiateProcessResource(void)
{
    /* Given that CSCA PMGR requests are handled in CSCGROUP1 of XXX_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_UDPHELPER_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &udphelper_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : UDPHELPER_PMGR_VlanCreate
* PURPOSE:
*     Create l3 interface
*
* INPUT:
*      vid: the port number
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_L3IfCreate(UI32_T ifindex)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_L3IF_CREATE;

    msg_p->data.ui32_v = ifindex;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_VlanDelete
* PURPOSE:
*     Delete l3 interface
*
* INPUT:
*      vid: the port number
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_L3IfDelete(UI32_T ifindex)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_L3IF_DELETE;

    msg_p->data.ui32_v = ifindex;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_RifCreate
* PURPOSE:
*     Add primary address for l3 interface
*
* INPUT:
*     
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_RifCreate(UI32_T ifindex, L_INET_AddrIp_T addr)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(arg5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_RIF_CREATE;

    msg_p->data.arg5.ifindex = ifindex;
    msg_p->data.arg5.addr = addr;  
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_RifDelete
* PURPOSE:
*     Delete primary address for l3 interface
*
* INPUT:
*     
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_RifDelete(UI32_T ifindex, L_INET_AddrIp_T addr)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(arg5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_RIF_DELETE;

    msg_p->data.arg5.ifindex = ifindex;
    msg_p->data.arg5.addr = addr;  
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_AddForwardUdpPort
* PURPOSE:
*     Add forward port
*
* INPUT:
*      port: the port number
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_AddForwardUdpPort(UI32_T port)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_UDPPORTSET;

    msg_p->data.ui32_v = port;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_DelForwardUdpPort
* PURPOSE:
*     Delete forward port
*
* INPUT:
*      port: the port number
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_DelForwardUdpPort(UI32_T port)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_UDPPORTUNSET;

    msg_p->data.ui32_v = port;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_AddHelperAddress
* PURPOSE:
*     Add helper address
*
* INPUT:
*      vid: the vlan id number
*      addr: the helper address
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_AddHelperAddress(UI32_T ifindex, L_INET_AddrIp_T addr)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(arg5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_HELPERSET;

    msg_p->data.arg5.ifindex = ifindex;
    msg_p->data.arg5.addr = addr;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_DelHelperAddress
* PURPOSE:
*     Delete helper address
*
* INPUT:
*      vid: the vlan id number
*      addr: the helper address
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_DelHelperAddress(UI32_T ifindex, L_INET_AddrIp_T addr)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(arg5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_HELPERUNSET;

    msg_p->data.arg5.ifindex = ifindex;
    msg_p->data.arg5.addr = addr;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_SetStatus
* PURPOSE:
*     Set the udp helper status
*
* INPUT:
*
* OUTPUT:
*      status: the status
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_SetStatus(UI32_T status)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_SET_STATUS;
    
    msg_p->data.ui32_v = status;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_GetStatus
* PURPOSE:
*     Get the udp helper status
*
* INPUT:
*
* OUTPUT:
*      status: the status
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetStatus(UI32_T *status)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_GET_STATUS;

    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    *status = msg_p->data.ui32_v;
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_GetNextForwardPort
* PURPOSE:
*     Get next forward port
*
* INPUT:
*
* OUTPUT:
*     
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetNextForwardPort(UI32_T *port)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_GETNEXT_FORWARD_PORT;

    msg_p->data.ui32_v = *port;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    *port = msg_p->data.ui32_v;
    
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_GetNextHelper
* PURPOSE:
*     Get next helper address
*
* INPUT:
*
* OUTPUT:
*     
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetNextHelper(UI32_T ifindex, L_INET_AddrIp_T *helper)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(arg5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_GETNEXT_HELPER;

    msg_p->data.arg5.ifindex = ifindex;
    msg_p->data.arg5.addr = *helper;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    *helper = msg_p->data.arg5.addr;

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_GetHelper
* PURPOSE:
*     Check if this helper address exist.
*
* INPUT:
*
* OUTPUT:
*     
* RETURN:
*       UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL  -- error.
*       UDPHELPER_TYPE_RESULT_FAIL  -- doesn't exist.
*       UDPHELPER_TYPE_RESULT_SUCCESS  -- exist.
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetHelper(UI32_T ifindex, L_INET_AddrIp_T *helper)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(arg5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_GET_HELPER;

    msg_p->data.arg5.ifindex = ifindex;
    msg_p->data.arg5.addr = *helper;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }
    
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : UDPHELPER_PMGR_GetForwardPort
* PURPOSE:
*     Get next forward port
*
* INPUT:
*
* OUTPUT:
*     
* RETURN:
*       UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL  -- error.
*       UDPHELPER_TYPE_RESULT_FAIL  -- doesn't exist.
*       UDPHELPER_TYPE_RESULT_SUCCESS  -- exist.
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetForwardPort(UI32_T port)
{
    UDPHELPER_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = UDPHELPER_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_UDPHELPER;
    msgbuf_p->msg_size = msg_size;
    msg_p = (UDPHELPER_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = UDPHELPER_MGR_IPCCMD_GET_FORWARD_PORT;

    msg_p->data.ui32_v = port;
    if (SYSFUN_SendRequestMsg(udphelper_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL;
    }    
    return msg_p->type.result_ui32;

}


