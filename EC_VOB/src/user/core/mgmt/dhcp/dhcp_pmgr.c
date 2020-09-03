/*-----------------------------------------------------------------------------
 * FILE NAME: DHCP_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    None.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/08/02     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "dhcp_mgr.h"
#include "dhcp_pmgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


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
 * FUNCTION NAME : DHCP_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for DHCP_PMGR in the calling process.
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
BOOL_T DHCP_PMGR_InitiateProcessResources(void)
{
    /* get the ipc message queues for DHCP MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
                       SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }
    
    return TRUE;
} /* End of DHCP_PMGR_InitiateProcessResources */

/* FUNCTION NAME : DHCP_PMGR_DisableDhcp
 * PURPOSE:
 *      Disable DHCP protocol to configure interface, the interfaces should be configured
 *      by user, not by DHCP protocol
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 *		TRUE	-- Dhcp disable.
 *		FALSE	-- Previous setting is not Dhcp.
 *
 * NOTES:
 *		1. After disable DHCP, the interface MUST be configured manually.
 *		2. These functions for DHCP/BOOTP disable/enable affecting whole CSC,
 *		   not one interface. If DHCP and BOOTP are disable, even interface is
 *		   indivial configured, still not activing auto-configure-interface.
 */
BOOL_T DHCP_PMGR_DisableDhcp(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(DHCP_MGR_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_DISABLEDHCP;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    
    return TRUE;
}
UI32_T DHCP_PMGR_CheckRestartObj(UI32_T restart_object)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_CHECKRESTARTOBJ;
    msg_p->data.arg_ui32 = restart_object;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
}
/* FUNCTION NAME : DHCP_PMGR_Restart3
 * PURPOSE:
 *      The funtion provided to CLI or web to only change the state
 *		(change to provision complete) and inform DHCP task to do
 *		 the restart.
 *
 *
 * INPUT:
 *      restart_object.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES: 1. it will not immediately restart when called this funtion
 *			instead, it will activate restart in DHCP_task when get
 *			provision complete state
 *		  2. Possible Value for restart_object:
 *			DHCP_MGR_RESTART_NONE
 *			DHCP_MGR_RESTART_CLIENT
 *			DHCP_MGR_RESTART_SERVER
 *			DHCP_MGR_RESTART_RELAY
 *
 */
void    DHCP_PMGR_Restart3(UI32_T restart_object)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RESTART3;
    msg_p->data.arg_ui32 = restart_object;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }
    
    return;
} /* End of DHCP_PMGR_Restart3 */

UI32_T   DHCP_PMGR_RemoveSystemRole(UI32_T role)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_REMOVESYSTEMROLE;
    msg_p->data.arg_ui32 = role;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_Restart3 */


/* FUNCTION NAME : DHCP_PMGR_Restart
 * PURPOSE:
 *      The funtion provided to CLI or web to only change the state
 *		(change to provision complete) and inform DHCP task to do
 *		 the restart.
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES: 1. it will not immediately restart when called this funtion
 *			instead, it will activate restart in DHCP_task when get
 *			provision complete state
 *		  2. Possible Value for restart_object:
 *			DHCP_TYPE_RESTART_NONE
 *			DHCP_TYPE_RESTART_CLIENT
 *			DHCP_TYPE_RESTART_SERVER
 *			DHCP_TYPE_RESTART_RELAY
 *
 */
void    DHCP_PMGR_Restart()
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(DHCP_MGR_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RESTART;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }
    
    return;
} /* End of DHCP_PMGR_Restart */

/* FUNCTION	NAME : DHCP_PMGR_C_SetClientId
 * PURPOSE:
 *		Define associated client identifier per interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		id_mode 	-- the client-port of dhcp.
 *		id_len 		-- the len of buffer.
 *		id_buf		-- the content of the cid.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- No more space in allocating memory
 *		DHCP_MGR_CID_EXCEED_MAX_SIZE -- cid exceeds the max size
 *
 * NOTES:
 *		1. Possible values of id_mode are {DHCP_TYPE_CID_HEX | DHCP_TYPE_CID_TEXT}.
 *
 */
UI32_T DHCP_PMGR_C_SetClientId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32_cid);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_SETCLIENTID;
    msg_p->data.arg_grp_ui32_ui32_ui32_cid.arg_ui32_1 = vid_ifIndex;
    msg_p->data.arg_grp_ui32_ui32_ui32_cid.arg_ui32_2 = id_mode;
    msg_p->data.arg_grp_ui32_ui32_ui32_cid.arg_ui32_3 = id_len;
    memcpy(msg_p->data.arg_grp_ui32_ui32_ui32_cid.arg_cid, id_buf,
           sizeof(msg_p->data.arg_grp_ui32_ui32_ui32_cid.arg_cid));
           
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_SetClientId */

/* FUNCTION	NAME : DHCP_PMGR_C_GetClientId
 * PURPOSE:
 *		Retrieve the associated client identifier for specified interface from OM.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		cid_p		-- the pointer of cid structure
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- failure.
 *		DHCP_MGR_NO_SUCH_INTERFACE -- the specified vlan not existed in DHCP_OM table
 *		DHCP_MGR_NO_CID	-- this interface has no CID configuration.
 *
 * NOTES:
 *		None.
 *
 */
UI32_T DHCP_PMGR_C_GetClientId(UI32_T vid_ifIndex, DHCP_MGR_ClientId_T *cid_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_clientid);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_GETCLIENTID;
    msg_p->data.arg_grp_ui32_clientid.arg_ui32 = vid_ifIndex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    *cid_p = msg_p->data.arg_grp_ui32_clientid.arg_client_id;
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_GetClientId */

/* FUNCTION	NAME : DHCP_PMGR_C_GetRunningClientId
 * PURPOSE:
 *		Get running config for CID in WA.
 *
 * INPUT:
 *		vid_ifIndex -- the key to identify ClientId
 *
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_PMGR_C_GetRunningClientId(UI32_T vid_ifIndex, DHCP_MGR_ClientId_T *cid_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_clientid);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_GETRUNNINGCLIENTID;
    msg_p->data.arg_grp_ui32_clientid.arg_ui32 = vid_ifIndex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    
    *cid_p = msg_p->data.arg_grp_ui32_clientid.arg_client_id;
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_GetRunningClientId */

/* FUNCTION	NAME : DHCP_PMGR_C_GetNextClientId
 * PURPOSE:
 *		Get the next client identifier for specified interface from OM.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- failure.
 *		DHCP_MGR_NO_SUCH_INTERFACE -- the specified vlan not existed in DHCP_OM table
 *		DHCP_MGR_NO_CID	-- this interface has no CID configuration.
 *
 * NOTES:
 *		1. (vid_ifIndex = 0) to get 1st interface, for layer 2, get the mgmt vlan id
 *
 */
UI32_T DHCP_PMGR_C_GetNextClientId(UI32_T *vid_ifIndex, DHCP_MGR_ClientId_T *cid_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_clientid);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_GETNEXTCLIENTID;
    msg_p->data.arg_grp_ui32_clientid.arg_ui32 = *vid_ifIndex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    *vid_ifIndex = msg_p->data.arg_grp_ui32_clientid.arg_ui32;
    *cid_p = msg_p->data.arg_grp_ui32_clientid.arg_client_id;
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_GetNextClientId */

/* FUNCTION	NAME : DHCP_PMGR_C_DeleteClientId
 * PURPOSE:
 *		Delete the client identifier for specified interface. (Restart DHCP needed)
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- the specified interface has no cid config, so can't delete cid
 *
 *
 * NOTES:
 *		1. This function will set Client_id structure to empty in WA. Until restart DHCP
 *		client to update WA info into OM
 *
 */
UI32_T DHCP_PMGR_C_DeleteClientId(UI32_T vid_ifIndex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_DELETECLIENTID;
    msg_p->data.arg_ui32 = vid_ifIndex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_DeleteClientId */

/* FUNCTION	NAME : DHCP_PMGR_SetIfRole
 * PURPOSE:
 *		Define interface role in DHCP; this info. is kept in DHCP_WA.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		role		-- one of { client | server | relay }
 *						DHCP_TYPE_INTERFACE_BIND_CLIENT
 *						DHCP_TYPE_INTERFACE_BIND_RELAY
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_NO_IP -- interface does not have ip bind to it
 *		DHCP_MGR_DYNAMIC_IP -- the interface is acting as client (bootp/dhcp)
 *		DHCP_MGR_SERVER_ON -- the system is running as a dhcp server
 *		DHCP_MGR_FAIL	--  fail to set interface role.
 *
 * NOTES:
 *		1. This function is not used in layer2 switch.
 */
UI32_T DHCP_PMGR_SetIfRole(UI32_T vid_ifIndex, UI32_T role)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_SETIFROLE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = vid_ifIndex;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = role;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_SetIfRole */


/* FUNCTION	NAME : DHCP_PMGR_C_SetVendorClassId
 * PURPOSE:
 *		Define associated class identifier per interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		id_mode 	-- the class id mode.
 *		id_len 		-- the len of buffer.
 *		id_buf		-- the content of the class id.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- No more space in allocating memory
 *		DHCP_MGR_CLASSID_EXCEED_MAX_SIZE -- class id exceeds the max size
 *
 * NOTES:
 *		1. Possible values of id_mode are {DHCP_TYPE_CLASSID_HEX | DHCP_TYPE_CLASSID_TEXT}.
 *
 */
UI32_T DHCP_PMGR_C_SetVendorClassId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_vendor_class_id);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    if (id_buf == NULL)
        return DHCP_MGR_FAIL;
        
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_C_SETVENDORCLASSID;
    msg_p->data.arg_grp_vendor_class_id.vid = vid_ifIndex;
    msg_p->data.arg_grp_vendor_class_id.vendordata.vendor_mode = id_mode;
    msg_p->data.arg_grp_vendor_class_id.vendordata.vendor_len = id_len;
    memcpy(msg_p->data.arg_grp_vendor_class_id.vendordata.vendor_buf, id_buf, id_len);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_SetVendorClassId */

