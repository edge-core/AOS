/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_DOT1X == TRUE)
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

#include "l_stdlib.h"

#include "dot1x_sm_auth.h"
#include "dot1x_timer_auth.h"
#include "dot1x_vm.h"
#include "1x_om.h"
#include "1x_common.h"
#include "1x_eapol.h"
#include "radius_mgr.h"
#include "netaccess_vm.h"

#include "vlan_om.h"
#include "vlan_pom.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T DOT1X_VM_ProcessEapolPacket(DOT1X_SM_AUTH_Obj_T *sm_p);
static BOOL_T DOT1X_VM_ProcessRadiusPacket(DOT1X_SM_AUTH_Obj_T *sm_p);
static void DOT1X_VM_ProecessEapRequestPacket(DOT1X_SM_AUTH_Obj_T *sm_p);
static BOOL_T DOT1X_VM_IsEapolPacketValid(DOT1X_SM_AUTH_Obj_T *sm_p);
static void DOT1X_VM_StoreIdFromRadiusPacket(DOT1X_SM_AUTH_Obj_T *sm_p);
static void DOT1X_VM_ProcessRadiusAccessReject(DOT1X_SM_AUTH_Obj_T *sm_p);
static void DOT1X_VM_ProcessRadiusAccessChallenge(DOT1X_SM_AUTH_Obj_T *sm_p);
static void DOT1X_VM_SendEventToStateMachine(UI32_T lport, DOT1X_SM_AUTH_Event_T ev);
static BOOL_T DOT1X_VM_IsStateMachineInAuthenticating(DOT1X_SM_AUTH_Obj_T *sm_p);

/* EXPORTED SUBPROGRAM BODIES
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_VM_AsyncAuthCheck
 * --------------------------------------------------------------------------
 * PURPOSE : Notify dot1x there is a new eap packet
 * INPUT   : lport - port number
 *           src_mac - source mac address
 *           dst_mac - destination mac address
 *           tag_info - dot1q pvid
 *           type - packet frame type
 *           eappkt_data - packet data
 *           eappkt_length - packet length
 *           eap_start_is_auto_created
 *           callback_function - callback function
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : Called by netaccess and dot1x port control mode must be in auto
 *           mode.
 * --------------------------------------------------------------------------
 */
