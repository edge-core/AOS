/* MODULE NAME:  udphelper_mgr.c
 * PURPOSE:
 *     This module provides APIs for UDPHELPER CSC to use.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    03/31/2009 - Lin.Li, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */
#include "sys_cpnt.h"
#if (SYS_CPNT_UDP_HELPER == TRUE)
/* INCLUDE FILE DECLARATIONS
 */
#include <netinet/in.h> 
#include <stdio.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include <sysfun.h>
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_sort_lst.h"
#include "l_inet.h"
#include "string.h"
#include "l_mm_type.h"
#include "dhcp_type.h"
#include "udphelper_type.h"
#include "udphelper_mgr.h"
#include "udphelper_vm.h"
#include "udphelper_om.h"
#include "udphelper_backdoor.h"
#include "l4_pmgr.h"
#include "swctrl_pmgr.h"
#include "ip_lib.h"
#if (UDPHELPER_SUPPORT_ACCTON_BACKDOOR == TRUE)
#include "backdoor_mgr.h"
#endif

/* NAME	CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS 
 */

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T is_provision_complete = FALSE;
SYSFUN_DECLARE_CSC;

/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * PURPOSE: Initialize process resources for UDPHELEPER_MGR
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_Initiate_System_Resources(void)
{
    /*INIT DATABASE*/
    UDP_HELPER_OM_Init();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set udphelper_mgr into master mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_EnterMasterMode(void)
{
    /*INIT DATABASE*/
    SYSFUN_ENTER_MASTER_MODE();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_ProvisionComplete
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set udphelper_mgr provision complete.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
} 


/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_EnterSlaveMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set udphelper_mgr into slave mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_EnterTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set udphelper_mgr into transition mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    /*clear database*/    
    is_provision_complete = FALSE;
    return;
} 

/*------------------------------------------------------------------------------
 * Function : UDPHELPER_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void UDPHELPER_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    return;
} 

/*--------------------------------------------------------------------------
 * FUNCTION NAME - UDPHELPER_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void UDPHELPER_MGR_Create_InterCSC_Relation(void)
{
    /* register callbacks */
#if (UDPHELPER_SUPPORT_ACCTON_BACKDOOR == TRUE)
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("UDPHELPER",
                                                       SYS_BLD_UDPHELPER_GROUP_IPCMSGQ_KEY, 
                                                       UDPHELPER_Backdoor_CallBack);
#endif

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_HandleHotInsertion
 * PURPOSE: Hot swap init function for insertion
 * INPUT:   starting_port_ifindex
 *          number_of_port
 *          use_default
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-------------------------------------------------------------------------*/
void UDPHELPER_MGR_HandleHotInsertion(UI32_T starting_port_ifindex,
                                    UI32_T number_of_port,
                                    BOOL_T use_default)
{
    return;
} /* end of UDPHELPER_MGR_HandleHotInsertion() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_HandleHotRemoval
 * PURPOSE: Hot swap init function for removal
 * INPUT:   starting_port_ifindex
 *          number_of_port
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 *------------------------------------------------------------------------*/
void UDPHELPER_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{ 
       return;
} /* end of UDPHELPER_MGR_HandleHotRemoval() */

/* FUNCTION NAME : UDPHELPER_MGR_SetStatus
 * PURPOSE: Enable or disable UDP helper mechanism.
 *
 *
 * INPUT:
 *      status: the status value, TRUE or FALSE
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *
 * NOTES:
 *      None.
 */      