/* FUNCTION	NAME : DHCP_PMGR_C_DeleteVendorClassId
 * PURPOSE:
 *		Delete the client identifier for specified interface, so that the
 *      vendor class option60 shall not be added into packet.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- the specified interface has no cid config, so can't delete class id
 *
 *
 * NOTES:
 *		1. This function will set class_id structure to empty in WA (Working Area).
 *
 */
UI32_T DHCP_PMGR_C_DeleteVendorClassId(UI32_T vid_ifIndex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_vendor_class_id);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_C_DELETEVENDORCLASSID;
    msg_p->data.arg_grp_vendor_class_id.vid = vid_ifIndex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_DeleteVendorClassId */

/* FUNCTION	NAME : DHCP_PMGR_C_GetVendorClassId
 * PURPOSE:
 *		Retrieve the associated class identifier for specified interface from OM.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *		class_id_p	-- the pointer of class id structure
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- failure.
 *		DHCP_MGR_NO_SUCH_INTERFACE -- the specified vlan not existed in DHCP_OM table
 *		DHCP_MGR_NO_CLASSID	-- this interface has no CLASSID configuration.
 *
 * NOTES:
 *		None.
 *
 */
UI32_T DHCP_PMGR_C_GetVendorClassId(UI32_T vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_vendor_class_id);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    if (class_id_p == NULL)
        return DHCP_MGR_FAIL;
        
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_C_GETVENDORCLASSID;
    msg_p->data.arg_grp_vendor_class_id.vid = vid_ifIndex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    memcpy(class_id_p, &msg_p->data.arg_grp_vendor_class_id.vendordata, sizeof(DHCP_MGR_Vendor_T));
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_GetVendorClassId */

/* FUNCTION	NAME : DHCP_PMGR_C_GetRunningVendorClassId
 * PURPOSE:
 *		Get running config for CLASS ID in WA (Working Area).
 *
 * INPUT:
 *		vid_ifIndex -- the key to identify Vendor Class Id
 *
 *
 * OUTPUT:
 *		class_id_p	-- the pointer of class id structure
 *
 * RETURN:
 *		SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_C_GetRunningVendorClassId(UI32_T vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_vendor_class_id);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    if (class_id_p == NULL)
        return DHCP_MGR_FAIL;
        
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_C_GETRUNNINGVENDORCLASSID;
    msg_p->data.arg_grp_vendor_class_id.vid = vid_ifIndex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    memcpy(class_id_p, &msg_p->data.arg_grp_vendor_class_id.vendordata, sizeof(DHCP_MGR_Vendor_T));
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_GetVendorClassId */

/* FUNCTION	NAME : DHCP_PMGR_C_GetNextVendorClassId
 * PURPOSE:
 *		Get the next class id for specified interface.
 *
 * INPUT:
 *		vid_ifIndex -- the current interface.
 *
 *
 * OUTPUT:
 *		vid_ifIndex --  the next active vlan interface.
 *      class_id_p	-- the pointer of class id structure
 *
 * RETURN:
 *		DHCP_MGR_OK
 *		DHCP_MGR_FAIL
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_C_GetNextVendorClassId(UI32_T *vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_vendor_class_id);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    if (class_id_p == NULL)
        return DHCP_MGR_FAIL;
        
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    
    msg_p->type.cmd = DHCP_MGR_IPC_C_GETNEXTVENDORCLASSID;
    msg_p->data.arg_grp_vendor_class_id.vid = *vid_ifIndex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    *vid_ifIndex = msg_p->data.arg_grp_vendor_class_id.vid;
    memcpy(class_id_p, &msg_p->data.arg_grp_vendor_class_id.vendordata, sizeof(DHCP_MGR_Vendor_T));
    
    return msg_p->type.ret_ui32;
} /* End of DHCP_PMGR_C_GetVendorClassId */


/* FUNCTION	NAME : DHCP_PMGR_ReleaseClientLease
 * PURPOSE:
 *		Release dhcp active client lease
 *
 * INPUT:
 *		ifindex         --  vlan interface index
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		DHCP_MGR_OK	    --  successfully.
 *		DHCP_MGR_FAIL	--  fail.
 *
 *
 * NOTES:
 *		this api will free dhcp engine's active lease and send DHCPRELEASE packet to dhcp server.
 *
 */
UI32_T DHCP_PMGR_ReleaseClientLease(UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_C_RELEASE_CLIENT_LEASE;
    msg_p->data.arg_ui32 = ifindex;
    
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
}

UI32_T DHCP_PMGR_SetRestartObject(UI32_T restart_obj)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_SETRESTARTOBJECT;
    msg_p->data.arg_ui32 = restart_obj;
    
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}
UI32_T DHCP_PMGR_DeleteIfRole(UI32_T ifindex, UI32_T role)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_DELETEIFROLE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = ifindex;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = role;
    
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}



#if (SYS_CPNT_DHCP_SERVER == TRUE)

/* FUNCTION NAME : DHCP_PMGR_EnterPool
 * PURPOSE: This function checks the specified pool's existence. If not existed,
 *          created the pool with the specified pool_name; if existed, do nothing
 *          just return.
 *
 * INPUT:
 *      pool_name -- specify the pool name to be entered.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_EXCEED_MAX_POOL_NAME -- exceed max size of pool name
 *      DHCP_MGR_FAIL -- Fail in allocating memory for pool creation
 *      DHCP_MGR_OK
 *
 * NOTES:
 *      1. Check pool name exceeds the max pool name length.
 *      2. Check pool existence. If not, create one with pool name.
 *
 */