BOOL_T DOT1X_VM_AsyncAuthCheck(UI32_T lport, UI8_T *src_mac, UI8_T *dst_mac,
    UI16_T tag_info, UI16_T type, UI8_T *eappkt_data, UI32_T eappkt_length,
        UI32_T callback_function)
{
    DOT1X_SM_AUTH_Obj_T *sm_p;
    VLAN_OM_Dot1qPortVlanEntry_T    port_vlan_entry;
    UI32_T vid;

    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        lib1x_message1(MESS_DBG_AUTHSM, "lport(%lu) is not existing", lport);
        return FALSE;
    }

    sm_p = DOT1X_OM_GetSMObjByPortMac(lport, src_mac);
    if (NULL == sm_p)
    {
        sm_p = DOT1X_OM_NewSMObj(lport, src_mac, eappkt_data ? TRUE : FALSE);

        if (NULL == sm_p)
        {
            lib1x_message(MESS_DBG_AUTHSM, "Not available SM Object");
            return FALSE;
        }

        /* Pass the src_mac before call DOT1X_SM_AUTH_Go.
         * The src_mac will be used as the dst_mac in EAP packet send to
         * supplicant.
         */
        memcpy(sm_p->dot1x_packet_t.src_mac, src_mac, sizeof(sm_p->dot1x_packet_t.src_mac));
        DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_INIT_EV);
    }

    /* For port based mode,
     * If one is authenticating, drop other EAP packet.
     */
    if (DOT1X_OM_IsPortBasedMode(lport))
    {
        /* When state machine enters authenticating status, we should check if one is doing authentication.
         * But if one port is applied Guest VLAN, we should skip the checking.
         */
        if (FALSE == sm_p->is_applied_guest_vlan)
        {
            if (TRUE == DOT1X_VM_IsStateMachineInAuthenticating(sm_p))
            {
                if (memcmp(sm_p->dot1x_packet_t.src_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN) != 0)
                {
                    lib1x_message(MESS_DBG_AUTHSM, "PAE is authenticating");
                    return FALSE;
                }
            }
        }
    }

    /* Drop all EAP packet if current PAE state is Held
     */
    if (DOT1X_SM_AUTH_HELD_ST == sm_p->current_state)
    {
        lib1x_message(MESS_DBG_AUTHSM, "PAE is in Held stste");
        return FALSE;
    }

    vid = tag_info & 0x0fff; /* 12 bit vid */

    /* if vid == 0 then need to get the PVID of this port
     */
    if (0 == vid)
    {
        if (FALSE == VLAN_POM_GetDot1qPortVlanEntry(lport, &port_vlan_entry))
        {
            lib1x_message1(MESS_DBG_AUTHSM, "VLAN_POM_GetDot1qPortVlanEntry failed with"
                " lport=%lu", lport);
            return FALSE;
        }

        vid = port_vlan_entry.dot1q_pvid_index;
    }

    sm_p->dot1x_packet_t.packet_data = eappkt_data;
    memcpy(sm_p->dot1x_packet_t.dst_mac, dst_mac, sizeof(sm_p->dot1x_packet_t.dst_mac));
    memcpy(sm_p->dot1x_packet_t.src_mac, src_mac, sizeof(sm_p->dot1x_packet_t.src_mac));
    sm_p->dot1x_packet_t.tag_info = tag_info;
    sm_p->dot1x_packet_t.packet_type = type;
    sm_p->dot1x_packet_t.packet_length = eappkt_length;
    sm_p->dot1x_packet_t.src_lport = lport;
    sm_p->dot1x_packet_t.src_vid = vid;

    if (eappkt_data)
    {
        return DOT1X_VM_ProcessEapolPacket(sm_p);
    }

    return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_VM_AnnounceRadiusPacket
 * ------------------------------------------------------------------------
 * PURPOSE : Whenever RADIUS client received a EAP attribute packet,it calls
 *           this function to handle the packet.
 * INPUT   : result -- authenticated result
 *           data_buf -- radius packet data
 *           data_len -- radius packet data length
 *           src_port -- the port receiving the packet
 *           src_mac -- source MAC address
 *           src_vid -- source VLAN id
 *           authorized_vlan_list -- vlan list stored in the radius packet
 *           authorized_qos_list -- qos list stored in the radius packet
 *           session_timeout -- session timeout
 *           server_ip -- radius server ip
 * RETURN  : None
 * NOTE    :
 * ------------------------------------------------------------------------
 */
void DOT1X_VM_AnnounceRadiusPacket(
    UI32_T  result,                 UI8_T *data_buf,
    UI32_T  data_len,               UI32_T  src_port,
    UI8_T   *src_mac,               UI32_T  src_vid,
    char    *authorized_vlan_list,  char    *authorized_qos_list,
    UI32_T   session_timeout,       UI32_T  server_ip)
{
    DOT1X_SM_AUTH_Obj_T *sm_p = NULL;

    sm_p = DOT1X_OM_GetSMObjByPortMac(src_port, src_mac);
    if (NULL == sm_p)
    {
        return;
    }

    memcpy(sm_p->radius_packet_t.packet_data, data_buf, data_len);
    sm_p->radius_packet_t.packet_length = data_len;

    sm_p->radius_packet_t.radius_auth_result = result;
    sm_p->radius_packet_t.authorized_vlan_list = authorized_vlan_list;
    sm_p->radius_packet_t.authorized_qos_list  = authorized_qos_list;
    sm_p->radius_packet_t.authorized_session_time = session_timeout;
    sm_p->radius_packet_t.server_ip = server_ip;
    if (strlen(authorized_vlan_list) >= 0)
    {
        sm_p->radius_packet_t.authenticated_vid = atoi(authorized_vlan_list);
    }
    else
    {
        sm_p->radius_packet_t.authenticated_vid = 0;
    }

    DOT1X_VM_ProcessRadiusPacket(sm_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_VM_NotifyPortLinkUp
 * ---------------------------------------------------------------------
 * PURPOSE  : port link up notification
 * INPUT    : unit, port
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : networkaccess is upper than dot1x.
 *            therefore, dot1x can't register callback and
 *            networkaccess just call dot1x directly
 * ---------------------------------------------------------------------
 */
void DOT1X_VM_NotifyPortLinkUp(UI32_T unit, UI32_T port)
{
    UI32_T lport;

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport))
    {
        return;
    }

    DOT1X_OM_SetPortEnabled(lport, TRUE);
    DOT1X_VM_SendEventToStateMachine(lport, DOT1X_SM_AUTH_PORT_UP_EV);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_VM_NotifyPortLinkDown
 * ---------------------------------------------------------------------
 * PURPOSE  : port link down notification
 * INPUT    : unit, port
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : networkaccess is upper than dot1x.
 *            therefore, dot1x can't register callback and
 *            networkaccess just call dot1x directly
 * ---------------------------------------------------------------------
 */
void DOT1X_VM_NotifyPortLinkDown(UI32_T unit, UI32_T port)
{
    UI32_T lport;

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport))
    {
        return;
    }

    DOT1X_OM_SetPortEnabled(lport, FALSE);
    DOT1X_VM_SendEventToStateMachine(lport, DOT1X_SM_AUTH_PORT_DOWN_EV);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_VM_NotifyPortAdminUp
 * ---------------------------------------------------------------------
 * PURPOSE  : port admin up notification
 * INPUT    : unit, port
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : networkaccess is upper than dot1x.
 *            therefore, dot1x can't register callback and
 *            networkaccess just call dot1x directly
 * ---------------------------------------------------------------------
 */
