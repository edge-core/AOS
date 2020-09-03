/* MODULE NAME:  netcfg_pom_ip.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    access NETCFG_OM_IP service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    02/19/2008 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sysfun.h"
#include "sys_bld.h"
#include "netcfg_pom_ip.h"
#include "netcfg_om_ip.h"
#include "netcfg_type.h"
#include "sys_module.h"
#include "sys_dflt.h"
#include "l_mm.h"

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
static SYSFUN_MsgQ_T netcfg_om_ip_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_POM_IP_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource for CSCA_POM in the calling process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use NETCFG_POM_IP, it should initiate the resource (get the message queue handler internally)
 
 *
 */
BOOL_T NETCFG_POM_IP_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_NETCFG_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &netcfg_om_ip_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME : NETCFG_POM_IP_GetRifConfig
 * PURPOSE:
 *      Get the rif by the ip address.
 *
 * INPUT:
 *      rif_config_p        -- pointer to the rif_config
 *      
 * OUTPUT:
 *      rif_config_p        -- pointer to the rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 */
UI32_T NETCFG_POM_IP_GetRifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETRIFCONFIG;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_POM_IP_GetNextRifConfig
 * PURPOSE:
 *      Get the rif by the ip address.
 *
 * INPUT:
 *      rif_config_p        -- pointer to the rif_config
 *      
 * OUTPUT:
 *      rif_config_p        -- pointer to the rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 *      If you need ifindex, Inet prefix as the KEY such as 
 *      SNMP GetNext, CLI running-config, this function is not
 *      the right choice.
 */
UI32_T NETCFG_POM_IP_GetNextRifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTRIFCONFIG;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME: NETCFG_POM_IP_GetRifFromInterface
 * PURPOSE:
 *      Get the IPv4 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->addr.addr         -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv4 only.
 */
UI32_T NETCFG_POM_IP_GetRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETRIFFROMINTERFACE;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME: NETCFG_POM_IP_GetPrimaryRifFromInterface
 * PURPOSE:
 *      Get the primary rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None
 */
UI32_T NETCFG_POM_IP_GetPrimaryRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETPRIMARYRIFFROMINTERFACE;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: NETCFG_POM_IP_GetSecondaryRifFromInterface
 * PURPOSE:
 *      Get the secondary rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None
 */
UI32_T NETCFG_POM_IP_GetNextSecondaryRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTSECONDARYRIFFROMINTERFACE;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME: NETCFG_POM_IP_GetNextRifFromInterface
 * PURPOSE:
 *      Get the next rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex   -- the beginning interface trying to search (key).
 *      rif_config_p->ip_addr   -- ip address (key).
 *      rif_config_p->mask      -- mask (key).
 *
 * OUTPUT:
 *      rif_config_p    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. This function WILL iterate to the next L3 interface.
 *      2. CLI "show ip interface" and SNMP GetNext should call this function.
 */
UI32_T NETCFG_POM_IP_GetNextRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTRIFFROMINTERFACE;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME: NETCFG_POM_IP_GetNextInetRifOfInterface
 * PURPOSE:
 *      Get the next rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p    -- pointer to the rif_config.
 *
 * OUTPUT:
 *      rif_config_p    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. This function WILL NOT iterate to the next L3 interface.
 *      2. For IPv4/IPv6.
 */
UI32_T NETCFG_POM_IP_GetNextInetRifOfInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTINETRIFOFINTERFACE;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_POM_IP_GetRifFromIp
 * PURPOSE:
 *      Get the rif whose subnet covers the target IP address.
 *
 * INPUT:
 *      rif_config_p->ip_addr   -- the target IP address for checking.
 *      
 * OUTPUT:
 *      rif_config_p            -- pointer to rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetRifFromIp(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETRIFFROMIP;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}



