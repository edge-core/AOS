/*-----------------------------------------------------------------------------
 * Module Name: cfm_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the CFM object manager
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/5/2006 - macauley_cheng, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */

#ifndef _CFM_OM_H
#define _CFM_OM_H

#include "cfm_type.h"
#include "cfm_timer.h"
#include "sysrsc_mgr.h"
#if (SYS_CPNT_CFM == TRUE)

#define CFM_OM_MD_MA_MEP_KEY           0x01
#define CFM_OM_LPORT_MD_MA_KEY         0x00
#define CFM_OM_MD_MA_LPORT_KEY         0x01
#define CFM_OM_MD_MA_MEP_SEQ_REC_KEY   0x00


typedef struct CFM_OM_VlanList_S
{
    UI16_T vlan_id;

    struct CFM_OM_VlanList_S *next_p;
}CFM_OM_VlanList_T;

typedef struct CFM_OM_MA_S
{
    CFM_TYPE_MA_Name_T               format;
    CFM_TYPE_MhfCreation_T           mhf_creation;
    CFM_TYPE_CcmInterval_T           ccm_interval;
    CFM_TYPE_CrossCheckStatus_T      cross_check_status;
    CFM_TYPE_CcmStatus_T             ccm_status;
    CFM_TYPE_MhfIdPermission_T       mhf_id_permission;

    CFM_TYPE_AIS_STATUS_T            ais_status;
    CFM_TYPE_AIS_STATUS_T            ais_supress_status;
    CFM_TYPE_MdLevel_T               ais_send_level;

    UI32_T                           index;
    UI32_T                           remote_mep_down_counter;
    UI32_T                           num_of_vids;
    UI16_T                           primary_vid;
    UI16_T                           fault_alarm_domain;
    UI16_T                           ais_period;
    I16_T                             ais_rcvd_timer_idx;
    UI8_T                            name_length;
    UI8_T                            name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
    UI8_T                            fault_alarm_des_address_a[SYS_ADPT_MAC_ADDR_LEN];
    I16_T                            md_ar_idx;
    I16_T                            next_ar_idx;
    I16_T                            ar_idx;
    BOOL_T                           used;
    UI8_T vid_bitmap_a[(SYS_DFLT_DOT1QMAXVLANID/8)+1];
}CFM_OM_MA_T;


typedef struct CFM_OM_MD_S
{
    CFM_TYPE_MD_Name_T              format;
    CFM_TYPE_MdLevel_T              level;
    CFM_TYPE_MhfCreation_T          mhf_creation;
    CFM_TYPE_FNG_LowestAlarmPri_T   low_pri_def;
    CFM_TYPE_MhfIdPermission_T      mhf_id_permission;

    UI32_T                          index;
    UI32_T                          mep_achive_holdtime;
    UI32_T                          next_available_ma_index;
    UI32_T                          fng_alarm_time;
    UI32_T                          fng_reset_time;
    UI16_T                          fault_alarm_domain;
    UI8_T                           name_length;
    UI8_T                           name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T                           fault_alarm_dest_address_a[SYS_ADPT_MAC_ADDR_LEN];
    BOOL_T                          used;
    I16_T                           ar_idx;
    I16_T                           next_ar_idx;
    I16_T                           ma_start_ar_idx;

}CFM_OM_MD_T;

#pragma pack(1)

typedef struct CFM_OM_MEP_S
{
    UI32_T                           md_index;         /*key*/
    UI32_T                           ma_index;         /*key*/
    UI32_T                           identifier;       /*key*/
    UI32_T                           lport;            /*key*/
    UI32_T                           ccm_ltm_priority;

    CFM_TYPE_CcmStatus_T             cci_status;

    UI16_T                           primary_vid;

    UI8_T                            mac_addr_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                            direction;

    BOOL_T                           active;
    BOOL_T                           enable_rmep_defect;

    /*fng
     */
    CFM_TYPE_FNG_State_T             fng_machine_state;
    CFM_TYPE_FNG_HighestDefectPri_T  highest_pri_defect;
    CFM_TYPE_FNG_LowestAlarmPri_T    lowest_priority_defect;
    CFM_TYPE_FNG_HighestDefectPri_T  last_notify_defect;            /* to record the last notified ccm defect */
    UI32_T                           fng_reset_time;
    UI32_T                           fng_alarm_time;

    /*MaDefectIndication
     */
    UI32_T                           error_ccm_last_failure_lenth;
    UI32_T                           xcon_ccm_last_failure_length;
    UI32_T                           rcvd_ccm_sequenc_errors;        /*total number of out-of-seq CCMs received from all remote meps*/
    UI32_T                           rcvd_ccm_amount;
    UI32_T                           cci_sent_ccms;                  /*total number of ccm transmitted.*/

    UI32_T                           rmep_port_down_cnt; /* 0 - no remote MEP port down     */
    UI32_T                           rmep_inf_down_cnt;  /* 0 - all remote MEP interface up */
    UI32_T                           rmep_ccm_loss_cnt;  /* 0 - no remote MEP CCM lost      */
    UI32_T                           rmep_rdi_on_cnt;    /* 0 - no remote MEP RDI           */
    UI32_T                           rmep_lrn_cnt;       /* 0 - no remote MEP learnt        */

    BOOL_T                           some_rdi_defect;     /* remote mep send rdi */
    BOOL_T                           err_mac_status;
    BOOL_T                           some_rmep_ccm_defect;
    BOOL_T                           error_ccm_defect;
    BOOL_T                           xcon_ccm_defect;

    UI8_T                            error_ccm_last_failure_a[CFM_TYPE_MAX_FRAME_RECORD_SIZE];      /*last-received CCM that triggered an defErrorCCm Fault*/
    UI8_T                            xcon_ccm_last_failure_a[CFM_TYPE_MAX_FRAME_RECORD_SIZE];       /*the last-received CCM that triggered a defXconCCM fault*/


    /*lbr
     */
    UI32_T                          lbr_out;                         /*total number of lbr transmitted*/
    UI32_T                          lbr_in;                          /*total number of valid, in-order lbr received*/
    UI32_T                          lbr_in_out_of_order;             /*total number of valid, out-of-order lbr received.*/
    UI32_T                          lbr_bad_msdu;                    /*option, 12.14.7.1.3:aa*/

    /*lbm
     */
    UI32_T                          next_lbm_trans_id;
    UI32_T                          transmit_lbm_seq_number;
    UI32_T                          lbm_current_recevied_seq_number;
    UI32_T                          transmit_lbm_data_tlv_length;
    I32_T                           transmit_lbm_vlan_priority;
    UI32_T                          transmit_lbm_dest_mep_id;
    I16_T                           transmit_lbm_messages;

    UI8_T                           transmit_lbm_dest_mac_address_a[SYS_ADPT_MAC_ADDR_LEN];

    UI8_T                           transmit_lbm_data_tlv_a[CFM_TYPE_MAX_FRAME_RECORD_SIZE];
    UI8_T                           transmit_lbm_data_a[CFM_TYPE_MAX_FRAME_SIZE];

    BOOL_T                          transmit_lbm_dest_is_mep_id;
    BOOL_T                          tramsmit_lbm_status;
    BOOL_T                          transmit_lbm_vlan_drop_enable;
    BOOL_T                          transmit_lbm_result_oK;


    /*ltr
     */
    UI32_T                         unexp_ltr_in;                       /*total nunber of unexpected LTRs received*/
    UI8_T                          ltr_reply_ttl;
    UI8_T                          ltr_relay_action;

    /*ltm
      */
    UI32_T                         ltm_next_seq_number;
    UI32_T                         transmit_ltm_target_mep_id;
    UI32_T                         transmit_ltm_seq_number;

    UI8_T                          transmit_ltm_target_mac_address_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                          transmit_ltm_egress_identifier[SYS_ADPT_MAC_ADDR_LEN+2];
    UI8_T                          transmit_ltm_ttl;

    BOOL_T                         transmit_ltm_target_is_mep_id;
    BOOL_T                         transmit_ltm_result;
    BOOL_T                         transmit_ltm_status;
    BOOL_T                         ltm_use_fdb_only;

    CFM_OM_MD_T                    *md_p;
    CFM_OM_MA_T                    *ma_p;
    I16_T                          md_ar_idx;
    I16_T                          ma_ar_idx;

    /*all timer
     */
    I16_T                          cci_while_timer_idx;
    I16_T                          error_ccm_while_timer_idx;
    I16_T                          xcon_ccm_while_timer_idx;
    I16_T                          fng_while_timer_idx;
    I16_T                          ais_send_timer_idx;
}CFM_OM_MEP_T;

typedef struct CFM_OM_MIP_S
{
    UI32_T                 md_index;   /*key*/
    UI32_T                 ma_index;   /*key*/
    UI32_T                 lport;      /*key*/
    CFM_OM_MD_T            *md_p;
    CFM_OM_MA_T            *ma_p;
    I16_T                  md_ar_idx;
    I16_T                  ma_ar_idx;
    UI8_T                  mac_address_a[SYS_ADPT_MAC_ADDR_LEN];
}CFM_OM_MIP_T;

typedef struct CFM_OM_REMOTE_MEP_S
{
    UI32_T                              md_index;       /*key*/
    UI32_T                              ma_index;       /*key*/
    UI32_T                              identifier;     /*key*/
    UI32_T                              next_sequence;
    UI32_T                              failed_ok_time; /*at ok or fail state timestamp*/
    UI32_T                              rcvd_lport;
    UI32_T                              cc_life_time;
    UI32_T                              frame_loss;
    UI32_T                              packet_received;
    UI32_T                              packet_error;
    I32_T                               age_out;

    UI8_T                               mac_addr_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                               sender_chassis_id_length;
    UI8_T                               sender_chassis_id[CFM_TYPE_MAX_CHASSIS_ID_LENGTH+1];

    UI8_T                               man_length;
    UI8_T                               man_address[CFM_TYPE_MAX_MAN_ADDRESS_LENGTH+1];

    UI8_T                               man_domain_length;
    UI8_T                               man_domain_address[CFM_TYPE_MAX_MAN_ADDRESS_LENGTH+1];

    BOOL_T                              mep_up;
    BOOL_T                              rdi;        /*20.19.2 rdi flag from last received ccm*/
    BOOL_T                              ccm_defect; /*20.19.1, report ccm has been received from reote mep for at least (3.25*ccm_interval)*/

    /*the mep id and direction of received the retmoe mep's CCM*/
    UI32_T                              rcvd_mep_id;
    CFM_TYPE_MP_Direction_T             rcvd_mep_direction;
    CFM_TYPE_PortStatus_T               port_status;
    CFM_TYPE_InterfaceStatus_T          interface_status;
    CFM_TYPE_TlvChassisIdSubtype_T      sender_chassis_id_sub_type;
    CFM_TYPE_RemoteMepState_T           machine_state;

    CFM_OM_MD_T                         *md_p;
    CFM_OM_MA_T                         *ma_p;

    I16_T                               md_ar_idx;
    I16_T                               ma_ar_idx;

    I16_T                               rmep_while_timer_idx;
    I16_T                               archive_hold_timer_idx;

}CFM_OM_REMOTE_MEP_T;


