/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>

#include "sys_cpnt.h"
#if (SYS_CPNT_DOT1X == TRUE)

#include "sys_module.h"
#include "sys_callback_mgr.h"
#include "sys_time.h"
#include "l_stdlib.h"

#include "dot1x_sm_auth.h"
#include "dot1x_timer_auth.h"
#include "1x_om.h"
#include "1x_mgr.h"
#include "1x_common.h"
#include "1x_eapol.h"
#if (SYS_CPNT_ACCOUNTING == TRUE)
#include "aaa_def.h"
#include "aaa_pmgr.h"
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
#include "radius_type.h"
#include "radius_pmgr.h"
#include "netaccess_om.h"
#include "netaccess_vm.h"

#include "vlan_om.h"
#include "vlan_pom.h"
#include "vlan_pmgr.h"
#include "swctrl_om.h"
#include "lan.h"
#include "nmtr_mgr.h"
#include "k_l_mm.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */

/* Note: the events are shown in state machine should transfer to XXX_BEGINE_ST, else transfer to itself (XXX_ST).
 */
static DOT1X_SM_AUTH_State_T fsm[DOT1X_SM_AUTH_NUMBER_OF_STABLE_STATE][DOT1X_SM_AUTH_NUMBER_OF_EVENT] =
{
    /*                                    DOT1X_SM_AUTH_DISABLE_EV         DOT1X_SM_AUTH_INIT_EV        DOT1X_SM_AUTH_PORT_ADMIN_DOWN_EV  DOT1X_SM_AUTH_PORT_DOWN_EV        DOT1X_SM_AUTH_PORT_UP_EV        DOT1X_SM_AUTH_FORCE_UNAUTH_EV        DOT1X_SM_AUTH_FORCE_AUTH_EV        DOT1X_SM_AUTH_AUTO_EV           DOT1X_SM_AUTH_EAPSTART_EV            DOT1X_SM_AUTH_EAPLOGOFF_EV           DOT1X_SM_AUTH_TIMEOUT_EV           DOT1X_SM_AUTH_REAUTH_EV            DOT1X_SM_AUTH_RESPID_EV          DOT1X_SM_AUTH_RXRESP_EV          DOT1X_SM_AUTH_AREQ_EV           DOT1X_SM_AUTH_ASUCCESS_EV          DOT1X_SM_AUTH_AFAIL_EV          DOT1X_SM_AUTH_MANAGER_SETUP_SUCC_EV   DOT1X_SM_AUTH_MANAGER_SETUP_FAIL_EV */
    /* DOT1X_SM_AUTH_DISABLED_ST      */ {DOT1X_SM_AUTH_DISABLED_ST,       DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_DISABLED_ST,        DOT1X_SM_AUTH_DISABLED_ST,        DOT1X_SM_AUTH_DISABLED_ST,      DOT1X_SM_AUTH_DISABLED_ST,           DOT1X_SM_AUTH_DISABLED_ST,         DOT1X_SM_AUTH_DISABLED_ST,      DOT1X_SM_AUTH_DISABLED_ST,           DOT1X_SM_AUTH_DISABLED_ST,           DOT1X_SM_AUTH_DISABLED_ST,         DOT1X_SM_AUTH_DISABLED_ST,         DOT1X_SM_AUTH_DISABLED_ST,       DOT1X_SM_AUTH_DISABLED_ST,       DOT1X_SM_AUTH_DISABLED_ST,      DOT1X_SM_AUTH_DISABLED_ST,         DOT1X_SM_AUTH_DISABLED_ST,      DOT1X_SM_AUTH_DISABLED_ST,            DOT1X_SM_AUTH_DISABLED_ST,          },
    /* DOT1X_SM_AUTH_PORT_DOWN_ST     */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_UP_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_ST,          DOT1X_SM_AUTH_PORT_DOWN_ST,        DOT1X_SM_AUTH_PORT_DOWN_ST,     DOT1X_SM_AUTH_PORT_DOWN_ST,          DOT1X_SM_AUTH_PORT_DOWN_ST,          DOT1X_SM_AUTH_PORT_DOWN_ST,        DOT1X_SM_AUTH_PORT_DOWN_ST,        DOT1X_SM_AUTH_PORT_DOWN_ST,      DOT1X_SM_AUTH_PORT_DOWN_ST,      DOT1X_SM_AUTH_PORT_DOWN_ST,     DOT1X_SM_AUTH_PORT_DOWN_ST,        DOT1X_SM_AUTH_PORT_DOWN_ST,     DOT1X_SM_AUTH_PORT_DOWN_ST,           DOT1X_SM_AUTH_PORT_DOWN_ST,         },
    /* DOT1X_SM_AUTH_AUTO_ST          */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_AUTO_ST,          DOT1X_SM_AUTH_AUTO_BEGIN_ST,         DOT1X_SM_AUTH_AUTO_BEGIN_ST,       DOT1X_SM_AUTH_AUTO_ST,          DOT1X_SM_AUTH_AUTO_ST,               DOT1X_SM_AUTH_AUTO_ST,               DOT1X_SM_AUTH_AUTO_ST,             DOT1X_SM_AUTH_AUTO_ST,             DOT1X_SM_AUTH_AUTO_ST,           DOT1X_SM_AUTH_AUTO_ST,           DOT1X_SM_AUTH_AUTO_ST,          DOT1X_SM_AUTH_AUTO_ST,             DOT1X_SM_AUTH_AUTO_ST,          DOT1X_SM_AUTH_AUTO_ST,                DOT1X_SM_AUTH_AUTO_ST,              },
    /* DOT1X_SM_AUTH_FORCE_AUTH_ST    */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_ST,    DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_ST,       DOT1X_SM_AUTH_AUTO_BEGIN_ST,    DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST,   DOT1X_SM_AUTH_FORCE_AUTH_ST,         DOT1X_SM_AUTH_FORCE_AUTH_ST,       DOT1X_SM_AUTH_FORCE_AUTH_ST,       DOT1X_SM_AUTH_FORCE_AUTH_ST,     DOT1X_SM_AUTH_FORCE_AUTH_ST,     DOT1X_SM_AUTH_FORCE_AUTH_ST,    DOT1X_SM_AUTH_FORCE_AUTH_ST,       DOT1X_SM_AUTH_FORCE_AUTH_ST,    DOT1X_SM_AUTH_FORCE_AUTH_ST,          DOT1X_SM_AUTH_FORCE_AUTH_ST,        },
    /* DOT1X_SM_AUTH_FORCE_UNAUTH_ST  */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_FORCE_UNAUTH_ST,  DOT1X_SM_AUTH_FORCE_UNAUTH_ST,       DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST, DOT1X_SM_AUTH_AUTO_BEGIN_ST,    DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_UNAUTH_ST,       DOT1X_SM_AUTH_FORCE_UNAUTH_ST,     DOT1X_SM_AUTH_FORCE_UNAUTH_ST,     DOT1X_SM_AUTH_FORCE_UNAUTH_ST,   DOT1X_SM_AUTH_FORCE_UNAUTH_ST,   DOT1X_SM_AUTH_FORCE_UNAUTH_ST,  DOT1X_SM_AUTH_FORCE_UNAUTH_ST,     DOT1X_SM_AUTH_FORCE_UNAUTH_ST,  DOT1X_SM_AUTH_FORCE_UNAUTH_ST,        DOT1X_SM_AUTH_FORCE_UNAUTH_ST,      },
    /* DOT1X_SM_AUTH_CONNECTING_ST    */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_CONNECTING_ST,    DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST, DOT1X_SM_AUTH_CONNECTING_ST,    DOT1X_SM_AUTH_CONNECTING_BEGIN_ST,   DOT1X_SM_AUTH_DISCONNECTED_BEGIN_ST, DOT1X_SM_AUTH_CONNECTING_BEGIN_ST, DOT1X_SM_AUTH_CONNECTING_BEGIN_ST, DOT1X_SM_AUTH_RESPONSE_BEGIN_ST, DOT1X_SM_AUTH_CONNECTING_ST,     DOT1X_SM_AUTH_CONNECTING_ST,    DOT1X_SM_AUTH_CONNECTING_ST,       DOT1X_SM_AUTH_CONNECTING_ST,    DOT1X_SM_AUTH_CONNECTING_ST,          DOT1X_SM_AUTH_CONNECTING_ST,        },
    /* DOT1X_SM_AUTH_DISCONNECTED_ST  */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_DISCONNECTED_ST,  DOT1X_SM_AUTH_DISCONNECTED_ST,       DOT1X_SM_AUTH_DISCONNECTED_ST,     DOT1X_SM_AUTH_DISCONNECTED_ST,  DOT1X_SM_AUTH_DISCONNECTED_ST,       DOT1X_SM_AUTH_DISCONNECTED_ST,       DOT1X_SM_AUTH_DISCONNECTED_ST,     DOT1X_SM_AUTH_DISCONNECTED_ST,     DOT1X_SM_AUTH_DISCONNECTED_ST,   DOT1X_SM_AUTH_DISCONNECTED_ST,   DOT1X_SM_AUTH_DISCONNECTED_ST,  DOT1X_SM_AUTH_DISCONNECTED_ST,     DOT1X_SM_AUTH_DISCONNECTED_ST,  DOT1X_SM_AUTH_DISCONNECTED_ST,        DOT1X_SM_AUTH_DISCONNECTED_ST,      },
    /* DOT1X_SM_AUTH_PROTO_UNAWARE_ST */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PROTO_UNAWARE_ST, DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST, DOT1X_SM_AUTH_PROTO_UNAWARE_ST, DOT1X_SM_AUTH_PROTO_UNAWARE_ST,      DOT1X_SM_AUTH_PROTO_UNAWARE_ST,      DOT1X_SM_AUTH_PROTO_UNAWARE_ST,    DOT1X_SM_AUTH_PROTO_UNAWARE_ST,    DOT1X_SM_AUTH_PROTO_UNAWARE_ST,  DOT1X_SM_AUTH_PROTO_UNAWARE_ST,  DOT1X_SM_AUTH_PROTO_UNAWARE_ST, DOT1X_SM_AUTH_PROTO_UNAWARE_ST,    DOT1X_SM_AUTH_PROTO_UNAWARE_ST, DOT1X_SM_AUTH_AUTHENTICATED_BEGIN_ST, DOT1X_SM_AUTH_DISCONNECTED_BEGIN_ST,},
    /* DOT1X_SM_AUTH_AUTHENTICATED_ST */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_AUTHENTICATED_ST, DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST, DOT1X_SM_AUTH_AUTHENTICATED_ST, DOT1X_SM_AUTH_CONNECTING_BEGIN_ST,   DOT1X_SM_AUTH_DISCONNECTED_BEGIN_ST, DOT1X_SM_AUTH_AUTHENTICATED_ST,    DOT1X_SM_AUTH_CONNECTING_BEGIN_ST, DOT1X_SM_AUTH_AUTHENTICATED_ST,  DOT1X_SM_AUTH_AUTHENTICATED_ST,  DOT1X_SM_AUTH_AUTHENTICATED_ST, DOT1X_SM_AUTH_AUTHENTICATED_ST,    DOT1X_SM_AUTH_AUTHENTICATED_ST, DOT1X_SM_AUTH_AUTHENTICATED_ST,       DOT1X_SM_AUTH_AUTHENTICATED_ST,     },
    /* DOT1X_SM_AUTH_HELD_ST          */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_HELD_ST,          DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST, DOT1X_SM_AUTH_HELD_ST,          DOT1X_SM_AUTH_HELD_ST,               DOT1X_SM_AUTH_HELD_ST,               DOT1X_SM_AUTH_CONNECTING_BEGIN_ST, DOT1X_SM_AUTH_HELD_ST,             DOT1X_SM_AUTH_HELD_ST,           DOT1X_SM_AUTH_HELD_ST,           DOT1X_SM_AUTH_HELD_ST,          DOT1X_SM_AUTH_HELD_ST,             DOT1X_SM_AUTH_HELD_ST,          DOT1X_SM_AUTH_HELD_ST,                DOT1X_SM_AUTH_HELD_ST,              },
    /* DOT1X_SM_AUTH_ABORTING_ST      */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_ABORTING_ST,      DOT1X_SM_AUTH_ABORTING_ST,           DOT1X_SM_AUTH_ABORTING_ST,         DOT1X_SM_AUTH_ABORTING_ST,      DOT1X_SM_AUTH_ABORTING_ST,           DOT1X_SM_AUTH_ABORTING_ST,           DOT1X_SM_AUTH_ABORTING_ST,         DOT1X_SM_AUTH_ABORTING_ST,         DOT1X_SM_AUTH_ABORTING_ST,       DOT1X_SM_AUTH_ABORTING_ST,       DOT1X_SM_AUTH_ABORTING_ST,      DOT1X_SM_AUTH_ABORTING_ST,         DOT1X_SM_AUTH_ABORTING_ST,      DOT1X_SM_AUTH_ABORTING_ST,            DOT1X_SM_AUTH_ABORTING_ST,          },
    /* DOT1X_SM_AUTH_LOGOFF_ST        */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_LOGOFF_ST,        DOT1X_SM_AUTH_LOGOFF_ST,             DOT1X_SM_AUTH_LOGOFF_ST,           DOT1X_SM_AUTH_LOGOFF_ST,        DOT1X_SM_AUTH_LOGOFF_ST,             DOT1X_SM_AUTH_LOGOFF_ST,             DOT1X_SM_AUTH_LOGOFF_ST,           DOT1X_SM_AUTH_LOGOFF_ST,           DOT1X_SM_AUTH_LOGOFF_ST,         DOT1X_SM_AUTH_LOGOFF_ST,         DOT1X_SM_AUTH_LOGOFF_ST,        DOT1X_SM_AUTH_LOGOFF_ST,           DOT1X_SM_AUTH_LOGOFF_ST,        DOT1X_SM_AUTH_LOGOFF_ST,              DOT1X_SM_AUTH_LOGOFF_ST,            },
    /* DOT1X_SM_AUTH_REQUEST_ST       */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_REQUEST_ST,       DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST, DOT1X_SM_AUTH_REQUEST_ST,       DOT1X_SM_AUTH_ABORTING_BEGIN_ST,     DOT1X_SM_AUTH_LOGOFF_BEGIN_ST,       DOT1X_SM_AUTH_REQUEST_BEGIN_ST,    DOT1X_SM_AUTH_ABORTING_BEGIN_ST,   DOT1X_SM_AUTH_REQUEST_ST,        DOT1X_SM_AUTH_RESPONSE_BEGIN_ST, DOT1X_SM_AUTH_REQUEST_ST,       DOT1X_SM_AUTH_REQUEST_ST,          DOT1X_SM_AUTH_REQUEST_ST,       DOT1X_SM_AUTH_REQUEST_ST,             DOT1X_SM_AUTH_REQUEST_ST,           },
    /* DOT1X_SM_AUTH_RESPONSE_ST      */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_RESPONSE_ST,      DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST, DOT1X_SM_AUTH_RESPONSE_ST,      DOT1X_SM_AUTH_ABORTING_BEGIN_ST,     DOT1X_SM_AUTH_LOGOFF_BEGIN_ST,       DOT1X_SM_AUTH_TIMEOUT_BEGIN_ST,    DOT1X_SM_AUTH_ABORTING_BEGIN_ST,   DOT1X_SM_AUTH_RESPONSE_ST,       DOT1X_SM_AUTH_RESPONSE_ST,       DOT1X_SM_AUTH_REQUEST_BEGIN_ST, DOT1X_SM_AUTH_EAPSUCCESS_BEGIN_ST, DOT1X_SM_AUTH_EAPFAIL_BEGIN_ST, DOT1X_SM_AUTH_RESPONSE_ST,            DOT1X_SM_AUTH_RESPONSE_ST,          },
    /* DOT1X_SM_AUTH_EAPSUCCESS_ST    */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_EAPSUCCESS_ST,    DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST, DOT1X_SM_AUTH_EAPSUCCESS_ST,    DOT1X_SM_AUTH_EAPSUCCESS_ST,         DOT1X_SM_AUTH_EAPSUCCESS_ST,         DOT1X_SM_AUTH_EAPSUCCESS_ST,       DOT1X_SM_AUTH_EAPSUCCESS_ST,       DOT1X_SM_AUTH_EAPSUCCESS_ST,     DOT1X_SM_AUTH_EAPSUCCESS_ST,     DOT1X_SM_AUTH_EAPSUCCESS_ST,    DOT1X_SM_AUTH_EAPSUCCESS_ST,       DOT1X_SM_AUTH_EAPSUCCESS_ST,    DOT1X_SM_AUTH_SUCCESS_BEGIN_ST,       DOT1X_SM_AUTH_ABORTING_BEGIN_ST,    },
    /* DOT1X_SM_AUTH_EAPFAIL_ST       */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_EAPFAIL_ST,       DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST, DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST, DOT1X_SM_AUTH_EAPFAIL_ST,       DOT1X_SM_AUTH_EAPFAIL_ST,            DOT1X_SM_AUTH_EAPFAIL_ST,            DOT1X_SM_AUTH_EAPFAIL_ST,          DOT1X_SM_AUTH_EAPFAIL_ST,          DOT1X_SM_AUTH_EAPFAIL_ST,        DOT1X_SM_AUTH_EAPFAIL_ST,        DOT1X_SM_AUTH_EAPFAIL_ST,       DOT1X_SM_AUTH_EAPFAIL_ST,          DOT1X_SM_AUTH_EAPFAIL_ST,       DOT1X_SM_AUTH_FAIL_BEGIN_ST,          DOT1X_SM_AUTH_ABORTING_BEGIN_ST,    },
    /* DOT1X_SM_AUTH_SUCCESS_ST       */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_SUCCESS_ST,       DOT1X_SM_AUTH_SUCCESS_ST,            DOT1X_SM_AUTH_SUCCESS_ST,          DOT1X_SM_AUTH_SUCCESS_ST,       DOT1X_SM_AUTH_SUCCESS_ST,            DOT1X_SM_AUTH_SUCCESS_ST,            DOT1X_SM_AUTH_SUCCESS_ST,          DOT1X_SM_AUTH_SUCCESS_ST,          DOT1X_SM_AUTH_SUCCESS_ST,        DOT1X_SM_AUTH_SUCCESS_ST,        DOT1X_SM_AUTH_SUCCESS_ST,       DOT1X_SM_AUTH_SUCCESS_ST,          DOT1X_SM_AUTH_SUCCESS_ST,       DOT1X_SM_AUTH_SUCCESS_ST,             DOT1X_SM_AUTH_SUCCESS_ST,           },
    /* DOT1X_SM_AUTH_FAIL_ST          */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_FAIL_ST,          DOT1X_SM_AUTH_FAIL_ST,               DOT1X_SM_AUTH_FAIL_ST,             DOT1X_SM_AUTH_FAIL_ST,          DOT1X_SM_AUTH_FAIL_ST,               DOT1X_SM_AUTH_FAIL_ST,               DOT1X_SM_AUTH_FAIL_ST,             DOT1X_SM_AUTH_FAIL_ST,             DOT1X_SM_AUTH_FAIL_ST,           DOT1X_SM_AUTH_FAIL_ST,           DOT1X_SM_AUTH_FAIL_ST,          DOT1X_SM_AUTH_FAIL_ST,             DOT1X_SM_AUTH_FAIL_ST,          DOT1X_SM_AUTH_FAIL_ST,                DOT1X_SM_AUTH_FAIL_ST,              },
    /* DOT1X_SM_AUTH_TIMEOUT_ST       */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_TIMEOUT_ST,       DOT1X_SM_AUTH_TIMEOUT_ST,            DOT1X_SM_AUTH_TIMEOUT_ST,          DOT1X_SM_AUTH_TIMEOUT_ST,       DOT1X_SM_AUTH_TIMEOUT_ST,            DOT1X_SM_AUTH_TIMEOUT_ST,            DOT1X_SM_AUTH_TIMEOUT_ST,          DOT1X_SM_AUTH_TIMEOUT_ST,          DOT1X_SM_AUTH_TIMEOUT_ST,        DOT1X_SM_AUTH_TIMEOUT_ST,        DOT1X_SM_AUTH_TIMEOUT_ST,       DOT1X_SM_AUTH_TIMEOUT_ST,          DOT1X_SM_AUTH_TIMEOUT_ST,       DOT1X_SM_AUTH_TIMEOUT_ST,             DOT1X_SM_AUTH_TIMEOUT_ST,           },
    /* DOT1X_SM_AUTH_INIT_ST          */ {DOT1X_SM_AUTH_DISABLED_BEGIN_ST, DOT1X_SM_AUTH_INIT_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST, DOT1X_SM_AUTH_INIT_ST,          DOT1X_SM_AUTH_INIT_ST,               DOT1X_SM_AUTH_INIT_ST,             DOT1X_SM_AUTH_INIT_ST,          DOT1X_SM_AUTH_INIT_ST,               DOT1X_SM_AUTH_INIT_ST,               DOT1X_SM_AUTH_INIT_ST,             DOT1X_SM_AUTH_INIT_ST,             DOT1X_SM_AUTH_INIT_ST,           DOT1X_SM_AUTH_INIT_ST,           DOT1X_SM_AUTH_INIT_ST,          DOT1X_SM_AUTH_INIT_ST,             DOT1X_SM_AUTH_INIT_ST,          DOT1X_SM_AUTH_INIT_ST,                DOT1X_SM_AUTH_INIT_ST,              },
};

