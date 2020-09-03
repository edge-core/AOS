/*-----------------------------------------------------------------------------
 * Module Name: cfm_mgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementation CFM Interface for other component
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/5/2006 - macauley_cheng, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysfun.h"
#include "cfm_mgr.h"
#include "cfm_engine.h"
#include "cfm_backdoor.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "vlan_om.h"
#include "vlan_lib.h"
#include "backdoor_mgr.h"
#include "xstp_om.h"
#include "cfm_om.h"
#include "rule_type.h"
#include "rule_ctrl.h"
#include "l2mux_mgr.h"

#if (SYS_CPNT_CFM == TRUE)

SYSFUN_DECLARE_CSC

enum {
    CFM_MGR_TRACE_ID_PROC_QINQ_PKT =0,
};

static CFM_TYPE_PaceketHeader_T packet_header_g;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_Process_QinQPacket
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the recevied CFMDU.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE - flood
 *            FALSE- not qinq packet.
 * NOTE     : this is call from the cfm_task.c
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_MGR_Process_QinQPacket(
	                        L_MM_Mref_Handle_T	*mref_handle_p,
                            UI8_T 	        dst_mac[6],
                            UI8_T 	        src_mac[6],
                            UI16_T 	        tag_info,
                            UI16_T          type,
                            UI32_T          pkt_length,
                            UI32_T 	        lport)
{/*flood packet when received on access port*/
    L_MM_Mref_Handle_T  *tx_mref_handle_p;
    void                *pdu_rx, *pdu_tx;
    UI32_T              pdu_len;
    BOOL_T              flood=FALSE;

    pdu_rx = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (mref_handle_p->pkt_info.inner_tag_info != 0)
    {/*when packet is dobule taged*/
        flood=TRUE;
    }

    if(flood)
    {
        UI8_T                           *port_list;
    	UI32_T                          i=0;
        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;

        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW,
            "flood qinq (vid/lport-%d/%ld)", (tag_info&0x0fff), (long)lport);

        if (VLAN_OM_GetDot1qVlanCurrentEntry(0, (tag_info&0x0fff), &vlan_info) == FALSE)
        {
            L_MM_Mref_Release(&mref_handle_p);
            return TRUE;
        }

        tx_mref_handle_p = L_MM_AllocateTxBufferForPktForwarding(
            mref_handle_p,
            pdu_len,
            L_MM_USER_ID2(SYS_MODULE_CFM, CFM_MGR_TRACE_ID_PROC_QINQ_PKT));

        if (tx_mref_handle_p == NULL)
        {
            L_MM_Mref_Release(&mref_handle_p);
            return TRUE;
        }

        pdu_tx = L_MM_Mref_GetPdu(tx_mref_handle_p, &pdu_len);
        memcpy(pdu_tx, pdu_rx, pdu_len);

        /*exclusive the recieved port*/
        port_list= vlan_info.dot1q_vlan_current_egress_ports;
        port_list[(lport-1)/8]&=(~(0x80>>((lport-1)%8)));

    #if (SYS_CPNT_MGMT_PORT == TRUE)
        port_list[(SYS_ADPT_MGMT_PORT-1)/8]&=(~(0x80>>((SYS_ADPT_MGMT_PORT-1)%8)));
    #endif

        L2MUX_MGR_SendMultiPacket(tx_mref_handle_p,                            /* L_MREF * */
                                  dst_mac,                                     /* dst mac */
                                  src_mac,                                     /* src mac */
                                  type,                                        /* packet type*/
                                  tag_info,                                    /* tag_info */
                                  pdu_len,                                     /* packet length */
                                  vlan_info.dot1q_vlan_current_egress_ports,   /* lport */
                                  vlan_info.dot1q_vlan_current_untagged_ports,
                                  (tag_info&0xe000)>>13);                      /* transfer priority*/

        L_MM_Mref_Release(&mref_handle_p);
    }

    return flood;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the recevied CFMDU.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : this is call from the cfm_task.c
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessRcvdPDU(
                            L_MM_Mref_Handle_T  *mref_handle_p,
                            UI8_T               dst_mac[6],
                            UI8_T               src_mac[6],
                            UI16_T              tag_info,
                            UI16_T              type,
                            UI32_T              pkt_length,
                            UI32_T              lport)
{
    CFM_TYPE_Msg_T msg_p;;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW, "");

        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if(TRUE == CFM_MGR_Process_QinQPacket(mref_handle_p, dst_mac, src_mac, tag_info, type, pkt_length, lport))
        return;

    msg_p.mem_ref_p=mref_handle_p;

    msg_p.packet_header_p=&packet_header_g;
    memcpy(msg_p.packet_header_p->dstMac,dst_mac,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(msg_p.packet_header_p->srcMac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
    msg_p.packet_header_p->lport = lport;
    msg_p.packet_header_p->tagInfo= tag_info;

    /*check the port exist or trunk member*/
    if(FALSE == SWCTRL_LogicalPortExisting(msg_p.packet_header_p->lport))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW, " logical port %ld oesn't exit", (long)msg_p.packet_header_p->lport);

        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /*check the vlan exit, not implement*/
    if(FALSE == VLAN_OM_IsVlanExisted(msg_p.packet_header_p->tagInfo&0x0fff))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW, " logical port %ld not vlan %d member", (long)msg_p.packet_header_p->lport, msg_p.packet_header_p->tagInfo&0x0fff);

        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /*process the PDU*/

    CFM_ENGINE_ProcessRcvdPDU(&msg_p);

    L_MM_Mref_Release(&mref_handle_p);
}/*End of CFM_MGR_ProcessRcvdPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : Process Timer event.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This only call by CFM_Task.main() one second
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTimerEvent()
{
    CFM_TYPE_CfmStatus_T global_status = CFM_TYPE_CFM_STATUS_DISABLE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    CFM_OM_GetCFMGlobalStatus(&global_status);

    /* if the global status is not enabled, do not handle timer
     */
    if (CFM_TYPE_CFM_STATUS_ENABLE != global_status)
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_TIMER, "cfm disabled globally");
        return;
    }

    CFM_ENGINE_ProcessTimer();
}/*End of CFM_MGR_ProcessTimerEvent*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the bridge operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T CFM_MGR_GetOperationMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}/* End of CFM_MGR_GetOperationMode*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by cfm_task.c
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_MGR_EnterMasterMode()
{
    SYSFUN_ENTER_MASTER_MODE();
    CFM_ENGINE_EnterMasterMode();

    {
        CFM_TYPE_CfmStatus_T status;
        RULE_TYPE_CpuRuleInfo_T control;
        CFM_OM_GetCFMGlobalStatus(&status);

        if(CFM_TYPE_CFM_STATUS_ENABLE == status)
        {
            control.cfm_to_cpu = TRUE;

            RULE_CTRL_TrapPacket2Cpu (TRUE, RULE_TYPE_PacketType_CFM, &control);
        }
        else
        {
            control.cfm_to_cpu = FALSE;

            RULE_CTRL_TrapPacket2Cpu (FALSE, RULE_TYPE_PacketType_CFM, &control);
        }
    }
}/*End of CFM_MGR_EnterMasterMode*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by cfm_task.c
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_MGR_EnterSlaveMode()
{
    SYSFUN_ENTER_SLAVE_MODE();
}/*End of CFM_MGR_EnterSlaveMode*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by cfm_task.c
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_MGR_EnterTransitionMode()
{
   SYSFUN_ENTER_TRANSITION_MODE();
}/*End of CFM_MGR_EnterTransitionMode*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by cfm_task.c
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_MGR_SetTransitionMode()
{
    SYSFUN_SET_TRANSITION_MODE();

    return;
}/* End of CFM_MGR_SetTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initial the semaphore and need default value variable in thie interface
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_MGR_Init()
{
    /* Init backdoor call back functions
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("cfm", SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
                                                      CFM_BACKDOOR_Main);

    return;
}/*End of CFM_MGR_Init*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetMD
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will create or modify the MD
 * INPUT    : md_index - the MD index
 *            *name_ap - the MD name array pointer
 *            name_len - the MD name length
 *            level    - the MD level
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR    - Operation mode error
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetMD(
                                UI32_T md_index,
                                UI8_T *name_ap,
                                UI16_T name_len,
                                CFM_TYPE_MdLevel_T level,
                                CFM_TYPE_MhfCreation_T create_way)
{
    BOOL_T ret_val=FALSE;


    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " md_index=%ld, create_way=%d", (long)md_index, create_way);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((level>CFM_TYPE_MD_LEVEL_7) ||( level <CFM_TYPE_MD_LEVEL_NONE))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((create_way<CFM_TYPE_MHF_CREATION_NONE)||(create_way>CFM_TYPE_MHF_CREATION_EXPLICIT))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(name_len > CFM_TYPE_MD_MAX_NAME_LENGTH)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*if there is a empty char in the middle of name*/
    if(name_len>0  &&  strchr((char *)name_ap, ' '))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    ret_val=CFM_ENGINE_SetMD(md_index,name_ap,name_len,level, create_way);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetMD*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteMD
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MD
 * INPUT    : md_index - the MD index
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at md parameter
 * NOTE     :If there exist MA in this domain, this MD can't be deleted
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteMD(UI32_T md_index)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val=CFM_ENGINE_DeleteMD(md_index);


    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_DeleteMD*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetMANameFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : To set the name format of MA
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            name_format - the name format
 *                          (CFM_TYPE_MA_NAME_CHAR_STRING,
 *                           CFM_TYPE_MA_NAME_ICC_BASED)
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetMANameFormat(
    UI32_T                  md_index,
    UI32_T                  ma_index,
    CFM_TYPE_MA_Name_T      name_format)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (  (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        ||(md_index > SYS_ADPT_CFM_MAX_MD_INDEX)
        ||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX)
        ||(ma_index < SYS_ADPT_CFM_MIN_MA_INDEX)
        ||(ma_index > SYS_ADPT_CFM_MAX_MA_INDEX)
        ||(  (name_format != CFM_TYPE_MA_NAME_CHAR_STRING)
           &&(name_format != CFM_TYPE_MA_NAME_ICC_BASED))
       )
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val = CFM_OM_SetMaNameFormat(md_index, ma_index, name_format);

    return (ret_val == TRUE?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetMANameFormat*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will create or modify the MA info
 * INPUT    : ma_index    - the ma index
 *            name_len    - the ma name
 *            md_index    - the md index
 *            primary_vid - the primary vid of the maS
 *            vlid_list   - the array store the vids
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetMA(
    UI32_T                  ma_index,
    UI8_T                   *name,
    UI32_T                  name_len,
    UI32_T                  md_index,
    UI16_T                  primary_vid,
    UI32_T                  vid_num,
    UI8_T                   vid_list[(SYS_DFLT_DOT1QMAXVLANID/8)+1],
    CFM_TYPE_MhfCreation_T  create_way)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((create_way<CFM_TYPE_MHF_CREATION_NONE)||(create_way>CFM_TYPE_MHF_CREATION_DEFER))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(NULL == name)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((primary_vid!=0)&&((primary_vid>SYS_ADPT_MAX_VLAN_ID)||(primary_vid<1)))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((primary_vid!=0) &&NULL ==vid_list)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((name_len == 0 )|| (name_len > CFM_TYPE_MA_MAX_NAME_LENGTH))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*if there is a empty char in the middle of name*/
    if(strchr((char *)name, ' '))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val = CFM_ENGINE_SetMA(
                ma_index, name, name_len, md_index,
                primary_vid, vid_num, vid_list, create_way);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetMA*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MA
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 * OUTPUT   :
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : If there still exist the Mep in this MA, this MA can't be deleted
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteMA(
                                    UI32_T md_index,
                                    UI32_T ma_index)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val=CFM_ENGINE_DeleteMA(md_index, ma_index);


    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_DeleteMA*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteMAVlan
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MA vlan
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            vid      - the vlan id
 * OUTPUT   :
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : if this vid is primary vid and ma still exit mep, this vid can't be deleted
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteMAVlan(
                                        UI32_T md_index,
                                        UI32_T ma_index,
                                        UI16_T vid)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val=CFM_ENGINE_DeleteMAVlan(md_index, ma_index,vid);


    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_DeleteMAVlan*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_AddnewMEP
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will add new Mep to a MD/MA
 * INPUT    : lport       - the logical port
 *            mep_id      - the mep identifier
 *            md_name_a   - the Md name
 *            md_name_len - the Md name length
 *            ma_name_a   - the Ma name
 *            ma_name_len - the Ma name length
 *            direction   - the direction of mep
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at mep parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_AddnewMEP(
    UI32_T                  lport,
    UI32_T                  mep_id,
    UI8_T                   md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T                  md_name_len,
    UI8_T                   ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T                  ma_name_len,
    CFM_TYPE_MP_Direction_T direction)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((lport<1)||(lport>SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(NULL == ma_name_a)
    {
         return CFM_TYPE_CONFIG_ERROR;
    }

    if((CFM_TYPE_MP_DIRECTION_UP!= direction)&&( CFM_TYPE_MP_DIRECTION_DOWN!=direction))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_CreateMEP(
                lport, mep_id, md_name_a, md_name_len,
                ma_name_a, ma_name_len, direction);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_AddnewMEP*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteMEP
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete a Mep from MD/MA
 * INPUT    : lport       - the lport which mep reside
 *            mep_id      - the mep id
 *            md_name_a   - the array store the Md name
 *            md_name_len - the md_name length
 *            ma_name_a   - the array store the Ma name
 *            ma_name_len - the ma_name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at mep parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteMEP(
    UI8_T   md_name_ap[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T  md_name_len,
    UI8_T   ma_name_ap[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T  ma_name_len,
    UI32_T  lport,
    UI32_T  mep_id)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(lport>SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(mep_id>SYS_ADPT_CFM_MAX_MEP_ID)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((0==lport)&&(0==mep_id))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(NULL == ma_name_ap)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_DeleteMEP(
                lport, mep_id,
                md_name_ap, md_name_len, ma_name_ap, ma_name_len);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_DeleteMEP*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCcmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the CCM transmit interval level
 * INPUT    : md_name_ap  - the md name array pointer
 *            md_name_len - md name length
 *            ma_name_ap  - the ma name array pointer
 *            ma_name_len - ma name length
 *            interval    - the ccm interval level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - configure success
 *            CFM_TYPE_CONFIG_ERROR   - configure fail at interval parameter
 * NOTE     : for our switch use 1 sec. as time unit, so contraint interval level as 1 sec.
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCcmInterval(
    UI8_T                   *md_name_ap,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_ap,
    UI32_T                  ma_name_len,
    CFM_TYPE_CcmInterval_T  interval)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if( (interval<CFM_TYPE_CCM_INTERVAL_1_S)||(interval>CFM_TYPE_CCM_INTERVAL_10_MIN))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val = CFM_ENGINE_SetCcmInterval(
                md_name_ap, md_name_len, ma_name_ap, ma_name_len, interval);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetCcmInterval*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCcmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the CCM transmit status
 * INPUT    : md_name_ap  - the MD name array pointer
 *            md_name_len - the MD name length
 *            ma_name_ap  - the MA name array pointer
 *            ma_name_len - the MA name length
 *            status      - the CFM enable status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at status parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCcmStatus(
    UI8_T                   *md_name_ap,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_ap,
    UI32_T                  ma_name_len,
    CFM_TYPE_CcmStatus_T    status)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if (NULL == ma_name_ap)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if ((status!=CFM_TYPE_CCM_STATUS_DISABLE)&&(status!=CFM_TYPE_CCM_STATUS_ENABLE))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val = CFM_ENGINE_SetCcmStatus(
                md_name_ap, md_name_len, ma_name_ap, ma_name_len, status);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetCcmStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetLinkTraceCacheStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the link trace cache status.
 * INPUT    : status  - the link trace cache status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at status parameter
 * NOTE     : If the link cache status set enable, then the om start to record the link trace reply
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetLinkTraceCacheStatus(CFM_TYPE_LinktraceStatus_T status)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((CFM_TYPE_LINKTRACE_STATUS_DISABLE!=status)&&(CFM_TYPE_LINKTRACE_STATUS_ENABLE!=status))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val= CFM_ENGINE_SetLinkTraceCacheStatus(status);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS: CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetLinkTraceCacheStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetLinkTraceCacheSize
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the link trace reply cache size
 * INPUT    : size - the link trace cache size
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at size parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetLinkTraceCacheSize(UI32_T size)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((CFM_TYPE_MAX_LINKTRACE_SIZE<size)||(CFM_TYPE_MIN_LINKTRACE_SIZE>size))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val= CFM_ENGINE_SetLinkTraceCacheSize(size);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS: CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetLinkTraceCacheSize*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetLinkTraceCacheHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the link trace cache entries hold time
 * INPUT    : hold_time - the link trace cache entries hold time
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at hold_time parameter
 * NOTE     : input in minutes, engine run in second.
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetLinkTraceCacheHoldTime(UI32_T hold_time)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((hold_time>CFM_TYPE_MAX_LINKTRACE_HOLD_TIME)||(hold_time<CFM_TYPE_MIN_LINKTRACE_HOLD_TIME))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*change the unit from minutes to second*/
    ret_val= CFM_ENGINE_SetLinkTraceCacheHoldTime(hold_time*60);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetLinkTraceCacheHoldTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearLinktraceCache
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear all the link trace reply in om
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearLinktraceCache()
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val= CFM_ENGINE_ClearLinkTraceCache();

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_ClearLinktraceCache*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetArchiveHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the remote mep info store hold time
 * INPUT    : md_index  - the md index
 *            hold_time - the remote mep info hold time
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at hold_time parameter
 * NOTE     : input in minutes, engine run in second.
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetArchiveHoldTime(UI32_T md_index, UI32_T hold_time)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index < SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index > SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((hold_time > CFM_TYPE_MAX_ARCHIVE_HOLD_TIME)||(hold_time< CFM_TYPE_MIN_ARCHIVE_HOLD_TIME))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetArchiveHoldTime(md_index,hold_time*60);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetArchiveHoldTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_TransmitLinktrace
 *-------------------------------------------------------------------------
 * PURPOSE  : This function trigger the link trace message be sent
 * INPUT    : src_mep_id  - the source mep id
 *            dst_mep_id  - the destination mep id
 *            mac_addr    - the dest mac address of link trace message
 *            md_name_a   - the md name array
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name array
 *            ma_name_len - the ma name length
 *            ttl         - the time to live
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail transmit linktrace
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_TransmitLinktrace(
    UI32_T  src_mep_id,
    UI32_T  dst_mep_id,
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T  md_name_len,
    UI8_T   ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T  ma_name_len,
    UI32_T  ttl,
    UI16_T  pkt_pri)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if ((ttl>SYS_ADPT_CFM_MAX_LINKTRACE_TTL)||(ttl<SYS_ADPT_CFM_MIN_LINKTRACE_TTL))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if (pkt_pri>CFM_TYPE_MAX_PDU_PRIORITY)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val= CFM_ENGINE_XmitLTM(
                md_name_a, md_name_len, ma_name_a, ma_name_len,
                dst_mep_id, mac_addr, src_mep_id, (UI8_T)ttl, pkt_pri);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_TransmitLinktrace*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_TransmitLoopback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function trigger the loop back message be sent
 * INPUT    : mep_id      - the target mep_id
 *            mac_addr    - the target dest mac
 *            md_name_a   - the md name array
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name array
 *            ma_name_len - the ma name length
 *            counts      - the transmit counts
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail transmit loopback
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_TransmitLoopback(
    UI32_T      mep_id,
    UI8_T       mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T       md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T      md_name_len,
    UI8_T       ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T      ma_name_len,
    UI32_T      counts)
{
    UI32_T  idx;
    BOOL_T  ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "count=%ld", (long)counts);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((counts<CFM_TYPE_MIN_LBM_COUNT)||(counts>CFM_TYPE_MAX_LBM_COUNT))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    for(idx=0;idx<counts;idx++)
    {
        ret_val= CFM_ENGINE_XmitLBM(
                    md_name_a, md_name_len, ma_name_a, ma_name_len, mep_id, mac_addr, 0);
    }
    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_TransmitLoopback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearLoopBackList
 *-------------------------------------------------------------------------
 * PURPOSE  : This function trigger the loop back message be sent
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ClearLoopBackList()
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    CFM_ENGINE_ClearLoopBackList();

    return;
}/*End of CFM_MGR_ClearLoopBackList*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCFMGlobalStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the globl CFM status
 * INPUT    : staus - the CFM status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCFMGlobalStatus(CFM_TYPE_CfmStatus_T status)
{
    BOOL_T                  ret_val=FALSE;
    RULE_TYPE_CpuRuleInfo_T control;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if ((status != CFM_TYPE_CFM_STATUS_ENABLE)&&(status!= CFM_TYPE_CFM_STATUS_DISABLE))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if (CFM_TYPE_CFM_STATUS_ENABLE == status)
    {
        control.cfm_to_cpu = TRUE;
        ret_val=RULE_CTRL_TrapPacket2Cpu (TRUE, RULE_TYPE_PacketType_CFM, &control);
    }
    else
    {
        control.cfm_to_cpu = FALSE;
        ret_val=RULE_CTRL_TrapPacket2Cpu (FALSE, RULE_TYPE_PacketType_CFM, &control);
    }

    if (TRUE == ret_val)
    {
        ret_val=CFM_ENGINE_SetCFMGlobalStatus( status);
    }

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetCFMGlobalStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCFMPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the each port CFM status
 * INPUT    : lport  - the logical port
 *            status - the CFM status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCFMPortStatus(
                                            UI32_T lport,
                                            CFM_TYPE_CfmStatus_T status)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((status != CFM_TYPE_CFM_STATUS_ENABLE)&&(status!= CFM_TYPE_CFM_STATUS_DISABLE))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetCFMPortStatus(lport, status);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetCFMPortStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_AddRemoteMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function add a cross check remote mep
 * INPUT    : md_index - the md index
 *            mep_id   - the mep identifier
 *            ma_name_a- the ma name array
 *            name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_AddRemoteMep(
                                        UI32_T md_index ,
                                        UI32_T mep_id,
                                        UI8_T ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
                                        UI32_T name_len)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index < SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index > SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id > SYS_ADPT_CFM_MAX_MEP_ID)||(mep_id< SYS_ADPT_CFM_MIN_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((NULL == ma_name_a)||(0 == name_len))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_CreateRemoteMep(md_index, ma_name_a, name_len, mep_id);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_AddRemoteMep*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteRemoteMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete a cross check remote mep
 * INPUT    : md_index  - the md index
 *            mep_id    - the mep identifier
 *            ma_name_a - the ma name stored array
 *            name_len  - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - delete failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteRemoteMep(
    UI32_T  md_index,
    UI32_T  mep_id,
    UI8_T   ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T  name_len)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index < SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index > SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((NULL == ma_name_a)||(0 == name_len))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id > SYS_ADPT_CFM_MAX_MEP_ID)||(mep_id< SYS_ADPT_CFM_MIN_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_DeleteRemoteMepByMdIndexMaName(
                md_index, ma_name_a, name_len, mep_id);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_DeleteRemoteMep*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearRemoteMepByDomain
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear remote mep info by MD domain
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 * OUTPUT   : None
 * RETURN   : * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR) - clear failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearRemoteMepAll()
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val =CFM_ENGINE_ClearRemoteMepAll();

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_ClearRemoteMepAll*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearRemoteMepByDomain
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear remote mep info by MD domain
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearRemoteMepByDomain(
                                                UI8_T *md_name_ap,
                                                UI32_T name_len)
{
    BOOL_T ret_val=FALSE;


    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(NULL == md_name_ap )
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val =CFM_ENGINE_ClearRemoteMepByDomain(md_name_ap, name_len);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_ClearRemoteMepByDomain */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearRemoteMepByLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear remote mep info by MD level
 * INPUT    : level - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearRemoteMepByLevel(CFM_TYPE_MdLevel_T level)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((level > CFM_TYPE_MD_LEVEL_7)||(level<CFM_TYPE_MD_LEVEL_0))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val =CFM_ENGINE_ClearRemoteMepByLevel(level);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_ClearRemoteMepByLevel*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCrossCheckStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the cross check status
 * INPUT    : status      - the cross check status
 *            md_name_ap  - the md name array pointer
 *            md_name_len - the md name length
 *            ma_name_ap  - the ma name array pointer
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     : CFM_TYPE_CROSS_CHECK_STATUS_DISABLE,
 *            CFM_TYPE_CROSS_CHECK_STATUS_ENABLE
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCrossCheckStatus(
    UI8_T                       *md_name_ap,
    UI32_T                      md_name_len,
    UI8_T                       *ma_name_ap,
    UI32_T                      ma_name_len,
    CFM_TYPE_CrossCheckStatus_T status)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(ma_name_len>CFM_TYPE_MA_MAX_NAME_LENGTH)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(NULL == ma_name_ap)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val= CFM_ENGINE_SetCrossCheckStatus(
                md_name_ap, md_name_len, ma_name_ap, ma_name_len, status);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetCrossCheckStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCrossCheckStartDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the cross check start delay
 * INPUT    : delay  - the cross check start delay
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCrossCheckStartDelay(UI32_T delay)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((delay>CFM_TYPE_MAX_CROSSCHECK_START_DELAY)||(delay<CFM_TYPE_MIN_CROSSCHECK_START_DELAY))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetCrossCheckStartDelay(delay);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetCrossCheckStartDelay*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetFaultNotifyLowestPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify lowest priority
 * INPUT    : md_ndex  - the md index
 *            priority - the lowest fault notify priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :  this function will set all the mep under this domaiin to this priority
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetFaultNotifyLowestPriority(
                                                UI32_T md_index,
                                                CFM_TYPE_FNG_LowestAlarmPri_T priority)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index < SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index > SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((priority < CFM_TYPE_FNG_LOWEST_ALARM_ALL)||(priority > CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetFaultNotifyLowestPriority(md_index, priority);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetFaultNotifyLowestPriority*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetFaultNotifyAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify alarm time
 * INPUT    : md_index   - the md index
 *            alarm_time - the fault notify alarm time in second
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetFaultNotifyAlarmTime(
                                            UI32_T md_index,
                                            UI32_T alarm_time)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index < SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index > SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((alarm_time > CFM_TYPE_FNG_MAX_ALARM_TIME)||(alarm_time< CFM_TYPE_FNG_MIN_ALARM_TIME))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetFaultNotifyAlarmTime(md_index, alarm_time);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetFaultNotifyAlarmTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetFaultNotifyRestTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify reset time
 * INPUT    : md_index   - the md index
 *            reset_time - the fault notify rest time in second
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set failed
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetFaultNotifyRestTime(
                                                UI32_T md_index,
                                                UI32_T reset_time)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index < SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index > SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((reset_time > CFM_TYPE_FNG_MAX_RESET_TIME)||(reset_time< CFM_TYPE_FNG_MIN_RESET_TIME))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetFaultNotifyResetTime(md_index, reset_time);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetFaultNotifyRestTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearErrorsListByMdNameOrLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the error list
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 *            levle       - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear failed
 * NOTE     : you can specify the md name or the level
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearErrorList()
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val =CFM_OM_ClearErrorsList();

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_ClearErrorList*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearErrorsListByMdNameOrLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the error list
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 *            levle       - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - clear failed
 * NOTE     : you can specify the md name or the level
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearErrorsListByMdNameOrLevel(
                                                    UI8_T *md_name_ap,
                                                    UI32_T name_len,
                                                    CFM_TYPE_MdLevel_T level)
{
    BOOL_T ret_val=FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_ClearErrorsListByMdNameOrLevel(md_name_ap,name_len,level);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
 }/*End of CFM_MGR_ClearErrorsListByMdNameOrLevel*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetSNMPCcStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set which SNMP trap enable
 * INPUT    : trap         - the snamp CC trap type
 *            trap_enabled - the trap status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set faileds
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetSNMPCcStatus(
                                        CFM_TYPE_SnmpTrapsCC_T trap,
                                        BOOL_T trap_enabled)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((trap>CFM_TYPE_SNMP_TRAPS_CC_ALL)||(trap<CFM_TYPE_SNMP_TRAPS_CC_MEP_UP))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_OM_SetSNMPCcStatus(trap,trap_enabled);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetSNMPCcStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetSNMPCrosscheckStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the the SNMP trap status
 * INPUT    : trap        - the snmp cross check trap type
 *            trap_enable - the trap status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set faileds
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetSNMPCrosscheckStatus(
                                            CFM_TYPE_SnmpTrapsCrossCheck_T trap,
                                            BOOL_T trap_enabled)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((trap<CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN)||(trap>CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_ALL))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_OM_SetSNMPCrossCheckStatus(trap,trap_enabled);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetSNMPCrosscheckStatus */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearAisError
 * ------------------------------------------------------------------------
 * PURPOSE  : This function clear ais error
 * INPUT    : mep_id      - the mep id
 *            md_name_p   - pointer to md name
 *            md_name_len - md name length
 *            ma_name_p   - pointer to ma name
 *            ma_name_len - ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set faileds
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearAisError(
    UI32_T  mep_id,
    UI8_T   *md_name_p,
    UI32_T  md_name_len,
    UI8_T   *ma_name_p,
    UI32_T  ma_name_len)
{
    BOOL_T ret=FALSE;

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(ma_name_len <= 0 || NULL == ma_name_p)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret=CFM_OM_RemoveAisErrorByMepMa(
            mep_id, md_name_p, md_name_len, ma_name_p, ma_name_len);

    return (ret==TRUE?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_ClearAisError*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais perirod
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            period      - the ais period
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : CFM_TYPE_AIS_PERIOD_1S, CFM_TYPE_AIS_PERIOD_60S
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetAisPeriod(
    UI8_T   *md_name_a,
    UI32_T  md_name_len,
    UI8_T   *ma_name_a,
    UI32_T  ma_name_len,
    UI32_T  period)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(period!=CFM_TYPE_AIS_PERIOD_1S && period!=CFM_TYPE_AIS_PERIOD_60S)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(ma_name_len <= 0 || NULL == ma_name_a)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_ENGINE_SetAisPeriod(
                    md_name_a, md_name_len, ma_name_a, ma_name_len, period))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return CFM_TYPE_CONFIG_SUCCESS;
}/*End of CFM_MGR_SetAisPeriod*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais level
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            ais_level   - the ais level
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : level 0 is default
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetAisLevel(
    UI8_T               *md_name_a,
    UI32_T              md_name_len,
    UI8_T               *ma_name_a,
    UI32_T              ma_name_len,
    CFM_TYPE_MdLevel_T  ais_level)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(ma_name_len <= 0 || NULL == ma_name_a)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(ais_level > CFM_TYPE_MD_LEVEL_7 || ais_level<CFM_TYPE_MD_LEVEL_0)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_a, md_name_len, ma_name_a, ma_name_len, &md_p, &ma_p))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /* AIS send level must > md's level (this ma's)
     */
    if(  (ais_level != SYS_DFLT_CFM_AIS_LEVEL)
       &&(ais_level <= md_p->level)
      )
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_SetAisLevel(md_p->index, ma_p->index, ais_level))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return CFM_TYPE_CONFIG_SUCCESS;
}/*End of CFM_MGR_SetAisLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais status
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            ais_status  - the ais status
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetAisStatus(
    UI8_T                   *md_name_a,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_a,
    UI32_T                  ma_name_len,
    CFM_TYPE_AIS_STATUS_T   ais_status)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(ma_name_len <= 0 || NULL == ma_name_a)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(ais_status!=CFM_TYPE_AIS_STATUS_DISABLE
       &&ais_status!=CFM_TYPE_AIS_STATUS_ENABLE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_ENGINE_SetAisStatus(
                    md_name_a, md_name_len, ma_name_a, ma_name_len, ais_status))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return CFM_TYPE_CONFIG_SUCCESS;
}/*End of CFM_MGR_SetAisStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetAisSuppressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais supress status
 * INPUT    : md_name_a          - the md name
 *            md_name_len        - the md name length
 *            ma_name_a          - the ma name
 *            ma_name_len        - the ma name length
 *            ais_supress_status - the ais suppress status
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetAisSuppressStatus(
    UI8_T                   *md_name_a,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_a,
    UI32_T                  ma_name_len,
    CFM_TYPE_AIS_STATUS_T   ais_suppress_status)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(ma_name_len <= 0 || NULL == ma_name_a)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(ais_suppress_status!=CFM_TYPE_AIS_STATUS_ENABLE
       &&ais_suppress_status!=CFM_TYPE_AIS_STATUS_DISABLE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMdMaByMdMaName(
                    md_name_a, md_name_len, ma_name_a, ma_name_len, &md_p, &ma_p))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_SetAisSuppressStatus(md_p->index, ma_p->index, ais_suppress_status))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return CFM_TYPE_CONFIG_SUCCESS;
}/*End of CFM_MGR_SetAisSuppressStatus*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the md
 * INPUT    : md_index  - the md index
 *            md_format - the md format
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMd(UI32_T md_index)
{
    UI8_T   default_md_name[]={"DEFAULT"};
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMD(md_index, default_md_name, strlen((char *)default_md_name), SYS_DFLT_CFM_MD_LEVEL, SYS_DFLT_CFM_MIP_CREATE);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMd*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteDot1agCfmMd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MD
 * INPUT    : md_index - the MD index
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at md parameter
 *            CFM_TYPE_CONFIG_ERROR    - Operation mode error
 * NOTE     :1. If there exist MA in this domain, this MD can't be deleted
 *           2. for mib
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteDot1agCfmMd(UI32_T md_index)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_DeleteMD(md_index);

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_DeleteDot1agCfmMd*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the md name formate
 * INPUT    : md_index  - the md index
 *            md_format - the md format
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function only for MIB
 *            2. only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdFormat(
                                                UI32_T md_index,
                                                CFM_TYPE_MD_Name_T md_format)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(CFM_TYPE_MD_NAME_CHAR_STRING != md_format)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return CFM_TYPE_CONFIG_SUCCESS;
}/*End of CFM_MGR_SetDot1agCfmMdFormat*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the md name
 * INPUT    : md_index    - the md index
 *            *name_ap    - the md name array pointer
 *            name_length - the md name length
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdName(
                                            UI32_T md_index,
                                            UI8_T *name_ap,
                                            UI32_T name_length)
{
    BOOL_T ret_val=FALSE;
    CFM_OM_MdInfo_T md_info;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((NULL == name_ap)||(name_length == 0))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((name_length>CFM_TYPE_MD_MAX_NAME_LENGTH)||(name_length<CFM_TYPE_MD_MIN_NAME_LENGTH))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMdInfo(md_index, &md_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*if there is a empty char in the middle of name*/
    if(strchr((char *)name_ap, ' '))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMD(md_index, name_ap, name_length, md_info.level, md_info.mhf_creation);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMdName*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the md level
 * INPUT    : md_index  - the md index
 *            level     - the md level
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdLevel(
                                            UI32_T md_index,
                                            CFM_TYPE_MdLevel_T level)
{
    BOOL_T ret_val=FALSE;
    CFM_OM_MdInfo_T md_info;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((level>CFM_TYPE_MD_LEVEL_7) ||( level <CFM_TYPE_MD_LEVEL_NONE))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMdInfo(md_index, &md_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMD(md_index, md_info.name_a,strlen((char *)md_info.name_a), level, md_info.mhf_creation);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMdLevel*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *            create_type - the mhf create type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdMhfCreation(
                                                    UI32_T md_index,
                                                    CFM_TYPE_MhfCreation_T create_type)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " md_index=%ld, create_type=%d", (long)md_index, create_type);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((CFM_TYPE_MHF_CREATION_NONE>create_type)||(CFM_TYPE_MHF_CREATION_EXPLICIT<create_type))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_OM_SetMdMhfCreation(md_index, create_type);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMdMhfCreation*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
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
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdMhfIdPermission (
                                                        UI32_T md_index,
                                                        CFM_TYPE_MhfIdPermission_T send_id_permission)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_OM_SetMdMhfIdPermission(md_index, send_id_permission);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMdMhfIdPermission */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMa
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma nam formate
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            format    - the ma name format
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :  only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMa(
                                        UI32_T md_index,
                                        UI32_T ma_index)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val=CFM_ENGINE_CreateMA(md_index, ma_index);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMa*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteDot1agCfmMa
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MA
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 * OUTPUT   :
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. If there still exist the Mep in this MA, this MA can't be deleted
 *           2. for mib only
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteDot1agCfmMa(
                                            UI32_T md_index,
                                            UI32_T ma_index)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val=CFM_ENGINE_DeleteMA(md_index, ma_index);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);

}/*End of CFM_MGR_DeleteDot1agCfmMa*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma nam formate
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            format    - the ma name format
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     :  only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaFormat(
                                                UI32_T md_index,
                                                UI32_T ma_index,
                                                CFM_TYPE_MA_Name_T format)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(CFM_TYPE_MA_NAME_CHAR_STRING!= format)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    return CFM_TYPE_CONFIG_SUCCESS;
}/*End of CFM_MGR_SetDot1agCfmMaFormat*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma name
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            *name_ap    - the ma name array pointer
 *            name_length - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaName(
    UI32_T  md_index,
    UI32_T  ma_index,
    UI8_T   *name_ap,
    UI32_T  name_length)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((NULL==name_ap)||(0==name_length))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*if there is a empty char in the middle of name*/
    if(strchr((char *)name_ap, ' '))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMaName(md_index, ma_index, name_ap, name_length);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);

}/*End of CFM_MGR_SetDot1agCfmMaName*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma mhf creation type
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            create_type -the mhf cration type
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaMhfCreation(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    CFM_TYPE_MhfCreation_T create_type)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((CFM_TYPE_MHF_CREATION_NONE>create_type)||(CFM_TYPE_MHF_CREATION_DEFER<create_type))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val=CFM_ENGINE_SetMaMhfCreation(md_index, ma_index, create_type);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMaMhfCreation*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           ma_index   - the ma index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaMhfIdPermission(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    CFM_TYPE_MhfIdPermission_T send_id_permission)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index <SYS_ADPT_CFM_MIN_MD_INDEX)||(md_index >SYS_ADPT_CFM_MAX_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val=CFM_OM_SetMaMhfIdPermission(md_index, ma_index, send_id_permission);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);

}/*End of CFM_MGR_SetDot1agCfmMaMhfIdPermission*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaCcmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma send ccm interval
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            interval  - the sending ccm interval
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *            only support 4-7
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaCcmInterval(
    UI32_T                  md_index,
    UI32_T                  ma_index,
    CFM_TYPE_CcmInterval_T  interval)
{
    CFM_OM_MdInfo_T     md_info;
    CFM_OM_MaInfo_T     ma_info;
    BOOL_T              ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if( (interval<CFM_TYPE_CCM_INTERVAL_1_S)||(interval>CFM_TYPE_CCM_INTERVAL_10_MIN))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMdInfo(md_index, &md_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMaInfo(md_index, ma_index, &ma_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetCcmInterval(
                md_info.name_a,    strlen((char *)md_info.name_a),
                ma_info.ma_name_a, strlen((char *)ma_info.ma_name_a), interval);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMaCcmInterval*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaMepListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma mep id list
 * INPUT    : md_index        - the md index
 *            ma_index        - the ma index
 *            mep_id          - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. not support this function, it will just return false
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaMepListEntry(
                                                UI32_T md_index,
                                                UI32_T ma_index,
                                                UI32_T mep_id)
{
    BOOL_T ret_val=FALSE;
    CFM_OM_MaInfo_T ma_info;
    CFM_OM_MepInfo_T mep_info;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id < SYS_ADPT_CFM_MIN_MEP_ID) ||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMaInfo(md_index, ma_index, &ma_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*if the mep id already been create on local mep, then skip this*/
    if(TRUE==CFM_OM_GetMepInfo(md_index, ma_index, mep_id, &mep_info))
    {
        return CFM_TYPE_CONFIG_SUCCESS;
    }

    /*add the remote mep*/

    ret_val=CFM_ENGINE_CreateRemoteMep(md_index, ma_info.ma_name_a, strlen((char *)ma_info.ma_name_a), mep_id);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMaMepListEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteDot1agCfmMaMepListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the ma mep id list
 * INPUT    : md_index        - the md index
 *            ma_index        - the ma index
 *            mep_id          - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. not support this function, it will just return false
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteDot1agCfmMaMepListEntry(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    UI32_T mep_id)
{
    BOOL_T ret_val=FALSE;
    CFM_OM_MaInfo_T ma_info;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMaInfo(md_index, ma_index, &ma_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*add the remote mep*/

    ret_val=CFM_ENGINE_DeleteRemoteMepByMdIndexMaName(md_index, ma_info.ma_name_a, strlen((char *)ma_info.ma_name_a), mep_id);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_DeleteDot1agCfmMaMepListEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmNumOfVids
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma can have more than on vid
 * INPUT    : md_index   - the md index
 *            ma_index   - the ma index
 *            vid_num    - can have more than one vid
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmNumOfVids(
                                                UI32_T md_index,
                                                UI32_T ma_index,
                                                UI32_T vid_num)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val=CFM_OM_SetMaNumOfVids(md_index, ma_index, vid_num);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmNumOfVids*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaPrimaryVlanVid
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma vlan id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            primary_vid - the vlan id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaPrimaryVlanVid(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    UI16_T primary_vid)
{
    CFM_OM_MaInfo_T ma_info;
    UI8_T vid_list [(SYS_DFLT_DOT1QMAXVLANID / 8)+ 1 ]={0};
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if( (primary_vid>SYS_ADPT_MAX_VLAN_ID)||(primary_vid<1))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMaInfo(md_index, ma_index, &ma_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    vid_list[(UI32_T)((primary_vid-1)/8)] |= (1 << (7 - ((primary_vid-1)%8)));


    ret_val=CFM_ENGINE_SetMA(ma_index, ma_info.ma_name_a, strlen((char *)ma_info.ma_name_a), md_index, primary_vid, 1, vid_list, ma_info.mhf_creation);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMaPrimaryVlanVid*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the new mep
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepEntry(
    UI32_T  md_index,
    UI32_T  ma_index,
    UI32_T  mep_id)
{
    CFM_OM_MaInfo_T     ma_info;
    CFM_OM_MdInfo_T     md_info;
    BOOL_T              ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMdInfo(md_index, &md_info))
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_GetMaInfo(md_index, ma_index, &ma_info))
    {
        return FALSE;
    }

    ret_val=CFM_ENGINE_CreateMEP(
                0, mep_id,
                md_info.name_a,    strlen((char *)md_info.name_a),
                ma_info.ma_name_a, strlen((char *)ma_info.ma_name_a),
                CFM_TYPE_MP_DIRECTION_DOWN);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteDot1agCfmMepEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the mep entry
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteDot1agCfmMepEntry(
    UI32_T  md_index,
    UI32_T  ma_index,
    UI32_T  mep_id)
{
    CFM_OM_MaInfo_T     ma_info;
    CFM_OM_MdInfo_T     md_info;
    BOOL_T              ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMdInfo(md_index, &md_info))
    {
        return FALSE;
    }

    if(FALSE == CFM_OM_GetMaInfo(md_index, ma_index, &ma_info))
    {
        return FALSE;
    }

    ret_val=CFM_ENGINE_DeleteMEP(
                0, mep_id,
                md_info.name_a,    strlen((char *)md_info.name_a),
                ma_info.ma_name_a, strlen((char *)ma_info.ma_name_a));

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepIfIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's logical port
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ifindex   - the logical port
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. the mep can't be configured on trunk member
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepIfIndex(
                                                UI32_T md_index,
                                                UI32_T ma_index,
                                                UI32_T mep_id,
                                                UI32_T ifindex)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    {
        UI32_T trunk_ifindex=0;
        BOOL_T is_static=FALSE;

        if(TRUE == SWCTRL_IsTrunkMember(ifindex, &trunk_ifindex, &is_static))
        {
            return CFM_TYPE_CONFIG_ERROR;
        }
    }

    ret_val=CFM_ENGINE_SetMepLport(md_index, ma_index, mep_id, ifindex);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepIfIndex*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepDirection
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            direction  - the mep direction
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepDirection(
                                            UI32_T md_index,
                                            UI32_T ma_index,
                                            UI32_T mep_id,
                                            CFM_TYPE_MP_Direction_T direction )
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMepDirection(md_index, ma_index, mep_id, direction);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepDirection*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepPrimaryVid
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepPrimaryVid(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    UI32_T mep_id,
                                                    UI16_T primary_vid)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val=CFM_ENGINE_SetMepPrimaryVid(md_index, ma_index, mep_id, primary_vid);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepPrimaryVid*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepActive
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's active status
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            active    - the mep active status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepActive(
                                                UI32_T md_index,
                                                UI32_T ma_index,
                                                UI32_T mep_id,
                                                BOOL_T active)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_OM_SetMepActive(md_index, ma_index, mep_id, active);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepActive*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepCciEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            status    - the cci status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. CFM_TYPE_CCI_STATUS_ENABLE
 *               CFM_TYPE_CCI_STATUS_DISABLE
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepCciEnable(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    UI32_T mep_id,
                                                    CFM_TYPE_CcmStatus_T cci_status)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMepCciStatus(md_index, ma_index, mep_id, cci_status);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepCciEnable*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepCcmLtmPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ccm_ltm_priority- the ccm and ltm default priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepCcmLtmPriority(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    UI32_T mep_id,
                                                    UI32_T ccm_ltm_priority)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMepCcmLtmPriority(md_index, ma_index, mep_id, ccm_ltm_priority);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepCcmLtmPriority*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepLowPrDef
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lowest alarm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            low_pri   - the lowest priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. CFM_TYPE_FNG_LOWEST_ALARM_ALL
 *               CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepLowPrDef(
                                                UI32_T md_index,
                                                UI32_T ma_index,
                                                UI32_T mep_id,
                                                CFM_TYPE_FNG_LowestAlarmPri_T low_pri)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMepLowPrDef(md_index, ma_index, mep_id, low_pri);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepLowPrDef*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepFngAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fng alarm time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            alarm_time  - the fault alarm time by ticks
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1.This function only for MIB
 *            2. only supoort 300...10000 ticks
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepFngAlarmTime(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    UI32_T mep_id,
                                                    UI32_T alarm_time)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*change ticks to sec.*/
    alarm_time=alarm_time/100;


    ret_val=CFM_ENGINE_SetMepFngAlarmTime(md_index, ma_index, mep_id, alarm_time);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepFngAlarmTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepFngResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fault alarm reset time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            reset_time- the fault alarm reset time by ticks
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1.This function only for MIB
 *            2. only supoort 300...10000 ticks
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepFngResetTime(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    UI32_T mep_id,
                                                    UI32_T reset_time)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*change ticks to sec*/
    reset_time=reset_time/100;

    ret_val=CFM_ENGINE_SetMepFngResetTime(md_index, ma_index, mep_id, reset_time);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepFngResetTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function transmit the lbm
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            lbm_status - the lbm status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmStatus(
    UI32_T  md_index,
    UI32_T  ma_index,
    UI32_T  mep_id)
{
    UI32_T              idx=0;
    CFM_OM_MaInfo_T     ma_info;
    CFM_OM_MdInfo_T     md_info;
    CFM_OM_MepInfo_T    mep_info;
    BOOL_T              ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMdInfo(md_index, &md_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMaInfo(md_index, ma_index, &ma_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMepInfo(md_index, ma_index, mep_id, &mep_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /*clear the old store infor*/
    CFM_ENGINE_ClearLoopBackList();

    if(FALSE == mep_info.active)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    for(idx=0;idx<mep_info.trans_lbm_msg;idx++)
    {
        if(TRUE == mep_info.trans_lbm_dst_is_mep_id)
        {
            ret_val= CFM_ENGINE_XmitLBM(
                        md_info.name_a,    strlen((char *)md_info.name_a),
                        ma_info.ma_name_a, strlen((char *)ma_info.ma_name_a),
                        mep_info.trans_lbm_dst_mep_id, NULL, mep_id);
        }
        else
        {
            ret_val= CFM_ENGINE_XmitLBM(
                        md_info.name_a,    strlen((char *)md_info.name_a),
                        ma_info.ma_name_a, strlen((char *)ma_info.ma_name_a),
                        0, mep_info.trans_lbm_dst_mac_addr_a, mep_id);
        }
    }

    return (ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLbmStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmDestMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination mac address
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mac   - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmDestMacAddress(
                                                            UI32_T md_index,
                                                            UI32_T ma_index,
                                                            UI32_T mep_id,
                                                            UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMepLbmDestMacAddress(md_index, ma_index, mep_id, dst_mac);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLbmDestMacAddress*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmDestMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lbm destination mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mep_id - the lbm destination mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmDestMepId(
                                                        UI32_T md_index,
                                                        UI32_T ma_index,
                                                        UI32_T mep_id,
                                                        UI32_T dst_mep_id)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMeptransmitLbmDestMepId(md_index, ma_index, mep_id, dst_mep_id);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLbmDestMepId*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmDestIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination use mep id or mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - set the destination address is mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmDestIsMepId(
                                                        UI32_T md_index,
                                                        UI32_T ma_index,
                                                        UI32_T mep_id,
                                                        BOOL_T is_mep_id )
{
    BOOL_T ret_val=FALSE;

   CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMepTransmitLbmDestIsMepId(md_index, ma_index, mep_id, is_mep_id);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLtmTargetIsMepId*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmMessages
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit the lbm counts
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            counts    - the lbm message counts
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmMessages(
                                                        UI32_T md_index,
                                                        UI32_T ma_index,
                                                        UI32_T mep_id,
                                                        UI32_T counts)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }


    ret_val= CFM_ENGINE_SetMepTransmitLbmMessages(md_index, ma_index, mep_id, counts);


    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLbmMessages*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmDataTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's tranmit lbm include data
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. this not suuport, it will just retrun fail
 *            3. not supported yet
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmDataTlv(
                                                        UI32_T md_index,
                                                        UI32_T ma_index,
                                                        UI32_T mep_id,
                                                        UI8_T *content,
                                                        UI32_T content_length)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /* not supported yet...
     */
    return CFM_TYPE_CONFIG_ERROR;
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLbmDataTlv*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmVlanPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit blm vlan priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            priority  - the lbm priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmVlanPriority(
                                                            UI32_T md_index,
                                                            UI32_T ma_index,
                                                            UI32_T mep_id,
                                                            UI32_T priority)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMepTransmitLbmVlanPriority(md_index, ma_index, mep_id, priority);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLbmVlanPriority*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmVlanDropEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's vlan drop ability
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_enable - the lbm vlan drop enable
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. this is not support will just return fail
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmVlanDropEnable(
                                                            UI32_T md_index,
                                                            UI32_T ma_index,
                                                            UI32_T mep_id,
                                                            BOOL_T is_enable)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /* not supported yet...
     */
    return CFM_TYPE_CONFIG_ERROR;
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLbmVlanDropEnable*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmFlags
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm transmit flags
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmStatus(
    UI32_T  md_index,
    UI32_T  ma_index,
    UI32_T  mep_id)
{
    CFM_OM_MaInfo_T     ma_info;
    CFM_OM_MdInfo_T     md_info;
    CFM_OM_MepInfo_T    mep_info;
    BOOL_T              ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMdInfo(md_index, &md_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMaInfo(md_index, ma_index, &ma_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if(FALSE == CFM_OM_GetMepInfo(md_index, ma_index, mep_id, &mep_info))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    if(FALSE == mep_info.active)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }
    if(TRUE == mep_info.trans_ltm_target_is_mep_id)
    {
        ret_val=CFM_ENGINE_XmitLTM(
                    md_info.name_a,                     md_info.name_len,
                    ma_info.ma_name_a,                  ma_info.ma_name_len,
                    mep_info.trans_ltm_target_mep_id,   NULL,
                    mep_id,                             mep_info.trans_ltm_ttl,
                    mep_info.ccm_ltm_priority);
    }
    else
    {
        ret_val=CFM_ENGINE_XmitLTM(
                    md_info.name_a,     md_info.name_len,
                    ma_info.ma_name_a,  ma_info.ma_name_len,
                    0,                  mep_info.trans_ltm_target_mac_addr_a,
                    mep_id,             mep_info.trans_ltm_ttl,
                    mep_info.ccm_ltm_priority);
    }

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmFlags
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm transmit flags
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 *            is_useFDBonly - set only use the fdb or not
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmFlags(
                                                        UI32_T md_index,
                                                        UI32_T ma_index,
                                                        UI32_T mep_id,
                                                        BOOL_T is_useFDBonly)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetMepTransmitLtmUseFDBOnly(md_index, ma_index, mep_id, is_useFDBonly);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLtmFlags*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mac-the ltm target address
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMacAddress(
                                                                UI32_T md_index,
                                                                UI32_T ma_index,
                                                                UI32_T mep_id,
                                                                UI8_T target_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMacAddress(md_index, ma_index, mep_id, target_mac);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMacAddress*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mp id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mep_id - the target mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMepId(
                                                            UI32_T md_index,
                                                            UI32_T ma_index,
                                                            UI32_T mep_id,
                                                            UI32_T target_mep_id)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetDot1agCfmMepTransmitLtmTargetMepId(md_index, ma_index, mep_id, target_mep_id);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMepId*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmTargetIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm target is the mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - the ltm target is the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmTargetIsMepId(
                                                            UI32_T md_index,
                                                            UI32_T ma_index,
                                                            UI32_T mep_id,
                                                            BOOL_T is_mep_id)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetDot1agCfmMepTransmitLtmTaragetIsMepId(md_index, ma_index, mep_id, is_mep_id);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLtmTargetIsMepId*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmTtl
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm ttl
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ttl       - the trnamsmit ttl
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmTtl(
                                                    UI32_T md_index,
                                                    UI32_T ma_index,
                                                    UI32_T mep_id,
                                                    UI32_T ttl)
{
    BOOL_T ret_val=FALSE;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((md_index > SYS_ADPT_CFM_MAX_MD_INDEX)||(md_index < SYS_ADPT_CFM_MIN_MD_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((ma_index<SYS_ADPT_CFM_MIN_MA_INDEX) || (ma_index>SYS_ADPT_CFM_MAX_MA_INDEX))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if((mep_id<SYS_ADPT_CFM_MIN_MEP_ID)||(mep_id>SYS_ADPT_CFM_MAX_MEP_ID))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    ret_val=CFM_ENGINE_SetDot1agCfmMepTransmitLtmTtl(md_index, ma_index, mep_id, ttl);

    return(ret_val?CFM_TYPE_CONFIG_SUCCESS:CFM_TYPE_CONFIG_ERROR);
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLtmTtl*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmEgressIdentifier
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's egress identifier
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            identifer - the egress identifier
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : 1. This function only for MIB
 *            2. this ability not suport, it will just return fail
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmEgressIdentifier(
                                                        UI32_T md_index,
                                                        UI32_T ma_index,
                                                        UI32_T mep_id,
                                                        UI8_T identifer[8])
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /* not supported yet...
     */
    return CFM_TYPE_CONFIG_ERROR;
}/*End of CFM_MGR_SetDot1agCfmMepTransmitLtmEgressIdentifier*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmDefaultMdLevelMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set default md level mhf creation type
 * INPUT    : md_primary_vid  - the md primary vid
 *           creation_type    - the mhf creation type
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *           default is none
 *            not supported yet
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmDefaultMdLevelMhfCreation(
                                                        UI16_T md_primary_vid,
                                                        CFM_TYPE_MhfCreation_T creation_type)
{

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /* not supported yet...
     */
    return CFM_TYPE_CONFIG_ERROR;
}/*End of CFM_MGR_SetDot1agCfmDefaultMdLevelMhfCreation*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmDefaultMdLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set default md level
 * INPUT    : md_primary_vid  - the md primary vid
 *           level            - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *           default is none
 *            not supported yet
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmDefaultMdLevel(
                                                    UI16_T md_primary_vid,
                                                    CFM_TYPE_MdLevel_T level)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /* not supported yet...
     */
    return CFM_TYPE_CONFIG_ERROR;
}/*End of CFM_MGR_SetDot1agCfmDefaultMdLevel*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmDefaultMdIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set default md level
 * INPUT    : md_primary_vid  - the md primary vid
 *           level            - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 *           default is none
 *            not supported yet
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmDefaultMdIdPermission(
                                                    UI16_T md_primary_vid,
                                                    CFM_TYPE_MhfIdPermission_T send_id_permission)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    /* not supported yet...
     */
    return CFM_TYPE_CONFIG_ERROR;
}/*End of CFM_MGR_SetDot1agCfmDefaultMdIdPermission*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTrunkAdd1stMember_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *           member_ifindex  - the member ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : when the membe_ifindex become the trunk port, the all mep on this port
 *           should be deleted
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTrunkAdd1stMember_Callback(
                                            UI32_T trunk_ifindex,
                                            UI32_T member_ifindex)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "t_if=%ld, m_if=%ld",
            (long)trunk_ifindex, (long)member_ifindex);
    CFM_ENGINE_ProcessTrunkAdd1stMemeber(trunk_ifindex, member_ifindex);

}/*end of CFM_MGR_ProcessTrunkAdd1stMember_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTrunkAddMember_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *           member_ifindex  - the member ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : when the membe_ifindex become the trunk port, the all mep on this port
 *           should be deleted
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTrunkAddMember_Callback(
                                            UI32_T trunk_ifindex,
                                            UI32_T member_ifindex)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "t_if=%ld, m_if=%ld",
            (long)trunk_ifindex, (long)member_ifindex);
    CFM_ENGINE_ProcessTrunkAddMemeber(trunk_ifindex, member_ifindex);

}/*end of CFM_MGR_ProcessTrunkAddMember_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTrunkDeleteLastMember_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member remote last membercall back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *           member_ifindex  - the member ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : when the trunk_ifindex has no trunk port, the all mep on this port
 *           should be deleted
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTrunkDeleteLastMember_Callback(
                                                UI32_T trunk_ifindex,
                                                UI32_T member_ifindex)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "t_if=%ld, m_if=%ld",
            (long)trunk_ifindex, (long)member_ifindex);
    CFM_ENGINE_ProcessTrunkDeleteLastMemeber(trunk_ifindex, member_ifindex);

}/*end of CFM_MGR_ProcessTrunkDeleteLastMember_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTrunkMemberDelete_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member remove member call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *           member_ifindex  - the member ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : when the trunk_ifindex has no trunk port, the all mep on this port
 *           should be deleted
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTrunkMemberDelete_Callback(
                                                UI32_T trunk_ifindex,
                                                UI32_T member_ifindex)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "t_if=%ld, m_if=%ld",
            (long)trunk_ifindex,(long)member_ifindex);
    CFM_ENGINE_ProcessTrunkMemberDelete(trunk_ifindex, member_ifindex);

}/*end of CFM_MGR_ProcessTrunkAddMember_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessInterfaceStatusChange_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the interface status change call back
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessInterfaceStatusChange_Callback(UI32_T ifindex)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "if=%ld", (long)ifindex);

    /*filter the non logical port*/
    if(FALSE == SWCTRL_LogicalPortExisting(ifindex))
    {
        return;
    }

    CFM_ENGINE_ProcessInterfaceStatusChange(ifindex);

}/*end of CFM_MGR_ProcessInterfaceStatusChange_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortStatusChange_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port status change call back
 * INPUT    : ifindex  - the port ifindex
 *           vid       - the vlan id
 *           is_forwarding - the xstp status is forarding or not
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortStatusChange_Callback(
                                                UI16_T vid,
                                                UI32_T ifindex,
                                                BOOL_T is_forwarding)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " vid=%d, t_if=%ld, is_forwarding=%s",
            vid, (long)ifindex, (is_forwarding?"forwarding":"not-forwarding"));
    /*filter the non logical port*/
    if(FALSE == SWCTRL_LogicalPortExisting(ifindex))
    {
        return;
    }

    CFM_ENGINE_ProcessPortStatusChange(vid, ifindex, is_forwarding);

}/*end of CFM_MGR_ProcessPortStatusChange_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortEnterForwarding
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port enter forwarding
 * INPUT    : xstp_id   - the spanning tree id
 *           ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : for sys_callback_mgr, cfm_group will call this
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortEnterForwarding(
                                        UI32_T xstp_id,
                                        UI32_T ifindex)
{
    UI32_T nxt_vid=0;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " xstp_id=%d, ifindex=%ld", xstp_id, (long)ifindex);
    /*filter the non logical port*/
    if(FALSE == SWCTRL_LogicalPortExisting(ifindex))
    {
        return;
    }

    while(TRUE == XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(xstp_id, &nxt_vid))
    {
        if(TRUE == VLAN_OM_IsVlanExisted(nxt_vid))
        {
            CFM_ENGINE_ProcessPortStatusChange(nxt_vid, ifindex, TRUE);
        }
    }

    return;
}/*end of CFM_MGR_ProcessPortEnterForwarding*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortLeaveForwarding
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port leave forwarding
 * INPUT    : xstp_id   - the spanning tree id
 *           ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : for sys_callback_mgr, cfm_group will call this
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortLeaveForwarding(
                                        UI32_T xstp_id,
                                        UI32_T ifindex)
{
    UI32_T nxt_vid=0;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " xstp_id=%d, ifindex=%ld", xstp_id, (long)ifindex);

    /*filter the non logical port*/
    if(FALSE == SWCTRL_LogicalPortExisting(ifindex))
    {
        return;
    }

    while(TRUE == XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(xstp_id, &nxt_vid))
    {
        if(TRUE == VLAN_OM_IsVlanExisted(nxt_vid))
        {
            CFM_ENGINE_ProcessPortStatusChange(nxt_vid, ifindex, FALSE);
        }
    }

    return;
}/*end of CFM_MGR_ProcessPortLeaveForwarding*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortAdminDisable_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port admin disable
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortAdminDisable_Callback(UI32_T ifindex)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " if=%ld", (long)ifindex);

    /*filter the non logical port*/
    if(FALSE == SWCTRL_LogicalPortExisting(ifindex))
    {
        return;
    }

    CFM_ENGINE_ProcessPortAdminDisable(ifindex);

}/*End of CFM_MGR_ProcessPortAdminDisable_Callback*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortAdminDisable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port admin disable
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : for sys_callback_mgr, cfm_group will call this
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortAdminDisable(UI32_T ifindex)
{
    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " if=%ld", (long)ifindex);

    /*filter the non logical port*/
    if(FALSE == SWCTRL_LogicalPortExisting(ifindex))
    {
        return;
    }

    CFM_ENGINE_ProcessPortAdminDisable(ifindex);

}/*End of CFM_MGR_ProcessPortAdminDisable*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessVlanCreate_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan create
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessVlanCreate_Callback(
                                    UI32_T vid_ifindex,
                                    UI32_T vlan_status)
{
    /* just return */
    return;
/*
    UI32_T vid=0;

    if(FALSE == VLAN_OM_ConvertFromIfindex(vid_ifindex, &vid))
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " vlan_if=%ld, vlan_status=%ld", vid_ifindex, vlan_status);
        return;
    }

    CFM_ENGINE_ProcessVlanCreate(vid);
*/
}/*End of CFM_MGR_ProcessVlanCreate_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessVlanDestory_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan destory
 * INPUT    : vid_ifindex   - the port ifindex
 *            vlan_status   -
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessVlanDestory_Callback(
    UI32_T  vid_ifindex,
    UI32_T  vlan_status)
{
    CFM_ENGINE_ProcessVlanDestory(vid_ifindex);
}/*End of CFM_MGR_ProcessVlanDestory_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessVlanMemberAdd_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan member add
 * INPUT    : vid_ifindex   - the port ifindex
 *            lport_ifindex -
 *            vlan_status   -
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessVlanMemberAdd_Callback(
    UI32_T  vid_ifindex,
    UI32_T  lport_ifindex,
    UI32_T  vlan_status)
{
    CFM_ENGINE_ProcessVlanMemberAdd(vid_ifindex,lport_ifindex);
}/*End of CFM_MGR_ProcessVlanMemberAdd_Callback*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessVlanMemberDelete_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan member delete
 * INPUT    : vid_ifindex   - the port ifindex
 *            lport_ifindex -
 *            vlan_status   -
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessVlanMemberDelete_Callback(
    UI32_T  vid_ifindex,
    UI32_T  lport_ifindex,
    UI32_T  vlan_status)
{
    CFM_ENGINE_ProcessVlanMemberDelete(vid_ifindex, lport_ifindex);
}/*End of CFM_MGR_ProcessVlanMemberDelete_Callback*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE: This function will initialize the port_p OM of the module ports
 *          when the option module is inserted.
 * INPUT  : in_starting_port_ifindex -- the ifindex of the first module port_p
 *                                      inserted
 *          in_number_of_port        -- the number of ports on the inserted
 *                                      module
 *          in_use_default           -- the flag indicating the default
 *                                      configuration is used without further
 *                                      provision applied; TRUE if a new module
 *                                      different from the original one is
 *                                      inserted
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void CFM_MGR_HandleHotInsertion(
    UI32_T  in_starting_port_ifindex,
    UI32_T  in_number_of_port,
    BOOL_T  in_use_default)
{
    /* 1. for global setting,   need to re-config the driver setting
     *
     * 2. for per port setting, cli will re-provision the setting
     *                          when inserting back
     * 3. is_use_default is used for cli provision,
     *      core layer should always reset the config to default
     */
    /* re-install global rule again
     */
    {
        CFM_TYPE_CfmStatus_T    old_gstatus;
        RULE_TYPE_CpuRuleInfo_T control;

        if (TRUE == CFM_OM_GetCFMGlobalStatus(&old_gstatus))
        {
            if (CFM_TYPE_CFM_STATUS_ENABLE == old_gstatus)
            {
                control.cfm_to_cpu = TRUE;
                RULE_CTRL_TrapPacket2Cpu (TRUE, RULE_TYPE_PacketType_CFM, &control);
            }
        }
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE: This function will clear the port_p OM of the module ports when
 *          the option module is removed.
 * INPUT  : in_starting_port_ifindex -- the ifindex of the first module port_p
 *                                      removed
 *          in_number_of_port        -- the number of ports on the removed
 *                                      module
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void CFM_MGR_HandleHotRemoval(
    UI32_T  in_starting_port_ifindex,
    UI32_T  in_number_of_port)
{
    UI32_T  ifindex;

    /* 1. for global setting,   do nothing
     *                          (bcz cli will not record it)
     * 2. for per port setting, clear the om
     */
    for (ifindex = in_starting_port_ifindex;
         ifindex<= in_starting_port_ifindex+in_number_of_port-1;
         ifindex++)
    {
        CFM_ENGINE_ClearOnePortConfig(ifindex);
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFM_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for cfm mgr.
 * INPUT:
 *    msg_p         --  the request ipc message buffer
 *    ipcmsgq_p     --  The handle of ipc message queue. The response message
 *                      will be sent through this handle.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    Need respond
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T CFM_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msg_p)
{
    CFM_MGR_IPCMsg_T *msg_data_p;
    msg_data_p=(CFM_MGR_IPCMsg_T *)msg_p->msg_buf;
    UI32_T cmd;
    cmd = msg_data_p->type.cmd;

    /* Every ipc request will fail when operating mode is not master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        msg_data_p->type.result_ui32 = CFM_TYPE_CONFIG_ERROR;
    }
    else
    {
        CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " cmd=%ld", (long)cmd);

        switch(cmd)
        {
            case CFM_MGR_IPCCMD_ADDNEWMEP:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_AddnewMEP(
                    msg_data_p->data.new_mep.lport,
                    msg_data_p->data.new_mep.mep_id,
                    msg_data_p->data.new_mep.md_name_ar,
                    msg_data_p->data.new_mep.md_name_len,
                    msg_data_p->data.new_mep.ma_name_ar,
                    msg_data_p->data.new_mep.ma_name_len,
                    msg_data_p->data.new_mep.direction);
            }
            break;

            case CFM_MGR_IPCCMD_ADDREMOTEMEP:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ma_name_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_AddRemoteMep(
                    msg_data_p->data.ma_name_u32a3.u32_a1,
                    msg_data_p->data.ma_name_u32a3.u32_a2,
                    msg_data_p->data.ma_name_u32a3.ma_name_ar,
                    msg_data_p->data.ma_name_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_CLEARERRORLIST:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32=CFM_MGR_ClearErrorList();
            }
            break;

            case CFM_MGR_IPCCMD_CLEARERRORSLISTBYMDNAMEORLEVEL:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.clear_error_by_md_level)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_ClearErrorsListByMdNameOrLevel(
                    msg_data_p->data.clear_error_by_md_level.md_name_ar,
                    msg_data_p->data.clear_error_by_md_level.name_len,
                    msg_data_p->data.clear_error_by_md_level.level);
            }
            break;

            case CFM_MGR_IPCCMD_CLEARLINKTRACECACHE:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32=CFM_MGR_ClearLinktraceCache();
            }
            break;

            case CFM_MGR_IPCCMD_CLEARLOOPBACKLIST:
            {
                CFM_MGR_ClearLoopBackList();
            }
            break;

            case CFM_MGR_IPCCMD_CLEARREMOTEMEPALL:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32=CFM_MGR_ClearRemoteMepAll();
            }
            break;

            case CFM_MGR_IPCCMD_CLEARREMOTEMEPBYDOMAIN:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.remote_mep_by_domain)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_ClearRemoteMepByDomain(
                    msg_data_p->data.remote_mep_by_domain.md_name_ar,
                    msg_data_p->data.remote_mep_by_domain.name_len);
            }
            break;

            case CFM_MGR_IPCCMD_CLEARREMOTEMEPBYLEVEL:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.md_level)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_ClearRemoteMepByLevel(
                    msg_data_p->data.md_level);
            }
            break;

            case CFM_MGR_IPCCMD_DELETEDOT1AGCFMMA:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DeleteDot1agCfmMa(
                    msg_data_p->data.u32a1_u32a2.u32_a1,
                    msg_data_p->data.u32a1_u32a2.u32_a2);
            }
            break;

            case CFM_MGR_IPCCMD_DELETEDOT1AGCFMMAMEPLISTENTRY:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DeleteDot1agCfmMaMepListEntry(
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_DELETEDOT1AGCFMMD:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ui32_v)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DeleteDot1agCfmMd(
                    msg_data_p->data.ui32_v);
            }
            break;

            case CFM_MGR_IPCCMD_DELETEDOT1AGCFMMEPENTRY:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DeleteDot1agCfmMepEntry(
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_DELETEMA:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DeleteMA(
                    msg_data_p->data.u32a1_u32a2.u32_a1,
                    msg_data_p->data.u32a1_u32a2.u32_a2);
            }
            break;

            case CFM_MGR_IPCCMD_DELETEMAVLAN:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u16a1)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DeleteMAVlan(
                    msg_data_p->data.u32a1_u32a2_u16a1.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u16a1.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u16a1.u16_a1);
            }
            break;

            case CFM_MGR_IPCCMD_DELETEMD:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ui32_v)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DeleteMD(
                    msg_data_p->data.ui32_v);
            }
            break;

            case CFM_MGR_IPCCMD_DELETEMEP:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DeleteMEP(
                    msg_data_p->data.md_ma_name_u32a2.md_name_ar,
                    msg_data_p->data.md_ma_name_u32a2.md_name_len,
                    msg_data_p->data.md_ma_name_u32a2.ma_name_ar,
                    msg_data_p->data.md_ma_name_u32a2.ma_name_len,
                    msg_data_p->data.md_ma_name_u32a2.u32_a1,
                    msg_data_p->data.md_ma_name_u32a2.u32_a2);
            }
            break;

            case CFM_MGR_IPCCMD_DELETEREMOTEMEP:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DeleteRemoteMep(
                    msg_data_p->data.ma_name_u32a3.u32_a1,
                    msg_data_p->data.ma_name_u32a3.u32_a2,
                    msg_data_p->data.ma_name_u32a3.ma_name_ar,
                    msg_data_p->data.ma_name_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_SETARCHIVEHOLDTIME:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetArchiveHoldTime(
                    msg_data_p->data.u32a1_u32a2.u32_a1,
                    msg_data_p->data.u32a1_u32a2.u32_a2);
            }
            break;

            case CFM_MGR_IPCCMD_SETCCMINTERVAL:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetCcmInterval(
                    msg_data_p->data.setting_ccm_interval.md_name_ar,
                    msg_data_p->data.setting_ccm_interval.md_name_len,
                    msg_data_p->data.setting_ccm_interval.ma_name_ar,
                    msg_data_p->data.setting_ccm_interval.ma_name_len,
                    msg_data_p->data.setting_ccm_interval.interval);
            }
            break;

            case CFM_MGR_IPCCMD_SETCCMSTATUS:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetCcmStatus(
                    msg_data_p->data.setting_ccm_status.md_name_ar,
                    msg_data_p->data.setting_ccm_status.md_name_len,
                    msg_data_p->data.setting_ccm_status.ma_name_ar,
                    msg_data_p->data.setting_ccm_status.ma_name_len,
                    msg_data_p->data.setting_ccm_status.status);
            }
            break;

            case CFM_MGR_IPCCMD_SETCFMGLOBALSTATUS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.cfm_status)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetCFMGlobalStatus(
                    msg_data_p->data.cfm_status);
            }
            break;

            case CFM_MGR_IPCCMD_SETCFMPORTSTATUS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.port_status)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetCFMPortStatus(
                    msg_data_p->data.port_status.lport,
                    msg_data_p->data.port_status.status);
            }
            break;

            case CFM_MGR_IPCCMD_SETCROSSCHECKSTARTDELAY:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ui32_v)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetCrossCheckStartDelay(
                    msg_data_p->data.ui32_v);
            }
            break;

            case CFM_MGR_IPCCMD_SETCROSSCHECKSTATUS:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetCrossCheckStatus(
                    msg_data_p->data.crosscheck_status.md_name_ar,
                    msg_data_p->data.crosscheck_status.md_name_len,
                    msg_data_p->data.crosscheck_status.ma_name_ar,
                    msg_data_p->data.crosscheck_status.ma_name_len,
                    msg_data_p->data.crosscheck_status.status);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMDEFAULTMDIDPERMISSION:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.vid_permission)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmDefaultMdIdPermission(
                    msg_data_p->data.vid_permission.md_primary_vid,
                    msg_data_p->data.vid_permission.send_id_permission);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMDEFAULTMDLEVEL:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.default_md_level)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmDefaultMdLevel(
                    msg_data_p->data.default_md_level.vid,
                    msg_data_p->data.default_md_level.level);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMA:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMa(
                    msg_data_p->data.u32a1_u32a2.u32_a1,
                    msg_data_p->data.u32a1_u32a2.u32_a2);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMACCMINTERVAL:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ma_ccm_interval)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMaCcmInterval(
                    msg_data_p->data.ma_ccm_interval.md_index,
                    msg_data_p->data.ma_ccm_interval.ma_index,
                    msg_data_p->data.ma_ccm_interval.interval);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMAFORMAT:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ma_format)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMaFormat(
                    msg_data_p->data.ma_format.md_index,
                    msg_data_p->data.ma_format.ma_index,
                    msg_data_p->data.ma_format.format);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMAMEPLISTENTRY:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMaMepListEntry(
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMAMHFCREATION:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ma_creation)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMaMhfCreation(
                    msg_data_p->data.ma_creation.md_index,
                    msg_data_p->data.ma_creation.ma_index,
                    msg_data_p->data.ma_creation.create_type);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMAMHFIDPERMISSION:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ma_permission)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMaMhfIdPermission(
                    msg_data_p->data.ma_permission.md_index,
                    msg_data_p->data.ma_permission.ma_index,
                    msg_data_p->data.ma_permission.send_id_permission);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMANAME:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ma_name_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMaName(
                    msg_data_p->data.ma_name_u32a3.u32_a1,
                    msg_data_p->data.ma_name_u32a3.u32_a2,
                    msg_data_p->data.ma_name_u32a3.ma_name_ar,
                    msg_data_p->data.ma_name_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMAPRIMARYVLANVID:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u16a1)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMaPrimaryVlanVid(
                    msg_data_p->data.u32a1_u32a2_u16a1.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u16a1.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u16a1.u16_a1);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMD:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ui32_v)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMd(
                    msg_data_p->data.ui32_v);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMDFORMAT:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.md_format)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMdFormat(
                    msg_data_p->data.md_format.md_index,
                    msg_data_p->data.md_format.md_format);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMDLEVEL:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.md_index_by_level)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMdLevel(
                    msg_data_p->data.md_index_by_level.md_index,
                    msg_data_p->data.md_index_by_level.level);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMDMHFCREATION:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.md_creation)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMdMhfCreation(
                    msg_data_p->data.md_creation.md_index,
                    msg_data_p->data.md_creation.create_type);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMDMHFIDPERMISSION:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.md_permission)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMdMhfIdPermission(
                    msg_data_p->data.md_permission.md_index,
                    msg_data_p->data.md_permission.send_id_permission);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMDNAME:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.md_index_by_name)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMdName(
                    msg_data_p->data.md_index_by_name.md_index,
                    msg_data_p->data.md_index_by_name.md_name_ar,
                    msg_data_p->data.md_index_by_name.name_len);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPACTIVE:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepActive(
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPCCIENABLE:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.mep_cci_status)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepCciEnable(
                    msg_data_p->data.mep_cci_status.md_index,
                    msg_data_p->data.mep_cci_status.ma_index,
                    msg_data_p->data.mep_cci_status.mep_id,
                    msg_data_p->data.mep_cci_status.status);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPCCMLTMPRIORITY:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepCcmLtmPriority(
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPDIRECTION:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.mep_direction_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepDirection(
                    msg_data_p->data.mep_direction_u32a3.u32_a1,
                    msg_data_p->data.mep_direction_u32a3.u32_a2,
                    msg_data_p->data.mep_direction_u32a3.u32_a3,
                    msg_data_p->data.mep_direction_u32a3.direction);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPENTRY:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepEntry(
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPFNGALARMTIME:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepFngAlarmTime(
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPFNGRESETTIME:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepFngResetTime(
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPIFINDEX:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepIfIndex(
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPLOWPRDEF:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.mep_low_pri)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepLowPrDef(
                    msg_data_p->data.mep_low_pri.md_index,
                    msg_data_p->data.mep_low_pri.ma_index,
                    msg_data_p->data.mep_low_pri.mep_id,
                    msg_data_p->data.mep_low_pri.priority);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPPRIMARYVID:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u16a1)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepPrimaryVid(
                    msg_data_p->data.u32a1_u32a2_u32a3_u16a1.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u16a1.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u16a1.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u16a1.u16_a1);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDATATLV:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.mep_error_ccm_fail)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLbmDataTlv(
                    msg_data_p->data.mep_error_ccm_fail.md_index,
                    msg_data_p->data.mep_error_ccm_fail.ma_index,
                    msg_data_p->data.mep_error_ccm_fail.mep_id,
                    msg_data_p->data.mep_error_ccm_fail.content_ar,
                    msg_data_p->data.mep_error_ccm_fail.content_len);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDESTISMEPID:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLbmDestIsMepId(
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDESTMACADDRESS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_mac)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLbmDestMacAddress(
                    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_mac.mac_ar);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDESTMEPID:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLbmDestMepId(
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMMESSAGES:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLbmMessages(
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMSTATUS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLbmStatus(
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMVLANDROPENABLE:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLbmVlanDropEnable(
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMVLANPRIORITY:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLbmVlanPriority(
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMEGRESSIDENTIFIER:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_idary)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLtmEgressIdentifier(
                    msg_data_p->data.u32a1_u32a2_u32a3_idary.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_idary.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_idary.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_idary.identifer_ar);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMFLAGS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLtmFlags(
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMSTATUS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLtmStatus(
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTARGETISMEPID:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_bool)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLtmTargetIsMepId(
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_bool.bool_a1);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTARGETMACADDRESS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_mac)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMacAddress(
                    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_mac.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_mac.mac_ar);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTARGETMEPID:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMepId(
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTTL:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3_u32a4)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmMepTransmitLtmTtl(
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a3,
                    msg_data_p->data.u32a1_u32a2_u32a3_u32a4.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_SETDOT1AGCFMNUMOFVIDS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2_u32a3)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetDot1agCfmNumOfVids(
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a1,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a2,
                    msg_data_p->data.u32a1_u32a2_u32a3.u32_a3);
            }
            break;

            case CFM_MGR_IPCCMD_SETFAULTNOTIFYALARMTIME:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetFaultNotifyAlarmTime(
                    msg_data_p->data.u32a1_u32a2.u32_a1,
                    msg_data_p->data.u32a1_u32a2.u32_a2);
            }
            break;

            case CFM_MGR_IPCCMD_SETFAULTNOTIFYLOWESTPRIORITY:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.fault_notify_pri)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetFaultNotifyLowestPriority(
                    msg_data_p->data.fault_notify_pri.md_index,
                    msg_data_p->data.fault_notify_pri.priority);
            }
            break;

            case CFM_MGR_IPCCMD_SETFAULTNOTIFYRESETTIME:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.u32a1_u32a2)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetFaultNotifyRestTime(
                    msg_data_p->data.u32a1_u32a2.u32_a1,
                    msg_data_p->data.u32a1_u32a2.u32_a2);
            }
            break;

            case CFM_MGR_IPCCMD_SETLINKTRACECACHEHOLDTIME:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ui32_v)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetLinkTraceCacheHoldTime(
                    msg_data_p->data.ui32_v);
            }
            break;

            case CFM_MGR_IPCCMD_SETLINKTRACECACHESIZE:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ui32_v)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetLinkTraceCacheSize(
                    msg_data_p->data.ui32_v);
            }
            break;

            case CFM_MGR_IPCCMD_SETLINKTRACECACHESTATUS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.linktrace_status)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetLinkTraceCacheStatus(
                    msg_data_p->data.linktrace_status);
            }
            break;

            case CFM_MGR_IPCCMD_SETMA:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ma_create_entry)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetMA(
                    msg_data_p->data.ma_create_entry.ma_index,
                    msg_data_p->data.ma_create_entry.ma_name_ar,
                    msg_data_p->data.ma_create_entry.name_len,
                    msg_data_p->data.ma_create_entry.md_index,
                    msg_data_p->data.ma_create_entry.primary_vid,
                    msg_data_p->data.ma_create_entry.vid_num,
                    msg_data_p->data.ma_create_entry.vid_list_ar,
                    msg_data_p->data.ma_create_entry.create_way);
            }
            break;

            case CFM_MGR_IPCCMD_SETMD:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.md_create_entry)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetMD(
                    msg_data_p->data.md_create_entry.md_index,
                    msg_data_p->data.md_create_entry.md_name_ar,
                    msg_data_p->data.md_create_entry.name_len,
                    msg_data_p->data.md_create_entry.level,
                    msg_data_p->data.md_create_entry.create_way);
            }
            break;

            case CFM_MGR_IPCCMD_SETSNMPCCSTATUS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.snmp_cc_status)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetSNMPCcStatus(
                    msg_data_p->data.snmp_cc_status.trap,
                    msg_data_p->data.snmp_cc_status.trap_enabled);
            }
            break;

            case CFM_MGR_IPCCMD_SETSNMPCROSSCHECKSTATUS:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.snmp_trap_cc_status)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetSNMPCrosscheckStatus(
                    msg_data_p->data.snmp_trap_cc_status.trap,
                    msg_data_p->data.snmp_trap_cc_status.trap_enabled);
            }
            break;

            case CFM_MGR_IPCCMD_TRANSMITLINKTRACE:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_TransmitLinktrace(
                    msg_data_p->data.transmit_settings.u32_a1,
                    msg_data_p->data.transmit_settings.u32_a2,
                    msg_data_p->data.transmit_settings.mac_ar,
                    msg_data_p->data.transmit_settings.md_name_ar,
                    msg_data_p->data.transmit_settings.md_name_len,
                    msg_data_p->data.transmit_settings.ma_name_ar,
                    msg_data_p->data.transmit_settings.ma_name_len,
                    msg_data_p->data.transmit_settings.u32_a3,
                    msg_data_p->data.transmit_settings.u32_a4);
            }
            break;

            case CFM_MGR_IPCCMD_TRANSMITLOOPBACK:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_TransmitLoopback(
                    msg_data_p->data.transmit_settings.u32_a1,
                    msg_data_p->data.transmit_settings.mac_ar,
                    msg_data_p->data.transmit_settings.md_name_ar,
                    msg_data_p->data.transmit_settings.md_name_len,
                    msg_data_p->data.transmit_settings.ma_name_ar,
                    msg_data_p->data.transmit_settings.ma_name_len,
                    msg_data_p->data.transmit_settings.u32_a2);
            }
            break;

            case CFM_MGR_IPCCMD_PROCESSPORTADMINDISABLED:
            {
                  CFM_MGR_ProcessPortAdminDisable(msg_data_p->data.ui32_v);
            }
            break;

            case CFM_MGR_IPCCMD_SETAISPERIOD:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetAisPeriod(
                    msg_data_p->data.md_ma_name_u32a1.md_name_ar,
                    msg_data_p->data.md_ma_name_u32a1.md_name_len,
                    msg_data_p->data.md_ma_name_u32a1.ma_name_ar,
                    msg_data_p->data.md_ma_name_u32a1.ma_name_len,
                    msg_data_p->data.md_ma_name_u32a1.u32_a1);
            }
            break;

            case CFM_MGR_IPCCMD_SETAISLEVEL:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetAisLevel(
                    msg_data_p->data.md_ma_name_u32a1.md_name_ar,
                    msg_data_p->data.md_ma_name_u32a1.md_name_len,
                    msg_data_p->data.md_ma_name_u32a1.ma_name_ar,
                    msg_data_p->data.md_ma_name_u32a1.ma_name_len,
                    msg_data_p->data.md_ma_name_u32a1.u32_a1);
            }
            break;

            case CFM_MGR_IPCCMD_SETAISSTATUS:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetAisStatus(
                    msg_data_p->data.md_ma_name_u32a1.md_name_ar,
                    msg_data_p->data.md_ma_name_u32a1.md_name_len,
                    msg_data_p->data.md_ma_name_u32a1.ma_name_ar,
                    msg_data_p->data.md_ma_name_u32a1.ma_name_len,
                    msg_data_p->data.md_ma_name_u32a1.u32_a1);
            }
            break;

            case  CFM_MGR_IPCCMD_SETAISSUPRESSSTATUS:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetAisSuppressStatus(
                    msg_data_p->data.md_ma_name_u32a1.md_name_ar,
                    msg_data_p->data.md_ma_name_u32a1.md_name_len,
                    msg_data_p->data.md_ma_name_u32a1.ma_name_ar,
                    msg_data_p->data.md_ma_name_u32a1.ma_name_len,
                    msg_data_p->data.md_ma_name_u32a1.u32_a1);
            }
            break;

            case CFM_MGR_IPCCMD_CLEARAISERROR:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_ClearAisError(
                                msg_data_p->data.md_ma_name_u32a1.u32_a1,
                                msg_data_p->data.md_ma_name_u32a1.md_name_ar,
                                msg_data_p->data.md_ma_name_u32a1.md_name_len,
                                msg_data_p->data.md_ma_name_u32a1.ma_name_ar,
                                msg_data_p->data.md_ma_name_u32a1.ma_name_len);
            }
            break;

            case CFM_MGR_IPCCMD_DODELAYMEASUREBYDMM:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DoDelayMeasureByDMM(
                    msg_data_p->data.transmit_dmm.src_mep_id,
                    msg_data_p->data.transmit_dmm.dst_mep_id,
                    msg_data_p->data.transmit_dmm.dst_mac_ar,
                    msg_data_p->data.transmit_dmm.md_name_ar,
                    msg_data_p->data.transmit_dmm.md_name_len,
                    msg_data_p->data.transmit_dmm.ma_name_ar,
                    msg_data_p->data.transmit_dmm.ma_name_len,
                    msg_data_p->data.transmit_dmm.counts,
                    msg_data_p->data.transmit_dmm.interval,
                    msg_data_p->data.transmit_dmm.timeout,
                    msg_data_p->data.transmit_dmm.pkt_size,
                    msg_data_p->data.transmit_dmm.pkt_pri);
            }
            break;

            case CFM_MGR_IPCCMD_ABORTDELAYMEASUREBYDMM:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_AbortDelayMeasureByDMM(
                    msg_data_p->data.abort_xmit.src_mep_id,
                    msg_data_p->data.abort_xmit.dst_mep_id,
                    msg_data_p->data.abort_xmit.dst_mac_ar,
                    msg_data_p->data.abort_xmit.md_name_ar,
                    msg_data_p->data.abort_xmit.md_name_len,
                    msg_data_p->data.abort_xmit.ma_name_ar,
                    msg_data_p->data.abort_xmit.ma_name_len);
            }
            break;

            case CFM_MGR_IPCCMD_ABORTTHRPTMEASUREBYLBM:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_AbortThrptMeasureByLBM(
                    msg_data_p->data.abort_xmit.src_mep_id,
                    msg_data_p->data.abort_xmit.dst_mep_id,
                    msg_data_p->data.abort_xmit.dst_mac_ar,
                    msg_data_p->data.abort_xmit.md_name_ar,
                    msg_data_p->data.abort_xmit.md_name_len,
                    msg_data_p->data.abort_xmit.ma_name_ar,
                    msg_data_p->data.abort_xmit.ma_name_len);
            }
            break;

            case CFM_MGR_IPCCMD_DOTHRPTMEASUREBYLBM:
            {
                msg_p->msg_size=CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_DoThrptMeasureByLBM(
                    msg_data_p->data.do_thrpt_by_lbm.src_mep_id,
                    msg_data_p->data.do_thrpt_by_lbm.dst_mep_id,
                    msg_data_p->data.do_thrpt_by_lbm.dst_mac_ar,
                    msg_data_p->data.do_thrpt_by_lbm.md_name_ar,
                    msg_data_p->data.do_thrpt_by_lbm.md_name_len,
                    msg_data_p->data.do_thrpt_by_lbm.ma_name_ar,
                    msg_data_p->data.do_thrpt_by_lbm.ma_name_len,
                    msg_data_p->data.do_thrpt_by_lbm.counts,
                    msg_data_p->data.do_thrpt_by_lbm.pkt_size,
                    msg_data_p->data.do_thrpt_by_lbm.pattern,
                    msg_data_p->data.do_thrpt_by_lbm.pkt_pri);
            }
            break;

            case CFM_MGR_IPCCMD_SETMANAMEFORMAT:
            {
                msg_p->msg_size=sizeof(msg_data_p->data.ma_format)
                        +CFM_MGR_MSGBUF_TYPE_SIZE;
                msg_data_p->type.result_ui32 = CFM_MGR_SetMANameFormat(
                    msg_data_p->data.ma_format.md_index,
                    msg_data_p->data.ma_format.ma_index,
                    msg_data_p->data.ma_format.format);
            }
            break;

            default:
                msg_data_p->type.result_ui32=CFM_TYPE_CONFIG_ERROR;
                msg_data_p->type.result_bool=FALSE;

                CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, " no this cmd=%ld", (long)cmd);

                return FALSE;
        }
    }

    /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
     */
    if(cmd<CFM_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}/*End of CFM_MGR_HandleIPCReqMsg*/

/* for delay measurement
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_AbortDelayMeasureByDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To abort the delay measure in progress.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_AbortDelayMeasureByDMM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T       md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T      md_name_len,
    UI8_T       ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T      ma_name_len)
{
    CFM_TYPE_Config_T   ret =CFM_TYPE_CONFIG_ERROR;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "enter");

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (TRUE == CFM_ENGINE_AbortBgXmitDMM(
                        src_mep_id, dst_mep_id, dst_mac_ar,
                        md_name_ar, md_name_len, ma_name_ar, ma_name_len))
        {

            ret = CFM_TYPE_CONFIG_SUCCESS;
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DoDelayMeasureByDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To do delay measure by sending DMM.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 *            counts      - the transmit counts
 *            interval    - the transmit interval
 *            timeout     - the timeout for waiting dmr
 *            pkt_size    - the transmit packet size
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DoDelayMeasureByDMM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T       md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T      md_name_len,
    UI8_T       ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T      ma_name_len,
    UI32_T      counts,
    UI32_T      interval,
    UI32_T      timeout,
    UI32_T      pkt_size,
    UI16_T      pkt_pri)
{
    CFM_TYPE_Config_T   ret =CFM_TYPE_CONFIG_ERROR;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "count/interval/timeout/pktsize-%ld/%ld/%ld/%ld",
            (long)counts, (long)interval, (long)timeout, (long)pkt_size);

    if ((SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
        && (CFM_TYPE_MIN_DMM_COUNT   <= counts)   && (counts <= CFM_TYPE_MAX_DMM_COUNT)
        && (CFM_TYPE_MIN_DMM_INTERVAL<= interval) && (interval <= CFM_TYPE_MAX_DMM_INTERVAL)
        && (CFM_TYPE_MIN_DMM_TIMEOUT <= timeout)  && (timeout <= CFM_TYPE_MAX_DMM_TIMEOUT)
        && (CFM_TYPE_MIN_DMM_PKTSIZE <= pkt_size) && (pkt_size <= CFM_TYPE_MAX_DMM_PKTSIZE))
    {
        if (TRUE == CFM_ENGINE_AddBgXmitDMM(
                        src_mep_id, dst_mep_id, dst_mac_ar,
                        md_name_ar, md_name_len, ma_name_ar, ma_name_len,
                        counts, interval, timeout, pkt_size, pkt_pri))
        {

            ret = CFM_TYPE_CONFIG_SUCCESS;
        }
    }

    return ret;
}
/* end for delay measurement
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_AbortThrptMeasureByLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To abort the throughput measure in progressing.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_AbortThrptMeasureByLBM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T       md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T      md_name_len,
    UI8_T       ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T      ma_name_len)
{
    CFM_TYPE_Config_T   ret =CFM_TYPE_CONFIG_ERROR;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "enter");

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (TRUE == CFM_ENGINE_AbortBgXmitLBM(
                        src_mep_id, dst_mep_id, dst_mac_ar,
                        md_name_ar, md_name_len, ma_name_ar, ma_name_len))
        {
            ret = CFM_TYPE_CONFIG_SUCCESS;
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DoThrptMeasureByLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To do throughput measure by sending LBM.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 *            counts      - the transmit counts
 *            pkt_size    - the transmit packet size
 *            pattern     - the pattern included in data TLV
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DoThrptMeasureByLBM(
    UI32_T  src_mep_id,
    UI32_T  dst_mep_id,
    UI8_T   dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T  md_name_len,
    UI8_T   ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T  ma_name_len,
    UI32_T  counts,
    UI32_T  pkt_size,
    UI16_T  pattern,
    UI16_T  pkt_pri)
{
    CFM_TYPE_Config_T   ret=CFM_TYPE_CONFIG_ERROR;

    CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_UI, "count/pktsize-%lu/%lu", (long)counts, (long)pkt_size);

    if ((SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) ||
        ((counts<CFM_TYPE_MIN_LBM_COUNT) || (counts>CFM_TYPE_MAX_LBM_COUNT)) ||
        ((pkt_size<CFM_TYPE_MIN_LBM_PKTSIZE) || (pkt_size>CFM_TYPE_MAX_LBM_PKTSIZE)))
    {
        return CFM_TYPE_CONFIG_ERROR;
    }

    if (TRUE == CFM_ENGINE_AddBgXmitLBM(
                    src_mep_id, dst_mep_id, dst_mac_ar,
                    md_name_ar, md_name_len, ma_name_ar, ma_name_len,
                    counts, CFM_TYPE_DEF_LBM_TIMEOUT, pkt_size, pattern, pkt_pri))
    {
        ret = CFM_TYPE_CONFIG_SUCCESS;
    }

    return ret;
}

#endif /*#if (SYS_CPNT_CFM == TRUE)*/

