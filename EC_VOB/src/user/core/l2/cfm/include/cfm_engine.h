/*-----------------------------------------------------------------------------
 * Module Name: cfm_engine.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the CFM utility API
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/15/2006 - Macualey Cheng  , Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */
#ifndef _CFM_ENGINE_H
#define _CFM_ENGINE_H

#include "cfm_om.h"
#include "cfm_type.h"
#include "cfm_mgr.h"
#if (SYS_CPNT_CFM == TRUE)
/*
 *********************************************
 * Define the CFMDU structure
 *********************************************
 */
typedef struct CFM_ENGINE_PduHead_S /*21.4*/
{
    UI32_T  pkt_dbg_flag;
    /* carried in common header*/
    UI8_T   md_level;
    UI8_T   version;
    UI16_T  op_code;
    UI8_T   flags;
    UI8_T   first_tlv_offset;

    /*carried in msg*/
    UI16_T  tag_info;
    UI32_T  lport;
    UI32_T  pdu_length;
    UI8_T   src_mac_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   dst_mac_a[SYS_ADPT_MAC_ADDR_LEN];
    BOOL_T  is_ing_lport_fwd;
}CFM_ENGINE_PduHead_T;


typedef struct CFM_ENGINE_SenderTlv_S
{
    UI8_T *type_p;            /*1 byte*/
    UI8_T *length_p;          /*2 byte*/
    UI8_T *chassis_id_length_p; /*1 byte*/
    UI8_T *chassis_id_sub_type_p;/*1 byte*/
    UI8_T *chassis_id_p;
    UI8_T *mgmt_addr_domain_len_p;
    UI8_T *mgmt_addr_domain_p;
    UI8_T *mgmt_addr_len_p;
    UI8_T *mgmt_addr_p;
}CFM_ENGINE_SenderTlv_T;

typedef struct CFM_ENGINE_CommonTlv_S
{
    UI8_T *type_p;             /*1 byte*/
    UI8_T *length_p;           /*2 byte*/
    UI8_T *value_p;

}CFM_ENGINE_CommonTlv_T;

typedef struct CFM_ENGINE_OrganizationTlv_S
{
    UI8_T *type_p;     /*1 byte*/
    UI8_T *length_p;   /*2 byte*/
    UI8_T *OUI_p;      /*3 byte*/
    UI8_T *Subtype_p;  /*1 byte*/
    UI8_T *Value_p;
}CFM_ENGINE_OrganizationTlv_T;

typedef struct CFM_ENGINE_CcmPdu_S
{
    CFM_ENGINE_PduHead_T *header_p;

    UI8_T *seq_num_p;    /*4 bytes*/
    UI8_T *mep_id_p;     /*2 bytes*/

    /*maid*/
    UI8_T *md_format_p; /*1 byte*/
    UI8_T *md_length_p; /*1 byte*/
    UI8_T *md_name_p;
    UI8_T *ma_format_p; /*1 byte*/
    UI8_T *ma_length_p; /*1 byte*/
    UI8_T *ma_name_p;

    /*tlv*/
    CFM_ENGINE_SenderTlv_T sender_tlv;
    /*port status*/
    UI8_T *port_status_type_p;       /*1 byte*/
    UI8_T *port_status_length_p;     /*2 byte*/
    UI8_T *port_status_value_p;      /*1 byte*/

    /*interface status*/
    UI8_T *interface_status_type_p;  /*1 byte*/
    UI8_T *interface_status_length_p;/*2 byte*/
    UI8_T *interface_status_value_p; /*1 byte*/

    /*data*/
    UI8_T *dat_type_p;             /*1 byte*/
    UI8_T *data_length_p;           /*2 byte*/
    UI8_T *data_value_p;

    /*oui*/
	CFM_ENGINE_OrganizationTlv_T org_tlv;
}CFM_ENGINE_CcmPdu_T;

typedef struct CFM_ENGINE_LtmPdu_S
{
    CFM_ENGINE_PduHead_T *header_p;

    UI8_T *trans_id_p;
    UI8_T *ttl_p;
    UI8_T *original_mac_p;
    UI8_T *target_mac_p;

    /*egress id tlv*/
    UI8_T *egress_type_p;      /*1 byte*/
    UI8_T *egress_length_p;    /*2 byte*/
    UI8_T *egress_id_p;        /*8 byte*/

    /*sender tlv*/
    CFM_ENGINE_SenderTlv_T sender_tlv;

    /*oui*/
	CFM_ENGINE_OrganizationTlv_T org_tlv;

    /*unknown tlv*/
    UI8_T unknown_tlv_a[CFM_TYPE_MAX_UNKNOWN_TLV_SIZE];
    UI32_T unknown_tlv_length;
}CFM_ENGINE_LtmPdu_T;

typedef struct CFM_ENGINE_LtrPdu_S
{
    CFM_ENGINE_PduHead_T *header_p;

    UI8_T *trans_id_p;             /*4 byte*/
    UI8_T *reply_ttl_p;            /*1 byte*/
    UI8_T *relay_action_p;         /*1 byte*/

    /*egress id tlv*/
    UI8_T *egress_type_p;          /*1 byte*/
    UI8_T *egress_length_p;        /*2 byte*/
    UI8_T *last_eegress_id_p;       /*8 byte*/
    UI8_T *next_eegress_id_p;       /*8 byte*/

    /*reply ingress tlv*/
    UI8_T *reply_ingress_type_p;    /*1 byte*/
    UI8_T *reply_ingress_length_p;  /*2 byte*/
    UI8_T *reply_ingress_action_p;  /*1 byte*/
    UI8_T *reply_ingress_mac_p;     /*6 byte*/
    UI8_T *reply_ingress_port_id_length_p;  /*1 byte*/
    UI8_T *reply_ingress_port_id_subtype; /*1 byte*/
    UI8_T *reply_ingress_port_id;

    /*reply egress tlv*/
    UI8_T *reply_egress_type;            /*1 byte*/
    UI8_T *reply_egress_length_p;          /*2 byte*/
    UI8_T *reply_egress_action_p;          /*1 byte*/
    UI8_T *reply_egress_mac_p;             /*6 byte*/
    UI8_T *reply_egress_port_id_length_p;    /*1 byte*/
    UI8_T *reply_egress_port_id_subtype;   /*1 byte*/
    UI8_T *reply_egress_port_id;

    /*sender tlv*/
    CFM_ENGINE_SenderTlv_T sender_tlv;

    /*oui*/
    CFM_ENGINE_OrganizationTlv_T org_tlv;

}CFM_ENGINE_LtrPdu_T;