static UI8_T pae_group_address[SYS_ADPT_MAC_ADDR_LEN] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};

const char *state_string_ar[] =
{
    DOT1X_SM_AUTH_STATE_STR_LIST_DEFINITION
};

const char *event_string_ar[] =
{
    DOT1X_SM_AUTH_EVENT_STR_LIST_DEFINITION
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T DOT1X_SM_AUTH_IsAccessPort(UI32_T lport);
static BOOL_T DOT1X_SM_AUTH_IsNeedJoinGuestVlan(DOT1X_SM_AUTH_Obj_T *sm_p);
static BOOL_T DOT1X_SM_AUTH_IsAuthFailVlanReady(DOT1X_SM_AUTH_Obj_T *sm_p);
static void DOT1X_SM_AUTH_InitEapolHeader(DOT1X_SM_AUTH_Obj_T *sm_p, UI32_T lport);
static void DOT1X_SM_AUTH_InitAuthTables(DOT1X_SM_AUTH_Obj_T *sm_p, UI32_T lport);
static void DOT1X_SM_AUTH_InitAuthStats(UI32_T lport);

#if (SYS_CPNT_ACCOUNTING == TRUE)
static void DOT1X_SM_AUTH_SendAccountingRequestToServer(DOT1X_SM_AUTH_Obj_T *sm_p, AAA_AccRequestType_T request_type, AAA_Authentic_T auth_by_whom, AAA_AccTerminateCause_T terminate_cause);
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */

static BOOL_T DOT1X_SM_AUTH_SendEapSuccToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p);
static BOOL_T DOT1X_SM_AUTH_SendEapFailToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p);
static BOOL_T DOT1X_SM_AUTH_SendEapSuccOrFailToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p, BOOL_T is_succ);
static BOOL_T DOT1X_SM_AUTH_SendEapRequestIdentityToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p);
static BOOL_T DOT1X_SM_AUTH_SendEapRequestChallengeToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p);
static BOOL_T DOT1X_SM_AUTH_SendEapPacketToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p, L_MM_Mref_Handle_T *mem_ref_p);
static BOOL_T DOT1X_SM_AUTH_SendRequestToAuthServer(DOT1X_SM_AUTH_Obj_T *sm_p);
static BOOL_T DOT1X_SM_AUTH_CancelRequestToAuthServer(DOT1X_SM_AUTH_Obj_T *sm_p);
static BOOL_T DOT1X_SM_AUTH_AnnounceAuthorizedResult(UI32_T lport, UI8_T *port_mac, int eap_identifier, UI8_T auth_result, char *authorized_vlan_list, char *authorized_qos_list, UI32_T session_time, UI32_T server_ip);
static void DOT1X_SM_AUTH_UpdateSessionId(DOT1X_SM_AUTH_Obj_T *sm_p);
static void DOT1X_SM_AUTH_StartTimeoutTimer(DOT1X_SM_AUTH_Obj_T *sm_p, I32_T seconds);
static void DOT1X_SM_AUTH_StopTimeoutTimer(DOT1X_SM_AUTH_Obj_T *sm_p);
static void DOT1X_SM_AUTH_StartReauthTimer(DOT1X_SM_AUTH_Obj_T *sm_p);
static void DOT1X_SM_AUTH_StopReauthTimer(DOT1X_SM_AUTH_Obj_T *sm_p);