typedef struct CFM_OM_LTR_S
{
    UI32_T                              md_index;/*Key*/
    UI32_T                              ma_index;/*Key*/
    UI32_T                              rcvd_mep_id;/*Key*/
    UI32_T                              seq_number;/*Key*/
    UI32_T                              receive_order;/*Key*/
    UI32_T                              reply_ttl;

    BOOL_T                              forwarded;
    BOOL_T                              terminal_mep;

    UI8_T                               last_egress_identifier[SYS_ADPT_MAC_ADDR_LEN+2];
    UI8_T                               next_egress_identifier[SYS_ADPT_MAC_ADDR_LEN+2];

    UI8_T                               chassis_id_length;
    UI8_T                               chassis_id[CFM_TYPE_MAX_CHASSIS_ID_LENGTH+1];

    UI8_T                               mgmt_addr_domain_len;
    UI8_T                               mgmt_addr_domain[CFM_TYPE_MAX_MAN_ADDRESS_LENGTH+1];
    UI8_T                               mgmt_addr_len;
    UI8_T                               mgmt_addr[CFM_TYPE_MAX_MAN_ADDRESS_LENGTH+1];

    UI8_T                               ingress_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                               ingress_port_id_subtype;
    UI8_T                               ingress_port_id_lenth;
    UI8_T                               ingress_port_Id[CFM_TYPE_MAX_PORT_ID_LENGTH];

    UI8_T                               egress_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                               egress_port_id_subtype;
    UI8_T                               egress_port_id_lenth;
    UI8_T                               egress_port_id[CFM_TYPE_MAX_PORT_ID_LENGTH];

    CFM_TYPE_EgressAction_T             egress_action;
    CFM_TYPE_IngressAction_T            ingress_action;
    CFM_TYPE_TlvChassisIdSubtype_T      chassis_id_subtype;
    CFM_TYPE_RelayAction_T              relay_action;

    UI32_T                              org_tlv_length;
    UI8_T                               org_specific_tlv_a[CFM_TYPE_MAX_ORGANIZATION_TLV_LENGTH];

    I16_T                               hold_timer_idx;
}CFM_OM_LTR_T;
#pragma pack()

typedef struct CFM_OM_GlobalConfig_S
{
    CFM_TYPE_CfmStatus_T                 cfm_status;

    /*cross check configuration
     */
    UI32_T                               remote_mep_start_delay;

    /*link trace global configuration
     */
    UI32_T                               link_trace_hold_time;
    UI32_T                               link_trace_size;
    CFM_TYPE_LinktraceStatus_T           link_trace_cache_status;

    /*snmp trap global configuration
     */
    BOOL_T                               snmp_cc_mep_up_trap;
    BOOL_T                               snmp_cc_mep_down_trap;
    BOOL_T                               snmp_cc_config_trap;
    BOOL_T                               snmp_cc_loop_trap;
    BOOL_T                               snmp_cross_check_mep_unknown_trap;
    BOOL_T                               snmp_cross_check_mep_missing_trap;
    BOOL_T                               snmp_cross_check_ma_up_trap;
}CFM_OM_GlobalConfig_T;

typedef struct CFM_OM_LTR_QueueElement_S
{
    UI32_T                               lport;
    UI16_T                               pduLen;
    UI16_T                               vid;
    UI16_T                               priority;

    UI8_T                                level;
    UI8_T                                *pdu_p;
    UI8_T                                src_mac_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                                dst_mac_a[SYS_ADPT_MAC_ADDR_LEN];

    struct CFM_OM_LTR_QueueElement_S     *next_element_p;
}CFM_OM_LTR_QueueElement_T;

typedef struct CFM_OM_LTR_QUEUE_HEAD_S
{
    UI32_T                               enqueued_LTRs;

    struct CFM_OM_LTR_QueueElement_S     *first_pdu_p;
    struct CFM_OM_LTR_QueueElement_S     *last_pdu_p;
}CFM_OM_LTR_QUEUE_HEAD_T;

typedef struct CFM_OM_ErrorListElement_S
{
    CFM_TYPE_MdLevel_T                   level;
    UI32_T                               mp_id;
    UI32_T                               lport;
    UI32_T                               error_time;

    CFM_TYPE_MEP_ConfigError_T           last_reason;

    UI16_T                               vid;
    I16_T                                md_ar_idx;
    I16_T                                ma_ar_idx;
    I16_T                                ar_idx;

    UI8_T                                mac_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                                reason_bit_map;
    BOOL_T                               used;

    I16_T     nxt_ar_idx;
}CFM_OM_ErrorListElement_T;

typedef struct CFM_OM_ErrorListHead_S
{
    UI32_T          errors_number;
    I16_T           first_ar_idx;
    I16_T           last_ar_idx;
}CFM_OM_ErrorListHead_T;

/*
  Below define the loop back message link list structure
*/
typedef struct CFM_OM_LoopBackListElement_S
{
    UI32_T   lbm_seq_number;
    UI32_T   received_time;

    BOOL_T   used;
    I16_T    ar_idx;
    I16_T    md_ar_idx;
    I16_T    ma_ar_idx;
    I16_T    next_ar_idx;
}CFM_OM_LoopBackListElement_T;

typedef struct CFM_OM_LoopBackListHead_S
{
    I16_T   first_ar_idx;
    I16_T   last_ar_idx;
}CFM_OM_LoopBackListHead_T;

/* for delay measurement
 */
typedef struct CFM_OM_DmrRec_S
{
    UI32_T      dmm_txf_10ms;
    UI32_T      frame_delay_ms;
    UI32_T      fdv_ms;        /* frame delay variation in ms */
    UI8_T       rec_state;
}CFM_OM_DmrRec_T;

typedef struct CFM_OM_DmmCtrlRec_S
{
    CFM_OM_DmrRec_T     dmr_rec_ar[CFM_TYPE_MAX_DMM_COUNT]; /* for send sequence and result */
    UI16_T              src_mep_id;
    UI16_T              dst_mep_id;
    UI16_T              pkt_size;           /* not including (da + sa + eth + fcs) = total size - 18 */
    UI16_T              pkt_pri;            /* priority to tx PDU */
    I16_T               tx_timer_idx;
    UI8_T               dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T               dmr_seq_ar[CFM_TYPE_MAX_DMM_COUNT]; /* 1-based, for rcvd sequence */
    UI8_T               md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T               ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
    UI8_T               md_name_len;
    UI8_T               ma_name_len;
    UI8_T               cur_dmm_seq;
    UI8_T               cur_rcv_idx;
    UI8_T               counts;
    UI8_T               interval;           /* sec      */
    UI8_T               timeout;            /* sec      */
    UI8_T               next_send_timer;    /* sec      */
    UI8_T               fst_timeout_id;     /* 0-based  */
    BOOL_T              is_busy;
}CFM_OM_DmmCtrlRec_T;

/* for throughput measurement
 */
typedef struct CFM_OM_LbmCtrlRec_S
{

    UI32_T              beg_trans_id;
    UI32_T              end_trans_id;
    UI16_T              src_mep_id;
    UI16_T              dst_mep_id;
    UI16_T              counts;             /* pkts     */
    UI16_T              pkt_size;           /* not including (da + sa + eth + fcs) = total size - 18 */
    UI16_T              real_send_counts;   /* pkts     */
    UI16_T              rcv_in_1sec;        /* pkts     */
    UI16_T              rcv_in_time;        /* pkts     */
    UI16_T              pattern;            /* pattern in data TLV */
    UI16_T              pkt_pri;            /* priority to tx PDU */
    I16_T               timeout_timer_idx;
    UI8_T               dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T               md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T               ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
    UI8_T               md_name_len;
    UI8_T               ma_name_len;
    UI8_T               timeout;            /* sec      */
    UI8_T               time_pass;          /* sec      */
    UI8_T               res_bmp;
    BOOL_T              is_busy;
}CFM_OM_LbmCtrlRec_T;

/*
Below define the information which will give to UI
*/
typedef struct CFM_OM_GlobalConfigInfo_S
{
    CFM_TYPE_CfmStatus_T               cfm_global_status;
    UI32_T                             start_delay;

    /*link trace*/
    CFM_TYPE_LinktraceStatus_T         linktrace_cache_status;
    UI32_T                             linktrace_cache_holdTime;
    UI32_T                             linktrace_cache_size;

    /*trap*/
    BOOL_T                             cc_mep_up;
    BOOL_T                             cc_mep_down;
    BOOL_T                             cc_config;
    BOOL_T                             cc_loop;
    BOOL_T                             cross_mep_unknown;
    BOOL_T                             cross_mep_missing;
    BOOL_T                             cross_ma_up;

}CFM_OM_GlobalConfigInfo_T;

typedef struct CFM_OM_MdInfo_S
{
    UI32_T                            index;
    CFM_TYPE_MD_Name_T                name_format;
    CFM_TYPE_MdLevel_T                level;

    CFM_TYPE_MhfCreation_T            mhf_creation;
    CFM_TYPE_MhfIdPermission_T        permission;
    CFM_TYPE_FNG_LowestAlarmPri_T     lowest_alarm_pri;

    UI32_T                            next_index;
    UI32_T                            mep_archive_hold_time;
    UI32_T                            fng_alarm_time;
    UI32_T                            fng_reset_time;
    UI32_T                            row_status;
    UI16_T                            name_len;

    UI8_T                             name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
}CFM_OM_MdInfo_T;

typedef struct CFM_OM_MaInfo_S
{
    UI32_T                              ma_index;
    UI32_T                              md_index;
    UI32_T                              num_of_vids;
    UI32_T                              row_status;

    CFM_TYPE_MA_Name_T                  name_format;
    CFM_TYPE_MhfCreation_T              mhf_creation;
    CFM_TYPE_MhfIdPermission_T          permission;
    CFM_TYPE_CcmInterval_T              interval;
    CFM_TYPE_MdLevel_T                  md_level;
    CFM_TYPE_CcmStatus_T                ccm_status;
    UI16_T                              primary_vid;
    UI16_T                              md_name_len;
    UI16_T                              ma_name_len;
    /*ais*/
    CFM_TYPE_AIS_STATUS_T               ais_status;
    /* ais_suppress_status == TRUE
     *   1. only suppress TRAP
     *     all         in CFM_ENGINE_XmitFaultAlarm &&
     *     MEP_MISSING in CFM_ENGINE_XmitCrossCheckTrap
     *   2. only be effective after AIS is received
     */
    CFM_TYPE_AIS_STATUS_T               ais_suppress_status;
    CFM_TYPE_MdLevel_T                  ais_level;
    UI16_T                              ais_period;
    /* ais_suppresing == TRUE if
     *   AIS received && suppress alarm enabled && crosscheck enabled
     */
    BOOL_T                              ais_suppresing;

    UI8_T                               vlan_list_a[(SYS_DFLT_DOT1QMAXVLANID/8)+1];
    UI8_T                               ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
    UI8_T                               md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
}CFM_OM_MaInfo_T;

typedef struct CFM_OM_MepFngConfig_E
{
    UI32_T                              mep_id;
    UI32_T                              fng_alarm_time;
    UI32_T                              fng_reset_time;

    CFM_TYPE_FNG_HighestDefectPri_T     highest_defect;     /* defect ever occurred b4 */
    CFM_TYPE_FNG_LowestAlarmPri_T       lowest_alarm_pri;

    UI16_T                              md_name_len;
    UI16_T                              ma_name_len;

    UI8_T                               md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T                               ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
}CFM_OM_MepFngConfig_T;

