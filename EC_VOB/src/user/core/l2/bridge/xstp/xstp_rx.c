/*-----------------------------------------------------------------------------
 * Module Name: xstp_rx.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the functions to process the received BPDU
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    05/30/2001 - Allen Cheng, Created
 *    06/12/2002 - Kelly Chen, Added
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "sys_time.h"
#include "sys_dflt.h"
#include "l_stdlib.h"
#include "backdoor_mgr.h"
#include "xstp_type.h"
#include "xstp_om_private.h"
#include "xstp_rx.h"
#include "leaf_es3626a.h"
#include "trap_event.h"
#include "snmp_pmgr.h"

#define XSTP_RX_SetBpduReceived(__pom_ptr, __bpdu)                                      \
{                                                                                       \
    (__pom_ptr)->common->bpdu       = (XSTP_TYPE_Bpdu_T*)(__bpdu);                      \
    (__pom_ptr)->common->rcvd_bpdu  = TRUE;                                             \
    (__pom_ptr)->common->oper_edge  = FALSE;                                            \
}
#ifdef XSTP_TYPE_PROTOCOL_RSTP
#define XSTP_RX_RecordMessage(__pom_ptr, __bpdu)                                        \
{                                                                                       \
    XSTP_TYPE_BridgeId_T    root_identifier;                                                             \
    XSTP_TYPE_BridgeId_T    bridge_identifier;                                                           \
    XSTP_TYPE_PortId_T      port_identifier;                                                             \
                                                                                                         \
    XSTP_OM_CPY_NTOH_BRIDGE_ID(root_identifier, (__bpdu)->root_identifier);                              \
    XSTP_OM_CPY_NTOH_BRIDGE_ID(bridge_identifier, (__bpdu)->bridge_identifier);                          \
    port_identifier.port_id = L_STDLIB_Ntoh16((__bpdu)->port_identifier.port_id);                        \
    XSTP_OM_PRIORITY_VECTOR_FORMAT( (__pom_ptr)->msg_priority,                          \
                                    root_identifier,                                                     \
                                    L_STDLIB_Ntoh32((__bpdu)->root_path_cost),                           \
                                    bridge_identifier,                                                   \
                                    port_identifier,                                                     \
                                    (__pom_ptr)->port_id);                              \
    XSTP_OM_TIMERS_FORMAT(          (__pom_ptr)->msg_times,                             \
                                    L_STDLIB_Ntoh16((__bpdu)->message_age)   / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    L_STDLIB_Ntoh16((__bpdu)->max_age)       / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    L_STDLIB_Ntoh16((__bpdu)->hello_time)    / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    L_STDLIB_Ntoh16((__bpdu)->forward_delay) / XSTP_TYPE_BPDU_TIME_UNIT);\
}
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

#ifdef XSTP_TYPE_PROTOCOL_MSTP
#define XSTP_RX_RecordMessage(__pom_ptr, __bpdu)                                        \
{                                                                                       \
    XSTP_TYPE_BridgeId_T    root_identifier;                                                             \
    XSTP_TYPE_BridgeId_T    bridge_identifier;                                                           \
    XSTP_TYPE_PortId_T      port_identifier;                                                             \
                                                                                                         \
    XSTP_OM_CPY_NTOH_BRIDGE_ID(root_identifier, (__bpdu)->root_identifier);                              \
    XSTP_OM_CPY_NTOH_BRIDGE_ID(bridge_identifier, (__bpdu)->bridge_identifier);                          \
    port_identifier.port_id = L_STDLIB_Ntoh16((__bpdu)->port_identifier.port_id);                        \
    XSTP_OM_PRIORITY_VECTOR_FORMAT( (__pom_ptr)->msg_priority,                          \
                                    root_identifier,                                                     \
                                    L_STDLIB_Ntoh32((__bpdu)->root_path_cost),                           \
                                    bridge_identifier,                                                   \
                                    0,                                                  \
                                    bridge_identifier,                                                   \
                                    port_identifier,                                                     \
                                    (__pom_ptr)->port_id);                              \
    XSTP_OM_TIMERS_FORMAT(          (__pom_ptr)->msg_times,                             \
                                    L_STDLIB_Ntoh16((__bpdu)->message_age)   / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    L_STDLIB_Ntoh16((__bpdu)->max_age)       / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    L_STDLIB_Ntoh16((__bpdu)->hello_time)    / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    L_STDLIB_Ntoh16((__bpdu)->forward_delay) / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    0);                                                 \
    (__pom_ptr)->msti_config_msg    = 0;                                                \
    (__pom_ptr)->rcvd_msg           = TRUE;                                             \
}

#define XSTP_RX_RecordCistMessage(__pom_ptr, __bpdu)                                    \
{                                                                                       \
    XSTP_TYPE_BridgeId_T    cist_root_identifier;                                                        \
    XSTP_TYPE_BridgeId_T    cist_regional_root_identifier;                                               \
    XSTP_TYPE_BridgeId_T    cist_bridge_identifier;                                                      \
    XSTP_TYPE_PortId_T      cist_port_identifier;                                                        \
                                                                                                         \
    XSTP_OM_CPY_NTOH_BRIDGE_ID(cist_root_identifier, (__bpdu)->cist_root_identifier);                    \
    XSTP_OM_CPY_NTOH_BRIDGE_ID(cist_regional_root_identifier, (__bpdu)->cist_regional_root_identifier);  \
    XSTP_OM_CPY_NTOH_BRIDGE_ID(cist_bridge_identifier, (__bpdu)->cist_bridge_identifier);                \
    cist_port_identifier.port_id = L_STDLIB_Ntoh16((__bpdu)->cist_port_identifier.port_id);              \
    XSTP_OM_PRIORITY_VECTOR_FORMAT( (__pom_ptr)->msg_priority,                          \
                                    cist_root_identifier,                                                \
                                    L_STDLIB_Ntoh32((__bpdu)->cist_external_path_cost),                  \
                                    cist_regional_root_identifier,                                       \
                                    L_STDLIB_Ntoh32((__bpdu)->cist_internal_root_path_cost),             \
                                    cist_bridge_identifier,                                              \
                                    cist_port_identifier,                                                \
                                    (__pom_ptr)->port_id);                              \
    XSTP_OM_TIMERS_FORMAT(          (__pom_ptr)->msg_times,                             \
                                    L_STDLIB_Ntoh16((__bpdu)->message_age)   / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    L_STDLIB_Ntoh16((__bpdu)->max_age)       / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    L_STDLIB_Ntoh16((__bpdu)->hello_time)    / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    L_STDLIB_Ntoh16((__bpdu)->forward_delay) / XSTP_TYPE_BPDU_TIME_UNIT, \
                                    (__bpdu)->cist_remaining_hops);                     \
    (__pom_ptr)->msti_config_msg    = 0;                                                \
    (__pom_ptr)->rcvd_msg           = TRUE;                                             \
}


static  void    XSTP_RX_ProcessMstiMessage(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, XSTP_TYPE_MstBpdu_T *mst_bpdu);
static  void XSTP_RX_RecordMstiMessage(XSTP_OM_PortVar_T *pom_ptr, XSTP_TYPE_MstBpdu_T *bpdu, XSTP_TYPE_MstiConfigMsg_T *msti_cfg_msg);
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_ProcessBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Process the received BPDU
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport             -- BPDU incoming port
 *            mref_handle_pp    -- BPDU mref
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : To process a received BPDU, the state machine (PIM in RSTP,
 *            or PRX in MSTP) is triggered by setting the variable rcvdBpdu.
 *            In 802.1w, this variable is not set until the BPDU is recorded.
 *            In 802.1s, this variable is set in order to trigger the PRX in
 *            which setRcvdMsgs() invokes the function to record the BPDU.
 *-------------------------------------------------------------------------
 */