UI32_T DHCP_PMGR_EnterPool(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_ENTERDHCPPOOL;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DeletePool
 * PURPOSE:
 *      To delete pool config entry
 *
 * INPUT:
 *      pool_name -- the name of the pool which will be deleted.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to delete pool.
 *
 * NOTES:
 *      1. key is pool_name -- must be specified in order to retrieve
 *
 */
UI32_T DHCP_PMGR_DeletePool(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELDHCPPOOL;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetExcludedIp
 * PURPOSE:
 *      To specify the excluded ip address (range).
 *
 * INPUT:
 *      low_address  -- the lowest ip in the excluded ip address range
 *      high_address -- the highest ip in the excluded ip address range
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to set excluded ip address.
 *      DHCP_MGR_IS_NETWORK_IP_OR_NETWORK_BROADCAST -- fail to set excluded ip as network address
 *                                                      or a network broadcast
 *      DHCP_MGR_EXCLUDE_ALL_IP_IN_SUBNET -- fail to set excluded ip range as (x.x.x.1 ~ x.x.x.254)
 *      DHCP_MGR_HIGH_IP_SMALLER_THAN_LOW_IP    -- fail to set excluded ip range that high ip smaller
 *                                              than low ip
 *      DHCP_MGR_EXCLUDED_IP_IN_OTHER_SUBNET    -- fail to set excluded ip in other subnet
 *
 * NOTES:
 *      1. To specify exluded ip address range, the high-address is required.
 *      Otherwise, only specify low-address is treated as excluding an address
 *      from available address range.
 *      2. (low_address != 0, high_address == 0) will treat low_address the only
 *      address to be excluded.
 */
UI32_T DHCP_PMGR_SetExcludedIp(UI32_T low_addr, UI32_T high_addr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETEXCLUDEDIP;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = low_addr;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = high_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelExcludedIp
 * PURPOSE:
 *      This function deletes the IP address(es) that the DHCP server
 *      should not assign to DHCP clients.
 *
 * INPUT:
 *      low_address  -- the lowest ip address of the excluded ip range
 *      high_address -- the highest ip address of the excluded ip range
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK -- successfully delete the specified excluded ip.
 *      DHCP_MGR_FAIL -- FAIL to delete the specified excluded ip.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelExcludedIp(UI32_T low_addr, UI32_T high_addr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELEXCLUDEDIP;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = low_addr;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = high_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextExcludedIp
 * PURPOSE:
 *      This function gets the next excluded IP address(es).
 *
 * INPUT:
 *      low_address  -- the lowest ip address of the excluded ip range
 *      high_address -- the highest ip address of the excluded ip range
 *
 * OUTPUT:
 *      low_address  -- the lowest ip address of the excluded ip range
 *      high_address -- the highest ip address of the excluded ip range
 *
 * RETURN:
 *      DHCP_MGR_OK -- successfully get the specified excluded ip.
 *      DHCP_MGR_FAIL -- FAIL to get the specified excluded ip.
 *
 * NOTES:
 *      1. low_address = 0 to get the 1st excluded IP entry.
 *      2. if return high_address = 0 which means there is no high address; this
 *          exclusion is only exclude an IP (low_address).
 */

UI32_T DHCP_PMGR_GetNextExcludedIp(UI32_T *low_addr, UI32_T *high_addr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTDHCPSERVEREXCLUDEDIPTABLE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = *low_addr;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = *high_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    *low_addr = msg_p->data.arg_grp_ui32_ui32.arg_ui32_1;
    *high_addr = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetNetworkToPoolConfigEntry
 * PURPOSE:
 *      Set network address to pool config entry (set by field) and link it to
 *      network sorted link list.
 *
 * INPUT:
 *      pool_name           -- pool name
 *      network_address     -- network ip address
 *      sub_netmask         -- subnet mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK                         -- successfully.
 *      DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS  -- pool has configured to host pool.
 *      DHCP_MGR_FAIL                       -- fail to set network to pool config
 *      DHCP_MGR_NO_SUCH_POOL_NAME          -- pool not existed.
 *      DHCP_MGR_INVALID_IP                 -- the specified network address is invalid.
 *
 * NOTES:
 *      1. Network address can't be specified to '0x0'
 *      2. If sub_netmask is 0x0 or user not specified, and also pool->sub_netmask
 *          is 0x0, we use default subnet mask = 255.255.255.0
 *      3. If sub_netmask is 0x0, but pool->sub_netmask is not 0x0, we still use
 *           pool->sub_netmask as subnet mask
 *      4. Add constraint checking for network pool:
 *          a). total numbers of network pool (SYS_ADPT_MAX_NBR_OF_DHCP_NETWORK_POOL)
 *          b). subnet mask checking for class C issue
 */
UI32_T DHCP_PMGR_SetNetworkToPoolConfigEntry(char *pool_name, UI32_T ipaddr, UI32_T mask)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETNETWORKTOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = ipaddr;
    msg_p->data.arg_grp_pool_config.arg_ui32_2 = mask;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelNetworkFromPoolConfigEntry
 * PURPOSE:
 *      Delete subnet number and mask from pool.
 *
 * INPUT:
 *      pool_name           -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to set network to pool config
 *      DHCP_MGR_NO_SUCH_POOL_NAME -- pool name not existed.
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelNetworkFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELNETWORKFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetMacToPoolConfigEntry
 * PURPOSE:
 *      Set hardware address to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *      mac       -- hardware address
 *      type      -- hardware address type  (DHCP_TYPE_HTYPE_ETHER / DHCP_TYPE_HTYPE_IEEE802)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK                 -- successfully.
 *      DHCP_MGR_FAIL               -- fail to set pool config entry to WA.
 *      DHCP_MGR_NO_SUCH_POOL_NAME  -- Can't find any pool by specified pool_name
 *      DHCP_MGR_MAC_OVERSIZE       -- MAC address len larger than requirment.
 *
 * NOTES:
 *      1. For type, if user types 'ethernet', type = DHCP_MGR_HTYPE_ETHER;
 *          if user types 'ieee802', type = DHCP_MGR_HTYPE_IEEE802. If user
 *          doesn't type anything, by default, type = DHCP_MGR_HTYPE_ETHER
 *
 */
UI32_T DHCP_PMGR_SetMacToPoolConfigEntry(char *pool_name, UI8_T *hw_addr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_phy_addr);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETMACTOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config_phy_addr.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    memcpy(msg_p->data.arg_grp_pool_config_phy_addr.phy_addr, hw_addr, SYS_ADPT_MAC_ADDR_LEN);
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelMacFromPoolConfigEntry
 * PURPOSE:
 *      Delete hardware address from pool config entry.
 *
 * INPUT:
 *      pool_name -- pool name.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to delete pool config entry to WA.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- Can't find any pool by specified pool_name
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelMacFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELMACFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetCidToPoolConfigEntry
 * PURPOSE:
 *      Set client identifier to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		cid_mode  -- cid mode (text format or hex format)
 *		cid_len	  -- length of cid
 *		buf		  -- buffer that contains cid
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully
 *		DHCP_MGR_FAIL				-- fail to set Cid to pool config
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- cid_len exceeds max size defined in leaf_es3626a.h
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed
 *
 * NOTES:
 *		1. possible values of mode {DHCP_MGR_CID_HEX | DHCP_MGR_CID_TEXT}
 */
UI32_T DHCP_PMGR_SetCidToPoolConfigEntry(char *pool_name, UI32_T cid_mode, UI32_T cid_len, char *cid_buf)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_cid_pool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETCLIENTIDENTIFIERTOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_ui32_ui32_cid_pool.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_ui32_ui32_cid_pool.arg_ui32_1 = cid_mode;
    msg_p->data.arg_grp_ui32_ui32_cid_pool.arg_ui32_2 = cid_len;
    memcpy(msg_p->data.arg_grp_ui32_ui32_cid_pool.arg_cid, cid_buf, cid_len);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelCidToPoolConfigEntry
 * PURPOSE:
 *      Delete client identifier from pool config entry
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set Cid to pool config
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelCidToPoolConfigEntry(char *pool_name)
{

    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELCLIENTIDENTIFIERFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDomainNameToPoolConfigEntry
 * PURPOSE:
 *      Set domain name to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name       -- pool name
 *      domain_name     -- domain name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK                 -- successfully.
 *      DHCP_MGR_FAIL               -- fail to set domain name
 *      DHCP_MGR_EXCEED_MAX_SIZE    -- the length of domain name exceeds MAX size.
 *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not exist.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_SetDomainNameToPoolConfigEntry(char *pool_name, char *domain_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_domain_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDOMAINNAMETOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_domain_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    strncpy(msg_p->data.arg_grp_pool_domain_name.domain_name, domain_name,SYS_ADPT_DHCP_MAX_DOMAIN_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelDomainNameFromPoolConfigEntry
 * PURPOSE:
 *      Delete domain name from pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set domain name
 *		DHCP_MGR_NO_SUCH_POOL_NAME -- pool not exist
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelDomainNameFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELDOMAINNAMEFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDnsServerToPoolConfigEntry
 * PURPOSE:
 *      Set domain name server ip to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		ip_array  -- array that contains DNS server ip address(es)
 *		size	  -- number of array elements
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set domain name
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- exceed max number of DNS server
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 2.
 */
UI32_T DHCP_PMGR_SetDnsServerToPoolConfigEntry(char *pool_name, UI32_T ip_array[], UI32_T size)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_iparray);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDNSSERVERTOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config_iparray.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    memcpy(msg_p->data.arg_grp_pool_config_iparray.ip_array.dns_server, ip_array,
           SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER*sizeof(UI32_T));
    msg_p->data.arg_grp_pool_config_iparray.size = size;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelDnsServerFromPoolConfigEntry
 * PURPOSE:
 *      Delete domain name server ip from pool config entry
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
  *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_FAIL	-- fail to remove dns server address(es)
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 8.
 */
UI32_T DHCP_PMGR_DelDnsServerFromPoolConfigEntry(char*pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELDNSSERVERFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetLeaseTimeToPoolConfigEntry
 * PURPOSE:
 *      Set lease time to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		day 	  -- day (0-365)
 *		hour 	  -- hour (0-23)
 *		minute 	  -- minute (0-59)
 *		is_infinite  -- infinite lease time (TRUE / FALSE)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set domain name
 *		DHCP_MGR_INVALID_ARGUMENT 	-- argument is over boundary
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_SetLeaseTimeToPoolConfigEntry(char *pool_name , UI32_T lease_time, UI32_T infinite)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETLEASETIMETOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = lease_time;
    msg_p->data.arg_grp_pool_config.arg_ui32_2 = infinite;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelLeaseTimeFromPoolConfigEntry
 * PURPOSE:
 *      Delete lease time from pool config entry and restore default lease time.
 *
 * INPUT:
 *      pool_name -- pool name
 *
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to delete
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelLeaseTimeFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELLEASETIMEFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDfltRouterToPoolConfigEntry
 * PURPOSE:
 *      Set default router address to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		ip_array  -- array that contains DNS server ip address(es)
 *		size	  -- number of array elements
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set domain name
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- exceed max number of dflt router
 *
 * NOTES:
 *		1. The max numbers of dflt router are 2.
 */
UI32_T DHCP_PMGR_SetDfltRouterToPoolConfigEntry(char *pool_name, UI32_T ip_array[], UI32_T size)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_iparray);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDEFAULTROUTETOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config_iparray.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    memcpy(msg_p->data.arg_grp_pool_config_iparray.ip_array.default_router, ip_array,
           SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER*sizeof(UI32_T));
    msg_p->data.arg_grp_pool_config_iparray.size = size;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelDfltRouterFromPoolConfigEntry
 * PURPOSE:
 *      Delete default router from pool config entry.
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_FAIL	-- fail to remove dflt router address(es)
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelDfltRouterFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELDEFAULTROUTEFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}


/* FUNCTION NAME : DHCP_PMGR_SetBootfileToPoolConfigEntry
 * PURPOSE:
 *      Set bootfile name to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		bootfile  -- bootfile name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set bootfile name
 *		DHCP_MGR_EXCEED_MAX_SIZE 	-- the length of bootfile name exceeds MAX size.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not exist.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_SetBootfileToPoolConfigEntry(char *pool_name, char *boot_file)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_boot_file);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETBOOTFILETOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_boot_file.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    strncpy(msg_p->data.arg_grp_pool_boot_file.bootfile, boot_file,SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelBootfileFromPoolConfigEntry
 * PURPOSE:
 *      Delete bootfile name from pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to remove bootfile name
 *		DHCP_MGR_NO_SUCH_POOL_NAME -- pool not exist
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelBootfileFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELBOOTFILEFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetNetbiosNameServerToPoolConfigEntry
 * PURPOSE:
 *      Set netbios name server ip to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		ip_array  -- array that contains Netbios Name Server ip address(es)
 *		size	  -- number of array elements
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set netbios name server ip
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- exceed max number of netbios name server
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 5.
 */
UI32_T DHCP_PMGR_SetNetbiosNameServerToPoolConfigEntry(char *pool_name, UI32_T ip_array[], UI32_T size)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_iparray);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETNETBIOSNAMESERVERTOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config_iparray.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    memcpy(msg_p->data.arg_grp_pool_config_iparray.ip_array.netbios_name_server, ip_array,
           SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER*sizeof(UI32_T));
    msg_p->data.arg_grp_pool_config_iparray.size = size;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_MGR_DelNetbiosNameServerFromPoolConfigEntry
 * PURPOSE:
 *      Delete netbios name server ip from pool config entry
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_FAIL	-- fail to remove netbios name server ip address(es)
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 8.
 */
UI32_T DHCP_PMGR_DelNetbiosNameServerFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELNETBIOSNAMESERVERFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetNetbiosNodeTypeToPoolConfigEntry
 * PURPOSE:
 *      Set netbios node type to pool config entry (set by field).
 *
 * INPUT:
 *      pool_name -- pool name
 *      node_type -- netbios node type
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to set netbios node type
 *      DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *
 * NOTES:
 *      1. possible value of node_type:
 *      DHCP_MGR_NETBIOS_NODE_TYPE_B_NODE -- broadcast
 *      DHCP_MGR_NETBIOS_NODE_TYPE_P_NODE -- peer to peer
 *      DHCP_MGR_NETBIOS_NODE_TYPE_M_NODE -- mixed
 *      DHCP_MGR_NETBIOS_NODE_TYPE_H_NODE -- hybrid (recommended)
 */
UI32_T DHCP_PMGR_SetNetbiosNodeTypeToPoolConfigEntry(char *pool_name, UI32_T type)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETNETBIOSNODETYPETOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelNetbiosNodeTypeFromPoolConfigEntry
 * PURPOSE:
 *      Delete netbios node type from pool config entry and restore default lease time.
 *
 * INPUT:
 *      pool_name -- pool name
 *
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to delete
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelNetbiosNodeTypeFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELNETBIOSNODETYPEFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetNextServerToPoolConfigEntry
 * PURPOSE:
 *      Set next server ip to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		ip_array  -- array that contains next server ip address(es)
 *		size	  -- number of array elements
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set next server
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- exceed max number of next server
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 5.
 */
UI32_T DHCP_PMGR_SetNextServerToPoolConfigEntry(char *pool_name, UI32_T ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETNEXTSERVERTOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = ipaddr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelNextServerFromPoolConfigEntry
 * PURPOSE:
 *      Delete next server ip from pool config entry
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_FAIL	-- fail to remove next server address(es)
 *
 * NOTES:
 *		1. The max numbers of next server ip are 8.
 */
UI32_T DHCP_PMGR_DelNextServerFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELNEXTSERVERFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetHostToPoolConfigEntry
 * PURPOSE:
 *      Set host address to pool config entry (set by field).
 *
 * INPUT:
 *      pool_name 			-- pool name
 *		network_address	  	-- host ip address
 *		sub_netmask	  		-- subnet mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK							-- successfully.
 *		DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS 	-- pool has configured to network pool.
 *		DHCP_MGR_FAIL						-- fail to set host to pool config
 *		DHCP_MGR_NO_SUCH_POOL_NAME 			-- pool not existed.
 *		DHCP_MGR_INVALID_IP 				-- the specified host address is invalid.
 *
 * NOTES:
 *		1. host address can't be specified to '0x0'
 *		2. If sub_netmask is 0x0 or user not specified, and also pool->sub_netmask
 *			is 0x0, we take natural subnet mask (class A, B or C).
 *		3. If sub_netmask is 0x0, but pool->sub_netmask is not 0x0, we still use
 *			 pool->sub_netmask as subnet mask
 */
UI32_T DHCP_PMGR_SetHostToPoolConfigEntry(char *pool_name, UI32_T ipaddr, UI32_T mask)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETHOSTTOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = ipaddr;
    msg_p->data.arg_grp_pool_config.arg_ui32_2 = mask;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DelHostFromPoolConfigEntry
 * PURPOSE:
 *      Delete host address and mask from pool.
 *
 * INPUT:
 *      pool_name 			-- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to remove host to pool config
 *		DHCP_MGR_NO_SUCH_POOL_NAME -- pool not existed
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_DelHostFromPoolConfigEntry(char *pool_name)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_DELHOSTFROMPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_name.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetIpBinding
 * PURPOSE:
 *      Get the specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for corresponding binding information
 *
 * OUTPUT:
 *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
 *					  others: the pointer to the ip_binding lease in memory.c
 *
 *
 * RETURN:
 *		DHCP_MGR_OK		--  successfully.
 *		DHCP_MGR_FAIL	--  FAIL to get ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_GetIpBinding(UI32_T ipaddr, DHCP_MGR_Server_Lease_Config_T *lease_entry)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_lease_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETIPBINDING;
    msg_p->data.arg_grp_lease_entry.ipaddr = ipaddr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    memcpy(lease_entry, &msg_p->data.arg_grp_lease_entry.lease_entry, sizeof(DHCP_MGR_Server_Lease_Config_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetActiveIpBinding
 * PURPOSE:
 *      Get the active specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for corresponding binding information
 *
 * OUTPUT:
 *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
 *                    others: the pointer to the ip_binding lease in memory.c
 *
 *
 * RETURN:
 *      DHCP_MGR_OK     --  successfully.
 *      DHCP_MGR_FAIL   --  FAIL to get ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_GetActiveIpBinding(UI32_T ip_address, DHCP_MGR_Server_Lease_Config_T *ip_binding)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_lease_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETACTIVEIPBINDING;
    msg_p->data.arg_grp_lease_entry.ipaddr = ip_address;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    memcpy(ip_binding, &msg_p->data.arg_grp_lease_entry.lease_entry, sizeof(DHCP_MGR_Server_Lease_Config_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextIpBinding
 * PURPOSE:
 *      Get the next specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for corresponding binding information
 *
 * OUTPUT:
 *     	ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
 *					  others: the pointer to the next ip_binding lease
 *						in memory.c
 *
 * RETURN:
 *
 *		DHCP_MGR_OK		--  successfully.
 *		DHCP_MGR_FAIL	--  FAIL to get next ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_GetNextIpBinding(UI32_T ipaddr, DHCP_MGR_Server_Lease_Config_T *lease_entry)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_lease_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTIPBINDING;
    msg_p->data.arg_grp_lease_entry.ipaddr = ipaddr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    memcpy(lease_entry, &msg_p->data.arg_grp_lease_entry.lease_entry, sizeof(DHCP_MGR_Server_Lease_Config_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextActiveIpBinding
 * PURPOSE:
 *      Get the next active specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for corresponding binding information
 *
 * OUTPUT:
 *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
 *                    others: the pointer to the next ip_binding lease
 *                      in memory.c
 *
 * RETURN:
 *
 *      DHCP_MGR_OK     --  successfully.
 *      DHCP_MGR_FAIL   --  FAIL to get next ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_GetNextActiveIpBinding(UI32_T ipaddr, DHCP_MGR_Server_Lease_Config_T *lease_entry)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_lease_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTACTIVEIPBINDING;
    msg_p->data.arg_grp_lease_entry.ipaddr = ipaddr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    memcpy(lease_entry, &msg_p->data.arg_grp_lease_entry.lease_entry, sizeof(DHCP_MGR_Server_Lease_Config_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_ClearIpBinding
 * PURPOSE:
 *      Clear the specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for marking available again in pool
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK -- successfully.
 *      DHCP_MGR_FAIL   --  FAIL to clear ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_ClearIpBinding(UI32_T ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_CLEARIPBINDING;
    msg_p->data.arg_ui32 = ipaddr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_ClearAllIpBinding
 * PURPOSE:
 *      Clear all ip bindings in server_om (memory.c)
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to clear ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_ClearAllIpBinding(void)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_CLEARALLIPBINDING;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextPoolConfig
 * PURPOSE:
 *      Get next config for pool in WA.
 *
 * INPUT:
 *      pool_config -- the pointer to the pool config structure
 *                  -- NULL to get the 1st pool config
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK -- successfully.
 *      DHCP_MGR_FAIL   --  FAIL to get.
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_PMGR_GetNextPoolConfig(DHCP_TYPE_PoolConfigEntry_T *pool_config_p)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTPOOLCONFIG;
    memcpy(&(msg_p->data.arg_grp_pool_config_entry.pool_config), pool_config_p, sizeof(DHCP_TYPE_PoolConfigEntry_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    memcpy(pool_config_p, &(msg_p->data.arg_grp_pool_config_entry.pool_config), sizeof(DHCP_TYPE_PoolConfigEntry_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION	NAME : DHCP_PMGR_GetNextActivePool
 * PURPOSE:
 *		Get next active pool for WA. Search key is pool_name + pool_range.low_address
 *
 * INPUT:
 *		pool_name -- the pointer to the pool name
 *                        -- get 1st pool if pool_name is null string
 *              pool_range_p --  pool_range to get next range. key is pool_range.low_address.
 *                           --  get 1st range if pool_range.low_address is 0
 * OUTPUT:
 *		pool_range_p.
 *      pool_name
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to get.
 *              DHCP_MGR_INVALID_ARGUMENT
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_PMGR_GetNextActivePool(char* pool_name, DHCP_MGR_ActivePoolRange_T *pool_range_p)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_active_pool_range);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTACTIVEPOOL;
    strncpy(msg_p->data.arg_active_pool_range.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    memcpy(&(msg_p->data.arg_active_pool_range.pool_range), pool_range_p, sizeof(DHCP_MGR_ActivePoolRange_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    memcpy(pool_range_p, &(msg_p->data.arg_active_pool_range.pool_range), sizeof(DHCP_MGR_ActivePoolRange_T));
    strncpy(pool_name, msg_p->data.arg_active_pool_range.pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetDhcpServerServiceStatus
 * PURPOSE:
 *      Get DHCP Server Service Status. (Enable / Disable)
 *
 * INPUT:
 *      None.
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- Indicate DHCP server is enabled.
 *      FALSE -- Indicate DHCP server is disabled.
 *
 * NOTES:
 *      This API is for WEB to get Currently system running status for DHCP Server.
 *
 */
BOOL_T  DHCP_PMGR_GetDhcpServerServiceStatus(void)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(DHCP_MGR_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = DHCP_MGR_IPCMSG_TYPE_SIZE;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_GETDHCPSERVERSERVICESTATUS;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_bool;
}

/* FUNCTION NAME : DHCP_PMGR_GetDhcpPoolTable
 * PURPOSE:
 *      Get dhcp pool to WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
  *		pool_table_p -- the pointer to dhcp pool table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to get dhcp pool table
 *
 * NOTES:
 */
UI32_T DHCP_PMGR_GetDhcpPoolTable(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_config)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETDHCPPOOLTABLE;
    strncpy(msg_p->data.arg_grp_pool_config_entry.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    memcpy(pool_config, &msg_p->data.arg_grp_pool_config_entry.pool_config, sizeof(DHCP_TYPE_PoolConfigEntry_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextDhcpPoolTable
 * PURPOSE:
 *      Get next dhcp pool to WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
 *		pool_table_p -- the pointer to dhcp pool table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to get next dhcp pool table.
 *
 * NOTES:
 *	1. *pool_name = 0 to get the 1st pool and returning pool_name in *pool_name.
 */
UI32_T DHCP_PMGR_GetNextDhcpPoolTable(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_config)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTDHCPPOOLTABLE;
    strncpy(msg_p->data.arg_grp_pool_config_entry.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }

    strncpy(pool_name, msg_p->data.arg_grp_pool_config_entry.pool_config.pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    memcpy(pool_config, &msg_p->data.arg_grp_pool_config_entry.pool_config, sizeof(DHCP_TYPE_PoolConfigEntry_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDhcpPoolPoolType
 * PURPOSE:
 *      Set pool type to the pool. (set by field)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
 *		pool_type		-- the type of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- pool not existed.
 *		DHCP_MGR_FAIL				-- fail to set the pool type.
 *
 * NOTES:
 *		1. If pool has been configured to other pool_type, user must delete the original
 *			pool before setting new pool type.
 */
UI32_T DHCP_PMGR_SetDhcpPoolPoolType(char *pool_name, UI32_T pool_type)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDHCPPOOLPOOLTYPE;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = pool_type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDhcpPoolPoolAddress
 * PURPOSE:
 *      Set pool address to the pool. (set by field)
 *
 * INPUT:
 *      pool_name       -- specify the pool name of the table.
 *      pool_address    -- the address of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK                         -- successfully.
 *      DHCP_MGR_NO_SUCH_POOL_NAME          -- pool not existed.
 *      DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS  -- fail to set pool address due to user not yet
 *                                              specified pool type.
 *      DHCP_MGR_INVALID_IP                 -- the specified network address is invalid.
 *      DHCP_MGR_FAIL                       --  fail to set the pool address.
 *
 * NOTES:
 *      1. User needs to specified pool type first before specified pool address.
 */
UI32_T DHCP_PMGR_SetDhcpPoolPoolAddress(char *pool_name, UI32_T ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDHCPPOOLADDR;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = ipaddr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDhcpPoolStatus
 * PURPOSE:
 *      Set pool status to the pool. (set by field)
 *
 * INPUT:
 *		pool_name 	-- specify the pool name of the table.
 *		pool_status	-- the status of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- pool not existed.
 *		DHCP_MGR_FAIL				-- fail to set the pool status.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_SetDhcpPoolStatus(char *pool_name, UI32_T status)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDHCPPOOLSTATUS;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = status;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetDhcpPoolOptionTable
 * PURPOSE:
 *      Get dhcp option table from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
  *		pool_option_p 	-- the pointer to dhcp option table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- pool not existed.
 *		DHCP_MGR_FAIL				-- fail to get dhcp option table
 *
 * NOTES:
 */
UI32_T DHCP_PMGR_GetDhcpPoolOptionTable(char *pool_name, DHCP_TYPE_ServerOptions_T *pool_option)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_option);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETDHCPPOOLOPTIONTABLE;
    strncpy(msg_p->data.arg_grp_pool_option.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    memcpy(pool_option, &msg_p->data.arg_grp_pool_option.pool_option, sizeof(DHCP_TYPE_ServerOptions_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextDhcpPoolOptionTable
 * PURPOSE:
 *      Get next dhcp option table from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
 *		pool_option_p 	-- the pointer to dhcp option table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to get dhcp option table
 *
 * NOTES:
 *	1. *pool_name = NULL to get the 1st pool option table and return 1st pool name in
 *		*pool_name.
 */
UI32_T DHCP_PMGR_GetNextDhcpPoolOptionTable(char *pool_name, DHCP_TYPE_ServerOptions_T *pool_option)
{

    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_option);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTDHCPPOOLOPTIONTABLE;
    strncpy(msg_p->data.arg_grp_pool_option.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    strncpy(pool_name, msg_p->data.arg_grp_pool_option.pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    memcpy(pool_option, &msg_p->data.arg_grp_pool_option.pool_option, sizeof(DHCP_TYPE_ServerOptions_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDhcpPoolOptionDnsSerIpAddress
 * PURPOSE:
 *      Set the dns server ip address.
 *
 * INPUT:
 *      pool_name   -- specify the pool name of the table
 *      index       -- dns server index
 *      ip_addr     -- dns server ip
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     --  successfully.
 *      DHCP_MGR_FAIL   --  fail to set the value
 *
 * NOTES:
 *  1. 'index' is not array's index; instead, it is the index using in SNMP to
 *      specify either the 1st dns server address or the second one.
 *  2. If index == 1, and ip_addr[2] != 0 and ip_addr[1] == 0, this configuration will
 *      clear out all ip setting.
 */
UI32_T DHCP_PMGR_SetDhcpPoolOptionDnsSerIpAddress(char *pool_name,
        UI32_T index,
        UI32_T ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDHCPPOOLOPTDNSSERIP;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = index;
    msg_p->data.arg_grp_pool_config.arg_ui32_2 = ipaddr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetDhcpPoolOptionDnsSerTable
 * PURPOSE:
 *      Get dns server of dhcp pool from WA.
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table
 *		index			-- the index of the address
 *		dns_server_p	-- the dns server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to get the value.
 *
 * NOTES:
 */
UI32_T DHCP_PMGR_GetDhcpPoolOptionDnsSerTable(char *pool_name, UI32_T index, UI32_T *ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETDHCPPOOLDNSSERVERTABLE;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = index;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    *ipaddr = msg_p->data.arg_grp_pool_config.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextDhcpPoolOptionDnsSerTable
 * PURPOSE:
 *      Get next dns server table of dhcp pool from WA.
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table
 *		index_p			-- the index of the address
 *		dns_server_p 	-- the array to store dns server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to get the value.
 *
 * NOTES:
 *
 *	1.  			---------> Get next
 * pool_name|        address index
 *         		[1]    	|    [2]     |
 *     		------------+------------+
 * pool1   	 10.1.1.1	| 10.1.1.2	 |
 *     		------------+------------+
 * pool2   	 10.1.2.1	| 10.1.2.2	 |
 *     		------------+------------+
 *	2. getNext(NULL, 0, &ip_address) will get the 1st pool and 1st index address, in above case
 *		ip_address = 10.1.1.1.
 *	3. getNext(pool1, 2, &ip_address), the result ip_address = 10.1.2.1
 */
UI32_T DHCP_PMGR_GetNextDhcpPoolOptionDnsSerTable(char *pool_name, UI32_T *index, UI32_T *ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTDHCPPOOLDNSSERVERTABLE;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = *index;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    strncpy(pool_name, msg_p->data.arg_grp_pool_config.pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    *index = msg_p->data.arg_grp_pool_config.arg_ui32_1;
    *ipaddr = msg_p->data.arg_grp_pool_config.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDhcpPoolOptDefaultRouterIpAddress
 * PURPOSE:
 *      Set the dflt router ip address.
 *
 * INPUT:
 *		pool_name 	-- specify the pool name of the table
 *		index		-- dflt router address index
 *		ip_addr	 	-- dflt router ip
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		--  successfully.
 *		DHCP_MGR_FAIL	--  fail to set the value
 *
 * NOTES:
 *	1. 'index' is not array's index; instead, it is the index using in SNMP to
 *		specify either the 1st dflt router address or the second one.
 */
UI32_T DHCP_PMGR_SetDhcpPoolOptDefaultRouterIpAddress(char *pool_name,
        UI32_T index,
        UI32_T ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDHCPPOOLOPTDEFAULTROUTERIP;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = index;
    msg_p->data.arg_grp_pool_config.arg_ui32_2 = ipaddr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDhcpPoolOptNetbiosServerIpAddress
 * PURPOSE:
 *      Set the netbios server ip address.
 *
 * INPUT:
 *		pool_name 	-- specify the pool name of the table
 *		index		-- netbios server index
 *		ip_addr	 	-- netbios server ip
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		--  successfully.
 *		DHCP_MGR_FAIL	--  fail to set the value
 *
 * NOTES:
 *	1. 'index' is not array's index; instead, it is the index using in SNMP to
 *		specify either the 1st netbios server address or the second one.
 */
UI32_T DHCP_PMGR_SetDhcpPoolOptNetbiosServerIpAddress(char *pool_name,
        UI32_T index,
        UI32_T ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDHCPPOOLOPTNETBIOSSERVERIP;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = index;
    msg_p->data.arg_grp_pool_config.arg_ui32_2 = ipaddr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDhcpPoolOptionCidBuffer
 * PURPOSE:
 *      Set client identifier in the buffer to the option table from WA.
 *
 * INPUT:
 *      pool_name   -- specify the pool name of the table.
 *      cid_buf     -- cid buffer for this pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     --  successfully.
 *      DHCP_MGR_FAIL   --  fail to set the value
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_SetDhcpPoolOptionCidBuffer(char *pool_name, char *cid_buffer)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_cid_pool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDHCPPOOLOPTCIDBUF;
    strncpy(msg_p->data.arg_grp_ui32_ui32_cid_pool.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    memcpy(msg_p->data.arg_grp_ui32_ui32_cid_pool.arg_cid, cid_buffer, MAXSIZE_dhcpcIfClientId);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDhcpPoolOptionCidMode
 * PURPOSE:
 *      Set client identifier mode to the option table from WA.
 *
 * INPUT:
 *      pool_name   -- specify the pool name of the table.
 *      cid_mode    -- cid mode for this pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK                 --  successfully.
 *      DHCP_MGR_NO_SUCH_POOL_NAME  --  pool not exist.
 *      DHCP_MGR_FAIL               --  fail to set the value
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_SetDhcpPoolOptionCidMode(char *pool_name, UI32_T cid_mode)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDHCPPOOLOPTCIDMODE;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = cid_mode;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetDhcpPoolSubnetMask
 * PURPOSE:
 *      Set subnet mask to the pool. (set by field)
 *
 * INPUT:
 *      pool_name           -- specify the pool name of the table.
 *      pool_subnet_mask    -- the subnet mask of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK                         -- successfully.
 *      DHCP_MGR_NO_SUCH_POOL_NAME          -- pool not existed.
 *      DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS  -- fail to set pool address due to user not yet
 *                                              specified pool type.
 *      DHCP_MGR_INVALID_IP                 -- the specified network address is invalid.
 *      DHCP_MGR_FAIL                       -- fail to set the pool subnet mask.
 *
 * NOTES:
 *  1. If pool_subnet_mask is the same as what we previous configured, do nothing for it.
 */
UI32_T DHCP_PMGR_SetDhcpPoolSubnetMask(char *pool_name, UI32_T mask)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETDHCPPOOLSUBNETMASK;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = mask;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}


/* FUNCTION NAME : DHCP_PMGR_SetMacTypeToPoolConfigEntry
 * PURPOSE:
 *      Set hardware type to the pool. (set by field)
 *
 * INPUT:
 *      pool_name           -- specify the pool name of the table.
 *      pool_hardware_type  -- the hardware type of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK                 -- successfully.
 *      DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed.
 *      DHCP_MGR_FAIL               -- fail to set the pool hardware type.
 *
 * NOTES:
 *  1. SNMP needs to change the hardware type from leaf constant value
 *      to the corresponding value presenting in DHCP_MGR.h
 */
UI32_T DHCP_PMGR_SetMacTypeToPoolConfigEntry(char *pool_name, UI32_T hw_addr_type)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_SETMACTYPETOPOOLCONFIG;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = hw_addr_type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetDhcpPoolOptDefaultRouterTable
 * PURPOSE:
 *      Get dflt router of dhcp pool from WA.
 *
 * INPUT:
 *      pool_name       -- specify the pool name of the table
 *      index           -- the address index
 *      dflt_router_p   -- the dflt router addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to get the value.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_GetDhcpPoolOptDefaultRouterTable(char *pool_name, UI32_T index, UI32_T *ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETDHCPPOOLDEFAULTROUTERTABLE;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = index;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    *ipaddr = msg_p->data.arg_grp_pool_config.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextDhcpPoolOptDefaultRouterTable
 * PURPOSE:
 *      Get next dflt router table of dhcp pool from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *      pool_name       -- specify the pool name of the table
 *      index_p         -- the address index
 *      dflt_router_p   -- the dflt router addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to get the value.
 *
 * NOTES:
 *  1.              ---------> Get next
 * pool_name|        address index
 *              [1]     |    [2]     |
 *          ------------+------------+
 * pool1     10.1.1.1   | 10.1.1.2   |
 *          ------------+------------+
 * pool2     10.1.2.1   | 10.1.2.2   |
 *          ------------+------------+
 *  2. getNext(NULL, 0, &ip_address) will get the 1st pool and 1st index address, in above case
 *      ip_address = 10.1.1.1.
 *  3. getNext(pool1, 2, &ip_address), the result ip_address = 10.1.2.1
 */
UI32_T DHCP_PMGR_GetNextDhcpPoolOptDefaultRouterTable(char *pool_name, UI32_T *index, UI32_T *ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTDHCPPOOLDEFAULTROUTERTABLE;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = *index;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    strncpy(pool_name, msg_p->data.arg_grp_pool_config.pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    *index = msg_p->data.arg_grp_pool_config.arg_ui32_1;
    *ipaddr = msg_p->data.arg_grp_pool_config.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetDhcpPoolOptNetbiosServerTable
 * PURPOSE:
 *      Get netbios server of dhcp pool from WA.
 *
 * INPUT:
 *      pool_name           -- specify the pool name of the table
 *      index               -- the address index
 *      netbios_server_p    -- the netbios server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to get the value.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_GetDhcpPoolOptNetbiosServerTable(char *pool_name, UI32_T index, UI32_T *ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETDHCPPOOLNETBIOSSERVERTABLE;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = index;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    *ipaddr = msg_p->data.arg_grp_pool_config.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextDhcpPoolOptNetbiosServerTable
 * PURPOSE:
 *      Get next netbios server of dhcp pool from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *      pool_name       -- specify the pool name of the table
 *      index_p             -- the address index
 *      netbios_server_p    -- the netbios server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to get the value.
 *
 * NOTES:
 *  1.              ---------> Get next
 * pool_name|        address index
 *              [1]     |    [2]     |
 *          ------------+------------+
 * pool1     10.1.1.1   | 10.1.1.2   |
 *          ------------+------------+
 * pool2     10.1.2.1   | 10.1.2.2   |
 *          ------------+------------+
 *  2. getNext(NULL, 0, &ip_address) will get the 1st pool and 1st index address, in above case
 *      ip_address = 10.1.1.1.
 *  3. getNext(pool1, 2, &ip_address), the result ip_address = 10.1.2.1
 */
UI32_T DHCP_PMGR_GetNextDhcpPoolOptNetbiosServerTable(char *pool_name, UI32_T *index, UI32_T *ipaddr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_pool_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETNEXTDHCPPOOLNETBIOSSERVERTABLE;
    strncpy(msg_p->data.arg_grp_pool_config.pool_name, pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    msg_p->data.arg_grp_pool_config.arg_ui32_1 = *index;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    strncpy(pool_name, msg_p->data.arg_grp_pool_config.pool_name,SYS_ADPT_DHCP_MAX_POOL_NAME_LEN);
    *index = msg_p->data.arg_grp_pool_config.arg_ui32_1;
    *ipaddr = msg_p->data.arg_grp_pool_config.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetDhcpServerExcludedIpAddrTable
 * PURPOSE:
 *      Get exclude IP addr table of dhcp pool from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *      low_address  -- the lowest ip address of the excluded ip range
 *      high_address -- the highest ip address of the excluded ip range
 *
 * OUTPUT:
 *      low_address  -- the lowest ip address of the excluded ip range
 *      high_address -- the highest ip address of the excluded ip range
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   -- fail to get the value.
 *
 * NOTES:
 *
 */
UI32_T DHCP_PMGR_GetDhcpServerExcludedIpAddrTable(UI32_T *low_addr, UI32_T *high_addr)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_S_GETDHCPSERVEREXCLUDEDIPTABLE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = *low_addr;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = *high_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    *low_addr = msg_p->data.arg_grp_ui32_ui32.arg_ui32_1;
    *high_addr = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;
    return msg_p->type.ret_ui32;
}
#endif


#if (SYS_CPNT_DHCP_RELAY == TRUE)
/* FUNCTION NAME : DHCP_PMGR_SetDhcpRelayServerAddrServerIp
 * PURPOSE:
 *      Set relay server address item by array index.
 *
 * INPUT:
 *      vid_ifIndex 	-- the interface to be configured.
 *	  	index			-- the index of the relay server array
 *		ip_address		-- the relay server address of the index to be set.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the relay server address.
 *
 * NOTES:
 *		1. The 1st relay ip's index is 1.
 *		2. If the ip_address = 0, it will clear out the rest of the relay server address
 *			the case of current index's address is 0 and the rest of the relay server IPs
 *			are not 0, we need to clear out the rest of the relay server address.
 *
 */
UI32_T DHCP_PMGR_SetDhcpRelayServerAddrServerIp(UI32_T ifindex, UI32_T index, UI32_T ip_address)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_R_SETRELAYSERVERADDRESS;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = ifindex;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = index;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3 = ip_address ;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_AddInterfaceRelayServerAddress
 * PURPOSE:
 *      Add dhcp relay server ip addresses to the interface.
 *
 * INPUT:
 *      vid_ifIndex             -- the interface id
 *      number_of_relay_server  -- the number of dhcp relay server addresses
 *      relay_server_address[]  -- the list of dhcp relay server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_NO_SUCH_INTERFACE  -- fail to find the interface
 *      DHCP_MGR_FAIL               -- fail to add the dhcp relay server address
 *      DHCP_MGR_OK                 -- successfully
 *      DHCP_MGR_EXCEED_CAPACITY    -- exceed the maximum limit of dhcp relay server address
 *
 *
 * NOTES:
 *  1. The function returns DHCP_MGR_NO_SUCH_INTERFACE if
 *     - vid_ifIndex was invalid
 *
 *  2. The function returns DHCP_MGR_FAIL if
 *     - the interface is in slave mode
 *     - it fails to add addresses
 *
 *  3. The function returns DHCP_MGR_EXCEED_CAPACITY if
 *     - the total amount is bigger than SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER
 *     example1: (A,B,C) + (C,D,E) = (A,B,C,D,E) does not exceed the capacity.
 *     example2: (A,B) + (C,D,E,F) = (A,B,C,D,E,F) exceeds the capacity.
 *
 *  4. The function appends new relay server ip addresses right after the current
 *     relay server addresses.
 *
 */
UI32_T DHCP_PMGR_AddInterfaceRelayServerAddress(UI32_T ifindex, UI32_T num, UI32_T ip_array[])
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_relay_server);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_R_ADDRELAYSERVERADDRESS;
    msg_p->data.arg_grp_relay_server.ifindex = ifindex;
    msg_p->data.arg_grp_relay_server.num = num;
    memcpy(msg_p->data.arg_grp_relay_server.ip_array, ip_array, num*sizeof(UI32_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetDhcpRelayServerAddrTable
 * PURPOSE:
 *      Get dhcp relay server ip address for relay agent to WA.
 *		(Get whole record function for SNMP)
 *
 * INPUT:
 *      vid_ifIndex 	-- the interface to be configured.
 *		index			-- the index number to specify which
 *							relay server address provided to SNMP
 *		ip_address_p	-- the relay server retrieved by above keys.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to get the relay server address.
 *
 * NOTES:
 *
 */

UI32_T DHCP_PMGR_GetDhcpRelayServerAddrTable(UI32_T ifindex, UI32_T index, UI32_T *ip_address_p)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_R_GETRELAYSERVERADDRESS;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = ifindex;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = index;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    *ip_address_p = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;
    
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetNextDhcpRelayServerAddrTable
 * PURPOSE:
 *      Get next dhcp relay server ip address for relay agent to WA.
 *
 * INPUT:
 *      vid_ifIndex_p   -- the pointer to the interface to be configured.
 *      index_p         -- the index number to specify which
 *                          relay server address provided to SNMP
 *      ip_address_p    -- the relay server next to above keys.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_OK     -- successfully.
 *      DHCP_MGR_FAIL   --  fail to get the relay server address.
 *
 * NOTES:
 *  1.              ---------> Get next
 * vid_ifIndex |        relay server address index
 *              [1]     |    [2]     |    [3]     |    [4]     |    [5]     |
 *          ------------+------------+------------+------------+------------+
 * 1001      10.1.1.1   | 10.1.1.2   | 10.1.1.3   | 10.1.1.4   | 10.1.1.5   |
 *          ------------+------------+------------+------------+------------+
 * 1002      10.1.2.1   | 10.1.2.2   | 10.1.2.3   | 10.1.2.4   | 10.1.2.5   |
 *          ------------+------------+------------+------------+------------+
 * 1003      10.1.3.1   | 10.1.3.2   | 10.1.3.3   | 10.1.3.4   | 10.1.3.5   |
 *          ------------+------------+------------+------------+------------+
 *  2. GetNext for (0,0, &ip_address) will get the 1st address of the 1st interface which in above
 *      example is 10.1.1.1;
 *  3. GetNext for (1001,5, &ip_address), the result will be ip_address = 10.1.2.1
 *
 *
 */
UI32_T DHCP_PMGR_GetNextDhcpRelayServerAddrTable(UI32_T *ifindex, UI32_T *index, UI32_T *ip_address_p)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_R_GETNEXTRELAYSERVERADDRESS;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = *ifindex;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = *index;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    *ifindex = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1;
    *index = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2;
    *ip_address_p = msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3;
    
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DeleteInterfaceRelayServerAddress
 * PURPOSE:
 *      Delete dhcp relay server ip addresses from the interface
 *
 * INPUT:
 *      vid_ifIndex             -- the interface id
 *      number_of_relay_server  -- the number of dhcp relay server addresses
 *      relay_server_address[]  -- the list of dhcp relay server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_NO_SUCH_INTERFACE  -- fail to find the interface
 *      DHCP_MGR_FAIL               -- fail to add the dhcp relay server address
 *      DHCP_MGR_OK                 -- successfully
 *
 * NOTES:
 *  1. The function returns DHCP_MGR_NO_SUCH_INTERFACE if
 *     - vid_ifIndex was invalid
 *
 *  2. The function returns DHCP_MGR_FAIL if
 *     - the interface is in slave mode.
 *     - it fails to set addresses.
 *
 *  3. The function deletes every dhcp relay server address before it
 *     returns DHCP_MGR_OK.
 *
 *  4. The function does not do any thing with an address which actually does not exist.
 *     If all the addresses do not exist, the function returns DHCP_MGR_OK.
 *
 */
UI32_T DHCP_PMGR_DeleteInterfaceRelayServerAddress(UI32_T ifindex, UI32_T num, UI32_T ip_array[])
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_relay_server);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_R_DELRELAYSERVERADDRESS;
    msg_p->data.arg_grp_relay_server.ifindex = ifindex;
    msg_p->data.arg_grp_relay_server.num = num;
    memcpy(msg_p->data.arg_grp_relay_server.ip_array, ip_array, num*sizeof(UI32_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

UI32_T DHCP_PMGR_DeleteAllRelayServerAddress(UI32_T ifindex)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_relay_server);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_R_DELALLRELAYSERVERADDRESS;
    msg_p->data.arg_grp_relay_server.ifindex = ifindex;
    
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}



/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_PMGR_GetIfRelayServerAddress
 *------------------------------------------------------------------------------
 * PURPOSE : Get relay server address for specified vlan interface
 * INPUT   : vid_ifIndex -- the interface vlan to be searched for the interface vlan's
 *                          relay server address.
 *           ip_array    -- the array for Relay server IP Address.
 * OUTPUT  : None
 * RETUEN  : DHCP_MGR_OK/DHCP_MGR_FAIL      
 * NOTE    : None
 *------------------------------------------------------------------------------
 */     
UI32_T DHCP_PMGR_GetIfRelayServerAddress(
    UI32_T vid_ifIndex, 
    UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER])
{

    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_relay_server);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_R_GETIFRELAYSERVERADDRESS;
    msg_p->data.arg_grp_relay_server.ifindex = vid_ifIndex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    memcpy(ip_array, msg_p->data.arg_grp_relay_server.ip_array, SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER*sizeof(UI32_T));
    return msg_p->type.ret_ui32;
    
}





#endif



#if (SYS_CPNT_DHCP_RELAY_OPTION82 ==TRUE)

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_PMGR_SetOption82Status
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 status : enable/disable
 * INPUT    : status you want to set. 1 :DHCP_OPTION82_DISABLE, 2 :DHCP_OPTION82_ENABLE
 * OUTPUT   : none
 * RETURN   : DHCP_MGR_OK : If success
 *			  DHCP_MGR_FAIL:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T DHCP_PMGR_SetOption82Status(UI32_T status)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_STATUS;
    msg_p->data.arg_ui32 = status;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_PMGR_SetOption82Policy
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 policy : drop/replace/keep
 * INPUT    : policy you want to set. 1 :DHCP_OPTION82_POLICY_DROP, 2 :DHCP_OPTION82_POLICY_REPLACE
                   3:DHCP_OPTION82_POLICY_KEEP
 * OUTPUT   : none
 * RETURN   : DHCP_MGR_OK : If success
 *			  DHCP_MGR_FAIL:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T DHCP_PMGR_SetOption82Policy(UI32_T policy)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_POLICY;
    msg_p->data.arg_ui32 = policy;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_PMGR_SetOption82RidMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 remote id mode
 * INPUT    : mode you want to set: DHCP_OPTION82_RID_MAC/DHCP_OPTION82_RID_IP
 * OUTPUT   : none
 * RETURN   : DHCP_MGR_OK : If success
 *			  DHCP_MGR_FAIL:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T DHCP_PMGR_SetOption82RidMode(UI32_T mode)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_RID_MODE;
    msg_p->data.arg_ui32 = mode;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_PMGR_SetOption82RidValue
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 remote id value
 * INPUT    : string   --  remote id string
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : max number of characters is 32.
 *------------------------------------------------------------------------------*/
UI32_T DHCP_PMGR_SetOption82RidValue(UI8_T *string)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_rid_value);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_RID_VALUE;
    memcpy(msg_p->data.arg_grp_option82_rid_value.arg_rid_value , string, (SYS_ADPT_MAX_LENGTH_OF_CID + 1));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_PMGR_SetOption82Format
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 encode format
 * INPUT    : subtype_format     -- Setting value
 * OUTPUT   : none
 * RETURN   : DHCP_MGR_OK : If success
 *			  DHCP_MGR_FAIL:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T  DHCP_PMGR_SetOption82Format(BOOL_T subtype_format)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_subtype_format);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_SUBTYPE_FORMAT;
    msg_p->data.arg_grp_option82_subtype_format.subtype_format = subtype_format;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetRelayServerAddress
 * PURPOSE:
 *      Set global dhcp server ip address for L2 relay agent to WA
 *
 * INPUT:
 *		ip1, ip2, ip3, ip4, ip5	-- the ip list of servers
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the relay server address.
 *
 * NOTES:
 *	1. The function will delete the previous setting for relay server address
 * 		and add the newly configured relay server address
 *	2. The maximun numbers of server ip are 5. If user specifies (ip1, ip2,0,0,0) which means
 *		the user only specifies 2 DHCP Relay Server.
 *	3. If user specifies (ip1, 0,ip3, ip4, ip5), (ip1,0,0,0,0)will be set to DHCP
 *		Relay Server Address List
 *
 */
UI32_T DHCP_PMGR_SetRelayServerAddress(UI32_T ip1, UI32_T ip2, UI32_T ip3, UI32_T ip4, UI32_T ip5)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_relay_server);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_SET_RELAY_SERVER;
    msg_p->data.arg_grp_option82_relay_server.arg_relay_server[0] = ip1;
    msg_p->data.arg_grp_option82_relay_server.arg_relay_server[1] = ip2;
    msg_p->data.arg_grp_option82_relay_server.arg_relay_server[2] = ip3;
    msg_p->data.arg_grp_option82_relay_server.arg_relay_server[3] = ip4;
    msg_p->data.arg_grp_option82_relay_server.arg_relay_server[4] = ip5;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_SetRelayServerAddressFromSnmp
 * PURPOSE:
 *      Set global dhcp server ip address for L2 relay agent to WA from snmp
 *
 * INPUT:
 *      index      -- entry index
 *		server_ip  -- ip address of server
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the relay server address.
 *
 * NOTES:
 *	This function is only used for SNMP, it can set relay server address for specified index
 *  If there's zero ip address in entry whose index is lower than specified index, we can't let user to set from snmp.
 */
UI32_T DHCP_PMGR_SetRelayServerAddressFromSnmp(UI32_T index, UI32_T server_ip)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_relay_server_snmp);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_SET_RELAY_SERVER_FROM_SNMP;
    msg_p->data.arg_grp_option82_relay_server_snmp.index = index;
    msg_p->data.arg_grp_option82_relay_server_snmp.server_ip = server_ip;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetRelayServerAddressFromSnmp
 * PURPOSE:
 *      Get global dhcp server ip address for L2 relay agent to WA from snmp
 *
 * INPUT:
 *      index      -- entry index
 *
 * OUTPUT:
 *      server_ip  -- ip address of server
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the relay server address.
 *
 * NOTES:
 *	This function is only used for SNMP, it can get relay server address for specified index
 */
UI32_T DHCP_PMGR_GetRelayServerAddressFromSnmp(UI32_T index, UI32_T *server_ip)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_relay_server_snmp);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_GET_RELAY_SERVER_FROM_SNMP;
    msg_p->data.arg_grp_option82_relay_server_snmp.index = index;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    
    *server_ip = msg_p->data.arg_grp_option82_relay_server_snmp.server_ip;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_DeleteGlobalRelayServerAddress
 * PURPOSE:
 *      Delete global Server addresses for DHCP L2 Relay Agent in WA
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to delete all relay server addresses.
 *
 * NOTES:1. For add relay server address, we can add all server IPs at
 *			one time. If we modify the server address twice, the last update
 *			would replace the previous setting.
 *
 *		 2. For delete relay server IP, we only allow delete all at a time
 *
 */
UI32_T DHCP_PMGR_DeleteGlobalRelayServerAddress()
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_DELETE_RELAY_SERVER;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              DHCP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return DHCP_MGR_FAIL;
    }
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME : DHCP_PMGR_GetRunningDhcpRelayOption82Status
 * PURPOSE:
 *     Get running dhcp relay option 82 status
 *
 * INPUT:
 *      status_p
 *
 * OUTPUT:
 *      status_p  -- DHCP_OPTION82_ENABLE/DHCP_OPTION82_DISABLE.
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES: None.
 *
 */
SYS_TYPE_Get_Running_Cfg_T DHCP_PMGR_GetRunningDhcpRelayOption82Status(UI32_T *status_p)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_STATUS;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    
    *status_p = msg_p->data.arg_ui32;
    return msg_p->type.result_running_cfg;
}

/* FUNCTION NAME : DHCP_PMGR_GetRunningDhcpRelayOption82Policy
 * PURPOSE:
 *     Get running dhcp relay option 82 policy
 *
 * INPUT:
 *      policy_p
 *
 * OUTPUT:
 *      policy_p  -- DHCP_OPTION82_POLICY_DROP/DHCP_OPTION82_POLICY_REPLACE/DHCP_OPTION82_POLICY_KEEP.
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES: None.
 *
 */
SYS_TYPE_Get_Running_Cfg_T DHCP_PMGR_GetRunningDhcpRelayOption82Policy(UI32_T *policy_p)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_POLICY;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    
    *policy_p = msg_p->data.arg_ui32;
    return msg_p->type.result_running_cfg;
}

/* FUNCTION NAME : DHCP_PMGR_GetRunningDhcpRelayOption82RidMode
 * PURPOSE:
 *     Get running dhcp relay option 82 remote id mode
 *
 * INPUT:
 *      mode_p
 *
 * OUTPUT:
 *      mode_p  -- DHCP_OPTION82_RID_MAC/DHCP_OPTION82_RID_IP.
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES: None.
 *
 */
SYS_TYPE_Get_Running_Cfg_T DHCP_PMGR_GetRunningDhcpRelayOption82RidMode(UI32_T *mode_p)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_RID_MODE;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    
    *mode_p = msg_p->data.arg_ui32;
    return msg_p->type.result_running_cfg;
}

/* FUNCTION NAME : DHCP_PMGR_GetRunningDhcpRelayOption82Format
 * PURPOSE:
 *     Get running dhcp relay option 82 subtype format
 *
 * INPUT:
 *      subtype_format_p
 *
 * OUTPUT:
 *      subtype_format_p  -- TRUE/FALSE
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES: None.
 *
 */
SYS_TYPE_Get_Running_Cfg_T DHCP_PMGR_GetRunningDhcpRelayOption82Format(BOOL_T *subtype_format_p)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_subtype_format);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_FORMAT;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    
    *subtype_format_p = msg_p->data.arg_grp_option82_subtype_format.subtype_format;
    return msg_p->type.result_running_cfg;
}

/* FUNCTION NAME : DHCP_PMGR_GetRunningRelayServerAddress
 * PURPOSE:
 *		Get running global relay server addresses.
 *
 * INPUT:
 *		relay_server -- the pointer to relay server
 *
 * OUTPUT:
 *		relay_server -- the pointer to relay server
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES: None.
 *
 */
SYS_TYPE_Get_Running_Cfg_T DHCP_PMGR_GetRunningRelayServerAddress(UI32_T *relay_server)
{
    const UI32_T msg_size = DHCP_MGR_GET_MSG_SIZE(arg_grp_option82_relay_server);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DHCP_MGR_IpcMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_DHCP;
    msgbuf_p->msg_size = msg_size;
    
    msg_p = (DHCP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_RELAY_SERVER;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    
    memcpy(relay_server, msg_p->data.arg_grp_option82_relay_server.arg_relay_server, SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER*sizeof(UI32_T));
    return msg_p->type.result_running_cfg;
}
#endif


