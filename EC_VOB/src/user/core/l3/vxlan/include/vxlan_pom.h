/* MODULE NAME:  vxlan_pom.h
 * PURPOSE:
 *
 * NOTES:
 *     None.
 *
 * HISTORY:
 *    mm/dd/yy
 *    04/21/15 -- Kelly, Create
 *
 * Copyright(C) Accton Corporation, 2015
 */
#ifndef _VXLAN_POM_H
#define _VXLAN_POM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l_inet.h"
#include "vxlan_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: VXLAN_POM_InitiateProcessResources
 * PURPOSE : Initiate system resource for VXLAN OM.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
BOOL_T VXLAN_POM_InitiateProcessResources(void);

/* FUNCTION NAME: VXLAN_POM_GetUdpDstPort
 * PURPOSE : Get UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : port_no -- UDP port number
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetUdpDstPort(UI16_T *port_no);

/* FUNCTION NAME: VXLAN_POM_GetRunningUdpDstPort
 * PURPOSE : Get UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : port_no -- UDP port number
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None.
 */
SYS_TYPE_Get_Running_Cfg_T VXLAN_POM_GetRunningUdpDstPort(UI16_T *port_no);

/* FUNCTION NAME: VXLAN_POM_GetFloodRVtepByVniIp
 * PURPOSE : Get remote VTEP with specified VNI+IP.
 * INPUT   : rvtep_entry_p->vni
 *           rvtep_entry_p->ip
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetFloodRVtepByVniIp(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_POM_GetNextFloodRVtepByVni
 * PURPOSE : Get next remote VTEP with VNI.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetNextFloodRVtepByVni(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_POM_GetNextFloodRVtep
 * PURPOSE : Get next remote VTEP with VNI+IP.
 * INPUT   : rvtep_entry_p->vni
 *           rvtep_entry_p->ip
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetNextFloodRVtep(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_POM_GetFloodMulticastByVni
 * PURPOSE : Get remote VTEP with multicast group.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetFloodMulticastByVni(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_POM_GetNextFloodMulticast
 * PURPOSE : Get Next remote VTEP.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetNextFloodMulticast(VXLAN_OM_RVtep_T *rvtep_entry_p);

/* FUNCTION NAME: VXLAN_POM_GetNextPortVlanVniMapByVni
 * PURPOSE : Get next port (and VLAN) to VNI mapping relationship.
 * INPUT   : vni -- VXLAN ID
 *           lport_p -- port ifindex
 *           vid_p -- VLAN ID
 * OUTPUT  : lport_p -- port ifindex
 *           vid_p -- VLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetNextPortVlanVniMapByVni(UI32_T vni, UI32_T *lport_p, UI16_T *vid_p);

/* FUNCTION NAME - VXLAN_POM_GetVlanVniMapEntry
 * PURPOSE : Get a VLAN-VNI entry.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_POM_GetVlanVniMapEntry(UI16_T vid, UI32_T *vni);

/* FUNCTION NAME - VXLAN_POM_GetNextVlanVniMapEntry
 * PURPOSE : Get next VLAN-VNI entry.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : vid -- VLAN ID
 *           vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : vid=0 to get the first entry
 */
UI32_T VXLAN_POM_GetNextVlanVniMapEntry(UI16_T *vid, UI32_T *vni);

/* FUNCTION NAME: VXLAN_POM_GetVniByVfi
 * PURPOSE : Get VNI value from VFI
 * INPUT   : vfi         -- virtual forwarding instance
 * OUTPUT  : None.
 * RETURN  : vni, returns -1 when error
 * NOTES   : None.
 */
I32_T VXLAN_POM_GetVniByVfi(UI32_T vfi);

/* FUNCTION NAME - VXLAN_POM_GetNextVniEntry
 * PURPOSE : Get a next VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : vni=0 to get the first entry
 */
UI32_T VXLAN_POM_GetNextVniEntry(VXLAN_OM_VNI_T *vni_entry_p);

/* FUNCTION NAME: VXLAN_POM_GetSrcIf
 * PURPOSE : Get source interface ifindex of VTEP.
 * INPUT   : None
 * OUTPUT  : ifindex_p -- source interface ifindex
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetSrcIf(UI32_T *ifindex_p);

/* FUNCTION NAME: VXLAN_POM_GetRunningSrcIf
 * PURPOSE : Get source interface ifindex of VTEP..
 * INPUT   : None
 * OUTPUT  : ifindex_p -- interface ifindex
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None.
 */
SYS_TYPE_Get_Running_Cfg_T VXLAN_POM_GetRunningSrcIf(UI32_T *ifindex_p);

/* FUNCTION NAME: VXLAN_POM_GetVlanAndVniByVfi
 * PURPOSE : Get VLAN and VNI value from VFI
 * INPUT   : vfi         -- virtual forwarding instance
 * OUTPUT  : vid         -- vlan ID
 *           vni         -- vxlan ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetVlanAndVniByVfi(UI32_T vfi, UI16_T *vid, UI32_T *vni);

/* FUNCTION NAME: VXLAN_POM_GetLportOfAccessPort
 * PURPOSE : Get logical port number of access port on specified VLAN.
 * INPUT   : vxlan_port  -- vxlan port nunmber of access port
 *           vid         -- vlan ID
 * OUTPUT  : lport       -- logical port number
 *           vni         -- vxlan ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetLportOfAccessPort(UI32_T vxlan_port, UI16_T vid, UI32_T *lport);

/* FUNCTION NAME: VXLAN_POM_GetNextAccessVxlanPort
 * PURPOSE : Get next access VXLAN port.
 * INPUT   : vid_p         -- VLAN ID.
 *           lport_p       -- port number.
 *           vxlan_port_p  -- output vxlan port.
 * OUTPUT  : TRUE/FALSE
 * RETURN  : None.
 * NOTES   : None.
 */
BOOL_T VXLAN_POM_GetNextAccessVxlanPort(UI16_T *vid_p, UI32_T *lport_p, UI32_T *vxlan_port_p);

/* FUNCTION NAME - VXLAN_POM_GetPortVlanVniMapEntry
 * PURPOSE : Get Port+VLAN to VNI mapping.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 * OUTPUT  : vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_POM_GetPortVlanVniMapEntry(UI32_T lport, UI16_T vid, UI32_T *vni);

/* FUNCTION NAME: VXLAN_POM_GetAccessVxlanPort
 * PURPOSE : Get access VXLAN port.
 * INPUT   : vid         -- VLAN ID.
 *           lport       -- port number.
 * OUTPUT  : vxlan_port  -- output vxlan port.
 * RETURN  : None.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetAccessVxlanPort(UI16_T vid, UI32_T vni);

/* FUNCTION NAME - VXLAN_POM_GetVniEntry
 * PURPOSE : Get a VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_POM_GetVniEntry(VXLAN_OM_VNI_T *vni_entry_p);

/* FUNCTION NAME: VXLAN_POM_GetVlanNlportOfAccessPort
 * PURPOSE : Get VID and port from access VXLAN port.
 * INPUT   : vxlan_port    -- vxlan port.
 * OUTPUT  : vid_p         -- VLAN ID.
 *           lport_p       -- port number.
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 */
BOOL_T VXLAN_POM_GetVlanNlportOfAccessPort(UI32_T vxlan_port, UI16_T *vid_p, UI32_T *lport_p);
#endif /* _VXLAN_POM_H */
