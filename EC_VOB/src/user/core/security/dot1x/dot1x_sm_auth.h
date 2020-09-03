#ifndef DOT1X_SM_AUTH_H
#define DOT1X_SM_AUTH_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "1x_types.h"
#include "1x_auth_pae.h"
#include "leaf_ieee8021x.h"

#include "swctrl.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define DOT1X_SM_AUTH_STATE_TEMP(_)             \
    _(DOT1X_SM_AUTH_DISABLED_ST)                \
    _(DOT1X_SM_AUTH_PORT_DOWN_ST)               \
    _(DOT1X_SM_AUTH_AUTO_ST)                    \
    _(DOT1X_SM_AUTH_FORCE_AUTH_ST)              \
    _(DOT1X_SM_AUTH_FORCE_UNAUTH_ST)            \
    _(DOT1X_SM_AUTH_CONNECTING_ST)              \
    _(DOT1X_SM_AUTH_DISCONNECTED_ST)            \
    _(DOT1X_SM_AUTH_PROTO_UNAWARE_ST)           \
    _(DOT1X_SM_AUTH_AUTHENTICATED_ST)           \
    _(DOT1X_SM_AUTH_HELD_ST)                    \
    _(DOT1X_SM_AUTH_ABORTING_ST)                \
    _(DOT1X_SM_AUTH_LOGOFF_ST)                  \
    _(DOT1X_SM_AUTH_REQUEST_ST)                 \
    _(DOT1X_SM_AUTH_RESPONSE_ST)                \
    _(DOT1X_SM_AUTH_EAPSUCCESS_ST)              \
    _(DOT1X_SM_AUTH_EAPFAIL_ST)                 \
    _(DOT1X_SM_AUTH_SUCCESS_ST)                 \
    _(DOT1X_SM_AUTH_FAIL_ST)                    \
    _(DOT1X_SM_AUTH_TIMEOUT_ST)                 \
    _(DOT1X_SM_AUTH_INIT_ST)                    \
                                                \
    _(DOT1X_SM_AUTH_NUMBER_OF_STABLE_STATE)     \
                                                \
    _(DOT1X_SM_AUTH_INIT_BEGIN_ST)              \
    _(DOT1X_SM_AUTH_PORT_UP_BEGIN_ST)           \
    _(DOT1X_SM_AUTH_DISABLED_BEGIN_ST)          \
    _(DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST)         \
    _(DOT1X_SM_AUTH_AUTO_BEGIN_ST)              \
    _(DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST)        \
    _(DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST)      \
    _(DOT1X_SM_AUTH_PRE_CONNECTING_BEGIN_ST)    \
    _(DOT1X_SM_AUTH_CONNECTING_BEGIN_ST)        \
    _(DOT1X_SM_AUTH_DISCONNECTED_BEGIN_ST)      \
    _(DOT1X_SM_AUTH_PROTO_UNAWARE_BEGIN_ST)     \
    _(DOT1X_SM_AUTH_AUTHENTICATED_BEGIN_ST)     \
    _(DOT1X_SM_AUTH_HELD_BEGIN_ST)              \
    _(DOT1X_SM_AUTH_ABORTING_BEGIN_ST)          \
    _(DOT1X_SM_AUTH_LOGOFF_BEGIN_ST)            \
    _(DOT1X_SM_AUTH_REQUEST_BEGIN_ST)           \
    _(DOT1X_SM_AUTH_RESPONSE_BEGIN_ST)          \
    _(DOT1X_SM_AUTH_EAPSUCCESS_BEGIN_ST)        \
    _(DOT1X_SM_AUTH_EAPFAIL_BEGIN_ST)           \
    _(DOT1X_SM_AUTH_SUCCESS_BEGIN_ST)           \
    _(DOT1X_SM_AUTH_FAIL_BEGIN_ST)              \
    _(DOT1X_SM_AUTH_TIMEOUT_BEGIN_ST)           \
                                                \
    _(DOT1X_SM_AUTH_NUMBER_OF_STATE)

#define DOT1X_SM_AUTH_EVENT_TEMP(_)             \
    _(DOT1X_SM_AUTH_DISABLE_EV)                 \
    _(DOT1X_SM_AUTH_INIT_EV)                    \
    _(DOT1X_SM_AUTH_PORT_ADMIN_DOWN_EV)         \
    _(DOT1X_SM_AUTH_PORT_DOWN_EV)               \
    _(DOT1X_SM_AUTH_PORT_UP_EV)                 \
    _(DOT1X_SM_AUTH_FORCE_UNAUTH_EV)            \
    _(DOT1X_SM_AUTH_FORCE_AUTH_EV)              \
    _(DOT1X_SM_AUTH_AUTO_EV)                    \
    _(DOT1X_SM_AUTH_EAPSTART_EV)                \
    _(DOT1X_SM_AUTH_EAPLOGOFF_EV)               \
    _(DOT1X_SM_AUTH_TIMEOUT_EV)                 \
    _(DOT1X_SM_AUTH_REAUTH_EV)                  \
    _(DOT1X_SM_AUTH_RESPID_EV)                  \
    _(DOT1X_SM_AUTH_RXRESP_EV)                  \
    _(DOT1X_SM_AUTH_AREQ_EV)                    \
    _(DOT1X_SM_AUTH_ASUCCESS_EV)                \
    _(DOT1X_SM_AUTH_AFAIL_EV)                   \
    _(DOT1X_SM_AUTH_MANAGER_SETUP_SUCC_EV)      \
    _(DOT1X_SM_AUTH_MANAGER_SETUP_FAIL_EV)      \
                                                \
    _(DOT1X_SM_AUTH_NUMBER_OF_EVENT)