static UI32_T UDPHELPER_MGR_SetStatus(UI32_T status)
{
    UI32_T old_status=0;
    /* get udp helper address */
    UDPHELPER_OM_GetStatus(&old_status);
    
    if(status == old_status)
        return UDPHELPER_TYPE_RESULT_SUCCESS;
    
    
    if(status)
    {
        /* set IP broadcast rule to chip */   
        
        if(TRUE != L4_PMGR_TrapPacket2Cpu(TRUE, RULE_TYPE_PacketType_IP_BCAST))
        {  
            return UDPHELPER_TYPE_RESULT_FAIL;
        }

        /* check if forwarding list has udp port 67,68 for dhcp */
        if(UDPHELPER_TYPE_RESULT_SUCCESS == UDPHELPER_OM_GetForwardPort(UDPHELPER_TYPE_BOOTP_CLIENT_PORT))
        {
            /* copy server side dhcp packet to cpu */
            
            if(TRUE != SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_DHCP_TRAP_BY_UDP_HELPER_68))
            {  
                return UDPHELPER_TYPE_RESULT_FAIL;
            }
        }

        if(UDPHELPER_TYPE_RESULT_SUCCESS == UDPHELPER_OM_GetForwardPort(UDPHELPER_TYPE_BOOTP_SERVER_PORT))
        {
            /* copy client side dhcp packet to cpu */
            
            if(TRUE != SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_DHCP_TRAP_BY_UDP_HELPER_67))
            {  
                return UDPHELPER_TYPE_RESULT_FAIL;
            }
        }
    }
    else
    {
        /* delete IP broadcast rule to chip */
        
        if(TRUE != L4_PMGR_TrapPacket2Cpu(FALSE, RULE_TYPE_PacketType_IP_BCAST))
        {  
            return UDPHELPER_TYPE_RESULT_FAIL;
        }

        /* check if forwarding list has udp port 67,68 for dhcp */
        if(UDPHELPER_TYPE_RESULT_SUCCESS == UDPHELPER_OM_GetForwardPort(UDPHELPER_TYPE_BOOTP_CLIENT_PORT))
        {
            /* delete copy server side dhcp packet to cpu */
            
            if(TRUE != SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_DHCP_TRAP_BY_UDP_HELPER_68))
            {  
                return UDPHELPER_TYPE_RESULT_FAIL;
            }
        }

        if(UDPHELPER_TYPE_RESULT_SUCCESS == UDPHELPER_OM_GetForwardPort(UDPHELPER_TYPE_BOOTP_SERVER_PORT))
        {
            /* delete copy client side dhcp packet to cpu */
            
            if(TRUE != SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_DHCP_TRAP_BY_UDP_HELPER_67))
            {  
                return UDPHELPER_TYPE_RESULT_FAIL;
            }
        }
    }
    
    

    
    
    return UDPHELPER_OM_SetStatus(status);
}
/* FUNCTION NAME : UDPHELPER_MGR_GetStatus
 * PURPOSE:Get UDP helper status
 *
 *
 * INPUT:
 *      status: the status value, TRUE or FALSE
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *
 * NOTES:
 *      None.
 */      
static UI32_T UDPHELPER_MGR_GetStatus(UI32_T *status)
{
    return UDPHELPER_OM_GetStatus(status);
}
/* FUNCTION NAME : UDPHELPER_MGR_AddForwardPort
 * PURPOSE:Add forward port to OM.
 *
 *
 * INPUT:
 *      port: the port number.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_FORWARD_PORT_FULL.
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
static UI32_T UDPHELPER_MGR_AddForwardPort(UI32_T port)
{
    UI32_T status = 0;
    if ( port < UDPHELPER_TYPE_MIN_FORWARD_PORT 
        || port > UDPHELPER_TYPE_MAX_FORWARD_PORT )
        return UDPHELPER_TYPE_RESULT_WRONG_VALUE;
    if ( UDPHELPER_TYPE_RESULT_SUCCESS == UDPHELPER_OM_GetForwardPort(port) )
        return UDPHELPER_TYPE_RESULT_SUCCESS;

    /* check if ip helper enabled, and udp port is 67,68 */
    UDPHELPER_OM_GetStatus(&status);
    
    if(status)
    {
        if(UDPHELPER_TYPE_BOOTP_CLIENT_PORT == port)
        {
            /* copy server side dhcp packet to cpu */
            
            if(TRUE != SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_DHCP_TRAP_BY_UDP_HELPER_68))
            {  
                return UDPHELPER_TYPE_RESULT_FAIL;
            }            
        }

        if(UDPHELPER_TYPE_BOOTP_SERVER_PORT == port)
        {
            /* copy client side dhcp packet to cpu */
            
            if(TRUE != SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_DHCP_TRAP_BY_UDP_HELPER_67))
            {  
                return UDPHELPER_TYPE_RESULT_FAIL;
            }            
        }
    }
    return UDPHELPER_OM_AddForwardPort(port);
}
/* FUNCTION NAME : UDPHELPER_MGR_DelForwardPort
 * PURPOSE:Delete forward port to OM.
 *
 *
 * INPUT:
 *      port: the port number.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *
 * NOTES:
 *      None.
 */      
