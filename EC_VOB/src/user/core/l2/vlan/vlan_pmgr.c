/*-----------------------------------------------------------------------------
 * FILE NAME: VLAN_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for VLAN MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/30     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "sysfun.h"
#include "vlan_mgr.h"
#include "vlan_pmgr.h"
#include "vlan_type.h"
#include "leaf_2863.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* trace id definition when using L_MM
 */
enum
{
    VLAN_PMGR_TRACEID_SETDOT1QVLANSTATICENTRY,
    VLAN_PMGR_TRACEID_GETDOT1QVLANSTATICENTRYAGENT,
    VLAN_PMGR_TRACEID_GETDOT1QVLANCURRENTENTRYAGENT,
    VLAN_PMGR_TRACEID_GETNEXTDOT1QVLANCURRENTENTRYAGENT,
};


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
 * ROUTINE NAME : VLAN_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for VLAN_PMGR in the calling process.
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
BOOL_T VLAN_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for VLAN MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of VLAN_PMGR_InitiateProcessResource */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qGvrpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the GVRP status of the bridge can be
 *            successfully Set.  Otherwise, return false.
 * INPUT    : gvrp_status - VAL_dot1qGvrpStatus_enabled \ VAL_dot1qGvrpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : The administrative status requested by management for GVRP.  The
 *            value enabled(1) indicates that GVRP should be enabled on this
 *            device, on all ports for which it has not been specifically disabled.
 *            When disabled(2), GVRP is disabled on all ports and all GVRP packets
 *            will be forwarded transparently.  This object affects all GVRP
 *            Applicant and Registrar state machines.  A transition from disabled(2)
 *            to enabled(1) will cause a reset of all GVRP state machines on all ports
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qGvrpStatus(UI32_T gvrp_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QGVRPSTATUS;
    msg_p->data.arg_ui32 = gvrp_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qGvrpStatus */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qConstraintTypeDefault
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the constrain type for this device
 *            can be retrieve successfully.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : VAL_dot1qConstraintTypeDefault_independent\
 *            VAL_dot1qConstraintTypeDefault_shared
 * RETURN   : TRUE \ FALSE
 * NOTES    : System_default is IVL.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qConstraintTypeDefault(UI32_T constrain_type)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QCONSTRAINTTYPEDEFAULT;
    msg_p->data.arg_ui32 = constrain_type;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qConstraintTypeDefault */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_CreateVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan is successfully created.
 *            Otherwise, false is returned.
 * INPUT    : vid         -- the new created vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : Vlan info is updated in the database.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The vlan_status parameter is used to identify dynamic (by GVRP)
 *               or static (by management).
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_CreateVlan(UI32_T vid, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_CREATEVLAN;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_CreateVlan */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DeleteVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan is successfully deleted
 *            from the database.  Otherwise, false is returned.
 * INPUT    : vid   -- the existed vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : The specific vlan is removed from the database.
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DeleteVlan(UI32_T vid, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DELETEVLAN;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_DeleteVlan */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DeleteNormalVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan that is not private
 *            vlan is successfully deleted from the database.
 *            Otherwise, false is returned.
 * INPUT    : vid   -- the existed vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : The specific vlan is removed from the database.
 * RETURN   : TRUE \ FALSE
 * NOTES    : If the specific vlan is private vlan, return FALSE.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DeleteNormalVlan(UI32_T vid, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DELETENORMALVLAN;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_DeleteNormalVlan */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qVlanStaticName
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan name is updated.  False otherwise.
 * INPUT    : vid   -- the vlan id
 *            value -- the vlan name
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : vlan name length is restricted between MINSIZE_dot1qVlanStaticName and
 *            MAXSIZE_dot1qVlanStaticName
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qVlanStaticName(UI32_T vid, char *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QVLANSTATICNAME;
    msg_p->data.arg_grp2.arg1 = vid;
    strncpy(msg_p->data.arg_grp2.arg2, value, SYS_ADPT_MAX_VLAN_NAME_LEN);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qVlanStaticName */
/*EPR:ES4827G-FLF-ZZ-00232
 *Problem: CLI:size of vlan name different in console and mib
 *Solution: add CLI command "alias" for interface set,the
 *          alias is different from name and port descrition,so
 *          need add new command.
 *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
 *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
 *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
 *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
 *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
 *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
 *Approved by:Hardsun
 *Fixed by:Dan Xie
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qVlanAlias
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan alias is updated.  False otherwise.
 * INPUT    : vid   -- the vlan id
 *            value -- the vlan name
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : vlan alias length is restricted between 0 and
 *            MAXSIZE_ifAlias
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qVlanAlias(UI32_T vid, char *value)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_alias);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QVLANALIAS;
    msg_p->data.arg_alias.vid = vid;
    strncpy(msg_p->data.arg_alias.alias, value, MAXSIZE_ifAlias);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_AddEgressPortMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully join vlan's
 *            egress port list.  Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is successfully joined vlan's egress port list.
 * RETURN   : TRUE \ FALSE
 * NOTES    : A port may not be added in this set if it is already a member of
 *            the set of ports in ForbiddenEgressPorts.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_AddEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_ADDEGRESSPORTMEMBER;
    msg_p->data.arg_grp3.arg1 = vid;
    msg_p->data.arg_grp3.arg2 = lport_ifindex;
    msg_p->data.arg_grp3.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_AddEgressPortMember */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_AddEgressPortMemberForGVRP
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully join vlan's
 *            egress port list.  Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is successfully joined vlan's egress port list.
 * RETURN   : TRUE \ FALSE
 * NOTES    : A port may not be added in this set if it is already a member of
 *            the set of ports in ForbiddenEgressPorts.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_AddEgressPortMemberForGVRP(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_ADDEGRESSPORTMEMBERFORGVRP;
    msg_p->data.arg_grp3.arg1 = vid;
    msg_p->data.arg_grp3.arg2 = lport_ifindex;
    msg_p->data.arg_grp3.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_AddEgressPortMemberForGVRP */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DeleteEgressPortMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully remove from vlan's
 *            egress port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is remove from vlan's egress port list.
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DeleteEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DELETEEGRESSPORTMEMBER;
    msg_p->data.arg_grp3.arg1 = vid;
    msg_p->data.arg_grp3.arg2 = lport_ifindex;
    msg_p->data.arg_grp3.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_DeleteEgressPortMember */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qVlanStaticEgressPorts
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if egress port list of the specific vlan
 *            is succesfully updated. Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            portlist   -- a port list that contains the ports that request to
 *                          join or leave the specific vlan.
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. A port may not be added in egress list if it is already a member of
 *               the set of ports in ForbiddenEgressPorts.
 *            2. In the case of SNMP command, vlan_mgr will do a row create when
 *               the specific vid does not already existed in vlan_om.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qVlanStaticEgressPorts(UI32_T vid, UI8_T *portlist, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QVLANSTATICEGRESSPORTS;
    msg_p->data.arg_grp4.arg1 = vid;
    memcpy(msg_p->data.arg_grp4.arg2, portlist,
        sizeof(msg_p->data.arg_grp4.arg2));
    msg_p->data.arg_grp4.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qVlanStaticEgressPorts */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_AddForbiddenEgressPortMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully included in  vlan's
 *            forbidden egress port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is prohibited to join vlan's egress port list
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1.A port may not be added in this set if it is already a member of
 *              the set of ports in the EgressPortlist
 *            2. lport_ifindex will not be permitted to join vlan's member list
 *            until it is removed from Forbidden_port list.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_AddForbiddenEgressPortMember(UI32_T vid, UI32_T lport_ifindex,UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_ADDFORBIDDENEGRESSPORTMEMBER;
    msg_p->data.arg_grp3.arg1 = vid;
    msg_p->data.arg_grp3.arg2 = lport_ifindex;
    msg_p->data.arg_grp3.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_AddForbiddenEgressPortMember */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DeleteForbiddenEgressPortMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully remove from vlan's
 *            forbidden egress port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is permitted to join vlan's egress port list
 * RETURN   : TRUE \ FALSE
 * NOTES    : lport_ifindex will be permitted to join vlan's member list
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DeleteForbiddenEgressPortMember(UI32_T vid, UI32_T lport_ifindex,UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DELETEFORBIDDENEGRESSPORTMEMBER;
    msg_p->data.arg_grp3.arg1 = vid;
    msg_p->data.arg_grp3.arg2 = lport_ifindex;
    msg_p->data.arg_grp3.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_DeleteForbiddenEgressPortMember */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qVlanForbiddenEgressPorts
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if forbidden port list of the specific vlan
 *            is succesfully updated. Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            portlist - a port list that contains the ports that are to be or not to be
 *                       forbidden from joining the specific vlan.
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1.A port may not be added in this set if it is already a member of
 *              the set of ports in the EgressPortlist
 *            2. lport_ifindex will not be permitted to join vlan's member list
 *            until it is removed from Forbidden_port list.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qVlanForbiddenEgressPorts(UI32_T vid, UI8_T *portlist, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QVLANFORBIDDENEGRESSPORTS;
    msg_p->data.arg_grp4.arg1 = vid;
    memcpy(msg_p->data.arg_grp4.arg2, portlist,
        sizeof(msg_p->data.arg_grp4.arg2));
    msg_p->data.arg_grp4.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qVlanForbiddenEgressPorts */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddUntagPortMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully included in vlan's
 *            untag port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex should transmit egress packets for this VLAN as untagged.
 * RETURN   : TRUE \ FALSE
 * NOTES    : lport_ifindex must exist in vlan's egress_port_list before it can join
 *            the untag_pot_list
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_AddUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_ADDUNTAGPORTMEMBER;
    msg_p->data.arg_grp3.arg1 = vid;
    msg_p->data.arg_grp3.arg2 = lport_ifindex;
    msg_p->data.arg_grp3.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_AddUntagPortMember */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DeleteUntagPortMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully remove from vlan's
 *            untag port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex should transmit egress packets for this VLAN as tagged.
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DeleteUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DELETEUNTAGPORTMEMBER;
    msg_p->data.arg_grp3.arg1 = vid;
    msg_p->data.arg_grp3.arg2 = lport_ifindex;
    msg_p->data.arg_grp3.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_DeleteUntagPortMember */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanStaticUntaggedPorts
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the untagged port list of the specific vlan
 *            is succesfully updated. Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            portlist - a portlist of untagged port.
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none.
 * RETURN   : TRUE \ FALSE
 * NOTES    : lport_ifindex must exist in vlan's egress_port_list before it can join
 *            the untag_pot_list
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qVlanStaticUntaggedPorts(UI32_T vid, UI8_T *portlist, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QVLANSTATICUNTAGGEDPORTS;
    msg_p->data.arg_grp4.arg1 = vid;
    memcpy(msg_p->data.arg_grp4.arg2, portlist,
        sizeof(msg_p->data.arg_grp4.arg2));
    msg_p->data.arg_grp4.arg3 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qVlanStaticUntaggedPorts */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qVlanStaticRowStatus
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the vlan entry can be
 *            set successfully.  Otherwise, return false.
 * INPUT    : vid         -- the vlan id
 *            row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qVlanStaticRowStatus(UI32_T vid, UI32_T row_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QVLANSTATICROWSTATUS;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = row_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qVlanStaticRowStatus */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qVlanStaticRowStatusForGVRP
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the vlan entry can be
 *            set successfully.  Otherwise, return false.
 * INPUT    : vid         -- the vlan id
 *            row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qVlanStaticRowStatusForGVRP(UI32_T vid, UI32_T row_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QVLANSTATICROWSTATUSFORGVRP;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = row_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qVlanStaticRowStatusForGVRP */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetVlanAddressMethod
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid             -- the specific vlan id.
 *          : address_method  -- VAL_vlanAddressMethod_user \
 *                               VAL_vlanAddressMethod_bootp \
 *                               VAL_vlanAddressMethod_dhcp
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetVlanAddressMethod(UI32_T vid, UI32_T address_method)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETVLANADDRESSMETHOD;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = address_method;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetVlanAddressMethod */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qVlanStaticEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific static vlan entry can be
              created successfully.  Otherwise, return FALSE.
 * INPUT    : vid       -- the specific vlan id.
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : This API only supports CAW and CAG commands for set by record.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qVlanStaticEntry(VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_vlan_entry);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    VLAN_MGR_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_VLAN,
                                 VLAN_PMGR_TRACEID_SETDOT1QVLANSTATICENTRY));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QVLANSTATICENTRY;
    msg_p->data.arg_vlan_entry = *vlan_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
} /* End of VLAN_PMGR_SetDot1qVlanStaticEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetVlanAdminStatus
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port admin status is set
 *            successfully.  Otherwise, return false.
 * INPUT    : vid - the specific vlan id
 *            admin_status - VAL_ifAdminStatus_up \ VAL_ifAdminStatus_down
 *                           VAL_ifAdminStatus_testing
 * OUTPUT   : none
 * RETURN   : TRUE  \ FALSE
 * NOTES    : Currently Not Supported in this version.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetVlanAdminStatus(UI32_T vid, UI32_T admin_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETVLANADMINSTATUS;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = admin_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetVlanAdminStatus */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetLinkUpDownTrapEnabled
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the trap status can be set successfully
 *            for the specific vlan.  Otherwise, return false.
 * INPUT    : vid - the specific vlan id
 *            trap_status - VAL_ifLinkUpDownTrapEnable_enabled \
 *                           VAL_ifLinkUpDownTrapEnable_disabled
 * OUTPUT   : none
 * RETURN   : TRUE  \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetLinkUpDownTrapEnabled(UI32_T vid, UI32_T trap_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETLINKUPDOWNTRAPENABLED;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = trap_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetLinkUpDownTrapEnabled */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qPvid
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the pvid of the specific port can be
 *            set successfully. Otherwise, return false.
 * INPUT    : lport_ifindex   -- the port number
 *            pvid     -- the pvid value
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qPvid(UI32_T lport_ifindex, UI32_T pvid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QPVID;
    msg_p->data.arg_grp1.arg1 = lport_ifindex;
    msg_p->data.arg_grp1.arg2 = pvid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qPvid */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qPortAcceptableFrameTypes
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port acceptable frame type is set
 *            successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex       -- the port number
 *            acceptable_frame_types - VAL_dot1qPortAcceptableFrameTypes_admitAll \
 *                             VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : The default value is VAL_dot1qPortAcceptableFrameTypes_admitAll.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qPortAcceptableFrameTypes(UI32_T lport_ifindex, UI32_T acceptable_frame_types)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QPORTACCEPTABLEFRAMETYPES;
    msg_p->data.arg_grp1.arg1 = lport_ifindex;
    msg_p->data.arg_grp1.arg2 = acceptable_frame_types;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qPortAcceptableFrameTypes */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qIngressFilter
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port ingress filter state is set
 *            successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex         -- the port number
 *            ingress_filter -- VAL_dot1qPortIngressFiltering_true \
 *                              VAL_dot1qPortIngressFiltering_false
 * OUTPUT   : none
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. Default value is VAL_dot1qPortIngressFiltering_false
 *            2. This control does not affect VLAN independent BPDU frames, such
 *               as GVRP and STP.  It does affect VLAN dependent BPDU frames, such
 *               as GMRP.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qIngressFilter(UI32_T lport_ifindex, UI32_T ingress_filter)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QINGRESSFILTER;
    msg_p->data.arg_grp1.arg1 = lport_ifindex;
    msg_p->data.arg_grp1.arg2 = ingress_filter;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qIngressFilter */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetVlanPortMode
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port_mode is set successfully.
 *            Otherwise, return false.
 * INPUT    : lport_ifindex  -- the port number
 *            port_mode -- VAL_vlanPortMode_hybrid \
 *                         VAL_vlanPortMode_dot1qTrunk
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The default value is "VLAN_MGR_HYBRID_LINK".
 *            2. Port_mode value is defined in leaf_es3626a.h
 *            3. Hybrid mode indicates the specific port can join both tagged and
 *               untagged vlan member list.
 *            4. Trunk mode indicates the specific port can only join tagged vlan
 *               member list and the Acceptable_frame_type field will automatically
 *               be set to VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetVlanPortMode(UI32_T lport_ifindex, UI32_T port_mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETVLANPORTMODE;
    msg_p->data.arg_grp1.arg1 = lport_ifindex;
    msg_p->data.arg_grp1.arg2 = port_mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetVlanPortMode */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qPortGvrpStatusEnabled
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the port gvrp status can be enable
 *            successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex - the logical port number
 *            gvrp_status   - VAL_dot1qPortGvrpStatus_enabled \
 *                            VAL_dot1qPortGvrpStatus_disabled
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qPortGvrpStatusEnabled(UI32_T lport_ifindex, UI32_T gvrp_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QPORTGVRPSTATUSENABLED;
    msg_p->data.arg_grp1.arg1 = lport_ifindex;
    msg_p->data.arg_grp1.arg2 = gvrp_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qPortGvrpStatusEnabled */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qPortGvrpFailedRegistrations
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the gvrp failed registration counter
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : dot1qPortGvrpFailedRegistrations in vlan_om is incremented.
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qPortGvrpFailedRegistrations(UI32_T lport_ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QPORTGVRPFAILEDREGISTRATIONS;
    msg_p->data.arg_ui32 = lport_ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qPortGvrpFailedRegistrations */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetDot1qPortGvrpLastPduOrigin
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the last pdu origin of the port can be
 *            set successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 *            pdu_mac_address - the source address of the last gvrp message receive
 *            on this port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetDot1qPortGvrpLastPduOrigin(UI32_T lport_ifindex, UI8_T *pdu_mac_address)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp_ui32_mac);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETDOT1QPORTGVRPLASTPDUORIGIN;
    msg_p->data.arg_grp_ui32_mac.arg_ui32 = lport_ifindex;
    memcpy(msg_p->data.arg_grp_ui32_mac.arg_mac, pdu_mac_address,
        sizeof(msg_p->data.arg_grp_ui32_mac.arg_mac));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetDot1qPortGvrpLastPduOrigin */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetPortEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific port info can
 *            be retrieve successfully.  Otherwiser, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : *entry   -- the whole data structure to store the value
 * RETURN   : TRUE  \ FALSE
 * NOTES    : The returned entry contains Dot1qPortVlanEntry as well as
 *            Dot1dPortGarpEntry
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_GetPortEntry(UI32_T lport_ifindex, VLAN_OM_Vlan_Port_Info_T *vlan_port_info)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETPORTENTRY;
    msg_p->data.arg_grp5.arg1 = lport_ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *vlan_port_info = msg_p->data.arg_grp5.arg2;

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_GetPortEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetVlanPortEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan_port entry can
 *            be retrieve successfully.  Otherwiser, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : *entry   -- the whole data structure to store the value
 * RETURN   : TRUE  \ FALSE
 * NOTES    : The returned entry contains Dot1qPortVlanEntry as well as
 *            Dot1dPortGarpEntry
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_GetVlanPortEntry(UI32_T lport_ifindex, VLAN_OM_VlanPortEntry_T *vlan_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETVLANPORTENTRY;
    msg_p->data.arg_grp6.arg1 = lport_ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *vlan_port_entry = msg_p->data.arg_grp6.arg2;

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_GetVlanPortEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetNextVlanPortEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available vlan_port entry can
 *            be retrieve successfully.  Otherwiser, return false.
 * INPUT    : *lport_ifindex  -- the logical port number
 * OUTPUT   : *lport_ifindex  -- the next logical port number
 *            *entry   -- the whole data structure to store the value
 * RETURN   : TRUE \ FALSE
 * NOTES    : The returned entry contains Dot1qPortVlanEntry as well as
 *            Dot1dPortGarpEntry
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_GetNextVlanPortEntry(UI32_T *lport_ifindex, VLAN_OM_VlanPortEntry_T *vlan_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETNEXTVLANPORTENTRY;
    msg_p->data.arg_grp6.arg1 = *lport_ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *lport_ifindex = msg_p->data.arg_grp6.arg1;
    *vlan_port_entry = msg_p->data.arg_grp6.arg2;

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_GetNextVlanPortEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetDot1qPortVlanEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific port entry can be retrieve
 *            from vlan_om.  Otherwise, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : *entry   -- the whole data structure to store the value
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_GetDot1qPortVlanEntry(UI32_T lport_ifindex, VLAN_OM_Dot1qPortVlanEntry_T *port_vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp7);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETDOT1QPORTVLANENTRY;
    msg_p->data.arg_grp7.arg1 = lport_ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *port_vlan_entry = msg_p->data.arg_grp7.arg2;

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_GetDot1qPortVlanEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetNextDot1qPortVlanEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available  port entry can be
 *            retrieve from vlan_om.  Otherwise, return false.
 * INPUT    : *lport_ifindex  -- the logical port number
 * OUTPUT   : *lport_ifindex  -- the next logical port number
 *            *entry   -- the whole data structure to store the value
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. The logical port number would be stored in the lport_ifindex variable
 *               if get the next port information.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_GetNextDot1qPortVlanEntry(UI32_T *lport_ifindex, VLAN_OM_Dot1qPortVlanEntry_T *port_vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp7);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETNEXTDOT1QPORTVLANENTRY;
    msg_p->data.arg_grp7.arg1 = *lport_ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *lport_ifindex = msg_p->data.arg_grp7.arg1;
    *port_vlan_entry = msg_p->data.arg_grp7.arg2;

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_GetNextDot1qPortVlanEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetManagementVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : Set the specified VLAN as management VLAN
 * INPUT    : vid - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. For backward compatible. In the new design, when setting IP Address
 *               L2 product shall call VLAN_MGR_SetGlobalManagementVlan() and
 *               VLAN_MGR_SetIpInterface(), and the L3 product shall call
 *               VLAN_MGR_SetIpInterface();
 *            2. This function willchange the global management VLAN variable,
 *               and this function will also set the is_ip_interface flag in
 *               VLAN_OM_Dot1qVlanCurrentEntry_T for the purpose to identify which
 *               VLAN is IP interface;
 *            3. Restriction:
 *               a. Row status of management vlan cannot be suspended.
 *               b. Management VLAN must be based on static VLAN.  Dynamic vlan,
 *                  created by Dynamic GVRP, cannot be selected as a management vlan.
 *            4. To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetManagementVlan(UI32_T vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETMANAGEMENTVLAN;
    msg_p->data.arg_ui32 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetManagementVlan */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetGlobalManagementVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : Set the specified VLAN as global management VLAN
 * INPUT    : vid   - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. if vid is 0, it means just clear the global management VLAN
 *            2. the VLAN that is IP interface could be global management VLAN
 *            3. this function is paired with VLAN_MGR_GetManagementVlan()
 *            4. To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetGlobalManagementVlan(UI32_T vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETGLOBALMANAGEMENTVLAN;
    msg_p->data.arg_ui32 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetGlobalManagementVlan */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_LeaveManagementVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : Remove the IP interface label from the specified VLAN
 * INPUT    : vid - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. This function is coupled with VLAN_PMGR_SetIpInterface()
 *               and VLAN_MGR_GetNextIpInterface()
 *            2. To be phased out, please use VLAN_PMGR_SetVlanMgmtIpState() instead
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_LeaveManagementVlan(UI32_T vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_LEAVEMANAGEMENTVLAN;
    msg_p->data.arg_ui32 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_LeaveManagementVlan */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetIpInterface
 *-----------------------------------------------------------------------------
 * PURPOSE  : Label the specified VLAN as IP interface
 * INPUT    : vid   - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. When setting the IP Address, L2 product shall call this function and
 *               VLAN_MGR_SetGlobalManagementVlan(), and the L3 product shall just call
 *               this function
 *            2. Restrictions: IP interface VLAN must be based on static VLAN.  Dynamic vlan,
 *               cannot be set as IP interface.
 *            3. This function is coupled with VLAN_PMGR_LeaveManagementVlan()
 *               and VLAN_MGR_GetNextIpInterface()
 *            4. To be phased out, please use VLAN_PMGR_SetVlanMgmtIpState() instead
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetIpInterface(UI32_T vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETIPINTERFACE;
    msg_p->data.arg_ui32 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetIpInterface */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetVlanMgmtIpState
 * ----------------------------------------------------------------------------
 * PURPOSE  : Set management vlan state and ip interface state for a vlan.
 * INPUT    : vid        - the identifier of specified vlan
 *            mgmt_state - TRUE  -> set as management vlan
 *                         FALSE -> do nothing
 *            ip_state   - VLAN_MGR_IP_STATE_NONE (0)
 *                         VLAN_MGR_IP_STATE_IPV4 (1)
 *                         VLAN_MGR_IP_STATE_IPV6 (2)
 *                         VLAN_MGR_IP_STATE_UNCHANGED(3) => keep the original
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetVlanMgmtIpState(UI32_T vid, BOOL_T mgmt_state, UI8_T ip_state)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETVLANMGMTIPSTATE;
    msg_p->data.arg_grp8.arg1 = vid;
    msg_p->data.arg_grp8.arg2 = mgmt_state;
    msg_p->data.arg_grp8.arg3 = ip_state;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetVlanMgmtIpState */
/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_VlanChangeToL3Type
 * ----------------------------------------------------------------------------
 * PURPOSE  : Set Ip type  for a vlan.
 * INPUT    : vid        - the identifier of specified vlan
 * OUTPUT   :
 *            vid_ifindex - vlan interface index
 *            if_status   - vlan interface status
 *
 * VAL_ifOperStatus_up	1L
 * VAL_ifOperStatus_down	2L
 * RETURN   :
 *VLAN_MGR_RETURN_OK = 0,
 *VLAN_MGR_OPER_MODE_ERROR = 1,
 *VLAN_MGR_VALUE_OUT_OF_RANGE,
 *VLAN_MGR_OM_GET_ERROR
  * NOTES    : None
 * ----------------------------------------------------------------------------
 */
UI32_T VLAN_PMGR_VlanChangeToL3Type(UI32_T vid, UI32_T * vid_ifindex, UI32_T* if_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_CHANGEl2IF2L3IF;
    msg_p->data.arg_grp3.arg1 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    *vid_ifindex = msg_p->data.arg_grp3.arg2;
	*if_status = msg_p->data.arg_grp3.arg3;
    return msg_p->type.ret_ui32;
} /* End of VLAN_PMGR_SetVlanMgmtIpState */
/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_VlanChangeToL2Type
 * ----------------------------------------------------------------------------
 * PURPOSE  : UnSet ip type  for a vlan.
 * INPUT    : vid        - the identifier of specified vlan
 * OUTPUT   :
 * RETURN   :
 *VLAN_MGR_RETURN_OK = 0,
 *VLAN_MGR_OPER_MODE_ERROR = 1,
 *VLAN_MGR_VALUE_OUT_OF_RANGE,
 *VLAN_MGR_OM_GET_ERROR
  * NOTES    : None
 * ----------------------------------------------------------------------------
 */
UI32_T  VLAN_PMGR_VlanChangeToL2Type(UI32_T vid){
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_CHANGEl3IF2L2IF;
    msg_p->data.arg_ui32 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_ui32;
} /* End of VLAN_PMGR_SetVlanMgmtIpState */
/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_VlanLogicalMacChange
 * ----------------------------------------------------------------------------
 * PURPOSE  : UnSet ip type  for a vlan.
 * INPUT    : vid        - the identifier of specified vlan
 * OUTPUT   :
 * RETURN   :
 *VLAN_MGR_RETURN_OK = 0,
 *VLAN_MGR_OPER_MODE_ERROR = 1,
 *VLAN_MGR_VALUE_OUT_OF_RANGE,
 *VLAN_MGR_OM_GET_ERROR
  * NOTES    : None
 * ----------------------------------------------------------------------------
 */
UI32_T  VLAN_PMGR_VlanLogicalMacChange(UI32_T vid,UI8_T * vlan_mac){
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp11);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_L3VLANLOGICALMACCHANGE;
    msg_p->data.arg_grp11.arg1 = vid;
    memcpy(msg_p->data.arg_grp11.arg2,vlan_mac,SYS_ADPT_MAC_ADDR_LEN);
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_ui32;
} /* End of VLAN_PMGR_SetVlanMgmtIpState */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetVlanMac
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns the MAC address of the specific vlan.
 * INPUT    : vid_ifindex -- specify which vlan
 * OUTPUT   : *vlan_mac -- Mac address of the vlan
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_GetVlanMac(UI32_T vid_ifindex, UI8_T *vlan_mac)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp11);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETVLANMAC;
    msg_p->data.arg_grp11.arg1 = vid_ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(vlan_mac, msg_p->data.arg_grp11.arg2,
        sizeof(msg_p->data.arg_grp11.arg2));

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_GetVlanMac */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_NotifyForwardingState
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function updates port forwarding state information in relationship
 *            to vlan_id.
 * INPUT    : vid - the specific vlan this lport_ifindex joins.  0 for all VLANs.
 *            lport_ifindex - the specific port which its forwarding state information
 *                            has changed.
 *            port_tate - forwarding or Not Forwarding.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
void VLAN_PMGR_NotifyForwardingState(UI32_T vid, UI32_T lport_ifindex, BOOL_T port_state)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp19);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_NOTIFYFORWARDINGSTATE;
    msg_p->data.arg_grp19.arg1 = vid;
    msg_p->data.arg_grp19.arg2 = lport_ifindex;
    msg_p->data.arg_grp19.arg3 = port_state;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    return;
} /* End of VLAN_PMGR_NotifyForwardingState */

#if (SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetNativeVlanAgent
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the pvid of the specific port can be
 *            set successfully. Otherwise, return false.
 * INPUT    : lport_ifindex   -- the port number
 *            pvid            -- the pvid value
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The API prohibited from entering/leaving critical section.
 *            2. The API prohibited from calling SYSFUN_USE_CSC() and VLAN_MGR_RELEASE_CSC()
 *            3. The API will only call these funstions which performed critical section.
 *            4. The API should not be called to restore value by using "no" command.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetNativeVlanAgent(UI32_T lport_ifindex, UI32_T pvid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETNATIVEVLANAGENT;
    msg_p->data.arg_grp1.arg1 = lport_ifindex;
    msg_p->data.arg_grp1.arg2 = pvid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetNativeVlanAgent */

#endif /*(SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)*/

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_EnablePortDualMode
 * ----------------------------------------------------------------------------
 * PURPOSE  : Enable dual-mode and lport joins to untagged menber list.
 * INPUT    : lport             -- logical port number
 *                              -- the range of the value is [1..SYS_ADPT_TOTAL_NBR_OF_LPORT]
 *            vid               -- the vlan id
 * OUTPUT   : None
 * RETURN   : TRUE if successful, else FALSE if failed.
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_EnablePortDualMode(UI32_T lport, UI32_T vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_ENABLEPORTDUALMODE;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_EnablePortDualMode */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DisablePortDualMode
 * ----------------------------------------------------------------------------
 * PURPOSE  : Disable dual-mode and lport joins to tagged menber list.
 * INPUT    : lport             -- logical port number
 *                              -- the range of the value is [1..SYS_ADPT_TOTAL_NBR_OF_LPORT]
 * OUTPUT   : None
 * RETURN   : TRUE if successful, else FALSE if failed.
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DisablePortDualMode(UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DISABLEPORTDUALMODE;
    msg_p->data.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_DisablePortDualMode */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetRunningPortDualMode
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Get the dual-mode status and mapped vlan id for this lport.
 * INPUT    :   lport                       -- logical port number
 * OUTPUT   :   BOOL_T *dual_mode_status    --pointer of dual_mode_status.
 *              UI32_T *vid                 -- pointer of mapped vlan id.
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ----------------------------------------------------------------------------
 */
UI32_T VLAN_PMGR_GetRunningPortDualMode(UI32_T lport, BOOL_T *dual_mode_status, UI32_T *vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp12);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETRUNNINGPORTDUALMODE;
    msg_p->data.arg_grp12.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *dual_mode_status = msg_p->data.arg_grp12.arg2;
    *vid = msg_p->data.arg_grp12.arg3;

    return msg_p->type.ret_ui32;
} /* End of VLAN_PMGR_GetRunningPortDualMode */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetDot1qVlanStaticEntryAgent
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid       -- the specific vlan id.
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : True/FALSE
 * NOTES    : only for CLI/WEB/SNMP in Dual mode feature
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_GetDot1qVlanStaticEntryAgent(UI32_T vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp13);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    VLAN_MGR_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_VLAN,
                                 VLAN_PMGR_TRACEID_GETDOT1QVLANSTATICENTRYAGENT));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETDOT1QVLANSTATICENTRYAGENT;
    msg_p->data.arg_grp13.arg1 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *vlan_entry = msg_p->data.arg_grp13.arg2;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
} /* End of VLAN_PMGR_GetDot1qVlanStaticEntryAgent */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetDot1qVlanCurrentEntryAgent
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : only for CLI/WEB/SNMP in Dual mode feature
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_GetDot1qVlanCurrentEntryAgent(UI32_T time_mark, UI32_T vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp14);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    VLAN_MGR_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_VLAN,
                                 VLAN_PMGR_TRACEID_GETDOT1QVLANCURRENTENTRYAGENT));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETDOT1QVLANCURRENTENTRYAGENT;
    msg_p->data.arg_grp14.arg1 = time_mark;
    msg_p->data.arg_grp14.arg2 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *vlan_entry = msg_p->data.arg_grp14.arg3;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
} /* End of VLAN_PMGR_GetDot1qVlanCurrentEntryAgent */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_GetNextDot1qVlanCurrentEntryAgent
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : only for CLI/WEB/SNMP in Dual mode feature
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_GetNextDot1qVlanCurrentEntryAgent(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp14);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    VLAN_MGR_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_VLAN,
                                 VLAN_PMGR_TRACEID_GETNEXTDOT1QVLANCURRENTENTRYAGENT));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETNEXTDOT1QVLANCURRENTENTRYAGENT;
    msg_p->data.arg_grp14.arg1 = time_mark;
    msg_p->data.arg_grp14.arg2 = *vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *vid = msg_p->data.arg_grp14.arg2;
    *vlan_entry = msg_p->data.arg_grp14.arg3;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
} /* End of VLAN_PMGR_GetNextDot1qVlanCurrentEntryAgent */

#endif /* SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DeleteNormalVlanAgent
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan that is not private
 *            vlan is successfully deleted from the database.
 *            Otherwise, false is returned.
 * INPUT    : vid   -- the existed vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The API prohibited from entering/leaving critical section.
 *            2. The API prohibited from calling SYSFUN_USE_CSC() and VLAN_MGR_RELEASE_CSC()
 *            3. The API will only call these funstions which performed critical section.
 *            4. The API should not be called to restore value by using "no" command.
 *            5. VLAN should be able to deleted even though it's being used by PVID on the port.
 *               -- Add port to member list of default vlan.
 *               -- Set pvid to default vlan.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DeleteNormalVlanAgent(UI32_T vid, UI32_T vlan_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DELETENORMALVLANAGENT;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = vlan_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_DeleteNormalVlanAgent */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_AddEgressPortMemberAgent
 * ----------------------------------------------------------------------------
 * PURPOSE  : lport joins to egress menber list.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- logical port number
 *            vlan_status   -- VAL_dot1qVlanStatus_other \
 *                             VAL_dot1qVlanStatus_permanent \
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 *            tagged        -- TRUE[tagged member]
 *                             FALSE[untagged member]
 * OUTPUT   : None
 * RETURN   : TRUE if successful, else FALSE if failed.
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_AddEgressPortMemberAgent(UI32_T vid, UI32_T lport, UI32_T vlan_status, BOOL_T tagged)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp18);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_ADDEGRESSPORTMEMBERAGENT;
    msg_p->data.arg_grp18.arg1 = vid;
    msg_p->data.arg_grp18.arg2 = lport;
    msg_p->data.arg_grp18.arg3 = vlan_status;
    msg_p->data.arg_grp18.arg4 = tagged;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_AddEgressPortMemberAgent */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DeleteEgressPortMemberAgent
 * ----------------------------------------------------------------------------
 * PURPOSE  : lport removes from egress menber list.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- logical port number
 *            vlan_status   -- VAL_dot1qVlanStatus_other \
 *                             VAL_dot1qVlanStatus_permanent \
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 *            tagged        -- TRUE[tagged member]
 *                             FALSE[untagged member]
 * OUTPUT   : None
 * RETURN   : TRUE if successful, else FALSE if failed.
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DeleteEgressPortMemberAgent(UI32_T vid, UI32_T lport, UI32_T vlan_status, BOOL_T tagged)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp18);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DELETEEGRESSPORTMEMBERAGENT;
    msg_p->data.arg_grp18.arg1 = vid;
    msg_p->data.arg_grp18.arg2 = lport;
    msg_p->data.arg_grp18.arg3 = vlan_status;
    msg_p->data.arg_grp18.arg4 = tagged;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_DeleteEgressPortMemberAgent */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetAuthorizedVlanList
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the authorized vlan list is set
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex     -- lport ifindex
 *            pvid              -- the authorized pvid
 *            tagged_vlist      -- the authorized tagged vlan list
 *            untagged_vlist    -- the authorized untagged vlan list
 *            is_guest          -- whether the port is assigned to Guest VLAN
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : vlan list for this port   | PVID                            | tagged-list   | untagged-list
 *            ---------------------------------------------------------------------------------------------
 *            both tagged and untagged    one member in untagged-list       tagged-list     untagged-list
 *            no tagged, no untagged      VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID  0               0
 *            untagged only               one member in untagged-list       0               untagged-list
 *            tagged only(no PVID)        VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID  tagged-list     0
 *            tagged only(with PVID)      one member in tagged-list         tagged-list     0
 *            restore to default          VLAN_TYPE_DOT1Q_NULL_VLAN_ID      0               0
 *--------------------------------------------------------------------------*/
BOOL_T  VLAN_PMGR_SetAuthorizedVlanList(UI32_T lport_ifindex, UI32_T pvid, VLAN_OM_VLIST_T *tagged_vlist, VLAN_OM_VLIST_T *untagged_vlist, BOOL_T is_guest)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_vlist_vlist_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETAUTHORIZEDVLANLIST;
    msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_ui32_1 = lport_ifindex;
    msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_ui32_2 = pvid;
    msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_vlist_1 = tagged_vlist;
    msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_vlist_2 = untagged_vlist;
    msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_bool = is_guest;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetAuthorizedVlanList */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetGlobalDefaultVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : vid
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetGlobalDefaultVlan(UI32_T vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETGLOBALDEFAULTVLAN;
    msg_p->data.arg_ui32 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetGlobalDefaultVlan */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetVoiceVlanId
 * ----------------------------------------------------------------------------
 * PURPOSE : Set or reset the Voice VLAN ID
 * INPUT   : vvid - voice VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : Set vvid = 0 to reset
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetVoiceVlanId(UI32_T vvid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETVOICEVLANID;
    msg_p->data.arg_ui32 = vvid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetVoiceVlanId */

BOOL_T VLAN_PMGR_GetVLANMemberByLport(UI32_T lport_ifindex,UI32_T *number,UI8_T *vlan_list)
{
     const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp20);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    memset(ipc_buf,0,sizeof(ipc_buf));
    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GETVLANMEMBERBYLPORT;
    msg_p->data.arg_grp20.arg1 = lport_ifindex;


    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
	  memcpy( vlan_list,msg_p->data.arg_grp20.arg2,
		  sizeof(msg_p->data.arg_grp20.arg2));
	  *number = msg_p->data.arg_grp20.arg3 ;

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetGlobalDefaultVlan */

BOOL_T VLAN_PMGR_GetPortlistByVid(UI32_T vid,UI32_T * number,UI8_T *portlist)
{

    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    memset(ipc_buf,0,sizeof(ipc_buf));
    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_GEtPORTLISTBYVID;
    msg_p->data.arg_grp4.arg1 = vid;


    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
	  memcpy( portlist,msg_p->data.arg_grp4.arg2,
		  sizeof(msg_p->data.arg_grp4.arg2));
	  *number = msg_p->data.arg_grp4.arg3 ;

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetGlobalDefaultVlan */

#if (SYS_CPNT_MAC_VLAN == TRUE)


/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC VLAN entry
 * INPUT    : mac_address_p   - only allow unitcast address
 *                            use 0 to get the first entry
 *            vid           - the VLAN ID
 *                            the valid value is 1 ~ SYS_DFLT_DOT1QMAXVLANID
 *            priority      - the priority
 *                            the valid value is 0 ~ 7
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : if SYS_CPNT_MAC_VLAN_WITH_PRIORITY == FALSE, it's recommanded
 *            that set input priority to 0.
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_SetMacVlanEntry(UI8_T *mac_address_p, UI8_T *mask_p, UI16_T vid, UI8_T priority)
{
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_mac_vid_pri);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    memset(ipc_buf,0,sizeof(ipc_buf));
    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETMACVLANENTRY;
    memcpy(msg_p->data.arg_mac_vid_pri.arg_mac, mac_address_p, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(msg_p->data.arg_mac_vid_pri.arg_mask, mask_p, SYS_ADPT_MAC_ADDR_LEN);
    msg_p->data.arg_mac_vid_pri.arg_vid = vid;
    msg_p->data.arg_mac_vid_pri.arg_pri = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DeleteMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete MAC VLAN entry
 * INPUT    : mac_address_p   - only allow unitcast address
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DeleteMacVlanEntry(UI8_T *mac_address_p, UI8_T *mask_p)
{
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_mac_vid_pri);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    memset(ipc_buf,0,sizeof(ipc_buf));
    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DELETEMACVLANENTRY;
    memcpy(msg_p->data.arg_mac_vid_pri.arg_mac, mac_address_p, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(msg_p->data.arg_mac_vid_pri.arg_mask, mask_p, SYS_ADPT_MAC_ADDR_LEN);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_DeleteAllMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete all MAC VLAN entry
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_PMGR_DeleteAllMacVlanEntry(void)
{
    const UI32_T msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    memset(ipc_buf,0,sizeof(ipc_buf));
    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_DELETEALLMACVLANENTRY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}
#endif /*end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_PMGR_SetPortVlanList
 *-----------------------------------------------------------------------------
 * PURPOSE : Set the VLAN membership as the given VLAN list for a logical port.
 * INPUT   : lport       - the specified logical port
 *           vlan_list_p - pointer to list of VLAN IDs for the lport to set
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : 1. all untagged for access/hybrid mode, all tagged for trunk mode
 *           2. fail if remove from PVID VLAN for access/hybrid mode
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_PMGR_SetPortVlanList(UI32_T lport, UI8_T *vlan_list_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = VLAN_MGR_GET_MSG_SIZE(arg_ui32_vlist);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    VLAN_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_VLAN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VLAN_MGR_IPC_SETPORTVLANLIST;
    msg_p->data.arg_ui32_vlist.arg_ui32 = lport;
    memcpy(msg_p->data.arg_ui32_vlist.arg_vlist, vlan_list_p,
        VLAN_TYPE_VLAN_LIST_SIZE);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            VLAN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of VLAN_PMGR_SetPortVlanList */

/* LOCAL SUBPROGRAM BODIES
 */