void DOT1X_VM_NotifyPortAdminUp(UI32_T unit, UI32_T port)
{
    UI32_T lport;

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport))
    {
        return;
    }

    DOT1X_OM_SetPortEnabled(lport, TRUE);
    DOT1X_VM_SendEventToStateMachine(lport, DOT1X_SM_AUTH_PORT_UP_EV);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_VM_NotifyPortAdminDown
 * ---------------------------------------------------------------------
 * PURPOSE  : port admin down notification
 * INPUT    : unit, port
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : networkaccess is upper than dot1x.
 *            therefore, dot1x can't register callback and
 *            networkaccess just call dot1x directly
 * ---------------------------------------------------------------------
 */
void DOT1X_VM_NotifyPortAdminDown(UI32_T unit, UI32_T port)
{
    UI32_T lport;

    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_UserPortToLogicalPort(unit, port, &lport))
    {
        return;
    }

    DOT1X_OM_SetPortEnabled(lport, FALSE);
    DOT1X_VM_SendEventToStateMachine(lport, DOT1X_SM_AUTH_PORT_ADMIN_DOWN_EV);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_VM_ProcessTimeoutEvent
 * ---------------------------------------------------------------------
 * PURPOSE : Process timeout event
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_VM_ProcessTimeoutEvent()
{
    UI32_T lport;

    if (VAL_dot1xPaeSystemAuthControl_disabled == DOT1X_OM_Get_SystemAuthControl())
    {
        return FALSE;
    }

    for (lport = 1; lport <= DOT1X_MAX_PORT; lport++)
    {
        UI32_T port_control;

        port_control = DOT1X_OM_Get_PortControlMode(lport);
        if (VAL_dot1xAuthAuthControlledPortControl_auto != port_control)
        {
            continue;
        }

        {
            DOT1X_TIMER_AUTH_ProcessTimeoutEvent(lport);
        }
    }

    return TRUE;
}