static UI32_T UDPHELPER_MGR_DelForwardPort(UI32_T port)
{
    UI32_T status=0;
    /* check if ip helper enabled, and udp port is 67,68 */
    UDPHELPER_OM_GetStatus(&status);
    
    if ( UDPHELPER_TYPE_RESULT_SUCCESS == UDPHELPER_OM_GetForwardPort(port) )
    {
        
        if(status)
        {
            if(UDPHELPER_TYPE_BOOTP_CLIENT_PORT == port)
            {
                /* delete copy server side dhcp packet to cpu */
                
                if(TRUE != SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_DHCP_TRAP_BY_UDP_HELPER_68))
                {  
                    return UDPHELPER_TYPE_RESULT_FAIL;
                }    
            }

            if(UDPHELPER_TYPE_BOOTP_SERVER_PORT == port)
            {
                /* delete copy client side dhcp packet to cpu */
                
                if(TRUE != SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_DHCP_TRAP_BY_UDP_HELPER_67))
                {  
                    return UDPHELPER_TYPE_RESULT_FAIL;
                }                 
            }
        }
        
        return UDPHELPER_OM_DelForwardPort(port);
    }

    return UDPHELPER_TYPE_RESULT_SUCCESS;
    
}
/* FUNCTION NAME : UDPHELPER_MGR_GetNextForwardPort
 * PURPOSE:Get next forward port from OM.
 *
 *
 * INPUT:
 *      port: the port number.
 *
 * OUTPUT:
 *      port: the port number.
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *      UDPHELPER_TYPE_RESULT_FAIL
 * NOTES:
 *      .
 */      
static UI32_T UDPHELPER_MGR_GetnextForwardPort(UI32_T *port)
{
    return UDPHELPER_OM_GetNextForwardPort(port);
}
/* FUNCTION NAME : UDPHELPER_MGR_AddIpHelperAddress
 * PURPOSE:Add ip helper address to OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      helper_addr: the ip helper address
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_IF_NOT_EXIST.
 *      UDPHELPER_TYPE_RESULT_HELPER_FULL
 *      UDPHELPER_TYPE_RESULT_CREATE_IF_HELPER_LIST_FAIL
 *      UDPHELPER_TYPE_RESULT_MALLOC_IF_FAIL
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
static UI32_T UDPHELPER_MGR_AddHelperAddr(UI32_T ifindex, L_INET_AddrIp_T addr)
{
	
	if ( UDPHELPER_TYPE_RESULT_SUCCESS == UDPHELPER_OM_GetHelper(ifindex, &addr) )
		return UDPHELPER_TYPE_RESULT_SUCCESS;

    /* check help address validation */
    if(IP_LIB_IsZeroNetwork(addr.addr)||
       IP_LIB_IsLoopBackIp(addr.addr)||
       IP_LIB_IsTestingIp(addr.addr)||
       IP_LIB_IsMulticastIp(addr.addr)||
       IP_LIB_IsBroadcastIp(addr.addr))
    {
        return UDPHELPER_TYPE_RESULT_INVALID_ARG;
    }        
    
    return UDPHELPER_OM_AddIpHelperAddress(ifindex, addr);
}
/* FUNCTION NAME : UDPHELPER_MGR_DelIpHelperAddress
 * PURPOSE:Delete ip helper address to OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      helper_addr: the ip helper address
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_IF_NOT_EXIST.
 *      UDPHELPER_TYPE_RESULT_CREATE_IF_HELPER_LIST_FAIL
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
static UI32_T UDPHELPER_MGR_DelHelperAddr(UI32_T ifindex, L_INET_AddrIp_T addr)
{
    return UDPHELPER_OM_DelIpHelperAddress(ifindex, addr);
}
/* FUNCTION NAME : UDPHELPER_OM_GetNextHelper
 * PURPOSE:Get next helper from OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      helper_addr: the ip helper address
 *
 * OUTPUT:
 *      helper_addr: the helper address
 *     
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_FAIL.
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
static UI32_T UDPHELPER_MGR_GetnextHelper(UI32_T ifindex, L_INET_AddrIp_T *helper)
{
    return UDPHELPER_OM_GetNextHelper(ifindex, helper);
}
/* FUNCTION NAME : UDPHELPER_MGR_L3IfCreate
 * PURPOSE:Add l3 interface to OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *      UDPHELPER_TYPE_RESULT_CREATE_IF_LIST_FAIL
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_MGR_L3IfCreate(UI32_T ifindex)
{
    return UDPHELPER_OM_L3IfCreate(ifindex);
}
/* FUNCTION NAME : UDPHELPER_MGR_L3IfDelete
 * PURPOSE:Delete l3 interface from OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_MGR_L3IfDelete(UI32_T ifindex)
{
    return UDPHELPER_OM_L3IfDelete(ifindex);
}
/* FUNCTION NAME : UDPHELPER_MGR_RifCreate
 * PURPOSE:Add ip address to OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      addr: the ip address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_IF_NOT_EXIST.
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_MGR_RifCreate(UI32_T ifindex, L_INET_AddrIp_T addr)
{
    return UDPHELPER_OM_RifCreate(ifindex, addr);
}
 
/* FUNCTION NAME : UDPHELPER_MGR_RifDelete
 * PURPOSE:Delete ip address from OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      addr: the ip address
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_IF_NOT_EXIST.
 *      UDPHELPER_TYPE_RESULT_ADDR_NOT_EXIST
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_MGR_RifDelete(UI32_T ifindex, L_INET_AddrIp_T addr)
{
    return UDPHELPER_OM_RifDelete(ifindex, addr);
}
/* FUNCTION NAME : UDPHELPER_MGR_GetHelper
 * PURPOSE:Get helper from OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      helper_addr: the ip helper address
 *
 * OUTPUT:
 *      helper_addr: the helper address
 *     
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_FAIL.
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
static UI32_T UDPHELPER_MGR_GetHelper(UI32_T ifindex, L_INET_AddrIp_T *helper)
{
    return UDPHELPER_OM_GetHelper(ifindex, helper);
}
/* FUNCTION NAME : UDPHELPER_MGR_GetForwardPort
 * PURPOSE:Get forward port from OM.
 *
 *
 * INPUT:
 *      port: the port number.
 *
 * OUTPUT:
 *      port: the port number.
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *      UDPHELPER_TYPE_RESULT_FAIL
 * NOTES:
 *      .
 */      
