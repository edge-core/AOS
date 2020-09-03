/* MODULE NAME: pppoe_ia_engine.c
 * PURPOSE:
 *   Definitions of engine APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *   11/26/09    -- Squid Ro, Modify for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_dflt.h"

#include "pppoe_ia_backdoor.h"
#include "pppoe_ia_type.h"
#include "pppoe_ia_engine.h"
#include "pppoe_ia_om.h"
#include "l_stdlib.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "l_mm.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"
#include "amtr_mgr.h"
#include "vlan_om.h"
#include "l2mux_mgr.h"

/* NAMING CONSTANT DECLARARTIONS
 */
/* uppercase
 */
#define PPPOE_IA_ENGINE_MAC_STR_FORMAT          "%02X-%02X-%02X-%02X-%02X-%02X"

/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T PPPOE_IA_ENGINE_LocalAddPaddingToEgrPdu(
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p);

static BOOL_T PPPOE_IA_ENGINE_LocalBuildEgrPdu(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    BOOL_T  ing_port_trust,
    BOOL_T  ing_port_strip,
    UI32_T  ing_lport,
    UI16_T  ing_vid,
    UI8_T   ing_code,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p,
    BOOL_T  *is_gen_er_p,
    UI8_T   *drop_reason_p);

static BOOL_T PPPOE_IA_ENGINE_LocalBuildMref(
    L_MM_Mref_Handle_T  **mref_pp,
    UI8_T               *pdu_p,
    UI16_T              pdu_len);

static BOOL_T PPPOE_IA_ENGINE_LocalBuildPadoWithGenEr(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p);

static BOOL_T PPPOE_IA_ENGINE_LocalBuildPadsWithGenEr(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p);

static PPPOE_IA_TYPE_ECODE_T PPPOE_IA_ENGINE_LocalCopyIngTAGtlvToEgrPdu(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    BOOL_T  is_strip_vendor,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p);

static BOOL_T PPPOE_IA_ENGINE_LocalDoEgressCheckForUnicast(
    BOOL_T  ing_port_trust,
    UI8_T   pdu_code,
    UI32_T  egr_lport,
    UI8_T   *drop_reason_p);

static BOOL_T PPPOE_IA_ENGINE_LocalDoIngressCheck(
    BOOL_T  ing_port_enable,
    BOOL_T  ing_port_trust,
    UI8_T   ing_pdu_code,
    UI32_T  ing_pdu_len,
    UI32_T  ing_payload_len,
    UI8_T   *drop_reason_p);

#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == TRUE)
static BOOL_T PPPOE_IA_ENGINE_LocalGetEgrPortsForRelay(
    UI32_T  ing_lport,
    UI16_T  ing_vid,
    UI8_T   *dst_mac_p,
    UI8_T   egr_pbmp_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

static void PPPOE_IA_ENGINE_LocalDoSoftwareRelay(
    PPPOE_IA_TYPE_PktHdr_T  *pkt_hdr_p,
    UI16_T                  ing_vid,
    UI8_T                   *ing_pdu_p,
    UI32_T                  ing_pdu_len);
#endif /* #if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == TRUE) */

static BOOL_T PPPOE_IA_ENGINE_LocalGetAccessLoopId(
    UI32_T  lport,
    UI16_T  vid,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_ACC_LOOP_ID_LEN],
    UI32_T  *out_buf_len_p);

static BOOL_T PPPOE_IA_ENGINE_LocalGetAccessNodeId(
    UI8_T   acc_nodeid_ar[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1],
    UI8_T   *nodeid_len_p);

static BOOL_T PPPOE_IA_ENGINE_LocalGetAcNameTag(
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p);

static BOOL_T PPPOE_IA_ENGINE_LocalGetCircuitId(
    UI32_T  lport,
    UI16_T  vid,
    UI8_T   circuit_id_ar[PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1],
    UI8_T   *ccid_len_p);

static UI8_T PPPOE_IA_ENGINE_LocalGetDefaultCircuitId(
    UI32_T  lport,
    UI16_T  vid,
    BOOL_T  is_syntax,
    UI8_T   circuit_id_ar[PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1]);

static UI8_T PPPOE_IA_ENGINE_LocalGetDefaultRemoteId(
    UI32_T  lport,
    UI8_T   remote_id_ar[PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1]);

static BOOL_T PPPOE_IA_ENGINE_LocalGetEgrPorts(
    UI32_T  ing_lport,
    UI16_T  ing_vid,
    UI8_T   pdu_code,
    UI8_T   *dst_mac_p,
    BOOL_T  ing_port_trust,
    BOOL_T  is_gen_er,
    UI8_T   egr_pbmp_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
    UI8_T   *drop_reason_p);

static BOOL_T PPPOE_IA_ENGINE_LocalGetEgrPortsForBroadcast(
    BOOL_T  ing_port_trust,
    UI8_T   pdu_code,
    UI32_T  *egr_pnum_p,
    UI8_T   egr_pbmp_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

static BOOL_T PPPOE_IA_ENGINE_LocalGetGenericErrMsg(
    UI8_T   gen_ermsg_ar[PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1],
    UI32_T  *ermsg_len_p);

static BOOL_T PPPOE_IA_ENGINE_LocalGetGenericErrTag(
    I32_T   outbuf_len_max,
    UI8_T   *outbuf_p,
    UI32_T  *outbuf_len_p);

static BOOL_T PPPOE_IA_ENGINE_LocalGetRemoteId(
    UI32_T  lport,
    UI8_T   remote_id_ar[PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1],
    UI8_T   *rmtid_len_p);

static BOOL_T PPPOE_IA_ENGINE_LocalGetSvcNameTagFromIngPdu(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p);

static void PPPOE_IA_ENGINE_LocalSendPacket(
    PPPOE_IA_TYPE_PktHdr_T  *pkt_hdr_p,
    BOOL_T                  is_gen_er,
    UI8_T                   *egr_pdu_p,
    UI32_T                  egr_pdu_len,
    UI8_T                   *egr_pbmp_p);

static void PPPOE_IA_ENGINE_LocalIncreaseStatistics(
    UI32_T                  ing_lport,
    UI8_T                   ing_code,
    UI8_T                   drop_reason);


/* STATIC VARIABLE DECLARATIONS
 */
static UI8_T egr_pkt_buf[PPPOE_IA_TYPE_MAX_FRAME_SIZE];

/* EXPORTED SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_AddFstTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the port joins the trunk
 *          as the 1st member
 * INPUT  : trk_ifidx - specify which trunk to join.
 *          mbr_ifidx - specify which member port being add to trunk.
 * OUTPUT : None
 * RETURN : None
 * NOTES  : mbr_ifidx is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_AddFstTrkMbr(
    UI32_T  trk_ifidx,
    UI32_T  mbr_ifidx)
{
    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "trk_ifidx/mgr_ifidx: %ld/%ld", trk_ifidx, mbr_ifidx);

    /* 1. copy the 1st member's property to trunk
     * 2. del 1st member from the port list
     * 3. add trunk to the port list
     */
    PPPOE_IA_OM_DelPortFromPorts(trk_ifidx);
    PPPOE_IA_OM_CopyPortConfigTo(mbr_ifidx, trk_ifidx);
    PPPOE_IA_OM_DelPortFromPorts(mbr_ifidx);
    PPPOE_IA_OM_AddPortToPorts(trk_ifidx);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_AddTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to join to
 *          mbr_ifidx - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_AddTrkMbr(
    UI32_T  trk_ifidx,
    UI32_T  mbr_ifidx)
{
    BOOL_T  global_enable, trk_enable;

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "trk_ifidx/mgr_ifidx: %ld/%ld", trk_ifidx, mbr_ifidx);

    /* 1. del member from the port list
     * 2. apply the trunk's property to member
     * 3. add/del rule according to the enabled status
     */
    PPPOE_IA_OM_GetBoolDataByField(
        0, PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE, &global_enable);

    PPPOE_IA_OM_GetBoolDataByField(
        trk_ifidx, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, &trk_enable);

    PPPOE_IA_OM_DelPortFromPorts(mbr_ifidx);
    PPPOE_IA_OM_CopyPortConfigTo(trk_ifidx, mbr_ifidx);

#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == FALSE)
    if ((TRUE == global_enable) && (TRUE == trk_enable))
    {
        SWCTRL_PMGR_SetPPPoEDPktToCpu(mbr_ifidx, TRUE);
    }
#endif
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_DelLstTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the last trunk member
 *          is removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to join to
 *          mbr_ifidx - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_DelLstTrkMbr(
    UI32_T  trk_ifidx,
    UI32_T  mbr_ifidx)
{
    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "trk_ifidx/mgr_ifidx: %ld/%ld", trk_ifidx, mbr_ifidx);

    /* 1. add member to the port list
     * 2. del trunk from the port list
     * 3. clear trunk's property
     */
    PPPOE_IA_OM_AddPortToPorts(mbr_ifidx);
    PPPOE_IA_OM_DelPortFromPorts(trk_ifidx);
    PPPOE_IA_OM_ClearPortConfig(trk_ifidx);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_DelTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to join to
 *          mbr_ifidx - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_DelTrkMbr(
    UI32_T  trk_ifidx,
    UI32_T  mbr_ifidx)
{
    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "trk_ifidx/mgr_ifidx: %ld/%ld", trk_ifidx, mbr_ifidx);

    /* 1. add member to the port list
     */
    PPPOE_IA_OM_AddPortToPorts(mbr_ifidx);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_GetAccessNodeId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global access node id.
 * INPUT  : None
 * OUTPUT : acc_nodeid_ar - pointer to output buffer
 *                          >= PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1
 *          nodeid_len_p  - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : access node id may be
 *           1. user configured id
 *           2. first ip address if 1 is not available
 *           3. cpu mac if 1 & 2 are not available
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_GetAccessNodeId(
    UI8_T   acc_nodeid_ar[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1],
    UI8_T   *nodeid_len_p)
{
    return PPPOE_IA_ENGINE_LocalGetAccessNodeId(acc_nodeid_ar, nodeid_len_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_GetGenericErrMsg
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global generic error message.
 * INPUT  : ermsg_len_p  - length of buffer to receive the string
 * OUTPUT : gen_ermsg_ar - pointer to output buffer
 *                         >= PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1
 *          ermsg_len_p  - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_GetGenericErrMsg(
    UI8_T   gen_ermsg_ar[PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1],
    UI32_T  *ermsg_len_p)
{
    return PPPOE_IA_ENGINE_LocalGetGenericErrMsg(gen_ermsg_ar, ermsg_len_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_GetPortOprCfgEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get the PPPoE IA port config entry for specified lport.
 * INPUT  : lport  - lport to get
 * OUTPUT : pcfg_p - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : 1. this api will fill the entry with default
 *             circuit-id/remote-id, while om api can not do this.
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_GetPortOprCfgEntry(
    UI32_T                          lport,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p)
{
    BOOL_T  ret = FALSE;

    if (   (NULL != pcfg_p)
         &&(TRUE == SWCTRL_LogicalPortExisting(lport))
       )
    {
        ret = PPPOE_IA_OM_GetPortAdmCfgEntry(lport, &pcfg_p->adm_cfg);

        if (TRUE == ret)
        {
            if (0 == pcfg_p->adm_cfg.circuit_id_len)
            {
                pcfg_p->opr_ccid_len = PPPOE_IA_ENGINE_LocalGetDefaultCircuitId(
                                            lport, 0, TRUE, pcfg_p->opr_ccid);
            }
            else
            {
                pcfg_p->opr_ccid_len = pcfg_p->adm_cfg.circuit_id_len;
                memcpy(pcfg_p->opr_ccid, pcfg_p->adm_cfg.circuit_id, pcfg_p->opr_ccid_len+1);
            }

            if (0 == pcfg_p->adm_cfg.remote_id_len)
            {
                pcfg_p->opr_rmid_len = PPPOE_IA_ENGINE_LocalGetDefaultRemoteId(
                                            lport, pcfg_p->opr_rmid);
            }
            else
            {
                pcfg_p->opr_rmid_len = pcfg_p->adm_cfg.remote_id_len;
                memcpy(pcfg_p->opr_rmid, pcfg_p->adm_cfg.remote_id, pcfg_p->opr_rmid_len+1);
            }
        }
    }
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_GetPortStrDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To get string data for specified ifindex and field id.
 * INPUT  : lport     - 1-based ifindex to get
 *          fld_id    - PPPOE_IA_TYPE_FLDID_E
 *                      field id to get
 *          is_oper   - TRUE to get operation string
 *          str_len_p - length of input buffer
 *                      (including null terminator)
 * OUTPUT : str_p     - pointer to output string data
 *          str_len_p - length of output buffer used
 *                      (not including null terminator)
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_MGR_SetPortAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_GetPortStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  is_oper,
    I32_T   *str_len_p,
    UI8_T   *string_p)
{
    BOOL_T  ret = FALSE;

    if (   (NULL != str_len_p)
         &&(NULL != string_p)
         &&(TRUE == SWCTRL_LogicalPortExisting(lport))
       )
    {
        I32_T   tmp_len = *str_len_p;

        ret = PPPOE_IA_OM_GetAdmStrDataByField(
                    lport, field_id, &tmp_len, string_p);

        if (TRUE == ret)
        {
            if ((TRUE == is_oper) && (0 == tmp_len))
            {
                ret = FALSE;

                switch(field_id)
                {
                case PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID:
                    if (*str_len_p > PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1)
                    {
                        tmp_len = PPPOE_IA_ENGINE_LocalGetDefaultCircuitId(
                                                    lport, 0, TRUE, string_p);
                        ret = TRUE;
                    }
                    break;
                case PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID:

                    if (*str_len_p > PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1)
                    {
                        tmp_len = PPPOE_IA_ENGINE_LocalGetDefaultRemoteId(
                                                    lport, string_p);
                        ret = TRUE;
                    }
                    break;
                default:
                    break;
                }
            }

            if (TRUE == ret)
                *str_len_p = tmp_len;
        }
    }
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE: To process the received PPPoE PDU. (PPPoE discover)
 * INPUT  : msg_p - pointer to the message get from the msg queue
 * OUTPUT : None
 * RETURN : None
 * NOTE   : called from the pppoe_ia_mgr.c
 *-------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_ProcessRcvdPDU(
    PPPOE_IA_TYPE_Msg_T     *msg_p)
{
    UI8_T                   *ing_pdu_p=NULL;
    PPPOE_PduCommonHdr_T    *pppoe_hdr_p=NULL;
    UI32_T                  ing_pdu_len=0,
                            egr_pdu_len=0,
                            payload_len=0;
    UI16_T                  ing_vid;
    UI8_T                   egr_lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                            drop_reason = PPPOE_IA_TYPE_E_NO_DROP,
                            ing_code = PPPOE_CODE_UNKN;
    BOOL_T                  ing_port_trust,
                            ing_port_strip,
                            ing_port_enable,
                            is_generic_error,
                            ret;
    if (  (NULL == msg_p) || (NULL == msg_p->mem_ref_p)
        ||(NULL == msg_p->pkt_hdr_p)
       )
    {
        return;
    }

    ing_pdu_p   = L_MM_Mref_GetPdu(msg_p->mem_ref_p, &ing_pdu_len);

    ret = PPPOE_IA_OM_GetBoolDataByField(
                    msg_p->pkt_hdr_p->lport, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, &ing_port_enable);
    ret = ret && PPPOE_IA_OM_GetBoolDataByField(
                    msg_p->pkt_hdr_p->lport, PPPOE_IA_TYPE_FLDID_PORT_TRUST,  &ing_port_trust);
    ret = ret && PPPOE_IA_OM_GetBoolDataByField(
                    msg_p->pkt_hdr_p->lport, PPPOE_IA_TYPE_FLDID_PORT_STRIP_VENDOR, &ing_port_strip);

    if (TRUE == ret)
    {
        ing_vid = msg_p->pkt_hdr_p->tag_info & 0xfff;

        PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "ing lport/vid/pdu_len: %ld/%d/%ld",
            msg_p->pkt_hdr_p->lport, ing_vid, ing_pdu_len);

#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == TRUE)
        /* do software flooding for unenabled port
         * this is for platform that traps packet to cpu globally,
         */
        if (FALSE == ing_port_enable)
        {
            PPPOE_IA_ENGINE_LocalDoSoftwareRelay(
                msg_p->pkt_hdr_p, ing_vid, ing_pdu_p, ing_pdu_len);
            return;
        }
#endif /* #if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == TRUE) */

        pppoe_hdr_p  = (PPPOE_PduCommonHdr_T *) ing_pdu_p;
        ing_code     = pppoe_hdr_p->code;
        payload_len  = L_STDLIB_Ntoh16(pppoe_hdr_p->payload_len);

        /*             INGRESS                  EGRESS
         *  REQUEST    TRUST                    UNICAST/FLOOD to TRUSTED
         *             add acc_loop_id if no id presents
         *             UNTRUST                  UNICAST/FLOOD to TRUSTED
         *             replace acc_loop_id
         *  REPLY      TRUST                    UNICAST/FLOOD to ENABLED
         *             remove acc_loop_id if needed
         *             UNTRUST                  DROPPED
         *             do nothing
         */
        if (TRUE == PPPOE_IA_ENGINE_LocalDoIngressCheck(
                        ing_port_enable,
                        ing_port_trust,
                        ing_code,
                        ing_pdu_len,
                        payload_len,
                        &drop_reason))
        {
            ret = PPPOE_IA_ENGINE_LocalBuildEgrPdu(
                        ing_pdu_p,
                        ing_pdu_len,
                        ing_port_trust,
                        ing_port_strip,
                        msg_p->pkt_hdr_p->lport,
                        ing_vid,
                        ing_code,
                        sizeof(egr_pkt_buf),
                        egr_pkt_buf,
                        &egr_pdu_len,
                        &is_generic_error,
                        &drop_reason);

            ret = ret && PPPOE_IA_ENGINE_LocalGetEgrPorts(
                            msg_p->pkt_hdr_p->lport,
                            ing_vid,
                            ing_code,
                            msg_p->pkt_hdr_p->dst_mac,
                            ing_port_trust,
                            is_generic_error,
                            egr_lports,
                            &drop_reason);

            if (TRUE == ret)
            {
                PPPOE_IA_ENGINE_LocalSendPacket(
                        msg_p->pkt_hdr_p, is_generic_error, egr_pkt_buf,
                        egr_pdu_len, egr_lports);
            }
        }
    }

    PPPOE_IA_ENGINE_LocalIncreaseStatistics(
        msg_p->pkt_hdr_p->lport, ing_code, drop_reason);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetDefaultConfig
 * ------------------------------------------------------------------------
 * PURPOSE: To do extra work for default configuration.
 *          (e.g. setup the rules.)
 * INPUT  : start_ifidx - start ifidx for extra config (1-based)
 *          end_ifidx   - end   ifidx for extra config (1-based)
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_ENGINE_SetDefaultConfig(
    UI32_T  start_ifidx,
    UI32_T  end_ifidx)
{
    UI32_T  ifidx;
    BOOL_T  port_enable, global_enable;

    PPPOE_IA_OM_GetBoolDataByField(
        0, PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE, &global_enable);

    for (ifidx =start_ifidx; ifidx <=end_ifidx; ifidx++)
    {
        if (SWCTRL_LogicalPortExisting(ifidx))
        {
            PPPOE_IA_OM_GetBoolDataByField(
                ifidx, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, &port_enable);

            if ((TRUE == global_enable) && (TRUE == port_enable))
            {
#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == FALSE)
                SWCTRL_PMGR_SetPPPoEDPktToCpu(ifidx, TRUE);
#endif
            }
            PPPOE_IA_OM_AddPortToPorts(ifidx);
        }
    }

#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == TRUE)
    /* use first available ifidx to install rule
     */
    if (TRUE == global_enable)
    {
        SWCTRL_PMGR_SetPPPoEDPktToCpuPerSystem(TRUE);
    }
#endif
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To enable or disable globally.
 * INPUT  : is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_SetGlobalEnable(
    BOOL_T  is_enable)
{
    UI32_T  ifidx;
    BOOL_T  ret, old_enable, port_enable;

    ret = PPPOE_IA_OM_GetBoolDataByField(
            0, PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE, &old_enable);

    if ((TRUE == ret) && (is_enable != old_enable))
    {
        for (ifidx =1; ifidx <=SYS_ADPT_TOTAL_NBR_OF_LPORT; ifidx++)
        {
            if (SWCTRL_LogicalPortExisting(ifidx))
            {
                PPPOE_IA_OM_GetBoolDataByField(
                    ifidx, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, &port_enable);

#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == FALSE)
                if (TRUE == port_enable)
                {
                    /* 1. global enable to disable, remove all rules on enabled ports
                     * 2. global disable to enable, add    all rules on enabled ports
                     */
                    SWCTRL_PMGR_SetPPPoEDPktToCpu(ifidx, is_enable);
                }
#endif
            }
        }

#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == TRUE)
        SWCTRL_PMGR_SetPPPoEDPktToCpuPerSystem(is_enable);
#endif

        ret = PPPOE_IA_OM_SetBoolDataByField(
                0, PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE, is_enable);
    }
    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set boolean data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_SetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  new_val)
{
    UI32_T                      i, unit, port, mbr_ifidx;
    SWCTRL_TrunkPortExtInfo_T   trunk_ext_p_info;
    BOOL_T                      old_val,
                                global_val,
                                is_trunk = FALSE,
                                ret = FALSE;

    if (SWCTRL_LogicalPortExisting(lport))
    {
        /* trunk can be configured when it has at least one member port
         */
        if (SWCTRL_LogicalPortIsTrunkPort(lport))
        {
            if (  (TRUE == SWCTRL_GetTrunkPortExtInfo(lport, &trunk_ext_p_info))
                &&(trunk_ext_p_info.member_number > 0)
               )
            {
                is_trunk = TRUE;
                ret      = TRUE;
            }
        }
        else
        {
            ret = TRUE;
        }

        ret = ret && PPPOE_IA_OM_GetBoolDataByField(lport, fld_id, &old_val);

        if ((TRUE == ret) && (new_val != old_val))
        {
            /* try to setup rule for port enable
             */
            if (fld_id == PPPOE_IA_TYPE_FLDID_PORT_ENABLE)
            {
                ret = PPPOE_IA_OM_GetBoolDataByField(
                        0, PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE, &global_val);

                if ((TRUE == ret) && (TRUE == global_val))
                {
                    /* global enable,
                     *  1. port enable to disable, remove rule on this port
                     *  2. port disable to enable, add    rule on this port
                     */
#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == FALSE)
                    ret = SWCTRL_PMGR_SetPPPoEDPktToCpu(lport, new_val);
#endif
                }
            }

            ret = ret && PPPOE_IA_OM_SetBoolDataByField(lport, fld_id, new_val);

            /* try to update the lport list
             */
            if (  (fld_id == PPPOE_IA_TYPE_FLDID_PORT_ENABLE)
                ||(fld_id == PPPOE_IA_TYPE_FLDID_PORT_TRUST)
               )
            {
                /* del will clear enable/trust/untrust
                 * add will update the enable/trust/untrust according to
                 *          the port properties.
                 */
                ret = ret && PPPOE_IA_OM_DelPortFromPorts(lport);
                ret = ret && PPPOE_IA_OM_AddPortToPorts(lport);
            }

            /* apply the setting to all member port if it's a trunk
             */
            if ((TRUE == ret) && (TRUE == is_trunk))
            {
                for (i=0; i<trunk_ext_p_info.member_number; i++)
                {
                    unit = trunk_ext_p_info.member_list[i].unit;
                    port = trunk_ext_p_info.member_list[i].port;

                    SWCTRL_UserPortToIfindex(unit, port, &mbr_ifidx);
                    PPPOE_IA_OM_SetBoolDataByField(
                            mbr_ifidx, fld_id, new_val);
                }
            }
        }
    }
    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set ui32 data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_SetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  new_val)
{
    UI32_T                      i, unit, port, mbr_ifidx;
    SWCTRL_TrunkPortExtInfo_T   trunk_ext_p_info;
    UI32_T                      old_val,
                                is_trunk = FALSE,
                                ret = FALSE;

    if (SWCTRL_LogicalPortExisting(lport))
    {
        /* trunk can be configured when it has at least one member port
         */
        if (SWCTRL_LogicalPortIsTrunkPort(lport))
        {
            if (  (TRUE == SWCTRL_GetTrunkPortExtInfo(lport, &trunk_ext_p_info))
                &&(trunk_ext_p_info.member_number > 0)
               )
            {
                is_trunk = TRUE;
                ret      = TRUE;
            }
        }
        else
        {
            ret = TRUE;
        }

        ret = ret && PPPOE_IA_OM_GetUi32DataByField(lport, fld_id, &old_val);

        if ((TRUE == ret) && (new_val != old_val))
        {
            ret = ret && PPPOE_IA_OM_SetUi32DataByField(lport, fld_id, new_val);

            /* apply the setting to all member port if it's a trunk
             */
            if ((TRUE == ret) && (TRUE == is_trunk))
            {
                for (i=0; i<trunk_ext_p_info.member_number; i++)
                {
                    unit = trunk_ext_p_info.member_list[i].unit;
                    port = trunk_ext_p_info.member_list[i].port;

                    SWCTRL_UserPortToIfindex(unit, port, &mbr_ifidx);
                    PPPOE_IA_OM_SetUi32DataByField(
                            mbr_ifidx, fld_id, new_val);
                }
            }
        }
    }
    return ret;
}
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_SetPortAdmStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set string data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          str_p   - pointer to input string data
 *          str_len - length of input string data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_ENGINE_SetPortAdmStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len)
{
    UI32_T                      i, unit, port, mbr_ifidx;
    SWCTRL_TrunkPortExtInfo_T   trunk_ext_p_info;
    BOOL_T                      is_trunk = FALSE,
                                ret = FALSE;

    if (TRUE == SWCTRL_LogicalPortExisting(lport))
    {
        /* trunk can be configured when it has at least one member port
         */
        if (SWCTRL_LogicalPortIsTrunkPort(lport))
        {
            if (  (TRUE == SWCTRL_GetTrunkPortExtInfo(lport, &trunk_ext_p_info))
                &&(trunk_ext_p_info.member_number > 0)
               )
            {
                is_trunk = TRUE;
                ret      = TRUE;
            }
        }
        else
        {
            ret = TRUE;
        }

        if (TRUE == ret)
        {
            ret = PPPOE_IA_OM_SetAdmStrDataByField(lport, fld_id, str_p, str_len);

            /* apply the setting to all member port if it's a trunk
             */
            if ((TRUE == ret) && (TRUE == is_trunk))
            {
                for (i=0; i<trunk_ext_p_info.member_number; i++)
                {
                    unit = trunk_ext_p_info.member_list[i].unit;
                    port = trunk_ext_p_info.member_list[i].port;

                    SWCTRL_UserPortToIfindex(unit, port, &mbr_ifidx);
                    PPPOE_IA_OM_SetAdmStrDataByField(
                            mbr_ifidx, fld_id, str_p, str_len);
                }
            }
        }
    }
    return ret;
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalAddPaddingToEgrPdu
 *-------------------------------------------------------------------------
 * PURPOSE: To add padding to the egress pdu if its length < 46.
 * INPUT  : egr_pdu_len_max - maximum length for the output buffer
 *          egr_pdu_len_p   - length of output buffer used before processing
 * OUTPUT : egr_pdu_p       - pointer to the output buffer
 *          egr_pdu_len_p   - length of the output buffer after processing
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalAddPaddingToEgrPdu(
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p)
{
    UI32_T  padding_num, padding_beg = 0;
    BOOL_T  ret = FALSE;

    if ((NULL != egr_pdu_p) && (NULL != egr_pdu_len_p))
    {
        if (egr_pdu_len_max >= 46)
        {
            padding_beg = *egr_pdu_len_p;
            if (padding_beg < 46)
            {
                padding_num = 46 - padding_beg;
                memset(egr_pdu_p+padding_beg, 0, padding_num);
                *egr_pdu_len_p = 46;
            }
            ret = TRUE;
        }
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "\r\n  egr_len_max/padding_beg/ret: %ld/%ld/%d",
            egr_pdu_len_max, padding_beg, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalIsAccessLoopIdTag
 *-------------------------------------------------------------------------
 * PURPOSE: To check if input tag is access loop id or not.
 * INPUT  : ing_tag_p   - pointer to value in the input tag
 *          ing_tag_len - length of the value in the input tag
 * OUTPUT : None
 * RETURN : TURE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalIsAccessLoopIdTag(
    UI8_T   *ing_tag_p,
    UI16_T  ing_tag_len)
{
    UI32_T  vndr_code, ing_tag_pos =0;
    UI32_T  type_pos_1, type_pos_2;
    BOOL_T  is_vsa_adsl_iana, ret = FALSE;

    /* refer to PPPOE_IA_ENGINE_LocalGetAccessLoopId
     *   for the detail format
     */
    vndr_code = L_STDLIB_Hton32(PPPOE_IA_TYPE_VENDOR_ID_ADSL_IANA);

    is_vsa_adsl_iana = (0 == memcmp(&ing_tag_p[ing_tag_pos],
                              &vndr_code,
                              sizeof(vndr_code))) ? TRUE : FALSE;

    if (TRUE == is_vsa_adsl_iana)
    {
        type_pos_1 = ing_tag_pos + 4;
        if (0x01 == ing_tag_p[type_pos_1])
        {
            type_pos_2 =   type_pos_1
                         + ing_tag_p[type_pos_1+1] + 2;

            if (  (type_pos_2 <= ing_tag_len)
                &&(0x02 == ing_tag_p[type_pos_2])
               )
            {
                ret = TRUE;
            }
        }
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG, "ADSL_IANA VSA/ret: %d/%d",
        is_vsa_adsl_iana, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalIsOldAccessLoopIdExist
 *-------------------------------------------------------------------------
 * PURPOSE: To check if old access loop id already exists in the pdu.
 * INPUT  : ing_pdu_p   - pointer to the input pdu
 *                        right after the PPPoE comman header
 *          ing_pdu_len - length of the input pdu
 * OUTPUT : None
 * RETURN : TURE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalIsOldAccessLoopIdExist(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len)
{
    UI32_T  ing_pdu_pos =0, whole_tag_len;
    UI16_T  ing_tag_dat[2];
    BOOL_T  ret = FALSE;

    if (NULL != ing_pdu_p)
    {
        while ((FALSE == ret) && (ing_pdu_pos < ing_pdu_len))
        {
            /* check if tag type + tag length is valid
             */
            if (ing_pdu_pos + sizeof(ing_tag_dat) > ing_pdu_len)
            {
                break;
            }

            memcpy(ing_tag_dat, &ing_pdu_p[ing_pdu_pos], sizeof(ing_tag_dat));
            ing_tag_dat[0] = L_STDLIB_Ntoh16(ing_tag_dat[0]);
            ing_tag_dat[1] = L_STDLIB_Ntoh16(ing_tag_dat[1]);

            /* check if this tag is complete
             */
            whole_tag_len = ing_tag_dat[1] + sizeof(ing_tag_dat);

            if (whole_tag_len + ing_pdu_pos > ing_pdu_len)
            {
                break;
            }

            /* check if tag is valid
             */
            switch(ing_tag_dat[0])
            {
            case PPPOE_TAGTYPE_END_OF_LST:
                /* no more tags exist, try to break the while,
                 * 1. maybe wrong if another tag follows ???
                 */
                ing_pdu_pos = ing_pdu_len;
                continue;

            case PPPOE_TAGTYPE_VNDR_SPEC:
                ret = PPPOE_IA_ENGINE_LocalIsAccessLoopIdTag(
                        &ing_pdu_p[ing_pdu_pos+4], ing_tag_dat[1]);
                break;

            case PPPOE_TAGTYPE_SVC_NAME:
            case PPPOE_TAGTYPE_AC_NAME:
            case PPPOE_TAGTYPE_HOST_INIQ:
            case PPPOE_TAGTYPE_AC_COOKIE:
            case PPPOE_TAGTYPE_RELAY_SID:
            case PPPOE_TAGTYPE_SVC_NAME_ER:
            case PPPOE_TAGTYPE_AC_SYS_ER:
            case PPPOE_TAGTYPE_GENERIC_ER:
            default:
                break;
            }

            ing_pdu_pos += whole_tag_len;
        }
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "ing pdu_len/ret: %ld/%d", ing_pdu_len, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalBuildEgrPdu
 *-------------------------------------------------------------------------
 * PURPOSE: To build egress pdu from ingress pdu.
 * INPUT  : ing_pdu_p       - pointer to the input pdu
 *                            right after the ethertype
 *          ing_pdu_len     - length of the input pdu
 *          ing_port_trust  - trust status of ingress port
 *          ing_port_strip  - strip status of ingress port
 *          ing_lport       - ingress lport
 *          ing_vid         - ingress vid
 *          ing_code        - ingress code of PPPoE discover
 *          egr_pdu_len_max - maximum length for the output buffer
 * OUTPUT : egr_pdu_p       - pointer to the output buffer
 *          egr_pdu_len_p   - length of the output buffer used
 *          is_gen_er_p     - pointer to the output generic error status,
 *                              if failed to add access loop id to egress pdu
 *          drop_reason_p   - pointer to the output drop reason,
 *                              if this pdu is finally dropped.
 *                            PPPOE_IA_TYPE_EDROP_E
 * RETURN : TRUE  - further processing is required
 *          FALSE - no further processing is required, i.e. packet is dropped
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalBuildEgrPdu(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    BOOL_T  ing_port_trust,
    BOOL_T  ing_port_strip,
    UI32_T  ing_lport,
    UI16_T  ing_vid,
    UI8_T   ing_code,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p,
    BOOL_T  *is_gen_er_p,
    UI8_T   *drop_reason_p)
{
    PPPOE_PduCommonHdr_T    *egr_pppoe_hdr_p;
    UI32_T                  pppoe_hdr_size  = 0,
                            egr_pdu_pos     = 0,
                            tmp_egr_pdu_len = 0,
                            acc_loopid_len  = 0,
                            ing_payload_len;
    PPPOE_IA_TYPE_ECODE_T   copy_tlv_res;
    BOOL_T                  is_strip_acc_loopid = FALSE,
                            is_add_acc_loopid   = FALSE,
                            is_generic_error    = FALSE,
                            is_req_from_trust   = FALSE,
                            ret = FALSE;

    if (  (NULL == ing_pdu_p)   || (NULL == egr_pdu_p) ||(NULL == egr_pdu_len_p)
        ||(NULL == is_gen_er_p) || (NULL == drop_reason_p)
       )
    {
        return FALSE;
    }

    *is_gen_er_p = FALSE;

    /*  1. decide a) if need to strip old access loop id
     *            b) if need to add   new access loop id
     *
     *  ING_INF    TRUST  PADI/PADR PADO/PADS/PADT
     *  ADD   NEW         TRUE      FALSE
     *  STRIP OLD         TRUE      CONFIG
     *  ING_INF  UNTRUST  PADI/PADR PADO/PADS/PADT
     *  ADD   NEW         TRUE      FALSE, PADO/PADS is dropped b4 coming here
     *  STRIP OLD         TRUE      DON'T CARE
     *
     */

    /* request from untrust
     *  1. always add new acc loop id
     *  2. always strip the old one
     * request from trust
     *  1. add new acc loop id if old does not exist
     */
    if ((PPPOE_CODE_PADI == ing_code) || (PPPOE_CODE_PADR == ing_code))
    {
        is_add_acc_loopid   = TRUE;
        is_strip_acc_loopid = TRUE;

        if (TRUE == ing_port_trust)
        {
            is_req_from_trust = TRUE;
        }
    }
    else
    {
        if (TRUE == ing_port_trust)
        {
            /*  PADO/PADS/PADT from trust to untrust/trust
             *  1. no new acc loop id
             *  2. strip the old one if configured
             */
            if (TRUE == ing_port_strip)
            {
                is_strip_acc_loopid = TRUE;
            }
        }
        else
        {
            /*  PADO/PADS/PADT from untrust to trust
             *  1. no new acc loop id
             *  2. do not care the old acc loop id
             */
        }
    }

    memcpy(egr_pdu_p, ing_pdu_p, sizeof(PPPOE_PduCommonHdr_T));
    pppoe_hdr_size = egr_pdu_pos = sizeof(PPPOE_PduCommonHdr_T);

    ing_payload_len = L_STDLIB_Ntoh16(((PPPOE_PduCommonHdr_T *) ing_pdu_p)->payload_len);

    /* update payload_len if payload_len is larger than received pdu
     * and use payload_len to do pdu processing
     */
    if (ing_payload_len > ing_pdu_len - pppoe_hdr_size)
    {
        ing_payload_len = ing_pdu_len - pppoe_hdr_size;
    }

    /* add this to keep the original access loop id in request received from
     *   the trust port, bcz some customers need to use trust port to connect
     *   the devices into a ring and hope to keep the original access loop id.
     */
    if (TRUE == is_req_from_trust)
    {
        if (TRUE == PPPOE_IA_ENGINE_LocalIsOldAccessLoopIdExist
                        (ing_pdu_p + pppoe_hdr_size, ing_payload_len))
        {
            if (ing_pdu_len < egr_pdu_len_max)
            {
                memcpy(egr_pdu_p, ing_pdu_p, ing_pdu_len);

                *egr_pdu_len_p = ing_pdu_len;
                ret = TRUE;
            }
            else
            {
                *drop_reason_p = PPPOE_IA_TYPE_E_MALOFORM;
            }

            PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
                    "forward with original acc_loop_id");
            return ret;
        }
    }

    /*  2. add new access loop id to egress pdu first
     */
    if (TRUE == is_add_acc_loopid)
    {
        ret = PPPOE_IA_ENGINE_LocalGetAccessLoopId(
                        ing_lport, ing_vid, egr_pdu_p + egr_pdu_pos, &acc_loopid_len);

        if (TRUE == ret)
        {
            egr_pdu_pos += acc_loopid_len;
        }
    }
    else
    {
        ret = TRUE;
    }

    /*  3. copy other tlv to egress pdu (with/without access loop id)
     */
    if (TRUE == ret)
    {
        copy_tlv_res = PPPOE_IA_ENGINE_LocalCopyIngTAGtlvToEgrPdu(
                            ing_pdu_p + pppoe_hdr_size,
                            ing_payload_len,
                            is_strip_acc_loopid,
                            egr_pdu_len_max - egr_pdu_pos,
                            egr_pdu_p + egr_pdu_pos,
                            &tmp_egr_pdu_len);

        if (PPPOE_IA_TYPE_E_OK != copy_tlv_res)
        {
            if (PPPOE_IA_TYPE_E_GEN_ER == copy_tlv_res)
            {
                /* send generic error message back
                 */
                switch (ing_code)
                {
                case PPPOE_CODE_PADI:
                    ret = PPPOE_IA_ENGINE_LocalBuildPadoWithGenEr(
                            ing_pdu_p + pppoe_hdr_size,
                            ing_payload_len,
                            egr_pdu_len_max - pppoe_hdr_size,
                            egr_pdu_p + pppoe_hdr_size,
                            &tmp_egr_pdu_len);
                    break;
                case PPPOE_CODE_PADR:
                    ret = PPPOE_IA_ENGINE_LocalBuildPadsWithGenEr(
                            ing_pdu_p + pppoe_hdr_size,
                            ing_payload_len,
                            egr_pdu_len_max - pppoe_hdr_size,
                            egr_pdu_p + pppoe_hdr_size,
                            &tmp_egr_pdu_len);
                    break;
                default:
                    /* 1. for PADT     , it's small enoough to add access loop id
                     * 2. for PADO/PADS, should not add access loop id to it
                     * 3. for others   , un-recognized
                     *    1, 2, 3 should not happen
                     *    just return FALSE to stop processing, if it happens.
                     */
                    /* dorpped due to malformed pdu
                     */
                    *drop_reason_p = PPPOE_IA_TYPE_E_MALOFORM;
                    ret = FALSE;
                    break;
                }

                is_generic_error = TRUE;
            }
            else
            {
                /* dorpped due to malformed pdu
                 */
                *drop_reason_p = PPPOE_IA_TYPE_E_MALOFORM;
                ret = FALSE;
            }
        }
        else
        {
            tmp_egr_pdu_len += acc_loopid_len;
        }
    }

    /* 4. update payloard length in pppoe pdu header
     */
    if (TRUE == ret)
    {
        egr_pppoe_hdr_p = (PPPOE_PduCommonHdr_T *) egr_pdu_p;
        egr_pppoe_hdr_p->payload_len = L_STDLIB_Hton16(tmp_egr_pdu_len);
        tmp_egr_pdu_len += sizeof(PPPOE_PduCommonHdr_T);
        *egr_pdu_len_p = tmp_egr_pdu_len;
        *is_gen_er_p   = is_generic_error;

        if (TRUE == is_generic_error)
        {
            egr_pppoe_hdr_p->ver_type   = 0x11;
            egr_pppoe_hdr_p->session_id = 0;

            if (PPPOE_CODE_PADI == egr_pppoe_hdr_p->code)
            {
                egr_pppoe_hdr_p->code = PPPOE_CODE_PADO;
            }

            if (PPPOE_CODE_PADR == egr_pppoe_hdr_p->code)
            {
                egr_pppoe_hdr_p->code = PPPOE_CODE_PADS;
            }
        }

        ret = PPPOE_IA_ENGINE_LocalAddPaddingToEgrPdu(
                egr_pdu_len_max, egr_pdu_p, egr_pdu_len_p);
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "\r\n  egr pdu_len/strip/add_acc/is_gen_er/drop/ret: %ld/%d/%d/%d/%d/%d",
            tmp_egr_pdu_len, is_strip_acc_loopid, is_add_acc_loopid,
            is_generic_error, *drop_reason_p, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalBuildtMref
 *-------------------------------------------------------------------------
 * PURPOSE: To build a mref for transmission from input pdu and length
 * INPUT  : pdu_p   - pointer to input pdu
 *          pdu_len - length of input pdu
 * OUTPUT : mref_pp - pointer to pointer of output mref
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalBuildMref(
    L_MM_Mref_Handle_T  **mref_pp,
    UI8_T               *pdu_p,
    UI16_T              pdu_len)
{
    UI32_T  len=0;
    UI8_T   *buf_p;

    if(NULL == ((*mref_pp)=L_MM_AllocateTxBuffer(
            pdu_len, L_MM_USER_ID2(SYS_MODULE_PPPOE_IA, PPPOE_IA_TYPE_TRACE_ID_PPPOE_IA_ENGINE_LOCALBUILDMREF))))
    {
        return FALSE;
    }

    (*mref_pp)->current_usr_id  = SYS_MODULE_PPPOE_IA;
    (*mref_pp)->next_usr_id     = SYS_MODULE_L2MUX;

    buf_p = (UI8_T *)L_MM_Mref_GetPdu(*mref_pp, &len);

    if(NULL == buf_p)
    {
        if(!L_MM_Mref_Release(mref_pp))
        {
            PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG, "can't free mref");
        }

        return FALSE;
    }

    memcpy(buf_p, pdu_p, pdu_len);

    return TRUE;

#if 0
    UI8_T       *pBuf;

    pBuf = (UI8_T *)L_MEM_Allocate(pdu_len + 18);
    if (pBuf == NULL)
    {
        return FALSE;
    }
    memset(pBuf, 0x00, pdu_len + 18);

    (*mref_pp) = L_MREF_Constructor(pBuf, pdu_len + 18, pBuf + 18, L_MEM_Free);
    if (NULL == (*mref_pp))
    {
        L_MEM_Free(pBuf);
        return FALSE;
    }
    L_MREF_SetMyId(*mref_pp, SYS_MODULE_PPPOE_IA, 0); /* for L_MREF debugging purpose */
    memcpy((*mref_pp)->pdu, pdu_p, pdu_len);
    (*mref_pp)->pdu_len = pdu_len;

    return TRUE;
#endif
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalBuildPadoWithGenEr
 *-------------------------------------------------------------------------
 * PURPOSE: To build PADO with Generic-Error tag.
 * INPUT  : ing_pdu_p       - pointer to the input pdu
 *                            right after the PPPoE comman header
 *          ing_pdu_len     - length of the input pdu
 *          egr_pdu_len_max - maximum length for the output buffer
 * OUTPUT : egr_pdu_p       - pointer to the output buffer
 *          egr_pdu_len_p   - length of the output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : for building generic error reply.
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalBuildPadoWithGenEr(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p)
{
    UI32_T  egr_pdu_pos  =0,
            tmp_tag_len  =0;
    BOOL_T  ret;

    /* according to RFC 2516, 5.2
     */
    /* 1 Service Name
     *
     * NOTE: if service name is too big to copy, just ignore it
     *       that's why the return value is not checked
     */
    ret = PPPOE_IA_ENGINE_LocalGetSvcNameTagFromIngPdu(
            ing_pdu_p,
            ing_pdu_len,
            egr_pdu_len_max,
            egr_pdu_p,
            &tmp_tag_len);

    egr_pdu_pos += tmp_tag_len;

    /* 1 AC Name
     */
    ret = PPPOE_IA_ENGINE_LocalGetAcNameTag(
            egr_pdu_len_max - egr_pdu_pos,
            egr_pdu_p + egr_pdu_pos,
            &tmp_tag_len);

    egr_pdu_pos += tmp_tag_len;

    /* 1 Generic error
     */
    ret = ret && PPPOE_IA_ENGINE_LocalGetGenericErrTag(
                    egr_pdu_len_max - egr_pdu_pos,
                    egr_pdu_p + egr_pdu_pos,
                    &tmp_tag_len);

    egr_pdu_pos += tmp_tag_len;

    if (TRUE == ret)
    {
        *egr_pdu_len_p  = egr_pdu_pos;
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "\r\n  ing_pdu_len/egr_len_max/egr_pdu_len/ret: %ld/%ld/%ld/%d",
            ing_pdu_len, egr_pdu_len_max, egr_pdu_pos, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalBuildPadsWithGenEr
 *-------------------------------------------------------------------------
 * PURPOSE: To build PADS with Generic-Error tag.
 * INPUT  : ing_pdu_p       - pointer to the input pdu
 *                            right after the PPPoE comman header
 *          ing_pdu_len     - length of the input pdu
 *          egr_pdu_len_max - maximum length for the output buffer
 * OUTPUT : egr_pdu_p       - pointer to the output buffer
 *          egr_pdu_len_p   - length of the output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : for building generic error reply.
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalBuildPadsWithGenEr(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p)
{
    UI32_T  egr_pdu_pos  =0,
            tmp_tag_len  =0;
    BOOL_T  ret;

    /* according to RFC 2516, 5.4
     */
    /* 1 Service Name
     *
     * NOTE: if service name is too big to copy, just ignore it
     *       that's why the return value is not checked
     */
    ret = PPPOE_IA_ENGINE_LocalGetSvcNameTagFromIngPdu(
            ing_pdu_p,
            ing_pdu_len,
            egr_pdu_len_max,
            egr_pdu_p,
            &tmp_tag_len);

    egr_pdu_pos += tmp_tag_len;

    /* 1 Generic error
     */
    ret = PPPOE_IA_ENGINE_LocalGetGenericErrTag(
                    egr_pdu_len_max - egr_pdu_pos,
                    egr_pdu_p + egr_pdu_pos,
                    &tmp_tag_len);

    egr_pdu_pos += tmp_tag_len;

    if (TRUE == ret)
    {
        *egr_pdu_len_p  = egr_pdu_pos;
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "\r\n  ing_pdu_len/egr_len_max/egr_pdu_len/ret: %ld/%ld/%ld/%d",
            ing_pdu_len, egr_pdu_len_max, egr_pdu_pos, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalCopyIngTAGtlvToEgrPdu
 *-------------------------------------------------------------------------
 * PURPOSE: To copy tags in ingress pdu to egress pdu.
 * INPUT  : ing_pdu_p       - pointer to the input pdu
 *                            right after the PPPoE comman header
 *          ing_pdu_len     - length of the input pdu
 *          egr_pdu_len_max - maximum length for the output buffer
 * OUTPUT : egr_pdu_p       - pointer to the output buffer
 *          egr_pdu_len_p   - length of the output buffer used
 * RETURN : PPPOE_IA_TYPE_ECODE_T
 * NOTE   : 1. only check if tag is known and if tag length is satisfied
 *             with the ingress pdu
 *-------------------------------------------------------------------------
 */
static PPPOE_IA_TYPE_ECODE_T PPPOE_IA_ENGINE_LocalCopyIngTAGtlvToEgrPdu(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    BOOL_T  is_strip_vendor,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p)
{
    UI32_T                  ing_pdu_pos =0,
                            egr_pdu_pos =0,
                            whole_tag_len;
    UI16_T                  ing_tag_dat[2];
    PPPOE_IA_TYPE_ECODE_T   ret = PPPOE_IA_TYPE_E_UKN_ER;

    if (   (NULL != ing_pdu_p) && (NULL != egr_pdu_p)
        && (NULL != egr_pdu_len_p)
       )
    {
        ret = PPPOE_IA_TYPE_E_OK;

        while (  (PPPOE_IA_TYPE_E_OK == ret)
               &&(ing_pdu_pos < ing_pdu_len)
              )
        {
            /* check if tag type + tag length is valid
             */
            if (ing_pdu_pos + sizeof(ing_tag_dat) > ing_pdu_len)
            {
                ret = PPPOE_IA_TYPE_E_TLV_ER;
                continue; /* try to break the while */
            }

            memcpy(ing_tag_dat, &ing_pdu_p[ing_pdu_pos], sizeof(ing_tag_dat));
            ing_tag_dat[0] = L_STDLIB_Ntoh16(ing_tag_dat[0]);
            ing_tag_dat[1] = L_STDLIB_Ntoh16(ing_tag_dat[1]);

            /* check if this tag is complete
             */
            whole_tag_len = ing_tag_dat[1] + sizeof(ing_tag_dat);

            if (whole_tag_len + ing_pdu_pos > ing_pdu_len)
            {
                ret = PPPOE_IA_TYPE_E_TLV_ER;
                continue; /* try to break the while */
            }

            /* check if tag is valid
             */
            switch(ing_tag_dat[0])
            {
            case PPPOE_TAGTYPE_END_OF_LST:
                /* no more tags exist, try to break the while,
                 * 1. maybe wrong if another tag follows ???
                 */
                ing_pdu_pos = ing_pdu_len;
                continue;

            case PPPOE_TAGTYPE_VNDR_SPEC:
                /* bcz vendor tag should be stripped,
                 * just skip the copy operation
                 *
                 * only ignore the access loop id
                 */
                if (  (TRUE == is_strip_vendor)
                    &&(TRUE == PPPOE_IA_ENGINE_LocalIsAccessLoopIdTag(
                            &ing_pdu_p[ing_pdu_pos+4], ing_tag_dat[1]))
                    )
                {
                    break;
                }
            case PPPOE_TAGTYPE_SVC_NAME:
            case PPPOE_TAGTYPE_AC_NAME:
            case PPPOE_TAGTYPE_HOST_INIQ:
            case PPPOE_TAGTYPE_AC_COOKIE:
            case PPPOE_TAGTYPE_RELAY_SID:
            case PPPOE_TAGTYPE_SVC_NAME_ER:
            case PPPOE_TAGTYPE_AC_SYS_ER:
            case PPPOE_TAGTYPE_GENERIC_ER:
                /* check if egr_pdu is big enough
                 */
                if (egr_pdu_pos + whole_tag_len > egr_pdu_len_max)
                {
                    ret = PPPOE_IA_TYPE_E_GEN_ER;
                    continue; /* try to break the while */
                }
                memcpy(&egr_pdu_p[egr_pdu_pos], &ing_pdu_p[ing_pdu_pos], whole_tag_len);
                egr_pdu_pos += whole_tag_len;
                break;
            default:
                ret = PPPOE_IA_TYPE_E_TLV_ER;
                continue;  /* try to break the while */
            }

            ing_pdu_pos += whole_tag_len;
        }
    }

    if (PPPOE_IA_TYPE_E_OK == ret)
    {
        *egr_pdu_len_p = egr_pdu_pos;
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "\r\n  ing pdu_len/is_strip, egr pdu_len/ret: %ld/%d, %ld/%d",
        ing_pdu_len, is_strip_vendor, egr_pdu_pos, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalDoEgressCheckForUnicast
 *-------------------------------------------------------------------------
 * PURPOSE: To do egress checking for specified ingress code, trust
 *          status of ingress port and egree lport (trust/enable status).
 * INPUT  : ing_port_trust - trust status of ingress port
 *          pdu_code       - ingress PPPoE code
 *          egr_lport      - 1-based ifindex for egress lport
 * OUTPUT : drop_reason_p  - pointer to output drop reason
 * RETURN : TRUE  - further processing is required
 *          FALSE - no further processing is required, i.e. packet is dropped
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalDoEgressCheckForUnicast(
    BOOL_T  ing_port_trust,
    UI8_T   pdu_code,
    UI32_T  egr_lport,
    UI8_T   *drop_reason_p)
{
    UI8_T   drop_reason = PPPOE_IA_TYPE_E_BAD_CODE;
    BOOL_T  egr_port_trust, egr_port_enable, ret = FALSE;

    ret = PPPOE_IA_OM_GetBoolDataByField(
                    egr_lport, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, &egr_port_enable);
    ret = ret && PPPOE_IA_OM_GetBoolDataByField(
                    egr_lport, PPPOE_IA_TYPE_FLDID_PORT_TRUST, &egr_port_trust);
    if (TRUE == ret)
    {
        /*  INGRESS   EGRESS
         *  TRUST     TRUST     OK for anything
         *            UNTRUST   OK for PADO/PADS/PADT
         *  UNTRUST   TRUST     OK for PADI/PADR/PADT
         *            UNTRUST   Dropped
         */
        if (TRUE == egr_port_enable)
        {
            if (TRUE == ing_port_trust)
            {
                if (TRUE == egr_port_trust)
                {
                    ret = TRUE;
                }
                else
                {
                    switch(pdu_code)
                    {
                    case PPPOE_CODE_PADO:
                    case PPPOE_CODE_PADS:
                    case PPPOE_CODE_PADT:
                        ret = TRUE;
                        break;
                    default:
                        ret = FALSE;
                        break;
                    }

                    if (FALSE == ret)
                    {
                        /* request to untrusted port, dropped
                         */
                        drop_reason = PPPOE_IA_TYPE_E_REQ_UNTRUST;
                    }
                }
            }
            else
            {
                if (TRUE == egr_port_trust)
                {
                    switch(pdu_code)
                    {
                    case PPPOE_CODE_PADI:
                    case PPPOE_CODE_PADR:
                    case PPPOE_CODE_PADT:
                        ret = TRUE;
                        break;
                    default:
                        ret = FALSE;
                        break;
                    }

                    if (FALSE == ret)
                    {
                        /* response from untrusted port, dropped
                         */
                        drop_reason = PPPOE_IA_TYPE_E_REP_UNTRUST;
                    }
                }
                else
                {
                    switch(pdu_code)
                    {
                    case PPPOE_CODE_PADI:
                    case PPPOE_CODE_PADR:
                    case PPPOE_CODE_PADT:
                        /* request to untrusted port, dropped
                         */
                        drop_reason = PPPOE_IA_TYPE_E_REQ_UNTRUST;
                        break;
                    default:
                        /* response from untrusted port, dropped
                         */
                        drop_reason = PPPOE_IA_TYPE_E_REP_UNTRUST;
                        break;
                    }

                    ret = FALSE;
                }
            }
        }
        else
        {
            /* egress port is not enabled, dropped
             */
            drop_reason = PPPOE_IA_TYPE_E_EGRESS_NOT_ENABLE;
            ret = FALSE;
        }
    }

    if ((FALSE == ret) && (NULL != drop_reason_p))
    {
        *drop_reason_p = drop_reason;
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "\r\n  ing port_trust/code, egr lport/port_trust/enable/drop/ret: %d/%d, %ld/%d/%d/%d/%d",
        ing_port_trust, pdu_code, egr_lport, egr_port_trust, egr_port_enable,
        drop_reason, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalDoIngressCheck
 *-------------------------------------------------------------------------
 * PURPOSE: To do ingress checking for specified input data.
 * INPUT  : ing_port_enable - enable status of ingress port
 *          ing_port_trust  - trust status of ingress port
 *          ing_pdu_code    - ingress PPPoE code
 *          ing_pdu_len     - pdu length including PPPoE common header
 *          ing_payload_len - payload length in PPPoE common header
 * OUTPUT : drop_reason_p   - pointer to output drop reason
 * RETURN : TRUE  - further processing is required
 *          FALSE - no further processing is required, i.e. packet is dropped
 * NOTE   : 1. only check if payload length in header is satisfied of pdu length
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalDoIngressCheck(
    BOOL_T  ing_port_enable,
    BOOL_T  ing_port_trust,
    UI8_T   ing_pdu_code,
    UI32_T  ing_pdu_len,
    UI32_T  ing_payload_len,
    UI8_T   *drop_reason_p)
{
    UI8_T   drop_code = PPPOE_IA_TYPE_E_NO_DROP;
    BOOL_T  ret = FALSE;

    if (ing_pdu_len - sizeof(PPPOE_PduCommonHdr_T) < ing_payload_len)
    {
        /* malformed pdu
         */
        drop_code = PPPOE_IA_TYPE_E_MALOFORM;
    }
    else
    {
        if (TRUE == ing_port_enable)
        {
            if (TRUE == ing_port_trust)
            {
                switch(ing_pdu_code)
                {
                case PPPOE_CODE_PADO:
                case PPPOE_CODE_PADS:
                case PPPOE_CODE_PADT:
                case PPPOE_CODE_PADI:
                case PPPOE_CODE_PADR:
                    ret = TRUE;
                    break;
                default:
                    drop_code = PPPOE_IA_TYPE_E_MALOFORM;
                    break;
                }
            }
            else
            {
                switch(ing_pdu_code)
                {
                case PPPOE_CODE_PADI:
                case PPPOE_CODE_PADR:
                case PPPOE_CODE_PADT:
                    ret = TRUE;
                    break;
                case PPPOE_CODE_PADO:
                case PPPOE_CODE_PADS:
                    /*  PADO/PADS from untrusted port, dropped
                     */
                    drop_code = PPPOE_IA_TYPE_E_REP_UNTRUST;
                    break;
                default:
                    drop_code = PPPOE_IA_TYPE_E_MALOFORM;
                    break;
                }
            }
        }
    }

    if (NULL != drop_reason_p)
    {
        *drop_reason_p = drop_code;
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "\r\n  ing pdu_len/payload/enable/trust/code/drop/ret: %ld/%ld/%d/%d/%d/%d/%d",
        ing_pdu_len, ing_payload_len, ing_port_enable, ing_port_trust,
        ing_pdu_code, drop_code, ret);

    return ret;
}

#if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetEgrPortsForRelay
 *-------------------------------------------------------------------------
 * PURPOSE: To get egress lport list to forward the pdu.
 * INPUT  : ing_lport      - 1-based ifindex of ingress lport
 *          ing_vid        - ingress vid
 *          dst_mac_p      - destination mac for unicast
 * OUTPUT : egr_pbmp_ar    - pointer to output egress lport list
 *                           >= SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
 * RETURN : TRUE  - if at least one egress lport is available
 *          FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetEgrPortsForRelay(
    UI32_T  ing_lport,
    UI16_T  ing_vid,
    UI8_T   *dst_mac_p,
    UI8_T   egr_pbmp_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    UI32_T                  egr_port_num = 0, lport_byte;
    AMTR_TYPE_AddrEntry_T   addr_entry = {0};
    UI8_T                   lport_byte_mask;
    UI8_T                   bcast_da[SYS_ADPT_MAC_ADDR_LEN] =
                                {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    BOOL_T                  is_broadcast = TRUE,
                            ret = FALSE;

    memset(egr_pbmp_ar, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (0 != memcmp(bcast_da, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN))
    {
        memcpy(addr_entry.mac, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN);
        addr_entry.vid = ing_vid;

        /* check if unicast
         */
        if (TRUE == AMTR_MGR_GetExactAddrEntry(&addr_entry))
        {
            if (addr_entry.ifindex != ing_lport)
            {
                lport_byte_mask = (1 << (7 - ((addr_entry.ifindex - 1) & 7)));
                lport_byte      = (addr_entry.ifindex - 1) >> 3;
                egr_pbmp_ar[lport_byte] |= lport_byte_mask;
                egr_port_num = 1;
            }

            is_broadcast = FALSE;
        }
    }

    if (TRUE == is_broadcast)
    {
        /* broadcast
         */
        memset(egr_pbmp_ar, 0xff, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

        /* remove the ingress port
         */
        lport_byte_mask = (1 << (7 - ((ing_lport - 1) & 7)));
        lport_byte      = (ing_lport - 1) >> 3;
        egr_port_num = 1; /* set this to 1 to return TRUE */
        egr_pbmp_ar[lport_byte] &= ~lport_byte_mask;
    }

    if (egr_port_num > 0)
    {
        ret = TRUE;
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "\r\n  egr port_num/bc/lport/ret: %ld/%d/%d/%d",
        egr_port_num, is_broadcast, addr_entry.ifindex, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalDoSoftwareRelay
 *-------------------------------------------------------------------------
 * PURPOSE: To send the packet to its destination by unicast or flooding.
 * INPUT  : pkt_hdr_p   - pointer to the input packet header
 *          ing_vid     - ingress vid
 * INPUT  : ing_pdu_p   - pointer to the input pdu
 *                        right after the ethertype
 *          ing_pdu_len - pdu length including PPPoE common header
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static void PPPOE_IA_ENGINE_LocalDoSoftwareRelay(
    PPPOE_IA_TYPE_PktHdr_T  *pkt_hdr_p,
    UI16_T                  ing_vid,
    UI8_T                   *ing_pdu_p,
    UI32_T                  ing_pdu_len)
{
    UI32_T  egr_pdu_len = ing_pdu_len;
    UI8_T   egr_lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    BOOL_T  ret = FALSE;

    ret = PPPOE_IA_ENGINE_LocalGetEgrPortsForRelay(
                    pkt_hdr_p->lport, ing_vid, pkt_hdr_p->dst_mac, egr_lports);
    if (TRUE == ret)
    {
        memcpy(egr_pkt_buf, ing_pdu_p, ing_pdu_len);

        ret = PPPOE_IA_ENGINE_LocalAddPaddingToEgrPdu(
                sizeof(egr_pkt_buf), egr_pkt_buf, &egr_pdu_len);

        PPPOE_IA_ENGINE_LocalSendPacket(pkt_hdr_p, FALSE, egr_pkt_buf, egr_pdu_len, egr_lports);
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "egr_pdu_len/ret: %ld/%d", egr_pdu_len, ret);
}
#endif /* #if (SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetAccessLoopId
 *-------------------------------------------------------------------------
 * PURPOSE: To build the access loop identification for the specified
 *          ingress lport/vid.
 * INPUT  : lport         - 1-based ifindex of ingress lport
 *          vid           - vid
 * OUTPUT : outbuf_ar     - pointer to output buffer
 *                          >= PPPOE_IA_TYPE_MAX_ACC_LOOP_ID_LEN
 *          out_buf_len_p - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetAccessLoopId(
    UI32_T  lport,
    UI16_T  vid,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_ACC_LOOP_ID_LEN],
    UI32_T  *out_buf_len_p)
{
    UI32_T  adsl_iana   = PPPOE_IA_TYPE_VENDOR_ID_ADSL_IANA;
    UI16_T  vndr_tl[2]  = {PPPOE_TAGTYPE_VNDR_SPEC, 0};
    UI8_T   acc_node_id[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1];
    UI8_T   circuit_id[PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1];
    UI8_T   agn_ccid[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1];
    UI8_T   remote_id[PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1];
    UI8_T   accnid_len, ccid_len, rmtid_len, agnccid_len;
    BOOL_T  ret = FALSE;

    /* according to TR101, 3.9.3.2 figure 21
     * 0              1              2              3              4
     * +--------------+--------------+--------------+--------------+
     * | 0x0105 (Vendor-Specific)    |         TAG_LENGTH          |
     * +--------------+--------------+--------------+--------------+
     * | 0x00000DE9 (3561 decimal, i.e. ADSL Forum IANA entry)     |
     * +--------------+--------------+--------------+--------------+
     * | 0x01         | length       | Agent Circuit ID value...   |
     * +--------------+--------------+--------------+--------------+
     * | Agent Circuit ID value (con't) ?                         |
     * +--------------+--------------+--------------+--------------+
     * | 0x02         | length       | Agent Remote ID value...    |
     * +--------------+--------------+--------------+--------------+
     * | Agent Remote ID value (con't) ?                          |
     * +--------------+--------------+--------------+--------------+
     */
    PPPOE_IA_ENGINE_LocalGetAccessNodeId(acc_node_id, &accnid_len);
    PPPOE_IA_ENGINE_LocalGetCircuitId(lport, vid, circuit_id, &ccid_len);
    PPPOE_IA_ENGINE_LocalGetRemoteId(lport, remote_id, &rmtid_len);

    /* AGENT_CIRCUIT_ID_LEN = ACCESS_NODE_ID_LEN + " eth " + CIRCUIT_ID_LEN <= 63
     */
    agnccid_len = accnid_len + 5 + ccid_len;
    if (agnccid_len <= PPPOE_IA_TYPE_MAX_AGENT_CIRCUIT_ID_LEN)
    {
        sprintf((char *)agn_ccid, PPPOE_IA_TYPE_DFLT_AGNT_CCID_SYNTAX,
                          acc_node_id, circuit_id);

        vndr_tl[1] = 4 + 4 + agnccid_len + rmtid_len;

        vndr_tl[0] = L_STDLIB_Hton16(vndr_tl[0]);
        vndr_tl[1] = L_STDLIB_Hton16(vndr_tl[1]);
        adsl_iana  = L_STDLIB_Hton32(adsl_iana);
        memcpy(outbuf_ar,     vndr_tl,    4);
        memcpy(&outbuf_ar[4], &adsl_iana, 4);
        outbuf_ar[8] = 0x01;
        outbuf_ar[9] = agnccid_len;
        memcpy(&outbuf_ar[10], agn_ccid, agnccid_len);
        outbuf_ar[10+agnccid_len] = 0x02;
        outbuf_ar[11+agnccid_len] = rmtid_len;
        memcpy(&outbuf_ar[12+agnccid_len], remote_id, rmtid_len);

        *out_buf_len_p = 8 + 4 + agnccid_len + rmtid_len;
        ret = TRUE;
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "out_buf_len/ret: %ld/%d", *out_buf_len_p, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetAccessNodeId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global access node id.
 * INPUT  : None
 * OUTPUT : acc_nodeid_ar - pointer to output buffer
 *                          >= PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1
 *          nodeid_len_p  - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : access node id may be
 *           1. user configured id
 *           2. first ip address if 1 is not available
 *           3. cpu mac if 1 & 2 are not available
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetAccessNodeId(
    UI8_T   acc_nodeid_ar[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1],
    UI8_T   *nodeid_len_p)
{
    I32_T                       nodeid_len =PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI8_T                       cpu_mac[SYS_ADPT_MAC_ADDR_LEN]= {0};
    BOOL_T                      ret = FALSE;

    if ((NULL != acc_nodeid_ar) && (NULL != nodeid_len_p))
    {
        /* 1. use global access node id if available
         */
        ret = PPPOE_IA_OM_GetAdmStrDataByField(
                    0,           PPPOE_IA_TYPE_FLDID_GLOBAL_ACC_NODE_ID,
                    &nodeid_len, acc_nodeid_ar);

        /* 2. use first system ip if available
         */
        if ((FALSE == ret) || (nodeid_len == 0))
        {
            ret = FALSE;

            memset(&rif_config, 0, sizeof(rif_config));
            while (NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextRifConfig(&rif_config))
            {
                if (L_INET_ADDR_TYPE_IPV4 == rif_config.addr.type)  /* should skip v6 */
                {
                    ret = TRUE;
                    nodeid_len = sprintf((char *) acc_nodeid_ar, "%d.%d.%d.%d",
                                            rif_config.addr.addr[0],
                                            rif_config.addr.addr[1],
                                            rif_config.addr.addr[2],
                                            rif_config.addr.addr[3]);

                    break;
                }
            }
        }

        /* 3. use cpu mac if available
         */
        if (FALSE == ret)
        {
            SWCTRL_GetCpuMac(cpu_mac);
            nodeid_len = sprintf((char *) acc_nodeid_ar, PPPOE_IA_ENGINE_MAC_STR_FORMAT,
                                        cpu_mac[0], cpu_mac[1], cpu_mac[2],
                                        cpu_mac[3], cpu_mac[4], cpu_mac[5]);
            ret = TRUE;
        }

        *nodeid_len_p = nodeid_len;

        PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "nodeid_len/ret: %ld/%d", nodeid_len, ret);
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetAcNameTag
 *-------------------------------------------------------------------------
 * PURPOSE: To get the AC-Name tag from local cpu mac.
 * INPUT  : egr_pdu_len_max - maximum length for the output buffer
 * OUTPUT : egr_pdu_p       - pointer to the output buffer
 *          egr_pdu_len_p   - length of the output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : for building generic error reply.
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetAcNameTag(
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p)
{
    UI16_T  acname_tlv[2] = {PPPOE_TAGTYPE_AC_NAME, 0};
    UI8_T   cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
    BOOL_T  ret = FALSE;

    if ((NULL != egr_pdu_p) && (NULL != egr_pdu_len_p))
    {
        /* TAG_TYPE + TAG_LEN + AC Name (1 NULL terminator for sprintf)
         *        2         2        18 = 22
         */
        if (egr_pdu_len_max >= 22)
        {
            SWCTRL_GetCpuMac(cpu_mac);
            acname_tlv[1] = sprintf((char *)
                                egr_pdu_p+4, PPPOE_IA_ENGINE_MAC_STR_FORMAT,
                                cpu_mac[0], cpu_mac[1], cpu_mac[2],
                                cpu_mac[3], cpu_mac[4], cpu_mac[5]);
            acname_tlv[0] = L_STDLIB_Hton16(acname_tlv[0]);
            acname_tlv[1] = L_STDLIB_Hton16(acname_tlv[1]);
            memcpy(egr_pdu_p, acname_tlv, 4);
            *egr_pdu_len_p = 21;
            ret = TRUE;
        }
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "egr_len_max/ac_tag_len/ret: %ld/%d/%d",
            egr_pdu_len_max, 4 + acname_tlv[1], ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetCircuitId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the circut-id for the specified ingress lport/vid.
 * INPUT  : lport         - 1-based ifindex of ingress lport
 *          vid           - vid
 * OUTPUT : circuit_id_ar - pointer to output buffer
 *                          >= PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1
 *          ccid_len_p    - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : circuit-id may be
 *           1. user configured id
 *           2. default syntax (i.e. unit/port:vid) if 1 is not available
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetCircuitId(
    UI32_T  lport,
    UI16_T  vid,
    UI8_T   circuit_id_ar[PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1],
    UI8_T   *ccid_len_p)
{
    I32_T               circuit_id_len =PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1;
    BOOL_T              ret = FALSE;

    if ((NULL != circuit_id_ar) && (NULL != ccid_len_p))
    {
        /* 1. use port agent circuit id if available
         */
        ret = PPPOE_IA_OM_GetAdmStrDataByField(
                    lport,           PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID,
                    &circuit_id_len, circuit_id_ar);

        /* 2. use default circuit id syntax
         */
        if ((FALSE == ret) || (circuit_id_len == 0))
        {
            circuit_id_len = PPPOE_IA_ENGINE_LocalGetDefaultCircuitId(
                                lport, vid, FALSE, circuit_id_ar);
            if (circuit_id_len > 0)
                ret = TRUE;
        }

        *ccid_len_p = circuit_id_len;

        PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "ccid_len/ret: %ld/%d", circuit_id_len, ret);
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetEgrPorts
 *-------------------------------------------------------------------------
 * PURPOSE: To get egress lport list to forward the pdu.
 * INPUT  : ing_lport      - 1-based ifindex of ingress lport
 *          ing_vid        - ingress vid
 *          pdu_code       - ingress PPPoE code
 *          dst_mac_p      - destination mac for unicast
 *          ing_port_trust - trust status of ingress port
 *          is_gen_er      - TRUE for sending generic error message back
 * OUTPUT : egr_pbmp_ar    - pointer to output egress lport list
 *                           >= PPPOE_IA_TYPE_MAX_ACC_LOOP_ID_LEN
 *          drop_reason_p  - pointer to output drop reason if it's finally dropped
 * RETURN : TRUE  - if at least one egress lport is available
 *          FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetEgrPorts(
    UI32_T  ing_lport,
    UI16_T  ing_vid,
    UI8_T   pdu_code,
    UI8_T   *dst_mac_p,
    BOOL_T  ing_port_trust,
    BOOL_T  is_gen_er,
    UI8_T   egr_pbmp_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
    UI8_T   *drop_reason_p)
{
    UI32_T                  egr_port_num = 0, lport_byte;
    AMTR_TYPE_AddrEntry_T   addr_entry = {0};
    UI8_T                   lport_byte_mask;
    UI8_T                   bcast_da[SYS_ADPT_MAC_ADDR_LEN] =
                                {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    BOOL_T                  is_broadcast = TRUE,
                            ret = TRUE;

    memset(egr_pbmp_ar, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (TRUE == is_gen_er)
    {
        /* use the ingress port to send generic error
         */
        lport_byte_mask = (1 << (7 - ((ing_lport - 1) & 7)));
        lport_byte      = (ing_lport - 1) >> 3;
        egr_pbmp_ar[lport_byte] |= lport_byte_mask;
        egr_port_num = 1;
    }
    else
    {
        if (0 != memcmp(bcast_da, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN))
        {
            memcpy(addr_entry.mac, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN);
            addr_entry.vid = ing_vid;

            /* check if unicast
             */
            if (TRUE == AMTR_MGR_GetExactAddrEntry(&addr_entry))
            {
                if (addr_entry.ifindex!= ing_lport)
                {
                    ret = PPPOE_IA_ENGINE_LocalDoEgressCheckForUnicast(
                            ing_port_trust, pdu_code, addr_entry.ifindex, drop_reason_p);

                    if (TRUE == ret)
                    {
                        lport_byte_mask = (1 << (7 - ((addr_entry.ifindex - 1) & 7)));
                        lport_byte      = (addr_entry.ifindex - 1) >> 3;
                        egr_pbmp_ar[lport_byte] |= lport_byte_mask;
                        egr_port_num = 1;
                    }
                }

                is_broadcast = FALSE;
            }
        }

        if (TRUE == is_broadcast)
        {
            /* broadcast
             */
            ret = PPPOE_IA_ENGINE_LocalGetEgrPortsForBroadcast(
                    ing_port_trust, pdu_code, &egr_port_num, egr_pbmp_ar);

            /* remove the ingress port
             */
            if ((TRUE == ret) && (egr_port_num > 0))
            {
                lport_byte_mask = (1 << (7 - ((ing_lport - 1) & 7)));
                lport_byte      = (ing_lport - 1) >> 3;
                if (egr_pbmp_ar[lport_byte] & lport_byte_mask)
                {
                    egr_port_num --;
                    egr_pbmp_ar[lport_byte] &= ~lport_byte_mask;
                }
            }
        }
    }

    if (egr_port_num == 0)
    {
        ret = FALSE;
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "\r\n  egr port_num/bc/gener/lport/ret: %ld/%d/%d/%d/%d",
        egr_port_num, is_broadcast, is_gen_er, addr_entry.ifindex, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetEgrPortsForBroadcast
 *-------------------------------------------------------------------------
 * PURPOSE: To get egress lport list to broadcasting the pdu.
 * INPUT  : pdu_code       - ingress PPPoE code
 *          ing_port_trust - trust status of ingress port
 * OUTPUT : egr_pbmp_ar    - pointer to output egress lport list
 *                           >= SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
 *          egr_pnum_p     - pointer to output egress port count
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetEgrPortsForBroadcast(
    BOOL_T  ing_port_trust,
    UI8_T   pdu_code,
    UI32_T  *egr_pnum_p,
    UI8_T   egr_pbmp_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    BOOL_T  ret = FALSE;

    /*  INGRESS   CODE              EGRESS
     *  TRUST     PADO/PADS/PADT    TRUST + UNTRUST (except ingress port)
     *            PADI/PADR         TRUST           (except ingress port)
     *  UNTRUST   PADO/PADS         DROPPED
     *            PADI/PADR/PADT    TRUST
     */

    switch (pdu_code)
    {
    case PPPOE_CODE_PADI:
    case PPPOE_CODE_PADR:
        ret = PPPOE_IA_OM_GetLports(PPPOE_IA_TYPE_PTYPE_TRUST,
                    egr_pbmp_ar, egr_pnum_p);
        break;
    case PPPOE_CODE_PADO:
    case PPPOE_CODE_PADS:
        if (TRUE == ing_port_trust)
        {
            ret = PPPOE_IA_OM_GetLports(PPPOE_IA_TYPE_PTYPE_ENABLED,
                    egr_pbmp_ar, egr_pnum_p);
        }
        break;
    case PPPOE_CODE_PADT:
        if (TRUE == ing_port_trust)
        {
            ret = PPPOE_IA_OM_GetLports(PPPOE_IA_TYPE_PTYPE_ENABLED,
                        egr_pbmp_ar, egr_pnum_p);
        }
        else
        {
            ret = PPPOE_IA_OM_GetLports(PPPOE_IA_TYPE_PTYPE_TRUST,
                        egr_pbmp_ar, egr_pnum_p);
        }
        break;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetGenericErrMsg
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global generic error message.
 * INPUT  : ermsg_len_p  - length of buffer to receive the string
 * OUTPUT : gen_ermsg_ar - pointer to output buffer
 *                         >= PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1
 *          ermsg_len_p  - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetGenericErrMsg(
    UI8_T   gen_ermsg_ar[PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1],
    UI32_T  *ermsg_len_p)
{
    I32_T   gener_len =0;
    BOOL_T  ret = FALSE;

    if ((NULL != gen_ermsg_ar) && (NULL != ermsg_len_p))
    {
        gener_len = *ermsg_len_p;

        ret = PPPOE_IA_OM_GetAdmStrDataByField(
                    0,           PPPOE_IA_TYPE_FLDID_GLOBAL_GEN_ERMSG,
                    &gener_len,  gen_ermsg_ar);

        if (TRUE == ret)
        {
            if (gener_len == 0)
            {
                gener_len = strlen(PPPOE_IA_TYPE_DFLT_GEN_ERMSG);
                if (*ermsg_len_p >= gener_len+1)
                {
                    memcpy (gen_ermsg_ar, PPPOE_IA_TYPE_DFLT_GEN_ERMSG, gener_len+1);
                    *ermsg_len_p = gener_len;
                }
                else
                {
                    ret = FALSE;
                }
            }
            else
            {
                *ermsg_len_p = gener_len;
            }
        }
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
        "gener_len/ret: %ld/%d", gener_len, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetGenericErrTag
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global generic error message.
 * INPUT  : outbuf_len_max - maximum length of output buffer
 * OUTPUT : outbuf_p       - pointer to output buffer
 *                           >= 4 + PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN
 *          out_buf_len_p  - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetGenericErrTag(
    I32_T   outbuf_len_max,
    UI8_T   *outbuf_p,
    UI32_T  *outbuf_len_p)
{
    UI32_T  gener_len = 0;
    UI16_T  gener_tl[2] = {PPPOE_TAGTYPE_GENERIC_ER, 0};
    BOOL_T  ret = FALSE;

    if ((NULL != outbuf_p) && (NULL != outbuf_len_p))
    {
        /*  4 = GENERIC TL(4) //+ END OF LIST TL(4)
         */
        if (outbuf_len_max > 4)
            gener_len = outbuf_len_max - 4;

        /* put generic error TLV
         */
        ret = PPPOE_IA_ENGINE_LocalGetGenericErrMsg(&outbuf_p[4], &gener_len);
        if (TRUE == ret)
        {
            gener_tl[0] = L_STDLIB_Hton16(gener_tl[0]);
            gener_tl[1] = L_STDLIB_Hton16(gener_len);

            memcpy(outbuf_p, gener_tl, 4);
            *outbuf_len_p = 4 + gener_len;
        }
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "egr_len_max/gen_er_tag_len/ret: %ld/%d/%d",
            outbuf_len_max, 4 + gener_tl[1], ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetDefaultCircuitId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the default circut-id for the specified ingress lport/vid.
 * INPUT  : lport         - 1-based ifindex of ingress lport
 *          vid           - vid
 *          is_syntax     - TRUE to get the syntax string (ex: 1/1:vid)
 * OUTPUT : circuit_id_ar - pointer to output buffer
 *                          >= PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1
 *          ccid_len_p    - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : 1. default syntax (i.e. unit/port:vid)
 *-------------------------------------------------------------------------
 */
static UI8_T PPPOE_IA_ENGINE_LocalGetDefaultCircuitId(
    UI32_T  lport,
    UI16_T  vid,
    BOOL_T  is_syntax,
    UI8_T   circuit_id_ar[PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1])
{
    UI32_T              unit, port, trunk_id;
    UI8_T               ccid_len = 0;
    SWCTRL_Lport_Type_T lport_type;

    if (NULL != circuit_id_ar)
    {
        lport_type = SWCTRL_LogicalPortToUserPort(
                        lport, &unit, &port, &trunk_id);

        if (FALSE == is_syntax)
        {
            switch(lport_type)
            {
            case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            case SWCTRL_LPORT_NORMAL_PORT:
                ccid_len = sprintf((char *) circuit_id_ar, PPPOE_IA_TYPE_DFLT_SLOT_PORT_SYNTAX, unit, port, vid);
                break;
            case SWCTRL_LPORT_TRUNK_PORT:
                ccid_len = sprintf((char *) circuit_id_ar, PPPOE_IA_TYPE_DFLT_SLOT_PORT_SYNTAX, unit, trunk_id, vid);
                break;
            default:
            case SWCTRL_LPORT_UNKNOWN_PORT:
                ccid_len = sprintf((char *) circuit_id_ar, PPPOE_IA_TYPE_DFLT_SLOT_PORT_SYNTAX, (UI32_T) 0, lport, vid);
                break;
            }
        }
        else
        {
            switch(lport_type)
            {
            case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            case SWCTRL_LPORT_NORMAL_PORT:
                ccid_len = sprintf((char *) circuit_id_ar, "%ld/%ld:vid", unit, port);
                break;
            case SWCTRL_LPORT_TRUNK_PORT:
                ccid_len = sprintf((char *) circuit_id_ar, "%ld/%ld:vid", unit, trunk_id);
                break;
            default:
                break;
            }
        }
    }

    return ccid_len;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetDefaultRemoteId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the default remote-id for the specified ingress lport.
 * INPUT  : lport         - 1-based ifindex of ingress lport
 * OUTPUT : remote_id_ar  - pointer to output buffer
 *                          >= PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1
 * RETURN : TRUE/FALSE
 * NOTE   : 1. default syntax (i.e. port mac)
 *-------------------------------------------------------------------------
 */
static UI8_T PPPOE_IA_ENGINE_LocalGetDefaultRemoteId(
    UI32_T  lport,
    UI8_T   remote_id_ar[PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1])
{
    UI8_T   port_mac[SYS_ADPT_MAC_ADDR_LEN]= {0};
    UI8_T   rmtid_len = 0;

    if (NULL != remote_id_ar)
    {
#if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE)
        Port_Info_T port_info;
        UI32_T      len_port_name;

        /* use port name if port name is configured
         */
        if (  (TRUE == SWCTRL_GetPortInfo(lport, &port_info))
            &&(port_info.port_name[0] != 0)
           )
        {
            len_port_name = strlen((char *)port_info.port_name);

            if (len_port_name > PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN)
            {
                len_port_name = PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN;
                port_info.port_name[len_port_name] = '\0';
            }

            rmtid_len = len_port_name;
            memcpy(remote_id_ar, port_info.port_name, len_port_name);
        }
        else
#endif /* #if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE) */
        {
            SWCTRL_GetPortMac(lport, port_mac);

            rmtid_len= sprintf((char *) remote_id_ar, PPPOE_IA_ENGINE_MAC_STR_FORMAT,
                                port_mac[0], port_mac[1], port_mac[2],
                                port_mac[3], port_mac[4], port_mac[5]);
        }
    }
    return rmtid_len;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetRemoteId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the remote-id for the specified ingress lport.
 * INPUT  : lport         - 1-based ifindex of ingress lport
 * OUTPUT : remote_id_ar  - pointer to output buffer
 *                          >= PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1
 *          rmtid_len_p   - pointer to length of output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : remote-id may be
 *           1. user configured id
 *           2. default syntax (i.e. port mac) if 1 is not available
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetRemoteId(
    UI32_T  lport,
    UI8_T   remote_id_ar[PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1],
    UI8_T   *rmtid_len_p)
{
    I32_T   remote_id_len =PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1;
    BOOL_T  ret = FALSE;

    if ((NULL != remote_id_ar) && (NULL != rmtid_len_p))
    {
        /* 1. use port remote id if available
         */
        ret = PPPOE_IA_OM_GetAdmStrDataByField(
                    lport,          PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID,
                    &remote_id_len, remote_id_ar);

        /* 2. use port mac
         */
        if ((FALSE == ret) || (remote_id_len == 0))
        {
            remote_id_len = PPPOE_IA_ENGINE_LocalGetDefaultRemoteId(
                                lport, remote_id_ar);
            if (remote_id_len > 0)
                ret = TRUE;
        }

#if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE)
        if (TRUE == ret)
        {
            UI32_T  rid_dascii;
            BOOL_T  tmp_ret, tmp_val;

            tmp_ret = PPPOE_IA_OM_GetBoolDataByField(
                        lport, PPPOE_IA_TYPE_FLDID_PORT_RID_DELIM, &tmp_val);
            if ((TRUE == tmp_ret) && (TRUE == tmp_val))
            {
                tmp_ret = PPPOE_IA_OM_GetUi32DataByField(
                            lport, PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII, &rid_dascii);
                if (TRUE == tmp_ret)
                {
                    if (remote_id_len >= PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN)
                    {
                        remote_id_len = PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN-1;
                    }

                    remote_id_ar[remote_id_len++] = rid_dascii;
                    remote_id_ar[remote_id_len] = '\0';
                }
            }
        }
#endif /* #if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE) */

        PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "rmtid_len/ret: %ld/%d", remote_id_len, ret);

        *rmtid_len_p = remote_id_len;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalGetSvcNameTagFromIngPdu
 *-------------------------------------------------------------------------
 * PURPOSE: To get the Service-Name tag from the ingress pdu
 * INPUT  : ing_pdu_p       - pointer to the input pdu
 *                            right after the PPPoE comman header
 *          ing_pdu_len     - length of the input pdu
 *          egr_pdu_len_max - maximum length for the output buffer
 * OUTPUT : egr_pdu_p       - pointer to the output buffer
 *          egr_pdu_len_p   - length of the output buffer used
 * RETURN : TRUE/FALSE
 * NOTE   : for building generic error reply.
 *-------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_ENGINE_LocalGetSvcNameTagFromIngPdu(
    UI8_T   *ing_pdu_p,
    I32_T   ing_pdu_len,
    I32_T   egr_pdu_len_max,
    UI8_T   *egr_pdu_p,
    UI32_T  *egr_pdu_len_p)
{
    UI32_T  ing_pdu_pos =0;
    UI16_T  ing_tag_dat[2];
    BOOL_T  ret = FALSE;

    if (   (NULL != ing_pdu_p) && (NULL != egr_pdu_p)
        && (NULL != egr_pdu_len_p)
       )
    {
        while (ing_pdu_pos < ing_pdu_len)
        {
            if (ing_pdu_pos +4 > ing_pdu_len)
            {
                break;
            }

            memcpy(ing_tag_dat, &ing_pdu_p[ing_pdu_pos], 4);
            ing_tag_dat[0] = L_STDLIB_Ntoh16(ing_tag_dat[0]);
            ing_tag_dat[1] = L_STDLIB_Ntoh16(ing_tag_dat[1]);

            if (ing_tag_dat[0] == PPPOE_TAGTYPE_SVC_NAME)
            {
                if (egr_pdu_len_max >= ing_tag_dat[1] +4)
                {
                    memcpy(egr_pdu_p, &ing_pdu_p[ing_pdu_pos], 4 + ing_tag_dat[1]);
                    *egr_pdu_len_p = 4 + ing_tag_dat[1];
                    ret = TRUE;
                }
                break;
            }

            ing_pdu_pos += (4 + ing_tag_dat[1]);
        }
    }

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
            "\r\n  egr_len_max/svc_tag_len/ret: %ld/%d/%d",
            egr_pdu_len_max, 4 + ing_tag_dat[1], ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalSendPacket
 *-------------------------------------------------------------------------
 * PURPOSE: To send the packet to the specifed egress lport list.
 * INPUT  : pkt_hdr_p   - pointer to the input packet header
 *          is_gen_er   - TRUE if this is a generic error reply
 *          egr_pdu_p   - pointer to the egress pdu
 *          egr_pdu_len - length of the egress pdu
 *          egr_pbmp_p  - pointer to the egress lport list
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static void PPPOE_IA_ENGINE_LocalSendPacket(
    PPPOE_IA_TYPE_PktHdr_T  *pkt_hdr_p,
    BOOL_T                  is_gen_er,
    UI8_T                   *egr_pdu_p,
    UI32_T                  egr_pdu_len,
    UI8_T                   *egr_pbmp_p)
{
    L_MM_Mref_Handle_T              *mref_p=NULL;
    UI8_T                           *src_mac_p,
                                    *dst_mac_p;
    UI16_T                          ing_vid;
    UI8_T                           new_src_mac[SYS_ADPT_MAC_ADDR_LEN];

    if (  (NULL != pkt_hdr_p) && (NULL != egr_pdu_p)
        &&(NULL != egr_pbmp_p)
       )
    {
        ing_vid = pkt_hdr_p->tag_info & 0xfff;

        if (TRUE == PPPOE_IA_ENGINE_LocalBuildMref(
                            &mref_p, egr_pdu_p, egr_pdu_len))
        {
            if (TRUE == is_gen_er)
            {
                SWCTRL_GetCpuMac(new_src_mac);
                src_mac_p = new_src_mac;
                dst_mac_p = pkt_hdr_p->src_mac;
            }
            else
            {
                src_mac_p = pkt_hdr_p->src_mac;
                dst_mac_p = pkt_hdr_p->dst_mac;
            }

            /* send packet */
            /*
             * for MO platform, if L2MUX_MGR_SendMultiPacket is flooding for vid,
             *   use L2MUX_MGR_SendMultiPacketOnebyOnePort instead.
             */
            L2MUX_MGR_SendMultiPacketByVlan(
                  mref_p,                           /* L_MREF        */
                  dst_mac_p,                        /* dst mac       */
                  src_mac_p,                        /* src mac       */
                  PPPOE_IA_TYPE_PPPOED_ETHER_TYPE,  /* packet type   */
                  ing_vid,                          /* tag_info      */
                  egr_pdu_len,                      /* packet length */
                  egr_pbmp_p,                       /* lport         */
                  pkt_hdr_p->tag_info >> 13);


            PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_ENG,
                    "egr_len/is_gen: %ld/%d", egr_pdu_len, is_gen_er);
        }
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_ENGINE_LocalIncreaseStatistics
 *-------------------------------------------------------------------------
 * PURPOSE: To increase the statistics for specifed lport.
 * INPUT  : ing_lport   - 1-based ifindex of ingress lport
 *          ing_code    - PPPOE_HDR_CODE_E
 *                        PPPoE code
 *          drop_reason - PPPOE_IA_TYPE_E_REQ_UNTRUST
 *                        drop reason if it's dropped
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static void PPPOE_IA_ENGINE_LocalIncreaseStatistics(
    UI32_T                  ing_lport,
    UI8_T                   ing_code,
    UI8_T                   drop_reason)
{
    UI32_T  ing_fld_id =PPPOE_IA_TYPE_FLDID_PORT_STS_UNKNOWN,
            er_fld_id =PPPOE_IA_TYPE_FLDID_PORT_STS_UNKNOWN;

    switch(ing_code)
    {
    case PPPOE_CODE_PADI:
        ing_fld_id = PPPOE_IA_TYPE_FLDID_PORT_STS_PADI;
        break;
    case PPPOE_CODE_PADO:
        ing_fld_id = PPPOE_IA_TYPE_FLDID_PORT_STS_PADO;
        break;
    case PPPOE_CODE_PADR:
        ing_fld_id = PPPOE_IA_TYPE_FLDID_PORT_STS_PADR;
        break;
    case PPPOE_CODE_PADS:
        ing_fld_id = PPPOE_IA_TYPE_FLDID_PORT_STS_PADS;
        break;
    case PPPOE_CODE_PADT:
        ing_fld_id = PPPOE_IA_TYPE_FLDID_PORT_STS_PADT;
        break;
    }

    switch(drop_reason)
    {
    case PPPOE_IA_TYPE_E_REQ_UNTRUST:
        er_fld_id = PPPOE_IA_TYPE_FLDID_PORT_STS_REQ_UNTRUST;
        break;
    case PPPOE_IA_TYPE_E_REP_UNTRUST:
        er_fld_id = PPPOE_IA_TYPE_FLDID_PORT_STS_REP_UNTRUST;
        break;
    case PPPOE_IA_TYPE_E_MALOFORM:
        er_fld_id = PPPOE_IA_TYPE_FLDID_PORT_STS_MALFORM;
        break;
    }

    PPPOE_IA_OM_IncreaseStatisticsByField(ing_lport, ing_fld_id, er_fld_id);
}