void    XSTP_RX_ProcessBpdu(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, L_MM_Mref_Handle_T **mref_handle_pp)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_Bpdu_T        *bpdu_new, *bpdu_old;
    UI32_T                   pdu_len;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    bpdu_new = (XSTP_TYPE_Bpdu_T*)L_MM_Mref_GetPdu(*mref_handle_pp, &pdu_len);

    if (bpdu_new == NULL)
        return;

    if (pom_ptr->common->mref_handle_p != NULL)
    {
        bpdu_old = (XSTP_TYPE_Bpdu_T*)L_MM_Mref_GetPdu(pom_ptr->common->mref_handle_p, &pdu_len);
        if (bpdu_old == NULL)
        {
            pom_ptr->common->mref_handle_p = *mref_handle_pp;
        }
        else
        {
            /*compare to keep better bpdu_new*/
            if(memcmp(&bpdu_old->mst_bpdu.cist_root_identifier, &bpdu_new->mst_bpdu.cist_root_identifier, 22)>=0) /*compare cist root id, cist ex path cost, cist region root id, port*/
            {/*old is worse or the same*/
                L_MM_Mref_Release(&(pom_ptr->common->mref_handle_p));
                pom_ptr->common->mref_handle_p = *mref_handle_pp;
            }
            else
            {/*old is better*/
                L_MM_Mref_Release(mref_handle_pp);
                return;
            }
        }
    }
    else
    {
       pom_ptr->common->mref_handle_p = *mref_handle_pp;
    }