static BOOL_T DOT1X_VM_ProcessEapolPacket(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    const struct lib1x_eapol *eapol_p;
    DOT1X_AuthStatsEntry_T *stats_p = NULL;

    if (NULL == sm_p)
    {
        lib1x_message(MESS_DBG_AUTHSM, "Null pointer");
        return FALSE;
    }

    if (FALSE == DOT1X_VM_IsEapolPacketValid(sm_p))
    {
        return FALSE;
    }

    sm_p->is_eapol_rx = TRUE;

    eapol_p = (struct lib1x_eapol *)sm_p->dot1x_packet_t.packet_data;
    stats_p = DOT1X_OM_GetAuthStats(sm_p->dot1x_packet_t.src_lport);

    stats_p->dot1xAuthEapolFramesRx++;
    stats_p->dot1xAuthLastEapolFrameVersion = eapol_p->protocol_version;
    memcpy(stats_p->dot1xAuthLastEapolFramesSource, sm_p->dot1x_packet_t.src_mac, SYS_ADPT_MAC_ADDR_LEN);

    switch (eapol_p->packet_type)
    {
        case LIB1X_EAPOL_LOGOFF:
            stats_p->dot1xAuthEapolLogoffFramesRx++;

            lib1x_message(MESS_DBG_AUTHSM, "(EAP Logoff) Send EAPLOGOFF_EV to sm");
            DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_EAPLOGOFF_EV);
            break;

        case LIB1X_EAPOL_START:
            /* Page 53.*/
            sm_p->is_a_mac_authenticating = TRUE;

            stats_p->dot1xAuthEapolStartFramesRx++;

            lib1x_message(MESS_DBG_AUTHSM, "(EAP Start) Send EAPSTART_EV to sm");
            DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_EAPSTART_EV);
            break;

        case LIB1X_EAPOL_EAPPKT:
        {
            /* Page 53.
             * TODO: Parse EAP PACKET and if EAP Response/Identity Packet
             * is received set rxRespId of authpae to TRUE;
             */
            sm_p->auth_session_stats_entry.dot1xAuthSessionFramesRx++;
            sm_p->auth_session_stats_entry.dot1xAuthSessionOctetsRx += sm_p->dot1x_packet_t.packet_length;

            DOT1X_VM_ProecessEapRequestPacket(sm_p);
        }
            break;

        case LIB1X_EAPOL_KEY:
        case LIB1X_EAPOL_ENCASFALERT:
            break;

        default:
            stats_p->dot1xAuthInvalidEapolFramesRx++;
            break;
    }

    return TRUE;
}

