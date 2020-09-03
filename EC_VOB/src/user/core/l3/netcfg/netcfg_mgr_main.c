/* Module Name: netcfg_mgr_main.c
 * Purpose:
 *      NETCFG_MGR_IP provides NETCFG misc. configuration management access-point for
 *      upper layer.
 *
 * Notes:
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *       01/29/2008 --  Vai Wang,   Created
 *
 * Copyright(C)      Accton Corporation, 2008.
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "netcfg_type.h"
#include "netcfg_mgr_main.h"
#include "netcfg_mgr_route.h"
#include "netcfg_mgr_ip.h"
#include "netcfg_mgr_nd.h"  
#if (SYS_CPNT_RIP == TRUE)
#include "netcfg_mgr_rip.h" /*Lin.Li, for RIP porting*/
#endif
#if (SYS_CPNT_OSPF == TRUE)
#include "netcfg_mgr_ospf.h" /*Lin.Li, for OSPF porting*/
#endif
#if (SYS_CPNT_PBR == TRUE)
#include "netcfg_mgr_pbr.h"
#endif

#include "netcfg_om_main.h"
#include "ip_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static void NETCFG_MGR_MAIN_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);
static void NETCFG_MGR_MAIN_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);
static BOOL_T NETCFG_MGR_MAIN_IsEmbeddedUdpPort(UI32_T udp_port);
static BOOL_T NETCFG_MGR_MAIN_IsEmbeddedTcpPort(UI32_T tcp_port);


/* STATIC VARIABLE DECLARATIONS 
 */
SYSFUN_DECLARE_CSC


/* LOCAL SUBPROGRAM
 */
/* FUNCTION NAME - NETCFG_MGR_MAIN_HandleHotInsertion
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 *
 */
static void NETCFG_MGR_MAIN_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    return;
}


/* FUNCTION NAME - NETCFG_MGR_MAIN_HandleHotRemoval
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 *
 */
static void NETCFG_MGR_MAIN_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    return;
}


/* FUNCTION NAME : NETCFG_MGR_MAIN_IsEmbeddedUdpPort
 * PURPOSE:
 *      Check the udp-port is used in protocol engine or not.
 *
 * INPUT:
 *      udp_port -- the udp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      None.
 */
static BOOL_T NETCFG_MGR_MAIN_IsEmbeddedUdpPort(UI32_T udp_port)
{
    return IP_MGR_IsEmbeddedUdpPort(udp_port);    
}

/* FUNCTION NAME : NETCFG_MGR_MAIN_IsEmbeddedTcpPort
 * PURPOSE:
 *      Check the tcp-port is used in protocol engine or not.
 *
 * INPUT:
 *      tcp_port -- the tcp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      None.
 */
static BOOL_T NETCFG_MGR_MAIN_IsEmbeddedTcpPort(UI32_T tcp_port)
{
    return IP_MGR_IsEmbeddedTcpPort(tcp_port);
}


/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : NETCFG_MGR_MAIN_SetTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_MAIN_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();

    NETCFG_MGR_ROUTE_SetTransitionMode();
    NETCFG_MGR_IP_SetTransitionMode();
    NETCFG_MGR_ND_SetTransitionMode(); /*Lin.Li, for ARP porting*/
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_MGR_RIP_SetTransitionMode(); /*Lin.Li, for RIP porting*/
#endif
#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_MGR_OSPF_SetTransitionMode(); /*Lin.Li, for OSPF porting*/
#endif

#if (SYS_CPNT_PBR == TRUE)
    NETCFG_MGR_PBR_SetTransitionMode();
#endif

}


/* FUNCTION NAME : NETCFG_MGR_MAIN_EnterTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_MAIN_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE();

    NETCFG_MGR_ROUTE_EnterTransitionMode();
    NETCFG_MGR_IP_EnterTransitionMode();
    NETCFG_MGR_ND_EnterTransitionMode();/*Lin.Li, for ARP porting*/
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_MGR_RIP_EnterTransitionMode();/*Lin.Li, for RIP porting*/
#endif
#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_MGR_OSPF_EnterTransitionMode();/*Lin.Li, for OSPF porting*/
#endif

