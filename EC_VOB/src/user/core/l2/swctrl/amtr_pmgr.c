/*-----------------------------------------------------------------------------
 * FILE NAME: AMTR_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for AMTR MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/14     --- Timon, Create
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
#include "l_mm.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "amtrdrv_pom.h"
#include "amtr_mgr.h"
#include "amtr_pmgr.h"
#include "amtr_type.h"
#include "amtr_om.h"
#include "amtr_om_private.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* trace id definition when using L_MM
 */
enum
{
    AMTR_PMGR_TRACEID_SETMULTICASTPORTMEMBER
};


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time, BOOL_T sync_op);

/* STATIC VARIABLE DEFINITIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : AMTR_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for AMTR_PMGR in the calling process.
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
BOOL_T AMTR_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for AMTR MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of AMTR_PMGR_InitiateProcessResource */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetRunningAgingTime
 *-----------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          non-default ageing time can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None
 * OUTPUT:  *aging_time   - the non-default ageing time
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default ageing time.
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningAgingTime(UI32_T *aging_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETRUNNINGAGINGTIME;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *aging_time = msg_p->data.arg_ui32;

    return msg_p->type.ret_running_cfg;
} /* End of AMTR_PMGR_GetRunningAgingTime */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_SetAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid - VLAN ID
 *           addr_entry -> mac - MAC address
 *           addr_entry -> ifindex - interface index
 *           addr_entry -> action -Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source -Config/Learn/Internal/Self
 *           addr_entry -> life_time -permanent/Other/Delete on Reset/Delete on Timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    : 1. parameters:
 *              action:     AMTRDRV_OM_ADDRESS_ACTION_FORWARDING
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                          AMTRDRV_OM_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *              source:     AMTRDRV_OM_ADDRESS_SOURCE_LEARN
 *                          AMTRDRV_OM_ADDRESS_SOURCE_CONFIG
 *                          AMTRDRV_OM_ADDRESS_SOURCE_INTERNAL
 *                          AMTRDRV_OM_ADDRESS_SOURCE_SELF
 *
 *              life_time:  AMTRDRV_OM_ADDRESS_LIFETIME_OTHER
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_INVALID
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_PERMANENT
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONRESET
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONTIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *             In AMTR, ifindex ==0 means just set to OM, don't program chip.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETADDRENTRY;
    msg_p->data.arg_addr_entry = *addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_SetAddrEntry */

#if (SYS_CPNT_MLAG == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_SetAddrEntryForMlag
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid - VLAN ID
 *           addr_entry -> mac - MAC address
 *           addr_entry -> ifindex - interface index
 *           addr_entry -> action -Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source -Config/Learn/Internal/Self
 *           addr_entry -> life_time -permanent/Other/Delete on Reset/Delete on Timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    : 1. parameters:
 *              action:     AMTRDRV_OM_ADDRESS_ACTION_FORWARDING
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                          AMTRDRV_OM_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *              source:     AMTRDRV_OM_ADDRESS_SOURCE_LEARN
 *                          AMTRDRV_OM_ADDRESS_SOURCE_CONFIG
 *                          AMTRDRV_OM_ADDRESS_SOURCE_INTERNAL
 *                          AMTRDRV_OM_ADDRESS_SOURCE_SELF
 *
 *              life_time:  AMTRDRV_OM_ADDRESS_LIFETIME_OTHER
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_INVALID
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_PERMANENT
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONRESET
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONTIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *             In AMTR, ifindex ==0 means just set to OM, don't program chip.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetAddrEntryForMlag(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETADDRENTRYFORMLAG;
    msg_p->data.arg_addr_entry = *addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_SetAddrEntryForMlag */
#endif

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddr
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will clear specific address table entry
 * INPUT   : UI16_T  vid   - vlan ID
 *           UI8_T  *Mac   - Specified MAC to be cleared
 *
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE:     This API can't delete CPU MAC!
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddr(UI32_T vid, UI8_T *mac)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEADDR;
    msg_p->data.arg_grp_ui32_mac.arg_ui32 = vid;
    memcpy(msg_p->data.arg_grp_ui32_mac.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_ui32_mac.arg_mac));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_DeleteAddr */

#if (SYS_CPNT_MLAG == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrForMlag
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will clear specific address table entry
 * INPUT   : UI16_T  vid   - vlan ID
 *           UI8_T  *Mac   - Specified MAC to be cleared
 *
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE:     This API can't delete CPU MAC!
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrForMlag(UI32_T vid, UI8_T *mac)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEADDRFORMLAG;
    msg_p->data.arg_grp_ui32_mac.arg_ui32 = vid;
    memcpy(msg_p->data.arg_grp_ui32_mac.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_ui32_mac.arg_mac));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_DeleteAddrForMlag */