#define DOT1X_SM_AUTH_ELEM(e)                   e,
#define DOT1X_SM_AUTH_STR_ELEM(e)               #e,

#define DOT1X_SM_AUTH_STATE_ENUM_DEFINITION     DOT1X_SM_AUTH_STATE_TEMP(DOT1X_SM_AUTH_ELEM)
#define DOT1X_SM_AUTH_STATE_STR_LIST_DEFINITION DOT1X_SM_AUTH_STATE_TEMP(DOT1X_SM_AUTH_STR_ELEM)

#define DOT1X_SM_AUTH_EVENT_ENUM_DEFINITION     DOT1X_SM_AUTH_EVENT_TEMP(DOT1X_SM_AUTH_ELEM)
#define DOT1X_SM_AUTH_EVENT_STR_LIST_DEFINITION DOT1X_SM_AUTH_EVENT_TEMP(DOT1X_SM_AUTH_STR_ELEM)

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    DOT1X_SM_AUTH_STATE_ENUM_DEFINITION
} DOT1X_SM_AUTH_State_T;

typedef enum
{
    DOT1X_SM_AUTH_EVENT_ENUM_DEFINITION
} DOT1X_SM_AUTH_Event_T;

typedef struct
{
    BOOL_T is_started;
    UI32_T seconds;
} DOT1X_SM_AUTH_Timer_T;

typedef struct
{
    /* status
     */
    DOT1X_SM_AUTH_State_T current_state;
    UI32_T port_status;
    UI32_T port_mode;
    UI32_T current_id;
    UI32_T reauth_count;
    UI32_T request_count;
    UI32_T id_from_server;
    UI32_T session_start_time;

    BOOL_T is_auth_succ_before;
    BOOL_T is_a_mac_authenticating; /* this should not used ?? */
    BOOL_T is_first_mac_auth;
    BOOL_T is_ever_pass_auth;
    BOOL_T is_eapol_rx;             /* TRUE, if receive EAPOL packet */
    BOOL_T is_applying_guest_vlan;  /* TRUE, means the state machine is waiting for applying guest VALN.
                                     */
    BOOL_T is_applied_guest_vlan;   /* TRUE, means the guest VLAN is applied already, FALSE, is not yet.
                                     */

    /* information from radius
     */
    struct radius_info rinfo;

    DOT1X_SM_AUTH_Timer_T gen_timer;
    DOT1X_SM_AUTH_Timer_T reauth_timer;

    /* last received dot1x and radius packets
     */
    DOT1X_PACKET_T dot1x_packet_t;
    RADIUS_PACKET_T radius_packet_t;

    /* used for MIB
     */
    DOT1X_AuthDiagEntry_T auth_diag_entry;
    DOT1X_AuthSessionStatsEntry_T auth_session_stats_entry;
    SWDRV_IfTableStats_T if_table_stats;
} DOT1X_SM_AUTH_Obj_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_Init
 * ---------------------------------------------------------------------
 * PURPOSE : Initialize authenticator state machine
 * INPUT   : lport - port number
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_SM_AUTH_Init(DOT1X_SM_AUTH_Obj_T *sm_p, UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_Go
 * ---------------------------------------------------------------------
 * PURPOSE : Run authenticator state machine
 * INPUT   : sm_p - pointer to the state machine object
 *           ev - received event type
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
void DOT1X_SM_AUTH_Go(DOT1X_SM_AUTH_Obj_T *sm_p, DOT1X_SM_AUTH_Event_T ev);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_EnableReauthTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Enable reauthentication timer
 * INPUT   : lport - port number
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : If timer is running, it needs to update the remaining
 *           seconds. Or, it will do nothing.
 * ---------------------------------------------------------------------
 */
void DOT1X_SM_AUTH_EnableReauthTimer(DOT1X_SM_AUTH_Obj_T *sm_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_DisableReauthTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Disable reauthentication timer
 * INPUT   : lport - port number
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : It will do the thing as to stop the timer whether it is
 *           started or stoped before.
 * ---------------------------------------------------------------------
 */
void DOT1X_SM_AUTH_DisableReauthTimer(DOT1X_SM_AUTH_Obj_T *sm_p);

#endif   /* End of DOT1X_SM_AUTH_H */