static BOOL_T DOT1X_VM_ProcessRadiusPacket(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    const struct lib1x_radiushdr *header_p;

    if (NULL == sm_p)
    {
        return FALSE;
    }

    if (RADIUS_AUTHENTICATION_TIMEOUT == sm_p->radius_packet_t.radius_auth_result)
    {
        lib1x_message(MESS_DBG_AUTHSM, "(Radius authentication timeout) Send TIMEOUT_EV to sm");
        DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_TIMEOUT_EV);

        return TRUE;
    }

    if (0 == sm_p->radius_packet_t.packet_length)
    {
        /* RADIUS return fail. One case of this failure is no configure any
         * RADIUS server IP address. RADIUS have no send any authentication
         * packet and quickly return.
         * So that take this case as authentication failure and then enter
         * HELD state. Do this can avoid the state machine loop with high speed
         * and send a lot of EAPOL req/ID to supplicant.
         */
        lib1x_message1(MESS_DBG_AUTHSM,
            "(Radius return code %d) Send AFAIL_EV to sm",
            (int)sm_p->radius_packet_t.radius_auth_result);
        DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_AFAIL_EV);

        return TRUE;
    }

    header_p = (struct lib1x_radiushdr *)sm_p->radius_packet_t.packet_data;

    switch (header_p->code)
    {
        case LIB1X_RAD_ACCACT:
        {
            BOOL_T authorized_result = TRUE;

            authorized_result &= NETACCESS_VM_IsValidVlanList(sm_p->dot1x_packet_t.src_lport,
                sm_p->radius_packet_t.authorized_vlan_list);

            authorized_result &= NETACCESS_VM_IsValidQosProfiles(sm_p->dot1x_packet_t.src_lport,
                sm_p->radius_packet_t.authorized_qos_list);

            /* do as LIB1X_RAD_ACCREJ case
             */
            if (FALSE == authorized_result)
            {
                lib1x_message(MESS_DBG_AUTHSM, "(Access-Accept) Authorized result is FALSE, do as Access-Reject case");
                DOT1X_VM_ProcessRadiusAccessReject(sm_p);
                break;
            }

            sm_p->auth_diag_entry.dot1xAuthBackendAuthSuccesses++;

            sm_p->rinfo.rad_stateavailable = FALSE;
            sm_p->rinfo.rad_statelength = 0;

            DOT1X_VM_StoreIdFromRadiusPacket(sm_p);

            lib1x_message(MESS_DBG_AUTHSM, "(Access-Accept) Send ASUCCESS_EV to sm");
            DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_ASUCCESS_EV);
            break;
        }

        case LIB1X_RAD_ACCREJ:
            DOT1X_VM_ProcessRadiusAccessReject(sm_p);
            break;

        case LIB1X_RAD_ACCCHL:
            sm_p->auth_diag_entry.dot1xAuthBackendAccessChallenges++;

            DOT1X_VM_ProcessRadiusAccessChallenge(sm_p);
            break;

        default:
            lib1x_message1(MESS_DBG_AUTHSM, "(Unknown Radius Packet) Type = %u", header_p->code);
            DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_AFAIL_EV);
            return FALSE;
    }

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */
static void DOT1X_VM_ProecessEapRequestPacket(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    const struct lib1x_eap *eap_p;
    const struct lib1x_eap_rr *eap_rr_p;
    DOT1X_AuthStatsEntry_T *stats_p;

    eap_p = (struct lib1x_eap * )(sm_p->dot1x_packet_t.packet_data + LIB1X_EAPOL_HDRLEN);
    eap_rr_p = (struct lib1x_eap_rr *)(sm_p->dot1x_packet_t.packet_data + LIB1X_EAPOL_HDRLEN + LIB1X_EAP_HDRLEN);
    stats_p = DOT1X_OM_GetAuthStats(sm_p->dot1x_packet_t.src_lport);

    if (LIB1X_EAP_RESPONSE != eap_p->code)
    {
        lib1x_message1(MESS_DBG_AUTHSM, "(EAP Packet) Receive a non EAP response packet from the peer (code = %u)", eap_p->code);
        return;
    }

    if (LIB1X_EAP_RRNAK != eap_rr_p->type)
    {
        lib1x_message(MESS_DBG_AUTHSM, "(EAP non-ack response packets) Receive a non-ack response");
        sm_p->auth_diag_entry.dot1xAuthBackendNonNakResponsesFromSupplicant++;
    }

    if (LIB1X_EAP_RRIDENTITY == eap_rr_p->type)
    {
        if (sm_p->current_id != eap_p->identifier)
        {
            lib1x_message2(MESS_DBG_AUTHSM, "(EAP Response/Identity) Id (%u) in the received is not equal to the one of the state machine (%lu). Abort.", eap_p->identifier, sm_p->current_id);
            return;
        }

        stats_p->dot1xAuthEapolRespIdFramesRx++;

        lib1x_message(MESS_DBG_AUTHSM, "(EAP Response/Identity) Send RESPID_EV to sm");
        DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_RESPID_EV);
    }
    else
    {
        stats_p->dot1xAuthEapolRepsFramesRx++;

        lib1x_message(MESS_DBG_AUTHSM, "(EAP response) Send RXRESP_EV to sm");
        DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_RXRESP_EV);
    }
}

static BOOL_T DOT1X_VM_IsEapolPacketValid(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    const struct lib1x_eapol *eapol_p;
    const struct lib1x_eap_rr *eap_rr_p;
    DOT1X_AuthStatsEntry_T *stats_p;
    UI16_T packet_body_length;

    eapol_p = (struct lib1x_eapol *)(sm_p->dot1x_packet_t.packet_data);
    eap_rr_p = (struct lib1x_eap_rr *)(sm_p->dot1x_packet_t.packet_data + LIB1X_EAPOL_HDRLEN + LIB1X_EAP_HDRLEN);
    stats_p = DOT1X_OM_GetAuthStats(sm_p->dot1x_packet_t.src_lport);
    packet_body_length = L_STDLIB_Ntoh16(eapol_p->packet_body_length);

    if (sm_p->dot1x_packet_t.packet_length < LIB1X_EAPOL_HDRLEN)
    {
        lib1x_message1(MESS_DBG_AUTHSM, "(EAPOL Packet) Packet length (%u) is too small", packet_body_length);

        stats_p->dot1xAuthEapLengthErrorFramesRx++;
        return FALSE;
    }

    if (   ((LIB1X_EAPOL_LOGOFF == eapol_p->packet_type) || (LIB1X_EAPOL_START == eapol_p->packet_type))
        && (0 != packet_body_length))
    {
        lib1x_message1(MESS_DBG_AUTHSM, "(EAPOL Packet) EAP Start/Logoff packet body length (%u) is not zero.", packet_body_length);

        stats_p->dot1xAuthEapLengthErrorFramesRx++;
        return FALSE;
    }

    if (LIB1X_EAPOL_EAPPKT == eapol_p->packet_type)
    {
        if (packet_body_length < LIB1X_EAPOL_HDRLEN)
        {
            lib1x_message1(MESS_DBG_AUTHSM, "(EAPOL Packet) Packet length specified in length field (value = %u) is too small", packet_body_length);

            stats_p->dot1xAuthEapLengthErrorFramesRx++;
            return FALSE;
        }

        switch (eap_rr_p->type)
        {
            case LIB1X_EAP_RRIDENTITY:
            case LIB1X_EAP_RRMD5:

#if ( SYS_CPNT_DOT1X_TTLS_TLS_PEAP == TRUE)
            case LIB1X_EAP_TLS:
            case LIB1X_EAP_TTLS:
            case LIB1X_EAP_PEAP:
#endif

            case LIB1X_EAP_ZLXEAP:
            case LIB1X_EAP_RRNAK:
                break;

            default:
                lib1x_message1(MESS_DBG_AUTHSM, "Unknown EAP packet type? (%u)", eap_rr_p->type);
                return FALSE;
        }
    }

    return TRUE;
}

