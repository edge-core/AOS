/* MODULE NAME:  vrrp_group.h
 * PURPOSE:
 *     This file provides APIs for implementations of VRRP csc group.
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */
#include "l_mm_type.h"
#ifndef VRRP_GROUP_H
#define VRRP_GROUP_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for VRRP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void VRRP_GROUP_InitiateProcessResources(void);

/* FUNCTION NAME:  VRRP_GROUP_Create_All_Threads
 * PURPOSE:
 *    This function will spawn all threads in CSCGroup.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    All threads in the same CSC group will join the same thread group.
 *
 */
void VRRP_GROUP_Create_All_Threads(void);
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for VRRP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void VRRP_GROUP_Create_InterCSC_Relation(void);


/* FUNCTION NAME: VRRP_GROUP_VrrpReceivePacketCallbackHandler
 *----------------------------------------------------------------------------------

* PURPOSE: As long as  received a VRRP packet,it calls
*          this function to request VRRP callback function to handle this
*          packet.
*----------------------------------------------------------------------------------
* INPUT:
*      L_MM_Mref_Handle_T *mref_handle_p   -- packet buffer and return buffer function pointer.
*      UI16_T   tag_info   -- tag information
*      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
*      UI32_T   pkt_length -- the length of the packet payload.
*      UI32_T  ifindex    -- the ifindex of vlan from where receive this packet.
*      UI8_T    dst_mac   -- the destination MAC address of this packet.
*      UI8_T    src_mac   -- the source MAC address of this packet.
*      UI32_T   ingress_vid    -- the vid of ingress
*      UI32_T   src_port  -- the source port of the packet
* OUTPUT:  None
* RETURN:  None
*----------------------------------------------------------------------------------
* NOTES:   This function is used to dispatch packet to upper layer.
*/

void VRRP_GROUP_VrrpReceivePacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI32_T    packet_length,
                                       UI32_T    ifindex,
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI16_T    ingress_vid,
                                       UI32_T    src_port);


#endif  /* End of VRRP_GROUP_H */