/* EXPORTED SUBPROGRAM BODIES
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
BOOL_T DOT1X_SM_AUTH_Init(DOT1X_SM_AUTH_Obj_T *sm_p, UI32_T lport)
{
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];

    /* On MAC based mode, sm_p->dot1x_packet_t.src_mac is a key and
     * be used as the dst MAC address in EAP packet which send to
     * suppliant. Here we should not clean it.
     */
    memcpy(src_mac, sm_p->dot1x_packet_t.src_mac, sizeof(src_mac));

    memset(sm_p, 0, sizeof(DOT1X_SM_AUTH_Obj_T));

    sm_p->current_state = DOT1X_SM_AUTH_DISABLED_ST;
    sm_p->port_status = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;
    sm_p->port_mode = DOT1X_DEFAULT_PORT_MODE;
    sm_p->current_id = 0;
    sm_p->reauth_count = 0;
    sm_p->request_count = 0;
    sm_p->id_from_server = 0;
    sm_p->session_start_time = 0;

    sm_p->is_auth_succ_before = FALSE;
    sm_p->is_a_mac_authenticating = FALSE;
    sm_p->is_first_mac_auth = FALSE;
    sm_p->is_ever_pass_auth = FALSE;
    sm_p->is_eapol_rx       = FALSE;
    sm_p->is_applying_guest_vlan = FALSE;
    sm_p->is_applied_guest_vlan  = FALSE;

    DOT1X_SM_AUTH_InitEapolHeader(sm_p, lport);

    memset(&sm_p->radius_packet_t, 0, sizeof(sm_p->radius_packet_t));

    DOT1X_SM_AUTH_InitAuthTables(sm_p, lport);

    memcpy(sm_p->dot1x_packet_t.src_mac, src_mac, sizeof(src_mac));
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_Go
 * ---------------------------------------------------------------------
 * PURPOSE : Run state machine
 * INPUT   : sm_p - pointer to the state machine object
 *           ev - received event type
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
void DOT1X_SM_AUTH_Go(DOT1X_SM_AUTH_Obj_T *sm_p, DOT1X_SM_AUTH_Event_T ev)
{
    DOT1X_SM_AUTH_State_T original_state = sm_p->current_state;
    DOT1X_SM_AUTH_State_T next_state = fsm[sm_p->current_state][ev];
    DOT1X_SM_AUTH_State_T running_state = next_state;
    UI32_T lport = sm_p->dot1x_packet_t.src_lport;

    /* if this event is unexpected, do nothing
     */
    if (next_state == sm_p->current_state)
    {
        lib1x_message2(MESS_DBG_AUTHSM, "State %s: Unexpected event %s, do nothing", state_string_ar[sm_p->current_state], event_string_ar[ev]);
        return;
    }

    lib1x_message3(MESS_DBG_AUTHSM, "From %s to %s (event = %s)", state_string_ar[sm_p->current_state], state_string_ar[next_state], event_string_ar[ev]);

    while (TRUE)
    {
        switch (running_state)
        {
            case DOT1X_SM_AUTH_DISABLED_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter DISABLED_BEGIN state");

                next_state = DOT1X_SM_AUTH_DISABLED_ST;
                break;

            case DOT1X_SM_AUTH_DISABLED_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter DISABLED state");

                DOT1X_SM_AUTH_StopTimeoutTimer(sm_p);
                DOT1X_SM_AUTH_StopReauthTimer(sm_p);

                if (FALSE == sm_p->is_applied_guest_vlan)
                {
#if (SYS_CPNT_ACCOUNTING == TRUE)
                    if (   (VAL_dot1xAuthAuthControlledPortControl_auto == sm_p->port_mode)
                        && (VAL_dot1xAuthAuthControlledPortStatus_authorized == sm_p->port_status) )
                    {
                        DOT1X_SM_AUTH_SendAccountingRequestToServer(sm_p,
                            AAA_ACC_STOP,
                            AAA_AUTHEN_BY_UNKNOWN /* don't care */,
                            AAA_ACC_TERM_BY_NAS_REQUEST);
                    }
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
                }
                /* For 802.1x capability device and it does not finish the
                 * authentication yet. Send it out an EAPOL/success to tell
                 * it to finish the authentication. Because the 802.1X
                 * function now is turn off.
                 */
                if (   (TRUE == sm_p->is_eapol_rx)
                    && (VAL_dot1xAuthAuthControlledPortControl_auto == sm_p->port_mode)
                    && (VAL_dot1xAuthAuthControlledPortStatus_authorized != sm_p->port_status))
                {
                    /* If go this stae from DISCONNECTED on MAC based mode, the
                     * sm_p->dot1x_packet_t.src_mac will be NULL and should not
                     * be used as DA in EAPOL/success.
                     * DOT1X_SM_AUTH_SendEapSuccToSupplicant should check this
                     * case.
                     */
                    DOT1X_SM_AUTH_SendEapSuccToSupplicant(sm_p);
                }

                /* Recycle the state machine object
                 */
                sm_p->is_first_mac_auth = FALSE;
                memset(sm_p->dot1x_packet_t.src_mac, 0, SYS_ADPT_MAC_ADDR_LEN);
                break;

            case DOT1X_SM_AUTH_INIT_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter INIT state");

                DOT1X_SM_AUTH_Init(sm_p, lport);

                next_state = (TRUE == DOT1X_OM_GetPortEnabled(lport)) ? DOT1X_SM_AUTH_PORT_UP_BEGIN_ST : DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST;
                break;

            case DOT1X_SM_AUTH_INIT_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Cannot to reach here");
                break;

            case DOT1X_SM_AUTH_PORT_DOWN_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter PORT_DOWN_BEGIN state");
                next_state = DOT1X_SM_AUTH_PORT_DOWN_ST;
                break;

            case DOT1X_SM_AUTH_PORT_DOWN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter PORT_DOWN state");

                DOT1X_SM_AUTH_StopTimeoutTimer(sm_p);
                DOT1X_SM_AUTH_StopReauthTimer(sm_p);

                if (FALSE == sm_p->is_applied_guest_vlan)
                {
#if (SYS_CPNT_ACCOUNTING == TRUE)
                    /* success before but link-down now
                     */
                    if (   (VAL_dot1xAuthAuthControlledPortControl_auto == sm_p->port_mode)
                        && (VAL_dot1xAuthAuthControlledPortStatus_authorized == sm_p->port_status) )
                    {
                        DOT1X_SM_AUTH_SendAccountingRequestToServer(sm_p,
                            AAA_ACC_STOP,
                            AAA_AUTHEN_BY_UNKNOWN /* don't care */,
                            (DOT1X_SM_AUTH_PORT_ADMIN_DOWN_EV == ev) ? AAA_ACC_TERM_BY_ADMIN_RESET : AAA_ACC_TERM_BY_USER_REQUEST);
                    }
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
                }
                memset(sm_p->dot1x_packet_t.src_mac, 0, sizeof(sm_p->dot1x_packet_t.src_mac));
                sm_p->port_status = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;

                sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause = (DOT1X_SM_AUTH_PORT_ADMIN_DOWN_EV == ev) ? VAL_dot1xAuthSessionTerminateCause_portAdminDisabled : VAL_dot1xAuthSessionTerminateCause_portFailure;

                break;

            case DOT1X_SM_AUTH_PORT_UP_BEGIN_ST:
            {
                UI32_T port_mode;

                lib1x_message(MESS_DBG_AUTHSM, "Enter PORT_UP_BEGIN state");

                DOT1X_SM_AUTH_Init(sm_p, lport);
                port_mode = DOT1X_OM_Get_PortControlMode(lport);

                switch (port_mode)
                {
                    case VAL_dot1xAuthAuthControlledPortControl_auto:
                        next_state = DOT1X_SM_AUTH_AUTO_BEGIN_ST;
                        break;

                    case VAL_dot1xAuthAuthControlledPortControl_forceAuthorized:
                        next_state = DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST;
                        break;

                    case VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized:
                        next_state = DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST;
                        break;

                    default:
                        lib1x_message1(MESS_DBG_AUTHSM, "Unknown port mode? (port mode = %lu)", port_mode);
                        break;
                }
            }
                break;

            case DOT1X_SM_AUTH_AUTO_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter AUTO_BEGIN state");
                next_state = DOT1X_SM_AUTH_AUTO_ST;
                break;

            case DOT1X_SM_AUTH_AUTO_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter AUTO state");

                sm_p->current_id = 0;
                sm_p->port_mode = VAL_dot1xAuthAuthControlledPortControl_auto;

                if (DOT1X_OM_IsPortBasedMode(lport))
                {
                    next_state = DOT1X_SM_AUTH_DISCONNECTED_BEGIN_ST;
                }
                else
                {
                    next_state = DOT1X_SM_AUTH_PRE_CONNECTING_BEGIN_ST;
                }
                break;

            case DOT1X_SM_AUTH_FORCE_AUTH_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter FORCE_AUTH_BEGIN state");
                next_state = DOT1X_SM_AUTH_FORCE_AUTH_ST;
                break;

            case DOT1X_SM_AUTH_FORCE_AUTH_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter FORCE_AUTH state");

                DOT1X_SM_AUTH_StopTimeoutTimer(sm_p);
                DOT1X_SM_AUTH_StopReauthTimer(sm_p);
                if (FALSE == sm_p->is_applied_guest_vlan)
                {
#if (SYS_CPNT_ACCOUNTING == TRUE)
                    /* success before but force-auth now
                     */
                    if (   (VAL_dot1xAuthAuthControlledPortControl_auto == sm_p->port_mode)
                        && (VAL_dot1xAuthAuthControlledPortStatus_authorized == sm_p->port_status) )
                    {
                        DOT1X_SM_AUTH_SendAccountingRequestToServer(sm_p,
                            AAA_ACC_STOP,
                            AAA_AUTHEN_BY_UNKNOWN /* don't care */,
                            AAA_ACC_TERM_BY_NAS_REQUEST);
                    }
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
                }
                sm_p->port_status = VAL_dot1xAuthAuthControlledPortStatus_authorized;
                sm_p->port_mode = VAL_dot1xAuthAuthControlledPortControl_forceAuthorized;

                /* SYSFUN_Sleep(3); */

                DOT1X_SM_AUTH_SendEapSuccToSupplicant(sm_p);

                INC(sm_p->current_id);
                break;

            case DOT1X_SM_AUTH_FORCE_UNAUTH_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter FORCE_UNAUTH_BEGIN state");
                next_state = DOT1X_SM_AUTH_FORCE_UNAUTH_ST;
                break;

            case DOT1X_SM_AUTH_FORCE_UNAUTH_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter FORCE_UNAUTH state");

                DOT1X_SM_AUTH_StopTimeoutTimer(sm_p);
                DOT1X_SM_AUTH_StopReauthTimer(sm_p);
                if (FALSE == sm_p->is_applied_guest_vlan)
                {
#if (SYS_CPNT_ACCOUNTING == TRUE)
                    /* success before but force-unauth now
                     */
                    if (   (VAL_dot1xAuthAuthControlledPortControl_auto == sm_p->port_mode)
                        && (VAL_dot1xAuthAuthControlledPortStatus_authorized == sm_p->port_status) )
                    {
                        DOT1X_SM_AUTH_SendAccountingRequestToServer(sm_p,
                            AAA_ACC_STOP,
                            AAA_AUTHEN_BY_UNKNOWN /* don't care */,
                            AAA_ACC_TERM_BY_NAS_REQUEST);
                    }
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
                }
                sm_p->port_status = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;
                sm_p->port_mode = VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized;

                /* SYSFUN_Sleep(3); */

                DOT1X_SM_AUTH_SendEapFailToSupplicant(sm_p);

                INC(sm_p->current_id);

                sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause = VAL_dot1xAuthSessionTerminateCause_authControlForceUnauth;
                break;

            case DOT1X_SM_AUTH_DISCONNECTED_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter DISCONNECTED_BEGIN state");
                next_state = DOT1X_SM_AUTH_DISCONNECTED_ST;
                break;

            case DOT1X_SM_AUTH_DISCONNECTED_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter DISCONNECTED state");

                DOT1X_SM_AUTH_SendEapFailToSupplicant(sm_p);

                if (   (DOT1X_SM_AUTH_AUTO_EV != ev)
                    && (VAL_dot1xAuthAuthControlledPortStatus_authorized == sm_p->port_status)
                    )
                {
                    SWDRV_IfTableStats_T if_table_stats;
                    UI32_T current_time;

                    if (FALSE == sm_p->is_applied_guest_vlan)
                    {
#if (SYS_CPNT_ACCOUNTING == TRUE)
                        /* success before but logoff now
                         */
                        DOT1X_SM_AUTH_SendAccountingRequestToServer(sm_p,
                            AAA_ACC_STOP,
                            AAA_AUTHEN_BY_UNKNOWN /* don't care */,
                            AAA_ACC_TERM_BY_USER_REQUEST);
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
                    }
#if (SYS_CPNT_NETACCESS == TRUE)
                    DOT1X_SM_AUTH_AnnounceAuthorizedResult(sm_p->dot1x_packet_t.src_lport, sm_p->dot1x_packet_t.src_mac,
                        sm_p->current_id, DOT1X_TYPE_AUTH_RESULT_LOGOFF,
                        sm_p->radius_packet_t.authorized_vlan_list,
                        sm_p->radius_packet_t.authorized_qos_list,
                        sm_p->radius_packet_t.authorized_session_time,
                        sm_p->radius_packet_t.server_ip);
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */

                    current_time = SYS_TIME_GetSystemTicksBy10ms();
                    sm_p->auth_session_stats_entry.dot1xAuthSessionTime = current_time - sm_p->session_start_time;

                    NMTR_MGR_GetIfTableStats(sm_p->dot1x_packet_t.src_lport, &if_table_stats);
                    sm_p->if_table_stats.ifInOctets = if_table_stats.ifInOctets - sm_p->if_table_stats.ifInOctets;
                    sm_p->if_table_stats.ifOutOctets = if_table_stats.ifOutOctets - sm_p->if_table_stats.ifOutOctets;
                    sm_p->if_table_stats.ifInUcastPkts = if_table_stats.ifInUcastPkts - sm_p->if_table_stats.ifInUcastPkts;
                    sm_p->if_table_stats.ifOutUcastPkts = if_table_stats.ifOutUcastPkts - sm_p->if_table_stats.ifOutUcastPkts;
                    sm_p->if_table_stats.ifInNUcastPkts = if_table_stats.ifInNUcastPkts - sm_p->if_table_stats.ifInNUcastPkts;
                    sm_p->if_table_stats.ifOutNUcastPkts = if_table_stats.ifOutNUcastPkts - sm_p->if_table_stats.ifOutNUcastPkts;
                }

                if (DOT1X_SM_AUTH_EAPLOGOFF_EV == ev)
                {
                    if (DOT1X_SM_AUTH_CONNECTING_ST == original_state)
                    {
                        sm_p->auth_diag_entry.dot1xAuthEapLogoffsWhileConnecting++;
                    }
                    else if (   (DOT1X_SM_AUTH_AUTHENTICATED_ST == original_state)
                             && (DOT1X_SM_AUTH_EAPLOGOFF_EV == ev) )
                    {
                        sm_p->auth_diag_entry.dot1xAuthAuthEapLogoffWhileAuthenticated++;
                        sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause = VAL_dot1xAuthSessionTerminateCause_supplicantLogoff;
                    }
                }

                INC(sm_p->current_id);

                sm_p->is_first_mac_auth = FALSE;
                sm_p->is_applying_guest_vlan = FALSE;
                sm_p->is_applied_guest_vlan = FALSE;
                memset(sm_p->dot1x_packet_t.src_mac, 0, SYS_ADPT_MAC_ADDR_LEN);
                sm_p->port_status = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;
                sm_p->reauth_count = 0;

                if (DOT1X_OM_IsPortBasedMode(lport))
                {
                    next_state = DOT1X_SM_AUTH_CONNECTING_BEGIN_ST;
                }
                else
                {
                    next_state = DOT1X_SM_AUTH_DISABLED_BEGIN_ST;
                }
                break;

            case DOT1X_SM_AUTH_PRE_CONNECTING_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter PRE_CONNECTING_MAC_BASED_BEGIN state");

                sm_p->is_first_mac_auth = FALSE;
                sm_p->port_status = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;
                sm_p->reauth_count = 0;
                INC(sm_p->current_id);

                next_state = DOT1X_SM_AUTH_CONNECTING_BEGIN_ST;
                break;

            case DOT1X_SM_AUTH_CONNECTING_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter CONNECTING_BEGIN state");

                if (sm_p->reauth_count >= DOT1X_OM_Get_PortReAuthMax(lport))
                {
                    next_state = DOT1X_SM_AUTH_PROTO_UNAWARE_BEGIN_ST;
                }
                else
                {
                    next_state = DOT1X_SM_AUTH_CONNECTING_ST;
                }
                break;

            case DOT1X_SM_AUTH_CONNECTING_ST:
            {
                UI32_T tx_period;

                lib1x_message(MESS_DBG_AUTHSM, "Enter CONNECTING state");

                tx_period = DOT1X_OM_Get_PortTxPeriod(lport);
                DOT1X_SM_AUTH_StartTimeoutTimer(sm_p, tx_period);

                DOT1X_SM_AUTH_StartReauthTimer(sm_p);

                sm_p->rinfo.rad_stateavailable = FALSE;
                sm_p->rinfo.rad_statelength = 0;

                DOT1X_SM_AUTH_SendEapRequestIdentityToSupplicant(sm_p);

                INC(sm_p->reauth_count);

                sm_p->auth_diag_entry.dot1xAuthEntersConnecting++;
                /* reset the session
                 */
                if (DOT1X_SM_AUTH_EAPSTART_EV == ev)
                {
                    sm_p->auth_session_stats_entry.dot1xAuthSessionFramesRx = 1;
                    sm_p->auth_session_stats_entry.dot1xAuthSessionOctetsRx = sm_p->dot1x_packet_t.packet_length;
                    sm_p->auth_session_stats_entry.dot1xAuthSessionOctetsTx = 0;
                    sm_p->auth_session_stats_entry.dot1xAuthSessionFramesTx = 0;

                    sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause = VAL_dot1xAuthSessionTerminateCause_notTerminatedYet;
                    memset(sm_p->auth_session_stats_entry.dot1xAuthSessionUserName, '\0', sizeof(sm_p->auth_session_stats_entry.dot1xAuthSessionUserName));
                    DOT1X_SM_AUTH_UpdateSessionId(sm_p);
                }
                if (DOT1X_SM_AUTH_AUTHENTICATED_ST == original_state)
                {
                    if (DOT1X_SM_AUTH_REAUTH_EV == ev)
                    {
                        sm_p->auth_diag_entry.dot1xAuthAuthReauthsWhileAuthenticated++;
                    }
                    else if (DOT1X_SM_AUTH_EAPSTART_EV == ev)
                    {
                        sm_p->auth_diag_entry.dot1xAuthAuthEapStartsWhileAuthenticated++;
                        sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause = VAL_dot1xAuthSessionTerminateCause_supplicantRestart;
                    }
                }
            }
                break;

            case DOT1X_SM_AUTH_PROTO_UNAWARE_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter PROTO_UNAWARE_BEGIN state");

                if (   (TRUE != DOT1X_SM_AUTH_IsNeedJoinGuestVlan(sm_p))
                    || (TRUE == DOT1X_SM_AUTH_IsAccessPort(sm_p->dot1x_packet_t.src_lport))
                    || (FALSE == DOT1X_OM_IsPortBasedMode(lport))
                    )
                {
                    next_state = DOT1X_SM_AUTH_DISCONNECTED_BEGIN_ST;
                }
                else
                {
                    next_state = DOT1X_SM_AUTH_PROTO_UNAWARE_ST;
                }
                break;

            case DOT1X_SM_AUTH_PROTO_UNAWARE_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter PROTO_UNAWARE state");

                DOT1X_SM_AUTH_StopTimeoutTimer(sm_p);
                DOT1X_SM_AUTH_StopReauthTimer(sm_p);
                /*  TRUE, means the state machine is watting for applying guest VALN
                 *  If apply guest VLAN succeed, the state machine will go to authenticated state
                 */
                sm_p->is_applying_guest_vlan = TRUE;

#if (SYS_CPNT_NETACCESS == TRUE)
                DOT1X_SM_AUTH_AnnounceAuthorizedResult(sm_p->dot1x_packet_t.src_lport, sm_p->dot1x_packet_t.src_mac,
                    sm_p->current_id, DOT1X_TYPE_AUTH_RESULT_NO_EAPOL, 0, 0, 0, 0);
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
                break;

            case DOT1X_SM_AUTH_AUTHENTICATED_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter AUTHENTICATED_BEGIN state");

                if (TRUE == sm_p->is_applying_guest_vlan)
                {
                    sm_p->is_applied_guest_vlan = TRUE;
                }

                sm_p->is_applying_guest_vlan = FALSE;

                next_state = DOT1X_SM_AUTH_AUTHENTICATED_ST;
                break;

            case DOT1X_SM_AUTH_AUTHENTICATED_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter AUTHENTICATED state");

                /*  Goto this status because applied guest VLAN. In this case we not need to
                 *  send accounting packet.
                 */
                if (FALSE == sm_p->is_applied_guest_vlan)
                {

                    DOT1X_SM_AUTH_StartReauthTimer(sm_p);

#if (SYS_CPNT_ACCOUNTING == TRUE)
                    /* first authentication (not reauth)
                     */
                    if (VAL_dot1xAuthAuthControlledPortStatus_unauthorized == sm_p->port_status)
                    {
                        DOT1X_SM_AUTH_SendAccountingRequestToServer(sm_p,
                        AAA_ACC_START,
                        AAA_AUTHEN_BY_RADIUS,
                        AAA_ACC_TERM_BY_UNKNOWN /* don't care */);
                    }
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
                }
                sm_p->port_status = VAL_dot1xAuthAuthControlledPortStatus_authorized;
                sm_p->reauth_count = 0;
                INC(sm_p->current_id);

                sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause = VAL_dot1xAuthSessionTerminateCause_notTerminatedYet;
                break;

            case DOT1X_SM_AUTH_HELD_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter HELD_BEGIN state");
                next_state = DOT1X_SM_AUTH_HELD_ST;
                break;

            case DOT1X_SM_AUTH_HELD_ST:
            {
                UI32_T quietPeriod;

                lib1x_message(MESS_DBG_AUTHSM, "Enter HELD state");

                quietPeriod = DOT1X_OM_Get_PortQuietPeriod(lport);
                if (FALSE == sm_p->is_applied_guest_vlan)
                {
                    DOT1X_SM_AUTH_StartTimeoutTimer(sm_p, quietPeriod);

#if (SYS_CPNT_ACCOUNTING == TRUE)
                    /* reauthentication fail
                     */
                    if (VAL_dot1xAuthAuthControlledPortStatus_authorized == sm_p->port_status)
                    {
                        DOT1X_SM_AUTH_SendAccountingRequestToServer(sm_p,
                            AAA_ACC_STOP,
                            AAA_AUTHEN_BY_UNKNOWN /* don't care */,
                            AAA_ACC_TERM_BY_USER_ERROR);
                    }
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */
                }
                sm_p->port_status = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;
                INC(sm_p->current_id);

                sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause = VAL_dot1xAuthSessionTerminateCause_reauthFailed;
            }
                break;

            case DOT1X_SM_AUTH_ABORTING_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter ABORTING_BEGIN state");
                next_state = DOT1X_SM_AUTH_ABORTING_ST;
                break;

            case DOT1X_SM_AUTH_ABORTING_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter ABORTING state");

                INC(sm_p->current_id);

                if (   ((DOT1X_SM_AUTH_REQUEST_ST == original_state) || (DOT1X_SM_AUTH_RESPONSE_ST == original_state))
                    && (DOT1X_SM_AUTH_REAUTH_EV == ev))
                {
                    sm_p->auth_diag_entry.dot1xAuthAuthReauthsWhileAuthenticating++;
                }
                if (   ((DOT1X_SM_AUTH_REQUEST_ST == original_state) || (DOT1X_SM_AUTH_RESPONSE_ST == original_state))
                    && (DOT1X_SM_AUTH_EAPSTART_EV == ev))
                {
                    sm_p->auth_diag_entry.dot1xAuthAuthEapStartsWhileAuthenticating++;

                    if (DOT1X_SM_AUTH_RESPONSE_ST == original_state)
                    {
                        DOT1X_SM_AUTH_CancelRequestToAuthServer(sm_p);
                    }
                }

                next_state = DOT1X_SM_AUTH_CONNECTING_BEGIN_ST;
                break;

            case DOT1X_SM_AUTH_LOGOFF_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter LOGOFF_BEGIN state");
                next_state = DOT1X_SM_AUTH_LOGOFF_ST;
                break;

            case DOT1X_SM_AUTH_LOGOFF_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter LOGOFF state");

                INC(sm_p->current_id);

                if (   ((DOT1X_SM_AUTH_REQUEST_ST == original_state) || (DOT1X_SM_AUTH_RESPONSE_ST == original_state))
                    && (DOT1X_SM_AUTH_EAPLOGOFF_EV == ev))
                {
                    sm_p->auth_diag_entry.dot1xAuthAuthEapLogoffWhileAuthenticating++;

                    if (DOT1X_SM_AUTH_RESPONSE_ST == original_state)
                    {
                        DOT1X_SM_AUTH_CancelRequestToAuthServer(sm_p);
                    }
                }

                next_state = DOT1X_SM_AUTH_DISCONNECTED_BEGIN_ST;
                break;

            case DOT1X_SM_AUTH_REQUEST_BEGIN_ST:
            {
                lib1x_message(MESS_DBG_AUTHSM, "Enter REQUEST_BEGIN state");

                if (sm_p->request_count >= DOT1X_OM_Get_PortMaxReq(lport))
                {
                    next_state = DOT1X_SM_AUTH_TIMEOUT_BEGIN_ST;
                }
                else
                {
                    next_state = DOT1X_SM_AUTH_REQUEST_ST;
                }
            }
                break;

            case DOT1X_SM_AUTH_REQUEST_ST:
            {
                UI32_T supp_timeout = DOT1X_OM_Get_AuthSuppTimeout(lport);

                lib1x_message(MESS_DBG_AUTHSM, "Enter REQUEST state");

                DOT1X_SM_AUTH_StartTimeoutTimer(sm_p, supp_timeout);

                DOT1X_SM_AUTH_StartReauthTimer(sm_p);

                sm_p->current_id = sm_p->id_from_server;

                DOT1X_SM_AUTH_SendEapRequestChallengeToSupplicant(sm_p);

                INC(sm_p->request_count);
            }
                break;

            case DOT1X_SM_AUTH_TIMEOUT_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter TIMEOUT_BEGIN state");
                next_state = DOT1X_SM_AUTH_TIMEOUT_ST;
                break;

            case DOT1X_SM_AUTH_TIMEOUT_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter TIMEOUT state");

                DOT1X_SM_AUTH_SendEapFailToSupplicant(sm_p);

                sm_p->auth_diag_entry.dot1xAuthAuthTimeoutsWhileAuthenticating++;

                next_state = DOT1X_SM_AUTH_ABORTING_BEGIN_ST;
                break;

            case DOT1X_SM_AUTH_RESPONSE_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter RESPONSE_BEGIN state");
                next_state = DOT1X_SM_AUTH_RESPONSE_ST;
                break;

            case DOT1X_SM_AUTH_RESPONSE_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter RESPONSE state");

                /* In order to prevent from time inconsistent, server timeout is
                 * leave to RADIUS CSC to raise up.
                 */

                DOT1X_SM_AUTH_StartReauthTimer(sm_p);

                sm_p->request_count = 0;

                DOT1X_SM_AUTH_SendRequestToAuthServer(sm_p);

                if (   (DOT1X_SM_AUTH_CONNECTING_ST == original_state)
                    && (DOT1X_SM_AUTH_RESPID_EV == ev))
                {
                    sm_p->is_a_mac_authenticating = FALSE;
                    sm_p->auth_diag_entry.dot1xAuthEntersAuthenticating++;
                }

                sm_p->auth_diag_entry.dot1xAuthBackendResponses++;
                if(DOT1X_SM_AUTH_TIMEOUT_EV ==ev)
                {
                    next_state = DOT1X_SM_AUTH_TIMEOUT_BEGIN_ST;
                }
                break;

            case DOT1X_SM_AUTH_EAPSUCCESS_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter EAPSUCCESS_BEGIN state");
                next_state = DOT1X_SM_AUTH_EAPSUCCESS_ST;
                break;

            case DOT1X_SM_AUTH_EAPSUCCESS_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter EAPSUCCESS state");

                DOT1X_SM_AUTH_StopTimeoutTimer(sm_p);
                DOT1X_SM_AUTH_StopReauthTimer(sm_p);

#if (SYS_CPNT_NETACCESS == TRUE)
                DOT1X_SM_AUTH_AnnounceAuthorizedResult(sm_p->dot1x_packet_t.src_lport, sm_p->dot1x_packet_t.src_mac,
                    sm_p->current_id, DOT1X_TYPE_AUTH_RESULT_SUCCESS,
                    sm_p->radius_packet_t.authorized_vlan_list,
                    sm_p->radius_packet_t.authorized_qos_list,
                    sm_p->radius_packet_t.authorized_session_time,
                    sm_p->radius_packet_t.server_ip);
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
                break;

            case DOT1X_SM_AUTH_EAPFAIL_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter EAPFAIL_BEGIN state");
                next_state = DOT1X_SM_AUTH_EAPFAIL_ST;
                break;

            case DOT1X_SM_AUTH_EAPFAIL_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter EAPFAIL state");

                DOT1X_SM_AUTH_StopTimeoutTimer(sm_p);
                DOT1X_SM_AUTH_StopReauthTimer(sm_p);

#if (SYS_CPNT_NETACCESS == TRUE)
                DOT1X_SM_AUTH_AnnounceAuthorizedResult(sm_p->dot1x_packet_t.src_lport, sm_p->dot1x_packet_t.src_mac,
                    sm_p->current_id, DOT1X_TYPE_AUTH_RESULT_FAIL,
                    sm_p->radius_packet_t.authorized_vlan_list,
                    sm_p->radius_packet_t.authorized_qos_list,
                    sm_p->radius_packet_t.authorized_session_time,
                    sm_p->radius_packet_t.server_ip);
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
                break;

            case DOT1X_SM_AUTH_SUCCESS_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter SUCCESS_BEGIN state");
                next_state = DOT1X_SM_AUTH_SUCCESS_ST;
                break;

            case DOT1X_SM_AUTH_SUCCESS_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter SUCCESS state");

                sm_p->current_id = sm_p->id_from_server;
                sm_p->is_auth_succ_before = TRUE;
                sm_p->is_ever_pass_auth = TRUE;
                sm_p->session_start_time = SYS_TIME_GetSystemTicksBy10ms();


                if (FALSE == sm_p->is_first_mac_auth)
                {
                    sm_p->is_first_mac_auth = TRUE;
                }

                DOT1X_SM_AUTH_SendEapSuccToSupplicant(sm_p);

                NMTR_MGR_GetIfTableStats(sm_p->dot1x_packet_t.src_lport, &sm_p->if_table_stats);

                sm_p->auth_diag_entry.dot1xAuthAuthSuccessWhileAuthenticating++;

                next_state = DOT1X_SM_AUTH_AUTHENTICATED_BEGIN_ST;
                break;

            case DOT1X_SM_AUTH_FAIL_BEGIN_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter FAIL_BEGIN state");

                if (   (TRUE == DOT1X_SM_AUTH_IsAuthFailVlanReady(sm_p))
                    && (TRUE != DOT1X_SM_AUTH_IsAccessPort(sm_p->dot1x_packet_t.src_lport))
                    && (TRUE == DOT1X_OM_IsPortBasedMode(sm_p->dot1x_packet_t.src_lport))
                    )
                {
                    sm_p->is_applied_guest_vlan = TRUE;  // TRUE, means the guest VLAN is applied already, FALSE, is not yet
                    next_state = DOT1X_SM_AUTH_SUCCESS_BEGIN_ST;
                }
                else
                {
                    next_state = DOT1X_SM_AUTH_FAIL_ST;
                }
                break;

            case DOT1X_SM_AUTH_FAIL_ST:
                lib1x_message(MESS_DBG_AUTHSM, "Enter FAIL state");

                sm_p->current_id = sm_p->id_from_server;

                DOT1X_SM_AUTH_SendEapFailToSupplicant(sm_p);

                sm_p->auth_diag_entry.dot1xAuthAuthFailWhileAuthenticating++;
                if (TRUE == sm_p->is_auth_succ_before)
                {
                    sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause = VAL_dot1xAuthSessionTerminateCause_reauthFailed;
                    sm_p->is_auth_succ_before = FALSE;
                }

                next_state = DOT1X_SM_AUTH_HELD_BEGIN_ST;
                break;

            default:
                break;
        } /* end of switch (running_state) */

        if (running_state == next_state)
        {
            break;
        }
        else
        {
            running_state = next_state;
        }
    } /* end of while (TRUE) */

    sm_p->current_state = running_state;
}

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
void DOT1X_SM_AUTH_EnableReauthTimer(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    UI32_T lport = sm_p->dot1x_packet_t.src_lport;

    if (FALSE == sm_p->reauth_timer.is_started)
    {
        return;
    }

    sm_p->reauth_timer.seconds = DOT1X_OM_Get_PortReAuthPeriod(lport);
}

