#ifndef HRDRV_TYPE_H
#define HRDRV_TYPE_H

#include "sys_type.h"
#include "leaf_es3626a.h"

typedef struct 
{
    /*
    field_bmp is the bitwise or of HRDRV_PktField_T
    */
    UI32_T field_bmp;
    BOOL_T isEnablePktformat;
    UI32_T inout;
    UI32_T type;
    UI8_T  smac_bitmask[6];
    UI8_T  dmac_bitmask[6];
    UI32_T sip_bitmask;
    UI32_T dip_bitmask;
    UI16_T sport_bitmask;
    UI16_T dport_bitmask;
    UI16_T vid_bitmask;
    UI16_T ethertype_bitmask;
    UI8_T  tcp_control_bitmask;
    
}HRDRV_TYPE_AclMaskEntry_T;   



typedef enum HRDRV_Operator_S
{
    HRDRV_OPERATOR_NO_OPERATOR = VAL_aclMacAceVidOp_noOperator,
    HRDRV_OPERATOR_EQUAL =VAL_aclMacAceVidOp_equal,   /* = , use will the bitmask */
    HRDRV_OPERATOR_RANGE =VAL_aclMacAceVidOp_range       /* a1 < x < a2 */
    
}HRDRV_Operator_T ;
 

#pragma pack(1)
typedef struct 
{
    UI32_T                  rid;
    /* 
    fun_type must specify 
    */
    UI32_T                 fun_type;    
    
    /*
    inout is one of HRDRV_Inout_T
    */
    UI8_T                   inout;
    /*
    specified a unit port each time
    */
    SYS_TYPE_Uport_T        uport;
    
    /* ratelimit class id 
     */
    BOOL_T                  is_meter_inuse;
     
    UI32_T                  meter_id;
   
    /* 
    action_bmp is the bitwise-or of  HRDRV_Action_T
    */
    UI32_T                  in_action_bmp;   
    
    UI32_T                  out_action_bmp;
    
    /* 
    action parameter 
    */
    UI8_T                   cos_pri;
    UI8_T                   in_new_dscp;
    
    UI8_T                   in_new_tos;
    UI8_T                   in_new_dot1p;
    
    UI8_T                   out_new_dscp;
    UI8_T                   out_new_tos;
    
    /*
    the pktformat is one of HRDRV_L2PktType_T
    */
    UI8_T                   pkt_format;    
    
    /*
    field_bmp is used to specify the field below
    */
    UI32_T                  field_bmp;   
    
    UI16_T                  tag_type;
    
    /*
    if pri is set, tag_type is set to be 0x8100 defaultly
    */
    UI8_T                   tag_pri;
    /*
    if vid is set, tag_type is set to be 0x8100 defaultly
    */
    
    /*
    all the following field is in Host Order
    */
    HRDRV_Operator_T        tag_vid_op;
    UI16_T                  tag_min_vid;
    UI16_T                  tag_max_vid;
    UI16_T                  tag_vid_bitmask;
    
    UI8_T                   smac[6];
    UI8_T                   smac_bitmask[6];
    UI8_T                   dmac[6];
    UI8_T                   dmac_bitmask[6];
    
    HRDRV_Operator_T        ethertype_op;
    UI16_T                  min_ethertype;
    UI16_T                  max_ethertype;
    UI16_T                  ethertype_bitmask;
    
    UI8_T                   ip_ver_len;
    UI8_T                   ip_precedence;
    UI8_T                   ip_tos;
    UI8_T                   ip_dscp;
    UI8_T                   ip_protocol;
    UI32_T                  sip;
    UI32_T                  sip_bitmask;
    UI32_T                  dip;
    UI32_T                  dip_bitmask;
    
    
    HRDRV_Operator_T        sport_op;
    UI16_T                  min_sport;
    UI16_T                  max_sport;
    UI16_T                  sport_bitmask;
    
    HRDRV_Operator_T        dport_op;
    UI16_T                  min_dport;
    UI16_T                  max_dport;
    UI16_T                  dport_bitmask;
    UI8_T                   tcp_control_flags;
    UI8_T                   tcp_control_bitmask;
   
   
    
}HRDRV_TYPE_ResouceEntity_T; 

#pragma pack()





























































































































































































































#endif
