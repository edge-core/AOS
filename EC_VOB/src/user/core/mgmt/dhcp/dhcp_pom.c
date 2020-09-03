/* MODULE NAME:  dhcp_pom.c
 * PURPOSE: For accessing om through IPC
 *
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    12/10/2009 - Jimi Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_module.h"
#include "sysfun.h"
#include "dhcp_om.h"
#include "sysfun.h"
#include "l_mm.h"
#include <string.h>
#include <sys_bld.h>

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
 * ROUTINE NAME : DHCP_POM_InitiateProcessResource
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
 *    TRUE/FALSE.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DHCP_POM_InitiateProcessResource(void)
{
    /* Given that CLUSTER is run in MGMT_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_IP_SERVICE_PROC_OM_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/* FUNCTION NAME : DHCP_POM_GetDhcpRelayOption82Status
 * PURPOSE:
 *     Get dhcp relay option 82 status
 *
 * INPUT:
 *      status     
 *
 * OUTPUT:
 *      status  -- DHCP_OPTION82_ENABLE/DHCP_OPTION82_DISABLE.
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82Status(UI32_T *status)
{
    const UI32_T msg_buf_size=DHCP_OM_GET_MSG_SIZE(ui32_v);
    SYSFUN_Msg_T *msg_p;
    DHCP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DHCP;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DHCP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DHCP_OM_IPCCMD_GET_OPTION82_STATUS;
    msg_data_p->data.ui32_v = *status;
    /*send ipc message
     */
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_buf_size, msg_p)!=SYSFUN_OK)
    {
        return DHCP_OM_FAIL;
    }

    *status = msg_data_p->data.ui32_v;

    return msg_data_p->type.result_ui32;
}

/* FUNCTION NAME : DHCP_POM_GetDhcpRelayOption82Policy
 * PURPOSE:
 *     Get dhcp relay option 82 policy
 *
 * INPUT:
 *      policy     
 *
 * OUTPUT:
 *      policy  -- DHCP_OPTION82_POLICY_DROP/DHCP_OPTION82_POLICY_REPLACE/DHCP_OPTION82_POLICY_KEEP.
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82Policy(UI32_T *policy)
{
    const UI32_T msg_buf_size=DHCP_OM_GET_MSG_SIZE(ui32_v);
    SYSFUN_Msg_T *msg_p;
    DHCP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DHCP;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DHCP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DHCP_OM_IPCCMD_GET_OPTION82_POLICY;

    msg_data_p->data.ui32_v = *policy;
    /*send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_buf_size, msg_p)!=SYSFUN_OK)
    {
        return DHCP_OM_FAIL;
    }

    *policy = msg_data_p->data.ui32_v;

    return msg_data_p->type.result_ui32;
}

/* FUNCTION NAME : DHCP_POM_GetDhcpRelayOption82RidMode
 * PURPOSE:
 *     Get dhcp relay option 82 remote id mode
 *
 * INPUT:
 *      mode     
 *
 * OUTPUT:
 *      mode  -- DHCP_OPTION82_RID_MAC/DHCP_OPTION82_RID_IP.
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_FAIL
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82RidMode(UI32_T *mode)
{
    const UI32_T msg_buf_size=DHCP_OM_GET_MSG_SIZE(ui32_v);
    SYSFUN_Msg_T *msg_p;
    DHCP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DHCP;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DHCP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DHCP_OM_IPCCMD_GET_OPTION82_RID_MODE;

    msg_data_p->data.ui32_v = *mode;
    /*send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_buf_size, msg_p)!=SYSFUN_OK)
    {
        return DHCP_OM_FAIL;
    }

    *mode = msg_data_p->data.ui32_v;

    return msg_data_p->type.result_ui32;
}

/* FUNCTION NAME : DHCP_OM_GetDhcpRelayOption82RidValue
 * PURPOSE:
 *     Get dhcp relay option 82 remote id value
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      string     --  configured string
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82RidValue(UI8_T *string)
{
    const UI32_T msg_buf_size=DHCP_OM_GET_MSG_SIZE(DHCP_Relay_Option82_Rid_Value_data);
    SYSFUN_Msg_T *msg_p;
    DHCP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DHCP;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DHCP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DHCP_OM_IPCCMD_GET_OPTION82_RID_VALUE;

    memcpy(msg_data_p->data.DHCP_Relay_Option82_Rid_Value_data.rid_value, string, SYS_ADPT_MAX_LENGTH_OF_RID + 1);
    /*send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_buf_size, msg_p)!=SYSFUN_OK)
    {
        return DHCP_OM_FAIL;
    }

    memcpy(string,msg_data_p->data.DHCP_Relay_Option82_Rid_Value_data.rid_value, SYS_ADPT_MAX_LENGTH_OF_RID + 1);

    return msg_data_p->type.result_ui32;
}

/* FUNCTION NAME : DHCP_POM_GetDhcpRelayOption82Format
 * PURPOSE:
 *     Get dhcp relay option 82 subtype format 
 *
 * INPUT:
 *      subtype_format_p
 *
 * OUTPUT:
 *      subtype_format_p
 * RETURN:
 *      DHCP_OM_OK
 *      DHCP_OM_INVALID_ARG
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_POM_GetDhcpRelayOption82Format(BOOL_T *subtype_format_p)
{
    const UI32_T msg_buf_size=DHCP_OM_GET_MSG_SIZE(bool_v);
    SYSFUN_Msg_T *msg_p;
    DHCP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DHCP;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DHCP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DHCP_OM_IPCCMD_GET_OPTION82_FORMAT;

    /*send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_buf_size, msg_p)!=SYSFUN_OK)
    {
        return DHCP_OM_FAIL;
    }

    *subtype_format_p = msg_data_p->data.bool_v;

    return msg_data_p->type.result_ui32;
}

/* FUNCTION	NAME : DHCP_POM_GetRelayServerAddress

 * PURPOSE:
 *		Get global relay server addresses.
 *
 * INPUT:
 *		relay_server -- the pointer to relay server
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_OM_OK	    -- successfully get
 *		DHCP_OM_FAIL	-- Fail to get server ip to interface
 *
 * NOTES:
 *		None.
 */
