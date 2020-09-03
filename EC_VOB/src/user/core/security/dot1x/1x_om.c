#include "sysfun.h"
#include "1x_om.h"
#include "1x_mgr.h"
#include "1x_eapol.h"
#include "1x_nal.h"
#include "swctrl.h"
#include "sysfun.h"
#include "stdio.h"
#include "sys_module.h"
#include "syslog_type.h"
#include "amtr_mgr.h"
#include "radius_pom.h"
#include "l_cvrt.h"
#include "sysfun.h"
#include "dot1x_sm_auth.h"
#include "nmtr_mgr.h"
#include "sys_time.h"

/* MACRO FUNCTION DECLARATIONS
 */
#define DOT1X_OM_ARRAY_SIZE(array) (UI32_T)(sizeof(array)/sizeof(array[0]))
#define DOT1X_OM_LPORT_TO_INDEX(l_port) (UI32_T)(l_port - 1)
#define DOT1X_OM_IS_VALID_ARRAY_INDEX(index, array) ((index < DOT1X_OM_ARRAY_SIZE(array))?TRUE:FALSE)

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T   dot1x_om_semid;
static UI32_T   dot1x_om_semid_orig_priority;
static  int     g_quietPeriod;
static  int     g_reAuthPeriod;
static  int     g_reAuthEnabled;
static  int     g_maxReq;
static  int     g_txPeriod;
static  UI32_T  system_auth_control;
#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
static  DOT1X_OM_EapolPassThru_T  dot1x_om_eapol_pass_through;
#endif
static  DOT1X_SM_AUTH_Obj_T dot1x_om_sm_objects[DOT1X_MAX_PORT][DOT1X_OM_MAX_SUPP_NBR_PER_PORT];
static  DOT1X_AuthStatsEntry_T dot1x_om_auth_stats_entries[DOT1X_MAX_PORT];

/***for MAC based 802.1X ***/
static  UI32_T      dot1x_om_port_txPeriod[DOT1X_MAX_PORT+1];
static  UI32_T      dot1x_om_port_quietPeriod[DOT1X_MAX_PORT+1];
static  UI32_T      dot1x_om_port_reAuthPeriod[DOT1X_MAX_PORT+1];
static  BOOL_T      dot1x_om_port_reAuthEnabled[DOT1X_MAX_PORT+1];
static  UI32_T      dot1x_om_port_maxReq[DOT1X_MAX_PORT+1];
static  UI32_T      dot1x_om_port_suppTimeout[DOT1X_MAX_PORT+1];
static  UI32_T      dot1X_om_port_serverTimeout[DOT1X_MAX_PORT+1];
static  UI32_T      dot1x_om_port_controlMode[DOT1X_MAX_PORT+1];
static  UI32_T      dot1x_om_port_operationMode[DOT1X_MAX_PORT+1];
static  UI32_T      dot1x_om_port_multihost_macCount[DOT1X_MAX_PORT+1];
static  BOOL_T      dot1x_om_port_enabled[DOT1X_MAX_PORT];
static  UI32_T      dot1x_om_port_key_tx_enabled[DOT1X_MAX_PORT];
static  UI32_T      dot1x_om_port_intrustion_action_status[DOT1X_MAX_PORT];
static  UI32_T      dot1x_om_port_reauth_max[DOT1X_MAX_PORT];
static  UI32_T      dot1x_om_port_admin_controlled_directions[DOT1X_MAX_PORT];

typedef struct DOT1X_OM_PortTimer_S
{
     int aWhile;
     /* timer used by backend auth state machine. */
     int quietWhile;
     /* timer used by auth sm during which it does not acquire a supp.*/
     int reAuthWhen;
     /* reauthentication timer state machine = reAuthPeriod.*/
     int txWhen;
     /* used by auth sm */
     int processWhile;
}DOT1X_OM_PortTimer_T;
DOT1X_OM_PortTimer_T dot1x_om_PortTimer[DOT1X_MAX_PORT+1];

static  UI32_T                DOT1X_OM_radius_msg_id;