#endif

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTime
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by life time
 * INPUT   : AMTR_TYPE_AddressLifeTime_T life_time
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTime(AMTR_TYPE_AddressLifeTime_T life_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_address_life_time);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEADDRBYLIFETIME;
    msg_p->data.arg_address_life_time = life_time;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_DeleteAddrByLifeTime */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTimeAndLPort
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time and Port.
 * INPUT   : UI32_T ifindex - interface index
 *           AMTR_MGR_LiftTimeMode_T life_time - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : when a instance has too many vlan trigger tc,
 *           it will generate may enven. Here change to asynchronize call
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(UI32_T ifindex, AMTR_TYPE_AddressLifeTime_T life_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_alt);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDLPORT;
    msg_p->data.arg_grp_ui32_alt.arg_ui32 = ifindex;
    msg_p->data.arg_grp_ui32_alt.arg_addresslifetime = life_time;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, NULL) != SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
} /* End of AMTR_PMGR_DeleteAddrByLifeTimeAndLPort */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrBySourceAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by specific source and Lport.
 * INPUT   : UI32_T ifindex               - interface index
 *           AMTR_MGR_SourceMode_T source - learnt, config, intrenal, self
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_DeleteAddrBySourceAndLPort(UI32_T ifindex, AMTR_TYPE_AddressSource_T source)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_alt);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEADDRBYSOURCEANDLPORT;
    msg_p->data.arg_grp_ui32_alt.arg_ui32 = ifindex;
    msg_p->data.arg_grp_ui32_alt.arg_addresslifetime = source;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, NULL) != SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
} /* End of AMTR_PMGR_DeleteAddrByLifeTimeAndLPort */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port.
 * INPUT   : UI32_T ifindex                     - interface index
 *           UI16_T start_vid                   - the starting vid
 *           UI16_T end_vid                     - the end vid
 *           AMTR_MGR_LiftTimeMode_T life_time  - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status                      - True : successs, False : failed
 * NOTE    : 
 *-----------------------------------------------------------------------------
 */
static BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time, BOOL_T sync_op)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32_ui32_ui16_ui16_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSFUN_Msg_T *resp_msgbuf_p = NULL;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDVIDRANGEANDLPORT;
    msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_ui32_1 = ifindex;
    msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_ui32_2 = life_time;
    msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_ui16_1 = start_vid;
    msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_ui16_2 = end_vid;
    msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_bool_1 = sync_op;

    if (sync_op==TRUE)
        resp_msgbuf_p=msgbuf_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, resp_msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (sync_op==TRUE)
    {
        return msg_p->type.ret_bool;
    }

    return TRUE;
} /* End of AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port.
 * INPUT   : UI32_T ifindex                     - interface index
 *           UI32_T vid                         - vlan id
 *           AMTR_MGR_LiftTimeMode_T life_time  - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status                      - True : successs, False : failed
 * NOTE    : When an instance has too many vlan trigger tc,
 *           it will generate many events. Here changes to asynchronize call
 *           and use start_vid and end_vid
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time)
{
    return AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal(ifindex, start_vid, end_vid, life_time, FALSE);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port
 *           synchronously.
 * INPUT   : UI32_T ifindex - interface index
 *           UI16_T start_vid - the starting vid
 *           UI16_T end_vid   - the end vid
 *           UI32_T vid     - vlan id
 *           AMTR_MGR_LiftTimeMode_T life_time - delete on timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : 1. This function will not return until the delete operation is done
 *           2. Only support life_time as AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time)
{
    return AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal(ifindex, start_vid, end_vid, life_time, TRUE);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTimeAndMstIDAndLPort
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port.
 * INPUT   : UI32_T ifindex                     - interface index
 *           UI32_T vid                         - vlan id
 *           AMTR_MGR_LiftTimeMode_T life_time  - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status                      - True : successs, False : failed
 * NOTE    : when a instance has too many vlan trigger tc,
 *           it will generate may enven. Here change to asynchronize call
 *           and use start_vid and end_vid
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndMstIDAndLPort(UI32_T ifindex, UI32_T mst_id, AMTR_TYPE_AddressLifeTime_T life_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDMSTIDANDLPORT;
    msg_p->data.arg_ui32_ui32_ui32.arg_ui32_1 = ifindex;
    msg_p->data.arg_ui32_ui32_ui32.arg_ui32_2 = mst_id;
    msg_p->data.arg_ui32_ui32_ui32.arg_ui32_3 = life_time;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, NULL) != SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
} /* End of AMTR_PMGR_DeleteAddrByLifeTimeAndMstIDAndLPort */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetExactAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get exact address table entry (vid+mac)
 * INPUT   : addr_entry->vid    - VLAN ID     (key)
 *           addr_entry->mac    - MAC address (key)
 * OUTPUT  : addr_entry         - address entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from macList
 *           FALSE if no address can be found
 * NOTE    : 1. using key generated from (vlanID,mac address)
 *           2. search the entry from Hash table(driver layer)
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetExactAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETEXACTADDRENTRY;
    msg_p->data.arg_addr_entry = *addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *addr_entry = msg_p->data.arg_addr_entry;

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_GetExactAddrEntry */


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetExactAddrEntryFromChip
 *------------------------------------------------------------------------
 * FUNCTION: This function will get exact address table entry (mac+vid)
 * INPUT   : addr_entry->mac - mac address
 *           addr_entry->vid - vlan id
 * OUTPUT  : addr_entry      - addresss entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from macList
 *           FALSE if no address can be found
 * NOTE    : 1.This API is only support IML_MGR to get exact mac address from chip
 *             We don't support MIB to get under_create entry since l_port =0
 *             will return false
 *           2.This API only support MV key or VM key not IVM key
 *------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_GetExactAddrEntryFromChip(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETEXACTADDRENTRYFROMCHIP;
    msg_p->data.arg_addr_entry = *addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *addr_entry = msg_p->data.arg_addr_entry;

    return msg_p->type.ret_bool;
}