/* LOCAL SUBPROGRAM BODIES
 */
static BOOL_T DOT1X_SM_AUTH_IsAccessPort(UI32_T lport)
{
    VLAN_OM_Vlan_Port_Info_T port_info;

    memset(&port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
    if(FALSE == VLAN_PMGR_GetPortEntry(lport, &port_info))
    {
        /* return TRUE to do the original path when error
         */
        return TRUE;
    }

   return (port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access) ? TRUE : FALSE;
}

static BOOL_T DOT1X_SM_AUTH_IsNeedJoinGuestVlan(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    UI32_T vid;

    if (TRUE == NETACCESS_VM_IsMacAuthEnabled(sm_p->dot1x_packet_t.src_lport))
    {
        return FALSE;
    }

    if (FALSE == NETACCESS_OM_GetSecureGuestVlanId(sm_p->dot1x_packet_t.src_lport, &vid))
    {
        lib1x_message1(MESS_DBG_AUTHSM, "%s: NETACCESS_OM_GetSecureGuestVlanId fail", __FUNCTION__);
        return FALSE;
    }

    /* guest vlan is not configurated
     */
    if (0 == vid)
    {
        return FALSE;
    }

    return VLAN_OM_IsVlanExisted(vid);
}

static BOOL_T DOT1X_SM_AUTH_IsAuthFailVlanReady(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    if (VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan != DOT1X_OM_GetPortIntrusionActionStatus(sm_p->dot1x_packet_t.src_lport))
    {
        return FALSE;
    }

    return DOT1X_SM_AUTH_IsNeedJoinGuestVlan(sm_p);
}

static void DOT1X_SM_AUTH_InitEapolHeader(DOT1X_SM_AUTH_Obj_T *sm_p, UI32_T lport)
{
    sm_p->dot1x_packet_t.tag_info = LIB1X_ETHER_TAG_INFO;
    sm_p->dot1x_packet_t.packet_type = LIB1X_ETHER_EAPOL_TYPE;

    sm_p->dot1x_packet_t.src_lport = lport;

    memset(sm_p->dot1x_packet_t.src_mac, 0, sizeof(sm_p->dot1x_packet_t.src_mac));
}

static void DOT1X_SM_AUTH_InitAuthTables(DOT1X_SM_AUTH_Obj_T *sm_p, UI32_T lport)
{
    DOT1X_SM_AUTH_InitAuthStats(lport);

    sm_p->auth_diag_entry.dot1xAuthEntersConnecting = 0;
    sm_p->auth_diag_entry.dot1xAuthEapLogoffsWhileConnecting = 0;
    sm_p->auth_diag_entry.dot1xAuthEntersAuthenticating = 0;
    sm_p->auth_diag_entry.dot1xAuthAuthSuccessWhileAuthenticating = 0;
    sm_p->auth_diag_entry.dot1xAuthAuthTimeoutsWhileAuthenticating = 0;
    sm_p->auth_diag_entry.dot1xAuthAuthFailWhileAuthenticating = 0;
    sm_p->auth_diag_entry.dot1xAuthAuthReauthsWhileAuthenticating = 0;
    sm_p->auth_diag_entry.dot1xAuthAuthEapStartsWhileAuthenticating = 0;
    sm_p->auth_diag_entry.dot1xAuthAuthEapLogoffWhileAuthenticating = 0;
    sm_p->auth_diag_entry.dot1xAuthAuthReauthsWhileAuthenticated = 0;
    sm_p->auth_diag_entry.dot1xAuthAuthEapStartsWhileAuthenticated = 0;
    sm_p->auth_diag_entry.dot1xAuthAuthEapLogoffWhileAuthenticated = 0;
    sm_p->auth_diag_entry.dot1xAuthBackendResponses = 0;
    sm_p->auth_diag_entry.dot1xAuthBackendAccessChallenges = 0;
    sm_p->auth_diag_entry.dot1xAuthBackendOtherRequestsToSupplicant = 0;
    sm_p->auth_diag_entry.dot1xAuthBackendNonNakResponsesFromSupplicant = 0;
    sm_p->auth_diag_entry.dot1xAuthBackendAuthSuccesses = 0;
    sm_p->auth_diag_entry.dot1xAuthBackendAuthFails = 0;

    sm_p->auth_session_stats_entry.dot1xAuthSessionOctetsRx = 0;
    sm_p->auth_session_stats_entry.dot1xAuthSessionOctetsTx = 0;
    sm_p->auth_session_stats_entry.dot1xAuthSessionFramesRx = 0;
    sm_p->auth_session_stats_entry.dot1xAuthSessionFramesTx = 0;
    memset(sm_p->auth_session_stats_entry.dot1xAuthSessionId, 0, sizeof(sm_p->auth_session_stats_entry.dot1xAuthSessionId));
    sm_p->auth_session_stats_entry.dot1xAuthSessionAuthenticMethod = VAL_dot1xAuthSessionAuthenticMethod_remoteAuthServer;
    sm_p->auth_session_stats_entry.dot1xAuthSessionTime = 0;
    sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause = VAL_dot1xAuthSessionTerminateCause_portReInit;
    memset(sm_p->auth_session_stats_entry.dot1xAuthSessionUserName, 0, sizeof(sm_p->auth_session_stats_entry.dot1xAuthSessionUserName));

    sm_p->if_table_stats.ifInOctets = 0;
    sm_p->if_table_stats.ifInUcastPkts = 0;
    sm_p->if_table_stats.ifInNUcastPkts = 0;
    sm_p->if_table_stats.ifInDiscards = 0;
    sm_p->if_table_stats.ifInErrors = 0;
    sm_p->if_table_stats.ifInUnknownProtos = 0;
    sm_p->if_table_stats.ifOutOctets = 0;
    sm_p->if_table_stats.ifOutUcastPkts = 0;
    sm_p->if_table_stats.ifOutNUcastPkts = 0;
    sm_p->if_table_stats.ifOutDiscards = 0;
    sm_p->if_table_stats.ifOutErrors = 0;
    sm_p->if_table_stats.ifOutQLen = 0;
}

static void DOT1X_SM_AUTH_InitAuthStats(UI32_T lport)
{
    DOT1X_AuthStatsEntry_T *stats_p = DOT1X_OM_GetAuthStats(lport);

    memset(stats_p, 0, sizeof(DOT1X_AuthStatsEntry_T));
}

#if (SYS_CPNT_ACCOUNTING == TRUE)
static void DOT1X_SM_AUTH_SendAccountingRequestToServer(DOT1X_SM_AUTH_Obj_T *sm_p, AAA_AccRequestType_T request_type, AAA_Authentic_T auth_by_whom, AAA_AccTerminateCause_T terminate_cause)
{
    AAA_AccRequest_T aaa_request;

    lib1x_message1(MESS_DBG_AUTHSM, "%s",
        (request_type==AAA_ACC_START) ? "ACC_START" :
        (request_type==AAA_ACC_STOP)  ? "ACC_STOP"  : "??");

    sm_p->rinfo.username_len = (sm_p->rinfo.username_len > SYS_ADPT_MAX_USER_NAME_LEN) ? SYS_ADPT_MAX_USER_NAME_LEN : sm_p->rinfo.username_len;

    memset(&aaa_request, 0, sizeof(aaa_request));

    aaa_request.ifindex = sm_p->dot1x_packet_t.src_lport;
    strncpy(aaa_request.user_name, sm_p->rinfo.username, sm_p->rinfo.username_len);

    aaa_request.client_type  = AAA_CLIENT_TYPE_DOT1X;
    aaa_request.request_type = request_type;
    aaa_request.auth_by_whom = auth_by_whom;
    aaa_request.terminate_cause = terminate_cause;

    memcpy(aaa_request.auth_mac, sm_p->dot1x_packet_t.src_mac, sizeof(aaa_request.auth_mac));

    AAA_PMGR_AsyncAccountingRequest(&aaa_request);
}
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */

static BOOL_T DOT1X_SM_AUTH_SendEapSuccToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    return DOT1X_SM_AUTH_SendEapSuccOrFailToSupplicant(sm_p, TRUE);
}