/*Lbm and lbr pdu format*/
typedef struct CFM_ENGINE_LbPdu_S
{
    CFM_ENGINE_PduHead_T *header_p;

    UI8_T *trans_id_p;

    /*sender tlv*/
    CFM_ENGINE_SenderTlv_T sender_tlv;

    /*data_p tlv*/
    UI8_T *data_type_p;                 /*1 byte*/
    UI8_T *data_length_p;               /*2 byte*/
    UI8_T *data_p;

    /*oui*/

    CFM_ENGINE_OrganizationTlv_T org_tlv;

}CFM_ENGINE_LbPdu_T;

/* DMM and DMR pdu format*/
typedef struct CFM_ENGINE_DmPdu_S
{
    CFM_ENGINE_PduHead_T    *header_p;
    CFM_TYPE_TimeReprFmt_T  *tx_time_stampf_p;   /* 8 bytes */
    CFM_TYPE_TimeReprFmt_T  *rx_time_stampf_p;   /* 8 bytes */
    CFM_TYPE_TimeReprFmt_T  *tx_time_stampb_p;   /* 8 bytes */
    CFM_TYPE_TimeReprFmt_T  *rx_time_stampb_p;   /* 8 bytes */
}CFM_ENGINE_DmPdu_T;

/*AIS format*/
typedef struct CFM_ENGINE_AisPdu_S
{
    CFM_ENGINE_PduHead_T *header_p;
}CFM_ENGINE_AisPdu_T;


