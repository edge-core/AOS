/*-----------------------------------------------------------------------------
 * MODULE NAME: TRK_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    None.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/06/29     --- Timon, Create
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
#include "sysfun.h"
#include "trk_mgr.h"
#include "trk_pmgr.h"


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
 * ROUTINE NAME : TRK_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for TRK_PMGR in the calling process.
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
BOOL_T TRK_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for TRK MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of TRK_PMGR_InitiateProcessResource */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_CreateTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a trunking port
 * INPUT   : trunk_id -- which trunking port to create
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_CreateTrunk(UI32_T trunk_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_CREATETRUNK;
    msg_p->data.arg_ui32 = trunk_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_CreateTrunk */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_DestroyTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will destroy a trunking port
 * INPUT   : trunk_id -- which trunking port to destroy
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_DestroyTrunk(UI32_T trunk_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_DESTROYTRUNK;
    msg_p->data.arg_ui32 = trunk_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_DestroyTrunk */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_CreateDynamicTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will allocate(create) a dynamic trunk
 * INPUT   : trunk_id -- The dynamic
 * OUTPUT  : None.
 * RETURN  : TRUE  -- 1. This trunk is dynamic already.
 *                    2. This trunk is created as dynamic trunk.
 *           FALSE -- 1. This trunk is static trunk already.
 *                    2. This trunk cannot be created.
 * NOTE    : for LACP.
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_CreateDynamicTrunk(UI32_T trunk_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_CREATEDYNAMICTRUNK;
    msg_p->data.arg_ui32 = trunk_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_CreateDynamicTrunk */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_FreeTrunkIdDestroyDynamic
 * -------------------------------------------------------------------------
 * FUNCTION: This function will free(destroy) a dynamic trunk
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_FreeTrunkIdDestroyDynamic(UI32_T trunk_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_FREETRUNKIDDESTROYDYNAMIC;
    msg_p->data.arg_ui32 = trunk_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_FreeTrunkIdDestroyDynamic */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_AddTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a trunking port
 * INPUT   : trunk_id -- which trunking port to add member
 *           unit     -- which unit to add
 *           port     -- which port to add
 * OUTPUT  : None
 * RETURN  : One of TRK_MGR_TRUNK_Error_E
 * NOTE    : Notice the return value
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_AddTrunkMember(UI32_T trunk_id, UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_ADDTRUNKMEMBER;
    msg_p->data.arg_grp_ui32_ui32.arg_1 = trunk_id;
    msg_p->data.arg_grp_ui32_ui32.arg_2 = ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return TRK_MGR_OTHER_WRONG;
    }

    return msg_p->type.ret_ui32;
} /* End of TRK_PMGR_AddTrunkMember */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_AddDynamicTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a member to a trunking port
 * INPUT   : trunk_id -- which trunking port to add member
 *           ifindex  -- which unit/port to add
 *           is_active_member -- active or inactive member
 * OUTPUT  : None
 * RETURN  : One of TRK_MGR_TRUNK_Error_E
 * NOTE    : Notice the return value, for LACP
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_AddDynamicTrunkMember(UI32_T trunk_id,UI32_T ifindex, BOOL_T is_active_member)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_ADDDYNAMICTRUNKMEMBER;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_1 = trunk_id;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_2 = ifindex;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_3 = is_active_member;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return TRK_MGR_OTHER_WRONG;
    }

    return msg_p->type.ret_ui32;
} /* End of TRK_PMGR_AddDynamicTrunkMember */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_DeleteTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a member from a trunking port
 * INPUT   : trunk_id -- which trunking port to delete member
 *           unit     -- which unit to delete
 *           port     -- which port to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_DeleteTrunkMember(UI32_T trunk_id, UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_DELETETRUNKMEMBER;
    msg_p->data.arg_grp_ui32_ui32.arg_1 = trunk_id;
    msg_p->data.arg_grp_ui32_ui32.arg_2 = ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_DeleteTrunkMember */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_DeleteDynamicTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a member from a trunking port
 * INPUT   : trunk_id -- which trunking port to delete member
 *           unit     -- which unit to delete
 *           port     -- which port to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_DeleteDynamicTrunkMember(UI32_T trunk_id, UI32_T ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_DELETEDYNAMICTRUNKMEMBER;
    msg_p->data.arg_grp_ui32_ui32.arg_1 = trunk_id;
    msg_p->data.arg_grp_ui32_ui32.arg_2 = ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_DeleteDynamicTrunkMember */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_SetTrunkName
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the name of a specific trunk
 * INPUT   : trunk_id -- which trunking port to set
 *           name     -- the name of this trunk
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_SetTrunkName(UI32_T trunk_id, UI8_T *name)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_trunk_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_SETTRUNKNAME;
    msg_p->data.arg_grp_trunk_name.arg_id = trunk_id;
    memcpy(msg_p->data.arg_grp_trunk_name.arg_name, name,
        sizeof(msg_p->data.arg_grp_trunk_name.arg_name));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_SetTrunkName */
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

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_SetTrunkAlias
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the name of a specific trunk
 * INPUT   : trunk_id -- which trunking port to set
 *           alias    -- the alias of this trunk
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_SetTrunkAlias(UI32_T trunk_id, UI8_T *alias)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_trunk_name);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_SETTRUNKALIAS;
    msg_p->data.arg_grp_trunk_name.arg_id = trunk_id;
    memcpy(msg_p->data.arg_grp_trunk_name.arg_name, alias,
        sizeof(msg_p->data.arg_grp_trunk_name.arg_name));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_IsDynamicTrunkId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the trunk is a dynamic trunk  or not
 * INPUT   : None
 * OUTPUT  : trunk_id -- dynamic trunk id
 * RETURN  : True: Dynamic, False: If not available
 * NOTE    : for LACP
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_IsDynamicTrunkId(UI32_T trunk_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_ISDYNAMICTRUNKID;
    msg_p->data.arg_ui32 = trunk_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_IsDynamicTrunkId */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetTrunkMemberCounts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return total trunk member numbers
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : total trunk member number
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_GetTrunkMemberCounts(UI32_T trunk_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_GETTRUNKMEMBERCOUNTS;
    msg_p->data.arg_ui32 = trunk_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return 0;
    }

    return msg_p->type.ret_ui32;
} /* End of TRK_PMGR_GetTrunkMemberCounts */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetTrunkCounts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return total trunk numbers which are created
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : total created trunk number
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_GetTrunkCounts(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(TRK_MGR_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_GETTRUNKCOUNTS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return 0;
    }

    return msg_p->type.ret_ui32;
} /* End of TRK_PMGR_GetTrunkCounts */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetNextTrunkId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the next available trunk ID
 * INPUT   : trunk_id -- the key to get
 * OUTPUT  : trunk_id -- from 0 to SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetNextTrunkId(UI32_T *trunk_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_GETNEXTTRUNKID;
    msg_p->data.arg_ui32 = *trunk_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *trunk_id = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_GetNextTrunkId */

/*----------------------------------------------------------------------*/
/* (trunkMgt 1)--ES3626A */
/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetTrunkMaxId
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the maximum number for a trunk identifier
 * INPUT   : None
 * OUTPUT  : trunk_max_id
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 1
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetTrunkMaxId(UI32_T *trunk_max_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_GETTRUNKMAXID;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *trunk_max_id = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_GetTrunkMaxId */

/*----------------------------------------------------------------------*/
/* (trunkMgt 2)--ES3626A */
/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetTrunkValidNumber
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the number of valid trunks
 * INPUT   : None
 * OUTPUT  : trunk_max_id
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 2
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetTrunkValidNumber(UI32_T *trunk_valid_numer)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_GETTRUNKVALIDNUMBER;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *trunk_valid_numer = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_GetTrunkValidNumber */

/*----------------------------------------------------------------------*/
/* (trunkMgt 3)--ES3626A */
/*
 *      INDEX       { trunkIndex }
 *      TrunkEntry ::= SEQUENCE
 *      {
 *          trunkIndex                Integer32,
 *          trunkPorts                PortList,
 *          trunkCreation             INTEGER,
 *          trunkStatus               INTEGER
 *      }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the trunk table entry info
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The trunk entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_trunk_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_GETTRUNKENTRY;
    msg_p->data.arg_trunk_entry = *trunk_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *trunk_entry = msg_p->data.arg_trunk_entry;

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_GetTrunkEntry */

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetNextTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next trunk table entry info
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The next trunk entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_GetNextTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_trunk_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_GETNEXTTRUNKENTRY;
    msg_p->data.arg_trunk_entry = *trunk_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *trunk_entry = msg_p->data.arg_trunk_entry;

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_GetNextTrunkEntry */

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetNextRunningTrunkEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next trunk entry of running config
 * INPUT   : trunk_entry->trunk_index - trunk id
 * OUTPUT  : trunk_entry              - The next trunk entry
 * RETURN  : One of SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : trunk_id = 0 ==> get the first trunk (exclude dynamic trunk)
 *------------------------------------------------------------------------
 */
UI32_T  TRK_PMGR_GetNextRunningTrunkEntry(TRK_MGR_TrunkEntry_T *trunk_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_trunk_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_GETNEXTRUNNINGTRUNKENTRY;
    msg_p->data.arg_trunk_entry = *trunk_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *trunk_entry = msg_p->data.arg_trunk_entry;

    return msg_p->type.ret_ui32;
} /* End of TRK_PMGR_GetNextRunningTrunkEntry */

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_SetTrunkPorts
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the trunk port list
 * INPUT   : trunk_id                       - trunk id
 *           trunk_portlist                 - trunk port list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. ES3626A MIB/trunkMgt 3
 *           2. For trunk_portlist, only the bytes of in the range of user
 *              port will be handle.
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_SetTrunkPorts(UI32_T trunk_id, UI8_T *trunk_portlist)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_trunk_portlist);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_SETTRUNKPORTS;
    msg_p->data.arg_grp_trunk_portlist.arg_id = trunk_id;

    if (trunk_portlist == NULL)
    {
        return FALSE;
    }
    memcpy(msg_p->data.arg_grp_trunk_portlist.arg_portlist, trunk_portlist,
        sizeof(msg_p->data.arg_grp_trunk_portlist.arg_portlist));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_SetTrunkPorts */

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_SetTrunkStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the trunk status
 * INPUT   : trunk_id                       - trunk id
 *           trunk_status                   - VAL_trunkStatus_valid
 *                                            VAL_trunkStatus_invalid
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/trunkMgt 3
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_SetTrunkStatus(UI32_T trunk_id, UI8_T trunk_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_SETTRUNKSTATUS;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = trunk_id;
    msg_p->data.arg_grp_ui32_ui8.arg_ui8 = trunk_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_SetTrunkStatus */

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_IsTrunkExist
 *------------------------------------------------------------------------
 * FUNCTION: Does this trunk exist or not.
 * INPUT   : trunk_id  -- trunk id
 * OUTPUT  : is_static -- TRUE/FASLE
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *------------------------------------------------------------------------
 */
BOOL_T TRK_PMGR_IsTrunkExist(UI32_T trunk_id, BOOL_T *is_static)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRK_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_ISTRUNKEXIST;

    if (is_static == NULL)
    {
        return FALSE;
    }
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = trunk_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *is_static = msg_p->data.arg_grp_ui32_bool.arg_bool;

    return msg_p->type.ret_bool;
} /* End of TRK_PMGR_IsTrunkExist */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRK_PMGR_GetLastChangeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the last change time of whole system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the time of last change of any port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
UI32_T TRK_PMGR_GetLastChangeTime(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(TRK_MGR_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRK_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRUNK;
    msgbuf_p->msg_size = TRK_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (TRK_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRK_MGR_IPC_GETLASTCHANGETIME;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            TRK_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return 0;
    }

    return msg_p->type.ret_ui32;
} /* End of TRK_PMGR_GetLastChangeTime */


/* LOCAL SUBPROGRAM BODIES
 */