static BOOL_T DOT1X_SM_AUTH_SendEapFailToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    return DOT1X_SM_AUTH_SendEapSuccOrFailToSupplicant(sm_p, FALSE);
}

static BOOL_T DOT1X_SM_AUTH_SendEapSuccOrFailToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p, BOOL_T is_succ)
{
    L_MM_Mref_Handle_T *mem_ref_p;
    UI8_T *frame_p;
    UI32_T pdu_len;

    struct lib1x_eapol *eapol;
    struct lib1x_eap *eap;
    UI8_T *send_packet;
    UI32_T send_packet_size;
    UI32_T lport;

    lport = sm_p->dot1x_packet_t.src_lport;

    send_packet_size = sizeof(struct lib1x_eapol) + sizeof(struct lib1x_eap);

    mem_ref_p = L_MM_AllocateTxBuffer(send_packet_size, L_MM_USER_ID2(SYS_MODULE_DOT1X, DOT1X_TYPE_TRACE_ID_DOT1X_SEND_PACKET));
    frame_p = (UI8_T *)L_MM_Mref_GetPdu(mem_ref_p, &pdu_len);
    if (NULL == frame_p)
    {
        lib1x_message1(MESS_DBG_AUTHSM, "%s: L_MM_Mref_GetPdu fail", __FUNCTION__);
        L_MM_Mref_Release(&mem_ref_p);
        return FALSE;
    }

    send_packet = frame_p;
    memset(send_packet, 0, send_packet_size);

    eapol = (struct lib1x_eapol *)send_packet;
    eap = (struct lib1x_eap *)(send_packet + LIB1X_EAPOL_HDRLEN);

    eapol->protocol_version = LIB1X_EAPOL_VER;
    eapol->packet_type = LIB1X_EAPOL_EAPPKT;
    eapol->packet_body_length = L_STDLIB_Hton16(LIB1X_EAPOL_HDRLEN);

    eap->code = (TRUE == is_succ) ? LIB1X_EAP_SUCCESS : LIB1X_EAP_FAILURE;
    eap->identifier = sm_p->current_id;
    eap->length = eapol->packet_body_length;

    lib1x_message1(MESS_DBG_AUTHSM, "Send EAP Packet/%s to supplicant", ((TRUE == is_succ) ? "Succ" : "Fail"));

    DOT1X_SM_AUTH_SendEapPacketToSupplicant(sm_p, mem_ref_p);

    return TRUE;
}

