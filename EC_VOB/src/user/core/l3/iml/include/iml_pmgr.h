#ifndef IML_PMGR_H
#define IML_PMGR_H

#include "sys_type.h"
#include "l_mm.h"
#include "vlan_om.h"

/* EXPORTED FUNCTION DECLARACTIONS
 */
/* ----------------------------------------------------------------------------------
 * FUNCTION : IML_PMGR_InitiateProcessResource
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Initial resource for process who want to call IML_PMGR
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE    -- success
 *            FALSE   -- fail
 * NOTE     : This function must be called before calling any other IML_PMGR functions
 * ----------------------------------------------------------------------------------*/
BOOL_T IML_PMGR_InitiateProcessResource(void);

UI32_T IML_PMGR_SendPkt(L_MM_Mref_Handle_T *mref_handle_p, UI32_T ifindex, UI32_T packet_length, 
                        UI8_T *dst_mac, UI8_T *src_mac, UI16_T packet_type, BOOL_T forward);

#if (SYS_CPNT_DHCPV6SNP == TRUE)
/* FUNCTION NAME: IML_PMGR_SendPktByVlanEntry
 * PURPOSE: Send  packet by DHCPV6SNP according to vlan entry
 * INPUT:  mref_handle_p -- L_MREF descriptor
 *             packet_length  -- packet length
 *             vid                  -- vid
 *             dst_mac_p      -- destination mac address
 *             src_mac_p      -- source mac address
 *             vlan_entry_p      -- the main purpose is to get the egress port list.
 * OUTPUT: none
 * RETURN: TRUE/FALSE
 * NOTES:  called by DHCPV6SNP  which is in dhcpsnp_engine.c
 */
BOOL_T IML_PMGR_SendPktByVlanEntry(L_MM_Mref_Handle_T *mref_handle_p, UI32_T packet_length, 
      UI32_T vid,UI8_T *dst_mac_p, UI8_T *src_mac_p, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry_p);
#endif
void IML_PMGR_GetManagementVid(UI32_T *vid_ifindex_p);

UI32_T IML_PMGR_SetManagementVid(UI32_T vid_ifIndex);

#if (SYS_CPNT_DAI == TRUE)
/* FUNCTION NAME: IML_PMGR_ARP_Inspection_SendPkt
 * PURPOSE: 
 *      Send packet.
 * INPUT:  
 *         *mem_ref -- L_MREF descriptor
 *         vid -- the vlan id
 *         packet_length -- packet length
 *         *dst_mac --
 *         *src_mac -- 
 *         packet_type --
 * OUTPUT: 
 *      None.
 * RETURN: 
 *      successful (0), failed (-1)
 * NOTES:
 *      None.   
 */
UI32_T IML_PMGR_ARP_Inspection_SendPkt(L_MM_Mref_Handle_T *mref_handle_p, UI32_T vid, UI32_T packet_length, 
                                        UI8_T *dst_mac, UI8_T *src_mac,UI16_T packet_type,UI32_T src_lport_ifindex);
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_PMGR_GetOrgDipAndIngLportBySrcIpAndSrcTcpPort
 *--------------------------------------------------------------------------
 * PURPOSE  : To get original dip and ingress lport
 *              by specified src ip/tcp port.
 * INPUT    : src_ip, src_tcpport
 * OUTPUT   : org_dip_p, lport_p
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. all args are in network order
 *            2. for webauth
 *--------------------------------------------------------------------------*/
BOOL_T IML_PMGR_GetOrgDipAndIngLportBySrcIpAndSrcTcpPort(
    UI32_T  src_ip,
    UI16_T  src_tcpport,
    UI32_T  *org_dip_p,
    UI32_T  *lport_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_PMGR_SetWebauthStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : To set webauth status by specified logical port.
 * INPUT    : lport (1-based), status
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. for webauth
 *--------------------------------------------------------------------------*/
BOOL_T IML_PMGR_SetWebauthStatus(
    UI32_T  lport,
    BOOL_T  status);

#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */


#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/* FUNCTION NAME: IML_PMGR_BroadcastBootPPkt_ExceptInport
 * PURPOSE: Send broadcast packet
 * INPUT:  *mem_ref_handle_p -- L_MREF descriptor
 *         vid_ifIndex       -- vlan ifIndex.
 *         packet_length     -- packet length
 *         *dst_mac_p        -- destination mac
 *         *src_mac_p        -- source mac
 *         src_lport_ifindex -- broadcast packet except this port
 * OUTPUT: none
 * RETURN: successful (0), failed (-1)
 * NOTES:  
 */
UI32_T IML_PMGR_BroadcastBootPPkt_ExceptInport(L_MM_Mref_Handle_T *mref_handle_p, UI32_T vid_ifindex,
    UI32_T packet_length, UI8_T *dst_mac_p, UI8_T *src_mac_p,UI32_T src_lport_ifindex);

/* FUNCTION NAME: IML_PMGR_SendPktToPort
 * PURPOSE: Send packet
 * INPUT:  *mref_handle_p   -- L_MREF descriptor
 *         packet_length    -- packet length
 *         *dst_mac_p       -- destination mac
 *         *src_mac_p       -- source mac
 *         vid              -- vid
 *         out_lport        -- output port
 *         packet_type      --
 * OUTPUT: None
 * RETURN: successful (0), failed (-1)
 * NOTES:  None
 */
UI32_T IML_PMGR_SendPktToPort(L_MM_Mref_Handle_T *mref_handle_p, UI32_T packet_length, UI8_T *dst_mac_p, UI8_T *src_mac_p, 
    UI32_T vid, UI32_T out_lport, UI16_T packet_type);

#endif

#endif