static void DOT1X_VM_StoreIdFromRadiusPacket(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    const struct lib1x_radiushdr *header_p;
    const struct lib1x_radiusattr *av_pair_p;
    const struct lib1x_eap *eap_p;
    UI16_T av_pairs_len;

    header_p = (struct lib1x_radiushdr *)(sm_p->radius_packet_t.packet_data);
    av_pair_p = (struct lib1x_radiusattr *)(sm_p->radius_packet_t.packet_data + LIB1X_RADHDRLEN);
    av_pairs_len = L_STDLIB_Ntoh16(header_p->length) - LIB1X_RADHDRLEN;

    while (TRUE)
    {
        if (av_pairs_len <= LIB1X_RADATTRLEN)
        {
            lib1x_message(MESS_DBG_AUTHSM, "No EAP message found in radius packet");
            break;
        }

        if (LIB1X_RAD_EAP_MESSAGE == av_pair_p->type)
        {
            /* update current id from server
             */
            eap_p = (struct lib1x_eap *)(((UI8_T *)av_pair_p) + LIB1X_RADATTRLEN);
            sm_p->id_from_server = eap_p->identifier;
            break;
        }

        av_pairs_len -= av_pair_p->length;
        if (av_pairs_len <= 0)
        {
            /* no eap in the packet, set id_from_server to the value of current_id
             */
            sm_p->id_from_server = sm_p->current_id;
            break;
        }

        av_pair_p = (struct lib1x_radiusattr *)(((UI8_T *)av_pair_p) + av_pair_p->length);
    }
}

static void DOT1X_VM_ProcessRadiusAccessReject(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    sm_p->auth_diag_entry.dot1xAuthBackendAuthFails++;

    DOT1X_VM_StoreIdFromRadiusPacket(sm_p);

    sm_p->rinfo.rad_stateavailable = FALSE;
    sm_p->rinfo.rad_statelength = 0;

    lib1x_message(MESS_DBG_AUTHSM, "(Access-Reject) Send AFAIL_EV to sm");
    DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_AFAIL_EV);
}