/***************************/
static  UI32_T  dot1x_om_msgq_id;
//static  void    (*task_service_fun_ptr[DOT1X_TASK_MAX_SERVICE_NO])();
static  UI32_T  authorized_result_cookie[DOT1X_TASK_MAX_SERVICE_NO];
static  UI32_T  dot1x_om_debug_flag;


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DOT1X_OM_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void DOT1X_OM_SetDebugFlag(UI32_T mode)
{
    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_debug_flag = mode ;
    DOT1X_OM_LeaveCriticalRegion();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DOT1X_OM_GetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : get the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI32_T DOT1X_OM_GetDebugFlag(void)
{
    return dot1x_om_debug_flag ;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DOT1X_OM_GetDebugPrompt
 *-------------------------------------------------------------------------
 * PURPOSE  : get the debug title
 * INPUT    : flag -- debug flag
 * OUTPUT   : none
 * RETURN   : string form of debug flag
 * NOTES    : none
 *-------------------------------------------------------------------------*/
const char* DOT1X_OM_GetDebugPrompt(UI32_T flag)
{
    if (flag & MESS_DBG_AUTH)
        return "dot1x/auth";
    if (flag & MESS_DBG_AUTHSM)
        return "dot1x/authsm";
    if (flag & MESS_DBG_AUTHNET)
        return "dot1x/authnet";
    if (flag & MESS_DBG_KRCSM)
        return "dot1x/krcsm";
    if (flag & MESS_DBG_KXSM)
        return "dot1x/kxsm";
    if (flag & MESS_DBG_SUPP)
        return "dot1x/supp";
    if (flag & MESS_DBG_NAL)
        return "dot1x/nal";
    if (flag & MESS_DBG_BSM)
        return "dot1x/bsm";
    if (flag & MESS_DBG_RAD)
        return "dot1x/rad";
    if (flag & MESS_AUTH_LOG)
        return "dot1x/log";
    if (flag & MESS_ERROR_OK)
        return "dot1x/warning";
    if (flag & MESS_ERROR_FATAL)
        return "dot1x/fatal";
    if (flag & MESS_DBG_SPECIAL)
        return "dot1x/special";
    if (flag & MESS_DBG_SUP_CONFIG)
        return "dot1x/supp/config";
    if (flag & MESS_DBG_SUP_STATE)
        return "dot1x/supp/state";
    if (flag & MESS_DBG_SUP_TIMER)
        return "dot1x/supp/timer";
    if (flag & MESS_DBG_SUP_ERROR)
        return "dot1x/supp/error";
    return "dot1x";
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for DOT1X objects
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTE     :   This function is invoked in DOT1X_TASK_Init.
 *-------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_InitSemaphore(void)
{
    if(SYSFUN_OK!=SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO,&dot1x_om_semid))
        return FALSE;
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_EnterCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical region before a task invokes the spanning
 *              tree objects.
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTE     :   This function make the DOT1X task and the other tasks
 *              keep mutual exclusive.
 *-------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_EnterCriticalRegion(void)
{
    dot1x_om_semid_orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(dot1x_om_semid);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_LeaveCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical region after a task invokes the spanning
 *              tree objects.
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTE     :   This function make the DOT1X task and the other tasks
 *              keep mutual exclusive.
 *-------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_LeaveCriticalRegion(void)
{
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dot1x_om_semid,dot1x_om_semid_orig_priority);
    return TRUE;
}

static BOOL_T DOT1X_OM_IsValidLport(UI32_T lport)
{
    return (0 < lport && lport <= DOT1X_MAX_PORT) ?
        TRUE : FALSE;
}

static BOOL_T DOT1X_OM_IsNullMac(const UI8_T *mac)
{
    return (mac[0]==0 && mac[1]==0 && mac[2]==0 && mac[3]==0 && mac[4]==0 && mac[5]==0) ?
        TRUE : FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetSMObjByPortMac
 * ---------------------------------------------------------------------
 * PURPOSE : Get state machine object
 * INPUT   : lport, mac
 * OUTPUT  : sm_p
 * RETURN  : The pointer of state machine object
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T *DOT1X_OM_GetSMObjByPortMac(UI32_T lport, UI8_T *mac)
{
    if (DOT1X_OM_IsPortBasedMode(lport))
    {
        return DOT1X_OM_GetSMObj(lport, 0);
    }
    else
    {
        UI32_T  i;

        for (i=0; i < DOT1X_OM_MAX_SUPP_NBR_PER_PORT; ++i)
        {
            DOT1X_SM_AUTH_Obj_T *obj_p = DOT1X_OM_GetSMObj(lport, i);

            if (memcmp(obj_p->dot1x_packet_t.src_mac, mac, SYS_ADPT_MAC_ADDR_LEN) == 0)
            {
                return obj_p;
            }
        }
    }

    return NULL;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_NewSMObj
 * ---------------------------------------------------------------------
 * PURPOSE : Allocate a new state machine object
 * INPUT   : lport      - logic port number
 *           mac        - source MAC of the supplicant
 *           eapol      - If TRUE, new state machine object for receiving
 *                        an EAPOL packet. It have higher priority than
 *                        state machine occupied by an non-EAPOL packet.
 * OUTPUT  : None
 * RETURN  : The pointer of the new state machine object
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T *DOT1X_OM_NewSMObj(UI32_T lport, UI8_T *mac, BOOL_T eapol)
{
    if (DOT1X_OM_IsPortBasedMode(lport))
    {
        return DOT1X_OM_GetSMObj(lport, 0);
    }
    else
    {
        UI32_T  i;

        for (i=0; i < DOT1X_OM_MAX_SUPP_NBR_PER_PORT; ++i)
        {
            DOT1X_SM_AUTH_Obj_T *obj_p = DOT1X_OM_GetSMObj(lport, i);

            if (DOT1X_OM_IsNullMac(obj_p->dot1x_packet_t.src_mac))
            {
                obj_p->is_eapol_rx = FALSE;
                return obj_p;
            }
        }

        /* If eapol is set, then find other state machine which occupied by
         * non-EAPOL packet (discovery the 802.1x capable client). It aviod
         * all state machine be occupied, if one real 802.1x capable client
         * want to do authentication.
         */
        if (eapol)
        {
            for (i=0; i < DOT1X_OM_MAX_SUPP_NBR_PER_PORT; ++i)
            {
                DOT1X_SM_AUTH_Obj_T *obj_p = DOT1X_OM_GetSMObj(lport, i);

                if (FALSE == obj_p->is_eapol_rx)
                {
                    return obj_p;
                }
            }
        }
    }

    return NULL;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextWorkingSMObj
 * ---------------------------------------------------------------------
 * PURPOSE : Get next working state machine object
 * INPUT   : lport, idx_p
 * OUTPUT  : None
 * RETURN  : The pointer of the new state machine object
 * NOTES   : Use *idx_p = 0 to get first
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T* DOT1X_OM_GetNextWorkingSMObj(UI32_T lport, UI32_T *idx_p)
{
    if (NULL == idx_p)
        return FALSE;

    if (!DOT1X_OM_IsValidLport(lport))
        return FALSE;

    if (DOT1X_OM_IsPortBasedMode(lport))
    {
        if (0 == *idx_p)
        {
            *idx_p = 1;
            return DOT1X_OM_GetSMObj(lport, 0);
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        DOT1X_SM_AUTH_Obj_T *store_p = DOT1X_OM_GetSMObj(lport, (*idx_p)++);

        while (store_p)
        {
            if (DOT1X_OM_IsNullMac(store_p->dot1x_packet_t.src_mac))
            {
                store_p = DOT1X_OM_GetSMObj(lport, (*idx_p)++);
                continue;
            }

            return store_p;
        }

        return NULL;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetStateMachineObj
 * ---------------------------------------------------------------------
 * PURPOSE : Get state machine object
 * INPUT   : lport
 * OUTPUT  : sm_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T *DOT1X_OM_GetStateMachineObj(UI32_T lport)
{
    return DOT1X_OM_GetSMObj(lport, 0);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetSMObj
 * ---------------------------------------------------------------------
 * PURPOSE : Get state machine object
 * INPUT   : lport, mac_index
 * OUTPUT  : sm_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T* DOT1X_OM_GetSMObj(UI32_T lport, UI32_T mac_index)
{
    if (!DOT1X_OM_IsValidLport(lport) || DOT1X_OM_MAX_SUPP_NBR_PER_PORT < mac_index)
    {
        return (DOT1X_SM_AUTH_Obj_T*)NULL;
    }

    return &dot1x_om_sm_objects[lport-1][mac_index];
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthStats
 * ---------------------------------------------------------------------
 * PURPOSE : Get dot1xAuthStatsTable entry pointer
 * INPUT   : lport, mac_index
 * OUTPUT  : None
 * RETURN  : The pointer of dot1xAuthStatsTable entry
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_AuthStatsEntry_T* DOT1X_OM_GetAuthStats(UI32_T lport)
{
    if (!DOT1X_OM_IsValidLport(lport))
    {
        return (DOT1X_AuthStatsEntry_T*)NULL;
    }

    return &dot1x_om_auth_stats_entries[lport-1];
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Initialize
 * ---------------------------------------------------------------------
 * PURPOSE : Init OM database
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Initialize()
 {
    UI32_T i;

    DOT1X_OM_SetConfigSettingToDefault();

    DOT1X_OM_EnterCriticalRegion();

    for (i = 0; i < DOT1X_MAX_PORT; i++)
    {
        dot1x_om_port_enabled[i] = FALSE;
    }

    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
 }

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE : Set config setting to default
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_SetConfigSettingToDefault()
{
    UI32_T i;
    UI32_T lport;

    DOT1X_OM_EnterCriticalRegion();

    system_auth_control = SYS_DFLT_DOT1X_PAE_SYSTEM_AUTH_CONTROL;
    g_quietPeriod = DOT1X_DEFAULT_QUIETPERIOD;
    g_txPeriod  = DOT1X_DEFAULT_TXPERIOD;
    g_maxReq    = DOT1X_DEFAULT_MAXREQ;
    g_reAuthPeriod  = DOT1X_DEFAULT_REAUTHPERIOD;
    g_reAuthEnabled = DOT1X_DEFAULT_REAUTHENABLED;

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
    dot1x_om_eapol_pass_through = DOT1X_DEFAULT_EAPOL_PASS_THRU_STATUS;
#endif

#if (SYS_CPNT_DOT1X == TRUE)
    for (i = 0; i < DOT1X_MAX_PORT; i++)
    {
        dot1x_om_port_controlMode[i] = DOT1X_DEFAULT_PORT_MODE;
        dot1x_om_port_txPeriod[i] = DOT1X_DEFAULT_TXPERIOD;
        dot1x_om_port_quietPeriod[i] = DOT1X_DEFAULT_QUIETPERIOD;
        dot1x_om_port_reAuthPeriod[i] = DOT1X_DEFAULT_REAUTHPERIOD;
        dot1x_om_port_reAuthEnabled[i] = DOT1X_DEFAULT_REAUTHENABLED;
        dot1x_om_port_maxReq[i] = DOT1X_DEFAULT_MAXREQ;
        dot1x_om_port_suppTimeout[i] = DOT1X_DEFAULT_SUPPLICANT_TIMEOUT;
        dot1X_om_port_serverTimeout[i] = DOT1X_DEFAULT_SERVER_TIMEOUT;
        dot1x_om_port_operationMode[i] = DOT1X_DEFAULT_PORT_OPERATION_MODE;
        dot1x_om_port_multihost_macCount[i] = DOT1X_DEFAULT_MULTI_HOST_MAC_COUNT;
        dot1x_om_port_key_tx_enabled[i] = VAL_dot1xAuthKeyTxEnabled_true;
        dot1x_om_port_intrustion_action_status[i] = DOT1X_DEFAULT_ACTION;
        dot1x_om_port_reauth_max[i] = LIB1X_AP_REAUTHMAX;
        dot1x_om_port_admin_controlled_directions[i] = VAL_dot1xAuthOperControlledDirections_both;
    }

    for (lport = 1; lport <= DOT1X_MAX_PORT; lport++)
    {
        for (i = 0; i < DOT1X_OM_MAX_SUPP_NBR_PER_PORT; ++i)
        {
            DOT1X_SM_AUTH_Obj_T *sm_p = DOT1X_OM_GetSMObj(lport, i);

            if (sm_p == NULL)
            {
                printf("lport %lu, i %lu\n", lport, i);
                continue;
            }

            DOT1X_SM_AUTH_Init(sm_p, lport);
        }
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_Authen_dot1x
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default Authen_dot1x is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_Authen_Dot1x(UI32_T *authen_do1x)
{
   UI32_T result;

    *authen_do1x = DOT1X_OM_Get_DOT1X_Authen_Enable_Status();
    if ( *authen_do1x != DOT1X_DEFAULT_AUTHEN_DOT1X_STATUS)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Set_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE : This function will set port control mode of 1X configuration
 * INPUT   : lport
 *           mode - VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized,
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized or
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortControlMode(UI32_T lport, UI32_T mode)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (DOT1X_OM_ARRAY_SIZE(dot1x_om_port_controlMode) <= index)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_controlMode[index] = mode;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE : This function will get port control mode of 1X configuration
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized,
 *           VAL_dot1xAuthAuthControlledPortControl_forceAuthorized or
 *           VAL_dot1xAuthAuthControlledPortControl_auto
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortControlMode(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T control_mode;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_controlMode) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    control_mode = dot1x_om_port_controlMode[index];
    DOT1X_OM_LeaveCriticalRegion();
    return control_mode;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetRunning_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *           the non-default PortControlMode is successfully retrieved.
 *           Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : value_p - VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized,
 *                     VAL_dot1xAuthAuthControlledPortControl_forceAuthorized or
 *                     VAL_dot1xAuthAuthControlledPortControl_auto
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE or
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default system name.
 *           3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_PortControlMode(UI32_T lport, UI32_T *value_p)
{
    *value_p = DOT1X_OM_Get_PortControlMode(lport);

    if (DOT1X_DEFAULT_PORT_MODE == *value_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get port control mode of 1X configuration
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  mode = 0 for ForceUnauthorized
                  1 for ForceAuthorized
                  2 for Auto
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortControlMode(UI32_T *index,UI32_T *mode)
{
    (*index)++;
    if(*index > DOT1X_MAX_PORT)
        return FALSE;
    *mode = DOT1X_OM_Get_PortControlMode(*index);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNextRunning_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the mode of 802.1x port support multiple-host
 * INPUT:  index.
 * OUTPUT: type.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetNextRunning_PortControlMode(UI32_T *index,UI32_T *type)
{
    UI32_T result;

    (*index)++;
    if(*index > DOT1X_MAX_PORT)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    result = DOT1X_OM_GetRunning_PortControlMode(*index,type);
    return result;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetPortAuthorized
 * --------------------------------------------------------------------------
 * PURPOSE : Get the current authorization state of the port
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_AUTHORIZED,
 *           DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_UNAUTHORIZED or
 *           DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_ERR
 * NOTE    : None
 * --------------------------------------------------------------------------
 */
DOT1X_TYPE_AuthControlledPortStatus_T DOT1X_OM_GetPortAuthorized(UI32_T lport)
{
    DOT1X_TYPE_AuthControlledPortStatus_T port_status = DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_UNAUTHORIZED;

    if (VAL_dot1xPaeSystemAuthControl_disabled == DOT1X_OM_Get_SystemAuthControl())
    {
        port_status = (TRUE == DOT1X_OM_GetPortEnabled(lport)) ?
            DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_AUTHORIZED :
            DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_UNAUTHORIZED;
        return port_status;
    }

    DOT1X_SM_AUTH_Obj_T *sm_p = NULL;
    UI32_T              i     = 0;
    sm_p = DOT1X_OM_GetNextWorkingSMObj(lport, &i);

    while (sm_p)
    {
        if (VAL_dot1xAuthAuthControlledPortStatus_authorized == sm_p->port_status)
        {
            return DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_AUTHORIZED;
        }

        sm_p = DOT1X_OM_GetNextWorkingSMObj(lport, &i);
    }

    return DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_UNAUTHORIZED;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Set_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Set period re-authentication status of the specified port
 * INPUT   : lport - port number
 *           value - VAL_dot1xPaePortReauthenticate_true
 *                   VAL_dot1xPaePortReauthenticate_false
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortReAuthEnabled(UI32_T lport, UI32_T value)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (   (VAL_dot1xPaePortReauthenticate_true != value)
        && (VAL_dot1xPaePortReauthenticate_false != value))
    {
        return FALSE;
    }

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_reAuthEnabled) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_reAuthEnabled[index] = value;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get period re-authentication status of the specified port
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : VAL_dot1xPaePortReauthenticate_true or
 *           VAL_dot1xPaePortReauthenticate_false
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortReAuthEnabled(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T value;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_reAuthEnabled) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    value = dot1x_om_port_reAuthEnabled[index];
    DOT1X_OM_LeaveCriticalRegion();

    return value;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetRunning_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *           the non-default PortControlMode is successfully retrieved.
 *           Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : lport
 * OUTPUT  : value_p - VAL_dot1xPaePortReauthenticate_true or
 *                     VAL_dot1xPaePortReauthenticate_false
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE or
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *           3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortReAuthEnabled(UI32_T lport, UI32_T *value_p)
{
    *value_p = DOT1X_OM_Get_PortReAuthEnabled(lport);

    if (DOT1X_DEFAULT_REAUTHENABLED == *value_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNext_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get period re-authentication status of the next port
 * INPUT   : lport_p
 * OUTPUT  : lport_p - the next port number
 *           value_p - VAL_dot1xPaePortReauthenticate_true or
 *                     VAL_dot1xPaePortReauthenticate_false
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortReAuthEnabled(UI32_T *lport_p, UI32_T *value_p)
{
    (*lport_p)++;

    if (*lport_p > DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    *value_p = DOT1X_OM_Get_PortReAuthEnabled(*lport_p);

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextRunning_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get period re-authentication status of the next port
 * INPUT   : lport_p
 * OUTPUT  : lport_p - the next port number
 *           value_p - VAL_dot1xPaePortReauthenticate_true or
 *                     VAL_dot1xPaePortReauthenticate_false
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE or
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default system name.
 *           3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetNextRunning_PortReAuthEnabled(UI32_T *lport_p, UI32_T *value_p)
{
    (*lport_p)++;

    if (*lport_p > DOT1X_MAX_PORT)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    return DOT1X_OM_GetRunning_PortReAuthEnabled(*lport_p, value_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable global period re-authentication of the client ,which
 *          is disabled by default.
 * INPUT:  control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  TRUE for Enable re-authentication
 *                    FALSE for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_ReAuthenticationMode(BOOL_T control)
{
    UI32_T i;

    DOT1X_OM_EnterCriticalRegion();
    for (i = 0; i < DOT1X_MAX_PORT; i++)
    {
        dot1x_om_port_reAuthEnabled[i] = control;
    }
    DOT1X_OM_LeaveCriticalRegion();
    DOT1X_OM_Set_GlobalReauthEnabled(control);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get port global re-authentication status of the client
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: mode.
 * NOTES:  mode =  TRUE for Enable re-authentication
 *                 FALSE for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_ReAuthenticationMode()
{
  BOOL_T mode;

  mode = DOT1X_OM_Get_GlobalReauthEnabled();
  return mode;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: ReAuthenticationMode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_ReAuthenticationMode(UI32_T *mode)
{
   UI32_T result;

    *mode = DOT1X_OM_Get_ReAuthenticationMode();
    if ( *mode != DOT1X_DEFAULT_REAUTHENABLED)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set the number of seconds between re-authentication attempts.
 *          The range is 1 to 65535;the default is 3600 seconds.
 * INPUT:  seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_ReAuthPeriod(UI32_T seconds)
{
    UI32_T i;

    if(seconds < 1 || seconds > 65535)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    for (i = 0; i < DOT1X_MAX_PORT; i++)
    {
        dot1x_om_port_reAuthPeriod[i] = seconds;
        dot1x_om_PortTimer[i].reAuthWhen = seconds;
    }
    DOT1X_OM_LeaveCriticalRegion();
    DOT1X_OM_Set_GlobalReauthPeriod(seconds);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set the number of seconds between re-authentication attempts for per-port.
 *          The range is 1 to 65535;the default is 3600 seconds.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortReAuthPeriod(UI32_T lport,UI32_T seconds)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if(seconds < 1 || seconds > 65535)
    {
        return FALSE;
    }

    if ( (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_PortTimer) == FALSE)
        || (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_reAuthPeriod) == FALSE))
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_PortTimer[index].reAuthWhen = seconds;
    dot1x_om_port_reAuthPeriod[index] = seconds;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of seconds between re-authentication attempts.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: ReAuthPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_ReAuthPeriod()
{
     UI32_T period;

     period = DOT1X_OM_Get_GlobalReauthPeriod();
     return period;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of seconds between re-authentication attempts for per-port.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: PortReAuthPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortReAuthPeriod(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T period;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_reAuthPeriod) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    period = dot1x_om_port_reAuthPeriod[index];
    DOT1X_OM_LeaveCriticalRegion();
    return period;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: ReAuthPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_ReAuthPeriod(UI32_T *seconds)
{
     UI32_T result;

     *seconds = DOT1X_OM_Get_ReAuthPeriod();
     if ( *seconds != DOT1X_DEFAULT_REAUTHPERIOD)
         result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
     else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
     return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: ReAuthPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_PortReAuthPeriod(UI32_T lport,UI32_T *seconds)
{
   UI32_T result;

    *seconds = DOT1X_OM_Get_PortReAuthPeriod(lport);
    if (*seconds  != DOT1X_DEFAULT_REAUTHPERIOD)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of seconds between re-authentication attempts for per-port.
 * INPUT:  index.
 * OUTPUT: times.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortReAuthPeriod(UI32_T *index,UI32_T *times)
{
    (*index)++;
    if(*index > DOT1X_MAX_PORT)
        return FALSE;
    *times = DOT1X_OM_Get_PortReAuthPeriod(*index);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of seconds that the switch remains in the quiet state
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.The range is from 1 to 65535.
 * INPUT:  seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_QuietPeriod(UI32_T seconds)
{
    UI32_T i;

    if ( seconds < 1 || seconds > 65535)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    for (i = 0; i < DOT1X_MAX_PORT; i++)
    {
        dot1x_om_port_quietPeriod[i] = seconds;
        dot1x_om_PortTimer[i].quietWhile = seconds;
    }
    DOT1X_OM_LeaveCriticalRegion();
    DOT1X_OM_Set_GlobalQuietPeriod(seconds);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of seconds for per-port that the switch remains in the quiet state
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.The range is from 1 to 65535.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortQuietPeriod(UI32_T lport,UI32_T seconds)
{
    UI32_T  index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if ( seconds < 1 || seconds > 65535)
    {
        return FALSE;
    }

    if ( (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_PortTimer) == FALSE)
        || (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_quietPeriod) == FALSE))
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_PortTimer[index].quietWhile = seconds;
    dot1x_om_port_quietPeriod[index] = seconds;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds that the switch remains in the quiet state
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.The range is from 1 to 65535.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: QuietPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_QuietPeriod( )
{
    UI32_T period;

    period = DOT1X_OM_Get_GlobalQuietPeriod();
    return period;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds for per-port that the switch remains in the quiet state
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.The range is from 1 to 65535.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortQuietPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortQuietPeriod(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T period;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_quietPeriod) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    period = dot1x_om_port_quietPeriod[index];
    DOT1X_OM_LeaveCriticalRegion();
    return period;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: QuietPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_QuietPeriod(UI32_T *seconds)
{
     UI32_T result;

    *seconds = DOT1X_OM_Get_QuietPeriod();
    if ( *seconds != DOT1X_DEFAULT_QUIETPERIOD)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: QuietPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_PortQuietPeriod(UI32_T lport,UI32_T *seconds)
{
    UI32_T result;

    *seconds = DOT1X_OM_Get_PortQuietPeriod(lport);
    if ( *seconds != DOT1X_DEFAULT_QUIETPERIOD)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds for per-port that the switch remains in the quiet state
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.
 * INPUT:  index.
 * OUTPUT: times.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortQuietPeriod(UI32_T *index,UI32_T *times)
{
    (*index)++;
    if(*index > DOT1X_MAX_PORT)
        return FALSE;
    *times = DOT1X_OM_Get_PortQuietPeriod(*index);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before
 *          retransmitting the request.
 *          The range is 1 to 65535 seconds;the default is 30 seconds.
 * INPUT:  seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_TxPeriod(UI32_T seconds)
{
    UI32_T i;

    if(seconds < 1 || seconds > 65535)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    for (i = 0; i < DOT1X_MAX_PORT; i++)
    {
        dot1x_om_port_txPeriod[i] = seconds;
        dot1x_om_PortTimer[i].txWhen = seconds;
    }
    DOT1X_OM_LeaveCriticalRegion();
    DOT1X_OM_Set_GlobalTxPeriod(seconds);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before
 *          retransmitting the request for per-port.
 *          The range is 1 to 65535 seconds;the default is 30 seconds.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortTxPeriod(UI32_T lport,UI32_T seconds)
{
    UI32_T  index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if(seconds < 1 || seconds > 65535)
    {
        return FALSE;
    }

    if ( (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_PortTimer) == FALSE)
        || (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_txPeriod) == FALSE))
    {
        return (FALSE);
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_PortTimer[index].txWhen = seconds;
    dot1x_om_port_txPeriod[index] = seconds;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before
 *          retransmitting the request.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TxPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_TxPeriod()
{
    UI32_T period;

    period = DOT1X_OM_Get_GlobalTxPeriod();
    return period;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before
 *          retransmitting the request for per-port.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortTxPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortTxPeriod(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T period;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_txPeriod) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    period = dot1x_om_port_txPeriod[index];
    DOT1X_OM_LeaveCriticalRegion();
    return period;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: TxPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_TxPeriod(UI32_T *seconds)
{
   UI32_T result;

    *seconds=DOT1X_OM_Get_TxPeriod();
    if ( *seconds != DOT1X_DEFAULT_TXPERIOD)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: TxPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_PortTxPeriod(UI32_T lport,UI32_T *seconds)
{
   UI32_T result;

    *seconds = DOT1X_OM_Get_PortTxPeriod(lport);
    if ( *seconds != DOT1X_DEFAULT_TXPERIOD)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before
 *          retransmitting the request for per-port.
 * INPUT:  index.
 * OUTPUT: seconds.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortTxPeriod(UI32_T *index,UI32_T *seconds)
{
    (*index)++;
    if(*index > DOT1X_MAX_PORT)
        return FALSE;
    *seconds = DOT1X_OM_Get_PortTxPeriod(*index);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_MaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of times that the switch will send an
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 *          The range is 1 to 10;the default is 2 .
 * INPUT:  times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_MaxReq(UI32_T times)
{
    UI32_T i;

    if(times < 1 || times > 10)
    {
        return FALSE;
    }

    DOT1X_OM_Set_GlobalMaxReq(times) ;

    DOT1X_OM_EnterCriticalRegion();
    for (i = 0; i < DOT1X_MAX_PORT; i++)
    {
        dot1x_om_port_maxReq[i] = times;
    }
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of times for per-port that the switch will send an
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 *          The range is 1 to 10;the default is 2 .
 * INPUT:  lport,times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortMaxReq(UI32_T lport,UI32_T times)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if(times < 1 || times > 10)
    {
        return FALSE;
    }

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_maxReq) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_maxReq[index] = times;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_MaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of times that the switch will send an
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: MaxReq.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_MaxReq()
{
    UI32_T max_req;

    max_req = DOT1X_OM_Get_GlobalMaxReq();
    return max_req;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of times for per-port that the switch will send an
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortMaxReq.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortMaxReq(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T max_req;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_maxReq) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    max_req = dot1x_om_port_maxReq[index];
    DOT1X_OM_LeaveCriticalRegion();
    return max_req;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_MaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_MaxReq(UI32_T *max_req)
{
    UI32_T result;

    *max_req = DOT1X_OM_Get_MaxReq();
    if ( *max_req != DOT1X_DEFAULT_MAXREQ)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortReAuthMax
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortReAuthMax(UI32_T lport,UI32_T *max_req)
{
   UI32_T result;

    *max_req = DOT1X_OM_Get_PortReAuthMax(lport);
    if ( *max_req != SYS_DFLT_DOT1X_AUTH_MAX_REAUTH_REQ)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortMaxReq(UI32_T lport,UI32_T *max_req)
{
   UI32_T result;

    *max_req = DOT1X_OM_Get_PortMaxReq(lport);
    if ( *max_req != DOT1X_DEFAULT_MAXREQ)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of times for per-port that the switch will send an
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 * INPUT:  index.
 * OUTPUT: max_req.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortMaxReq(UI32_T *index,UI32_T *max_req)
{
    (*index)++;
    if(*index > DOT1X_MAX_PORT)
        return FALSE;
    *max_req = DOT1X_OM_Get_PortMaxReq(*index);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNextRunning_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetNextRunning_PortMaxReq(UI32_T *index,UI32_T *max_req)
{
    (*index)++;
    if(*index > DOT1X_MAX_PORT)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    return DOT1X_OM_GetRunning_PortMaxReq(*index,max_req);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Set the administrative enable/disable state for Port Access
            Control in a system.
 * INPUT:  control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
           VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_SystemAuthControl(UI32_T control)
{
     DOT1X_OM_EnterCriticalRegion();
     system_auth_control = control;
     DOT1X_OM_LeaveCriticalRegion();
     return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Get the administrative enable/disable state for Port Access
            Control in a system.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: control.
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
           VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_SystemAuthControl()
{
      UI32_T auth_control;

      DOT1X_OM_EnterCriticalRegion();
      auth_control = system_auth_control;
      DOT1X_OM_LeaveCriticalRegion();
      return auth_control;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: SystemAuthControl
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_SystemAuthControl(UI32_T *control)
{
    UI32_T result;

    *control = DOT1X_OM_Get_SystemAuthControl();
    if ( *control != SYS_DFLT_DOT1X_PAE_SYSTEM_AUTH_CONTROL)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_SetPortEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Set the value of port enabled
 * INPUT   : lport -- port number
 *           value -- TRUE: port is link up
 *                    FALSE: port is link down
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_SetPortEnabled(UI32_T lport, BOOL_T value)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_enabled) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_enabled[index] = value;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetPortEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get the value of port enabled
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : TRUE: port is link up
 *           FALSE: port is link down
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetPortEnabled(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    BOOL_T value;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_enabled) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    value = dot1x_om_port_enabled[index];
    DOT1X_OM_LeaveCriticalRegion();

    return value;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_AdminCtrlDirections
 * ---------------------------------------------------------------------
 * PURPOSE : Set the value of dot1xAuthAdminControlledDirections
 * INPUT   : lport - port number
 *           value - VAL_dot1xAuthAdminControlledDirections_both or
 *                   VAL_dot1xAuthAdminControlledDirections_in
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AdminCtrlDirections(UI32_T lport, UI32_T value)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (   (VAL_dot1xAuthAdminControlledDirections_both != value)
        && (VAL_dot1xAuthAdminControlledDirections_in != value))
    {
        return FALSE;
    }

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_admin_controlled_directions) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_admin_controlled_directions[index] = value;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_AdminCtrlDirections
 * ---------------------------------------------------------------------
 * PURPOSE : Get the value of dot1xAuthAdminControlledDirections
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : VAL_dot1xAuthAdminControlledDirections_both or
 *           VAL_dot1xAuthAdminControlledDirections_in
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_AdminCtrlDirections(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T value;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_admin_controlled_directions) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    value = dot1x_om_port_admin_controlled_directions[index];
    DOT1X_OM_LeaveCriticalRegion();

    return value;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_CtrlPortControl
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value of the controlled port parameter for the port.
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =   0 for ForceUnauthorized
                       1 for ForceAuthorized
                       2 for Auto
 * ---------------------------------------------------------------------
 */
#if 0
BOOL_T DOT1X_OM_Set_CtrlPortControl(UI32_T lport,UI32_T control)
{
   Global_Params *pae_state;

   pae_state=DOT1X_OM_GetStateMachineWorkingArea(lport);
   if(pae_state == NULL)/*ES3550C-ZZ-00007*/
     return FALSE;
   DOT1X_OM_EnterCriticalRegion();
   pae_state -> portControl= control;
   DOT1X_OM_LeaveCriticalRegion();
   return TRUE;
}
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_CtrlPortControl
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value of the controlled port parameter for the port.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: control.
 * NOTES:  control =   0 for ForceUnauthorized
                       1 for ForceAuthorized
                       2 for Auto
 * ---------------------------------------------------------------------
 */
#if 0
UI32_T DOT1X_OM_Get_CtrlPortControl(UI32_T lport)
{
    UI32_T control;
    Global_Params *pae_state;

    pae_state=DOT1X_OM_GetStateMachineWorkingArea(lport);
        control = pae_state -> portControl;
    return control;
}
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the suppTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AuthSuppTimeout(UI32_T lport,UI32_T seconds)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_suppTimeout) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_suppTimeout[index] = seconds;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the suppTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: AuthSuppTimeout.
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_AuthSuppTimeout(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T seconds;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_suppTimeout) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    seconds = dot1x_om_port_suppTimeout[index];
    DOT1X_OM_LeaveCriticalRegion();
    return seconds;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default supp-timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: AuthSuppTimeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_AuthSuppTimeout(UI32_T lport,UI32_T *seconds)
{
   UI32_T result;

    *seconds = DOT1X_OM_Get_AuthSuppTimeout(lport);
    if ( *seconds != DOT1X_DEFAULT_SUPPLICANT_TIMEOUT)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_AuthServerTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the serverTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  ES4549-08-00364,MIB do not support set function now
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AuthServerTimeout(UI32_T lport,UI32_T seconds)
{
     return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_AuthServerTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the serverTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: AuthServerTimeout.
 * NOTES: Problem:when system brign up ,sometimes it will hang.
 *        LLDP will call pom to radus to get data ,and radius  is taking sem of it.
 *        RootCause:LLDP will call pom to radus to get data ,and radius  is taking sem of it.
 *        Solution:when call pom,it doesn't need to sem protect
 *        File:1x_om.c
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_AuthServerTimeout(UI32_T lport)
{
    UI32_T seconds = 0;
    UI32_T server_index = 0;
    RADIUS_Server_Host_T server_host;

    while(RADIUS_POM_GetNext_Server_Host(&server_index,&server_host) == TRUE)
    {
        seconds += server_host.timeout * (1 + server_host.retransmit);
    }

    return seconds;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Set_AuthTxEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Set the value of the keyTransmissionEnabled constant currently
             in use by the Authenticator PAE state machine.
 * INPUT   : lport - port number
 *           value - VAL_dot1xAuthKeyTxEnabled_true or
 *                   VAL_dot1xAuthKeyTxEnabled_false
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AuthTxEnabled(UI32_T lport, UI32_T value)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (   (value != VAL_dot1xAuthKeyTxEnabled_true)
        && (value != VAL_dot1xAuthKeyTxEnabled_false))
    {
        return FALSE;
    }

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_key_tx_enabled) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_key_tx_enabled[index] = value;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_AuthTxEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get the value of the keyTransmissionEnabled constant currently
             in use by the Authenticator PAE state machine.
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : VAL_dot1xAuthKeyTxEnabled_true or
 *           VAL_dot1xAuthKeyTxEnabled_false
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_AuthTxEnabled(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T value;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_key_tx_enabled) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    value = dot1x_om_port_key_tx_enabled[index];
    DOT1X_OM_LeaveCriticalRegion();

    return value;
}

#if 0
BOOL_T DOT1X_OM_Set_Svr_Port(UI32_T port_number)
{
    svr_port = port_number ;
    return TRUE;
}

UI32_T DOT1X_OM_Get_Svr_Port(void)
{
    return svr_port ;
}

BOOL_T DOT1X_OM_Set_RADIUSId(UI32_T id)
{
    radius_id = id ;
    return TRUE;
}

UI32_T DOT1X_OM_Get_RADIUSId(void)
{
    return radius_id ;
}

BOOL_T DOT1X_OM_Set_TotalLPort(UI32_T port_number)
{
    total_lport = port_number ;
    return TRUE;
}

UI32_T DOT1X_OM_Get_TotalLPort(void)
{
    return total_lport ;
}
#endif
BOOL_T DOT1X_OM_Set_GlobalQuietPeriod(int period)
{
    DOT1X_OM_EnterCriticalRegion();
    g_quietPeriod = period ;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

int DOT1X_OM_Get_GlobalQuietPeriod(void)
{
      int quietPeriod;

      DOT1X_OM_EnterCriticalRegion();
      quietPeriod = g_quietPeriod;
      DOT1X_OM_LeaveCriticalRegion();
      return  quietPeriod;
}

BOOL_T DOT1X_OM_Set_GlobalReauthPeriod(int period)
{
     DOT1X_OM_EnterCriticalRegion();
     g_reAuthPeriod = period ;
     DOT1X_OM_LeaveCriticalRegion();
     return TRUE;
}

int DOT1X_OM_Get_GlobalReauthPeriod(void)
{
      int reAuthPeriod;

      DOT1X_OM_EnterCriticalRegion();
      reAuthPeriod = g_reAuthPeriod;
      DOT1X_OM_LeaveCriticalRegion();
      return  reAuthPeriod;
}

BOOL_T DOT1X_OM_Set_GlobalReauthEnabled(BOOL_T mode)
{
    DOT1X_OM_EnterCriticalRegion();
    g_reAuthEnabled = mode ;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T DOT1X_OM_Get_GlobalReauthEnabled(void)
{
      BOOL_T reAuthEnabled;

      DOT1X_OM_EnterCriticalRegion();
      reAuthEnabled = g_reAuthEnabled;
      DOT1X_OM_LeaveCriticalRegion();
      return  reAuthEnabled;
}

BOOL_T DOT1X_OM_Set_GlobalMaxReq(UI32_T times)
{
     DOT1X_OM_EnterCriticalRegion();
     g_maxReq = times ;
     DOT1X_OM_LeaveCriticalRegion();
     return TRUE;
}

UI32_T DOT1X_OM_Get_GlobalMaxReq(void)
{
      UI32_T maxReq;

      DOT1X_OM_EnterCriticalRegion();
      maxReq = g_maxReq;
      DOT1X_OM_LeaveCriticalRegion();
      return  maxReq;
}

BOOL_T DOT1X_OM_Set_GlobalTxPeriod(int period)
{
     DOT1X_OM_EnterCriticalRegion();
     g_txPeriod = period ;
     DOT1X_OM_LeaveCriticalRegion();
     return TRUE;
}

int DOT1X_OM_Get_GlobalTxPeriod(void)
{
      int txPeriod;

      DOT1X_OM_EnterCriticalRegion();
      txPeriod = g_txPeriod;
      DOT1X_OM_LeaveCriticalRegion();
      return txPeriod;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the authenticator's operation mode
 * INPUT:  lport,mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *         DOT1X_PORT_OPERATION_MODE_MACBASED  for MAC-based
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortOperationMode(UI32_T lport,UI32_T mode)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if ( (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_operationMode) == FALSE)
        || (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_PortTimer) == FALSE))
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    /* when port-based <==> MAC-based, reset port timer
     */
    if(((dot1x_om_port_operationMode[index] == DOT1X_PORT_OPERATION_MODE_MACBASED) && (mode != DOT1X_PORT_OPERATION_MODE_MACBASED)) ||
       ((dot1x_om_port_operationMode[index] != DOT1X_PORT_OPERATION_MODE_MACBASED) && (mode == DOT1X_PORT_OPERATION_MODE_MACBASED)))
    {
          dot1x_om_PortTimer[index].txWhen = dot1x_om_port_txPeriod[index];
          dot1x_om_PortTimer[index].quietWhile = dot1x_om_port_quietPeriod[index];
          dot1x_om_PortTimer[index].reAuthWhen= dot1x_om_port_reAuthPeriod[index];
    }
    dot1x_om_port_operationMode[index] = mode;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the authenticator's operation mode
 * INPUT:  lport.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *         DOT1X_PORT_OPERATION_MODE_MACBASED  for MAC-based
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_PortOperationMode(UI32_T lport,UI32_T *mode)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_operationMode) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    *mode = dot1x_om_port_operationMode[index];
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE ;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function return SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default SystemOperationMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: SystemOperationMode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortOperationMode(UI32_T lport,UI32_T *mode)
{
    UI32_T result;

    DOT1X_OM_Get_PortOperationMode(lport,mode);
    if ( *mode != DOT1X_DEFAULT_PORT_OPERATION_MODE)
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get port operation mode of 1X configuration
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:   VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *          VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortOperationMode(UI32_T *index,UI32_T *mode)
{
    (*index)++;
    if(*index > DOT1X_MAX_PORT)
        return FALSE;
    return DOT1X_OM_Get_PortOperationMode(*index,mode);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_IsPortBasedMode
 * ---------------------------------------------------------------------
 * PURPOSE : Is the port running on port based mode or not
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : TRUE -- port based mode
 *           FALSE -- non port based mode (may be MAC-based)
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_IsPortBasedMode(UI32_T lport)
{
#if (SYS_CPNT_DOT1X_MACBASED_AUTH == TRUE)
    UI32_T mode;
    if (FALSE == DOT1X_OM_Get_PortOperationMode(lport, &mode))
    {
        lib1x_message(MESS_ERROR_FATAL, "DOT1X_OM_Get_PortOperationMode fail");
        return FALSE;
    }

    return (DOT1X_PORT_OPERATION_MODE_MACBASED == mode) ? FALSE : TRUE;
#else
    return TRUE;
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Set the max allowed MAC number in multi-host mode .
 * INPUT:  lport,count.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortMultiHostMacCount(UI32_T lport,UI32_T count)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_multihost_macCount) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_multihost_macCount[index] = count;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Get the max allowed MAC number in multi-host mode .
 * INPUT:  lport,times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_PortMultiHostMacCount(UI32_T lport,UI32_T *count)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_multihost_macCount) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    *count = dot1x_om_port_multihost_macCount[index];
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE ;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortMultiHostMacCount is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: PortMultiHostMacCount
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortMultiHostMacCount(UI32_T lport,UI32_T *count)
{
    UI32_T result;

    DOT1X_OM_Get_PortMultiHostMacCount(lport,count);
    if ( *count != DOT1X_DEFAULT_MULTI_HOST_MAC_COUNT)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Get the next max allowed MAC number in multi-host mode
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortMultiHostMacCount(UI32_T *index,UI32_T *count)
{
    (*index)++;
    if(*index > DOT1X_MAX_PORT)
        return FALSE;
    return DOT1X_OM_Get_PortMultiHostMacCount(*index,count);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Set_PortReAuthMax
 * ---------------------------------------------------------------------
 * PURPOSE : Set the port's re-auth max
 * INPUT   : lport, reauth_max
 * OUTPUT  : None
 * RETURN  : reauth_max
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortReAuthMax(UI32_T lport, UI32_T reauth_max)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_reauth_max) == FALSE)
    {
        return FALSE;
    }

    if (    (reauth_max < MIN_dot1xAuthMaxReauthReq)
        ||  (MAX_dot1xAuthMaxReauthReq < reauth_max))
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_reauth_max[index] = reauth_max;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_PortReAuthMax
 * ---------------------------------------------------------------------
 * PURPOSE : Get the port's re-auth max
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : reauth_max
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortReAuthMax(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T value;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_reauth_max) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    value = dot1x_om_port_reauth_max[index];
    DOT1X_OM_LeaveCriticalRegion();

    return value;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortTimer
 * ---------------------------------------------------------------------
 * PURPOSE: Set the port's timer
 * INPUT:  lport,timer,type.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortTimer(UI32_T lport,int port_timer,DOT1X_OM_TimerType type)
{
    UI32_T  index = DOT1X_OM_LPORT_TO_INDEX(lport);
    BOOL_T  ret = TRUE;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_PortTimer) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    switch(type)
    {
    case DOT1X_OM_TIMER_AWHILE :
        dot1x_om_PortTimer[index].aWhile = port_timer;
        break;
    case DOT1X_OM_TIMER_QUIETWHILE:
        dot1x_om_PortTimer[index].quietWhile = port_timer;
        break;
    case DOT1X_OM_TIMER_REAUTHWHEN:
        dot1x_om_PortTimer[index].reAuthWhen = port_timer;
        break;
    case DOT1X_OM_TIMER_TXWHEN:
        dot1x_om_PortTimer[index].txWhen = port_timer;
        break;
    case DOT1X_OM_TIMER_PROCESSWHILE:
        dot1x_om_PortTimer[index].processWhile = port_timer;
        break;
    default:
        lib1x_message(MESS_DBG_AUTHNET, "Set Timer type error!\n");
        ret = FALSE;
        break;
    }
    DOT1X_OM_LeaveCriticalRegion();
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortTimer
 * ---------------------------------------------------------------------
 * PURPOSE: Get the port's timer
 * INPUT:  lport,type.
 * OUTPUT: None.
 * RETURN: port timer.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
int DOT1X_OM_Get_PortTimer(UI32_T lport,DOT1X_OM_TimerType type)
{
    int port_timer;
    UI32_T  index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_PortTimer) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    switch(type)
    {
    case DOT1X_OM_TIMER_AWHILE :
        port_timer = dot1x_om_PortTimer[index].aWhile;
        break;
    case DOT1X_OM_TIMER_QUIETWHILE:
        port_timer = dot1x_om_PortTimer[index].quietWhile;
        break;
    case DOT1X_OM_TIMER_REAUTHWHEN:
        port_timer = dot1x_om_PortTimer[index].reAuthWhen;
        break;
    case DOT1X_OM_TIMER_TXWHEN:
        port_timer = dot1x_om_PortTimer[index].txWhen;
        break;
    case DOT1X_OM_TIMER_PROCESSWHILE:
        port_timer = dot1x_om_PortTimer[index].processWhile;
        break;
    default:
        lib1x_message(MESS_DBG_AUTHNET, "Get Timer type error!\n");
        port_timer = 0;
        break;
    }
    DOT1X_OM_LeaveCriticalRegion();
    return port_timer;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Init_PortTimer
 * ---------------------------------------------------------------------
 * PURPOSE: Initialize the port's timer
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Init_PortTimer(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_PortTimer) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_PortTimer[index].aWhile = 0;
    dot1x_om_PortTimer[index].quietWhile = 0;
    dot1x_om_PortTimer[index].reAuthWhen = 0;
    dot1x_om_PortTimer[index].txWhen = 0;
    dot1x_om_PortTimer[index].processWhile = 0;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
}

UI32_T DOT1X_OM_Get_RadiusMsgQid()
{
    return DOT1X_OM_radius_msg_id;
}

BOOL_T DOT1X_OM_Set_RadiusMsgQid(UI32_T msg_id)
{
    DOT1X_OM_radius_msg_id = msg_id ;
    return TRUE;
}

BOOL_T DOT1X_OM_SetDot1xMsgQId(UI32_T dot1x_msgq_id)
{
    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_msgq_id = dot1x_msgq_id;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

BOOL_T DOT1X_OM_GetDot1xMsgQId(UI32_T *dot1x_msgq_id)
{
    DOT1X_OM_EnterCriticalRegion();
    *dot1x_msgq_id = dot1x_om_msgq_id;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

BOOL_T DOT1X_OM_SetTaskServiceFunPtr(UI32_T cookie, UI8_T service_type)
{
    DOT1X_OM_EnterCriticalRegion();
    authorized_result_cookie[service_type] = cookie;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

BOOL_T DOT1X_OM_GetTaskServiceFunPtr(UI32_T *cookie, UI8_T service_type)
{
    BOOL_T ret=FALSE;

    if (NULL != cookie)
    {
        DOT1X_OM_EnterCriticalRegion();
        *cookie = authorized_result_cookie[service_type];
        DOT1X_OM_LeaveCriticalRegion();

        ret = TRUE;
    }
    return ret;
}

/* -----------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_SetPortIntrusionActionStatus
 * -----------------------------------------------------------------------
 * FUNCTION : Set per-port intrusion action
 * INPUT    : lport - port number
 *            value - VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic or
 *                    VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * -----------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_SetPortIntrusionActionStatus(UI32_T lport, UI32_T value)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);

    if (   (VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic != value)
        && (VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan != value))
    {
        return FALSE;
    }

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_intrustion_action_status) == FALSE)
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_port_intrustion_action_status[index] = value;
    DOT1X_OM_LeaveCriticalRegion();

    return TRUE;
}

/* -----------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetPortIntrusionActionStatus
 * -----------------------------------------------------------------------
 * FUNCTION : Get per-port intrusion action
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic or
 *            VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * NOTE     : None
 * -----------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetPortIntrusionActionStatus(UI32_T lport)
{
    UI32_T index = DOT1X_OM_LPORT_TO_INDEX(lport);
    UI32_T value;

    if (DOT1X_OM_IS_VALID_ARRAY_INDEX(index, dot1x_om_port_intrustion_action_status) == FALSE)
    {
        return 0;
    }

    DOT1X_OM_EnterCriticalRegion();
    value = dot1x_om_port_intrustion_action_status[index];
    DOT1X_OM_LeaveCriticalRegion();

    return value;
}

/* ---------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_ConvertAuthStateToPaeState
 * ---------------------------------------------------------------------
 * PURPOSE : Convert state of state machine to backend state
 * INPUT   : state
 * OUTPUT  : pae_value_p
 * RETURN  : None
 * NOTE    : None
 * ---------------------------------------------------------------------
 */
void DOT1X_OM_ConvertAuthStateToPaeState(DOT1X_SM_AUTH_State_T state, UI32_T *pae_value_p)
{
    if (NULL == pae_value_p)
    {
        return;
    }

    switch (state)
    {
        case DOT1X_SM_AUTH_DISABLED_ST:
        case DOT1X_SM_AUTH_PORT_DOWN_ST:
        case DOT1X_SM_AUTH_INIT_ST:
            *pae_value_p = VAL_dot1xAuthPaeState_initialize;
            break;

        case DOT1X_SM_AUTH_DISCONNECTED_ST:
            *pae_value_p = VAL_dot1xAuthPaeState_disconnected;
            break;

        case DOT1X_SM_AUTH_AUTO_ST:
        case DOT1X_SM_AUTH_CONNECTING_ST:
        case DOT1X_SM_AUTH_PROTO_UNAWARE_ST:
            *pae_value_p = VAL_dot1xAuthPaeState_connecting;
            break;

        case DOT1X_SM_AUTH_RESPONSE_ST:
        case DOT1X_SM_AUTH_REQUEST_ST:
        case DOT1X_SM_AUTH_EAPSUCCESS_ST:
        case DOT1X_SM_AUTH_EAPFAIL_ST:
        case DOT1X_SM_AUTH_SUCCESS_ST:
        case DOT1X_SM_AUTH_FAIL_ST:
            *pae_value_p = VAL_dot1xAuthPaeState_authenticating;
            break;

        case DOT1X_SM_AUTH_AUTHENTICATED_ST:
            *pae_value_p = VAL_dot1xAuthPaeState_authenticated;
            break;

        case DOT1X_SM_AUTH_ABORTING_ST:
        case DOT1X_SM_AUTH_LOGOFF_ST:
        case DOT1X_SM_AUTH_TIMEOUT_ST:
            *pae_value_p = VAL_dot1xAuthPaeState_aborting;
            break;

        case DOT1X_SM_AUTH_HELD_ST:
            *pae_value_p = VAL_dot1xAuthPaeState_held;
            break;

        case DOT1X_SM_AUTH_FORCE_AUTH_ST:
            *pae_value_p = VAL_dot1xAuthPaeState_forceAuth;
            break;

        case DOT1X_SM_AUTH_FORCE_UNAUTH_ST:
            *pae_value_p = VAL_dot1xAuthPaeState_forceUnauth;
            break;

        default:
            lib1x_message1(MESS_ERROR_FATAL, "Unknown auth pae state: %d", state);
            *pae_value_p = 0;
            break;
    }
}

/* ---------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_ConvertAuthStateToBackendState
 * ---------------------------------------------------------------------
 * PURPOSE : Convert state of state machine to backend state
 * INPUT   : state
 * OUTPUT  : backend_state_p
 * RETURN  : None
 * NOTE    : None
 * ---------------------------------------------------------------------
 */
void DOT1X_OM_ConvertAuthStateToBackendState(DOT1X_SM_AUTH_State_T state, UI32_T *backend_state_p)
{
    if (NULL == backend_state_p)
    {
        return;
    }

    switch (state)
    {
        case DOT1X_SM_AUTH_DISABLED_ST:
        case DOT1X_SM_AUTH_PORT_DOWN_ST:
        case DOT1X_SM_AUTH_INIT_ST:
            *backend_state_p = VAL_dot1xAuthBackendAuthState_initialize;
            break;

        case DOT1X_SM_AUTH_AUTO_ST:
        case DOT1X_SM_AUTH_CONNECTING_ST:
        case DOT1X_SM_AUTH_DISCONNECTED_ST:
        case DOT1X_SM_AUTH_PROTO_UNAWARE_ST:
        case DOT1X_SM_AUTH_AUTHENTICATED_ST:
        case DOT1X_SM_AUTH_HELD_ST:
        case DOT1X_SM_AUTH_ABORTING_ST:
        case DOT1X_SM_AUTH_LOGOFF_ST:
        case DOT1X_SM_AUTH_FORCE_AUTH_ST:
        case DOT1X_SM_AUTH_FORCE_UNAUTH_ST:
        case DOT1X_SM_AUTH_EAPSUCCESS_ST:
        case DOT1X_SM_AUTH_EAPFAIL_ST:
            *backend_state_p = VAL_dot1xAuthBackendAuthState_idle;
            break;

        case DOT1X_SM_AUTH_RESPONSE_ST:
            *backend_state_p = VAL_dot1xAuthBackendAuthState_response;
            break;

        case DOT1X_SM_AUTH_REQUEST_ST:
            *backend_state_p = VAL_dot1xAuthBackendAuthState_request;
            break;

        case DOT1X_SM_AUTH_SUCCESS_ST:
            *backend_state_p = VAL_dot1xAuthBackendAuthState_success;
            break;

        case DOT1X_SM_AUTH_FAIL_ST:
            *backend_state_p = VAL_dot1xAuthBackendAuthState_fail;
            break;

        case DOT1X_SM_AUTH_TIMEOUT_ST:
            *backend_state_p = VAL_dot1xAuthBackendAuthState_timeout;
            break;

        default:
            lib1x_message1(MESS_ERROR_FATAL, "Unknown auth backend state: %d", state);
            *backend_state_p = 0;
            break;
    }
}

/* for 802.1x MIB (IEEE8021-PAE-MIB)
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetPaePortEntry(UI32_T lport, DOT1X_PaePortEntry_T *entry_p)
{
    if (lport < 1 || lport > DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    if (NULL == entry_p)
    {
        return FALSE;
    }

    entry_p->dot1xPaePortNumber = lport;
    entry_p->dot1xPaePortProtocolVersion = LIB1X_EAPOL_VER;

    entry_p->dot1xPaePortCapabilities = L_CVRT_SNMP_BIT_VALUE_32(VAL_dot1xPaePortCapabilities_dot1xPaePortAuthCapable);

    /* MIB dot1xPaePortInitialize description:
     * The initialization control for this Port. Setting this
 	 * attribute TRUE causes the Port to be initialized.
 	 * The attribute value reverts to FALSE once initialization
 	 * has completed.
     */
    entry_p->dot1xPaePortInitialize = VAL_dot1xPaePortInitialize_false;

    /* MIB dot1xPaePortReauthenticate description:
     * The reauthentication control for this port. Setting
 	 * this attribute TRUE causes the Authenticator PAE state
 	 * machine for the Port to reauthenticate the Supplicant.
 	 * Setting this attribute FALSE has no effect.
 	 * This attribute always returns FALSE when it is read.
     */
    entry_p->dot1xPaePortReauthenticate = VAL_dot1xPaePortReauthenticate_false;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextPaePortEntry(UI32_T *lport_p, DOT1X_PaePortEntry_T *entry_p)
{
    if (*lport_p < 0 || *lport_p >= DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    *lport_p = *lport_p + 1;
    return DOT1X_OM_GetPaePortEntry(*lport_p, entry_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetAuthConfigEntry(UI32_T lport, DOT1X_AuthConfigEntry_T *entry_p)
{
    if (lport < 1 || lport > DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    if (NULL == entry_p)
    {
        return FALSE;
    }

    if (VAL_dot1xPaeSystemAuthControl_enabled == DOT1X_OM_Get_SystemAuthControl())
    {
        if (DOT1X_OM_IsPortBasedMode(lport))
        {
            DOT1X_SM_AUTH_Obj_T *sm_p = DOT1X_OM_GetStateMachineObj(lport);

            if (NULL == sm_p)
            {
                return FALSE;
            }

            DOT1X_OM_ConvertAuthStateToPaeState(sm_p->current_state, &entry_p->dot1xAuthPaeState);
            DOT1X_OM_ConvertAuthStateToBackendState(sm_p->current_state, &entry_p->dot1xAuthBackendAuthState);
        }
        else
        {
            /* can not get the state for mac-based
             */
            entry_p->dot1xAuthPaeState = VAL_dot1xAuthPaeState_initialize;
            entry_p->dot1xAuthBackendAuthState = VAL_dot1xAuthBackendAuthState_initialize;
        }
    }
    else
    {
        entry_p->dot1xAuthPaeState = VAL_dot1xAuthPaeState_initialize;
        entry_p->dot1xAuthBackendAuthState = VAL_dot1xAuthBackendAuthState_initialize;
    }

    entry_p->dot1xAuthAdminControlledDirections = DOT1X_OM_Get_AdminCtrlDirections(lport);
    entry_p->dot1xAuthOperControlledDirections = VAL_dot1xAuthOperControlledDirections_both;
    entry_p->dot1xAuthAuthControlledPortStatus = DOT1X_OM_GetPortAuthorized(lport);

    entry_p->dot1xAuthAuthControlledPortControl = DOT1X_OM_Get_PortControlMode(lport);;
    entry_p->dot1xAuthQuietPeriod = DOT1X_OM_Get_PortQuietPeriod(lport);
    entry_p->dot1xAuthTxPeriod = DOT1X_OM_Get_PortTxPeriod(lport);
    entry_p->dot1xAuthSuppTimeout = DOT1X_OM_Get_AuthSuppTimeout(lport);
    entry_p->dot1xAuthServerTimeout = DOT1X_OM_Get_AuthServerTimeout(lport);
    entry_p->dot1xAuthMaxReq = DOT1X_OM_Get_PortMaxReq(lport);
    entry_p->dot1xAuthReAuthPeriod = DOT1X_OM_Get_PortReAuthPeriod(lport);
    entry_p->dot1xAuthReAuthEnabled = DOT1X_OM_Get_PortReAuthEnabled(lport);
    entry_p->dot1xAuthKeyTxEnabled = DOT1X_OM_Get_AuthTxEnabled(lport);

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextAuthConfigEntry(UI32_T *lport_p, DOT1X_AuthConfigEntry_T *entry_p)
{
    if (*lport_p < 0 || *lport_p >= DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    *lport_p = *lport_p + 1;
    return DOT1X_OM_GetAuthConfigEntry(*lport_p, entry_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetAuthStatsEntry(UI32_T lport, DOT1X_AuthStatsEntry_T *entry_p)
{
    DOT1X_AuthStatsEntry_T *stats_p = NULL;

    if (NULL == entry_p)
    {
        return FALSE;
    }

    stats_p = DOT1X_OM_GetAuthStats(lport);
    if (NULL == stats_p)
    {
        return FALSE;
    }

    memcpy(entry_p, stats_p, sizeof(DOT1X_AuthStatsEntry_T));
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextAuthStatsEntry(UI32_T *lport_p, DOT1X_AuthStatsEntry_T *entry_p)
{
    if (*lport_p < 0 || *lport_p >= DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    *lport_p = *lport_p + 1;
    return DOT1X_OM_GetAuthStatsEntry(*lport_p, entry_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetAuthDiagEntry(UI32_T lport, DOT1X_AuthDiagEntry_T *entry_p)
{
    DOT1X_SM_AUTH_Obj_T *sm_p;

    if (lport < 1 || lport > DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    if (!DOT1X_OM_IsPortBasedMode(lport))
    {
        return FALSE;
    }

    if (NULL == entry_p)
    {
        return FALSE;
    }

    sm_p = DOT1X_OM_GetStateMachineObj(lport);
    if (NULL == sm_p)
    {
        return FALSE;
    }

    memcpy(entry_p, &sm_p->auth_diag_entry, sizeof(sm_p->auth_diag_entry));

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextAuthDiagEntry(UI32_T *lport_p, DOT1X_AuthDiagEntry_T *entry_p)
{
    if (*lport_p < 0 || *lport_p >= DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    *lport_p = *lport_p + 1;
    return DOT1X_OM_GetAuthDiagEntry(*lport_p, entry_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthSessionStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthSessionStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetAuthSessionStatsEntry(UI32_T lport, DOT1X_AuthSessionStatsEntry_T *entry_p)
{
    DOT1X_SM_AUTH_Obj_T *sm_p;

    if (lport < 1 || lport > DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    if (!DOT1X_OM_IsPortBasedMode(lport))
    {
        return FALSE;
    }

    if (NULL == entry_p)
    {
        return FALSE;
    }

    sm_p = DOT1X_OM_GetStateMachineObj(lport);
    if (NULL == sm_p)
    {
        return FALSE;
    }

    sprintf(sm_p->auth_session_stats_entry.dot1xAuthSessionId, "(%ld, %02X-%02X-%02X-%02X-%02X-%02X)",
        lport,
        sm_p->dot1x_packet_t.src_mac[0],
        sm_p->dot1x_packet_t.src_mac[1],
        sm_p->dot1x_packet_t.src_mac[2],
        sm_p->dot1x_packet_t.src_mac[3],
        sm_p->dot1x_packet_t.src_mac[4],
        sm_p->dot1x_packet_t.src_mac[5]);

    strncpy(entry_p->dot1xAuthSessionId, sm_p->auth_session_stats_entry.dot1xAuthSessionId, DOT1X_SESSION_ID_LENGTH);
    entry_p->dot1xAuthSessionId[DOT1X_SESSION_ID_LENGTH] = '\0';

    entry_p->dot1xAuthSessionAuthenticMethod = sm_p->auth_session_stats_entry.dot1xAuthSessionAuthenticMethod;

    if (DOT1X_SM_AUTH_AUTHENTICATED_ST == sm_p->current_state)
    {
        UI32_T current_time;
        SWDRV_IfTableStats_T if_table_stats;

        current_time = SYS_TIME_GetSystemTicksBy10ms();
        entry_p->dot1xAuthSessionTime = current_time - sm_p->session_start_time;

        NMTR_MGR_GetIfTableStats(lport, &if_table_stats);
        entry_p->dot1xAuthSessionOctetsRx = if_table_stats.ifInOctets - sm_p->if_table_stats.ifInOctets;
        entry_p->dot1xAuthSessionOctetsTx = if_table_stats.ifOutOctets - sm_p->if_table_stats.ifOutOctets;

        entry_p->dot1xAuthSessionFramesRx =
            (if_table_stats.ifInUcastPkts - sm_p->if_table_stats.ifInUcastPkts) +
            (if_table_stats.ifInNUcastPkts - sm_p->if_table_stats.ifInNUcastPkts);

        entry_p->dot1xAuthSessionFramesTx =
            (if_table_stats.ifOutUcastPkts - sm_p->if_table_stats.ifOutUcastPkts) +
            (if_table_stats.ifOutNUcastPkts - sm_p->if_table_stats.ifOutNUcastPkts);
    }
    else
    {
        entry_p->dot1xAuthSessionTime = sm_p->auth_session_stats_entry.dot1xAuthSessionTime;
        entry_p->dot1xAuthSessionOctetsRx = sm_p->if_table_stats.ifInOctets;
        entry_p->dot1xAuthSessionOctetsTx = sm_p->if_table_stats.ifOutOctets;

        entry_p->dot1xAuthSessionFramesRx =
            sm_p->if_table_stats.ifInUcastPkts +
            sm_p->if_table_stats.ifInNUcastPkts;

        entry_p->dot1xAuthSessionFramesTx =
            sm_p->if_table_stats.ifOutUcastPkts +
            sm_p->if_table_stats.ifOutNUcastPkts;
    }

    entry_p->dot1xAuthSessionTerminateCause = sm_p->auth_session_stats_entry.dot1xAuthSessionTerminateCause;
    entry_p->dot1xAuthSessionEverSuccess = sm_p->is_ever_pass_auth;

    strncpy(entry_p->dot1xAuthSessionUserName, sm_p->auth_session_stats_entry.dot1xAuthSessionUserName, DOT1X_USERNAME_LENGTH);
    entry_p->dot1xAuthSessionUserName[DOT1X_USERNAME_LENGTH] = '\0';

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextAuthSessionStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthSessionStatsTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextAuthSessionStatsEntry(UI32_T *lport_p, DOT1X_AuthSessionStatsEntry_T *entry_p)
{
    if (*lport_p < 0 || *lport_p >= DOT1X_MAX_PORT)
    {
        return FALSE;
    }

    *lport_p = *lport_p + 1;
    return DOT1X_OM_GetAuthSessionStatsEntry(*lport_p, entry_p);
}

/*****************For CLI Show do1x ****************************************/
/*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_Global_Parameters
 *---------------------------------------------------------------------------
 * PURPOSE:  Get dot1x global parameters.
 * INPUT:    global_parameters pointer.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 void DOT1X_OM_Get_Global_Parameters(DOT1X_Global_Parameters_T * global_parameters)
 {
   BOOL_T reauth_enabled;
    reauth_enabled = DOT1X_OM_Get_GlobalReauthEnabled();
    if ( reauth_enabled == TRUE)
       global_parameters -> reauth_enabled = VAL_dot1xPaePortReauthenticate_true;
    else
       global_parameters -> reauth_enabled = VAL_dot1xPaePortReauthenticate_false;
    global_parameters -> reauth_period  = DOT1X_OM_Get_GlobalReauthPeriod();
    global_parameters -> quiet_period   = DOT1X_OM_Get_GlobalQuietPeriod();
    global_parameters -> tx_period      = DOT1X_OM_Get_GlobalTxPeriod();
    global_parameters -> supp_timeout   = DOT1X_DEFAULT_SUPPLICANT_TIMEOUT;
    global_parameters -> server_timeout = RADIUS_POM_Get_Request_Timeout() * RADIUS_POM_Get_Retransmit_Times()/*DOT1X_DEFAULT_SERVER_TIMEOUT*/;
    global_parameters -> reauth_max     = DOT1X_DEFAULT_REAUTHMAX;
    global_parameters -> max_req        = DOT1X_OM_Get_GlobalMaxReq();
  return;
 }

/* ------------------------------------------------------------------------
 * FUNCTION NAME: DOT1X_OM_Get_Port_Details
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_Port_Details(UI32_T lport, DOT1X_PortDetails_T *port_details_p)
{
    DOT1X_SM_AUTH_Obj_T *sm_p;

    if (NULL == port_details_p)
    {
        return FALSE;
    }

    if (!DOT1X_OM_IsPortBasedMode(lport))
    {
        return FALSE;
    }

    sm_p = DOT1X_OM_GetStateMachineObj(lport);
    if (NULL == sm_p)
    {
        return FALSE;
    }

    port_details_p->port_control = DOT1X_OM_Get_PortControlMode(lport);
    port_details_p->status = sm_p->port_status;

    memcpy(port_details_p->supplicant, sm_p->dot1x_packet_t.src_mac, sizeof(sm_p->dot1x_packet_t.src_mac));
    port_details_p->current_id = sm_p->current_id;

    DOT1X_OM_ConvertAuthStateToPaeState(sm_p->current_state, &port_details_p->apsm_state);
    DOT1X_OM_ConvertAuthStateToBackendState(sm_p->current_state, &port_details_p->basm_state);

    port_details_p->reauth_count = sm_p->reauth_count;
    port_details_p->request_count = sm_p->request_count;
    port_details_p->identifier = sm_p->id_from_server;
    port_details_p->resm_state = resm_Initialize;

    return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME: DOT1X_OM_GetNextPortDetails
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : index = 0 to get first
 * ------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextPortDetails(UI32_T lport, UI32_T *index_p, DOT1X_PortDetails_T *details_p)
{
    DOT1X_SM_AUTH_Obj_T *obj_p;

    if (NULL == details_p)
    {
        return FALSE;
    }

    obj_p = DOT1X_OM_GetNextWorkingSMObj(lport, index_p);
    if (NULL == obj_p)
    {
        return FALSE;
    }

    details_p->port_control = DOT1X_OM_Get_PortControlMode(lport);

    details_p->status = obj_p->port_status;
    memcpy(details_p->supplicant, obj_p->dot1x_packet_t.src_mac, SYS_ADPT_MAC_ADDR_LEN);
    details_p->current_id = obj_p->current_id;

    DOT1X_OM_ConvertAuthStateToPaeState(obj_p->current_state, &details_p->apsm_state);
    DOT1X_OM_ConvertAuthStateToBackendState(obj_p->current_state, &details_p->basm_state);

    details_p->reauth_count = obj_p->reauth_count;
    details_p->request_count = obj_p->request_count;
    details_p->identifier = obj_p->id_from_server;
    details_p->resm_state = resm_Initialize;

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_SetEapolPassThrough
 *------------------------------------------------------------------------
 * FUNCTION: Set status of EAPOL frames pass-through
 * INPUT   : status - DOT1X_OM_EAPOL_PASS_THRU_ENABLED(1)
 *                    DOT1X_OM_EAPOL_PASS_THRU_DISABLED(2)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_SetEapolPassThrough(DOT1X_OM_EapolPassThru_T status)
{
#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
    if ((status != DOT1X_OM_EAPOL_PASS_THRU_ENABLED)
        && (status != DOT1X_OM_EAPOL_PASS_THRU_DISABLED))
    {
        return FALSE;
    }

    DOT1X_OM_EnterCriticalRegion();
    dot1x_om_eapol_pass_through = status;
    DOT1X_OM_LeaveCriticalRegion();
    return TRUE;
#else
    return FALSE;
#endif
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetEapolPassThrough
 *------------------------------------------------------------------------
 * FUNCTION: Get the status of EAPOL frames pass-through
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : status - DOT1X_OM_EAPOL_PASS_THRU_ENABLED(1)
 *                    DOT1X_OM_EAPOL_PASS_THRU_DISABLED(2)
 * NOTE    :
 *------------------------------------------------------------------------
 */
DOT1X_OM_EapolPassThru_T DOT1X_OM_GetEapolPassThrough()
{
#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
    UI32_T  status;

    DOT1X_OM_EnterCriticalRegion();
    status = dot1x_om_eapol_pass_through;
    DOT1X_OM_LeaveCriticalRegion();

    return status;
#else
    return DOT1X_OM_EAPOL_PASS_THRU_DISABLED;
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DOT1X_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    DOT1X_OM_IPCMsg_T *msg_data_p;
    UI32_T cmd;

    if(ipcmsg_p==NULL)
    {
        return FALSE;
    }

    msg_data_p= (DOT1X_OM_IPCMsg_T*)ipcmsg_p->msg_buf;
    cmd = msg_data_p->type.cmd;

    switch(cmd)
    {
        case DOT1X_OM_IPCCMD_GET_PORTMAXREQ:
            msg_data_p->type.result_ui32=DOT1X_OM_Get_PortMaxReq(msg_data_p->data.ui32_v);
            break;

        case DOT1X_OM_IPCCMD_GET_GETRUNNING_MAXREQ:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_MaxReq(&msg_data_p->data.ui32_v);
            break;

        case DOT1X_OM_IPCCMD_GET_RUNNING_PORT_REAUTH_MAX:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_PortReAuthMax(msg_data_p->data.lport_ui32arg.lport,
                &msg_data_p->data.lport_ui32arg.ui32arg);
            break;

        case DOT1X_OM_IPCCMD_GETRUNNING_PORTMAXREQ:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_PortMaxReq(msg_data_p->data.lport_ui32arg.lport,
                &msg_data_p->data.lport_ui32arg.ui32arg);
            break;
        case DOT1X_OM_IPCCMD_GET_PORTCONTROLMODE:
            msg_data_p->type.result_ui32=DOT1X_OM_Get_PortControlMode(msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_PORTCONTROLMODE:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_PortControlMode(msg_data_p->data.lport_ui32arg.lport,
                &msg_data_p->data.lport_ui32arg.ui32arg);
            break;
        case DOT1X_OM_IPCCMD_GET_PORTREAUTHENABLED:
            msg_data_p->type.result_ui32=DOT1X_OM_Get_PortReAuthEnabled(msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_PORTREAUTHENABLED:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_PortReAuthEnabled(msg_data_p->data.lport_ui32arg.lport,
                &msg_data_p->data.lport_ui32arg.ui32arg);
            break;
        case DOT1X_OM_IPCCMD_GET_PORTQUIETPERIOD:
            msg_data_p->type.result_ui32=DOT1X_OM_Get_PortQuietPeriod(msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GET_GETRUNNING_QUIETPERIOD:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_QuietPeriod(&msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_PORTQUIETPERIOD:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_PortQuietPeriod(msg_data_p->data.lport_ui32arg.lport,
                &msg_data_p->data.lport_ui32arg.ui32arg);
            break;
        case DOT1X_OM_IPCCMD_GET_PORTREAUTHPERIOD:
            msg_data_p->type.result_ui32=DOT1X_OM_Get_PortReAuthPeriod(msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_REAUTHPERIOD:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_ReAuthPeriod(&msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_PORTREAUTHPERIOD:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_PortReAuthPeriod(msg_data_p->data.lport_ui32arg.lport,
                &msg_data_p->data.lport_ui32arg.ui32arg);
            break;
        case DOT1X_OM_IPCCMD_GET_PORTTXPERIODD:
            msg_data_p->type.result_ui32=DOT1X_OM_Get_PortTxPeriod(msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_TXPERIOD:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_TxPeriod(&msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_PORTTXPERIOD:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_PortTxPeriod(msg_data_p->data.lport_ui32arg.lport,
                &msg_data_p->data.lport_ui32arg.ui32arg);
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_AUTH_SUPP_TIMEOUT:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_AuthSuppTimeout(msg_data_p->data.lport_ui32arg.lport,
                &msg_data_p->data.lport_ui32arg.ui32arg);
            break;
        case DOT1X_OM_IPCCMD_GET_SYSTEMAUTHCONTROL:
            msg_data_p->type.result_ui32=DOT1X_OM_Get_SystemAuthControl();
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_SYSTEMAUTHCONTROL:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_SystemAuthControl(&msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GET_PAE_PORT_ENTRY:
            msg_data_p->type.result_bool=DOT1X_OM_GetPaePortEntry(msg_data_p->data.lport_paeportentry.lport,
                &msg_data_p->data.lport_paeportentry.paeportentry);
            break;
        case DOT1X_OM_IPCCMD_GET_NEXT_PAE_PORT_ENTRY:
            msg_data_p->type.result_bool=DOT1X_OM_GetNextPaePortEntry(&msg_data_p->data.lport_paeportentry.lport,
                &msg_data_p->data.lport_paeportentry.paeportentry);
            break;
        case DOT1X_OM_IPCCMD_GET_AUTH_CONFIG_ENTRY:
            msg_data_p->type.result_bool=DOT1X_OM_GetAuthConfigEntry(msg_data_p->data.lport_authconfigentry.lport,
                &msg_data_p->data.lport_authconfigentry.authconfigentry);
            break;
        case DOT1X_OM_IPCCMD_GET_NEXT_AUTH_CONFIG_ENTRY:
            msg_data_p->type.result_bool=DOT1X_OM_GetNextAuthConfigEntry(&msg_data_p->data.lport_authconfigentry.lport,
                &msg_data_p->data.lport_authconfigentry.authconfigentry);
            break;
        case DOT1X_OM_IPCCMD_GET_AUTH_STATS_ENTRY:
            msg_data_p->type.result_bool=DOT1X_OM_GetAuthStatsEntry(msg_data_p->data.lport_authstateentry.lport,
                &msg_data_p->data.lport_authstateentry.authstateentry);
            break;
        case DOT1X_OM_IPCCMD_GET_NEXT_AUTH_STATS_ENTRY:
            msg_data_p->type.result_bool=DOT1X_OM_GetNextAuthStatsEntry(&msg_data_p->data.lport_authstateentry.lport,
                &msg_data_p->data.lport_authstateentry.authstateentry);
            break;
        case DOT1X_OM_IPCCMD_GET_AUTH_DIAG_ENTRY:
            msg_data_p->type.result_bool=DOT1X_OM_GetAuthDiagEntry(msg_data_p->data.lport_authdiagentry.lport,
                &msg_data_p->data.lport_authdiagentry.authdiagentry);
            break;
        case DOT1X_OM_IPCCMD_GET_NEXT_AUTH_DIAG_ENTRY:
            msg_data_p->type.result_bool=DOT1X_OM_GetNextAuthDiagEntry(&msg_data_p->data.lport_authdiagentry.lport,
                &msg_data_p->data.lport_authdiagentry.authdiagentry);
            break;
        case DOT1X_OM_IPCCMD_GET_GLOBAL_PARAMETERS:
            DOT1X_OM_Get_Global_Parameters(&msg_data_p->data.global_parameters);
            break;
        case DOT1X_OM_IPCCMD_GET_PORT_AUTHORIZED:
            msg_data_p->type.result_ui32=DOT1X_OM_GetPortAuthorized(msg_data_p->data.ui32_v);
            break;
        case DOT1X_OM_IPCCMD_GET_PORT_DETAILS:
            msg_data_p->type.result_bool=DOT1X_OM_Get_Port_Details(msg_data_p->data.lport_portdetails.lport,
                &msg_data_p->data.lport_portdetails.port_details);
            break;
        case DOT1X_OM_IPCCMD_GET_NEXT_PORT_DETAILS:
            msg_data_p->type.result_bool=DOT1X_OM_GetNextPortDetails(
                msg_data_p->data.lport_macidx_portdetails.lport,
                &msg_data_p->data.lport_macidx_portdetails.mac_index,
                &msg_data_p->data.lport_macidx_portdetails.port_details);
            break;
        case DOT1X_OM_IPCCMD_GETRUNNING_PORTMULTIHOSTMACCOUNT:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_PortMultiHostMacCount(msg_data_p->data.lport_ui32arg.lport,
                &msg_data_p->data.lport_ui32arg.ui32arg);
            break;
        case DOT1X_OM_IPCCMD_GET_PORTREAUTHMAX:
            msg_data_p->type.result_ui32=DOT1X_OM_Get_PortReAuthMax(msg_data_p->data.ui32_v);
            break;

        case DOT1X_OM_IPCCMD_GETRUNNING_PORTOPERATIONMODE:
            msg_data_p->type.result_ui32=DOT1X_OM_GetRunning_PortOperationMode(msg_data_p->data.lport_mode.lport,
                &msg_data_p->data.lport_mode.mode);
            break;

        default:
            msg_data_p->type.result_ui32=0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;
    }

    /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
     */
    if(cmd<DOT1X_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

