/*-----------------------------------------------------------------------------
 * FILE NAME: XSTP_POM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for XSTP OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/23     --- Timon, Create
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
#include "l_mm.h"
#include "sysfun.h"
#include "xstp_om.h"
#include "xstp_pom.h"
#include "xstp_type.h"
#include "xstp_om_private.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* trace id definition when using L_MM
 */
enum
{
    XSTP_POM_TRACEID_GETMSTPINSTANCEVLANMAPPED,
    XSTP_POM_TRACEID_GETMSTPINSTANCEVLANMAPPEDFORMSB,
    XSTP_POM_TRACEID_GETNEXTMSTPINSTANCEVLANMAPPED,
    XSTP_POM_TRACEID_GETNEXTMSTPINSTANCEVLANMAPPEDFORMSB,
    XSTP_POM_TRACEID_GETMSTPINSTANCEVLANCONFIGURATION,
    XSTP_POM_TRACEID_GETMSTPINSTANCEVLANCONFIGURATIONFORMSB,
    XSTP_POM_TRACEID_GETRUNNINGMSTPINSTANCEVLANCONFIGURATION,
    XSTP_POM_TRACEID_GETNEXTMSTPINSTANCEVLANCONFIGURATION,
    XSTP_POM_TRACEID_GETNEXTMSTPINSTANCEVLANCONFIGURATIONFORMSB
};
#define XSTP_POM_InstanceInfo shmem_data_p->XSTP_OM_InstanceInfo
#define XSTP_POM_Mst_Configuration_Table shmem_data_p->XSTP_OM_Mst_Configuration_Table
#define XSTP_POM_SystemInfo shmem_data_p->XSTP_OM_SystemInfo
#define XSTP_POM_InstanceEntryIndex shmem_data_p->XSTP_OM_InstanceEntryIndex
#define XSTP_POM_PortCommonInfo shmem_data_p->XSTP_OM_PortCommonInfo


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
 * FUNCTION NAME - XSTP_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for XSTP_POM in the calling process.
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
BOOL_T XSTP_POM_InitiateProcessResource(void)
{
    /* get the ipc message queues for XSTP OM
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): L_IPCMSGQ_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of XSTP_POM_InitiateProcessResource */
static UI32_T xstp_om_sem_id;
static XSTP_OM_SHARE_T *shmem_data_p;

void XSTP_SHARE_OM_InitiateSystemResources(void)
{
    shmem_data_p = (XSTP_OM_SHARE_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_XSTP_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_XSTP_OM, &xstp_om_sem_id);
}


void XSTP_SHARE_OM_AttachSystemResources(void)
{
    shmem_data_p = (XSTP_OM_SHARE_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_XSTP_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_XSTP_OM, &xstp_om_sem_id);
}


void XSTP_SHARE_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_XSTP_SHMEM_SEGID;
    *seglen_p = sizeof(XSTP_OM_SHARE_T);
}

/* ===================================================================== */
/* System Information function
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetForceVersion
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the system force version
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : force version
 *-------------------------------------------------------------------------
 */
UI8_T   XSTP_POM_GetForceVersion(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETFORCEVERSION;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui8;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMaxHopCount
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the max_hop_count
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : max_hop_count
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetMaxHopCount(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMAXHOPCOUNT;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMaxInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the max_instance_number
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : max_instance_number
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetMaxInstanceNumber(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMAXINSTANCENUMBER;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNumOfActiveTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the num_of_active_tree
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : num_of_active_tree
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetNumOfActiveTree(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNUMOFACTIVETREE;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRegionName
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the region name
 * INPUT    : None
 * OUTPUT   : str       -- region name
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_POM_GetRegionName(UI8_T *str)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ar1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETREGIONNAME;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    memcpy(str, msg_p->data.arg_ar1, sizeof(msg_p->data.arg_ar1));

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRegionRevision
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the region_revision
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : region_revision
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetRegionRevision(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETREGIONREVISION;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the spanning tree status
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : force version
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetSpanningTreeStatus(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETSPANNINGTREESTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTrapFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_tc
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_tc
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetTrapFlagTc(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETTRAPFLAGTC;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTrapFlagNewRoot
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_new_root
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_new_root
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetTrapFlagNewRoot(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETTRAPFLAGNEWROOT;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstidFromMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_POM_GetMstidFromMstConfigurationTableByVlan(UI32_T vid, UI32_T *mstid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTIDFROMMSTCONFIGURATIONTABLEBYVLAN;
    msg_p->data.arg_grp1.arg1 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    *mstid = msg_p->data.arg_grp1.arg2;

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextXstpMemberFromMstConfigurationTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next XSTP member form mst config table for a
 *            specified instance
 * INPUT    : mstid     -- this instance value
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetNextXstpMemberFromMstConfigurationTable(UI32_T mstid,
                                                            UI32_T *vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTXSTPMEMBERFROMMSTCONFIGURATIONTABLE;
    msg_p->data.arg_grp1.arg1 = mstid;
    msg_p->data.arg_grp1.arg2 = *vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *vid = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsMstFullMemberTopology
 * ------------------------------------------------------------------------
 * PURPOSE  : This function returns TRUE if mst_topology_method is full member topology.
 *            Otherwise, return FALSE.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 *-------------------------------------------------------------------------
 */
BOOL_T   XSTP_POM_IsMstFullMemberTopology(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_ISMSTFULLMEMBERTOPOLOGY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetCurrentCfgInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the current instance number created by user.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : num_of_cfg_msti
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetCurrentCfgInstanceNumber(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_OM_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETCURRENTCFGINSTANCENUMBER;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetInstanceEntryId
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the entry_id for the specified xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : None
 * RETUEN   : entry_id (stg_id) for the specified xstid
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI8_T   XSTP_POM_GetInstanceEntryId(UI32_T xstid)
{
    return XSTP_POM_InstanceEntryIndex[xstid];
#if 0
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETINSTANCEENTRYID;
    msg_p->data.arg_ui32 = xstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui8;
#endif
}

/*=============================================================================
 * Move from xstp_svc.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   U32_T  *state   -- the pointer of state value
 *                                 VAL_dot1dStpPortState_blocking
 *                                 VAL_dot1dStpPortState_listening
 *                                 VAL_dot1dStpPortState_learning
 *                                 VAL_dot1dStpPortState_forwarding
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */

#if (SYS_CPNT_EAPS == TRUE)
#define XSTP_OM_EthRingProle    shmem_data_p->XSTP_OM_EthRingProle

/* should be the same as XSTP_OM_GetEthRingPortRole
 * duplicated here for performance
 */
BOOL_T XSTP_POM_GetEthRingPortRole_Local(
    UI32_T  lport,
    UI32_T  *port_role_p)
{
    BOOL_T  ret = FALSE;

    if (  (NULL != port_role_p)
        &&(1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
       )
    {
        *port_role_p = XSTP_OM_EthRingProle[lport-1];
        ret = TRUE;
    }

    return ret;
}

/* should be the same as XSTP_OM_GetEthRingPortStatus
 * duplicated here for performance
 */
BOOL_T  XSTP_POM_GetEthRingPortStatus_Local(UI32_T lport, UI32_T vid, BOOL_T *is_blk_p)
{
    BOOL_T  ret = FALSE;

    {
        UI32_T  rp_role;

        if (TRUE == XSTP_POM_GetEthRingPortRole_Local(lport, &rp_role))
        {
            switch (rp_role)
            {
#if (SYS_CPNT_EAPS == TRUE)
            case XSTP_TYPE_ETH_RING_PORT_ROLE_EAPS:
                *is_blk_p = EAPS_OM_IsPortBlockingByVLAN(vid, lport);
                ret = TRUE;
                break;
#endif

            default:
                break;
            }
        }
    }

    return ret;
}
#endif /* #if (SYS_CPNT_EAPS == TRUE) */

static void XSTP_POM_GetMstidFromMstConfigurationTableByVlan_Local(UI32_T vid, UI32_T *mstid)
{
    UI8_T   odd_byte, even_byte;

    *mstid       =   0;
    if (    (vid > 0)
        &&  (vid <= SYS_DFLT_DOT1QMAXVLANID )
       )
    {
        even_byte   =   XSTP_POM_Mst_Configuration_Table[2*vid];
        odd_byte    =   XSTP_POM_Mst_Configuration_Table[2*vid+1];

        *mstid = (((*mstid | even_byte) << 8 ) | odd_byte );
    }

    return;
}

static XSTP_OM_InstanceData_T*   XSTP_POM_GetInstanceInfoPtr1(UI32_T xstid)
{
    return  (&(XSTP_POM_InstanceInfo[XSTP_POM_InstanceEntryIndex[xstid]]));
} /* End of XSTP_OM_GetInstanceInfoPtr */
static BOOL_T XSTP_POM_GetPortStateByInstance1(UI32_T xstid, UI32_T lport, UI32_T *state)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_InstanceData_T  *cist_om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_OM_PortVar_T       *cist_pom_ptr;

    BOOL_T                  result;

    if (xstid > XSTP_TYPE_MAX_MSTID)
    {
        return FALSE;
    }
    if (    (lport == 0)
        ||  (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
       )
    {
        return FALSE;
    }
    /*To now , I just support the cist parent index is right. Tony.Lei*/
    cist_om_ptr  = XSTP_POM_GetInstanceInfoPtr1(XSTP_TYPE_CISTID);

    om_ptr  = XSTP_POM_GetInstanceInfoPtr1(xstid);

    cist_pom_ptr = &(cist_om_ptr->port_info[lport-1]);

    /* convert to trunk ifindex if it's a trunk member port
     */
    if(cist_pom_ptr->parent_index > 0 ){
        lport = cist_pom_ptr->parent_index;
    }

/* trunk member ifindex must be convered into trunk ifindex,
 * before checking the ring port status
 */
#if (SYS_CPNT_EAPS == TRUE)
    /* get port status for ethernet ring protocol, if it's a ring port
     */
    {
        BOOL_T  is_blk;

        if (TRUE == XSTP_POM_GetEthRingPortStatus_Local(lport, 0, &is_blk))
        {
            if (TRUE == is_blk)
                *state = VAL_dot1dStpPortState_blocking;
            else
                *state = VAL_dot1dStpPortState_forwarding;
            return TRUE;
        }
    }
#endif /* #if (SYS_CPNT_EAPS == TRUE) */

    pom_ptr = &(om_ptr->port_info[lport -1]);

    *state = (UI32_T)VAL_dot1dStpPortState_blocking;

    result  = TRUE;

    if (XSTP_POM_PortCommonInfo[lport - 1].port_enabled)
    {
        if (pom_ptr->learning && !pom_ptr->forwarding )
        {
            *state = (UI32_T)VAL_dot1dStpPortState_learning;
        }
        else if (pom_ptr->learning && pom_ptr->forwarding )
        {
            *state = (UI32_T)VAL_dot1dStpPortState_forwarding;
        }
        else if (!pom_ptr->learning && pom_ptr->forwarding)
        {
            result  = FALSE;
        }
    } /* End of if (pom_ptr->common->port_enabled) */

    return  result;
} /* End of XSTP_OM_GetPortStateByInstance */
BOOL_T  XSTP_POM_GetPortStateByVlan(UI32_T vid, UI32_T lport, UI32_T *state)
{
#if 0
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTSTATEBYVLAN;
    msg_p->data.arg_grp2.arg1 = vid;
    msg_p->data.arg_grp2.arg2 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *state = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_bool;
#else
    UI32_T  xstid;
    UI32_T  current_st_status;
    UI32_T  current_st_mode;

    current_st_status   = XSTP_POM_SystemInfo.spanning_tree_status;
    current_st_mode     = (UI32_T)XSTP_POM_SystemInfo.force_version;

    if (    (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_POM_GetMstidFromMstConfigurationTableByVlan_Local(vid, &xstid);
    }
    else
    {
        xstid = XSTP_TYPE_CISTID;
    }
    return XSTP_POM_GetPortStateByInstance1(xstid, lport, state);

#endif
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T Xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   U32_T  *state   -- the pointer of state value
 *                                 VAL_dot1dStpPortState_blocking
 *                                 VAL_dot1dStpPortState_listening
 *                                 VAL_dot1dStpPortState_learning
 *                                 VAL_dot1dStpPortState_forwarding
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPortStateByInstance(UI32_T xstid, UI32_T lport, UI32_T *state)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTSTATEBYINSTANCE;
    msg_p->data.arg_grp2.arg1 = xstid;
    msg_p->data.arg_grp2.arg2 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *state = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsPortForwardingStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in VAL_dot1dStpPortState_forwarding,
 *              else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_IsPortForwardingStateByVlan(UI32_T vid, UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_ISPORTFORWARDINGSTATEBYVLAN;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsPortForwardingStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in VAL_dot1dStpPortState_forwarding,
 *              else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_IsPortForwardingStateByInstance(UI32_T xstid, UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_ISPORTFORWARDINGSTATEBYINSTANCE;
    msg_p->data.arg_grp1.arg1 = xstid;
    msg_p->data.arg_grp1.arg2 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsPortBlockingStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Blocking, else FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_IsPortBlockingStateByVlan(UI32_T vid, UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_ISPORTBLOCKINGSTATEBYVLAN;
    msg_p->data.arg_grp1.arg1 = vid;
    msg_p->data.arg_grp1.arg2 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsPortBlockingStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Blocking, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_IsPortBlockingStateByInstance(UI32_T xstid, UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_ISPORTBLOCKINGSTATEBYINSTANCE;
    msg_p->data.arg_grp1.arg1 = xstid;
    msg_p->data.arg_grp1.arg2 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextExistingMstidByLport
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next mst instance id with a specified lport.
 * INPUT    :   UI32_T lport    -- lport number
 * OUTPUT   :   UI32_T *xstid   -- mst instance id
 * OUTPUT   :   UI32_T *xstid   -- next mst instance id
 * RETURN   :   TRUE if the next mstid is existing, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetNextExistingMstidByLport(UI32_T lport, UI32_T *xstid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTEXISTINGMSTIDBYLPORT;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = *xstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *xstid = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextExistingMemberVidByMstid
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next member vlan id with a specified mst instance id.
 * INPUT    :   UI32_T xstid    -- mst instance id
 * OUTPUT   :   UI32_T *vid     -- vlan id
 * OUTPUT   :   UI32_T *vid     -- next member vlan id
 * RETURN   :   TRUE if the next member vlan is existing, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetNextExistingMemberVidByMstid(UI32_T xstid, UI32_T *vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTEXISTINGMEMBERVIDBYMSTID;
    msg_p->data.arg_grp1.arg1 = xstid;
    msg_p->data.arg_grp1.arg2 = *vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *vid = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstInstanceIndexByMstid
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the entry_id for the specified xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : mstudx -- MST instance INDEX
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T   XSTP_POM_GetMstInstanceIndexByMstid(UI32_T xstid, UI32_T *mstidx)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTINSTANCEINDEXBYMSTID;
    msg_p->data.arg_grp1.arg1 = xstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mstidx = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/*=============================================================================
 * Move from xstp_mgr.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningStaSystemStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global spanning tree status.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningSystemSpanningTreeStatus(UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGSYSTEMSPANNINGTREESTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetStaSystemStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global spanning tree status.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetSystemSpanningTreeStatus(UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETSYSTEMSPANNINGTREESTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *status = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningSystemSpanningTreeVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree mode.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *mode          -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningSystemSpanningTreeVersion(UI32_T *mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGSYSTEMSPANNINGTREEVERSION;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *mode = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetSystemSpanningTreeVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree mode.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *mode          -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetSystemSpanningTreeVersion(UI32_T *mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETSYSTEMSPANNINGTREEVERSION;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mode = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the forward_delay time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *forward_delay -- pointer of the forward_delay value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningForwardDelay(UI32_T *forward_delay)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGFORWARDDELAY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *forward_delay = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the forward_delay time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *forward_delay -- pointer of the forward_delay value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetForwardDelay(UI32_T *forward_delay)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETFORWARDDELAY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *forward_delay = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the hello_time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningHelloTime(UI32_T *hello_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGHELLOTIME;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *hello_time = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the hello_time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetHelloTime(UI32_T *hello_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETHELLOTIME;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *hello_time = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the max_age information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *max_age         -- pointer of the max_age value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMaxAge(UI32_T *max_age)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGMAXAGE;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *max_age = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the max_age information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *max_age         -- pointer of the max_age value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMaxAge(UI32_T *max_age)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMAXAGE;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *max_age = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost calculation method.
 * INPUT    :   UI32_T  *pathcost_method  -- pointer of the method value
 * OUTPUT   :   None
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPathCostMethod(UI32_T *pathcost_method)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPATHCOSTMETHOD;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *pathcost_method = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPathCostMethod_Ex
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost calculation method.
 * INPUT    :   UI32_T  *pathcost_method  -- pointer of the method value
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPathCostMethod_Ex(UI32_T *pathcost_method)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPATHCOSTMETHOD_EX;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *pathcost_method = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the transmission limit count vlaue.
 * INPUT    :   None
 * OUTPUT   :   UI32_T  *tx_hold_count  -- pointer of the TXHoldCount value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningTransmissionLimit(UI32_T *tx_hold_count)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGTRANSMISSIONLIMIT;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *tx_hold_count = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the transmission limit count vlaue.
 * INPUT    :   None
 * OUTPUT   :   UI32_T  *tx_hold_count  -- pointer of the TXHoldCount value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetTransmissionLimit(UI32_T *tx_hold_count)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETTRANSMISSIONLIMIT;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *tx_hold_count = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge priority information.
 * INPUT    :   None
 * OUTPUT   :   UI16_T  mstid            -- instance value
 *              UI32_T  *priority        -- pointer of the priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstPriority(UI32_T mstid, UI32_T *priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGMSTPRIORITY;
    msg_p->data.arg_grp1.arg1 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *priority = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge priority information.
 * INPUT    :   None
 * OUTPUT   :   UI16_T  mstid            -- instance value
 *              UI32_T  *priority        -- pointer of the priority value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstPriority(UI32_T mstid, UI32_T *priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPRIORITY;
    msg_p->data.arg_grp1.arg1 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *priority = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port priority for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *priority        -- pointer of the priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   RFC-1493/dot1dStpPortEntry 2
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstPortPriority(UI32_T lport,
                                          UI32_T mstid,
                                          UI32_T *priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGMSTPORTPRIORITY;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *priority = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port priority for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *priority        -- pointer of the priority value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * REF      :   RFC-1493/dot1dStpPortEntry 2
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstPortPriority(UI32_T lport,
                                   UI32_T mstid,
                                   UI32_T *priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPORTPRIORITY;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *priority = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortLinkTypeMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port link_type mode of the port for the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetRunningPortLinkTypeMode(UI32_T lport,
                                            UI32_T mstid,
                                            UI32_T *mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTLINKTYPEMODE;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *mode = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortLinkTypeMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port link_type mode of the port for the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPortLinkTypeMode(UI32_T lport,
                                     UI32_T mstid,
                                     UI32_T *mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTLINKTYPEMODE;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mode = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortProtocolMigration
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get protocol_migration status for a port in the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetRunningPortProtocolMigration(UI32_T lport,
                                                 UI32_T mstid,
                                                 UI32_T *mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTPROTOCOLMIGRATION;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *mode = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortProtocolMigration
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get protocol_migration status for a port in the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPortProtocolMigration(UI32_T lport,
                                          UI32_T mstid,
                                          UI32_T *mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTPROTOCOLMIGRATION;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mode = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortAdminEdgePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get edge_port status for a port for in specified spanning
 *              tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetRunningPortAdminEdgePort(UI32_T lport,
                                             UI32_T mstid,
                                             UI32_T *mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTADMINEDGEPORT;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *mode = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortAdminEdgePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get edge_port status for a port for in specified spanning
 *              tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPortAdminEdgePort(UI32_T lport,
                                      UI32_T mstid,
                                      UI32_T *mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTADMINEDGEPORT;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mode = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetDot1dMstPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 *              UI32_T mstid                -- instance value
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetDot1dMstPortEntry(UI32_T mstid,
                                     XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETDOT1DMSTPORTENTRY;
    msg_p->data.arg_grp3.arg1 = mstid;
    msg_p->data.arg_grp3.arg2 = *port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *port_entry = msg_p->data.arg_grp3.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_POM_GetNextDot1dMstPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 *              mstid                       -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   If next available port entry is available, the
 *              port_entry->dot1d_stp_port will be updated and the entry
 *              info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextDot1dMstPortEntry(UI32_T *mstid,
                                         XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTDOT1DMSTPORTENTRY;
    msg_p->data.arg_grp3.arg1 = *mstid;
    msg_p->data.arg_grp3.arg2 = *port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mstid = msg_p->data.arg_grp3.arg1;
    *port_entry = msg_p->data.arg_grp3.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_POM_GetNextPortMemberByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              lport                       -- lport value
 * OUTPUT   :   lport                       -- next lport
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextPortMemberByInstance(UI32_T mstid, UI32_T *lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTPORTMEMBERBYINSTANCE;
    msg_p->data.arg_grp1.arg1 = mstid;
    msg_p->data.arg_grp1.arg2 = *lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *lport = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetDot1dMstPortEntryX
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 *              UI32_T mstid                -- instance value
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. State is backing for a port with no link.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetDot1dMstPortEntryX(UI32_T mstid,
                                      XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETDOT1DMSTPORTENTRYX;
    msg_p->data.arg_grp3.arg1 = mstid;
    msg_p->data.arg_grp3.arg2 = *port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *port_entry = msg_p->data.arg_grp3.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_POM_GetNextDot1dMstPortEntryX
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 *              mstid                       -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. If next available port entry is available, the
 *                 port_entry->dot1d_stp_port will be updated and the entry
 *                 info will be retrieved from the table.
 *              2. State is backing for a port with no link.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextDot1dMstPortEntryX(UI32_T *mstid,
                                          XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTDOT1DMSTPORTENTRYX;
    msg_p->data.arg_grp3.arg1 = *mstid;
    msg_p->data.arg_grp3.arg2 = *port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mstid = msg_p->data.arg_grp3.arg1;
    *port_entry = msg_p->data.arg_grp3.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetDot1dMstExtPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *ext_port_entry                 -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetDot1dMstExtPortEntry(UI32_T mstid,
                                        UI32_T lport,
                                        XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETDOT1DMSTEXTPORTENTRY;
    msg_p->data.arg_grp4.arg1 = mstid;
    msg_p->data.arg_grp4.arg2 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *ext_port_entry = msg_p->data.arg_grp4.arg3;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_POM_GetNextDot1dMstExtPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available ext port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   *lport                          -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *ext_port_entry                 -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   If next available port ext_entry is available, the
 *              ext_port_entry->dot1d_stp_port will be updated and the
 *              entry info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextDot1dMstExtPortEntry(UI32_T mstid,
                                            UI32_T *lport,
                                            XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTDOT1DMSTEXTPORTENTRY;
    msg_p->data.arg_grp4.arg1 = mstid;
    msg_p->data.arg_grp4.arg2 = *lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *lport = msg_p->data.arg_grp4.arg2;
    *ext_port_entry = msg_p->data.arg_grp4.arg3;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * The following Functions only provide for MSTP
 *-------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstpRevisionLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP revision level value.
 * INPUT    :
 * OUTPUT   :   U32_T *revision     -- pointer of the revision value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstpRevisionLevel(UI32_T *revision)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGMSTPREVISIONLEVEL;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *revision = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpRevisionLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP revision level value.
 * INPUT    :
 * OUTPUT   :   U32_T *revision     -- pointer of the revision value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpRevisionLevel(UI32_T *revision)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPREVISIONLEVEL;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *revision = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP Max_Hop count.
 * INPUT    :
 * OUTPUT   :   U32_T *hop_count              -- pointer of max_hop count
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstpMaxHop(UI32_T *hop_count)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGMSTPMAXHOP;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *hop_count = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP Max_Hop count.
 * INPUT    :
 * OUTPUT   :   U32_T *hop_count              -- pointer of max_hop count
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpMaxHop(UI32_T *hop_count)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPMAXHOP;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *hop_count = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpConfigurationEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the configuration entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :
 * OUTPUT   :   *mstp_entry      -- pointer of the configuration entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpConfigurationEntry(XSTP_MGR_MstpEntry_T  *mstp_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_mstentry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPCONFIGURATIONENTRY;
    msg_p->data.arg_mstentry = *mstp_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mstp_entry = msg_p->data.arg_mstentry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpInstanceVlanMapped
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Mapping_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpInstanceVlanMapped(UI32_T mstid,
                                          XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    XSTP_OM_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_XSTP,
                                 XSTP_POM_TRACEID_GETMSTPINSTANCEVLANMAPPED));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPINSTANCEVLANMAPPED;
    msg_p->data.arg_grp5.arg1 = mstid;
    msg_p->data.arg_grp5.arg2 = *mstp_instance_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *mstp_instance_entry = msg_p->data.arg_grp5.arg2;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpInstanceVlanMappedForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Mapping_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpInstanceVlanMappedForMSB(UI32_T mstid,
                                                XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    XSTP_OM_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_XSTP,
                                 XSTP_POM_TRACEID_GETMSTPINSTANCEVLANMAPPEDFORMSB));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPINSTANCEVLANMAPPEDFORMSB;
    msg_p->data.arg_grp5.arg1 = mstid;
    msg_p->data.arg_grp5.arg2 = *mstp_instance_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *mstp_instance_entry = msg_p->data.arg_grp5.arg2;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_NextGetMstpInstanceVlanMapped
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Mapping_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpInstanceVlanMapped(UI32_T *mstid,
                                              XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    XSTP_OM_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_XSTP,
                                 XSTP_POM_TRACEID_GETNEXTMSTPINSTANCEVLANMAPPED));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANMAPPED;
    msg_p->data.arg_grp5.arg1 = *mstid;
    msg_p->data.arg_grp5.arg2 = *mstp_instance_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *mstid = msg_p->data.arg_grp5.arg1;
    *mstp_instance_entry = msg_p->data.arg_grp5.arg2;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstpInstanceVlanMappedForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Mapping_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpInstanceVlanMappedForMSB(UI32_T *mstid,
                                                    XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    XSTP_OM_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_XSTP,
                                 XSTP_POM_TRACEID_GETNEXTMSTPINSTANCEVLANMAPPEDFORMSB));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANMAPPEDFORMSB;
    msg_p->data.arg_grp5.arg1 = *mstid;
    msg_p->data.arg_grp5.arg2 = *mstp_instance_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *mstid = msg_p->data.arg_grp5.arg1;
    *mstp_instance_entry = msg_p->data.arg_grp5.arg2;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Configuration_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpInstanceVlanConfiguration(UI32_T mstid,
                                                 XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    XSTP_OM_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_XSTP,
                                 XSTP_POM_TRACEID_GETMSTPINSTANCEVLANCONFIGURATION));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPINSTANCEVLANCONFIGURATION;
    msg_p->data.arg_grp5.arg1 = mstid;
    msg_p->data.arg_grp5.arg2 = *mstp_instance_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *mstp_instance_entry = msg_p->data.arg_grp5.arg2;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpInstanceVlanConfigurationForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Configuration_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpInstanceVlanConfigurationForMSB(UI32_T mstid,
                                                       XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    XSTP_OM_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_XSTP,
                                 XSTP_POM_TRACEID_GETMSTPINSTANCEVLANCONFIGURATIONFORMSB));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPINSTANCEVLANCONFIGURATIONFORMSB;
    msg_p->data.arg_grp5.arg1 = mstid;
    msg_p->data.arg_grp5.arg2 = *mstp_instance_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *mstp_instance_entry = msg_p->data.arg_grp5.arg2;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
  * RETURN   :  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   From Configuration_Table.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstpInstanceVlanConfiguration(UI32_T mstid,
                                                        XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    XSTP_OM_IpcMsg_T *msg_p;
    UI32_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_XSTP,
                                 XSTP_POM_TRACEID_GETRUNNINGMSTPINSTANCEVLANCONFIGURATION));
    if (msgbuf_p == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGMSTPINSTANCEVLANCONFIGURATION;
    msg_p->data.arg_grp5.arg1 = mstid;
    msg_p->data.arg_grp5.arg2 = *mstp_instance_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *mstp_instance_entry = msg_p->data.arg_grp5.arg2;
    result = msg_p->type.ret_ui32;
    L_MM_Free(msgbuf_p);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Configuration_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpInstanceVlanConfiguration(UI32_T *mstid,
                                                     XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    XSTP_OM_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_XSTP,
                                 XSTP_POM_TRACEID_GETNEXTMSTPINSTANCEVLANCONFIGURATION));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANCONFIGURATION;
    msg_p->data.arg_grp5.arg1 = *mstid;
    msg_p->data.arg_grp5.arg2 = *mstp_instance_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *mstid = msg_p->data.arg_grp5.arg1;
    *mstp_instance_entry = msg_p->data.arg_grp5.arg2;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstpInstanceVlanConfigurationForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Configuration_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpInstanceVlanConfigurationForMSB(UI32_T *mstid,
                                                           XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    XSTP_OM_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_XSTP,
                                 XSTP_POM_TRACEID_GETNEXTMSTPINSTANCEVLANCONFIGURATIONFORMSB));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANCONFIGURATIONFORMSB;
    msg_p->data.arg_grp5.arg1 = *mstid;
    msg_p->data.arg_grp5.arg2 = *mstp_instance_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *mstid = msg_p->data.arg_grp5.arg1;
    *mstp_instance_entry = msg_p->data.arg_grp5.arg2;
    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsMstInstanceExisting
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the mst instance exist for mst
 *              mapping table (active).Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_IsMstInstanceExisting(UI32_T mstid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_ISMSTINSTANCEEXISTING;
    msg_p->data.arg_ui32 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextExistedInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance(active) for mst mapping table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextExistedInstance(UI32_T *mstid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTEXISTEDINSTANCE;
    msg_p->data.arg_ui32 = *mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mstid = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsMstInstanceExistingInMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the mst instance exist for mst
 *              config table(inactive).Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_IsMstInstanceExistingInMstConfigTable(UI32_T mstid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_ISMSTINSTANCEEXISTINGINMSTCONFIGTABLE;
    msg_p->data.arg_ui32 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextExistedInstanceForMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance (inactive) for mst config
 *            table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextExistedInstanceForMstConfigTable(UI32_T *mstid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTEXISTEDINSTANCEFORMSTCONFIGTABLE;
    msg_p->data.arg_ui32 = *mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mstid = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port role in a specified spanning tree.
 * INPUT    :   UI32_T lport                 -- lport number
 *              UI32_T mstid                 -- instance value
 * OUTPUT   :   UI32_T  *role                -- the pointer of role value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetMstPortRole(UI32_T lport,
                               UI32_T mstid,
                               UI32_T *role)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPORTROLE;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *role = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_ui32;
}

 /*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state in a specified spanning tree.
 * INPUT    :   UI32_T lport                 -- lport number
 *              UI32_T mstid                 -- instance value
 * OUTPUT   :   U32_T  *state                -- the pointer of state value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetMstPortState(UI32_T lport,
                                UI32_T mstid,
                                UI32_T *state)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPORTSTATE;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *state = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextVlanMemberByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next vlan member for a specified instance
 * INPUT    : mstid                 -- instance value
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetNextVlanMemberByInstance(UI32_T mstid, UI32_T *vid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTVLANMEMBERBYINSTANCE;
    msg_p->data.arg_grp1.arg1 = mstid;
    msg_p->data.arg_grp1.arg2 = *vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *vid = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstidFromMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstidFromMstConfigurationTableByVlan(UI32_T *vid, UI32_T *mstid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTMSTIDFROMMSTCONFIGURATIONTABLEBYVLAN;
    msg_p->data.arg_grp1.arg1 = *vid;
    msg_p->data.arg_grp1.arg2 = *mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *vid = msg_p->data.arg_grp1.arg1;
    *mstid = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsMemberPortOfInstanceEx
 *-------------------------------------------------------------------------
 * PURPOSE  : Check whether the specified lport is the member of this
 *            spanning tree instance
 * INPUT    : mstid     -- mstid value
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified vlan is the member of this instance, else
 *            FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_IsMemberPortOfInstanceEx(UI32_T mstid, UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_ISMEMBERPORTOFINSTANCEEX;
    msg_p->data.arg_grp1.arg1 = mstid;
    msg_p->data.arg_grp1.arg2 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            XSTP_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

#ifdef  XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetConfigDigest
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the configuration digest
 * INPUT    :   None
 * OUTPUT   :   config_digest           -- pointer of a 16 octet buffer for
 *                                         the configuration digest
 * RETURN   :   TRUE/FALSE
 * NOTES    :   Ref to the description in 13.7, IEEE 802.1s-2002
 * ------------------------------------------------------------------------
 */
BOOL_T    XSTP_POM_GetConfigDigest(UI8_T *config_digest)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ar2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETCONFIGDIGEST;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(config_digest, msg_p->data.arg_ar2, sizeof(msg_p->data.arg_ar2));

    return msg_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the entry can be
 *            get successfully.  Otherwise, return false.
 * INPUT    : mstid       -- instance value
 * OUTPUT   : row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpRowStatus(UI32_T mstid, UI32_T *row_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPROWSTATUS;
    msg_p->data.arg_grp1.arg1 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *row_status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstpRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the entry can be
 *            get successfully.  Otherwise, return false.
 * INPUT    : mstid       -- instance value
 * OUTPUT   : row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpRowStatus(UI32_T *mstid, UI32_T *row_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETNEXTMSTPROWSTATUS;
    msg_p->data.arg_grp1.arg1 = *mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mstid = msg_p->data.arg_grp1.arg1;
    *row_status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortAdminPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the admin path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *admin_path_cost -- admin path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. If the default Path Cost is being used, return '0'.
 *              2. It is equal to external_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortAdminPathCost(UI32_T lport, UI32_T *admin_path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTADMINPATHCOST;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *admin_path_cost = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortOperPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the oper path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *oper_path_cost  -- oper path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   It is equal to external_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortOperPathCost(UI32_T lport, UI32_T *oper_path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTOPERPATHCOST;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *oper_path_cost = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortAdminPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the admin path_cost of the port for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *admin_path_cost -- admin path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. If the default Path Cost is being used, return '0'.
 *              2. It is equal to internal_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetMstPortAdminPathCost(UI32_T lport,
                                        UI32_T mstid,
                                        UI32_T *admin_path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPORTADMINPATHCOST;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *admin_path_cost = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortOperPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the oper path_cost of the port for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *oper_path_cost  -- oper path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   It is equal to internal_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetMstPortOperPathCost(UI32_T lport,
                                       UI32_T mstid,
                                       UI32_T *oper_path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETMSTPORTOPERPATHCOST;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *oper_path_cost = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_ui32;
}

/* per_port spanning tree : begin */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortSpanningTreeStatus(UI32_T lport, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTSPANNINGTREESTATUS;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortSpanningTreeStatus(UI32_T lport, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTSPANNINGTREESTATUS;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetDesignatedRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated root for specified instance.
 * INPUT    :   mstid                    -- instance value
 * OUTPUT   :   *designated_root         -- pointer of the specified
 *                                          designated_root
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetDesignatedRoot(UI32_T mstid,
                                  XSTP_MGR_BridgeIdComponent_T *designated_root)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETDESIGNATEDROOT;
    msg_p->data.arg_grp6.arg1 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *designated_root = msg_p->data.arg_grp6.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetBridgeIdComponent
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge_id_component for specified instance.
 * INPUT    :   mstid                    -- instance value
 * OUTPUT   :   *designated_root         -- pointer of the specified
 *                                          bridge_id_component
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetBridgeIdComponent(UI32_T mstid,
                                     XSTP_MGR_BridgeIdComponent_T *bridge_id_component)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETBRIDGEIDCOMPONENT;
    msg_p->data.arg_grp6.arg1 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *bridge_id_component = msg_p->data.arg_grp6.arg2;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortDesignatedRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated root for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              mstid                           -- instance value
 * OUTPUT   :   *designated_root                -- pointer of the specified
 *                                                 designated_root
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortDesignatedRoot(UI32_T lport,
                                      UI32_T mstid,
                                      XSTP_MGR_BridgeIdComponent_T *designated_root)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp7);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTDESIGNATEDROOT;
    msg_p->data.arg_grp7.arg1 = lport;
    msg_p->data.arg_grp7.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *designated_root = msg_p->data.arg_grp7.arg3;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortDesignatedBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated bridge for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *designated_bridge              -- pointer of the specified
 *                                                 port designated_bridge
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortDesignatedBridge(UI32_T lport,
                                        UI32_T mstid,
                                        XSTP_MGR_BridgeIdComponent_T *designated_bridge)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp7);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTDESIGNATEDBRIDGE;
    msg_p->data.arg_grp7.arg1 = lport;
    msg_p->data.arg_grp7.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *designated_bridge = msg_p->data.arg_grp7.arg3;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortDesignatedPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated port for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *designated_port                -- pointer of the specified
 *                                                 designated_port
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortDesignatedPort(UI32_T lport,
                                      UI32_T mstid,
                                      XSTP_MGR_PortIdComponent_T *designated_port)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTDESIGNATEDPORT;
    msg_p->data.arg_grp8.arg1 = lport;
    msg_p->data.arg_grp8.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *designated_port = msg_p->data.arg_grp8.arg3;

    return msg_p->type.ret_bool;
}

/* per_port spanning tree : end */

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- status value.
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortRootGuardStatus(UI32_T lport, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTROOTGUARDSTATUS;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get running root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- status value.
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :   None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortRootGuardStatus(UI32_T lport, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTROOTGUARDSTATUS;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortBpduGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU guard status on the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortBpduGuardStatus(UI32_T lport, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTBPDUGUARDSTATUS;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortBpduGuardStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get running BPDU Guard status on the specified port.
 * INPUT    : lport     -- the logical port number
 * OUTPUT   : status    -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : (interface function)
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortBpduGuardStatus(UI32_T lport, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTBPDUGUARDSTATUS;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortBPDUGuardAutoRecovery
 * ------------------------------------------------------------------------
 * PURPOSE : Get the BPDU guard auto recovery status on the specified port.
 * INPUT   : lport -- lport number
 * OUTPUT  : status -- the status value
 * RETURN  : XSTP_TYPE_RETURN_CODE_E
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortBPDUGuardAutoRecovery(UI32_T lport, UI32_T  *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTBPDUGUARDAUTORECOVERY;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortBPDUGuardAutoRecovery
 *-------------------------------------------------------------------------
 * PURPOSE : Get running BPDU guard auto recovery status for the specified
 *           port.
 * INPUT   : lport -- the logical port number
 * OUTPUT  : status -- the status value
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortBPDUGuardAutoRecovery(UI32_T lport,
                                                    UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTBPDUGUARDAUTORECOVERY;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortBPDUGuardAutoRecoveryInterval
 * ------------------------------------------------------------------------
 * PURPOSE : Get the BPDU guard auto recovery interval on the specified
 *           port.
 * INPUT   : lport -- lport number
 * OUTPUT  : interval -- the interval value
 * RETURN  : XSTP_TYPE_RETURN_CODE_E
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortBPDUGuardAutoRecoveryInterval(UI32_T lport,
                                                     UI32_T *interval)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTBPDUGUARDAUTORECOVERYINTERVAL;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *interval = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortBPDUGuardAutoRecoveryInterval
 *-------------------------------------------------------------------------
 * PURPOSE : Get running BPDU guard auto recovery interval for the specified
 *           port.
 * INPUT   : lport -- the logical port number
 * OUTPUT  : interval -- the status value
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortBPDUGuardAutoRecoveryInterval(UI32_T lport,
                                                            UI32_T *interval)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTBPDUGUARDAUTORECOVERYINTERVAL;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *interval = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortBpduFilterStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU filter status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortBpduFilterStatus(UI32_T lport, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTBPDUFILTERSTATUS;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortBpduFilterStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get per port variable bpdu_filter_status value.
 * INPUT    : lport     -- the logical port number
 * OUTPUT   : status    -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : (interface function)
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortBpduFilterStatus(UI32_T lport, UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTBPDUFILTERSTATUS;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}
#endif /* #if (SYS_CPNT_STP_BPDU_FILTER == TRUE) */

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetCiscoPrestandardCompatibility
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the cisco prestandard compatibility status
 * INPUT    :   UI32_T status           -- the status value
 * OUTPUT   :   UI32_T status           -- the status value
 * RETURN   :   None
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
void XSTP_POM_GetCiscoPrestandardCompatibility(UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETCISCOPRESTANDARDCOMPATIBILITY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return ;
    }

    *status = msg_p->data.arg_ui32;

    return;
}/* End of XSTP_POM_GetCiscoPrestandardCompatibility() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningCiscoPrestandardCompatibility
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the cisco prestandard compatibility status.
 * INPUT    : UI32_T status           -- the status value
 * OUTPUT   : UI32_T status           -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    :
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningCiscoPrestandardCompatibility(UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGCISCOPRESTANDARDCOMPATIBILITY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;;
}/* End of XSTP_POM_GetRunningCiscoPrestandardCompatibility() */
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */


#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get TC propage stop status
 * INPUT    :   UI32_T lport            -- lport number
 *              BOOL)T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortTcPropStop(UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETPORTTCPROPSTOP;
    msg_p->data.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get TC propage stop status
 * INPUT    :   UI32_T lport            -- lport number
 *              BOOL)T status           -- the status value
 * OUTPUT   : None
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS - need to store configuration
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - configure no change
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortTcPropStop(UI32_T lport, BOOL_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETRUNNINGPORTTCPROPSTOP;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *status = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
}
#endif /*#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)*/

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTcPropGroupPortbitmap
 * ------------------------------------------------------------------------
 * PURPOSE  : Get port list for specified group ID.
 * INPUT    : group_id     -- group ID
 * OUTPUT   : portbitmap   -- group member ports
 *            has_port_p   -- have port or not
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetTcPropGroupPortbitmap(UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                BOOL_T *has_port_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp9);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETTCPROPGROUPPORTBITMAP;
    msg_p->data.arg_grp9.arg1 = group_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(portbitmap,
           msg_p->data.arg_grp9.arg_ar1,
           sizeof(msg_p->data.arg_grp9.arg_ar1)
          );
    *has_port_p = msg_p->data.arg_grp9.arg_bool;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTcPropNextGroupPortbitmap
 * ------------------------------------------------------------------------
 * PURPOSE  : Get next group ID and port list.
 * INPUT    : group_id_p   -- group ID pointer
 * OUTPUT   : group_id_p   -- group ID pointer
 *            portbitmap   -- group member ports
 *            has_port_p   -- have port or not
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetTcPropNextGroupPortbitmap(UI32_T *group_id_p,
                                     UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                     BOOL_T *has_port_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp9);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_OM_IPC_GETTCPROPNEXTGROUPPORTBITMAP;
    msg_p->data.arg_grp9.arg1 = *group_id_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *group_id_p = msg_p->data.arg_grp9.arg1;
    memcpy(portbitmap,
           msg_p->data.arg_grp9.arg_ar1,
           sizeof(msg_p->data.arg_grp9.arg_ar1)
          );
    *has_port_p = msg_p->data.arg_grp9.arg_bool;

    return msg_p->type.ret_bool;
}
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/
/* LOCAL SUBPROGRAM BODIES
 */