#if (SYS_CPNT_PBR == TRUE)
    NETCFG_MGR_PBR_EnterTransitionMode();
#endif

}

/*Donny.li modify for ARP stacking.2008.08.07 */
/* FUNCTION NAME : NETCFG_MGR_MAIN_EnterMasterMode
 * PURPOSE:
 *      Let default gateway CFGDB into route when provision complete.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */

void NETCFG_MGR_MAIN_ProvisionComplete(void)
{   
    NETCFG_MGR_ND_ProvisionComplete();
    NETCFG_MGR_IP_ProvisionComplete();
}
/*Donny.li end modify for ARP stacking.2008.08.07 */


/* FUNCTION NAME : NETCFG_MGR_MAIN_EnterMasterMode
 * PURPOSE:
 *      Enter master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void NETCFG_MGR_MAIN_EnterMasterMode (void)
{
#if 0 /* kernel has known CRAFT port's MAC address already */
//#if (SYS_CPNT_CRAFT_PORT == TRUE)
    char cmd_str[56];
    UI8_T cpu_mac[SYS_ADPT_MAC_ADDR_LEN];

    if(STKTPLG_POM_GetLocalUnitBaseMac(cpu_mac)==TRUE)
    {
        snprintf(cmd_str, sizeof(cmd_str), "/sbin/ifconfig CRAFT down hw ether %02X:%02X:%02X:%02X:%02X:%02X",
            cpu_mac[0], cpu_mac[1], cpu_mac[2], cpu_mac[3], cpu_mac[4], cpu_mac[5]);
        system(cmd_str);
        snprintf(cmd_str, sizeof(cmd_str), "/sbin/ifconfig CRAFT up");
        system(cmd_str);
    }
    else
    {
        printf("%s: Failed to get cpu mac.\r\n", __FUNCTION__);
    }
#endif

    SYSFUN_ENTER_MASTER_MODE();

    NETCFG_MGR_ROUTE_EnterMasterMode();
    NETCFG_MGR_IP_EnterMasterMode();
    NETCFG_MGR_ND_EnterMasterMode();/*Lin.Li, for ARP porting*/
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_MGR_RIP_EnterMasterMode();/*Lin.Li, for RIP porting*/
#endif
#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_MGR_OSPF_EnterMasterMode();/*Lin.Li, for OSPF porting*/
#endif

#if (SYS_CPNT_PBR == TRUE)
    NETCFG_MGR_PBR_EnterMasterMode();
#endif
}


/* FUNCTION NAME : NETCFG_MGR_MAIN_EnterSlaveMode
 * PURPOSE:
 *      Enter slave mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void NETCFG_MGR_MAIN_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();

    NETCFG_MGR_IP_EnterSlaveMode();
    NETCFG_MGR_ROUTE_EnterSlaveMode();
    NETCFG_MGR_ND_EnterSlaveMode();/*Lin.Li, for ARP porting*/
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_MGR_RIP_EnterSlaveMode();/*Lin.Li, for RIP porting*/
#endif
#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_MGR_OSPF_EnterSlaveMode();/*Lin.Li, for OSPF porting*/
#endif
#if (SYS_CPNT_PBR == TRUE)
    NETCFG_MGR_PBR_EnterSlaveMode();
#endif

}


/* FUNCTION NAME : NETCFG_MGR_MAIN_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 */
void NETCFG_MGR_MAIN_Create_InterCSC_Relation(void)
{
    NETCFG_MGR_ROUTE_Create_InterCSC_Relation();
    NETCFG_MGR_IP_Create_InterCSC_Relation();
    NETCFG_MGR_ND_Create_InterCSC_Relation();/*Lin.Li, for ARP porting*/
}


 /* FUNCTION NAME : NETCFG_MGR_MAIN_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_MAIN used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success
 *      FALSE - Fail
 */
BOOL_T NETCFG_MGR_MAIN_InitiateProcessResources(void)
{
    NETCFG_OM_MAIN_InitateProcessResources();

    NETCFG_MGR_ROUTE_InitiateProcessResources();
    NETCFG_MGR_IP_InitiateProcessResources();
    NETCFG_MGR_ND_InitiateProcessResources();

#if (SYS_CPNT_RIP == TRUE)
    NETCFG_MGR_RIP_InitiateProcessResources();/*Lin.Li, for RIP porting*/
#endif
#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_MGR_OSPF_InitiateProcessResources();/*Lin.Li, for OSPF porting*/
#endif

#if (SYS_CPNT_PBR == TRUE)
    NETCFG_MGR_PBR_InitiateSystemResources();
#endif
    return TRUE;
}


/* FUNCTION NAME : NETCFG_MGR_MAIN_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG mgr main.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *
 */
BOOL_T NETCFG_MGR_MAIN_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    NETCFG_MGR_MAIN_IPCMsg_T *netcfg_mgr_msg_p;
    BOOL_T need_respond=TRUE;

    if(ipcmsg_p==NULL)
        return FALSE;

    netcfg_mgr_msg_p = (NETCFG_MGR_MAIN_IPCMsg_T*)ipcmsg_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        switch(netcfg_mgr_msg_p->type.cmd)
        {
            /* If no need to return, do not send respond */
            case NETCFG_MGR_MAIN_IPCCMD_HANDLEHOTINSERTION:
            case NETCFG_MGR_MAIN_IPCCMD_HANDLEHOTREMOVAL:

                need_respond = FALSE;
                break;
            default:
                netcfg_mgr_msg_p->type.result_ui32 = NETCFG_TYPE_FAIL;
                ipcmsg_p->msg_size = NETCFG_MGR_MAIN_MSGBUF_TYPE_SIZE;
                need_respond = TRUE;
                break;
        }

        return need_respond;
    }

    switch(netcfg_mgr_msg_p->type.cmd)
    {
        /* System Wise Configuration */
        case NETCFG_MGR_MAIN_IPCCMD_HANDLEHOTINSERTION:
            NETCFG_MGR_MAIN_HandleHotInsertion(
                netcfg_mgr_msg_p->data.hot_inerstion_handle.starting_port_ifindex,
                netcfg_mgr_msg_p->data.hot_inerstion_handle.number_of_port,
                netcfg_mgr_msg_p->data.hot_inerstion_handle.use_default);
            need_respond = FALSE;
            break;
        case NETCFG_MGR_MAIN_IPCCMD_HANDLEHOTREMOVAL:
            NETCFG_MGR_MAIN_HandleHotRemoval(
                netcfg_mgr_msg_p->data.u32a1_u32a2.u32_a1,
                netcfg_mgr_msg_p->data.u32a1_u32a2.u32_a2);
            need_respond = FALSE;
            break;
        case NETCFG_MGR_MAIN_IPCCMD_ISEMBEDDEDUDPPORT:
            netcfg_mgr_msg_p->type.result_bool = NETCFG_MGR_MAIN_IsEmbeddedUdpPort(
                netcfg_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size=NETCFG_MGR_MAIN_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_MAIN_IPCCMD_ISEMBEDDEDTCPPORT:
            netcfg_mgr_msg_p->type.result_bool = NETCFG_MGR_MAIN_IsEmbeddedTcpPort(
                netcfg_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size=NETCFG_MGR_MAIN_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            netcfg_mgr_msg_p->type.result_ui32 = NETCFG_TYPE_FAIL;
            ipcmsg_p->msg_size = NETCFG_MGR_MAIN_MSGBUF_TYPE_SIZE;
    }

    return need_respond;

}

