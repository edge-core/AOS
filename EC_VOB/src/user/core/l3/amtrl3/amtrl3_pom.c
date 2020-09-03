/*-----------------------------------------------------------------------------
 * FILE NAME: AMTRL3_POM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for AMTRL3 OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/01/30     --- djd, Create
 *
 * Copyright(C)      Accton Corporation, 2008
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
#include "amtrl3_pom.h"
#include "amtrl3_type.h"
#include "amtrl3_om.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* trace id definition when using L_MM
 */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

static SYSFUN_MsgQ_T amtrl3_ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for AMTRL3_POM in the calling process.
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
BOOL_T AMTRL3_POM_InitiateProcessResource(void)
{
    /* get the ipc message queues for AMTRL3 OM
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &amtrl3_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetIpNetToPhysicalEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  To get record of IpNetToPhysical entry matching specified key
 *
 * INPUT: 
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      ip_net_to_physical_entry -- get the entry match the key fields in this structure
 *                                  KEY: 
 *                                  ip_net_to_physical_if_index
 *                                  ip_net_to_physical_net_address_type
 *                                  ip_net_to_physical_net_address
 *
 *
 * OUTPUT:   ip_net_to_physical_entry -- record of IpNetToPhysical table.
 *
 * RETURN:   TRUE - Successfully
 *           FALSE- can't get the specified record.
 *
 * NOTES:
 *      1. action flag AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *         address type in ip_net_to_physical_entry structure.
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_POM_GetIpNetToPhysicalEntry(
                                                UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_TYPE_ipNetToPhysicalEntry_T  *ip_net_to_physical_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_OM_GET_MSG_SIZE(ip_net_to_physical);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_OM_IPCCMD_GETIPNETTOPHYSICALENTRY;
    msg_p->data.ip_net_to_physical.action_flags = action_flags;
    msg_p->data.ip_net_to_physical.fib_id = fib_id;
    memcpy(&msg_p->data.ip_net_to_physical.ip_net_to_physical_entry, ip_net_to_physical_entry, sizeof(AMTRL3_TYPE_ipNetToPhysicalEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    
    memcpy(ip_net_to_physical_entry, &msg_p->data.ip_net_to_physical.ip_net_to_physical_entry, sizeof(AMTRL3_TYPE_ipNetToPhysicalEntry_T));
    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetNextIpNetToPhysicalEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  To get the next record of ipNetToPhysical entry after the specified key
 *
 * INPUT:    
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      ip_net_to_physical_entry -- get the entry match the key fields in this structure
 *                                  KEY: 
 *                                  ip_net_to_physical_if_index
 *                                  ip_net_to_physical_net_address_type
 *                                  ip_net_to_physical_net_address
 *
 * OUTPUT:   ip_net_to_physical_entry -- record of ipNetToPhysical table.
 *
 * RETURN:   TRUE - Successfully
 *           FALSE- No more record (EOF) or can't get other record.
 *
 * NOTES:    
 *      1. If only AMTRL3_TYPE_FLAGS_IPV4 is set (address type must be IPV4), 
 *         only traverse all ipv4 entries.
 *      2. If only AMTRL3_TYPE_FLAGS_IPV6 is set (address type must be IPV6)
 *         only traverse all ipv6 entries.
 *      3. If both IPV4 and IPV6 flag are set (address type may be IPV4 or IPV6),  
 *         if address type is IPV4, traverse IPV4 entries at first then continue on IPV6 entries.
 *         if address type is IPV6, only traverse IPV6 entries.
 *      4. if all keys are zero, means get first one.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_POM_GetNextIpNetToPhysicalEntry (UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_TYPE_ipNetToPhysicalEntry_T  *ip_net_to_physical_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_OM_GET_MSG_SIZE(ip_net_to_physical);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_OM_IPCCMD_GETNEXTIPNETTOPHYSICALENTRY;
    msg_p->data.ip_net_to_physical.action_flags = action_flags;
    msg_p->data.ip_net_to_physical.fib_id = fib_id;
    memcpy(&msg_p->data.ip_net_to_physical.ip_net_to_physical_entry, ip_net_to_physical_entry, sizeof(AMTRL3_TYPE_ipNetToPhysicalEntry_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(ip_net_to_physical_entry, &msg_p->data.ip_net_to_physical.ip_net_to_physical_entry, sizeof(AMTRL3_TYPE_ipNetToPhysicalEntry_T));
    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetInetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get ipv4 or ipv6 Host Route Entry
 * INPUT:   
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      inet_host_route_entry->dst_vid_ifindex --  ifindex (KEY)
 *      inet_host_route_entry->inet_address --  Destination IPv4/v6 Address (KEY)
 * OUTPUT:  inet_host_route_entry
 * RETURN:  TRUE / FALSE    
 * NOTES:
 *      1. If AMTRL3_TYPE_FLAGS_IPV4 is set, only search all ipv4 entries.
 *      2. If AMTRL3_TYPE_FLAGS_IPV6 is set, only search all ipv6 entries.
 *      3. Can not set these 2 flags at same time.
 *         flag must be consistent with inet_host_route_entry->inet_address_type.
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_POM_GetInetHostRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_OM_GET_MSG_SIZE(host_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_OM_IPCCMD_GETINETHOSTROUTEENTRY;
    msg_p->data.host_route.action_flags = action_flags;
    msg_p->data.host_route.fib_id = fib_id;
    msg_p->data.host_route.host_entry.dst_vid_ifindex = host_entry->dst_vid_ifindex;
    memcpy(&msg_p->data.host_route.host_entry.dst_inet_addr, &host_entry->dst_inet_addr, sizeof(L_INET_AddrIp_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    
    memcpy(host_entry, &msg_p->data.host_route.host_entry, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetNextInetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get next ipv4 or ipv6 Host Route Entry
 * INPUT:   
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      inet_host_route_entry->dst_vid_ifindex --  ifindex (KEY)
 *      inet_host_route_entry->inet_address --  Destination IPv4/v6 Address (KEY)
 * OUTPUT:  inet_host_route_entry
 * RETURN:  TRUE / FALSE    
 * NOTES:
 *      1. If AMTRL3_TYPE_FLAGS_IPV4 is set, only search all ipv4 entries.
 *      2. If AMTRL3_TYPE_FLAGS_IPV6 is set, only search all ipv6 entries.
 *      3. If the inet_host_route_entry->inet_address = 0, get the first entry
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_POM_GetNextInetHostRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_OM_GET_MSG_SIZE(host_route);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_OM_IPCCMD_GETNEXTINETHOSTROUTEENTRY;
    msg_p->data.host_route.action_flags = action_flags;
    msg_p->data.host_route.fib_id = fib_id;
    msg_p->data.host_route.host_entry.dst_vid_ifindex = host_entry->dst_vid_ifindex;
    memcpy(&msg_p->data.host_route.host_entry.dst_inet_addr, &host_entry->dst_inet_addr, sizeof(L_INET_AddrIp_T));

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    
    memcpy(host_entry, &msg_p->data.host_route.host_entry, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    return msg_p->type.result_bool;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetTotalHostRouteNumber
 * -------------------------------------------------------------------------
 * PURPOSE:  To get the total number of host route entries include ipv4 and ipv6 
 * INPUT:  
 *        action_flags:   AMTRL3_TYPE_FLAGS_IPV4 -- include ipv4 entries
 *                        AMTRL3_TYPE_FLAGS_IPV6 -- include ipv6 entries 
 *        fib_id      :   FIB id
 * OUTPUT:
 * RETURN: total number of host route number which satisfying action_flags
 * NOTES:  none
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_POM_GetTotalHostRouteNumber(UI32_T action_flags, UI32_T fib_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_OM_GET_MSG_SIZE(total_host_route_nbr);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_OM_IPCCMD_GETTOTALHOSTROUTENUMBER;
    msg_p->data.total_host_route_nbr.action_flags = action_flags;
    msg_p->data.total_host_route_nbr.fib_id = fib_id;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return 0;
    }

    return msg_p->type.result_ui32;
}

#if (SYS_CPNT_VXLAN == TRUE)
BOOL_T AMTRL3_POM_GetVxlanTunnelEntryByVxlanPort(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T *tunnel_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_OM_GET_MSG_SIZE(vxlan_tunnel);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_OM_IPCCMD_GET_VXLAN_TUNNEL_ENTRY_BY_VXLAN_PORT;
    msg_p->data.vxlan_tunnel.tunnel = *tunnel_entry_p;
    msg_p->data.vxlan_tunnel.fib_id = fib_id;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (msg_p->type.result_bool == TRUE)
        *tunnel_entry_p = msg_p->data.vxlan_tunnel.tunnel;

    return msg_p->type.result_bool;
}

BOOL_T AMTRL3_POM_GetVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_OM_GET_MSG_SIZE(vxlan_tunnel_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_OM_IPCCMD_GET_VXLAN_TUNNEL_ENTRY;
    msg_p->data.vxlan_tunnel_entry.tunnel = *tunnel_entry_p;
    msg_p->data.vxlan_tunnel_entry.fib_id = fib_id;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (msg_p->type.result_bool == TRUE)
        *tunnel_entry_p = msg_p->data.vxlan_tunnel_entry.tunnel;

    return msg_p->type.result_bool;
}

BOOL_T AMTRL3_POM_GetNextVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = AMTRL3_OM_GET_MSG_SIZE(vxlan_tunnel_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTRL3_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTRL3;
    msgbuf_p->msg_size = msg_size;

    msg_p = (AMTRL3_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTRL3_OM_IPCCMD_GET_NEXT_VXLAN_TUNNEL_ENTRY;
    msg_p->data.vxlan_tunnel_entry.tunnel = *tunnel_entry_p;
    msg_p->data.vxlan_tunnel_entry.fib_id = fib_id;

    if (SYSFUN_SendRequestMsg(amtrl3_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (msg_p->type.result_bool == TRUE)
        *tunnel_entry_p = msg_p->data.vxlan_tunnel_entry.tunnel;

    return msg_p->type.result_bool;
}

#endif