UI32_T DHCP_POM_GetRelayServerAddress(UI32_T *relay_server)
{
    const UI32_T msg_buf_size=DHCP_OM_GET_MSG_SIZE(DHCP_Relay_Option82_Relay_Server_data);
    SYSFUN_Msg_T *msg_p;
    DHCP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DHCP;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DHCP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DHCP_OM_IPCCMD_GET_RELAY_SERVER_ADDRESS;

    memcpy(msg_data_p->data.DHCP_Relay_Option82_Relay_Server_data.relay_server, relay_server, sizeof(UI32_T)*SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER);
    /*send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_buf_size, msg_p)!=SYSFUN_OK)
    {
        return DHCP_OM_FAIL;
    }

    memcpy( relay_server, msg_data_p->data.DHCP_Relay_Option82_Relay_Server_data.relay_server, sizeof(UI32_T)*SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER);

    return msg_data_p->type.result_ui32;
}

#endif

#if (SYS_CPNT_DHCP_RELAY == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCP_POM_GetIfRelayServerAddress
 *------------------------------------------------------------------------------
 * PURPOSE: Get all interface relay server addresses associated with interface.
 * INPUT :  vid_ifindex  -- the interface to be searched for relay server address.
 * OUTPUT:  relay_server -- the pointer to relay server
 * RETURN:  DHCP_OM_OK/DHCP_OM_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCP_POM_GetIfRelayServerAddress(UI32_T vid_ifindex, UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER])
{
    const UI32_T msg_buf_size=DHCP_OM_GET_MSG_SIZE(DHCP_Relay_If_Relay_Server_data);
    SYSFUN_Msg_T *msg_p;
    DHCP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_DHCP;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(DHCP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = DHCP_OM_IPCCMD_GET_IF_RELAY_SERVER_ADDRESS;

    msg_data_p->data.DHCP_Relay_If_Relay_Server_data.vid_ifindex = vid_ifindex;

    /*send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_buf_size, msg_p)!=SYSFUN_OK)
    {
        return DHCP_OM_FAIL;
    }

    memcpy(ip_array, msg_data_p->data.DHCP_Relay_If_Relay_Server_data.ip_array, sizeof(UI32_T)*SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER);
    return msg_data_p->type.result_ui32;
}
#endif