static void DOT1X_VM_ProcessRadiusAccessChallenge(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    const struct lib1x_radiushdr *header_p;
    const struct lib1x_radiusattr *av_pair_p;
    const struct lib1x_eap *eap_p;
    UI16_T av_pairs_len;

    header_p = (struct lib1x_radiushdr *)(sm_p->radius_packet_t.packet_data);
    av_pair_p = (struct lib1x_radiusattr *)(sm_p->radius_packet_t.packet_data + LIB1X_RADHDRLEN);
    av_pairs_len = L_STDLIB_Ntoh16(header_p->length) - LIB1X_RADHDRLEN;

    sm_p->rinfo.eap_messlen_frmserver = 0;

    sm_p->rinfo.rad_stateavailable = FALSE;
    sm_p->rinfo.rad_statelength = 0;

    while (TRUE)
    {
        if (av_pairs_len <= LIB1X_RADATTRLEN)
        {
            lib1x_message(MESS_DBG_AUTHSM, "Malformed RADIUS packet");
            break;
        }

        if (LIB1X_RAD_EAP_MESSAGE == av_pair_p->type)
        {
            lib1x_message(MESS_DBG_AUTHSM, "(Radius Request, Challenge) Process EAP message");
            sm_p->rinfo.eap_messlen_frmserver += (av_pair_p->length - LIB1X_RADATTRLEN);

            eap_p = (struct lib1x_eap *)(((UI8_T *)av_pair_p) + LIB1X_RADATTRLEN);
            sm_p->id_from_server = eap_p->identifier;
        }

        if (LIB1X_RAD_STATE == av_pair_p->type)
        {
            lib1x_message(MESS_DBG_AUTHSM, "(Radius Request, Challenge) Process State");
            sm_p->rinfo.rad_stateavailable = TRUE;
            sm_p->rinfo.rad_statelength = av_pair_p->length - LIB1X_RADATTRLEN;
            memcpy(sm_p->rinfo.radius_state, ((UI8_T *)av_pair_p) + LIB1X_RADATTRLEN , sm_p->rinfo.rad_statelength);
        }

        av_pairs_len -= av_pair_p->length;
        if (av_pairs_len  <= 0)
        {
            /* no eap in the packet, set id_from_server to the value of current_id
             */
            sm_p->id_from_server = sm_p->current_id;
            break;
        }

        av_pair_p = (struct lib1x_radiusattr *)(((UI8_T *)av_pair_p) + av_pair_p->length);
    }

    if (0 < sm_p->rinfo.eap_messlen_frmserver)
    {
        lib1x_message(MESS_DBG_AUTHSM, "(Radius Request, Challenge) Send AREQ_EV to sm");
        DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_AREQ_EV);
    }
}

static void DOT1X_VM_SendEventToStateMachine(UI32_T lport, DOT1X_SM_AUTH_Event_T ev)
{
    if (VAL_dot1xPaeSystemAuthControl_enabled == DOT1X_OM_Get_SystemAuthControl())
    {
        DOT1X_VM_SendEvent(lport, ev);
    }
}

static BOOL_T DOT1X_VM_IsStateMachineInAuthenticating(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    return (DOT1X_SM_AUTH_AUTHENTICATED_ST == sm_p->current_state
         || DOT1X_SM_AUTH_REQUEST_ST       == sm_p->current_state
         || DOT1X_SM_AUTH_RESPONSE_ST      == sm_p->current_state
         || DOT1X_SM_AUTH_EAPSUCCESS_ST    == sm_p->current_state
         || DOT1X_SM_AUTH_EAPFAIL_ST       == sm_p->current_state
         || DOT1X_SM_AUTH_HELD_ST          == sm_p->current_state)? TRUE : FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_VM_SendEvent
 * ---------------------------------------------------------------------
 * PURPOSE : Send event to all working state machine on the specified port
 * INPUT   : lport, event
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
void DOT1X_VM_SendEvent(UI32_T lport, DOT1X_SM_AUTH_Event_T ev)
{
    DOT1X_SM_AUTH_Obj_T *obj_p = NULL;
    UI32_T pi;
    UI32_T i = 0;

    pi = i;
    obj_p = DOT1X_OM_GetNextWorkingSMObj(lport, &i);
    while (obj_p)
    {
        lib1x_message1(MESS_DBG_AUTHSM, "obj[%lu]", pi);
        DOT1X_SM_AUTH_Go(obj_p, ev);

        pi = i;
        obj_p = DOT1X_OM_GetNextWorkingSMObj(lport, &i);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_VM_EnableReauthTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Enable the reauthentication timer of the specified port
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
void DOT1X_VM_EnableReauthTimer(UI32_T lport)
{
    DOT1X_SM_AUTH_Obj_T *obj_p = NULL;
    UI32_T i = 0;

    obj_p = DOT1X_OM_GetNextWorkingSMObj(lport, &i);

    if (obj_p)
    {
        DOT1X_SM_AUTH_EnableReauthTimer(obj_p);

        obj_p = DOT1X_OM_GetNextWorkingSMObj(lport, &i);
    }
}
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */