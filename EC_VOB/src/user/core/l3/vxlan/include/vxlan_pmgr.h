/* MODULE NAME:  vxlan_pmgr.h
 * PURPOSE:
 *     VXLAN PMGR APIs.
 *
 * NOTES:
 *
 * HISTORY:
 *    mm/dd/yy
 *    04/21/15 -- Kelly, Create
 *
 * Copyright(C) Accton Corporation, 2015
 */
#ifndef VXLAN_PMGR_H
#define VXLAN_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "vxlan_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: VXLAN_PMGR_InitiateProcessResources
 * PURPOSE: Initiate process resources for VXLAN_PMGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 * NOTES   : None.
 */
BOOL_T VXLAN_PMGR_InitiateProcessResources(void);

/* FUNCTION NAME: VXLAN_PMGR_SetUdpDstPort
 * PURPOSE : Set UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_SetUdpDstPort(UI16_T port_no);

/* FUNCTION NAME: VXLAN_PMGR_AddFloodRVtep
 * PURPOSE : Flooding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : vni -- VXLAN ID
 *           ip_p -- IP address of remote VTEP
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_AddFloodRVtep(UI32_T vni, L_INET_AddrIp_T *ip_p);

/* FUNCTION NAME: VXLAN_PMGR_DelFloodRVtep
 * PURPOSE : Delete remote VTEP with specified IP.
 * INPUT   : vni -- VXLAN ID
 *           ip_p -- IP address of remote VTEP
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_DelFloodRVtep(UI32_T vni, L_INET_AddrIp_T *ip_p);

/* FUNCTION NAME: VXLAN_PMGR_AddFloodMulticast
 * PURPOSE : Flooding to which multicast group, when received packet lookup bridge table fail.
 * INPUT   : vni -- VXLAN ID
 *           m_ip_p -- IP address of multicast group
 *           vid -- VLAN ID
 *           lport -- logical port
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_AddFloodMulticast(UI32_T vni, L_INET_AddrIp_T *m_ip_p, UI16_T vid, UI32_T lport);

/* FUNCTION NAME: VXLAN_PMGR_DelFloodMulticast
 * PURPOSE : Delete flooding multicast group.
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_DelFloodMulticast(UI32_T vni);

/* FUNCTION NAME: VXLAN_PMGR_AddVpn
 * PURPOSE : Create Vxlan VPN
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_ENTRY_EXISTED /
 *           VXLAN_TYPE_RETURN_ERROR
 * NOTES   : None
 */
UI32_T VXLAN_PMGR_AddVpn(UI32_T vni);

/* FUNCTION NAME: VXLAN_PMGR_DeleteVpn
 * PURPOSE : Destroy a Vxlan VPN
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_ENTRY_NOT_EXISTED /
 *           VXLAN_TYPE_ENTRY_EXISTED /
 *           VXLAN_TYPE_RETURN_ERROR
 * NOTES   : None
 */
UI32_T VXLAN_PMGR_DeleteVpn(UI32_T vni);


/* FUNCTION NAME: VXLAN_PMGR_SetVlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : vni -- VXLAN ID
 *           vid -- VLAN ID
 *           is_add -- TRUE means add, FALSE means delete.
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_SetVlanVniMap(UI16_T vid, UI32_T vni, BOOL_T is_add);

/* FUNCTION NAME: VXLAN_PMGR_SetPortVlanVniMap
 * PURPOSE : Configure port (and VLAN) to VNI mapping relationship.
 * INPUT   : lport -- port ifindex
 *           vni -- VXLAN ID
 *           vid -- VLAN ID
 *           is_add -- TRUE means add, FALSE means delete.
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_SetPortVlanVniMap(UI32_T lport, UI16_T vid, UI32_T vni, BOOL_T is_add);

/* FUNCTION NAME: VXLAN_PMGR_SetSrcIf
 * PURPOSE : Set source interface ifindex of VTEP.
 * INPUT   : ifindex -- interface index
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_PMGR_SetSrcIf(UI32_T ifindex);
#endif    /* End of VXLAN_PMGR_H */

