/* Module Name:UDPHELPER_OM.H
 * Purpose: To store UDPHELPER DATABASE.
 *
 * Notes:
 *      
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *    2009/04/02    --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2009.
 */
#ifndef _UDPHELPER_OM_H_
#define _UDPHELPER_OM_H_
#include "l_hash.h"
#include "l_linklist.h"
#include "l_inet.h"

/* structure for the request/response ipc message in csc pmgr and mgr
 */

/* interface and helper address structure */ 
typedef struct UDPHELPER_OM_Interface_S
{
    /* vlan id */
    UI32_T ifindex;    
    /* The primary address of this interface */
    L_INET_AddrIp_T rif;
    struct L_list *helper_list;
}UDPHELPER_OM_Interface_T;

/* forward port structure */
typedef struct UDPHELPER_OM_ForwardPort_S
{
    /* The current port number, maximum number 
           is: SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT */
    UI32_T num_port;
    /* Port array */
    UI32_T forward_port_array[SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT];
}UDPHELPER_OM_ForwardPort_T;

/* global structure for UDP helper mechanism */
typedef struct UDPHELPER_OM_S
{
    UI32_T udphelper_semaphore; 
    /* The global switch of UDP helper mechanism */
    UI32_T udphelper_status;
    /* the current number of helper on this interface, maximum number 
           is: SYS_ADPT_UDPHELPER_MAX_HELPER */
    UI32_T num_helper;
    /* The interface list */
    struct L_list *if_list;
    /* The forward port */
    UDPHELPER_OM_ForwardPort_T forward_port;
}UDPHELPER_OM_T;
/* FUNCTION NAME : UDP_HELPER_OM_Init
 * PURPOSE:Init UDPHELPER_OM_OSPF database, create semaphore
 *
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
 *      None.
 */      
void UDP_HELPER_OM_Init(void);
/* FUNCTION NAME : UDPHELPER_OM_IfAdd
 * PURPOSE:Add l3 interface to OM.
 *
 *
 * INPUT:
 *      vid: the vlan id number.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_L3IfCreate(UI32_T ifindex);
/* FUNCTION NAME : UDPHELPER_OM_IfDel
 * PURPOSE:Deletel3 interface from OM.
 *
 *
 * INPUT:
 *      vid: the vlan id number.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_L3IfDelete(UI32_T ifindex);
/* FUNCTION NAME : UDPHELPER_OM_RifAdd
 * PURPOSE:Add ip address to OM.
 *
 *
 * INPUT:
 *      vid: the vlan id number.
 *      addr: the ip address
 *      mask: the mask
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_RifCreate(UI32_T ifindex, L_INET_AddrIp_T addr);
/* FUNCTION NAME : UDPHELPER_OM_RifDel
 * PURPOSE:Delete ip address from OM.
 *
 *
 * INPUT:
 *      vid: the vlan id number.
 *      addr: the ip address
 *      mask: the mask
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_RifDelete(UI32_T ifindex, L_INET_AddrIp_T addr);
/* FUNCTION NAME : UDPHELPER_OM_SetStatus
 * PURPOSE:Set UDP helper status
 *
 *
 * INPUT:
 *      status: the status value, TRUE or FALSE
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_SetStatus(UI32_T status);
/* FUNCTION NAME : UDPHELPER_OM_GetStatus
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
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_GetStatus(UI32_T *status);
/* FUNCTION NAME : UDPHELPER_OM_AddForwardPort
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
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_AddForwardPort(UI32_T port);
/* FUNCTION NAME : UDPHELPER_OM_DelForwardPort
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
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_DelForwardPort(UI32_T port);
/* FUNCTION NAME : UDPHELPER_OM_GetNextForwardPort
 * PURPOSE:Get next forward port from OM.
 *
 *
 * INPUT:
 *      first_entry_b: get the first entry or not
 *      port: the port number.
 *
 * OUTPUT:
 *      first_entry_b: get the first entry or not
 *      port: the port number.
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_GetNextForwardPort(UI32_T *port);
/* FUNCTION NAME : UDPHELPER_OM_AddIpHelperAddress
 * PURPOSE:Add ip helper address to OM.
 *
 *
 * INPUT:
 *      vid: the vlan id number.
 *      addr: the ip address
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_AddIpHelperAddress(UI32_T ifindex, L_INET_AddrIp_T helper_addr);
/* FUNCTION NAME : UDPHELPER_OM_DelIpHelperAddress
 * PURPOSE:Delete ip helper address to OM.
 *
 *
 * INPUT:
 *      vid: the vlan id number.
 *      addr: the ip address
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_DelIpHelperAddress(UI32_T ifindex, L_INET_AddrIp_T helper_addr);
/* FUNCTION NAME : UDPHELPER_OM_GetNextHelper
 * PURPOSE:Get next helper from OM.
 *
 *
 * INPUT:
 *      vid: vlan id number
 *      helper: the helper address.
 *
 * OUTPUT:
 *      helper: the helper address
 *     
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_GetNextHelper(UI32_T ifindex, L_INET_AddrIp_T *helper_addr);
/* FUNCTION NAME : UDPHELPER_OM_CheckHelper
 * PURPOSE:Check if we should do some operations for helper.
 *
 *
 * INPUT:
 *      ifindex:the layer3 interface ifindex.
 *      dst_ip: IP destination address of this packet.
 *      port: the destination UDP port number
 * OUTPUT:
 *      helper: the helper address
 *     
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_CheckHelper(UI32_T ifindex, UI32_T dst_ip, UI32_T port, UI32_T *if_addr);

/* FUNCTION NAME : UDPHELPER_OM_GetForwardPort
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
UI32_T UDPHELPER_OM_GetForwardPort(UI32_T port);

/* FUNCTION NAME : UDPHELPER_OM_GetHelper
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
UI32_T UDPHELPER_OM_GetHelper(UI32_T ifindex, L_INET_AddrIp_T *helper_addr);


void UDPHELPER_OM_SetDebugStatus(UI32_T status);
#endif