static BOOL_T DOT1X_SM_AUTH_SendEapRequestIdentityToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    L_MM_Mref_Handle_T *mem_ref_p;
    UI8_T *frame_p;
    UI32_T pdu_len;

    struct lib1x_eapol *eapol;
    struct lib1x_eap *eap;
    struct lib1x_eap_rr *eaprr;
    UI8_T *send_packet;
    UI32_T send_packet_size;
    UI32_T lport;

    lport = sm_p->dot1x_packet_t.src_lport;
    send_packet_size = sizeof(struct lib1x_eapol) + sizeof(struct lib1x_eap) + sizeof(struct lib1x_eap_rr);

    mem_ref_p = L_MM_AllocateTxBuffer(send_packet_size, L_MM_USER_ID2(SYS_MODULE_DOT1X, DOT1X_TYPE_TRACE_ID_DOT1X_SEND_PACKET));
    frame_p = (UI8_T *)L_MM_Mref_GetPdu(mem_ref_p, &pdu_len);
    if (NULL == frame_p)
    {
        lib1x_message1(MESS_DBG_AUTHSM, "%s: L_MM_Mref_GetPdu fail", __FUNCTION__);
        L_MM_Mref_Release(&mem_ref_p);
        return FALSE;
    }

    send_packet = frame_p;

    memset(send_packet, 0, send_packet_size);


    eapol = (struct lib1x_eapol *)send_packet;
    eap = (struct lib1x_eap *)(send_packet + LIB1X_EAPOL_HDRLEN);
    eaprr = (struct lib1x_eap_rr *)(send_packet + LIB1X_EAPOL_HDRLEN + LIB1X_EAP_HDRLEN);

    eapol->protocol_version = LIB1X_EAPOL_VER;
    eapol->packet_type = LIB1X_EAPOL_EAPPKT;
    eapol->packet_body_length = L_STDLIB_Hton16(LIB1X_EAPOL_HDRLEN + sizeof(struct lib1x_eap_rr));

    eap->code = LIB1X_EAP_REQUEST;
    eap->identifier = sm_p->current_id;
    eap->length = eapol->packet_body_length;

    eaprr->type = LIB1X_EAP_RRIDENTITY;

    lib1x_message(MESS_DBG_AUTHSM, "Send EAP Request/Identity to supplicant");

    DOT1X_SM_AUTH_SendEapPacketToSupplicant(sm_p, mem_ref_p);

    DOT1X_OM_GetAuthStats(lport)->dot1xAuthEapolReqIdFramesTx++;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_ExtractEapMessage
 * ---------------------------------------------------------------------
 * PURPOSE : Extract Eap Message from a RADIUS packet
 * INPUT   : sm_p
 * OUTPUT  : buf
 * RETURN  : None
 * NOTES   : The memory space of buf must be allocated by
 *           rinfo.eap_messlen_frmserver first.
 * ---------------------------------------------------------------------
 */
