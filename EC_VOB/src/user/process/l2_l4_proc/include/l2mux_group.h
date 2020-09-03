/* MODULE NAME:  l2mux_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of l2mux group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/19/2007 - KH shi, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef L2MUX_GROUP_H
#define L2MUX_GROUP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_mm.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in L2MUX_Group.
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
 *    All threads in the same L2MUX group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void L2MUX_GROUP_Create_All_Threads(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_GROUP_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init  L2MUX Group.
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
 *------------------------------------------------------------------------------
 */
void L2MUX_GROUP_InitiateProcessResource(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2MUX_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for L2MUX Group.
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
void L2MUX_GROUP_Create_InterCSC_Relation(void);

void L2MUX_GROUP_L2muxReceivePacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p, 
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN], 
                                       UI16_T    tag_info,
                                       UI16_T    type,     
                                       UI32_T    pkt_length, 
                                       UI32_T    unit_no,  
                                       UI32_T    port_no,
                                       UI32_T    packet_class);

void L2MUX_GROUP_L2muxReceiveIPPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no,
                                       UI32_T    packet_class);

void L2MUX_GROUP_ImlReceivePacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p, 
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN], 
                                       UI16_T    tag_info,
                                       UI16_T    ether_type,     
                                       UI32_T    packet_length, 
                                       UI32_T    l_port);

#endif