typedef struct CFM_ENGINE_PortStatusCallback_S
{
    BOOL_T admin_disable;
    BOOL_T xstp_forwarding;

}CFM_ENGINE_PortStatusCallback_T;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_Init
 *-------------------------------------------------------------------------
 * PURPOSE  : Intial the system
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function will set each port trap the cfm pdu and port mac
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_Init();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 * NOTE     : This function will creat the timer which need run when systerm up
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_EnterMasterMode();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_EnterTransitionMode();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the recevied CFMDU.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 * NOTE     : this is call from the cfm_mgr.c
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessRcvdPDU(CFM_TYPE_Msg_T *msg);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CcmWait_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function do the standard's CCM wait state
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_CcmWait_Callback(
                                   void *timer_para_p);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CCMTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the CCM time in second
 *            according to the input interval level
 * INPUT    : Interval - the CCM interval
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CFM_ENGINE_CCMTime(CFM_TYPE_CcmInterval_T interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ErrorCcmDefect_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : The error error ccm defect state timer time out
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ErrorCcmDefect_Callback(
                                         void *timer_para_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XconDefect_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : The funciton do the xcon defect timer end
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_XconDefect_Callback(
                                      void *timer_para_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_RemoteMepOk_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : The function decide go to which remote mep state machine state
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_RemoteMepOk_Callback(
                                                    void *timer_para_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_RemoteStartDelay_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : The function decide go to which remote mep state machine state when
 *            start delay timer out
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_RemoteStartDelay_Callback(
                                           void *timer_para_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FngDefectState_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : process the fngAlarmtime timeout
 * INPUT    : *timer_para_p    - the parameter pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_FngDefectState_Callback(
                                               void *timer_para_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FngDefectClearingState_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : process the fngResettime t timeout
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_FngDefectClearingState_Callback(
                                                 void *timer_para_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will transmit the LBM packet
 * INPUT    : md_name_ap      - the md name array pointer
 *            md_name_len     - the md name length
 *            ma_name_ap      - the ma name array pointer
 *            ma_name_len     - the ma name length
 *            dst_mep_id      - the dstnation mep id
 *            dst_mac         - the destination mac address
 *            trasnmit_mep_id - the mep id which will transmit the lbm
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : 1. if transmit_mep_id not 0, the it will send lbm through this mep id,
 *               if transmit_mep_id is 0, the it will look the mep to transmit mep by dst mep id and dst mac
 *            2. You can choose use the dst mep id or dst mac. To use th dst mac
 *               set the mep id 0
 *            3. this api may not be used when
 *               CFM_MGR_TransmitLoopback & CFM_MGR_SetDot1agCfmMepTransmitLbmStatus
 *               are changed to use CFM_ENGINE_AddBgXmitLBM.
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_XmitLBM(
    UI8_T       *md_name_ap,
    UI32_T      md_name_len,
    UI8_T       *ma_name_ap,
    UI32_T      ma_name_len,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T      transmit_mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearLoopBackList
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will clear the loop back list
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ClearLoopBackList();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextLoopBackInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the Next loop back info
 * INPUT    : *loop_back_info_p.index  - the loop back info index
 * OUTPUT   : *loop_back_info_p   - the loop back info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextLoopBackInfo(CFM_OM_LoopbackInfo_T *loop_back_info_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitLTM
 *-------------------------------------------------------------------------
 * PURPOSE  : This function transmit the LTM
 * INPUT    : md_name_ap      - the md name array pointer
 *            md_name_len     - the md name length
 *            ma_name_ap      - the ma name array pointer
 *            ma_name_len     - the ma name length
 *            dst_mep_id      - the destination mep id
 *            dest_mac        - the destination mac (can not be null)
 *            transmit_mep_id - specify the mep to transmit ltm
 *            ttl             - the transmit time to live
 *            pkt_pri         - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_XmitLTM(
    UI8_T   *md_name_ap,
    UI32_T  md_name_len,
    UI8_T   *ma_name_ap,
    UI32_T  ma_name_len,
    UI32_T  dst_mep_id,
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T  transmit_mep_id,
    UI8_T   ttl,
    UI16_T  pkt_pri);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearPendingLTRs_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the pending LTR in queue
 * INPUT    : *queue_head_p - the pending LTR queue
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearPendingLTRs_CallBack(void *queue_head_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMD
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the MD, create or modify
 * INPUT    : md_index  - the md index
 *            *name_ap  - the md name
 *            name_len  - the md name length
 *            level     - the md level
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMD(UI32_T md_index, UI8_T *name_ap, UI16_T name_len, CFM_TYPE_MdLevel_T level, CFM_TYPE_MhfCreation_T create_way);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function Get the Md info
 * INPUT    : md_inde     - the md index
 * OUTPUT   : return_md_p - the md pointer from mgr.
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetMdInfo(UI32_T md_index,  CFM_OM_MdInfo_T *return_md_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextMdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function Get the next Md info
 * INPUT    :*md_index     - the md index to get next md index
 * OUTPUT   :  *md_index   - the current md index
 *           *return_md_p - the md pointer from mgr.
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextMdInfo(UI32_T *md_index, CFM_OM_MdInfo_T *return_md_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DeleteMD
 *-------------------------------------------------------------------------
 * PURPOSE  : this function delete the md
 * INPUT    : md_index  - the md index
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_DeleteMD(UI32_T md_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CreateMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the empty ma, and this ma can't be used
 * INPUT    : ma_index   - the ma index
 *            md_index   - the md index
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :This function only will create ma with default value, and this ma can't be used.
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_CreateMA(UI32_T md_index, UI32_T ma_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the Ma
 * INPUT    : ma_index   - the ma index
 *            *name_ap   - the ma name pointer
 *            name_len   - the ma name length
 *            md_index   - the md index
 *            primary_vid  -the primary vid of the ma
 *            vid_num    - the vid number in vid list
 *            vlid_list  - the array store the vid list. one element store one vid
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMA(UI32_T ma_index, UI8_T *name_ap, UI32_T name_len, UI32_T md_index,
                         UI16_T primary_vid,UI32_T vid_num,UI8_T vid_list[SYS_ADPT_MAX_NBR_OF_VLAN], CFM_TYPE_MhfCreation_T create_way);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DeleteMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the Ma
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_DeleteMA(UI32_T md_index,UI32_T ma_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DeleteMAVlan
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the Ma vlan
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if this vid is primary vid and ma still exit mep, this vid can't be deleted
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_DeleteMAVlan(UI32_T md_index,UI32_T ma_index,UI16_T vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMAName
 *-------------------------------------------------------------------------
 * PURPOSE  : this function set the ma name
 * INPUT    : md_index     - the md index
 *            ma_index     - the ma index
 *            *name_ap      - the ma name array pointer
 *            name_length  - the ma name length
 * OUTPUT   :None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMaName(UI32_T md_index, UI32_T ma_index, UI8_T *name_ap, UI32_T name_length);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMaInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : this function get the ma info
 * INPUT    : md_index       - the md index
 *            ma_index       - the ma index
 * OUTPUT   : *return_ma_p   - the return ma pointer from mgr
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetMaInfo(UI32_T md_index,UI32_T ma_index, CFM_OM_MaInfo_T *return_ma_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextMaInfo
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
BOOL_T CFM_ENGINE_GetNextMaInfo(UI32_T md_index,UI32_T *ma_index, CFM_OM_MaInfo_T *return_ma_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CreateMEP
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the mep
 * INPUT    : lport        - the logical port
 *            mep_id       - the mep id
 *            md_name_ap   - the md naem array pointer
 *            md_name_len  - the md name length
 *            ma_name_ap   - the ma naem array pointer
 *            ma_name_len  - the ma name length
 *            direction    - the mep direction
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_CreateMEP(
    UI32_T                  lport,
    UI32_T                  mep_id,
    UI8_T                   *md_name_ap,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_ap,
    UI32_T                  ma_name_len,
    CFM_TYPE_MP_Direction_T direction);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DeleteMEP
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delte the MEP
 * INPUT    : lport       - the logical port
 *            mep_id      - the mep id
 *            md_name_ap  - the md naem array pointer
 *            md_name_len - the md name length
 *            ma_name_ap  - the ma naem array pointer
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_DeleteMEP(
    UI32_T  lport,
    UI32_T  mep_id,
    UI8_T   *md_name_ap,
    UI32_T  md_name_len,
    UI8_T   *ma_name_ap,
    UI32_T  ma_name_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DeleteMipByMa
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the MIP by ma
 * INPUT    : *md_p   - md pointer
 *                *ma_p   - the ma pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_DeleteMipByMa(
                        CFM_OM_MD_T *md_p,
                        CFM_OM_MA_T *ma_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CreateMIPByMa
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the MIP
 * INPUT    : checked_level  - up to the level need be checked
 *            checked_vid    - this vid need be checked
 *            checked_lport  - the lport will be checked
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_CreateMIPByMa(CFM_TYPE_MdLevel_T checked_level, UI16_T checked_vid, UI32_T checked_lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CreateMIPByMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the MIP
 * INPUT    : checked_level  - up to the level need be checked
 *            checked_vid    - this vid need be checked
 *            checked_lport  - the lport will be checked
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_CreateMIPByMep(CFM_TYPE_MdLevel_T checked_level, UI16_T checked_vid, UI32_T checked_lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CheckOverLevelMipShouldExist
 *-------------------------------------------------------------------------
 * PURPOSE  : This function check th over level mip shoud still need exit or not
 *          if needn't exit then remove the mip
 * INPUT    : checked_level  - up to the level need be checked
 *            checked_vid    - this vid need be checked
 *            checked_lport -  the checked logical port
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : find the next higher level, and just check the first next higher level
 *           1. if has mip exist then delete it
 *           2. if has no mip exist then return
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_CheckOverLevelMipShouldExist(CFM_TYPE_MdLevel_T checked_level, UI16_T checked_vid,UI32_T chekced_lport);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : count down the timer in timer list and process the time out timer
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : this is call from the cfm_mgr.c
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessTimer();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCFMGlobalStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the global CFM status
 * INPUT    : status - the cfm status
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetCFMGlobalStatus(CFM_TYPE_CfmStatus_T  status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetCFMGlobalStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the cfm global configuratoin
 * INPUT    : None
 * OUTPUT   : *status  - the cfm status pointer
 * RETURN   : TRUE     - success
 *            FALSE    - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetCFMGlobalStatus(CFM_TYPE_CfmStatus_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCFMPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the CFM port status
 * INPUT    : status - the cfm status
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetCFMPortStatus( UI32_T lport,CFM_TYPE_CfmStatus_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetCFMPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the port cfm status
 * INPUT    : None
 * OUTPUT   : *status- the cfm status pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetCFMPortStatus(UI32_T lport, CFM_TYPE_CfmStatus_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCcmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ccm interval
 * INPUT    : md_name_ap  - the md name array pointer
 *            md_name_len - md name length
 *            ma_name_ap  - the ma name array pointer
 *            ma_name_len - ma name length
 *            interval    - the ccm interval
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetCcmInterval(
    UI8_T                   *md_name_ap,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_ap,
    UI32_T                  ma_name_len,
    CFM_TYPE_CcmInterval_T  interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetCcmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the ccm interval
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : *interval   - the ma ccm interval pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetCcmInterval(UI32_T md_index,UI32_T ma_index,UI8_T *ma_name_ap, UI32_T name_len, CFM_TYPE_CcmInterval_T *interval);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextCcmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next ccm interval
 * INPUT    : md_index     - the md index
 *            *ma_index    - the ma index
 *            *ma_name_ap  - the ma name array pointer
 *            name_len     - the ma name length
 * OUTPUT   : *interval    - the ccm interval
 *            *ma_index    - the next ma index
 *            *ma_name_ap  - the name ma name array pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextCcmInterval(UI32_T md_index,UI32_T *ma_index,UI8_T *ma_name_ap, UI32_T name_len, CFM_TYPE_CcmInterval_T *interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCcmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ccm status
 * INPUT    : md_name_ap  - the md name array pointer
 *            md_name_len - the md name length
 *            ma_name_ap  - the ma name array pointer
 *            ma_name_len - the ma name length
 *            status      - the ccm status
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetCcmStatus(
    UI8_T                   *md_name_ap,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_ap,
    UI32_T                  ma_name_len,
    CFM_TYPE_CcmStatus_T    status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetCcmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the CCM status
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : *status     - the ccm status
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetCcmStatus(UI32_T md_index,UI32_T ma_index, UI8_T *ma_name_ap, UI32_T name_len, CFM_TYPE_CcmStatus_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextCcmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next ccm status
 * INPUT    : md_index    - the md index
 *            *ma_index   - the ma_index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : *status     - the ccm status
 *            *ma_index   - the next ma index
 *            *ma_name_ap - the next ma name array pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextCcmStatus(UI32_T md_index,UI32_T *ma_index, UI8_T *ma_name_ap, UI32_T name_len, CFM_TYPE_CcmStatus_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CreateRemoteMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function add the remote mpep
 * INPUT    : md_index - the md index
 *            *ma_name_ap  - the ma name array pointer
 *            name_len     - the ma name length
 *            mep_id       - the mep id
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_CreateRemoteMep(UI32_T md_index,UI8_T *ma_name_ap, UI32_T name_len, UI32_T mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DeleteRemoteMepByMdIndexMaName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the remtoe mep by md index and ma name
 * INPUT    : md_index     - the md index
 *            *ma_name_ap  - the ma name array pointer
 *            name_len     - the ma name length
 *            mep_id       - the remote mep id
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_DeleteRemoteMepByMdIndexMaName(UI32_T md_index,UI8_T *ma_name_ap, UI32_T name_len, UI32_T mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DeleteRemoteMepByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the remtoe mep by mep index
 * INPUT    : md_index       - the md index
 *            ma_index       - the ma index
 *            remote_mep_id  - the remote mep id
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_DeleteRemoteMepByIndex(UI32_T md_index,UI32_T ma_index,UI32_T remote_mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetCfmGlobalCofigurationGlobalInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the global configuration info
 * INPUT    : *global_info  - the global configuration info
 * OUTPUT   : global_info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetCfmGlobalCofigurationGlobalInfo(CFM_OM_GlobalConfigInfo_T *global_info);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetCfmGlobalCofigurationTrapInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the trap configuration info
 * INPUT    : *trap_info  - the trap configuration info
 * OUTPUT   : trap_info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetCfmGlobalCofigurationTrapInfo(CFM_OM_GlobalConfigInfo_T *trap_info);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCrossCheckStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the cross check status
 * INPUT    : status        - the cross check status
 *            md_name_ap - the md name array pointer
 *            md_len     - the md len
 *            ma_name_ap - the ma name array pointer
 *            ma_len     - the ma len
 *            ma_len        - the ma len
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :  CFM_TYPE_CROSS_CHECK_STATUS_DISABLE,
 *             CFM_TYPE_CROSS_CHECK_STATUS_ENABLE
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetCrossCheckStatus(
    UI8_T                           *md_name_ap,
    UI32_T                          md_name_len,
    UI8_T                           *ma_name_ap,
    UI32_T                          ma_name_len,
    CFM_TYPE_CrossCheckStatus_T     status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCrossCheckStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the cross check status
 * INPUT    : *ma_name_ap   - the ma name array pointer
 *            *ma_len       - the ma length
 * OUTPUT   : *status       - cross check status
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetCrossCheckStatus(CFM_TYPE_CrossCheckStatus_T *status,UI8_T *ma_name_ap, UI32_T ma_len);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCrossCheckStartDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the cross check start delay
 * INPUT    : delay  - the delay time
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetCrossCheckStartDelay(UI32_T delay);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetCrossCheckStartDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the cross Check start delay
 * INPUT    : None
 * OUTPUT   : *delay - the cross check start delay
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetCrossCheckStartDelay(UI32_T *delay);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetLinkTraceCacheStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the link trace cache status
 * INPUT    : status - the link trace cache status
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetLinkTraceCacheStatus(CFM_TYPE_LinktraceStatus_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetLinkTraceCacheStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the link trace cache status
 * INPUT    : None
 * OUTPUT   : *status- the link cache status
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetLinkTraceCacheStatus(CFM_TYPE_LinktraceStatus_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetLinkTraceCacheSize
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the link trace cache size
 * INPUT    : size   - the link trace cache size
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetLinkTraceCacheSize(UI32_T size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetLinkTraceCacheSize
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the link trace cache size
 * INPUT    : None
 * OUTPUT   : *size  - the link trace cache size
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetLinkTraceCacheSize(UI32_T *size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetLinkTraceCacheHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the link trace cache hold time
 * INPUT    : hold_tie   - the link trace cache hold time
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetLinkTraceCacheHoldTime(UI32_T hold_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetLinkTraceCacheHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the link trace cache hold time
 * INPUT    : None
 * OUTPUT   : *hold_time  - the link trace cache hold time pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetLinkTraceCacheHoldTime(UI32_T *hold_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearLinkTraceCache
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the link trace cache
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearLinkTraceCache();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearLinkTraceReplyEntry_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the link trace reply entry
 * INPUT    : *link_trace_reply_p  - the link trace entry's pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearLinkTraceReplyEntry_Callback(void *link_trace_reply_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetArchiveHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the archive hold time
 * INPUT    : md_index   - the md index
 *            hold_time  - the archive hold time
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetArchiveHoldTime(UI32_T md_index, UI32_T hold_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetArchiveHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the archive hold time
 * INPUT    : md_index   - the md index
 * OUTPUT   : *hold_tim  - the archive hold time
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetArchiveHoldTime(UI32_T md_index, UI32_T *hold_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetFaultNotifyLowestPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify lowest priority
 * INPUT    : md_index  - md index
 *            priority  - the fault notify lowest priority
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetFaultNotifyLowestPriority(UI32_T md_index, CFM_TYPE_FNG_LowestAlarmPri_T priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetFaultNotifyLowestPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This funciton get the fault notify lowest priority
 * INPUT    : md_index    - the md index
 * OUTPUT   : *prirority  - the lowest priority pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetFaultNotifyLowestPriority(UI32_T md_index, CFM_TYPE_FNG_LowestAlarmPri_T *priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetFaultNotifyAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault alarm time
 * INPUT    : md_index   - the md index
 *            alarm time - the fault alarm time
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetFaultNotifyAlarmTime(UI32_T md_index, UI32_T alarm_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetFaultNotifyAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This funcion get the fault notify alarm time
 * INPUT    : md_index    - the md index
 * OUTPUT   : *alarm_tim  - the fault alarm time poiner
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetFaultNotifyAlarmTime(UI32_T md_index, UI32_T *alarm_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetFaultNotifyResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : Tis function set the fault rest time
 * INPUT    : md_index    - the md index
 *            reset_time - the fault notify rest time
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetFaultNotifyResetTime(UI32_T md_index,UI32_T  reset_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetFaultNotifyResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the fault notify rest time
 * INPUT    : md_index    - the md index
 * OUTPUT   : *reset_time - the fault reset time pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetFaultNotifyResetTime(UI32_T md_index,UI32_T  *reset_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearRemoteMepByRemoteMep_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  :This function Clear the remote mep by remtoe mep depend on archive hold time
 * INPUT    : *mp  - the remote mep which will reset the data base
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearRemoteMepByRemoteMep_Callback(void *mp);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearRemoteMepByDomain
 *-------------------------------------------------------------------------
 * PURPOSE  :This function Clear the remote mep by domain name
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearRemoteMepAll();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearRemoteMepByDomain
 *-------------------------------------------------------------------------
 * PURPOSE  :This function Clear the remote mep by domain name
 * INPUT    : *md_name_ap  - the md name array pointer
 *            md_name_len  - the md name length
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearRemoteMepByDomain(UI8_T *md_name_ap, UI32_T md_name_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearRemoteMepByLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the remote mep by md level
 * INPUT    : level  - the md level
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearRemoteMepByLevel(CFM_TYPE_MdLevel_T level);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearErrorsListByMdNameOrLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the error list by md name or md level
 * INPUT    : *md_name_ap  - the md name array pointer
 *            name_len     - the md name length
 *            levle        - the md level
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearErrorsListByMdNameOrLevel(UI8_T *md_name_ap, UI32_T name_len,CFM_TYPE_MdLevel_T level);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearOnePortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : To clear one port's config including MEP/MIP.
 * INPUT    : lport - lport to clear
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : for CFM_MGR_HandleHotRemoval.
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ClearOnePortConfig(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextErrorInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the Next error info
 * INPUT    : *vid   - the vlan id
 *            *lport - the logical port
 *¡@¡@¡@¡@¡@*error_info_p   - the error_p info pointer
 * OUTPUT   : *error_info_p - the error element in list
 * RETURN   : TRUE     - success
 *            FALSE    - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextErrorInfo(UI16_T *vid, UI32_T *lport, CFM_OM_Error_T *error_info_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetErrorInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the configure error entry
 * INPUT    : vid   - the vlan id
 *           lport - the ifindex
 *          *error_info - the error info pointer to put the error info
 * OUTPUT   : *error_info - the error info
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetErrorInfo(UI16_T vid, UI32_T lport, CFM_OM_Error_T *error_info);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMdIndexByName
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
BOOL_T CFM_ENGINE_GetMdIndexByName(UI8_T *md_name_ap, UI32_T name_len, UI32_T *md_index_p );

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextMdIndexByIndexAndLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next md index by md index and level
 * INPUT    : level         - the md level
 * OUTPUT   : *md_index_p   - the md index pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : the next md is not the same level as the input md index's level
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextMdIndexByIndexAndLevel(UI8_T level, UI32_T *md_index_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextRemoteMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next cross check remote mep info
 * INPUT    : *md_index_p    - the md index poiner
 *            *ma_index_p    - the ma index pointer
 *            *mep_id_p      - the mep id pointer
 * OUTPUT   : *md_index_p    - the next md index poiner
 *            *ma_index_p    - the next ma index pointer
 *            *mep_id_p      - the next mep id pointer
 *            *remote_mep_p   - the remote mep info
 *
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextRemoteMepInfo(UI32_T *md_index_p, UI32_T *ma_index_p, UI32_T *mep_id_p, CFM_OM_RemoteMepCrossCheck_T *remote_mep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetRemoteMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the cross check remote mep info
 * INPUT    : md_index    - the md index
 *           ma_index    - the ma index
 *           mep_id     - the remtoe mep id
 * OUTPUT   : *remote_mep_p  - the remote mep info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetRemoteMepInfo(UI32_T md_index, UI32_T ma_index, UI32_T remote_mep_id, CFM_OM_RemoteMepCrossCheck_T *return_rmep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMepDbTable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get mep info
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            remote_mep_id - the egress identifier
 * OUTPUT   : *return_rmep_p - the remtoe mep info
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetMepDbTable(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T remote_mep_id, CFM_OM_RemoteMepCrossCheck_T *return_rmep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextMepDbTable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get next mep info
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            remote_mep_id - the egress identifier
 * OUTPUT   : *return_rmep_p - the remtoe mep info
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextMepDbTable(UI32_T *md_index_p, UI32_T *ma_index_p, UI32_T *mep_id_p, UI32_T *remote_mep_id_p,
                                                         CFM_OM_RemoteMepCrossCheck_T *return_rmep_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextLinktraceReplyInfo
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
BOOL_T CFM_ENGINE_GetNextLinktraceReplyInfo(UI32_T *md_index_p, UI32_T *ma_index_p, UI32_T *mep_id_p, UI32_T *seq_num_p, UI32_T *rcvd_order_p,CFM_OM_LinktraceReply_T *linktrace_reply_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetLinktraceReplyInfo
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
BOOL_T CFM_ENGINE_GetLinktraceReplyInfo(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T seq_num, UI32_T rcvd_order, CFM_OM_LinktraceReply_T *linktrace_reply_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep info
 * INPUT    : md_index    - the md index
 *           ma_index    - the ma index
 *           mep_id      - the mep id
 * OUTPUT   : *return_mep_p  - the mep info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if lport != 0, it means you want to get the mep specify on this port
 *           so, when get next mep is not on this port, it will return false
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetMepInfo(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, CFM_OM_MepInfo_T *return_mep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextMepInfo
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
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextMepInfo(UI32_T *md_index_p, UI32_T *ma_index_p, UI32_T *mep_id,UI32_T lport,CFM_OM_MepInfo_T *return_mep_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMepErrorCCMLastFailure
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
BOOL_T CFM_ENGINE_GetMepErrorCCMLastFailure(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T *content_p, UI32_T *content_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextMepErrorCCMLastFailure
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
BOOL_T CFM_ENGINE_GetNextMepErrorCCMLastFailure(UI32_T *md_index, UI32_T *ma_index, UI32_T *mep_id, UI8_T *content_p, UI32_T *content_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMepXconCcmLastFailure
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
BOOL_T CFM_ENGINE_GetMepXconCcmLastFailure(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T *content_p, UI32_T *content_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMepNextXconCcmLastFailure
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
BOOL_T CFM_ENGINE_GetMepNextXconCcmLastFailure(UI32_T *md_index, UI32_T *ma_index, UI32_T *mep_id, UI8_T *content_p, UI32_T *content_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMepTransmitLbmDataTlv
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
BOOL_T CFM_ENGINE_GetMepTransmitLbmDataTlv(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T *content_p, UI32_T *content_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMepNextTransmitLbmDataTlv
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
 *          *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetMepNextTransmitLbmDataTlv(UI32_T *md_index, UI32_T *ma_index, UI32_T *mep_id, UI8_T *content_p, UI32_T *content_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextMipInfo
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
BOOL_T CFM_ENGINE_GetNextMipInfo(UI32_T *md_index_p, UI32_T *ma_index_p,UI32_T *lport,CFM_OM_MipInfo_T *return_mip_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMdMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMdMhfCreation(UI32_T md_index, CFM_TYPE_MhfCreation_T create_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMdSendIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the sender id permission
 * INPUT    : md_index  - the md index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMdSendIdPermission(UI32_T md_index, CFM_TYPE_MhfIdPermission_T send_id_permission);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_Getdot1agCfmMdTableNextIndex
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
BOOL_T CFM_ENGINE_Getdot1agCfmMdTableNextIndex(UI32_T *next_available_md_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMaMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma mhf creation type
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            create_type -the mhf cration type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 * 	      CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMaMhfCreation(UI32_T md_index, UI32_T ma_index, CFM_TYPE_MhfCreation_T create_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMaMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           ma_index   - the ma index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMaMhfIdPermission(UI32_T md_index, UI32_T ma_index, CFM_TYPE_MhfIdPermission_T send_id_permission);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMaNumOfVids
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma can have more than on vid
 * INPUT    : md_index   - the md index
 *            ma_index   - the ma index
 *            vid_num    - can have more than one vid
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMaNumOfVids(UI32_T md_index, UI32_T ma_index, UI32_T vid_num);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepLport
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's logical port
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            lport   - the logical port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *            2. the mep can't be configured on trunk member
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepLport(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepDirection
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            direction  - the mep direction
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepDirection(UI32_T md_index, UI32_T ma_index, UI32_T mep_id,CFM_TYPE_MP_Direction_T direction);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepPrimaryVid
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepPrimaryVid(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI16_T primary_vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepActiveStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's active status
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            active    - the mep active status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepActiveStatus(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T active);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepCciStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            status    - the cci status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *            2. CFM_TYPE_CCI_STATUS_ENABLE
 *               CFM_TYPE_CCI_STATUS_DISABLE
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepCciStatus(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, CFM_TYPE_CcmStatus_T cci_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepCcmLtmPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ccm_ltm_priority- the ccm and ltm default priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepCcmLtmPriority(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ccm_ltm_priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetDot1agCfmMepFaultAlarmDestDomain
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's Fault alarm dest domain
 * INPUT    : md_index       - the md index
 *            ma_index       - the ma index
 *            mep_id         - the mep id
 *            *dest_domain_ap- destination domain array pointer
 *            doman_length   - destination domain length
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.This function only for MIB
 *            2. not support this function, it will just return false
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepFaultAlarmDestDomain(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T *dest_domain_ap, UI32_T doman_length);/*not support*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetDot1agCfmMepFaultAlarmDestAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fault alarm address
 * INPUT    : md_index        - the md index
 *            ma_index        - the ma index
 *            mep_id          - the mep id
 *            *dest_address_ap- the domain address array pointer
 *            address_length  - the address length
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.This function only for MIB
 *            2. not support this function, it will just return false
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepFaultAlarmDestAddress(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T *dest_address_ap, UI32_T address_length);/*not support*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepLowPrDef
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lowest alarm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            low_pri   - the lowest priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *            2. CFM_TYPE_FNG_LOWEST_ALARM_ALL
 *               CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepLowPrDef(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, CFM_TYPE_FNG_LowestAlarmPri_T low_pri);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepFngAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fng alarm time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            alarm_time  - the fault alarm time by ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.This function only for MIB
 *            2. only supoort 300...10000 ticks
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepFngAlarmTime(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T alarm_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepFngResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fault alarm reset time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            reset_time- the fault alarm reset time by ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.This function only for MIB
 *            2. only supoort 300...10000 ticks
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepFngResetTime(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T reset_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepLbmDestMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination mac address
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mac   - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepLbmDestMacAddress(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN]);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMeptransmitLbmDestMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lbm destination mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mac   - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMeptransmitLbmDestMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T dst_mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepTransmitLbmDestIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination use mep id or mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - set the destination address is mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepTransmitLbmDestIsMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_mep_id );

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepTransmitLbmMessages
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit the lbm counts
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            counts    - the lbm message counts
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepTransmitLbmMessages(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T counts);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetDot1agCfmMepTransmitLbmDataTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's tranmit lbm include data
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *            2. this not suuport, it will just retrun false
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepTransmitLbmDataTlv(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T *content, UI32_T content_length);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepTransmitLbmVlanPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit blm vlan priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            priority  - the lbm priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepTransmitLbmVlanPriority(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepTransmitLtmUseFDBOnly
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm transmit flags
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 *            is_useFDBonly - set only use the fdb or not
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepTransmitLtmUseFDBOnly(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_useFDBonly);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mac-the ltm target address
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMacAddress(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T target_mac[SYS_ADPT_MAC_ADDR_LEN]);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mp id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mep_id - the target mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T target_mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetDot1agCfmMepTransmitLtmTaragetIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm target is the mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - the ltm target is the mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepTransmitLtmTaragetIsMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetDot1agCfmMepTransmitLtmTtl
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm ttl
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ttl       - the trnamsmit ttl
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepTransmitLtmTtl(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ttl);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetStackEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the mep information
 * INPUT    : stack_ifindex - the md index
 *            vid          - the ma index
 *            md_level     - the mep id
 *            stack_direction - the egress identifier
 *          *mep_info_p - the mep info pointer to put the mep info
 * OUTPUT   : *mep_info_p - the mep info
 * RETURN   : None
 * NOTE     :  This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetStackEntry(UI32_T stack_ifindex, UI16_T vid, CFM_TYPE_MdLevel_T level, CFM_TYPE_MP_Direction_T stack_direction, CFM_OM_MepInfo_T *mep_info_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextStackEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep information
 * INPUT    : *stack_ifindex - the md index
 *            *vid          - the ma index
 *            *level     - the mep id
 *            *stack_direction - the egress identifier
 *          *mep_info_p - the mep info pointer to put the mep info
 * OUTPUT   : *stack_ifindex - the md index
 *            *vid          - the ma index
 *            *level     - the mep id
 *            *stack_direction - the egress identifier
 *          *mep_info_p - the mep info
 * RETURN   : None
 * NOTE     :  This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetNextStackEntry(UI32_T *stack_lport, UI16_T *vid, CFM_TYPE_MdLevel_T *level, CFM_TYPE_MP_Direction_T *stack_direction, CFM_OM_MepInfo_T *return_mep_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais status
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            ais_status  - the ais status
 * OUTPUT   : None
 * RETUEN   : True - Sucess
 *            False - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetAisStatus(
    UI8_T                   *md_name_a,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_a,
    UI32_T                  ma_name_len,
    CFM_TYPE_AIS_STATUS_T   ais_status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais status
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETUEN   : True - Sucess
 *            False - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetAisPeriod(
    UI8_T   *md_name_a,
    UI32_T  md_name_len,
    UI8_T   *ma_name_a,
    UI32_T  ma_name_len,
    UI32_T  period);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_AIS_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process AIS timeout
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_AIS_Callback(
    void    *timer_para_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitAis_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process AIS transmit timeout and tranmit again
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_XmitAis_Callback(
    void    *timer_para_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessTrunkAdd1stMemeber
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : when the first membe_ifindex become the trunk port, the all mep on this port
 *           should change lport to trunk_ifindex and update om
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessTrunkAdd1stMemeber(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessTrunkAddMemeber
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *           member_ifindex  - the member ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessTrunkAddMemeber(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessTrunkDeleteLastMemeber
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :like add first member to trunk port, the leave should do the same thing.
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessTrunkDeleteLastMemeber(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessTrunkMemberDelete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : when remote the member from the trunk port, this member shall recalcuate
 *           the mip, because this member ever delete all mip when it join the trunk port
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessTrunkMemberDelete(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessInterfaceStatusChange
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the interface status change call back
 * INPUT    : lport  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : not operation up/ operation up
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessInterfaceStatusChange(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessPortStatusChange
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port status change call back
 * INPUT    : vid    - the vlan id
 *           lport  - the port ifindex
 *           is_forwarding - the port's xstp status is at forwarding or not
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : not operation up/ operation up
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessPortStatusChange(UI16_T vid, UI32_T lport, BOOL_T is_forwarding);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessPortAdminDisable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port admin disable
 * INPUT    : lport  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : not operation up/ operation up
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessPortAdminDisable (UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessVlanCreate_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan create
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessVlanCreate(UI32_T vlan_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessVlanDestory
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan destory and remove the mep and mip
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessVlanDestory(UI32_T vlan_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessVlanMemberAdd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan member add and create the mip
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessVlanMemberAdd(UI32_T vlan_ifindex, UI32_T lport_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessVlanMemberDelete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan member delete and remove the mip or mep
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessVlanMemberDelete(UI32_T vlan_ifindex, UI32_T lport_ifindex);

/* for delay measurement
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_AbortBgXmitDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To abort the background dmm transmission.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_p   - pointer to content of destination mac
 *            md_name_p   - pointer to content of md name
 *            md_name_len - the md name length
 *            ma_name_p   - pointer to content of ma name
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_AbortBgXmitDMM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       *dst_mac_p,
    UI8_T       *md_name_p,
    UI32_T      md_name_len,
    UI8_T       *ma_name_p,
    UI32_T      ma_name_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_AddBgXmitDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To add a background dmm transmission.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_p   - pointer to content of destination mac
 *            md_name_p   - pointer to content of md name
 *            md_name_len - the md name length
 *            ma_name_p   - pointer to content of ma name
 *            ma_name_len - the ma name length
 *            counts      - the counts to transmit
 *            interval    - the transmit interval
 *            timeout     - the timeout for waiting reply
 *            pkt_size    - the packet size to transmit
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_AddBgXmitDMM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       *dst_mac_p,
    UI8_T       *md_name_p,
    UI32_T      md_name_len,
    UI8_T       *ma_name_p,
    UI32_T      ma_name_len,
    UI8_T       counts,
    UI8_T       interval,
    UI8_T       timeout,
    UI16_T      pkt_size,
    UI16_T      pkt_pri);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetDelayMeasureResult
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the delay measure result.
 * INPUT    : None
 * OUTPUT   : avg_fd_ms_p - pointer to content of avarge frame delay in ms
 *            min_fd_ms_p - pointer to content of minimum frame delay in ms
 *            max_fd_ms_p - pointer to content of maximum frame delay in ms
 *            succ_cnt_p  - pointer to content of received dmr's count
 *            total_cnt_p - pointer to content of sent dmm's count
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_GetDelayMeasureResult(
    UI32_T  *avg_fd_ms_p,
    UI32_T  *min_fd_ms_p,
    UI32_T  *max_fd_ms_p,
    UI32_T  *succ_cnt_p,
    UI32_T  *total_cnt_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextDmmReplyRec
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the next reply record of dmm.
 * INPUT    : idx_p     - pointer to content of record idx to get
 *                        (0 - to get first)
 * OUTPUT   : idx_p     - pointer to content of next record idx
 *            fd_ms_p   - pointer to content of frame delay in ms
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
BOOL_T CFM_ENGINE_GetNextDmmReplyRec(
    UI32_T  *idx_p,
    UI32_T  *fd_ms_p,
    UI8_T   *res_p,
    BOOL_T  *is_succ_p);

/* end for delay measurement
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_AbortBgXmitLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To abort the background lbm transmission.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_p   - pointer to content of destination mac
 *            md_name_p   - pointer to content of md name
 *            md_name_len - the md name length
 *            ma_name_p   - pointer to content of ma name
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_AbortBgXmitLBM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       *dst_mac_p,
    UI8_T       *md_name_p,
    UI32_T      md_name_len,
    UI8_T       *ma_name_p,
    UI32_T      ma_name_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_AddBgXmitLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To add a background lbm transmission.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_p   - pointer to content of destination mac
 *            md_name_p   - pointer to content of md name
 *            md_name_len - the md name length
 *            ma_name_p   - pointer to content of ma name
 *            ma_name_len - the ma name length
 *            counts      - the counts to transmit
 *            timeout     - the timeout for waiting reply
 *            pkt_size    - the packet size to transmit
 *            pattern     - the pattern included in data TLV
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_AddBgXmitLBM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       *dst_mac_p,
    UI8_T       *md_name_p,
    UI32_T      md_name_len,
    UI8_T       *ma_name_p,
    UI32_T      ma_name_len,
    UI32_T      counts,
    UI8_T       timeout,
    UI16_T      pkt_size,
    UI16_T      pattern,
    UI16_T      pkt_pri);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetThrpMeasureResult
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
BOOL_T CFM_ENGINE_GetThrpMeasureResult(
    UI32_T  *real_send_p,
    UI32_T  *rcvd_1sec_p,
    UI32_T  *rcvd_total_p,
    UI8_T   *res_bmp_p);

#endif /*#if (SYS_CPNT_CFM == TRUE)*/

#endif /*END _CFM_ENGINE_H*/