static UI32_T DOT1X_SM_AUTH_ExtractEapMessage(DOT1X_SM_AUTH_Obj_T *sm_p, UI8_T *buf)
{
    const struct lib1x_radiushdr *header_p;
    const struct lib1x_radiusattr *av_pair_p;
    UI16_T av_pairs_len;
    UI32_T buf_len = 0;

    header_p = (struct lib1x_radiushdr *)(sm_p->radius_packet_t.packet_data);
    av_pair_p = (struct lib1x_radiusattr *)(sm_p->radius_packet_t.packet_data + LIB1X_RADHDRLEN);
    av_pairs_len = L_STDLIB_Ntoh16(header_p->length) - LIB1X_RADHDRLEN;

    while (TRUE)
    {
        if (av_pairs_len <= LIB1X_RADATTRLEN)
        {
            lib1x_message(MESS_DBG_AUTHSM, "Malformed RADIUS packet");
            break;
        }

        if (LIB1X_RAD_EAP_MESSAGE == av_pair_p->type)
        {
            memcpy(buf + buf_len, ((UI8_T *)av_pair_p) + LIB1X_RADATTRLEN, av_pair_p->length - LIB1X_RADATTRLEN);
            buf_len += (av_pair_p->length - LIB1X_RADATTRLEN);
        }

        av_pairs_len -= av_pair_p->length;
        if (av_pairs_len <= 0)
        {
            break;
        }
        av_pair_p = (struct lib1x_radiusattr *)(((UI8_T *)av_pair_p) + av_pair_p->length);
    }

    return buf_len;
}

static BOOL_T DOT1X_SM_AUTH_SendEapRequestChallengeToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    L_MM_Mref_Handle_T *mem_ref_p;
    UI8_T *frame_p;
    UI32_T pdu_len;

    struct lib1x_eapol *eapol;
    struct lib1x_eap *eap;
    UI8_T *send_packet;
    BOOL_T result;

    UI32_T lport = sm_p->dot1x_packet_t.src_lport;
    UI32_T send_packet_size = LIB1X_EAPOL_HDRLEN + sm_p->rinfo.eap_messlen_frmserver;

    mem_ref_p = L_MM_AllocateTxBuffer(send_packet_size, L_MM_USER_ID2(SYS_MODULE_DOT1X, DOT1X_TYPE_TRACE_ID_DOT1X_SEND_PACKET));
    frame_p = (UI8_T *)L_MM_Mref_GetPdu(mem_ref_p, &pdu_len);
    if (NULL == frame_p)
    {
        lib1x_message1(MESS_DBG_AUTHSM, "%s: L_MM_Mref_GetPdu fail", __FUNCTION__);
        L_MM_Mref_Release(&mem_ref_p);
        return FALSE;
    }

    send_packet = frame_p;

    eapol = (struct lib1x_eapol *)send_packet;
    eap = (struct lib1x_eap *)(send_packet + LIB1X_EAPOL_HDRLEN);

    eapol->protocol_version = LIB1X_EAPOL_VER;
    eapol->packet_type = LIB1X_EAPOL_EAPPKT;
    eapol->packet_body_length = L_STDLIB_Hton16(sm_p->rinfo.eap_messlen_frmserver);

    /* copy eap packet from the received authentication server packet
     */
    DOT1X_SM_AUTH_ExtractEapMessage(sm_p, (UI8_T *)eap);
    eap->length = eapol->packet_body_length;

    lib1x_message(MESS_DBG_AUTHSM, "Send EAP Request/Challenge to supplicant");

    result = DOT1X_SM_AUTH_SendEapPacketToSupplicant(sm_p, mem_ref_p);

    DOT1X_OM_GetAuthStats(lport)->dot1xAuthEapolReqFramesTx++;
    sm_p->auth_diag_entry.dot1xAuthBackendOtherRequestsToSupplicant++;

    return TRUE;
}

