#ifndef DOT1X_VM_H
#define DOT1X_VM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_DOT1X == TRUE)
#include "dot1x_sm_auth.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
        UI32_T callback_function);

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
    UI32_T   session_timeout,       UI32_T  server_ip);

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
void DOT1X_VM_NotifyPortLinkUp(UI32_T unit, UI32_T port);

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
void DOT1X_VM_NotifyPortLinkDown(UI32_T unit, UI32_T port);

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
void DOT1X_VM_NotifyPortAdminUp(UI32_T unit, UI32_T port);

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
void DOT1X_VM_NotifyPortAdminDown(UI32_T unit, UI32_T port);

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
BOOL_T DOT1X_VM_ProcessTimeoutEvent();

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
void DOT1X_VM_SendEvent(UI32_T lport, DOT1X_SM_AUTH_Event_T ev);

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
void DOT1X_VM_EnableReauthTimer(UI32_T lport);

#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
#endif   /* End of DOT1X_VM_H */