static UI32_T UDPHELPER_MGR_GetForwardPort(UI32_T port)
{
    return UDPHELPER_OM_GetForwardPort(port);
}

/* FUNCTION NAME : UDPHELPER_MGR_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for UDPHELPER mgr.
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
BOOL_T UDPHELPER_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    UDPHELPER_MGR_IPCMsg_T *udphelper_mgr_msg_p;
    BOOL_T need_respond = TRUE;

    if(ipcmsg_p == NULL)
        return FALSE;

    udphelper_mgr_msg_p = (UDPHELPER_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_TYPE_RESULT_FAIL;
        ipcmsg_p->msg_size = UDPHELPER_MGR_MSGBUF_TYPE_SIZE;
        return TRUE;
    }

    switch(udphelper_mgr_msg_p->type.cmd)
    {
        case UDPHELPER_MGR_IPCCMD_SET_STATUS:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_SetStatus(udphelper_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);
            break;
        case UDPHELPER_MGR_IPCCMD_UDPPORTSET:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_AddForwardPort(udphelper_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);    
            break;
        case UDPHELPER_MGR_IPCCMD_UDPPORTUNSET:    
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_DelForwardPort(udphelper_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);
            break;
        case UDPHELPER_MGR_IPCCMD_HELPERSET:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_AddHelperAddr(udphelper_mgr_msg_p->data.arg5.ifindex,
                                                                                 udphelper_mgr_msg_p->data.arg5.addr);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);    
            break;
        case UDPHELPER_MGR_IPCCMD_HELPERUNSET:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_DelHelperAddr(udphelper_mgr_msg_p->data.arg5.ifindex,
                                                                                 udphelper_mgr_msg_p->data.arg5.addr);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);   
            break;
        case UDPHELPER_MGR_IPCCMD_L3IF_CREATE:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_L3IfCreate(udphelper_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);    
            break;
        case UDPHELPER_MGR_IPCCMD_L3IF_DELETE:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_L3IfDelete(udphelper_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);    
            break;
        case UDPHELPER_MGR_IPCCMD_RIF_CREATE:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_RifCreate(udphelper_mgr_msg_p->data.arg5.ifindex,
                                                                            udphelper_mgr_msg_p->data.arg5.addr);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);    
            break;
        case UDPHELPER_MGR_IPCCMD_RIF_DELETE:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_RifDelete(udphelper_mgr_msg_p->data.arg5.ifindex,
                                                                            udphelper_mgr_msg_p->data.arg5.addr);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);    
            break;
        case UDPHELPER_MGR_IPCCMD_GET_STATUS:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_GetStatus(&udphelper_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U) + sizeof(UI32_T);    
            break;
        case UDPHELPER_MGR_IPCCMD_GETNEXT_FORWARD_PORT:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_GetnextForwardPort(&udphelper_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U) + sizeof(UI32_T);    
            break;
        case UDPHELPER_MGR_IPCCMD_GETNEXT_HELPER:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_GetnextHelper(udphelper_mgr_msg_p->data.arg5.ifindex,
                                                                                &udphelper_mgr_msg_p->data.arg5.addr);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U) + sizeof(udphelper_mgr_msg_p->data.arg5);    
            break;  
        case UDPHELPER_MGR_IPCCMD_GET_HELPER:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_GetHelper(udphelper_mgr_msg_p->data.arg5.ifindex,
                                                                            &udphelper_mgr_msg_p->data.arg5.addr);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);    
            break;                
        case UDPHELPER_MGR_IPCCMD_GET_FORWARD_PORT:
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_MGR_GetForwardPort(udphelper_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = sizeof(union UDPHELPER_MGR_IPCMsg_Type_U);    
            break;                              
        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            udphelper_mgr_msg_p->type.result_ui32 = UDPHELPER_TYPE_RESULT_FAIL;
            ipcmsg_p->msg_size = UDPHELPER_MGR_MSGBUF_TYPE_SIZE;
    }

    return need_respond;

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - UDPHELPER_MGR_do_packet
 *------------------------------------------------------------------------
 * FUNCTION: This is the callback function for udphelper receive packet.
 *
 * INPUT   : 
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 *------------------------------------------------------------------------*/  
void UDPHELPER_MGR_do_packet(L_MM_Mref_Handle_T *mref_handle_p,
                                                       UI32_T packet_length,
                                                       UI32_T ifindex,
                                                       UI8_T *dst_mac,
                                                       UI8_T *src_mac,
                                                       UI32_T vid,
                                                       UI32_T src_port)
{
    UI32_T pdu_len;
    UI32_T from_port;
    DHCP_TYPE_IpHeader_T    *ip_pkt;
    DHCP_TYPE_UdpHeader_T   *udp_header;
    struct dhcp_packet      *packet;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (L_MM_Mref_Release(&mref_handle_p)==L_MM_ERROR_RETURN_VAL)
        {
            /*  Log to system : Invalid incoming packet */
        }
        printf("Not in master mode.\r\n");
        return;
    } 
    if (mref_handle_p == NULL)
    {
        /*  log msg to system : MSG, Receive a NULL packet  */
        printf("mref_handle_p is null.\r\n");
        return;
    }        
    ip_pkt = (DHCP_TYPE_IpHeader_T*) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    udp_header = (DHCP_TYPE_UdpHeader_T*) ((UI8_T*)ip_pkt + (ip_pkt->ip_ver_hlen & 0x0F)*4);
    from_port = ntohl(udp_header->src_port);
    /* BOOTP packet */
    if ( UDPHELPER_TYPE_BOOTP_CLIENT_PORT == from_port
         || UDPHELPER_TYPE_BOOTP_SERVER_PORT == from_port )
    {
        packet = (struct dhcp_packet*) ((UI8_T*)udp_header + sizeof(DHCP_TYPE_UdpHeader_T));        
        UDPHELPER_VM_BOOTPRelay(ifindex, ip_pkt, udp_header, packet);
    }
    else
        UDPHELPER_VM_UDPRelay(ifindex, ip_pkt, udp_header);
    
    if(L_MM_Mref_Release(&mref_handle_p)==L_MM_ERROR_RETURN_VAL)
    {
        /* log error message to system : FREE_MEMORY_ERROR    */
        SYSFUN_LogDebugMsg(" DHCPSNP_TASK_DhcpsnpRx_Callback : L_MM_Mref_Release fail..\n");
    }    
    return;
} 
#endif