static BOOL_T DOT1X_SM_AUTH_SendEapPacketToSupplicant(DOT1X_SM_AUTH_Obj_T *sm_p, L_MM_Mref_Handle_T *mem_ref_p/*UI8_T *packet, UI32_T packet_size*/)
{
    UI8_T *frame_p;
    UI32_T pdu_len;
    UI32_T tpid;
    UI32_T unit, port, trunk_id;
    UI32_T lport = sm_p->dot1x_packet_t.src_lport;
    UI16_T tag_info;
    VLAN_OM_Dot1qPortVlanEntry_T port_vlan_entry;
    UI8_T src_mac[SYS_ADPT_MAC_ADDR_LEN]    = {0};
    UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN]    = {0};
    UI8_T null_mac[SYS_ADPT_MAC_ADDR_LEN]   = {0};

    SWCTRL_GetCpuMac(src_mac);

    if (DOT1X_OM_IsPortBasedMode(sm_p->dot1x_packet_t.src_lport))
    {
        memcpy(dst_mac, pae_group_address, SYS_ADPT_MAC_ADDR_LEN);
    }
    else
    {
        memcpy(dst_mac, sm_p->dot1x_packet_t.src_mac, SYS_ADPT_MAC_ADDR_LEN);
    }

    if (0 == memcmp(src_mac, null_mac, sizeof(null_mac)))
    {
        lib1x_message(MESS_DBG_AUTHSM, "Warning! src_mac in EAP packet is zero");
        return FALSE;
    }

    if (0 == memcmp(dst_mac, null_mac, sizeof(null_mac)))
    {
        lib1x_message(MESS_DBG_AUTHSM, "Warning! dst_mac in EAP packet is zero");
        return FALSE;
    }

    /* construct ethernet frame data
     */
    frame_p = (UI8_T *)L_MM_Mref_GetPdu(mem_ref_p, &pdu_len);

    VLAN_POM_GetDot1qPortVlanEntry(sm_p->dot1x_packet_t.src_lport ,&port_vlan_entry);
    tag_info = (UI16_T)port_vlan_entry.dot1q_pvid_index;

    tpid = SYS_DFLT_DOT1Q_PORT_TPID_FIELD;

    SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    LAN_SendPacket(mem_ref_p, dst_mac, src_mac, LIB1X_ETHER_EAPOL_TYPE, tag_info, pdu_len, unit, port, FALSE, 3);

    DOT1X_OM_GetAuthStats(lport)->dot1xAuthEapolFramesTx++;
    sm_p->auth_session_stats_entry.dot1xAuthSessionOctetsTx += pdu_len;
    sm_p->auth_session_stats_entry.dot1xAuthSessionFramesTx++;

    return TRUE;
}

static BOOL_T DOT1X_SM_AUTH_SendRequestToAuthServer(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    const struct lib1x_eapol *eapol = (struct lib1x_eapol *)sm_p->dot1x_packet_t.packet_data;
    const struct lib1x_eap *eap = (struct lib1x_eap *)(sm_p->dot1x_packet_t.packet_data + LIB1X_EAPOL_HDRLEN);
    const struct lib1x_eap_rr *eaprr = (struct lib1x_eap_rr *)(sm_p->dot1x_packet_t.packet_data + LIB1X_EAPOL_HDRLEN + LIB1X_EAP_HDRLEN);
    struct radius_info *rinfo = &sm_p->rinfo;
    UI16_T eap_len;

    eap_len = L_STDLIB_Ntoh16(eap->length);
    if (eap_len > DOT1X_PACKET_BUFFER_LEN)
    {
        lib1x_message1(MESS_DBG_AUTHSM, "ERROR! EAP packet size (%u) is large than available buffer", eap_len);
        eap_len = DOT1X_PACKET_BUFFER_LEN;
    }

    if (LIB1X_EAP_RRIDENTITY == eaprr->type)
    {
        /* store identity (username) if it appears in the packet
         */
        if (eap_len > (LIB1X_EAP_HDRLEN + sizeof(struct lib1x_eap_rr)))
        {
            rinfo->username_len = eap_len - LIB1X_EAP_HDRLEN - sizeof(struct lib1x_eap_rr);
            rinfo->username_len = (rinfo->username_len > DOT1X_USERNAME_LENGTH) ? DOT1X_USERNAME_LENGTH : rinfo->username_len;

            strncpy(rinfo->username, ((char *)eaprr) + sizeof(struct lib1x_eap_rr), rinfo->username_len);
            rinfo->username[rinfo->username_len] = '\0';

            strncpy(sm_p->auth_session_stats_entry.dot1xAuthSessionUserName, rinfo->username, rinfo->username_len);
            sm_p->auth_session_stats_entry.dot1xAuthSessionUserName[rinfo->username_len] = '\0';

            DOT1X_SM_AUTH_UpdateSessionId(sm_p);
        }
    }

    rinfo->identifier++;

    if (   (LIB1X_EAPOL_EAPPKT == eapol->packet_type)
        && (LIB1X_EAP_RRIDENTITY == eaprr->type))
    {
        sm_p->rinfo.rad_statelength = 0;
    }

    sm_p->radius_packet_t.radius_auth_result = ERROR_RC;

    lib1x_message1(MESS_DBG_AUTHSM, "Send auth. request to auth. server %lu", sm_p->radius_packet_t.server_ip);

    RADIUS_PMGR_AsyncEapAuthCheck((UI8_T *)eap, eap_len, sm_p->current_id, sm_p->rinfo.radius_state,
        sm_p->rinfo.rad_statelength, sm_p->dot1x_packet_t.src_lport, sm_p->dot1x_packet_t.src_mac, sm_p->dot1x_packet_t.src_vid,
        SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY, RADIUS_DOT1X_SERVICE,
        sm_p->radius_packet_t.server_ip, rinfo->username,
        RADIUS_ASYNC_REQ_FLAG_NULL);

    return TRUE;
}

static BOOL_T DOT1X_SM_AUTH_CancelRequestToAuthServer(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    const struct lib1x_eapol *eapol = (struct lib1x_eapol *)sm_p->dot1x_packet_t.packet_data;
    const struct lib1x_eap *eap = (struct lib1x_eap *)(sm_p->dot1x_packet_t.packet_data + LIB1X_EAPOL_HDRLEN);
    const struct lib1x_eap_rr *eaprr = (struct lib1x_eap_rr *)(sm_p->dot1x_packet_t.packet_data + LIB1X_EAPOL_HDRLEN + LIB1X_EAP_HDRLEN);
    struct radius_info *rinfo = &sm_p->rinfo;
    UI16_T eap_len;

    eap_len = L_STDLIB_Ntoh16(eap->length);
    if (eap_len > DOT1X_PACKET_BUFFER_LEN)
    {
        lib1x_message1(MESS_DBG_AUTHSM, "ERROR! EAP packet size (%u) is large than available buffer", eap_len);
        eap_len = DOT1X_PACKET_BUFFER_LEN;
    }

    if (   (LIB1X_EAPOL_EAPPKT == eapol->packet_type)
        && (LIB1X_EAP_RRIDENTITY == eaprr->type))
    {
        sm_p->rinfo.rad_statelength = 0;
    }

    sm_p->radius_packet_t.radius_auth_result = ERROR_RC;

    lib1x_message1(MESS_DBG_AUTHSM, "Cancel auth. request to auth. server %lu", sm_p->radius_packet_t.server_ip);

    RADIUS_PMGR_AsyncEapAuthCheck((UI8_T *)eap, eap_len, sm_p->current_id, sm_p->rinfo.radius_state,
        sm_p->rinfo.rad_statelength, sm_p->dot1x_packet_t.src_lport, sm_p->dot1x_packet_t.src_mac, sm_p->dot1x_packet_t.src_vid,
        SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY, RADIUS_DOT1X_SERVICE,
        sm_p->radius_packet_t.server_ip, rinfo->username,
        RADIUS_ASYNC_REQ_FLAG_CANCEL_REQUEST);

    return TRUE;
}

static BOOL_T DOT1X_SM_AUTH_AnnounceAuthorizedResult(UI32_T lport, UI8_T *port_mac, int eap_identifier, UI8_T auth_result, char *authorized_vlan_list, char *authorized_qos_list, UI32_T session_time, UI32_T server_ip)
{
    UI32_T cookie;

    DOT1X_OM_GetTaskServiceFunPtr(&cookie, DOT1X_TASK_ASYNC_AUTH_CHECK);
    if (0 == cookie)
    {
        lib1x_message1(MESS_DBG_AUTHSM, "%s: DOT1X_OM_GetTaskServiceFunPtr fail", __FUNCTION__);
        return FALSE;
    }

    SYS_CALLBACK_MGR_AnnounceDot1xAuthorizedResult(
        SYS_MODULE_DOT1X, cookie,
        lport, port_mac,
        eap_identifier, auth_result,
        authorized_vlan_list, authorized_qos_list,
        session_time, server_ip);

    return TRUE;
}

static void DOT1X_SM_AUTH_UpdateSessionId(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    sprintf(sm_p->auth_session_stats_entry.dot1xAuthSessionId, "(%lu, %02X-%02X-%02X-%02X-%02X-%02X)",
        sm_p->dot1x_packet_t.src_lport,
        sm_p->dot1x_packet_t.src_mac[0],
        sm_p->dot1x_packet_t.src_mac[1],
        sm_p->dot1x_packet_t.src_mac[2],
        sm_p->dot1x_packet_t.src_mac[3],
        sm_p->dot1x_packet_t.src_mac[4],
        sm_p->dot1x_packet_t.src_mac[5]);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_StartTimeoutTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Start timeout timer
 * INPUT   : sm_p - pointer to the state machine object
 *           seconds - time to wait to timeout
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
static void DOT1X_SM_AUTH_StartTimeoutTimer(DOT1X_SM_AUTH_Obj_T *sm_p, I32_T seconds)
{
    lib1x_message1(MESS_DBG_BSM, "Timeout timer START (seconds = %lu)", seconds);

    sm_p->gen_timer.is_started = TRUE;
    sm_p->gen_timer.seconds = seconds;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_StopTimeoutTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Stop timeout timer
 * INPUT   : sm_p - pointer to the state machine object
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
static void DOT1X_SM_AUTH_StopTimeoutTimer(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    lib1x_message(MESS_DBG_BSM, "Timeout timer STOP");

    sm_p->gen_timer.is_started = FALSE;
    sm_p->gen_timer.seconds = 0;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_StartReauthTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Start reauthentication timer
 * INPUT   : sm_p - pointer to the state machine object
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
static void DOT1X_SM_AUTH_StartReauthTimer(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    UI32_T reauth_time = DOT1X_OM_Get_PortReAuthPeriod(sm_p->dot1x_packet_t.src_lport);

    lib1x_message1(MESS_DBG_BSM, "Reauth timer START (seconds = %lu)", reauth_time);

    sm_p->reauth_timer.is_started = TRUE;
    sm_p->reauth_timer.seconds = reauth_time;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_SM_AUTH_StopReauthTimer
 * ---------------------------------------------------------------------
 * PURPOSE : Stop reauthentication timer
 * INPUT   : sm_p - pointer to the state machine object
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
static void DOT1X_SM_AUTH_StopReauthTimer(DOT1X_SM_AUTH_Obj_T *sm_p)
{
    lib1x_message(MESS_DBG_BSM, "Reauth timer STOP");

    sm_p->reauth_timer.is_started = FALSE;
    sm_p->reauth_timer.seconds = 0;
}

#endif /* (SYS_CPNT_DOT1X == TRUE) */
