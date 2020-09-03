#ifndef LIB1X_COMMON_H
#define LIB1X_COMMON_H

#include "1x_types.h"
#include "security_backdoor.h"

/*for EH *//*Mercury_V2-00030*/
#define DOT1X_OM_SET_REAUTHPERIOD_FUNC_NO   1
#define DOT1X_OM_SET_QUIETPERIOD_FUNC_NO    2
#define DOT1X_OM_SET_TXPERIOD_FUNC_NO       3
#define DOT1X_OM_SET_MAXREQ_FUNC_NO     4
#define DOT1X_LIB1X_INIT_AUTHENTICATOR_FUNC_NO  5
#define DOT1X_SEND_PACKET_FUNC_NO       6
#define DOT1X_TASK_CREATEDOT1XTASK_FUNC_NO  7
#define DOT1X_TASK_ANNOUNCEDOT1XPACKET_FUNC_NO  8

#define INC(X)  { (X) = (X) + 1; if (X > 255) X = 0; }

typedef enum DOT1X_DebugType_E
{
    MESS_DBG_AUTH       = 0x00000001,
    MESS_DBG_AUTHSM     = 0x00000002,
    MESS_DBG_AUTHNET    = 0x00000004,
    MESS_DBG_KRCSM      = 0x00000008,
    MESS_DBG_KXSM       = 0x00000010,
    MESS_DBG_SUPP       = 0x00000020,
    MESS_DBG_NAL        = 0x00000040,
    MESS_DBG_BSM        = 0x00000080,
    MESS_DBG_RAD        = 0x00000100,
    MESS_AUTH_LOG       = 0x00000200,
    MESS_ERROR_OK       = 0x00000400,
    MESS_ERROR_FATAL    = 0x00000800,
    MESS_DBG_SPECIAL    = 0x00001000,

    MESS_DBG_SUP_CONFIG = 0x00002000,
    MESS_DBG_SUP_STATE  = 0x00004000,
    MESS_DBG_SUP_TIMER  = 0x00008000,
    MESS_DBG_SUP_ERROR  = 0x00010000,
    MESS_DBG_SUP_ALL    = 0x00020000

}DOT1X_DebugType_T;

#define lib1x_message(op, msg)                      SECURITY_DEBUG_PRINT0(DOT1X_OM_GetDebugFlag(), op, DOT1X_OM_GetDebugPrompt(op), msg)
#define lib1x_message1(op, msg, arg)                SECURITY_DEBUG_PRINT1(DOT1X_OM_GetDebugFlag(), op, DOT1X_OM_GetDebugPrompt(op), msg, arg)
#define lib1x_message2(op, msg, arg1, arg2)         SECURITY_DEBUG_PRINT2(DOT1X_OM_GetDebugFlag(), op, DOT1X_OM_GetDebugPrompt(op), msg, arg1, arg2)
#define lib1x_message3(op, msg, arg1, arg2, arg3)   SECURITY_DEBUG_PRINT3(DOT1X_OM_GetDebugFlag(), op, DOT1X_OM_GetDebugPrompt(op), msg, arg1, arg2, arg3)
#define lib1x_message4(op, msg, arg1, arg2, arg3, arg4)   SECURITY_DEBUG_PRINT4(DOT1X_OM_GetDebugFlag(), op, DOT1X_OM_GetDebugPrompt(op), msg, arg1, arg2, arg3, arg4)

#define lib1x_sup_assert(tf, desc, terminal)        DOT1X_COMMON_SupAsserMsg(tf, desc, terminal,\
                                                         __FILE__, __LINE__, __FUNCTION__)

struct  PKT_LSTNR_tag;
struct  PKT_XMIT_tag;
struct Auth_Pae_tag;
struct Supp_Pae_tag;    
struct lib1x_ptsm;
/*
#define MAX_AUTHENTICATED_MAC  10
typedef struct DOT1X_Authencated_MAC_S
{
 BOOL_T authenticated;
 UI8_T authenticated_mac[6];
}DOT1X_Authenticated_MAC_T;
*/
typedef struct Global_Params_tag
{
     BOOL_T      authAbort;
     BOOL_T      authFail;
     BOOL_T      authStart;
     BOOL_T      authTimeout;
     BOOL_T      authSuccess;
     int     currentId;      /* Id for current authentication session*/
     BOOL_T      initialize;
     BOOL_T      portEnabled;
     /*PORT_STATUS_TYPE*/UI32_T  portStatus;
     /*PORT_STATUS_TYPE*/UI32_T  pre_portStatus;
     BOOL_T      reAuthenticate;
     int     receivedId;
     /*PORT_STATUS_TYPE*/UI32_T  suppStatus;
     ROLE         currentRole; 
     struct Auth_Pae_tag * theAuthenticator;
     struct Supp_Pae_tag * theSupplicant;    
     /*kevin add*/
     UI32_T which_event;
     //BOOL_T set_MAC_to_PSEC;
     //UI8_T MAC_in_PSEC[6];
     BOOL_T is_port_admin_enable;
     UI32_T session_start_time;/*for MIB*/
     //DOT1X_Authenticated_MAC_T authenticated[MAX_AUTHENTICATED_MAC];
     //UI32_T authenticated_counter;
     BOOL_T authenticating_flag;
     BOOL_T eap_start_is_auto_created;/* check if eap-start is auto created ,not forward */
     UI8_T first_mac[6];
     BOOL_T is_first_mac_auth;
     UI32_T authorized_vid;
     UI32_T mac_entry_index;
     BOOL_T mac_is_authorized;
} Global_Params;

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  DOT1X_COMMON_SupAsserMsg
 *------------------------------------------------------------------------------
 * PURPOSE  : Check the value of 'tf'.  If it returns false, print out some debug
 *            information and either return FALSE, or terminate.  (Depending on the
 *            value of terminal.)  In general, this function should be called via the
 *            DOT1X_DEBUG_SUP_Assert() macro, so that the filename, line number,
 *            and function name are automatically filled in.
 * INPUT    : tf, desc
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
int DOT1X_COMMON_SupAsserMsg(int tf, char *desc, int terminal, char *file, int line,
		     const char *function);

#endif /*LIB1X_COMMON_H*/