/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextMVAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (mac+vid)
 * INPUT   : addr_entry->mac    - MAC address (primary key)
 *           addr_entry->vid    - VLAN ID     (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if mac[0]~mac[5] == \0 => get the first entry
 *           2. search the entry from Hisam table(core layer)
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextMVAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_addrentry_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETNEXTMVADDRENTRY;
    msg_p->data.arg_grp_addrentry_ui32.arg_addr_entry = *addr_entry;
    msg_p->data.arg_grp_addrentry_ui32.arg_ui32 = get_mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *addr_entry = msg_p->data.arg_grp_addrentry_ui32.arg_addr_entry;

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_GetNextMVAddrEntry */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextVMAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (vid+mac)
 * INPUT   : addr_entry->vid    - VLAN ID        (primary key)
 *           addr_entry->mac    - MAC address    (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if vid == 0 => get the first entry
 *           2. using key generated from (vlanID,mac address)
 *           3. search the entry from Hisam table(core layer)
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextVMAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_addrentry_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETNEXTVMADDRENTRY;
    msg_p->data.arg_grp_addrentry_ui32.arg_addr_entry = *addr_entry;
    msg_p->data.arg_grp_addrentry_ui32.arg_ui32 = get_mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *addr_entry = msg_p->data.arg_grp_addrentry_ui32.arg_addr_entry;

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_GetNextVMAddrEntry */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextIfIndexAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (ifindex+vid+mac)
 * INPUT   : addr_entry->l_port - interface index   (primary key)
 *           addr_entry->vid    - vlan id           (key)
 *           addr_entry->mac    - MAC address       (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. l_port is a physical port or trunk port
 *           2. search the entry from Hisam table(core layer)
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextIfIndexAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_addrentry_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETNEXTIFINDEXADDRENTRY;
    msg_p->data.arg_grp_addrentry_ui32.arg_addr_entry = *addr_entry;
    msg_p->data.arg_grp_addrentry_ui32.arg_ui32 = get_mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *addr_entry = msg_p->data.arg_grp_addrentry_ui32.arg_addr_entry;

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_GetNextIfIndexAddrEntry */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextRunningStaticAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           static address can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL is returned.
 * INPUT   : addr_entry->mac    - MAC address (primary key)
 *           addr_entry->vid    - VLAN ID     (key)
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if mac[0]~mac[5] == \0 => get the first entry
 *           2. the function shall be only invoked by cli
 *           3. search the entry from Hash table
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetNextRunningStaticAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETNEXTRUNNINGSTATICADDRENTRY;
    msg_p->data.arg_addr_entry = *addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *addr_entry = msg_p->data.arg_addr_entry;

    return msg_p->type.ret_running_cfg;
} /* End of AMTR_PMGR_GetNextRunningStaticAddrEntry */

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetInterventionEntry
 *-----------------------------------------------------------------------------
 * Purpose  : This routine will add a CPU MAC to the chip.  Packets with this
 *            MAC in the specified VLAN will trap to CPU.
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 * OUTPUT   : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTE     : This API doesn't forbid that CPU has only one MAC.  It will only
 *            depend on spec.
 *-----------------------------------------------------------------------------
 */
AMTR_TYPE_Ret_T  AMTR_PMGR_SetInterventionEntry(UI32_T vid, UI8_T *mac)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETINTERVENTIONENTRY;
    msg_p->data.arg_grp_ui32_mac.arg_ui32 = vid;
    memcpy(msg_p->data.arg_grp_ui32_mac.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_ui32_mac.arg_mac));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return AMTR_TYPE_RET_ERROR_UNKNOWN;
    }

    return msg_p->type.ret_amtr_type;
} /* End of AMTR_PMGR_SetInterventionEntry */

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_DeleteInterventionEntry
 *-----------------------------------------------------------------------------
 * Purpose  : This function will delete a intervention mac address from
 *            address table
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_DeleteInterventionEntry(UI32_T vid, UI8_T *mac)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEINTERVENTIONENTRY;
    msg_p->data.arg_grp_ui32_mac.arg_ui32 = vid;
    memcpy(msg_p->data.arg_grp_ui32_mac.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_ui32_mac.arg_mac));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_DeleteInterventionEntry */

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_CreateMulticastAddrTblEntry
 *-----------------------------------------------------------------------------
 * Purpose  : This function will create a multicast address table entry
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_CreateMulticastAddrTblEntry(UI32_T vid, UI8_T *mac)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_CREATEMULTICASTADDRTBLENTRY;
    msg_p->data.arg_grp_ui32_mac.arg_ui32 = vid;
    memcpy(msg_p->data.arg_grp_ui32_mac.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_ui32_mac.arg_mac));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_CreateMulticastAddrTblEntry */

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_DestroyMulticastAddrTblEntry
 *-----------------------------------------------------------------------------
 * Purpose  : This function will destroy a multicast address table entry
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_DestroyMulticastAddrTblEntry(UI32_T vid, UI8_T* mac)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DESTROYMULTICASTADDRTBLENTRY;
    msg_p->data.arg_grp_ui32_mac.arg_ui32 = vid;
    memcpy(msg_p->data.arg_grp_ui32_mac.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_ui32_mac.arg_mac));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_DestroyMulticastAddrTblEntry */

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMulticastPortMember
 *-----------------------------------------------------------------------------
 * Purpose  : This function sets the port member(s) of a given multicast address
 * INPUT    : UI32_T vid
 *            UI8_T *mac                                                                                - multicast MAC address
 *            UI8_T ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]   - the member ports of the MAC
 *            UI8_T tagged[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] - tagged/untagged member
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMulticastPortMember(UI32_T vid,
                                         UI8_T *mac,
                                         UI8_T ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                         UI8_T tagged[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac_ports_tagged);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    AMTR_MGR_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_AMTR,
                                 AMTR_PMGR_TRACEID_SETMULTICASTPORTMEMBER));
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETMULTICASTPORTMEMBER;
    msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_ui32 = vid;
    memcpy(msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_mac));
    memcpy(msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_ports, ports,
        sizeof(msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_ports));
    memcpy(msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_tagged, tagged,
        sizeof(msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_tagged));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    result = msg_p->type.ret_bool;
    L_MM_Free(msgbuf_p);

    return result;
} /* End of AMTR_PMGR_SetMulticastPortMember */

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_GetAgingStatus
 *-----------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T *status - VAL_amtrMacAddrAgingStatus_enabled
 *                             VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_GetAgingStatus(UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETAGINGSTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *status = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetAgingStatus */

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_GetRunningAgingStatus
 *-----------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T *status -  VAL_amtrMacAddrAgingStatus_enabled
 *                              VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T -
 *                              1. SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *                              2. SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *                              3. SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T  AMTR_PMGR_GetRunningAgingStatus(UI32_T *status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETRUNNINGAGINGSTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = msg_p->data.arg_ui32;

    return msg_p->type.ret_running_cfg;
} /* End of AMTR_PMGR_GetRunningAgingStatus */

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetAgingStatus
 *-----------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T status  - VAL_amtrMacAddrAgingStatus_enabled
 *                             VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetAgingStatus(UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETAGINGSTATUS;
    msg_p->data.arg_ui32 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_SetAgingStatus */

/*---------------------------------------------------------------------- */
/* The dot1dTp group (dot1dBridge 4 ) -- dot1dTp 2 & dot1dTp 3 */
/*
 *         dot1dTpAgingTime
 *             SYNTAX   INTEGER (10..1000000)
 *             ::= { dot1dTp 2 }
 *----------------------------------------------------------------------
 *         dot1dTpFdbTable OBJECT-TYPE
 *             SYNTAX  SEQUENCE OF Dot1dTpFdbEntry
 *             ::= { dot1dTp 3 }
 *         Dot1dTpFdbEntry
 *             INDEX   { dot1dTpFdbAddress }
 *             ::= { dot1dTpFdbTable 1 }
 *         Dot1dTpFdbEntry ::=
 *             SEQUENCE {
 *                 dot1dTpFdbAddress    MacAddress,
 *                 dot1dTpFdbPort       INTEGER,
 *                 dot1dTpFdbStatus     INTEGER
 *             }
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetDot1dTpAgingTime
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified aging time
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   aging_time                  -- aging time
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTp 2
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1dTpAgingTime(UI32_T *aging_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETDOT1DTPAGINGTIME;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *aging_time = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetDot1dTpAgingTime */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dTpAgingTime
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified aging time
 *              can be successfully set. Otherwise, false is returned.
 * INPUT    :   aging_time                  -- aging time
 * OUTPUT   :   NOne
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. RFC1493/dot1dTp 2
 *              2. aging time is in [10..1000000] seconds
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetDot1dTpAgingTime(UI32_T aging_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETDOT1DTPAGINGTIME;
    msg_p->data.arg_ui32 = aging_time;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_SetDot1dTpAgingTime */

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetHashLookupDepthFromConfig
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified hash lookup depth
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   lookup_depth_p   -- hash lookup depth
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetHashLookupDepthFromConfig(UI32_T *lookup_depth_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETHASHLOOKUPDEPTHFROMCONFIG;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *lookup_depth_p = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetHashLookupDepthFromChip
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified max hash lookup length
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   lookup_depth_p   -- hash lookup depth
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetHashLookupDepthFromChip(UI32_T *lookup_depth_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETHASHLOOKUPDEPTHFROMCHIP;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *lookup_depth_p = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetHashLookupDepth
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified max hash lookup length
 *              can be successfully set. Otherwise, false is returned.
 * INPUT    :   lookup_depth     -- hash lookup depth
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetHashLookupDepth(UI32_T lookup_depth)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETHASHLOOKUPDEPTH;
    msg_p->data.arg_ui32 = lookup_depth;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    return msg_p->type.ret_bool;
}
#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetDot1dTpFdbEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified forwarding database
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   tp_fdb_entry -- Forwarding Database for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpFdbTable 1
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1dTpFdbEntry(AMTR_MGR_Dot1dTpFdbEntry_T *tp_fdb_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1d_tp_fdb_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETDOT1DTPFDBENTRY;
    msg_p->data.arg_dot1d_tp_fdb_entry = *tp_fdb_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *tp_fdb_entry = msg_p->data.arg_dot1d_tp_fdb_entry;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetDot1dTpFdbEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetNextDot1dTpFdbEntry
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified forwarding database
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   tp_fdb_entry -- Forwarding Database for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpFdbTable 1
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1dTpFdbEntry(AMTR_MGR_Dot1dTpFdbEntry_T *tp_fdb_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1d_tp_fdb_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETNEXTDOT1DTPFDBENTRY;
    msg_p->data.arg_dot1d_tp_fdb_entry = *tp_fdb_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *tp_fdb_entry = msg_p->data.arg_dot1d_tp_fdb_entry;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetNextDot1dTpFdbEntry */

/*---------------------------------------------------------------------- */
/* The Static (Destination-Address Filtering) Database (dot1dBridge 5) */
/*
 *         Dot1dStaticEntry
 *             INDEX   { dot1dStaticAddress, dot1dStaticReceivePort }
 *             ::= { dot1dStaticTable 1 }
 *
 *         Dot1dStaticEntry ::=
 *             SEQUENCE {
 *                 dot1dStaticAddress           MacAddress,
 *                 dot1dStaticReceivePort       INTEGER,
 *                 dot1dStaticAllowedToGoTo     OCTET STRING,
 *                 dot1dStaticStatus            INTEGER
 *             }
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetDot1dStaticEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   static_entry->dot1d_static_address
 * OUTPUT   :   static_entry                  -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dStaticTable 1
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1dStaticEntry(AMTR_MGR_Dot1dStaticEntry_T *static_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1d_static_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETDOT1DSTATICENTRY;
    msg_p->data.arg_dot1d_static_entry = *static_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *static_entry = msg_p->data.arg_dot1d_static_entry;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetDot1dStaticEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetNextDot1dStaticEntry
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   static_entry->dot1d_static_address
 * OUTPUT   :   static_entry                  -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dStaticTable 1
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1dStaticEntry(AMTR_MGR_Dot1dStaticEntry_T *static_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1d_static_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETNEXTDOT1DSTATICENTRY;
    msg_p->data.arg_dot1d_static_entry = *static_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *static_entry = msg_p->data.arg_dot1d_static_entry;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetNextDot1dStaticEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dStaticAddress
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set the static address information.
 * INPUT    :   UI8_T old_mac -- the original mac address (key)
 *              UI8_T new_mac -- the new mac to replace original mac
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 1
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1dStaticAddress(UI8_T *old_mac, UI8_T *new_mac)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_mac_mac);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETDOT1DSTATICADDRESS;
    memcpy(msg_p->data.arg_grp_mac_mac.arg_mac_1, old_mac,
        sizeof(msg_p->data.arg_grp_mac_mac.arg_mac_1));
    memcpy(msg_p->data.arg_grp_mac_mac.arg_mac_2, new_mac,
        sizeof(msg_p->data.arg_grp_mac_mac.arg_mac_2));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_SetDot1dStaticAddress */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dStaticReceivePort
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set the static receive port information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI32_T receive_port     -- the receive port number
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 2
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1dStaticReceivePort(UI8_T *mac, UI32_T receive_port)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_mac_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETDOT1DSTATICRECEIVEPORT;
    memcpy(msg_p->data.arg_grp_mac_ui32.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_mac_ui32.arg_mac));
    msg_p->data.arg_grp_mac_ui32.arg_ui32 = receive_port;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_SetDot1dStaticReceivePort */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dStaticAllowedToGoTo
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set the static allowed to go to information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI8_T allow_to_go_to    -- the set of ports
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 3
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1dStaticAllowedToGoTo(UI8_T *mac, UI8_T *allowed_to_go_to)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_mac_ports);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETDOT1DSTATICALLOWEDTOGOTO;
    memcpy(msg_p->data.arg_grp_mac_ports.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_mac_ports.arg_mac));
    memcpy(msg_p->data.arg_grp_mac_ports.arg_ports, allowed_to_go_to,
        sizeof(msg_p->data.arg_grp_mac_ports.arg_ports));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_SetDot1dStaticAllowedToGoTo */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dStaticStatus
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set/Create the static status information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI32_T status           -- VAL_dot1dStaticStatus_other
 *                                         VAL_dot1dStaticStatus_invalid
 *                                         VAL_dot1dStaticStatus_permanent
 *                                         VAL_dot1dStaticStatus_deleteOnReset
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 4
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1dStaticStatus(UI8_T *mac, UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_mac_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETDOT1DSTATICSTATUS;
    memcpy(msg_p->data.arg_grp_mac_ui32.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_mac_ui32.arg_mac));
    msg_p->data.arg_grp_mac_ui32.arg_ui32 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_SetDot1dStaticStatus */

/*---------------------------------------------------------------------- */
/* the current Filtering Database Table (the dot1qTp group 1) */
/*
 *       INDEX   { dot1qFdbId }
 *       Dot1qFdbEntry ::=
 *       SEQUENCE {
 *           dot1qFdbId             Unsigned32,
 *           dot1qFdbDynamicCount   Counter32
 *       }
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetDot1qFdbEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get the filtering database entry info
 * INPUT   : dot1q_fdb_entry->dot1q_fdb_id  - The identity of this Filtering Database
 * OUTPUT  : dot1q_fdb_entry                - The  Filtering Database entry
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 1
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1qFdbEntry(AMTR_MGR_Dot1qFdbEntry_T *dot1q_fdb_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1q_fdb_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETDOT1QFDBENTRY;
    msg_p->data.arg_dot1q_fdb_entry = *dot1q_fdb_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *dot1q_fdb_entry = msg_p->data.arg_dot1q_fdb_entry;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetDot1qFdbEntry */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextDot1qFdbEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get the next filtering database entry info
 * INPUT   : dot1q_fdb_entry->dot1q_fdb_id  - The identity of this Filtering Database
 * OUTPUT  : dot1q_fdb_entry                - The  Filtering Database entry
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 1
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1qFdbEntry(AMTR_MGR_Dot1qFdbEntry_T *dot1q_fdb_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1q_fdb_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETNEXTDOT1QFDBENTRY;
    msg_p->data.arg_dot1q_fdb_entry = *dot1q_fdb_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *dot1q_fdb_entry = msg_p->data.arg_dot1q_fdb_entry;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetNextDot1qFdbEntry */

/*---------------------------------------------------------------------- */
/* (the dot1qTp group 2) */
/*
 *      INDEX   { dot1qFdbId, dot1qTpFdbAddress }
 *      Dot1qTpFdbEntry ::=
 *          SEQUENCE {
 *              dot1qTpFdbAddress  MacAddress,
 *              dot1qTpFdbPort     INTEGER,
 *              dot1qTpFdbStatus   INTEGER
 *          }
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetDot1qTpFdbEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get the dot1qTpFdbEntry info
 * INPUT   : dot1q_tp_fdb_entry->dot1q_fdb_id  - vlan id
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_address - mac address
 * OUTPUT  : dot1q_tp_fdb_entry                - The dot1qTpFdbEntry info
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 2
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_status == addr_entry->source
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1qTpFdbEntry(UI32_T dot1q_fdb_id, AMTR_MGR_Dot1qTpFdbEntry_T *dot1q_tp_fdb_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1qtpfdb);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETDOT1QTPFDBENTRY;
    msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_ui32 = dot1q_fdb_id;
    msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_dot1q_tp_fdb = *dot1q_tp_fdb_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *dot1q_tp_fdb_entry = msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_dot1q_tp_fdb;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetDot1qTpFdbEntry */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextDot1qTpFdbEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get the next dot1qTpFdbEntry info
 * INPUT   : dot1q_tp_fdb_entry->dot1q_fdb_id  - vlan id
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_address - mac address
 * OUTPUT  : dot1q_tp_fdb_entry             - The dot1qTpFdbEntry info
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 2
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_status == addr_entry->source
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1qTpFdbEntry(UI32_T *dot1q_fdb_id, AMTR_MGR_Dot1qTpFdbEntry_T *dot1q_tp_fdb_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1qtpfdb);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETNEXTDOT1QTPFDBENTRY;
    msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_ui32 = *dot1q_fdb_id;
    msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_dot1q_tp_fdb = *dot1q_tp_fdb_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *dot1q_fdb_id = msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_ui32;
    *dot1q_tp_fdb_entry = msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_dot1q_tp_fdb;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetNextDot1qTpFdbEntry */

/*---------------------------------------------------------------------- */
/* The Static (Destination-Address Filtering) Database (dot1qStatic 1) */
/*
 *             INDEX   { dot1qFdbId, dot1qStaticUnicastAddress, dot1qStaticUnicastReceivePort }
 *             ::= { dot1qStaticUnicastTable 1 }
 *
 *         Dot1qStaticUnicastEntry ::=
 *             SEQUENCE {
 *                 dot1qStaticUnicastAddress           MacAddress,
 *                 dot1qStaticUnicastReceivePort       INTEGER,
 *                 dot1qStaticUnicastAllowedToGoTo     OCTET STRING,
 *                 dot1qStaticUnicastStatus            INTEGER
 *             }
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetDot1qStaticUnicastEntry
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   dot1q_fdb_id                -- vlan id
 *              static_unitcast_entry->dot1q_static_unicast_address
 * OUTPUT   :   static_unitcast_entry       -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674q/dot1qStaticUnicastTable 1
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1qStaticUnicastEntry(UI32_T dot1q_fdb_id,
                                            AMTR_MGR_Dot1qStaticUnicastEntry_T *static_unitcast_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1qstaticunicast);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETDOT1QSTATICUNICASTENTRY;
    msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_ui32 = dot1q_fdb_id;
    msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_dot1q_static_unicast = *static_unitcast_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *static_unitcast_entry = msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_dot1q_static_unicast;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetDot1qStaticUnicastEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetNextDot1qStaticUnicastEntry
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   dot1q_fdb_id                -- vlan id
 *              static_unitcast_entry->dot1q_static_unicast_address
 * OUTPUT   :   static_unitcast_entry       -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674q/dot1qStaticUnicastTable 1
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1qStaticUnicastEntry(UI32_T *dot1q_fdb_id,
                                                AMTR_MGR_Dot1qStaticUnicastEntry_T *static_unitcast_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1qstaticunicast);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETNEXTDOT1QSTATICUNICASTENTRY;
    msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_ui32 = *dot1q_fdb_id;
    msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_dot1q_static_unicast = *static_unitcast_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    *dot1q_fdb_id = msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_ui32;
    *static_unitcast_entry = msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_dot1q_static_unicast;

    return msg_p->type.ret_bool;;
} /* End of AMTR_PMGR_GetNextDot1qStaticUnicastEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1qStaticUnicastAllowedToGoTo
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set the static allowed to go to information.
 * INPUT    :   UI32_T vid              -- vlan id
 *              UI8_T mac               -- the mac address (key)
 *              UI8_T allow_to_go_to    -- the set of ports
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC2674q/dot1qStaticUnicastEntry 3
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1qStaticUnicastAllowedToGoTo(UI32_T vid, UI8_T *mac, UI8_T *allowed_to_go_to)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac_ports);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETDOT1QSTATICUNICASTALLOWEDTOGOTO;
    msg_p->data.arg_grp_ui32_mac_ports.arg_ui32 = vid;
    memcpy(msg_p->data.arg_grp_ui32_mac_ports.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_ui32_mac_ports.arg_mac));
    memcpy(msg_p->data.arg_grp_ui32_mac_ports.arg_ports, allowed_to_go_to,
        sizeof(msg_p->data.arg_grp_ui32_mac_ports.arg_ports));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_SetDot1qStaticUnicastAllowedToGoTo */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1qStaticUnicastStatus
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set/Create the static status information.
 * INPUT    :   UI32_T vid              -- vlan id
 *              UI8_T mac               -- the mac address (key)
 *              UI32_T status           -- VAL_dot1qStaticUnicastStatus_other
 *                                         VAL_dot1qStaticUnicastStatus_invalid
 *                                         VAL_dot1qStaticUnicastStatus_permanent
 *                                         VAL_dot1qStaticUnicastStatus_deleteOnReset
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC2674q/dot1qStaticUnicastEntry 4
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1qStaticUnicastStatus(UI32_T vid, UI8_T *mac, UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETDOT1QSTATICUNICASTSTATUS;
    msg_p->data.arg_grp_ui32_mac_ui32.arg_ui32_1 = vid;
    memcpy(msg_p->data.arg_grp_ui32_mac_ui32.arg_mac, mac,
        sizeof(msg_p->data.arg_grp_ui32_mac_ui32.arg_mac));
    msg_p->data.arg_grp_ui32_mac_ui32.arg_ui32_2 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_SetDot1qStaticUnicastStatus */

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetLearningMode
 *-----------------------------------------------------------------------------
 * Purpose  : This function will set the learning mode for whole system
 * INPUT    : UI32_T learning_mode    - VAL_dot1qConstraintType_independent (IVL)
 *                                      VAL_dot1qConstraintType_shared (SVL)
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : RFC2674q
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetLearningMode(UI32_T learning_mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETLEARNINGMODE;
    msg_p->data.arg_ui32 = learning_mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_SetLearningMode */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_IsPortSecurityEnabled
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return Ture if the port security is enabled
 * INPUT   : ifindex        -- which port to get
 * OUTPUT  : none
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T  AMTR_PMGR_IsPortSecurityEnabled(UI32_T ifindex)
{
    AMTR_MGR_PortInfo_T      *amtr_port_info;

    if(AMTR_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if(ifindex == 0 || ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return FALSE;

    amtr_port_info = AMTR_OM_GetPortInfoPtr();
    if (amtr_port_info[ifindex-1].protocol != AMTR_MGR_PROTOCOL_NORMAL)
    {
        /* whether the secur port is full or not
         */
#if (SYS_DFLT_L2_ADDR_PSEC_LEARN_COUNT_INCLUDE_CONFIG)
        if((AMTRDRV_OM_GetLearntCounterByport(ifindex) + AMTRDRV_OM_GetConfigCounterByPort(ifindex))
             >= amtr_port_info[ifindex-1].learn_with_count)
            return TRUE;
#else
        if(AMTRDRV_OM_GetLearntCounterByport(ifindex) >= amtr_port_info[ifindex-1].learn_with_count)
            return TRUE;
#endif
    }/*end if(prtotcol != Normal)*/

    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_Notify_IntrusionMac
 * -------------------------------------------------------------------------
 * FUNCTION: When detecting intrusion mac, AMTR will notify other CSCs by this function.
 * INPUT   : vid    -- which vlan id
 *           mac    -- mac address
 *           ifindex-- which port
 *           is_age -- learned / aged
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_Notify_IntrusionMac(UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_ui16_mac_mac_ui16);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_NOTIFYINTRUSIONMAC;
    msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_ui32   = src_lport;
    msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_ui16_1 = vid;
    msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_ui16_2 = ether_type;
    memcpy(msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_mac_1,src_mac,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_mac_2,dst_mac,SYS_ADPT_MAC_ADDR_LEN);
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    return msg_p->type.ret_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_Notify_SecurityPortMove
 * -------------------------------------------------------------------------
 * FUNCTION: When port move, AMTR will notify other CSCs by this function.
 * INPUT   : ifindex          -- port whcih the mac is learnt now
 *           vid              -- which vlan id
 *           mac              -- mac address
 *           original_ifindex -- original port which the mac was learnt before
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_Notify_SecurityPortMove( UI32_T ifindex,
                                          UI32_T vid,
                                          UI8_T  *mac,
                                          UI32_T original_ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_mac_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_NOTIFYSECURITYPORTMOVE;
    msg_p->data.arg_grp_ui32_ui32_mac_ui32.arg_ui32_1   = ifindex;
    msg_p->data.arg_grp_ui32_ui32_mac_ui32.arg_ui32_2 = vid;
    msg_p->data.arg_grp_ui32_ui32_mac_ui32.arg_ui32_3 = original_ifindex;
    memcpy(msg_p->data.arg_grp_ui32_ui32_mac_ui32.arg_mac,mac,SYS_ADPT_MAC_ADDR_LEN);
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByVidAndLifeTime
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time and vid.
 * INPUT   : UI32_T vid - vlan id
 *           AMTR_MGR_LiftTimeMode_T life_time - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : when a instance has too many vlan trigger tc,
 *           it will generate may enven. Here change to asynchronize call
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByVidAndLifeTime(UI32_T vid, AMTR_TYPE_AddressLifeTime_T life_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_alt);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDVID;
    msg_p->data.arg_grp_ui32_alt.arg_ui32 = vid;
    msg_p->data.arg_grp_ui32_alt.arg_addresslifetime = life_time;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of AMTR_PMGR_DeleteAddrByVidAndLifeTime */

/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetCpuMac
 *------------------------------------------------------------------------------
 * Purpose  : This routine will add a CPU MAC to the chip.  Packets with this
 *            MAC in the specified VLAN will trap to CPU.
 * INPUT    : vid        -- vlan ID
 *            mac        -- MAC address
 *            is_routing -- whether the routing feature is supported and enabled
 * OUTPUT  : None
 * RETURN  : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
AMTR_TYPE_Ret_T  AMTR_PMGR_SetCpuMac(UI32_T vid, UI8_T *mac, BOOL_T is_routing)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETCPUMAC;
    msg_p->data.arg_grp_ui32_mac_bool.arg_ui32 = vid;
    msg_p->data.arg_grp_ui32_mac_bool.arg_bool = is_routing;
    memcpy(msg_p->data.arg_grp_ui32_mac_bool.arg_mac, mac, AMTR_TYPE_MAC_LEN);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return AMTR_TYPE_RET_ERROR_UNKNOWN;
    }
    return msg_p->type.ret_amtr_type;
}

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetRouterAdditionalCtrlReg
 *------------------------------------------------------------------------------
 * Purpose  : For enable static  route or meter
 * INPUT    : value
 *                      True -- Set RouterAdditionalCtrlReg, meter work
 *                      False -- unSet RouterAdditionalCtrlReg, static route work
 * OUTPUT   : None
 * RETURN   : True : successs, False : failed
 * NOTE     : None
 *------------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetRouterAdditionalCtrlReg(UI32_T value)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_mac_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETROUTERADDITIONALCTRLREG;
    msg_p->data.arg_ui32 = value;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,/*pgr0695*/
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_bool;
}
#endif /*#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)*/

/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMacNotifyGlobalStatus
 *------------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap global status
 * INPUT   : None
 * OUTPUT  : is_enabled - global status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMacNotifyGlobalStatus(BOOL_T is_enabled)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETMACNOTIFYGLOBALSTATUS;
    msg_p->data.arg_bool = is_enabled;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_bool;
}
/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMacNotifyInterval
 *------------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap interval
 * INPUT   : None
 * OUTPUT  : interval - interval to set (in seconds)
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMacNotifyInterval(UI32_T  interval)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETMACNOTIFYINTERVAL;
    msg_p->data.arg_ui32 = interval;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap port status
 * INPUT   : ifidx      - lport ifindex
 * OUTPUT  : is_enabled - port status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETMACNOTIFYPORTSTATUS;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = ifidx;
    msg_p->data.arg_grp_ui32_bool.arg_bool = is_enabled;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_bool;
}
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetRunningMacNotifyInterval
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default interval can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : interval_p   - the non-default interval
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default interval.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningMacNotifyInterval(UI32_T  *interval_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETRUNNINGMACNOTIFYINTERVAL;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *interval_p = msg_p->data.arg_ui32;

    return msg_p->type.ret_running_cfg;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetRunningMacNotifyGlobalStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default global status can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : is_enabled_p   - the non-default global status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default global status.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningMacNotifyGlobalStatus(BOOL_T  *is_enabled_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETRUNNINGMACNOTIFYGLOBALSTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *is_enabled_p = msg_p->data.arg_bool;

    return msg_p->type.ret_running_cfg;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetRunningMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default port status can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : ifidx          - lport ifindex
 * OUTPUT  : is_enabled_p   - the non-default port status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default port status.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningMacNotifyPortStatus(
    UI32_T  ifidx,
    BOOL_T  *is_enabled_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETRUNNINGMACNOTIFYPORTSTATUS;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = ifidx;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *is_enabled_p = msg_p->data.arg_grp_ui32_bool.arg_bool;

    return msg_p->type.ret_running_cfg;
}

#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_PMGR_ClearCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Remove all entries in the collision mac table.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_ClearCollisionVlanMacTable(void)
{
    const UI32_T msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_CLEARCOLLISIONVLANMACTABLE;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_bool;
}
#endif
/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteCpuMac
 *-----------------------------------------------------------------------------
 * FUNCTION: This routine will delete the CPU MAC of DUT
 * INPUT   : vid   -- vlan id
 *           mac   -- MAC address
 * OUTPUT  : None
 * RETURN  : True : successs, False : failed
 * NOTE    : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_DeleteCpuMac(UI32_T vid, UI8_T *mac)
{
    return AMTR_PMGR_DeleteInterventionEntry(vid, mac);
}

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_PMGR_GetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : *learning_p
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetVlanLearningStatus(UI32_T vid, BOOL_T *learning_p)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETVLANLEARNINGSTATUS;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *learning_p = msg_p->data.arg_grp_ui32_bool.arg_bool;

    return msg_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_PMGR_GetRunningVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get running vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : *learning_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningVlanLearningStatus(UI32_T vid, BOOL_T *learning_p)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_GETRUNNINGVLANLEARNINGSTATUS;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    *learning_p = msg_p->data.arg_grp_ui32_bool.arg_bool;
    return msg_p->type.ret_running_cfg;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_PMGR_SetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable vlan learning of specified vlan
 * INPUT    : vid
 *            learning
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetVlanLearningStatus(UI32_T vid, BOOL_T learning)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETVLANLEARNINGSTATUS;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = vid;
    msg_p->data.arg_grp_ui32_bool.arg_bool = learning;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_bool;
}
#endif
#if (SYS_CPNT_MLAG == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMlagMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * FUNCTION: To set the MLAG mac notify port status
 * INPUT   : ifidx      - lport ifindex
 *           is_enabled - port status to set
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMlagMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_SETMLAGMACNOTIFYPORTSTATUS;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = ifidx;
    msg_p->data.arg_grp_ui32_bool.arg_bool = is_enabled;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    return msg_p->type.ret_bool;
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_AuthenticatePacket
 * -------------------------------------------------------------------------
 * FUNCTION: This API will handle the packet by authenticated result
 * INPUT   : src_mac     --  source mac address
 *           vid         --  VLAN id
 *           lport       --  logical port ifindex
 *           auth_result --  authentication result
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *--------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_AuthenticatePacket(
    UI8_T *src_mac,
    UI32_T vid,
    UI32_T lport,
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T auth_result)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_auth_pkt);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_AUTHENTICATEPACKET;
    msg_p->data.arg_auth_pkt.vid = vid;
    msg_p->data.arg_auth_pkt.lport = lport;
    msg_p->data.arg_auth_pkt.auth_result = auth_result;
    memcpy(msg_p->data.arg_auth_pkt.src_mac,src_mac, SYS_ADPT_MAC_ADDR_LEN);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return  FALSE;
    }

    return msg_p->type.ret_bool;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByVidAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by vid and Port.
 * INPUT   : UI32_T ifindex - interface index
 *           UI32_T vid - VLAN ID
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_DeleteAddrByVidAndLPort(UI32_T lport, UI32_T vid)
{
    const UI32_T msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSFUN_Msg_T *resp_msgbuf_p = NULL;
    AMTR_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_MGR_IPC_DELETEADDRBYVIDANDLPORT;
    msg_p->data.arg_ui32_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_ui32_ui32_ui32.arg_ui32_2 = vid;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            AMTR_MGR_IPCMSG_TYPE_SIZE, resp_msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
} /* End of AMTR_PMGR_DeleteAddrByVidAndLPort */
/* LOCAL SUBPROGRAM BODIES
 */