#ifdef XSTP_TYPE_PROTOCOL_RSTP
    XSTP_RX_RecordBpdu(om_ptr, lport, bpdu_new);
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

    XSTP_RX_SetBpduReceived(pom_ptr, bpdu_new);

    return;
} /* End of XSTP_RX_ProcessBpdu */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_RecordBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Copy the received BPDU pointer to the port om
 * INPUT    : lport     -- lport
 *            bpdu      -- BPDU
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. For CIST only
 *            2. Compliant to the procedure for receiving BPDUs in IEEE Std 802.1w
 *            3. Ref to the description in 13.26.15, IEEE Std 802.1s(D14.1)-2002
 *            4. This procedure should be maintained carefully with both the
 *               RSTP and MSTP considered.
 *            5. The MREF can't be freed here, becuase state machine still access them later.
 *-------------------------------------------------------------------------
 */
void    XSTP_RX_RecordBpdu(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, XSTP_TYPE_Bpdu_T *bpdu)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_BpduHeader_T  *bpdu_header;

    bpdu_header = (XSTP_TYPE_BpduHeader_T*)bpdu;

    switch (bpdu_header->bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
#ifdef XSTP_TYPE_PROTOCOL_MSTP
            if (    (XSTP_TYPE_PROTOCOL_VERSION_ID == XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID)
                &&  (bpdu_header->protocol_version_identifier == XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID)
               )
            {
                XSTP_TYPE_MstBpdu_T *bpdu_ptr;
                bpdu_ptr    = (XSTP_TYPE_MstBpdu_T*)bpdu;
                pom_ptr     = &(om_ptr->port_info[lport-1]);

                /* Process MSTI configuration messages */
                /* Process this first because XSTP_RX_RecordCistMessage sets rcvd_bpdu
                 * to trigger the port receive state machine
                 */
                if (pom_ptr->common->rcvd_internal)
                {
                    XSTP_RX_ProcessMstiMessage(om_ptr, lport, (XSTP_TYPE_MstBpdu_T*)bpdu_ptr);
                }

                XSTP_RX_RecordCistMessage(pom_ptr, bpdu_ptr);

                if((bpdu_ptr->cist_flags & XSTP_TYPE_BPDU_FLAGS_TC) != 0)
                {
                    XSTP_OM_SetTcCausePortAndBrdgMac(lport, bpdu_ptr->cist_bridge_identifier.addr);
                    XSTP_OM_SetTrapRxFlagTc(TRUE);
                }

                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_RXRSTP))
                {
                    UI8_T           i;
                    UI32_T          current_time;

                    SYS_TIME_GetRealTimeBySec(&current_time);

                    BACKDOOR_MGR_Printf("\r\n");
                    BACKDOOR_MGR_Printf("\r\nRXMSTP::");
                    BACKDOOR_MGR_Printf("[RxFmLport=%ld]", lport);
                    BACKDOOR_MGR_Printf("[Flags = 0x%02x]", bpdu_ptr->cist_flags);
                    BACKDOOR_MGR_Printf("[CistRootId=%4x", bpdu_ptr->cist_root_identifier.bridge_id_priority.bridge_priority);
                    for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", bpdu_ptr->cist_root_identifier.addr[i]);
                    BACKDOOR_MGR_Printf("][CistERC = 0x%04lx]", bpdu_ptr->cist_external_path_cost);
                    BACKDOOR_MGR_Printf("[RRoot=%4x", bpdu_ptr->cist_regional_root_identifier.bridge_id_priority.bridge_priority);
                    for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", bpdu_ptr->cist_regional_root_identifier.addr[i]);
                    BACKDOOR_MGR_Printf("][CistIRC = 0x%04lx]", bpdu_ptr->cist_internal_root_path_cost);
                    BACKDOOR_MGR_Printf("[DBridgeId=%4x", bpdu_ptr->cist_bridge_identifier.bridge_id_priority.bridge_priority);
                    for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", bpdu_ptr->cist_bridge_identifier.addr[i]);
                    BACKDOOR_MGR_Printf("][PortId = 0x%02x]", bpdu_ptr->cist_port_identifier.port_id);
                    BACKDOOR_MGR_Printf("[RxTime = %ld]", current_time);
                }
            }
            else
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
            {
                XSTP_TYPE_RstBpdu_T *bpdu_ptr;
                bpdu_ptr        = (XSTP_TYPE_RstBpdu_T*)bpdu;

                pom_ptr         = &(om_ptr->port_info[lport-1]);
                XSTP_RX_RecordMessage(pom_ptr, bpdu_ptr);
                if((bpdu_ptr->flags & XSTP_TYPE_BPDU_FLAGS_TC) != 0)
                {
                    XSTP_OM_SetTcCausePortAndBrdgMac(lport, bpdu_ptr->bridge_identifier.addr);
                    XSTP_OM_SetTrapRxFlagTc(TRUE);				
                }

                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_RXRSTP))
                {
                    UI8_T           i;
                    UI32_T          current_time;

                    SYS_TIME_GetRealTimeBySec(&current_time);

                    BACKDOOR_MGR_Printf("\r\n");
                    BACKDOOR_MGR_Printf("\r\nRXRSTP::");
                    BACKDOOR_MGR_Printf("[RxFmLport=%ld]", lport);
                    BACKDOOR_MGR_Printf("[Flags = 0x%02x]", bpdu_ptr->flags);
                    BACKDOOR_MGR_Printf("[RootId=%4x", bpdu_ptr->root_identifier.bridge_id_priority.bridge_priority);
                    for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", bpdu_ptr->root_identifier.addr[i]);
                    BACKDOOR_MGR_Printf("][RPC = 0x%04lx]", bpdu_ptr->root_path_cost);
                    BACKDOOR_MGR_Printf("[BridgeId=%4x", bpdu_ptr->bridge_identifier.bridge_id_priority.bridge_priority);
                    for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", bpdu_ptr->bridge_identifier.addr[i]);
                    BACKDOOR_MGR_Printf("][PortId = 0x%02x]", bpdu_ptr->port_identifier.port_id);
                    BACKDOOR_MGR_Printf("[RxTime = %ld]", current_time);
                }
            }
            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
            if (bpdu_header->protocol_version_identifier == XSTP_TYPE_STP_PROTOCOL_VERSION_ID)
            {
                XSTP_TYPE_ConfigBpdu_T  *bpdu_ptr;
                bpdu_ptr    = (XSTP_TYPE_ConfigBpdu_T*)bpdu;

                pom_ptr         = &(om_ptr->port_info[lport-1]);
                XSTP_RX_RecordMessage(pom_ptr, bpdu_ptr);

                if((bpdu_ptr->flags & XSTP_TYPE_BPDU_FLAGS_TC) != 0)
                {
                    XSTP_OM_SetTcCausePortAndBrdgMac(lport, bpdu_ptr->bridge_identifier.addr);
                    XSTP_OM_SetTrapRxFlagTc(TRUE);					
                }

                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_RXCFG))
                {
                    UI8_T           i;
                    UI32_T          current_time;

                    SYS_TIME_GetRealTimeBySec(&current_time);

                    BACKDOOR_MGR_Printf("\r\n");
                    BACKDOOR_MGR_Printf("\r\nRXConfig::");
                    BACKDOOR_MGR_Printf("[RxFmLport=%ld]", lport);
                    BACKDOOR_MGR_Printf("[Flags = 0x%02x]", bpdu_ptr->flags);
                    BACKDOOR_MGR_Printf("[RootId=%4x", bpdu_ptr->root_identifier.bridge_id_priority.bridge_priority);
                    for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", bpdu_ptr->root_identifier.addr[i]);
                    BACKDOOR_MGR_Printf("][RPC = 0x%04lx]", bpdu_ptr->root_path_cost);
                    BACKDOOR_MGR_Printf("[BridgeId=%4x", bpdu_ptr->bridge_identifier.bridge_id_priority.bridge_priority);
                    for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", bpdu_ptr->bridge_identifier.addr[i]);
                    BACKDOOR_MGR_Printf("][PortId = 0x%02x]", bpdu_ptr->port_identifier.port_id);
                    BACKDOOR_MGR_Printf("[RxTime = %ld]", current_time);
                }
            }
            break;

        case XSTP_TYPE_BPDU_TYPE_TCN:
            if (bpdu_header->protocol_version_identifier == XSTP_TYPE_STP_PROTOCOL_VERSION_ID)
            {
                pom_ptr         = &(om_ptr->port_info[lport-1]);
#ifdef XSTP_TYPE_PROTOCOL_MSTP
                pom_ptr->rcvd_msg           = TRUE;
#endif
                XSTP_OM_SetTcCausePortAndBrdgMac(lport, NULL);
                XSTP_OM_SetTrapRxFlagTc(TRUE);
                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_RXTCN))
                {
                    UI32_T          current_time;

                    SYS_TIME_GetRealTimeBySec(&current_time);

                    BACKDOOR_MGR_Printf("\r\n");
                    BACKDOOR_MGR_Printf("\r\nRXTCN::[RxFmLport=%ld]", lport);
                    BACKDOOR_MGR_Printf("[Time = %ld]", current_time);
                }
            }
            break;
    }

    pom_ptr         = &(om_ptr->port_info[lport-1]);

    return;
} /* End of XSTP_RX_RecordBpdu */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_GetMstidFromBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next mstid
 * INPUT    : index     -- index of the MSTI configuration message
 *            mst_bpdu  -- mst bpdu
 * OUTPUT   : None
 * RETURN   : XSTP_TYPE_CISTID (0) if the indexed MSTI message does not exist,
 *            or the MSTID
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_RX_GetMstidFromBpdu(UI32_T index, XSTP_TYPE_MstBpdu_T *mst_bpdu)
{
#ifdef XSTP_TYPE_PROTOCOL_MSTP
    XSTP_TYPE_MstiConfigMsg_T   *msti_config_msg;
    UI32_T                      mstid;
    UI16_T                      version_3_length, msti_length;
    UI16_T                      msti_num;

    /* 13.26.22 : No more than 64 MSTIs may be supported. */
    /* There are (64+1) instances including CIST supported. */
    if (index > XSTP_TYPE_MAX_MSTID)
    {
        return XSTP_TYPE_CISTID;
    }

    version_3_length    = L_STDLIB_Ntoh16(mst_bpdu->version_3_length);
    if (version_3_length < 64)
    {
        return XSTP_TYPE_CISTID;
    }
    msti_length         = version_3_length - 64;
    if ( msti_length % sizeof(XSTP_TYPE_MstiConfigMsg_T) )
    {
        return XSTP_TYPE_CISTID;
    }
    msti_num    = msti_length / sizeof(XSTP_TYPE_MstiConfigMsg_T);
    if (    (msti_num >= XSTP_TYPE_MAX_INSTANCE_NUM)
         || (index >= msti_num)
       )
    {
        return XSTP_TYPE_CISTID;
    }

    msti_config_msg = &(mst_bpdu->msti_configureation_message[index]);
    mstid = (UI32_T)(L_STDLIB_Ntoh16(msti_config_msg->msti_regional_root_identifier.bridge_id_priority.bridge_priority) & 0x0FFF);
    return mstid;
#else
    return XSTP_TYPE_CISTID;
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
} /* End of XSTP_RX_GetMstidFromBpdu */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_ValidateReceivedBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Validate the received BPDUs
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 *            bpdu
 *            length    -- packet length including 802.2 LLC (3 bytes)
 * OUTPUT   : bpdu
 * RETURN   : TRUE if the bpdu is valid, else FALSE
 * NOTE     : 1.Ref to the description in 14.4, IEEE Std 802.1s(D14.1)-2002.
 * 2.Discard superior BPDU when RootGuard is in operation.
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_RX_ValidateReceivedBpdu(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, XSTP_TYPE_Bpdu_T *bpdu, UI32_T length)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    UI16_T                  protocol_identifier;
    UI8_T                   protocol_version_identifier;
    UI8_T                   bpdu_type;
    UI32_T                  bpdu_length;
    BOOL_T                  valid;
    I32_T                   cmp_bridgeid, cmp_portid;
    BOOL_T                  is_dsgn;
    XSTP_TYPE_BridgeId_T    bridge_identifier;
    XSTP_TYPE_PortId_T      port_identifier;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    /* Octets 1 and 2 */
    protocol_identifier         = L_STDLIB_Ntoh16(bpdu->bpdu_header.protocol_identifier);
    /* Octet 3 */
    protocol_version_identifier = bpdu->bpdu_header.protocol_version_identifier;
    /* Octet 4 */
    bpdu_type                   = bpdu->bpdu_header.bpdu_type;
    /* Total length of the received BPDU */
    bpdu_length                 = length - 3;

    if (protocol_identifier == XSTP_TYPE_BPDU_PROTOCOL_IDENTIFIER)
    {
        switch (bpdu_type)
        {
            /* 14.4(a) */
            case XSTP_TYPE_BPDU_TYPE_CONFIG:
                XSTP_OM_CPY_NTOH_BRIDGE_ID(bridge_identifier, bpdu->config_bpdu.bridge_identifier);
                XSTP_OM_CMP_BRIDGE_ID(cmp_bridgeid, bridge_identifier, om_ptr->bridge_info.bridge_identifier);
                if (protocol_version_identifier == XSTP_TYPE_STP_PROTOCOL_VERSION_ID)
                {
                    port_identifier.port_id = L_STDLIB_Ntoh16(bpdu->config_bpdu.port_identifier.port_id);
                    XSTP_OM_CMP_PORT_ID(cmp_portid, port_identifier, pom_ptr->port_id);
                }
                else
                {
                    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                        BACKDOOR_MGR_Printf("\r\nXSTP_RX_ValidateReceivedBpdu : Discard an invalid CFG BPDU containing the invalid protocol_version_identifier %d (shall be 0) received at lport %lu", protocol_version_identifier, lport);
                    valid = FALSE;
                    break;
                }
                if (    (cmp_bridgeid == 0)
                    &&  (cmp_portid == 0)
                   )
                {
                    /* 9.3.4 (NOTE1) */
                    /* Discard this BPDU */
                    valid  = FALSE;
                    pom_ptr->common->oper_edge          = FALSE;
                    pom_ptr->common->edge_delay_while   = SYS_DFLT_STP_MIGRATE_TIME;
                    pom_ptr->agreed                     = FALSE;
                    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                        BACKDOOR_MGR_Printf("\r\nXSTP_RX_ValidateReceivedBpdu : Discard an echo BPDU received at lport %lu", lport);
                }
                else
                {
                    if (bpdu_length >= 35)
                    {
                        valid  = TRUE;
                    }
                    else
                    {
                        valid  = FALSE;
                    }
                }
                break;

            /* 14.4(b) */
            case XSTP_TYPE_BPDU_TYPE_TCN:
                XSTP_OM_CPY_NTOH_BRIDGE_ID(bridge_identifier, bpdu->config_bpdu.bridge_identifier);
                XSTP_OM_CMP_BRIDGE_ID(cmp_bridgeid, bridge_identifier, om_ptr->bridge_info.bridge_identifier);
                if (protocol_version_identifier != XSTP_TYPE_STP_PROTOCOL_VERSION_ID)
                {
                    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                        BACKDOOR_MGR_Printf("\r\nXSTP_RX_ValidateReceivedBpdu : Discard an invalid TCN BPDU containing the invalid protocol_version_identifier %d (shall be 0) received at lport %lu", protocol_version_identifier, lport);
                    valid  = FALSE;
                }
                else
                if (bpdu_length >= 4)
                {
                    valid  = TRUE;
                }
                else
                {
                    valid  = FALSE;
                }
                break;

            /* 14.4(c) */
            case XSTP_TYPE_BPDU_TYPE_XST:
                if (protocol_version_identifier == XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID)
                {
                    XSTP_OM_CPY_NTOH_BRIDGE_ID(bridge_identifier, bpdu->rst_bpdu.bridge_identifier);
                    XSTP_OM_CMP_BRIDGE_ID(cmp_bridgeid, bridge_identifier, om_ptr->bridge_info.bridge_identifier);
                    port_identifier.port_id = L_STDLIB_Ntoh16(bpdu->rst_bpdu.port_identifier.port_id);
                    XSTP_OM_CMP_PORT_ID(cmp_portid, port_identifier, pom_ptr->port_id);
                    is_dsgn     = (   (bpdu->rst_bpdu.flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                                   == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED
                                  );
                }
                else if (protocol_version_identifier >= XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID)
                {
                    XSTP_OM_CPY_NTOH_BRIDGE_ID(bridge_identifier, bpdu->mst_bpdu.cist_bridge_identifier);
                    XSTP_OM_CMP_BRIDGE_ID(cmp_bridgeid, bridge_identifier, om_ptr->bridge_info.bridge_identifier);
                    port_identifier.port_id = L_STDLIB_Ntoh16(bpdu->mst_bpdu.cist_port_identifier.port_id);
                    XSTP_OM_CMP_PORT_ID(cmp_portid, port_identifier, pom_ptr->port_id);
                    is_dsgn     = (   (bpdu->mst_bpdu.cist_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                                   == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED
                                  );
                }
                else
                {
                    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                        BACKDOOR_MGR_Printf("\r\nXSTP_RX_ValidateReceivedBpdu : Discard an invalid RSTP/MSTP BPDU containing the invalid protocol_version_identifier %d (shall be greater than or equal to 2) received at lport %lu", protocol_version_identifier, lport);
                    valid = FALSE;
                    break;
                }

                if (    (cmp_bridgeid == 0)
                    &&  (cmp_portid == 0)
                    &&  is_dsgn
                   )
                {
                    /* 9.3.4 (NOTE1) */
                    /* Discard this BPDU */
                    valid  = FALSE;
                    pom_ptr->common->oper_edge          = FALSE;
                    pom_ptr->common->edge_delay_while   = SYS_DFLT_STP_MIGRATE_TIME;
                    pom_ptr->agreed                     = FALSE;
                    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                        BACKDOOR_MGR_Printf("\r\nXSTP_RX_ValidateReceivedBpdu : Discard an echo BPDU received at lport %lu", lport);
                }
                else
                {
                    if (    (bpdu_length >= 36)
                         && (protocol_version_identifier >= 2)
                       )
                    {
                        /* 14.4(d)(e) */
                        if (    (protocol_version_identifier >= 3)
                             && (bpdu_length >= 102)                    /* (1) */
                             && (bpdu->rst_bpdu.version_1_length == 0)  /* (2) */
                           )
                        {
                            /* (3) */
                            UI16_T  version_3_length;

                            version_3_length    = L_STDLIB_Ntoh16(bpdu->mst_bpdu.version_3_length);
                            if (version_3_length < 64)
                            {
                                protocol_version_identifier = XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID;
                                bpdu->bpdu_header.protocol_version_identifier   = protocol_version_identifier;
                            }
                            else
                            {
                                UI16_T  msti_length;

                                msti_length     = version_3_length - 64;
                                if ( msti_length % sizeof(XSTP_TYPE_MstiConfigMsg_T) )
                                {
                                    protocol_version_identifier = XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID;
                                    bpdu->bpdu_header.protocol_version_identifier   = protocol_version_identifier;
                                }
                                else
                                {
                                    UI16_T  msti_num;

                                    msti_num    = msti_length / sizeof(XSTP_TYPE_MstiConfigMsg_T);
                                    if (msti_num >= XSTP_TYPE_MAX_INSTANCE_NUM)
                                    {
                                        protocol_version_identifier = XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID;
                                        bpdu->bpdu_header.protocol_version_identifier   = protocol_version_identifier;
                                    } /* End of if (msti_num is not less than XSTP_TYPE_MAX_INSTANCE_NUM) */
                                } /* End of if (version_3_length is representing an integral number) */
                            } /* End of if (version_3_length is less than 64 else msti_length is equal to or larger than zero) */
                        }
                        else
                        {
                            protocol_version_identifier = XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID;
                            bpdu->bpdu_header.protocol_version_identifier   = protocol_version_identifier;
                        }
                        valid  = TRUE;
                    }
                    else
                    {
                        valid  = FALSE;
                    }
                }
                break;

            /* 14.4(f) */
            default:
                valid  = FALSE;
                break;
        }

    }
    else
    {
        valid  = FALSE;
    }

    return valid;
} /* End of XSTP_RX_ValidateReceivedBpdu */

#ifdef XSTP_TYPE_PROTOCOL_MSTP
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_ProcessMstiMessage
 * ------------------------------------------------------------------------
 * PURPOSE  : Validate BPDU MSTI part and record the MSTI Configuration Message parameters
 * INPUT    : mst_bpdu  -- mst bpdu
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : If MSTP is not supported, XSTP_RX_ProcessMstiMessage does nothing.
 *            Ref to the description in 13.26.15, IEEE Std 802.1s(D14.1)-2002.
 *-------------------------------------------------------------------------
 */
static  void    XSTP_RX_ProcessMstiMessage(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, XSTP_TYPE_MstBpdu_T *mst_bpdu)
{
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_OM_InstanceData_T      *a_msti_ptr;
    XSTP_OM_PortVar_T           *temp_pom_ptr;
    XSTP_TYPE_MstiConfigMsg_T   *msti_config_msg;
    UI32_T                      mstid;
    UI16_T                      index;
    UI16_T                      version_3_length, msti_length;
    UI16_T                      msti_num;
    BOOL_T                      rcvd_internal;

    version_3_length    = L_STDLIB_Ntoh16(mst_bpdu->version_3_length);
    if (version_3_length < 64)
    {
        return;
    }
    msti_length         = version_3_length - 64;
    if ( msti_length % sizeof(XSTP_TYPE_MstiConfigMsg_T) )
    {
        return;
    }

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    rcvd_internal   = pom_ptr->common->rcvd_internal;

    if (rcvd_internal)
    {
        msti_num    = msti_length / sizeof(XSTP_TYPE_MstiConfigMsg_T);
        if (msti_num < SYS_ADPT_MAX_NBR_OF_MST_INSTANCE)
        {
            for (index = 0; index < msti_num; index++)
            {
                msti_config_msg     = &(mst_bpdu->msti_configureation_message[index]);
                mstid = (UI32_T)(L_STDLIB_Ntoh16(msti_config_msg->msti_regional_root_identifier.bridge_id_priority.bridge_priority) & 0x0FFF);
                a_msti_ptr         = XSTP_OM_GetInstanceInfoPtr(mstid);
                if (a_msti_ptr->instance_exist && a_msti_ptr->delay_flag)
                {
                    temp_pom_ptr    = &(a_msti_ptr->port_info[lport-1]);
                    if (temp_pom_ptr->is_member)
                    {
                        XSTP_RX_RecordMstiMessage(temp_pom_ptr, mst_bpdu, msti_config_msg);
                    }
                }
            } /* End of for */
        } /* End of if */
    } /* End of if */
    return;
} /* End of XSTP_RX_ProcessMstiMessage */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_RecordMstiMessage
 * ------------------------------------------------------------------------
 * PURPOSE  : Stores message priority vector and timers from BPDU to OM
 * INPUT    : bpdu    -- whole BPDU pointer
 *            msti_cfg_msg -- pointer of one MSTI configuration message
 * OUTPUT   : pom_ptr -- per port variables pointer
 * RETURN   : None
 * NOTE     : MSTI priority vector doesn't define the components root_id and ext_root_path_cost.
 *            We put cist_root_identifier and cist_external_path_cost though they are meaningless.
 *-------------------------------------------------------------------------
 */
static  void XSTP_RX_RecordMstiMessage(XSTP_OM_PortVar_T *pom_ptr, XSTP_TYPE_MstBpdu_T *bpdu, XSTP_TYPE_MstiConfigMsg_T   *msti_cfg_msg)
{
    XSTP_TYPE_BridgeId_T    zero_bridge_id;
    XSTP_TYPE_BridgeId_T    dsgn_bridge_id;
    XSTP_TYPE_PortId_T      dsgn_port_id;
    XSTP_TYPE_BridgeId_T    regional_root_id;

    memset(&zero_bridge_id, 0, XSTP_TYPE_BRIDGE_ID_LENGTH);
    XSTP_OM_CPY_NTOH_BRIDGE_ID(regional_root_id,msti_cfg_msg->msti_regional_root_identifier);
    /* fill designated bridge id as sending bridge id */
    XSTP_OM_CPY_NTOH_BRIDGE_ID(dsgn_bridge_id, bpdu->cist_bridge_identifier);
    XSTP_OM_SET_BRIDGE_ID_PRIORITY(dsgn_bridge_id, (UI16_T)(msti_cfg_msg->msti_bridge_priority <<8));
    XSTP_OM_SET_BRIDGE_ID_SYSIDEXT(dsgn_bridge_id, (UI16_T)L_STDLIB_Ntoh16(msti_cfg_msg->msti_regional_root_identifier.bridge_id_priority.bridge_priority));
    /* fill port id priority */
    XSTP_OM_SET_PORT_ID_PRIORITY(dsgn_port_id, (UI16_T)msti_cfg_msg->msti_port_priority);

    dsgn_port_id.port_id            = (UI16_T)((L_STDLIB_Ntoh16(bpdu->cist_port_identifier.port_id)) & 0x0FFF);
    dsgn_port_id.port_id            |= ((UI16_T)(msti_cfg_msg->msti_port_priority << 8) & 0xF000);
    XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->msg_priority,
                                    zero_bridge_id,
                                    0,
                                    regional_root_id,
                                    L_STDLIB_Ntoh32(msti_cfg_msg->msti_internal_root_path_cost),
                                    dsgn_bridge_id,
                                    dsgn_port_id,
                                    pom_ptr->port_id);

    XSTP_OM_TIMERS_FORMAT(pom_ptr->msg_times,
                                                    0,
                                                    0,
                                                    0,
                                                    0,
                                                    (msti_cfg_msg)->msti_remaining_hops);
    (pom_ptr)->msti_config_msg    = (msti_cfg_msg);
    (pom_ptr)->rcvd_msg           = TRUE;
}

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

