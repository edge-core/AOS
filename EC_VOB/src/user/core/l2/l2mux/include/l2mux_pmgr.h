#ifndef L2MUX_PMGR_H
#define L2MUX_PMGR_H

#include "sys_type.h"

/* EXPORTED FUNCTION DECLARACTIONS
 */
BOOL_T L2MUX_PMGR_InitiateProcessResource(void);

void L2MUX_PMGR_SendBPDU(L_MM_Mref_Handle_T *mref_handle_p,
                          UI8_T       dst_mac[6],
                          UI8_T       src_mac[6],
                          UI16_T      type,
                          UI16_T      tag_info,
                          UI32_T      packet_length,
                          UI32_T      lport,           
                          BOOL_T      is_tagged, 
                          UI32_T      cos_value,
                          BOOL_T      is_send_to_trunk_members);


void L2MUX_PMGR_SendPacket(L_MM_Mref_Handle_T *mref_handle_p,
                          UI8_T       dst_mac[6],
                          UI8_T       src_mac[6],
                          UI16_T      type,
                          UI16_T      tag_info,
                          UI32_T      packet_length,
                          UI32_T      lport,           
                          BOOL_T      is_tagged, 
                          UI32_T      cos_value,
                          BOOL_T      is_send_to_trunk_members);


void L2MUX_PMGR_SendMultiPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                UI8_T             dst_mac[6], 
                                UI8_T             src_mac[6],
                                UI16_T            type,        
                                UI16_T            tag_info,    
                                UI32_T            packet_length,
                                UI8_T             *lport_list, 
                                UI8_T             *untagged_list,
                                UI32_T            cos_value);

void L2MUX_PMGR_SendMultiPacketByVlan(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T dst_mac[6],
    UI8_T src_mac[6],
    UI16_T type,
    UI16_T tag_info,
    UI32_T packet_length,
    UI8_T *lport_list,
    UI32_T cos_value);

void L2MUX_PMGR_SendPacketPipeline(L_MM_Mref_Handle_T *mref_handle_p,
                          UI32_T      packet_length,
                          UI32_T      in_port);

#endif

