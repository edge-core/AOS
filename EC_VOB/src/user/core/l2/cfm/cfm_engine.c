/*-----------------------------------------------------------------------------
 * Module Name: cfm_engine.c
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

 /* INCLUDE FILE DECLARATIONS
  */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys_module.h"
#include "sys_time.h"
#include "l_mm.h"
#include "l_stdlib.h"


#include "cfm_engine.h"
#include "cfm_timer.h"
#include "cfm_backdoor.h"
#include "backdoor_mgr.h"

#include "swdrv.h"
#include "swctrl.h"
#include "netcfg_pom_ip.h"
#include "l2mux_mgr.h"
#include "vlan_om.h"
#include "vlan_mgr.h"
#include "vlan_lib.h"
#include "xstp_mgr.h"
#include "amtr_mgr.h"
#include "trap_mgr.h"
#include "trap_event.h"
#include "snmp_pmgr.h"
#include "netcfg_type.h"
#include "sys_callback_mgr.h"

#if (SYS_CPNT_CFM == TRUE)
/* NAMING CONSTANT DECLARATIONS
 */
#define CFM_ENGINE_ASN1_TRANS_DOMAIN_OID_LEN    10

/* MACRO FUNCTIONS DECLARACTION
 */
#define CFM_ENGINE_ASSEMBLE_DEST_MAC(dst_mac, level)    \
        {                                               \
            memcpy(dst_mac, CFM_TYPE_LEVLE_0_DST_ADDR_MAC, SYS_ADPT_MAC_ADDR_LEN);  \
            dst_mac[SYS_ADPT_MAC_ADDR_LEN-1] |= (((UI8_T)level)&0x0f);              /*the last 4bits shall be the level*/ \
        }

/* 1      sec = 10^3  ms
 * 1 nano sec = 10^-6 ms
 */
#define CFM_ENGINE_10MS_TO_SEC(ms)      ((ms) / 100)
#define CFM_ENGINE_10MS_TO_NANOSEC(ms)  ((ms) * 10000000)
#define CFM_ENGINE_CNV_SEC_P_NS_TO_10MS(sec, ns) (((sec)*100) + ((ns)/10000000))

#define CFM_ENGINE_UPD_ERR_MAC_STATUS(mep_p)                    \
    {                                                           \
        if (mep_p->rmep_lrn_cnt == 0)                           \
            mep_p->err_mac_status = FALSE;                      \
        else                                                    \
        {                                                       \
            if (  (mep_p->rmep_inf_down_cnt > 0)                \
                ||(mep_p->rmep_port_down_cnt == mep_p->rmep_lrn_cnt) \
               )                                                \
                mep_p->err_mac_status = TRUE;                   \
            else                                                \
                mep_p->err_mac_status = FALSE;                  \
        }                                                       \
    }

#define CFM_ENGINE_UPD_SOME_RDI_DEF(mep_p)      \
    {                                           \
        if (mep_p->rmep_rdi_on_cnt > 0)         \
            mep_p->some_rdi_defect = TRUE;      \
        else                                    \
            mep_p->some_rdi_defect = FALSE;     \
    }

#define CFM_ENGINE_UPD_SOME_RMEP_CCM_DEF(mep_p)     \
    {                                               \
        if (mep_p->rmep_ccm_loss_cnt > 0)           \
            mep_p->some_rmep_ccm_defect = TRUE;     \
        else                                        \
            mep_p->some_rmep_ccm_defect = FALSE;    \
    }


/* DATA TYPE DECLARATIONS
 */
typedef union CFM_ENGINE_TmpCfmMsg_S
{
    CFM_ENGINE_DmPdu_T  dm_pdu;
    CFM_ENGINE_LbPdu_T  lb_pdu;
    CFM_ENGINE_LtrPdu_T ltr_pdu;
} CFM_ENGINE_TmpCfmMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T CFM_ENGINE_DecomposeCommonHeader(UI8_T *pdu_p,  CFM_ENGINE_PduHead_T *header_p);
static BOOL_T CFM_ENGINE_IsExistHigherLevelMp( UI32_T checked_lport,
                                                CFM_TYPE_MdLevel_T checked_Level,
                                                UI16_T checked_vid,
                                                CFM_TYPE_MP_Direction_T direction,
                                                BOOL_T check_mip,
                                                CFM_OM_MEP_T *mep_p,
                                                CFM_OM_MIP_T *mip_p);
static BOOL_T CFM_ENGINE_ProcessCCM(UI8_T *pdu_p, CFM_ENGINE_PduHead_T *header_p);
static BOOL_T CFM_ENGINE_ProcessLowCCM(CFM_ENGINE_CcmPdu_T *ccm_p, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_VerifyAndDecomposeCCM(UI8_T *pdu_p, CFM_ENGINE_PduHead_T *header_p, CFM_ENGINE_CcmPdu_T *ccm_p);
static BOOL_T CFM_ENGINE_MepProcessEqualCCM(CFM_ENGINE_CcmPdu_T *ccm_p, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_MepExecuseCCM(CFM_ENGINE_CcmPdu_T *ccm_p, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_MipProcessEqualCCM(CFM_ENGINE_CcmPdu_T *ccm_p, CFM_OM_MIP_T *mip_p);
static BOOL_T CFM_ENGINE_MipExecuseCCM(CFM_ENGINE_CcmPdu_T *ccm_p, CFM_OM_MIP_T *mip_p);
static BOOL_T CFM_ENGINE_ErrCcmDefectState(CFM_ENGINE_CcmPdu_T *ccm_p, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_XconDefectState(CFM_ENGINE_CcmPdu_T *ccm_p, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_RemoteMepOkState(CFM_ENGINE_CcmPdu_T *ccm_p, CFM_OM_REMOTE_MEP_T *remote_mep_p, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_FngStateMachine(CFM_TYPE_FNG_HighestDefectPri_T defect_type, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_FngReportDefectState(CFM_TYPE_FNG_HighestDefectPri_T defect_type, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_FngDefectClearingState(CFM_TYPE_FNG_HighestDefectPri_T defect_type, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_FngDefectState(CFM_TYPE_FNG_HighestDefectPri_T defect_type, CFM_OM_MEP_T *mep_p);

static BOOL_T CFM_ENGINE_XmitFaultAlarm(CFM_TYPE_FNG_HighestDefectPri_T defect_type, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_XmitCCTrap(CFM_OM_MD_T *md_p, CFM_OM_MA_T *ma_p, UI16_T mep_id, UI16_T rmep_id, CFM_TYPE_SnmpTrapsCC_T trap);
static BOOL_T CFM_ENGINE_XmitCrossCheckTrap(CFM_OM_MD_T *md_p, CFM_OM_MA_T *ma_p, UI16_T mep_id, UI16_T rmep_id, CFM_TYPE_SnmpTrapsCrossCheck_T trap);
static BOOL_T CFM_ENGINE_MepProcessLBM (CFM_ENGINE_LbPdu_T *lbm_p, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_MipProcessLBM( CFM_ENGINE_LbPdu_T *lbm_p, CFM_OM_MIP_T *mip_p);
static BOOL_T CFM_ENGINE_MepProcessLBR( CFM_ENGINE_LbPdu_T *lbr, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_VerifyAndDecomposeLB(UI8_T *pdu_p, CFM_ENGINE_PduHead_T *header_p, CFM_ENGINE_LbPdu_T *lb_p);
static void   CFM_ENGINE_XmitLBR(CFM_ENGINE_LbPdu_T *lbr_p, CFM_TYPE_MdLevel_T level, UI8_T src_addr[SYS_ADPT_MAC_ADDR_LEN]);
static void   CFM_ENGINE_ConstructLBPDU(
    UI8_T               *pdu_p,
    UI16_T              *pdu_len_p,
    CFM_OM_MEP_T        *mep_p,
    CFM_TYPE_OpCode_T   opcode,
    UI16_T              data_len,
    UI16_T              pattern);
static BOOL_T CFM_ENGINE_ProcessLTM(UI8_T *pdu_p, CFM_ENGINE_PduHead_T *header_p);
static BOOL_T CFM_ENGINE_DownMhfProcessLTM(CFM_OM_MIP_T *mip_p, CFM_ENGINE_LtmPdu_T *ltm_p);
static BOOL_T CFM_ENGINE_VerifyAndDecomposeLTM(UI8_T *pdu_p, CFM_ENGINE_PduHead_T *header_p, CFM_ENGINE_LtmPdu_T *ltm_p);
static BOOL_T CFM_ENGINE_FindLtmEgressPort(CFM_OM_MD_T *md_p, CFM_OM_MA_T *ma_p, UI8_T egress_mac[SYS_ADPT_MAC_ADDR_LEN], CFM_ENGINE_PduHead_T *header_p, UI32_T *egress_port, CFM_TYPE_RelayAction_T *relay_by);
static BOOL_T CFM_ENGINE_IsArrivedTargetMacEx(
    UI32_T                  checked_dst_lport,
    UI8_T                   checked_dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    CFM_TYPE_MP_Direction_T checked_direction,
    BOOL_T                  *is_matched_cpu_p);
static BOOL_T CFM_ENGINE_IsArrivedTargetMac(
    UI32_T                  checked_dst_lport,
    UI8_T                   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    CFM_TYPE_MP_Direction_T dirction);
#if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE)
static BOOL_T CFM_ENGINE_IsExistSameLevelMp(
    CFM_OM_MD_T             *md_p,
    CFM_OM_MA_T             *ma_p,
    CFM_TYPE_MP_Direction_T direction);
#endif /* #if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE) */
static void   CFM_ENGINE_ConstructLTRPDU(
    CFM_ENGINE_LtmPdu_T     *ltm_p,
    UI8_T                   *pdu_p,
    UI16_T                  *pdu_len_p,
    BOOL_T                  forwarded,
    CFM_TYPE_MdLevel_T      level,
    UI16_T                  primaryVid,
    UI32_T                  egress_port,
    UI32_T                  ingress_port,
    CFM_TYPE_RelayAction_T  relay_action,
    BOOL_T                  is_mep,
    BOOL_T                  need_reply_ingress_tlv,
    BOOL_T                  need_reply_egress_tlv);
static BOOL_T CFM_ENGINE_MepProcessLTR(CFM_ENGINE_LtrPdu_T *ltr_pdu_p, CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_VerifyAndDecomposeLTR(UI8_T *pdu_p, CFM_ENGINE_PduHead_T *header_p, CFM_ENGINE_LtrPdu_T *ltr_p);
static void   CFM_ENGINE_EnqueueLTR(
    CFM_ENGINE_LtmPdu_T     *ltm_p,
    UI32_T                  ingress_port,
    UI32_T                  egress_port,
    BOOL_T                  forwarded,
    CFM_TYPE_RelayAction_T  relay_action,
    CFM_OM_MD_T             *md_p,
    CFM_OM_MA_T             *ma_p,
    BOOL_T                  is_mep,
    BOOL_T                  need_reply_ingress_tlv,
    BOOL_T                  need_reply_egress_tlv,
    UI8_T                   *src_mac_p);
static void   CFM_ENGINE_ConstructLTMPDU(UI8_T *pdu_p, UI16_T *pdu_len_p, UI8_T target_mac[SYS_ADPT_MAC_ADDR_LEN], CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_ForwardLTM(CFM_ENGINE_LtmPdu_T *ltm_p, UI32_T through_port);
static BOOL_T CFM_ENGINE_RespondeUpMepLtm(
    CFM_OM_MD_T             *md_p,
    CFM_OM_MA_T             *ma_p,
    CFM_OM_MEP_T            *mep_p,
    UI8_T                   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T                  *egress_port_p,
    BOOL_T                  *is_forward_ltm_p);
static void CFM_ENGINE_XmitCCM(CFM_OM_MEP_T *mep_p, CFM_ENGINE_PortStatusCallback_T *notify_port_status_change);
static void CFM_ENGINE_ConstructCCMPDU(UI8_T *pdu_p, UI16_T *pdu_len_p, CFM_OM_MEP_T *mep_p, CFM_ENGINE_PortStatusCallback_T *port_status_notify);
static void CFM_ENGINE_ConstructCommonHeader(UI8_T *pdu_p, UI16_T *pdu_len, UI8_T level, UI8_T flag, CFM_TYPE_OpCode_T opcode);
static void CFM_ENGINE_ConstructSenderTLV(UI8_T *pdu_p, UI16_T *tlv_len, UI32_T lport);
static void CFM_ENGINE_ConstructPortStatusTLV(UI8_T *pdu_p, UI16_T *tlv_len, UI16_T vid, UI32_T lport, CFM_TYPE_MP_Direction_T direction, CFM_ENGINE_PortStatusCallback_T *port_status_notify);
static void CFM_ENGINE_ConstructInterfaceStatusTLV(UI8_T *pdu_p, UI16_T *tlv_len, UI32_T lport, CFM_TYPE_MP_Direction_T direction, CFM_ENGINE_PortStatusCallback_T *port_status_notify);
static void CFM_ENGINE_ConstructDataTLV(
    UI8_T   *pdu_p,
    UI16_T  *tlv_len,
    UI16_T  data_len,
    UI16_T  pattern);
static void CFM_ENGINE_ConstructEndTLV(UI8_T *pdu_p, UI16_T *tlv_len);
static void CFM_ENGINE_ConstructOrginizationTLV(UI8_T *pdu_p, UI16_T *tlv_len, UI32_T lport, CFM_TYPE_OrganizationSubtype_T subtype);
static BOOL_T CFM_ENGINE_DecomposeAndVerifySenderTlv(
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_SenderTlv_T  *sender_tlv_p,
    UI8_T                   *tlv_p);
static BOOL_T CFM_ENGINE_ConstructMref(L_MM_Mref_Handle_T **mref_p, UI8_T *buf_p, UI8_T *pdu_p, UI16_T  pdu_len, CFM_TYPE_TxReason_T tx_reason);

static void CFM_ENGINE_FloodPDU(
    UI32_T  rcvd_port,
    UI16_T  tag_info,
    UI8_T   dst_mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   src_mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   *pdu_p,
    UI32_T  pdu_len,
    BOOL_T  is_src_lport_fwd);
static CFM_TYPE_MpType_T CFM_ENGINE_GetMpProcessRcvdPacket(
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_OM_MEP_T            *mep_p,
    CFM_OM_MIP_T            *mip_p);
static CFM_TYPE_MpType_T CFM_ENGINE_GetMpProcessRcvdPacket_Ex(
    UI16_T                  md_level,
    UI16_T                  vid,
    UI32_T                  lport,
    CFM_OM_MEP_T            *mep_p,
    CFM_OM_MIP_T            *mip_p);
static void CFM_ENGINE_XmitPDU(UI32_T lport, UI16_T vid, UI8_T level, UI32_T priority, BOOL_T need_tagged, UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                             UI8_T src_mac[SYS_ADPT_MAC_ADDR_LEN], CFM_TYPE_MP_Direction_T direction, UI8_T *pdu, UI32_T pdu_len);
static BOOL_T CFM_ENGINE_GetNextVidFromMaVidBmp(
                            CFM_OM_MA_T             *ma_p,
                            UI16_T                  *vid_p);
static BOOL_T CFM_ENGINE_DoIngProcForUniPdu(
    CFM_ENGINE_PduHead_T    *header_p,
    UI8_T                   *pdu_p,
    UI32_T                  pdu_len);
static BOOL_T CFM_ENGINE_DoEgrProcForForwarding(
    CFM_ENGINE_PduHead_T    *header_p,
    UI8_T                   *pdu_p,
    UI32_T                  pdu_len,
    BOOL_T                  is_need_tag);
static BOOL_T CFM_ENGINE_IsDstMacForUs(
    UI32_T                  ing_lport,
    UI8_T                   *dst_mac_p);
static BOOL_T CFM_ENGINE_ForwardUnicastPdu(
    CFM_ENGINE_PduHead_T    *header_p,
    UI8_T                   *pdu_p,
    UI32_T                  pdu_len,
    BOOL_T                  is_need_tag);
static BOOL_T CFM_ENGINE_FindTargetMacAndMep(
    UI32_T          md_index,
    UI32_T          ma_index,
    UI16_T          vid,
    UI32_T          dst_mep_id,
    UI8_T           dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T          dbg_flag,
    UI8_T           target_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T          *egress_port,
    CFM_OM_MEP_T    *mep_p);
static BOOL_T CFM_ENGINE_FindOperationUpMep(
    UI32_T          md_index,
    UI32_T          ma_index,
    CFM_OM_MEP_T    *mep_p,
    UI8_T           *chk_mac_p,
    UI32_T          lport_to_skip);
static BOOL_T CFM_ENGINE_ResetRcvdMepStateOfRemoteMep(
    CFM_OM_REMOTE_MEP_T     *remote_mep_p);

static BOOL_T CFM_ENGINE_ConstructAISPdu(UI8_T *pdu_p, UI16_T *pdu_len_p, CFM_TYPE_MdLevel_T level, UI16_T ais_period);
static BOOL_T CFM_ENGINE_XmitAis( CFM_OM_MEP_T *mep_p, BOOL_T is_forward);
static BOOL_T CFM_ENGINE_MepProcessAIS(CFM_ENGINE_PduHead_T *header_p, CFM_OM_MEP_T *mep_p);

static void CFM_ENGINE_UpdateMepHighestDefect(CFM_OM_MEP_T *mep_p);
static BOOL_T CFM_ENGINE_LocalValidateSAandDA(
    CFM_ENGINE_PduHead_T    *header_p,
    UI8_T                   *pdu_p,
    UI32_T                  pdu_len);
static BOOL_T CFM_ENGINE_ProcessCfmMsg(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p);
static char *CFM_ENGINE_LocalGetOpcodeStr(
    UI8_T   op_code);

/* for delay measurement
 */
static void CFM_ENGINE_AddDmrToResult(
                             CFM_ENGINE_DmPdu_T      *dmr_p);
static BOOL_T CFM_ENGINE_BgXmitDMM_CallBack(
                             void                    *rec_p);
static UI32_T CFM_ENGINE_ComputeFramDealy(
                            CFM_ENGINE_DmPdu_T      *dmr_p,
                            UI32_T                  *txf_10ms_p);
static BOOL_T CFM_ENGINE_ConstructDMPDU(
                            CFM_OM_MEP_T            *src_mep_p,
                            CFM_TYPE_OpCode_T       opcode,
                            UI8_T                   *rcvd_pdu_p,
                            UI16_T                  rcvd_pdu_len,
                            UI8_T                   *tran_pdu_p,
                            UI16_T                  *tran_pdu_len_p,
                            UI32_T                  *tx_time_10ms_p);
static void CFM_ENGINE_ExpireDmrRecord(
                            CFM_OM_DmmCtrlRec_T     *dmm_ctrl_rec_p,
                            UI32_T                  curr_time_10ms);
static BOOL_T CFM_ENGINE_MepProcessDMM(
                            CFM_ENGINE_DmPdu_T      *dmm_p,
                            CFM_OM_MEP_T            *mep_p);
static BOOL_T CFM_ENGINE_MepProcessDMR(
                            CFM_ENGINE_DmPdu_T      *dmr_p,
                            CFM_OM_MEP_T            *mep_p);
static BOOL_T CFM_ENGINE_VerifyAndDecomposeDM(
                            UI8_T                   *pdu_p,
                            CFM_ENGINE_PduHead_T    *header_p,
                            CFM_ENGINE_DmPdu_T      *dm_p);
static BOOL_T CFM_ENGINE_XmitDMM(
                            UI32_T      src_mep_id,
                            UI32_T      dst_mep_id,
                            UI8_T       *dst_mac_p,
                            UI8_T       *md_name_p,
                            UI32_T      md_name_len,
                            UI8_T       *ma_name_p,
                            UI32_T      ma_name_len,
                            UI32_T      pkt_size,
                            UI16_T      pkt_pri,
                            UI32_T      *tx_time_10ms_p,
                            BOOL_T      is_test);
static BOOL_T CFM_ENGINE_XmitDMR(
                            CFM_ENGINE_DmPdu_T      *dmm_p,
                            CFM_TYPE_MdLevel_T      level,
                            UI8_T                   *src_addr_p);
/* end for delay measurement
 */

static BOOL_T CFM_ENGINE_BgXmitLBM_CallBack(
    void                    *rec_p);
static void CFM_ENGINE_FreeLbmTransmitData(
                            CFM_OM_LbmCtrlRec_T     *lbm_ctrl_rec_p);
static BOOL_T CFM_ENGINE_GetEgressPortToXmitCfmPdu(
                            UI32_T          src_mep_id,
                            UI32_T          dst_mep_id,
                            UI8_T           *dst_mac_p,
                            UI8_T           *md_name_p,
                            UI32_T          md_name_len,
                            UI8_T           *ma_name_p,
                            UI32_T          ma_name_len,
                            UI32_T          *egress_port_p,
                            UI8_T           *target_mac_p,
                            CFM_OM_MEP_T    *src_mep_p);
static BOOL_T CFM_ENGINE_SendLbmForThrptMeasure(
                            CFM_OM_LbmCtrlRec_T     *lbm_ctrl_rec_p);
static void CFM_ENGINE_XmitPduBySrcMep(
                            CFM_OM_MEP_T            *src_mep_p,
                            UI32_T                  egress_port,
                            UI32_T                  priority,
                            UI8_T                   *target_mac_p,
                            UI8_T                   *pdu_p,
                            UI32_T                  pdu_len);
static void CFM_ENGINE_LocalAbortAllBgOperations(void);
static BOOL_T CFM_ENGINE_LocalAddLtrToOM (
    CFM_OM_LTR_T            *ltr_om_p);
static void CFM_ENGINE_LocalFreeMepTimer(
    CFM_OM_MEP_T            *mep_p);
static void CFM_ENGINE_LocalGetCurRMepMacStatusAndRdiCntInOneMa(
    UI32_T                  md_index,
    UI32_T                  ma_index,
    CFM_OM_MEP_T            *mep_p);
static void CFM_ENGINE_LocalGetCurRMepCcmLostCntInOneMa(
    UI32_T                  md_index,
    UI32_T                  ma_index,
    CFM_OM_MEP_T            *mep_p);
static void CFM_ENGINE_LocalUpdateRMepCcmLostCntInOneMa(
    CFM_OM_REMOTE_MEP_T     *remote_mep_p,
    CFM_OM_MEP_T            *cur_rcvd_mep_p,
    BOOL_T                  is_add);
static void CFM_ENGINE_LocalUpdateRcvdMepMacStatusAndRdiCnt(
    CFM_ENGINE_CcmPdu_T     *ccm_p,
    CFM_OM_REMOTE_MEP_T     *remote_mep_p,
    CFM_OM_MEP_T            *cur_rcvd_mep_p);
static BOOL_T CFM_ENGINE_LocalReplceLmepId(
    CFM_OM_MEP_T            *lmep_p,
    UI32_T                  new_lmep_id);


/* STATIC VARIABLE DECLARATIONS
 */
/*assemble the dst mac*/
const static UI8_T CFM_TYPE_LEVLE_0_DST_ADDR_MAC[SYS_ADPT_MAC_ADDR_LEN]= {0x01, 0x80, 0xC2, 0x00, 0x00, 0x30};

/* this part is the oid already encoded in asn.1 format,
 *   according to 802.1ag-D8 subclause 21.5.3.5 (x.690, subclause 8.19)
 *
 * original value is 1.3.6.1.2.1.100.1.1~16
 *
 * refer to CFM_OM_CnvOidToData if need to do the conversion in code.
 */
const static UI8_T  asn1_transport_domain_oid [][CFM_ENGINE_ASN1_TRANS_DOMAIN_OID_LEN] =
                        {{6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 1},    /*transportDomainUdpIpv4  */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 2},    /*transportDomainUdpIpv6  */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 3},    /*transportDomainUdpIpv4z */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 4},    /*transportDomainUdpIpv6z */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 5},    /*transportDomainTcpIpv4  */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 6},    /*transportDomainTcpIpv6  */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 7},    /*transportDomainTcpIpv4z */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 8},    /*transportDomainTcpIpv6z */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 9},    /*transportDomainSctpIpv4 */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 10},   /*transportDomainSctpIpv6 */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 11},   /*transportDomainSctpIpv4z*/
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 12},   /*transportDomainSctpIpv6z*/
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 13},   /*transportDomainLocal    */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 14},   /*transportDomainUdpDns   */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 15},   /*transportDomainTcpDns   */
                         {6, 8, 0x2B, 6, 1, 2, 1, 100, 1, 16}    /*transportDomainSctpDns  */
                         };

/* temp buffer for engine api
 */
CFM_OM_MEP_T mep_tmp1_g, mep_tmp2_g, mep_tmp3_g, mep_tmp4_g;
CFM_OM_LTR_T ltr_tmp_g, ltr_tmp2_g;

/* EXPORTED SUBPROGRAM BODIES
 */
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
void CFM_ENGINE_Init()
{

}/*End of CFM_ENGINE_Init*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function will creat the timer which need run when systerm up
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_EnterMasterMode()
{
    {/*create link trace pending list timer*/
        I16_T lft_timer_idx;
        lft_timer_idx=CFM_TIMER_CreateTimer(
                                    CFM_ENGINE_ClearPendingLTRs_CallBack,
                                    NULL,
                                    1 ,
                                    CFM_TIMER_CYCLIC);
        CFM_TIMER_StartTimer(lft_timer_idx);
    }

    {
        CFM_OM_LbmCtrlRec_T *lbm_ctrl_rec_p;

        /* init the timer's data for future usage
         */
        lbm_ctrl_rec_p = CFM_OM_GetLbmCtrlRecPtr();
        if (NULL != lbm_ctrl_rec_p)
        {
            lbm_ctrl_rec_p->timeout_timer_idx=CFM_TIMER_CreateTimer(
                                                CFM_ENGINE_BgXmitLBM_CallBack,
                                                NULL,
                                                1,
                                                CFM_TIMER_CYCLIC);
        }
    }

    {
        CFM_OM_DmmCtrlRec_T *dmm_ctrl_rec_p;

        /* init the timer's data for future usage
         */
        dmm_ctrl_rec_p = CFM_OM_GetDmmCtrlRecPtr();
        if (NULL != dmm_ctrl_rec_p)
        {
            dmm_ctrl_rec_p->tx_timer_idx=CFM_TIMER_CreateTimer(
                                            CFM_ENGINE_BgXmitDMM_CallBack,
                                            NULL,
                                            1,
                                            CFM_TIMER_CYCLIC);
        }
    }

    return;
}/* End of CFM_ENGINE_EnterMasterMode*/

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
void CFM_ENGINE_EnterTransitionMode()
{
}/*End of CFM_ENGINE_EnterTransitionMode*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_IsValidMcDA
 *-------------------------------------------------------------------------
 * PURPOSE  : To check if the DA is valid Multicast DA with specified
 *              md level and multicast class.
 * INPUT    : level      - md level to check
 *            class_type - Multciast Class Type to check
 *                         Class 1 - CCM/LBM/AIS ...
 *                         Class 2 - LTM
 *            da_p       - pointer to da to check
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_IsValidMcDA(
    UI8_T                   level,
    UI8_T                   class_type,
    UI8_T                   *da_p)
{
    UI8_T   mc_mac[SYS_ADPT_MAC_ADDR_LEN] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x30};
    BOOL_T  ret = FALSE;

    mc_mac[5] |= ((level & 0x7) + ((class_type == CFM_TYPE_DA_MC_CLASS_2) ? 8 : 0));

    if (0 == memcmp(da_p, mc_mac, sizeof(mc_mac)))
    {
        ret = TRUE;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalGetOpcodeStr
 *-------------------------------------------------------------------------
 * PURPOSE  : To get debug string for CFM PDU OpCode.
 * INPUT    : op_code - CFM PDU OpCode to get debug string
 * OUTPUT   : None
 * RETURN   : pointer to debug string for OpCode.
 * NOTE     : for debug
 *-------------------------------------------------------------------------
 */
static char *CFM_ENGINE_LocalGetOpcodeStr(
    UI8_T   op_code)
{
    static char ukn_buf[20];
    char    *ret_p = ukn_buf;

    switch (op_code)
    {
    case CFM_TYPE_OPCODE_CCM:
        ret_p = "CCM";
        break;
    case CFM_TYPE_OPCODE_LBM:
        ret_p = "LBM";
        break;
    case CFM_TYPE_OPCODE_LBR:
        ret_p = "LBR";
        break;
    case CFM_TYPE_OPCODE_LTM:
        ret_p = "LTM";
        break;
    case CFM_TYPE_OPCODE_LTR:
        ret_p = "LTR";
        break;
    case CFM_TYPE_OPCODE_AIS:
        ret_p = "AIS";
        break;
    case CFM_TYPE_OPCODE_DMM:
        ret_p = "DMM";
        break;
    case CFM_TYPE_OPCODE_DMR:
        ret_p = "DMR";
        break;
    default:
        sprintf(ret_p, "Op-%d", op_code);
        break;
    }

    return ret_p;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalValidateSAandDA
 *-------------------------------------------------------------------------
 * PURPOSE  : To validate the SA and DA of received CFM PDU.
 * INPUT    : header_p - pointer to the common header of the received pdu
 *            pdu_p    - pointer to the received pdu packet
 *            pdu_len  - length of pdu
 * OUTPUT   : None
 * RETURN   : TRUE  -
 *            FALSE - validation failed, no further processing
 * NOTE     : PDU will be forwarded here, if it's not for us.
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_LocalValidateSAandDA(
    CFM_ENGINE_PduHead_T    *header_p,
    UI8_T                   *pdu_p,
    UI32_T                  pdu_len)
{
    BOOL_T  is_da_uc = FALSE, is_need_tag = FALSE;
    BOOL_T  ret = TRUE;

    /* 20.2.2 Loopback Message reception and Loopback Reply transmission
     * it shall be discarded if the source_address is a Group
     * and not an Individual MAC address.
     *
     * from Y.1731
     * The SA in an OAM frame is always Unicast.
     */
    if (0 != (header_p->src_mac_a[0] & 0x01))
    {
        CFM_BD_MSG(header_p->pkt_dbg_flag, "(%ld), SA is not unicast",
                    (long)header_p->lport);
        return FALSE;
    }

    if (0 == (header_p->dst_mac_a[0] & 0x01))
        is_da_uc = TRUE;

    /* LTM/LTR should be always tagged, bcz linktrace need to know
     *  if the packet's vid is different from the recieved port
     */
    if (header_p->op_code == CFM_TYPE_OPCODE_LTR)
        is_need_tag = TRUE;

    /* according to Y.1731, Table 10-1 OAM frame DA
     */
    switch (header_p->op_code)
    {
    case CFM_TYPE_OPCODE_LTM:
        /* multicast class 2
         */
        if (  (TRUE == is_da_uc)
            ||(FALSE == CFM_ENGINE_IsValidMcDA(
                        header_p->md_level, CFM_TYPE_DA_MC_CLASS_2, header_p->dst_mac_a))
           )
        {
            CFM_BD_MSG(header_p->pkt_dbg_flag, "(%ld), DA is not valid MC2",
                        (long)header_p->lport);
            ret = FALSE;
        }
        break;
    case CFM_TYPE_OPCODE_CCM:
        /* for 802.1ag-2007 Table 19-2,
         *   No address matching performed for CCM.
         * for Y.1731
         *   CCM can use unicast DA.
         */

    case CFM_TYPE_OPCODE_LBM:
    case CFM_TYPE_OPCODE_AIS:
    case CFM_TYPE_OPCODE_DMM:
        /* unicast or multicast class 1
         */
        if (FALSE == is_da_uc)
        {
            /* do multicast part...
             */
            if (FALSE == CFM_ENGINE_IsValidMcDA(
                        header_p->md_level, CFM_TYPE_DA_MC_CLASS_1, header_p->dst_mac_a))
            {
                CFM_BD_MSG(header_p->pkt_dbg_flag, "(%ld), DA is not valid MC1",
                            (long)header_p->lport);
                ret = FALSE;
            }
            break;
        }

        /* continue to do unicast part below...
         */
    case CFM_TYPE_OPCODE_LBR:
    case CFM_TYPE_OPCODE_LTR:
    case CFM_TYPE_OPCODE_DMR:
        /* unicast
         */
        if (TRUE == is_da_uc)
        {
            /* da is unicast, check if da is for us
             */
            if (TRUE == CFM_ENGINE_ForwardUnicastPdu(
                            header_p, pdu_p, pdu_len, is_need_tag))
            {
                CFM_BD_MSG(header_p->pkt_dbg_flag, "(%ld), Not for us, forwarded",
                            (long)header_p->lport);
                ret = FALSE;
            }
        }
        else
        {
            CFM_BD_MSG(header_p->pkt_dbg_flag, "(%ld), DA is not UC",
                        (long)header_p->lport);
            ret = FALSE;
        }
        break;
    default:
        /* not supported yet
         */
#if (SYS_CPNT_CFM_FORWARD_Y1731_PDU_TRANSPARENTLY == TRUE)
        CFM_BD_MSG(header_p->pkt_dbg_flag, "Flood(%ld/ing_fwd-%d), opcode not supported",
                    (long)header_p->lport, header_p->is_ing_lport_fwd);

        /* for AGG, relay the pdu not for us.
         */
        CFM_ENGINE_FloodPDU(
            header_p->lport,       header_p->tag_info, header_p->dst_mac_a,
            header_p->src_mac_a,   pdu_p,              header_p->pdu_length,
            header_p->is_ing_lport_fwd);
        ret = FALSE;
#else
        /* let other opcode go through the normal processing as 19.2.7, Table 19-1
         */
        ret = TRUE;
#endif

        break;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the recevied CFMDU.
 * INPUT    : *msg   - the message get from the msg queue
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : this is only called by the cfm_mgr.c
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessRcvdPDU(CFM_TYPE_Msg_T *msg)
{
    UI8_T                   *pdu_p=NULL;
    UI32_T                  pdu_len=0;
    CFM_ENGINE_PduHead_T    header;
    CFM_TYPE_CfmStatus_T    global_status=CFM_TYPE_CFM_STATUS_DISABLE,
                            port_status=CFM_TYPE_CFM_STATUS_DISABLE;

    pdu_p = L_MM_Mref_GetPdu(msg->mem_ref_p, &pdu_len);

    if(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW))
    {
        if(NULL != pdu_p)
        {
            CFM_BD_MSG_S("(%5d):%s, taginfo=%04x, lport=%ld, du_len=%ld\r\n",
                __LINE__, __FUNCTION__, msg->packet_header_p->tagInfo, (long)msg->packet_header_p->lport, (long)pdu_len);
        }
    }

    /*1. decompose common header
     */
    CFM_ENGINE_DecomposeCommonHeader(pdu_p, &header);
    header.pdu_length=pdu_len;
    header.tag_info=msg->packet_header_p->tagInfo;
    memcpy(header.src_mac_a, msg->packet_header_p->srcMac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(header.dst_mac_a, msg->packet_header_p->dstMac, SYS_ADPT_MAC_ADDR_LEN);

    {/*check and transfer the input port to logical port*/
        UI32_T trunk_ifindex=0;
        BOOL_T is_static=FALSE;

        if(TRUE == SWCTRL_IsTrunkMember(msg->packet_header_p->lport, &trunk_ifindex, &is_static))
        {
            header.lport=trunk_ifindex;
        }
        else
        {
            header.lport=msg->packet_header_p->lport;
        }
    }

    header.is_ing_lport_fwd = XSTP_OM_IsPortForwardingStateByVlan(
                                (header.tag_info&0x0fff), header.lport);

    /*check the cfm status.
     */
    CFM_OM_GetCFMGlobalStatus(&global_status);
    CFM_OM_GetCFMPortStatus(header.lport, &port_status);
    if(global_status==CFM_TYPE_CFM_STATUS_DISABLE||port_status==CFM_TYPE_CFM_STATUS_DISABLE)
    {
        CFM_ENGINE_FloodPDU(
            header.lport,       header.tag_info,    header.dst_mac_a,
            header.src_mac_a,   pdu_p,              header.pdu_length,
            header.is_ing_lport_fwd);
        CFM_BD_MSG(header.pkt_dbg_flag, "Flood(%ld/fwd-%d), cfm is not enable",
                    (long)header.lport, header.is_ing_lport_fwd);
        return;
    }
    /*2. check the md level or the accept md level
     */
    if (CFM_TYPE_MD_LEVEL_7< header.md_level)
    {
        CFM_BD_MSG(header.pkt_dbg_flag, "Discard(%ld), md level-%d over 7",
                   (long)header.lport, header.md_level);
        return;
    }

    /*if pdu ingress port is not in the vlan tagged in the pdu,discard the pdu except LTM and LTR
    Because if receive a wrong vid LTM the dut will respond a LTR with ingress action ingVID bit set*/
    if((CFM_TYPE_OPCODE_LTM!=header.op_code)&&(CFM_TYPE_OPCODE_LTR!=header.op_code))
    {
        UI32_T vlan_ifindex;

        VLAN_OM_ConvertToIfindex((header.tag_info&0x0fff),&vlan_ifindex);
        if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, header.lport))
        {
            CFM_BD_MSG(header.pkt_dbg_flag, "Discard(%ld), not in vlan %d",
                        (long)header.lport,(header.tag_info&0x0fff));
            return;
        }
    }

    if (FALSE == CFM_ENGINE_LocalValidateSAandDA(&header, pdu_p, pdu_len))
    {
        CFM_BD_MSG(header.pkt_dbg_flag,
            "pkt proccessing terminated(%ld), %s", (long)header.lport,
            CFM_ENGINE_LocalGetOpcodeStr(header.op_code));
        return;
    }

    /*3. check the md level over the max md level of the system, it mean it can't find the MP to process
     */
    if(header.md_level > CFM_OM_GetMaxMdLevel())
    {
        CFM_ENGINE_FloodPDU(header.lport,       header.tag_info,    header.dst_mac_a,
                            header.src_mac_a,   pdu_p,              header.pdu_length,
                            header.is_ing_lport_fwd);
        CFM_BD_MSG(header.pkt_dbg_flag, "Flood(%ld/fwd-%d), md level-%d over max md level",
                    (long)header.lport, header.is_ing_lport_fwd, header.md_level);
        return;
    }

    /*4. switch accroding to the receive PDU type
     */
    CFM_BD_MSG(header.pkt_dbg_flag,
                "%s received(%ld), level-%d",
                CFM_ENGINE_LocalGetOpcodeStr(header.op_code),
                (long)header.lport, header.md_level);

    switch(header.op_code)
    {
    case CFM_TYPE_OPCODE_CCM:
        /*     CCM case: lower level CCM will be processed by higher level MEP
         */
        CFM_ENGINE_ProcessCCM(pdu_p, &header);
        break;

    case CFM_TYPE_OPCODE_LBM:
    case CFM_TYPE_OPCODE_DMM:
    case CFM_TYPE_OPCODE_LBR:
    case CFM_TYPE_OPCODE_DMR:
    case CFM_TYPE_OPCODE_LTR:
    case CFM_TYPE_OPCODE_AIS:
        /* non CCM case: lower level PDU will be dropped by higher level MEP
         */
        CFM_ENGINE_ProcessCfmMsg(pdu_p, &header);
        break;

    case CFM_TYPE_OPCODE_LTM:
        /* should be the same as non CCM case,
         * but the common processing model need to get more complete design
         */
        CFM_ENGINE_ProcessLTM(pdu_p, &header);
        break;

    default:
        break;
    }
}/*End of CFM_ENGINE_ProcessRcvdPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FloodPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : the system needn't to handle this PDU, so flood to all port
 * INPUT    : rcvd_port    - which port receieve the packet
 *            tag_info     - the received packet's tag info
 *            dst_mac_addr - the array store the flood packet destination mac
 *            src_mac_addr - the array store the flood packet source mac
 *            *pud_p       - the oontent pointer of the flood packet
 *            pdu_len      - the pdu lenght of the flood packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_FloodPDU(
    UI32_T  rcvd_port,
    UI16_T  tag_info,
    UI8_T   dst_mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   src_mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   *pdu_p,
    UI32_T  pdu_len,
    BOOL_T  is_src_lport_fwd)
{
    L_MM_Mref_Handle_T  *mref_p=NULL;
    UI8_T               *buf_p=NULL;

    if(  (FALSE == is_src_lport_fwd)
       ||(FALSE == CFM_ENGINE_ConstructMref(&mref_p, buf_p, pdu_p, pdu_len, CFM_TYPE_FLOODPDU))
      )
    {
        return;
    }

    if(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_TX_PACKET_FLOW))
    {
        if(NULL != pdu_p)
        {
            CFM_BD_MSG_S("(%5d):%s, ing_lport/tag_info/pdu_len-%ld/%04x/%ld\r\n",
                __LINE__, __FUNCTION__, (long)rcvd_port, tag_info, (long)pdu_len);
        }
    }

    /* send packet */
    {
        UI8_T *port_list;
        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;

        if (VLAN_OM_GetDot1qVlanCurrentEntry(0, (tag_info&0x0fff), &vlan_info) == FALSE)
        {
            return;
        }

        /*exclusive the recieved port*/
        port_list= vlan_info.dot1q_vlan_current_egress_ports;
        port_list[(rcvd_port-1)/8]&=(~(0x80>>((rcvd_port-1)%8)));

        port_list=vlan_info.dot1q_vlan_current_untagged_ports;
        port_list[(rcvd_port-1)/8]&=(~(0x80>>((rcvd_port-1)%8)));

#if (SYS_CPNT_MGMT_PORT == TRUE)
        port_list[(SYS_ADPT_MGMT_PORT-1)/8]&=(~(0x80>>((SYS_ADPT_MGMT_PORT-1)%8)));
#endif

        L2MUX_MGR_SendMultiPacket(mref_p,                                        /* L_MREF * */
                                  dst_mac_addr,                                /* dst mac */
                                  src_mac_addr,                                /* src mac */
                                  CFM_TYPE_CFM_ETHER_TYPE,                     /* packet type */
                                  (tag_info&0x0fff),                           /* tag_info */
                                  pdu_len,                                     /* packet length */
                                  vlan_info.dot1q_vlan_current_egress_ports,   /* lport */
                                  vlan_info.dot1q_vlan_current_untagged_ports,
                                  (tag_info&0xe000)>>13);                      /* transfer priority*/
    }

    return;
}/*End of CFM_ENGINE_FloodPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitPDU
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will transmit the packet.
 * INPUT    : lport        - the packet will be transmitted from this port.
 *            vid          - the packet's vlan id
 *            priority     - the packet's priority
 *            level        - the md level
 *            need_tagged  - this packet need forwarded with tag
 *            dst_mac      - the packet's destination mac
 *            direction    - the mp direction
 *            src_mac_addr - the array store the src mac of the received packet
 *            *pdu_p       - the packet content pointer
 *            pdu_len      - the packet's length
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_XmitPDU(
    UI32_T                  lport,
    UI16_T                  vid,
    UI8_T                   level,
    UI32_T                  priority,
    BOOL_T                  need_tagged,
    UI8_T                   dst_mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T                   src_mac_addr[SYS_ADPT_MAC_ADDR_LEN] ,
    CFM_TYPE_MP_Direction_T direction,
    UI8_T                   *pdu_p,
    UI32_T                  pdu_len)
{
    L_MM_Mref_Handle_T  *mref_p=NULL;
    UI8_T               *mref_pdu_p=NULL;
    UI32_T              vid_ifindex;
    BOOL_T              is_tagged;
    UI16_T              tag_info = 0;

    if(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_TX_PACKET_FLOW))
    {
        if(NULL != pdu_p)
        {
            CFM_BD_MSG_S("(%5d):%s, vid=%d, port=%ld, level=%d, pdu_len=%ld\n\r",
                __LINE__, __FUNCTION__, vid, (long)lport, level, (long)pdu_len);
        }
    }

    /*if port on operation up and down mep, it needn't send, because it won't send out*/
    if(FALSE == SWCTRL_IsPortOperationUp(lport))
    {
        return ;
    }

#if (SYS_CPNT_MGMT_PORT == TRUE)
    if(lport==SYS_ADPT_MGMT_PORT)
    {
       return ;
    }
#endif

    if(FALSE == CFM_ENGINE_ConstructMref(&mref_p, mref_pdu_p, pdu_p, pdu_len, CFM_TYPE_XMITPDU))
    {
        return;
    }

    if(FALSE == need_tagged)
    {
        /*check the packet shall tagged or not*/
        VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);
        if(VLAN_OM_IsVlanUntagPortListMember(vid_ifindex, lport)==FALSE)
        {
            is_tagged=TRUE;
        }
        else
        {
            is_tagged=FALSE;
        }
    }
    else
    {
        is_tagged=TRUE;
    }

    tag_info = ((0x0FFF & vid) | ((0x0007 & (UI16_T)priority) << 13));

    /* send packet */
    if(CFM_TYPE_MP_DIRECTION_UP==direction)
    {
        L2MUX_MGR_SendPacket(mref_p,                       /* L_MREF * */
                             dst_mac_addr,                 /* dst mac */
                             src_mac_addr,                 /* src mac */
                             CFM_TYPE_CFM_ETHER_TYPE,      /* packet type */
                             tag_info,                     /* tag_info */
                             pdu_len,                      /* packet length */
                             lport,                        /* lport */
                             is_tagged,                    /* is_tagged */
                             priority,                     /* transfer priority*/
                             FALSE                         /* is sent to trunk member */
                             );

    }
    else if(CFM_TYPE_MP_DIRECTION_DOWN==direction)
    {
        L2MUX_MGR_SendBPDU(mref_p,                       /* L_MREF * */
                           dst_mac_addr,                 /* dst mac */
                           src_mac_addr,                 /* src mac */
                           CFM_TYPE_CFM_ETHER_TYPE,      /* packet type */
                           tag_info,                     /* tag_info */
                           pdu_len,                      /* packet length */
                           lport,                        /* lport */
                           is_tagged,                    /* is_tagged */
                           priority,                     /* transfer priority*/
                           FALSE                         /* is sent to trunk member */
                           );

    }
}/*End of CFM_ENGINE_XmitPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DoIngProcForUniPdu
 *-------------------------------------------------------------------------
 * PURPOSE  : To do ingress checking for unicast pdu (not for our device)
 * INPUT    : header_p - pointer to the content of header info
 *            *pud_p   - the oontent pointer of the flood packet
 *            pdu_len  - the pdu lenght of the flood packet
 * OUTPUT   : None
 * RETURN   : TRUE  - packet is filtered, no further processing is needed
 *            FALSE - need to do egerss processing
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_DoIngProcForUniPdu(
    CFM_ENGINE_PduHead_T    *header_p,
    UI8_T                   *pdu_p,
    UI32_T                  pdu_len)
{
    CFM_TYPE_MpType_T   mp_type=CFM_TYPE_MP_TYPE_NONE;
    CFM_OM_MEP_T        *mep_p=&mep_tmp1_g;
    CFM_OM_MEP_T        *high_level_mep_p=&mep_tmp2_g;
    CFM_OM_MIP_T        mip;
    BOOL_T              ret = FALSE;

    /* ingress port is blocking, should not need to do forwarding
     */
    if (FALSE == header_p->is_ing_lport_fwd)
    {
        CFM_BD_MSG(header_p->pkt_dbg_flag,
            "dropped due to ingress port is blocked, lport/level(%ld/%d)",
            (long)header_p->lport, header_p->md_level);
        return TRUE;
    }

    /* 1. drop the packet if the md level can not be decided
     *    according to 19.2.6 (a), MP Level Demultiplexer
     */
    if(header_p->md_level > CFM_TYPE_MD_LEVEL_7)
    {
        CFM_BD_MSG(header_p->pkt_dbg_flag,
            "ingress do nothing, lport/level(%ld/%d) can not be decided",
            (long)header_p->lport, header_p->md_level);
        ret = TRUE;
    }

    if (FALSE == ret)
    {
        /* 2. check if ingress port has mp to process
         *    a. MEP,  drop, bcz this unicast is not for us
         *    b. MIP,  go to egress
         *    c. NONE, if higher level mp exists drop
         *             else go to egress
         */
        mp_type= CFM_ENGINE_GetMpProcessRcvdPacket(header_p, mep_p, &mip);
        switch(mp_type)
        {
        case CFM_TYPE_MP_TYPE_MEP:
            CFM_BD_MSG(header_p->pkt_dbg_flag,
                "drop, bcz it's not for this MEP(lport/id-%ld/%ld)",
                (long)mep_p->lport, (long)mep_p->identifier);
            ret = TRUE;
            break;
        case CFM_TYPE_MP_TYPE_MIP:
            CFM_BD_MSG(header_p->pkt_dbg_flag,
                "MIP(lport-%ld), go to egress", (long)mip.lport);
            break;
        case CFM_TYPE_MP_TYPE_NONE:
        default:
            if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                        header_p->lport,            header_p->md_level,
                        header_p->tag_info&0x0fff,  CFM_TYPE_MP_DIRECTION_UP_DOWN,
                        FALSE,                      high_level_mep_p,
                        NULL))
            {
                CFM_BD_MSG(header_p->pkt_dbg_flag,
                    "drop, higher level MEP exists(lport-%ld)", (long)header_p->lport);
                /* packet is dropped by higher level mep...
                 */
                ret = TRUE;
            }
            break;
        }
    }

    /*  TRUE: this packet is filtered by MP/LEVEL checking
     */
    CFM_BD_MSG(header_p->pkt_dbg_flag,
        "go through egress processing (%c)", (TRUE == ret)?'F':'T');

    return ret;
}/*End of CFM_ENGINE_DoIngProcForUniPdu*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DoEgrProcForForwarding
 *-------------------------------------------------------------------------
 * PURPOSE  : To do egress processing for pdu forwarding (not for our device)
 * INPUT    : header_p    - pointer to the content of header info
 *            *pud_p      - the oontent pointer of the flood packet
 *            pdu_len     - the pdu lenght of the flood packet
 *            is_need_tag - if this packet should be always forwarded with tag
 * OUTPUT   : None
 * RETURN   : TRUE  - packet is forwarded
 *            FALSE - packet is not forwarded at all
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_DoEgrProcForForwarding(
    CFM_ENGINE_PduHead_T    *header_p,
    UI8_T                   *pdu_p,
    UI32_T                  pdu_len,
    BOOL_T                  is_need_tag)
{
    CFM_OM_MIP_T            mip;
    AMTR_TYPE_AddrEntry_T   addr_entry;
    CFM_OM_MEP_T            *mep_p=&mep_tmp1_g;
    UI32_T                  vlan_ifindex, egress_lport = 0;
    CFM_TYPE_MpType_T       mp_type=CFM_TYPE_MP_TYPE_NONE;
    BOOL_T                  is_flood = TRUE, ret = FALSE, is_mc = FALSE;

    is_mc = (0 != (header_p->dst_mac_a[0] & 0x01)) ? TRUE : FALSE;

    /* find egress port by FDB
     */
    if (FALSE == is_mc)
    {
        memcpy(addr_entry.mac, header_p->dst_mac_a, SYS_ADPT_MAC_ADDR_LEN);
        addr_entry.vid  = header_p->tag_info&0x0fff;
        if (TRUE == AMTR_MGR_GetExactAddrEntry(&addr_entry))
        {
            egress_lport = addr_entry.ifindex;
            CFM_BD_MSG(header_p->pkt_dbg_flag,
                "forward unicast pdu to port-%ld", (long)addr_entry.ifindex);

            is_flood = FALSE;
        }
    }

    if (TRUE == is_flood)
    {
        CFM_BD_MSG(header_p->pkt_dbg_flag, "flood pdu to all port");
    }

    VLAN_OM_ConvertToIfindex(header_p->tag_info&0x0fff, &vlan_ifindex);

    /* check each egress port:
     *  if higher level mp exists, drop
     *  else forward it out
     */
    do
    {
        if (TRUE == is_flood)
        {
            if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_GetNextLogicalPort(&egress_lport))
            {
                break;
            }
        }

        /* ingress port shall not the same as egress port
         */
        if(egress_lport == header_p->lport )
        {
            continue;
        }

        /* check whether the port is vlan's member */
        if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, egress_lport))
        {
            continue;
        }

        /* 2. check if egress port has any mp
         *    a. MEP,  drop, bcz this unicast is not for us
         *    b. MIP,  go to transmit
         *    c. NONE, if higher level mp exists drop
         *             else transmit
         */
        mp_type= CFM_ENGINE_GetMpProcessRcvdPacket_Ex(
                    header_p->md_level, (header_p->tag_info&0x0fff),
                    egress_lport, mep_p, &mip);

        switch(mp_type)
        {
        case CFM_TYPE_MP_TYPE_MEP:
            CFM_BD_MSG(header_p->pkt_dbg_flag,
                "drop at egress, bcz it's not for this MEP(port-%ld)", (long)egress_lport);
            continue;
            break;

        case CFM_TYPE_MP_TYPE_MIP:
            /* If the destination_address parameter contains a Group address
             * and the MP Loopback Responder state machine resides in an MHF
             * (rather than in a MEP), ProcessLBM() discards the LBM and
             * performs no further processing;
             *
             * for Y.1731, A MIP is transparent to the Multicast frames
             * with ETH-LB request information, so we need to forward multicast LBM
             */
#if 0
            if (  (CFM_TYPE_OPCODE_LBM == header_p->op_code)
                &&(0x01 == header_p->dst_mac_a[0])
               )
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "drop packet, SA is a Group address");
                continue;
            }
#endif
            break;

        case CFM_TYPE_MP_TYPE_NONE:
        default:
            /* in current design, MIP will not block CFM PDU of lower levels.
             */
            if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                        egress_lport,               header_p->md_level,
                        header_p->tag_info&0x0fff,  CFM_TYPE_MP_DIRECTION_UP_DOWN,
                        FALSE,                      mep_p,
                        NULL))
            {
                /* packet is dropped by higher level mep...
                 */
                CFM_BD_MSG(header_p->pkt_dbg_flag,
                    "drop by higher level MEP at egress(port-%ld)", (long)egress_lport);
                continue;
            }
            break;
        }

        CFM_BD_MSG(header_p->pkt_dbg_flag,
            "xmit pdu to egress(port-%ld)", (long)egress_lport);

        CFM_ENGINE_XmitPDU(
            egress_lport, header_p->tag_info&0x0fff, header_p->md_level,
            (header_p->tag_info&0xf000)>>13, is_need_tag, header_p->dst_mac_a,
            header_p->src_mac_a, CFM_TYPE_MP_DIRECTION_UP, pdu_p, header_p->pdu_length);

        ret = TRUE;

    } while (TRUE == is_flood);

    return ret;
}/*End of CFM_ENGINE_DoEgrProcForForwarding*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_IsDstMacForUs
 *-------------------------------------------------------------------------
 * PURPOSE  : To check if dst mac is DUT's cpu mac or port mac
 * INPUT    : ing_lport - ingress lport
 *            dst_mac_p - pointer to destination mac to check
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_IsDstMacForUs(
    UI32_T  ing_lport,
    UI8_T   *dst_mac_p)
{
    UI8_T   port_mac_a[SYS_ADPT_MAC_ADDR_LEN] ={0},
            cpu_mac_a[SYS_ADPT_MAC_ADDR_LEN] ={0};
    BOOL_T  ret = FALSE;

    SWCTRL_GetPortMac(ing_lport, port_mac_a);
    SWCTRL_GetCpuMac(cpu_mac_a);

    if (  (0 == memcmp(port_mac_a, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN))
        ||(0 == memcmp(cpu_mac_a,  dst_mac_p, SYS_ADPT_MAC_ADDR_LEN))
       )
    {
        ret = TRUE;
    }
    else
    {
        UI32_T  lport =0;
        while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
        {
            if (ing_lport == lport)
                continue;

            SWCTRL_GetPortMac(lport, port_mac_a);
            if (0 == memcmp(port_mac_a, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN))
            {
                ret = TRUE;
                break;
            }
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ForwardUnicastPdu
 *-------------------------------------------------------------------------
 * PURPOSE  : the system needn't to handle this PDU, so forward to the
 *              destination or flood to all ports
 * INPUT    : header_p - pointer to the content of header info
 *            *pud_p   - the oontent pointer of the flood packet
 *            pdu_len  - the pdu lenght of the flood packet
 * OUTPUT   : None
 * RETURN   : TRUE  - packet is forwarded, does not need to do further processing
 *            FALSE - packet should be processed by our engine
 * NOTE     :
 *            1. only forward to non-blocking port
 *            2.
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ForwardUnicastPdu(
    CFM_ENGINE_PduHead_T    *header_p,
    UI8_T                   *pdu_p,
    UI32_T                  pdu_len,
    BOOL_T                  is_need_tag)
{
    BOOL_T  ret = FALSE;

    if (FALSE == CFM_ENGINE_IsDstMacForUs(header_p->lport, header_p->dst_mac_a))
    {
        /* this packet is not for this device,
         *  try to forward if conditional checking is ok
         */
        if (FALSE == CFM_ENGINE_DoIngProcForUniPdu(header_p, pdu_p, pdu_len))
        {
            CFM_ENGINE_DoEgrProcForForwarding(
                header_p, pdu_p, pdu_len, is_need_tag);
        }

        ret = TRUE;
    }

    return ret;
}/*End of CFM_ENGINE_ForwardUnicastPdu*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DecomposeCommonHeader
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the common head content of the packet
 * INPUT    : *pdu_p    - the packet content pointer
 * OUTPUT   : *header_p - the common head pointer
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_DecomposeCommonHeader(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p)
{
    header_p->md_level =(((UI8_T)*pdu_p)>>5 );
    header_p->version =(((UI8_T)*pdu_p)&0x1f);
    header_p->op_code= (UI8_T)(*(pdu_p+1));
    header_p->flags= (UI8_T)(*(pdu_p+2));
    header_p->first_tlv_offset= (UI8_T)(*(pdu_p+3));

    switch (header_p->op_code)
    {
    case CFM_TYPE_OPCODE_CCM:
        header_p->pkt_dbg_flag = CFM_BACKDOOR_DEBUG_FLAG_CCM;
        break;
    case CFM_TYPE_OPCODE_LBR:
    case CFM_TYPE_OPCODE_LBM:
        header_p->pkt_dbg_flag = CFM_BACKDOOR_DEBUG_FLAG_LB;
        break;
    case CFM_TYPE_OPCODE_LTR:
    case CFM_TYPE_OPCODE_LTM:
        header_p->pkt_dbg_flag = CFM_BACKDOOR_DEBUG_FLAG_LT;
        break;
    case CFM_TYPE_OPCODE_AIS:
        header_p->pkt_dbg_flag = CFM_BACKDOOR_DEBUG_FLAG_AIS;
        break;
    case CFM_TYPE_OPCODE_DMR:
    case CFM_TYPE_OPCODE_DMM:
        header_p->pkt_dbg_flag = CFM_BACKDOOR_DEBUG_FLAG_DM;
        break;
    default:
        header_p->pkt_dbg_flag = CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW;
        break;
    }
    return TRUE;
}/*End of CFM_ENGINE_DecomposeCommonHeader*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructCCMPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will construct the CCM packet
 * INPUT    : *pdu_p            - the array which use to store the contructed CCM packet
 *            *pdu_len_p        - the CCM packet length after contruct
 *            *mep_p            - the mep pointer which contruct the mep
 *            *port_status_notify  - the port port status notify information
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructCCMPDU(
                                        UI8_T *pdu_p,
                                        UI16_T *pdu_len_p,
                                        CFM_OM_MEP_T *mep_p,
                                        CFM_ENGINE_PortStatusCallback_T *port_status_notify)
{
    {/*1. constrcut common header, common header has 4 bytes, the end of tlv will be add at the end of pdu*/
        CFM_TYPE_FNG_HighestDefectPri_T     cur_max_err_pri;
        UI8_T                               flag=0;

        /*20.9.3 and 21.6.1.1*/

        /* 20.9.6
         * presentRDI is true if and only if one or more of the variables
         * someRMEPCCMdefect, someMACstatusDefect, errorCCMdefect,
         * or xconCCMdefect is true, and if the corresponding priority
         * of that variable
         */
        cur_max_err_pri = CFM_OM_GetCurrentMaxErrPri(mep_p, TRUE);
        if ((long)cur_max_err_pri >= (long)mep_p->lowest_priority_defect)
        {
            if(mep_p->ma_p->cross_check_status ==CFM_TYPE_CROSS_CHECK_STATUS_ENABLE)
            {
                flag|=(0x01<<7); /* RDI flag */
            }
        }

        /*interval*/
        flag|=((UI8_T)mep_p->ma_p->ccm_interval)&0x07;

        CFM_ENGINE_ConstructCommonHeader(pdu_p, pdu_len_p, mep_p->md_p->level,flag, CFM_TYPE_OPCODE_CCM);

        /*move to next octect*/
        pdu_p+=*pdu_len_p;
    }
    {/*2. sequnece number*/
        UI32_T seq=0;

        seq=mep_p->cci_sent_ccms;
        seq=L_STDLIB_Hton32(seq);

        memcpy(pdu_p, &seq, 4);

        /*move to next octect and increase len*/
        pdu_p+=4;
        *pdu_len_p+=4;

    }

    {/*3. construct mepid*/
        UI16_T mep_id=L_STDLIB_Hton16(mep_p->identifier);
        memcpy(pdu_p, &mep_id, 2);

        /*move to next octect and increase len*/
        pdu_p+=2;
        *pdu_len_p+=2;
    }

    {/*4. construct maid*/
        if(CFM_TYPE_MA_NAME_ICC_BASED == mep_p->ma_p->format)
        {
            /* for Y.1731, ICC-BASED
             *  according to Y.1731 Annex A, Figure A.1
             */
            memset(pdu_p, 0, CFM_TYPE_MAID_NAME_LENGTH);

            *pdu_p     = CFM_TYPE_MD_NAME_NONE;
            *(pdu_p+1) = CFM_TYPE_MA_NAME_ICC_BASED;
            *(pdu_p+2) = CFM_TYPE_MA_MAX_NAME_LENGTH_FOR_Y1731;
            memcpy((pdu_p+3), mep_p->ma_p->name_a, mep_p->ma_p->name_length);
        }
        else
        {
        *pdu_p=(UI8_T)mep_p->md_p->format;
        if(CFM_TYPE_MD_NAME_NONE==mep_p->md_p->format)
        {
            *(pdu_p+1)=(UI8_T)mep_p->ma_p->format;
            *(pdu_p+2)=(UI8_T)mep_p->ma_p->name_length;
            memcpy((pdu_p+3), mep_p->ma_p->name_a, mep_p->ma_p->name_length);
        }
        else
        {
            UI8_T *ma_start=NULL;

            ma_start=pdu_p+2+mep_p->md_p->name_length;

            /*copy md*/
            *(pdu_p+1)=(UI8_T)mep_p->md_p->name_length;
            memcpy((pdu_p+2), mep_p->md_p->name_a, mep_p->md_p->name_length);

            /*copy ma*/
            *ma_start=mep_p->ma_p->format;
            *(ma_start+1)=(UI8_T)mep_p->ma_p->name_length;
            memcpy((ma_start+2), mep_p->ma_p->name_a, mep_p->ma_p->name_length);
            }
        }

        /*move to next octect and increase len*/
        /*maid has 48 bytes,  although it may not use them all*/
        *pdu_len_p+=CFM_TYPE_MAID_NAME_LENGTH;
        pdu_p+=CFM_TYPE_MAID_NAME_LENGTH;
    }
    {/*5. 59-74, 16 bytes, define by ITU-T Y.1731*/
        /*still not defined, so not implemented*/
        pdu_p+=CFM_TYPE_CCM_ITU_FIELD_LENGTH;
        *pdu_len_p+=CFM_TYPE_CCM_ITU_FIELD_LENGTH;
    }
    {/*6. construct tlv optionally*/
        UI16_T tlv_len=0;

        /* for BCM ASIC, the port status and interface status TLV can be
         * recognized if they are put at the first place
         */
        /*construct port status tlv*/
        /* 21.5.4 A MEP that is configured in a Bridge in a position that
         * is not associated with a single value for the Port State
         * and VID member set shall not transmit the Port Status TLV.
         *
         * this should not be our case...
         */
        CFM_ENGINE_ConstructPortStatusTLV(pdu_p, &tlv_len, mep_p->primary_vid, mep_p->lport, mep_p->direction, port_status_notify);
        pdu_p+=tlv_len;
        *pdu_len_p+=tlv_len;

        /*construct interface status tlv*/
        CFM_ENGINE_ConstructInterfaceStatusTLV( pdu_p, &tlv_len, mep_p->lport, mep_p->direction, port_status_notify);
        pdu_p+=tlv_len;
        *pdu_len_p+=tlv_len;

        /*construct sender tlv*/
        CFM_ENGINE_ConstructSenderTLV(pdu_p, &tlv_len, mep_p->lport);
        pdu_p+=tlv_len;
        *pdu_len_p+=tlv_len;

        /* 21.6 Continuity Check Message format
         * An MP shall be able to receive and process any valid CCM PDU that
         * is 128 octets in length or less, starting with the MD Level/Version
         * octet, and including the End TLV.. An MP shall not transmit a CCM PDU
         * exceeding this length.
         *
         * currently it may exceed this length after sender tlv is included,
         * so the org tlv will be skipped if no enough space left.
         *  9 - for org tlv SUBTYPE_PORT_VLAN_ID
         *  1 - for end tlv
         */
        if (*pdu_len_p + 9 + 1 <= 128)
        {
            /*construct orginization tlv*/
            CFM_ENGINE_ConstructOrginizationTLV(pdu_p, &tlv_len, mep_p->lport, CFM_TYPE_ORGANIZATION_SUBTYPE_PORT_VLAN_ID);
            pdu_p+=tlv_len;
            *pdu_len_p+=tlv_len;
        }

        /*7. construct end tlv*/
        CFM_ENGINE_ConstructEndTLV(pdu_p, &tlv_len);
        *pdu_len_p+=tlv_len;
    }

}/*End of CFM_ENGINE_ConstructCCMPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructCommonHeader
 *------------------------------------------------------------------------
 * PURPOSE  : This function will contructio the common header
 * INPUT    : *pdu_p   - the packet content pointer which will
 *                       store the common header
 *            *pdu_len - the packet content length now
 *             level   - the md level
 *             flag    - the flag in header
 *            opcode   - the opcode which will carry in the common header
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructCommonHeader(
                                            UI8_T *pdu_p,
                                            UI16_T *pdu_len,
                                            UI8_T level,
                                            UI8_T flag,
                                            CFM_TYPE_OpCode_T opcode)
{

    /*md leve*/
    {
        UI8_T md_leve_ver=0;
        md_leve_ver=level<<5;
        *pdu_p=md_leve_ver;

        /*move to next octect and increase len*/
        *pdu_len+=1;
        pdu_p+=1;
    }
    {/*op code*/
        *pdu_p=opcode;

        /*move to next octect and increase len*/
        *pdu_len+=1;
        pdu_p+=1;
    }
    {/*flags*/
        *pdu_p=flag;

        /*move to next octect and increase len*/
         *pdu_len+=1;
         pdu_p+=1;
    }
    {/*first tlv off set */
        switch(opcode)
        {
            case CFM_TYPE_OPCODE_CCM:
                *pdu_p=70;
                break;
            case CFM_TYPE_OPCODE_LBM:
            case CFM_TYPE_OPCODE_LBR:
                *pdu_p=4;
                break;
            case CFM_TYPE_OPCODE_LTM:
                *pdu_p=17;
                break;
            case CFM_TYPE_OPCODE_LTR:
                *pdu_p=6;
                break;
            case CFM_TYPE_OPCODE_DMM:
            case CFM_TYPE_OPCODE_DMR:
                *pdu_p=32;
                break;
            case CFM_TYPE_OPCODE_AIS:
            default:
                *pdu_p=0;
                break;
        }

        /*move to next octect and increase len*/
        pdu_p+=1;
        *pdu_len+=1;
    }
    {/*end of tlv not do here, it do after pdu finished input tlv*/


    }
    return;
}/*End of CFM_ENGINE_ConstructCommonHeader*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructSenderTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will contruct the snder TLV
 * INPUT    : *pdu_p   - the packet content pointer which use to store the sender tlv
 *            *tlv_len - the sender tlv lenght pointer
 *             lport   - the logical port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : implement v8.1 sendter tlv
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructSenderTLV(
                                        UI8_T *pdu_p,
                                        UI16_T *tlv_len,
                                        UI32_T lport)
{
    NETCFG_TYPE_InetRifConfig_T  rif_config;
    UI16_T sender_len=0;
    UI8_T  chassis_subtype_len=0, *sender_len_p=NULL;
    BOOL_T ip_exist=FALSE;

    /*creat chassis id*/
    *pdu_p=(UI8_T)CFM_TYPE_TLV_SENDER_ID;                 /*1 byte*/

    sender_len_p=(pdu_p+1);                             /*send tlv length 2 bytes*/

    chassis_subtype_len=SYS_ADPT_MAC_ADDR_LEN;            /*we use the mac address*/
    *(pdu_p+3)=chassis_subtype_len;                       /* chassis id length 1 byte*/
    *(pdu_p+4)=CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_MAC_ADDR;  /*chassis id subtype 1 byte*/
    SWCTRL_GetCpuMac(pdu_p+5);                            /*use the port cpu mac as the chassis id*/

    sender_len+=(2+chassis_subtype_len);                  /*chassis id length field + chassis id subtype field + chassisID length*/

    /*move to management length field*/
    pdu_p+=(sender_len+3);

    memset(&rif_config, 0, sizeof(rif_config));

    while(NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextRifConfig(&rif_config))
    {
        if(TRUE == VLAN_OM_IsPortVlanMember(rif_config.ifindex, lport))
        {
            ip_exist=TRUE;
            *pdu_p = CFM_ENGINE_ASN1_TRANS_DOMAIN_OID_LEN;
            memcpy(pdu_p +1, asn1_transport_domain_oid[0], CFM_ENGINE_ASN1_TRANS_DOMAIN_OID_LEN);
            sender_len  += CFM_ENGINE_ASN1_TRANS_DOMAIN_OID_LEN + 1;
            pdu_p       += CFM_ENGINE_ASN1_TRANS_DOMAIN_OID_LEN + 1;

            *pdu_p = 4; /*ip addr len*/
            memcpy(pdu_p +1, rif_config.addr.addr, sizeof(UI32_T));

            sender_len += sizeof(UI32_T) +1 /*left is mgmt addr*/;
            break;
        }
    }

    if(FALSE== ip_exist)
    {/* if there is no ip exist*/
        *pdu_p = 0;
        sender_len+=1;
    }

    {/*write the sender tlv length*/
        UI16_T tmp16=0;

        tmp16= L_STDLIB_Hton16(sender_len);
        memcpy(sender_len_p, &tmp16, sizeof(UI16_T));

        *tlv_len=sender_len+3;
    }

    return;
}/*CFM_ENGINE_ConstructSenderTLV*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructPortStatusTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will create the port status tlv
 * INPUT    : *pdu_p              - the packet content pointer which use to store the tlv
 *            *tlv_len            - the sender tlv lenght pointer
 *            vid                - the port's vid
 *            lport              - the logical port
 *            direction           - mp directon
 *            port_status_notify  - the port status change notify information
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructPortStatusTLV(
                                            UI8_T *pdu_p,
                                            UI16_T *tlv_len,
                                            UI16_T vid,
                                            UI32_T lport,
                                            CFM_TYPE_MP_Direction_T direction,
                                            CFM_ENGINE_PortStatusCallback_T *port_status_notify)
{
    UI16_T t_len=0;

    *pdu_p=(UI8_T)CFM_TYPE_TLV_PORT_STATUS;
    t_len=L_STDLIB_Hton16(1);
    memcpy(pdu_p +1, &t_len, sizeof(UI16_T));

/* let UP MEP use current forwarding state to construct
 *   port status TLV
 */
#if 0
    if(CFM_TYPE_MP_DIRECTION_UP==direction)
    {
        *(pdu_p+3)=(UI8_T)CFM_TYPE_PORT_STATUS_UP;
        *tlv_len=4;

        return;
    }
#endif

    /*if the construct is according to the callback notify*/
    if(NULL != port_status_notify)
    {
        if( (FALSE == port_status_notify->xstp_forwarding)||(TRUE == port_status_notify->admin_disable))
        {
            *(pdu_p+3)=(UI8_T)CFM_TYPE_PORT_STATUS_BLOCKED;
        }
        else
        {
            *(pdu_p+3)=(UI8_T)CFM_TYPE_PORT_STATUS_UP;
        }
        *tlv_len=4;

        return;
    }

    /*ask the xstp to the get port status*/
    if(FALSE == XSTP_OM_IsPortForwardingStateByVlan(vid, lport))
    {
        *(pdu_p+3)=(UI8_T)CFM_TYPE_PORT_STATUS_BLOCKED;
    }
    else
    {
        *(pdu_p+3)=(UI8_T)CFM_TYPE_PORT_STATUS_UP;
    }

    *tlv_len=4;

    return;
}/* End of CFM_ENGINE_ConstructPortStatusTLV*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructInterfaceStatusTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : This functioin will creat the interface tlv
 * INPUT    : *pdu_p            - the packet content pointer which use to store the tlv
 *            *tlv_len          - the sender tlv lenght pointer
 *            lport             - the logical port
 *            direction         - mp direction
 *            port_status_notify  - the port status change notify information
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructInterfaceStatusTLV(
                                                    UI8_T *pdu_p,
                                                    UI16_T *tlv_len,
                                                    UI32_T lport,
                                                    CFM_TYPE_MP_Direction_T direction,
                                                    CFM_ENGINE_PortStatusCallback_T *port_status_notify)
{
    UI32_T port_state=CFM_TYPE_INTERFACE_STATUS_NO_INTERFACE_STATUS_TLV;
    UI16_T t_len=0;

    *pdu_p=(UI8_T)CFM_TYPE_TLV_INTERFACE_STATUS;
    t_len=L_STDLIB_Hton16(1);
    memcpy(pdu_p +1, &t_len, sizeof(UI16_T));

/* let UP MEP use current oper status to construct
 *   interface status TLV
 */
#if 0
    if(CFM_TYPE_MP_DIRECTION_UP==direction)
    {
        *(pdu_p+3)=(UI8_T)CFM_TYPE_INTERFACE_STATUS_UP;
        *tlv_len=4;

        return;
    }
#endif

    if(NULL != port_status_notify)
    {
        if(TRUE == port_status_notify->admin_disable)
        {
            *(pdu_p+3)=(UI8_T)CFM_TYPE_INTERFACE_STATUS_DOWN;
            *tlv_len=4; /*this tlv has 4 bytes*/
            return;
        }
    }

    if(FALSE == SWCTRL_GetPortOperStatus( lport, &port_state))
    {
        *(pdu_p+3)=(UI8_T)CFM_TYPE_INTERFACE_STATUS_LOWERLAYERDOWN;
    }
    else
    {
        *(pdu_p+3)=port_state;
    }

    *tlv_len=4; /*this tlv has 4 bytes*/

    return;
}/* End of CFM_ENGINE_ConstructInterfaceStatusTLV*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructDataTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : This funciton will create the Data tlv
 * INPUT    : pdu_p    - the packet content pointer which use to store the tlv
 *            tlv_len  - the sender tlv lenght pointer
 *            data_len - the empty data lenght will be put into the packet
 *            pattern  - the pattern included in data TLV
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructDataTLV(
    UI8_T   *pdu_p,
    UI16_T  *tlv_len,
    UI16_T  data_len,
    UI16_T  pattern)
{
    UI16_T  d_len=0;
    UI8_T   pattern_be[2];

    *pdu_p=(UI8_T)CFM_TYPE_TLV_DATA;

    d_len=L_STDLIB_Hton16(data_len);
    memcpy(pdu_p+1, &d_len, sizeof(UI16_T));

    *tlv_len=3+data_len; /*this tlv bytes are count type(1)+length(2)+data(data_len)*/

    /* fill the data with specifed pattern
     */
    pattern_be[0] = (pattern >> 8) & 0xff;
    pattern_be[1] = pattern & 0xff;
    pdu_p +=3;
    while (data_len >=2)
    {
        pdu_p[0] = pattern_be[0];
        pdu_p[1] = pattern_be[1];

        pdu_p    += 2;
        data_len -= 2;
    }

    if (data_len > 0)
    {
        pdu_p[0] = pattern_be[0];
    }
}/*End of CFM_ENGINE_ConstructDataTLV*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructEndTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will create the end tlv
 * INPUT    : *pdu_p   - the packet content pointer which use to store the sender tlv
 *            *tlv_len - the sender tlv lenght pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructEndTLV(
                                        UI8_T *pdu_p,
                                        UI16_T *tlv_len)
{
    *pdu_p=(UI8_T)CFM_TYPE_TLV_END;
    *tlv_len=1;

    return;
}/* End of CFM_ENGINE_ConstructEndTLV*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructOrginizationTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will contruct the organization tlv
 * INPUT    : *pdu_p   - the packet content pointer which use to store the tlv
 *            *tlv_len - the sender tlv lenght pointer
 *            lport  -the logical port
 *            subtype -the organization subtype
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructOrginizationTLV(
    UI8_T                           *pdu_p,
    UI16_T                          *tlv_len,
    UI32_T                          lport,
    CFM_TYPE_OrganizationSubtype_T  subtype)
{
    UI16_T tlv_length=0, pvid=0;

    *pdu_p=(UI8_T)CFM_TYPE_TLV_ORGANIZATION;
    pdu_p[6]=(UI8_T)subtype;

    /* OUI: 0080C2, for IEEE VLAN related TLV
     */
    switch(subtype)
    {
        case CFM_TYPE_ORGANIZATION_SUBTYPE_PORT_VLAN_ID:
        {
            VLAN_OM_Vlan_Port_Info_T vlan_port_info;

            memset(&vlan_port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));

            VLAN_MGR_GetPortEntry(lport,  &vlan_port_info);

            /*assign oui*/
            pdu_p[3]=0x00;
            pdu_p[4]=0x80;
            pdu_p[5]=0xc2;

            /*assign pvid*/
            pvid=(UI16_T)vlan_port_info.port_item.dot1q_pvid_index;
            pvid=L_STDLIB_Hton16(pvid);
            memcpy(pdu_p+7, &pvid, sizeof(UI16_T));

            /*assign tlv length*/
            tlv_length=3+1+2; /*oui+subtype+pvid*/
            break;
        }
        case CFM_TYPE_ORGANIZATION_SUBTYPE_PORT_PROTOCOL_VLAN_ID:
        {
            /*assign oui*/
            pdu_p[3]=0x00;
            pdu_p[4]=0x80;
            pdu_p[5]=0xc2;

            /*assign flag, 1 byte*/
            pdu_p[7]=0;   /*no supported*/

            /*assign pvid, 2 byte*/
            memset(pdu_p+8, 0, sizeof(UI16_T));

            /*assign tlv length*/
            tlv_length=3+1+1+2; /*oui+subtype+flag+pvid*/

            break;
        }
        case CFM_TYPE_ORGANIZATION_SUBTYPE_VLAN_NAME:
        {
            VLAN_OM_Vlan_Port_Info_T vlan_port_info;
            VLAN_MGR_Dot1qVlanStaticEntry_T vlan_info;

            memset(&vlan_port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
            memset (&vlan_info, 0, sizeof(VLAN_MGR_Dot1qVlanStaticEntry_T));

            VLAN_MGR_GetPortEntry(lport,  &vlan_port_info);

            /*assign oui, 3 bytes*/
            pdu_p[3]=0x00;
            pdu_p[4]=0x80;
            pdu_p[5]=0xc2;

            /*assign pvid, 2 types*/
            pvid=(UI16_T)vlan_port_info.port_item.dot1q_pvid_index;
            pvid=L_STDLIB_Hton16(pvid);
            memcpy(pdu_p+7, &pvid, sizeof(UI16_T));

            if(TRUE == VLAN_OM_GetDot1qVlanStaticEntry(vlan_port_info.port_item.dot1q_pvid_index, &vlan_info))
            {
                /*assign vlan name length, 1 btye*/
                pdu_p[9]=strlen(vlan_info.dot1q_vlan_static_name);
                memcpy(pdu_p+10, (UI8_T *)vlan_info.dot1q_vlan_static_name, pdu_p[9]);

                /*assign tlv length*/
                tlv_length=3+1+2+1+pdu_p[9]; /*oui+subtype+pvid+vlan name length+vlan name*/
            }
            else
            {
                /*assign vlan name length, 1 btye*/
                pdu_p[9]=0;

                /*assign tlv length*/
                tlv_length=3+1+2+1; /*oui+subtype+pvid+vlan name length+vlan name*/
            }

            break;
        }
        case CFM_TYPE_ORGANIZATION_SUBTYPE_PROTOCOL_ID:
        {
            /*assign oui*/
            pdu_p[3]=0x00;
            pdu_p[4]=0x80;
            pdu_p[5]=0xc2;

            /*assign flag, 1 byte*/
            pdu_p[7]=0;   /*no supported*/

            /*assign tlv length*/
            tlv_length=3+1+1; /*oui+subtype+protocol length*/
            break;
        }
        default:
            return;
    }

    {
        UI16_T tmp16=0;

        tmp16=L_STDLIB_Hton16(tlv_length);
        memcpy(pdu_p+1, &tmp16, sizeof(UI16_T));
    }
    *tlv_len=tlv_length+1+2; /* + tlv type + tlv length field*/
}/* End of CFM_ENGINE_ConstructOrginizationTLV*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DecomposeAndVerifySenderTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : This function decompose and verify the sender tlv
 * INPUT    : header_p     - the common head pointer
 *            sender_tlv_p - the sender tlv pointer
 *            tlv_p        - the tlv to decompose and verify
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_DecomposeAndVerifySenderTlv(
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_SenderTlv_T  *sender_tlv_p,
    UI8_T                   *tlv_p)
{
    UI16_T sender_length=0;

    sender_tlv_p->type_p=tlv_p;
    sender_tlv_p->length_p=tlv_p+1;
    memcpy(&sender_length, sender_tlv_p->length_p, sizeof(UI16_T));
    sender_length=L_STDLIB_Ntoh16(sender_length);

    sender_tlv_p->chassis_id_length_p=tlv_p+3;

    if(0 != *sender_tlv_p->chassis_id_length_p)
    {
        if(*sender_tlv_p->chassis_id_length_p +2/*chassis_id_len+chassis_id_subtyp*/ >sender_length)
        {
            CFM_BD_MSG(header_p->pkt_dbg_flag, "sender tlv chassis id len is wrong");
            return FALSE;
        }

        sender_tlv_p->chassis_id_sub_type_p=tlv_p+4;
        sender_tlv_p->chassis_id_p=tlv_p+5;

        /*locate management domain addres length address*/
        sender_tlv_p->mgmt_addr_domain_len_p = tlv_p+3/*sender id + tlv len*/+1/*subtype*/+1/*chassis id*/
                                              +*sender_tlv_p->chassis_id_length_p;
    }
    else /*no chassis id*/
    {
        CFM_BD_MSG(header_p->pkt_dbg_flag, "sender tlv no chassis id len");
        /*locate management domain addres length address*/
        sender_tlv_p->mgmt_addr_domain_len_p = tlv_p+3/*sender id + tlv len*/+1;
    }

    if(0 != *sender_tlv_p->mgmt_addr_domain_len_p)
    {
        if(*sender_tlv_p->mgmt_addr_domain_len_p +
            *sender_tlv_p->chassis_id_length_p +2/*chassis_id_len+chassis_id_subtyp*/ >sender_length)
        {
            CFM_BD_MSG(header_p->pkt_dbg_flag, "sender tlv mgmt addr domain len and chassis id len is wrongl ");
            return FALSE;
        }

        sender_tlv_p->mgmt_addr_domain_p = sender_tlv_p->mgmt_addr_domain_len_p+1;

        sender_tlv_p->mgmt_addr_len_p = sender_tlv_p->mgmt_addr_domain_p +*sender_tlv_p->mgmt_addr_domain_len_p;

        if(0 != *sender_tlv_p->mgmt_addr_len_p )
        {
            if(*sender_tlv_p->mgmt_addr_len_p +
                *sender_tlv_p->mgmt_addr_domain_len_p +
                *sender_tlv_p->chassis_id_length_p +2/*chassis_id_len+chassis_id_subtyp*/ >sender_length)
            {
                CFM_BD_MSG(header_p->pkt_dbg_flag, "sender tlv mgmt addr len and domain len and chassis id  len faill ");
                return FALSE;
            }

            sender_tlv_p->mgmt_addr_p = sender_tlv_p->mgmt_addr_len_p +1;
        }
    }

    return TRUE;

}/*End of CFM_ENGINE_DecomposeAndVerifySenderTlv*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DecomposeAndVerifyOrganizationTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : This function decompose and verify the organization tlv
 * INPUT    : *sender_tlv_p - the sender tlv pointer
 *           *tlv_p      - the tlv to decompose and verify
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_DecomposeAndVerifyOrganizationTlv(
                                                        CFM_ENGINE_OrganizationTlv_T *org_tlv_p,
                                                        UI8_T *tlv_p)
{
    org_tlv_p->type_p=tlv_p;
    org_tlv_p->length_p=tlv_p+1;
    org_tlv_p->OUI_p=tlv_p+3;
    org_tlv_p->Subtype_p=tlv_p+6;
    org_tlv_p->Value_p=tlv_p+7;

    return TRUE;
}/*End of */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMpProcessRcvdPacket_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the mep pointer from om
 * INPUT    : md_level - the md level to get
 *            vid      - the vid to get
 *            lport    - the lport to get mep
 * OUTPUT   : mep_p    - the mep pointer from the om
 *            mip_p    - the mip pointer from the om
 * RETURN   : CFM_TYPE_MP_TYPE_NONE - there is no mp on this port
 *            CFM_TYPE_MP_TYPE_MEP  - there is mep on this port
 *            CFM_TYPE_MP_TYPE_MIP  - there is mip on this port
 * NOTE     : This is get the mep and mip pointer store in the om, so be carful use this
 *           This function is used for process the received packet
 *           1. get the received port's mep first.
 *-------------------------------------------------------------------------
 */
static CFM_TYPE_MpType_T CFM_ENGINE_GetMpProcessRcvdPacket_Ex(
    UI16_T          md_level,
    UI16_T          vid,
    UI32_T          lport,
    CFM_OM_MEP_T    *mep_p,
    CFM_OM_MIP_T    *mip_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;

    /*get the md and ma index*/
    if (FALSE == CFM_OM_GetMdMaByLevelVid(md_level, vid,  &md_p, &ma_p))
    {
        return CFM_TYPE_MP_TYPE_NONE;
    }

    /*get mep on the received port*/
    if(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        return CFM_TYPE_MP_TYPE_MEP;
    }
    else if(TRUE == CFM_OM_GetMip(md_p->index, ma_p->index, lport, CFM_OM_LPORT_MD_MA_KEY, mip_p))
    {
        return CFM_TYPE_MP_TYPE_MIP;
    }

    return CFM_TYPE_MP_TYPE_NONE;
}/*End of CFM_ENGINE_GetMpProcessRcvdPacket_Ex*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetMpProcessRcvdPacket
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the mep pointer from om
 * INPUT    : *heaer_p - the common header pointer
 * OUTPUT   : *mep_p   - the mep pointer from the om
 *            *mip_p   - the mip pointer from the om
 * RETURN   : CFM_TYPE_MP_TYPE_NONE - there is no mp on this port
 *            CFM_TYPE_MP_TYPE_MEP  - there is mep on this port
 *            CFM_TYPE_MP_TYPE_MIP  - there is mip on this port
 * NOTE     : This is get the mep and mip pointer store in the om, so be carful use this
 *            This function is used for process the received packet
 *            1. get the received port's mep first.
 *-------------------------------------------------------------------------
 */
static CFM_TYPE_MpType_T CFM_ENGINE_GetMpProcessRcvdPacket(
                                                        CFM_ENGINE_PduHead_T *header_p,
                                                        CFM_OM_MEP_T *mep_p,
                                                        CFM_OM_MIP_T *mip_p)
{
    return CFM_ENGINE_GetMpProcessRcvdPacket_Ex(
                header_p->md_level, (header_p->tag_info&0x0fff), header_p->lport,
                mep_p, mip_p);

}/*End of CFM_ENGINE_GetMpProcessRcvdPacket*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FindTargetMacAndMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will find the target mac according dest mep id or dest_mac
 *            and a mep to process.
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            vid         - the vid to find the target mac
 *            dst_mep_id  - the dstnation mep id
 *            dst_mac     - the destination mac address
 *            dbg_flag    - debug flag to use
 * OUTPUT   : tartget_mac - the target mac
 *            egress_port - which port send the mac
 *            mep_p       - use which mep
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : You can choose use the dst mep id or dst mac. To use th dst mac
 *            set the dst_mep id 0
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_FindTargetMacAndMep(
    UI32_T          md_index,
    UI32_T          ma_index,
    UI16_T          vid,
    UI32_T          dst_mep_id,
    UI8_T           dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T          dbg_flag,
    UI8_T           target_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T          *egress_port,
    CFM_OM_MEP_T    *mep_p)
{
    CFM_OM_REMOTE_MEP_T remote_mep;
    UI8_T               broadcast_mac[SYS_ADPT_MAC_ADDR_LEN]={0xff,0xff,0xff,0xff,0xff,0xff};

    if ((dst_mep_id == 0) && (NULL == dst_mac))
    {
        CFM_BD_MSG(dbg_flag, "no dst_mep_id and ds_mac");
        return FALSE;
    }

    if(dst_mep_id!=0)
    {
        /*1. if the remote mep has ever received, then use the remtoe mep's mac as the dest mac
          2. if the remote mep never received, use the broadcast mac as the dest mac to flood to all port
         */
        if(TRUE == CFM_OM_GetRemoteMep(md_index, ma_index, dst_mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
        {
            if((0 == remote_mep.rcvd_mep_id)||
              (FALSE == CFM_OM_GetMep(md_index, ma_index, remote_mep.rcvd_mep_id,  0, CFM_OM_MD_MA_MEP_KEY, mep_p)))
            {
                /*find a mep to transmit*/
                if(FALSE == CFM_ENGINE_FindOperationUpMep(
                                md_index, ma_index, mep_p, NULL, 0))
                {
                    CFM_BD_MSG(dbg_flag, "can't find operation up mep");
                    return FALSE;
                }
            }

            if(memcmp(remote_mep.mac_addr_a,broadcast_mac,SYS_ADPT_MAC_ADDR_LEN))
            {
                CFM_BD_MSG(dbg_flag, "use remote mep mac as dest mac");
                memcpy(target_mac, remote_mep.mac_addr_a, SYS_ADPT_MAC_ADDR_LEN);
                *egress_port = remote_mep.rcvd_lport;
            }
            else
            {
                /*never learned remote mep info ,so used the multicast LBM*/
                CFM_BD_MSG(dbg_flag, "use the multicast mac as the dest mac");
                CFM_ENGINE_ASSEMBLE_DEST_MAC(target_mac, remote_mep.md_p->level);

                /* use mep's lport to send if it's a down mep,
                 * try to flood if it's a up mep.
                 */
                if (CFM_TYPE_MP_DIRECTION_DOWN == mep_p->direction)
                {
                    *egress_port = mep_p->lport;
                }
                else
                {
                    *egress_port = 0;
                }
            }
            return TRUE;
        }

        /*can't find the traget ma, bacause of the can't find the remote mep record*/
        CFM_BD_MSG(dbg_flag, "can't find the traget maa, bacause of the can't find the remote mep record");
        return FALSE;
    }
    else /*by mac*/
    {
        AMTR_TYPE_AddrEntry_T addr_entry;

        if(0 == vid)
        {
            CFM_BD_MSG(dbg_flag, "vid is 0");
            return FALSE;
        }

        memcpy(addr_entry.mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);
        addr_entry.vid=vid;

        if(FALSE == AMTR_MGR_GetExactAddrEntry(&addr_entry))
        {
             CFM_BD_MSG(dbg_flag, "can't find mac from amtr");
            /*can't find the port which learned the dest_mac
             */
            if(FALSE == CFM_ENGINE_FindOperationUpMep(
                            md_index, ma_index, mep_p, dst_mac, 0))
            {
                CFM_BD_MSG(dbg_flag, "can't find operation up mep");
                return FALSE;
            }

            memcpy(target_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);

            if(CFM_TYPE_MP_DIRECTION_DOWN == mep_p->direction)
            {
                *egress_port = mep_p->lport;
            }
            else
            {
                *egress_port =0;
            }
        }
        else/*find the port which learned the dest mac*/
        {
            /*find a mep to transmit the lbm
             */
            CFM_BD_MSG(dbg_flag, "find mac from amtr");

            /* find a MEP on other port if
             *  1. no MEP on mac learned port
             *  2. UP MEP on mac learned port
             */
            if (  (FALSE == CFM_OM_GetMep(md_index, ma_index, 0, addr_entry.ifindex, CFM_OM_LPORT_MD_MA_KEY, mep_p))
                ||(mep_p->direction == CFM_TYPE_MP_DIRECTION_UP)
               )
            {
                CFM_BD_MSG(dbg_flag, "can't find mep on this port ");
                if(FALSE == CFM_ENGINE_FindOperationUpMep(
                                md_index, ma_index, mep_p, dst_mac, addr_entry.ifindex))
                {
                    CFM_BD_MSG(dbg_flag, "can't find operation up mep");
                    return FALSE;
                }
            }

            memcpy(target_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);

            if(CFM_TYPE_MP_DIRECTION_DOWN == mep_p->direction)
            {
                *egress_port = mep_p->lport;
            }
            else
            {
                *egress_port =0;

                /*  UP MEP found and MAC learned are at the same port
                 */
                if (mep_p->lport == addr_entry.ifindex)
                {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}/*End of CFM_ENGINE_FindTargetMacAndMep*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FindOperationUpMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This fuction find the mep with face the operation up port.
 * INPUT    : pdu_p         - the packet content pointer
 *            header_p      - the common header pointer
 *            mep_p         - the mep pointer
 *            chk_mac_p     - pointer to the mac for comparing with the mep's
 *            lport_to_skip - lport to skip if an UP mep is found
 *                            (0 to skip the checking)
 * OUTPUT   : mep_p         - the mep's pointer
 * RETURN   : TRUE / FALSE
 * NOTE     : if the mep is down -> it will check the mep's lport's operation status
 *            if the mep is up -> it always up
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_FindOperationUpMep(
    UI32_T          md_index,
    UI32_T          ma_index,
    CFM_OM_MEP_T    *mep_p,
    UI8_T           *chk_mac_p,
    UI32_T          lport_to_skip)
{
    UI32_T nxt_mep_id=0;

    while(TRUE == CFM_OM_GetNextMep(md_index, ma_index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        if((mep_p->md_p->index!=md_index)||(mep_p->ma_p->index!=ma_index))
        {
            return FALSE;
        }

        /* need to skip the mep with the same chk_mac
         */
        if (  (NULL == chk_mac_p)
            ||(0 != memcmp(chk_mac_p, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN))
           )
        {
            if(CFM_TYPE_MP_DIRECTION_UP==mep_p->direction)
            {
                if (  (0 == lport_to_skip)
                    ||(lport_to_skip != mep_p->lport)
                   )
                {
                    return TRUE;
                }
            }
            else if((CFM_TYPE_MP_DIRECTION_DOWN==mep_p->direction)&&
                  (TRUE == SWCTRL_IsPortOperationUp(mep_p->lport)))
            {
                return TRUE;
            }
        }

        nxt_mep_id=mep_p->identifier;
    }

    return FALSE;
}/*End of CFM_ENGINE_FindOperationUpMep*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessCCM
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the received CCM packet which has the smae level of the mep
 * INPUT    : *pdu_p            - the packet content pointer
 *            *header_p         - the common header pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ProcessCCM(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p)
{
    CFM_ENGINE_CcmPdu_T     ccm;
    CFM_TYPE_MpType_T       mp_type=CFM_TYPE_MP_TYPE_NONE;
    CFM_OM_MEP_T            *mep_p=&mep_tmp1_g;
    CFM_OM_MIP_T            mip;
    CFM_OM_MD_T             *md_p  =NULL;
    CFM_OM_MA_T             *ma_p  =NULL;
    CFM_OM_MEP_T            *high_level_mep_p=&mep_tmp2_g;
    UI32_T                  lport=0;
    BOOL_T                  no_md_ma=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "lport-%ld", (long)header_p->lport);

    memset(&ccm, 0, sizeof(CFM_ENGINE_CcmPdu_T));
    if(FALSE == CFM_ENGINE_VerifyAndDecomposeCCM(pdu_p, header_p, &ccm))
    {
        return FALSE;
    }

    mp_type= CFM_ENGINE_GetMpProcessRcvdPacket(header_p, mep_p, &mip);

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mp type=%d", mp_type);

    if(CFM_TYPE_MP_TYPE_MEP == mp_type)
    {
        if(CFM_TYPE_MP_DIRECTION_UP==mep_p->direction)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "Drop packet, lport=%ld CCM was received by up MEP from back side", (long)header_p->lport);
            return FALSE;
        }

        CFM_ENGINE_MepProcessEqualCCM(&ccm, mep_p);
        return TRUE;
    }

    if(CFM_TYPE_MP_TYPE_NONE == mp_type)
    {
        if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                    header_p->lport,            header_p->md_level,
                    header_p->tag_info&0x0fff,  CFM_TYPE_MP_DIRECTION_UP_DOWN,
                    FALSE,  high_level_mep_p,   NULL))
        {
            /* for ingress port,
             *   higher level down MEP => process low CCM
             *   higher level up   MEP => drop
             */
            if (CFM_TYPE_MP_DIRECTION_DOWN == high_level_mep_p->direction)
                CFM_ENGINE_ProcessLowCCM(&ccm, high_level_mep_p);
            return TRUE;
        }

        /*if this port donesn't has higher level mep, (don't care the mip, because mip still pass through the lower pdu)
         *flood to other port.
         */
    }
    /*if receved by mip or no mp, the pdu still need be forward to other port*/
    else if(CFM_TYPE_MP_TYPE_MIP == mp_type)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mp type=%d", mp_type);
        CFM_ENGINE_MipProcessEqualCCM(&ccm, &mip);
        /*if the pdu received by mip, this pdu should fowrad to other port's mep*/
    }

    /* ingress port is blocking, should not need to do further processing
     */
    if (FALSE == header_p->is_ing_lport_fwd)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "dropped due to ingress port is blocked");
        return TRUE;
    }

    /*if this switch doesn't has the md ma, then flood to each port, and according to each port's mep configuration
     */
    if(FALSE == CFM_OM_GetMdMaByLevelVid(header_p->md_level, header_p->tag_info&0x0fff, &md_p, &ma_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mp type=%d", mp_type);
        no_md_ma=TRUE;
    }

    /*  check all other port -> no mep -flood, up mep- process, down mep-skip*/
    while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
    {
        if(lport == header_p->lport )
        {
            continue;
        }

        if((FALSE == no_md_ma)&&(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p)))
        {
            if(CFM_TYPE_MP_DIRECTION_UP == mep_p->direction)
            {
                CFM_ENGINE_MepProcessEqualCCM(&ccm, mep_p);
            }

            continue;
        }
        else /* ccm flood to this port*/
        {
            UI32_T vlan_ifindex=0;

            VLAN_OM_ConvertToIfindex(header_p->tag_info&0x0fff, &vlan_ifindex);

            /* check whether the port is vlan's member */
            if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, lport))
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mp type=%d", mp_type);
                continue;
            }

            if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                            lport, header_p->md_level, header_p->tag_info&0x0fff,
                            CFM_TYPE_MP_DIRECTION_UP_DOWN, FALSE,
                            high_level_mep_p, NULL))
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mp type=%d", mp_type);

                /* for egress port,
                 *   higher level down MEP => drop
                 *   higher level up   MEP => process low CCM
                 */
                if (CFM_TYPE_MP_DIRECTION_UP == high_level_mep_p->direction)
                    CFM_ENGINE_ProcessLowCCM(&ccm, high_level_mep_p);

                continue;
            }

            CFM_ENGINE_XmitPDU(lport, header_p->tag_info&0x0fff, header_p->md_level, (header_p->tag_info&0xf000)>>13, FALSE,
                                header_p->dst_mac_a, header_p->src_mac_a, CFM_TYPE_MP_DIRECTION_UP, pdu_p, header_p->pdu_length);
        }
    }

    return TRUE;
}/*End of CFM_ENGINE_ProcessCCM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MepProcessEqualCCM
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the received CCM packet which has the smae level of the mep
 * INPUT    : ccm_p - the ccm packet content pointer
 *            mep_p - the mep pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MepProcessEqualCCM(
    CFM_ENGINE_CcmPdu_T     *ccm_p,
    CFM_OM_MEP_T            *mep_p)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "");

    /*check mep active status*/
    if(FALSE == mep_p->active)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mep id=%ld is inactive", (long)mep_p->identifier);
        return TRUE;
    }

    /*check the cross check status*/
    if(CFM_TYPE_CROSS_CHECK_STATUS_DISABLE==mep_p->ma_p->cross_check_status)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mep id=%ld cross check status is disabled", (long)mep_p->identifier);
        return TRUE;
    }

    if(FALSE == CFM_ENGINE_MepExecuseCCM(ccm_p, mep_p))
    {
        CFM_OM_StoreMep( mep_p);
        return FALSE;
    }

    CFM_OM_StoreMep( mep_p);
    return TRUE;
}/*End of CFM_ENGINE_MepProcessEqualCCM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessLowCCM
 *-------------------------------------------------------------------------
 * PURPOSE  : process the CCM PDU with level low than the system' meps' level
 * INPUT    :*ccm_p   - the ccm packet content pointer
 *           *mep_p   - the mep pointer which process the lower level ccm
 * OUTPUT   : None
 * RETURN   : TRUE  -  success
 *            FALSE  - fail
 * NOTE     : This function not use now
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ProcessLowCCM(
                                        CFM_ENGINE_CcmPdu_T *ccm_p,
                                        CFM_OM_MEP_T *mep_p)
{
    if(NULL == mep_p)
    {
        return FALSE;
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM,"Lower level CCM recevied by MP, level=%d, lport=%ld, mep id=%ld",
                          ccm_p->header_p->md_level, (long)ccm_p->header_p->lport, (long)mep_p->identifier);

    mep_p->xcon_ccm_defect=TRUE;

    CFM_ENGINE_XconDefectState(ccm_p, mep_p);

    CFM_OM_StoreMep(mep_p);

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_VerifyAndDecomposeCCM
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get all the content's pointer from the
 *            receive packet for using after
 * INPUT    : *pdu_p   - the pdu content pointer
 *            header_p - the common head pointer
 *            ccm_p    - the CCM packet contnet pointer, which conntent
 *            the all need field pointer point to received packet.
 * OUTPUT   : None
 * RETURN   : TRUE  -  success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_VerifyAndDecomposeCCM(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_CcmPdu_T     *ccm_p)
{
    {/*ccm interval*/
        UI8_T interval=header_p->flags & 0x07;

        if((CFM_TYPE_CCM_INTERVAL_INVALID>=interval)||
          (CFM_TYPE_CCM_INTERVAL_10_MIN<interval))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "ccm interval is in wrong range");
            return FALSE;
        }
    }
    {/*first tlv offset*/

        if(header_p->first_tlv_offset<70)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "first tlv offset is wrong");
            return FALSE;
        }
    }

    /*common header has no problem assign th header.*/
    ccm_p->header_p=header_p;

    {/*check Sequence Number*/
        ccm_p->seq_num_p=(pdu_p+4);
    }
    { /*check Mep id*/
        UI16_T mepid=0;

        ccm_p->mep_id_p=(pdu_p+8);
        memcpy(&mepid, ccm_p->mep_id_p, sizeof(UI16_T));
        mepid=L_STDLIB_Ntoh16(mepid);

        if (  (SYS_ADPT_CFM_MAX_MEP_ID < mepid)
            ||(SYS_ADPT_CFM_MIN_MEP_ID > mepid)
           )
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mep id=%d range is wrong", mepid);
            return FALSE;
        }
    }
    {/* check MAID*/
        UI8_T *maid_p=NULL;

        maid_p=(pdu_p+10);
        ccm_p->md_format_p=maid_p;
        /* 1. decompose
         */
        switch (*ccm_p->md_format_p)
        {
        case CFM_TYPE_MD_NAME_NONE:
            /* no domain name,
             * valid for ma_format_p, ma_length_p, ma_name_p
             *
             * for Y.1731, ICC-Based ma name
             */
            ccm_p->md_length_p= NULL;
            ccm_p->md_name_p  = NULL;
            ccm_p->ma_format_p= maid_p+1;
            ccm_p->ma_length_p= maid_p+2;
            ccm_p->ma_name_p  = maid_p+3;
            break;
        case CFM_TYPE_MD_NAME_DNS_LIKE_NAME:
        case CFM_TYPE_MD_NAME_MAC_ADDRESS_AND_UNIT:
        case CFM_TYPE_MD_NAME_CHAR_STRING:
            /* domain name present, modify md, ma fields
             * valid for md_length_p, md_name_p,
             *           ma_format_p, ma_length_p, ma_name_p
             */
            ccm_p->md_length_p= maid_p+1;
            ccm_p->md_name_p  = maid_p+2;
            ccm_p->ma_format_p= maid_p+1+*ccm_p->md_length_p+1;
            ccm_p->ma_length_p= maid_p+1+*ccm_p->md_length_p+2;
            ccm_p->ma_name_p  = maid_p+1+*ccm_p->md_length_p+3;
            break;
        default:
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "MAID format=%d is wrong", *(UI8_T*)ccm_p->md_format_p);
            return FALSE;
        }

        /* 2. check length
         */
        switch (*ccm_p->md_format_p)
        {
        case CFM_TYPE_MD_NAME_DNS_LIKE_NAME:
        case CFM_TYPE_MD_NAME_MAC_ADDRESS_AND_UNIT:
        case CFM_TYPE_MD_NAME_CHAR_STRING:
            /* domain name present, check md length, ma format, ma length
             */
            if (  (CFM_TYPE_MD_MAX_NAME_LENGTH < *(UI8_T*)ccm_p->md_length_p)
                ||(CFM_TYPE_MD_MIN_NAME_LENGTH > *(UI8_T*)ccm_p->md_length_p)
               )
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "md len=%d is wrong", *(UI8_T*)ccm_p->md_length_p);
                return FALSE;
            }

        case CFM_TYPE_MD_NAME_NONE:
            /* no domain name, check ma format, length
             */
            if (  (CFM_TYPE_MA_NAME_RFC2856_VPN_ID < *(UI8_T*)ccm_p->ma_format_p)
                ||(CFM_TYPE_MA_NAME_PRIMARY_VID    > *(UI8_T*)ccm_p->ma_format_p)
               )
            {
                if (CFM_TYPE_MA_NAME_ICC_BASED != *(UI8_T*)ccm_p->ma_format_p)
                {
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "ma format=%d is wrong", *(UI8_T*)ccm_p->ma_format_p);
                    return FALSE;
                }
            }

            /* bcz CFM_TYPE_MA_MAX_NAME_LENGTH may be smaller than the leaf
             */
            if (  (MAXSIZE_dot1agCfmMaName < *(UI8_T*)ccm_p->ma_length_p)
                ||(MINSIZE_dot1agCfmMaName > *(UI8_T*)ccm_p->ma_length_p)
               )
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "ma len=%d is wrong", *(UI8_T*)ccm_p->ma_length_p);
                return FALSE;
            }
            break;

        default:
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "MAID format=%d is wrong", *(UI8_T*)ccm_p->md_format_p);
            return FALSE;
        }
    }

    {/*option tlv*/
        UI8_T *tlv=NULL, tlv_type=0;
        UI16_T tlv_length=0;
        UI32_T len_accum=0, pdu_len=header_p->pdu_length;

        tlv=(pdu_p+74);
        pdu_len=pdu_len-74;

        while(TRUE)
        {
            tlv_type=*tlv;
            memcpy(&tlv_length, &tlv[1], sizeof(UI16_T));
            tlv_length = L_STDLIB_Ntoh16(tlv_length);

            /*check the tlv type*/
            switch(tlv_type)
            {
            case CFM_TYPE_TLV_SENDER_ID:
                if(FALSE == CFM_ENGINE_DecomposeAndVerifySenderTlv(
                                header_p, &ccm_p->sender_tlv, tlv))
                {
                    return FALSE;
                }
                break;
            case CFM_TYPE_TLV_PORT_STATUS:
                ccm_p->port_status_type_p=tlv;
                ccm_p->port_status_length_p=tlv+1;
                ccm_p->port_status_value_p=tlv+3;

                if ((CFM_TYPE_PORT_STATUS_BLOCKED!=*(UI8_T*)ccm_p->port_status_value_p)&&(CFM_TYPE_PORT_STATUS_UP!=*(UI8_T*)ccm_p->port_status_value_p))
                {
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "port status tlv is wrong");
                    return FALSE;
                }
                break;

            case CFM_TYPE_TLV_INTERFACE_STATUS:
                ccm_p->interface_status_type_p=tlv;
                ccm_p->interface_status_length_p=tlv+1;
                ccm_p->interface_status_value_p=tlv+3;

                if ((CFM_TYPE_INTERFACE_STATUS_LOWERLAYERDOWN<*(UI8_T*)ccm_p->interface_status_value_p)||(CFM_TYPE_INTERFACE_STATUS_UP>*(UI8_T*)ccm_p->interface_status_value_p))
                {
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "interface status is in wrong range");
                    return FALSE;
                }
                break;

            case CFM_TYPE_TLV_ORGANIZATION:
                if(FALSE == CFM_ENGINE_DecomposeAndVerifyOrganizationTlv(&ccm_p->org_tlv, tlv))
                {
                    return FALSE;
                }
                break;
            case CFM_TYPE_TLV_END:
                return TRUE;
            default:
                break;
            }

            /*accum tlv length over pdu length*/
            len_accum+=(tlv_length+3);

            if(len_accum==pdu_len)
                return TRUE;

            if (len_accum>pdu_len)
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "tlv accume leng over packet length pdu_len:%ld,len_accum:%ld",(long)pdu_len,(long)len_accum);
                return FALSE;
            }

            tlv=(tlv+tlv_length+3);
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "verify fail");
    return FALSE;
}/*end of CFM_ENGINE_VerifyAndDecomposeCCM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CheckMaidOfCCM
 *-------------------------------------------------------------------------
 * PURPOSE  : To check if Maid in CCM is ok for this MEP
 * INPUT    : *ccm_p - the CCM content pointer point to the receive CCM packet
 *            *mep_p - teh mep pointer which will process the CCM
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_CheckMaidOfCCM(
    CFM_ENGINE_CcmPdu_T     *ccm_p,
    CFM_OM_MEP_T            *mep_p)
{
    /* step 1, check if format of ccm is correct
     */
    if (CFM_TYPE_MA_NAME_ICC_BASED == *ccm_p->ma_format_p)
    {
        if (CFM_TYPE_MD_NAME_NONE != *ccm_p->md_format_p)
        {
            return FALSE;
        }
    }

    /* step 2, check if the field is valid
     */
    if (CFM_TYPE_MD_NAME_NONE == *ccm_p->md_format_p)
    {
        if (CFM_TYPE_MA_NAME_CHAR_STRING == *ccm_p->ma_format_p)
        {
            /* if   ccm's md format is md none and
             *   && ccm's ma format is char string
             * then local md's name length must be 0
             */
            if (0 != mep_p->md_p->name_length)
            {
                return FALSE;
            }
        }

        /* Both ICC and CharString need check these
         */
        if (  (mep_p->ma_p->format != *(UI8_T*)ccm_p->ma_format_p)
            ||(mep_p->ma_p->primary_vid != (ccm_p->header_p->tag_info&0x0fff))
           )
        {
            return FALSE;
        }

        if (CFM_TYPE_MA_NAME_CHAR_STRING == mep_p->ma_p->format)
        {
            if (  (mep_p->ma_p->name_length != *(UI8_T*)ccm_p->ma_length_p)
                ||(memcmp(mep_p->ma_p->name_a, ccm_p->ma_name_p,*(UI8_T*)ccm_p->ma_length_p ))
               )
            {
                return FALSE;
            }
        }
        else  /* CFM_TYPE_MA_NAME_ICC_BASED */
        {
            /* ma name length is 13 => should total compare
             */
            if (  (CFM_TYPE_MA_MAX_NAME_LENGTH_FOR_Y1731 == mep_p->ma_p->name_length)
                &&(0 != memcmp(mep_p->ma_p->name_a, ccm_p->ma_name_p,*(UI8_T*)ccm_p->ma_length_p))
               )
            {
                return FALSE;
            }
            else  /* ma name length less than 13 */
            {
                if (0 != memcmp(mep_p->ma_p->name_a, ccm_p->ma_name_p, mep_p->ma_p->name_length))
                {
                    return FALSE;
                }
                else  /* the remnant should be all 0x00 */
                {
                    UI8_T   *chk_ch_p;
                    UI32_T  i, ext_length;

                    ext_length = (CFM_TYPE_MA_MAX_NAME_LENGTH_FOR_Y1731 - mep_p->ma_p->name_length);
                    chk_ch_p   = &ccm_p->ma_name_p[mep_p->ma_p->name_length];

                    for (i=0; i<ext_length; i++)
                    {
                        if (0x00 != chk_ch_p[i])
                        {
                            return FALSE;
                        }
                    }
                } /* (0 != memcmp(mep_p->ma_p->name_a,... */
            }
        } /* if (CFM_TYPE_MA_NAME_CHAR_STRING == mep_p->ma_p->format) */
    }
    else
    {
        if(  (mep_p->md_p->name_length != *(UI8_T*)ccm_p->md_length_p)
           ||(memcmp(mep_p->md_p->name_a, ccm_p->md_name_p,*(UI8_T*)ccm_p->md_length_p ))
           ||(mep_p->ma_p->format != *(UI8_T*)ccm_p->ma_format_p)
           ||(mep_p->ma_p->name_length != *(UI8_T*)ccm_p->ma_length_p)
           ||(memcmp(mep_p->ma_p->name_a, ccm_p->ma_name_p,*(UI8_T*)ccm_p->ma_length_p ))
           ||(mep_p->primary_vid != (ccm_p->header_p->tag_info&0x0fff))
          )
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MepExecuseCCM
 *-------------------------------------------------------------------------
 * PURPOSE  : This is stardard's function which process the CCM content
 * INPUT    : ccm_p - the CCM content pointer point to the receive CCM packet
 *            mep_p - the mep pointer which will process the CCM
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function do three state machine, error, xcon,remote mep ok
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MepExecuseCCM(
    CFM_ENGINE_CcmPdu_T     *ccm_p,
    CFM_OM_MEP_T            *mep_p)
{
    CFM_OM_REMOTE_MEP_T     remote_mep;
    CFM_OM_MD_T             *md_p=NULL;
    CFM_OM_MA_T             *ma_p=NULL;
    UI16_T                  remote_mep_id=0;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mep id=%ld", (long)mep_p->identifier);

    /*increase total receive CCM*/
    mep_p->rcvd_ccm_amount++;

    /*get remote mep id*/
    memcpy(&remote_mep_id, ccm_p->mep_id_p, sizeof(UI16_T));
    remote_mep_id=L_STDLIB_Ntoh16(remote_mep_id);

    if(FALSE == CFM_OM_GetMdMaByLevelVid(ccm_p->header_p->md_level, (ccm_p->header_p->tag_info&0x0fff), &md_p, &ma_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "drop Packet, donesn't exist this md or ma");

        CFM_ENGINE_ErrCcmDefectState(ccm_p, mep_p);
        CFM_ENGINE_XmitCrossCheckTrap(mep_p->md_p, mep_p->ma_p, mep_p->identifier, remote_mep_id, CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN);
        return TRUE;
    }

    /*check the loop*/
    if(!memcmp(ccm_p->header_p->src_mac_a, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "drop packet, loop src mac is equal to mep's mac, mep mac=%02x:%02x:%02x:%02x:%02x:%02x, src_mac=%02x:%02x:%02x:%02x:%02x:%02x",
                              mep_p->mac_addr_a[0], mep_p->mac_addr_a[1], mep_p->mac_addr_a[2], mep_p->mac_addr_a[3], mep_p->mac_addr_a[4], mep_p->mac_addr_a[5],
                              ccm_p->header_p->src_mac_a[0], ccm_p->header_p->src_mac_a[1], ccm_p->header_p->src_mac_a[2], ccm_p->header_p->src_mac_a[3], ccm_p->header_p->src_mac_a[4], ccm_p->header_p->src_mac_a[5]);

        CFM_ENGINE_XmitCCTrap(mep_p->md_p, mep_p->ma_p, mep_p->identifier, mep_p->identifier, CFM_TYPE_SNMP_TRAPS_CC_LOOP);
        return FALSE;
    }

    /*20.17.1:b, check maid*/
    if(FALSE == CFM_ENGINE_CheckMaidOfCCM(ccm_p, mep_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "drop packet, MAID is wrong");

        CFM_ENGINE_XconDefectState(ccm_p, mep_p);
        return FALSE;
    }

    /*check 20.17.1.c
     */
    /*20.17.1:c.1, get the remote mep pionter*/
    if(FALSE == CFM_OM_GetRemoteMep(md_p->index, ma_p->index, remote_mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mep id=%d doesn't configure the remote mep", remote_mep_id);

        CFM_ENGINE_ErrCcmDefectState(ccm_p, mep_p);
        CFM_ENGINE_XmitCrossCheckTrap(mep_p->md_p, mep_p->ma_p, mep_p->identifier, remote_mep_id, CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN);
        return FALSE;
    }
    else
    {
        /* if this remote mep's ccm has been received by another UP MEP,
         * return here to avoid corrupting the mac status counter, etc...
         *
         * if received local MEP changed from A to B,
         *   1. A and B has the same UP direction
         *     only record the remote MEP's counter on 1st UP MEP,
         *     so return here.
         *     if A was removed by user, then remote MEP will be re-learnt
         *     after the remote MEP age out.
         *   2. A and B has differet direction
         *     the CFM_ENGINE_RemoteMepOkState will update the
         *     counter of A and B.
         */
        if (  (remote_mep.rcvd_mep_id != 0)
            &&(remote_mep.rcvd_mep_id != mep_p->identifier)
            &&(remote_mep.rcvd_mep_direction == mep_p->direction)
            &&(CFM_TYPE_MP_DIRECTION_UP == mep_p->direction)
           )
        {
            return TRUE;
        }
    }

    /*20.17.1:c.2 , Configured mep id is the same as the remote mep id*/
    if(remote_mep_id == mep_p->identifier)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "drop pacekt, remote mep id is the same as the receiving mep id=%d", remote_mep_id);

        CFM_ENGINE_ErrCcmDefectState(ccm_p, mep_p);
        CFM_ENGINE_XmitCCTrap(mep_p->md_p, mep_p->ma_p, mep_p->identifier, remote_mep_id, CFM_TYPE_SNMP_TRAPS_CC_CONFIG);

        remote_mep.packet_error++;
        CFM_OM_SetRemoteMep(&remote_mep);
        return FALSE;
    }

    /*20.17.1:c.3, check the interval flag*/
    if((ccm_p->header_p->flags&0x07)!=mep_p->ma_p->ccm_interval)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "drop packet, recevied ccm interval flag=%d is not the same as the mep", (ccm_p->header_p->flags&0x07));

        CFM_ENGINE_ErrCcmDefectState(ccm_p, mep_p);

        remote_mep.packet_error++;
        CFM_OM_SetRemoteMep(&remote_mep);
        return FALSE;
    }

    /*update counters, timer*/
    {
        UI32_T archive_hold_time=0;
        UI16_T ccm_time=0;

        remote_mep.packet_received++;
        remote_mep.cc_life_time+=((CFM_ENGINE_CCMTime(mep_p->ma_p->ccm_interval)*35/10)-CFM_TIMER_QureyTime(remote_mep.rmep_while_timer_idx));

        CFM_OM_GetArchiveHoldTime(remote_mep.md_index, &archive_hold_time);

        if(FALSE == CFM_TIMER_UpdateTimer(remote_mep.archive_hold_timer_idx, archive_hold_time, CFM_ENGINE_ClearRemoteMepByRemoteMep_Callback))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE,
                        "remote mep=%ld can't update archive hold time %ld",
                        (long)remote_mep.identifier, (long)remote_mep.archive_hold_timer_idx);
        }

        ccm_time=(UI16_T)(CFM_ENGINE_CCMTime(mep_p->ma_p->ccm_interval)*35/10);
        remote_mep.age_out = SYS_TIME_GetSystemTicksBy10ms()/100 +ccm_time;

        if(CFM_TYPE_REMOTE_MEP_STATE_START == remote_mep.machine_state)
        {
            CFM_Timer_CallBackPara_T para;
            CFM_TIMER_AssignTimerParameter(&para, remote_mep.md_index, remote_mep.ma_index, remote_mep.identifier, 0, 0);
            if(FALSE == CFM_TIMER_ModifyTimer(remote_mep.rmep_while_timer_idx,
                                              CFM_ENGINE_RemoteMepOk_Callback,
                                              &para,
                                              ccm_time,
                                              CFM_TIMER_CYCLIC))
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE,
                            "remote mep=%ld can't modify remote mep ccm timer %ld",
                            (long)remote_mep.identifier, (long)remote_mep.rmep_while_timer_idx);
            }
        }
        else
        {
            if(FALSE == CFM_TIMER_UpdateTimer(remote_mep.rmep_while_timer_idx,
                                              ccm_time,
                                              CFM_ENGINE_RemoteMepOk_Callback))
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE,
                            "remote mep=%ld can't update remote mep ccm timer %ld",
                            (long)remote_mep.identifier, (long)remote_mep.rmep_while_timer_idx);
            }
        }
    }

    /*20.17.1:d, check other remote info comformance*/
    CFM_ENGINE_RemoteMepOkState(ccm_p, &remote_mep, mep_p);

    /* 20.17.1
     * f)If both values are not 0, and if the new value is not 1 greater than
     *   the last, increment CCMsequenceErrors (20.16.12); and
     * g)Store the received Sequence Number in the MEP CCM Database.
     */
    /*check and update the mep sequence num*/
    {
        UI32_T  seq_num=0;

        memcpy(&seq_num, ccm_p->seq_num_p, sizeof(UI32_T));
        seq_num=L_STDLIB_Ntoh32(seq_num);

        if (  (0 != remote_mep.next_sequence)
            &&(0 != seq_num )
            &&(remote_mep.next_sequence != seq_num)
           )
        {
            mep_p->rcvd_ccm_sequenc_errors+=1;
            remote_mep.packet_error++;
        }

        /*update the next ccm sequence*/
        remote_mep.next_sequence = seq_num;

        if (0 != remote_mep.next_sequence)
        {
            remote_mep.next_sequence++;
        }
    }
    CFM_OM_SetRemoteMep(&remote_mep);

    CFM_ENGINE_UPD_ERR_MAC_STATUS(mep_p);
    CFM_ENGINE_UPD_SOME_RDI_DEF(mep_p);
    CFM_ENGINE_UPD_SOME_RMEP_CCM_DEF(mep_p);
    CFM_ENGINE_UpdateMepHighestDefect(mep_p);

    return TRUE;
}/*End of CFM_ENGINE_MepExecuseCCM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MipProcessEqualCCM
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process is used for mip to handle the ccm
 * INPUT    : *ccm_p - the CCM content pointer point to the receive CCM packet
 *            *mip_p - teh mep pointer which will process the CCM
 * OUTPUT   : None
 * RETURN   : TRUE  -  success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MipProcessEqualCCM(
                                            CFM_ENGINE_CcmPdu_T *ccm_p,
                                            CFM_OM_MIP_T *mip_p)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "");

    CFM_ENGINE_MipExecuseCCM(ccm_p, mip_p);

    return TRUE;
}/*End of CFM_ENGINE_MipProcessEqualCCM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MipExecuseCCM
 *-------------------------------------------------------------------------
 * PURPOSE  : This is stardard's function which process the CCM content
 * INPUT    : *ccm_p - the CCM content pointer point to the receive CCM packet
 *            *mip_p - teh mep pointer which will process the CCM
 * OUTPUT   : None
 * RETURN   : TRUE  -  success
 *            FALSE  - fail
 * NOTE     :  MIP will save the FDB entry, this FDB already be done on AMTR
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MipExecuseCCM(
                                        CFM_ENGINE_CcmPdu_T *ccm_p,
                                        CFM_OM_MIP_T *mip_p)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "");

    /* 19.3.9
     * all CCMs at the same MD Level as the MHF are presented to the
     * MHF Continuity Check Receiver. No other information in the
     * received CCMs is examined. In particular, the MAID in the CCM
     * is not compared to the MHF's MAID.
     */
    return TRUE;
}/*End of CFM_ENGINE_MipExecuseCCM*/

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
                                            void *timer_para_p)
{
    /* use to count all payload size */
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    CFM_Timer_CallBackPara_T *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "");

    if(FALSE == CFM_OM_GetMep(para_p->md_index, para_p->ma_index, para_p->mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        return FALSE;
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "mep id=%ld", (long)mep_p->identifier);

    /*check mep active status*/
    if(FALSE == mep_p->active)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "mep id=%ld is inactive", (long)mep_p->identifier);
        return TRUE;
    }

    if(CFM_TYPE_CCM_STATUS_DISABLE == mep_p->cci_status)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "mep id=%ld cci disable", (long)mep_p->identifier);
        return TRUE;
    }

    CFM_ENGINE_XmitCCM(mep_p, NULL);
    CFM_OM_StoreMep(mep_p);
    return TRUE;
}/*End of CFM_ENGINE_CcmWait_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitCCM
 *-------------------------------------------------------------------------
 * PURPOSE  : This function transmit the CCM
 * INPUT    : *mep_p        - the mep pointer which will trasnmit the CCM
 *           port_status_notify - port status change notify information
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_XmitCCM(
                                CFM_OM_MEP_T *mep_p,
                                CFM_ENGINE_PortStatusCallback_T *port_status_notify)
{
    UI16_T  pdu_len = 0;
    UI8_T   *pdu_p=NULL;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN]={0};
    CFM_TYPE_CfmStatus_T global_status=CFM_TYPE_CFM_STATUS_DISABLE, port_status=CFM_TYPE_CFM_STATUS_DISABLE;

    CFM_OM_GetCFMGlobalStatus(&global_status);
    CFM_OM_GetCFMPortStatus(mep_p->lport, &port_status);

#if (SYS_CPNT_MGMT_PORT == TRUE)
    if(mep_p->lport==SYS_ADPT_MGMT_PORT)
    {
        return;
    }
#endif

    if(CFM_TYPE_CCM_STATUS_DISABLE == mep_p->cci_status)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mep id=%ld cci disable", (long)mep_p->identifier);
        return;
    }

    /*if the port's and global status not enable, the ccm won't send*/
    if((CFM_TYPE_CFM_STATUS_ENABLE != global_status)||(CFM_TYPE_CFM_STATUS_ENABLE!=port_status))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "cfm disabled port=%ld", (long)mep_p->lport);
        return;
    }

    if(NULL == (pdu_p=(UI8_T *)L_MM_Malloc(CFM_TYPE_MAX_FRAME_SIZE, L_MM_USER_ID2(SYS_MODULE_CFM, CFM_TYPE_XMITCCM))))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "can't alloc memeory to transmit packet");
        return;
    }
    memset(pdu_p, 0, CFM_TYPE_MAX_FRAME_SIZE);

    /*construct PDU to send*/
    CFM_ENGINE_ConstructCCMPDU(pdu_p, &pdu_len, mep_p, port_status_notify);

    /*increase sent ccm counts*/
    mep_p->cci_sent_ccms+=1;

    CFM_ENGINE_ASSEMBLE_DEST_MAC(dst_mac, mep_p->md_p->level);

    if(CFM_TYPE_MP_DIRECTION_DOWN == mep_p->direction)
    {
        CFM_ENGINE_XmitPDU(mep_p->lport, mep_p->primary_vid, mep_p->md_p->level, mep_p->ccm_ltm_priority, FALSE, dst_mac,
                            mep_p->mac_addr_a, CFM_TYPE_MP_DIRECTION_DOWN, pdu_p, pdu_len);
    }
    else
    {
        UI32_T          lport=0, vlan_ifindex=0;
        CFM_OM_MEP_T    *high_level_mep_p=&mep_tmp2_g, *same_md_ma_mep=&mep_tmp3_g;

        while(SWCTRL_LPORT_UNKNOWN_PORT !=SWCTRL_GetNextLogicalPort(&lport))
        {
            if(lport == mep_p->lport)
            {
                continue;
            }

#if (SYS_CPNT_MGMT_PORT == TRUE)
            if(mep_p->lport==SYS_ADPT_MGMT_PORT)
            {
                return;
            }
#endif

            VLAN_OM_ConvertToIfindex(mep_p->primary_vid, &vlan_ifindex);

            /* check whether the port is vlan's member */
            if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, lport))
            {
                continue;
            }
            if(TRUE == CFM_OM_GetMep(mep_p->md_index, mep_p->ma_index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, same_md_ma_mep))
            {
                continue;
            }

            if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                            lport, mep_p->md_p->level, mep_p->primary_vid,
                            CFM_TYPE_MP_DIRECTION_UP_DOWN, FALSE,
                            high_level_mep_p, NULL))
            {
                continue;
            }

            CFM_ENGINE_XmitPDU(lport, mep_p->primary_vid, mep_p->md_p->level, mep_p->ccm_ltm_priority, FALSE,
                                dst_mac, mep_p->mac_addr_a, CFM_TYPE_MP_DIRECTION_UP, pdu_p, pdu_len);
        }
    }

   L_MM_Free(pdu_p);
   return;
}/*End of CFM_ENGINE_XmitCCM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CCMTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the CCM time in second
 *            according to the input interval level
 * INPUT    : Interval - the CCM interval
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :input interval flag is wrong then give 60 sec.
 *-------------------------------------------------------------------------
 */
UI32_T CFM_ENGINE_CCMTime(
                        CFM_TYPE_CcmInterval_T interval)
{
    switch(interval)
    {
    case CFM_TYPE_CCM_INTERVAL_300_HZ:
        return 1;
    case CFM_TYPE_CCM_INTERVAL_10_MS:
        return 1;
    case CFM_TYPE_CCM_INTERVAL_100_MS:
        return 1;
    case CFM_TYPE_CCM_INTERVAL_1_S:
        return 1;
    case CFM_TYPE_CCM_INTERVAL_10_S:
        return 10;
    case CFM_TYPE_CCM_INTERVAL_1_MIN:
        return 60;
    case CFM_TYPE_CCM_INTERVAL_10_MIN:
        return 10*60;
    default:
        return 60;
    }
}/*End of CFM_ENGINE_CCMTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_UpdateMepHighestDefect
 *-------------------------------------------------------------------------
 * PURPOSE  : To update the highest defect of specified MEP.
 * INPUT    : mep_p  - pointer to mep for processing
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_UpdateMepHighestDefect(CFM_OM_MEP_T *mep_p)
{
    CFM_TYPE_FNG_HighestDefectPri_T  current_defect;

    current_defect = CFM_OM_GetCurrentMaxErrPri(mep_p, FALSE);

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM,
        "mep/def -%ld/%d", (long)mep_p->identifier, current_defect);

    CFM_ENGINE_FngStateMachine(current_defect, mep_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ErrCcmDefectState
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will do the job in Error CCM stat
 * INPUT    : *ccm_p  - the ccm content pointer.
 *            *mep_p  - the mep poiner which will process the error ccm
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ErrCcmDefectState(
                                            CFM_ENGINE_CcmPdu_T *ccm_p,
                                            CFM_OM_MEP_T *mep_p)
{
    UI32_T  max_time=0, end_time_of_errWhile=0,
            end_time_of_rcvd_interval=0, record_len=0;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "mep id=%ld", (long)mep_p->identifier);

    /* For CCM interval=4 case, the calculated CCM expire time should be longer
     *  than the default fng alarm time, otherwise the trap will be lost
     *
     */
    end_time_of_rcvd_interval =
        (CFM_ENGINE_CCMTime(ccm_p->header_p->flags&0x07)*35 + 9)/10;

    mep_p->error_ccm_defect = TRUE;

    if(-1 !=mep_p->error_ccm_while_timer_idx)
    {
        end_time_of_errWhile=CFM_TIMER_QureyTime(mep_p->error_ccm_while_timer_idx);
        max_time=( end_time_of_errWhile> end_time_of_rcvd_interval ? end_time_of_errWhile : end_time_of_rcvd_interval);
        CFM_TIMER_UpdateTimer(mep_p->error_ccm_while_timer_idx, max_time, CFM_ENGINE_ErrorCcmDefect_Callback);
    }
    else
    {
        CFM_Timer_CallBackPara_T para;
        CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);

        mep_p->error_ccm_while_timer_idx=CFM_TIMER_CreateTimer(
                                                       CFM_ENGINE_ErrorCcmDefect_Callback,
                                                       &para,
                                                       end_time_of_rcvd_interval,
                                                       CFM_TIMER_ONE_TIME);
        CFM_TIMER_StartTimer(mep_p->error_ccm_while_timer_idx);
    }

    /*record the error ccm content
     */
    memset(mep_p->error_ccm_last_failure_a, 0, CFM_TYPE_MAX_FRAME_RECORD_SIZE);
    mep_p->error_ccm_last_failure_lenth=0;

    record_len=ccm_p->header_p->pdu_length>CFM_TYPE_MAX_FRAME_RECORD_SIZE?
                 CFM_TYPE_MAX_FRAME_RECORD_SIZE:ccm_p->header_p->pdu_length;
    memcpy(mep_p->error_ccm_last_failure_a, ccm_p->seq_num_p-4, record_len);
    mep_p->error_ccm_last_failure_lenth=record_len;

    /*check the fng state machine state*/
    CFM_ENGINE_UpdateMepHighestDefect(mep_p);

    return TRUE;
}/* End of CFM_ENGINE_ErrCcmDefectState*/

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
                                                void *timer_para_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    CFM_Timer_CallBackPara_T *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "");

    if(FALSE == CFM_OM_GetMep(para_p->md_index, para_p->ma_index, para_p->mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        return FALSE;
    }
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "mep id=%ld", (long)mep_p->identifier);

    mep_p->error_ccm_defect=FALSE;

    memset(mep_p->error_ccm_last_failure_a, 0, CFM_TYPE_MAX_FRAME_RECORD_SIZE);
    mep_p->error_ccm_last_failure_lenth=0;

    CFM_ENGINE_UpdateMepHighestDefect(mep_p);

    CFM_TIMER_FreeTimer(&mep_p->error_ccm_while_timer_idx);
    CFM_OM_StoreMep(mep_p);
    return TRUE;
 }/* End of CFM_ENGINE_ErrorCcmDefect_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XconDefectState
 *-------------------------------------------------------------------------
 * PURPOSE  : The funciton do the job in Xcon machine xcon state
 * INPUT    : *ccm_p - the ccm content pointer pointe to the ccm packet
 *            *mep_p - the mep pointe which process the xcon state
 * OUTPUT   : None
 * RETURN   : TRUE  -  success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_XconDefectState(
    CFM_ENGINE_CcmPdu_T *ccm_p,
    CFM_OM_MEP_T        *mep_p)
{
    UI32_T max_time=0,end_time_of_xconWhile=0,end_time_of_rcvd_interval=0;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "mep id=%ld", (long)mep_p->identifier);

    /* For CCM interval=4 case, the calculated CCM expire time should be longer
     *  than the default fng alarm time, otherwise the trap will be lost
     *
     */
    end_time_of_rcvd_interval =
        (CFM_ENGINE_CCMTime(ccm_p->header_p->flags&0x07)*35 + 9)/10;

    mep_p->xcon_ccm_defect = TRUE;

    if(-1 !=mep_p->xcon_ccm_while_timer_idx)
    {
        end_time_of_xconWhile=CFM_TIMER_QureyTime(mep_p->xcon_ccm_while_timer_idx);
        max_time=( end_time_of_xconWhile> end_time_of_rcvd_interval ? end_time_of_xconWhile : end_time_of_rcvd_interval);
        CFM_TIMER_UpdateTimer(mep_p->xcon_ccm_while_timer_idx, max_time, CFM_ENGINE_XconDefect_Callback);
    }
    else
    {
        CFM_Timer_CallBackPara_T para;

        CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);

        mep_p->xcon_ccm_while_timer_idx=CFM_TIMER_CreateTimer(
                                       CFM_ENGINE_XconDefect_Callback,
                                       &para,
                                       end_time_of_rcvd_interval,
                                       CFM_TIMER_ONE_TIME);
        CFM_TIMER_StartTimer(mep_p->xcon_ccm_while_timer_idx);
    }

    /*record the xcon ccm packet*/
    {
        UI32_T record_len=0;

        memset(mep_p->xcon_ccm_last_failure_a, 0, sizeof(mep_p->xcon_ccm_last_failure_a));
        record_len=ccm_p->header_p->pdu_length>CFM_TYPE_MAX_FRAME_RECORD_SIZE?CFM_TYPE_MAX_FRAME_RECORD_SIZE:ccm_p->header_p->pdu_length;
        memcpy(mep_p->xcon_ccm_last_failure_a, ccm_p->seq_num_p-4, record_len);
        mep_p->xcon_ccm_last_failure_length=record_len;
    }

    /*check the fault notify fng state*/
    CFM_ENGINE_UpdateMepHighestDefect(mep_p);

    return TRUE;
}/*End of CFM_ENGINE_XconDefectState*/

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
    void    *timer_para_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    CFM_Timer_CallBackPara_T *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "");

    if(FALSE == CFM_OM_GetMep(para_p->md_index, para_p->ma_index, para_p->mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        return FALSE;
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "mep id=%ld", (long)mep_p->identifier);

    mep_p->xcon_ccm_defect=FALSE;

    memset(mep_p->xcon_ccm_last_failure_a, 0, sizeof(CFM_TYPE_MAX_FRAME_RECORD_SIZE));
    mep_p->xcon_ccm_last_failure_length=0;

    CFM_ENGINE_UpdateMepHighestDefect(mep_p);

    CFM_TIMER_FreeTimer(&mep_p->xcon_ccm_while_timer_idx);

    CFM_OM_StoreMep(mep_p);
    return TRUE;
}/* End of CFM_ENGINE_XconDefect_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_RemoteMepOkState
 *-------------------------------------------------------------------------
 * PURPOSE  : The function do the job in Remote mep machine OK state
 * INPUT    : *ccm_p        - the ccm content pointer point to the packet
 *            *remote_mep_p - the remote mep which process the mep
 *            *mep_p        - the mep pointer which process ccm
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_RemoteMepOkState(
    CFM_ENGINE_CcmPdu_T     *ccm_p,
    CFM_OM_REMOTE_MEP_T     *remote_mep_p,
    CFM_OM_MEP_T            *mep_p)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "mep id=%ld, enter time=%lu", (long)mep_p->identifier, (long)SYS_TIME_GetSystemTicksBy10ms());

    /*set state*/
    remote_mep_p->failed_ok_time = SYS_TIME_GetSystemTicksBy10ms();

    /*1. clear other mep has the info of this remote mep has no mep
      2. assign the mep of receiving this remote mep's ccm
     */
    if (0 == remote_mep_p->rcvd_mep_id)
    {
        CFM_ENGINE_LocalUpdateRMepCcmLostCntInOneMa(
            remote_mep_p, mep_p, FALSE);

        /* bcz FNG state machine should not be updated twice,
         * so need to update the ccm loss cnt here
         */
        if (0 < mep_p->rmep_ccm_loss_cnt)
        {
            mep_p->rmep_ccm_loss_cnt--;
        }

        mep_p->rmep_lrn_cnt++;
    }
    else
    {
        if (remote_mep_p->machine_state == CFM_TYPE_REMOTE_MEP_STATE_FAILD)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "");

            mep_p->rmep_ccm_loss_cnt --;
        }

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM, "");
        CFM_ENGINE_LocalUpdateRcvdMepMacStatusAndRdiCnt(ccm_p, remote_mep_p, mep_p);
    }

    remote_mep_p->machine_state      = CFM_TYPE_REMOTE_MEP_STATE_OK;
    remote_mep_p->ccm_defect         = FALSE;
    remote_mep_p->rcvd_mep_id        = mep_p->identifier;
    remote_mep_p->rcvd_mep_direction = mep_p->direction;
    remote_mep_p->rcvd_lport         = ccm_p->header_p->lport;

    /*copy mac*/
    memcpy(remote_mep_p->mac_addr_a, ccm_p->header_p->src_mac_a, SYS_ADPT_MAC_ADDR_LEN);

    /*copy chassis id and mangement address*/
    if (NULL!=ccm_p->sender_tlv.type_p)
    {
        /*chassis id*/
        remote_mep_p->sender_chassis_id_length=(UI8_T)*ccm_p->sender_tlv.chassis_id_length_p;
        if (remote_mep_p->sender_chassis_id_length > CFM_TYPE_MAX_CHASSIS_ID_LENGTH)
        {
            remote_mep_p->sender_chassis_id_length = CFM_TYPE_MAX_CHASSIS_ID_LENGTH;
        }
        if (remote_mep_p->sender_chassis_id_length>0)
        {
            remote_mep_p->sender_chassis_id_sub_type = (UI8_T)*ccm_p->sender_tlv.chassis_id_sub_type_p;
            memcpy(remote_mep_p->sender_chassis_id,ccm_p->sender_tlv.chassis_id_p, remote_mep_p->sender_chassis_id_length);
        }

        /*management domain*/
        remote_mep_p->man_domain_length= *(ccm_p->sender_tlv.mgmt_addr_domain_len_p);
        if (remote_mep_p->man_domain_length > CFM_TYPE_MAX_MAN_DOMAIN_LENGTH)
        {
            remote_mep_p->man_domain_length = CFM_TYPE_MAX_MAN_DOMAIN_LENGTH;
        }

        if ( (remote_mep_p->man_domain_length>0)&&
             (NULL!=ccm_p->sender_tlv.mgmt_addr_domain_p))
        {
            memcpy(remote_mep_p->man_domain_address, ccm_p->sender_tlv.mgmt_addr_domain_p, remote_mep_p->man_domain_length);

            /*management address*/
            remote_mep_p->man_length= *(ccm_p->sender_tlv.mgmt_addr_len_p);
            if (remote_mep_p->man_length > CFM_TYPE_MAX_MAN_ADDRESS_LENGTH)
            {
                remote_mep_p->man_length = CFM_TYPE_MAX_MAN_ADDRESS_LENGTH;
            }

            if ((remote_mep_p->man_length>0) && ( NULL != ccm_p->sender_tlv.mgmt_addr_p))
            {
                memcpy(remote_mep_p->man_address, ccm_p->sender_tlv.mgmt_addr_p, remote_mep_p->man_length);
            }
        }
    }

    return TRUE;
}/*End of CFM_ENGINE_RemoteMepOkState*/

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
    void    *timer_para_p)
{
    CFM_OM_REMOTE_MEP_T         remote_mep;
    CFM_Timer_CallBackPara_T    *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "");

    if (FALSE == CFM_OM_GetRemoteMep(para_p->md_index, para_p->ma_index, para_p->mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        return FALSE;
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "remote mep id=%ld", (long)remote_mep.identifier);

    if (CFM_TYPE_CROSS_CHECK_STATUS_DISABLE == remote_mep.ma_p->cross_check_status)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "cross check status is disabled");
        return TRUE;
    }

    /* from ok/start to failed,
     *   rcvd id == 0 => all mep inc ccm lost cnt
     *           != 0 => one mep inc ccm lost cnt
     *   update ma's remote_mep_down_counter
     *
     * from failed to failed,
     *   create local mep later ????
     */
    if (  (CFM_TYPE_REMOTE_MEP_STATE_START == remote_mep.machine_state)
        ||(CFM_TYPE_REMOTE_MEP_STATE_OK == remote_mep.machine_state)
       )
    {
        CFM_OM_MEP_T *tmp_mep_p = &mep_tmp1_g;

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "mep id=%ld", (long)remote_mep.identifier);

        remote_mep.machine_state = CFM_TYPE_REMOTE_MEP_STATE_FAILD;
        remote_mep.failed_ok_time = SYS_TIME_GetSystemTicksBy10ms();

        if (remote_mep.ccm_defect == FALSE)
        {
            remote_mep.ccm_defect = TRUE;
            CFM_ENGINE_XmitCrossCheckTrap(
                remote_mep.md_p, remote_mep.ma_p, remote_mep.rcvd_mep_id,
                remote_mep.identifier, CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_MISSING);

            if (0 != remote_mep.rcvd_mep_id)
            {
                if (  (FALSE == CFM_OM_GetMep(remote_mep.md_index, remote_mep.ma_index,
                                    remote_mep.rcvd_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, tmp_mep_p))
                    ||(tmp_mep_p->direction != remote_mep.rcvd_mep_direction)
                   )
                {
                    /* maybe the old one was replaced by a new one
                     */
                    remote_mep.rcvd_mep_id = 0;
                }
            }

            if (0 == remote_mep.rcvd_mep_id)
            {
                /* reset remote mep to avoid corrupting the mac status counter, etc...
                 */
                remote_mep.rdi              = FALSE;
                remote_mep.interface_status = CFM_TYPE_INTERFACE_STATUS_NO_INTERFACE_STATUS_TLV;
                remote_mep.port_status      = CFM_TYPE_PORT_STATUS_NO_PORT_STATE_TLV;

                if (TRUE == remote_mep.mep_up)
                {
                    remote_mep.mep_up = FALSE;
                    remote_mep.ma_p->remote_mep_down_counter++;
                }

                CFM_ENGINE_LocalUpdateRMepCcmLostCntInOneMa(
                    &remote_mep, NULL, TRUE);
            }
            else
            {
                tmp_mep_p->rmep_ccm_loss_cnt ++;

                /* reset the port/inf/rdi counter for local MEP
                 * bcz the remote MEP is lost
                 */
                CFM_ENGINE_LocalUpdateRcvdMepMacStatusAndRdiCnt(
                    NULL, &remote_mep, tmp_mep_p);

                CFM_ENGINE_UPD_ERR_MAC_STATUS(tmp_mep_p);
                CFM_ENGINE_UPD_SOME_RDI_DEF(tmp_mep_p);
                CFM_ENGINE_UPD_SOME_RMEP_CCM_DEF(tmp_mep_p);
                CFM_ENGINE_UpdateMepHighestDefect(tmp_mep_p);

                CFM_OM_StoreMep(tmp_mep_p);
            }
        }
    }

    if (remote_mep.machine_state == CFM_TYPE_REMOTE_MEP_STATE_FAILD)
    {
        remote_mep.frame_loss++;
    }

    CFM_OM_SetRemoteMep(&remote_mep);

    return TRUE;
}/*End of CFM_ENGINE_RemoteMepOk_Callback*/

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
    void    *timer_para_p)
{
    CFM_Timer_CallBackPara_T    *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;
    CFM_OM_REMOTE_MEP_T         remote_mep;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "");

    if(FALSE == CFM_OM_GetRemoteMep(para_p->md_index, para_p->ma_index, para_p->mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        return FALSE;
    }

    if(CFM_TYPE_REMOTE_MEP_STATE_START == remote_mep.machine_state)
    {
        CFM_Timer_CallBackPara_T para;
        UI32_T cylic_time=0;

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "remote mep id=%ld start delay remote start state", (long)remote_mep.identifier);

        CFM_TIMER_AssignTimerParameter(&para, remote_mep.md_index, remote_mep.ma_index, remote_mep.identifier, 0, 0);
        /* enter fail state
         */
        CFM_ENGINE_RemoteMepOk_Callback(&para);

        if(FALSE == CFM_OM_GetRemoteMep(para_p->md_index, para_p->ma_index, para_p->mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
        {
            return FALSE;
        }

        remote_mep.failed_ok_time=0;

        /*change the timer
         */
        cylic_time=(UI16_T)(CFM_ENGINE_CCMTime(remote_mep.ma_p->ccm_interval)*35/10);
        CFM_TIMER_StartTimer(remote_mep.rmep_while_timer_idx);
        CFM_TIMER_ModifyTimer(remote_mep.rmep_while_timer_idx,
                                              CFM_ENGINE_RemoteMepOk_Callback,
                                              &para,
                                              cylic_time,
                                              CFM_TIMER_CYCLIC);
        CFM_OM_SetRemoteMep(&remote_mep);
    }

    return TRUE;
}/*End of CFM_ENGINE_RemoteStartDelay_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FngStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : Thie function decide fng state machine go to whihc state machine
 * INPUT    : defect_type - CCM defect type
 *            *mep_p      - the mep pointer which process the mep
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_FngStateMachine(
                                        CFM_TYPE_FNG_HighestDefectPri_T defect_type,
                                        CFM_OM_MEP_T *mep_p)
{
    /*if the mep is NULL, it means the original
     mep which receive this remote CCM is deleted'*/
    if(NULL == mep_p)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "mep is already deleted");
        return TRUE;
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE,
                    "mep id=%ld, new defect type=%d, state=%d",
                    (long)mep_p->identifier, defect_type, mep_p->fng_machine_state);

    /* bcz under ERPS env,
     * local MEP                                remote MEP
     * port status block                        port status forward
     * send CCM with port state block TLV ----> get a MAC status error for CCM,
     *                                          then send CCM with RDI
     * recv CCM with RDI,                 <----
     * and keep notifing the RDI defect
     */
    if (mep_p->last_notify_defect != defect_type)
    {
        mep_p->last_notify_defect = defect_type;

        /* try to notify the cfm defect
         */
        switch (defect_type)
        {
        case CFM_TYPE_FNG_HIGHEST_DEFECT_REMOTE_CCM:
        case CFM_TYPE_FNG_HIGHEST_DEFECT_ERROR_CCM:
        case CFM_TYPE_FNG_HIGHEST_DEFECT_XCON_CCM:
        case CFM_TYPE_FNG_HIGHEST_DEFECT_RDI_CCM:
        case CFM_TYPE_FNG_HIGHEST_DEFECT_MAC_STATUS:
            SYS_CALLBACK_MGR_CFM_DefectNotify(
                     SYS_MODULE_CFM,
                     defect_type,
                     mep_p->identifier,
                     mep_p->lport,
                     mep_p->md_p->level,
                     mep_p->ma_p->primary_vid,
                     TRUE);
             break;
        case CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE:
        default:
            SYS_CALLBACK_MGR_CFM_DefectNotify(
                    SYS_MODULE_CFM,
                    defect_type,
                    mep_p->identifier,
                    mep_p->lport,
                    mep_p->md_p->level,
                    mep_p->ma_p->primary_vid,
                    FALSE);
            break;
        }
    }

    /*if the defect type is the same as old, it needn't do advance*/
    if (  (mep_p->highest_pri_defect == defect_type)
        &&(CFM_TYPE_FNG_STATE_CLERING != mep_p->fng_machine_state)
       )
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "highest priority defect is the same");
        return TRUE;
    }

    switch (defect_type)
    {
    case CFM_TYPE_FNG_HIGHEST_DEFECT_RDI_CCM:
        /* RDI of received CCM is on.
         */
        if(CFM_TYPE_FNG_LOWEST_ALARM_ALL != mep_p->lowest_priority_defect)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "");
            return TRUE;
        }
        break;

    case CFM_TYPE_FNG_HIGHEST_DEFECT_MAC_STATUS:
        /* remote MEP's port or interface status is DOWN.
         */
        if(CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON <mep_p->lowest_priority_defect)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "");
            return TRUE;
        }
        break;

    case CFM_TYPE_FNG_HIGHEST_DEFECT_REMOTE_CCM:
        /* CCM of remote MEP ID is lost.
         */
        if(CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON <mep_p->lowest_priority_defect)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "");
            return TRUE;
        }
        break;

    case CFM_TYPE_FNG_HIGHEST_DEFECT_ERROR_CCM:
        /* CCM's content is not correct
         * e.g. Unknown remote MEP ID/Collision of remote & local MEP ID/
         *      CCM Interval not compatible/VID not compatible.
         */
        if(CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON <mep_p->lowest_priority_defect)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "");
            return TRUE;
        }
        break;

    case CFM_TYPE_FNG_HIGHEST_DEFECT_XCON_CCM:
        /* cross connect CCM,
         * e.g. lower level CCM received by higher level MEP.
         *      MAID is wrong in received CCM.
         */
        if(CFM_TYPE_FNG_LOWEST_ALARM_XCON <mep_p->lowest_priority_defect)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "");
            return TRUE;
        }
        break;
    case CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE:
        break;

    default:
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "");
        return TRUE;
    }

    switch(mep_p->fng_machine_state)
    {
    case CFM_TYPE_FNG_STATE_RESET:
        CFM_ENGINE_FngDefectState(defect_type, mep_p);
        break;
    case CFM_TYPE_FNG_STATE_DEFECT:
        CFM_ENGINE_FngDefectState(defect_type, mep_p);
        break;
    case CFM_TYPE_FNG_STATE_REPORTED:
        CFM_ENGINE_FngReportDefectState(defect_type, mep_p);
        break;
    case CFM_TYPE_FNG_STATE_CLERING:
        CFM_ENGINE_FngDefectClearingState(defect_type, mep_p);
        break;
    }

    return TRUE;
}/*End of CFM_ENGINE_FngStateMachine*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FngReportDefectState
 *-------------------------------------------------------------------------
 * PURPOSE  : This function do the fng report defect state job
 * INPUT    : defect_type - the ccm defect type
 *            *mep_p      - the mep pointer which proess the ccm
 * OUTPUT   : None
 * RETURN   : TRUE  -  success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_FngReportDefectState(
                                            CFM_TYPE_FNG_HighestDefectPri_T defect_type,
                                            CFM_OM_MEP_T *mep_p)
{
    if(CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE == defect_type)
    {
        if (CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE !=
                CFM_OM_GetCurrentMaxErrPri(mep_p, FALSE))
            return TRUE;

        mep_p->fng_machine_state = CFM_TYPE_FNG_STATE_CLERING;
        /* should wait reset time
         * mep_p->highest_pri_defect=CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE;
         */

        CFM_ENGINE_FngDefectClearingState(defect_type, mep_p);

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "mep id %ld enter clearing state, time=%lu", (long)mep_p->identifier, (long)SYS_TIME_GetSystemTicksBy10ms());
    }
    else
    {
        if((defect_type > mep_p->highest_pri_defect)||(CFM_TYPE_FNG_STATE_DEFECT == mep_p->fng_machine_state))
        {
            mep_p->highest_pri_defect=defect_type;
            CFM_ENGINE_XmitFaultAlarm(defect_type, mep_p);
        }

        mep_p->fng_machine_state = CFM_TYPE_FNG_STATE_REPORTED;

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "mep id %ld enter defect Report state, time=%lu", (long)mep_p->identifier, (long)SYS_TIME_GetSystemTicksBy10ms());
    }

    return TRUE;
}/*End of CFM_ENGINE_FngReportDefectState*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitFaultAlarm
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will send the trap
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE  -  success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_XmitFaultAlarm(
                                        CFM_TYPE_FNG_HighestDefectPri_T defect_type,
                                        CFM_OM_MEP_T *mep_p)
{
    TRAP_EVENT_TrapData_T     trap_data;
    UI32_T                    trap_style=0;

    /* suppress the TRAP if
     *   AIS received && suppress alarm enabled && crosscheck enabled
     *
     * crosscheck must be enabled already when entering here,
     * so ignore the crosscheck status.
     */
    if (  (-1 != mep_p->ma_p->ais_rcvd_timer_idx)
        &&(CFM_TYPE_AIS_STATUS_ENABLE == mep_p->ma_p->ais_supress_status)
       )
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TRAP, "suppress alarm, mep id %ld  defect type=%d", (long)mep_p->identifier, defect_type);
        return TRUE;
    }
    else
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TRAP, "mep id %ld  defect type=%d", (long)mep_p->identifier, defect_type);
    }

    memset(&trap_data, 0, sizeof(TRAP_EVENT_TrapData_T));

    trap_data.trap_type = TRAP_EVENT_CFM_FAULT_ALARM;
    trap_style = TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP;
    trap_data.community_specified = FALSE;

    trap_data.u.cfm_fault_alarm.instance_dot1agCfmMepHighestPrDefect[0]=mep_p->md_index;
    trap_data.u.cfm_fault_alarm.instance_dot1agCfmMepHighestPrDefect[1]=mep_p->ma_index;
    trap_data.u.cfm_fault_alarm.instance_dot1agCfmMepHighestPrDefect[2]=mep_p->identifier;
    trap_data.u.cfm_fault_alarm.dot1agCfmMepHighestPrDefect=defect_type;

#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_MGR_ReqSendTrap(&trap_data);
#else
    SNMP_PMGR_ReqSendTrap(&trap_data);
#endif

    return TRUE;
}/*End of CFM_ENGINE_XmitFaultAlarm*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitCCTrap
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will send the trap
 * INPUT    : *md_p    - the md pointer
 *            *ma_p    - the ma pointer
 *            mep_id  - the mep id
 *            rmep_id - the remote mep id
 *            trap - the CC trap type
 * OUTPUT   : None
 * RETURN   : TRUE  -  success
 *            FALSE  - fail
 * NOTE     : This function need wait to trap MIB defined
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_XmitCCTrap(
                                    CFM_OM_MD_T *md_p,
                                    CFM_OM_MA_T *ma_p,
                                    UI16_T mep_id,
                                    UI16_T rmep_id,
                                    CFM_TYPE_SnmpTrapsCC_T trap)
{
    TRAP_EVENT_TrapData_T     trap_data;
    UI32_T                    trap_style=0;
    BOOL_T trap_enabled;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TRAP, "trap=%d", trap);

    trap_data.community_specified = FALSE;

    memset(&trap_data,0,sizeof(TRAP_EVENT_TrapData_T));

    if (FALSE == CFM_OM_GetSNMPCcStatus(trap, &trap_enabled))
    {
        return FALSE;
    }

    if(FALSE == trap_enabled)
    {
        return TRUE;
    }

    trap_style = TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP;

    switch(trap)
    {
    case CFM_TYPE_SNMP_TRAPS_CC_CONFIG:
        /* MEP config error,
         *  e.g. remote MEP and local MEP have the same MEP id.
         */
        trap_data.u.cfm_config_fail.instance_dot1agCfmMepIdentifier[0]=md_p->index;
        trap_data.u.cfm_config_fail.instance_dot1agCfmMepIdentifier[1]=ma_p->index;
        trap_data.u.cfm_config_fail.instance_dot1agCfmMepIdentifier[2]=mep_id;
        trap_data.u.cfm_config_fail.dot1agCfmMepIdentifier=mep_id;

        trap_data.trap_type = TRAP_EVENT_CFM_CONFIG_FAIL;
        break;

    case CFM_TYPE_SNMP_TRAPS_CC_LOOP:
        /* received CCM's sa is the same as the received MEP's,
         */
        trap_data.u.cfm_loop_find.instance_dot1agCfmMepIdentifier[0]=md_p->index;
        trap_data.u.cfm_loop_find.instance_dot1agCfmMepIdentifier[1]=ma_p->index;
        trap_data.u.cfm_loop_find.instance_dot1agCfmMepIdentifier[2]=mep_id;
        trap_data.u.cfm_loop_find.dot1agCfmMepIdentifier=mep_id;

        trap_data.trap_type = TRAP_EVENT_CFM_LOOP_FIND;
        break;

    case CFM_TYPE_SNMP_TRAPS_CC_MEP_DOWN:
        /* remote MEP's port status or interface status is DOWN,
         *   e.g. CFM_TYPE_FNG_HIGHEST_DEFECT_MAC_STATUS
         */
        trap_data.u.cfm_mep_down.instance_dot1agCfmMepIdentifier[0]=md_p->index;
        trap_data.u.cfm_mep_down.instance_dot1agCfmMepIdentifier[1]=md_p->index;
        trap_data.u.cfm_mep_down.instance_dot1agCfmMepIdentifier[2]=mep_id;
        trap_data.u.cfm_mep_down.dot1agCfmMepDbRMepIdentifier=rmep_id;

        trap_data.trap_type = TRAP_EVENT_CFM_MEP_DOWN;
        break;

    case CFM_TYPE_SNMP_TRAPS_CC_MEP_UP:
        /* remote MEP's port status or interface status is UP,
         */
        trap_data.u.cfm_mep_up.instance_dot1agCfmMepIdentifier[0]=md_p->index;
        trap_data.u.cfm_mep_up.instance_dot1agCfmMepIdentifier[1]=md_p->index;
        trap_data.u.cfm_mep_up.instance_dot1agCfmMepIdentifier[2]=mep_id;
        trap_data.u.cfm_mep_up.instance_dot1agCfmMepIdentifier[3]=rmep_id;
        trap_data.u.cfm_mep_up.dot1agCfmMepDbRMepIdentifier=rmep_id;

        trap_data.trap_type = TRAP_EVENT_CFM_MEP_UP;
        break;

    default:
        return FALSE;
    }

#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_MGR_ReqSendTrap(&trap_data);
#else
    SNMP_PMGR_ReqSendTrap(&trap_data);
#endif

    return TRUE;
}/*End of CFM_ENGINE_XmitCCTrap*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitCrossCheckTrap
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will send the trap
 * INPUT    : *md_p  - the md pointer
 *            *ma_p  - the ma pointer
 *            mep_id  - the mep id
 *            rmep_id - the remote mep id
 *            trap   - the cross check trap type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function need wait to trap MIB defined
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_XmitCrossCheckTrap(
    CFM_OM_MD_T                     *md_p,
    CFM_OM_MA_T                     *ma_p,
    UI16_T                          mep_id,
    UI16_T                          rmep_id,
    CFM_TYPE_SnmpTrapsCrossCheck_T  trap)
{
    TRAP_EVENT_TrapData_T   trap_data;
    UI32_T                  trap_style=0;
    BOOL_T                  trap_enabled;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TRAP, "Cross Check trap=%d", trap);

    memset(&trap_data, 0, sizeof(TRAP_EVENT_TrapData_T));

    trap_data.community_specified = FALSE;

    /*check if the cross check already enable*/
    if(CFM_TYPE_CROSS_CHECK_STATUS_DISABLE == ma_p->cross_check_status)
    {
        return TRUE;
    }

    if (FALSE == CFM_OM_GetSNMPCrossCheckStatus(trap, &trap_enabled))
    {
        return FALSE;
    }

    if(FALSE == trap_enabled)
    {
        return TRUE;
    }

    trap_style = TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP;

    switch(trap)
    {
    case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MA_UP:
        /* all remote MEPs in one MA is up.
         */
        trap_data.u.cfm_ma_up.instance_dot1agCfmMaIndex[0]=md_p->index;
        trap_data.u.cfm_ma_up.instance_dot1agCfmMaIndex[1]=ma_p->index;
        trap_data.u.cfm_ma_up.dot1agCfmMaIndex=ma_p->index;

        trap_data.trap_type = TRAP_EVENT_CFM_MA_UP;
        break;

    case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_MISSING:
        /* CCM of remote MEP is lost.
         */
        /* suppress the LOC TRAP if
         *   AIS received && suppress alarm enabled && crosscheck enabled
         *
         * crosscheck must be enabled already when entering here,
         * so ignore the crosscheck status.
         */
        if (  (CFM_TYPE_AIS_STATUS_ENABLE == ma_p->ais_supress_status)
            &&(-1 != ma_p->ais_rcvd_timer_idx)
           )
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TRAP, "suppress by AIS");
            return TRUE;
        }

        trap_data.u.cfm_mep_missing.instance_dot1agCfmMepIdentifier[0]=md_p->index;
        trap_data.u.cfm_mep_missing.instance_dot1agCfmMepIdentifier[1]=ma_p->index;
        trap_data.u.cfm_mep_missing.instance_dot1agCfmMepIdentifier[2]=mep_id;
        trap_data.u.cfm_mep_missing.dot1agCfmMepDbRMepIdentifier=rmep_id;

        trap_data.trap_type = TRAP_EVENT_CFM_MEP_MISSING;
        break;

    case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN:
        /* remote MEP id or MD level of received CCM does not exist.
         */
        trap_data.u.cfm_mep_unknown.instance_dot1agCfmMepIdentifier[0]=md_p->index;
        trap_data.u.cfm_mep_unknown.instance_dot1agCfmMepIdentifier[1]=ma_p->index;
        trap_data.u.cfm_mep_unknown.instance_dot1agCfmMepIdentifier[2]=mep_id;
        trap_data.u.cfm_mep_unknown.dot1agCfmMepIdentifier=mep_id;

        trap_data.trap_type = TRAP_EVENT_CFM_MEP_UNKNOWN;
        break;

     default:
        return FALSE;
    }

#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_MGR_ReqSendTrap(&trap_data);
#else
    SNMP_PMGR_ReqSendTrap(&trap_data);
#endif

    return TRUE;
}/*End of CFM_ENGINE_XmitCrossCheckTrap*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FngDefectState
 *-------------------------------------------------------------------------
 * PURPOSE  : The function decide update the defect timer or stop the timer
 * INPUT    : defect_type - the ccm defect type
 *            *mep_p      - the mep pointer which process the ccm
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_FngDefectState(
                                        CFM_TYPE_FNG_HighestDefectPri_T defect_type,
                                        CFM_OM_MEP_T *mep_p)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE,
                    "mep id=%ld, new defect type=%d, state=%d",
                    (long)mep_p->identifier, defect_type, mep_p->fng_machine_state);

    if(CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE==defect_type)
    {
        if(-1 !=mep_p->fng_while_timer_idx)
        {
            /*remove the fngAlarmTime*/
            if (TRUE == CFM_TIMER_StopTimer(mep_p->fng_while_timer_idx))
            {
                CFM_TIMER_FreeTimer(&mep_p->fng_while_timer_idx);
            }
        }

        mep_p->fng_machine_state = CFM_TYPE_FNG_STATE_RESET;
        mep_p->highest_pri_defect = CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE;

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "enter reset state, mep id=%ld, time=%lu", (long)mep_p->identifier, (long)SYS_TIME_GetSystemTicksBy10ms());

        if(-1 !=mep_p->ais_send_timer_idx)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "clear ais send timer");
            if(TRUE == CFM_TIMER_StopTimer(mep_p->ais_send_timer_idx))
            {
                CFM_TIMER_FreeTimer(&mep_p->ais_send_timer_idx);
            }
        }
    }
    else
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "enter defect state, mep id=%ld, time=%lu", (long)mep_p->identifier, (long)SYS_TIME_GetSystemTicksBy10ms());

        if(CFM_TYPE_AIS_STATUS_ENABLE == mep_p->ma_p->ais_status
           && -1 == mep_p->ais_send_timer_idx)
        {
            CFM_Timer_CallBackPara_T    para;;

            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "creat ais timer and xmit ais");

            CFM_ENGINE_XmitAis(mep_p, FALSE); /*transmit AIS*/

            CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);
            mep_p->ais_send_timer_idx=CFM_TIMER_CreateTimer(
                                           CFM_ENGINE_XmitAis_Callback,
                                           &para,
                                           mep_p->ma_p->ais_period == CFM_TYPE_AIS_PERIOD_1S?1:60,
                                           CFM_TIMER_CYCLIC);
            if(mep_p->ais_send_timer_idx!=-1)
                CFM_TIMER_StartTimer(mep_p->ais_send_timer_idx);
        }

        /*if the alarm timer not create then create it, else the alarm timer alrady run*/
        if(-1 == mep_p->fng_while_timer_idx)
        {
            CFM_Timer_CallBackPara_T para;;

            CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);

            /*start the fngAlarmTime*/
            mep_p->fng_while_timer_idx=CFM_TIMER_CreateTimer(
                                          CFM_ENGINE_FngDefectState_Callback,
                                          &para,
                                          mep_p->fng_alarm_time,
                                          CFM_TIMER_ONE_TIME);
            CFM_TIMER_StartTimer(mep_p->fng_while_timer_idx);
        }

        if(defect_type>mep_p->highest_pri_defect)
        {
            mep_p->highest_pri_defect=defect_type;
        }

        mep_p->fng_machine_state = CFM_TYPE_FNG_STATE_DEFECT;
    }

    return TRUE;
}/*end of CFM_ENGINE_FngDefectState*/

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
                                               void *timer_para_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    CFM_Timer_CallBackPara_T *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "");

    if(FALSE == CFM_OM_GetMep(para_p->md_index, para_p->ma_index, para_p->mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        return FALSE;
    }

    CFM_TIMER_FreeTimer(&mep_p->fng_while_timer_idx);

    /*enter report defect state*/
    CFM_ENGINE_FngReportDefectState(mep_p->highest_pri_defect, mep_p);
    CFM_OM_StoreMep(mep_p);
    return TRUE;
}/*End of CFM_ENGINE_FngDefectState_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FngDefectClearingState
 *-------------------------------------------------------------------------
 * PURPOSE  : This function decide to create the fng timer or stop the timer
 *            in clear state
 * INPUT    : defect_type - the ccm defect type
 *            *mep_p      - the mep which enter fng defecte clearing state
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_FngDefectClearingState(
                                                CFM_TYPE_FNG_HighestDefectPri_T defect_type,
                                                CFM_OM_MEP_T *mep_p)
{

    if(CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE==defect_type)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "mep id=%ld, start time=%lu", (long)mep_p->identifier, (long)SYS_TIME_GetSystemTicksBy10ms());

        mep_p->fng_machine_state=CFM_TYPE_FNG_STATE_CLERING;
        /* should wait reset time
         * mep_p->highest_pri_defect=CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE;
         */

        if(-1 == mep_p->fng_while_timer_idx)
        {
            CFM_Timer_CallBackPara_T para;;
            CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);

            /*start the fngRestTime*/
            mep_p->fng_while_timer_idx=CFM_TIMER_CreateTimer(
                                          CFM_ENGINE_FngDefectClearingState_Callback,
                                          &para,
                                          mep_p->fng_reset_time,
                                          CFM_TIMER_ONE_TIME);
            CFM_TIMER_StartTimer(mep_p->fng_while_timer_idx);
        }
    }
    else
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "stop timer and go beck to report defect state, mep id=%ld", (long)mep_p->identifier);

        if(-1 !=mep_p->fng_while_timer_idx)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "stop timer time=%ld", (long)SYS_TIME_GetSystemTicksBy10ms());

            /*remove the fngRestTime*/
            if(TRUE == CFM_TIMER_StopTimer(mep_p->fng_while_timer_idx))
            {
                CFM_TIMER_FreeTimer(&mep_p->fng_while_timer_idx);
            }
        }
        CFM_ENGINE_FngReportDefectState(defect_type, mep_p);
    }

    return TRUE;
}/*End of CFM_ENGINE_FngDefectClearingState*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FngDefectClearingState_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : process the fngResettime t timeout
 * INPUT    : timer_para_p - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_FngDefectClearingState_Callback(
                                                        void *timer_para_p)
{
    CFM_OM_MEP_T *mep_p =&mep_tmp1_g;
    CFM_Timer_CallBackPara_T *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;

    if(FALSE == CFM_OM_GetMep(para_p->md_index, para_p->ma_index, para_p->mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        return FALSE;
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "mep id=%ld", (long)mep_p->identifier);

    CFM_TIMER_FreeTimer(&mep_p->fng_while_timer_idx);

    if (CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE ==
            CFM_OM_GetCurrentMaxErrPri(mep_p, FALSE))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "mep id=%ld, stop timer time=%lu", (long)mep_p->identifier, (long)SYS_TIME_GetSystemTicksBy10ms());

        mep_p->fng_machine_state = CFM_TYPE_FNG_STATE_RESET;
        mep_p->highest_pri_defect = CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE;

        if(-1 !=mep_p->ais_send_timer_idx)
        {
            /* Keep the ais_send_timer if AIS is ever received before
             *   resetting the fng_machine_state(, when crosscheck is disabled).
             *
             * In other words, free the ais_send_timer if AIS never received
             *   or crosscheck is enabled.
             */
            if(  (-1 == mep_p->ma_p->ais_rcvd_timer_idx)
               ||(CFM_TYPE_CROSS_CHECK_STATUS_ENABLE == mep_p->ma_p->cross_check_status)
              )
            {
                if(TRUE == CFM_TIMER_StopTimer(mep_p->ais_send_timer_idx))
                {
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "clear ais send timer");
                    CFM_TIMER_FreeTimer(&mep_p->ais_send_timer_idx);
                }
            }
        }
    }

    CFM_OM_StoreMep(mep_p);

    return TRUE;
}/* End of CFM_ENGINE_FngDefectClearingState_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MepProcessLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To process the recieved LBM packet by specified MEP.
 * INPUT    : lbm_p  - pointer to received LBM packet
 *            mep_p  - pointer to MEP to process the LBM
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MepProcessLBM(
    CFM_ENGINE_LbPdu_T  *lbm_p,
    CFM_OM_MEP_T        *mep_p)
{
    UI8_T   lbm_multicast_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "mep id %ld recevied LBM", (long)mep_p->identifier);

    CFM_ENGINE_ASSEMBLE_DEST_MAC(lbm_multicast_mac,mep_p->md_p->level);

    if (  memcmp(lbm_p->header_p->dst_mac_a, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN)
        &&memcmp(lbm_p->header_p->dst_mac_a,lbm_multicast_mac,SYS_ADPT_MAC_ADDR_LEN)
       )
    {
        /*check if the mep's mac is the pdu's dst mac*/
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "drop packet not recevied mep's MAC");
        return FALSE;
    }

    /*transmit lbr*/
    mep_p->lbr_out++;
    CFM_ENGINE_XmitLBR(lbm_p, mep_p->md_p->level, mep_p->mac_addr_a);

    CFM_OM_StoreMep(mep_p);
    return TRUE;
}/*End of CFM_ENGINE_ProcessLBM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MipProcessLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To process the recieved LBM packet by specified MIP.
 * INPUT    : lbm_p  - pointer to received LBM packet
 *            mip_p  - pointer to MIP to process the LBM
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MipProcessLBM(
    CFM_ENGINE_LbPdu_T  *lbm_p,
    CFM_OM_MIP_T        *mip_p)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "mip recevied LBM at lport =%ld", (long)mip_p->lport);

    /*check if the mip's mac is the pdu's dst mac*/
    if(memcmp(lbm_p->header_p->dst_mac_a, mip_p->mac_address_a, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB,
            "dst mac is not recevied mip's MAC");
        return FALSE;
    }

    /*transmit lbr*/
    CFM_ENGINE_XmitLBR(lbm_p, mip_p->md_p->level, mip_p->mac_address_a);

    return TRUE;
}/*End of CFM_ENGINE_MipProcessLBM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MepProcessLBR
 *-------------------------------------------------------------------------
 * PURPOSE  : Theis function process the received LBR packet
 * INPUT    : *pdu_p    - the packet pointer of the received LBR
 *            *header_p - the common header of the receive LBR
 *            mep_p     - the mep pointe which process the lbm
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MepProcessLBR(
    CFM_ENGINE_LbPdu_T  *lbr_p,
    CFM_OM_MEP_T        *mep_p)
{
    UI8_T *last_lbm_p=mep_p->transmit_lbm_data_a;
    UI32_T received_transId=0;

    memcpy(&received_transId, lbr_p->trans_id_p, sizeof(UI32_T) );
    received_transId=L_STDLIB_Ntoh32(received_transId);

    /* check if the mep's mac is the pdu's dst mac
     */
    if(memcmp(lbr_p->header_p->dst_mac_a, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM|CFM_BACKDOOR_DEBUG_FLAG_LB, "mep mac is not the pdu dst mac");
        return FALSE;
    }

    /* 20.31.1 ProcessLBR()
     * (c) the Loopback Transaction Identifier field of the LBR is
     *     compared to expectedLBRtransID
     */
    if (received_transId != mep_p->lbm_current_recevied_seq_number+1)
    {
        mep_p->lbr_in_out_of_order++;

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM|CFM_BACKDOOR_DEBUG_FLAG_LB,
            "LBR packet out of order cur-seq(%lu), rcv-tid(%lu)",
            (long)mep_p->lbm_current_recevied_seq_number, (long)received_transId);
    }
    else
    {
        mep_p->lbr_in++;
    }

    /* (d) may perform a bit-by-by comparison of the received LBR
     *     against the LBM...
     */
    if (((lbr_p->header_p->md_level<<5)!= last_lbm_p[0]) ||
        (lbr_p->header_p->flags != last_lbm_p[2]) ||
        (lbr_p->header_p->first_tlv_offset != last_lbm_p[3]) ||
        (memcmp(lbr_p->trans_id_p+4, last_lbm_p+8, lbr_p->header_p->pdu_length-8)))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM|CFM_BACKDOOR_DEBUG_FLAG_LB,
            "LBR bad MSDU, packet content is not the same as LBM");
        mep_p->lbr_bad_msdu++;
    }

    /* some implementation didn't copy the content of LBM into LBR,
     * so we skip (d) to relax the checking for CLI display.
     */
    {
        CFM_OM_LbmCtrlRec_T     *lbm_ctrl_rec_p;
        BOOL_T                  add_to_list =TRUE;

        lbm_ctrl_rec_p = CFM_OM_GetLbmCtrlRecPtr();
        if (NULL != lbm_ctrl_rec_p)
        {
            /* for throughput measurement
             */
            if (lbm_ctrl_rec_p->is_busy == TRUE)
            {
                if ((lbm_ctrl_rec_p->beg_trans_id <= received_transId) &&
                    (received_transId <= lbm_ctrl_rec_p->end_trans_id))
                {
                    if (lbm_ctrl_rec_p->time_pass <= 1)
                    {
                        lbm_ctrl_rec_p->rcv_in_1sec++;
                    }
                    else
                    {
                        lbm_ctrl_rec_p->rcv_in_time++;
                    }
                    /* this lbr is for throughput measurement,
                     * doesn't need to add to list.
                     */
                    add_to_list = FALSE;
                }
            }
        }

        if (TRUE == add_to_list)
        {
            UI32_T seq_num=0, rcvd_time=0;
            memcpy(&seq_num, lbr_p->trans_id_p, sizeof(UI32_T));
            seq_num=L_STDLIB_Ntoh32(seq_num);
            rcvd_time=SYS_TIME_GetSystemTicksBy10ms();

            CFM_OM_AddLoopBack(seq_num, rcvd_time, mep_p);
        }
    }

    /* update current receieved LBM sequence. this is for check the sequence out of order or not
     */
    if (received_transId > mep_p->lbm_current_recevied_seq_number)
    {
        mep_p->lbm_current_recevied_seq_number = received_transId;
    }

    CFM_OM_StoreMep(mep_p);
    return TRUE;
}/*End of CFM_ENGINE_MepProcessLBR*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_VerifyAndDecomposeLB
 *-------------------------------------------------------------------------
 * PURPOSE  : This function verify the receive LBM or LBR packet
 * INPUT    : *pdu_p    - the pdu content pointer
 *            *header_p - the common head pointer
 *            *lb_p     - the lbr content pointer point to the receive lbr/lbm packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_VerifyAndDecomposeLB(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_LbPdu_T      *lb_p)
{
    {/*first tlv offset is 4*/
        if(header_p->first_tlv_offset != 4)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "first tlv offset fail ");
            return FALSE;
        }
    }

    /*common header has no problem assign th header.*/
    lb_p->header_p=header_p;

    /*transaction id*/
    lb_p->trans_id_p=(pdu_p+4);

    {/*option tlv*/
        UI8_T *tlv=NULL, tlv_type=0;
        UI16_T tlv_length=0;
        UI32_T len_accum=8, pdu_len=header_p->pdu_length;

        tlv=(pdu_p+8);
        while(TRUE)
        {
            tlv_type=*tlv;
            memcpy(&tlv_length, &tlv[1], sizeof(UI16_T));
            tlv_length = L_STDLIB_Ntoh16(tlv_length);

            /*check the tlv type*/
            switch(tlv_type)
            {
            case CFM_TYPE_TLV_SENDER_ID:
                if(FALSE == CFM_ENGINE_DecomposeAndVerifySenderTlv(
                                header_p, &lb_p->sender_tlv, tlv))
                {
                    return FALSE;
                }
                break;
            case CFM_TYPE_TLV_DATA:
                lb_p->data_type_p=tlv;
                lb_p->data_length_p=tlv+1;
                lb_p->data_p=tlv+3;
                break;
            case CFM_TYPE_TLV_ORGANIZATION:
                if(FALSE == CFM_ENGINE_DecomposeAndVerifyOrganizationTlv(&lb_p->org_tlv, tlv))
                {
                    return FALSE;
                }
                break;
            case CFM_TYPE_TLV_END:
                return TRUE;
            default:
                break;
            }

            /*accum tlv length over pdu length*/
            len_accum+=(tlv_length+3);
            if (len_accum>pdu_len)
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "accume tlv length over packet lengthl ");
                return FALSE;
            }

            tlv=(tlv+tlv_length+3);
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "verify fail ");
    return FALSE;
}/*End of CFM_ENGINE_VerifyAndDecomposeLB*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalIsValidDstMac
 *-------------------------------------------------------------------------
 * PURPOSE  : To check if dst mac is valid for LBM/LTM/DMM
 * INPUT    : dst_mac_p    - pointer to dst mac to check
 *            md_level     - md level to check
 *            is_allow_mc  - TRUE if multicast MAC is allowed
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : 1. LTM     can not use multicast group address
 *            2. LBM/DMM can only use ccm group destination mac address
 *                                    (Multicast Class 1 in Y.1731)
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_LocalIsValidDstMac(
    UI8_T   *dst_mac_p,
    UI8_T   md_level,
    BOOL_T  is_allow_mc)
{
    UI8_T bc_mac[SYS_ADPT_MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    UI8_T nu_mac[SYS_ADPT_MAC_ADDR_LEN] = {0, 0, 0, 0, 0, 0};
    UI8_T mc_mac[SYS_ADPT_MAC_ADDR_LEN] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x30};
    BOOL_T  ret = FALSE;

    if (NULL != dst_mac_p)
    {
        if (  (memcmp (dst_mac_p, bc_mac, SYS_ADPT_MAC_ADDR_LEN))
            &&(memcmp (dst_mac_p, nu_mac, SYS_ADPT_MAC_ADDR_LEN))
           )
        {
            if (0 == (dst_mac_p[0] & 0x1))
            {
                ret = TRUE;
            }
            else
            {
                if (TRUE == is_allow_mc)
                {
                    mc_mac[5] |= md_level; /* ccm group */

                    if (0 == memcmp(dst_mac_p, mc_mac, SYS_ADPT_MAC_ADDR_LEN))
                    {
                        ret = TRUE;
                    }
                }
            } /* if (0 == (dst_mac_p[0] & 0x1)) */
        }
    } /* if (NULL != dst_mac_p) */

    return ret;
}

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
    UI32_T      transmit_mep_id)
{
    CFM_OM_MEP_T    *mep_p=&mep_tmp2_g;
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    UI8_T           *pdu_p=NULL;
    UI32_T          egress_port=0;
    UI16_T          pdu_len = 0;
    UI8_T           target_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

    /* 0. if global lbm control record is busy, return false
     *    allow one user to do lbm at a time only...
     */

    /* 1. first find the remote mep or the local mep to send the lbm
     */
    if(FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_ap, md_name_len, ma_name_ap, ma_name_len, &md_p, &ma_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "can't get the md ma by ma name");
        return FALSE;
    }

    if (  (dst_mep_id == 0)
        &&(FALSE == CFM_ENGINE_LocalIsValidDstMac(
                    dst_mac, md_p->level, TRUE))
       )
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "invalid mac");
        return FALSE;
    }

    if(FALSE == CFM_ENGINE_FindTargetMacAndMep(
        md_p->index, ma_p->index, ma_p->primary_vid, dst_mep_id, dst_mac,
        CFM_BACKDOOR_DEBUG_FLAG_LB, target_mac, &egress_port, mep_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "can't find target mac and mep");
        return FALSE;
    }

    if(FALSE == mep_p->tramsmit_lbm_status)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "transmit lbm status is false");
        return FALSE;
    }

    /* if choose to use specify mep to transmit lbm
     */
    if((0!= transmit_mep_id)&&(mep_p->identifier != transmit_mep_id))
    {
        if(FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, transmit_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "can't get mep");
            return FALSE;
        }

        if(CFM_TYPE_MP_DIRECTION_DOWN ==mep_p->direction)
        {
            egress_port = mep_p->lport;
        }
    }

    /* should not use mep's da for destination mac
     */
    if (0 == memcmp(mep_p->mac_addr_a, dst_mac, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "target is mep mac");
        return FALSE;
    }

    /* 2. check cfm status.
     */
    {
        CFM_TYPE_CfmStatus_T global_status=CFM_TYPE_CFM_STATUS_DISABLE,port_status=CFM_TYPE_CFM_STATUS_DISABLE;

        CFM_OM_GetCFMGlobalStatus(&global_status);
        CFM_OM_GetCFMPortStatus(mep_p->lport, &port_status);

        /* if the port's and global status not enable, the ccm won't send
         */
        if((CFM_TYPE_CFM_STATUS_ENABLE != global_status)||(CFM_TYPE_CFM_STATUS_ENABLE!=port_status))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "mep global status is fail");
            return FALSE;
        }
    }

    mep_p->transmit_lbm_result_oK = FALSE;

    /* 3. start to send the lbm
     */
    /* assign transmit sequence number
     */
    mep_p->transmit_lbm_seq_number=mep_p->next_lbm_trans_id;

    /* save the transmit data,
     * 1. free old data it. 2. save the new data
     */
    pdu_p = mep_p->transmit_lbm_data_a;
    memset(pdu_p, 0, sizeof(mep_p->transmit_lbm_data_a));

    CFM_ENGINE_ConstructLBPDU(
        pdu_p, &pdu_len, mep_p, CFM_TYPE_OPCODE_LBM,
        mep_p->transmit_lbm_data_tlv_length, 0xff);

     /*send lbm*/
    if(CFM_TYPE_MP_DIRECTION_DOWN==mep_p->direction)
    {
        CFM_ENGINE_XmitPDU(
            mep_p->lport,
            mep_p->primary_vid,
            mep_p->md_p->level,
            mep_p->transmit_lbm_vlan_priority,
            FALSE,
            target_mac,
            mep_p->mac_addr_a,
            CFM_TYPE_MP_DIRECTION_DOWN,
            pdu_p,
            pdu_len);

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "LBM tx thorough port %ld, transaction id %ld", (long)mep_p->lport, (long)mep_p->next_lbm_trans_id);
    }
    else if (0!=egress_port)
    {
        CFM_ENGINE_XmitPDU(
            egress_port,
            mep_p->primary_vid,
            mep_p->md_p->level,
            mep_p->transmit_lbm_vlan_priority,
            FALSE,
            target_mac,
            mep_p->mac_addr_a,
            CFM_TYPE_MP_DIRECTION_UP,
            pdu_p,
            pdu_len);

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "LBM tx thorough port %ld, transaction id %ld", (long)egress_port, (long)mep_p->next_lbm_trans_id);
    }
    else /*mep up and no egress port*/
    {
        CFM_ENGINE_FloodPDU(
            mep_p->lport,
            mep_p->primary_vid,
            target_mac,
            mep_p->mac_addr_a,
            pdu_p,
            pdu_len,
            TRUE);

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "LBM flood through port %ld, transaction id %ld", (long)mep_p->lport, (long)mep_p->next_lbm_trans_id);
    }

    /*increase next trasaction id*/
    mep_p->next_lbm_trans_id++;
    mep_p->transmit_lbm_result_oK = TRUE;

    CFM_OM_StoreMep(mep_p);
    return TRUE;
}/*End of CFM_ENGINE_XmitLBM*/
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
void CFM_ENGINE_ClearLoopBackList()
{
    CFM_OM_ClearLoopBackList();
    return;
}/*End of CFM_ENGINE_ClearLoopBackList*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitLBR
 *-------------------------------------------------------------------------
 * PURPOSE  : This function transmit the LBR
 * INPUT    : *lbr_p   -the lbr pointer pointe to the will be transmitted lbr
 *            level    - the md level
 *            src_addr - the src mac address
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_XmitLBR(
    CFM_ENGINE_LbPdu_T  *lbr_p,
    CFM_TYPE_MdLevel_T  level,
    UI8_T               src_addr[SYS_ADPT_MAC_ADDR_LEN])
{
    UI8_T *rcvd_pdu_p=NULL;
    UI8_T *tran_pdu_p=NULL;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LB, "LBR transmit throught port %ld", (long)lbr_p->header_p->lport);

    if(NULL == (tran_pdu_p=(UI8_T *)L_MM_Malloc(
                            lbr_p->header_p->pdu_length,
                            L_MM_USER_ID2(SYS_MODULE_CFM, CFM_TYPE_XMITLBR))))
    {
        return;
    }

    rcvd_pdu_p=(lbr_p->trans_id_p-4); /*point to the lbm pdu start*/

    memcpy(tran_pdu_p, rcvd_pdu_p, lbr_p->header_p->pdu_length);

    *(tran_pdu_p+1)=(UI8_T)CFM_TYPE_OPCODE_LBR;

    /*trnsmit the LBR*/
    CFM_ENGINE_XmitPDU(lbr_p->header_p->lport, lbr_p->header_p->tag_info&0x0fff, level,( lbr_p->header_p->tag_info&0xE000)>>13, FALSE,
                       lbr_p->header_p->src_mac_a,src_addr, CFM_TYPE_MP_DIRECTION_DOWN, tran_pdu_p, lbr_p->header_p->pdu_length);

    L_MM_Free(tran_pdu_p);
    return;
}/*End of CFM_ENGINE_XmitLBR*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructLBPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : This fnction construct the LBM/LBR pdu
 * INPUT    : pdu_p     - the packet content pointer
 *            pdu_len_p - the curretn packet content length
 *            mep_p     - the mep pointer which contruct the LBR/LBM
 *            opcode    - opcode for LB PDU
 *            data_len  - the length of data TLV
 *            pattern   - the pattern included in data TLV
 * OUTPUT   : pdu_len_p - the current packet content length
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructLBPDU(
    UI8_T               *pdu_p,
    UI16_T              *pdu_len_p,
    CFM_OM_MEP_T        *mep_p,
    CFM_TYPE_OpCode_T   opcode,
    UI16_T              data_len,
    UI16_T              pattern)
{
    /* 1. constrcut common header, common header has 4 bytes, the end of tlv will be add at the end of pdu
     */
    CFM_ENGINE_ConstructCommonHeader(pdu_p, pdu_len_p, mep_p->md_p->level,0, opcode);

    /* move to next octect
     */
    pdu_p+=*pdu_len_p;

    {/* loop back transaction identifier
      */
        UI32_T trans_id=L_STDLIB_Hton32(mep_p->next_lbm_trans_id);

        memcpy(pdu_p, &trans_id, sizeof(UI32_T));

        /*move to next octect, update pdu length
         */
        *pdu_len_p+=4;
        pdu_p+=4;
    }
    {/* 6. construct tlv
      */
        UI16_T tlv_len=0;

        CFM_ENGINE_ConstructSenderTLV(pdu_p, &tlv_len, mep_p->lport);
        pdu_p+=tlv_len;
        *pdu_len_p+=tlv_len;

     /* 7 construct the data tlv
      */
        if (data_len > 0)
        {
            CFM_ENGINE_ConstructDataTLV(
                pdu_p, &tlv_len, data_len, pattern);
            pdu_p+=tlv_len;
            *pdu_len_p+=tlv_len;
        }

     /* 8. construct end tlv
      */
        CFM_ENGINE_ConstructEndTLV(pdu_p, &tlv_len);
        *pdu_len_p+=tlv_len;
    }

}/*end of CFM_ENGINE_ConstructLBPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessLTM
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the LTM packet
 * INPUT    : *pdu_p    - the packet pointer of the received packet
 *            *header_p - the common header pointer of the received packet
 * OUTPUT   : None
 * RETURN   : TRUE  -  success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ProcessLTM(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p)
{
    CFM_ENGINE_LtmPdu_T     ltm;
    CFM_OM_MEP_T            *mep_p=&mep_tmp1_g,
                            *high_level_mep_p=&mep_tmp2_g;
    CFM_OM_MIP_T            mip;
    CFM_TYPE_RelayAction_T  relay_action=CFM_TYPE_RELAY_ACTION_FDB;
    CFM_TYPE_MpType_T       mp_type=CFM_TYPE_MP_TYPE_NONE;
    UI32_T                  egress_port=0;
    BOOL_T                  is_forward;

    memset(&ltm,0,sizeof(CFM_ENGINE_LtmPdu_T));

    /*decompose the ltm pdu*/
    if(FALSE == CFM_ENGINE_VerifyAndDecomposeLTM(pdu_p, header_p, &ltm))
    {
        return FALSE;
    }

    /*get mp on the received port*/
    mp_type= CFM_ENGINE_GetMpProcessRcvdPacket(header_p, mep_p, &mip);

    /* 1. the ingress port has no mp to process it--> check all port's mp to process or pass through
       2. the ingress port has mp to process t -> process it
      */
    if(CFM_TYPE_MP_TYPE_NONE == mp_type)
    {
        CFM_OM_MD_T     *md_p   =NULL;
        CFM_OM_MA_T     *ma_p   =NULL;
        UI32_T          lport=0;

        if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                    header_p->lport, header_p->md_level, header_p->tag_info&0x0fff,
                    CFM_TYPE_MP_DIRECTION_UP_DOWN, FALSE,
                    high_level_mep_p, NULL))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "Drop packet, LTM has higher Level MEP");
            return FALSE;
        }

        if(FALSE == header_p->is_ing_lport_fwd)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "Ingress port %ld is not a forwarding port", (long)header_p->lport);
            return FALSE;
        }

        if (FALSE == CFM_OM_GetMdMaByLevelVid(header_p->md_level, header_p->tag_info&0x0fff, &md_p, &ma_p))
        {
            /* can not find the md/ma, so forward the pdu to other port
             */
            CFM_ENGINE_DoEgrProcForForwarding(
                header_p, pdu_p, header_p->pdu_length, TRUE);
            return FALSE;
        }

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "LTM Forward to Other port, Ingress port has no MP, so check other port's MP");

        /*the received port has no mep, so flood to other port
         *1. if the other port has up mep -> do received by up mep
         *2. if the other port has down mep -> skip it
         *3. if the other port has no mep  ->pass through this port
         */
        while(SWCTRL_LPORT_UNKNOWN_PORT!= SWCTRL_GetNextLogicalPort(&lport))
        {
            /*skip the the port which received ltm*/
            if(lport == header_p->lport)
            {
                continue;
            }

            /*forward to other port, we need check other port has higher leve mep*/
            if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                        lport, header_p->md_level, header_p->tag_info&0x0fff,
                        CFM_TYPE_MP_DIRECTION_UP_DOWN, FALSE,
                        high_level_mep_p, NULL))
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "lport:%lu. has higher mep", (long)lport);
                continue;
            }

            if(FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
            {
                if(FALSE == CFM_OM_GetMip(md_p->index, ma_p->index, lport, CFM_OM_LPORT_MD_MA_KEY, &mip))
                {
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "Pass Through, LTM was pass through port %ld", (long)lport);

                    /* this port has no mp -> pass through this port
                     * should be ok to use old da for forwarding
                     */
#if 0
                    CFM_ENGINE_ASSEMBLE_DEST_MAC(dst_mac, header_p->md_level);  /*calculate the dst mac*/
#endif

                    CFM_ENGINE_XmitPDU(lport,
                                       (header_p->tag_info&0x0fff),
                                       header_p->md_level,
                                       (header_p->tag_info&0xe000)>>13,
                                       TRUE,
                                       header_p->dst_mac_a,
                                       header_p->src_mac_a,
                                       CFM_TYPE_MP_DIRECTION_UP, /*becaues forward to other port from cpu*/
                                       pdu_p,
                                       header_p->pdu_length);
                    continue;
                }
                else
                {
                    /* 20.42.1.6 LTM is received by an Up MHF
                     */
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "MIP Process, LTM was processsed by UP MHF");

                    /* process by up mhf
                      */
                    if(memcmp(mip.mac_address_a, ltm.target_mac_p, SYS_ADPT_MAC_ADDR_LEN))
                    {
                        /*target mac is not this port mac*/
                        if(FALSE == CFM_ENGINE_FindLtmEgressPort(md_p, ma_p, ltm.target_mac_p, header_p , &egress_port, &relay_action))
                        {
                            continue;
                        }

                        /* 20.42.1.6 LTM is received by an Up MHF
                         * b) ...if the Egress Port is not the Bridge Port on which the receiving MHF is configured,
                         * then the LTM is discarded, and no further processing takes place.
                         */
                        if(egress_port != lport)
                        {
                            continue;
                        }

                        is_forward = CFM_ENGINE_ForwardLTM(&ltm, egress_port);

                        CFM_ENGINE_EnqueueLTR(&ltm, header_p->lport, egress_port,
                            is_forward, relay_action, md_p, ma_p, FALSE, FALSE, TRUE, mip.mac_address_a);
                        continue;
                    }
                    else /* target is the mip */
                    {
                        CFM_ENGINE_EnqueueLTR(&ltm, header_p->lport, 0, FALSE,
                            CFM_TYPE_RELAY_ACTION_HIT, md_p, ma_p, FALSE, FALSE, FALSE, mip.mac_address_a);
                        continue;
                    }
                }
            }
            else
            {
                /* process by up mep
                 */
                /*meet down mep -> skip it*/
                if(CFM_TYPE_MP_DIRECTION_DOWN == mep_p->direction)
                {
                    continue;
                }

                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "MEP Process, LTM was processsed by up MEP");

                if(memcmp(mep_p->mac_addr_a, ltm.target_mac_p, SYS_ADPT_MAC_ADDR_LEN))
                {   /*target mac is not this port mac*/
                    if(FALSE == CFM_ENGINE_FindLtmEgressPort(md_p, ma_p, ltm.target_mac_p, header_p, &egress_port, &relay_action))
                    {
                        continue;
                    }

                    /* 20.42.1.5 LTM is received by an Up MEP
                     * b) ... if the Egress Port is not the Bridge Port on which the receiving MEP is configured,
                     * then the LTM is discarded and no further processing takes place.
                     */
                    if(egress_port != lport)
                    {
                        continue;
                    }

                    CFM_ENGINE_EnqueueLTR(&ltm, header_p->lport, egress_port,
                        FALSE, relay_action, md_p, ma_p, TRUE, FALSE, TRUE, mep_p->mac_addr_a);
                    continue;
                }
                else
                {
                    CFM_ENGINE_EnqueueLTR(&ltm, header_p->lport, 0, FALSE,
                        CFM_TYPE_RELAY_ACTION_HIT, md_p, ma_p, FALSE, FALSE, FALSE, mep_p->mac_addr_a);
                    continue;
                } /* if(memcmp(mep_p->mac_addr_a ... */
            } /* if(FALSE == CFM_OM_GetMep(...*/
        } /* while(SWCTRL_LPORT_UNKNOWN_PORT != ... */
        return TRUE;
    }
    else if(CFM_TYPE_MP_TYPE_MEP == mp_type)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "MEP Process, LTM was received by down MEP");

        if(CFM_TYPE_MP_DIRECTION_UP==mep_p->direction)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "MEP Process, LTM was received by up MEP from back side");
            return FALSE;
        }

        /* received by down mep
         */
        if(memcmp(mep_p->mac_addr_a, ltm.target_mac_p, SYS_ADPT_MAC_ADDR_LEN))
        {
            /*target mac is not this port mac*/

            if(FALSE == header_p->is_ing_lport_fwd)
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "port is not forwarding");
                return TRUE;
            }

            if(FALSE == CFM_ENGINE_FindLtmEgressPort(mep_p->md_p, mep_p->ma_p, ltm.target_mac_p, header_p, &egress_port, &relay_action))
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "can't find egress port");
                return FALSE;
            }

            CFM_ENGINE_EnqueueLTR(&ltm, header_p->lport, 0, FALSE, relay_action, mep_p->md_p, mep_p->ma_p, TRUE, TRUE, FALSE, NULL);
        }
        else
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "enqueue LTR");

            CFM_ENGINE_EnqueueLTR(&ltm, header_p->lport, 0, FALSE, CFM_TYPE_RELAY_ACTION_HIT, mep_p->md_p, mep_p->ma_p, TRUE, TRUE, FALSE, NULL);
            return TRUE;
        } /* if(memcmp(mep_p->mac_addr_a, ... */
    }
    else
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "MEP Process, LTM was received by down MHF");

        if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                    header_p->lport, header_p->md_level, header_p->tag_info&0x0fff,
                    CFM_TYPE_MP_DIRECTION_UP_DOWN, FALSE, high_level_mep_p, NULL))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "received by higher level mp");
            return FALSE;
        }

        /* received by down mip
         */
        CFM_ENGINE_DownMhfProcessLTM(&mip, &ltm);
    } /* if(CFM_TYPE_MP_TYPE_NONE == mp_type) */

    return TRUE;
}/*End of CFM_ENGINE_ProcessLTM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_DownMhfProcessLTM
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the LTM packet
 * INPUT    : *mip_p  - the mip pointer
 *            rcvd_loprt - the port which recevied the LTM
 *            *ltm_p   - the ltm pointer which point to the packet content pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_DownMhfProcessLTM(
    CFM_OM_MIP_T        *mip_p,
    CFM_ENGINE_LtmPdu_T *ltm_p)
{
    CFM_TYPE_RelayAction_T  relay_action=CFM_TYPE_RELAY_ACTION_HIT;
    CFM_ENGINE_PduHead_T    *header_p;
    CFM_OM_MD_T             *md_p;
    CFM_OM_MA_T             *ma_p;
    UI32_T                  egress_port=0;
    BOOL_T                  is_matched_cpu=FALSE,
                            is_same_lvl_down_mep_at_egr = FALSE,
                            is_forward;

    if((NULL == mip_p)||(NULL == ltm_p))
    {
        return FALSE;
    }

    md_p    = mip_p->md_p;
    ma_p    = mip_p->ma_p;
    header_p= ltm_p->header_p;

    /* target mac is not this port mac or cpu mac
     */
    if(FALSE == CFM_ENGINE_IsArrivedTargetMacEx(
                    header_p->lport,                ltm_p->target_mac_p,
                    CFM_TYPE_MP_DIRECTION_UP_DOWN,  &is_matched_cpu))
    {
        if(FALSE == header_p->is_ing_lport_fwd)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "Ingress port %ld is not a forwarding port",  (long)header_p->lport);
            return FALSE;
        }

        {  /* find the egress port and enqueue of forward ltm*/
            CFM_OM_MEP_T *egress_mep_p=&mep_tmp2_g;
            CFM_OM_MIP_T egress_mip;
            CFM_OM_MEP_T *high_level_mep_p=&mep_tmp3_g;

            if(FALSE == CFM_ENGINE_FindLtmEgressPort(md_p, ma_p, ltm_p->target_mac_p, header_p, &egress_port, &relay_action))
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "Can't find egress port");
                return FALSE;
            }

            /* 20.42.1.4.f egress meet up mhf
             * 1) ProcessLTM() calls enqueLTR to enqueue an LTR for the MHF
             *    Linktrace SAP of the Egress Port's Up MHF.
             *
             * so let's use the egress mip's mac for source mac of LTR
             */
            if(TRUE == CFM_OM_GetMip(md_p->index, ma_p->index, egress_port, CFM_OM_LPORT_MD_MA_KEY, &egress_mip))
            {
                if(TRUE == CFM_ENGINE_IsArrivedTargetMac(egress_port, ltm_p->target_mac_p, CFM_TYPE_MP_DIRECTION_DOWN))
                {
                    /*target mac*/
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "enqueue LTR");

                    relay_action=CFM_TYPE_RELAY_ACTION_HIT;
                    CFM_ENGINE_EnqueueLTR(ltm_p, header_p->lport, egress_port, FALSE, relay_action, md_p, ma_p, FALSE, TRUE, FALSE, egress_mip.mac_address_a);

                    return TRUE;
                }

                is_forward = CFM_ENGINE_ForwardLTM(ltm_p, egress_port);
                CFM_ENGINE_EnqueueLTR(ltm_p, header_p->lport, egress_port, is_forward, relay_action, md_p, ma_p, FALSE, TRUE, TRUE, egress_mip.mac_address_a);
                return TRUE;
            }

            /* 20.42.1.4.g egress meet up mep
             */
            if(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, egress_port, CFM_OM_LPORT_MD_MA_KEY, egress_mep_p))
            {
                /* egress meet up MEP, target mac is not cpu mac
                 */
                if(CFM_TYPE_MP_DIRECTION_UP == egress_mep_p->direction)
                {
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "enqueue LTR");

#if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC == TRUE)
                    if (0 == memcmp(egress_mep_p->mac_addr_a, ltm_p->target_mac_p, SYS_ADPT_MAC_ADDR_LEN))
                    {
                        /* target mac is up mep's mac
                         */
                        CFM_ENGINE_EnqueueLTR(ltm_p, header_p->lport, 0, FALSE,
                            CFM_TYPE_RELAY_ACTION_HIT, md_p, ma_p, FALSE, TRUE, FALSE, ltm_p->target_mac_p);
                    }
                    else
#endif /* #if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC == TRUE) */
                    {
                        /* if TerminalMEP is set, then FwdYes must not be set
                         */
                        CFM_ENGINE_EnqueueLTR(ltm_p, header_p->lport, egress_port,
                            FALSE, relay_action, md_p, ma_p, TRUE, TRUE, TRUE, egress_mep_p->mac_addr_a);
                    }
                    return TRUE;
                }
                else
                {
                    is_same_lvl_down_mep_at_egr = TRUE;
                }
            }

            /*20.42.1.4.h check if there is a higher level up mep exist*/
            if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                        egress_port, mip_p->md_p->level, mip_p->ma_p->primary_vid,
                        CFM_TYPE_MP_DIRECTION_UP, FALSE, high_level_mep_p, NULL))
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "exist higher level mep");
                return FALSE;
            }

            /*20.42.1.4.i: nor same level or higher leve up mep or up mhf exist, enqueue and forward ltm*/
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "enqueue LTR, egress port %ld", (long)egress_port);

                /*if egress port meet down mep. then this ltm will be drop by that mep. we needn't to forward this ltm*/
                is_forward =(  (!is_same_lvl_down_mep_at_egr)
                             &&(CFM_ENGINE_ForwardLTM(ltm_p, egress_port)));

                /* forwarded will be TRUE if is_same_lvl_down_mep_at_egr == FALSE
                 */
                CFM_ENGINE_EnqueueLTR(ltm_p, header_p->lport, egress_port,
                    is_forward, relay_action, md_p, ma_p, FALSE, TRUE, FALSE, NULL);

                return TRUE;
            }
         } /* end find the egress port and enqueue of forward ltm*/
    }
    else
    {
        /* matched cpu mac or mip mac
         */
        if (TRUE == is_matched_cpu)
        {
            UI8_T   is_same_level_up_mep_exist = FALSE;

#if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE)
            is_same_level_up_mep_exist =
                CFM_ENGINE_IsExistSameLevelMp(md_p, ma_p, CFM_TYPE_MP_DIRECTION_UP);
#endif /* #if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE) */

            if (FALSE == is_same_level_up_mep_exist)
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "enqueue LTR");

                /* 20.42.1.4.i, this LTM has reached the cpu,
                 *              and has nowhere to go.
                 */
                CFM_ENGINE_EnqueueLTR(ltm_p, header_p->lport, 0, FALSE,
                    CFM_TYPE_RELAY_ACTION_HIT, md_p, ma_p,
                    FALSE, TRUE, FALSE, NULL);
            }
            else
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "enqueue LTR");

                /* 20.42.1.4.g egress meet up mep, target mac is cpu mac
                 */
                CFM_ENGINE_EnqueueLTR(ltm_p, header_p->lport, 0, FALSE,
                    CFM_TYPE_RELAY_ACTION_HIT, md_p, ma_p,
                    FALSE, TRUE, FALSE, ltm_p->target_mac_p);
            }
        }
        else
        {
            /* 20.42.1.4.b target mac is mip
             */
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "enqueue LTR");
            CFM_ENGINE_EnqueueLTR(ltm_p, header_p->lport, 0, FALSE,
                CFM_TYPE_RELAY_ACTION_HIT, md_p, ma_p, FALSE, TRUE, FALSE, NULL);
        }
        return TRUE;
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "function end fail");
    return FALSE;
}/*End of CFM_ENGINE_DownMhfProcessLTM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_IsExistHigherLevelMp
 *-------------------------------------------------------------------------
 * PURPOSE  : This funciton check higher level up direction mep exist or mip exists
 * INPUT    : checked_lport - the lport will be checked
 *            checked_level - the md level will be checked
 *            checked_vid   - the vid will be checked
 *            direction     - the check mep direction
 *            check_mip     - check the mip or not
 * OUTPUT   : mep_pp        - the mep pointer
 *            mip_pp        - the mip pointer
 * RETURN   : TRUE  - higher leve mp exists
 *            FALSE - higher leve mp not exists
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_IsExistHigherLevelMp(
    UI32_T                  checked_lport,
    CFM_TYPE_MdLevel_T      checked_Level,
    UI16_T                  checked_vid,
    CFM_TYPE_MP_Direction_T direction,
    BOOL_T                  check_mip,
    CFM_OM_MEP_T            *mep_p,
    CFM_OM_MIP_T            *mip_p)
{
    UI32_T nxt_md_index=0, nxt_ma_index=0;

    while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, checked_lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        if(checked_lport != mep_p->lport )
        {
            break;
        }

        if( (mep_p->md_p->level > checked_Level)&& (checked_vid == mep_p->primary_vid))
        {
            if(CFM_TYPE_MP_DIRECTION_UP_DOWN == direction)
            {
                return TRUE;
            }
            else if(direction==mep_p->direction)
            {
                return TRUE;
            }
        }

       nxt_md_index=mep_p->md_p->index;
       nxt_ma_index=mep_p->ma_p->index;
    }

    if(FALSE == check_mip)
    {
        return FALSE;
    }

    nxt_md_index=0;
    nxt_ma_index=0;

    while(TRUE == CFM_OM_GetNextMip(nxt_md_index, nxt_ma_index, checked_lport, CFM_OM_LPORT_MD_MA_KEY, mip_p))
    {
        if(checked_lport != mip_p->lport )
        {
            break;
        }

        if((mip_p->md_p->level>checked_Level)&&(checked_vid == mip_p->ma_p->primary_vid))
        {
            return TRUE;
        }

        nxt_md_index=mip_p->md_p->index;
        nxt_ma_index=mip_p->ma_p->index;
    }

    return FALSE;
}/*End of CFM_ENGINE_IsExistHigherLevelMp*/

#if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_IsExistSameLevelMp
 *-------------------------------------------------------------------------
 * PURPOSE  : This funciton check same level exist or not
 * INPUT    : md_p      - the md pointer
 *            ma_p      - the ma pinter
 *            direction - mep direction
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_IsExistSameLevelMp(
    CFM_OM_MD_T             *md_p,
    CFM_OM_MA_T             *ma_p,
    CFM_TYPE_MP_Direction_T direction)
{
    CFM_OM_MEP_T    *tmp_mep_p=&mep_tmp3_g;
    UI32_T          nxt_mep_id=0;

    while(TRUE == CFM_OM_GetNextMep(md_p->index, ma_p->index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, tmp_mep_p))
    {
        nxt_mep_id = tmp_mep_p->identifier;
        if((md_p->index != tmp_mep_p->md_index)||(ma_p->index!=tmp_mep_p->ma_index))
        {
            break;
        }
        if(CFM_TYPE_MP_DIRECTION_UP_DOWN == direction)
        {
            return TRUE;
        }
        else if(direction==tmp_mep_p->direction)
        {
            return TRUE;
        }
    }

    return FALSE;
}
#endif /* #if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_IsExistSameOrHigherLevelMp
 *-------------------------------------------------------------------------
 * PURPOSE  : This funciton check higher level up direction mep exist or mip exit
 * INPUT    : md_p  - the md pointer
 *           ma_p  - the ma pinter
 *           direction - mep direction
 *           mep_id - the mep id user create now
 * OUTPUT   : None
 * RETURN   : TRUE  - exit the higher leve mp
 *            FALSE - not exit the higher leve mp
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_IsExistSameOrHigherLevelMp(
    CFM_OM_MD_T             *md_p,
    CFM_OM_MA_T             *ma_p,
    CFM_TYPE_MP_Direction_T direction,
    UI32_T                  mep_id)
{
    CFM_OM_MEP_T *tmp_mep_p=&mep_tmp3_g;
    UI32_T nxt_md_index=0, nxt_ma_index=0, nxt_mep_id=0;

    while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, tmp_mep_p))
    {
       nxt_md_index=tmp_mep_p->md_p->index;
       nxt_ma_index=tmp_mep_p->ma_p->index;
       nxt_mep_id = tmp_mep_p->identifier;

        if(tmp_mep_p->md_p == md_p
            &&tmp_mep_p->ma_p == ma_p
            &&tmp_mep_p->identifier == mep_id)
        {
            continue;
        }

        if( (tmp_mep_p->md_p->level >= md_p->level)
            && (ma_p->primary_vid== tmp_mep_p->primary_vid))
        {
            if(CFM_TYPE_MP_DIRECTION_UP_DOWN == direction)
            {
                return TRUE;
            }
            else if(direction==tmp_mep_p->direction)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_IsArrivedTargetMacEx
 *-------------------------------------------------------------------------
 * PURPOSE  : This function chekc the dst mac is the reach the target mac
 * INPUT    : checked_dst_lport - the check dst port
 *            checked_dst_mac   - the dst mac will be checked
 *            checked_direction - the checked direction
 * OUTPUT   : is_matched_cpu_p  - pointer to output matched status
 *                                TRUE if cpu mac matched
 * RETURN   : TRUE  - arrive the target mac
 *            FALSE - doesn't arrive the target mac
 * NOTE     : use the CPU mac to be the up direction mep mac
 *            use the port mac to be the down directoin mep mac
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_IsArrivedTargetMacEx(
    UI32_T                  checked_dst_lport,
    UI8_T                   checked_dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    CFM_TYPE_MP_Direction_T checked_direction,
    BOOL_T                  *is_matched_cpu_p)
{
    if((CFM_TYPE_MP_DIRECTION_DOWN == checked_direction)||(CFM_TYPE_MP_DIRECTION_UP_DOWN== checked_direction))
    {
        UI8_T port_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

        SWCTRL_GetPortMac(checked_dst_lport,port_mac);

        if(!(memcmp(port_mac, checked_dst_mac, SYS_ADPT_MAC_ADDR_LEN)) )
        {
            if (NULL != is_matched_cpu_p)
            {
                *is_matched_cpu_p = FALSE;
            }
            return TRUE;
        }
    }

    if((CFM_TYPE_MP_DIRECTION_UP == checked_direction)||(CFM_TYPE_MP_DIRECTION_UP_DOWN== checked_direction))
    {
        UI8_T cpu_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

        SWCTRL_GetCpuMac(cpu_mac);

        if(!(memcmp(checked_dst_mac, cpu_mac, SYS_ADPT_MAC_ADDR_LEN)) )
        {
            if (NULL != is_matched_cpu_p)
            {
                *is_matched_cpu_p = TRUE;
            }
            return TRUE;
        }
    }

    return FALSE;
}/* End of CFM_ENGINE_IsArrivedTargetMacEx */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_IsArrivedTargetMac
 *-------------------------------------------------------------------------
 * PURPOSE  : This function chekc the dst mac is the reach the target mac
 * INPUT    : checked_dst_lport - the check dst port
 *            checked_dst_mac   - the dst mac will be checked
 *            checked_direction - the checked direction
 * OUTPUT   : None
 * RETURN   : TRUE  - arrive the target mac
 *            FALSE - doesn't arrive the target mac
 * NOTE     : use the CPU mac to be the up direction mep mac
 *            use the port mac to be the down directoin mep mac
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_IsArrivedTargetMac(
    UI32_T                  checked_dst_lport,
    UI8_T                   checked_dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    CFM_TYPE_MP_Direction_T checked_direction)
{
    return CFM_ENGINE_IsArrivedTargetMacEx(
                checked_dst_lport, checked_dst_mac, checked_direction, NULL);
}/* End of CFM_ENGINE_IsArrivedTargetMac*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FindLtmEgressPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function find the egress port and relay action for the input mac and vid
 * INPUT    : md_p        - the md pointer
 *            ma_p        - the ma pointer
 *            egress_mac  - ther egress mac addres of the egress port
 *            header_p    - the ltm packet header pointer
 * OUTPUT   : egress_port - the funded egress port
 *            relay_by    - the relay by which way
 *
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_FindLtmEgressPort(
    CFM_OM_MD_T             *md_p,
    CFM_OM_MA_T             *ma_p,
    UI8_T                   egress_mac[SYS_ADPT_MAC_ADDR_LEN],
    CFM_ENGINE_PduHead_T    *header_p,
    UI32_T                  *egress_port,
    CFM_TYPE_RelayAction_T  *relay_by)
{
    AMTR_TYPE_AddrEntry_T addr_entry;
    UI32_T                ingress_port = header_p->lport;
    UI16_T                vid=header_p->tag_info&0x0fff;
    BOOL_T                useFDBonly=(0x80 == (header_p->flags&0x80))?TRUE:FALSE;

    /* find egress port by FDB
     */
    memcpy(addr_entry.mac, egress_mac, SYS_ADPT_MAC_ADDR_LEN);
    addr_entry.vid=vid;

    /* 20.42.1.2
     * NOTE 2¡XThis query cannot produce an Egress Port that is the same as the
     * Ingress Port, since the Ingress Port is not included in the set of
     * potential transmission ports.
     */
    if((TRUE == AMTR_MGR_GetExactAddrEntry(&addr_entry))
     &&(ingress_port!=addr_entry.ifindex)/*ingress port shall not the same as egress port*/)
    {
        *egress_port=addr_entry.ifindex;
        *relay_by=CFM_TYPE_RELAY_ACTION_FDB;
        return TRUE;
    }

    /* find each port's port mac
     */
    {
        UI32_T lport=0;
        UI8_T port_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

        while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
        {
            if (ingress_port == lport)
            {
                /* ingress port shall not the same as egress port
                 */
                continue;
            }

            SWCTRL_GetPortMac(lport, port_mac);

            if(!memcmp(port_mac,egress_mac, SYS_ADPT_MAC_ADDR_LEN))
            {
                *egress_port=lport;
                *relay_by=CFM_TYPE_RELAY_ACTION_FDB;
                return TRUE;
            }
        }
    }

    /* find the egress by FDB without care the vid, this after port mac
     */
    addr_entry.vid=0;
    while(TRUE == AMTR_MGR_GetNextMVAddrEntry(&addr_entry,AMTR_MGR_GET_ALL_ADDRESS))
    {
        if(addr_entry.vid==vid && addr_entry.ifindex==ingress_port)
        {
          continue;
        }
        if (0 == memcmp(addr_entry.mac, egress_mac, SYS_ADPT_MAC_ADDR_LEN))
        {
            *egress_port=addr_entry.ifindex;
            *relay_by=CFM_TYPE_RELAY_ACTION_FDB;
            return TRUE;
        }
    }

    if(TRUE == useFDBonly)
    {
        return FALSE;
    }

    /* Find Egress port by Remote database
     */
    {
        CFM_OM_REMOTE_MEP_T remote_mep;
        UI32_T nxt_md_index=0, nxt_ma_index=0, nxt_remote_mep_id=0;

        while(TRUE == CFM_OM_GetNextRemoteMep(nxt_md_index, nxt_ma_index, nxt_remote_mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
        {
            if(!memcmp(remote_mep.mac_addr_a, egress_mac, SYS_ADPT_MAC_ADDR_LEN)
             && (ingress_port!=remote_mep.rcvd_lport))
            {
                if(0==remote_mep.rcvd_lport)
                {
                    return FALSE;
                }

                *egress_port = remote_mep.rcvd_lport;
                *relay_by=CFM_TYPE_RELAY_ACTION_MPDB;

                return TRUE;
            }

            nxt_md_index = remote_mep.md_index;
            nxt_ma_index = remote_mep.ma_index;
            nxt_remote_mep_id= remote_mep.identifier;
        }
    }

    return FALSE;
}/*End of CFM_ENGINE_FindLtmEgressPort*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_VerifyAndDecomposeLTM
 *-------------------------------------------------------------------------
 * PURPOSE  : This function verify and decompose the received LTM packet
 * INPUT    : *pdu_p    - the pdu content pointer
 *            *header_p - the common head pointer
 *            *ltm_p    - the ltm content pointer point to the pdu
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_VerifyAndDecomposeLTM(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_LtmPdu_T     *ltm_p)
{
    {/*first tlv offset is 17*/
        if(header_p->first_tlv_offset!=17)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "first tlv offset is wrong ");
            return FALSE;
        }
    }
    /*common header has no problem assign th header.*/
    ltm_p->header_p=header_p;

    /*transaction id*/
    ltm_p->trans_id_p=(pdu_p+4);

    /*ltm ttl*/
    ltm_p->ttl_p=(pdu_p+8);

    /*origianl mac*/
    ltm_p->original_mac_p=(pdu_p+9);
    /*check the first octect of mac to make sure the group address*/
    if(*ltm_p->original_mac_p == 0x01)
    {
        /*mac first octect is the group address*/
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "mac first octect is the group address");
        return FALSE;
    }

    /*target mac*/
    ltm_p->target_mac_p=(pdu_p+15);
    if(*ltm_p->target_mac_p == 0x01)
    {
        /*mac first octect is the group address*/
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "mac first octect is the group address");
        return FALSE;
    }

    {/*option tlv*/
        UI32_T len_accum=21, pdu_len=header_p->pdu_length;
        UI16_T tlv_length=0;
        UI8_T *tlv=NULL, tlv_type=0;
        BOOL_T has_egress_id_tlv=FALSE;

        tlv=(pdu_p+21);
        while(TRUE)
        {
            tlv_type=*tlv;
            memcpy(&tlv_length, &tlv[1], sizeof(UI16_T));
            tlv_length =L_STDLIB_Ntoh16(tlv_length);

            /*check the tlv type*/
            switch(tlv_type)
            {
            case CFM_TYPE_TLV_SENDER_ID:
                if(FALSE == CFM_ENGINE_DecomposeAndVerifySenderTlv(
                                header_p, &ltm_p->sender_tlv, tlv))
                {
                    return FALSE;
                }
                break;
            case CFM_TYPE_TLV_LTM_EGRESS_ID:
                ltm_p->egress_type_p=tlv;
                ltm_p->egress_length_p=tlv+1;
                ltm_p->egress_id_p=tlv+3;
                has_egress_id_tlv=TRUE;
                break;
            case CFM_TYPE_TLV_ORGANIZATION:
                if(FALSE == CFM_ENGINE_DecomposeAndVerifyOrganizationTlv(&ltm_p->org_tlv, tlv))
                {
                    return FALSE;
                }
                break;
            case CFM_TYPE_TLV_END:
                /* check have the egress id tlv
                 */
                if(FALSE == has_egress_id_tlv)
                {
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "has no egress tlv but reach end tlv now");

                    /* 20.42.4 enqueLTR()
                     * k) Copies the LTM Egress Identifier TLV (21.8.8) value from the LTM to
                     * the Last Egress Identifier field (21.9.7.1) of the LTR Egress Identifier TLV,
                     * or places 0 in that field, if there is no LTM Egress Identifier TLV in the received LTM
                     */
                    #if 0
                    return FALSE;
                    #endif
                }
                return TRUE;
            default:
                /*LTM need forward the unknown TLV, so record the unknown tlv*/
                if(ltm_p->unknown_tlv_length+tlv_length+3 <= CFM_TYPE_MAX_UNKNOWN_TLV_SIZE)
                {
                    memcpy(&ltm_p->unknown_tlv_a[ltm_p->unknown_tlv_length],tlv,tlv_length+3);
                    ltm_p->unknown_tlv_length+=tlv_length+3;
                }
                break;
            }

            /*accum tlv length over pdu length*/
            len_accum+=(tlv_length+3);
            if (len_accum>pdu_len)
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "tlv accume length is over packet len");
                return FALSE;
            }

            tlv=(tlv+tlv_length+3);
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "verify fail");
    return FALSE;
}/*End of CFM_ENGINE_VerifyAndDecomposeLTM*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ForwardLTM
 *-------------------------------------------------------------------------
 * PURPOSE  : This function forward the LTM
 * INPUT    : *ltm_p       - the ltm content pointer point to the recevie pdu
 *            through_port - through which port forward the LTM
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ForwardLTM(
    CFM_ENGINE_LtmPdu_T *ltm_p,
    UI32_T              through_port)
{
    UI32_T  vlan_ifindex;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN]={0}, src_mac[SYS_ADPT_MAC_ADDR_LEN]={0};
    UI8_T   *pdu_p=NULL, *tmp_pdu_p=NULL;
    UI16_T  pdu_len=0;

    /* some platform may learn the MAC/VLAN even the port is not the member of
     * VLAN, add check to stop forwarding the LTM to non-member VLAN port
     */
    VLAN_OM_ConvertToIfindex((ltm_p->header_p->tag_info &0x0fff), &vlan_ifindex);
    if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, through_port))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "No forward(%ld), not in vlan %d",
            (long)through_port, ltm_p->header_p->tag_info &0x0fff);
        return FALSE;
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "Packet forward, lport %ld,", (long)through_port);

    if((0 == *(UI8_T *)ltm_p->ttl_p) ||(1 == *(UI8_T *)ltm_p->ttl_p) )
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "ttl is %d =1 or =0 stop to forward LTM to lport %ld,", *(UI8_T *)ltm_p->ttl_p, (long)through_port);
        return FALSE;
    }

    /*constrcut the LTM pdu and send pdu
     */
    if(NULL == (pdu_p=(UI8_T *)L_MM_Malloc(CFM_TYPE_MAX_FRAME_SIZE, L_MM_USER_ID2(SYS_MODULE_CFM, CFM_TYPE_XMITLTM))))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "can't allocate memory");
        return FALSE;
    }

    memset(pdu_p, 0, CFM_TYPE_MAX_FRAME_SIZE);
    tmp_pdu_p = pdu_p;

    /*copy the common header, transaciton id, ttl, original mac, target mac, 21 bytes*/
    /* ltm_p may be used by other API, should not be modified directly.
     *
     *  *(UI8_T *)ltm_p->ttl_p=*(UI8_T *)ltm_p->ttl_p-1;
     */
    memcpy(tmp_pdu_p, ltm_p->trans_id_p-4, 21);

    /* 20.42.3 e) Places in the forwarded LTM TTL field the value
     *            from the input LTM TTL field decremented by 1
     */
    *(tmp_pdu_p+8) -= 1;

    tmp_pdu_p+=21;
    pdu_len+=21;

    /*change egress id*/
    {
        UI16_T tmp16=0;

        *tmp_pdu_p=(UI8_T)CFM_TYPE_TLV_LTM_EGRESS_ID;
        tmp16=L_STDLIB_Hton16(8);
        memcpy(tmp_pdu_p+1, &tmp16, sizeof(UI16_T));

        tmp16=L_STDLIB_Hton16(through_port);
        memcpy(tmp_pdu_p+3, &tmp16, sizeof(UI16_T));

        SWCTRL_GetPortMac(through_port,tmp_pdu_p+5);

        tmp_pdu_p+=11;
        pdu_len+=11;
    }

    /*replace the sender tlv, when received ltm has sender and we replace, if the received ltm don't have sender tlv we won't add*/
    if(NULL!=ltm_p->sender_tlv.type_p)
    {
        UI16_T tlv_len=0;

        CFM_ENGINE_ConstructSenderTLV(tmp_pdu_p, &tlv_len, through_port);
        tmp_pdu_p+=tlv_len;
        pdu_len+=tlv_len;
    }

    /*copy organization*/
    if(NULL!=ltm_p->org_tlv.type_p)
    {
        UI16_T organization_length=0;

        memcpy(&organization_length, ltm_p->org_tlv.length_p, sizeof(UI16_T));
        organization_length=L_STDLIB_Ntoh16(organization_length);
        organization_length+=3;
        memcpy(tmp_pdu_p, ltm_p->org_tlv.type_p, organization_length);
        tmp_pdu_p+=organization_length;
        pdu_len+=organization_length;
    }

    if(0!= ltm_p->unknown_tlv_length)
    {
        memcpy(tmp_pdu_p, ltm_p->unknown_tlv_a, ltm_p->unknown_tlv_length);
        tmp_pdu_p+=ltm_p->unknown_tlv_length;
        pdu_len+=ltm_p->unknown_tlv_length;
    }

    /*construct end tlv*/
    {
        UI16_T tlv_len=0;

        CFM_ENGINE_ConstructEndTLV(tmp_pdu_p, &tlv_len);
        tmp_pdu_p+=tlv_len;
        pdu_len+=tlv_len;
    }

    /*assembler the ltm destination group address*/
    CFM_ENGINE_ASSEMBLE_DEST_MAC(dst_mac, ltm_p->header_p->md_level+CFM_TYPE_LTM_MD_LEVEL_0);
    SWCTRL_GetPortMac(through_port, src_mac);

    /*transmit pdu*/
    CFM_ENGINE_XmitPDU(
        through_port,               (ltm_p->header_p->tag_info &0x0fff),
        ltm_p->header_p->md_level,  (ltm_p->header_p->tag_info &0xe000)>>13,
        TRUE, dst_mac, src_mac, CFM_TYPE_MP_DIRECTION_DOWN, pdu_p, pdu_len);

    /*because xmit pdu will copy the content, so here free the allocated memory*/
    L_MM_Free(pdu_p);
    return TRUE;
}/*End of CFM_ENGINE_ForwardLTM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MepProcessLTR
 *-------------------------------------------------------------------------
 * PURPOSE  : this function process the received LTR
 * INPUT    : *pdu_p    - the received pdu packet
 *            *header_p - the common header of the received LTR
 *            *mep_p    - the mep pointer which receive the LTR
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MepProcessLTR(
    CFM_ENGINE_LtrPdu_T *ltr_pdu_p,
    CFM_OM_MEP_T        *mep_p)
{
    CFM_TYPE_LinktraceStatus_T  status=CFM_TYPE_LINKTRACE_STATUS_ENABLE;
    CFM_OM_LTR_T                *ltr_om_p=&ltr_tmp_g;
    UI32_T                      trans_id=0;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "mep id %ld recevied LTR", (long)mep_p->identifier);

    /*check if the mep's mac is the pdu's dst mac*/
    if(memcmp(ltr_pdu_p->header_p->dst_mac_a, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "mep's mac is not dst mac, mep mac=%02x:%02x:%02x:%02x:%02x:%02x",
                              mep_p->mac_addr_a[0], mep_p->mac_addr_a[1], mep_p->mac_addr_a[2], mep_p->mac_addr_a[3], mep_p->mac_addr_a[4], mep_p->mac_addr_a[5]);
        return FALSE;
    }

    /*check link trace cache status, if link trace cache no eanble then needn't process*/
    if( (FALSE == CFM_OM_GetLinkTraceCacheStatus(&status)) ||(CFM_TYPE_LINKTRACE_STATUS_DISABLE == status))
    {
        return TRUE;
    }

    /*check the received ltr is the righ  transaction id which mep just send ltm*/
    memcpy(&trans_id, ltr_pdu_p->trans_id_p, sizeof(UI32_T));
    trans_id=L_STDLIB_Ntoh32(trans_id);

    if(trans_id!=mep_p->transmit_ltm_seq_number)
    {
        mep_p->unexp_ltr_in ++;
        CFM_OM_StoreMep(mep_p);

        /* to avoid the display problem when LTR is received for previous test
         */
        return FALSE;
    }

    memset(ltr_om_p, 0 ,sizeof(CFM_OM_LTR_T));

    ltr_om_p->md_index=mep_p->md_index;
    ltr_om_p->ma_index=mep_p->ma_index;
    ltr_om_p->rcvd_mep_id=mep_p->identifier;
    memcpy(&ltr_om_p->seq_number, ltr_pdu_p->trans_id_p, sizeof(UI32_T));
    ltr_om_p->seq_number=L_STDLIB_Ntoh32(ltr_om_p->seq_number);

    /* from D8/Y.1731, LTR's ttl is already minus 1 from the LTM's
     */
    ltr_om_p->receive_order=mep_p->transmit_ltm_ttl - (*(UI8_T*)(ltr_pdu_p->reply_ttl_p));
    ltr_om_p->reply_ttl=*(UI8_T*)(ltr_pdu_p->reply_ttl_p);

    ltr_om_p->forwarded=(ltr_pdu_p->header_p->flags&0x40?TRUE:FALSE);

    ltr_om_p->terminal_mep=(ltr_pdu_p->header_p->flags&0x20?TRUE:FALSE);

    ltr_om_p->relay_action=*(UI8_T *)ltr_pdu_p->relay_action_p;

    /*ltr egress identifier tlv*/
    if(NULL!=ltr_pdu_p->egress_type_p)
    {
        memcpy(ltr_om_p->last_egress_identifier, ltr_pdu_p->last_eegress_id_p,8);
        memcpy(ltr_om_p->next_egress_identifier, ltr_pdu_p->next_eegress_id_p,8);
    }

    /*record sender tlv
     */
    if(NULL!=ltr_pdu_p->sender_tlv.chassis_id_sub_type_p)
    {
        ltr_om_p->chassis_id_subtype=*(UI8_T *)ltr_pdu_p->sender_tlv.chassis_id_sub_type_p;
        ltr_om_p->chassis_id_length=*(UI8_T *)ltr_pdu_p->sender_tlv.chassis_id_length_p;
        if (ltr_om_p->chassis_id_length > CFM_TYPE_MAX_CHASSIS_ID_LENGTH)
        {
            ltr_om_p->chassis_id_length = CFM_TYPE_MAX_CHASSIS_ID_LENGTH;
        }
        memcpy(ltr_om_p->chassis_id, ltr_pdu_p->sender_tlv.chassis_id_p, ltr_om_p->chassis_id_length);

        /*
         *sender mgmt donmain len, mgmt domain, mgmt addr len, mgmt addr
         */
        if(*(ltr_pdu_p->sender_tlv.mgmt_addr_domain_len_p)!=0)
        {
            ltr_om_p->mgmt_addr_domain_len=*((UI8_T *)ltr_pdu_p->sender_tlv.mgmt_addr_domain_len_p);
            if (ltr_om_p->mgmt_addr_domain_len > CFM_TYPE_MAX_MAN_DOMAIN_LENGTH)
            {
                ltr_om_p->mgmt_addr_domain_len = CFM_TYPE_MAX_MAN_DOMAIN_LENGTH;
            }
            memcpy(ltr_om_p->mgmt_addr_domain, ltr_pdu_p->sender_tlv.mgmt_addr_domain_p,
                   ltr_om_p->mgmt_addr_domain_len);

            if(*(ltr_pdu_p->sender_tlv.mgmt_addr_len_p) !=0)
            {
                ltr_om_p->mgmt_addr_len=*((UI8_T *)ltr_pdu_p->sender_tlv.mgmt_addr_len_p);
                if (ltr_om_p->mgmt_addr_len > CFM_TYPE_MAX_MAN_ADDRESS_LENGTH)
                {
                    ltr_om_p->mgmt_addr_len = CFM_TYPE_MAX_MAN_ADDRESS_LENGTH;
                }
                memcpy(ltr_om_p->mgmt_addr, ltr_pdu_p->sender_tlv.mgmt_addr_p,
                       (ltr_om_p->mgmt_addr_len>CFM_TYPE_MAX_MAN_ADDRESS_LENGTH?CFM_TYPE_MAX_MAN_ADDRESS_LENGTH:ltr_om_p->mgmt_addr_len));
            }
        }
    }

    /*record ingress tlv
     */
    if(NULL!=ltr_pdu_p->reply_ingress_action_p)
    {
        ltr_om_p->ingress_action=*(UI8_T *)ltr_pdu_p->reply_ingress_action_p;
        memcpy(ltr_om_p->ingress_mac, ltr_pdu_p->reply_ingress_mac_p, SYS_ADPT_MAC_ADDR_LEN);
        ltr_om_p->ingress_port_id_subtype=*(UI8_T *)ltr_pdu_p->reply_ingress_port_id_subtype;
        ltr_om_p->ingress_port_id_lenth=*(UI8_T *)ltr_pdu_p->reply_ingress_port_id_length_p;
        memcpy(ltr_om_p->ingress_port_Id, ltr_pdu_p->reply_ingress_port_id,(ltr_om_p->ingress_port_id_lenth>CFM_TYPE_MAX_PORT_ID_LENGTH?CFM_TYPE_MAX_PORT_ID_LENGTH:ltr_om_p->ingress_port_id_lenth));
    }

    /*record egress tlv
     */
    if(NULL!=ltr_pdu_p->reply_egress_action_p)
    {
        ltr_om_p->egress_action=*(UI8_T *)ltr_pdu_p->reply_egress_action_p;
        memcpy(ltr_om_p->egress_mac, ltr_pdu_p->reply_egress_mac_p, SYS_ADPT_MAC_ADDR_LEN);
        ltr_om_p->egress_port_id_subtype=*(UI8_T *)ltr_pdu_p->reply_egress_port_id_subtype;
        ltr_om_p->egress_port_id_lenth=*(UI8_T *)ltr_pdu_p->reply_egress_port_id_length_p;
        memcpy(ltr_om_p->egress_port_id, ltr_pdu_p->reply_egress_port_id,
               (ltr_om_p->egress_port_id_lenth>CFM_TYPE_MAX_PORT_ID_LENGTH?CFM_TYPE_MAX_PORT_ID_LENGTH:ltr_om_p->egress_port_id_lenth));
    }

    /*record organization tlv
     */
    if(NULL!=ltr_pdu_p->org_tlv.type_p)
    {
        UI16_T record_len;

        memcpy(&record_len, ltr_pdu_p->org_tlv.length_p, sizeof(record_len));

        record_len = L_STDLIB_Ntoh16(record_len);

        /* truncate the org_specific_tlv if it's too large
         */
        if (record_len > CFM_TYPE_MAX_ORGANIZATION_TLV_LENGTH)
        {
            record_len = CFM_TYPE_MAX_ORGANIZATION_TLV_LENGTH;
        }

        ltr_om_p->org_tlv_length=record_len;
        memcpy(ltr_om_p->org_specific_tlv_a, ltr_pdu_p->org_tlv.OUI_p, record_len);
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT,
        "LTR content md index=%ld, ma index=%ld, receved mep id=%ld, sequ_num=%ld, received order=%ld",
         (long)ltr_om_p->md_index,    (long)ltr_om_p->ma_index,     (long)ltr_om_p->rcvd_mep_id,
         (long)ltr_om_p->seq_number,  (long)ltr_om_p->receive_order);

    if (FALSE == CFM_ENGINE_LocalAddLtrToOM(ltr_om_p))
    {
        return FALSE;
    }

    return TRUE;
}/*End of CFM_ENGINE_MepProcessLTR*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_VerifyAndDecomposeLTR
 *-------------------------------------------------------------------------
 * PURPOSE  :This function verify the and decompose the received LTR
 * INPUT    : *pdu_p    - the pdu content pointer
 *            *header_p - the common head pointer
              *ltr_p    - the ltr content pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_VerifyAndDecomposeLTR(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_LtrPdu_T     *ltr_p)
{
    {/*first tlv offset is 17*/
        if(header_p->first_tlv_offset!=6)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "first tlv offset is wrongl");
            return FALSE;
        }
    }
    /*common header has no problem assign th header.*/
    ltr_p->header_p=header_p;

    /*transaction id*/
    ltr_p->trans_id_p=(pdu_p+4);

    /*reply ttl*/
    ltr_p->reply_ttl_p=(pdu_p+8);

    /*relay action*/
    ltr_p->relay_action_p=(pdu_p+9);
    /*check the relay action value*/
    if((*ltr_p->relay_action_p >CFM_TYPE_RELAY_ACTION_MPDB)||(*ltr_p->relay_action_p==0) )
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "relay action value over range fail");
        return FALSE;
    }

    {/*option tlv*/
        UI8_T *tlv=NULL, tlv_type=0;
        UI16_T tlv_length=0;
        UI32_T len_accum=10, pdu_len=header_p->pdu_length;
        BOOL_T has_egress_id_tlv=FALSE, has_reply_tlv=FALSE;

        tlv=(pdu_p+10);
        while(TRUE)
        {
            tlv_type=*tlv;
            memcpy(&tlv_length, &tlv[1], sizeof(UI16_T));
            tlv_length=L_STDLIB_Ntoh16(tlv_length);

            /*check the tlv type*/
            switch(tlv_type)
            {
                case CFM_TYPE_TLV_SENDER_ID:
                    if(FALSE == CFM_ENGINE_DecomposeAndVerifySenderTlv(
                                    header_p, &ltr_p->sender_tlv, tlv))
                    {
                        return FALSE;
                    }
                    break;
                case CFM_TYPE_TLV_LTR_EGRESS_ID:
                    ltr_p->egress_type_p=tlv;
                    ltr_p->egress_length_p=tlv+1;
                    ltr_p->last_eegress_id_p=tlv+3;
                    ltr_p->next_eegress_id_p=tlv+11;
                    has_egress_id_tlv=TRUE;
                    break;
                case CFM_TYPE_TLV_REPLY_INGRESS:
                {
                    UI16_T reply_ingress_length=0;

                    ltr_p->reply_ingress_type_p=tlv;
                    ltr_p->reply_ingress_length_p=tlv+1;
                    ltr_p->reply_ingress_action_p=tlv+3;
                    ltr_p->reply_ingress_mac_p=tlv+4;
                    ltr_p->reply_ingress_port_id_length_p=tlv+10;
                    ltr_p->reply_ingress_port_id_subtype=tlv+11;
                    ltr_p->reply_ingress_port_id=tlv+12;
                    has_reply_tlv=TRUE;

                    /*check the relay action value*/
                    if((*ltr_p->reply_ingress_action_p >(UI8_T)CFM_TYPE_INGRESS_ACTION_VID)||(*ltr_p->reply_ingress_action_p==0) )
                    {
                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "reply ingress action vaule over range fail");
                        return FALSE;
                    }

                    /*check the mac address should not be the group address*/
                    if(*ltr_p->reply_ingress_mac_p == 0x01)
                    {
                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "reply ingress mac is group addr");
                        return FALSE;
                    }

                    /*check the legnth field*/
                    memcpy(&reply_ingress_length, ltr_p->reply_ingress_length_p, sizeof(UI16_T));
                    reply_ingress_length=L_STDLIB_Ntoh16(reply_ingress_length);
                    if(( reply_ingress_length!=7)&& (reply_ingress_length<(9+ *ltr_p->reply_ingress_port_id_length_p) ))
                    {
                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "reply ingress legnth value wrong");
                        return FALSE;
                    }
                    break;
                }
                case CFM_TYPE_TLV_REPLY_EGRESS:
                {
                    UI16_T relay_egress_length=0;

                    ltr_p->reply_egress_type=tlv;
                    ltr_p->reply_egress_length_p=tlv+1;
                    ltr_p->reply_egress_action_p=tlv+3;
                    ltr_p->reply_egress_mac_p=tlv+4;
                    ltr_p->reply_egress_port_id_length_p=tlv+10;
                    ltr_p->reply_egress_port_id_subtype=tlv+11;
                    ltr_p->reply_egress_port_id=tlv+12;
                    has_reply_tlv=TRUE;

                    /*check the relay action value*/
                    if( (*ltr_p->reply_egress_action_p >(UI8_T)CFM_TYPE_EGRESS_ACTION_VID)||(*ltr_p->reply_egress_action_p==0) )
                    {
                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "reply egress action value over range");
                        return FALSE;
                    }

                    /*check the mac address should not be the group address, check first octect*/
                    if(*ltr_p->reply_egress_mac_p == 0x01)
                    {
                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "reply egress mac is group addrl");
                        return FALSE;
                    }

                    /*check the legnth field*/
                    memcpy(&relay_egress_length, ltr_p->reply_egress_length_p, sizeof(UI16_T));
                    relay_egress_length=L_STDLIB_Ntoh16(relay_egress_length);
                    if(( relay_egress_length!=7)&& (relay_egress_length<(9+*ltr_p->reply_egress_port_id_length_p) ))
                    {
                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "relay egress legnth value is wrong");
                        return FALSE;
                    }
                    break;
                }
                case CFM_TYPE_TLV_ORGANIZATION:
                    if(FALSE == CFM_ENGINE_DecomposeAndVerifyOrganizationTlv(&ltr_p->org_tlv, tlv))
                    {
                        return FALSE;
                    }
                    break;
                case CFM_TYPE_TLV_END:
                    return TRUE;
                default:
                    break;
            }

            /*accum tlv length over pdu length*/
            len_accum+=(tlv_length+3);
            if (len_accum>pdu_len)
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "tlv accume length is over packet length");
                return FALSE;
            }

            tlv=(tlv+tlv_length+3);
         }

        /*check have the egress id tlv*/
        if(FALSE == has_egress_id_tlv)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "has no egress id tlv");
            return FALSE;
        }

        /*check have the reply tlv*/
        if(FALSE == has_reply_tlv)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "has no reply tlvl");
            return FALSE;
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "verify fail");
    return FALSE;
}/*end of CFM_ENGINE_VerifyAndDecomposeLTR*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearPendingLTRs_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the pending LTR in queue
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearPendingLTRs_CallBack(
    void    *timer_para_p)
{
    CFM_OM_LTR_QueueElement_T *queue_element_p=NULL;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "");

    while(TRUE==CFM_OM_GetRemoveFirstElementFromLTrQueue(&queue_element_p))
    {
        if(NULL!=queue_element_p)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "through port =%ld, md level=%d, vid=%d",
                (long)queue_element_p->lport, queue_element_p->level, queue_element_p->vid);

            CFM_ENGINE_XmitPDU(
                        queue_element_p->lport,
                        queue_element_p->vid,
                        queue_element_p->level,
                        queue_element_p->priority,
                        TRUE,
                        queue_element_p->dst_mac_a,
                        queue_element_p->src_mac_a,
                        CFM_TYPE_MP_DIRECTION_DOWN,
                        queue_element_p->pdu_p,
                        queue_element_p->pduLen);

            if(NULL!= queue_element_p->pdu_p)
                L_MM_Free(queue_element_p->pdu_p);

            L_MM_Free(queue_element_p);
        }
        queue_element_p=NULL;
    }

    return TRUE;
}/*End of CFM_ENGINE_ClearPendingLTRs_CallBack*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_EnqueueLTR
 *-------------------------------------------------------------------------
 * PURPOSE  : This function enqueue the LTR in to queue
 * INPUT    : ltm_p        - pointer to the ltm pdu
 *            ingress_port - the LTM ingress port
 *            egress port  - the LTM egress port
 *            forwarded    - the LTM is forwarded or not
 *            relay_action - the LTM realy action
 *            md_p         - the md pointer
 *            ma_p         - the ma pointer
 *            is_mep       - TRUE to set TerminalMEP flag
 *            need_reply_ingress_tlv - TRUE to construct reply ingress tlv
 *            need_reply_egress_tlv  - TRUE to construct reply egress tlv
 *            src_mac_p    - pointer to LTR src mac
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : refer to 20.42.4 o) for Reply Ingress TLV
 *                             p) for Reply Egress TLV,
 *            and it seems no reply egress tlv needed for HIT
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_EnqueueLTR(
    CFM_ENGINE_LtmPdu_T     *ltm_p,
    UI32_T                  ingress_port,
    UI32_T                  egress_port,
    BOOL_T                  forwarded,
    CFM_TYPE_RelayAction_T  relay_action,
    CFM_OM_MD_T             *md_p,
    CFM_OM_MA_T             *ma_p,
    BOOL_T                  is_mep,
    BOOL_T                  need_reply_ingress_tlv,
    BOOL_T                  need_reply_egress_tlv,
    UI8_T                   *src_mac_p)
{
    CFM_OM_LTR_QueueElement_T   *queue_element_p;
    CFM_OM_MEP_T                *egress_mep_p=&mep_tmp2_g,
                                *ingress_mep_p=&mep_tmp3_g;
    CFM_OM_MIP_T                egress_mip;
    UI16_T                      pdu_len=0, egress_vid=0;
    UI8_T                       *pdu_p=NULL;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "");

    /*if ttl is 0, don't process*/
    if(*(UI8_T *)ltm_p->ttl_p == 0)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "don't send the LTR because of  ttl is 0");
        return;
    }

    if(NULL==(pdu_p=(UI8_T *)L_MM_Malloc(CFM_TYPE_MAX_FRAME_SIZE, L_MM_USER_ID2(SYS_MODULE_CFM, CFM_TYPE_XMITLTR))))
    {
        return;
    }

    /* ltm_p may be used by other API, should not be modified directly.
     *
     *   *(UI8_T *)ltm_p->ttl_p -= 1;
     */
    memset(pdu_p, 0, CFM_TYPE_MAX_FRAME_SIZE);

    /* 20.42.4 enqueLTR()
     * If the LTR includes a Reply Egress TLV,
     * uses the Primary VID of the MP on that Egress Port as the vlan_identifier of the LTR,
     * else it uses the Primary VID of the MP on the Ingress Port as the vlan_identifier of the LTR;
     */
    if(0 != egress_port)
    {
        if(TRUE==CFM_OM_GetMep(md_p->index, ma_p->index, 0, egress_port, CFM_OM_LPORT_MD_MA_KEY, egress_mep_p))
        {
            egress_vid = egress_mep_p->primary_vid;
        }
        else if(TRUE == CFM_OM_GetMip(md_p->index, ma_p->index, egress_port, CFM_OM_LPORT_MD_MA_KEY, &egress_mip))
        {
            egress_vid = egress_mip.ma_p->primary_vid;
        }
        else /* egress mep and mip donesn't exist, use the vid carried in packet */
        {
            egress_vid =ltm_p->header_p->tag_info&0x0fff;
        }
    }
    else /* if(0 == egress_port) */
    {
        if(TRUE==CFM_OM_GetMep( md_p->index, ma_p->index, 0, ingress_port, CFM_OM_LPORT_MD_MA_KEY, ingress_mep_p))
        {
            egress_vid=ingress_mep_p->primary_vid;
        }
        else /*if ingress MEP not exist, use the vid carried in packet*/
        {
            egress_vid=ltm_p->header_p->tag_info&0x0fff;
        }
    }

    /*contruct the LTR PDU*/
    CFM_ENGINE_ConstructLTRPDU(ltm_p,
                               pdu_p,
                               &pdu_len,
                               forwarded,
                               ltm_p->header_p->md_level,
                               ltm_p->header_p->tag_info&0x0fff,
                               egress_port,
                               ingress_port,
                               relay_action,
                               is_mep,
                               need_reply_ingress_tlv,
                               need_reply_egress_tlv);

    /*construct ltr queue element
     */
    if(NULL==(queue_element_p=(CFM_OM_LTR_QueueElement_T *)L_MM_Malloc(sizeof(CFM_OM_LTR_QueueElement_T), L_MM_USER_ID2(SYS_MODULE_CFM, CFM_TYPE_XMITLTR))))
    {
        L_MM_Free(pdu_p);
        return;
    }

    memset(queue_element_p, 0, sizeof(CFM_OM_LTR_QueueElement_T));

    queue_element_p->pdu_p=pdu_p;
    memcpy(queue_element_p->dst_mac_a, ltm_p->original_mac_p, SYS_ADPT_MAC_ADDR_LEN);

    if (NULL == src_mac_p)
        SWCTRL_GetPortMac(ingress_port, queue_element_p->src_mac_a);
    else
        memcpy(queue_element_p->src_mac_a, src_mac_p, SYS_ADPT_MAC_ADDR_LEN);

    queue_element_p->vid=egress_vid;
    queue_element_p->priority= (ltm_p->header_p->tag_info&0xe000)>>13;
    queue_element_p->next_element_p=NULL;
    queue_element_p->lport=ingress_port;
    queue_element_p->pduLen=pdu_len;
    queue_element_p->level=ltm_p->header_p->md_level;

    /*put into queue*/
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT,
            "enqueue LTM ingress port/LTR egress port =%ld, LTM ingress vid=%d, LTM egress port=%ld, LTR egress vid=%d",
            (long)queue_element_p->lport, ltm_p->header_p->tag_info&0x0fff,
            (long)egress_port,            queue_element_p->vid);
    CFM_OM_AddToLtrQueue(queue_element_p);
    return;
}/*End of CFM_ENGINE_EnqueueLTR*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetEgressActionForLTR
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the egress action for LTR
 * INPUT    : pri_vid      - the primary vid
 *            egress_lport - the egres port, that the ltm will be forwarded.
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_EgressAction_T
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static CFM_TYPE_EgressAction_T CFM_ENGINE_GetEgressActionForLTR(
    UI16_T  pri_vid,
    UI32_T  egress_lport)
{
    UI32_T                      vlan_ifindex;
    CFM_TYPE_EgressAction_T     ret = CFM_TYPE_EGRESS_ACTION_OK;

    /* according to 802.1ag-D8.pdf,
     *   21.9.9.1 Egress Action, Table 21-32
     */
    VLAN_OM_ConvertToIfindex(pri_vid, &vlan_ifindex);
    if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, egress_lport))
    {
        ret = CFM_TYPE_EGRESS_ACTION_VID;
    }
    else if(FALSE == SWCTRL_IsPortOperationUp(egress_lport))
    {
        /* xstp will say blocked when operation is down,
         * so should check operation before xstp.
         */
        ret = CFM_TYPE_EGRESS_ACTION_DOWN;
    }
    else if(FALSE == XSTP_OM_IsPortForwardingStateByVlan (pri_vid, egress_lport))
    {
        ret = CFM_TYPE_EGRESS_ACTION_BLOCKED;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructLTRPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will construct the LTR packet
 * INPUT    : ltm_p        - pointer to ltm pdu
 *            pdu_p        - pointer to ltr pdu
 *            pdu_len      - the pdu_len pointer of the packet content length
 *            forwarded    - LTM is forwarded or not
 *            level        - the md level
 *            primaryVid   - the primary vid
 *            egress_port  - the egres port, that the ltm will be forwarded.
 *            ingress_port - the ingress port that the ltm come in
 *            relay_action - relay action
 *            is_mep       - TRUE to set TerminalMEP flag
 *            need_reply_ingress_tlv - TRUE to construct reply ingress tlv
 *            need_reply_egress_tlv  - TRUE to construct reply egress tlv
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructLTRPDU(
    CFM_ENGINE_LtmPdu_T     *ltm_p,
    UI8_T                   *pdu_p,
    UI16_T                  *pdu_len_p,
    BOOL_T                  forwarded,
    CFM_TYPE_MdLevel_T      level,
    UI16_T                  primaryVid,
    UI32_T                  egress_port,
    UI32_T                  ingress_port,
    CFM_TYPE_RelayAction_T  relay_action,
    BOOL_T                  is_mep,
    BOOL_T                  need_reply_ingress_tlv,
    BOOL_T                  need_reply_egress_tlv)
{
    {/*construct common header*/
        UI8_T flag=ltm_p->header_p->flags &0x9f; /*20.42.4:i*/

        /*assign relay action in flag*/
        if(TRUE == forwarded)
        {
            flag|=0x40;
        }

        if (TRUE == is_mep)
        {
            flag|=0x20;
        }

        CFM_ENGINE_ConstructCommonHeader(pdu_p, pdu_len_p, level,flag, CFM_TYPE_OPCODE_LTR);

        /*move to next octect*/
        pdu_p+=*pdu_len_p;
    }

    {/*copy transaction id*/
        memcpy(pdu_p, ltm_p->trans_id_p,4);
        pdu_p+=4;
        *pdu_len_p+=4;
    }
    {/*copy reply ttl*/
        *pdu_p=*ltm_p->ttl_p -1 ;  /* 20.42.4 m) */
        pdu_p+=1;
        *pdu_len_p+=1;
    }
    {/*assign relay action*/
        *pdu_p=relay_action;
        pdu_p+=1;
        *pdu_len_p+=1;
    }
    /*egress identifier*/

    /* 20.42.4 enqueLTR()
     * k)Copies the LTM Egress Identifier TLV (21.8.8) value from the LTM
     * to the Last Egress Identifier field (21.9.7.1) of the LTR Egress Identifier TLV,
     * or places 0 in that field, if there is no LTM Egress Identifier TLV in the received LTM
     */
    {/*copy egress id tlv*/
        UI16_T egress_id=L_STDLIB_Hton16(16);

        *pdu_p=(UI8_T) CFM_TYPE_TLV_LTR_EGRESS_ID;
        memcpy(pdu_p+1, &egress_id, sizeof(UI16_T));

        if(NULL != ltm_p->egress_id_p)
        {
            /*last egress identifier field*/
            memcpy(pdu_p+3, ltm_p->egress_id_p,8);
        }

        /*next egress identifier field*/
        if(0!=egress_port)
        {
            UI16_T port_num=(UI16_T) egress_port;

            SWCTRL_GetPortMac(egress_port,pdu_p+13); /*the two higher order octect was skiped*/

            port_num=L_STDLIB_Hton16(port_num);
            memcpy(pdu_p+11, &port_num,2);
        }

        pdu_p+=19;
        *pdu_len_p+=19;
    }

    /* reply ingress tlv
     * If the LTM was received by a Down MEP or a Down MHF,
     * the LTR includes a Reply Ingress TLV (21.9.8) describing that MP.
     */
    if(TRUE == need_reply_ingress_tlv)
    {
        UI32_T                      vlan_ifindex=0;
        VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
        UI16_T                      tlv_len=0;
        BOOL_T                      ExistHigherLevelMp=FALSE;
        CFM_OM_MEP_T                mep;

        ExistHigherLevelMp=CFM_ENGINE_IsExistHigherLevelMp(
            ingress_port, level,primaryVid, CFM_TYPE_MP_DIRECTION_DOWN,
            FALSE, &mep, NULL);

        *pdu_p=(UI8_T)CFM_TYPE_TLV_REPLY_INGRESS;

        /*tlv lengh, if there is not port id the length is 7, if there is a port id, the length is (9+port id length)*/
        tlv_len=L_STDLIB_Hton16(9+SYS_ADPT_MAC_ADDR_LEN);
        memcpy(pdu_p+1, &tlv_len, sizeof(UI16_T));

        /*ingress action*/
        VLAN_OM_ConvertToIfindex(primaryVid, &vlan_ifindex);
        VLAN_MGR_GetPortEntry(ingress_port, &vlan_port_info);
        if( (FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, ingress_port))&&
         (VAL_dot1qPortIngressFiltering_true == vlan_port_info.port_item.dot1q_port_ingress_filtering) )
        {
            *(pdu_p+3)=(UI8_T)CFM_TYPE_INGRESS_ACTION_VID;
        }
        else if(FALSE == ltm_p->header_p->is_ing_lport_fwd)
        {
            *(pdu_p+3)=CFM_TYPE_INGRESS_ACTION_BLOCKED;
        }
        else if(FALSE == SWCTRL_IsPortOperationUp(ingress_port)||TRUE==ExistHigherLevelMp)
        {
            *(pdu_p+3)=CFM_TYPE_INGRESS_ACTION_DOWN;
        }
        else
        {
            *(pdu_p+3)=CFM_TYPE_INGRESS_ACTION_OK;
        }

        /*ingress mac*/
        SWCTRL_GetPortMac(ingress_port,pdu_p+4);
        /*ingress port id length, mac length*/
        *(pdu_p+10)=SYS_ADPT_MAC_ADDR_LEN;
        /*ingress port id subtype*/
        *(pdu_p+11)=(UI8_T)CFM_TYPE_PORT_ID_SUBTYPE_MAC_ADDR;
        /*ingress port id*/
        SWCTRL_GetPortMac(ingress_port,(pdu_p+12));

        /*skip this tlv = port id length + manatary length*/
        pdu_p+=(12+SYS_ADPT_MAC_ADDR_LEN);
        *pdu_len_p+=(12+SYS_ADPT_MAC_ADDR_LEN);

    }

    /* reply egress tlv
     * If not received by a Down MEP, and if the Egress Port has an Up MEP
     * or Up MHF configured for the LTM¡¦s MA, the LTR includes a Reply Egress TLV (21.9.9).
     */
    if(TRUE == need_reply_egress_tlv)
    {
        if(egress_port!=0)
        {
            UI16_T tlv_len=0;

            *pdu_p=(UI8_T)CFM_TYPE_TLV_REPLY_EGRESS;

            /*tlv lengh, if there is not port id the length is 7, if there is a port id, the length is (9+port id length)*/
            tlv_len=L_STDLIB_Hton16(9+SYS_ADPT_MAC_ADDR_LEN);
            memcpy(pdu_p+1, &tlv_len, sizeof(UI16_T));

            /*egress action*/
            *(pdu_p+3) = CFM_ENGINE_GetEgressActionForLTR(primaryVid, egress_port);

            /*egress mac*/
            SWCTRL_GetPortMac(egress_port,pdu_p+4);

            /*egress port id length, mac length*/
            *(pdu_p+10)=SYS_ADPT_MAC_ADDR_LEN;
            /*egress port id subtype*/
            *(pdu_p+11)=(UI8_T)CFM_TYPE_PORT_ID_SUBTYPE_MAC_ADDR;
            /*egress port id*/
            SWCTRL_GetPortMac(egress_port,(pdu_p+12));

            /*skip this tlv = port id length + manatary length*/
            pdu_p+=(12+SYS_ADPT_MAC_ADDR_LEN);
            *pdu_len_p+=(12+SYS_ADPT_MAC_ADDR_LEN);
        }
    }
    {
        UI16_T tlv_len=0;

        /*construct sender tlv*/
        CFM_ENGINE_ConstructSenderTLV(pdu_p, &tlv_len,ingress_port);
        pdu_p+=tlv_len;
        *pdu_len_p+=tlv_len;

        /*construct orginization tlv*/
        if((NULL!=ltm_p->org_tlv.type_p)&&(0!=ltm_p->org_tlv.length_p))
        {
            UI16_T org_len=0;

            memcpy(&org_len, ltm_p->org_tlv.length_p, sizeof(UI16_T) );
            org_len = L_STDLIB_Ntoh16(org_len) + 3 /* type(1) + length(2) */;
            memcpy(pdu_p, ltm_p->org_tlv.type_p, org_len);

            pdu_p+=org_len;
            *pdu_len_p+=org_len;
        }

        /*copy unknown tlv*/
        if((0!=ltm_p->unknown_tlv_length)&&(CFM_TYPE_MAX_UNKNOWN_TLV_SIZE>=ltm_p->unknown_tlv_length))
        {
            memcpy(pdu_p, ltm_p->unknown_tlv_a, ltm_p->unknown_tlv_length);
            pdu_p+=ltm_p->unknown_tlv_length;
            *pdu_len_p+=ltm_p->unknown_tlv_length;
        }

        /*construct end tlv*/
        CFM_ENGINE_ConstructEndTLV(pdu_p, &tlv_len);
        *pdu_len_p+=tlv_len;
    }
}

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
BOOL_T CFM_ENGINE_XmitLTM (
    UI8_T   *md_name_ap,
    UI32_T  md_name_len,
    UI8_T   *ma_name_ap,
    UI32_T  ma_name_len,
    UI32_T  dst_mep_id,
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T  transmit_mep_id,
    UI8_T   ttl,
    UI16_T  pkt_pri)
{
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    UI8_T           *pdu_p=NULL;      /* max payload length of L2 frame */
    UI32_T          egress_port=0;
    UI16_T          pdu_len = 0;
    UI8_T           target_mac[SYS_ADPT_MAC_ADDR_LEN]={0};
    BOOL_T          is_forward_ltm = FALSE;

    /*1. first find the remote mep or the local mep to send the ltm
     */
    if(FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_ap, md_name_len, ma_name_ap, ma_name_len, &md_p, &ma_p))
    {
        return FALSE;
    }

    if (  (dst_mep_id == 0)
        &&(FALSE == CFM_ENGINE_LocalIsValidDstMac(
                    dst_mac, md_p->level, FALSE))
       )
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "invalid mac");
        return FALSE;
    }

    /* 21.8.6 Target MAC Address
     * Validation Test: The Target MAC Address field contains an Individual,
     * and not a Group, MAC address.
     */
    if (  (FALSE == CFM_ENGINE_FindTargetMacAndMep(md_p->index,
                                               ma_p->index,
                                               ma_p->primary_vid,
                                               dst_mep_id,
                                               dst_mac,
                                               CFM_BACKDOOR_DEBUG_FLAG_LT,
                                               target_mac,
                                               &egress_port,
                                               mep_p))
        ||(target_mac[0] == 0x01)
       )
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "can't find target mac and mep");
        return FALSE;
    }

    if((0!= transmit_mep_id)&&(mep_p->identifier != transmit_mep_id))
    {
        /*if choose to use specify mep to transmit lbm*/
        if(FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, transmit_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
        {
            return FALSE;
        }

        if(CFM_TYPE_MP_DIRECTION_DOWN ==mep_p->direction)
        {
            egress_port = mep_p->lport;
        }
    }

    /* should not use mep's da for destination mac
     */
    if (0 == memcmp(mep_p->mac_addr_a, dst_mac, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "target is mep mac");
        return FALSE;
    }

    /*reset the result status, after transmit ltm then set the status*/
    mep_p->transmit_ltm_result=FALSE;

    if(FALSE == mep_p->transmit_ltm_status)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "ltm transmit status is false");
        CFM_OM_StoreMep(mep_p);
        return FALSE;
    }

    /*2.check cfm status.
     */
    {
        CFM_TYPE_CfmStatus_T global_status=CFM_TYPE_CFM_STATUS_DISABLE,port_status=CFM_TYPE_CFM_STATUS_DISABLE;

        CFM_OM_GetCFMGlobalStatus(&global_status);
        CFM_OM_GetCFMPortStatus(mep_p->lport, &port_status);

        /*if the port's and global status not enable, the ccm won't send*/
        if((CFM_TYPE_CFM_STATUS_ENABLE != global_status)||(CFM_TYPE_CFM_STATUS_ENABLE!=port_status))
        {
            CFM_OM_StoreMep(mep_p);
            return FALSE;
        }
    }

    /*record ttl*/
    mep_p->transmit_ltm_ttl=ttl;

    /*check if the ttl is 0 if the ttl is 0,
     *then it needn't to be process by up/down mep, beause it already reach ttl 0
     */
    if(ttl == 0)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "ttl is 0");
        CFM_OM_StoreMep(mep_p);
        return TRUE;
    }

    mep_p->transmit_ltm_seq_number=mep_p->ltm_next_seq_number;
    mep_p->ltm_next_seq_number++;

    /*if the mep is the up mep, find the egress port and do the first respond for the ltm*/
    if(mep_p->direction == CFM_TYPE_MP_DIRECTION_UP)
    {
        CFM_OM_MEP_T *high_level_mep_p=&mep_tmp2_g;

        if (FALSE == CFM_ENGINE_RespondeUpMepLtm(
                        md_p, ma_p, mep_p, target_mac, &egress_port, &is_forward_ltm))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "up mep response ltm fail");
            CFM_OM_StoreMep(mep_p);
            return FALSE;
        }

        /*check if the ttl is 1, if the ttl is 1 then ttl will be 0 after up mep processs it won't forwarded,
          so it needn't transmit the ltm
         */
        if (ttl == 1)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "ttl is 1, so will not forward");
            CFM_OM_StoreMep(mep_p);
            return TRUE;
        }

        if (FALSE == is_forward_ltm)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT,
                "not forward(up mep on egress port/target arrived)");
            CFM_OM_StoreMep(mep_p);
            return TRUE;
        }

        /*if egress port exist higher level, same direciton mep, return*/
        if (TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                        egress_port, mep_p->md_p->level, mep_p->primary_vid,
                        CFM_TYPE_MP_DIRECTION_UP_DOWN, FALSE, high_level_mep_p, NULL))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "exist higher level mp");
            CFM_OM_StoreMep(mep_p);
            return FALSE;
        }
    }
    else
    {
        egress_port = mep_p->lport;
    }

    /*assing the egress identifier*/
    {
        UI16_T port_num=(UI16_T)mep_p->lport;

#if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE)
        if(mep_p->direction == CFM_TYPE_MP_DIRECTION_UP)
        {
            SWCTRL_GetCpuMac(&mep_p->transmit_ltm_egress_identifier[2]);
        }
        else
#endif /* #if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE) */
        {
            SWCTRL_GetPortMac(mep_p->lport, &mep_p->transmit_ltm_egress_identifier[2]);
        }

        port_num = L_STDLIB_Hton16(port_num);
        memcpy(mep_p->transmit_ltm_egress_identifier, &port_num,2);
    }

    /*3. transmit the ltm
     */
    if(NULL==(pdu_p=(UI8_T *)L_MM_Malloc(CFM_TYPE_MAX_FRAME_SIZE, L_MM_USER_ID2(SYS_MODULE_CFM, CFM_TYPE_XMITLTM))))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "can't allocate memory");
        CFM_OM_StoreMep(mep_p);
        return FALSE;
    }

    memset(pdu_p, 0, CFM_TYPE_MAX_FRAME_SIZE);

    /*constrcut the LTM pdu*/
    CFM_ENGINE_ConstructLTMPDU(pdu_p, &pdu_len, target_mac, mep_p);

    {
        UI8_T   tmp_dst_mac[SYS_ADPT_MAC_ADDR_LEN];

        /*assemble the ltm destination group address*/
        CFM_ENGINE_ASSEMBLE_DEST_MAC(tmp_dst_mac, mep_p->md_p->level+CFM_TYPE_LTM_MD_LEVEL_0)

        CFM_ENGINE_XmitPDU(egress_port, mep_p->primary_vid, mep_p->md_p->level, pkt_pri, TRUE,
                           tmp_dst_mac, mep_p->mac_addr_a, CFM_TYPE_MP_DIRECTION_DOWN, pdu_p, pdu_len);
    }

    /*set the transmit status*/
    mep_p->transmit_ltm_result=TRUE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "transmitted through port=%ld", (long)egress_port);

    if(NULL!=pdu_p)
        L_MM_Free(pdu_p);

    CFM_OM_StoreMep(mep_p);
    return TRUE;
}/*End of CFM_ENGINE_XmitLTM*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructLTMPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will construct the LTM packet
 * INPUT    : *pdu_p   - the packet pointer
 *            *pdu_len - the pdu_len pointer of the packet content length
 *           *target_mac  - ltm trace target mac
 *            *mep_p   - the mep pointe which tranmit the ltm
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ConstructLTMPDU(
    UI8_T           *pdu_p,
    UI16_T          *pdu_len_p,
    UI8_T           target_mac[SYS_ADPT_MAC_ADDR_LEN],
    CFM_OM_MEP_T    *mep_p)
{
    UI8_T flag=0;

    {/*1. constrcut common header, common header has 4 bytes, the end of tlv will be add at the end of pdu*/
        if(TRUE == mep_p->ltm_use_fdb_only)
        {
            flag=0x80;
        }

        CFM_ENGINE_ConstructCommonHeader(pdu_p, pdu_len_p, mep_p->md_p->level, flag, CFM_TYPE_OPCODE_LTM);

        /*move to next octect*/
        pdu_p+=*pdu_len_p;
    }

    {/*2. assign LTM transaction id*/
        UI32_T tmp=L_STDLIB_Hton32(mep_p->transmit_ltm_seq_number);
        memcpy(pdu_p, &tmp, sizeof(UI32_T));
        pdu_p+=4;
        *pdu_len_p+=4;
    }

    {/*3. assign ttl*/
        if(CFM_TYPE_MP_DIRECTION_UP==mep_p->direction)
        {
            *pdu_p=mep_p->transmit_ltm_ttl-1;
        }
        else
        {
            *pdu_p=mep_p->transmit_ltm_ttl;
        }

        /*move to next octect*/
        pdu_p+=1;
        *pdu_len_p+=1;
    }

    {/*4. original mac*/
        memcpy(pdu_p, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN);

        /*move to next octect*/
        pdu_p+=6;
        *pdu_len_p+=6;
    }
    {/*5. target mac*/
        memcpy(pdu_p,target_mac, SYS_ADPT_MAC_ADDR_LEN);

        /*move to next octect*/
        pdu_p+=6;
        *pdu_len_p+=6;
    }

    /*6. tlv*/
    {/*construct egress tlv*/
        UI16_T len=0;

        *pdu_p=CFM_TYPE_TLV_LTM_EGRESS_ID;
        len=L_STDLIB_Hton16(8);
        memcpy(pdu_p+1, &len, sizeof(len));

        memcpy(pdu_p+3, mep_p->transmit_ltm_egress_identifier, SYS_ADPT_MAC_ADDR_LEN+2);

        /*move to next octect*/
        pdu_p+=11;
        *pdu_len_p+=11;
    }
    {/*constrcut sender tlv*/
        UI16_T tlv_len=0;
        CFM_ENGINE_ConstructSenderTLV(pdu_p, &tlv_len, mep_p->lport);
        pdu_p+=tlv_len;
        *pdu_len_p+=tlv_len;

      /*constrcut end tlv*/
        CFM_ENGINE_ConstructEndTLV( pdu_p, &tlv_len);
        *pdu_len_p+=tlv_len;
    }
    return;
}/*End of CFM_ENGINE_ConstructLTMPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_RespondeUpMepLtm
 *-------------------------------------------------------------------------
 * PURPOSE  : This function do the UP mep respond for LTM
 * INPUT    : md_p             - the md pointer
 *            ma_p             - the ma pointer
 *            mep_p            - the mep pointer
 *            dst_mac          - the destination mac
 * OUTPUT   : egress_port_p    - the egress LTR forward egress port
 *            is_forward_ltm_p - TRUE to forward the LTM via egress port
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_RespondeUpMepLtm(
    CFM_OM_MD_T     *md_p,
    CFM_OM_MA_T     *ma_p,
    CFM_OM_MEP_T    *mep_p,
    UI8_T           dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T          *egress_port_p,
    BOOL_T          *is_forward_ltm_p)
{
    CFM_OM_MEP_T                *egress_mep_p=&mep_tmp2_g;
    CFM_OM_MEP_T                *high_level_mep_p= &mep_tmp3_g;
    CFM_OM_LTR_T                *ltr_p=&ltr_tmp_g;
    CFM_OM_MIP_T                egress_mip;
    CFM_ENGINE_PduHead_T        header;
    CFM_TYPE_LinktraceStatus_T  status=CFM_TYPE_LINKTRACE_STATUS_ENABLE;
    CFM_TYPE_RelayAction_T      relay_action=CFM_TYPE_RELAY_ACTION_FDB;
    BOOL_T                      reply_mp_ltm=FALSE;
    BOOL_T                      arrive_target=FALSE;
    BOOL_T                      need_reply_egress_tlv = FALSE;  /* TRUE for egress port is mip or up_mep */

    /*assign the necessary value for find egress port*/
    header.tag_info  = mep_p->primary_vid;
    header.flags     = ((TRUE == mep_p->ltm_use_fdb_only)?0x80:0);
    header.lport     = mep_p->lport;
    *is_forward_ltm_p= FALSE;

    /*find the ltm egress port and relay action*/
    if(FALSE == CFM_ENGINE_FindLtmEgressPort(md_p, ma_p, dst_mac, &header, egress_port_p, &relay_action))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "can't find egress port");
        return FALSE;
    }

    /*20.42.1.4.f egress meet up mhf*/
    if(TRUE == CFM_OM_GetMip(md_p->index, ma_p->index, *egress_port_p, CFM_OM_LPORT_MD_MA_KEY, &egress_mip))
    {
        reply_mp_ltm          = TRUE;
        need_reply_egress_tlv = TRUE;

        if(TRUE == CFM_ENGINE_IsArrivedTargetMac(*egress_port_p, dst_mac, CFM_TYPE_MP_DIRECTION_DOWN))
        {/*target mac*/
            arrive_target     = TRUE;
        }
        else
        {
            *is_forward_ltm_p = TRUE;
        }
    }
    /*20.42.1.4.g check egress port has same level mep*/
    else if (TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, *egress_port_p, CFM_OM_LPORT_MD_MA_KEY, egress_mep_p))
    {
        /* egress meet up MEP*/
        if(CFM_TYPE_MP_DIRECTION_UP == egress_mep_p->direction)
        {
            reply_mp_ltm          = TRUE;
            need_reply_egress_tlv = TRUE;
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "meet up mep");
        }
        else /*meet down mp*/
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "meet down mep fail");
            return FALSE;
        }
    }
    /*20.42.1.4.h check if there are a higher level up mep or mip exist*/
    else if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                        *egress_port_p, mep_p->md_p->level, mep_p->primary_vid,
                        CFM_TYPE_MP_DIRECTION_UP, FALSE, high_level_mep_p, NULL))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "exist higher level mp");
        return FALSE;
    }
    /*20.42.1.4.i nor same level or higher leve up mep or up mhf exist, enqueue and forward ltm*/
    else
    {
        reply_mp_ltm      = TRUE;
        *is_forward_ltm_p = TRUE;
    }

    /*check link trace cache status, if link trace cache no eanble then needn't process*/
    if( (FALSE == CFM_OM_GetLinkTraceCacheStatus(&status)) ||(CFM_TYPE_LINKTRACE_STATUS_DISABLE == status))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "link trace cache status is disabled");
        return TRUE;
    }

    /*if up mep generate the ltm just put it into record*/
    if(TRUE == reply_mp_ltm)
    {
        UI8_T egress_port_mac[SYS_ADPT_MAC_ADDR_LEN];

        memset(ltr_p,0,sizeof(CFM_OM_LTR_T));

        /*copy the reply message to the ltr_om*/
        ltr_p->md_index     = mep_p->md_index;
        ltr_p->ma_index     = mep_p->ma_index;
        ltr_p->rcvd_mep_id  = mep_p->identifier;
        ltr_p->seq_number   = mep_p->transmit_ltm_seq_number,
        ltr_p->receive_order= 1;

        if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                    *egress_port_p,             mep_p->md_p->level,
                    mep_p->primary_vid,         CFM_TYPE_MP_DIRECTION_UP_DOWN,
                    FALSE,  high_level_mep_p,   NULL))
        {
            ltr_p->forwarded=FALSE;
        }
        else
        {
            ltr_p->forwarded= ((*is_forward_ltm_p) && (mep_p->transmit_ltm_ttl >1)) ? TRUE : FALSE;
        }

        ltr_p->terminal_mep=(*egress_port_p!=0?TRUE:FALSE);

        memcpy(&ltr_p->last_egress_identifier[2], mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN);

        SWCTRL_GetPortMac(*egress_port_p,egress_port_mac);
        memcpy(&ltr_p->next_egress_identifier[2],egress_port_mac, SYS_ADPT_MAC_ADDR_LEN);

        /*sender tlv
         */
        ltr_p->chassis_id_subtype=CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_MAC_ADDR;
        ltr_p->chassis_id_length=SYS_ADPT_MAC_ADDR_LEN;
        SWCTRL_GetCpuMac(ltr_p->chassis_id);

        {/*assign mgmt*/
            NETCFG_TYPE_InetRifConfig_T  rif_config;
            BOOL_T ip_exist=FALSE;

            memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

            while(NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextRifConfig(&rif_config))
            {
                if(TRUE == VLAN_OM_IsPortVlanMember(rif_config.ifindex, mep_p->lport))
                {
                    ltr_p->mgmt_addr_domain_len = CFM_ENGINE_ASN1_TRANS_DOMAIN_OID_LEN;
                    memcpy(ltr_p->mgmt_addr_domain, asn1_transport_domain_oid[0], CFM_ENGINE_ASN1_TRANS_DOMAIN_OID_LEN);
                    ltr_p->mgmt_addr_len=SYS_ADPT_IPV4_ADDR_LEN;
                    memcpy(ltr_p->mgmt_addr, rif_config.addr.addr, ltr_p->mgmt_addr_len);
                    ip_exist=TRUE;
                    break;
                }
            }

            if(FALSE == ip_exist)
            {
                ltr_p->mgmt_addr_domain_len=0;
            }
        }

        ltr_p->relay_action=relay_action;

        /* If the LTM was received by a Down MEP or a Down MHF, the LTR includes a Reply Ingress TLV
        {
            ltr_p->ingress_action=CFM_TYPE_INGRESS_ACTION_OK;
            memcpy(ltr_p->ingress_mac, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN);
            ltr_p->ingress_port_id_subtype=CFM_TYPE_PORT_ID_SUBTYPE_MAC_ADDR;
            ltr_p->ingress_port_id_lenth=SYS_ADPT_MAC_ADDR_LEN;
            memcpy(ltr_p->ingress_port_Id, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN);
        }
         */

        /* refer to CFM_ENGINE_ConstructLTRPDU,
         *   the code here should do the same thing as there
         */
        /*find egress port action*/
        if (TRUE == need_reply_egress_tlv)
        {
            ltr_p->egress_action = CFM_ENGINE_GetEgressActionForLTR(
                                        mep_p->primary_vid, *egress_port_p);

            memcpy(ltr_p->egress_mac, egress_port_mac, SYS_ADPT_MAC_ADDR_LEN);
            ltr_p->egress_port_id_subtype=CFM_TYPE_PORT_ID_SUBTYPE_MAC_ADDR;
            ltr_p->egress_port_id_lenth=SYS_ADPT_MAC_ADDR_LEN;
            SWCTRL_GetPortMac(*egress_port_p, ltr_p->egress_port_id);
        }

        if (FALSE == CFM_ENGINE_LocalAddLtrToOM(ltr_p))
        {
            return FALSE;
        }
    }

    return TRUE;
}/*End of CFM_ENGINE_RespondeUpMepLtm*/

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessAIS
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the received AIS message
 * INPUT    : *pdu_p     - the pdu pointer
 *                *header_p - the common header and msge infomation pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : 1. there is no function to verify AIS pdu is ok or not, because AIS all information is in common header
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ProcessAIS(
                                    UI8_T *pdu_p,
                                    CFM_ENGINE_PduHead_T *header_p)
{
    CFM_TYPE_MpType_T   mp_type=CFM_TYPE_MP_TYPE_NONE;
    CFM_OM_MEP_T        *mep_p=&mep_tmp1_g;
    CFM_OM_MIP_T        mip;
    CFM_OM_MD_T         *md_p=NULL;
    CFM_OM_MA_T         *ma_p=NULL;
    CFM_OM_MEP_T        *high_level_mep_p=&mep_tmp2_g;
    CFM_OM_MIP_T        high_level_mip;
    CFM_ENGINE_AisPdu_T ais_pdu;
    UI32_T              lport=0;
    BOOL_T              no_md_ma=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "");

    ais_pdu.header_p = header_p;

    /*verify Flags, we only process 1 sec(0x04) and 1 min(0x06)*/
    if( (header_p->flags&0x07) != 4 && (header_p->flags&0x07) != 6)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "Drop packet, lport=%ld, AIS flag is wrong", header_p->lport);
        return FALSE;
    }

    /*get a mp to process the message*/
    mp_type= CFM_ENGINE_GetMpProcessRcvdPacket(header_p, mep_p, &mip);

    if(CFM_TYPE_MP_TYPE_MEP == mp_type)
    {
        if(CFM_TYPE_MP_DIRECTION_UP==mep_p->direction)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "Drop packet, lport=%ld, AIS was received by up MEP from back side", header_p->lport);
            return FALSE;
        }

        CFM_ENGINE_MepProcessAIS(pdu_p, header_p, mep_p);
        CFM_OM_StoreMep(mep_p);
        return TRUE;
    }
    else if(CFM_TYPE_MP_TYPE_NONE == mp_type)
    {
        if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(header_p->lport, header_p->md_level, header_p->tag_info&0x0fff,
                                                                             CFM_TYPE_MP_DIRECTION_DOWN, FALSE, high_level_mep_p, &high_level_mip))
        {
            /*higher level recieve lower level message, drop it*/
            return TRUE;
        }
        /*this port donesn't has higher level mep, (don't care the mip, because mip still pass through the lower pdu)
          flood to other port.
          */
    }
    /*if receved by mip or no mp, the pdu still need be forward to other port*/
    else if(CFM_TYPE_MP_TYPE_MIP == mp_type)
    {
        /*if the pdu received by mip, this pdu should fowrad to other port's mep*/
    }

    /*if this switch doesn't has the md ma, then flood to each port, and according to each port's mep configuration
     */
    if(FALSE == CFM_OM_GetMdMaByLevelVid(header_p->md_level, header_p->tag_info&0x0fff, &md_p, &ma_p))
    {
        no_md_ma=TRUE;
    }

    /* ingress port is blocking, should not need to do forwarding
     */
    if (FALSE == header_p->is_ing_lport_fwd)
    {
        return TRUE;
    }

    /*  check all other port -> no mep -flood, up mep- process, down mep-skip*/
    while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
    {
        if(lport == header_p->lport )
        {
            continue;
        }

        if((FALSE == no_md_ma)&&(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p)))
        {
            if(CFM_TYPE_MP_DIRECTION_UP == mep_p->direction)
            {
                CFM_ENGINE_MepProcessAIS(pdu_p, header_p, mep_p);
                CFM_OM_StoreMep(mep_p);
            }

            continue;
        }
        else /* ais flood to this port*/
        {
            UI32_T vlan_ifindex=0;

            VLAN_OM_ConvertToIfindex(header_p->tag_info&0x0fff, &vlan_ifindex);

            /* check whether the port is vlan's member */
            if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, lport))
            {
                continue;
            }

            if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                        lport, header_p->md_level, header_p->tag_info&0x0fff,
                        CFM_TYPE_MP_DIRECTION_UP_DOWN, FALSE,
                        high_level_mep_p, &high_level_mip))
            {
                continue;
            }

            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "Xmit to port %ld", lport);
            CFM_ENGINE_XmitPDU(lport,
                                header_p->tag_info&0x0fff,
                                header_p->md_level,
                                (header_p->tag_info&0xf000)>>13,
                                FALSE,
                                header_p->dst_mac_a,
                                header_p->src_mac_a,
                                CFM_TYPE_MP_DIRECTION_UP,
                                pdu_p, header_p->pdu_length);
        }
    }
    return TRUE;
}/*End of CFM_ENGINE_ProcessAIS*/
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructAISPdu
 *-------------------------------------------------------------------------
 * PURPOSE  : This function construct the AIS packet
 * INPUT    : *ma_p   - the ma pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ConstructAISPdu(
                                        UI8_T *pdu_p,
                                        UI16_T *pdu_len_p,
                                        CFM_TYPE_MdLevel_T level,
                                        UI16_T ais_period)
{
    UI8_T flag=0;

    memset(pdu_p, 0, CFM_TYPE_MAX_FRAME_SIZE);
    *pdu_len_p =0;

    if( CFM_TYPE_AIS_PERIOD_1S == ais_period)
    {
        flag= 0x04;
    }
    else
    {
        flag=0x06;
    }

    CFM_ENGINE_ConstructCommonHeader(pdu_p, pdu_len_p, level, flag, CFM_TYPE_OPCODE_AIS);

    /*add end tlv*/
    *(pdu_p+(*pdu_len_p)) = 0;
    *pdu_len_p+=1;

    return TRUE;
}/*End of CFM_ENGINE_ConstructAISPdu*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitAis
 *-------------------------------------------------------------------------
 * PURPOSE  : This function send the AIS
 * INPUT    : mep_p      - the mep pointer
 *            is_forward - to indicate if it's to fordward the AIS
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_XmitAis(
                                CFM_OM_MEP_T    *mep_p,
                                BOOL_T          is_fordward)
{
    CFM_OM_MD_T     *nxt_md_p=NULL;
    CFM_OM_MA_T     *nxt_ma_p=NULL;
    CFM_OM_MEP_T    *nxt_mep_p=&mep_tmp4_g;
    UI32_T          vlan_ifindex, nxt_md_idx=0, nxt_ma_idx=0;
    UI16_T          pdu_len=0;
    UI8_T           *pdu_p=NULL, dst_mac_addr [ SYS_ADPT_MAC_ADDR_LEN ]={0};

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS,
        "mep-%ld, lvl-%d, fwd-%d", (long)mep_p->identifier, mep_p->md_p->level, is_fordward);

    /* according to Y.1731 7.4
     * 1. Signal fail conditions in the case that ETH-CC is enabled.
     * 2. AIS condition or LCK condition in the case that ETH-CC is disabled.
     *
     * in current design, use crosscheck status to do the checking
     */
    if (CFM_TYPE_CROSS_CHECK_STATUS_ENABLE == mep_p->ma_p->cross_check_status)
    {
        /* cc enable && fordward      => do nothing
         */
        if (TRUE  == is_fordward)
            return FALSE;
    }
    else
    {
        /* cc disable && not fordward => do nothing
         */
        if (FALSE == is_fordward)
            return FALSE;
    }

    if(NULL == (pdu_p=(UI8_T *)L_MM_Malloc(CFM_TYPE_MAX_FRAME_SIZE, L_MM_USER_ID2(SYS_MODULE_CFM, CFM_TYPE_XMITAIS))))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "allocate memory failed");
        return FALSE;
    }

    while(NULL!=(nxt_md_p=CFM_OM_GetNextMdByIndex(nxt_md_idx)))
    {
        nxt_md_idx = nxt_md_p->index;

        if(     nxt_md_p->level > mep_p->md_p->level
            &&(   mep_p->ma_p->ais_send_level == nxt_md_p->level /*send to designated level*/
                 ||mep_p->ma_p->ais_send_level == 0 /*default, so send all higher level md*/
              )
          )
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "lvl-%d", nxt_md_p->level);

            nxt_ma_idx=0;
            /*lookup all ma to send ais*/
            while(NULL!=(nxt_ma_p =CFM_OM_GetNextMaByMaIndex(nxt_md_p->index, nxt_ma_idx)))
            {
                nxt_ma_idx = nxt_ma_p->index;

                /* ais is disabled, does not need to process
                 */
                if(CFM_TYPE_AIS_STATUS_DISABLE  == nxt_ma_p->ais_status)
                {
                    continue;
                }

                /* current ma's primary vid is not the same as mep's
                 * ,so do nothing...
                 */
                if(nxt_ma_p->primary_vid!=mep_p->ma_p->primary_vid)
                {
                    continue;
                }

                /*only process the port joined vlan*/
                VLAN_OM_ConvertToIfindex( mep_p->ma_p->primary_vid, &vlan_ifindex);
                if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, mep_p->lport))
                {
                    continue;
                }

                if(TRUE == CFM_OM_GetMep(nxt_md_p->index, nxt_ma_p->index, 0, mep_p->lport, CFM_OM_LPORT_MD_MA_KEY, nxt_mep_p))
                {/*becaue this level has mep, so search other ma or more higher level*/
                    continue;
                }

                /* update the ma of upper level's suppressing status,
                 *  bcz AIS is notified from lower level mep
                 *
                 *  if dosen't receive AIS, create a timer
                 *  if ever receive AIS, update timer
                  */
                if(-1 == nxt_ma_p->ais_rcvd_timer_idx) /*never received ais, create new timer*/
                {
                    CFM_Timer_CallBackPara_T para;

                    CFM_TIMER_AssignTimerParameter(&para, nxt_md_p->index, nxt_ma_p->index, 0, 0, 0);

                    nxt_ma_p->ais_rcvd_timer_idx=CFM_TIMER_CreateTimer(
                                                         CFM_ENGINE_AIS_Callback,
                                                         &para,
                                                         3.5*nxt_ma_p->ais_period,
                                                         CFM_TIMER_ONE_TIME);
                    CFM_TIMER_StartTimer(nxt_ma_p->ais_rcvd_timer_idx);
                }
                else/*ever recieve the ais, updat the timer*/
                {
                    CFM_TIMER_UpdateTimer(nxt_ma_p->ais_rcvd_timer_idx,
                                                          3.5*nxt_ma_p->ais_period,
                                                          CFM_ENGINE_AIS_Callback);
                }

                CFM_ENGINE_ConstructAISPdu(pdu_p, &pdu_len,  nxt_md_p->level, nxt_ma_p->ais_period);

                /*calculate the dst mac*/
                CFM_ENGINE_ASSEMBLE_DEST_MAC(dst_mac_addr, nxt_md_p->level);

                /* send AIS to the backward direction of MEP
                 */
                if(CFM_TYPE_MP_DIRECTION_DOWN == mep_p->direction)
                {
                    UI32_T lport=0;

                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "");

                    /* 1. in higher level, if other port has down mep, continue
                     * 2. in higher level, if other port has no mep, xmit.
                     * 3. in higher level, if other port has up mep, search higher level again.
                     */
                    while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
                    {
                        CFM_OM_MEP_T*    tmp_mep_p=&mep_tmp4_g;
                        CFM_OM_MD_T     *tmp_md_p=NULL;
                        CFM_OM_MA_T     *tmp_ma_p=NULL;
                        UI32_T          tmp_nxt_md_idx=0, tmp_nxt_ma_idx=0;
                        BOOL_T          up_mep_xmit=FALSE;

                        if(lport == mep_p->lport)
                            continue;

                        if(FALSE == CFM_OM_GetMep(nxt_md_p->index, nxt_ma_p->index, 0, lport,  CFM_OM_LPORT_MD_MA_KEY, tmp_mep_p))
                        {
                            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "Xmit to port %ld", (long)lport);
                            CFM_ENGINE_XmitPDU(lport,
                                              nxt_ma_p->primary_vid,
                                              nxt_md_p->level,
                                              mep_p->ccm_ltm_priority,
                                              FALSE,
                                              dst_mac_addr,
                                              mep_p->mac_addr_a,
                                              CFM_TYPE_MP_DIRECTION_DOWN,
                                              pdu_p,
                                              pdu_len);
                        }
                        else if(CFM_TYPE_MP_DIRECTION_DOWN == tmp_mep_p->direction)
                        {
                            continue;
                        }
                        else /* 3. in higher level, if other port has up mep, search higher level again.*/
                        {
                            up_mep_xmit=FALSE;

                            /* when coming here we found a tmp_mep whose level
                             *  a. equal to mep_p->ma_p->ais_send_level
                             *     or
                             *  b. equal to the first level to send out AIS,
                             *     if mep_p->ma_p->ais_send_level is not configured
                             *
                             *     only condition b. need to search if there is another
                             *     good level to send out AIS.
                             */
                            if (mep_p->ma_p->ais_send_level != SYS_DFLT_CFM_AIS_LEVEL)
                            {
                                continue;
                            }

                            /* keep searching another good level to send AIS,
                             * if mep_p->ma_p->ais_send_level is not configured
                             */
                            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "up mep -%ld, lport -%ld", (long)tmp_mep_p->identifier, (long)lport);
                            tmp_nxt_md_idx=0;
                            while(FALSE == up_mep_xmit
                                    && NULL!=(tmp_md_p=CFM_OM_GetNextMdByIndex(tmp_nxt_md_idx)))
                            {
                                tmp_nxt_md_idx = tmp_md_p->index;

                                if(tmp_md_p->level > nxt_md_p->level )
                                {
                                    tmp_nxt_ma_idx=0;
                                    /*lookup all ma to send ais*/
                                    while(FALSE == up_mep_xmit
                                            && NULL!=(tmp_ma_p =CFM_OM_GetNextMaByMaIndex(tmp_md_p->index, tmp_nxt_ma_idx)) )
                                    {
                                        UI8_T   tmp_dst_mac_addr[SYS_ADPT_MAC_ADDR_LEN];

                                        tmp_nxt_ma_idx = tmp_ma_p->index;

                                        if(CFM_TYPE_AIS_STATUS_DISABLE  == tmp_ma_p->ais_status)
                                        {
                                            continue;
                                        }

                                        /* current tmp ma's primary vid is not the same as mep's
                                         * ,so do nothing...
                                         */
                                        if(tmp_ma_p->primary_vid != nxt_ma_p->primary_vid)
                                        {
                                            continue;
                                        }

                                        /*only process the port joined vlan*/
                                        if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, lport))
                                        {
                                            continue;
                                        }

                                        if(TRUE == CFM_OM_GetMep(tmp_md_p->index, tmp_ma_p->index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, tmp_mep_p))
                                        {/*becaue this level has mep, so search other ma or more higher level*/
                                            continue;
                                        }

                                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "lvl -%ld, lport -%ld", (long)tmp_md_p->level, (long)lport);

                                        /* update the ma of upper level's suppressing status (searched from the up mep),
                                         *  bcz AIS is notified from lower level mep
                                         *
                                         *  if dosen't receive AIS, create a timer
                                         *  if ever receive AIS, update timer
                                          */
                                        if(-1 == tmp_ma_p->ais_rcvd_timer_idx) /*never received ais, create new timer*/
                                        {
                                            CFM_Timer_CallBackPara_T para;

                                            CFM_TIMER_AssignTimerParameter(&para, tmp_md_p->index, tmp_ma_p->index, 0, 0, 0);

                                            tmp_ma_p->ais_rcvd_timer_idx=CFM_TIMER_CreateTimer(
                                                                                 CFM_ENGINE_AIS_Callback,
                                                                                 &para,
                                                                                 3.5*tmp_ma_p->ais_period,
                                                                                 CFM_TIMER_ONE_TIME);
                                            CFM_TIMER_StartTimer(tmp_ma_p->ais_rcvd_timer_idx);
                                        }
                                        else/*ever recieve the ais, updat the timer*/
                                        {
                                            CFM_TIMER_UpdateTimer(tmp_ma_p->ais_rcvd_timer_idx,
                                                                                  3.5*tmp_ma_p->ais_period,
                                                                                  CFM_ENGINE_AIS_Callback);
                                        }

                                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "Xmit to port %ld", (long)lport);

                                        /* if (crosscheck is disable) {
                                         *   if (ais send timer does not exist) {
                                         *     send AIS and create ais send timer
                                         *   }
                                         *   else {
                                         *     do nothing, since the timer will do this when time up
                                         *   }
                                         *   set up_mep_xmit to TRUE
                                         * }
                                         * else {
                                         *   do nothing, bcz no forwarding when cc is enabled
                                         * }
                                         *
                                         * use send timer to prevent forwarding too many AIS packets
                                         */
                                        if (CFM_TYPE_CROSS_CHECK_STATUS_DISABLE == tmp_mep_p->ma_p->cross_check_status)
                                        {
                                            if (-1 == tmp_mep_p->ais_send_timer_idx)
                                            {
                                                CFM_Timer_CallBackPara_T    para;

                                                /*calculate the dst mac*/
                                                CFM_ENGINE_ASSEMBLE_DEST_MAC(tmp_dst_mac_addr, tmp_md_p->level);

                                                CFM_ENGINE_XmitPDU(lport,
                                                                  tmp_ma_p->primary_vid,
                                                                  tmp_md_p->level,
                                                                  mep_p->ccm_ltm_priority,
                                                                  FALSE,
                                                                  tmp_dst_mac_addr,
                                                                  mep_p->mac_addr_a,
                                                                  CFM_TYPE_MP_DIRECTION_DOWN,
                                                                  pdu_p,
                                                                  pdu_len);

                                                CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);
                                                mep_p->ais_send_timer_idx=CFM_TIMER_CreateTimer(
                                                                               CFM_ENGINE_XmitAis_Callback,
                                                                               &para,
                                                                               mep_p->ma_p->ais_period == CFM_TYPE_AIS_PERIOD_1S?1:60,
                                                                               CFM_TIMER_CYCLIC);
                                                if(mep_p->ais_send_timer_idx!=-1)
                                                    CFM_TIMER_StartTimer(mep_p->ais_send_timer_idx);
                                            }
                                            else
                                            {
                                                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "by timer");
                                            }

                                            up_mep_xmit=TRUE;
                                        }
                                    }/* while(FALSE == up_mep_xmit && NULL!=(tmp_ma_p =CFM_OM_GetNextMaByMaIndex(tmp_md_p->index, tmp_nxt_ma_idx)) )*/
                                }/*end if(tmp_md_p->level > nxt_md_p->level )*/
                            }/* while(FALSE == up_mep_xmit && NULL!=(tmp_md_p=CFM_OM_GetNextMdByIndex(tmp_nxt_md_idx)))*/
                        }/*end  3. in higher level, if other port has up mep, search higher level again.*/
                    }
                }
                else
                {
                    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "Xmit to port %ld", (long)mep_p->lport);
                    CFM_ENGINE_XmitPDU(mep_p->lport,
                                      nxt_ma_p->primary_vid,
                                      nxt_md_p->level,
                                      mep_p->ccm_ltm_priority,
                                      FALSE,
                                      dst_mac_addr,
                                      mep_p->mac_addr_a,
                                      CFM_TYPE_MP_DIRECTION_DOWN,
                                      pdu_p,
                                      pdu_len);
                }
            } /* while(NULL!=(nxt_ma_p =CFM_OM_GetNextMaByMaIndex(nxt_md_p->index, nxt_ma_idx))) */
            break;
        }/* if(     nxt_md_p->level > mep_p->md_p->level &&(   mep_p->ma_p->ais_send_level == nxt_md_p->level ||mep_p->ma_p->ais_send_level == 0 )*/
    } /* while(NULL!=(nxt_md_p=CFM_OM_GetNextMdByIndex(nxt_md_idx))) */

    L_MM_Free(pdu_p);
    return TRUE;
}/*End of CFM_ENGINE_XmitAis*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MepProcessAIS
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the received AIS message
 * INPUT    : heder_p - the message header pointer
 *            mep_p   - the mep pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MepProcessAIS(
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_OM_MEP_T            *mep_p)
{
    CFM_OM_MA_T     *ma_p=mep_p->ma_p;
    UI32_T          ais_interval= (((header_p->flags&0x07) == 4)?1:60);

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS,
        "mep-%ld, lvl-%d", (long)mep_p->identifier, mep_p->md_p->level);

    /* ais is disabled, does not need to process
     */
    if(CFM_TYPE_AIS_STATUS_DISABLE == ma_p->ais_status)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "ais status disable");
        return TRUE;
    }

    /*add to error list
      */
    CFM_OM_AddError(SYS_TIME_GetSystemTicksBy10ms() ,CFM_TYPE_CONFIG_ERROR_AIS, mep_p, header_p->src_mac_a);

    /*
       if dosen't receive AIS, create a timer
       if ever receive AIS, update timer
      */

    if(-1 == ma_p->ais_rcvd_timer_idx) /*never received ais, create new timer*/
    {
        CFM_Timer_CallBackPara_T    para;

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "create ais_rcvd_timer");

        CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);

        ma_p->ais_rcvd_timer_idx=CFM_TIMER_CreateTimer(
                                             CFM_ENGINE_AIS_Callback,
                                             &para,
                                             3.5*ais_interval,
                                             CFM_TIMER_ONE_TIME);
        CFM_TIMER_StartTimer(ma_p->ais_rcvd_timer_idx);
    }
    else/*ever recieve the ais, updat the timer*/
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "update ais_rcvd_timer");
        CFM_TIMER_UpdateTimer(ma_p->ais_rcvd_timer_idx,
                                              3.5*ais_interval,
                                              CFM_ENGINE_AIS_Callback);
    }

    if(-1 == ma_p->ais_rcvd_timer_idx) /*doesn't create timer sucess*/
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "");
        return FALSE;
    }

    /* if (crosscheck is disable) {
     *   if (ais send timer does not exist) {
     *     send AIS and create ais send timer
     *   }
     *   else {
     *     do nothing, since the timer will do this when time up
     *   }
     * }
     *
     * use send timer to prevent forwarding too many AIS packets
     */
    if (CFM_TYPE_CROSS_CHECK_STATUS_DISABLE == mep_p->ma_p->cross_check_status)
    {
        if (-1 == mep_p->ais_send_timer_idx)
        {
            CFM_Timer_CallBackPara_T    para;

            CFM_ENGINE_XmitAis(mep_p, TRUE); /*transmit AIS*/

            CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);
            mep_p->ais_send_timer_idx=CFM_TIMER_CreateTimer(
                                           CFM_ENGINE_XmitAis_Callback,
                                           &para,
                                           mep_p->ma_p->ais_period == CFM_TYPE_AIS_PERIOD_1S?1:60,
                                           CFM_TIMER_CYCLIC);
            if(mep_p->ais_send_timer_idx!=-1)
                CFM_TIMER_StartTimer(mep_p->ais_send_timer_idx);

            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "forward AIS packet and create timer");
        }
        else
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "AIS packet will be forwarded by timer");
        }
    }

    CFM_OM_StoreMep(mep_p);
    return TRUE;
}/*End of CFM_ENGINE_MepProcessAIS*/

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
BOOL_T CFM_ENGINE_SetMD(
                        UI32_T md_index,
                        UI8_T *name_ap,
                        UI16_T name_len,
                        CFM_TYPE_MdLevel_T level,
                        CFM_TYPE_MhfCreation_T create_way)
{
    CFM_OM_MD_T *md_p=NULL;

    md_p = CFM_OM_GetMdByIndex(md_index);

    if(NULL == md_p)
    {/*don't have this md*/
        if(FALSE == CFM_OM_AddNewMd(md_index, level, name_len, name_ap, create_way))
        {
            return FALSE;
        }
    }
    else
    { /*have this md, make sure change name*/

        if(md_p->name_length != name_len)
        {
            UI32_T max_ma_name_len =0;

            max_ma_name_len = CFM_OM_GetMaxMaNameLengthInMd(md_p);

            if((0!=name_len)&&((name_len+max_ma_name_len)>(CFM_TYPE_MAID_NAME_LENGTH)-4))
            {
                return FALSE;
            }

        }

        if(FALSE== CFM_OM_SetMd(md_p, level, name_len, name_ap, create_way))
        {
            return FALSE;
        }
    }

    return TRUE;
}/*End of CFM_ENGINE_SetMD*/

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
BOOL_T CFM_ENGINE_DeleteMD(
                            UI32_T md_index)
{
     CFM_OM_MD_T *md_p=NULL;

     md_p=CFM_OM_GetMdByIndex(md_index);

    if(NULL == md_p)
    {
        return FALSE;
    }

    if(md_p->ma_start_ar_idx >=0 )
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_DeleteMd(md_index))
    {
        return FALSE;
    }

    return TRUE;
}/*End of CFM_ENGINE_DeleteMD*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_CreateMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the empty ma, and this ma can't be used
 * INPUT    : ma_index   - the ma index
 *            md_index   - the md index
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :1. This function only will create ma with default value, and this ma can't be used.
 *           2. only for mib.
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_CreateMA(
                            UI32_T md_index,
                            UI32_T ma_index)
{
    CFM_OM_MD_T *md_p=NULL;
    UI16_T  primary_vid=CFM_TYPE_DFT_MA_PRIMARY_VID;
    UI8_T vid_list [(SYS_DFLT_DOT1QMAXVLANID / 8)+ 1 ]={0};

    if(NULL == (md_p=CFM_OM_GetMdByIndex(md_index)))
    {
        return FALSE;
    }

    if(primary_vid > 0)
    {
        vid_list[(UI32_T)((primary_vid-1)/8)] |= (1 << (7 - ((primary_vid-1)%8)));
    }

    /*ma use the md name first when create ma, so md name can't be 0, because ma name should not empty*/
    if(0==md_p->name_length)
    {
        return FALSE;
    }

    if(md_p->name_length*2 > (CFM_TYPE_MAID_NAME_LENGTH-4))
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_AddNewMa( md_index, ma_index, md_p->name_length, md_p->name_a, primary_vid, 1, vid_list, SYS_DFLT_CFM_MIP_CREATE ))
    {
        return FALSE;
    }

    return TRUE;
}/*CFM_ENGINE_CreateMA*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the Ma
 * INPUT    : ma_index   - the ma index
 *            *name_ap   - the ma name pointer
 *            name_len   - the ma name length
 *            md_index   - the md index
 *            primary_vid - the primary vid of the ma
 *            vid_num    - the vid number in vid list
 *            vlid_list  - the array store the vid list. one element store one vid
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMA(
    UI32_T                  ma_index,
    UI8_T                   *name_ap,
    UI32_T                  name_len,
    UI32_T                  md_index,
    UI16_T                  primary_vid,
    UI32_T                  vid_num,
    UI8_T                   vid_list[(SYS_DFLT_DOT1QMAXVLANID/8)+1],
    CFM_TYPE_MhfCreation_T  create_way)
{
    CFM_OM_MA_T *ma_p=NULL;
    CFM_OM_MD_T *md_p=NULL;
    BOOL_T      is_create_mip = FALSE;

    /*find the md*/
    if (NULL == (md_p=CFM_OM_GetMdByIndex(md_index)))
    {
        /*md doesn't exist*/
        return FALSE;
    }

    /*md !=0 , md and ma name share the CFM_TYPE_MAID_NAME_LENGTH-3 length*/
    if((md_p->name_length!=0)&&((md_p->name_length + name_len )>(CFM_TYPE_MAID_NAME_LENGTH-4)))
    {
       return FALSE;
    }

    /*md =0, ma name share the CFM_TYPE_MAID_NAME_LENGTH-3 length*/
    if((md_p->name_length==0)&&(name_len >(CFM_TYPE_MAID_NAME_LENGTH-3)))
    {
        return FALSE;
    }

    /*ma need name*/
    if((0==name_len)&&(NULL == name_ap))
    {
        return FALSE;
    }

    ma_p = CFM_OM_GetMa(md_index, ma_index);

    /*modify ma*/
    if(NULL!=ma_p)
    {
        CFM_OM_MEP_T *mep_p=&mep_tmp1_g;

        /*change priamry vid, it shall has no mep on this ma*/
        if(primary_vid!=ma_p->primary_vid)
        {
            if(TRUE == CFM_OM_GetNextMep(md_index, ma_index, 0, 0, CFM_OM_MD_MA_LPORT_KEY, mep_p))
            {
                if((mep_p->md_index==md_p->index)&&(mep_p->ma_index==ma_p->index))
                {
                    return FALSE;
                }
            }
        }

        /* if ma exists, check the name-format and name length
         */
        if (CFM_TYPE_MA_NAME_ICC_BASED == ma_p->format)
        {
            if (name_len > CFM_TYPE_MA_MAX_NAME_LENGTH_FOR_Y1731)
            {
                return FALSE;
            }
        }

        /* primary vid or create_way is changed, delete mip first
         */
        if(  (primary_vid != ma_p->primary_vid)
           ||(create_way  != ma_p->mhf_creation)
          )
        {
            CFM_ENGINE_DeleteMipByMa(md_p, ma_p);
            is_create_mip = TRUE;
        }

        if(FALSE == CFM_OM_SetMa(md_p, ma_p, name_ap, name_len, primary_vid, vid_num, vid_list, create_way))
        {
            return FALSE;
        }
    }
    /*create new ma*/
    else
    {
        if(FALSE == CFM_OM_AddNewMa(md_index, ma_index, name_len,
                        name_ap, primary_vid, vid_num, vid_list, create_way))
        {
            return FALSE;
        }
        is_create_mip = TRUE;
    }

    /* create mip for
     * 1. new ma
     * 2. old ma but primary vid or create_way is changed
     */
    if ((TRUE == is_create_mip) && (0 != primary_vid))
    {
        UI32_T lport=0;

        while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort( &lport))
        {
            CFM_ENGINE_CreateMIPByMa(md_p->level, primary_vid, lport);
        }
    }

    return TRUE;
}/* End of CFM_ENGINE_SetMA*/

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
BOOL_T CFM_ENGINE_DeleteMA(
                            UI32_T md_index,
                            UI32_T ma_index)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    CFM_OM_REMOTE_MEP_T remote_mep;

    if(FALSE == CFM_OM_GetMdMaByMdMaIndex( md_index, ma_index, &md_p, &ma_p))
    {
        return TRUE;
    }

    /*check if still exist mep*/
    if(TRUE == CFM_OM_GetNextMep(md_index, ma_index,0, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        if((md_index==mep_p->md_index)&&(ma_index==mep_p->ma_index))
        {
            return FALSE;
        }
    }

    /*check if exist user configured remote mep*/
    if(TRUE == CFM_OM_GetNextRemoteMep(md_index, ma_index,0, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        if((md_index==remote_mep.md_index)&&(ma_index==remote_mep.ma_index))
        {
            return FALSE;
        }
    }

    /*delete all mip on this ma and check the new mip*/
    {
        UI32_T      lport=0;
        CFM_OM_MD_T *tmp_md_p=NULL;
        CFM_OM_MA_T *tmp_ma_p=NULL;
        BOOL_T      has_high_level=FALSE;

        if(TRUE ==CFM_OM_GetFirstHighLevelMdMaBylevelVid(md_p->level, ma_p->primary_vid, &tmp_md_p, &tmp_ma_p))
        {
            has_high_level=TRUE;
        }

        /*delete the ma*/
        if(FALSE == CFM_OM_DeleteMa(md_index, ma_index))
        {
            return FALSE;
        }

        while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
        {
            if(FALSE == CFM_OM_DeleteMip(md_index, ma_index, lport, CFM_OM_LPORT_MD_MA_KEY))
            {
                return FALSE;
            }

            if(TRUE == has_high_level)
            {
                if(tmp_ma_p->primary_vid!=0)
                    CFM_ENGINE_CreateMIPByMa(tmp_md_p->level, tmp_ma_p->primary_vid, lport);
            }
        }
    }

    return TRUE;
}/* End of CFM_ENGINE_DeleteMA*/

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
BOOL_T CFM_ENGINE_DeleteMAVlan(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI16_T vid)
{
    CFM_OM_MA_T     *ma_p =NULL;
    CFM_OM_MD_T     *md_p =NULL;
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    UI16_T          nxt_vid =0;

    if(FALSE == CFM_OM_GetMdMaByMdMaIndex( md_index, ma_index, &md_p, &ma_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "md/ma/vid - %ld/%ld/%d", (long)md_index, (long)ma_index, vid);
        return FALSE;
    }

    /* check if still exist mep
     */
    if((ma_p->primary_vid == vid )&&(TRUE == CFM_OM_GetNextMep(md_index, ma_index,0, 0, CFM_OM_MD_MA_MEP_KEY, mep_p)))
    {
        if((md_index==mep_p->md_index)&&(ma_index==mep_p->ma_index))
        {
            return FALSE;
        }
    }

    /* find the next vid, nxt_vid == 0 means no other vid can be used
     */
    CFM_ENGINE_GetNextVidFromMaVidBmp(ma_p, &nxt_vid);

    if(FALSE == CFM_OM_DeleteMaVlan(md_index, ma_index, vid))
    {
        return FALSE;
    }

    /* delete all mip on this ma and check the new mip
     */
    if(ma_p->primary_vid == vid )
    {
        UI32_T lport=0;
        CFM_OM_MD_T *tmp_md_p=NULL;
        CFM_OM_MA_T *tmp_ma_p=NULL;
        BOOL_T has_high_level=FALSE;

        if(TRUE ==CFM_OM_GetFirstHighLevelMdMaBylevelVid(md_p->level, ma_p->primary_vid, &tmp_md_p, &tmp_ma_p))
        {
            has_high_level=TRUE;
        }

        while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
        {
            if(FALSE == CFM_OM_DeleteMip(md_index, ma_index, lport, CFM_OM_LPORT_MD_MA_KEY))
            {
                return FALSE;
            }

            /* try to switch the primary id to next available vid in vid list
             */
            if (nxt_vid != 0)
            {
                CFM_ENGINE_CreateMIPByMa(md_p->level, nxt_vid, lport);
            }
            else if(TRUE == has_high_level)
            {
                if(tmp_ma_p->primary_vid!=0)
                CFM_ENGINE_CreateMIPByMa(tmp_md_p->level, tmp_ma_p->primary_vid, lport);
            }
        }

        ma_p->primary_vid = nxt_vid;
    }

    return TRUE;
}/* End of CFM_ENGINE_DeleteMAVlan*/

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
BOOL_T CFM_ENGINE_SetMaName(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI8_T *name_ap,
                            UI32_T name_length)
{
    CFM_OM_MA_T  *ma_p =NULL;
    CFM_OM_MD_T  *md_p =NULL;

    if(FALSE == CFM_OM_GetMdMaByMdMaIndex( md_index, ma_index, &md_p, &ma_p))
    {
        return FALSE;
    }

    /*md =0, ma name share the CFM_TYPE_MAID_NAME_LENGTH-3 length*/
    if((md_p->name_length!=0)&&((md_p->name_length + name_length )>(CFM_TYPE_MAID_NAME_LENGTH-4)))
    {
        return FALSE;
    }
    /*md !=0 , md and ma name share the CFM_TYPE_MAID_NAME_LENGTH-3 length*/
    if((md_p->name_length==0)&&(name_length >(CFM_TYPE_MAID_NAME_LENGTH-3)))
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_SetMaName(md_p, ma_p, name_ap, name_length))
    {
        return FALSE;
    }

    return TRUE;
}/*End of CFM_ENGINE_SetMaName*/

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
    CFM_TYPE_MP_Direction_T direction)
{
    CFM_OM_MD_T             *md_p=NULL;
    CFM_OM_MA_T             *ma_p=NULL;
    CFM_OM_MEP_T            *mep_p=&mep_tmp1_g;
    CFM_TYPE_CfmStatus_T    old_gstatus, old_pstatus;
    BOOL_T                  is_mep_enable = FALSE, is_ccm_tx = FALSE;

    if (FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_ap, md_name_len, ma_name_ap, ma_name_len, &md_p, &ma_p))
    {
        return FALSE;
    }

    /*the ma without vlan can't have mep*/
    if (ma_p->primary_vid == 0)
    {
        return FALSE;
    }

    if (lport != 0)
    {
        UI32_T vlan_ifindex=0;

        VLAN_OM_ConvertToIfindex(ma_p->primary_vid, &vlan_ifindex);
        /* check whether the port is vlan's member */
        if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, lport))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI,"MEP: port %ld does not belong to vlan %ld",
                    (long)lport, (long)ma_p->primary_vid );
            return FALSE;
        }
    }

    /*check already exist this mep*/
    if (TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        if (mep_p->direction != direction||mep_p->lport != lport)
        {
            return FALSE;
        }
        return TRUE;
    }

    if (TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        if (mep_p->direction != direction)
        {
            return FALSE;
        }
        /* only modify the local mep id
         */
        return CFM_ENGINE_LocalReplceLmepId(mep_p, mep_id);
    }

    /*check this is remote mep id*/
    {
        CFM_OM_REMOTE_MEP_T remote_mep;

        if(TRUE == CFM_OM_GetRemoteMep(md_p->index, ma_p->index, mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
        {
            return FALSE;
        }
    }

    if (FALSE == CFM_OM_AddNewMep(md_p, ma_p, mep_id, lport, direction))
    {
        if (TRUE == CFM_ENGINE_IsExistSameOrHigherLevelMp(md_p, ma_p, CFM_TYPE_MP_DIRECTION_UP_DOWN, mep_id))
        {
            mep_p->primary_vid=ma_p->primary_vid;
            mep_p->md_p=md_p;
            mep_p->identifier=mep_id;
            memset(mep_p->mac_addr_a, 0, SYS_ADPT_MAC_ADDR_LEN);
            mep_p->md_ar_idx=md_p->ar_idx;
            mep_p->ma_ar_idx=ma_p->ar_idx;
            mep_p->lport=lport;
            CFM_OM_AddError(SYS_TIME_GetSystemTicksBy10ms(), CFM_TYPE_CONFIG_ERROR_OVERLAPPED_LEVELS,  mep_p, NULL);
        }
        return FALSE;
    }

    if (FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        return FALSE;
    }

#if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE)
    if (CFM_TYPE_MP_DIRECTION_UP==direction)
    {
        SWCTRL_GetCpuMac( mep_p->mac_addr_a);
    }
    else
#endif /* #if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE) */
    {
        SWCTRL_GetPortMac(lport, mep_p->mac_addr_a);
    }

    /* operation from MIB may have invalid lport (eg. create the MEP 1st time)
     */
    if (lport != 0)
    {
        if (  (FALSE == CFM_OM_GetCFMGlobalStatus(&old_gstatus))
            ||(FALSE == CFM_OM_GetCFMPortStatus(mep_p->lport, &old_pstatus))
           )
        {
            /* should not occur... do rollback if it happens...
             */
            CFM_OM_DeleteMep(md_p->index, ma_p->index, mep_id, 0, CFM_OM_MD_MA_MEP_KEY);
            return FALSE;
        }

        if (  (CFM_TYPE_CFM_STATUS_ENABLE == old_pstatus)
            &&(CFM_TYPE_CFM_STATUS_ENABLE == old_gstatus)
           )
        {
            is_mep_enable = TRUE;

            if (  (CFM_TYPE_CCM_STATUS_ENABLE == mep_p->cci_status)
                &&(CFM_TYPE_CCM_STATUS_ENABLE == mep_p->ma_p->ccm_status)
               )
            {
                is_ccm_tx = TRUE;
            }
        }
    } /* if (lport != 0) */

    {
        CFM_Timer_CallBackPara_T para;;
        CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);

        mep_p->cci_while_timer_idx=CFM_TIMER_CreateTimer(
                                     CFM_ENGINE_CcmWait_Callback,
                                     &para,
                                     CFM_ENGINE_CCMTime(ma_p->ccm_interval),
                                     CFM_TIMER_CYCLIC);
    }

    /*start the timer to send ccm*/
    if (TRUE == is_ccm_tx)
    {
        CFM_TIMER_StartTimer(mep_p->cci_while_timer_idx);
    }

    mep_p->cci_status = ma_p->ccm_status;

    if (CFM_TYPE_CROSS_CHECK_STATUS_ENABLE == ma_p->cross_check_status)
    {
        CFM_ENGINE_LocalGetCurRMepCcmLostCntInOneMa(
            md_p->index, ma_p->index, mep_p);

        CFM_ENGINE_LocalGetCurRMepMacStatusAndRdiCntInOneMa(
            md_p->index, ma_p->index, mep_p);

        CFM_ENGINE_UPD_ERR_MAC_STATUS(mep_p);
        CFM_ENGINE_UPD_SOME_RDI_DEF(mep_p);
        CFM_ENGINE_UPD_SOME_RMEP_CCM_DEF(mep_p);
        CFM_ENGINE_UpdateMepHighestDefect(mep_p);
    }

    CFM_OM_StoreMep(mep_p);

    if (lport != 0)
    {
        /* 1. delet the mip under the level which mep will create on.
           2. check lower leve has down mep or not, if it don't has down mep on a port, CFMLeak error
         */
        CFM_TYPE_MdLevel_T  level = 0, checked_level= md_p->level;
        CFM_OM_MEP_T        *temp_mep_p=&mep_tmp2_g;
        BOOL_T              has_lower_level_mep=FALSE;

        for (level=checked_level; level>=CFM_TYPE_MD_LEVEL_0; level--)
        {
            if (FALSE== CFM_OM_GetMdMaByLevelVid(level, ma_p->primary_vid, &md_p, &ma_p))
            {
                continue;
            }

            CFM_OM_DeleteMip(md_p->index, ma_p->index, lport, CFM_OM_LPORT_MD_MA_KEY);

            if (FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, temp_mep_p))
            {
                if (FALSE ==has_lower_level_mep)
                {
                    has_lower_level_mep=TRUE;

                    CFM_OM_AddError(SYS_TIME_GetSystemTicksBy10ms(), CFM_TYPE_CONFIG_ERROR_LEAK,  mep_p, NULL);
                }
            }
        }

        /* create the mip
         */
        CFM_ENGINE_CreateMIPByMep(mep_p->md_p->level, mep_p->ma_p->primary_vid, lport);
    } /* if (lport != 0) */

    return TRUE;
}/* End of CFM_ENGINE_CreateMEP*/

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
    UI32_T  ma_name_len)
{
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    UI32_T          key_type = (mep_id!=0?CFM_OM_MD_MA_MEP_KEY:CFM_OM_LPORT_MD_MA_KEY);

    if (FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_ap, md_name_len, ma_name_ap, ma_name_len, &md_p, &ma_p))
    {
        return FALSE;
    }

    if (  (FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, mep_id, lport, key_type, mep_p))
        ||(lport != mep_p->lport)
       )
    {
        return FALSE;
    }

    /*delet the mep*/
    if (FALSE == CFM_OM_DeleteMep(md_p->index, ma_p->index, mep_id, lport, key_type))
    {
        return FALSE;
    }

     /*20.5*/
     CFM_ENGINE_LocalFreeMepTimer(mep_p);

    {
        CFM_OM_MEP_T *higher_level_mep_p=&mep_tmp2_g;

        if (TRUE == CFM_ENGINE_IsExistHigherLevelMp(lport,
                                                   md_p->level,
                                                   ma_p->primary_vid,
                                                   CFM_TYPE_MP_DIRECTION_DOWN,
                                                   FALSE,
                                                   higher_level_mep_p,
                                                   NULL))
        {
            CFM_OM_AddError(SYS_TIME_GetSystemTicksBy10ms(), CFM_TYPE_CONFIG_ERROR_LEAK,  higher_level_mep_p, NULL);
        }
    }

    /*process the mip creation
     */

    /*check the over level mip should exist or not*/
    CFM_ENGINE_CheckOverLevelMipShouldExist(mep_p->md_p->level, mep_p->primary_vid, mep_p->lport);

    /*create mip, check this md ma shall has mip or not after remove mep*/
    if (ma_p->primary_vid!=0)
        CFM_ENGINE_CreateMIPByMa(md_p->level, ma_p->primary_vid, lport);

    return TRUE;
}/* End of CFM_ENGINE_DeleteMEP*/

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
                        CFM_OM_MA_T *ma_p)
{
    UI32_T lport=0;

    while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort( &lport))
    {
        CFM_OM_DeleteMip(md_p->index, ma_p->index, lport, CFM_OM_LPORT_MD_MA_KEY);
    }
    return;
}/*End of CFM_ENGINE_DeleteMipByMa*/

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
void CFM_ENGINE_CreateMIPByMa(
                            CFM_TYPE_MdLevel_T checked_level,
                            UI16_T checked_vid,
                            UI32_T checked_lport)
{
    CFM_OM_MD_T *md_p=NULL, *check_md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL, *check_ma_p=NULL;
    CFM_OM_MEP_T *mep_p=&mep_tmp4_g;
    CFM_OM_MIP_T mip;
    CFM_TYPE_MdLevel_T  next_lower_level=CFM_TYPE_MD_LEVEL_0, next_higher_level=CFM_TYPE_MD_LEVEL_0;
    CFM_TYPE_MhfCreation_T mhf_creation=CFM_TYPE_MHF_CREATION_NONE;
    BOOL_T lower_level_has_mep=FALSE, has_lower_level_md=FALSE, has_higher_level_mip=FALSE;
    UI8_T mac_a[SYS_ADPT_MAC_ADDR_LEN];

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "cl-%d, cv-%d, cp-%ld",
                checked_level, checked_vid, (long)checked_lport);

    /*get the md and ma*/
    if(FALSE== CFM_OM_GetMdMaByLevelVid(checked_level, checked_vid, &md_p, &ma_p))
    {
        return;
    }

    /*check if this md ma lport already have mep on it.*/
    if(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, checked_lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        return;
    }

    /*check the level already has the mip or not  */
    if(TRUE == CFM_OM_GetMip(md_p->index, ma_p->index, checked_lport, CFM_OM_LPORT_MD_MA_KEY, &mip))
    {
        return;
    }

    /* check uper level has mep or not
     */
    for(next_higher_level=checked_level+1; next_higher_level<=CFM_TYPE_MD_LEVEL_7; next_higher_level++)
    {
        if(FALSE== CFM_OM_GetMdMaByLevelVid(next_higher_level, checked_vid, &check_md_p, &check_ma_p))
        {
            continue;
        }

        if(TRUE == CFM_OM_GetMep(check_md_p->index, check_ma_p->index, 0, checked_lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
        {
            return;
        }
    }

    /* check has lower level or mep
     */
    for(next_lower_level=checked_level-1; (next_lower_level>=CFM_TYPE_MD_LEVEL_0)&&(FALSE == has_lower_level_md); next_lower_level--)
    {
        if(FALSE== CFM_OM_GetMdMaByLevelVid(next_lower_level, checked_vid, &check_md_p, &check_ma_p))
        {
            continue;
        }

        if(TRUE == CFM_OM_GetMip(md_p->index, ma_p->index, checked_lport, CFM_OM_LPORT_MD_MA_KEY, &mip))
        {
            return;
        }

        has_lower_level_md=TRUE;

        if(TRUE == CFM_OM_GetMep(check_md_p->index, check_ma_p->index, 0, checked_lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
        {
            lower_level_has_mep=TRUE;
            break;
        }
    }

    /* crate mip, just check this md level only
     */
    mhf_creation= (CFM_TYPE_MHF_CREATION_DEFER == ma_p->mhf_creation ? md_p->mhf_creation : ma_p->mhf_creation);

    if( (CFM_TYPE_MHF_CREATION_EXPLICIT == mhf_creation) && (FALSE == lower_level_has_mep) )
    {
        return;
    }
    else if(CFM_TYPE_MHF_CREATION_NONE == mhf_creation)
    {
        return;
    }
    else if((CFM_TYPE_MHF_CREATION_DEFAULT== mhf_creation)
     &&(TRUE == has_lower_level_md)
     &&(FALSE == lower_level_has_mep))
    {
        return;
    }

    SWCTRL_GetPortMac(checked_lport, mac_a);

    /* delete higher level mip, because only one lower level mip is created for this port
     */
    for(next_higher_level=checked_level+1;next_higher_level<=CFM_TYPE_MD_LEVEL_7;next_higher_level++)
    {
        if(FALSE== CFM_OM_GetMdMaByLevelVid(next_higher_level, checked_vid, &check_md_p, &check_ma_p))
        {
            continue;
        }

        /*check the level already has the mip or not  */
        if(TRUE == CFM_OM_GetMip(check_md_p->index, check_ma_p->index, checked_lport, CFM_OM_LPORT_MD_MA_KEY, &mip))
        {
            CFM_OM_DeleteMip(check_md_p->index, check_ma_p->index, checked_lport, CFM_OM_LPORT_MD_MA_KEY);
            has_higher_level_mip=TRUE;
        }
    }

    {
        UI32_T vlan_ifindex=0;

        VLAN_OM_ConvertToIfindex(ma_p->primary_vid, &vlan_ifindex);

        /* check whether the port is vlan's member */
        if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, checked_lport))
        {
            /* MIP not created due to VLAN member checking failed,
             * return here to avoid counting this into ERROR_EXCESSIVE_LEVELS.
             */
            return;
        }
    }

    /*create this level vid's mip*/
    if(FALSE== CFM_OM_AddNewMip(md_p, ma_p, checked_lport, mac_a))
    {
        mep_p->primary_vid= ma_p->primary_vid;
        mep_p->md_p= md_p;
        mep_p->identifier=0;
        mep_p->lport = checked_lport;
        memcpy(mep_p->mac_addr_a, mac_a, SYS_ADPT_MAC_ADDR_LEN);
        mep_p->md_ar_idx=md_p->ar_idx;
        mep_p->ma_ar_idx=ma_p->ar_idx;
        CFM_OM_AddError(SYS_TIME_GetSystemTicksBy10ms(), CFM_TYPE_CONFIG_ERROR_EXCESSIVE_LEVELS,  mep_p, NULL);
    }
}/*End of CFM_ENGINE_CreateMIPByMa*/

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
void CFM_ENGINE_CreateMIPByMep(
                                CFM_TYPE_MdLevel_T checked_level,
                                UI16_T checked_vid,
                                UI32_T checked_lport)
{
    CFM_OM_MD_T  *md_p =NULL;
    CFM_OM_MA_T  *ma_p =NULL;
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;
    CFM_OM_MIP_T mip;
    CFM_TYPE_MdLevel_T next_higher_level=CFM_TYPE_MD_LEVEL_0;
    CFM_TYPE_MhfCreation_T mhf_creation=CFM_TYPE_MHF_CREATION_NONE;
    BOOL_T has_next_higher_level=FALSE;
    UI8_T mac_a[SYS_ADPT_MAC_ADDR_LEN];

    /*check uper next_higher_level has mp or not
     */
    for(next_higher_level=checked_level+1; (next_higher_level<=CFM_TYPE_MD_LEVEL_7)&&(FALSE ==has_next_higher_level); next_higher_level++)
    {
        if(FALSE== CFM_OM_GetMdMaByLevelVid(next_higher_level, checked_vid, &md_p, &ma_p))
        {
            continue;
        }

        if(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, checked_lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
        {
            return;
        }

        if(TRUE == CFM_OM_GetMip(md_p->index, ma_p->index, checked_lport, CFM_OM_LPORT_MD_MA_KEY, &mip))
        {
            return;
        }

        has_next_higher_level=TRUE;
    }

    if((NULL == md_p)||(NULL == ma_p))
    {
        return;
    }

    /*crate mip.
     * upper layer has no mp exist
     */
    mhf_creation= (CFM_TYPE_MHF_CREATION_DEFER == ma_p->mhf_creation ? md_p->mhf_creation : ma_p->mhf_creation);

    if(CFM_TYPE_MHF_CREATION_NONE== mhf_creation)
    {
        return;
    }

    if(TRUE == CFM_OM_GetMip(md_p->index, ma_p->index, checked_lport, CFM_OM_LPORT_MD_MA_KEY, &mip))
    {
        return;
    }

    if(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, checked_lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        return;
    }

    {
        UI32_T vlan_ifindex=0;

        VLAN_OM_ConvertToIfindex(ma_p->primary_vid, &vlan_ifindex);

        /* check whether the port is vlan's member */
        if(FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, checked_lport))
        {
            /* MIP not created due to VLAN member checking failed,
             * return TRUE here to avoid counting this into ERROR_EXCESSIVE_LEVELS.
             */
            return;
        }
    }

    SWCTRL_GetPortMac(checked_lport, mac_a);

    if(FALSE== CFM_OM_AddNewMip(md_p, ma_p, checked_lport, mac_a))
    {
        mep_p->primary_vid= ma_p->primary_vid;
        mep_p->md_p= md_p;
        mep_p->identifier=0;
        mep_p->lport = checked_lport;
        memcpy(mep_p->mac_addr_a, mac_a, SYS_ADPT_MAC_ADDR_LEN);
        mep_p->md_ar_idx=md_p->ar_idx;
        mep_p->ma_ar_idx=ma_p->ar_idx;
        CFM_OM_AddError(SYS_TIME_GetSystemTicksBy10ms(), CFM_TYPE_CONFIG_ERROR_EXCESSIVE_LEVELS,  mep_p, NULL);
    }
}/*End of CFM_ENGINE_CreateMIPByMep*/

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
void CFM_ENGINE_CheckOverLevelMipShouldExist(
                                            CFM_TYPE_MdLevel_T checked_level,
                                            UI16_T checked_vid,
                                            UI32_T checked_lport)
{
    CFM_OM_MD_T         *md_p =NULL;
    CFM_OM_MA_T         *ma_p =NULL;
    CFM_OM_MEP_T        *mep_p=&mep_tmp3_g;
    CFM_OM_MIP_T        mip;
    CFM_TYPE_MdLevel_T  next_higher_level=CFM_TYPE_MD_LEVEL_0;
    BOOL_T              has_next_higher_level=FALSE;

    /*check uper level has mep or not*/
    for(next_higher_level=checked_level+1; (next_higher_level<=CFM_TYPE_MD_LEVEL_7)&&(FALSE == has_next_higher_level); next_higher_level++)
    {
        if(FALSE== CFM_OM_GetMdMaByLevelVid(next_higher_level, checked_vid, &md_p, &ma_p))
        {
            continue;
        }

        if(CFM_TYPE_MHF_CREATION_EXPLICIT !=ma_p->mhf_creation)
        {
            continue;
        }

        /*just only check the level vid has the mep or not, if has then don't delet mip, because these mep need it*/
        if(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, checked_lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
        {
            return;
        }

        if(TRUE == CFM_OM_GetMip(md_p->index, ma_p->index, checked_lport, CFM_OM_LPORT_MD_MA_KEY, &mip))
        {
            CFM_OM_DeleteMip( md_p->index, ma_p->index, checked_lport, CFM_OM_LPORT_MD_MA_KEY);
            return;
        }

        has_next_higher_level= TRUE;
    }

    return;
}/*End of CFM_ENGINE_CheckOverLevelMipShouldExist*/

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
void CFM_ENGINE_ProcessTimer()
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "");
    CFM_TIMER_ProcessTimer();
    return;
}/* End of CFM_ENGINE_ProcessTimer*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCFMGlobalStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the global CFM status
 * INPUT    : status    - the cfm status
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetCFMGlobalStatus(
    CFM_TYPE_CfmStatus_T    status)
{
    CFM_TYPE_CfmStatus_T    old_gstatus, old_pstatus;
    BOOL_T                  is_mep_enable = FALSE, is_ccm_tx = FALSE;

    if (FALSE == CFM_OM_GetCFMGlobalStatus(&old_gstatus))
    {
        return FALSE;
    }

    if (old_gstatus == status)
    {
        return TRUE;
    }

    if (FALSE == CFM_OM_SetCFMGlobalStatus(status))
    {
        return FALSE;
    }

#if 0 /*if the chip can trap the cfm packet already, below needn't executed*/
    {/*set port cfm status to default*/
        UI32_T port_ifindex=0;

        while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&port_ifindex))
        {
            UI32_T unit=0,port=0,trunk_id=0;

            if(TRUE == SWCTRL_LogicalPortToUserPort(port_ifindex, &unit, &port, &trunk_id))
            {
                SWDRV_SetDot1AgTrap(unit, port, CFM_TYPE_CFM_STATUS_ENABLE ==status?TRUE:FALSE);
            }
        }
    }
#endif

    /* refer to CFM_ENGINE_SetMepCciStatus
     */
    {
        CFM_OM_MEP_T    *mep_p = &mep_tmp1_g;
        UI32_T          nxt_md_index=0, nxt_ma_index=0, nxt_mep_id=0;

        while (TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
        {
            nxt_md_index = mep_p->md_index;
            nxt_ma_index = mep_p->ma_index;
            nxt_mep_id   = mep_p->identifier;

            if (FALSE == CFM_OM_GetCFMPortStatus(mep_p->lport, &old_pstatus))
            {
                /* should not occur...
                 */
                continue;
            }

            is_ccm_tx = is_mep_enable = FALSE;
            if (  (CFM_TYPE_CFM_STATUS_ENABLE == old_pstatus)
                &&(CFM_TYPE_CFM_STATUS_ENABLE == status)
               )
            {
                is_mep_enable = TRUE;

                if (  (CFM_TYPE_CCM_STATUS_ENABLE == mep_p->cci_status)
                    &&(CFM_TYPE_CCM_STATUS_ENABLE == mep_p->ma_p->ccm_status)
                   )
                {
                    is_ccm_tx = TRUE;
                }
            }

            if (TRUE == is_ccm_tx)
            {
                if (TRUE == CFM_TIMER_StartTimer(mep_p->cci_while_timer_idx))
                {
                    CFM_TIMER_UpdateTimer(mep_p->cci_while_timer_idx,
                                        CFM_ENGINE_CCMTime(mep_p->ma_p->ccm_interval),
                                        CFM_ENGINE_CcmWait_Callback);
                }
            }
            else
            {
                CFM_TIMER_StopTimer(mep_p->cci_while_timer_idx);
            }
        }
    }

    /* the timer event will not be processed when global disabled,
     * so the background operation will not be stopped correctly.
     */
    if (status == CFM_TYPE_CFM_STATUS_DISABLE)
    {
        CFM_ENGINE_LocalAbortAllBgOperations();
    }

    return TRUE;
}/*End of CFM_ENGINE_SetCFMGlobalStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCFMPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the CFM port status
 *            lport   - lport to set
 * INPUT    : status  - the cfm status
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetCFMPortStatus(
    UI32_T                  lport,
    CFM_TYPE_CfmStatus_T    status)
{
    CFM_TYPE_CfmStatus_T    old_gstatus, old_pstatus;

    if (  (FALSE == CFM_OM_GetCFMPortStatus(lport, &old_pstatus))
        ||(FALSE == CFM_OM_GetCFMGlobalStatus(&old_gstatus))
       )
    {
        return FALSE;
    }

    if (status == old_pstatus)
    {
        return TRUE;
    }

    if (FALSE == CFM_OM_SetCFMPortStatus(lport, status))
    {
        return FALSE;
    }

    /* refer to CFM_ENGINE_SetMepCciStatus
     */
    {
        CFM_OM_MEP_T    *mep_p = &mep_tmp1_g;
        UI32_T          nxt_md_index=0, nxt_ma_index=0, nxt_mep_id=0;
        BOOL_T          is_mep_enable = FALSE, is_ccm_tx = FALSE;

        if (  (CFM_TYPE_CFM_STATUS_ENABLE == old_gstatus)
            &&(CFM_TYPE_CFM_STATUS_ENABLE == status)
           )
        {
            is_mep_enable = TRUE;
        }

        while (TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, nxt_mep_id, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
        {
            nxt_md_index = mep_p->md_index;
            nxt_ma_index = mep_p->ma_index;
            nxt_mep_id = mep_p->identifier;

            if (mep_p->lport != lport)
                break;

            is_ccm_tx = FALSE;
            if (TRUE == is_mep_enable)
            {
                if (  (CFM_TYPE_CCM_STATUS_ENABLE == mep_p->cci_status)
                    &&(CFM_TYPE_CCM_STATUS_ENABLE == mep_p->ma_p->ccm_status)
                   )
                {
                    is_ccm_tx = TRUE;
                }
            }

            if (TRUE == is_ccm_tx)
            {
                if (TRUE == CFM_TIMER_StartTimer(mep_p->cci_while_timer_idx))
                {
                    CFM_TIMER_UpdateTimer(mep_p->cci_while_timer_idx,
                                        CFM_ENGINE_CCMTime(mep_p->ma_p->ccm_interval),
                                        CFM_ENGINE_CcmWait_Callback);
                }
            }
            else
            {
                CFM_TIMER_StopTimer(mep_p->cci_while_timer_idx);
            }
        } /* while (TRUE == CFM_OM_GetNextMep(...) */
    }

    return TRUE;
}/*End of CFM_ENGINE_SetCFMPortStatus*/

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
    CFM_TYPE_CcmInterval_T  interval)
{
    CFM_OM_MD_T             *md_p=NULL;
    CFM_OM_MA_T             *ma_p=NULL;
    CFM_OM_MEP_T            *mep_p=&mep_tmp1_g;
    CFM_OM_REMOTE_MEP_T     remote_mep;
    UI32_T                  nxt_mep_id=0;
    CFM_TYPE_CfmStatus_T    old_gstatus, old_pstatus;
    BOOL_T                  is_mep_enable, is_ccm_tx;

    if (FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_ap, md_name_len, ma_name_ap, ma_name_len, &md_p, &ma_p))
    {
        return FALSE;
    }

    if (FALSE == CFM_OM_SetMaCCInterval(md_p->index, ma_p->index, interval))
    {
        return FALSE;
    }

    if (FALSE == CFM_OM_GetCFMGlobalStatus(&old_gstatus))
    {
        /* should not occur...
         */
        return FALSE;
    }

    /*update all mep timer to send CCM*/
    while (TRUE == CFM_OM_GetNextMep(md_p->index, ma_p->index, nxt_mep_id,0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        if ((mep_p->md_index!=md_p->index)||(mep_p->ma_index!=ma_p->index))
        {
            break;
        }

        if (FALSE == CFM_OM_GetCFMPortStatus(mep_p->lport, &old_pstatus))
        {
            return FALSE;
        }

        is_ccm_tx = is_mep_enable = FALSE;
        if (  (CFM_TYPE_CFM_STATUS_ENABLE == old_pstatus)
            &&(CFM_TYPE_CFM_STATUS_ENABLE == old_gstatus)
           )
        {
            is_mep_enable = TRUE;

            if (  (CFM_TYPE_CCM_STATUS_ENABLE == mep_p->cci_status)
                &&(CFM_TYPE_CCM_STATUS_ENABLE == mep_p->ma_p->ccm_status)
               )
            {
                is_ccm_tx = TRUE;
            }
        }

        if (TRUE == is_ccm_tx)
        {
            CFM_TIMER_UpdateTimer(mep_p->cci_while_timer_idx, CFM_ENGINE_CCMTime(interval), CFM_ENGINE_CcmWait_Callback);
        }

        nxt_mep_id=mep_p->identifier;
    }

    nxt_mep_id=0;
    while (TRUE == CFM_OM_GetNextRemoteMep(md_p->index, ma_p->index, nxt_mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        if( (remote_mep.md_index!=md_p->index)||(remote_mep.ma_index!=ma_p->index))
        {
            break;
        }

        if (CFM_TYPE_REMOTE_MEP_STATE_START != remote_mep.machine_state)
        {
            CFM_TIMER_UpdateTimer(remote_mep.rmep_while_timer_idx,(CFM_ENGINE_CCMTime(interval)*35/10), CFM_ENGINE_RemoteMepOk_Callback );
        }

        nxt_mep_id=remote_mep.identifier;
    }

    return TRUE;
}/* End of CFM_ENGINE_SetCcmInterval*/

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
    CFM_TYPE_CcmStatus_T    status)
{
    CFM_OM_MD_T             *md_p=NULL;
    CFM_OM_MA_T             *ma_p=NULL;
    CFM_OM_MEP_T            *mep_p=&mep_tmp1_g;
    UI32_T                  nxt_mep_id=0;
    CFM_TYPE_CfmStatus_T    old_gstatus, old_pstatus;
    BOOL_T                  is_mep_enable = FALSE, is_ccm_tx = FALSE;

    if (FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_ap, md_name_len, ma_name_ap, ma_name_len, &md_p, &ma_p))
    {
        return FALSE;
    }

    if (ma_p->ccm_status == status)
    {
        return TRUE;
    }

    if (FALSE == CFM_OM_GetCFMGlobalStatus(&old_gstatus))
    {
        return FALSE;
    }

    ma_p->ccm_status = status;

    while (TRUE == CFM_OM_GetNextMep(md_p->index, ma_p->index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        if ((md_p->index != mep_p->md_index) || (ma_p->index != mep_p->ma_index))
        {
            return TRUE;
        }

        mep_p->cci_status = status;

        if (FALSE == CFM_OM_GetCFMPortStatus(mep_p->lport, &old_pstatus))
        {
            /* should not occur...
             */
            continue;
        }

        /* refer to CFM_ENGINE_SetMepCciStatus
         */
        is_ccm_tx = is_mep_enable = FALSE;
        if (  (CFM_TYPE_CFM_STATUS_ENABLE == old_gstatus)
            &&(CFM_TYPE_CFM_STATUS_ENABLE == old_pstatus)
           )
        {
            is_mep_enable = TRUE;

            if (CFM_TYPE_CCM_STATUS_ENABLE == status)
                is_ccm_tx = TRUE;
        }

        if (TRUE == is_ccm_tx)
        {
            if (TRUE == CFM_TIMER_StartTimer(mep_p->cci_while_timer_idx))
            {
                CFM_TIMER_UpdateTimer(mep_p->cci_while_timer_idx,
                                    CFM_ENGINE_CCMTime(mep_p->ma_p->ccm_interval),
                                    CFM_ENGINE_CcmWait_Callback);
            }
        }
        else
        {
            CFM_TIMER_StopTimer(mep_p->cci_while_timer_idx);
        }

        CFM_OM_StoreMep(mep_p);

        nxt_mep_id=mep_p->identifier;
    }

    return TRUE;
}/* End of CFM_ENGINE_SetCcmStatus*/

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
 * NOTE     :if the mip already create this mip, then this mip created remote mep will be deleted
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_CreateRemoteMep(
                                UI32_T md_index,
                                UI8_T *ma_name_ap,
                                UI32_T name_len,
                                UI32_T mep_id)
{
    CFM_OM_REMOTE_MEP_T remote_mep;
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    UI32_T start_delay=0, archive_hold_time=0;

    if (FALSE == CFM_OM_GetMdMaByMdIndxMaName(md_index, ma_name_ap, name_len, &md_p, &ma_p))
    {
        return FALSE;
    }

    /*check if already exit or mip ever created*/
    if(TRUE == CFM_OM_GetRemoteMep( md_index, ma_p->index, mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        return TRUE;
    }

    {
        CFM_OM_MEP_T *mep_p=&mep_tmp1_g;

        /*if the configure remote mep's id is the same as the local mep*/
        if(TRUE == CFM_OM_GetMep( md_index, ma_p->index, mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
        {
            return FALSE;
        }
    }

    if(FALSE == CFM_OM_AddNewRemoteMep(md_p, ma_p, mep_id))
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_GetRemoteMep(md_index, ma_p->index, mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "can add but can't get remote mep=%ld", (long)remote_mep.identifier);
        return FALSE;
    }

    {
        CFM_Timer_CallBackPara_T para;;
        CFM_TIMER_AssignTimerParameter(&para, remote_mep.md_index, remote_mep.ma_index, remote_mep.identifier, 0, 0);

        /*create the timer */
        CFM_OM_GetCrossCheckStartDelay(&start_delay);

        remote_mep.rmep_while_timer_idx=CFM_TIMER_CreateTimer(
                                       CFM_ENGINE_RemoteStartDelay_Callback,
                                       &para,
                                       start_delay,
                                       CFM_TIMER_ONE_TIME);

        CFM_TIMER_StartTimer(remote_mep.rmep_while_timer_idx);

        CFM_OM_GetArchiveHoldTime(md_index, &archive_hold_time);

        remote_mep.archive_hold_timer_idx=CFM_TIMER_CreateTimer(
                                              CFM_ENGINE_ClearRemoteMepByRemoteMep_Callback,
                                              &para,
                                              archive_hold_time,
                                              CFM_TIMER_CYCLIC);

        CFM_TIMER_StartTimer(remote_mep.archive_hold_timer_idx);
    }

    if(FALSE == CFM_OM_SetRemoteMep(&remote_mep))
    {
        CFM_TIMER_DestroyTimer(&remote_mep.rmep_while_timer_idx);
        CFM_TIMER_DestroyTimer(&remote_mep.archive_hold_timer_idx);
        return FALSE;
    }
    return TRUE;
}/*End of CFM_ENGINE_CreateRemoteMep*/

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
BOOL_T CFM_ENGINE_DeleteRemoteMepByMdIndexMaName(
                                                UI32_T md_index,
                                                UI8_T *ma_name_ap,
                                                UI32_T name_len,
                                                UI32_T mep_id)
{
    CFM_OM_MD_T         *md_p=NULL;
    CFM_OM_MA_T         *ma_p=NULL;
    CFM_OM_REMOTE_MEP_T remote_mep;

    if (FALSE == CFM_OM_GetMdMaByMdIndxMaName(md_index, ma_name_ap, name_len, &md_p, &ma_p))
    {
        return FALSE;
    }

    if (FALSE == CFM_OM_GetRemoteMep( md_p->index, ma_p->index, mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        return TRUE;
    }

    if (FALSE == CFM_OM_DeleteRemoteMep(md_index, ma_p->index, mep_id, CFM_OM_MD_MA_MEP_KEY))
    {
        return FALSE;
    }

    /*free the memory before delete the record*/
    CFM_TIMER_StopTimer(remote_mep.rmep_while_timer_idx);
    CFM_TIMER_FreeTimer(&remote_mep.rmep_while_timer_idx);
    CFM_TIMER_StopTimer(remote_mep.archive_hold_timer_idx);
    CFM_TIMER_FreeTimer(&remote_mep.archive_hold_timer_idx);

    CFM_ENGINE_ResetRcvdMepStateOfRemoteMep(&remote_mep);

    if (  (FALSE == remote_mep.mep_up)
        &&(remote_mep.ma_p->remote_mep_down_counter > 0)
       )
        remote_mep.ma_p->remote_mep_down_counter--;

    return TRUE;
}/*End of CFM_ENGINE_DeleteRemoteMepByMdIndexMaName*/

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
BOOL_T CFM_ENGINE_DeleteRemoteMepByIndex(
                                        UI32_T md_index,
                                        UI32_T ma_index,
                                        UI32_T remote_mep_id)
{
    CFM_OM_REMOTE_MEP_T remote_mep;

    if (FALSE == CFM_OM_GetRemoteMep( md_index, ma_index, remote_mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        return TRUE;
    }

    if (FALSE == CFM_OM_DeleteRemoteMep(md_index, ma_index, remote_mep_id, CFM_OM_MD_MA_MEP_KEY))
    {
        return FALSE;
    }

    /*free the memory before delete the record*/
    CFM_TIMER_StopTimer(remote_mep.rmep_while_timer_idx);
    CFM_TIMER_FreeTimer(&remote_mep.rmep_while_timer_idx);
    CFM_TIMER_StopTimer(remote_mep.archive_hold_timer_idx);
    CFM_TIMER_FreeTimer(&remote_mep.archive_hold_timer_idx);

    CFM_OM_SetRemoteMep(&remote_mep);
    return TRUE;
}/* End of CFM_ENGINE_DeleteRemoteMepByIndex*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetCrossCheckStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the cross check status
 * INPUT    : status      - the cross check status
 *            md_name_ap  - the md name array pointer
 *            md_name_len - the md name length
 *            ma_name_ap  - the ma name array pointer
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : CFM_TYPE_CROSS_CHECK_STATUS_DISABLE,
 *            CFM_TYPE_CROSS_CHECK_STATUS_ENABLE
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetCrossCheckStatus(
    UI8_T                           *md_name_ap,
    UI32_T                          md_name_len,
    UI8_T                           *ma_name_ap,
    UI32_T                          ma_name_len,
    CFM_TYPE_CrossCheckStatus_T     status)
{
    CFM_OM_MD_T             *md_p=NULL;
    CFM_OM_MA_T             *ma_p=NULL;
    CFM_OM_REMOTE_MEP_T     remote_mep;

    if (FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_ap, md_name_len, ma_name_ap, ma_name_len, &md_p, &ma_p))
    {
        return FALSE;
    }
    else
    {
        if (ma_p->cross_check_status == status)
            return TRUE;

        if (FALSE == CFM_OM_SetCrossCheckStatus(status, ma_p))
            return FALSE;
    }

    if (status == CFM_TYPE_CROSS_CHECK_STATUS_DISABLE)
    {
        remote_mep.identifier = 0;
        while(TRUE == CFM_OM_GetNextRemoteMep(md_p->index, ma_p->index,
                        remote_mep.identifier, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
        {
            if (  (remote_mep.md_index != md_p->index)
                ||(remote_mep.ma_index != ma_p->index)
               )
            {
                break;
            }

            CFM_ENGINE_ResetRcvdMepStateOfRemoteMep(&remote_mep);

            remote_mep.rcvd_mep_id   = 0;
            remote_mep.ccm_defect    = FALSE;
            remote_mep.machine_state = CFM_TYPE_REMOTE_MEP_STATE_START;
            remote_mep.sender_chassis_id_length = 0;
            remote_mep.man_domain_length = 0;
            remote_mep.man_length = 0;

            CFM_OM_SetRemoteMep(&remote_mep);
        }
    }

    return TRUE;
}/* End of CFM_ENGINE_SetCrossCheckStatus*/

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
BOOL_T CFM_ENGINE_SetCrossCheckStartDelay(UI32_T delay)
{

    if(FALSE == CFM_OM_SetCrossCheckStartDelay(delay))
    {
        return FALSE;
    }

    return TRUE;
}/*End of CFM_ENGINE_SetCrossCheckStartDelay*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetLinkTraceCacheStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the link trace cache status
 * INPUT    : status  - the link trace cache status
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetLinkTraceCacheStatus(
                                            CFM_TYPE_LinktraceStatus_T status)
{
    if(FALSE == CFM_OM_SetLinkTraceCacheStatus(status))
    {
        return FALSE;
    }

    return TRUE;
}/*End of CFM_ENGINE_SetLinkTraceCacheStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetLinkTraceCacheSize
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the link trace cache size
 * INPUT    : size  - the link trace cache size
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetLinkTraceCacheSize(UI32_T size)
{
    CFM_TYPE_LinktraceStatus_T status=CFM_TYPE_LINKTRACE_STATUS_ENABLE;

    if(FALSE == CFM_OM_GetLinkTraceCacheStatus(&status) || (CFM_TYPE_LINKTRACE_STATUS_DISABLE == status))
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_SetLinkTraceCacheSize(size))
    {
        return FALSE;
    }

    return TRUE;
}/*End of CFM_ENGINE_SetLinkTraceCacheSize*/

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
BOOL_T CFM_ENGINE_SetLinkTraceCacheHoldTime(UI32_T hold_time)
{
    CFM_TYPE_LinktraceStatus_T status=CFM_TYPE_LINKTRACE_STATUS_ENABLE;

    if(FALSE == CFM_OM_GetLinkTraceCacheStatus(&status) || (CFM_TYPE_LINKTRACE_STATUS_DISABLE == status))
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_SetLinkTraceCacheHoldTime(hold_time))
    {
        return FALSE;
    }

    return TRUE;
}/* End of CFM_ENGINE_SetLinkTraceCacheHoldTime*/

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
BOOL_T CFM_ENGINE_ClearLinkTraceCache()
{
    CFM_OM_LTR_T    ltr;
    UI32_T          md_index=0;
    UI32_T          ma_index=0;
    UI32_T          seq_num=0;
    UI32_T          rcvd_order=0;
    UI32_T          mep_id=0;

    while(TRUE == CFM_OM_GetNextLtr(md_index, ma_index, mep_id, seq_num, rcvd_order, CFM_OM_MD_MA_MEP_SEQ_REC_KEY, &ltr))
    {
        md_index   = ltr.md_index;
        ma_index   = ltr.ma_index;
        seq_num    = ltr.seq_number;
        rcvd_order = ltr.receive_order;
        mep_id     = ltr.rcvd_mep_id;

        CFM_OM_DeleteLtr(&ltr);

        CFM_TIMER_StopTimer(ltr.hold_timer_idx);
        CFM_TIMER_FreeTimer(&ltr.hold_timer_idx);
    }

    return TRUE;
}/* End of CFM_ENGINE_ClearLinkTraceCache*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearLinkTraceReplyEntry_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the link trace reply entry
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearLinkTraceReplyEntry_Callback(
                                                    void *timer_para_p)
{
    CFM_OM_LTR_T                *ltr_p=&ltr_tmp2_g;
    CFM_Timer_CallBackPara_T    *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "time=%ld md/ma/mep/seq/rcv-%ld/%ld/%ld/%ld/%ld",
            (long)SYS_TIME_GetSystemTicksBy10ms(),
            (long)para_p->md_index, (long)para_p->ma_index, (long)para_p->mep_id,
            (long)para_p->seq_num,  (long)para_p->rcvd_order);

    if (TRUE == CFM_OM_GetLtr(
                    para_p->md_index, para_p->ma_index,
                    para_p->mep_id,   para_p->seq_num,
                    para_p->rcvd_order, CFM_OM_MD_MA_MEP_SEQ_REC_KEY, ltr_p))
    {
        CFM_OM_DeleteLtr(ltr_p);
        CFM_TIMER_FreeTimer(&ltr_p->hold_timer_idx);
    }
    else
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "record not found");
    }
    return TRUE;
}/* End of CFM_ENGINE_ClearLinkTraceReplyEntry_Callback*/

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
BOOL_T CFM_ENGINE_SetArchiveHoldTime(
                                    UI32_T md_index,
                                    UI32_T hold_time)
{
    if(FALSE ==CFM_OM_SetArchiveHoldTime(md_index, hold_time))
    {
        return FALSE;
    }

    return TRUE;
}/* End of CFM_ENGINE_SetArchiveHoldTime*/

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
BOOL_T CFM_ENGINE_SetFaultNotifyLowestPriority(
    UI32_T                          md_index,
    CFM_TYPE_FNG_LowestAlarmPri_T   priority)
{
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    UI32_T          nxt_ma_index=0, nxt_mep_id=0;

    while(TRUE == CFM_OM_GetNextMep(md_index, nxt_ma_index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        if(mep_p->md_index!=md_index)
        {
            break;
        }

        CFM_OM_SetMepLowPrDef(md_index, mep_p->ma_index, mep_p->identifier, priority);

        nxt_ma_index=mep_p->ma_index;
        nxt_mep_id=mep_p->identifier;
    }

    if(FALSE == CFM_OM_SetMdLowPrDef(md_index, priority))
    {
        return FALSE;
    }

    return TRUE;
}/*End of CFM_ENGINE_SetFaultNotifyLowestPriority*/

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
BOOL_T CFM_ENGINE_SetFaultNotifyAlarmTime(
                                        UI32_T md_index,
                                        UI32_T alarm_time)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    UI32_T nxt_ma_index=0, nxt_mep_id=0;

    while(TRUE == CFM_OM_GetNextMep(md_index, nxt_ma_index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        if(mep_p->md_index!=md_index)
        {
            break;
        }

        CFM_OM_SetMepFngAlarmTime(md_index, mep_p->ma_index, mep_p->identifier, alarm_time);

        nxt_ma_index=mep_p->ma_index;
        nxt_mep_id=mep_p->identifier;
    }

    if(FALSE == CFM_OM_SetFaultNotifyAlarmTime(md_index, alarm_time))
    {
        return FALSE;
    }

    return TRUE;
}/* End of CFM_ENGINE_SetFaultNotifyAlarmTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetFaultNotifyResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : Tis function set the fault rest time
 * INPUT    : md_index   - the md index
 *            reset_time - the fault notify rest time
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetFaultNotifyResetTime(
                                            UI32_T md_index,
                                            UI32_T  reset_time)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    UI32_T nxt_ma_index=0, nxt_mep_id=0;

    while(TRUE == CFM_OM_GetNextMep(md_index, nxt_ma_index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        if(mep_p->md_index!=md_index)
        {
            break;
        }

        CFM_OM_SetMepFngResetTime(md_index, mep_p->ma_index, mep_p->identifier, reset_time);

        nxt_ma_index=mep_p->ma_index;
        nxt_mep_id=mep_p->identifier;
    }

    if(FALSE == CFM_OM_SetFaultNotifyRestTime(md_index, reset_time))
    {
        return FALSE;
    }

    return TRUE;
}/*End of CFM_ENGINE_SetFaultNotifyResetTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearRemoteMepByRemoteMep_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  :This function Clear the remote mep by remtoe mep depend on archive hold time
 * INPUT    : *timer_para_p    - the parameter pointer
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_ClearRemoteMepByRemoteMep_Callback(
                                                                        void *timer_para_p)
{
    CFM_OM_REMOTE_MEP_T remote_mep;
    CFM_Timer_CallBackPara_T *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "");

    if(FALSE == CFM_OM_GetRemoteMep(para_p->md_index, para_p->ma_index, para_p->mep_id,
                        CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        return FALSE;
    }

    CFM_OM_ResetRemoteMepData(&remote_mep);

    CFM_OM_SetRemoteMep(&remote_mep);

    return TRUE;
}/*End of CFM_ENGINE_ClearRemoteMepByRemoteMep_Callback*/

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
BOOL_T CFM_ENGINE_ClearRemoteMepAll()
{
    CFM_OM_REMOTE_MEP_T remote_mep;
    UI32_T nxt_ma_index=0, nxt_md_index=0, nxt_mep_id=0;

   CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER,  "time=%ld", (long)SYS_TIME_GetSystemTicksBy10ms());

    while(TRUE == CFM_OM_GetNextRemoteMep(nxt_md_index, nxt_ma_index, nxt_mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        CFM_OM_ResetRemoteMepData(&remote_mep);

        nxt_md_index = remote_mep.md_index;
        nxt_ma_index=remote_mep.ma_index;
        nxt_mep_id=remote_mep.identifier;
    }

    return TRUE;
}/*End of CFM_ENGINE_ClearRemoteMepAll*/

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
BOOL_T CFM_ENGINE_ClearRemoteMepByDomain(
                                        UI8_T *md_name_ap,
                                        UI32_T md_name_len)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_REMOTE_MEP_T remote_mep;
    UI32_T nxt_ma_index=0, nxt_mep_id=0;

    if(NULL ==(md_p = CFM_OM_GetMdByName(md_name_ap, md_name_len)))
    {
        return FALSE;
    }

    while(TRUE == CFM_OM_GetNextRemoteMep(md_p->index, nxt_ma_index, nxt_mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        if(remote_mep.md_index!= md_p->index)
        {
            return TRUE;
        }

        CFM_OM_ResetRemoteMepData(&remote_mep);

        nxt_ma_index=remote_mep.ma_index;
        nxt_mep_id=remote_mep.identifier;
    }

    return TRUE;
}/*End of CFM_ENGINE_ClearRemoteMepByDomain*/

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
BOOL_T CFM_ENGINE_ClearRemoteMepByLevel(
                                        CFM_TYPE_MdLevel_T level)
{
    CFM_OM_MD_T             *md_p=NULL;
    CFM_OM_REMOTE_MEP_T     remote_mep;
    UI32_T                  nxt_ma_index=0, nxt_mep_id=0, nxt_md_index=0;

    /*1. first get the md by level, may have many md at the same level*/
    while(NULL !=(md_p = CFM_OM_GetNextMdByLevel(nxt_md_index, level)))
    {
         nxt_md_index = md_p->index;

        /*2. get the all remote mep under the md*/
        while(TRUE == CFM_OM_GetNextRemoteMep(nxt_md_index, nxt_ma_index, nxt_mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
        {
            /*3. if the md index not the same as the under md index, break to get next*/
            if(remote_mep.md_index != nxt_md_index)
            {
                break;
            }

            nxt_ma_index=remote_mep.ma_index;
            nxt_mep_id=remote_mep.identifier;

            /*init the remote mep*/
            CFM_OM_ResetRemoteMepData(&remote_mep);
        }
    }

    return TRUE;
}/*End of CFM_ENGINE_ClearRemoteMepByLevel*/

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
BOOL_T CFM_ENGINE_ClearErrorsListByMdNameOrLevel(
                                                UI8_T *md_name_ap,
                                                UI32_T name_len,
                                                CFM_TYPE_MdLevel_T level)
{
    if(name_len != 0)
    {
        if(FALSE == CFM_OM_RemoveErrorByDomainName(md_name_ap, name_len))
        {
            return FALSE;
        }
    }
    else if(CFM_TYPE_MD_LEVEL_NONE != level)
    {
        if(FALSE ==CFM_OM_RemoveErrorByLevel(level))
        {
            return FALSE;
        }
    }

    return TRUE;
}/*End of CFM_ENGINE_ClearErrorsListByMdNameOrLevel*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ClearOnePortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : To clear one port's config including MEP/MIP.
 * INPUT    : lport - lport to clear
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : for CFM_MGR_HandleHotRemoval.
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ClearOnePortConfig(UI32_T lport)
{
    UI32_T          nxt_md_index=0, nxt_ma_index=0;
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    CFM_OM_MIP_T    mip;

    /* 1. reset port status
     */
    CFM_OM_SetCFMPortStatus(lport, SYS_DFLT_CFM_PORT_STATUS);

    /* 2. delete MEP
     *    refer to CFM_ENGINE_DeleteMEP
     */
    while (TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        if (lport !=mep_p->lport)
        {
            break;
        }

        CFM_ENGINE_LocalFreeMepTimer(mep_p);

        CFM_OM_DeleteMep(
            mep_p->md_index, mep_p->ma_index, mep_p->identifier, lport, CFM_OM_MD_MA_MEP_KEY);

        nxt_md_index = mep_p->md_index;
        nxt_ma_index = mep_p->ma_index;
    }

    /* 3. delete MIP
     */
    nxt_md_index=0;
    nxt_ma_index=0;
    while (TRUE == CFM_OM_GetNextMip(nxt_md_index, nxt_ma_index, lport, CFM_OM_LPORT_MD_MA_KEY, &mip))
    {
        if (lport!=mip.lport)
        {
            break;
        }

        CFM_OM_DeleteMip(mip.md_p->index, mip.ma_p->index, lport, CFM_OM_LPORT_MD_MA_KEY);

        nxt_md_index = mip.md_p->index;
        nxt_ma_index = mip.ma_p->index;
    }
}

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
BOOL_T CFM_ENGINE_SetMdMhfCreation(UI32_T md_index, CFM_TYPE_MhfCreation_T create_type)
{
     return CFM_OM_SetMdMhfCreation(md_index, create_type);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMdSendIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the sender id permission
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
BOOL_T CFM_ENGINE_SetMdSendIdPermission(UI32_T md_index, CFM_TYPE_MhfIdPermission_T send_id_permission)
{
    return CFM_OM_SetMdMhfIdPermission(md_index, send_id_permission);

}/*End of CFM_ENGINE_SetMdSendIdPermission*/

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
 *        CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMaMhfCreation(
                                    UI32_T md_index,
                                    UI32_T ma_index,
                                    CFM_TYPE_MhfCreation_T create_type)
{
    CFM_OM_MD_T *md_p= NULL;
    CFM_OM_MA_T *ma_p=NULL;
    UI32_T lport=0;

    if(FALSE == CFM_OM_GetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        return FALSE;
    }

    if(create_type == ma_p->mhf_creation)
    {
        return TRUE;
    }

    if(CFM_TYPE_MHF_CREATION_NONE == create_type)
    {
        while(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
        {
            CFM_OM_DeleteMip(md_index, ma_index, lport, CFM_OM_LPORT_MD_MA_KEY);
        }
    }
    else
    {
        if(ma_p->primary_vid!=0)
            CFM_ENGINE_CreateMIPByMa(md_p->level, ma_p->primary_vid, lport);
    }

    return CFM_OM_SetMaMhfCreation(md_index, ma_index, create_type);

}/*End of CFM_ENGINE_SetMaMhfCreation*/

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
BOOL_T CFM_ENGINE_SetMaMhfIdPermission(
                                        UI32_T md_index,
                                        UI32_T ma_index,
                                        CFM_TYPE_MhfIdPermission_T send_id_permission)
{
    return CFM_OM_SetMaMhfIdPermission(md_index, ma_index, send_id_permission);
}/*End of CFM_ENGINE_SetMaMhfIdPermission*/

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
 * move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMaNumOfVids(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T vid_num)
{
    return CFM_OM_SetMaNumOfVids(md_index, ma_index, vid_num);
}/*End of CFM_ENGINE_SetMaNumOfVids*/

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
BOOL_T CFM_ENGINE_SetMepLport(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T lport)
{
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    UI32_T          old_mep_lport;
    UI8_T           new_mac_addr_a[SYS_ADPT_MAC_ADDR_LEN];

    if (  (FALSE == CFM_OM_GetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
        ||(FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
       )
    {
        return FALSE;
    }

    old_mep_lport = mep_p->lport;

    if (lport == 0)
    {
        memset(new_mac_addr_a, 0, sizeof(new_mac_addr_a));
    }
    else
    {
#if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE)
        if (CFM_TYPE_MP_DIRECTION_UP == mep_p->direction)
        {
            SWCTRL_GetCpuMac(new_mac_addr_a);
        }
        else
#endif /* #if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE) */
        {
            SWCTRL_GetPortMac(lport, new_mac_addr_a);
        }
    }

    {
        UI32_T  vlan_ifindex=0;

        VLAN_OM_ConvertToIfindex(ma_p->primary_vid, &vlan_ifindex);

        /* check whether the port is vlan's member */
        if (FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, lport))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI,"port %ld does not belong to vlan %ld",
                (long)lport, (long)ma_p->primary_vid );
            return FALSE;
        }
    }

    {
        CFM_OM_MEP_T    *nxt_mep_p=&mep_tmp2_g;
        UI32_T          nxt_mep_id=0;

        /* check the same lport has already exist mep in the same ma
         * in current design, only one MEP for per MD/MA/LPORT
         */
        if (TRUE == CFM_OM_GetNextMep(md_p->index, ma_p->index, nxt_mep_id, lport, CFM_OM_LPORT_MD_MA_KEY, nxt_mep_p))
        {
            if (  (lport       == nxt_mep_p->lport)
                &&(md_p->index == nxt_mep_p->md_index)
                &&(ma_p->index == nxt_mep_p->ma_index)
               )
            {
                if (  (nxt_mep_p->identifier == mep_id)
                    &&(nxt_mep_p->direction  == mep_p->direction)
                   )
                    return TRUE;
                else
                    return FALSE;
            } /* if (  (lport       == nxt_mep_p->lport)... */

            /* no MEP with same MD/MA/LPORT found
             */
         } /* if (TRUE == CFM_OM_GetNextMep... */
    }

    if (FALSE == CFM_OM_SetMeplport(md_index, ma_index, mep_id, lport, new_mac_addr_a))
    {
        return FALSE;
    }

    /* update the mip/check error for the old mep lport
     */
    if (old_mep_lport > 0)
    {
        CFM_OM_MEP_T *higher_level_mep_p=&mep_tmp2_g;

        if(TRUE == CFM_ENGINE_IsExistHigherLevelMp(old_mep_lport,
                                                   md_p->level,
                                                   ma_p->primary_vid,
                                                   CFM_TYPE_MP_DIRECTION_DOWN,
                                                   FALSE,
                                                   higher_level_mep_p,
                                                   NULL))
        {
            CFM_OM_AddError(SYS_TIME_GetSystemTicksBy10ms(), CFM_TYPE_CONFIG_ERROR_LEAK,  higher_level_mep_p, NULL);
        }

        /*check the over level mip should exist or not*/
        CFM_ENGINE_CheckOverLevelMipShouldExist(mep_p->md_p->level, mep_p->primary_vid, old_mep_lport);

        /*create mip, check this md ma shall has mip or not after remove mep*/
        if(ma_p->primary_vid!=0)
            CFM_ENGINE_CreateMIPByMa(md_p->level, ma_p->primary_vid, old_mep_lport);
    }

    /* update the mip/check error for the new mep lport
     */
    if (lport > 0)
    {
        /* 1. delet the mip under the level which mep will create on.
           2. check lower leve has down mep or not, if it don't has down mep on a port, CFMLeak error
         */
        CFM_TYPE_MdLevel_T  level = 0, checked_level= md_p->level;
        CFM_OM_MEP_T        *temp_mep_p=&mep_tmp2_g;
        BOOL_T              has_lower_level_mep=FALSE;

        for(level=checked_level; level>=CFM_TYPE_MD_LEVEL_0; level--)
        {
            if(FALSE== CFM_OM_GetMdMaByLevelVid(level, ma_p->primary_vid, &md_p, &ma_p))
            {
                continue;
            }

            CFM_OM_DeleteMip(md_p->index, ma_p->index, lport, CFM_OM_LPORT_MD_MA_KEY);

            if(FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, temp_mep_p))
            {
                if(FALSE ==has_lower_level_mep)
                {
                    has_lower_level_mep=TRUE;

                    CFM_OM_AddError(SYS_TIME_GetSystemTicksBy10ms(), CFM_TYPE_CONFIG_ERROR_LEAK,  mep_p, NULL);
                }
            }
        }

        /* create the mip
         */
        CFM_ENGINE_CreateMIPByMep(mep_p->md_p->level, mep_p->ma_p->primary_vid, lport);
    }

    return TRUE;
}/*End of CFM_ENGINE_SetMepLport*/

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
BOOL_T CFM_ENGINE_SetMepDirection(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, CFM_TYPE_MP_Direction_T direction )
{
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;

    if (  (FALSE == CFM_OM_GetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
        ||(FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
       )
    {
        return FALSE;
    }

    mep_p->direction = direction;

#if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE)
    if (CFM_TYPE_MP_DIRECTION_UP == mep_p->direction)
    {
        SWCTRL_GetCpuMac(mep_p->mac_addr_a);
    }
    else
#endif /* #if (SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC != TRUE) */
    {
        SWCTRL_GetPortMac(mep_p->lport, mep_p->mac_addr_a);
    }

    CFM_OM_StoreMep(mep_p);
    return TRUE;
}/*End of CFM_ENGINE_SetMepDirection*/

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
BOOL_T CFM_ENGINE_SetMepPrimaryVid(
                                    UI32_T md_index,
                                    UI32_T ma_index,
                                    UI32_T mep_id, UI16_T primary_vid)
{
    CFM_OM_MA_T *ma_p=NULL;

    ma_p= CFM_OM_GetMa(md_index, ma_index);

    if(NULL == ma_p)
    {
        return FALSE;
    }

    if(primary_vid!= ma_p->primary_vid)
    {
        return FALSE;
    }

    return CFM_OM_SetMepPrimaryVid(md_index, ma_index, mep_id, primary_vid);

}/*End of CFM_ENGINE_SetMepPrimaryVid*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepActiveStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's active status
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            active_status- the mep active status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepActiveStatus(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T active_status)
{
    return CFM_OM_SetMepActive(md_index, ma_index, mep_id, active_status);
}/*End of CFM_ENGINE_SetMepActiveStatus*/

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
BOOL_T CFM_ENGINE_SetMepCciStatus(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, CFM_TYPE_CcmStatus_T cci_status)
{
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;

    if (  (FALSE == CFM_OM_GetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
        ||(FALSE == CFM_OM_GetMep(md_p->index, ma_p->index, mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
       )
    {
        return FALSE;
    }

    if (mep_p->cci_status != cci_status)
    {
        CFM_TYPE_CfmStatus_T    old_gstatus, old_pstatus;
        BOOL_T                  is_mep_enable = FALSE, is_ccm_tx = FALSE;

        /* 1. global    - no ccm rx/tx
         * 2. port      - no ccm rx/tx
         * 3. ma cc     - no ccm tx
         * 4. mep cci   - no ccm tx
         */
        if (  (FALSE == CFM_OM_GetCFMPortStatus(mep_p->lport, &old_pstatus))
            ||(FALSE == CFM_OM_GetCFMGlobalStatus(&old_gstatus))
           )
        {
            return FALSE;
        }

        if (  (CFM_TYPE_CFM_STATUS_ENABLE == old_gstatus)
            &&(CFM_TYPE_CFM_STATUS_ENABLE == old_pstatus)
           )
        {
            is_mep_enable = TRUE;

            if (  (CFM_TYPE_CCM_STATUS_ENABLE == cci_status)
                &&(CFM_TYPE_CCM_STATUS_ENABLE == mep_p->ma_p->ccm_status)
               )
            {
                is_ccm_tx = TRUE;
            }
        }

        if (TRUE == is_ccm_tx)
        {
            if (TRUE == CFM_TIMER_StartTimer(mep_p->cci_while_timer_idx))
            {
                CFM_TIMER_UpdateTimer(mep_p->cci_while_timer_idx,
                                    CFM_ENGINE_CCMTime(mep_p->ma_p->ccm_interval),
                                    CFM_ENGINE_CcmWait_Callback);
            }
        }
        else
        {
            CFM_TIMER_StopTimer(mep_p->cci_while_timer_idx);
        }

        mep_p->cci_status = cci_status;
        return CFM_OM_StoreMep(mep_p);
    }

    return TRUE;
}/*End of CFM_ENGINE_SetMepCciStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepCcmLtmPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *           ccm_ltm_priority-the ccm and ltm packet priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepCcmLtmPriority(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ccm_ltm_priority)
{
    return CFM_OM_SetMepCcmLtmPriority(md_index, ma_index, mep_id, ccm_ltm_priority);
}/*End of CFM_ENGINE_SetMepCcmLtmPriority*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMepLowPrDef
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lowest alarm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            low_pri   - the lowerst defect priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *            2. CFM_TYPE_FNG_LOWEST_ALARM_ALL
 *               CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepLowPrDef(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, CFM_TYPE_FNG_LowestAlarmPri_T low_pri)
{
    return CFM_OM_SetMepLowPrDef(md_index, ma_index, mep_id, low_pri);
}/*End of CFM_ENGINE_SetMepLowPrDef*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepFngAlarmTime(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T alarm_time)
{
    return CFM_OM_SetMepFngAlarmTime(md_index, ma_index, mep_id, alarm_time);
}/*End of CFM_ENGINE_SetMepFngAlarmTime*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepFngResetTime(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T reset_time)
{
    return CFM_OM_SetMepFngResetTime(md_index, ma_index, mep_id, reset_time);
}/*End of CFM_ENGINE_SetMepFngResetTime*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepLbmDestMacAddress(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    return CFM_OM_SetMepLbmDstMac(md_index, ma_index, mep_id, dst_mac);
}/*End of CFM_ENGINE_SetMepLbmDestMacAddress*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetMeptransmitLbmDestMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lbm destination mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mep_id- the lbm destination mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMeptransmitLbmDestMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T dst_mep_id)
{
    return CFM_OM_SetMepLbmDestMepId(md_index, ma_index, mep_id, dst_mep_id);
}/*End of CFM_ENGINE_SetMeptransmitLbmDestMepId*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepTransmitLbmDestIsMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_mep_id )
{
    return CFM_OM_SetMepLbmTargetIsMepId(md_index, ma_index, mep_id, is_mep_id);
}/*End of CFM_ENGINE_SetMepTransmitLbmDestIsMepId*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepTransmitLbmMessages(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T counts)
{
    return CFM_OM_SetMepLbmMessages(md_index, ma_index, mep_id, counts);

}/*End of CFM_ENGINE_SetMepTransmitLbmMessages*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepTransmitLbmVlanPriority(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T priority)
{
    return CFM_OM_SetMepLbmVlanPriority(md_index, ma_index, mep_id, priority);
}/*End of CFM_ENGINE_SetMepTransmitLbmVlanPriority*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetMepTransmitLtmUseFDBOnly(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_useFDBonly)
{
    return CFM_OM_SetMepLtmFlags(md_index, ma_index, mep_id, is_useFDBonly);
}/*End of CFM_ENGINE_SetMepTransmitLtmUseFDBOnly*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMacAddress(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T target_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    return CFM_OM_SetMepLtmTargetMacAddress(md_index, ma_index, mep_id, target_mac);
}/*End of CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMacAddress*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T target_mep_id)
{
    return CFM_OM_SetMepLtmTargetMepId(md_index, ma_index, mep_id, target_mep_id);
}/*End of CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMepId*/

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
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepTransmitLtmTaragetIsMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_mep_id)
{
    return CFM_OM_SetMepLtmTargetIsMepId(md_index, ma_index, mep_id, is_mep_id);

}/*End of CFM_ENGINE_SetDot1agCfmMepTransmitLtmTaragetIsMepId*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetDot1agCfmMepTransmitLtmTtl
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm ttl
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ttl       - the trnamsmit ttl
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This function only for MIB
move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_ENGINE_SetDot1agCfmMepTransmitLtmTtl(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ttl)
{
    return CFM_OM_SetMepLtmTtl(md_index, ma_index, mep_id, ttl);
}/*End of CFM_ENGINE_SetDot1agCfmMepTransmitLtmTtl*/

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
    CFM_TYPE_AIS_STATUS_T   ais_status)
{
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    UI32_T          nxt_mep_id=0;
    BOOL_T          have_defect=FALSE;

    if(FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_a, md_name_len, ma_name_a, ma_name_len, &md_p, &ma_p))
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_SetAisStatus(md_p->index, ma_p->index, ais_status))
    {
        return FALSE;
    }

    /*start to send ais*/
    if(CFM_TYPE_AIS_STATUS_ENABLE == ais_status)
    {
        while(TRUE == CFM_OM_GetNextMep(md_p->index, ma_p->index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
        {
            if(md_p->index!=mep_p->md_index
               || ma_p->index!=mep_p->ma_index)
               break;

            if(CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE!=mep_p->highest_pri_defect)
            {/*mep has defect or some remote mep still not recieved ccm so that these remote mep has not associted mep*/
                have_defect=TRUE;
                break;
            }
            nxt_mep_id = mep_p->identifier;
        }

        if(TRUE == have_defect && -1 == mep_p->ais_send_timer_idx)
        {
            CFM_Timer_CallBackPara_T    para;

            CFM_ENGINE_XmitAis(mep_p, FALSE); /*transmit AIS*/
            CFM_TIMER_AssignTimerParameter(&para, mep_p->md_index, mep_p->ma_index, mep_p->identifier, 0, 0);
            mep_p->ais_send_timer_idx=CFM_TIMER_CreateTimer(
                                              CFM_ENGINE_XmitAis_Callback,
                                              &para,
                                              mep_p->ma_p->ais_period == CFM_TYPE_AIS_PERIOD_1S?1:60,
                                              CFM_TIMER_CYCLIC);
            CFM_TIMER_StartTimer(mep_p->ais_send_timer_idx);
            CFM_OM_StoreMep(mep_p);
        }
    }
    else
    {
        while(TRUE == CFM_OM_GetNextMep(md_p->index, ma_p->index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
        {
            if(md_p->index!=mep_p->md_index
               || ma_p->index!=mep_p->ma_index)
               break;

            if(-1 != mep_p->ais_send_timer_idx)
            {
                if(TRUE == CFM_TIMER_StopTimer(mep_p->ais_send_timer_idx))
                {
                    CFM_TIMER_FreeTimer(&mep_p->ais_send_timer_idx);
                    CFM_OM_StoreMep(mep_p);
                }
            }
            nxt_mep_id = mep_p->identifier;
        }
    }
    return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SetAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais period
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            period      - the ais period
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
    UI32_T  period)
{
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    UI32_T          nxt_mep_id=0;

    if(FALSE == CFM_OM_GetMdMaByMdMaName(md_name_a, md_name_len, ma_name_a, ma_name_len, &md_p, &ma_p))
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_SetAisPeriod(md_p->index, ma_p->index, period))
    {
        return FALSE;
    }

    while(TRUE == CFM_OM_GetNextMep(md_p->index, ma_p->index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        if(md_p->index!=mep_p->md_index
           || ma_p->index!=mep_p->ma_index)
           break;

        if(mep_p->ais_send_timer_idx!=-1)
            CFM_TIMER_UpdateTimer(mep_p->ais_send_timer_idx, period, CFM_ENGINE_XmitAis_Callback);

        nxt_mep_id = mep_p->identifier;
    }

    return TRUE;
}/*End of CFM_ENGINE_SetAisPeriod*/

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
                                void *timer_para_p)
{
    CFM_OM_MA_T *ma_p= NULL;
    CFM_Timer_CallBackPara_T *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "md -%ld, ma -%ld", (long)para_p->md_index, (long)para_p->ma_index);

    if(NULL ==(ma_p = CFM_OM_GetMa(para_p->md_index, para_p->ma_index)))
    {
        return FALSE;
    }

    CFM_TIMER_FreeTimer(&ma_p->ais_rcvd_timer_idx);

    return TRUE;
}/*End of  CFM_ENGINE_AIS_Callback*/

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
                                                        void *timer_para_p)
{
    CFM_OM_MEP_T                *mep_p =&mep_tmp1_g;
    CFM_OM_MA_T                 *ma_p;
    CFM_Timer_CallBackPara_T    *para_p=(CFM_Timer_CallBackPara_T *)timer_para_p;
    BOOL_T                      is_send_ais = FALSE, is_forward = FALSE;

    if(FALSE == CFM_OM_GetMep(para_p->md_index, para_p->ma_index, para_p->mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
    {
        return FALSE;
    }

    if (NULL  != (ma_p = CFM_OM_GetMa (para_p->md_index, para_p->ma_index)))
    {
        /* 1. crosscheck disable && ma's AIS condition  => need to send AIS, forward
         * 2. crosscheck enable  && mep is in defect    => need to send AIS
         */
        if (CFM_TYPE_CROSS_CHECK_STATUS_DISABLE == mep_p->ma_p->cross_check_status)
        {
            if (-1 != ma_p->ais_rcvd_timer_idx)
            {
                is_send_ais = TRUE;
                is_forward  = TRUE;
            }
        }
        else
        {
            if (CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE != mep_p->highest_pri_defect)
            {
                is_send_ais = TRUE;
            }
        }
    }

    /* if does not need to send AIS, just stop this timer
     */
    if (FALSE == is_send_ais)
    {
        if(TRUE == CFM_TIMER_StopTimer(mep_p->ais_send_timer_idx))
        {
            CFM_TIMER_FreeTimer(&mep_p->ais_send_timer_idx);
        }
    }
    else
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_AIS, "");
        CFM_ENGINE_XmitAis(mep_p, is_forward);
    }

    CFM_OM_StoreMep(mep_p);

    return TRUE;
}/*End of CFM_ENGINE_XmitAis_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessTrunkAddMemeber
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : when the membe_ifindex become the trunk port, the all mep on this port
 *            should be deleted
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessTrunkAddMemeber(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    UI32_T nxt_md_index=0, nxt_ma_index=0;
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g, *trk_mep_p=&mep_tmp2_g;
    CFM_OM_MIP_T mip;

    while (TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, member_ifindex, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        nxt_md_index = mep_p->md_index;
        nxt_ma_index = mep_p->ma_index;

        if (member_ifindex != mep_p->lport)
        {
            break;
        }

        /* mep config will record on the 1st member port,
         * and we can not guarantee the 1st trunk member port
         * will be the first one join the lacp trunk,
         * so we can only make trunk take every MEP from member
         *
         * if trunk already has a MEP with same md/ma, the
         * MEP on member will be removed
         */
        if (TRUE == CFM_OM_GetMep(mep_p->md_index, mep_p->ma_index, 0, trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY, trk_mep_p))
        {
            if (TRUE == CFM_OM_DeleteMep(mep_p->md_index, mep_p->ma_index, mep_p->identifier, member_ifindex, CFM_OM_MD_MA_MEP_KEY))
                CFM_ENGINE_LocalFreeMepTimer(mep_p);
        }
        else
        {
            CFM_OM_SetMep(mep_p->md_index, mep_p->ma_index, mep_p->identifier, member_ifindex, mep_p->identifier, trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY);
        }
#if 0
        CFM_ENGINE_DeleteMEP(member_ifindex, 0,
            mep_p->md_p->name_a, mep_p->md_p->name_length,
            mep_p->ma_p->name_a, mep_p->ma_p->name_length);
#endif
    }

    nxt_md_index=0;
    nxt_ma_index=0;
    while (TRUE == CFM_OM_GetNextMip(nxt_md_index, nxt_ma_index, member_ifindex, CFM_OM_LPORT_MD_MA_KEY , &mip))
    {
        if (member_ifindex!=mip.lport)
        {
            break;
        }

        nxt_md_index=mip.md_p->index;
        nxt_md_index=mip.ma_p->index;

        CFM_OM_DeleteMip(mip.md_p->index, mip.ma_p->index, member_ifindex, CFM_OM_LPORT_MD_MA_KEY);
    }

    /* check if Port MAC of TRUNK is changed
     */
    {
        UI8_T   trk_pmac[SYS_ADPT_MAC_ADDR_LEN];

        if (TRUE == SWCTRL_GetPortMac(trunk_ifindex, trk_pmac))
        {
            nxt_md_index = nxt_ma_index =0;
            while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY, mep_p))
            {
                if(trunk_ifindex!=mep_p->lport)
                {
                    break;
                }

                if (0 != memcmp(mep_p->mac_addr_a, trk_pmac, sizeof(trk_pmac)))
                {
                    memcpy(mep_p->mac_addr_a, trk_pmac, sizeof(trk_pmac));
                    CFM_OM_StoreMep(mep_p);
                }

                nxt_md_index = mep_p->md_index;
                nxt_ma_index = mep_p->ma_index;
            }

            nxt_md_index = nxt_ma_index =0;
            while(TRUE == CFM_OM_GetNextMip(nxt_md_index, nxt_ma_index, trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY , &mip))
            {
                if(trunk_ifindex!=mip.lport)
                {
                    break;
                }

                if (0 != memcmp(mip.mac_address_a, trk_pmac, sizeof(trk_pmac)))
                {
                    memcpy(mip.mac_address_a, trk_pmac, sizeof(trk_pmac));
                    CFM_OM_StoreMip(&mip);
                }

                nxt_md_index = mip.md_p->index;
                nxt_ma_index = mip.ma_p->index;
            }
        }
    }

    /*configuartion follow trunk*/
    {
        CFM_TYPE_CfmStatus_T status=CFM_TYPE_CFM_STATUS_ENABLE;

        CFM_OM_GetCFMPortStatus(trunk_ifindex, &status);
        CFM_OM_SetCFMPortStatus(member_ifindex, status);
    }
}/*End of CFM_ENGINE_ProcessTrunkAddMemeber*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessTrunkAdd1stMemeber
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : when the first membe_ifindex become the trunk port, the all mep on this port
 *            should change lport to trunk_ifindex and update om
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessTrunkAdd1stMemeber(
                                            UI32_T trunk_ifindex,
                                            UI32_T member_ifindex)
{
    UI32_T nxt_md_index=0, nxt_ma_index=0;
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    CFM_OM_MIP_T mip;

    while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, member_ifindex, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        if(member_ifindex!=mep_p->lport)
        {
            break;
        }

        CFM_OM_SetMep(mep_p->md_index, mep_p->ma_index, mep_p->identifier, member_ifindex, mep_p->identifier, trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY);

        nxt_md_index = mep_p->md_index;
        nxt_ma_index = mep_p->ma_index;
    }

    nxt_md_index=0;
    nxt_ma_index=0;

    while(TRUE == CFM_OM_GetNextMip(nxt_md_index, nxt_ma_index, member_ifindex, CFM_OM_LPORT_MD_MA_KEY, &mip))
    {
        if(member_ifindex!=mip.lport)
        {
            break;
        }

        if(TRUE == CFM_OM_SetMip(mip.md_p->index, mip.ma_p->index, member_ifindex, trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY))
        {
            mip.lport=trunk_ifindex;
        }

        nxt_md_index = mip.md_p->index;
        nxt_ma_index = mip.ma_p->index;
    }

    /*configuartion follow first member*/
    {
        CFM_TYPE_CfmStatus_T status=CFM_TYPE_CFM_STATUS_ENABLE;

        CFM_OM_GetCFMPortStatus(member_ifindex, &status);
        CFM_OM_SetCFMPortStatus(trunk_ifindex, status);
    }

}/*End of CFM_ENGINE_ProcessTrunkAdd1stMemeber*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessTrunkDeleteLastMemeber
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : like add first member to trunk port, the leave should do the same thing.
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessTrunkDeleteLastMemeber(
                                            UI32_T trunk_ifindex,
                                            UI32_T member_ifindex)
{
    UI32_T nxt_md_index=0, nxt_ma_index=0;
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    CFM_OM_MIP_T mip;

    while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0,trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        if(trunk_ifindex!=mep_p->lport)
        {
            break;
        }

        if(TRUE ==CFM_OM_SetMep(mep_p->md_index, mep_p->ma_index, mep_p->identifier, trunk_ifindex , mep_p->identifier, member_ifindex , CFM_OM_LPORT_MD_MA_KEY))
        {
            mep_p->lport=member_ifindex;
        }
        nxt_md_index = mep_p->md_index;
        nxt_ma_index = mep_p->ma_index;
    }

    nxt_md_index=0;
    nxt_ma_index=0;

    while(TRUE == CFM_OM_GetNextMip(nxt_md_index, nxt_ma_index, trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY , &mip))
    {
        if(trunk_ifindex!=mip.lport)
        {
            break;
        }

        if(TRUE==CFM_OM_SetMip(mip.md_p->index, mip.ma_p->index, trunk_ifindex , member_ifindex, CFM_OM_LPORT_MD_MA_KEY))
        {
            mip.lport=member_ifindex;
        }
        nxt_md_index = mip.md_p->index;
        nxt_ma_index = mip.ma_p->index;
    }

    /* sync from ASE4512BBS-FLF-P5-01125
     */
    /*configuartion follow trunk*/
    {
        CFM_TYPE_CfmStatus_T status=CFM_TYPE_CFM_STATUS_ENABLE;

        CFM_OM_GetCFMPortStatus(trunk_ifindex, &status);
        CFM_OM_SetCFMPortStatus(member_ifindex, status);
    }
}/*End of CFM_ENGINE_ProcessTrunkDeleteLastMemeber*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessTrunkMemberDelete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : when remote the member from the trunk port, this member shall recalcuate
 *            the mip, because this member ever delete all mip when it join the trunk port
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessTrunkMemberDelete(
                                        UI32_T trunk_ifindex,
                                        UI32_T member_ifindex)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    UI32_T nxt_ma_index=0, nxt_md_index=0;
    CFM_TYPE_MdLevel_T level=CFM_TYPE_MD_LEVEL_0;
    UI8_T checked_vid[(SYS_DFLT_DOT1QMAXVLANID/8)+1] ={0};

    for(;level<=CFM_TYPE_MD_LEVEL_7;level++)
    {
        nxt_md_index=0;

        while(NULL!=(md_p=CFM_OM_GetNextMdByLevel(nxt_md_index, level)))
        {
            nxt_ma_index=0;

            while(NULL != (ma_p=CFM_OM_GetNextMaByMaIndex(md_p->index, nxt_ma_index)))
            {
                nxt_ma_index = ma_p->index;

                /*check this vid already create mip or not, if already, skip this vid*/
                if(checked_vid[(UI32_T)((ma_p->primary_vid-1)/8)]&(7 - (ma_p->primary_vid-1)%8))
                {
                    continue;
                }
                else
                {
                    checked_vid[(UI32_T)((ma_p->primary_vid-1)/8)]|=(7 - (ma_p->primary_vid-1)%8);
                }

                if(ma_p->primary_vid!=0)
                    CFM_ENGINE_CreateMIPByMa(level, ma_p->primary_vid, member_ifindex);
            }

            nxt_md_index=md_p->index;
        }
    }

    /* check if Port MAC of TRUNK is changed
     */
    {
        CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
        CFM_OM_MIP_T    mip;
        UI8_T           trk_pmac[SYS_ADPT_MAC_ADDR_LEN];

        if (TRUE == SWCTRL_GetPortMac(trunk_ifindex, trk_pmac))
        {
            nxt_md_index = nxt_ma_index =0;
            while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY, mep_p))
            {
                if(trunk_ifindex!=mep_p->lport)
                {
                    break;
                }

                if (0 != memcmp(mep_p->mac_addr_a, trk_pmac, sizeof(trk_pmac)))
                {
                    memcpy(mep_p->mac_addr_a, trk_pmac, sizeof(trk_pmac));
                    CFM_OM_StoreMep(mep_p);
                }

                nxt_md_index = mep_p->md_index;
                nxt_ma_index = mep_p->ma_index;
            }

            nxt_md_index = nxt_ma_index =0;
            while (TRUE == CFM_OM_GetNextMip(nxt_md_index, nxt_ma_index, trunk_ifindex, CFM_OM_LPORT_MD_MA_KEY , &mip))
            {
                if (trunk_ifindex!=mip.lport)
                {
                    break;
                }

                if (0 != memcmp(mip.mac_address_a, trk_pmac, sizeof(trk_pmac)))
                {
                    memcpy(mip.mac_address_a, trk_pmac, sizeof(trk_pmac));
                    CFM_OM_StoreMip(&mip);
                }

                nxt_md_index = mip.md_p->index;
                nxt_ma_index = mip.ma_p->index;
            }
        }
    }

    /* sync from ASE4512BBS-FLF-P5-01125
     */
    /*configuartion follow trunk*/
    {
        CFM_TYPE_CfmStatus_T status=CFM_TYPE_CFM_STATUS_ENABLE;

        CFM_OM_GetCFMPortStatus(trunk_ifindex, &status);
        CFM_OM_SetCFMPortStatus(member_ifindex, status);
    }

    return;
}/*End of CFM_ENGINE_ProcessTrunkMemberDelete*/

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
void CFM_ENGINE_ProcessInterfaceStatusChange(UI32_T lport)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    UI32_T nxt_md_index=0, nxt_ma_index=0;

    while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        if(mep_p->lport != lport)
        {
            return;
        }

        if(mep_p->ma_p->ccm_interval>= CFM_TYPE_CCM_INTERVAL_10_S)
        {
            CFM_ENGINE_XmitCCM(mep_p,  NULL);
            CFM_OM_StoreMep(mep_p);
        }

        nxt_md_index=mep_p->md_index;
        nxt_ma_index=mep_p->ma_index;
    }

}/*End of CFM_ENGINE_ProcessInterfaceStatusChange*/

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
void CFM_ENGINE_ProcessPortStatusChange(
                                        UI16_T vid,
                                        UI32_T lport,
                                        BOOL_T is_forwarding)
{
    CFM_ENGINE_PortStatusCallback_T port_status_notify;
    CFM_OM_MEP_T                    *mep_p=&mep_tmp1_g;
    UI32_T                          nxt_md_index=0, nxt_ma_index=0;

    /*set the notify interface status on*/
    memset(&port_status_notify, 0, sizeof(CFM_ENGINE_PortStatusCallback_T));
    port_status_notify.xstp_forwarding=is_forwarding;

    while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        if(mep_p->lport != lport)
        {
            return;
        }

        if( ((mep_p->primary_vid == vid)||(0==vid)) &&
          (mep_p->ma_p->ccm_interval>= CFM_TYPE_CCM_INTERVAL_10_S))
        {
            CFM_ENGINE_XmitCCM(mep_p, &port_status_notify);
            CFM_OM_StoreMep(mep_p);
        }

        nxt_md_index=mep_p->md_index;
        nxt_ma_index=mep_p->ma_index;
    }
}/*End of CFM_ENGINE_ProcessPortStatusChange*/

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
void CFM_ENGINE_ProcessPortAdminDisable (UI32_T lport)
{
    CFM_ENGINE_PortStatusCallback_T port_status_notify;
    CFM_OM_MEP_T *mep_p=&mep_tmp1_g;
    UI32_T nxt_md_index=0, nxt_ma_index=0;

    /*set the notify interface status on*/
    memset(&port_status_notify, 0, sizeof(CFM_ENGINE_PortStatusCallback_T));
    port_status_notify.admin_disable=TRUE;

    while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        if(mep_p->lport != lport)
        {
            return;
        }

        if(mep_p->ma_p->ccm_interval>= CFM_TYPE_CCM_INTERVAL_10_S)
        {
            CFM_ENGINE_XmitCCM(mep_p, &port_status_notify);
            CFM_OM_StoreMep(mep_p);
        }

        nxt_md_index=mep_p->md_index;
        nxt_ma_index=mep_p->ma_index;
    }

    return;
}/*End of CFM_ENGINE_ProcessPortAdminDisable*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessVlanCreate
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan create and add cpu mac
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessVlanCreate(UI32_T vlan_ifindex)
{
    return;
}/*End of CFM_ENGINE_ProcessVlanCreate*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessVlanDestory
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan destory and remove the mep and mip
 * INPUT    : vid  - the logical vid
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_ENGINE_ProcessVlanDestory(UI32_T vlan_ifindex)
{
    UI32_T          md_level=MIN_dot1agCfmMdMdLevel;
    UI32_T          port_ifindex;
    UI32_T          vid;
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    CFM_OM_MEP_T    mep;
    CFM_OM_MIP_T    mip;

    VLAN_IFINDEX_CONVERTTO_VID (vlan_ifindex, vid);

    for(;md_level<=MAX_dot1agCfmMdMdLevel;md_level++)
    {
        if(CFM_OM_GetMdMaByLevelVid(md_level,vid,&md_p,&ma_p)==FALSE)
            continue;

        for (port_ifindex = 1; port_ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT; port_ifindex++)
        {
            if (SWCTRL_LogicalPortExisting(port_ifindex) == FALSE)
                continue;

            if(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, port_ifindex, CFM_OM_LPORT_MD_MA_KEY, &mep))
            {
                CFM_ENGINE_DeleteMEP(port_ifindex, 0,
                    mep.md_p->name_a, mep.md_p->name_length,
                    mep.ma_p->name_a, mep.ma_p->name_length);
            }
            if(TRUE==CFM_OM_GetMip(md_p->index, ma_p->index, port_ifindex, CFM_OM_LPORT_MD_MA_KEY,&mip))
            {
                CFM_OM_DeleteMip(mip.md_p->index, mip.ma_p->index, port_ifindex, CFM_OM_LPORT_MD_MA_KEY);
            }
        }
    }
}/*End of CFM_ENGINE_ProcessVlanDestory*/

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
void CFM_ENGINE_ProcessVlanMemberAdd(UI32_T vlan_ifindex, UI32_T lport_ifindex)
{
    UI32_T      md_level=MIN_dot1agCfmMdMdLevel;
    UI32_T      vid;
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;

    VLAN_IFINDEX_CONVERTTO_VID (vlan_ifindex, vid);

    for(;md_level<=MAX_dot1agCfmMdMdLevel;md_level++)
    {
        if(CFM_OM_GetMdMaByLevelVid(md_level,vid,&md_p,&ma_p)==FALSE)
            continue;

        CFM_ENGINE_CreateMIPByMa(md_p->level, ma_p->primary_vid, lport_ifindex);
    }
}/*End of CFM_ENGINE_ProcessVlanMemberAdd*/

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
void CFM_ENGINE_ProcessVlanMemberDelete(
    UI32_T  vlan_ifindex,
    UI32_T  lport_ifindex)
{
    UI32_T          md_level=MIN_dot1agCfmMdMdLevel;
    UI32_T          vid;
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    CFM_OM_MEP_T    mep;
    CFM_OM_MIP_T    mip;

    VLAN_IFINDEX_CONVERTTO_VID (vlan_ifindex, vid);

    for(;md_level<=MAX_dot1agCfmMdMdLevel;md_level++)
    {
        if(CFM_OM_GetMdMaByLevelVid(md_level,vid,&md_p,&ma_p)==FALSE)
            continue;

        if(TRUE == CFM_OM_GetMep(md_p->index, ma_p->index, 0, lport_ifindex, CFM_OM_LPORT_MD_MA_KEY, &mep))
        {
            CFM_ENGINE_DeleteMEP(
                lport_ifindex, 0,
                mep.md_p->name_a, mep.md_p->name_length,
                mep.ma_p->name_a, mep.ma_p->name_length);
        }
        if(TRUE==CFM_OM_GetMip(md_p->index, ma_p->index, lport_ifindex, CFM_OM_LPORT_MD_MA_KEY,&mip))
        {
            CFM_OM_DeleteMip(mip.md_p->index, mip.ma_p->index, lport_ifindex, CFM_OM_LPORT_MD_MA_KEY);
        }
     }
}/*End of CFM_ENGINE_ProcessVlanMemberAdd*/

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
    UI32_T      ma_name_len)
{
    CFM_OM_DmmCtrlRec_T     *dmm_ctrl_rec_p;
    BOOL_T                  ret = FALSE;

    dmm_ctrl_rec_p = CFM_OM_GetDmmCtrlRecPtr();

    if (NULL != dmm_ctrl_rec_p)
    {
        if (TRUE == dmm_ctrl_rec_p->is_busy)
        {
            /* check if abort the correct mission...
             */
            if (  (dmm_ctrl_rec_p->src_mep_id == src_mep_id)
                &&(dmm_ctrl_rec_p->dst_mep_id == dst_mep_id)
                &&(memcmp(dmm_ctrl_rec_p->dst_mac_ar, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN) == 0)
                &&(memcmp(dmm_ctrl_rec_p->md_name_ar, md_name_p, md_name_len) == 0)
                &&(memcmp(dmm_ctrl_rec_p->ma_name_ar, ma_name_p, ma_name_len) == 0)
               )
            {
                CFM_ENGINE_ExpireDmrRecord(dmm_ctrl_rec_p, 0);
                ret = TRUE;
            }
        }
    }

    return ret;
}

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
    UI16_T      pkt_pri)
{
    CFM_OM_DmmCtrlRec_T     *dmm_ctrl_rec_p;
    BOOL_T                  ret = FALSE;

    dmm_ctrl_rec_p = CFM_OM_GetDmmCtrlRecPtr();

    if (NULL != dmm_ctrl_rec_p)
    {
        if (FALSE == dmm_ctrl_rec_p->is_busy)
        {
            dmm_ctrl_rec_p->is_busy = TRUE;

            if (TRUE == CFM_ENGINE_XmitDMM(src_mep_id, dst_mep_id, dst_mac_p,
                            md_name_p, md_name_len, ma_name_p, ma_name_len,
                            pkt_size, pkt_pri, NULL, TRUE))
            {
                memcpy(dmm_ctrl_rec_p->dst_mac_ar, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN);
                memcpy(dmm_ctrl_rec_p->md_name_ar, md_name_p, md_name_len+1);
                memcpy(dmm_ctrl_rec_p->ma_name_ar, ma_name_p, ma_name_len+1);
                memset(dmm_ctrl_rec_p->dmr_rec_ar, 0, sizeof(dmm_ctrl_rec_p->dmr_rec_ar));
                memset(dmm_ctrl_rec_p->dmr_seq_ar, 0, sizeof(dmm_ctrl_rec_p->dmr_seq_ar));
                dmm_ctrl_rec_p->src_mep_id = src_mep_id;
                dmm_ctrl_rec_p->dst_mep_id = dst_mep_id;
                dmm_ctrl_rec_p->cur_dmm_seq= 0;
                dmm_ctrl_rec_p->cur_rcv_idx= 0;
                dmm_ctrl_rec_p->counts     = counts;
                dmm_ctrl_rec_p->interval   = interval;
                dmm_ctrl_rec_p->timeout    = timeout;
                dmm_ctrl_rec_p->pkt_size   = pkt_size - 18; /* not including (da + sa + eth + fcs) = total size - 18 */
                dmm_ctrl_rec_p->pkt_pri    = pkt_pri;
                dmm_ctrl_rec_p->md_name_len= md_name_len;
                dmm_ctrl_rec_p->ma_name_len= ma_name_len;

                dmm_ctrl_rec_p->fst_timeout_id  = 0;
                dmm_ctrl_rec_p->next_send_timer = 1;

                CFM_TIMER_StartTimer(dmm_ctrl_rec_p->tx_timer_idx);
                ret = TRUE;
            }
            else
            {
                /* test failed, return FALSE to abort the mission
                 */
                dmm_ctrl_rec_p->is_busy = FALSE;
            }
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "ret-%d", ret);
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_AddDmrToResult
 *-------------------------------------------------------------------------
 * PURPOSE  : To add the dmr record to result.
 * INPUT    : dmr_p - pointer to content of dmr pdu
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_AddDmrToResult(
    CFM_ENGINE_DmPdu_T      *dmr_p)
{
    CFM_OM_DmmCtrlRec_T     *dmm_ctrl_rec_p;
    UI32_T                  idx, txf_10ms, cur_time_10ms, fd_ms;

    dmm_ctrl_rec_p = CFM_OM_GetDmmCtrlRecPtr();

    if (NULL != dmm_ctrl_rec_p)
    {
        if (TRUE == dmm_ctrl_rec_p->is_busy)
        {
            cur_time_10ms = SYS_TIME_GetSystemTicksBy10ms();
            dmr_p->rx_time_stampb_p->secs      = L_STDLIB_Hton32(CFM_ENGINE_10MS_TO_SEC(cur_time_10ms));
            dmr_p->rx_time_stampb_p->nano_secs = L_STDLIB_Hton32(CFM_ENGINE_10MS_TO_NANOSEC(cur_time_10ms % 100));

            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "rx-10ms: %lu", (long)cur_time_10ms);
            fd_ms = CFM_ENGINE_ComputeFramDealy(dmr_p, &txf_10ms);

            for (idx =dmm_ctrl_rec_p->fst_timeout_id; idx < dmm_ctrl_rec_p->cur_dmm_seq; idx++)
            {
                if (dmm_ctrl_rec_p->dmr_rec_ar[idx].rec_state == CFM_TYPE_DMM_REPLY_REC_STATE_WAITREPLY)
                {
                    if (txf_10ms == dmm_ctrl_rec_p->dmr_rec_ar[idx].dmm_txf_10ms)
                    {
                        /* make sure the dmr is received in time...
                         */
                        if (fd_ms < dmm_ctrl_rec_p->timeout * 1000)
                        {
                            UI32_T  prv_rcvd_rec_id = 0xffffffff,
                                    prv_rcvd_seq_id, tmp_rec_id;

                            dmm_ctrl_rec_p->dmr_rec_ar[idx].rec_state      = CFM_TYPE_DMM_REPLY_REC_STATE_RECEIVED;
                            dmm_ctrl_rec_p->dmr_rec_ar[idx].frame_delay_ms = fd_ms;

                            /* calculate fdv from received sequence
                             * step 1. find previous received record idx
                             */
                            prv_rcvd_seq_id = dmm_ctrl_rec_p->cur_rcv_idx;
                            while (prv_rcvd_seq_id > 0)
                            {
                                tmp_rec_id = dmm_ctrl_rec_p->dmr_seq_ar[prv_rcvd_seq_id - 1] - 1;
                                if (dmm_ctrl_rec_p->dmr_rec_ar[tmp_rec_id].rec_state == CFM_TYPE_DMM_REPLY_REC_STATE_RECEIVED)
                                {
                                    prv_rcvd_rec_id = tmp_rec_id;
                                    break;
                                }
                                prv_rcvd_seq_id--;
                            }
                            /* step 2. calculate fdv
                             */
                            if (prv_rcvd_rec_id == 0xffffffff)
                            {
                                /* no previous received record idx, i.e.
                                 *  it's the first one
                                 */
                                dmm_ctrl_rec_p->dmr_rec_ar[idx].fdv_ms = 0;
                            }
                            else
                            {
                                /* calculate the absolute value of fdv
                                 */
                                if (  dmm_ctrl_rec_p->dmr_rec_ar[idx].frame_delay_ms
                                    > dmm_ctrl_rec_p->dmr_rec_ar[prv_rcvd_rec_id].frame_delay_ms)
                                {
                                    dmm_ctrl_rec_p->dmr_rec_ar[idx].fdv_ms =
                                      dmm_ctrl_rec_p->dmr_rec_ar[idx].frame_delay_ms
                                    - dmm_ctrl_rec_p->dmr_rec_ar[prv_rcvd_rec_id].frame_delay_ms;
                                }
                                else
                                {
                                    dmm_ctrl_rec_p->dmr_rec_ar[idx].fdv_ms =
                                      dmm_ctrl_rec_p->dmr_rec_ar[prv_rcvd_rec_id].frame_delay_ms
                                    - dmm_ctrl_rec_p->dmr_rec_ar[idx].frame_delay_ms;
                                }
                            }
                        }
                        else
                        {
                            dmm_ctrl_rec_p->dmr_rec_ar[idx].rec_state      = CFM_TYPE_DMM_REPLY_REC_STATE_TIMEOUT;
                        }

                        dmm_ctrl_rec_p->dmr_seq_ar[dmm_ctrl_rec_p->cur_rcv_idx++] = idx+1;

                        if (dmm_ctrl_rec_p->fst_timeout_id == idx)
                        {
                            dmm_ctrl_rec_p->fst_timeout_id++;
                        }

                        if (dmr_p->header_p->pdu_length != dmm_ctrl_rec_p->pkt_size)
                        {
                            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "wrong pkt_size-%lu", (long)dmr_p->header_p->pdu_length);
                        }
                    }
                }
            }

            if (dmm_ctrl_rec_p->cur_rcv_idx == dmm_ctrl_rec_p->counts)
            {
                CFM_TIMER_StopTimer(dmm_ctrl_rec_p->tx_timer_idx);
                dmm_ctrl_rec_p->is_busy = FALSE;
            }
        }
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_BgXmitDMM_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : To transmit dmm in background when callback from the timer.
 * INPUT    : rec_p - pointer to content of global dmm control record
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_BgXmitDMM_CallBack(
    void    *rec_p)
{
    CFM_OM_DmmCtrlRec_T *dmm_ctrl_rec_p;
    UI32_T              tx_time_10ms;
    BOOL_T              ret = FALSE;

    dmm_ctrl_rec_p = CFM_OM_GetDmmCtrlRecPtr();

    if (NULL != dmm_ctrl_rec_p)
    {
        if (TRUE == dmm_ctrl_rec_p->is_busy)
        {
            if (dmm_ctrl_rec_p->cur_dmm_seq < dmm_ctrl_rec_p->counts)
            {
                if (dmm_ctrl_rec_p->next_send_timer > 0)
                    --dmm_ctrl_rec_p->next_send_timer;

                if (dmm_ctrl_rec_p->next_send_timer == 0)
                {
                    if (TRUE == CFM_ENGINE_XmitDMM(
                                    dmm_ctrl_rec_p->src_mep_id,
                                    dmm_ctrl_rec_p->dst_mep_id,
                                    dmm_ctrl_rec_p->dst_mac_ar,
                                    dmm_ctrl_rec_p->md_name_ar,
                                    dmm_ctrl_rec_p->md_name_len,
                                    dmm_ctrl_rec_p->ma_name_ar,
                                    dmm_ctrl_rec_p->ma_name_len,
                                    dmm_ctrl_rec_p->pkt_size,
                                    dmm_ctrl_rec_p->pkt_pri,
                                    &tx_time_10ms,
                                    FALSE))
                    {
                        dmm_ctrl_rec_p->dmr_rec_ar[dmm_ctrl_rec_p->cur_dmm_seq].rec_state    = CFM_TYPE_DMM_REPLY_REC_STATE_WAITREPLY;
                        dmm_ctrl_rec_p->dmr_rec_ar[dmm_ctrl_rec_p->cur_dmm_seq].dmm_txf_10ms = tx_time_10ms;
                    }
                    else
                    {
                        dmm_ctrl_rec_p->dmr_rec_ar[dmm_ctrl_rec_p->cur_dmm_seq].rec_state  = CFM_TYPE_DMM_REPLY_REC_STATE_SENDFAIL;
                        dmm_ctrl_rec_p->dmr_seq_ar[dmm_ctrl_rec_p->cur_rcv_idx++]          = dmm_ctrl_rec_p->cur_dmm_seq+1;
                    }

                    ++dmm_ctrl_rec_p->cur_dmm_seq;

                    dmm_ctrl_rec_p->next_send_timer = dmm_ctrl_rec_p->interval;
                }
            }

            if (dmm_ctrl_rec_p->cur_dmm_seq != dmm_ctrl_rec_p->cur_rcv_idx)
            {
                CFM_ENGINE_ExpireDmrRecord(dmm_ctrl_rec_p, SYS_TIME_GetSystemTicksBy10ms());
            }
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "ret-%d", ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ComputeDiffOfTimes
 *-------------------------------------------------------------------------
 * PURPOSE  : To compute the result of time_a minus time_b.
 * INPUT    : time_a - time stamp a
 *            time_b - time stamp b
 * OUTPUT   : None
 * RETURN   : (time_a - time_b)
 * NOTE     : 1. to fix the problem when integer wrapping happen to time_a
 *
 *-------------------------------------------------------------------------
 */
static UI32_T CFM_ENGINE_ComputeDiffOfTwoTimeStamp(
    UI32_T      time_a,
    UI32_T      time_b)
{
    UI32_T  ret;

    if (time_a < time_b)
    {
        /* integer wrapping happen to time_b
         */
        ret = 0xffffffff - time_b + time_a;
    }
    else
    {
        ret = time_a - time_b;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ComputeFramDealy
 *-------------------------------------------------------------------------
 * PURPOSE  : To compute the frame delay from the DMR content.
 * INPUT    : dmr_p     - pointer to content of dmr pdu
 * OUTPUT   : txf_ms_p  - pointer to content of txf stamp in ms
 * RETURN   : frame delay in ms
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static UI32_T CFM_ENGINE_ComputeFramDealy(
    CFM_ENGINE_DmPdu_T      *dmr_p,
    UI32_T                  *txf_10ms_p)
{
    UI32_T  rxb_10ms, txf_10ms, txb_10ms = 0, rxf_10ms =0, fd1_10ms =0, fd2_10ms =0;

    /* according to y1731_ww9-consented.doc, 8.2,
     *   Frame Delay = (RxTimeStampb-TxTimeStampf)-(TxTimeStampb-RxTimeStampf)
     */
    rxb_10ms = CFM_ENGINE_CNV_SEC_P_NS_TO_10MS(
                L_STDLIB_Ntoh32(dmr_p->rx_time_stampb_p->secs),
                L_STDLIB_Ntoh32(dmr_p->rx_time_stampb_p->nano_secs));

    txf_10ms = CFM_ENGINE_CNV_SEC_P_NS_TO_10MS(
                L_STDLIB_Ntoh32(dmr_p->tx_time_stampf_p->secs),
                L_STDLIB_Ntoh32(dmr_p->tx_time_stampf_p->nano_secs));

    if ((0 != dmr_p->tx_time_stampb_p->secs)      &&
        (0 != dmr_p->tx_time_stampb_p->nano_secs) &&
        (0 != dmr_p->rx_time_stampf_p->secs)      &&
        (0 != dmr_p->rx_time_stampf_p->nano_secs))
    {
        txb_10ms = CFM_ENGINE_CNV_SEC_P_NS_TO_10MS(
                    L_STDLIB_Ntoh32(dmr_p->tx_time_stampb_p->secs),
                    L_STDLIB_Ntoh32(dmr_p->tx_time_stampb_p->nano_secs));

        rxf_10ms = CFM_ENGINE_CNV_SEC_P_NS_TO_10MS(
                    L_STDLIB_Ntoh32(dmr_p->rx_time_stampf_p->secs),
                    L_STDLIB_Ntoh32(dmr_p->rx_time_stampf_p->nano_secs));

        fd2_10ms = CFM_ENGINE_ComputeDiffOfTwoTimeStamp(txb_10ms, rxf_10ms);
    }

    fd1_10ms = CFM_ENGINE_ComputeDiffOfTwoTimeStamp(rxb_10ms, txf_10ms);

    if (fd1_10ms < fd2_10ms)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "negative frame delay");
        fd1_10ms = 0;
    }
    else
    {
        /* due to the timeout limitation, overflow of (10 * fd1_10ms) should not occur
         */
        fd1_10ms -= fd2_10ms;
    }

    if (NULL != txf_10ms_p)
    {
        *txf_10ms_p = txf_10ms;
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "rxb_10ms-%lu, rxf_10ms-%lu", (long)rxb_10ms, (long)rxf_10ms);
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "txb_10ms-%lu, txf_10ms-%lu", (long)txb_10ms, (long)txf_10ms);
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "frame delay %lu 10ms(millisecond)", (long)fd1_10ms);

    return (10 * fd1_10ms);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructDMPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : To construct the DMM/DMR pdu
 * INPUT    : src_mep_p      - pointer to content of source mep to construct pdu
 *            opcode         - which kind of pdu to construct
 *                             CFM_TYPE_OPCODE_DMM/CFM_TYPE_OPCODE_DMR
 *            rcvd_pdu_p     - pointer to content of received pdu
 *            rcvd_pdu_len   - length of received pdu (dmm)
 * OUTPUT   : tran_pdu_p     - pointer to content of pdu to transmit
 *            tran_pdu_len_p - pointer to content of pdu length to transmit
 *            tx_time_10ms_p - pointer to content of tx time stamp in 10 ms
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ConstructDMPDU(
    CFM_OM_MEP_T        *src_mep_p,
    CFM_TYPE_OpCode_T   opcode,
    UI8_T               *rcvd_pdu_p,
    UI16_T              rcvd_pdu_len,
    UI8_T               *tran_pdu_p,
    UI16_T              *tran_pdu_len_p,
    UI32_T              *tx_time_10ms_p)
{
    BOOL_T  ret = FALSE;

    switch(opcode)
    {
    case CFM_TYPE_OPCODE_DMM:
        if ((NULL != src_mep_p) && (NULL != tran_pdu_p) && (NULL != tran_pdu_len_p))
        {
            CFM_TYPE_TimeReprFmt_T  cur_trf;
            UI32_T                  cur_time_10ms;
            UI16_T                  tlv_len = 0;

            /* 1. constrcut common header, common header has 4 bytes,
             *      the end of tlv will be add at the end of pdu
             */
            CFM_ENGINE_ConstructCommonHeader(
                tran_pdu_p, tran_pdu_len_p, src_mep_p->md_p->level, 0, opcode);

            /* move to next octect
             */
            tran_pdu_p += *tran_pdu_len_p;

            memset(tran_pdu_p, 0, 4 * sizeof(cur_trf));
            *tran_pdu_len_p += 4 * sizeof (cur_trf);

            CFM_ENGINE_ConstructEndTLV(tran_pdu_p+ (4 * sizeof (cur_trf)), &tlv_len);
            *tran_pdu_len_p += tlv_len;

            cur_time_10ms     = SYS_TIME_GetSystemTicksBy10ms();
            cur_trf.secs      = L_STDLIB_Hton32(CFM_ENGINE_10MS_TO_SEC(cur_time_10ms));
            cur_trf.nano_secs = L_STDLIB_Hton32(CFM_ENGINE_10MS_TO_NANOSEC(cur_time_10ms % 100));
            memcpy(tran_pdu_p, &cur_trf, sizeof(cur_trf));

            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM,
                "cur_10ms-%lu, sec-%lu, nsec-%lu",
                (long)cur_time_10ms,
                (long)L_STDLIB_Ntoh32(cur_trf.secs),
                (long)L_STDLIB_Ntoh32(cur_trf.nano_secs));

            if (NULL != tx_time_10ms_p)
            {
                *tx_time_10ms_p = cur_time_10ms;
            }
            ret = TRUE;
        }
        break;

    case CFM_TYPE_OPCODE_DMR:
        if ((NULL != rcvd_pdu_p) && (NULL != tran_pdu_p) && (NULL != tran_pdu_len_p))
        {
            /* according to y1731_ww9-consented.doc, 9.16.1,
             */
            /* copy TxTimeStampf from received pdu
             */
            memcpy(tran_pdu_p, rcvd_pdu_p, rcvd_pdu_len);
            *(tran_pdu_p+1) = CFM_TYPE_OPCODE_DMR;

            /*   set RxTimeStampf, TxTimeStampb, RxTimeStampb to 0
             */
            memset(tran_pdu_p+4+sizeof(CFM_TYPE_TimeReprFmt_T), 0, 3 * sizeof(CFM_TYPE_TimeReprFmt_T));

            *tran_pdu_len_p = rcvd_pdu_len;

            ret = TRUE;
        }
        break;

    default:
        break;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ExpireDmrRecord
 *-------------------------------------------------------------------------
 * PURPOSE  : To expire the dmr record which is waiting for reply.
 * INPUT    : dmm_ctrl_rec_p - pointer to content of global dmm control record
 *            curr_time_10ms - current time stamp in 10 ms
 *                             (0 - to expire all)
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_ExpireDmrRecord(
    CFM_OM_DmmCtrlRec_T     *dmm_ctrl_rec_p,
    UI32_T                  curr_time_10ms)
{
    CFM_OM_DmrRec_T     *dmr_rec_p;
    UI32_T              idx;
    UI32_T              elapsed_sec;

    if (NULL != dmm_ctrl_rec_p)
    {
        if (TRUE == dmm_ctrl_rec_p->is_busy)
        {
            for (idx =dmm_ctrl_rec_p->fst_timeout_id; idx < dmm_ctrl_rec_p->cur_dmm_seq; idx++)
            {
                dmr_rec_p = &dmm_ctrl_rec_p->dmr_rec_ar[idx];
                if (dmr_rec_p->rec_state == CFM_TYPE_DMM_REPLY_REC_STATE_WAITREPLY)
                {
                    if (curr_time_10ms != 0)
                    {
                        /* curr time is wrapped...
                         */
                        if (curr_time_10ms < dmr_rec_p->dmm_txf_10ms)
                        {
                            elapsed_sec = ((0xffffffff - dmr_rec_p->dmm_txf_10ms)/100) + (curr_time_10ms/100);
                        }
                        else
                        {
                            elapsed_sec = (curr_time_10ms/100) - (dmr_rec_p->dmm_txf_10ms/100);
                        }
                    }
                    else
                    {
                        elapsed_sec = (dmm_ctrl_rec_p->timeout);
                    }

                    if (elapsed_sec >= dmm_ctrl_rec_p->timeout)
                    {
                        dmr_rec_p->rec_state= CFM_TYPE_DMM_REPLY_REC_STATE_TIMEOUT;
                        dmm_ctrl_rec_p->dmr_seq_ar[dmm_ctrl_rec_p->cur_rcv_idx++] = idx +1;
                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "expire idx-%lu, txf-%lu, cur-%lu",
                            (long)idx, (long)dmr_rec_p->dmm_txf_10ms, (long)curr_time_10ms);
                    }
                    else
                    {
                        /* if current one is not timeout, next one is not timeout, either...
                         * start to timeout from this idx next time...
                         */
                        dmm_ctrl_rec_p->fst_timeout_id = idx;
                        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "expire idx-%lu, txf-%lu, cur-%lu",
                            (long)idx, (long)dmr_rec_p->dmm_txf_10ms, (long)curr_time_10ms);
                        break;
                    }
                }
            }

            if (  (dmm_ctrl_rec_p->cur_rcv_idx >= dmm_ctrl_rec_p->counts)
                ||(0 == curr_time_10ms)
               )
            {
                CFM_TIMER_StopTimer(dmm_ctrl_rec_p->tx_timer_idx);
                dmm_ctrl_rec_p->is_busy = FALSE;

                if (dmm_ctrl_rec_p->cur_rcv_idx != dmm_ctrl_rec_p->counts)
                {
                    dmm_ctrl_rec_p->counts = dmm_ctrl_rec_p->cur_dmm_seq;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MepProcessDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To process the received DMM packet for Mep.
 * INPUT    : dmm_p - pointer to content of dmm pdu
 *            mep_p - pointer to content of mep to process pdu
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MepProcessDMM(
    CFM_ENGINE_DmPdu_T      *dmm_p,
    CFM_OM_MEP_T            *mep_p)
{
    UI8_T   dmm_multicast_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "mep id %ld recevied DMM", (long)mep_p->identifier);

    CFM_ENGINE_ASSEMBLE_DEST_MAC(dmm_multicast_mac,mep_p->md_p->level);

    if (  memcmp(dmm_p->header_p->dst_mac_a, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN)
        &&memcmp(dmm_p->header_p->dst_mac_a, dmm_multicast_mac, SYS_ADPT_MAC_ADDR_LEN)
       )
    {
        /*check if the mep's mac is the pdu's dst mac*/
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "drop packet not recevied mep's MAC");
        return FALSE;
    }

    /*transmit dmr*/
    CFM_ENGINE_XmitDMR(dmm_p, mep_p->md_p->level, mep_p->mac_addr_a);

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MepProcessDMR
 *-------------------------------------------------------------------------
 * PURPOSE  : To process the received DMR packet for Mep.
 * INPUT    : dmm_p - pointer to content of dmr pdu
 *            mep_p - pointer to content of mep to process pdu
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MepProcessDMR(
    CFM_ENGINE_DmPdu_T      *dmr_p,
    CFM_OM_MEP_T            *mep_p)
{
    /*check if the mep's mac is the pdu's dst mac*/
    if(memcmp(dmr_p->header_p->dst_mac_a, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "mep mac is not the pdu dst mac");
        return FALSE;
    }

    /*3. add to dmr result list
     */
    {
        CFM_ENGINE_AddDmrToResult(dmr_p);
    }

    return TRUE;
}

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MipProcessDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To process the received DMM packet for Mip.
 * INPUT    : dmm_p - pointer to content of dmm pdu
 *            mip_p - pointer to content of mip to process pdu
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MipProcessDMM(
    CFM_ENGINE_DmPdu_T      *dmm_p,
    CFM_OM_MIP_T            *mip_p)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "mip recevied DMM at lport =%ld", mip_p->lport);

    /*check if the mip's mac is the pdu's dst mac*/
    if(memcmp(dmm_p->header_p->dst_mac_a, mip_p->mac_address_a, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "drop packet not recevied mep's MAC");
        return FALSE;
    }

    /*transmit dmr*/
    CFM_ENGINE_XmitDMR(dmm_p, mip_p->md_p->level, mip_p->mac_address_a);
    return TRUE;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_VerifyAndDecomposeDM
 *-------------------------------------------------------------------------
 * PURPOSE  : To verify and extract the common dm header of received DMM or DMR packet
 * INPUT    : pdu_p    - pointer to content of received pdu
 *            header_p - pointer to content of cfm common header
 *            dm_p     - pointer to content of dm pdu (dmm/dmr)
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_VerifyAndDecomposeDM(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_DmPdu_T      *dm_p)
{
    {/* first tlv offset is 32 */
        if(header_p->first_tlv_offset != 32)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "first tlv offset fail");
            return FALSE;
        }
    }

    /* common header has no problem assign th header. */
    dm_p->header_p         = header_p;
    dm_p->tx_time_stampf_p = (CFM_TYPE_TimeReprFmt_T *) (pdu_p + 4);
    dm_p->rx_time_stampf_p = (CFM_TYPE_TimeReprFmt_T *) (pdu_p + 4 + 1 * sizeof(CFM_TYPE_TimeReprFmt_T));
    dm_p->tx_time_stampb_p = (CFM_TYPE_TimeReprFmt_T *) (pdu_p + 4 + 2 * sizeof(CFM_TYPE_TimeReprFmt_T));
    dm_p->rx_time_stampb_p = (CFM_TYPE_TimeReprFmt_T *) (pdu_p + 4 + 3 * sizeof(CFM_TYPE_TimeReprFmt_T));

    {/*option tlv*/
        UI8_T *tlv=NULL, tlv_type=0;

        tlv= (pdu_p + 4 + header_p->first_tlv_offset);
        tlv_type=*tlv;

        /*check the tlv type*/
        switch(tlv_type)
        {
        case CFM_TYPE_TLV_END:
            return TRUE;

        default:
            break;
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "fail");
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_VerifyAndDecomposeCfmMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : To verify and extract the common header of received pdu
 *              by CFM opcode.
 * INPUT    : pdu_p      - pointer to the received pdu packet
 *            header_p   - pointer to the common header of the received pdu
 * OUTPUT   : cfm_msg_p  - pointer to the header decomposed for pdu's opcode
 * RETURN   : TRUE/FALSE
 * NOTE     : for LBM/LBR, DMM/DMR, LTR, AIS
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_VerifyAndDecomposeCfmMsg(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_TmpCfmMsg_T  *cfm_msg_p)
{
    BOOL_T  ret = FALSE;

    switch (header_p->op_code)
    {
    case CFM_TYPE_OPCODE_LBM:
    case CFM_TYPE_OPCODE_LBR:
        ret = CFM_ENGINE_VerifyAndDecomposeLB(pdu_p, header_p, &cfm_msg_p->lb_pdu);
        break;
    case CFM_TYPE_OPCODE_DMM:
    case CFM_TYPE_OPCODE_DMR:
        ret = CFM_ENGINE_VerifyAndDecomposeDM(pdu_p, header_p, &cfm_msg_p->dm_pdu);
        break;
    case CFM_TYPE_OPCODE_LTR:
        ret = CFM_ENGINE_VerifyAndDecomposeLTR(pdu_p, header_p, &cfm_msg_p->ltr_pdu);
        break;
    case CFM_TYPE_OPCODE_AIS:
        /*verify Flags, we only process 1 sec(0x04) and 1 min(0x06)*/
        if ((header_p->flags&0x07) != 4 && (header_p->flags&0x07) != 6)
        {
            CFM_BD_MSG(header_p->pkt_dbg_flag,
                "Drop packet, lport=%ld, AIS flag is wrong", (long)header_p->lport);
        }
        else
        {
            ret = TRUE;
        }
        break;
    default:
        break;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MepProcessCfmMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : To process the received CFM Message for MEP.
 * INPUT    : pdu_p    - pointer to the received pdu packet
 *            header_p - pointer to the common header of the received pdu
 *            mep_p    - pointer to MEP to process pdu
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : for LBM/LBR, DMM/DMR, LTR, AIS
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MepProcessCfmMsg(
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_TmpCfmMsg_T  *cfm_msg_p,
    CFM_OM_MEP_T            *mep_p)
{
    BOOL_T  ret = FALSE;

    switch (header_p->op_code)
    {
    case CFM_TYPE_OPCODE_LBM:
        ret = CFM_ENGINE_MepProcessLBM(&cfm_msg_p->lb_pdu, mep_p);
        break;
    case CFM_TYPE_OPCODE_LBR:
        ret = CFM_ENGINE_MepProcessLBR(&cfm_msg_p->lb_pdu, mep_p);
        break;
    case CFM_TYPE_OPCODE_DMM:
        ret = CFM_ENGINE_MepProcessDMM(&cfm_msg_p->dm_pdu, mep_p);
        break;
    case CFM_TYPE_OPCODE_DMR:
        ret = CFM_ENGINE_MepProcessDMR(&cfm_msg_p->dm_pdu, mep_p);
        break;
    case CFM_TYPE_OPCODE_LTR:
        ret = CFM_ENGINE_MepProcessLTR(&cfm_msg_p->ltr_pdu, mep_p);
        break;
    case CFM_TYPE_OPCODE_AIS:
        ret = CFM_ENGINE_MepProcessAIS(header_p, mep_p);
        break;
    default:
        break;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_MipProcessCfmMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : To process the received CFM Message for MIP.
 * INPUT    : pdu_p    - pointer to the received pdu packet
 *            header_p - pointer to the common header of the received pdu
 *            mip_p    - pointer to MIP to process pdu
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : for LBM/LBR, DMM/DMR, LTR, AIS
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_MipProcessCfmMsg(
    CFM_ENGINE_PduHead_T    *header_p,
    CFM_ENGINE_TmpCfmMsg_T  *cfm_msg_p,
    CFM_OM_MIP_T            *mip_p)
{
    BOOL_T  ret = FALSE;

    CFM_BD_MSG(header_p->pkt_dbg_flag, "(%ld) %s MHF process %s",
        (long)mip_p->lport,
        (mip_p->lport == header_p->lport) ? "down" : "up",
        CFM_ENGINE_LocalGetOpcodeStr(header_p->op_code));

    switch (header_p->op_code)
    {
    case CFM_TYPE_OPCODE_LBM:
        /* MIP only process unicast-LBM
         */
        ret = CFM_ENGINE_MipProcessLBM(&cfm_msg_p->lb_pdu, mip_p);
        break;
    case CFM_TYPE_OPCODE_DMM:
    case CFM_TYPE_OPCODE_LBR:
    case CFM_TYPE_OPCODE_DMR:
    case CFM_TYPE_OPCODE_LTR:
    case CFM_TYPE_OPCODE_AIS:
    default:
        CFM_BD_MSG(header_p->pkt_dbg_flag, "transparent for MIP");
        break;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ProcessCfmMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : To process the received CFM Message
 * INPUT    : pdu_p    - pointer to the received pdu packet
 *            header_p - pointer to the common header of the received pdu
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : for LBM/LBR, DMM/DMR, LTR, AIS
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ProcessCfmMsg(
    UI8_T                   *pdu_p,
    CFM_ENGINE_PduHead_T    *header_p)
{
    CFM_ENGINE_TmpCfmMsg_T  tmp_msg;
    CFM_OM_MIP_T            mip;
    CFM_OM_MEP_T            *mep_p=&mep_tmp1_g;
    CFM_OM_MD_T             *md_p=NULL;
    CFM_OM_MA_T             *ma_p=NULL;
    CFM_TYPE_MpType_T       mp_type=CFM_TYPE_MP_TYPE_NONE;
    UI32_T                  nxt_mep_id=0, nxt_lport=0;
    BOOL_T                  tmp_ret;

    /* use tmp_msg = {{0}} will only initial the 1st member,
     * and it may cause error for other uninitialized member.
     */
    memset(&tmp_msg, 0, sizeof(tmp_msg));

    /* 1. verify and decompose PDU
     */
    if (FALSE == CFM_ENGINE_VerifyAndDecomposeCfmMsg(
                    pdu_p, header_p, &tmp_msg))
    {
        CFM_BD_MSG(header_p->pkt_dbg_flag, "decompose failed");
        return FALSE;
    }

    /* 2. process PDU by down MEP/MIP of ingress port
     */
    mp_type= CFM_ENGINE_GetMpProcessRcvdPacket(header_p, mep_p, &mip);
    switch (mp_type)
    {
    case CFM_TYPE_MP_TYPE_MEP:
        if (CFM_TYPE_MP_DIRECTION_UP == mep_p->direction)
        {
            CFM_BD_MSG(header_p->pkt_dbg_flag,
                "drop packet, received by up MEP from back side");
            return FALSE;
        }

        tmp_ret = CFM_ENGINE_MepProcessCfmMsg(header_p, &tmp_msg, mep_p);
        return tmp_ret;

    case CFM_TYPE_MP_TYPE_MIP:
        tmp_ret = CFM_ENGINE_MipProcessCfmMsg(header_p, &tmp_msg, &mip);
        if (TRUE == tmp_ret)
        {
            return TRUE;
        }
        break;

    case CFM_TYPE_MP_TYPE_NONE:
        /* if received port has a higher level MEP, return FALSE;
         */
        if (TRUE == CFM_ENGINE_IsExistHigherLevelMp(
                        header_p->lport,
                        header_p->md_level,
                        header_p->tag_info&0x0fff,
                        CFM_TYPE_MP_DIRECTION_UP_DOWN,
                        FALSE,
                        mep_p,
                        NULL))
        {
            CFM_BD_MSG(header_p->pkt_dbg_flag, "higher level MEP exists");
            return FALSE;
        }
        break;
    }

    /* 3. drop if ingress port is blocking
     */
    if (FALSE == header_p->is_ing_lport_fwd)
    {
        CFM_BD_MSG(header_p->pkt_dbg_flag, "dropped due to ingress port is blocked");
        return TRUE;
    }

    /* 4. process PDU by up MEP/MIP of other port,
     */
    if (TRUE == CFM_OM_GetMdMaByLevelVid(
                    header_p->md_level, header_p->tag_info&0x0fff, &md_p, &ma_p))
    {
        while (TRUE == CFM_OM_GetNextMep(
                        md_p->index, ma_p->index, nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
        {
            if ((md_p->index != mep_p->md_index)||(ma_p->index != mep_p->ma_index))
            {
                break;
            }

            if (CFM_TYPE_MP_DIRECTION_UP == mep_p->direction)
            {
                tmp_ret = CFM_ENGINE_MepProcessCfmMsg(header_p, &tmp_msg, mep_p);
                if (TRUE == tmp_ret)
                {
                    /* for multicast should continue to process/forward
                     */
                    if (0x01 != header_p->dst_mac_a[0])
                    {
                        return TRUE;
                    }
                }
            }

            nxt_mep_id = mep_p->identifier;
        }

        /* MIP only process unicast-LBM
         * from Y.1731,
         *  A MIP is transparent to the frames with ETH-DM ...
         *  A MIP is transparent to the Multicast frames with ETH-LB request ...
         *  A MIP is transparent to frames with ETH-AIS information
         */
        switch (header_p->op_code)
        {
        case CFM_TYPE_OPCODE_LBM:
            if (0x01 == header_p->dst_mac_a[0])
            {
                CFM_BD_MSG(header_p->pkt_dbg_flag, "transparent for MIP");
                break;
            }

            while (TRUE == CFM_OM_GetNextMip(md_p->index, ma_p->index, nxt_lport, CFM_OM_MD_MA_LPORT_KEY, &mip))
            {
                if ((md_p->index != mip.md_index)||(ma_p->index != mip.ma_index))
                {
                    break;
                }

                tmp_ret = CFM_ENGINE_MipProcessCfmMsg(header_p, &tmp_msg, &mip);
                if (TRUE == tmp_ret)
                {
                    return TRUE;
                }

                nxt_lport=mip.lport;
            }
            break;
        default:
            CFM_BD_MSG(header_p->pkt_dbg_flag, "transparent for MIP");
            break;
        }
    } /* if(TRUE == CFM_OM_GetMdMaByLevelVid(...) */

    /* 5. forwarding to other port
     */
    tmp_ret = CFM_ENGINE_ForwardUnicastPdu(
                header_p, pdu_p, header_p->pdu_length, FALSE);

    CFM_BD_MSG(header_p->pkt_dbg_flag,
        "No ports process this msg, forwarding (ret-%d).", tmp_ret);

    return tmp_ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To transmit a dmm.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_p   - pointer to content of destination mac
 *            md_name_p   - pointer to content of md name
 *            md_name_len - the md name length
 *            ma_name_p   - pointer to content of ma name
 *            ma_name_len - the ma name length
 *            pkt_size    - the packet size to transmit
 *            pkt_pri     - the priority to transmit packet
 *            is_test     - test only if it's TRUE
 * OUTPUT   : tx_time_10ms_p - pointer to content of tx time stamp in 10ms
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_XmitDMM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       *dst_mac_p,
    UI8_T       *md_name_p,
    UI32_T      md_name_len,
    UI8_T       *ma_name_p,
    UI32_T      ma_name_len,
    UI32_T      pkt_size,
    UI16_T      pkt_pri,
    UI32_T      *tx_time_10ms_p,
    BOOL_T      is_test)
{
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;
    UI8_T           *pdu_p=NULL/* *dulicate_pdu_p=NULL*/;
    UI32_T          egress_port=0;
    UI16_T          pdu_len = 0;
    UI8_T           target_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

    /* 1. first find the remote mep or the local mep to send the lbm
     */
    if(FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_p, md_name_len, ma_name_p, ma_name_len, &md_p, &ma_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "can't get the md/ma by md/ma name");
        return FALSE;
    }

    if (  (dst_mep_id == 0)
        &&(FALSE == CFM_ENGINE_LocalIsValidDstMac(
                    dst_mac_p, md_p->level, TRUE))
       )
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "invalid mac");
        return FALSE;
    }

    if(FALSE == CFM_ENGINE_FindTargetMacAndMep(md_p->index, ma_p->index,
                    ma_p->primary_vid, dst_mep_id, dst_mac_p,
                    CFM_BACKDOOR_DEBUG_FLAG_DM, target_mac, &egress_port, mep_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "can't find target mac and mep");
        return FALSE;
    }

    /* if choose to use specify mep to transmit dmm
     */
    if((0!= src_mep_id)&&(mep_p->identifier != src_mep_id))
    {
        if(FALSE == CFM_OM_GetMep(md_p->index, ma_p->index,
                        src_mep_id , 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "can't get mep");
            return FALSE;
        }

        if(CFM_TYPE_MP_DIRECTION_DOWN == mep_p->direction)
        {
            egress_port = mep_p->lport;
        }
    }

    /* should not use mep's da for destination mac
     */
    if (0 == memcmp(mep_p->mac_addr_a, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "target is mep mac");
        return FALSE;
    }

    /* 2. check cfm status.
     */
    {
        CFM_TYPE_CfmStatus_T    global_status=CFM_TYPE_CFM_STATUS_DISABLE,
                                port_status  =CFM_TYPE_CFM_STATUS_DISABLE;

        CFM_OM_GetCFMGlobalStatus(&global_status);
        CFM_OM_GetCFMPortStatus(mep_p->lport, &port_status);

        /*if the port's and global status not enable, the ccm won't send*/
        if((CFM_TYPE_CFM_STATUS_ENABLE != global_status)||(CFM_TYPE_CFM_STATUS_ENABLE!=port_status))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "mep global status is fail");
            return FALSE;
        }
    }

    if (FALSE == is_test)
    {
        /* 3. start to send the dmm
         */
        pdu_p = (UI8_T *) L_MM_Malloc(CFM_TYPE_MAX_FRAME_SIZE,
                            L_MM_USER_ID2(SYS_MODULE_CFM, CFM_TYPE_XMITDMM));

        if(NULL == pdu_p)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "can't allocate memory");
            return FALSE;
        }

        CFM_ENGINE_ConstructDMPDU(mep_p, CFM_TYPE_OPCODE_DMM, NULL, 0, pdu_p, &pdu_len, tx_time_10ms_p);

        if ((pdu_len < pkt_size) && (pkt_size <= CFM_TYPE_MAX_FRAME_SIZE))
        {
            memset (pdu_p+pdu_len, 0, pkt_size-pdu_len);
            pdu_len = pkt_size;
        }

        /* send dmm
         */
        if(CFM_TYPE_MP_DIRECTION_DOWN == mep_p->direction)
        {
            CFM_ENGINE_XmitPDU(
                mep_p->lport,               mep_p->primary_vid,
                mep_p->md_p->level,         pkt_pri,
                FALSE,                      target_mac,
                mep_p->mac_addr_a,          CFM_TYPE_MP_DIRECTION_DOWN,
                pdu_p,                      pdu_len);

            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "DMM tx thorough port %ld", (long)mep_p->lport);
        }
        else if (0 != egress_port)
        {
            CFM_ENGINE_XmitPDU(
                egress_port,                mep_p->primary_vid,
                mep_p->md_p->level,         pkt_pri,
                FALSE,                      target_mac,
                mep_p->mac_addr_a,          CFM_TYPE_MP_DIRECTION_UP,
                pdu_p,                      pdu_len);

            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "DMM tx thorough port %ld", (long)mep_p->lport);
        }
        else /*mep up and no egress port*/
        {
            CFM_ENGINE_FloodPDU(
                mep_p->lport,       ((mep_p->primary_vid) | pkt_pri << 13), target_mac,
                mep_p->mac_addr_a,  pdu_p,              pdu_len,    TRUE);

            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "DMM flood through port %ld", (long)mep_p->lport);
        }

        L_MM_Free(pdu_p);
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitDMR
 *-------------------------------------------------------------------------
 * PURPOSE  : To transmit a dmr.
 * INPUT    : dmm_p      - pointer to content of dmm pdu
 *            level      - the md level
 *            src_addr_p - pointer to content of source mac
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_XmitDMR(
    CFM_ENGINE_DmPdu_T  *dmm_p,
    CFM_TYPE_MdLevel_T  level,
    UI8_T               *src_addr_p)
{
    UI8_T   *tran_pdu_p =NULL;
    UI16_T  tran_pdu_len;
    BOOL_T  ret = FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_DM, "DMR transmit throught port %ld", (long)dmm_p->header_p->lport);

    tran_pdu_p = (UI8_T *) L_MM_Malloc(CFM_TYPE_MAX_FRAME_SIZE,
                        L_MM_USER_ID2(SYS_MODULE_CFM, CFM_TYPE_XMITDMR));

    if (NULL != tran_pdu_p)
    {
        CFM_ENGINE_ConstructDMPDU(
            NULL, CFM_TYPE_OPCODE_DMR, (UI8_T *)(dmm_p->tx_time_stampf_p)-4,
            dmm_p->header_p->pdu_length, tran_pdu_p, &tran_pdu_len, NULL);

        /* trnsmit the DMR
         */
        CFM_ENGINE_XmitPDU(dmm_p->header_p->lport,  dmm_p->header_p->tag_info&0x0fff,
                           level,                   (dmm_p->header_p->tag_info&0xE000)>>13,
                           FALSE,                   dmm_p->header_p->src_mac_a,
                           src_addr_p,              CFM_TYPE_MP_DIRECTION_DOWN,
                           tran_pdu_p,              dmm_p->header_p->pdu_length);

        L_MM_Free(tran_pdu_p);
        ret = TRUE;
    }

    return ret;
}

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
    UI32_T      ma_name_len)
{
    CFM_OM_LbmCtrlRec_T     *lbm_ctrl_rec_p;
    BOOL_T                  ret = FALSE;

    lbm_ctrl_rec_p = CFM_OM_GetLbmCtrlRecPtr();

    if (NULL != lbm_ctrl_rec_p)
    {
        if (TRUE == lbm_ctrl_rec_p->is_busy)
        {
            /* check if abort the correct mission...
             */
            if (  /* (lbm_ctrl_rec_p->src_mep_id == src_mep_id)
                   * src_mep_id will be changed to the real mep id which send the lbm,
                   * so it can not be used to compare here.
                   */
                  (lbm_ctrl_rec_p->dst_mep_id == dst_mep_id)
                &&(memcmp(lbm_ctrl_rec_p->dst_mac_ar, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN) == 0)
                &&(memcmp(lbm_ctrl_rec_p->md_name_ar, md_name_p, md_name_len) == 0)
                &&(memcmp(lbm_ctrl_rec_p->ma_name_ar, ma_name_p, ma_name_len) == 0)
               )
            {
                CFM_TIMER_StopTimer(lbm_ctrl_rec_p->timeout_timer_idx);

                lbm_ctrl_rec_p->res_bmp = CFM_TYPE_THRPT_MEASURE_RES_SENDPKTS       |
                                          CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_1SEC  |
                                          CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_TOUT;
                lbm_ctrl_rec_p->is_busy = FALSE;

                CFM_ENGINE_FreeLbmTransmitData(lbm_ctrl_rec_p);
                ret = TRUE;
            }
        }
    }

    return ret;
}

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
    UI16_T      pkt_pri)
{
    CFM_OM_LbmCtrlRec_T     *lbm_ctrl_rec_p;
    CFM_OM_MEP_T            *src_mep_p=&mep_tmp1_g;
    UI32_T                  egress_port;
    UI8_T                   target_mac[SYS_ADPT_MAC_ADDR_LEN];
    BOOL_T                  ret = FALSE;

    lbm_ctrl_rec_p = CFM_OM_GetLbmCtrlRecPtr();

    if (NULL != lbm_ctrl_rec_p)
    {
        if (FALSE == lbm_ctrl_rec_p->is_busy)
        {
            lbm_ctrl_rec_p->is_busy = TRUE;

            if (TRUE == CFM_ENGINE_GetEgressPortToXmitCfmPdu(
                            src_mep_id, dst_mep_id, dst_mac_p,
                            md_name_p, md_name_len,
                            ma_name_p, ma_name_len,
                            &egress_port, target_mac, src_mep_p))
            {
                memcpy(lbm_ctrl_rec_p->dst_mac_ar, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN);
                memcpy(lbm_ctrl_rec_p->md_name_ar, md_name_p, md_name_len+1);
                memcpy(lbm_ctrl_rec_p->ma_name_ar, ma_name_p, ma_name_len+1);

                lbm_ctrl_rec_p->src_mep_id = src_mep_id;
                lbm_ctrl_rec_p->dst_mep_id = dst_mep_id;
                lbm_ctrl_rec_p->counts     = counts;
                lbm_ctrl_rec_p->timeout    = timeout;
                lbm_ctrl_rec_p->pkt_size   = pkt_size - 18; /* not including (da + sa + eth + fcs) = total size - 18 */
                lbm_ctrl_rec_p->md_name_len= md_name_len;
                lbm_ctrl_rec_p->ma_name_len= ma_name_len;
                lbm_ctrl_rec_p->time_pass       = 0;
                lbm_ctrl_rec_p->real_send_counts= 0;
                lbm_ctrl_rec_p->rcv_in_1sec     = 0;
                lbm_ctrl_rec_p->rcv_in_time     = 0;
                lbm_ctrl_rec_p->res_bmp         = 0;
                lbm_ctrl_rec_p->pattern         = pattern;
                lbm_ctrl_rec_p->pkt_pri         = pkt_pri;

                CFM_TIMER_StartTimer(lbm_ctrl_rec_p->timeout_timer_idx);
                ret = TRUE;
            }
            else
            {
                /* test failed, return FALSE to abort the mission
                 */
                lbm_ctrl_rec_p->is_busy = FALSE;
            }
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "ret-%d", ret);
    return ret;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_BgXmitLBM_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : To transmit lbm in background when callback from the timer.
 * INPUT    : rec_p - pointer to content of global lbm control record
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_BgXmitLBM_CallBack(
    void    *rec_p)
{
    CFM_OM_LbmCtrlRec_T *lbm_ctrl_rec_p;
    BOOL_T              ret = FALSE;

    lbm_ctrl_rec_p = CFM_OM_GetLbmCtrlRecPtr();

    if (NULL != lbm_ctrl_rec_p)
    {
        if (TRUE == lbm_ctrl_rec_p->is_busy)
        {
            if (lbm_ctrl_rec_p->res_bmp == 0)
            {
                CFM_ENGINE_SendLbmForThrptMeasure(lbm_ctrl_rec_p);
            }
            else
            {
                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "time_pass -%d", lbm_ctrl_rec_p->time_pass);

                if (lbm_ctrl_rec_p->time_pass <= lbm_ctrl_rec_p->timeout)
                {
                    lbm_ctrl_rec_p->time_pass++;
                }

                if ((lbm_ctrl_rec_p->time_pass > 1) ||
                    (lbm_ctrl_rec_p->rcv_in_1sec == lbm_ctrl_rec_p->real_send_counts))
                {
                    lbm_ctrl_rec_p->res_bmp |= CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_1SEC;
                }

                if ((lbm_ctrl_rec_p->time_pass > lbm_ctrl_rec_p->timeout) ||
                    ((lbm_ctrl_rec_p->rcv_in_1sec + lbm_ctrl_rec_p->rcv_in_time) == lbm_ctrl_rec_p->real_send_counts))
                {
                    CFM_TIMER_StopTimer(lbm_ctrl_rec_p->timeout_timer_idx);
                    lbm_ctrl_rec_p->res_bmp |= CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_TOUT;
                    lbm_ctrl_rec_p->is_busy = FALSE;

                    CFM_ENGINE_FreeLbmTransmitData(lbm_ctrl_rec_p);
                }
            }

            ret = TRUE;
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "ret-%d", ret);
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_FreeLbmTransmitData
 *-------------------------------------------------------------------------
 * PURPOSE  : To free lbm transmit data buffer kept in mep.
 * INPUT    : lbm_ctrl_rec_p - pointer to content of global lbm control record
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_FreeLbmTransmitData(
    CFM_OM_LbmCtrlRec_T     *lbm_ctrl_rec_p)
{
    CFM_OM_MEP_T    *mep_p=&mep_tmp1_g;
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;

    if ((NULL != lbm_ctrl_rec_p) && (lbm_ctrl_rec_p->src_mep_id != 0))
    {
        if (FALSE == CFM_OM_GetMdMaByMdMaName(
                        lbm_ctrl_rec_p->md_name_ar, lbm_ctrl_rec_p->md_name_len,
                        lbm_ctrl_rec_p->ma_name_ar, lbm_ctrl_rec_p->ma_name_len,
                        &md_p,                      &ma_p))
        {
            return;
        }

        if (TRUE == CFM_OM_GetMep(md_p->index, ma_p->index,
                        lbm_ctrl_rec_p->src_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p))
        {
                memset(mep_p->transmit_lbm_data_a, 0, sizeof(mep_p->transmit_lbm_data_a));
                CFM_OM_StoreMep(mep_p);
        }
    }

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetEgressPortToXmitCfmPdu
 *-------------------------------------------------------------------------
 * PURPOSE  : To get egress port ot transmit cfm pdu.
 * INPUT    : src_mep_id   - the source mep_id
 *            dst_mep_id   - the destination mep_id
 *            dst_mac_p    - pointer to content of destination mac
 *            md_name_p    - pointer to content of md name
 *            md_name_len  - the md name length
 *            ma_name_p    - pointer to content of ma name
 *            ma_name_len  - the ma name length
 * OUTPUT   : egress_port_p- pointer to content of egress port
 *            target_mac_p - pointer to content of target mac
 *            src_mep_pp   - pointer to content of source mep's pointer
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_GetEgressPortToXmitCfmPdu(
    UI32_T          src_mep_id,
    UI32_T          dst_mep_id,
    UI8_T           *dst_mac_p,
    UI8_T           *md_name_p,
    UI32_T          md_name_len,
    UI8_T           *ma_name_p,
    UI32_T          ma_name_len,
    UI32_T          *egress_port_p,
    UI8_T           *target_mac_p,
    CFM_OM_MEP_T    *src_mep_p)
{
    CFM_OM_MD_T     *md_p=NULL;
    CFM_OM_MA_T     *ma_p=NULL;

    /* 1. first find the remote mep or the local mep to send the cfm pdu
     */
    if (FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_p, md_name_len, ma_name_p, ma_name_len, &md_p, &ma_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "can't get the md/ma by md/ma name");
        return FALSE;
    }

    if (  (dst_mep_id == 0)
        &&(FALSE == CFM_ENGINE_LocalIsValidDstMac(
                    dst_mac_p, md_p->level, TRUE))
       )
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "invalid mac");
        return FALSE;
    }

    if (FALSE == CFM_ENGINE_FindTargetMacAndMep(
                    md_p->index, ma_p->index, ma_p->primary_vid, dst_mep_id,
                    dst_mac_p, CFM_BACKDOOR_DEBUG_FLAG_TM,
                    target_mac_p, egress_port_p, src_mep_p))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "can't find target mac and mep");
        return FALSE;
    }

    /* if choose to use specify mep to transmit cfm pdu
     */
    if ((0 != src_mep_id) && (src_mep_p->identifier != src_mep_id))
    {
        if (FALSE == CFM_OM_GetMep(md_p->index, ma_p->index,
                        src_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, src_mep_p))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "can't get mep");
            return FALSE;
        }

        if (CFM_TYPE_MP_DIRECTION_DOWN == src_mep_p->direction)
        {
            *egress_port_p = src_mep_p->lport;
        }
    }

    /* should not use mep's da for destination mac
     */
    if (0 == memcmp(src_mep_p->mac_addr_a, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "target is mep mac");
        return FALSE;
    }

    /* 2. check cfm status.
     */
    {
        CFM_TYPE_CfmStatus_T    global_status=CFM_TYPE_CFM_STATUS_DISABLE,
                                port_status  =CFM_TYPE_CFM_STATUS_DISABLE;

        CFM_OM_GetCFMGlobalStatus(&global_status);
        CFM_OM_GetCFMPortStatus(src_mep_p->lport, &port_status);

        /* if the port's and global status not enable, the lbm won't send
         */
        if ((CFM_TYPE_CFM_STATUS_ENABLE != global_status) ||
            (CFM_TYPE_CFM_STATUS_ENABLE != port_status))
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "mep global status is fail");
            return FALSE;
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_SendLbmForThrptMeasure
 *-------------------------------------------------------------------------
 * PURPOSE  : To transmit lbm as many as possible in 1 sec for throughput measure.
 * INPUT    : lbm_ctrl_rec_p - pointer to content of global lbm control record
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_SendLbmForThrptMeasure(
    CFM_OM_LbmCtrlRec_T     *lbm_ctrl_rec_p)
{
    CFM_OM_MEP_T    *src_mep_p = &mep_tmp1_g;
    UI8_T           *pdu_p;
    UI32_T          egress_port, cur_time_10ms, beg_time_10ms,
                    trans_id, loop_cnt=0;
    UI16_T          pdu_len =0;
    UI8_T           target_mac[SYS_ADPT_MAC_ADDR_LEN];
    BOOL_T          ret = FALSE;

    if (NULL == lbm_ctrl_rec_p)
    {
        return FALSE;
    }

    /* 1. collect needed info
     */
    if (TRUE == CFM_ENGINE_GetEgressPortToXmitCfmPdu(
                    lbm_ctrl_rec_p->src_mep_id, lbm_ctrl_rec_p->dst_mep_id,
                    lbm_ctrl_rec_p->dst_mac_ar,
                    lbm_ctrl_rec_p->md_name_ar,
                    lbm_ctrl_rec_p->md_name_len,
                    lbm_ctrl_rec_p->ma_name_ar,
                    lbm_ctrl_rec_p->ma_name_len,
                    &egress_port, target_mac, src_mep_p))
    {
        if (FALSE == src_mep_p->tramsmit_lbm_status)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TX_PACKET_FLOW, "transmit lbm status is false");
        }
        else
        {
            ret = TRUE;
        }
    }

    /* 2. construct pdu
     */
    if (FALSE == ret)
    {
        return FALSE;
    }

    src_mep_p->transmit_lbm_result_oK = FALSE;
    pdu_p = src_mep_p->transmit_lbm_data_a;

    memset(pdu_p, 0, CFM_TYPE_MAX_FRAME_SIZE);

    trans_id = src_mep_p->next_lbm_trans_id;

    /* calculate the packet length of LB without data TLV
     */
    CFM_ENGINE_ConstructLBPDU(
        pdu_p, &pdu_len, src_mep_p, CFM_TYPE_OPCODE_LBM, 0, 0);

    /* add data TLV if packet size is large enough
     */
    if ((lbm_ctrl_rec_p->pkt_size - pdu_len) >= 4)
    {
        /* data tlv: 1 (t)+ 2(l) + ?(v) = 3 + ?
         */
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "data tlv(%lu), pkt_size(%lu) >= pdu_len(%lu)+4",
            (long)src_mep_p->transmit_lbm_data_tlv_length, (long)lbm_ctrl_rec_p->pkt_size, (long)pdu_len);

        src_mep_p->transmit_lbm_data_tlv_length = lbm_ctrl_rec_p->pkt_size - pdu_len - 3;
        pdu_len =0;
        CFM_ENGINE_ConstructLBPDU(
            pdu_p, &pdu_len, src_mep_p, CFM_TYPE_OPCODE_LBM,
            src_mep_p->transmit_lbm_data_tlv_length, lbm_ctrl_rec_p->pattern);
    }
    else
    {
        if (lbm_ctrl_rec_p->pkt_size > pdu_len)
        {
            pdu_len = lbm_ctrl_rec_p->pkt_size;
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "no data tlv, pkt_size(%lu)", (long)lbm_ctrl_rec_p->pkt_size);
        }
        else
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "pdu_len(%lu)> pkt_size(%lu)", (long)pdu_len, (long)lbm_ctrl_rec_p->pkt_size);
        }
    }

    /* 3. loop to send pkt in 1 sec
     */
    lbm_ctrl_rec_p->beg_trans_id = trans_id;
    if (src_mep_p->lbm_current_recevied_seq_number != trans_id -1)
    {
        src_mep_p->lbm_current_recevied_seq_number = trans_id -1;
    }

    beg_time_10ms = SYS_TIME_GetSystemTicksBy10ms();
    for (;loop_cnt < lbm_ctrl_rec_p->counts;)
    {
        if (((++loop_cnt) & 0x7) == 0)
        {
            cur_time_10ms = SYS_TIME_GetSystemTicksBy10ms();
            if ((cur_time_10ms - beg_time_10ms) >= 100)
            {
                loop_cnt--;
                break;
            }
        }

        {
            UI32_T  tmp_trans_id = L_STDLIB_Hton32(trans_id);

            trans_id++;
            memcpy (pdu_p+4, &tmp_trans_id, sizeof(tmp_trans_id));
            /* the code below has alignment issue on ARM platform
             *  *((UI32_T *)(pdu_p+4)) = L_STDLIB_Hton32(trans_id++);
             */
        }

        CFM_ENGINE_XmitPduBySrcMep(
            src_mep_p,  egress_port, lbm_ctrl_rec_p->pkt_pri,
            target_mac, pdu_p,       pdu_len);
    }

    src_mep_p->next_lbm_trans_id        = trans_id;
    src_mep_p->transmit_lbm_seq_number  = trans_id-1;
    src_mep_p->transmit_lbm_result_oK   = TRUE;
    CFM_OM_StoreMep(src_mep_p);

    if (lbm_ctrl_rec_p->src_mep_id == 0)
    {
        lbm_ctrl_rec_p->src_mep_id = src_mep_p->identifier;
    }

    lbm_ctrl_rec_p->end_trans_id        = trans_id-1;
    lbm_ctrl_rec_p->real_send_counts    = loop_cnt;
    lbm_ctrl_rec_p->res_bmp             = CFM_TYPE_THRPT_MEASURE_RES_SENDPKTS;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TM, "rsc-%lu", (long)loop_cnt);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_XmitPduBySrcMep
 *-------------------------------------------------------------------------
 * PURPOSE  : To transmit cfm pdu by specified mep.
 * INPUT    : src_mep_p    - pointer to content of source mep
 *            egress_port  - egress port to transmit
 *            priority     - priority of pdu to transmit
 *            target_mac_p - pointer to content of target mac
 *            pdu_p        - pointer to content of pdu to transmit
 *            pdu_len      - pdu length to transmit
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_XmitPduBySrcMep(
    CFM_OM_MEP_T    *src_mep_p,
    UI32_T          egress_port,
    UI32_T          priority,
    UI8_T           *target_mac_p,
    UI8_T           *pdu_p,
    UI32_T          pdu_len)
{
    /*send pdu */
    if (0 != egress_port)
    {
        CFM_ENGINE_XmitPDU(
            egress_port,                src_mep_p->primary_vid,
            src_mep_p->md_p->level,     priority,
            FALSE,                      target_mac_p,
            src_mep_p->mac_addr_a,      src_mep_p->direction,
            pdu_p,                      pdu_len);

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TX_PACKET_FLOW, "pdu transmitted thorough port %ld", (long)egress_port);
    }
    else /*mep up and no egress port*/
    {
        CFM_ENGINE_FloodPDU(
            src_mep_p->lport,       ((src_mep_p->primary_vid) | priority << 13),     target_mac_p,
            src_mep_p->mac_addr_a,  pdu_p,                      pdu_len,    TRUE);

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TX_PACKET_FLOW, "pdu flood through port %ld", (long)src_mep_p->lport);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ConstructMref
 *-------------------------------------------------------------------------
 * PURPOSE  : To transmit cfm pdu by specified mep.
 * INPUT    : mref_pp   - mref pinter's pointer
 *            buf_p     - the buffer to transmit pdu
 *            pdu_p     - the packet content
 *            pdu_len   - the pdu length
 *            tx_reason - the tx reason
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ConstructMref(
    L_MM_Mref_Handle_T  **mref_pp,
    UI8_T               *buf_p,
    UI8_T               *pdu_p,
    UI16_T              pdu_len,
    CFM_TYPE_TxReason_T tx_reason)
{
    UI32_T len=0;

    if(NULL == ((*mref_pp)=L_MM_AllocateTxBuffer(pdu_len, L_MM_USER_ID2(SYS_MODULE_CFM, tx_reason))))
    {
        return FALSE;
    }

    (*mref_pp)->current_usr_id = SYS_MODULE_CFM;
    (*mref_pp)->next_usr_id = SYS_MODULE_L2MUX;

    buf_p = (UI8_T *)L_MM_Mref_GetPdu(*mref_pp, &len);

    if(NULL == buf_p)
    {
        if(!L_MM_Mref_Release(mref_pp))
        {
            CFM_BD_MSG_S("can't free mref");
        }
        return FALSE;
    }

    memcpy(buf_p, pdu_p, pdu_len);

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_GetNextVidFromMaVidBmp
 *-------------------------------------------------------------------------
 * PURPOSE  : To get next availabled vid different from current primary vid
 *              in ma's vid list
 * INPUT    : ma_p  - pointer to the contents of ma
 * OUTPUT   : vid_p - pointer to the buffer of return vid
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_GetNextVidFromMaVidBmp(
                                CFM_OM_MA_T *ma_p,
                                UI16_T      *vid_p)
{
    UI16_T  nxt_vid, loop_idx, loop_len = sizeof(ma_p->vid_bitmap_a);
    UI8_T   tmp_bmp, bit_pos, bit_mask;
    BOOL_T  ret = FALSE;

    if ((NULL != ma_p) && (NULL != vid_p))
    {
        if (ma_p->num_of_vids > 1)
        {
            for (loop_idx =0; (loop_idx <loop_len) && (FALSE == ret); loop_idx++)
            {
                if (ma_p->vid_bitmap_a[loop_idx] != 0)
                {
                    tmp_bmp  = ma_p->vid_bitmap_a[loop_idx];
                    bit_mask = 0x80;

                    for (bit_pos =1; (bit_pos <=8) && (tmp_bmp != 0); bit_pos++)
                    {
                        if (tmp_bmp & bit_mask)
                        {
                            nxt_vid = 8 * loop_idx + bit_pos;

                            if (nxt_vid != ma_p->primary_vid)
                            {
                                ret = TRUE;
                                *vid_p = nxt_vid;
                                break;
                            }
                            tmp_bmp ^= bit_mask;
                        }
                        bit_mask >>= 1;
                    }
                }
            }
        }
    }

    return ret;
}/* End of CFM_ENGINE_GetNextVidFromMaVidBmp*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_ResetRcvdMepStateOfRemoteMep
 *-------------------------------------------------------------------------
 * PURPOSE  : To reset the state of local MEP which
 *              received the specifed remote mep's CCM.
 * INPUT    : remote_mep_p - pointer to the content of remote mep
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_ResetRcvdMepStateOfRemoteMep(
    CFM_OM_REMOTE_MEP_T     *remote_mep_p)
{
    CFM_OM_MEP_T    *tmp_mep_p=&mep_tmp2_g;
    UI32_T          nxt_mep_id =0;

    /*this remote mep still not received any CCM and timeout, so clear the flag at each mep, or the RDI will always on*/
    if (0 == remote_mep_p->rcvd_mep_id)
    {
        /* this remote mep has not been received by any local mep,
         *   if (it's state not ok)
         *     update the ccm loss cnt
         *
         * local mep must guarantee the loss cnt is correct
         * if it's created after the remote mep enter failed
         */
        if (CFM_TYPE_REMOTE_MEP_STATE_FAILD == remote_mep_p->machine_state)
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "");

            while (TRUE == CFM_OM_GetNextMep(
                            remote_mep_p->md_index, remote_mep_p->ma_index,
                            nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, tmp_mep_p))
            {
                if ((tmp_mep_p->md_index != remote_mep_p->md_index)||
                    (tmp_mep_p->ma_index != remote_mep_p->ma_index))
                {
                    break;
                }

                /* clear highest defect in show fault-notify-generator
                 */
                if (tmp_mep_p->rmep_ccm_loss_cnt > 0)
                    tmp_mep_p->rmep_ccm_loss_cnt--;

                CFM_ENGINE_UPD_SOME_RMEP_CCM_DEF(tmp_mep_p);
                CFM_ENGINE_UpdateMepHighestDefect(tmp_mep_p);
                CFM_OM_StoreMep(tmp_mep_p);

                nxt_mep_id = tmp_mep_p->identifier;
            }
        }
        else
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE, "");
        }
    }
    else
    {
        /* this remote mep has been received by a local mep,
         *   update the mac status/rdi cnt for the local mep
         *   if (it's state not ok)
         *     update the ccm loss cnt
         *
         */
        if (TRUE == CFM_OM_GetMep(
                        remote_mep_p->md_index, remote_mep_p->ma_index,
                        remote_mep_p->rcvd_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, tmp_mep_p))
        {
            /* reset the port/inf/rdi counter for local MEP
             */
            CFM_ENGINE_LocalUpdateRcvdMepMacStatusAndRdiCnt(
                NULL, remote_mep_p, tmp_mep_p);

            /* clear highest defect in show fault-notify-generator
             */
            if (CFM_TYPE_REMOTE_MEP_STATE_FAILD == remote_mep_p->machine_state)
            {
                if (tmp_mep_p->rmep_ccm_loss_cnt > 0)
                    tmp_mep_p->rmep_ccm_loss_cnt--;
             }

            if (tmp_mep_p->rmep_lrn_cnt > 0)
                tmp_mep_p->rmep_lrn_cnt--;

            CFM_ENGINE_UPD_ERR_MAC_STATUS(tmp_mep_p);
            CFM_ENGINE_UPD_SOME_RDI_DEF(tmp_mep_p);
            CFM_ENGINE_UPD_SOME_RMEP_CCM_DEF(tmp_mep_p);
            CFM_ENGINE_UpdateMepHighestDefect(tmp_mep_p);
            CFM_OM_StoreMep(tmp_mep_p);
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalAbortAllBgOperations
 *-------------------------------------------------------------------------
 * PURPOSE  : To abort all the background operation.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_LocalAbortAllBgOperations(void)
{
    CFM_OM_DmmCtrlRec_T     *dmm_ctrl_rec_p;
    CFM_OM_LbmCtrlRec_T     *lbm_ctrl_rec_p;

    dmm_ctrl_rec_p = CFM_OM_GetDmmCtrlRecPtr();
    if (  (NULL != dmm_ctrl_rec_p)
        &&(TRUE == dmm_ctrl_rec_p->is_busy)
       )
    {
        CFM_ENGINE_ExpireDmrRecord(dmm_ctrl_rec_p, 0);
    }

    lbm_ctrl_rec_p = CFM_OM_GetLbmCtrlRecPtr();
    if (  (NULL != lbm_ctrl_rec_p)
        &&(TRUE == lbm_ctrl_rec_p->is_busy)
       )
    {
        CFM_TIMER_StopTimer(lbm_ctrl_rec_p->timeout_timer_idx);

        lbm_ctrl_rec_p->res_bmp = CFM_TYPE_THRPT_MEASURE_RES_SENDPKTS       |
                                  CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_1SEC  |
                                  CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_TOUT;
        lbm_ctrl_rec_p->is_busy = FALSE;

        CFM_ENGINE_FreeLbmTransmitData(lbm_ctrl_rec_p);
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalAddLtrToOM
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new ltr pointer to om
 * INPUT    : ltr_om_p - the pointer to the content of ltr
 * OUTPUT   : None
 * RETUEN   : TRUE  - success
 *            FALSE - fail
 * NOTES    : 1. if cache is full it will age out the first LTR entry
 *               in timer list (i.e. the oldest entry)
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_LocalAddLtrToOM (
    CFM_OM_LTR_T    *ltr_om_p)
{
    CFM_Timer_CallBackPara_T    para;
    UI32_T                      hold_time=0;
    I16_T                       ageout_timer_idx;
    CFM_OM_LTR_T                *ltr_p=&ltr_tmp2_g;

    if (TRUE == CFM_OM_GetLtr(
                    ltr_om_p->md_index, ltr_om_p->ma_index,
                    ltr_om_p->rcvd_mep_id, ltr_om_p->seq_number,
                    ltr_om_p->receive_order, CFM_OM_MD_MA_MEP_SEQ_REC_KEY, ltr_p))
    {
        /* old one already exists, do nothing */
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "old one already exists !!!");
        return TRUE;
    }

    /* age-out the first LinkTraceReplyEntry if cache is full
     */
    while (TRUE == CFM_OM_IsLTCacheFull())
    {
        if (TRUE == CFM_TIMER_FindFirstTimerByCBFunc(
                        CFM_ENGINE_ClearLinkTraceReplyEntry_Callback,
                        &para, &ageout_timer_idx))
        {
            CFM_TIMER_StopTimer(ageout_timer_idx);
            CFM_ENGINE_ClearLinkTraceReplyEntry_Callback(&para);
        }
        else
        {
            /* if cache is full but can not age-out any entry,
             * should not occur...
             */
            return FALSE;
        }
    }

    /* need to get new timer first, before adding the ltr record.
     */
    CFM_TIMER_AssignTimerParameter(
        &para,                  ltr_om_p->md_index,   ltr_om_p->ma_index,
        ltr_om_p->rcvd_mep_id,  ltr_om_p->seq_number, ltr_om_p->receive_order);

    CFM_OM_GetLinkTraceCacheHoldTime(&hold_time);
    ltr_om_p->hold_timer_idx=CFM_TIMER_CreateTimer(
                                      CFM_ENGINE_ClearLinkTraceReplyEntry_Callback,
                                      &para,
                                      hold_time,
                                      CFM_TIMER_ONE_TIME);

    CFM_TIMER_StartTimer(ltr_om_p->hold_timer_idx);
    if(FALSE == CFM_OM_AddLtr(ltr_om_p))
    {
        CFM_TIMER_DestroyTimer(&ltr_om_p->hold_timer_idx);
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_LT, "Store LTR fail");
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalFreeMepTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : To free timers for specified MEP.
 * INPUT    : mep_p - pointer to the mep to free timers
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_LocalFreeMepTimer(
    CFM_OM_MEP_T    *mep_p)
{
    if(-1 !=mep_p->cci_while_timer_idx)
    {
        CFM_TIMER_StopTimer(mep_p->cci_while_timer_idx);
        CFM_TIMER_FreeTimer(&mep_p->cci_while_timer_idx);
    }
    if(-1 !=mep_p->error_ccm_while_timer_idx)
    {
        CFM_TIMER_StopTimer(mep_p->error_ccm_while_timer_idx);
        CFM_TIMER_FreeTimer(&mep_p->error_ccm_while_timer_idx);
    }
    if(-1 !=mep_p->xcon_ccm_while_timer_idx)
    {
        CFM_TIMER_StopTimer(mep_p->xcon_ccm_while_timer_idx);
        CFM_TIMER_FreeTimer(&mep_p->xcon_ccm_while_timer_idx);
    }
    if(-1 !=mep_p->fng_while_timer_idx)
    {
        CFM_TIMER_StopTimer(mep_p->fng_while_timer_idx);
        CFM_TIMER_FreeTimer(&mep_p->fng_while_timer_idx);
    }
    if(-1 !=mep_p->ais_send_timer_idx)
    {
        CFM_TIMER_StopTimer(mep_p->ais_send_timer_idx);
        CFM_TIMER_FreeTimer(&mep_p->ais_send_timer_idx);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalGetCurRMepMacStatusAndRdiCntInOneMa
 *-------------------------------------------------------------------------
 * PURPOSE  : To get current remote MEP mac status/rdi/learn
 *              counter for specified MA and local MEP.
 * INPUT    : md_index - md index to get
 *            ma_index - ma index to get
 *            mep_p    - pointer to the new mep created
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : for Create MEP (Fix the old one was replaced by new one issue)
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_LocalGetCurRMepMacStatusAndRdiCntInOneMa(
    UI32_T          md_index,
    UI32_T          ma_index,
    CFM_OM_MEP_T    *mep_p)
{
    CFM_OM_REMOTE_MEP_T     remote_mep;

    remote_mep.identifier = 0;
    while(TRUE == CFM_OM_GetNextRemoteMep(md_index, ma_index,
                    remote_mep.identifier, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        if (  (remote_mep.md_index != md_index)
            ||(remote_mep.ma_index != ma_index)
           )
        {
            break;
        }

        /* only count remote mep which is learned by this MEP b4
         */
        if (  (remote_mep.rcvd_mep_id        == mep_p->identifier)
            &&(remote_mep.rcvd_mep_direction == mep_p->direction)
           )
        {
            switch (remote_mep.machine_state)
            {
            case CFM_TYPE_REMOTE_MEP_STATE_OK:
                mep_p->rmep_lrn_cnt++;

                if (  (remote_mep.port_status != CFM_TYPE_PORT_STATUS_NO_PORT_STATE_TLV)
                    &&(remote_mep.port_status != CFM_TYPE_PORT_STATUS_UP)
                   )
                    mep_p->rmep_port_down_cnt++;

                if (  (remote_mep.interface_status != CFM_TYPE_INTERFACE_STATUS_NO_INTERFACE_STATUS_TLV)
                    &&(remote_mep.interface_status != CFM_TYPE_INTERFACE_STATUS_UP)
                   )
                    mep_p->rmep_inf_down_cnt++;

                if (remote_mep.rdi != FALSE)
                    mep_p->rmep_rdi_on_cnt++;
                break;
            case CFM_TYPE_REMOTE_MEP_STATE_FAILD:
                mep_p->rmep_lrn_cnt++;
                mep_p->rmep_ccm_loss_cnt++;
                break;
            default:
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalGetCurRMepCcmLostCntInOneMa
 *-------------------------------------------------------------------------
 * PURPOSE  : To get current remote MEP ccm lost counter
 *              for specified MA and local MEP.
 * INPUT    : md_index - md index to get
 *            ma_index - ma index to get
 *            mep_p    - pointer to the new mep created
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : for Create MEP (Fix the old one was replaced by new one issue)
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_LocalGetCurRMepCcmLostCntInOneMa(
    UI32_T          md_index,
    UI32_T          ma_index,
    CFM_OM_MEP_T    *mep_p)
{
    CFM_OM_REMOTE_MEP_T     remote_mep;

    remote_mep.identifier = 0;
    while (TRUE == CFM_OM_GetNextRemoteMep(md_index, ma_index,
                    remote_mep.identifier, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        if (  (remote_mep.md_index != md_index)
            ||(remote_mep.ma_index != ma_index)
           )
        {
            break;
        }

        /* only count remote mep whose rcvd id == 0 && state is failed
         */
        if (  (CFM_TYPE_REMOTE_MEP_STATE_FAILD == remote_mep.machine_state)
            &&(remote_mep.rcvd_mep_id == 0)
           )
        {
            mep_p->rmep_ccm_loss_cnt ++;
        }
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalUpdateRMepCcmLostCntInOneMa
 *-------------------------------------------------------------------------
 * PURPOSE  : To update the remote MEP ccm lost counter
 *              for all local MEPs in specified MA.
 * INPUT    : remote_mep_p   - pointer to remote MEP to update
 *            cur_rcvd_mep_p - pointer to current received local MEP
 *            is_add         - TRUE to add counter
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_LocalUpdateRMepCcmLostCntInOneMa(
    CFM_OM_REMOTE_MEP_T     *remote_mep_p,
    CFM_OM_MEP_T            *cur_rcvd_mep_p,
    BOOL_T                  is_add)
{
    CFM_OM_MEP_T    *tmp_mep_p=&mep_tmp2_g;
    UI32_T          nxt_mep_id=0;

    while (TRUE == CFM_OM_GetNextMep(
                remote_mep_p->md_index, remote_mep_p->ma_index,
                nxt_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, tmp_mep_p))
    {
        if (  (tmp_mep_p->md_index != remote_mep_p->md_index)
            ||(tmp_mep_p->ma_index != remote_mep_p->ma_index)
           )
        {
            break;
        }

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM,
            "update other mep not receive problem, update mep -%ld", (long)tmp_mep_p->identifier);

        if (TRUE == is_add)
        {
            tmp_mep_p->rmep_ccm_loss_cnt++;
         }
        else
        {
            if (0 < tmp_mep_p->rmep_ccm_loss_cnt)
            {
                tmp_mep_p->rmep_ccm_loss_cnt--;
            }
        }

        /* cur rcvd MEP's status will be updated by caller API
         * bcz FNG state machine should not be updated twice,
         * (the ccm loss cnt will no be saved for cur rcvd MEP)
         */
        if (  (NULL == cur_rcvd_mep_p)
            ||(tmp_mep_p->identifier != cur_rcvd_mep_p->identifier)
           )
        {
            CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM,
                "update mep/rmep_ccm_loss_cnt -%ld/%ld",
                (long)tmp_mep_p->identifier, (long)tmp_mep_p->rmep_ccm_loss_cnt);

            CFM_ENGINE_UPD_SOME_RMEP_CCM_DEF(tmp_mep_p);
            CFM_ENGINE_UpdateMepHighestDefect(tmp_mep_p);
            CFM_OM_StoreMep(tmp_mep_p);
        }

        nxt_mep_id = tmp_mep_p->identifier;
    }
}

#define CFM_ENGINE_GET_DELTA_DOWN_CNT_BY_INF_STS(inf, cnt) \
    {                   \
        switch(inf)     \
        {               \
        case CFM_TYPE_INTERFACE_STATUS_NO_INTERFACE_STATUS_TLV: \
            cnt = 0;    \
            break;      \
        case CFM_TYPE_INTERFACE_STATUS_UP:  \
            cnt = 0;    \
            break;      \
        default:        \
            cnt = 1;    \
            break;      \
        }               \
    }

#define CFM_ENGINE_GET_DELTA_DOWN_CNT_BY_PORT_STS(port, cnt) \
    {                   \
        switch(port)    \
        {               \
        case CFM_TYPE_PORT_STATUS_NO_PORT_STATE_TLV: \
            cnt = 0;    \
            break;      \
        case CFM_TYPE_PORT_STATUS_UP:   \
            cnt = 0;    \
            break;      \
        default:        \
            cnt = 1;    \
            break;      \
        }               \
    }

#define CFM_ENGINE_GET_DELTA_ON_CNT_BY_RDI_FLG(rdi, cnt) \
    {                                  \
        cnt = (TRUE == rdi) ? 1 : 0;   \
    }

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalUpdateRcvdMepMacStatusAndRdiCnt
 *-------------------------------------------------------------------------
 * PURPOSE  : To update the mac status/rdi counter
 *              for specified local MEP.
 * INPUT    : ccm_p          - pointer to ccm received
 *                             NULL to reset the remote MEP port/inf/rdi status
 *            remote_mep_p   - pointer to remote MEP
 *            cur_rcvd_mep_p - pointer to current received local MEP
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_ENGINE_LocalUpdateRcvdMepMacStatusAndRdiCnt(
    CFM_ENGINE_CcmPdu_T     *ccm_p,
    CFM_OM_REMOTE_MEP_T     *remote_mep_p,
    CFM_OM_MEP_T            *cur_rcvd_mep_p)
{
    CFM_OM_MEP_T                *old_rcvd_mep_p = &mep_tmp4_g;
    I32_T                       old_dta_cnt, new_dta_cnt, fin_dta_cnt;
    CFM_TYPE_PortStatus_T       old_port_sts, new_port_sts;
    CFM_TYPE_InterfaceStatus_T  old_inf_sts, new_inf_sts;
    BOOL_T                      new_rdi_flg, old_rdi_flg,
                                is_upd_same_mep = TRUE,
                                is_upd_old = FALSE,
                                is_rmep_up = FALSE;

    if (remote_mep_p->rcvd_mep_id != cur_rcvd_mep_p->identifier)
    {
        is_upd_same_mep = FALSE;

        if (remote_mep_p->rcvd_mep_id != 0)
        {
            if (TRUE == CFM_OM_GetMep(remote_mep_p->md_index, remote_mep_p->ma_index,
                            remote_mep_p->rcvd_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, old_rcvd_mep_p))
            {
                is_upd_old = TRUE;
            }
        }
    }

    new_inf_sts  = CFM_TYPE_INTERFACE_STATUS_NO_INTERFACE_STATUS_TLV;
    new_port_sts = CFM_TYPE_PORT_STATUS_NO_PORT_STATE_TLV;
    new_rdi_flg  = FALSE;

    /* compute the new inf/port/rdi if ccm_p != null
     */
    if (NULL != ccm_p)
    {
        if  (NULL != ccm_p->interface_status_type_p)
        {
            new_inf_sts = *ccm_p->interface_status_value_p;
        }

        if  (NULL != ccm_p->port_status_type_p)
        {
            new_port_sts = *ccm_p->port_status_value_p;
        }

        if (  (new_inf_sts  == CFM_TYPE_INTERFACE_STATUS_UP)
            &&(new_port_sts == CFM_TYPE_PORT_STATUS_UP)
           )
            is_rmep_up = TRUE;

        new_rdi_flg  = (ccm_p->header_p->flags & 0x80) ? TRUE : FALSE;
    }

    if ((NULL != ccm_p) && (FALSE == is_rmep_up))
    {
        remote_mep_p->packet_error++;
    }

    old_inf_sts  = remote_mep_p->interface_status;
    old_port_sts = remote_mep_p->port_status;
    old_rdi_flg  = remote_mep_p->rdi;

    if (TRUE == is_upd_same_mep)
    {
        if (  (new_rdi_flg  == old_rdi_flg)
            &&(new_inf_sts  == old_inf_sts)
            &&(new_port_sts == old_port_sts)
           )
            return;
    }

    /* inf status, down counter */
    {
        CFM_ENGINE_GET_DELTA_DOWN_CNT_BY_INF_STS(old_inf_sts, old_dta_cnt);
        CFM_ENGINE_GET_DELTA_DOWN_CNT_BY_INF_STS(new_inf_sts, new_dta_cnt);

        if (TRUE == is_upd_same_mep)
        {
            fin_dta_cnt     = new_dta_cnt - old_dta_cnt;
            cur_rcvd_mep_p->rmep_inf_down_cnt += fin_dta_cnt;
        }
        else
        {
            if (TRUE == is_upd_old)
                old_rcvd_mep_p->rmep_inf_down_cnt -= old_dta_cnt;
            cur_rcvd_mep_p->rmep_inf_down_cnt += new_dta_cnt;
        }
    }

    /* port status, up counter*/
    {
        CFM_ENGINE_GET_DELTA_DOWN_CNT_BY_PORT_STS(old_port_sts, old_dta_cnt);
        CFM_ENGINE_GET_DELTA_DOWN_CNT_BY_PORT_STS(new_port_sts, new_dta_cnt);

        if (TRUE == is_upd_same_mep)
        {
            fin_dta_cnt     = new_dta_cnt - old_dta_cnt;
            cur_rcvd_mep_p->rmep_port_down_cnt += fin_dta_cnt;
        }
        else
        {
            if (TRUE == is_upd_old)
                old_rcvd_mep_p->rmep_port_down_cnt -= old_dta_cnt;
            cur_rcvd_mep_p->rmep_port_down_cnt += new_dta_cnt;
        }
    }

    /* rdi flag, on counter */
    {
        CFM_ENGINE_GET_DELTA_ON_CNT_BY_RDI_FLG(old_rdi_flg, old_dta_cnt);
        CFM_ENGINE_GET_DELTA_ON_CNT_BY_RDI_FLG(new_rdi_flg, new_dta_cnt);

        if (TRUE == is_upd_same_mep)
        {
            fin_dta_cnt     = new_dta_cnt - old_dta_cnt;
            cur_rcvd_mep_p->rmep_rdi_on_cnt += fin_dta_cnt;
        }
        else
        {
            if (TRUE == is_upd_old)
                old_rcvd_mep_p->rmep_rdi_on_cnt -= old_dta_cnt;
            cur_rcvd_mep_p->rmep_rdi_on_cnt += new_dta_cnt;
        }
    }

    if (TRUE == is_upd_old)
       CFM_OM_StoreMep(old_rcvd_mep_p);

    if (FALSE == is_rmep_up)
    {
        /* from up to down
         */
        if (remote_mep_p->mep_up == TRUE)
        {
            remote_mep_p->mep_up = FALSE;
            remote_mep_p->ma_p->remote_mep_down_counter ++;

            if (NULL != ccm_p)
            {
                CFM_ENGINE_XmitCCTrap(
                    cur_rcvd_mep_p->md_p, cur_rcvd_mep_p->ma_p,
                    cur_rcvd_mep_p->identifier, remote_mep_p->identifier,
                    CFM_TYPE_SNMP_TRAPS_CC_MEP_DOWN);
            }
        }
    }
    else
    {
        /* from down to up
         */
        if (FALSE == remote_mep_p->mep_up)
        {
            CFM_ENGINE_XmitCCTrap(
                cur_rcvd_mep_p->md_p, cur_rcvd_mep_p->ma_p,
                cur_rcvd_mep_p->identifier, remote_mep_p->identifier,
                CFM_TYPE_SNMP_TRAPS_CC_MEP_UP);

            if (remote_mep_p->ma_p->remote_mep_down_counter > 0)
                remote_mep_p->ma_p->remote_mep_down_counter --;

            if (0 == remote_mep_p->ma_p->remote_mep_down_counter)
            {
                CFM_ENGINE_XmitCrossCheckTrap(
                    cur_rcvd_mep_p->md_p, cur_rcvd_mep_p->ma_p,
                    cur_rcvd_mep_p->identifier, remote_mep_p->identifier,
                    CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MA_UP);
            }

            remote_mep_p->mep_up = TRUE;
        }
    }

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_CCM,
        "cur rcvd mep/rmep-%ld/%ld, rdi/port/inf-%ld/%ld/%ld",
            (long)cur_rcvd_mep_p->identifier,
            (long)remote_mep_p->identifier,
            (long)cur_rcvd_mep_p->rmep_rdi_on_cnt,
            (long)cur_rcvd_mep_p->rmep_port_down_cnt,
            (long)cur_rcvd_mep_p->rmep_inf_down_cnt);

    remote_mep_p->interface_status = new_inf_sts;
    remote_mep_p->port_status      = new_port_sts;
    remote_mep_p->rdi              = new_rdi_flg;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_ENGINE_LocalReplceLmepId
 *-------------------------------------------------------------------------
 * PURPOSE  : To modify the MEP id for a local MEP.
 * INPUT    : lmep_p      - pointer to local MEP
 *            new_lmep_id - new MEP id for the old local MEP.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_ENGINE_LocalReplceLmepId(
    CFM_OM_MEP_T    *lmep_p,
    UI32_T          new_lmep_id)
{
    CFM_OM_REMOTE_MEP_T rmep;
    UI32_T              next_rmep_id=0;
    BOOL_T              ret = TRUE;

    /* update all remote mep's rcvd_mep_id
     */
    while (TRUE ==CFM_OM_GetNextRemoteMep(lmep_p->md_index, lmep_p->ma_index, next_rmep_id, CFM_OM_MD_MA_MEP_KEY, &rmep))
    {
        if (  (rmep.md_index != lmep_p->md_index)
            ||(rmep.ma_index != lmep_p->ma_index)
           )
        {
            break;
        }

        next_rmep_id = rmep.identifier;

        if (rmep.rcvd_mep_id == lmep_p->identifier)
        {
            rmep.rcvd_mep_id = new_lmep_id;
            CFM_OM_SetRemoteMep(&rmep);
        }
    }

    lmep_p->identifier = new_lmep_id;

    if (TRUE == ret)
    {
        lmep_p->identifier = new_lmep_id;
        CFM_OM_StoreMep(lmep_p);
    }

    return ret;
}

#endif /*#if (SYS_CPNT_CFM == TRUE)*/