typedef struct CFM_OM_MepInfo_S
{
    UI32_T                              md_index;
    UI32_T                              identifier;
    UI32_T                              lport;                                                /*2*/
    UI32_T                              ma_index;
    UI32_T                              primary_vid;                                          /*4*/
    UI32_T                              ccm_ltm_priority;                                     /*8*/
    UI32_T                              row_status;

    CFM_TYPE_MdLevel_T                  md_level;
    CFM_TYPE_CcmStatus_T                ccm_status;                                           /*7*/
    CFM_TYPE_MP_Direction_T             direction;                                            /*3*/

    UI16_T                              md_name_len;
    UI16_T                              ma_name_len;

    BOOL_T                              active;                                               /*5*/

    UI8_T                               mac_addr_a[SYS_ADPT_MAC_ADDR_LEN];                    /*9*/
    UI8_T                               md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T                               ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1];

    /*fng
      */
    CFM_TYPE_FNG_State_T                fng_state;                                            /*6*/
    CFM_TYPE_FNG_HighestDefectPri_T     highest_pri_defect;     /* defect ever occurred b4 */     /*13*/
    CFM_TYPE_FNG_HighestDefectPri_T     cur_highest_pri_defect; /* current highest defect  */
    CFM_TYPE_FNG_LowestAlarmPri_T       low_pri_def;                                          /*10*/
    UI32_T                              fng_reset_time;                                       /*12*/
    UI32_T                              fng_alarm_time;                                       /*11*/

    /*MaDefectIndication
      */
    UI8_T                               defects;                                              /*14*/

    UI32_T                              ccm_seq_error;                                        /*17*/
    UI32_T                              cci_sent_ccms;                                        /*18*/

    /*lbr
      */
    UI32_T                              lbr_out;                                               /*25*/
    UI32_T                              lbr_in;                                                /*20*/
    UI32_T                              lbr_in_out_of_order;                                   /*21*/
    UI32_T                              lbr_bad_msdu;                                          /*22*/

    /*lbm
      */
    UI32_T                              next_lbm_trans_id;                                     /*19*/
    UI32_T                              trans_lbm_seq_num;                                     /*35*/
    UI32_T                              trans_lbm_dst_mep_id;                                  /*28*/
    I32_T                               trans_lbm_vlan_priority;                               /*32*/

    I16_T                               trans_lbm_msg;                                         /*30*/

    BOOL_T                              trans_lbm_status;                                      /*26*/
    BOOL_T                              trans_lbm_dst_is_mep_id;                               /*29*/
    BOOL_T                              trans_lbm_vlan_drop_enabled;                           /*33*/
    BOOL_T                              trans_lbm_result_ok;                                   /*34*/

    UI8_T                               trans_lbm_dst_mac_addr_a[SYS_ADPT_MAC_ADDR_LEN];       /*27*/

    /*ltr
     */
    UI32_T                              unexp_ltr_in;                                          /*24*/
    UI8_T                               ltr_reply_ttl;
    UI8_T                               ltr_relay_action;

    /*ltm
      */
    UI32_T                              ltm_next_seq_num;                                      /*23*/
    UI32_T                              trans_ltm_target_mep_id;                               /*39*/
    UI32_T                              trans_ltm_seq_num;                                     /*43*/

    BOOL_T                              trans_ltm_status;                                      /*36*/
    BOOL_T                              trans_ltm_target_is_mep_id;                            /*40*/
    BOOL_T                              trans_ltm_result;                                      /*42*/
    BOOL_T                              ltm_use_fdb_only;                                      /*37*/

    UI8_T                               trans_ltm_ttl;                                         /*41*/
    UI8_T                               trans_ltm_egress_id_a[SYS_ADPT_MAC_ADDR_LEN+2];        /*44*/
    UI8_T                               trans_ltm_target_mac_addr_a[SYS_ADPT_MAC_ADDR_LEN];    /*38*/
}CFM_OM_MepInfo_T;

typedef struct CFM_OM_MipInfo_S
{
    UI32_T                              md_index;
    UI32_T                              ma_index;
    UI32_T                              vid;
    UI32_T                              lport;

    UI16_T                              md_name_len;
    UI16_T                              ma_name_len;

    CFM_TYPE_MdLevel_T                  md_level;

    UI8_T                               md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T                               ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
}CFM_OM_MipInfo_T;

typedef struct CFM_OM_RemoteMepCrossCheck_S
{
    UI32_T                              incoming_port;
    UI32_T                              cc_life_time;
    UI32_T                              age_of_last_cc;
    UI32_T                              frame_loss;
    UI32_T                              packet_rcvd_count;
    UI32_T                              packet_error_count;
    UI32_T                              mep_id;
    UI32_T                              primary_vid;
    UI32_T                              failed_ok_time;
    UI32_T                              row_status;

    UI32_T                              chassis_id_len;
    UI32_T                              mgmt_len;
    UI32_T                              mgmt_addr_domain_len;

    UI16_T                              md_name_len;
    UI16_T                              ma_name_len;

    CFM_TYPE_TlvChassisIdSubtype_T      chassis_id_subtype;
    CFM_TYPE_RemoteMepState_T           mep_state;
    CFM_TYPE_MdLevel_T                  level;
    CFM_TYPE_PortStatus_T               port_status;
    CFM_TYPE_InterfaceStatus_T          interface_status;

    BOOL_T                              mep_up;
    BOOL_T                              rdi;
    BOOL_T                              cross_check_enabled;

    UI8_T                               md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T                               ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
    UI8_T                               mep_mac_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                               chassis_id_a[CFM_TYPE_MAX_CHASSIS_ID_LENGTH+1];
    UI8_T                               mgmt_addr_domain_a[CFM_TYPE_MAX_MAN_DOMAIN_LENGTH+1];
    UI8_T                               mgmt_addr_a[CFM_TYPE_MAX_MAN_ADDRESS_LENGTH+1];
}CFM_OM_RemoteMepCrossCheck_T;

typedef struct CFM_OM_LinktraceReply_S
{
    UI32_T                              hops;
    UI32_T                              reply_ttl;
    UI32_T                              chassis_id_len;
    UI32_T                              mgmt_addr_len;
    UI32_T                              mgmt_domain_len;
    UI32_T                              ingress_port_id_len;
    UI32_T                              egress_port_id_len;
    UI32_T                              org_specific_tlv_len;

    UI16_T                              md_name_len;
    UI16_T                              ma_name_len;

    CFM_TYPE_TlvChassisIdSubtype_T      chassis_id_subtype;
    CFM_TYPE_PortIdSubtype_T            ingress_port_id_subtype;
    CFM_TYPE_PortIdSubtype_T            egress_port_id_subtype;
    CFM_TYPE_IngressAction_T            ingress_action;
    CFM_TYPE_EgressAction_T             egress_action;
    CFM_TYPE_RelayAction_T              relay_action;

    BOOL_T                              forwarded;
    BOOL_T                              terminal_mep;

    UI8_T                               *org_specific_tlv_p;
    UI8_T                               md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T                               ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
    UI8_T                               chassis_id_a[CFM_TYPE_MAX_CHASSIS_ID_LENGTH+1];
    UI8_T                               mgmt_domain_a[CFM_TYPE_MAX_MAN_DOMAIN_LENGTH+1];
    UI8_T                               mgmt_addr_a[CFM_TYPE_MAX_MAN_ADDRESS_LENGTH+1];
    UI8_T                               last_egress_id_a[SYS_ADPT_MAC_ADDR_LEN+2];
    UI8_T                               next_egress_id_a[SYS_ADPT_MAC_ADDR_LEN+2];
    UI8_T                               ingress_port_mac_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                               ingress_port_id_a[CFM_TYPE_MAX_PORT_ID_LENGTH];
    UI8_T                               egress_port_mac_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                               egress_port_id_a[CFM_TYPE_MAX_PORT_ID_LENGTH];
}CFM_OM_LinktraceReply_T;

typedef struct CFM_OM_Error_S
{
    UI32_T                              mep_id;
    UI32_T                              lport;

    CFM_TYPE_MdLevel_T                  level;
    CFM_TYPE_MEP_ConfigError_T          reason;

    UI16_T                              vlan_id;
    UI16_T                              md_name_len;
    UI16_T                              ma_name_len;

    UI8_T                               reason_bit_map;
    UI8_T                               md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T                               ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
    UI8_T                               mac_addr_a[SYS_ADPT_MAC_ADDR_LEN];
}CFM_OM_Error_T;

typedef struct CFM_OM_LoopbackInfo_S
{
    UI32_T                              lbm_seq_num;
    UI32_T                              receivedTime;
    UI16_T                              md_name_len;
    UI16_T                              ma_name_len;
    UI8_T                               md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
    UI8_T                               ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
}CFM_OM_LoopbackInfo_T;



