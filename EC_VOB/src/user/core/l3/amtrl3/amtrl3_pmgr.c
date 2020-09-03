/* MODULE NAME:  amtrl3_pmgr.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access AMTRL3_MGR and AMTRL3_OM service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call AMTRL3_PMGR_XXX for APIs AMTRL3_MGR_XXX provided by AMTRL3, and same as AMTRL3_POM for
 *    AMTRL3_OM APIs
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    01/16/2008 - djd, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_bld.h"
#include "amtrl3_pmgr.h"
#include "sys_module.h"
#include "l_mm.h"
#include "amtrl3_type.h"
#include "amtrl3_mgr.h"

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
static SYSFUN_MsgQ_T amtrl3_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : AMTRL3_PMGR_InitiateProcessResource
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
 *    Before other CSC use AMTRL3_PMGR, it should initiate the resource (get the message queue handler internally)
 *
 */
BOOL_T AMTRL3_PMGR_InitiateProcessResource(void)
{
    /* Given that CSCA PMGR requests are handled in CSCGROUP1 of XXX_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &amtrl3_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_CreateFIB
 * -------------------------------------------------------------------------
 * PURPOSE:  create a FIB instance
 * INPUT:    fib_id       --  FIB id (1 ~ 255).
 * OUTPUT:   none.
 * RETURN:   AMTRL3_TYPE_FIB_SUCCESS, 
 *           AMTRL3_TYPE_FIB_ALREADY_EXIST,
 *           AMTRL3_TYPE_FIB_OM_ERROR,
 *           AMTRL3_TYPE_FIB_FAIL.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_PMGR_CreateFIB(UI32_T fib_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_CREATEFIB;
    msg_p->data.ui32_v = fib_id;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return AMTRL3_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_DeleteFIB()
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete a FIB
 * INPUT:    fib_id  --  FIB id (1 ~ 255).
 * OUTPUT:   none.
 * RETURN:   AMTRL3_PMGR_FIB_SUCCESS, 
 *           AMTRL3_PMGR_FIB_NOT_EXIST,
 *           AMTRL3_PMGR_FIB_OM_ERROR,
 *           AMTRL3_PMGR_FIB_FAIL.
 * NOTES:   
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_PMGR_DeleteFIB(UI32_T fib_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DELETEFIB;
    msg_p->data.ui32_v = fib_id;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return AMTRL3_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_EnableIpForwarding
 * -------------------------------------------------------------------------
 * PURPOSE:  Enable ipv4 or ipv6 forwarding function in chip.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           vr_id          --  Virtual Router id            
 * OUTPUT:   none.               
 * RETURN:   TRUE / FALSE
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_EnableIpForwarding(UI32_T action_flags, UI32_T vr_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(ip_forwarding_status_index);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_ENABLEIPFORWARDING;
    msg_p->data.ip_forwarding_status_index.action_flags = action_flags;
    msg_p->data.ip_forwarding_status_index.vr_id = vr_id;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_DisableIpForwarding
 * -------------------------------------------------------------------------
 * PURPOSE:  Disable ipv4 or ipv6 forwarding function in chip.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           vr_id          --  Virtual Router id            
 * OUTPUT:   none.               
 * RETURN:   TRUE / FALSE
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_DisableIpForwarding(UI32_T action_flags, UI32_T vr_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(ip_forwarding_status_index);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DISABLEIPFORWARDING;
    msg_p->data.ip_forwarding_status_index.action_flags = action_flags;
    msg_p->data.ip_forwarding_status_index.vr_id = vr_id;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_SetHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one ipv4 or ipv6 host route to Host Route database 
 *           and host route table in chip.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id            
 *           host_entry      --  Inet Host Route Entry
 *                               all structure fileds must be set
 *           type            --  VAL_ipNetToPhysicalExtType_other     \
 *                               VAL_ipNetToPhysicalExtType_invalid   \
 *                               VAL_ipNetToPhysicalExtType_dynamic   \
 *                               VAL_ipNetToPhysicalExtType_static    \
 *                               VAL_ipNetToPhysicalExtType_local     \
 *                               VAL_ipNetToPhysicalExtType_broadcast \
 *                               VAL_ipNetToPhysicalExtType_vrrp      \  
 * OUTPUT:   none.               
 * RETURN:   TRUE / FALSE
 * NOTES:    1. IML should get the return value to decide whether the ARP pkt
 *              should be sent to TCP/IP stack, if it is TRUE, need send.
 *           2. For link local ipv6 address, address type and zone index must 
 *              be correctly set.
 *           3. only one flag can be set in one time
 *           4. if host exists, will replace it
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_SetHostRoute(UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_TYPE_InetHostRouteEntry_T *host_entry,
                                            UI32_T type)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(host_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_SETHOSTROUTE;
    msg_p->data.host_route.action_flags = action_flags;
    msg_p->data.host_route.fib_id = fib_id;
    memcpy(&msg_p->data.host_route.host_entry, host_entry, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    msg_p->data.host_route.type = type;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_DeleteHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:
 *      Remove IPv4 or IPv6 neighbor entry base on given IpNetToPhsical entry information
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id          -- FIB id
 *      inet_host_entry -- delete the entry match the key fields in this structure
 *                         KEY: dst_inet_addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE : - Successfully delete the entry.
 *      FALSE: -- the host entry do not exist.
 *             -- Error in removing specific entry
 *
 * NOTES:
 *      1. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *         address type in ip_net_to_physical_entry structure.
 -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_DeleteHostRoute(UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_TYPE_InetHostRouteEntry_T *host_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(host_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DELETEHOSTROUTE;
    msg_p->data.host_route.action_flags = action_flags;
    msg_p->data.host_route.fib_id = fib_id;
    memcpy(&msg_p->data.host_route.host_entry, host_entry, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

#if (SYS_CPNT_PBR == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_DeleteHostRouteForPbr
 * -------------------------------------------------------------------------
 * PURPOSE:  Add a host route entry in OM if it doesn't exist yet
 * INPUT:    action_flags   -- AMTRL3_TYPE_FLAGS_IPV4
 *           fib_id         -- FIB id            
 *           host_entry     -- host route entry
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTES:    1. dst_mac is ignored.
 *           2. do address resolution on dst_vid_ifindex.
 *           3. increase ref_count & pbr_ref_count, and do address resolution(ARP)
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_PMGR_SetHostRouteForPbr(UI32_T action_flags,
                                      UI32_T fib_id,
                                      AMTRL3_TYPE_InetHostRouteEntry_T *host_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(host_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_SETHOSTROUTEFORPBR;
    msg_p->data.host_route.action_flags = action_flags;
    msg_p->data.host_route.fib_id = fib_id;
    memcpy(&msg_p->data.host_route.host_entry, host_entry, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_DeleteHostRouteForPbr
 * -------------------------------------------------------------------------
 * PURPOSE:  Unreference host route entry.
 * INPUT:    action_flags   -- AMTRL3_TYPE_FLAGS_IPV4
 *           fib_id         -- FIB id            
 *           host_entry     -- host route entry
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTES:    1. decrease the ref_count & pbr_ref_count
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_DeleteHostRouteForPbr(UI32_T action_flags,
                                         UI32_T fib_id,
                                         AMTRL3_TYPE_InetHostRouteEntry_T *host_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(host_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DELETEHOSTROUTEFORPBR;
    msg_p->data.host_route.action_flags = action_flags;
    msg_p->data.host_route.fib_id = fib_id;
    memcpy(&msg_p->data.host_route.host_entry, host_entry, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}
#endif


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_ReplaceExistHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Modify an existing host route, this function will search the host
 *           table, if the IP existed, then update the entry based on the new
 *           information, if the IP is not exist in the table then do nothing.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       --  FIB id
 *           host_entry      --  host route entry
 *           type            --  VAL_ipNetToPhysicalExtType_other     \
 *                               VAL_ipNetToPhysicalExtType_invalid   \
 *                               VAL_ipNetToPhysicalExtType_dynamic   \
 *                               VAL_ipNetToPhysicalExtType_static    \
 *                               VAL_ipNetToPhysicalExtType_local     \
 *                               VAL_ipNetToPhysicalExtType_broadcast \
 *                               VAL_ipNetToPhysicalExtType_vrrp      \  
 * OUTPUT:   none.               
 * RETURN:   TRUE / FALSE
 * NOTE:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_ReplaceExistHostRoute(UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry,
                                        UI32_T type)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(host_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_REPLACEHOSTROUTE;
    msg_p->data.host_route.action_flags = action_flags;
    msg_p->data.host_route.fib_id = fib_id;
    memcpy(&msg_p->data.host_route.host_entry, host_entry, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    msg_p->data.host_route.type = type;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_SetIpNetToPhysicalEntryTimeout
 * -------------------------------------------------------------------------
 * PURPOSE:  Set host route table ageout timer
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       --  FIB id
 *           v4_timeout      --  ageout time in seconds for ipv4 entries
 *           v6_timeout      --  ageout time in seconds for ipv6 entries
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:    Only set the age timer with the corresponding flag set.
 *           If only AMTRL3_TYPE_FLAGS_IPV4 was set, only ipv4 entries
 *           ageout timer will be set.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_SetIpNetToPhysicalEntryTimeout(UI32_T action_flags,
                                                    UI32_T fib_id,
                                                    UI32_T v4_timeout,
                                                    UI32_T v6_timeout)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(host_timeout);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_SETHOSTROUTETIMEOUT;
    msg_p->data.host_timeout.action_flags = action_flags;
    msg_p->data.host_timeout.fib_id = fib_id;
    msg_p->data.host_timeout.v4_timeout = v4_timeout;
    msg_p->data.host_timeout.v6_timeout = v6_timeout;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_SetInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Set one record to ipv4 or ipv6 net route table 
 *          and set configuration to driver
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- record of forwarding table.
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: 
 *      TRUE : AMTRL3_MGR_OK              -- Successfully insert to database
 *      FALSE: AMTRL3_MGR_ALREADY_EXIST   -- this entry existed in chip/database.
 *             AMTRL3_MGR_TABLE_FULL      -- over reserved space
 *                                       
 * NOTES: 1. If the entry with same key value already exist, this function will
 *        update this entry except those key fields value.
 *        2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *        address type in net_route_entry structure.
 *        3. for dest and nexthop address, unused bytes must be zero. 
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_SetInetCidrRouteEntry(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(net_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_SETINETCIDRROUTE;
    msg_p->data.net_route.action_flags = action_flags;
    msg_p->data.net_route.fib_id = fib_id;
    memcpy(&msg_p->data.net_route.net_route_entry.partial_entry, &net_route_entry->partial_entry, sizeof(AMTRL3_TYPE_InetCidrRoutePartialEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_DeleteInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Delete specified ipv4 or ipv6 record with the key from database and chip.
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- record of forwarding table, only key fields are useful
 *                         in this function.
 *                         KEY :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.

 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    - AMTRL3_MGR_OK                    -- Successfully remove from database
 *      FALSE   - AMTRL3_MGR_CAN_NOT_FIND          -- This entry does not exist in chip/database.
 *                AMTRL3_MGR_CAN_NOT_DELETE        -- Cannot Delete
 *
 * NOTES:
 *      1. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *      address type in net_route_entry structure.
 *      2. Only key fields in net_route_entry are used in this function. 
 *      Other fields can have random value.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_DeleteInetCidrRouteEntry( UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(net_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DELETEINETCIDRROUTE;
    msg_p->data.net_route.action_flags = action_flags;
    msg_p->data.net_route.fib_id = fib_id;
    memcpy(&msg_p->data.net_route.net_route_entry.partial_entry, &net_route_entry->partial_entry, sizeof(AMTRL3_TYPE_InetCidrRoutePartialEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_AddECMPRouteMultiPath
 * -------------------------------------------------------------------------
 * PURPOSE: Set ECMP route with multiple paths
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- array records of forwarding table, only key fields are useful
 *                          in this function
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK       -- Successfully insert to database
 *         FALSE: AMTRL3_MGR_TABLE_FULL      -- over reserved space
 * NOTES: 1. The key fields (dest_type, dest, pfxlen, policy) must be 
 *           the same in all records of the array. Upper layer (NSM) must confirm.
 *        2. If route exists, it must set the flag AMTRL3_TYPE_FLAGS_ECMP.
 *        3. If path number overflows the spec, only add those in front.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_AddECMPRouteMultiPath(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry,
                                          UI32_T num)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(ecmp_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_ADDECMPROUTEMULTIPATH;
    msg_p->data.ecmp_route.action_flags = action_flags;
    msg_p->data.ecmp_route.fib_id = fib_id;
    msg_p->data.ecmp_route.num = num;
    memcpy(msg_p->data.ecmp_route.net_route_entry, net_route_entry, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T) * num);

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_DeleteECMPRoute
 * -------------------------------------------------------------------------
 * PURPOSE: Delete the ECMP route with all paths.
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- only key fields (dest and pfxlen) are useful for deleting
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK      -- Successfully remove from database
 *         FALSE: 
 *                -AMTRL3_MGR_CAN_NOT_FIND -- This entry does not exist in chip/database.
 *                -AMTRL3_MGR_CAN_NOT_DELETE   -- Cannot Delete 
 * NOTES: 1. If route exists, it must set the flag AMTRL3_TYPE_FLAGS_ECMP. 
 *        2. action_flags: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with address type in dest
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_DeleteECMPRoute(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(net_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DELETEECMPROUTE;
    msg_p->data.net_route.action_flags = action_flags;
    msg_p->data.net_route.fib_id = fib_id;
    memcpy(&msg_p->data.net_route.net_route_entry, net_route_entry, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_AddECMPRouteOnePath
 * -------------------------------------------------------------------------
 * PURPOSE: Set ECMP route with one path
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- records of forwarding table, only key fields are useful 
 *                          in this function
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK       -- Successfully insert to database
 *         FALSE: AMTRL3_MGR_TABLE_FULL      -- over reserved space
 * NOTES: 1. If route exists, it must set the flag AMTRL3_TYPE_FLAGS_ECMP. 
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_AddECMPRouteOnePath(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(net_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_ADDECMPROUTEONEPATH;
    msg_p->data.net_route.action_flags = action_flags;
    msg_p->data.net_route.fib_id = fib_id;
    memcpy(&msg_p->data.net_route.net_route_entry, net_route_entry, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_DeleteECMPRouteOnePath
 * -------------------------------------------------------------------------
 * PURPOSE: Delete the ECMP route with the given path
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- records of forwarding table, only key fields are useful 
 *                          in this function
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK       -- Successfully remove from database
 *         FALSE: 
 *                   -AMTRL3_MGR_CAN_NOT_FIND -- This entry does not exist in chip/database.
 *                    AMTRL3_MGR_CAN_NOT_DELETE        -- Cannot Delete
 * NOTES: 1. If route exists, it must set the flag AMTRL3_TYPE_FLAGS_ECMP. 
 *        2. action_flags: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with address type in dest.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_DeleteECMPRouteOnePath(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(net_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DELETEECMPROUTEONEPATH;
    msg_p->data.net_route.action_flags = action_flags;
    msg_p->data.net_route.fib_id = fib_id;
    memcpy(&msg_p->data.net_route.net_route_entry, net_route_entry, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_PMGR_AddL3Mac
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one MAC address with L3 bit On
 * INPUT:    
 *           l3_mac:       MAC address of vlan interface
 *           vid_ifindex:    VLAN ifindex of this MAC address
 * OUTPUT:   none.
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------*/ 
BOOL_T AMTRL3_PMGR_AddL3Mac(UI8_T *l3_mac, UI32_T vid_ifIndex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(l3_if);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_ADDL3MAC;
    memcpy(&msg_p->data.l3_if.route_mac, l3_mac, SYS_ADPT_MAC_ADDR_LEN);
    msg_p->data.l3_if.ifindex = vid_ifIndex;
    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_PMGR_DeleteL3Mac
 * -------------------------------------------------------------------------
 * PURPOSE: Remove one L3 MAC address that belonging to one vlan interface.
 * INPUT:   
 *          l3_mac:       MAC address of vlan interface
 *          vid_ifIndex:    VLAN ifIndex of this MAC address
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/ 
BOOL_T AMTRL3_PMGR_DeleteL3Mac(UI8_T *l3_mac, UI32_T vid_ifIndex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(l3_if);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DELETEL3MAC;
    memcpy(&msg_p->data.l3_if.route_mac, l3_mac, SYS_ADPT_MAC_ADDR_LEN);
    msg_p->data.l3_if.ifindex = vid_ifIndex;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_ClearAllDynamicARP
 * -------------------------------------------------------------------------
 * PURPOSE: Clear all dynamic ARP from chip and TCP/IP stack
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id          -- FIB id
 *
 * OUTPUT: none.
 *
 * RETURN: 
 *          TRUE        -- Delete successfully
 *          FALSE:      -- Delete fail
 * NOTES:  If set AMTRL3_TYPE_FLAGS_IPV4 flag, only delete all ipv4 dynamic ARP.
 *         If set AMTRL3_TYPE_FLAGS_IPV6 flag, only delete all ipv6 dynamic ARP.
 *         If all flag set, delete all ipv4 & ipv6 dynamic ARP.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_ClearAllDynamicARP(UI32_T action_flags, UI32_T fib_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(clear_arp);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_CLEARALLDYNAMICARP;
    msg_p->data.clear_arp.action_flags = action_flags;
    msg_p->data.clear_arp.fib_id = fib_id;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_PMGR_MACTableDeleteByMstidnPort
 *------------------------------------------------------------------------
 * FUNCTION: The function is used to delete port mac on this msit associated vlan
 * INPUT   : lport_ifindex - specific port removed from vlan member list
 *           mstid - specific spaning-tree msti index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : none                                                         
 *------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_MACTableDeleteByMstidOnPort(UI32_T mstid, UI32_T lport_ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(arg_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_MACTABLEDELETEBYMSTIDONPORT;
    msg_p->data.arg_ui32_ui32.ui32_1 = mstid;
    msg_p->data.arg_ui32_ui32.ui32_2 = lport_ifindex;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, NULL) != SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_PMGR_SignalL3IfRifDestroy
 *------------------------------------------------------------------------
 * FUNCTION: The function is used to inform the destroy of rif on interface
 * INPUT   : ifindex   - the interface where rif is set
 *           ip_addr_p - the ip address of the rif
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : none                                                         
 *------------------------------------------------------------------------
 */
BOOL_T AMTRL3_PMGR_SignalL3IfRifDestroy(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(arg_ui32_addr);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_SIGNAL_L3IF_RIF_DESTROY;
    msg_p->data.arg_ui32_addr.ui32_1 = ifindex;
    msg_p->data.arg_ui32_addr.addr_2 = *ip_addr_p;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, NULL) != SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
}

#if (SYS_CPNT_IP_TUNNEL == TRUE)
/* no longer use */
#if 0
BOOL_T  AMTRL3_PMGR_SetDynamicTunnel(UI32_T  tidIfindex, UI32_T  tunnel_type,   L_INET_AddrIp_T * src_addr, L_INET_AddrIp_T * dest_addr)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(tunnel_host_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_CREATEDYNAMICTUNNEL;
    msg_p->data.tunnel_host_route.egress_tidIfindex =  tidIfindex;
    msg_p->data.tunnel_host_route.host_type = tunnel_type;
    msg_p->data.tunnel_host_route.src_addr = *src_addr;
    msg_p->data.tunnel_host_route.dest_addr = *dest_addr;


    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, NULL) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}
#endif
/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_PMGR_DeleteTunnelEntries
 *--------------------------------------------------------------------------
 * PURPOSE  : This function deletes tunnel's host route
 * INPUT    : fibid        -- fib index
 *            tidIfindex   -- tunnle l3 ifindex 
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T AMTRL3_PMGR_DeleteTunnelEntries(UI32_T fib_id, UI32_T  tidIfindex)
{

    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(arg_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DELETETUNNELENTRIES;
    msg_p->data.arg_ui32_ui32.ui32_1 = fib_id;
    msg_p->data.arg_ui32_ui32.ui32_2 = tidIfindex;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return AMTRL3_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME:  AMTRL3_PMGR_TunnelUpdateTtl
 * PURPOSE : Update chip's tunnel ttl value 
 *
 * INPUT   : host_entry->dst_vid_ifindex         -- tunnel's vlan ifindex
 *                       tunnel_dest_inet_addr   -- ipv4 destination endpoint address
 *                       tunnel_src_vidifindex   -- ipv4 source endpoint vlan ifindex
 *                       tunnel_entry_type       -- tunnel mode
 *           ttl                                 -- ttl in ipv4 header   
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE/FALSE.
 *
 * NOTES   : None.
 *
 */
BOOL_T AMTRL3_PMGR_TunnelUpdateTtl(AMTRL3_TYPE_InetHostRouteEntry_T *host_entry, UI32_T  ttl)
{

    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(tunnel_update_ttl);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_UPDATETUNNELTTL;
    memcpy(&(msg_p->data.tunnel_update_ttl.host_entry),host_entry, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    msg_p->data.tunnel_update_ttl.ttl = ttl;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return AMTRL3_TYPE_FAIL;
    }

    return msg_p->type.result_bool;
}

#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_VXLAN == TRUE)
BOOL_T AMTRL3_PMGR_AddVxlanTunnel(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T* vxlan_tunnel_p)
{
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(vxlan_tunnel);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_ADD_VXLAN_TUNNEL;
    msg_p->data.vxlan_tunnel.fib_id = fib_id;
    memcpy(&msg_p->data.vxlan_tunnel.tunnel, vxlan_tunnel_p, sizeof(AMTRL3_TYPE_VxlanTunnelEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

BOOL_T AMTRL3_PMGR_DeleteVxlanTunnel(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T* vxlan_tunnel_p)
{
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(vxlan_tunnel);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DEL_VXLAN_TUNNEL;
    msg_p->data.vxlan_tunnel.fib_id = fib_id;
    memcpy(&msg_p->data.vxlan_tunnel.tunnel, vxlan_tunnel_p, sizeof(AMTRL3_TYPE_VxlanTunnelEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

BOOL_T AMTRL3_PMGR_AddVxlanTunnelNexthop(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelNexthopEntry_T* tunnel_nexthop_p)
{
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(vxlan_tunnel_nexthop);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_ADD_VXLAN_TUNNEL_NEXTHOP;
    msg_p->data.vxlan_tunnel_nexthop.fib_id = fib_id;
    memcpy(&msg_p->data.vxlan_tunnel_nexthop.nexthop, tunnel_nexthop_p, sizeof(AMTRL3_TYPE_VxlanTunnelNexthopEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}

BOOL_T AMTRL3_PMGR_DeleteVxlanTunnelNexthop(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelNexthopEntry_T* tunnel_nexthop_p)
{
    const UI32_T msg_size = AMTRL3_MGR_GET_MSG_SIZE(vxlan_tunnel_nexthop);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_MGR_IPCCMD_DEL_VXLAN_TUNNEL_NEXTHOP;
    msg_p->data.vxlan_tunnel_nexthop.fib_id = fib_id;
    memcpy(&msg_p->data.vxlan_tunnel_nexthop.nexthop, tunnel_nexthop_p, sizeof(AMTRL3_TYPE_VxlanTunnelNexthopEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}
#endif