/* FUNCTION NAME : NETCFG_POM_IP_GetRifFromExactIp
 * PURPOSE:
 *      Find the interface which this ip on.
 *
 * INPUT:
 *      rif_config_p->ip_addr   -- the target IP address for checking.
 *
 * OUTPUT:
 *      rif_config_p            -- pointer to rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. For both IPv4/IPv6.
 */
UI32_T NETCFG_POM_IP_GetRifFromExactIp(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETRIFFROMEXACTIP;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_POM_IP_GetIpAddressMode
 * PURPOSE:
 *      Get the addr_mode of specified L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *      
 * OUTPUT:
 *      mode_p      -- pointer to the mode.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetIpAddressMode(UI32_T ifindex, UI32_T *mode_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_u32a2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETIPADDRESSMODE;

    msg_p->data.u32a1_u32a2.u32_a1 = ifindex;

    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *mode_p = msg_p->data.u32a1_u32a2.u32_a2;
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_POM_IP_GetNextIpAddressMode
 * PURPOSE:
 *      Get the addr_mode for the L3 interface next to the specified one.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *      
 * OUTPUT:
 *      ifindex     -- the ifindex of next L3 interface.
 *      mode_p      -- pointer to the mode.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetNextIpAddressMode(UI32_T *ifindex_p, UI32_T *mode_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_u32a2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTIPADDRESSMODE;

    msg_p->data.u32a1_u32a2.u32_a1 = *ifindex_p;
    msg_p->data.u32a1_u32a2.u32_a2 = *mode_p;

    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *ifindex_p = msg_p->data.u32a1_u32a2.u32_a1;
    *mode_p = msg_p->data.u32a1_u32a2.u32_a2;
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_POM_IP_GetIpInterface
 * PURPOSE:
 *      Get the L3 interface.
 *
 * INPUT:
 *      intf_p->ifindex     -- the ifindex of L3 interface.
 *      
 * OUTPUT:
 *      intf_p
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetL3Interface(NETCFG_TYPE_L3_Interface_T *intf_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(l3_interface_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETL3INTERFACE;

    memcpy(&(msg_p->data.l3_interface_v), intf_p, sizeof(msg_p->data.l3_interface_v));

    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(intf_p, &(msg_p->data.l3_interface_v), sizeof(msg_p->data.l3_interface_v));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_POM_IP_GetNextIpInterface
 * PURPOSE:
 *      Get Next L3 interface.
 *
 * INPUT:
 *      intf_p->ifindex     -- the ifindex of L3 interface.
 *      
 * OUTPUT:
 *      intf_p
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetNextL3Interface(NETCFG_TYPE_L3_Interface_T *intf_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(l3_interface_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTL3INTERFACE;

    memcpy(&(msg_p->data.l3_interface_v), intf_p, sizeof(msg_p->data.l3_interface_v));

    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(intf_p, &(msg_p->data.l3_interface_v), sizeof(msg_p->data.l3_interface_v));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME: NETCFG_POM_IP_GetIpAddrEntry
 * PURPOSE:
 *      Get IpAddrEntry.
 * INPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * OUTPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetIpAddrEntry(NETCFG_TYPE_ipAddrEntry_T  *ip_addr_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(ip_addr_entry_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETIPADDRENTRY;
    
    memcpy(&(msg_p->data.ip_addr_entry_v), ip_addr_entry_p, sizeof(msg_p->data.ip_addr_entry_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(ip_addr_entry_p, &(msg_p->data.ip_addr_entry_v), sizeof(msg_p->data.ip_addr_entry_v));
    return msg_p->type.result_bool;
}


/* FUNCTION NAME: NETCFG_POM_IP_GetNextIpAddrEntry
 * PURPOSE:
 *      Get next IpAddrEntry.
 * INPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * OUTPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NO_MORE_ENTRY
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetNextIpAddrEntry(NETCFG_TYPE_ipAddrEntry_T  *ip_addr_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(ip_addr_entry_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTIPADDRENTRY;
    
    memcpy(&(msg_p->data.ip_addr_entry_v), ip_addr_entry_p, sizeof(msg_p->data.ip_addr_entry_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(ip_addr_entry_p, &(msg_p->data.ip_addr_entry_v), sizeof(msg_p->data.ip_addr_entry_v));
    return msg_p->type.result_bool;
}

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : NETCFG_POM_IP_GetIpNetToMediaProxyStatus
 * PURPOSE:
 *      Get proxy ARP status.
 *
 * INPUT:
 *      ifindex -- the interface.
 *
 * OUTPUT:
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * RETURN:
 *     NETCFG_TYPE_OK/
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_POM_IP_GetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T *status)
{
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETARPPROXYSTATUS;
    msg_p->data.u32a1_bla2.u32_a1= ifindex;

    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *status = msg_p->data.u32a1_bla2.bl_a2;
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_POM_IP_GetNextIpNetToMediaProxyStatus
 * PURPOSE:
 *      Get next interface's proxy ARP status.
 *
 * INPUT:
 *      ifindex -- the interface.
 *
 * OUTPUT:
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * RETURN:
 *     NETCFG_TYPE_OK/
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_POM_IP_GetNextIpNetToMediaProxyStatus(UI32_T *ifindex, BOOL_T *status)
{
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTARPPROXYSTATUS;
    msg_p->data.u32a1_bla2.u32_a1= *ifindex;

    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *status = msg_p->data.u32a1_bla2.bl_a2;
    *ifindex = msg_p->data.u32a1_bla2.u32_a1;
    return msg_p->type.result_ui32;

}
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

#if (SYS_CPNT_IPV6 == TRUE)

/* FUNCTION NAME : NETCFG_POM_IP_GetIPv6EnableStatus
 * PURPOSE:
 *      Get "ipv6 enable" configuration.
 *
 * INPUT:
 *      ifindex     -- the interface.
 *
 * OUTPUT:
 *      status_p    -- status
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. There is no ipv6 enable configuration in linux kernel, thus we only get it in OM.
 */
UI32_T NETCFG_POM_IP_GetIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETIPV6ENABLESTATUS;
    msg_p->data.u32a1_bla2.u32_a1= ifindex;
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *status_p = msg_p->data.u32a1_bla2.bl_a2;
    return msg_p->type.result_ui32;

}
/* FUNCTION NAME : NETCFG_POM_IP_GetIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      Get IPv6 address autoconfig status of the L3 interface.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      status_p    -- pointer to status
 *                     TRUE: autoconfig enabled.
 *                     FALSE: autoconfig disabled.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETIPV6ADDRAUTOCONFIGENABLESTATUS;
    
    msg_p->data.u32a1_bla2.u32_a1= ifindex;
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *status_p = msg_p->data.u32a1_bla2.bl_a2;
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_POM_IP_GetNextIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      Get next L3 interface's IPv6 address autoconfig status.
 *
 * INPUT:
 *      ifindex_p   -- interface index
 *
 * OUTPUT:
 *      ifindex_p   -- interface index
 *      status_p    -- pointer to status
 *                     TRUE: autoconfig enabled.
 *                     FALSE: autoconfig disabled.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetNextIPv6AddrAutoconfigEnableStatus(UI32_T *ifindex_p, BOOL_T *status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTIPV6ADDRAUTOCONFIGENABLESTATUS;
    
    msg_p->data.u32a1_bla2.u32_a1= *ifindex_p;
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *ifindex_p = msg_p->data.u32a1_bla2.u32_a1;
    *status_p = msg_p->data.u32a1_bla2.bl_a2;
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_POM_IP_GetIPv6RifConfig
 * PURPOSE:
 *      Get the rif by the ip address.
 *
 * INPUT:
 *      rif_config_p        -- pointer to the rif_config
 *      
 * OUTPUT:
 *      rif_config_p        -- pointer to the rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 */
UI32_T NETCFG_POM_IP_GetIPv6RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETIPV6RIFCONFIG;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_POM_IP_GetNextIPv6RifConfig
 * PURPOSE:
 *      Get the next rif by the ip address.
 *
 * INPUT:
 *      rif_config_p        -- pointer to the rif_config
 *      
 * OUTPUT:
 *      rif_config_p        -- pointer to the rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 *      If you need ifindex, Inet prefix as the KEY such as 
 *      SNMP GetNext, CLI running-config, this function is not
 *      the right choice.
 */
UI32_T NETCFG_POM_IP_GetNextIPv6RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTIPV6RIFCONFIG;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: NETCFG_POM_IP_GetIPv6RifFromInterface
 * PURPOSE:
 *      Get the IPv6 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->addr.addr           -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv6 only.
 */
UI32_T NETCFG_POM_IP_GetIPv6RifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETIPV6RIFFROMINTERFACE;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: NETCFG_POM_IP_GetNextIPv6RifFromInterface
 * PURPOSE:
 *      Get the IPv6 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->ip_addr           -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv6 only.
 */
UI32_T NETCFG_POM_IP_GetNextIPv6RifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTIPV6RIFFROMINTERFACE;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_POM_IP_GetLinkLocalRifFromInterface
 * PURPOSE:
 *      Get RIF with link local address for IPv4 or IPv6 in 
 *      the specified L3 interface.
 *
 * INPUT:
 *      rif_p       -- pointer to rif.
 *
 * OUTPUT:
 *      rif_p       -- pointer to rif.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_IMPLEMENT
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      1. rif_p->addr.type should be L_INET_ADDR_TYPE_IPV4Z or L_INET_ADDR_TYPE_IPV6Z
 */
UI32_T NETCFG_POM_IP_GetLinkLocalRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETLINKLOCALRIFFROMINTERFACE;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_POM_IP_GetIPv6InterfaceMTU
 * PURPOSE:
 *      Get IPv6 interface MTU from OM.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *
 * OUTPUT:
 *      mtu_p   -- MTU
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_u32a2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETIPV6INTERFACEMTU;
    
    msg_p->data.u32a1_u32a2.u32_a1 = ifindex;
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    
    *mtu_p = msg_p->data.u32a1_u32a2.u32_a2;
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_POM_IP_GetRunningIPv6EnableStatus
 * PURPOSE:
 *      Get running config of ipv6 enable status
 *
 * INPUT:
 *      ifindex  -- the interface.
 *
 * OUTPUT:
 *      status_p -- status
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      None
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_IP_GetRunningIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p)
{
    if(ifindex > SYS_ADPT_MAX_VLAN_ID || ifindex <= 0)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (NETCFG_POM_IP_GetIPv6EnableStatus(ifindex, status_p) == NETCFG_TYPE_OK)
    {
        if (SYS_DFLT_IPV6_ENABLE != *status_p)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

/* FUNCTION NAME : NETCFG_POM_IP_GetRunningIPv6InterfaceMTU
 * PURPOSE:
 *      Get running config of ipv6 interface MTU
 *
 * INPUT:
 *      ifindex  -- the interface ifindex
 *
 * OUTPUT:
 *      mtu_p    -- interface MTU
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      None
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_IP_GetRunningIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p)
{
    if (NETCFG_POM_IP_GetIPv6InterfaceMTU(ifindex, mtu_p) == NETCFG_TYPE_OK)
    {
        if (SYS_DFLT_IPV6_INTERFACE_MTU != *mtu_p)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

/* FUNCTION NAME : NETCFG_POM_IP_GetRunningIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      Get running config of ipv6 interface autoconfig enable status
 *
 * INPUT:
 *      ifindex  -- the interface ifindex
 *
 * OUTPUT:
 *      status_p -- interface autoconfig enable status
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      None
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_IP_GetRunningIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p)
{
    if (NETCFG_POM_IP_GetIPv6AddrAutoconfigEnableStatus(ifindex, status_p) == NETCFG_TYPE_OK)
    {
        if (SYS_DFLT_IPV6_ADDR_AUTOCONFIG != *status_p)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}


#endif /* SYS_CPNT_IPV6 */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
/* FUNCTION NAME : NETCFG_POM_IP_GetCraftInterfaceInetAddress
 * PURPOSE:
 *      To get ipv4/v6 address on craft interface
 * INPUT:
 *      craft_addr_p        -- pointer to address
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_POM_IP_GetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(craft_addr);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETCRAFTINTERFACEINETADDRESS;
    
    memcpy(&(msg_p->data.craft_addr), craft_addr_p, sizeof(msg_p->data.craft_addr));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(craft_addr_p, &(msg_p->data.craft_addr), sizeof(*craft_addr_p));
    return msg_p->type.result_ui32;
}

UI32_T NETCFG_POM_IP_GetIPv6EnableStatus_Craft(UI32_T ifindex, BOOL_T *status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETIPV6ENABLESTATUS_CRAFT;
    
    msg_p->data.u32a1_bla2.u32_a1 = ifindex;
    msg_p->data.u32a1_bla2.bl_a2 = *status_p;
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *status_p = msg_p->data.u32a1_bla2.bl_a2;

    return msg_p->type.result_ui32;
}
#endif /* SYS_CPNT_CRAFT_PORT */

#if (SYS_CPNT_DHCP_INFORM == TRUE)
/* FUNCTION NAME : NETCFG_POM_IP_GetDhcpInform
 * PURPOSE:
 *      Get the DHCP inform of L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *
 * OUTPUT:
 *      do_enable_p -- status of dhcp inform
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. This is only for IPv4.
 */
UI32_T NETCFG_POM_IP_GetDhcpInform(UI32_T ifindex, BOOL_T *do_enable_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GET_DHCP_INFORM;
    
    msg_p->data.u32a1_bla2.u32_a1 = ifindex;
    msg_p->data.u32a1_bla2.bl_a2 = *do_enable_p;
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *do_enable_p = msg_p->data.u32a1_bla2.bl_a2;

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_POM_IP_GetRunningDhcpInform
 * PURPOSE:
 *      Get running config of DHCP inform of L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *
 * OUTPUT:
 *      do_enable_p -- status of dhcp inform
 *
 * RETURN:
 *   SYS_TYPE_GET_RUNNING_CFG_SUCCESS  
 *   SYS_TYPE_GET_RUNNING_CFG_FAIL     
 *   SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  
 * NOTES:
 *      1. This is only for IPv4.
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_IP_GetRunningDhcpInform(UI32_T ifindex, BOOL_T *do_enable_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(u32a1_bla2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GET_RUNNING_DHCP_INFORM;
    
    msg_p->data.u32a1_bla2.u32_a1 = ifindex;
    msg_p->data.u32a1_bla2.bl_a2 = *do_enable_p;
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *do_enable_p = msg_p->data.u32a1_bla2.bl_a2;

    return msg_p->type.result_running_cfg;
}
#endif /* SYS_CPNT_DHCP_INFORM */
#if (SYS_CPNT_VIRTUAL_IP == TRUE)
UI32_T NETCFG_POM_IP_GetNextVirtualRifByIfindex(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_OM_IP_GET_MSG_SIZE(inet_rif_config_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_IP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_IPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_IP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_IP_IPCCMD_GETNEXTVIRTUALRIFBYIFINDEX;
    
    memcpy(&(msg_p->data.inet_rif_config_v), rif_config_p, sizeof(msg_p->data.inet_rif_config_v));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_ip_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(rif_config_p, &(msg_p->data.inet_rif_config_v), sizeof(msg_p->data.inet_rif_config_v));
    return msg_p->type.result_ui32;
}
#endif