/*---------------------------------------------------------------------------------
 * FUNCTION : CFM_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for CFM OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void CFM_OM_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the cfm size for share memory
 * INPUT    : None
 * OUTPUT   : segid_p  -- shared memory segment id to be recorded by SYSRSC_MGR
 *            seglen_p -- length of the shared memroy segment to be recorded by SYSRSC_MGR
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_OM_GetShMemInfo(
                        SYSRSC_MGR_SEGID_T *segid_p,
                        UI32_T *seglen_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_Init(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_EnterSlaveMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM value
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_EnterSlaveMode(void);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_EnterTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM value
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_EnterTransitionMode();
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetOperatingMode
 * ------------------------------------------------------------------------
 * PURPOSE  : get the operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T CFM_OM_GetOperatingMode(void);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE  : set the operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_SetTransitionMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_EnterMasterMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM and global value
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_EnterMasterMode(void);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get the mep pointer from the hisame table
 * INPUT    : md_index   - the maintenance domain index
 *            ma_index   - the maintenance association index
 *            l_port     - the logical port
 *            key_type   - use which key type
 *                       - CFM_OM_MD_MA_MEP_KEY
 *                       - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT   : mep_p       - the pointer of mep store in hisam
 * RETUEN   : TRUE       - get success
 *            FALSE      - get fail, no record.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMep(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T mep_id,
                    UI32_T l_port,
                    UI16_T key_type,
                    CFM_OM_MEP_T *mep_p);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMep
 * ------------------------------------------------------------------------
 * PURPOSE  :This will get the next  mep pointer from the hisame table
 * INPUT    :md_index  - the maintenance domain index
 *           ma_index  - the maintenance association index
 *           l_port    - the logical port
 *           key_type  - use which key type
 *                     - CFM_OM_MD_MA_MEP_KEY
 *                     - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT   :mep_p      - the pointer of mep store in hisam
 * RETUEN   :TRUE      - get success
 *           FALSE     - get fail, no record.
 * NOTES    :None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMep(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T l_port ,
                        UI16_T key_type,
                        CFM_OM_MEP_T *mep_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new  mep pointer to the hisame table
 * INPUT    : *md_p     - the maintenance domain pointer
 *            *ma_p     - the maintenance association pointer
 *            l_port    - the logical port
 *            direction - the mep direction
 * OUTPUT   :
 * RETUEN   : TRUE      - add success
 *            FALSE     - add fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewMep(
                        CFM_OM_MD_T *md_p,
                        CFM_OM_MA_T *ma_p,
                        UI32_T mep_id,
                        UI32_T l_port,
                        CFM_TYPE_MP_Direction_T direction);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a delete a  mep pointer from the hisame table
 * INPUT    : md_index - the maintenance domain index
 *            ma_index - the maintenance association index
 *            mep_id   - the mep id
 *            l_port   - the logical port
 *            key_type - use which key type
 *                     - CFM_OM_MD_MA_MEP_KEY
 *                     - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT   :
 * RETUEN   : TRUE     - delet success
 *            FALSE    - delet fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMep(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T l_port,
                        UI16_T key_type);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMep
 * ------------------------------------------------------------------------
 * PURPOSE : This function will modify the mep key in hisam table
 * INPUT   : md_index   - the maintenance domain index
 *           ma_index   - the maintenance association index
 *           old_mep_id - the original mep id
 *           old_lport  - the origianl logical port
 *           new_mep_id - the new mep id
 *           new_lport  - the new lport
 *           key_type   - use which key type
 *                      - CFM_OM_MD_MA_MEP_KEY
 *                      - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT  :
 * RETUEN  : TRUE       - success
 *           FALSE      - fail
 * NOTES   : because key change, so must delete the old record then add new record
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMep(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T old_mep_id,
                    UI32_T old_lport,
                    UI32_T new_mep_id,
                    UI32_T new_lport,
                    UI16_T key_type);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_StoreMep
 * ------------------------------------------------------------------------
 * PURPOSE : This function store the mep's all information again
 * INPUT   : mep_p - the pointer to mep which has content to store
 * OUTPUT  :
 * RETUEN  : TRUE       - success
 *           FALSE      - fail
 * NOTES   : because key change, so must delete the old record then add new record
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_StoreMep(
                       CFM_OM_MEP_T *mep_p );

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMeplport
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's logical port
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            lport     - the logical port
 *            new_mac_p - pointer to new mac address
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.delete the old mep record from hisam table
 *            2. add new mep record to hisam table
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMeplport(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T  lport,
                        UI8_T   *new_mac_p);

#if 0
/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepDirection
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's logical port
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            lport   - the logical port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepDirection(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            CFM_TYPE_MP_Direction_T direction);
#endif /* #if 0 */

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepPrimaryVid
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's primary vid
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            lport   - the logical port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepPrimaryVid(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI16_T primary_vid);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepActive
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's active status
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            active_status- the mep active status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepActive(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        BOOL_T active_status);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepCciStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            status    - the cci status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : CFM_TYPE_CCI_STATUS_ENABLE
 *            CFM_TYPE_CCI_STATUS_DISABLE
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepCciStatus(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            CFM_TYPE_CcmStatus_T cci_status);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepCcmLtmPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ccm_ltm_priority-the ccm and ltm packet priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepCcmLtmPriority(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T ccm_ltm_priority);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLowPrDef
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lowest alarm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            low_pri   - the lowerst defect priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : CFM_TYPE_FNG_LOWEST_ALARM_ALL
 *            CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON
 *            CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON
 *            CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON
 *            CFM_TYPE_FNG_LOWEST_ALARM_XCON
 *            CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLowPrDef(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            CFM_TYPE_FNG_LowestAlarmPri_T low_pri);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepFngAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fng alarm time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            alarm_time  - the fault alarm time by ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepFngAlarmTime(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T alarm_time);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepFngResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fault alarm reset time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            reset_time- the fault alarm reset time by ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepFngResetTime(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T reset_time);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmDstMac
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lbm destination mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mac   - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmDstMac(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN]);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmDestMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination mac address
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mep_id - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmDestMepId(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T dst_mep_id);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmTargetIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lbm destination mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mep_id- the lbm destination mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmTargetIsMepId(
                                    UI32_T md_index,
                                    UI32_T ma_index,
                                    UI32_T mep_id,
                                    UI8_T dst_mep_id);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmMessages
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit the lbm counts
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            counts    - the lbm message counts
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmMessages(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI32_T counts);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmVlanPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit blm vlan priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            priority  - the lbm priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmVlanPriority(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T priority);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmFlags
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm transmit flags
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 *            is_useFDBonly - set only use the fdb or not
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmFlags(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            BOOL_T is_useFDBonly);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmTargetMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mac-the ltm target address
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmTargetMacAddress(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T target_mac[SYS_ADPT_MAC_ADDR_LEN]);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmTargetMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mp id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mep_id - the target mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmTargetMepId(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T target_mep_id);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmTargetIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm target is the mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - the ltm target is the mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmTargetIsMepId(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                BOOL_T is_mep_id);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmTtl
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm ttl
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ttl       - the trnamsmit ttl
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmTtl(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T ttl);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMip
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get the mip pointer from the hisame table
 * INPUT    : md_index   - the maintenance domain index
 *            ma_index   - the maintenance association index
 *            lport      - the logical port
 *            key_type   - use which key type
 *                       - CFM_OM_LPORT_MD_MA_KEY
 *                        CFM_OM_MD_MA_LPORT_KEY
 * OUTPUT   : *mip_pp   - the pointer of mip store in hisam
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMip(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T lport,
                    UI16_T key_type,
                    CFM_OM_MIP_T *mip_p);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMip
 * ------------------------------------------------------------------------
 * PURPOSE  :This will get the next  mip pointer from the hisame table
 * INPUT    :md_index   - the maintenance domain index
 *           ma_index   - the maintenance association index
 *           key_type   - use which key type
 *           lport      - the logical port
 *                      - CFM_OM_LPORT_MD_MA_KEY
 *                        CFM_OM_MD_MA_LPORT_KEY
 * OUTPUT   :*mip_pp   - the pointer of mip store in hisam
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    :None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMip(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T lport,
                        UI16_T key_type,
                        CFM_OM_MIP_T *mip_p);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewMip
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new mip pointer to the hisame table
 * INPUT    : md_p   - the maintenance domain pointer
 *            ma_p   - the maintenance association pointer
 *            lport  - the logical port, the mip create on
 *            mac  - the mip mac
 * OUTPUT   :
 * RETUEN   : TRUE   - success
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewMip(
                        CFM_OM_MD_T *md_p,
                        CFM_OM_MA_T *ma_p,
                        UI32_T lport,
                        UI8_T mac_a[SYS_ADPT_MAC_ADDR_LEN]);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMip
 * ------------------------------------------------------------------------
 * PURPOSE : This function will modify the mip key in hisam table
 * INPUT   : md_index   - the maintenance domain index
 *           ma_index   - the maintenance association index
 *           old_lport  - the origianl logical port
 *           new_lport  - the new lport
 *           key_type   - use which key type
 *                      - CFM_OM_MD_MA_LPORT_KEY
 *                      - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT  :
 * RETUEN  : TRUE       - success
 *           FALSE      - fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMip(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T old_lport,
                    UI32_T new_lport,
                    UI16_T key_type);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_StoreMip
 * ------------------------------------------------------------------------
 * PURPOSE : This function store the mip's all information again
 * INPUT   : mip_p - the pointer to mep which has content to store
 * OUTPUT  :
 * RETUEN  : TRUE       - success
 *           FALSE      - fail
 * NOTES   : because key change, so must delete the old record then add new record
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_StoreMip(
                       CFM_OM_MIP_T *mip_p );
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMip
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a delete a  mip pointer from the hisame table
 * INPUT    : md_index- the maintenance domain index
 *            ma_index- the maintenance association index
 *            lport   - the logical port
 *            key_type - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT   :
 * RETUEN   : TRUE    - success
 *            FALSE   - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMip(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T lport,
                        UI16_T key_type);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRemoteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get a remote mep pointer from the hisame table
 * INPUT    : md_index  - the maintenance domain index of rcvd md
 *            ma_index  - the maintenance association index of rcvd ma
 *            mep_id    - the mep id of remote mep
 *            key_type  - use which key type
 *                       - CFM_OM_MD_MA_MEP_KEY
 * OUTPUT   : *remote_mep_pp - the pointer of remote mep store in hisam
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetRemoteMep(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T key_type,
                        CFM_OM_REMOTE_MEP_T *remote_mep_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRemoteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get next remote mep pointer from the hisame table
 * INPUT    : md_index   - the maintenance domain index of rcvd md
 *            ma_index   - the maintenance association index of rcvd ma
 *            mep_id     - the mep id or remote
 *            key_type   - use which key type
 *                        - CFM_OM_MD_MA_MEP_KEY
 * OUTPUT   : remote_mep_p - the pointer of mep store in hisam
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextRemoteMep(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI16_T key_type,
                            CFM_OM_REMOTE_MEP_T *remote_mep_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewRemoteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new remote mep pointer to the hisame table
 * INPUT    : *md_p    - the maintenance domain pointer of the rcvd mep
 *            *ma_p    - the maintenance association pointer of the rcvd mep
 *            r_mep_id   - the remote mep pointer want to save in hisam
 * OUTPUT   :
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewRemoteMep(
                            CFM_OM_MD_T *md_p,
                            CFM_OM_MA_T *ma_p,
                            UI32_T r_mep_id);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_ResetRemoteMepData
 *-------------------------------------------------------------------------
 * PURPOSE  : This function initial the remote mep
 * INPUT    : *r_mep_p   - the remote mep pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_OM_ResetRemoteMepData(
                        CFM_OM_REMOTE_MEP_T *r_mep_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteRemoteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will delete a remote mep pointer from the hisame table
 * INPUT    : md_index   - the maintenance domain index of the rcvd mep
 *            ma_index   - the maintenance association index of the rcvd mep
 *            r_mep_id     - the mep id of the remote mep
 *            key_type   - use which key type
 *                         - CFM_OM_MD_MA_MEP_KEY
 * OUTPUT   :
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteRemoteMep(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T r_mep_id,
                            UI32_T key_type);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will modify the mep content in hisam table
 * INPUT    :  r_mep_p       - the content will replace the old content in hiame table
 * OUTPUT   :
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetRemoteMep(
                    CFM_OM_REMOTE_MEP_T *r_mep_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new  mep pointer to the hisame table
 * INPUT    : md_index  - the maintenance domain index
 *            ma_index  - the maintenance association index
 *            mep_id    - the mep id
 *            seq_num   - the seq_num of ltr
 *            rcvd_order- the receive ltr order
 *            key_type  - use which key type
 *                       - CFM_OM_MD_MA_MEP_SEQ_REC_KEY
 * OUTPUT   : ltr_p      - the pointer of ltr store in hisam
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetLtr(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T mep_id,
                    UI32_T seq_num,
                    UI32_T rcvd_order,
                    UI32_T key_type,
                    CFM_OM_LTR_T *ltr_p);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get next ltr pointer from the hisame table
 * INPUT    : md_index  - the maintenance domain index
 *            ma_index  - the maintenance association index
 *            mep_id    - the mep id
 *            seq_num   - the seq_num of ltr
 *            rcvd_order- the receive ltr order
 *            key_type  - use which key type
 *                       - CFM_OM_MD_MA_MEP_SEQ_REC_KEY
 * OUTPUT   : pLtr       - the pointer of ltr store in hisam
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextLtr(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T seq_num,
                        UI32_T rcvd_order,
                        UI32_T key_type,
                        CFM_OM_LTR_T *ltr_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_IsLTCacheFull
 * ------------------------------------------------------------------------
 * PURPOSE  : To chcek if link trace cache is full
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE  - FULL
 *            FALSE - NOT FULL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_IsLTCacheFull(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new ltr pointer to the hisame table
 * INPUT    : ltr_p - the pointer to the content of ltr
 * OUTPUT   : None
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddLtr(
                    CFM_OM_LTR_T *ltr_p);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a delete ltr pointer from the hisame table
 * INPUT    : ltr_p - the ltr which want to delete
 * OUTPUT   :
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : The key value already exsit in ltr
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteLtr(
                 CFM_OM_LTR_T *ltr_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will modify the ltr contnet in hisam table
 * INPUT    :  ltr_p       - the point to the content which will replace the old ltr in hisam table
 * OUTPUT   :
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetLtr(
                        CFM_OM_LTR_T *ltr_p);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByMdMaIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the input level and vid.
 * INPUT    : ma_idx        - ma index
 * OUTPUT   : **return_md_p - the md record
 *            **return_ma_p - the ma record
 * RETUEN   : TRUE          - success
 *            FALSE         - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByMdMaIndex(
                                UI32_T md_index,
                                UI32_T ma_idx,
                                CFM_OM_MD_T **return_md_pp,
                                CFM_OM_MA_T **return_ma_pp);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the input ma name.
 * INPUT    : *ma_name       - the ma name pointer
 *            name_len       - ma name length
 * OUTPUT   : **return_md_pp - the md record
 *            **return_ma_pp - the ma record
 * RETUEN   : TRUE           - success
 *            FALSE          - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByMaName(
                            UI8_T *ma_name_ap,
                            UI32_T name_len,
                            CFM_OM_MD_T **return_md_pp,
                            CFM_OM_MA_T **return_ma_pp);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByMdMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : To get pointer to md/ma for specified md and ma name.
 * INPUT    : md_name_p    - the md name pointer
 *            md_name_len  - md name length
 *            ma_name_p    - the ma name pointer
 *            ma_name_len  - ma name length
 * OUTPUT   : return_md_pp - the md record
 *            return_ma_pp - the ma record
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByMdMaName(
    UI8_T           *md_name_p,
    UI32_T          md_name_len,
    UI8_T           *ma_name_p,
    UI32_T          ma_name_len,
    CFM_OM_MD_T     **return_md_pp,
    CFM_OM_MA_T     **return_ma_pp);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByMdIndxMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the input ma name.
 * INPUT    : md_index       - the md index
 *            *ma_name_ap    - the ma name array pointer
 *            name_len       - the ma name length
 * OUTPUT   : **return_md_pp - the md record
 *            **return_ma_pp - the ma record
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByMdIndxMaName(
                                UI32_T md_index,
                                UI8_T *ma_name_ap,
                                UI32_T name_len,
                                CFM_OM_MD_T **return_md_pp,
                                CFM_OM_MA_T **return_ma_pp);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByLevelVid
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the level vid
 * INPUT    : level         - the md level
 *            vid           - the vlan id
 * OUTPUT   : *return_md_p  - the md record
 *            **return_ma_p - the ma record
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByLevelVid(
                            CFM_TYPE_MdLevel_T level,
                            UI16_T vid,
                            CFM_OM_MD_T **return_md_pp,
                            CFM_OM_MA_T **return_ma_pp);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetFirstHighLevelMdMaBylevelVid
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the first low level md and ma index according to the level vid
 * INPUT    : level         - the md level
 *            vid           - the vlan id
 * OUTPUT   : *return_md_p  - the md record
 *            **return_ma_p - the ma record
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetFirstHighLevelMdMaBylevelVid(
                                CFM_TYPE_MdLevel_T level,
                                UI16_T vid,
                                CFM_OM_MD_T **return_md_pp,
                                CFM_OM_MA_T **return_ma_pp);

 /* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdByIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the Md pointer
 * INPUT    : md_index - md index
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_MD_T* CFM_OM_GetMdByIndex(
                                UI32_T md_index);

 /* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMdByIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the next Md pointer
 * INPUT    : md_index - md index
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_MD_T* CFM_OM_GetNextMdByIndex(
                                    UI32_T md_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdByName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the Md pointer
 * INPUT    : level - the md level
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : this will get the first md in the same level
 * ------------------------------------------------------------------------
 */
CFM_OM_MD_T* CFM_OM_GetMdByLevel(
                                CFM_TYPE_MdLevel_T level);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMdByLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md according to the input level.
 * INPUT    : level - md level
 *            index - index of current md and to get the next md
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
*/
CFM_OM_MD_T* CFM_OM_GetNextMdByLevel(
                                    UI32_T md_index,
                                    CFM_TYPE_MdLevel_T level);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdByName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the Md pointer
 * INPUT    : *md_name_ap  - md name
 *            name_len  - md name length
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    :
 * ------------------------------------------------------------------------
 */
CFM_OM_MD_T* CFM_OM_GetMdByName(
                                UI8_T *md_name_ap,
                                UI32_T name_len);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewMd
 * ------------------------------------------------------------------------
 * PURPOSE  : This function add a new md to the md link list
 * INPUT    : md_index    - md index
 *            level       - md level
 *            name_length - md name length
 *            *name_ap        - md name pointer
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETUEN   : TRUE         - success
 *            FALSE        - it has already has this md index
 *                               or it allocate space for store new md
 * NOTES    : Md in the sorted linked list by md index
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewMd(
                        UI32_T md_index,
                        CFM_TYPE_MdLevel_T level,
                        UI16_T name_length,
                        UI8_T *name_ap,
                        CFM_TYPE_MhfCreation_T create_way);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMd
 * ------------------------------------------------------------------------
 * PURPOSE  : This function delet a md from the md link list
 * INPUT    : md_index - md index
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMd(
                        UI32_T md_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMd
 * ------------------------------------------------------------------------
 * PURPOSE  : This function  modify the md content
 * INPUT    : *md_p       - md record pointer
 *            level       - md level
 *            name_length - md name length
 *            *name_ap   - md name pointer
 *            create_way  - the mip create way
 * OUTPUT   :
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMd(
                    CFM_OM_MD_T *md_p,
                    UI8_T level,
                    UI16_T name_length,
                    UI8_T *name_ap,
                    CFM_TYPE_MhfCreation_T create_way);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMdMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           create_type - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMdMhfCreation(
                            UI32_T md_index,
                            CFM_TYPE_MhfCreation_T create_type);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMdMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMdMhfIdPermission(
                            UI32_T md_index,
                            CFM_TYPE_MhfIdPermission_T send_id_permission);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetNextAvailableMdIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next md index
 * INPUT    :
 * OUTPUT   :
 * RETURN   :the next md index can use
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextAvailableMdIndex();

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetMaxMaNameLengthInMd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the max ma name length in this md
 * INPUT    :
 * OUTPUT   :
 * RETURN   :the max ma name length
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetMaxMaNameLengthInMd(
                                    CFM_OM_MD_T *md_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function add a new ma
 * INPUT    : md_index     - md index
 *            ma_index     - ma index
 *            ccm_interval - the ccm trasmit interval
 *            name_length  - md name length
 *            *name_ap        - md name
 *            primary_vid  - primary vid of the ma
 *            vid_num      - the vid num in list
 *            vid_list     - the array store the vid list
 *            create_way  - the mip create way
 * OUTPUT   :
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewMa(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI16_T name_length,
                    UI8_T *name_ap,
                    UI16_T primary_vid,
                    UI32_T vid_num,
                    UI8_T vid_list[(SYS_DFLT_DOT1QMAXVLANID/8)+1],
                    CFM_TYPE_MhfCreation_T create_way);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function delete a ma
 * INPUT    : md_index   - md index
 *            ma_index   - ma index
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMa(
                    UI32_T md_index,
                    UI32_T ma_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function delete a ma
 * INPUT    : md_index   - md index
 *            ma_index   - ma index
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMaVlan(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI16_T vid);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function modify the ma name
 * INPUT    : *md_p        - the md record pointer
 *            *ma_p        - the ma record pointer
 *            *name_ap     - the m name array pointer
 *            name_len     - md name length
 *            primary_vid  - the primary vid of the ma
 *            vid_num      - the vid number is vid_list
 *            vlid_list    - the vid lists store the vids
 *            create_way  - the mip create way
 * OUTPUT   :
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMa(
                    CFM_OM_MD_T *md_p,
                    CFM_OM_MA_T *ma_p,
                    UI8_T *name_ap,
                    UI32_T name_len,
                    UI16_T primary_vid,
                    UI32_T vid_num,
                    UI8_T vid_list[(SYS_DFLT_DOT1QMAXVLANID/8)+1],
                    CFM_TYPE_MhfCreation_T create_way);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMaMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma mhf creation type
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            create_type -the mhf cration type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 * 	          CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaMhfCreation(
                        UI32_T md_index,
                        UI32_T ma_index,
                        CFM_TYPE_MhfCreation_T create_type);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMaMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           ma_index   - the ma index
 *           mhf_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaMhfIdPermission(
                            UI32_T md_index,
                            UI32_T ma_index,
                            CFM_TYPE_MhfIdPermission_T mhf_id_permission);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMaNumOfVids
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma can have more than on vid
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            vid_num   - can have more than one vid
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaNumOfVids(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T vid_num);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMaName
 *-------------------------------------------------------------------------
 * PURPOSE  : this function set the ma name
 * INPUT    : md_p         - the md pointer
 *            ma_p         - the ma pointer
 *            name_ap      - the ma name array pointer
 *            name_length  - the ma name length
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaName(
    CFM_OM_MD_T *md_p,
    CFM_OM_MA_T *ma_p,
    UI8_T       *name_ap,
    UI32_T      name_length);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMaNameFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : To set the name format of MA
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            name_format - the name format
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : if name_format == CFM_TYPE_MA_NAME_ICC_BASED
 *               name_length must <= 13
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaNameFormat(
    UI32_T              md_index,
    UI32_T              ma_index,
    CFM_TYPE_MA_Name_T  name_format);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMaCCInterval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function modify the ma ccm interval
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            interval - the ccm trasmit interval
 * OUTPUT   :
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaCCInterval(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI8_T interval);

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMaCCInterval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function modify the ma ccm interval
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : *interval   - the ma ccm interval pointer
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTE     : if ma_name_len specified, it will use ma name to be the key
 *                of it will use the ma_index to be the key
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMaCCInterval(
                              UI32_T md_index,
                              UI32_T ma_index,
                              UI8_T *ma_name_ap,
                              UI32_T name_len,
                              CFM_TYPE_CcmInterval_T *interval);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMaCcInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next ccm interval
 * INPUT    : md_index     - the md index
 *            *ma_index_p    - the ma index
 *            *ma_name_ap  - the ma name array pointer
 *            aname_len     - the ma name length
 * OUTPUT   : *interval_p    - the ccm interval
 *            *ma_index_p    - the next ma index
 *            *ma_name_ap  - the name ma name array pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if ma_name_len specified, it will use ma name to be the key
 *                of it will use the ma_index to be the key
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMaCcInterval(
                             UI32_T md_index,
                             UI32_T *ma_index_p,
                             UI8_T *ma_name_ap,
                             UI32_T name_len,
                             CFM_TYPE_CcmInterval_T *interval_p);

#if 0 //not used at present
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMaCcStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the CCM status
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : *status_p   - the ccm status
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if ma_name_len specified, it will use ma name to be the key
 *            of it will use the ma_index to be the key
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMaCcStatus(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI8_T *ma_name_ap,
                            UI32_T name_len,
                            CFM_TYPE_CcmStatus_T *status_p);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMaCcStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next ccm status
 * INPUT    : md_index    - the md index
 *            *ma_index   - the ma_index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : *status_p   - the ccm status
 *            *ma_index   - the next ma index
 *            *ma_name_ap - the next ma name array pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if ma_name_len specified, it will use ma name to be the key
 *                of it will use the ma_index to be the key
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMaCcStatus(
                           UI32_T md_index,
                           UI32_T *ma_index,
                           UI8_T *ma_name_ap,
                           UI32_T name_len,
                           CFM_TYPE_CcmStatus_T *status_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function modify the ma content
 * INPUT    : md_index - md index
 *            ma_index - ma index
 * OUTPUT   : ma pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_MA_T* CFM_OM_GetMa(
                        UI32_T md_index,
                        UI32_T ma_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMaByMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get  the next ma pointer
 * INPUT    : md_index - md index
 *            ma_index - ma index
 * OUTPUT   : ma pointer
 * RETUEN   : None
 * NOTES    : This get next ma is get the next ma under the md
 * ------------------------------------------------------------------------
 */
CFM_OM_MA_T* CFM_OM_GetNextMaByMaIndex(
                                    UI32_T md_index,
                                    UI32_T ma_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMaByMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get  the next ma pointer
 * INPUT    : md_index    - the md index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : ma pointer
 * RETUEN   : None
 * NOTES    : This get next ma is get the next ma under the md
 * ------------------------------------------------------------------------
 */
CFM_OM_MA_T* CFM_OM_GetNextMaByMaName(
                                    UI32_T md_index,
                                    UI8_T *ma_name_ap,
                                    UI32_T name_len);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMaxMdLevelOfAllMd
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the max level
 * INPUT    : None
 * OUTPUT   : max level of all md
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_MdLevel_T CFM_OM_GetMaxMdLevel( );

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddToLtrQueue
 * ------------------------------------------------------------------------
 * PURPOSE  : the function add the ltr which will reply to the sender into the queue
 * INPUT    : *ltr_queue_p  -the element which want to put into list
 * OUTPUT   : None
 * RETUEN   : TRUE          - success
 *            FALSE         - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddToLtrQueue(
                            CFM_OM_LTR_QueueElement_T *ltr_queue_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRemoveFirstElementFromLTrQueue
 * ------------------------------------------------------------------------
 * PURPOSE  : the function get the first element and unlink the first element from list
 * INPUT    : **ltr_queue_pp - the element pointer which get from list
 * OUTPUT   : None
 * RETUEN   : TRUE          - success
 *            FALSE         - fail
 * NOTES    : This function won't free the return pointer
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetRemoveFirstElementFromLTrQueue(
                                CFM_OM_LTR_QueueElement_T **ltr_queue_pp);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteAllLTrQueueElement
 * ------------------------------------------------------------------------
 * PURPOSE  :This function will delet and free all pending element in queue
 * INPUT    None
 * OUTPUT   : None
 * RETUEN   : TRUE          - success
 *            FALSE         - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteAllLTrQueueElement();
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLtrQueueHeadPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : the funciton will return the ltr queue header ptr
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_LTR_QUEUE_HEAD_T * CFM_OM_GetLtrQueueHeadPtr();

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetCrossCheckStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck status.
 * INPUT    : status - the cross check status
 *            ma_p   - the ma pointer
 * OUTPUT   : None
 * RETUEN   : TRUE   - success
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetCrossCheckStatus(
                            CFM_TYPE_CrossCheckStatus_T status,
                            CFM_OM_MA_T *ma_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCrossCheckStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the crosscheck status.
 * INPUT    : *ma_name_ap   - the ma name array pointer
 *            *ma_len       - the ma length
 * OUTPUT   : *status_p       - cross check status
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCrossCheckStatus(
                              CFM_TYPE_CrossCheckStatus_T *status_p,
                              UI8_T *ma_name_ap,
                              UI32_T ma_len);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetCrossCheckStartDelay
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : delay - the start delay time
 * OUTPUT   : None
 * RETUEN   : TRUE  - success
 *            FALSE - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetCrossCheckStartDelay(
                            UI32_T delay);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCrossCheckStartDelay
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    :
 * OUTPUT   : *delay_p   - the start delay time
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCrossCheckStartDelay(
                                UI32_T *delay_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetLinkTraceCacheStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : status   - the linktrace cache status
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetLinkTraceCacheStatus(
                            CFM_TYPE_LinktraceStatus_T status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLinkTraceCacheStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : None
 * OUTPUT   : *status_p  - the link trace cache status
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T  CFM_OM_GetLinkTraceCacheStatus(
                            CFM_TYPE_LinktraceStatus_T *status_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetLinkTraceCacheSize
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : size     - the link trace cache size
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetLinkTraceCacheSize(
                                UI32_T size);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLinkTraceCacheSize
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : None
 * OUTPUT   : *size_p    - the link trace cache size
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetLinkTraceCacheSize(
                            UI32_T *size_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetLinkTraceCacheHoldTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : hold_time - the link trace cache hold time
 * OUTPUT   : None
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetLinkTraceCacheHoldTime(
                                UI32_T hold_time);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLinkTraceCacheHoldTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    :  None
 * OUTPUT   : *hold_time_p  - the link trace cache hold time pointer
 * RETUEN   : TRUE        - success
 *            FALSE       - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetLinkTraceCacheHoldTime(
                                    UI32_T *hold_time_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetCFMGlobalStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : status   - the CFM global status
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetCFMGlobalStatus(
                                CFM_TYPE_CfmStatus_T status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCFMGlobalStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    :
 * OUTPUT   : *status_p  - the CFM global status
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCFMGlobalStatus(
                                CFM_TYPE_CfmStatus_T *status_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetCFMPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : lport  - the logical port
 *            status - the CFM port status
 * OUTPUT   : None
 * RETUEN   : TRUE   - success
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetCFMPortStatus(
                            UI32_T lport,
                            CFM_TYPE_CfmStatus_T status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCFMPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : lport    - the logical port
 * OUTPUT   : *status_p  - the CFM port status
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCFMPortStatus(
                            UI32_T lport,
                            CFM_TYPE_CfmStatus_T *status_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetArchiveHoldTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index   - the md index
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetArchiveHoldTime(
                                UI32_T md_index,
                                UI32_T hold_time);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetArchiveHoldTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index   - the md index
 * OUTPUT   : *hold_time_p - the archive hold time
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetArchiveHoldTime(
                                UI32_T md_index,
                                UI32_T *hold_time_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMdLowPrDef
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the lowest priotiry on md
 * INPUT    : md_index   - the md index
 *            priority   - the lowest fault notify priority
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMdLowPrDef(
                            UI32_T md_index,
                            CFM_TYPE_FNG_LowestAlarmPri_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetFaultNotifyLowestPriority
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index - the md index
 * OUTPUT   : *priority_p - loweset priority
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetFaultNotifyLowestPriority(
                            UI32_T md_index,
                            CFM_TYPE_FNG_LowestAlarmPri_T *priority_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetFaultNotifyAlarmTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index   - the md index
 *            alarm_time - the fault notify alarm time
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetFaultNotifyAlarmTime(
                                    UI32_T md_index,
                                    UI32_T alarm_time);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetFaultNotifyAlarmTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index    - the md index
 * OUTPUT   : *alarm_time_p - the fault notify alarm time pointer in second
 * RETUEN   : TRUE        - success
 *            FALSE       - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetFaultNotifyAlarmTime(
                                UI32_T md_index,
                                UI32_T *alarm_time_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetFaultNotifyRestTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index   - the md index
 *            reset_time - the fault notify reset time in second
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetFaultNotifyRestTime(
                                UI32_T md_index,
                                UI32_T reset_time);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_ClearErrorsList
 * ------------------------------------------------------------------------
 * PURPOSE  : This function clear the error list
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_ClearErrorsList();

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_RemoveErrorByLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function remove the error form the error list accrodin to the domain level
 * INPUT    : level - the md level
 * OUTPUT   : None
 * RETUEN   : TRUE  - sucess
 *            FALSE - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_RemoveErrorByLevel(
                                CFM_TYPE_MdLevel_T level);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_RemoveErrorByAis
 * ------------------------------------------------------------------------
 * PURPOSE  : This function remove the error form the error list accrodin
 *              to the specified mep id, md name and ma name.
 * INPUT    : mep_id      - the mep id
 *            md_name_p   - pointer to md name
 *            md_name_len - md name length
 *            ma_name_p   - pointer to ma name
 *            ma_name_len - ma name length
 * OUTPUT   : None
 * RETUEN   : TRUE  - sucess
 *            FALSE - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_RemoveAisErrorByMepMa(
    UI32_T  mep_id,
    UI8_T   *md_name_p,
    UI32_T  md_name_len,
    UI8_T   *ma_name_p,
    UI32_T  ma_name_len);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_RemoveErrorByDomainName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function remove the error form the error list accrodin to the domain names
 * INPUT    : *name_ap  - the md name array pointer
 *            name_len  - the md name length
 * OUTPUT   : None
 * RETUEN   : TRUE      - sucess
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_RemoveErrorByDomainName(
                                UI8_T *name_ap,
                                UI32_T name_len);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_ClearLoopBackList
 * ------------------------------------------------------------------------
 * PURPOSE  : This function clear the loopbacklist
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_ClearLoopBackList();

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddLoopBack
 * ------------------------------------------------------------------------
 * PURPOSE  : This function add the loopback
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddLoopBack(UI32_T seq_num, UI32_T rcvd_time, CFM_OM_MEP_T *mep_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextLoopBackElement
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the next loop back
 * INPUT    :   seq_num - the next sequal number
 *                  **loop_back_pp.index - the loop back index
 * OUTPUT   : **loop_back_pp   - the next loop back pointer
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : if *error_pp is NULL, then return the fisrt element in list
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextLoopBackElement(
                            UI32_T   seq_num,
                            CFM_OM_LoopBackListElement_T **loop_back_pp);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetErrorListHeadPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get continuous check snmp trap status
 * INPUT    : trap         - the continuous check snmp trap type
 *            trap_enabled - the trap enable status
 * OUTPUT   : None
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetSNMPCcStatus(
                            CFM_TYPE_SnmpTrapsCC_T trap,
                            BOOL_T trap_enabled);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetSNMPCcStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the snmp continous check status
 * INPUT    : trap            - the cc trap type
 *            *trap_enabled_p - the trap enable status
 * OUTPUT   : None
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetSNMPCcStatus(
                            CFM_TYPE_SnmpTrapsCC_T trap,
                            BOOL_T *trap_enabled_p );

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetSNMPCrossCheckStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the snmp cross check status
 * INPUT    : trap   - the snmp cross check type
 *            tra_enable - the trap type enable status
 * OUTPUT   :
 * RETUEN   : TRUE     : success
 *            FALSE    : fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetSNMPCrossCheckStatus(
                                CFM_TYPE_SnmpTrapsCrossCheck_T trap,
                                BOOL_T trap_enabled);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetSNMPCrossCheckStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the snmp cross check trap enabled status
 * INPUT    : trap            - the snamp trape type
 *            *trap_enabled_p - the trap type enable status
 * OUTPUT   :
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetSNMPCrossCheckStatus(
                                    CFM_TYPE_SnmpTrapsCrossCheck_T trap,
                                    BOOL_T *trap_enabled_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the ma ais period
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *period_p - the ais period pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetAisPeriod(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T *period_p);

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the next ma ais period
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *period_p - the ais period pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextAisPeriod(
                            UI32_T md_index,
                            UI32_T *ma_index_p,
                            UI32_T *period_p);
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  :This function set the ais period
 * INPUT    : md_index  - the md index
 *                ma_index  - the ma index
 *                period      - the ais period
 * OUTPUT   :
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    :CFM_TYPE_AIS_PERIOD_1S, CFM_TYPE_AIS_PERIOD_60S
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetAisPeriod(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T period);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the ais level
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *ais_level_p - the ais level pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetAisLevel(
                        UI32_T md_index,
                        UI32_T ma_index,
                        CFM_TYPE_MdLevel_T *ais_level_p);

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the next ma ais level
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *ais_level_p - the ais level pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextAisLevel(
                            UI32_T md_index,
                            UI32_T *ma_index_p,
                            CFM_TYPE_MdLevel_T *ais_level_p);
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :This function se the ma ais level
 * INPUT    : md_index  - the md index
 *                ma_index  - the ma index
 *                ais_level - the ais level
 * OUTPUT   :
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetAisLevel(
                        UI32_T md_index,
                        UI32_T ma_index,
                        CFM_TYPE_MdLevel_T ais_level);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *ais_status_p - the ais status pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetAisStatus(
                        UI32_T md_index,
                        UI32_T ma_index,
                        CFM_TYPE_AIS_STATUS_T *ais_status_p);

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the next ma ais status
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *ais_status_p - the ais status pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextAisStatus(
                            UI32_T md_index,
                            UI32_T *ma_index_p,
                            CFM_TYPE_AIS_STATUS_T *ais_status_p);
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function set the ais status
 * INPUT    : md_index  - the md index
 *                ma_index  - the ma index
 *                ais_status - the ais status
 * OUTPUT   :
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetAisStatus(
                        UI32_T md_index,
                        UI32_T ma_index,
                        CFM_TYPE_AIS_STATUS_T ais_status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetAisSupressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the AIS suppress status
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *ais_supress_status_p - the ais suppress status pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetAisSupressStatus(
                            UI32_T md_index,
                            UI32_T ma_index,
                            CFM_TYPE_AIS_STATUS_T *ais_supress_status_p);

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextAisSuppressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the next ais suppress status
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *ais_supress_status_p - the ais suppress status pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextAisSuppressStatus(
                                UI32_T md_index,
                                UI32_T *ma_index_p,
                                CFM_TYPE_AIS_STATUS_T *ais_supress_status_p);
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetAisSuppressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function se the AIS suppress status
 * INPUT    : md_index              - the md index
 *                ma_index              - the ma index
 *                ais_supress_status - the ais suppress status
 * OUTPUT   :None
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetAisSuppressStatus(
                            UI32_T md_index,
                            UI32_T ma_index,
                            CFM_TYPE_AIS_STATUS_T ais_supress_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCfmGlobalCofigurationGlobalInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the cfm global configuration
 * INPUT    : None
 * OUTPUT   : *global_info_p
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCfmGlobalCofigurationGlobalInfo(
                            CFM_OM_GlobalConfigInfo_T *global_info_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCfmGlobalCofigurationTrapInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the trap configuration info
 * INPUT    : *trap_info  - the trap configuration info
 * OUTPUT   : trap_info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCfmGlobalCofigurationTrapInfo(
                        CFM_OM_GlobalConfigInfo_T *global_info_p);

/*------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetStackEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the mep information
 * INPUT    : stack_ifindex - the md index
 *            vid          - the ma index
 *            level        - the mep id
 *            stack_direction - the egress identifier
 *          *mep_info_p - the mep info pointer to put the mep info
 * OUTPUT   : *mep_info_p - the mep info
 * RETURN   : None
 * NOTE     :  This function only for MIB
 *            because a MEP Only can be configured one direction, so it needn't take care direction
 *            move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetStackEntry(
                            UI32_T stack_lport,
                            UI16_T vid,
                            CFM_TYPE_MdLevel_T level,
                            CFM_TYPE_MP_Direction_T stack_direction,
                            CFM_OM_MepInfo_T *mep_info_p);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetNextStackEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep information
 * INPUT    : *stack_ifindex_p - the md index
 *            *vid_p          - the ma index
 *            *level_p     - the mep id
 *            *stack_direction_p - the egress identifier
 *          *mep_info_p - the mep info pointer to put the mep info
 * OUTPUT   : *stack_ifindex_p - the md index
 *            *vid_p          - the ma index
 *            *level_p     - the mep id
 *            *stack_direction_p - the egress identifier
 *          *mep_info_p - the mep info
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextStackEntry(
                            UI32_T *stack_lport_p,
                            UI16_T *vid_p,
                            CFM_TYPE_MdLevel_T *level_p,
                            CFM_TYPE_MP_Direction_T *stack_direction_p,
                            CFM_OM_MepInfo_T *return_mep_p);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetErrorInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the configure error entry
 * INPUT    : vid   - the vlan id
 *           lport - the ifindex
 *          *error_info_p - the error info pointer to put the error info
 * OUTPUT   : *error_info - the error info
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetErrorInfo(
                        UI16_T vid,
                        UI32_T lport,
                        CFM_OM_Error_T *error_info_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextErrorInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the Next error info
 * INPUT    : *vid_p        - the vlan id
 *            *lport_p      - the logical port
 *¡@¡@¡@¡@¡@   *error_info_p - the error_p info pointer
 * OUTPUT   : *error_info_p - the error element in list
 * RETURN   : TRUE     - success
 *            FALSE    - fail
 * NOTE     :move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextErrorInfo(
                        UI16_T *vid_p,
                        UI32_T *lport_p,
                        CFM_OM_Error_T *error_info_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextFngConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the fault notify configure
 * INPUT    : *md_index_p   - the md index pointer
 *            *ma_index_p   - the ma index pointer
 *            *mep_id_p     - the mep id pointer
 * OUTPUT   : *fng_config_p - the fng configure pointer
 *            *md_index_p   - the next md index pointer
 *            *ma_index_p   - the next ma index pointer
 *            *mep_id_p     - the nextx mep id pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextFngConfig(
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p,
                            UI32_T *mep_id_p,
                            CFM_OM_MepFngConfig_T *fng_config_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetFaultNotifyResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the fault notify rest time
 * INPUT    : md_index      - the md index
 * OUTPUT   : *reset_time_p - the fault reset time pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetFaultNotifyResetTime(
                            UI32_T md_index,
                            UI32_T  *reset_time_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaIndexByMaName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the md ma index by ma name
 * INPUT    : *ma_name_ap   - the ma name array pointer
 *            name_len      - the ma name length
 * OUTPUT   : *md_index_p   - the md index pointer
 *            *ma_index_p   - the ma index pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaIndexByMaName(
                            UI8_T *ma_name_ap,
                            UI32_T  name_len,
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdIndexByName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the md index by md name
 * INPUT    : *md_name_ap   - the md name array pointer
 *            name_len      - the md name length
 * OUTPUT   : *md_index_p   - the md index pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdIndexByName(
                            UI8_T *md_name_ap,
                            UI32_T name_len,
                            UI32_T *md_index_p );

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMdIndexByIndexAndLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next md index by md index and level
 * INPUT    : level         - the md level
 * OUTPUT   : *md_index_p   - the md index pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : the next md is not the same level as the input md index's level
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMdIndexByIndexAndLevel(
                            UI8_T level,
                            UI32_T *md_index_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextLinktraceReplyInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next link trace reply
 * INPUT    : *md_index_p        - the md index poiner
 *            *ma_index_p        - the ma index pointer
 *            *mep_id_p          - the mep id pointer
 * OUTPUT   : *md_index_p        - the next md index poiner
 *            *ma_index_p        - the next ma index pointer
 *            *mep_id_p          - the next mep id pointer
 *            *linktrace_reply_p - the linktrace raply pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextLinktraceReplyInfo(
                                UI32_T *md_index_p,
                                UI32_T *ma_index_p,
                                UI32_T *mep_id_p,
                                UI32_T *seq_num_p,
                                UI32_T *rcvd_order_p,
                                CFM_OM_LinktraceReply_T *linktrace_reply_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLinktraceReplyInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next link trace reply
 * INPUT    : md_index        - the md index
 *           ma_index        - the ma index
 *           mep_id          - the mep id
 *           seq_num         - the sequence number
 *           rcvd_order      - the recevd order
 * OUTPUT   : *linktrace_reply_p - the linktrace raply pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetLinktraceReplyInfo(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T seq_num,
                                UI32_T rcvd_order,
                                CFM_OM_LinktraceReply_T *linktrace_reply_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextLoopBackInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the Next loop back info
 * INPUT    : *loop_back_info_p.index  - the loop back info index
 * OUTPUT   : *loop_back_info_p        - the loop back info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextLoopBackInfo(
                                    CFM_OM_LoopbackInfo_T *loop_back_info_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepErrorCCMLastFailure
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get error ccm content
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *content_p   - the failure array
 *          *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepErrorCCMLastFailure(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T *content_p,
                            UI32_T *content_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMepErrorCCMLastFailure
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get next error ccm content
 * INPUT    : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p   - the failure array
 *            *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMepErrorCCMLastFailure(
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p,
                            UI32_T *mep_id_p,
                            UI8_T *content_p,
                            UI32_T *content_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepXconCcmLastFailure
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get xcon ccm content
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *content_p   - the failure array
 *          *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepXconCcmLastFailure(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T *content_p,
                            UI32_T *content_len_p);

#if 0 //not used at present
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepNextXconCcmLastFailure
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get next xcon  ccm content
 * INPUT    : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p   - the failure array
 *            *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepNextXconCcmLastFailure(
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p,
                            UI32_T *mep_id_p,
                            UI8_T *content_p,
                            UI32_T *content_len_p);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepTransmitLbmDataTlv
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get data tlv content
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *content_p   - the failure array
 *          *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepTransmitLbmDataTlv(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T *content_p,
                            UI32_T *content_len_p);

#if 0 //not used at present
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepNextTransmitLbmDataTlv
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get next data  tlv content
 * INPUT    : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p   - the failure array
 *            *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepNextTransmitLbmDataTlv(
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p,
                            UI32_T *mep_id_p,
                            UI8_T *content_p,
                            UI32_T *content_len_p);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_Getdot1agCfmMdTableNextIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next md index
 * INPUT    :
 * OUTPUT   : *next_available_md_index - the next available md index
 * RETURN   : None
 * NOTE     : 1.This function only for MIB
 *           2. if the input md index doesn't exist, then next md index is the input md index.
 *             or find the index which is not used
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_Getdot1agCfmMdTableNextIndex(
                            UI32_T *next_available_md_index);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function Get the Md info
 * INPUT    : md_inde     - the md index
 * OUTPUT   : return_md_p - the md pointer from mgr.
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdInfo(
                        UI32_T md_index,
                        CFM_OM_MdInfo_T *return_md_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function Get the next Md info
 * INPUT    : md_index_p  - the md index to get next md index
 * OUTPUT   : md_index_p  - the current md index
 *            return_md_p - the md pointer from mgr.
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMdInfo(
                        UI32_T *md_index_p,
                        CFM_OM_MdInfo_T *return_md_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMaInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : this function get the ma info
 * INPUT    : md_index     - the md index
 *            ma_index     - the ma index
 * OUTPUT   : *return_ma_p - the return ma pointer from mgr
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMaInfo(
                    UI32_T md_index,
                    UI32_T ma_index,
                    CFM_OM_MaInfo_T *return_ma_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMaInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next Ma info
 * INPUT    : md_index      - the md index
 *            *ma_index      - the ma index
 * OUTPUT   : *ma_index the current ma index
 *          *return_ma_p  - the return ma info pointer from mgr
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMaInfo(
                    UI32_T md_index,
                    UI32_T *ma_index_p,
                    CFM_OM_MaInfo_T *return_ma_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep info
 * INPUT    : md_index     - the md index
 *            ma_index     - the ma index
 *            mep_id       - the mep id
 * OUTPUT   : return_mep_p - the mep info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if lport != 0, it means you want to get the mep specify on this port
 *            so, when get next mep is not on this port, it will return false
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepInfo(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T mep_id,
                    CFM_OM_MepInfo_T *return_mep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep info
 * INPUT    : *md_index_p    - the md index poiner
 *            *ma_index_p    - the ma index pointer
 *            *mep_id_p      - the mep id pointer
 * OUTPUT   : *md_index_p    - the next md index poiner
 *            *ma_index_p    - the next ma index pointer
 *            *mep_id_p      - the next mep id pointer
 *            *return_mep_p  - the mep info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : 1. if lport != 0, it means you want to get the mep specify on this port
 *               so, when get next mep is not on this port, it will return false
 *
 *            2. for CLI/WEB, mep with lport == 0 "CAN NOT BE" got from this api.
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMepInfo(
                        UI32_T *md_index_p,
                        UI32_T *ma_index_p,
                        UI32_T *mep_id_p,
                        UI32_T lport,
                        CFM_OM_MepInfo_T *return_mep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetDot1agNextMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep info
 * INPUT    : *md_index_p    - the md index poiner
 *            *ma_index_p    - the ma index pointer
 *            *mep_id_p      - the mep id pointer
 * OUTPUT   : *md_index_p    - the next md index poiner
 *            *ma_index_p    - the next ma index pointer
 *            *mep_id_p      - the next mep id pointer
 *            *return_mep_p  - the mep info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : 1. if lport != 0, it means you want to get the mep specify on this port
 *               so, when get next mep is not on this port, it will return false
 *
 *            2. for SNMP, mep with lport == 0 "CAN BE" got from this api.
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetDot1agNextMepInfo(
                        UI32_T *md_index_p,
                        UI32_T *ma_index_p,
                        UI32_T *mep_id_p,
                        UI32_T lport,
                        CFM_OM_MepInfo_T *return_mep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMipInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep info
 * INPUT    : *md_index_p    - the md index poiner
 *            *ma_index_p    - the ma index pointer
 *            *lport               - the logical port
 * OUTPUT   : *md_index_p    - the next md index poiner
 *            *ma_index_p    - the next ma index pointer
 *            *lport               - the logical port
 *            *return_mip_p  - the mep info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if lport != 0, it means you want to get the mep specify on this port
 *           so, when get next mep is not on this port, it will return false
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMipInfo(
                    UI32_T *md_index_p,
                    UI32_T *ma_index_p,
                    UI32_T *lport_p,
                    CFM_OM_MipInfo_T *return_mip_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRemoteMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next cross check remote mep info
 * INPUT    : *md_index_p    - the md index poiner
 *            *ma_index_p    - the ma index pointer
 *            *mep_id_p      - the mep id pointer
 *            current_time  - current_time
 * OUTPUT   : *md_index_p    - the next md index poiner
 *            *ma_index_p    - the next ma index pointer
 *            *mep_id_p      - the next mep id pointer
 *            *remote_mep_p  - the remote mep info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextRemoteMepInfo(
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p,
                            UI32_T *mep_id_p,
                            UI32_T current_time,
                            CFM_OM_RemoteMepCrossCheck_T *return_rmep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRemoteMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the cross check remote mep info
 * INPUT    : md_index    - the md index
 *           ma_index    - the ma index
 *           remote_mep_id     - the remtoe mep id
 *           current_tiem      - current time
 * OUTPUT   : *remote_mep_p  - the remote mep info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetRemoteMepInfo(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T remote_mep_id,
                            UI32_T current_time,
                            CFM_OM_RemoteMepCrossCheck_T *return_rmep_p);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetMepDbTable
 *-------------------------------------------------------------------------
 * PURPOSE  :This function get mep info
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            remote_mep_id - the egress identifier
 * OUTPUT   : *return_rmep_p - the remtoe mep info
 * RETURN   : None
 * NOTE     :move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepDbTable(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T remote_mep_id,
                        UI32_T current_time,
                        CFM_OM_RemoteMepCrossCheck_T *return_rmep_p);

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetNextMepDbTable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get next mep info
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            current_time - the current time
 *            remote_mep_id_p - the remote mep identifier
 * OUTPUT   : *md_index_p  - the next md index
 *                  *ma_index_p  - the next ma index
 *                  *mep_id_p      - the next mep id
 *                  *remote_mep_id_p - the next remote mep id
 *                  *return_rmep_p - the remtoe mep info
 * RETURN   : None
 * NOTE     :move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMepDbTable(
                        UI32_T *md_index_p,
                        UI32_T *ma_index_p,
                        UI32_T *mep_id_p,
                        UI32_T *remote_mep_id_p,
                        UI32_T current_time,
                        CFM_OM_RemoteMepCrossCheck_T *return_rmep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetDelayMeasureResult
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the delay measure result.
 * INPUT    : None
 * OUTPUT   : avg_fd_ms_p  - pointer to content of avarge frame delay in ms
 *            min_fd_ms_p  - pointer to content of minimum frame delay in ms
 *            max_fd_ms_p  - pointer to content of maximum frame delay in ms
 *            avg_fdv_ms_p - pointer to content of
 *                             avarage frame delay variation in ms
 *            succ_cnt_p   - pointer to content of received dmr's count
 *            total_cnt_p  - pointer to content of sent dmm's count
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetDelayMeasureResult(
                    UI32_T  *avg_fd_ms_p,
                    UI32_T  *min_fd_ms_p,
                    UI32_T  *max_fd_ms_p,
                    UI32_T  *avg_fdv_ms_p,
                    UI32_T  *succ_cnt_p,
                    UI32_T  *total_cnt_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextDmmReplyRec
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the next reply record of dmm.
 * INPUT    : recv_idx_p- pointer to content of record idx to get
 *                        (0 - to get first)
 * OUTPUT   : recv_idx_p- pointer to content of next record idx
 *            send_idx_p- pointer to content of send idx
 *            fd_ms_p   - pointer to content of frame delay in ms
 *            fdv_ms_p  - pointer to content of frame delay variation in ms
 *            res_p     - pointer to content of this record'result
 *                        CFM_TYPE_DMM_REPLY_REC_STATE_TIMEOUT  - dmr not received.
 *                        CFM_TYPE_DMM_REPLY_REC_STATE_RECEIVED - dmr     received
 *            is_succ_p - pointer to content of operation flag
 *                        TRUE  - this record is retrieved successfully
 *                        FALSE - try to get again later
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextDmmReplyRec(
                        UI32_T  *recv_idx_p,
                        UI32_T  *send_idx_p,
                        UI32_T  *fd_ms_p,
                        UI32_T  *fdv_ms_p,
                        UI8_T   *res_p,
                        BOOL_T  *is_succ_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetThrpMeasureResult
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the throughput measure result.
 * INPUT    : None
 * OUTPUT   : real_send_p - the count of packets sent in one sec
 *            rcvd_1sec_p - the count of packets received in one sec
 *            rcvd_total_p- the count of packets received before timeout
 *            res_bmp_p   - the bitmap to indicate which data is retrieved
 *                          CFM_TYPE_THRPT_MEASURE_RES_SENDPKTS,
 *                          CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_1SEC,
 *                          CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_TOUT
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetThrpMeasureResult(
                    UI32_T  *real_send_p,
                    UI32_T  *rcvd_1sec_p,
                    UI32_T  *rcvd_total_p,
                    UI8_T   *res_bmp_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddError
 *-------------------------------------------------------------------------
 * PURPOSE  : This function add  the error to list
 * INPUT    : error_time - the error start time
 *            reason     - the error type
 *            mep_p      - the mep pointer
 *            rem_mac_p  - pointer to remote mac address to record
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddError(
                    UI32_T error_time,
                    CFM_TYPE_MEP_ConfigError_T reason,
                    CFM_OM_MEP_T *mep_p,
                    UI8_T *rem_mac_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCurrentMaxErrPri
 *-------------------------------------------------------------------------
 * PURPOSE  : To get current highest defect priority of a MEP.
 * INPUT    : mep_p         - pointer to MEP to get
 *            is_skip_rdi   - TRUE to ignore the RDI defect
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_FNG_HighestDefectPri_T
 * NOTE     : will check mep->lowest_priority_defect
 *-------------------------------------------------------------------------
 */
CFM_TYPE_FNG_HighestDefectPri_T
CFM_OM_GetCurrentMaxErrPri(
    CFM_OM_MEP_T    *mep_p,
    BOOL_T          is_skip_rdi);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningCfmGlobalStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :This function get running cfm global status configuration
 * INPUT    : None
 * OUTPUT   : *cfmStatus_p - the CFM status pionter
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm enable
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningCfmGlobalStatus(
                                    CFM_TYPE_CfmStatus_T *cfmStatus_p );

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningCfmPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running cfm port  status configuration
 * INPUT    : lport              - the logical port
 * OUTPUT   : *cfm_port_Status_p - the cfm status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm port-enable
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningCfmPortStatus(
                              UI32_T lport,
                              CFM_TYPE_CfmStatus_T *cfm_port_Status_p );

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningCrossCheckStartDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running global cross check start delayconfiguration
 * INPUT    : None
 * OUTPUT   :*delay_p - the cross check start delay pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm mep crosscheck start-delay delay
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningCrossCheckStartDelay(
                                        UI32_T *delay_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningLinktraceHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running linktrace hold time configuration
 * INPUT    : None
 * OUTPUT   : *hold_time_p - the linnk trace reply entries hold time pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm linktrace cache hold-time minutes
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningLinktraceHoldTime(
                                        UI32_T *hold_time_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningLinktraceCacheSize
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running linktrace cache size configuration
 * INPUT    : None
 * OUTPUT   : size_p - the link trace cache size pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm linktrace cache size entries
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningLinktraceCacheSize(
                                        UI32_T *size_p);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningLinktraceCacheStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running linktrace cache status configuration
 * INPUT    : None
 * OUTPUT   : status_p - the link trace cache status pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm linktrace cache
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningLinktraceCacheStatus(
                                CFM_TYPE_LinktraceStatus_T *status_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningSnmpCcTrap
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running linktrace hold time configuration
 * INPUT    : None
 * OUTPUT   : *changed_trap_p - the byte bitmap store the present the traps type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : enable_traps
 *           |76543210|
 *            bit 7 : ALL, bit 6: MEP_UP, bit 5:MEP_DOWN,bit 4:CONFIG,bit 3:LOOP
 *  snmp-server enable traps ethernet cfm cc [mep-up|mep-down|config|loop]
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningSnmpCcTrap(
                        UI8_T *changed_trap_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningSnmpCrossCheckTrap
 *-------------------------------------------------------------------------
 * PURPOSE  :This function get running linktrace hold time configuration
 * INPUT    : None
 * OUTPUT   : *enabledt_traps_p - the byte bitmap store the present the traps type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : enable_traps
 *           |76543210|
 *            bit 7 : ALL, bit 6: MA_UP, bit 5:MEP_MISSING,bit 4:MEP_UNKNOWN,bit
 *  snmp-server enable traps ethernet cfm crosscheck [mep-unknown | mep-missing | MA-up]
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningSnmpCrossCheckTrap(
                                 UI8_T *changed_trap_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningFaultNotifyLowestPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running fault notify lowest priority configuration
 * INPUT    : None
 * OUTPUT   : priority_p - the fault notify lowest priority pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : mep fault-notify  lowest-priority priority
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningFaultNotifyLowestPriority(
                                UI32_T md_index,
                                CFM_TYPE_FNG_LowestAlarmPri_T *priority_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningCCmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running ccm status configuration
 * INPUT    : md_index - the md index
 * OUTPUT   : *ma_index - the ma index
 *          ma_name_p - the ma name
 *          *status_p - the CCM transmit status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm cc enable ma ma-name
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningCCmStatus(
                                  UI32_T md_index,
                                  UI32_T *ma_index,
                                  UI8_T ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
                                  CFM_TYPE_CcmStatus_T *status_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetnNextRunningCCmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running ccm interval  configuration
 * INPUT    : ma_index - the md index
 * OUTPUT   : *ma_index - the ma index
 *          ma_name_p-the ma name
 *          *interval_p - the cc interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm cc ma ma-name interval interval-level
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningCCmInterval(
                                    UI32_T md_index,
                                    UI32_T *ma_index_p,
                                    UI8_T ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
                                    CFM_TYPE_CcmInterval_T *interval_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRunningMd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running md configuration
 * INPUT    : *md_index_p - the md index pointer
 * OUTPUT   : *md_info_p - the md next info  pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :  ethernet cfm domain index index name domain-name level level-id
 *             mep archive-hold-time minutes
 *             mep fault-notify  lowest-priority priority
 *             mep fault-notify  alarm-time time
 *             mep fault-notify  reset-time timere
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningMd(
                           UI32_T *md_index_p,
                           CFM_OM_MdInfo_T *md_info_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRunningMa
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running md configuration
 * INPUT    : md_index    - the md index
 *            *ma_index   - the currnet ma index
 * OUTPUT   : * ma_index     - the next ma index
 *          *ma_info_p       - the next ma info
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :ma index index name name vlan vid-list
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningMa(
                        UI32_T md_index,
                        UI32_T *ma_index,
                        CFM_OM_MaInfo_T *ma_info_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRunnningMep
 *-------------------------------------------------------------------------
 * PURPOSE  :This function get running md configuration
 * INPUT    : *md_index - the md index pointer
 *            *ma_index - the ma index pointer
 *            lport     - the logical port*
 * OUTPUT   : *md_index - the next md index pointer
 *            *ma_index - the next ma index pointer
 *            *ret_mep_p    - the mep info
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :ethernet cfm mep mpid mpid ma ma-name [up]
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunnningMep(
                             UI32_T *md_index_p,
                             UI32_T *ma_index_p,
                             UI32_T lport,
                             CFM_OM_MepInfo_T *ret_mep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_GetRunningNextCrossCheckMpId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running md configuration
 * INPUT    : md_index     - the md index
 *            *ma_index_p   - the ma index pointer
 *            *remtoe_mep_p - the remote mep identifier pointer
 *            *ma_name_a    - the ma name array
 *            current_time  - current time
 * OUTPUT   : *ma_index_p   - the ma next index pointer
 *            *remtoe_mep_p - the remote mep next identifier pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :  mep crosscheck mpid id ma ma-name
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningCrossCheckMpIdInfo(
                                    UI32_T md_index,
                                    UI32_T *ma_index_p,
                                    UI32_T *remote_mep_id,
                                    UI32_T current_time,
                                    UI8_T ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1]);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the ma ais period running configuration
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :*period_p - the ais period pointer
 * RETUEN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS            - have change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL           - fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningAisPeriod(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T *period_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the running config ais level
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :*ais_level_p - the ais level pointer
 * RETUEN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS            - have change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL           - fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningAisLevel(
                            UI32_T md_index,
                            UI32_T ma_index,
                            CFM_TYPE_MdLevel_T *ais_level_p);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningAisSuppressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the running configure
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :*ais_supress_status_p - the ais suppress status pointer
 * RETUEN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS            - have change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL           - fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningAisSuppressStatus(
                                UI32_T md_index,
                                UI32_T ma_index,
                                CFM_TYPE_AIS_STATUS_T *ais_supress_status_p);
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the ais running status
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :*ais_status_p - the ais status pointer
 * RETUEN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS            - have change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL           - fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningAisStatus(
                            UI32_T md_index,
                            UI32_T ma_index,
                            CFM_TYPE_AIS_STATUS_T *ais_status_p);

/* for delay measurement
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetDmmCtrlRecPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : To get pointer to global dmm control record.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : pointer to global dmm control record
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_DmmCtrlRec_T *CFM_OM_GetDmmCtrlRecPtr(void);

/* for throughput measurement
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLbmCtrlRecPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : To get pointer to global lbm control record.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : pointer to global lbm control record
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_LbmCtrlRec_T *CFM_OM_GetLbmCtrlRecPtr(void);

#endif /*#if (SYS_CPNT_CFM == TRUE)*/

#endif /* _CFM_OM_H */

